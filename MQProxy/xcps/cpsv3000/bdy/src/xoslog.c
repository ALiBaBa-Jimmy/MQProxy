/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xoslog.c
**   author          date              modification
     maliming        2013.12.17          重新整理,采用tcp进行远程保存
**************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

/**************************Win32 Headfile*************************/
#ifdef  XOS_WIN32
#include <direct.h>
#include <io.h>
#endif /*WINDOWS*/
/**************************************************************/

/**************************Linux Solar HeadFile****************/
#if ( defined (XOS_LINUX) || defined (XOS_SOLARIS) )
#include <unistd.h>
#include <sys/stat.h>
//modified for xos log optimization zenghaixin 20100817
#include <sys/vfs.h>
#include <dirent.h>   
#include <sys/types.h>  
//end modified for xos log optimization zenghaixin 20100817
#endif
/**************************************************************/

#if defined XOS_VXWORKS
#include <usrLib.h>
#endif

#include "xoslog.h"
#include "xosencap.h"

#define STACKINFOLOG "syslog.txt"
#define TIME_STR_LEN      (128)
#define MAX_LOGHEAD_LEN   (512)
#define MAX_TCP_BUF_LEN   (5000)    /*最大报文缓存*/
#define MAX_LOG_TCP_DATA_LEN (512+11)  /*最大报文长度*/
#define MIN_LOG_TCP_DATA_LEN (11)      /*最小报文长度*/
#define MAX_SHAKE_TIMEOUT (60000)      /*握手定时器长度*/
#define MAX_RETCP_TIMEOUT  (4000)      /*tcp重连间隔*/
#define LOG_MAX_SHAKE_SEND (3)        /*最大握手超时次数*/
#define LOG_TEST_MSG ("test")
#define LOG_XOS_XML ("xos.xml")       /*xos.xml配置文件*/
#define LOG_CONFIG_NUM (5)
#define LOG_ADDR_LEN (32)
const char *ppLogTip[] = 
    {"pl_mini","pl_debug","pl_info","pl_warn","pl_err","pl_exception","pl_log", "pl_max"};

const char *ppLogLinkStatus[] = {"null","init","start","connecting","connected", "stop"};
/*远程日志服务器链路状态*/
typedef enum 
{
    LOG_LINK_NULL = 0,
    LOG_LINK_INIT,
    LOG_LINK_START,
    LOG_LINK_CONNECTING,
    LOG_LINK_CONNECTED,
    LOG_LINK_STOP
}LINK_STATUS;

/*定时器类型*/
typedef enum
{
    LOG_INVALID = 0,
    LOG_TIMER_SHAKE = 1,    /*握手定时器*/
    LOG_TIMER_TEST = 2,
    LOG_TIMER_RETCP = 3     /*tcp重连*/
}LOG_TIMER_TYPE;


e_PRINTLEVEL   g_lPrintToLogLevel            = PL_ERR; /*控制输出到日志文件的信息级别*/
XU8 uAutoTest = 0;                 /*自动测试*/

t_LogCfg g_LogCfg = {0};           /*log服务端参数配置*/
t_LogStatus g_LogStatus = {{0}, {0}, NULL, 0, 0};     /*LOG状态*/

XBOOL g_logInitFlag = (XBOOL)FALSE;      /*是否已经初始化*/

/*链路状态*/
const XPOINT g_LogLinkNumber = 1;       /*链路号*/
HLINKHANDLE g_LogLinkHaddle = NULL;   /*ntl分配的链路句柄*/ 
XU8 g_serverLinkStatus = 0;           /*日志服务器链路状态*/

/*本地全局TCP消息数据缓冲*/
XSTATIC XU16 usSrvIpDataLen = 0;
XSTATIC XU8  ucSrvIpDataBuf[MAX_TCP_BUF_LEN];

XSTATIC PTIMER g_LogShakeTimer;       /*握手定时器句柄*/
XSTATIC PTIMER g_LogTcpReconnTimer;       /*tcp重连接定时器*/
XS32   g_LogexpireTimes;    /*定时器超时次数*/


XU32 g_AutotestInterval = 200;        /*自动测试间隔*/

XU32 g_LogShakeSend = 0;              /*连续发送握手次数*/

XU32   g_LogRecordSn = 0;             /*日志记录序列号*/

t_MSGFIDCHK g_msgqFidChk[FID_MAX];    /*每个模块的流量统计*/

typedef struct{
t_SOCKSET readSet;
t_SOCKSET writeSet;
t_XOSMUTEXID mutex;

}LogFdSet;

LogFdSet g_LogFdSet;
/*链路管理*/
logTcpSocket g_Log_Link;
t_XOSTASKID g_LogRecvTaskId;
t_XOSMUTEXID g_LogWriteMutex;






/********************************** 
函数名称    : Log_GetRecordSn
作者        : 
设计日期    : 
功能描述    : 生成日志记录的序列号
参数        : 无
返回值      : XU32  
************************************/
XU32 Log_GetRecordSn()
{
    return ++g_LogRecordSn;
}

/********************************** 
函数名称    : LOG_CheckFidFlow
作者        : 
设计日期    : 
功能描述    : 模块流量检查函数
参数        : XU32 chk_fid
返回值      : XS32  
************************************/
XS32  Log_CheckFidFlow(XU32 chk_fid)
{
    time_t  l_now = 0;
    time_t  time_delta = 0;
    XU32  peak_packet = 0;

    if(chk_fid >= FID_MAX)
    {
      return XSUCC;
    }   

    /*流量检测为0，则不进行过载检测*/
    if(0 == g_msgqFidChk[chk_fid].fid_msgmax)
    {
        return XSUCC;
    }

    g_msgqFidChk[chk_fid].fid_msgcount++;


    l_now = time(XNULL);

    time_delta = l_now - (time_t)(g_msgqFidChk[chk_fid].fid_msgchktime);

    /*相差30秒才有下一个报文，过载自动解除*/
    if(time_delta >30 )
    {
        /*短期峰值处理0.5分钟,计数器清零,赋当前时间*/
        g_msgqFidChk[chk_fid].fid_msgcount = 0;
        g_msgqFidChk[chk_fid].fid_msgfilter = 0;
        g_msgqFidChk[chk_fid].fid_msgpeakcount = 0;
        g_msgqFidChk[chk_fid].fid_msgchktime = (XU32)l_now;        
        return XSUCC;
    } 

    /*2 second check,limit fid_msgmax error packet*/
    if(time_delta <2)
    {
        if(g_msgqFidChk[chk_fid].fid_msgcount <= g_msgqFidChk[chk_fid].fid_msgmax)
        {
            /*未超时,低载*/
            return XSUCC;
        }
        else
        {
            /*未超时,过载*/
            g_msgqFidChk[chk_fid].fid_msgfilter++;

            if((g_msgqFidChk[chk_fid].fid_msgcount) % FID_CHECK_NUM == 0)
            {
                if(time_delta >0 )
                {
                    peak_packet = (XU32)g_msgqFidChk[chk_fid].fid_msgcount/(XU32)time_delta;
                    if(g_msgqFidChk[chk_fid].fid_msgpeakcount < peak_packet)
                    {
                        g_msgqFidChk[chk_fid].fid_msgpeakcount = peak_packet;
                    }                  
                }
            }

            XOS_CliInforPrintf("\r\nLog_CheckFidFlow overload %d\r\n", chk_fid);
            return XERROR;
        }
    }
    else
    {
        if(g_msgqFidChk[chk_fid].fid_msgcount <= g_msgqFidChk[chk_fid].fid_msgmax)
        {
            /*超时,低载，自动解除*/
            g_msgqFidChk[chk_fid].fid_msgcount = 0;
            g_msgqFidChk[chk_fid].fid_msgfilter = 0;
            g_msgqFidChk[chk_fid].fid_msgpeakcount = 0;
            g_msgqFidChk[chk_fid].fid_msgchktime = (XU32)l_now;
            return XSUCC;
        }
        else
        {
            /*超时,继续过载*/
            g_msgqFidChk[chk_fid].fid_msgfilter++;
            if((g_msgqFidChk[chk_fid].fid_msgcount) % FID_CHECK_NUM == 0)
            {
                if(time_delta > 0)
                {
                    peak_packet = (XU32)g_msgqFidChk[chk_fid].fid_msgcount/(XU32)time_delta;
                    if(g_msgqFidChk[chk_fid].fid_msgpeakcount < peak_packet)
                    {
                        g_msgqFidChk[chk_fid].fid_msgpeakcount = peak_packet;
                    }                  
                }
            }

            XOS_CliInforPrintf("\r\nLog_CheckFidFlow overload %d\r\n", chk_fid);

            return XERROR;
        }
    }
}


/*review ok*/
/********************************** 
函数名称    : Log_CliCommandInit
作者        : Jeff.Zeng
设计日期    : 2008年3月21日
功能描述    : 
返回值      : XSTATIC XS32 
************************************/
XS32 Log_CliCommandInit(int cmdMode)
{
    XS32  ret=0,ret1=0,reg_result;
    ret1 = XOS_RegistCmdPrompt( cmdMode, "plat", "plat", "no parameter" );
    if ( XERROR >= ret1 )
    {
       XOS_CliInforPrintf("\r\nLog CliInit,call xos_RegistCmdPrompt failed,error num=%d\r\n",ret);
       return XERROR;
    }
    ret = XOS_RegistCmdPrompt(ret1,"log","-d- log","no parameter");
    if ( 0 >= ret1)
    {
        XOS_CliInforPrintf("\r\nfailed to register log command prompt\r\n");
        return XERROR;
    }

    /*设置写入到log的打印级别*/
    reg_result = XOS_RegistCommand(ret, Log_CmdSetLogLevel,
                "setprinttologlev", "set output to log file filter level",
                "设置打印到log的打印级别\r\n"        \
                "SYNOPSIS:setprinttologlev fid level\r\n"\
                "example:setprinttologlev 3   3    \r\n" \
                "PL_MIN = 0\r\n"                 \
                "PL_DBG = 1\r\n"                 \
                "PL_INFO = 2\r\n"                \
                "PL_WARN = 3\r\n"                \
                "PL_ERR = 4\r\n"                 \
                "PL_EXP = 5\r\n"                 \
                "PL_LOG = 6\r\n");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for setprinttologlev failed,error num=%d\r\n",reg_result);
    }
    

    /*重置log服务器地址*/
    reg_result = XOS_RegistCommand(ret, Log_CmdRestartLink,
                 "restartserver", "delete old log connection and build new one",
                 "example:restartserver localport remoteip remoteport");
    if ( XERROR >= reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for logconn failed\r\n");
    }

    /*显示当着链路状态与配置*/
    reg_result = XOS_RegistCommand(ret,Log_CmdShowInfo,
                 "showloglinkinfo", "show log configuration","example:showloglinkinfo");
    if ( XERROR >= reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for loginfo failed,error num=%d\r\n",reg_result);
    }

    /*设置最大流量检测*/
    reg_result = XOS_RegistCommand(ret, Log_CmdSetFidCheck,
                "setfidcheck", "set given fid peak msg count limit",
                "example:setfidcheck fid msg_max\r\n if msg_max=0, no check ");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for setfidcheck failed,error num=%d\r\n",reg_result);
    }


    /*显示指定fid流量情况*/
    reg_result = XOS_RegistCommand(ret,Log_CmdShowFidCheck,
                "showfidcheck", "show fid check",
                "example:showfidcheck [fid]  \r\nfid=0 show all");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for showfidcheck failed,error num=%d\r\n",reg_result);
    }

    /*关闭远程日志*/
    reg_result = XOS_RegistCommand(ret,Log_CmdCloseRemoteLog,
               "closeremotelog", "close remote log",
               "example:closeremotelog");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for closeremotelog failed,error num=%d\r\n",reg_result);
    }


    /*开启远程日志*/
    reg_result = XOS_RegistCommand(ret,Log_CmdOpenRemoteLog,
              "openremotelog", "open remote log",
              "example:openremotelog");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for openremotelog failed,error num=%d\r\n",reg_result);
    }

    /*显示指定模块的log级别*/
    reg_result = XOS_RegistCommand(ret,Log_CmdShowFidLevel,
                  "showfidlevel", "show fid level",
                  "example:showfidlevel fid");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for showfidlevel failed,error num=%d\r\n",reg_result);
    }

    

    return XSUCC;
}



/************************************************************************
 函数名:Log_SendShakeReqToRemote
 功能:  发送握手请求报文
 输入:  pBackPara 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 Log_SendShakeReqToRemote(void)
{
    t_LogShakeReq LogShakeReq;

    LogShakeReq._tHead._nBeginFlag = XOS_HtoNs(LOGDATA_BEGIN_FLAG);
    LogShakeReq._tHead._nSize = XOS_HtoNs(sizeof(t_LogShakeRsp));
    LogShakeReq._tHead._nType = e_SHAKEREQ;
    LogShakeReq._tHead._nSn = XOS_HtoNl(0);
    LogShakeReq._nEndFlag = XOS_HtoNs(LOGDATA_END_FLAG);

    return Log_SendTcpData((XCHAR*)&LogShakeReq, sizeof(LogShakeReq));    
}

/************************************************************************
 函数名:Log_SendShakeRspToRemote
 功能:  发送握手响应报文
 输入:  pBackPara 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 Log_SendShakeRspToRemote(void)
{
    t_LogShakeRsp LogShakeRsp;

    LogShakeRsp._tHead._nBeginFlag = XOS_HtoNs(LOGDATA_BEGIN_FLAG);
    LogShakeRsp._tHead._nSize = XOS_HtoNs(sizeof(t_LogShakeRsp));
    LogShakeRsp._tHead._nType = e_SHAKERSP;
    LogShakeRsp._tHead._nSn = XOS_HtoNl(0);
    LogShakeRsp._nEndFlag = XOS_HtoNs(LOGDATA_END_FLAG);

    return Log_SendTcpData((XCHAR*)&LogShakeRsp, sizeof(t_LogShakeRsp));

}


/************************************************************************
 函数名:Log_SendDataToRemote
 功能:  发送日志报文
 输入:  pBackPara 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 Log_SendDataToRemote(XU32 logid, e_PRINTLEVEL infotype, XCONST XCHAR *msg_out, XU16 msgLen)
{
    t_LogDataHead *logDataHead = NULL;
    XCHAR *pBuffer = NULL;
    XU16 *pEnd = NULL;

    XOS_UNUSED(infotype);

    if(NULL == msg_out || msgLen > LOG_ITEM_MAX) 
    {
        XOS_CliInforPrintf("\r\nLog_SendDataToRemote()->msg_out is null\r\n");
        return XERROR;
    }

    /*链路不正常*/
    if(LOG_LINK_CONNECTED != g_serverLinkStatus)
    {
        //XOS_CliInforPrintf("\r\nLog_SendDataToRemote()->LOG_LINK_CONNECTED != g_serverLinkStatus[%d]\r\n", g_serverLinkStatus);
        return XERROR;
    }
    
    /*流量检测*/
    if(XSUCC != Log_CheckFidFlow(logid))
    {
        return XERROR;
    }

    /*申请消息内容*/
    pBuffer = (XCHAR*)XOS_MemMalloc(FID_LOG, sizeof(t_LogDataHead) + msgLen + sizeof(t_LogDataTail));

    logDataHead = (t_LogDataHead*)pBuffer;
    if(NULL == logDataHead)
    {
        XOS_CliInforPrintf("\r\nLog_SendDataToRemote()->XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    logDataHead->_nBeginFlag = XOS_HtoNs(LOGDATA_BEGIN_FLAG);
    logDataHead->_nSize = XOS_HtoNs(sizeof(t_LogDataHead) + msgLen + sizeof(t_LogDataTail));
    logDataHead->_nType = e_LOGREQ;
    logDataHead->_nSn = XOS_HtoNl(Log_GetRecordSn());

    memcpy(pBuffer+sizeof(t_LogDataHead), msg_out, msgLen);

    pEnd = (XU16*)(pBuffer+sizeof(t_LogDataHead) + msgLen);

    *pEnd = XOS_HtoNs(LOGDATA_END_FLAG);

    Log_SendTcpData(pBuffer, sizeof(t_LogDataHead)+msgLen+sizeof(t_LogDataTail));

    XOS_MemFree(FID_LOG, pBuffer);

    return XSUCC;
    
}

#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) )
/********************************** 
函数名称    : Log_SpackRefresh
作者        : Zenghaixin
设计日期    : 2013年8月13日
功能描述    : 将总的log日志大小限制在500M以内，
            : 只保留最新的500M日志
参数        : XVOID
返回值      : XS32 
************************************/
XS32 Log_SpackRefresh ( XVOID )
{
    XCHAR szLogDir[XOS_MAX_PATHLEN + 1] = {0};
    XS32 files_in_log_dir = 0; //log目录下文件个数
    XSFILESIZE files_size_in_log_dir = 0; //log目录下log文件总大小
    XS32 files_in_log = 0; //log目录下log文件总个数
    XS32 nIndex = 0, delete_flag = 0;
    XCHAR filename[(XOS_MAX_PATHLEN + 1)*2+1];
    XUFILESIZE size = 0;
    struct dirent **namelist = NULL;

    XOS_MemSet(szLogDir, 0x0, XOS_MAX_PATHLEN + 1);
    
#ifdef XOS_EW_START
    /*生成全路径log 文件名*/
    if( XSUCC != XOS_GetSysPath(szLogDir, sizeof(szLogDir)-1))
    {
        return XERROR;
    }
#endif
    XOS_StrNCat(szLogDir, LOGDIR, XOS_MIN((sizeof(szLogDir)-1)-XOS_StrLen(szLogDir), XOS_StrLen(LOGDIR)));
    
    files_in_log_dir = scandir(szLogDir, &namelist, 0, alphasort);
    //printf("scandir(alphasort) = [%d]\n\n", files_in_log_dir);
    if( (-1) == files_in_log_dir )
    {
        return XERROR;
    }

    for( nIndex = files_in_log_dir - 1; nIndex >=0; nIndex-- )
    {
        //printf("d_name: [%s]\n\n", namelist[index]->d_name);
        //if( namelist[index]->d_type & DT_DIR ) //子目录不处理
        //{
        //    continue;
        //}
        if ( 0 != XOS_StrNcmp(namelist[nIndex]->d_name, "xoslog_", 7 ) )//log文件以xoslog_开头
        {
            free(namelist[nIndex]);   
            continue;
        }
        sprintf(filename,"%s%s", szLogDir, namelist[nIndex]->d_name);   
        //printf("for_test: filename:(%s)\r\n",filename);
        if( 1 == delete_flag )//delete_flag置位，直接删除
        {
            remove(filename);
            //printf("for_test: 删除文件(%s)\r\n", filename);  
            free(namelist[nIndex]);   
            continue;
        }
        if ( XSUCC != XOS_GetFileLen(filename, &size) )
        {
            XOS_CliInforPrintf("\r\nINFO:log get filename(%s) filesize failed!\r\n", filename);
            for(; nIndex >= 0; nIndex--)
            {
                free(namelist[nIndex]);   
            }
             free(namelist);   
            return XERROR;
        }
        files_size_in_log_dir += size;
        files_in_log++;
        
        //printf("for_test: filename:(%s) filesize:(%d)\r\n", filename, size);
        //printf("for_test: files_size_in_log_dir(%d)\r\n", files_size_in_log_dir);  
        //printf("for_test: files_in_log(%d)\r\n", files_in_log);  
        if( (MAX_LOGS_SIZE < files_size_in_log_dir)
         || (MAX_LOGS < files_in_log) )
        //if( 1000 < files_size_in_log_dir )    //for_test
        //if( 2 < files_in_log )    //for_test
        {
            remove(filename);
            //printf("for_test: 删除文件(%s)\r\n", filename);  
            delete_flag = 1;//表示删除之前生成的所有log文件 
        }
        free(namelist[nIndex]);   
    }

    free(namelist);   

    return XSUCC;
}

#elif (defined(XOS_WIN32))
XS32 Log_SpackRefresh ( XVOID )
{  
    /*此函数只支持windows 32位*/
    XCHAR szLogDir[XOS_MAX_PATHLEN + 1] = {0};
    XCHAR szCurDir[(XOS_MAX_PATHLEN + 1)*2+1] = {0};
    XCHAR szTmpDir[(XOS_MAX_PATHLEN + 1)*2+1] = {0};

    XS32 handle = XSUCC;  
    struct _finddata_t fileinfo;  
    XU32 nCount = 0;
    XS32 nTotalSize = 0;

#ifdef XOS_EW_START
    /*生成全路径log 文件名*/
    if( XSUCC != XOS_GetSysPath(szLogDir, sizeof(szLogDir)-1))
    {
        return XERROR;
    }
#endif
    XOS_StrNCat(szLogDir, LOGDIR, XOS_MIN(((sizeof(szLogDir)-1)-strlen(szLogDir)), strlen(LOGDIR)));
    memcpy(szCurDir, szLogDir, strlen(szLogDir));
    XOS_StrNCat(szCurDir, "\\", XOS_MIN((sizeof(szCurDir)-1)-strlen(szCurDir), 1));    
    
    XOS_StrNCat(szLogDir, "\\*.*", XOS_MIN((sizeof(szLogDir)-1)-strlen(szLogDir),4));    

    handle = (XS32)_findfirst(szLogDir, &fileinfo);  
    if(XERROR == handle) 
    {
        return XERROR;    
    }  
    //printf("%s\n",fileinfo.name);  
    while(!_findnext(handle, &fileinfo))  
    {  
        if(strcmp(fileinfo.name, ".")==0||strcmp(fileinfo.name,"..") == 0)  
        {  
           continue;                                                         
        }  
        if(fileinfo.attrib == _A_SUBDIR)  
        {                        
           continue;
        }  
        nCount++;
        nTotalSize += fileinfo.size;
        //printf("%s\tsize:%d\n",fileinfo.name,fileinfo.size);                               
    }  
    _findclose(handle); 

    /*如果数量超出*/
    if(nCount > MAX_LOGS)
    {
        /*删除最早的*/
        nCount -= MAX_LOGS;
        handle = (XS32)_findfirst(szLogDir, &fileinfo);  
        if(XERROR == handle) 
        {
            return XERROR;      
        }
        //printf("%s\n",fileinfo.name);  
        while(nCount > 0 && !_findnext(handle,&fileinfo))  
        {  
            if(strcmp(fileinfo.name,".")==0||strcmp(fileinfo.name,"..")==0)  
            {  
                continue;                                                         
            }  
            if(fileinfo.attrib==_A_SUBDIR)  
            {  
                continue;   
            }  
            nCount--;
            nTotalSize -= fileinfo.size;
            memset(szTmpDir, 0, sizeof(szTmpDir));
            XOS_StrNCat(szTmpDir, szCurDir, XOS_MIN((sizeof(szTmpDir)-1)-strlen(szTmpDir),strlen(szCurDir)));
            XOS_StrNCat(szTmpDir, fileinfo.name, XOS_MIN((sizeof(szTmpDir)-1)-strlen(szTmpDir), strlen(fileinfo.name)));
            XOS_DeleteFile(szTmpDir);
            //printf("%s\tsize:%d\n",fileinfo.name,fileinfo.size);                               
        }  
        _findclose(handle); 
    }

    /*如果总大小超出*/
    if(nTotalSize > MAX_LOGS_SIZE)
    {
        /*删除最早的*/
        nTotalSize -= MAX_LOGS_SIZE;
        handle = (XS32)_findfirst(szLogDir, &fileinfo);  
        if(XERROR == handle) 
        {
            return XERROR;  
        }
        //printf("%s\n",fileinfo.name);  
        while(nTotalSize > 0 && !_findnext(handle,&fileinfo))  
        {  
            if(strcmp(fileinfo.name,".")==0||strcmp(fileinfo.name,"..")==0)  
            {  
                continue;                                                         
            }  
            if(fileinfo.attrib==_A_SUBDIR)  
            {  
                continue;     
            }  
            nCount--;
            nTotalSize -= fileinfo.size;
            memset(szTmpDir, 0, sizeof(szTmpDir));
            XOS_StrNCat(szTmpDir, szCurDir, XOS_MIN((sizeof(szTmpDir)-1)-strlen(szTmpDir), strlen(szCurDir)));
            XOS_StrNCat(szTmpDir, fileinfo.name, XOS_MIN((sizeof(szTmpDir)-1)-strlen(szTmpDir), strlen(fileinfo.name)));
            XOS_DeleteFile(szTmpDir);
            //printf("%s\tsize:%d\n",fileinfo.name,fileinfo.size);                               
        }  
        _findclose(handle); 
    }

    return XSUCC;
}  



#endif


/************************************************************************
 函数名:Log_Write
 功能:  将打印消息发送给log模块自己
 输入:  
        logId    --打印的fid
        level --打印的级别
        logmsg --打印的内容
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明: 此函数不能用xos_trace,只能用printf，否则，会形成函数调用死循环，导致栈溢出。
************************************************************************/
XS8 Log_SendToLog(XU32 logId, e_PRINTLEVEL level, XCHAR* logmsg)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    t_LogLocal logLocal;
    XS32 ret = 0;
    XU32 len = 0;
    
    if(XNULLP == logmsg)
    {
        //printf("\r\nXNULLP == logmsg\r\n");
        return XERROR;
    }

    memset(&logLocal, 0 , sizeof(t_LogLocal));
    logLocal._level = level;
    logLocal._nLogId = logId;

    /*最大日志报文*/
    logLocal._nLen = (XU16)XOS_MIN(strlen(logmsg), LOG_ITEM_MAX);

    logLocal._pszMsg = (XCHAR*)XOS_MemMalloc(FID_LOG, logLocal._nLen);
    if(NULL == logLocal._pszMsg)
    {
        //printf("\r\nXNULLP == logLocal._pszMsg\r\n");
        return XERROR;
    }

    memcpy(&logLocal._pszMsg[0], logmsg, logLocal._nLen);

    len = (XU32)sizeof(t_LogLocal);
    
    pMsg = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_LOG, len);
    if ( XNULL == pMsg )
    {
        XOS_MemFree((XU32)FID_LOG, logLocal._pszMsg);
        //printf("\r\nXNULL == pMsg\r\n");
        return XERROR;
    }
        
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datasrc.FID = (XU32)FID_LOG;
    pMsg->length = len;
    pMsg->msgID = eSendData;
    pMsg->prio = eNormalMsgPrio;
    pMsg->datadest.FID = (XU32)FID_LOG;
    pMsg->datadest.PID = XOS_GetLocalPID();

    XOS_MemCpy(pMsg->message, &logLocal, len);

    ret = XERROR;

    /*发送数据*/
    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        /*其他消息类型应该将消息内存释放*/
        XOS_MemFree((XU32)FID_LOG, logLocal._pszMsg);
        XOS_MsgMemFree((XU32)FID_LOG, pMsg);
        //printf("\r\nLog_SendToLog->XOS_MsgSend failed\r\n");
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
 函数名:Log_Write
 功能:  将打印消息发送给log模块
 输入:  
        logid    --打印的fid
        infotype --打印的级别
        msg_out --打印的内容
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:此函数不能用xos_trace,只能用printf，否则，会形成函数调用死循环，导致栈溢出。
************************************************************************/
XS32 Log_Write (const t_FIDCB *pFidCb, XU32 logid, e_PRINTLEVEL infotype, XCHAR *msg_out)
{

    if ( g_logInitFlag == XFALSE)
    {
        //printf("\r\nINFO :LOG_Write->log moudle initing...\r\n");
        return XSUCC;
    }

    if ( infotype >=PL_MAX || infotype < PL_MIN )
    {
        //printf("\r\nINFO :LOG_Write->para infotype %d is wrong!\r\n", (XU32)infotype);
        return XERROR;
    }

    
    if(NULL != pFidCb)
    {
        if ( pFidCb->logLevel > infotype )
        {
           /*该条信息的级别不够记录到日志文件中*/
           return XSUCC;
        }
    }
    else{}/*表示未注册模块,也要打印日志*/        

    /*发送到LOG队列*/
    XOS_MutexLock(&g_LogWriteMutex);
    
    Log_SendToLog(logid, infotype, (XCHAR *)msg_out);

    XOS_MutexUnlock(&g_LogWriteMutex);

    return XSUCC;
}

/************************************************************************
 函数名:Log_Write
 功能:  将打印消息发送给log模块
 输入:  
        logid    --打印的fid
        infotype --打印的级别
        level   --日志的过滤级别
        msg_out --打印的内容
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:此函数不能用xos_trace,只能用printf，否则，会形成函数调用死循环，导致栈溢出。
************************************************************************/
XS32 Log_SessionWrite ( const t_FIDCB *pFidCb, XU32 logid, e_PRINTLEVEL infotype, e_PRINTLEVEL level,XCHAR *msg_out)
{

    if ( g_logInitFlag == XFALSE)
    {
        //printf("\r\nINFO :LOG_Write->log moudle initing...\r\n");
        return XSUCC;
    }

    if ( infotype >=PL_MAX || infotype < PL_MIN )
    {
        //printf("\r\nINFO :LOG_Write->para infotype %d is wrong!\r\n", (XU32)infotype);
        return XERROR;
    }

    
    if(NULL != pFidCb)
    {
        if ( level > infotype )
        {
           /*该条信息的级别不够记录到日志文件中*/
           return XSUCC;
        }
    }
    else{}/*表示未注册模块,也要打印日志*/        

    /*发送到LOG队列*/
    XOS_MutexLock(&g_LogWriteMutex);
    
    Log_SendToLog(logid, infotype, (XCHAR *)msg_out);

    XOS_MutexUnlock(&g_LogWriteMutex);

    return XSUCC;
}
/********************************** 
函数名称    : Log_TraceTaskProc
作者        : 
设计日期    : 2008年3月21日
功能描述    : 处理LOG模块消息队列，
              1、如果开启了远程日志功能，则将日志发送给远程服务器，
              2、写本地文件(win32,linux, vxworks不写本地)
参数        :  
            logId  -- 
            level  --
            logmsg --

返回值      : XS32 
************************************/
XS32 Log_TraceTaskProc(XS32 logId, e_PRINTLEVEL level, XCHAR* logmsg, XU16 msgLen)
{
#ifndef XOS_VXWORKS
    XU64    diskcap; 
    XCHAR   strlog[ LOG_ITEM_MAX +1]  = {0};
#endif

    if((logmsg == XNULL) || msgLen < 1 || msgLen > LOG_ITEM_MAX)
    {
        XOS_CliInforPrintf("\r\nILog_TraceTaskProc->para is unvalid\r\n");
        return XERROR;
    }

    if(msgLen > LOG_ITEM_MAX)
    {
        msgLen = LOG_ITEM_MAX;
    }

    
    /*判断远程是否开启远程日志*/
    if(g_LogCfg._nEnableRemote) 
    {
        if(XSUCC != Log_SendDataToRemote((XU32)logId, level, logmsg, msgLen))
        {
            //XOS_CliInforPrintf("\r\nILog_TraceTaskProc->Log_SendDataToRemote failed\r\n");
        }
    }
    
    /* 没有定义VXWORKS宏时，则开始记录到本地日志文件中*/
#ifndef XOS_VXWORKS
    /*判断磁盘空间是否足够，否则打印提示信息*/
    if(XSUCC != Log_GetSysDiskCap(&diskcap))
    {
        return XERROR;
    }
    /*判断磁盘空间是否足够，否则打印提示信息*/
    if ( diskcap <= MIN_CAP )
    {
        XOS_CliInforPrintf("\r\ncan not write log ->the disk is full!\r\n");
        return XERROR;
    }
    
    XOS_StrNcpy(strlog, logmsg, msgLen);
    strlog[LOG_ITEM_MAX] = '\0';

    /*检测文件是否满*/
    Log_CheckFileFull();
    
    /*写日志文件*/
    Log_FormatWriteLog(strlog, g_LogStatus._pLogFileHandle);
    fflush(g_LogStatus._pLogFileHandle);

#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_WIN32))
    Log_SpackRefresh();/*将总的log日志大小限制在500M以内*/
#endif

#endif  //no define vxworks
    return XSUCC;
}
/********************************** 
函数名称    : Log_CheckFileFull
作者        : 
设计日期    : 2013年8月13日
功能描述    : 检查文件是否写满，写满时重新生成新的日志文件
参数        : 
返回值      : 如果要文件存在，则返回实际大小，如果文件不存在，则返回-1
************************************/
XS32 Log_CheckFileFull(void)
{
    XSFILESIZE usFileLen = 0;

    usFileLen = Log_GetFileSize(g_LogStatus._szCurrLogname);
    if (usFileLen <= XERROR || (XUFILESIZE)usFileLen >= (g_LogCfg._nLogSize * MIN_LOGFILESIZE))
    {
        /*日志文件已满，关闭当前日志文件指针*/
        if ( XNULLP != g_LogStatus._pLogFileHandle )
        {
            XOS_Fclose(g_LogStatus._pLogFileHandle);
            g_LogStatus._pLogFileHandle = NULL;
        }


        /*修改全局变量关于记录日志文件信息*/
        g_LogStatus._uCurSize++;
        g_LogStatus._uSeqNum++;

        /*生成新的日志文件名*/
        if(XSUCC != Log_CreateRunFileName(g_LogStatus._szCurrLogname, XTRUE))
        {
            XOS_CliInforPrintf("\r\nLOG_write,create new log file failed!\r\n");
            return XERROR;
        }
        
        /*打开新的日志文件*/
        if ( XNULLP == ( g_LogStatus._pLogFileHandle = XOS_Fopen(g_LogStatus._szCurrLogname, "a")) )
        {
            XOS_CliInforPrintf("\r\nnew log file %s can not open!\r\n", g_LogStatus._szCurrLogname);
            return XERROR;
        }
    }

    return XSUCC;
}
/*-----------------------------------------------
review ok
------------------------------------------------*/
/********************************** 
函数名称    : Log_GetSysDiskCap
作者        : lxn; adjust Zengguanqun
设计日期    : 2008年3月21日
功能描述    : 
参数        : XU32 *cap
返回值      : XS32 
************************************/
XS32 Log_GetSysDiskCap(XU64 *cap)
{

#if (defined (XOS_WIN32))
    XSTATIC XU32 disk_acess_times=0;
    XU64   tmpdiskbytes       = 0;
    struct _diskfree_t t_df   = {0};
    XS8 ret = 0;
    if(NULL == cap)
    {
        return XERROR;
    }
    /*系统硬盘大小结构*/
    if(disk_acess_times++  < 1000)
    {
        /*不频繁访问*/
        *cap = 2 * MIN_CAP;
        return XSUCC;
    }
    disk_acess_times=0;

    ret = _getdiskfree(_getdrive(),&t_df);
    if( 0 != ret )
    {
        XOS_CliInforPrintf("\r\nINFO: getdiskfree is wrong\r\n");
        return XERROR;
    }
    tmpdiskbytes = (XU64) t_df.avail_clusters/(1024);
    *cap = tmpdiskbytes * t_df.sectors_per_cluster * t_df.bytes_per_sector;

#endif /*win32*/

//modified for xos log optimization zenghaixin 20100816
#if  ( defined (XOS_LINUX) || defined (XOS_SOLARIS) )
    struct statfs diskStatfs;   
    if(NULL == cap)
    {
        return XERROR;
    } 
    statfs(".", &diskStatfs);    
    *cap = ((unsigned long long)diskStatfs.f_bfree * diskStatfs.f_bsize) >> 10;//diskStatfs.f_bfree强制转换为64位，以防止相乘结果溢出
    //printf("for_test: disk [%d] kb free : \r\n", *cap);
#endif /*linux/solaris*/

#if  defined (XOS_VXWORKS)
    if(NULL == cap)
    {
        return XERROR;
    }
    *cap = 2 * MIN_CAP;
#endif /*vxworks*/

    return XSUCC;
}

/*review ok-----------------------------------*/
/********************************** 
函数名称    : Log_GetFileSize
作者        : lxn; adjust Zengguanqun
设计日期    : 2008年3月21日
功能描述    : 返回K的大小
参数        : XCHAR*file
返回值      : XS32 
************************************/
XSFILESIZE Log_GetFileSize(XCHAR*file)
{
    XUFILESIZE size = 0;
    if ( XSUCC != XOS_GetFileLen(file,&size) )
    {
        XOS_CliInforPrintf("\r\nINFO:log get filesize failed!\r\n");
        return XERROR;
    }
    size = (XU32)(size/1024);
    return size;
}

/********************************** 
函数名称    : Log_FormatWriteLog
作者        : 
设计日期    : 2013年8月12日
功能描述    : 格式化日志消息，写日志文件
参数        : str 日志消息
返回值      : XS32 
************************************/
XS32 Log_FormatWriteLog(XCHAR *str,FILE * tofile)
{
    XU32 strLen = 0;
    XU32 retLen = 0;
    
    if ( XNULLP == str || XNULLP == tofile)  
    {
        return XSUCC;
    }

    strLen = (XU32)XOS_StrLen(str);
    while(strLen > 0) 
    {
        retLen = (XU32)XOS_Fwrite(str, 1, strLen,tofile);
        if(0 == retLen)
        {
            break;
        }
        strLen -= retLen;
        str += retLen;
    }
    XOS_Fwrite("\r\n", 2, 1, tofile);
    return XSUCC;
}

/*****************************************内部要用的函数****************/
/**-*-*-*-*-*-*-*-**-*-*-*-*-**-*-*-*-*-**-*-*-*-*-**\

功  能:获取log模块初始化时间，并处理为字符串格式

*-*-*-**-*-*-*-**-*-*-*-*-**-*-*-*-*-**-*-*-*-*-**/
XS32 Log_GetLoctime(XCHAR *datet)
{
    time_t tmpTime = time(NULL);
    struct tm *mm = localtime(&tmpTime);
    if ( XNULLP== datet )
    {
        return XERROR;
    }
    sprintf(datet, "%04d%02d%02d_%02d%02d%02d",
    mm->tm_year+1900,
    mm->tm_mon+1,
    mm->tm_mday,
    mm->tm_hour,
    mm->tm_min,
    mm->tm_sec);
    return XSUCC;
}

/*--------------------------------------------------------
review OK
---------------------------------------------------------*/
/********************************** 
函数名称    : LOG_CreateRunFileName
作者        : lxn; adjust Zengguanqun
设计日期    : 2008年3月21日
功能描述    : 
参数        : XCHAR *filename
参数        : XBOOL flag
返回值      : XS32  
************************************/
XS32  Log_CreateRunFileName(XCHAR *filename,XBOOL flag)
{
    XCHAR szLogDir[XOS_MAX_PATHLEN+1] = {0};
    XOS_MemSet(szLogDir,0x0,XOS_MAX_PATHLEN+1);
#ifdef XOS_EW_START
    /*生成全路径log 文件名*/
    if( XSUCC != XOS_GetSysPath(szLogDir, XOS_MAX_PATHLEN))
    {   
        
        XOS_CliInforPrintf("\r\nLog_CreateRunFileName()->XOS_GetSysPath failed\r\n");
        return XERROR;
    }
#endif
    XOS_StrNCat(szLogDir, LOGDIR, XOS_MIN((sizeof(szLogDir)-1)-strlen(szLogDir), strlen(LOGDIR)));
    if((XOS_StrLen(szLogDir)<1) || (XOS_StrLen(szLogDir)>XOS_MAX_PATHLEN))
    {
      return XERROR;
    }

    /*检测文件目录是否存在*/
    XOS_LsCheckLogDir(szLogDir);

    if(!filename)
    {
      return XERROR;
    }
    if(XFALSE == flag)
    {
       /*start file*/
       XOS_Sprintf(filename, XOS_MAX_PATHLEN,"%sxoslog_%s.log",szLogDir, g_LogStatus._szSystemUptime);
    }
    else
    {
       /*new file*/
       XOS_Sprintf(filename, XOS_MAX_PATHLEN, "%sxoslog_%s_%010d.log",szLogDir, g_LogStatus._szSystemUptime, g_LogStatus._uSeqNum);
    }
    return XSUCC;

}

XS8 XOS_LsCheckLogDir(XCHAR *pszDir)
{
    if(NULL == pszDir)
    {
        return XERROR;
    }

    #if (defined(XOS_WIN32))
    if (0 != access(pszDir, 0))
    {
        if(0 != mkdir(pszDir))
        {
            XOS_CliInforPrintf("mkdir %s failed", pszDir);
            return XERROR;
        }
    }
#endif

#if ((defined (XOS_LINUX)) || (defined (XOS_SOLARIS)) )
    if (0 != access(pszDir, 0))
    {
        if(0 != mkdir(pszDir, S_IRWXU))
        {
            XOS_CliInforPrintf("mkdir %s failed", pszDir);
            return XERROR;
        }
    }   
#endif

    return XSUCC;
}


/************************************************************************
 函数名:Log_SendShakeMsg
 功能:  发送握手消息
 输入:  pBackPara 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_SendShakeReqMsg(void)
{    
    XS32 ret = 0;

    if(LOG_LINK_CONNECTED == g_serverLinkStatus)
    {
        if(g_LogShakeSend < LOG_MAX_SHAKE_SEND)
        {
            /*发送握手消息*/
            ret = Log_SendShakeReqToRemote();
            if(XSUCC == ret)
            {
                g_LogShakeSend++;
                return XSUCC;
            }
            else
            {
                return XERROR;
            }
        }
        else
        {
            XOS_CliInforPrintf("\r\nLog_SendShakeReqMsg timeout\r\n");

            g_LogShakeSend = 0;

            /*链路状态正常，却收不到握手响应，则说明对端异常(fin包未到达)重启链路*/
             Log_LinkStopAndInit();

            return XERROR;
        }        
    }
    else
    {
        g_LogShakeSend = 0;
    }

    return XERROR;
}

/************************************************************************
 函数名:Log_SendShakeRspMsg
 功能:  发送握手响应消息
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_SendShakeRspMsg(void)
{
    return Log_SendShakeRspToRemote();
}

/************************************************************************
 函数名:Log_RecvShakeReqProc
 功能:  发送握手请求消息
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_RecvShakeReqProc(XU8* pData, XU16 len)
{
    /*报文检测*/
    if (XNULL == pData)
    {
        XOS_CliInforPrintf("\r\n pData == NULL is error");
        return XERROR;
    }

    if(len != sizeof(t_LogShakeReq))
    {
        XOS_CliInforPrintf("\r\nlen != sizeof(t_LogShakeReq)\r\n");
        return XERROR;
    }
    
    /*发送握手响应*/
    return Log_SendShakeRspMsg();
}

/************************************************************************
 函数名:Log_RecvShakeRspProc
 功能:  处理握手响应消息
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_RecvShakeRspProc(XU8* pData, XU16 len)
{    
    /*报文检测*/
    if (XNULL == pData)
    {
        XOS_CliInforPrintf("\r\n pData == NULL is error\r\n");
        return XERROR;
    }

    if(len != sizeof(t_LogShakeRsp))
    {
        XOS_CliInforPrintf("\r\nlen != sizeof(t_LogShakeReq)\r\n");
        return XERROR;
    }

    /*握手计数器置0*/
    g_LogShakeSend = 0;

    return XSUCC;

}

/************************************************************************
 函数名:Log_DealNtlStreamProc
 功能:  处理收到的日志报文
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_DealTcpStreamProc(XU8* pData, XU16 len)
{
    t_LogDataHead* MsgHead = XNULL;

    /*头部长度预测*/
    if (pData == NULL)
    {
        XOS_CliInforPrintf("\r\npData == NULL is error\r\n");
        return XERROR;
    }

    if (len < sizeof(t_LogDataHead))
    {
        XOS_CliInforPrintf("\r\nlen < sizeof(t_LogDataHead) len = 0x%x\r\n", len);
        return XERROR;
    }

    MsgHead = (t_LogDataHead*)pData;

    switch(MsgHead->_nType)
    {
        case e_SHAKEREQ:    /*握手请求消息*/
            Log_RecvShakeReqProc(pData, len);
            break;
        case e_SHAKERSP:    /*握手响应消息*/
            Log_RecvShakeRspProc(pData, len);
            break;
        case e_LOGREQ:
            break;
        default:
            XOS_CliInforPrintf("\r\nMsgHead._nType is error  %d \r\n", MsgHead->_nType);
            break;
    }     
    return XSUCC;
    
}

/************************************************************************
 函数名:Log_DealNtlStream
 功能:  处理从ntl模块接收到的tcp流, 分解出一个完整的报文
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_DealTcpStream(XU8* ipData, XU16 len)
{
    XS16* p16Value = XNULL;
    XU16  pktLen = 0;

    if(NULL == ipData)
    {
        XOS_CliInforPrintf("\r\nLog_DealNtlStream()->ipData is null\r\n");
        return XERROR;
    }

    if(len + usSrvIpDataLen >= MAX_TCP_BUF_LEN)
    {
        XOS_CliInforPrintf("\r\nLog_DealNtlStream: Buf overload! len=%d,dataLen=%d.\r\n", len, usSrvIpDataLen);
        usSrvIpDataLen = 0x00;
        return XERROR;
    }

    memcpy(ucSrvIpDataBuf + usSrvIpDataLen, ipData, len);
    usSrvIpDataLen += len;

    while(1)
    {
        if(0x7e != ucSrvIpDataBuf[0] || 0xa5 != ucSrvIpDataBuf[1])
        {
            XOS_CliInforPrintf("\r\nLog_DealNtlStream: BeginFlag is wrong!!\r\n");
            usSrvIpDataLen = 0x00;
            break;
        }

        p16Value = (XS16* )&ucSrvIpDataBuf[2];
        pktLen = (XU16)ntohs(*p16Value);
        if(pktLen > MAX_LOG_TCP_DATA_LEN) // 5 mean pkgType + sessionid 's len*/
        {
            XOS_CliInforPrintf("\r\nLog_DealNtlStream: Data len %d is wrong!\r\n", pktLen);
            usSrvIpDataLen = 0x00;
            break;
        }

        /* 4字节为FLAG的头和尾 */
        if(pktLen  <= usSrvIpDataLen)
        {
            if(0x7e != ucSrvIpDataBuf[pktLen - 2] || 0x0d != ucSrvIpDataBuf[pktLen - 1])
            {
                XOS_CliInforPrintf("\r\nLog_DealNtlStream: EndFlag is wrong!\r\n");
                usSrvIpDataLen = 0x00;
                break;
            }

            Log_DealTcpStreamProc(ucSrvIpDataBuf, (XU16)(pktLen)) ;

            /*此处应该优化，做成一个环形队列，避免内存移动*/
            usSrvIpDataLen = usSrvIpDataLen - pktLen;
            memmove(ucSrvIpDataBuf, &ucSrvIpDataBuf[pktLen], usSrvIpDataLen);
        }
        else
        {
            break;
        }
    }

    return XSUCC;
}


/************************************************************************
 函数名:Log_TraceMsgPro
 功能:  trace消息处理
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 Log_TraceMsgPro(t_XOSCOMMHEAD* pMsg)
{
    t_LogLocal *pLog = NULL;
    XS32 ret = 0;
    
    if(NULL == pMsg)
    {
        XOS_CliInforPrintf("\r\nLog_TraceMsgPro()->pMsg is null\r\n");
        return XERROR;
    }

    pLog = (t_LogLocal*)pMsg->message;
    
    if(NULL != pLog)
    {
        ret = Log_TraceTaskProc((XS32)pLog->_nLogId, pLog->_level, pLog->_pszMsg, pLog->_nLen);

        XOS_MemFree(FID_LOG, pLog->_pszMsg);

        return ret;
    }

    return XERROR;
}
/************************************************************************
 函数名:Log_StartShakeTimer
 功能:  启动握手定时器
 输入:  
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_StartShakeTimer(void)
{
    t_PARA timerpara;
    t_BACKPARA backpara = {0};

    /*开始握手定时器*/
    timerpara.fid = FID_LOG;
    timerpara.len = MAX_SHAKE_TIMEOUT;
    timerpara.mode = TIMER_TYPE_LOOP;
    timerpara.pre  = TIMER_PRE_LOW;
    backpara.para1 = LOG_TIMER_SHAKE;

    if (XSUCC != XOS_TimerStart(&g_LogShakeTimer, &timerpara, &backpara))
    {
        XOS_CliInforPrintf("\r\nLog_StartShakeTimer->XOS_TimerStart() failed!\r\n");
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
 函数名:Log_StopShakeTimer
 功能:  关闭握手定时器
 输入:  
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_StopShakeTimer(void)
{
    /*停止定时器*/
    if(g_LogShakeTimer == 0)
    {
        //XOS_CliInforPrintf("\r\nLog_StopTcpClientTimer()->XOS_TimerStop pTcpCb->timerId 0!\r\n");
        return XERROR;    
    }
    
    if (XSUCC != XOS_TimerStop(FID_LOG, g_LogShakeTimer))
    {
        XOS_CliInforPrintf("\r\nLog_StopShakeTimer->XOS_TimerStop() failed!\r\n");
        return XERROR;
    }

    g_LogShakeTimer = NULL;

    return XSUCC;
}


/************************************************************************
 函数名:Log_TimerProc
 功能:  定时器处理函数
 输入:  pBackPara 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_TimerProc( t_BACKPARA* pBackPara)
{
    XUTIMERPARA timerid = 0;

    if(NULL == pBackPara)
    {
        XOS_CliInforPrintf("\r\nLog_TimerProc()->pBackPara is nul\r\n");
        return XERROR;
    }

    timerid = pBackPara->para1;
    
    switch (timerid)
    {
        case LOG_TIMER_SHAKE: 
            /*发送握手消息*/
            Log_SendShakeReqMsg();
            break; 
        case LOG_TIMER_RETCP:
            Log_TcpReConnect();
            break;
        default:
            XOS_CliInforPrintf("\r\ntimer msg error!timerid=%d\r\n", timerid);
            break;
    }

    return XSUCC;
}

/************************************************************************
 函数名:Log_MsgProc
 功能:  模块消息处理函数
 输入:  
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_MsgProc(XVOID* pMsgP, XVOID* pRVoid)
{

    XS8 ret = 0 ;  /* 返回标志*/

    t_XOSCOMMHEAD* pMsg = NULL;

    XOS_UNUSED(pRVoid);

    if(NULL == pMsgP)
    {
        XOS_CliInforPrintf("\r\nLog_MsgProc()->pMsgP is null\r\n");
        return XERROR;
    }

    pMsg = (t_XOSCOMMHEAD*)pMsgP;

    /*判断消息来源*/
    switch ( pMsg->datasrc.FID )
    {
        case FID_LOG:
            ret = (XS8)Log_TraceMsgPro(pMsg);
            break;

        default:
            XOS_CliInforPrintf("\r\nFailed to Error Msg!\r\n");
            break;
    }

    return ret ;
}







/************************************************************************
 函数名:Log_NoticeProc
 功能:  模块通知函数
 输入:  
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_NoticeProc(XVOID *t, XVOID *v)
{

    XOS_UNUSED(t);
    XOS_UNUSED(v);

    if(g_LogCfg._nEnableRemote)
    { 
        XOS_MutexLock(&g_LogFdSet.mutex);
        Log_TcpLinkStart();
        XOS_MutexUnlock(&g_LogFdSet.mutex);
    }

    return XSUCC;
}

/************************************************************************
 函数名:Log_Init
 功能:  初始化函数
 输入:  pBackPara －定时器指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_Init (XVOID* p1, XVOID* p2)
{
    XU32 fid_index = 0;
    XS32 ret = 0;

    XOS_UNUSED(p1);
    XOS_UNUSED(p2);

    /*初始化启动时间*/
    Log_GetLoctime(g_LogStatus._szSystemUptime);

    /*初始化流量控制表*/
    XOS_MemSet(g_msgqFidChk, 0x0, (FID_MAX*sizeof(t_MSGFIDCHK)));

    /*读取配置文件xos.xml中的日志配置规则*/
    if (!XML_ReadLogCfg(&g_LogCfg, XOS_CliGetXmlName())) 
    {
        XOS_CliInforPrintf("\r\nINFO:LOG_INIT->read xos.xml file for logcfg failed\r\n");
        return XERROR;
    }

    /*设置FID最大流量控制值*/
    for(fid_index = 0; fid_index <FID_MAX; fid_index++)
    {
       g_msgqFidChk[fid_index].fid_msgmax = g_LogCfg._nMaxFlow;
    }
    if(g_msgqFidChk[FID_TELNETD].fid_msgmax != 0)
    {
        g_msgqFidChk[FID_TELNETD].fid_msgmax = 100;  /*telnet模块的流量控制独立设置*/
    }

    /*初始化本地日志文件*/
    Log_InitLocalLogFile();
    
    /*设置默认级别*/
    g_lPrintToLogLevel = PL_ERR;

    /*注册定时器*/
    ret = XOS_TimerReg(FID_LOG, 100, 5, 0);
    if( ret != XSUCC)
    {
        XOS_CliInforPrintf("\r\nlog init,XOS_TimerRegfailed\r\n");
        return XERROR;
    }


    XOS_MutexCreate(&g_LogWriteMutex);
    XOS_MutexCreate(&g_LogFdSet.mutex);
    
    /*创建接收线程*/
    ret = XOS_TaskCreate("Tsk_log_recv",TSK_PRIO_LOWEST,NTL_RECV_TSK_STACK_SIZE,
                        (os_taskfunc)Log_TcpRecv,NULL,&g_LogRecvTaskId);    

    /*初始化命令行*/
    if(XSUCC != Log_CliCommandInit(SYSTEM_MODE))
    {
        return XERROR;
    }    
    
    /*日志模块初始化完成*/
    g_logInitFlag  = XTRUE;

    return(XSUCC);
}



/************************************************************************
 函数名:Log_InitLocalLogFile
 功能:  初始化本地日志文件
 输入:  
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_InitLocalLogFile(void)
{
    XU64  DisCap = 0;
    XCHAR szLogDir[XOS_MAX_PATHLEN+1] = {0};

#ifdef XOS_EW_START
    if( XSUCC != XOS_GetSysPath(szLogDir, XOS_MAX_PATHLEN) ) 
    {
        return XERROR;
    }
#endif  /*XOS_EW_START*/

    XOS_StrNCat(szLogDir, LOGDIR, XOS_MIN((sizeof(szLogDir)-1)-strlen(szLogDir), strlen(LOGDIR)));
    if(XOS_StrLen(szLogDir) > (XOS_MAX_PATHLEN -32))
    {
      XOS_CliInforPrintf("\r\nlog init,init log dir is too long,please check pwd path\r\n");
      return XERROR;
    }

    /*check disk storage whether enough*/
    Log_GetSysDiskCap(&DisCap);
    if ( DisCap < MIN_CAP )
    {
       XOS_CliInforPrintf("\r\nlog init,the sysDiskCap is not enough\r\n");
       return XERROR;
    }
    
    
    /*本地日志文件初始化，VXWORKS下不需要初始化，只针对WIN32和LINUX*/
    /*--建立保存日志文件的目录 VTA及VxWorks没有存贮系统,文件及目录操作不支持--*/
#if (defined(XOS_WIN32))
    if (0 != chdir(szLogDir))
    {
      mkdir(szLogDir);
    }
    else
    {
      chdir("..");
    }
#endif

#if ((defined (XOS_LINUX)) || (defined (XOS_SOLARIS)) )
    if (0 != chdir(szLogDir))
    {
      mkdir(szLogDir,LOG_MODE);
    }
    else
    {
      chdir("..");
    }
#endif
        
#if( defined (XOS_WIN32) || defined (XOS_LINUX) || defined (XOS_SOLARIS) )
    if(XSUCC == Log_CreateRunFileName(g_LogStatus._szCurrLogname, XFALSE)) 
    {
        if ( XNULLP == (g_LogStatus._pLogFileHandle = XOS_Fopen(g_LogStatus._szCurrLogname,"a")) )
        {
           XOS_CliInforPrintf("log init,open log file %s failed\r\n", g_LogStatus._szCurrLogname);
           return XERROR;
        }
    }
#endif
    
    return XSUCC;
}



/************************************************************************
 函数名:XOS_LogSetLevel
 功能:  设置打印到日志的log级别,由业务通过API进行调用
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8  XOS_LogSetFidLevel(XU32 fid, e_PRINTLEVEL logLevel)
{
    t_FIDCB *pFidCb = NULL;

    if(fid >= FID_MAX)
    {
        //printf("\r\nXOS_LogSetLevel()->fid error\r\n");/*trace模块可能没有启动*/
        return XERROR;
    }
    
    if(logLevel >= PL_MAX)
    {
        //printf("\r\nXOS_LogSetLevel()->logLevel error\r\n");
        return XERROR;
    }

    pFidCb = (t_FIDCB *)MOD_getFidCb(fid);
    if(NULL != pFidCb)
    {
        /*设置日志打印级别*/
        pFidCb->logLevel = logLevel;
    }
    else
    {
         //printf("\r\nXOS_LogSetLevel()->pFidCb error, fid = %d, logLevel = %d\r\n",fid, logLevel);
         return XERROR;
    }
    
    return XSUCC;

}


/************************************************************************
 函数名:Log_CmdSetLogLevel
 功能:  设置打印到日志的log级别
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XVOID Log_CmdSetLogLevel(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32 fid = 0;
    e_PRINTLEVEL logLevel = PL_MIN;
    t_FIDCB *pFidCb = NULL;
    XU16 nIndex = 0;
    
    if ( 3 != siArgc || NULL == ppArgv)
    {
        XOS_CliExtPrintf(pCliEnv,"input para error\r\n");
        return ;
    }

    fid  = (XU32)atoi(ppArgv[1]);
    if(fid >= FID_MAX)
    {
        XOS_CliExtPrintf(pCliEnv,"input para error\r\n");
        return ;
    }
    
    logLevel = (e_PRINTLEVEL ) atoi(ppArgv[2]);
    if(logLevel >= PL_MAX)
    {
        XOS_CliExtPrintf(pCliEnv,"input para error\r\n");
        return ;
    }
    /*设置所有模块*/
    if(0 == fid)
    {
        for(nIndex = FID_XOSMIN; nIndex<FID_MAX; nIndex++)
        {
            pFidCb = (t_FIDCB*)XNULL;
            pFidCb = (t_FIDCB*)MOD_getFidCb(nIndex);
            if(pFidCb == XNULLP)
            {
                continue;
            }
            
            /*设置日志打印级别*/
            pFidCb->logLevel = logLevel;       
        }
        
        XOS_CliExtPrintf(pCliEnv,"set all fid log to %s print level ok\r\n", ppLogTip[logLevel]);
    }
    else
    {
        pFidCb = (t_FIDCB *)MOD_getFidCb(fid);
        if(NULL != pFidCb)
        {
            /*设置日志打印级别*/
            pFidCb->logLevel = logLevel;
            XOS_CliExtPrintf(pCliEnv,"set fid %d log to %s print level ok\r\n", fid, ppLogTip[logLevel]);
        }
        else
        {
            XOS_CliExtPrintf(pCliEnv,"input para error\r\n");
        }
    }

    
    
    g_lPrintToLogLevel = (e_PRINTLEVEL ) atoi(ppArgv[2]);
    
    return ;

}

/************************************************************************
 函数名:Log_CmdShowFidCheck
 功能:  显示流量控制
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XVOID Log_CmdShowFidCheck(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32 fid=0;
    XU32 chk_fid;

    if(2 != siArgc || NULL == ppArgv)
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong!\r\n");
        return;
    }

    fid  = (XU32)atoi(ppArgv[1]);

    XOS_CliExtPrintf(pCliEnv,"\r\n---------------------------------------------------------------------------\r\n");

    XOS_CliExtPrintf(pCliEnv,"%-12s%-12s%-12s%-12s%-12s\r\n",
    "FID","cfgmsgmax","msgcurcount","msgfilter","msgpeakcount");
    if(fid > 0 && fid <FID_MAX)
    {
            XOS_CliExtPrintf(pCliEnv,"%-12u%-12u%-12u%-12u%-12u\r\n",
            fid,
            g_msgqFidChk[fid].fid_msgmax,
            g_msgqFidChk[fid].fid_msgcount,
            g_msgqFidChk[fid].fid_msgfilter,
            g_msgqFidChk[fid].fid_msgpeakcount);
            XOS_CliExtPrintf(pCliEnv,"---------------------------------------------------------------------------\r\n");
            return;
    }

    /*非法fid时，显示所有的*/
    for(chk_fid=0; chk_fid<FID_MAX; chk_fid++)
    {
       if(g_msgqFidChk[chk_fid].fid_msgcount >0)
       {
            XOS_CliExtPrintf(pCliEnv,"%-12u%-12u%-12u%-12u%-12u\r\n",
            chk_fid,
            g_msgqFidChk[chk_fid].fid_msgmax,
            g_msgqFidChk[chk_fid].fid_msgcount,
            g_msgqFidChk[chk_fid].fid_msgfilter,
            g_msgqFidChk[chk_fid].fid_msgpeakcount);
       }
    }
    XOS_CliExtPrintf(pCliEnv,"---------------------------------------------------------------------------\r\n");

    return;
}

/************************************************************************
 函数名:Log_CmdShowFidCheck
 功能:  设置最大流量值及峰值
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XVOID Log_CmdSetFidCheck(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32 cmd_fid = 0;
    XU32 fid_msgpeakvalue=10;
    XUTIME  l_now = 0;
    XUTIME  time_delta = 0;
    l_now = (XUTIME)time(XNULL);

    if((4<siArgc)|| (2>siArgc) || NULL == ppArgv)
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
        return;
    }
    cmd_fid = (XU32)atoi(ppArgv[1]);
    if(cmd_fid >= FID_MAX)
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
        return;
    }

   /*最大流量*/
   if(siArgc >= 3)
   {
      fid_msgpeakvalue = (XU32)atoi(ppArgv[2]);
      if( 0 == fid_msgpeakvalue)
      {
          g_msgqFidChk[cmd_fid].fid_msgmax = fid_msgpeakvalue;
          
      }    
      else if(fid_msgpeakvalue > 300 || fid_msgpeakvalue <10)
      {
          XOS_CliExtPrintf(pCliEnv,"input parameter is wrong!,msg_max must be limit to [10,300]|[0].\r\n");
          return;
      }
      else
      {
          g_msgqFidChk[cmd_fid].fid_msgmax = fid_msgpeakvalue;
      }
      
   }

   if(siArgc == 3)
   {
       XOS_CliExtPrintf(pCliEnv,"fid %d msgmax msg count is set to %d,current is %d\r\n",
       cmd_fid,g_msgqFidChk[cmd_fid].fid_msgmax,g_msgqFidChk[cmd_fid].fid_msgcount);
   }
   else if(siArgc == 4)
   {
       time_delta = l_now - g_msgqFidChk[cmd_fid].fid_msgchktime;
       XOS_CliExtPrintf(pCliEnv,"time_delta %d !\r\n",time_delta);
       XOS_CliExtPrintf(pCliEnv,"fid %d peak msg count is set to %d,current is %d,filter count %d \r\n",
       cmd_fid,g_msgqFidChk[cmd_fid].fid_msgmax,g_msgqFidChk[cmd_fid].fid_msgcount,g_msgqFidChk[cmd_fid].fid_msgfilter);
   }
   else
   {
       XOS_CliExtPrintf(pCliEnv,"fid %d peak msg count is set to %d\r\n",
       cmd_fid,g_msgqFidChk[cmd_fid].fid_msgmax);
   }

   return ;
}

/************************************************************************
 函数名:Log_CmdShowInfo
 功能:  log模块状态及参数显示
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XVOID Log_CmdShowInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{   
    
    XOS_UNUSED(siArgc);
    XOS_UNUSED(ppArgv);

    XOS_CliExtPrintf(pCliEnv,"log cfg _nEnableRemote = %s\r\n", g_LogCfg._nEnableRemote == 1 ? "enable":"unable");
    XOS_CliExtPrintf(pCliEnv,"log cfg _nRemoteAddr = 0x%08x\r\n", g_LogCfg._nRemoteAddr);
    XOS_CliExtPrintf(pCliEnv,"log cfg _nRemotePort = %d\r\n", g_LogCfg._nRemotePort);
    XOS_CliExtPrintf(pCliEnv,"log cfg _nLocalPort = %d\r\n", g_LogCfg._nLocalPort);
    XOS_CliExtPrintf(pCliEnv,"log cfg _nMaxFlow = %d\r\n", g_LogCfg._nMaxFlow);
    XOS_CliExtPrintf(pCliEnv,"log cfg _nLogSize = %d\r\n", g_LogCfg._nLogSize);

    /*链路状态*/
    if(g_LogCfg._nEnableRemote)
    {        
        XOS_CliExtPrintf(pCliEnv,"log link status %s\r\n", ppLogLinkStatus[g_serverLinkStatus]);
    }

    return ;
}

/************************************************************************
 函数名:Log_CmdShowFidLevel
 功能:  显示log级别
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XVOID Log_CmdShowFidLevel(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XU16 i = 0;
    XU32 fid = 0;
    t_FIDCB *pFidCb = NULL;
    
    XOS_UNUSED(siArgc);

    fid  = (XU32)atoi(ppArgv[1]);
    if(fid >= FID_MAX)
    {
        XOS_CliExtPrintf(pCliEnv,"input para error\r\n");
        return ;
    }

    if(0 == fid)
    {
        /*显示所有模块的日志级别*/
        for(i = FID_XOSMIN; i < FID_MAX; i++)
        {
            pFidCb = (t_FIDCB*)XNULL;
            pFidCb = (t_FIDCB *)MOD_getFidCb(i);
            if(pFidCb == XNULLP)
            {
                continue;
            }

            XOS_CliExtPrintf(pCliEnv,"fid %4d log is %s print level\r\n", i,
            ppLogTip[pFidCb->logLevel%(sizeof(ppLogTip)/sizeof(ppLogTip[0]))]);
        }
    }
    else
    {
        pFidCb = (t_FIDCB *)MOD_getFidCb(fid);
        if(NULL != pFidCb)
        {
            XOS_CliExtPrintf(pCliEnv,"fid %4d log is %s print level\r\n", fid,
            ppLogTip[pFidCb->logLevel%(sizeof(ppLogTip)/sizeof(ppLogTip[0]))]);
        }
        else
        {
            XOS_CliExtPrintf(pCliEnv,"input para error\r\n");
        }
    }
        
    return ;
}
/************************************************************************
 函数名:Log_CmdRestartLink
 功能:  重建log服务链路
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XVOID Log_CmdRestartLink(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU16 local_port = 0;
    XU32 log_ip = 0;
    XU16 log_port = 0;

    if ( 4 != siArgc || NULL == ppArgv|| NULL == pCliEnv)
    {
        XOS_CliExtPrintf(pCliEnv,"input para error\r\n");
        return ;
    }

    local_port = (XU16)atoi(ppArgv[1]);
    XOS_StrtoIp(ppArgv[2],&log_ip);
    log_port = (XU16)atoi(ppArgv[3]);

    if(log_ip== 0 || log_port == 0)
    {
        XOS_CliExtPrintf(pCliEnv,"input para error\r\n");
        return ;
    }

    if(XFALSE == Log_WriteConfigToXml(LOG_XOS_XML, g_LogCfg._nEnableRemote, local_port, log_ip, log_port, g_LogCfg._nMaxFlow))
    {
       XOS_CliExtPrintf(pCliEnv,"Log_WriteConfigToXml failed\r\n"); 
    }

    /*开启远程日志的情况下才有效*/
    if(1 == g_LogCfg._nEnableRemote)
    {
        g_LogCfg._nLocalPort = local_port;
        g_LogCfg._nRemoteAddr = log_ip;
        g_LogCfg._nRemotePort = log_port;

        Log_LinkStopAndInit();
    }
    else
    {
        g_LogCfg._nLocalPort = local_port;
        g_LogCfg._nRemoteAddr = log_ip;
        g_LogCfg._nRemotePort = log_port;

        XOS_CliExtPrintf(pCliEnv,"remote log not open\r\n");
    }
}

/************************************************************************
 函数名:Log_CmdCloseRemoteLog
 功能:  关闭远程日志服务
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XVOID Log_CmdCloseRemoteLog(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XOS_UNUSED(siArgc);
    XOS_UNUSED(ppArgv);

    if(1 == g_LogCfg._nEnableRemote)
    {
        g_LogCfg._nEnableRemote = 0;
        Log_StopLink();
        
        if(XFALSE == Log_WriteConfigToXml(LOG_XOS_XML, g_LogCfg._nEnableRemote, g_LogCfg._nLocalPort, 
                             g_LogCfg._nRemoteAddr, g_LogCfg._nRemotePort, g_LogCfg._nMaxFlow))
        {
            XOS_CliExtPrintf(pCliEnv,"Log_WriteConfigToXml failed\r\n"); 
        }
        XOS_CliExtPrintf(pCliEnv,"remote log was closed \r\n");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"remote log not open\r\n");
    }
}

/************************************************************************
 函数名:Log_CmdOpenRemoteLog
 功能:  打开远程日志服务
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XVOID Log_CmdOpenRemoteLog(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XOS_UNUSED(siArgc);
    XOS_UNUSED(ppArgv);

    if(0 == g_LogCfg._nEnableRemote)
    {
        g_LogCfg._nEnableRemote = 1;
        
        if(XFALSE == Log_WriteConfigToXml(LOG_XOS_XML, g_LogCfg._nEnableRemote, g_LogCfg._nLocalPort, 
                             g_LogCfg._nRemoteAddr, g_LogCfg._nRemotePort, g_LogCfg._nMaxFlow))
        {
            XOS_CliExtPrintf(pCliEnv,"Log_WriteConfigToXml failed\r\n"); 
        }

        /*开始建立链路*/
        Log_TcpLinkStart();     
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"remote log was runned\r\n");
    }
}


/************************************************************************
函数名: Log_tskStackReason
功能：  记录任务栈原因
输入：
输出：
返回：
说明：
************************************************************************/
XPUBLIC XS32 Log_tskStackReason(XCHAR *pReason)
{
    XCHAR timetmp[TIME_STR_LEN] = {0};
    XCHAR buf[MAX_LOGHEAD_LEN] = {0};
    FILE *fpw = NULL;
    
    if(pReason == XNULL)
    {
        return XERROR;
    }
    if ( XERROR == TRC_formateloctime(timetmp) )
    {
        timetmp[0] = '\0';
    }

    XOS_Sprintf(buf,MAX_LOGHEAD_LEN,"\r\n\n\n\nWARN: {%s} %s\n",timetmp,pReason);
    buf[MAX_LOGHEAD_LEN-1] = '\0';
    fpw = XOS_Fopen(STACKINFOLOG, "a");
    if(NULL == fpw)
    {
        //printf("open file fail!");
        return XERROR;    
    }
    
    XOS_Fwrite(buf,XOS_StrLen(buf),1,fpw);   
    fflush(fpw);
    XOS_Fclose(fpw);
    return XSUCC;
}

/************************************************************************
函数名: Log_tskStackInfoVx
功能：  记录任务栈原因
输入：
输出：
返回：
说明：
************************************************************************/
#ifdef XOS_VXWORKS
XS32 Log_tskStackInfoVx(XS32 taskid)
{
    XS32 oldStdIn, oldStdOut, oldStdErr ;

    XS32 newStd;
    FILE  *newfile;
    
    newfile=XOS_Fopen (STACKINFOLOG, "a");
    
    taskDelay(1);
    
    if(newfile==NULL)
    {
        //printf ("\n Unable to open file\n");
        return XERROR;
    }     
    else
    {
        newStd = (XS32)fileno (newfile);
        
        if( newStd != XERROR )
        {
            oldStdIn = ioTaskStdGet( 0, STD_IN ) ;
            oldStdOut = ioTaskStdGet( 0, STD_OUT ) ;
            oldStdErr = ioTaskStdGet( 0, STD_ERR ) ;
            
            ioTaskStdSet( 0, STD_IN, newStd ) ;
            ioTaskStdSet( 0, STD_OUT, newStd ) ;
            ioTaskStdSet( 0, STD_ERR, newStd ) ;
            taskDelay(2);
                   
            ti(taskid);
            taskDelay(2);  
            
            (void)tt(taskid);
            taskDelay(2);

            ioTaskStdSet( 0, STD_IN, oldStdIn ) ;
            ioTaskStdSet( 0, STD_OUT, oldStdOut ) ;
            ioTaskStdSet( 0, STD_ERR, oldStdErr ) ;
        }
    }
     
    (void)XOS_Fclose(newfile);
    return XSUCC;
       
}
#endif

/************************************************************************
函数名: LOG_TskStackInfo
功能：  记录任务栈信息
输入：
输出：
返回：
说明：
************************************************************************/
XPUBLIC XS32 LOG_TskStackInfo(t_XOSTASKID *pTskID,XCHAR *pReason)
{
    
#ifdef XOS_LINUX     
    pid_t curPid;
    XCHAR exeCmd[TIME_STR_LEN] = {0};
#endif

    if(XSUCC != Log_tskStackReason(pReason))
    {
         //printf("write head failed\r\n");
    }

#ifdef XOS_WIN32  /*不支持WIN32*/ 
    return XSUCC;
#endif

#ifdef XOS_LINUX   
    curPid = getpid();
    XOS_Sprintf(exeCmd, TIME_STR_LEN, "pstack %d >> "STACKINFOLOG,curPid);
  
    if( 0 != system(exeCmd) )
     {
         //printf("linux,exec cmd %s failed.\r\n",exeCmd);
         
         return XERROR;
     }
    XOS_Sleep(1000);
#endif

#ifdef XOS_VXWORKS
    /*lint -e715*/
    if(NULL != pTskID)
    {
        Log_tskStackInfoVx(*(XS32*)pTskID);
    }
    /*lint +e715*/
#endif
  /*lint -e527*/
   return XSUCC;
  /*lint +e527*/
}

/************************************************************************
 函数名:XOS_LogSetLinkPara
 功能: 设置日志模块的参数配置
 输入:  
     enable:      是否打开远程日志,0为关闭，1为打开
     localPort:   本端tcp端口
     remoteAddr:  日志服务器地址
     remotePort:  日志服务器端口
     flowCtrol:   流量控制大小，0:不进行流量控制
 输出:
 返回:  连接成功返回XSUCC , 未连接返回XERROR
 说明:
************************************************************************/
XS8 XOS_LogSetLinkPara(XU8 enable, XU16 localPort, XU32 remoteAddr, XU16 remotePort, XU16 flowCtrol)
{
    if(enable != 0 )
    {
        enable = 1;
    }
    
    g_LogCfg._nEnableRemote = enable;
    g_LogCfg._nLocalPort = localPort;
    g_LogCfg._nRemoteAddr = remoteAddr;
    g_LogCfg._nRemotePort = remotePort;
    g_LogCfg._nMaxFlow = flowCtrol;

    /*写入xos.xml*/
    if(XFALSE == Log_WriteConfigToXml(LOG_XOS_XML, enable, localPort, remoteAddr, remotePort, flowCtrol))
    {
        XOS_CliInforPrintf("Log_WriteConfigToXml()->failed!");
        return XERROR;
    }

    /*如果打开，则重启*/
    if(1 == g_LogCfg._nEnableRemote)
    {
        Log_LinkStopAndInit();
    }
    else
    {
        /*关闭,由定时器关闭*/        
    }

    return XSUCC;
}

/************************************************************************
 函数名:XOS_LogGetLinkPara
 功能:  获取日志模块的参数配置
 输入:  
 输出:
        enable:      是否打开远程日志,0为关闭，1为打开
        localPort:   本端tcp端口
        remoteAddr:  日志服务器地址
        remotePort:  日志服务器端口
        flowCtrol:   流量控制大小，0:不进行流量控制
        
 返回:  连接成功返回XSUCC , 未连接返回XERROR
 说明:
************************************************************************/
XS8 XOS_LogGetLinkPara(XU8 *pEnable, XU16 *pLocalPort, XU32 *pRemoteAddr, XU16 *pRemotePort, XU16 *pFlowCtrol)
{
    if(NULL != pEnable && NULL != pLocalPort && 
        NULL != pRemoteAddr && NULL != pRemotePort &&
        NULL != pFlowCtrol)
    {
        *pEnable = g_LogCfg._nEnableRemote;
        *pLocalPort = g_LogCfg._nLocalPort;
        *pRemoteAddr = g_LogCfg._nRemoteAddr;
        *pRemotePort = g_LogCfg._nRemotePort;
        *pFlowCtrol = g_LogCfg._nMaxFlow;

        return XSUCC;
    }
    else
    {
        return XERROR;
    }    
}

/************************************************************************
 函数名:XOS_LogGetLinkStatus
 功能:  获取日志模块的链路状态
 输入:  
 输出:
 返回:  连接成功返回XSUCC , 未连接返回XERROR
 说明:
************************************************************************/
XS8 XOS_LogGetLinkStatus(void)
{
    if(1 == g_LogCfg._nEnableRemote)
    {
        if(g_serverLinkStatus == LOG_LINK_CONNECTED)
        {
            return XSUCC;
        }
        else
        {
            return XERROR;
        }
    }
    else
    {
        return XERROR;
    }
}

/************************************************************************
 函数名:Log_WriteConfigToXml
 功能:  将配置参数写入到xos.xml
 输入:  
 输出:
 返回:  写入成功返回XTRUE , 写入失败返回XFALSE
 说明:
************************************************************************/
XBOOL Log_WriteConfigToXml(XCHAR *filename, XU8 enable, XU16 localPort, XU32 remoteAddr, XU16 remotePort, XU16 flowCtrol)
{
    xmlDocPtr  doc               = NULL;
    xmlNodePtr cur               = NULL;
    xmlNodePtr logNode           = NULL;
    xmlNodePtr logChildNode      = NULL;
    xmlNodePtr rootNode          = NULL;
    
    xmlDocPtr newDoc             = NULL;
    xmlNodePtr newRootNode       = NULL;
    XCHAR szBuf[LOG_ADDR_LEN]    = {0};
    XU8 szFlag[LOG_CONFIG_NUM]   = {0};

    if(NULL == filename)   
    {
        return XFALSE;
    } 

    doc = xmlParseFile(filename);
    if (doc == XNULL)
    {
        return (XFALSE);
    }


    /*找根节点*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    /*根节点*/
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }

    rootNode = cur;

    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    /*LOGCFG主节点*/
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "LOGCFG" ) )
        {
            break;
        }
        cur = cur->next;
    }

    /*如果没有LOGCFG，则写入子节点*/
    if ( XNULL == cur )
    {
        logNode = xmlAddChildNode(rootNode,XML_ELEMENT_NODE, "LOGCFG", "");

        logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "ENABLEREMOTE", "");
        XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", enable);
        xmlSetNodeContent(logChildNode, szBuf);

        logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOG_SERVER", "");
        XOS_IptoStr(remoteAddr, szBuf);
        xmlSetNodeContent(logChildNode, szBuf);

        logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOG_TCP_PORT", "");
        XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", remotePort);
        xmlSetNodeContent(logChildNode, szBuf);

        logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOCAL_TCP_PORT", "");
        XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", localPort);
        xmlSetNodeContent(logChildNode, szBuf);

        logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOG_FLOW", "");
        XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", flowCtrol);
        xmlSetNodeContent(logChildNode, szBuf);
    }
    else
    {
        logNode = cur;
        cur = cur->xmlChildrenNode;
        while ( cur && xmlIsBlankNode ( cur ) )
        {
            cur = cur ->next;
        }

        /*没有子结点*/
        if ( cur == XNULL )
        {
            /*增加所有子结点*/
            logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "ENABLEREMOTE", "");
            XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", enable);
            xmlSetNodeContent(logChildNode, szBuf);

            logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOG_SERVER", "");
            XOS_IptoStr(remoteAddr, szBuf);
            xmlSetNodeContent(logChildNode, szBuf);

            logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOG_TCP_PORT", "");
            XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", remotePort);
            xmlSetNodeContent(logChildNode, szBuf);

            logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOCAL_TCP_PORT", "");
            XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", localPort);
            xmlSetNodeContent(logChildNode, szBuf);

            logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOG_FLOW", "");
            XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", flowCtrol);
            xmlSetNodeContent(logChildNode, szBuf);
        }
        else
        {
            /*修改存在的结点。*/
            while(cur != XNULL )
            {
                if ( !XOS_StrCmp(cur->name, "ENABLEREMOTE" ) )
                {
                    XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", enable);
                    xmlSetNodeContent(cur, szBuf);
                    szFlag[0] = 1;
                }
                /*LOG服务器IP地址*/
                if ( !XOS_StrCmp(cur->name, "LOG_SERVER" ) )
                {
                    XOS_IptoStr(remoteAddr, szBuf);
                    xmlSetNodeContent(cur, szBuf);
                    szFlag[1] = 1;        
                }
                /*LOG服务器端口*/
                if ( !XOS_StrCmp(cur->name, "LOG_TCP_PORT" ) )
                {
                    XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", remotePort);
                    xmlSetNodeContent(cur, szBuf);
                    szFlag[2] = 1;           
                }
                /*本地端口号*/
                if ( !XOS_StrCmp(cur->name, "LOCAL_TCP_PORT") )
                {
                    XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", localPort);
                    xmlSetNodeContent(cur, szBuf);
                    szFlag[3] = 1;            
                }
                /*流量控制*/
                if ( !XOS_StrCmp(cur->name, "LOG_FLOW" ) )
                {
                    XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", flowCtrol);
                    xmlSetNodeContent(cur, szBuf);
                    szFlag[4] = 1;        
                }
                cur = cur->next;
            }

            /*增加不存在的结点。*/
            if(0 == szFlag[0])
            {
                logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "ENABLEREMOTE", "");
                XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", enable);
                xmlSetNodeContent(logChildNode, szBuf);
            }
            if(0 == szFlag[1])
            {
                logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOG_SERVER", "");
                XOS_IptoStr(remoteAddr, szBuf);
                xmlSetNodeContent(logChildNode, szBuf);
            }
            if(0 == szFlag[2])
            {
                logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOG_TCP_PORT", "");
                XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", remotePort);
                xmlSetNodeContent(logChildNode, szBuf);
            }
            if(0 == szFlag[3])
            {
                logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE, "LOCAL_TCP_PORT", "");
                XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", localPort);
                xmlSetNodeContent(logChildNode, szBuf);
            }
            if(0 == szFlag[4])
            {
                logChildNode = xmlAddChildNode(logNode,XML_ELEMENT_NODE,"LOG_FLOW", "");
                XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", flowCtrol);
                xmlSetNodeContent(logChildNode, szBuf);
            }
        }
    }
    
    /*创建新文档*/
    newDoc = xmlCreateWriter("", NULL);
    if(NULL != newDoc) 
    {
        newRootNode = xmlGetRootNode(newDoc);
        if(NULL != newRootNode)
        {   /*替换要节点*/
            xmlReplaceNode(newRootNode, rootNode);
            xmlDumpToFile(newDoc, filename, NULL);
        }
        xmlReleaseDoc(newDoc);
    }
    else
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }    

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XTRUE;
}
/************************************************************************
 函数名:Log_SetReadSet
 功能:  设置读集
 输入:  
 输出:
 返回:  
 说明:
************************************************************************/
XVOID Log_SetReadSet(void)
{
    XOS_INET_FD_SET(&(g_Log_Link.SockId),&(g_LogFdSet.readSet.fdSet));
}

/************************************************************************
 函数名:Log_SetWriteSet
 功能:  设置写集
 输入:  
 输出:
 返回:  
 说明:
************************************************************************/
XVOID Log_SetWriteSet(void)
{
    XOS_INET_FD_SET(&(g_Log_Link.SockId),&(g_LogFdSet.writeSet.fdSet));

}

/************************************************************************
 函数名:Log_SetRWSet
 功能:  设置读写集
 输入:  
 输出:
 返回:  
 说明:
************************************************************************/
XVOID Log_SetRWSet(void)
{
    XOS_INET_FD_SET(&(g_Log_Link.SockId),&(g_LogFdSet.readSet.fdSet));
    XOS_INET_FD_SET(&(g_Log_Link.SockId),&(g_LogFdSet.writeSet.fdSet));
}

/************************************************************************
 函数名:Log_ClearWriteSet
 功能:  清除读集
 输入:  
 输出:
 返回:  
 说明:
************************************************************************/
XVOID Log_ClearReadSet(void)
{
    XOS_INET_FD_ZERO(&(g_LogFdSet.readSet.fdSet));
}

/************************************************************************
 函数名:Log_ClearWriteSet
 功能:  清除写集
 输入:  
 输出:
 返回:  
 说明:
************************************************************************/
XVOID Log_ClearWriteSet(void)
{
    XOS_INET_FD_ZERO(&(g_LogFdSet.writeSet.fdSet));
}

/************************************************************************
 函数名:Log_ClearRWSet
 功能:  清除读写集
 输入:  
 输出:
 返回:  
 说明:
************************************************************************/
XVOID Log_ClearRWSet(void)
{
    XOS_INET_FD_ZERO(&(g_LogFdSet.readSet.fdSet));
    XOS_INET_FD_ZERO(&(g_LogFdSet.writeSet.fdSet));
}

/************************************************************************
 函数名:Log_StartTcpReconnTimer
 功能:  开始tcp重连接定时器
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 Log_StartTcpReconnTimer(void)
{
    t_PARA timerPara;
    t_BACKPARA backPara;
    XS32 ret = 0;

    XOS_MemSet(&timerPara,0,sizeof(t_PARA));
    XOS_MemSet(&backPara,0,sizeof(t_BACKPARA));
    
    timerPara.fid  = FID_LOG;
    timerPara.len  = MAX_RETCP_TIMEOUT;
    timerPara.mode = TIMER_TYPE_LOOP;
    timerPara.pre  = TIMER_PRE_LOW;
    backPara.para1 = LOG_TIMER_RETCP;

    g_LogexpireTimes = 0;

    g_LogTcpReconnTimer = NULL;
    
    ret =  XOS_TimerStart(&g_LogTcpReconnTimer, &timerPara, &backPara);
    if(XSUCC != ret)
    {
        printf("\r\nLog_StartTcpClientTimer()->XOS_TimerStart failed\r\n");    
    }

    return ret;    

}

/************************************************************************
 函数名:Log_StopTcpReconnTimer
 功能:  停止tcp重连定时器
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 Log_StopTcpReconnTimer(void)
{
    XS32 ret = 0;

    
    /*停止定时器*/
    if(g_LogTcpReconnTimer == 0)
    {
        //printf("\r\nLog_StopTcpClientTimer()->XOS_TimerStop pTcpCb->timerId 0!\r\n");
        return XERROR;    
    }
    
    ret = XOS_TimerStop(FID_LOG, g_LogTcpReconnTimer);

    if(ret != XSUCC)
    {
        printf("\r\nLog_StopTcpClientTimer()->XOS_TimerStop failed!\r\n");
    }

    g_LogTcpReconnTimer = NULL;
    
    g_LogexpireTimes = 0;
    
    return ret;    
}


/************************************************************************
 函数名:Log_TcpLinkStart
 功能:  开始tcp链路连接
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_TcpLinkStart(void)
{
    XS32 ret = 0;
    XU32 optVal = 0;

    Log_ClearRWSet();

    g_Log_Link.ServerAddr.ip = g_LogCfg._nRemoteAddr;
    g_Log_Link.ServerAddr.port = g_LogCfg._nRemotePort;

    g_Log_Link.LocalAddr.ip = 0;
    g_Log_Link.LocalAddr.port = g_LogCfg._nLocalPort;

    g_serverLinkStatus = LOG_LINK_INIT;

    
    ret = XINET_Socket(XOS_INET_STREAM, &(g_Log_Link.SockId));
    if (ret != XSUCC)
    {
        printf("Log_TcpLinkStart()-> open sock failed !");
        return XERROR;
    }

    ret = XINET_Bind(&(g_Log_Link.SockId), &(g_Log_Link.LocalAddr));
    if (ret != XSUCC)
    {
        printf("Log_TcpLinkStart()-> XINET_Bind sock failed !");
        return XERROR;
    }

    
    optVal = XOS_INET_OPT_ENABLE;
    XINET_SetOpt(&g_Log_Link.SockId, SOL_SOCKET, XOS_INET_OPT_LINGER, (XU32 *)&optVal); 

    ret = XINET_Connect(&(g_Log_Link.SockId), &(g_Log_Link.ServerAddr));
    if (ret != XSUCC)
    {
        g_serverLinkStatus = LOG_LINK_CONNECTING;
        
        //printf("Log_TcpLinkStart()-> XINET_Connect sock failed error = connecting !");

        /*加入可写集*/
        Log_SetWriteSet();
        /*开启重连接定时器*/
        Log_StartTcpReconnTimer();
        return XERROR;
    }
    else
    {
        g_serverLinkStatus = LOG_LINK_CONNECTED;
        /*加入可读集*/
        Log_SetReadSet();
    }
    
    return XSUCC;
}

/************************************************************************
 函数名:Log_CloseTcp
 功能:  关闭tcp
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 Log_CloseTcp(void)
{
    /*清空读写集*/
    Log_ClearRWSet();

    g_serverLinkStatus = LOG_LINK_STOP;
    
    /*关闭socket*/
    if ( XSUCC != XINET_CloseSock(&(g_Log_Link.SockId)))
    {
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
 函数名:Log_StopLink
 功能:  停止链路
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_StopLink(void)
{
    XOS_MutexLock(&g_LogFdSet.mutex);
    /*停止握手定时器*/
    Log_StopShakeTimer();

    /*停止重连定时器*/
    Log_StopTcpReconnTimer();

    /*停止链路*/    
    Log_CloseTcp();    

    XOS_MutexUnlock(&g_LogFdSet.mutex);

    return XSUCC;
}

/************************************************************************
 函数名:Log_LinkStopAndInit
 功能:  停止并初始化链路
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 Log_LinkStopAndInit(void)
{   

    XOS_MutexLock(&g_LogFdSet.mutex);
    /*停止握手定时器*/
    Log_StopShakeTimer();

    /*停止重连定时器*/
    Log_StopTcpReconnTimer();

    /*停止链路*/    
    Log_CloseTcp();    
    
    /*重新开启链路*/
    Log_TcpLinkStart();

    XOS_MutexUnlock(&g_LogFdSet.mutex);

    return XSUCC;
}


/************************************************************************
 函数名:Log_TcpReConnect
 功能:  处理链路超时重连接
 输入:  
 输出:
 返回:  
 说明: 
************************************************************************/
XS32 Log_TcpReConnect(void)
{

    XOS_MutexLock(&g_LogFdSet.mutex);
    switch(g_serverLinkStatus)
    {
        case LOG_LINK_CONNECTING:
            g_LogexpireTimes ++;
            if(g_LogexpireTimes >= 5)
            {
                /*关闭tcp*/
                Log_CloseTcp();
                g_serverLinkStatus = LOG_LINK_INIT;
                g_LogexpireTimes = 0;

                /*等待4秒后重连*/
            }
            else{} /*继续等待*/
            break;
        case LOG_LINK_INIT:                
            {
                //printf("\r\nLog_TcpReConnect start reconnect\r\n");
                Log_StopTcpReconnTimer();
                Log_TcpLinkStart();
            }
            break;
        case LOG_LINK_CONNECTED:
            /*关闭定时器*/
            Log_StopTcpReconnTimer();
            break;
        default:
            printf("\r\nlog tcp status error %d\r\n", g_serverLinkStatus);
            break;
    }

    XOS_MutexUnlock(&g_LogFdSet.mutex);

    return XSUCC;
}

/************************************************************************
 函数名:Log_TcpSelectWProc
 功能:  处理网络写事件
 输入:  
        pReadSet--读集
        pWriteSet--写集
 输出:
 返回:  
 说明: 
************************************************************************/
void Log_TcpSelectWProc(t_FDSET *pReadSet, t_FDSET *pWriteSet)
{
    /*同时可读可写，说明异常*/
    if(XOS_INET_FD_ISSET(&(g_Log_Link.SockId),pReadSet) || g_Log_Link.SockId.fd <= 0)
    {
        printf("\r\nLog_TcpSelectWProc(),tcp client fd[port:%d] = %d disconnect,try reconnect!\r\n",
                                        g_Log_Link.SockId.fd,  g_Log_Link.LocalAddr.port);
        /*关定时器*/
        Log_StopTcpReconnTimer();
        
        /*停止链路*/
        Log_CloseTcp();

        g_serverLinkStatus = LOG_LINK_INIT;

        /*开启重连接定时器*/
        Log_StartTcpReconnTimer();                   
    }
    else
    {                            
        if(XSUCC != XINET_TcpConnectCheck(&(g_Log_Link.SockId)))/*连接失败*/
        {
            //printf("\r\nLog_tcpCliRcvFunc()->connect tcp server failed! myaddr.ip = 0x%08x,myaddr.port = %d;peeraddr.ip = 0x%08x,peeraddr.port = %d\r\n",
            //g_Log_Link.LocalAddr.ip, g_Log_Link.LocalAddr.port, 
            //g_Log_Link.ServerAddr.ip, g_Log_Link.ServerAddr.port);
                
            /*从write 集中清除,等待定时器到期进行处理*/
            Log_ClearWriteSet(); 
        }
        else/*连接成功*/
        {
            Log_ClearWriteSet();/*从 写 集中清除*/
            Log_SetReadSet();   /*添加到 读 集合中*/
            
            /*客户端连接成功*/
            if(g_serverLinkStatus != LOG_LINK_CONNECTED)
            {
                g_serverLinkStatus = LOG_LINK_CONNECTED;
                
                //printf("\r\nLog_tcpCliRcvFunc(),connect tcp server successed!\r\n");
                /*关闭重连接定时器*/
                Log_StopTcpReconnTimer();
                /*开启握手定时器*/
                Log_StartShakeTimer();
             }
        }
    }

}

/************************************************************************
 函数名:Log_TcpSelectRProc
 功能:  处理网络读事件
 输入:  
        pReadSet--读集
        pWriteSet--写集
 输出:
 返回:  
 说明: 
************************************************************************/
XVOID Log_TcpSelectRProc(t_FDSET *pReadSet, t_FDSET *pWriteSet)
{
    XCHAR *pData = NULL;
    XS32 len = 0;
    XS32 ret = 0;
    
    /*可读或有异常，都通过这个case来处理*/
    if((pReadSet != XNULLP) && XOS_INET_FD_ISSET(&(g_Log_Link.SockId),pReadSet))
    {
        pData = (XCHAR*)XNULLP;
        len = XOS_INET_READ_ANY;

        /*从网络上收数据*/
        ret = XINET_RecvMsg(&(g_Log_Link.SockId),(t_IPADDR*)XNULLP,
            &pData,&len,XOS_INET_STREAM,XOS_INET_CTR_COMPATIBLE_TCPEEN);
        if(ret == XINET_CLOSE)/*连接断开*/
        {
            printf("\r\nLog_TcpSelectRProc(),tcp client disconnect,try reconnect!\r\n");

            /*停止握手定时器*/
            Log_StopShakeTimer();
            
            /*停止链路*/
            Log_CloseTcp();

            g_serverLinkStatus = LOG_LINK_INIT;

            /*开启重连接定时器*/
            Log_StartTcpReconnTimer();             
        }
        else if(ret != XSUCC)
        {
            printf("\r\nLog_TcpSelectRProc()->receive tcp data error[ret = %d]!\r\n",ret);
        }
        if(pData != XNULL && len > 0)
        {
            /*接收数据*/
            Log_DealTcpStream((XU8*)pData, (XU16)len);

            /*释放内存*/
            XOS_MemFree(FID_LOG, pData);
            
        }
    }
}

/************************************************************************
 函数名:Log_TcpCliRcvFunc
 功能:  网络事件监听任务，捕捉读写事件
 输入:  
 输出:
 返回:  
 说明: 
************************************************************************/
XVOID  Log_TcpRecv(XVOID* taskPara)
{
    const XU32 timeOut = 2;
    XS16 setNum = 0;
    t_FDSET tmpCliRead;
    t_FDSET tmpCliWrite;   
    XS16 ret = 0;
    
    while(1)
    {
        XOS_MutexLock(&(g_LogFdSet.mutex));

        /*copy read set */
        XOS_MemCpy(&tmpCliRead,&(g_LogFdSet.readSet),sizeof(t_FDSET));
            
        /*copy write set*/
        XOS_MemCpy(&tmpCliWrite,&(g_LogFdSet.writeSet),sizeof(t_FDSET));
        
        ret = XINET_Select(&tmpCliRead, &tmpCliWrite, (XU32*)&timeOut,(XS16*)&setNum);
        if(ret != XSUCC)
        {
            /*select 超时*/
            if(ret == XINET_TIMEOUT)
            {
                goto AGAIN;
            }
            else  
            { 
                XOS_Sleep(1);                
                goto AGAIN;
            }
        }
    
        if(setNum > 0)
        {           
           if( XOS_INET_FD_ISSET(&(g_Log_Link.SockId), &tmpCliWrite))
           {
                Log_TcpSelectWProc(&tmpCliRead, &tmpCliWrite);
           }
           else if(XOS_INET_FD_ISSET(&(g_Log_Link.SockId),&tmpCliRead))
           {
                Log_TcpSelectRProc(&tmpCliRead, &tmpCliWrite);
           }
        }     
AGAIN:
        XOS_MutexUnlock(&(g_LogFdSet.mutex));
        XOS_Sleep(1);
    }
}

/************************************************************************
 函数名:Log_SendTcpData
 功能:  发送数据给服务器
 输入:  
 输出:
 返回:  成功返回XSUCC , 失败返回XERROR
 说明: 目前不进行错误处理，发送失败则进行丢弃
************************************************************************/
XS32 Log_SendTcpData(XCONST XCHAR *pData, XS32 dataLen)
{
    XS32 out_len = 0;
    
    XOS_MutexLock(&(g_LogFdSet.mutex));
    XINET_SendMsg(&g_Log_Link.SockId, eTCPClient, NULL, dataLen, (XCHAR*)pData, &out_len);
    XOS_MutexUnlock(&(g_LogFdSet.mutex));

    /*失败暂不处理*/
    return XSUCC;    
}


#ifdef __cplusplus
}
#endif
