/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xoslog.c
**   author          date              modification
     maliming        2013.12.17          ��������,����tcp����Զ�̱���
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
#define MAX_TCP_BUF_LEN   (5000)    /*����Ļ���*/
#define MAX_LOG_TCP_DATA_LEN (512+11)  /*����ĳ���*/
#define MIN_LOG_TCP_DATA_LEN (11)      /*��С���ĳ���*/
#define MAX_SHAKE_TIMEOUT (60000)      /*���ֶ�ʱ������*/
#define MAX_RETCP_TIMEOUT  (4000)      /*tcp�������*/
#define LOG_MAX_SHAKE_SEND (3)        /*������ֳ�ʱ����*/
#define LOG_TEST_MSG ("test")
#define LOG_XOS_XML ("xos.xml")       /*xos.xml�����ļ�*/
#define LOG_CONFIG_NUM (5)
#define LOG_ADDR_LEN (32)
const char *ppLogTip[] = 
    {"pl_mini","pl_debug","pl_info","pl_warn","pl_err","pl_exception","pl_log", "pl_max"};

const char *ppLogLinkStatus[] = {"null","init","start","connecting","connected", "stop"};
/*Զ����־��������·״̬*/
typedef enum 
{
    LOG_LINK_NULL = 0,
    LOG_LINK_INIT,
    LOG_LINK_START,
    LOG_LINK_CONNECTING,
    LOG_LINK_CONNECTED,
    LOG_LINK_STOP
}LINK_STATUS;

/*��ʱ������*/
typedef enum
{
    LOG_INVALID = 0,
    LOG_TIMER_SHAKE = 1,    /*���ֶ�ʱ��*/
    LOG_TIMER_TEST = 2,
    LOG_TIMER_RETCP = 3     /*tcp����*/
}LOG_TIMER_TYPE;


e_PRINTLEVEL   g_lPrintToLogLevel            = PL_ERR; /*�����������־�ļ�����Ϣ����*/
XU8 uAutoTest = 0;                 /*�Զ�����*/

t_LogCfg g_LogCfg = {0};           /*log����˲�������*/
t_LogStatus g_LogStatus = {{0}, {0}, NULL, 0, 0};     /*LOG״̬*/

XBOOL g_logInitFlag = (XBOOL)FALSE;      /*�Ƿ��Ѿ���ʼ��*/

/*��·״̬*/
const XPOINT g_LogLinkNumber = 1;       /*��·��*/
HLINKHANDLE g_LogLinkHaddle = NULL;   /*ntl�������·���*/ 
XU8 g_serverLinkStatus = 0;           /*��־��������·״̬*/

/*����ȫ��TCP��Ϣ���ݻ���*/
XSTATIC XU16 usSrvIpDataLen = 0;
XSTATIC XU8  ucSrvIpDataBuf[MAX_TCP_BUF_LEN];

XSTATIC PTIMER g_LogShakeTimer;       /*���ֶ�ʱ�����*/
XSTATIC PTIMER g_LogTcpReconnTimer;       /*tcp�����Ӷ�ʱ��*/
XS32   g_LogexpireTimes;    /*��ʱ����ʱ����*/


XU32 g_AutotestInterval = 200;        /*�Զ����Լ��*/

XU32 g_LogShakeSend = 0;              /*�����������ִ���*/

XU32   g_LogRecordSn = 0;             /*��־��¼���к�*/

t_MSGFIDCHK g_msgqFidChk[FID_MAX];    /*ÿ��ģ�������ͳ��*/

typedef struct{
t_SOCKSET readSet;
t_SOCKSET writeSet;
t_XOSMUTEXID mutex;

}LogFdSet;

LogFdSet g_LogFdSet;
/*��·����*/
logTcpSocket g_Log_Link;
t_XOSTASKID g_LogRecvTaskId;
t_XOSMUTEXID g_LogWriteMutex;






/********************************** 
��������    : Log_GetRecordSn
����        : 
�������    : 
��������    : ������־��¼�����к�
����        : ��
����ֵ      : XU32  
************************************/
XU32 Log_GetRecordSn()
{
    return ++g_LogRecordSn;
}

/********************************** 
��������    : LOG_CheckFidFlow
����        : 
�������    : 
��������    : ģ��������麯��
����        : XU32 chk_fid
����ֵ      : XS32  
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

    /*�������Ϊ0���򲻽��й��ؼ��*/
    if(0 == g_msgqFidChk[chk_fid].fid_msgmax)
    {
        return XSUCC;
    }

    g_msgqFidChk[chk_fid].fid_msgcount++;


    l_now = time(XNULL);

    time_delta = l_now - (time_t)(g_msgqFidChk[chk_fid].fid_msgchktime);

    /*���30�������һ�����ģ������Զ����*/
    if(time_delta >30 )
    {
        /*���ڷ�ֵ����0.5����,����������,����ǰʱ��*/
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
            /*δ��ʱ,����*/
            return XSUCC;
        }
        else
        {
            /*δ��ʱ,����*/
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
            /*��ʱ,���أ��Զ����*/
            g_msgqFidChk[chk_fid].fid_msgcount = 0;
            g_msgqFidChk[chk_fid].fid_msgfilter = 0;
            g_msgqFidChk[chk_fid].fid_msgpeakcount = 0;
            g_msgqFidChk[chk_fid].fid_msgchktime = (XU32)l_now;
            return XSUCC;
        }
        else
        {
            /*��ʱ,��������*/
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
��������    : Log_CliCommandInit
����        : Jeff.Zeng
�������    : 2008��3��21��
��������    : 
����ֵ      : XSTATIC XS32 
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

    /*����д�뵽log�Ĵ�ӡ����*/
    reg_result = XOS_RegistCommand(ret, Log_CmdSetLogLevel,
                "setprinttologlev", "set output to log file filter level",
                "���ô�ӡ��log�Ĵ�ӡ����\r\n"        \
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
    

    /*����log��������ַ*/
    reg_result = XOS_RegistCommand(ret, Log_CmdRestartLink,
                 "restartserver", "delete old log connection and build new one",
                 "example:restartserver localport remoteip remoteport");
    if ( XERROR >= reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for logconn failed\r\n");
    }

    /*��ʾ������·״̬������*/
    reg_result = XOS_RegistCommand(ret,Log_CmdShowInfo,
                 "showloglinkinfo", "show log configuration","example:showloglinkinfo");
    if ( XERROR >= reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for loginfo failed,error num=%d\r\n",reg_result);
    }

    /*��������������*/
    reg_result = XOS_RegistCommand(ret, Log_CmdSetFidCheck,
                "setfidcheck", "set given fid peak msg count limit",
                "example:setfidcheck fid msg_max\r\n if msg_max=0, no check ");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for setfidcheck failed,error num=%d\r\n",reg_result);
    }


    /*��ʾָ��fid�������*/
    reg_result = XOS_RegistCommand(ret,Log_CmdShowFidCheck,
                "showfidcheck", "show fid check",
                "example:showfidcheck [fid]  \r\nfid=0 show all");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for showfidcheck failed,error num=%d\r\n",reg_result);
    }

    /*�ر�Զ����־*/
    reg_result = XOS_RegistCommand(ret,Log_CmdCloseRemoteLog,
               "closeremotelog", "close remote log",
               "example:closeremotelog");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for closeremotelog failed,error num=%d\r\n",reg_result);
    }


    /*����Զ����־*/
    reg_result = XOS_RegistCommand(ret,Log_CmdOpenRemoteLog,
              "openremotelog", "open remote log",
              "example:openremotelog");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("\r\nLog CliInit,call XOS_RegistCommand for openremotelog failed,error num=%d\r\n",reg_result);
    }

    /*��ʾָ��ģ���log����*/
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
 ������:Log_SendShakeReqToRemote
 ����:  ��������������
 ����:  pBackPara ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:Log_SendShakeRspToRemote
 ����:  ����������Ӧ����
 ����:  pBackPara ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:Log_SendDataToRemote
 ����:  ������־����
 ����:  pBackPara ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

    /*��·������*/
    if(LOG_LINK_CONNECTED != g_serverLinkStatus)
    {
        //XOS_CliInforPrintf("\r\nLog_SendDataToRemote()->LOG_LINK_CONNECTED != g_serverLinkStatus[%d]\r\n", g_serverLinkStatus);
        return XERROR;
    }
    
    /*�������*/
    if(XSUCC != Log_CheckFidFlow(logid))
    {
        return XERROR;
    }

    /*������Ϣ����*/
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
��������    : Log_SpackRefresh
����        : Zenghaixin
�������    : 2013��8��13��
��������    : ���ܵ�log��־��С������500M���ڣ�
            : ֻ�������µ�500M��־
����        : XVOID
����ֵ      : XS32 
************************************/
XS32 Log_SpackRefresh ( XVOID )
{
    XCHAR szLogDir[XOS_MAX_PATHLEN + 1] = {0};
    XS32 files_in_log_dir = 0; //logĿ¼���ļ�����
    XSFILESIZE files_size_in_log_dir = 0; //logĿ¼��log�ļ��ܴ�С
    XS32 files_in_log = 0; //logĿ¼��log�ļ��ܸ���
    XS32 nIndex = 0, delete_flag = 0;
    XCHAR filename[(XOS_MAX_PATHLEN + 1)*2+1];
    XUFILESIZE size = 0;
    struct dirent **namelist = NULL;

    XOS_MemSet(szLogDir, 0x0, XOS_MAX_PATHLEN + 1);
    
#ifdef XOS_EW_START
    /*����ȫ·��log �ļ���*/
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
        //if( namelist[index]->d_type & DT_DIR ) //��Ŀ¼������
        //{
        //    continue;
        //}
        if ( 0 != XOS_StrNcmp(namelist[nIndex]->d_name, "xoslog_", 7 ) )//log�ļ���xoslog_��ͷ
        {
            free(namelist[nIndex]);   
            continue;
        }
        sprintf(filename,"%s%s", szLogDir, namelist[nIndex]->d_name);   
        //printf("for_test: filename:(%s)\r\n",filename);
        if( 1 == delete_flag )//delete_flag��λ��ֱ��ɾ��
        {
            remove(filename);
            //printf("for_test: ɾ���ļ�(%s)\r\n", filename);  
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
            //printf("for_test: ɾ���ļ�(%s)\r\n", filename);  
            delete_flag = 1;//��ʾɾ��֮ǰ���ɵ�����log�ļ� 
        }
        free(namelist[nIndex]);   
    }

    free(namelist);   

    return XSUCC;
}

#elif (defined(XOS_WIN32))
XS32 Log_SpackRefresh ( XVOID )
{  
    /*�˺���ֻ֧��windows 32λ*/
    XCHAR szLogDir[XOS_MAX_PATHLEN + 1] = {0};
    XCHAR szCurDir[(XOS_MAX_PATHLEN + 1)*2+1] = {0};
    XCHAR szTmpDir[(XOS_MAX_PATHLEN + 1)*2+1] = {0};

    XS32 handle = XSUCC;  
    struct _finddata_t fileinfo;  
    XU32 nCount = 0;
    XS32 nTotalSize = 0;

#ifdef XOS_EW_START
    /*����ȫ·��log �ļ���*/
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

    /*�����������*/
    if(nCount > MAX_LOGS)
    {
        /*ɾ�������*/
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

    /*����ܴ�С����*/
    if(nTotalSize > MAX_LOGS_SIZE)
    {
        /*ɾ�������*/
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
 ������:Log_Write
 ����:  ����ӡ��Ϣ���͸�logģ���Լ�
 ����:  
        logId    --��ӡ��fid
        level --��ӡ�ļ���
        logmsg --��ӡ������
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��: �˺���������xos_trace,ֻ����printf�����򣬻��γɺ���������ѭ��������ջ�����
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

    /*�����־����*/
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

    /*��������*/
    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        /*������Ϣ����Ӧ�ý���Ϣ�ڴ��ͷ�*/
        XOS_MemFree((XU32)FID_LOG, logLocal._pszMsg);
        XOS_MsgMemFree((XU32)FID_LOG, pMsg);
        //printf("\r\nLog_SendToLog->XOS_MsgSend failed\r\n");
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
 ������:Log_Write
 ����:  ����ӡ��Ϣ���͸�logģ��
 ����:  
        logid    --��ӡ��fid
        infotype --��ӡ�ļ���
        msg_out --��ӡ������
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:�˺���������xos_trace,ֻ����printf�����򣬻��γɺ���������ѭ��������ջ�����
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
           /*������Ϣ�ļ��𲻹���¼����־�ļ���*/
           return XSUCC;
        }
    }
    else{}/*��ʾδע��ģ��,ҲҪ��ӡ��־*/        

    /*���͵�LOG����*/
    XOS_MutexLock(&g_LogWriteMutex);
    
    Log_SendToLog(logid, infotype, (XCHAR *)msg_out);

    XOS_MutexUnlock(&g_LogWriteMutex);

    return XSUCC;
}

/************************************************************************
 ������:Log_Write
 ����:  ����ӡ��Ϣ���͸�logģ��
 ����:  
        logid    --��ӡ��fid
        infotype --��ӡ�ļ���
        level   --��־�Ĺ��˼���
        msg_out --��ӡ������
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:�˺���������xos_trace,ֻ����printf�����򣬻��γɺ���������ѭ��������ջ�����
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
           /*������Ϣ�ļ��𲻹���¼����־�ļ���*/
           return XSUCC;
        }
    }
    else{}/*��ʾδע��ģ��,ҲҪ��ӡ��־*/        

    /*���͵�LOG����*/
    XOS_MutexLock(&g_LogWriteMutex);
    
    Log_SendToLog(logid, infotype, (XCHAR *)msg_out);

    XOS_MutexUnlock(&g_LogWriteMutex);

    return XSUCC;
}
/********************************** 
��������    : Log_TraceTaskProc
����        : 
�������    : 2008��3��21��
��������    : ����LOGģ����Ϣ���У�
              1�����������Զ����־���ܣ�����־���͸�Զ�̷�������
              2��д�����ļ�(win32,linux, vxworks��д����)
����        :  
            logId  -- 
            level  --
            logmsg --

����ֵ      : XS32 
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

    
    /*�ж�Զ���Ƿ���Զ����־*/
    if(g_LogCfg._nEnableRemote) 
    {
        if(XSUCC != Log_SendDataToRemote((XU32)logId, level, logmsg, msgLen))
        {
            //XOS_CliInforPrintf("\r\nILog_TraceTaskProc->Log_SendDataToRemote failed\r\n");
        }
    }
    
    /* û�ж���VXWORKS��ʱ����ʼ��¼��������־�ļ���*/
#ifndef XOS_VXWORKS
    /*�жϴ��̿ռ��Ƿ��㹻�������ӡ��ʾ��Ϣ*/
    if(XSUCC != Log_GetSysDiskCap(&diskcap))
    {
        return XERROR;
    }
    /*�жϴ��̿ռ��Ƿ��㹻�������ӡ��ʾ��Ϣ*/
    if ( diskcap <= MIN_CAP )
    {
        XOS_CliInforPrintf("\r\ncan not write log ->the disk is full!\r\n");
        return XERROR;
    }
    
    XOS_StrNcpy(strlog, logmsg, msgLen);
    strlog[LOG_ITEM_MAX] = '\0';

    /*����ļ��Ƿ���*/
    Log_CheckFileFull();
    
    /*д��־�ļ�*/
    Log_FormatWriteLog(strlog, g_LogStatus._pLogFileHandle);
    fflush(g_LogStatus._pLogFileHandle);

#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_WIN32))
    Log_SpackRefresh();/*���ܵ�log��־��С������500M����*/
#endif

#endif  //no define vxworks
    return XSUCC;
}
/********************************** 
��������    : Log_CheckFileFull
����        : 
�������    : 2013��8��13��
��������    : ����ļ��Ƿ�д����д��ʱ���������µ���־�ļ�
����        : 
����ֵ      : ���Ҫ�ļ����ڣ��򷵻�ʵ�ʴ�С������ļ������ڣ��򷵻�-1
************************************/
XS32 Log_CheckFileFull(void)
{
    XSFILESIZE usFileLen = 0;

    usFileLen = Log_GetFileSize(g_LogStatus._szCurrLogname);
    if (usFileLen <= XERROR || (XUFILESIZE)usFileLen >= (g_LogCfg._nLogSize * MIN_LOGFILESIZE))
    {
        /*��־�ļ��������رյ�ǰ��־�ļ�ָ��*/
        if ( XNULLP != g_LogStatus._pLogFileHandle )
        {
            XOS_Fclose(g_LogStatus._pLogFileHandle);
            g_LogStatus._pLogFileHandle = NULL;
        }


        /*�޸�ȫ�ֱ������ڼ�¼��־�ļ���Ϣ*/
        g_LogStatus._uCurSize++;
        g_LogStatus._uSeqNum++;

        /*�����µ���־�ļ���*/
        if(XSUCC != Log_CreateRunFileName(g_LogStatus._szCurrLogname, XTRUE))
        {
            XOS_CliInforPrintf("\r\nLOG_write,create new log file failed!\r\n");
            return XERROR;
        }
        
        /*���µ���־�ļ�*/
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
��������    : Log_GetSysDiskCap
����        : lxn; adjust Zengguanqun
�������    : 2008��3��21��
��������    : 
����        : XU32 *cap
����ֵ      : XS32 
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
    /*ϵͳӲ�̴�С�ṹ*/
    if(disk_acess_times++  < 1000)
    {
        /*��Ƶ������*/
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
    *cap = ((unsigned long long)diskStatfs.f_bfree * diskStatfs.f_bsize) >> 10;//diskStatfs.f_bfreeǿ��ת��Ϊ64λ���Է�ֹ��˽�����
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
��������    : Log_GetFileSize
����        : lxn; adjust Zengguanqun
�������    : 2008��3��21��
��������    : ����K�Ĵ�С
����        : XCHAR*file
����ֵ      : XS32 
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
��������    : Log_FormatWriteLog
����        : 
�������    : 2013��8��12��
��������    : ��ʽ����־��Ϣ��д��־�ļ�
����        : str ��־��Ϣ
����ֵ      : XS32 
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

/*****************************************�ڲ�Ҫ�õĺ���****************/
/**-*-*-*-*-*-*-*-**-*-*-*-*-**-*-*-*-*-**-*-*-*-*-**\

��  ��:��ȡlogģ���ʼ��ʱ�䣬������Ϊ�ַ�����ʽ

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
��������    : LOG_CreateRunFileName
����        : lxn; adjust Zengguanqun
�������    : 2008��3��21��
��������    : 
����        : XCHAR *filename
����        : XBOOL flag
����ֵ      : XS32  
************************************/
XS32  Log_CreateRunFileName(XCHAR *filename,XBOOL flag)
{
    XCHAR szLogDir[XOS_MAX_PATHLEN+1] = {0};
    XOS_MemSet(szLogDir,0x0,XOS_MAX_PATHLEN+1);
#ifdef XOS_EW_START
    /*����ȫ·��log �ļ���*/
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

    /*����ļ�Ŀ¼�Ƿ����*/
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
 ������:Log_SendShakeMsg
 ����:  ����������Ϣ
 ����:  pBackPara ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_SendShakeReqMsg(void)
{    
    XS32 ret = 0;

    if(LOG_LINK_CONNECTED == g_serverLinkStatus)
    {
        if(g_LogShakeSend < LOG_MAX_SHAKE_SEND)
        {
            /*����������Ϣ*/
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

            /*��·״̬������ȴ�ղ���������Ӧ����˵���Զ��쳣(fin��δ����)������·*/
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
 ������:Log_SendShakeRspMsg
 ����:  ����������Ӧ��Ϣ
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_SendShakeRspMsg(void)
{
    return Log_SendShakeRspToRemote();
}

/************************************************************************
 ������:Log_RecvShakeReqProc
 ����:  ��������������Ϣ
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_RecvShakeReqProc(XU8* pData, XU16 len)
{
    /*���ļ��*/
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
    
    /*����������Ӧ*/
    return Log_SendShakeRspMsg();
}

/************************************************************************
 ������:Log_RecvShakeRspProc
 ����:  ����������Ӧ��Ϣ
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_RecvShakeRspProc(XU8* pData, XU16 len)
{    
    /*���ļ��*/
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

    /*���ּ�������0*/
    g_LogShakeSend = 0;

    return XSUCC;

}

/************************************************************************
 ������:Log_DealNtlStreamProc
 ����:  �����յ�����־����
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_DealTcpStreamProc(XU8* pData, XU16 len)
{
    t_LogDataHead* MsgHead = XNULL;

    /*ͷ������Ԥ��*/
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
        case e_SHAKEREQ:    /*����������Ϣ*/
            Log_RecvShakeReqProc(pData, len);
            break;
        case e_SHAKERSP:    /*������Ӧ��Ϣ*/
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
 ������:Log_DealNtlStream
 ����:  �����ntlģ����յ���tcp��, �ֽ��һ�������ı���
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

        /* 4�ֽ�ΪFLAG��ͷ��β */
        if(pktLen  <= usSrvIpDataLen)
        {
            if(0x7e != ucSrvIpDataBuf[pktLen - 2] || 0x0d != ucSrvIpDataBuf[pktLen - 1])
            {
                XOS_CliInforPrintf("\r\nLog_DealNtlStream: EndFlag is wrong!\r\n");
                usSrvIpDataLen = 0x00;
                break;
            }

            Log_DealTcpStreamProc(ucSrvIpDataBuf, (XU16)(pktLen)) ;

            /*�˴�Ӧ���Ż�������һ�����ζ��У������ڴ��ƶ�*/
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
 ������:Log_TraceMsgPro
 ����:  trace��Ϣ����
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:Log_StartShakeTimer
 ����:  �������ֶ�ʱ��
 ����:  
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_StartShakeTimer(void)
{
    t_PARA timerpara;
    t_BACKPARA backpara = {0};

    /*��ʼ���ֶ�ʱ��*/
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
 ������:Log_StopShakeTimer
 ����:  �ر����ֶ�ʱ��
 ����:  
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_StopShakeTimer(void)
{
    /*ֹͣ��ʱ��*/
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
 ������:Log_TimerProc
 ����:  ��ʱ��������
 ����:  pBackPara ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
            /*����������Ϣ*/
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
 ������:Log_MsgProc
 ����:  ģ����Ϣ������
 ����:  
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_MsgProc(XVOID* pMsgP, XVOID* pRVoid)
{

    XS8 ret = 0 ;  /* ���ر�־*/

    t_XOSCOMMHEAD* pMsg = NULL;

    XOS_UNUSED(pRVoid);

    if(NULL == pMsgP)
    {
        XOS_CliInforPrintf("\r\nLog_MsgProc()->pMsgP is null\r\n");
        return XERROR;
    }

    pMsg = (t_XOSCOMMHEAD*)pMsgP;

    /*�ж���Ϣ��Դ*/
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
 ������:Log_NoticeProc
 ����:  ģ��֪ͨ����
 ����:  
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:Log_Init
 ����:  ��ʼ������
 ����:  pBackPara ����ʱ��ָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_Init (XVOID* p1, XVOID* p2)
{
    XU32 fid_index = 0;
    XS32 ret = 0;

    XOS_UNUSED(p1);
    XOS_UNUSED(p2);

    /*��ʼ������ʱ��*/
    Log_GetLoctime(g_LogStatus._szSystemUptime);

    /*��ʼ���������Ʊ�*/
    XOS_MemSet(g_msgqFidChk, 0x0, (FID_MAX*sizeof(t_MSGFIDCHK)));

    /*��ȡ�����ļ�xos.xml�е���־���ù���*/
    if (!XML_ReadLogCfg(&g_LogCfg, XOS_CliGetXmlName())) 
    {
        XOS_CliInforPrintf("\r\nINFO:LOG_INIT->read xos.xml file for logcfg failed\r\n");
        return XERROR;
    }

    /*����FID�����������ֵ*/
    for(fid_index = 0; fid_index <FID_MAX; fid_index++)
    {
       g_msgqFidChk[fid_index].fid_msgmax = g_LogCfg._nMaxFlow;
    }
    if(g_msgqFidChk[FID_TELNETD].fid_msgmax != 0)
    {
        g_msgqFidChk[FID_TELNETD].fid_msgmax = 100;  /*telnetģ����������ƶ�������*/
    }

    /*��ʼ��������־�ļ�*/
    Log_InitLocalLogFile();
    
    /*����Ĭ�ϼ���*/
    g_lPrintToLogLevel = PL_ERR;

    /*ע�ᶨʱ��*/
    ret = XOS_TimerReg(FID_LOG, 100, 5, 0);
    if( ret != XSUCC)
    {
        XOS_CliInforPrintf("\r\nlog init,XOS_TimerRegfailed\r\n");
        return XERROR;
    }


    XOS_MutexCreate(&g_LogWriteMutex);
    XOS_MutexCreate(&g_LogFdSet.mutex);
    
    /*���������߳�*/
    ret = XOS_TaskCreate("Tsk_log_recv",TSK_PRIO_LOWEST,NTL_RECV_TSK_STACK_SIZE,
                        (os_taskfunc)Log_TcpRecv,NULL,&g_LogRecvTaskId);    

    /*��ʼ��������*/
    if(XSUCC != Log_CliCommandInit(SYSTEM_MODE))
    {
        return XERROR;
    }    
    
    /*��־ģ���ʼ�����*/
    g_logInitFlag  = XTRUE;

    return(XSUCC);
}



/************************************************************************
 ������:Log_InitLocalLogFile
 ����:  ��ʼ��������־�ļ�
 ����:  
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
    
    
    /*������־�ļ���ʼ����VXWORKS�²���Ҫ��ʼ����ֻ���WIN32��LINUX*/
    /*--����������־�ļ���Ŀ¼ VTA��VxWorksû�д���ϵͳ,�ļ���Ŀ¼������֧��--*/
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
 ������:XOS_LogSetLevel
 ����:  ���ô�ӡ����־��log����,��ҵ��ͨ��API���е���
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8  XOS_LogSetFidLevel(XU32 fid, e_PRINTLEVEL logLevel)
{
    t_FIDCB *pFidCb = NULL;

    if(fid >= FID_MAX)
    {
        //printf("\r\nXOS_LogSetLevel()->fid error\r\n");/*traceģ�����û������*/
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
        /*������־��ӡ����*/
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
 ������:Log_CmdSetLogLevel
 ����:  ���ô�ӡ����־��log����
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
    /*��������ģ��*/
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
            
            /*������־��ӡ����*/
            pFidCb->logLevel = logLevel;       
        }
        
        XOS_CliExtPrintf(pCliEnv,"set all fid log to %s print level ok\r\n", ppLogTip[logLevel]);
    }
    else
    {
        pFidCb = (t_FIDCB *)MOD_getFidCb(fid);
        if(NULL != pFidCb)
        {
            /*������־��ӡ����*/
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
 ������:Log_CmdShowFidCheck
 ����:  ��ʾ��������
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

    /*�Ƿ�fidʱ����ʾ���е�*/
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
 ������:Log_CmdShowFidCheck
 ����:  �����������ֵ����ֵ
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

   /*�������*/
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
 ������:Log_CmdShowInfo
 ����:  logģ��״̬��������ʾ
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

    /*��·״̬*/
    if(g_LogCfg._nEnableRemote)
    {        
        XOS_CliExtPrintf(pCliEnv,"log link status %s\r\n", ppLogLinkStatus[g_serverLinkStatus]);
    }

    return ;
}

/************************************************************************
 ������:Log_CmdShowFidLevel
 ����:  ��ʾlog����
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
        /*��ʾ����ģ�����־����*/
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
 ������:Log_CmdRestartLink
 ����:  �ؽ�log������·
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

    /*����Զ����־������²���Ч*/
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
 ������:Log_CmdCloseRemoteLog
 ����:  �ر�Զ����־����
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:Log_CmdOpenRemoteLog
 ����:  ��Զ����־����
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

        /*��ʼ������·*/
        Log_TcpLinkStart();     
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"remote log was runned\r\n");
    }
}


/************************************************************************
������: Log_tskStackReason
���ܣ�  ��¼����ջԭ��
���룺
�����
���أ�
˵����
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
������: Log_tskStackInfoVx
���ܣ�  ��¼����ջԭ��
���룺
�����
���أ�
˵����
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
������: LOG_TskStackInfo
���ܣ�  ��¼����ջ��Ϣ
���룺
�����
���أ�
˵����
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

#ifdef XOS_WIN32  /*��֧��WIN32*/ 
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
 ������:XOS_LogSetLinkPara
 ����: ������־ģ��Ĳ�������
 ����:  
     enable:      �Ƿ��Զ����־,0Ϊ�رգ�1Ϊ��
     localPort:   ����tcp�˿�
     remoteAddr:  ��־��������ַ
     remotePort:  ��־�������˿�
     flowCtrol:   �������ƴ�С��0:��������������
 ���:
 ����:  ���ӳɹ�����XSUCC , δ���ӷ���XERROR
 ˵��:
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

    /*д��xos.xml*/
    if(XFALSE == Log_WriteConfigToXml(LOG_XOS_XML, enable, localPort, remoteAddr, remotePort, flowCtrol))
    {
        XOS_CliInforPrintf("Log_WriteConfigToXml()->failed!");
        return XERROR;
    }

    /*����򿪣�������*/
    if(1 == g_LogCfg._nEnableRemote)
    {
        Log_LinkStopAndInit();
    }
    else
    {
        /*�ر�,�ɶ�ʱ���ر�*/        
    }

    return XSUCC;
}

/************************************************************************
 ������:XOS_LogGetLinkPara
 ����:  ��ȡ��־ģ��Ĳ�������
 ����:  
 ���:
        enable:      �Ƿ��Զ����־,0Ϊ�رգ�1Ϊ��
        localPort:   ����tcp�˿�
        remoteAddr:  ��־��������ַ
        remotePort:  ��־�������˿�
        flowCtrol:   �������ƴ�С��0:��������������
        
 ����:  ���ӳɹ�����XSUCC , δ���ӷ���XERROR
 ˵��:
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
 ������:XOS_LogGetLinkStatus
 ����:  ��ȡ��־ģ�����·״̬
 ����:  
 ���:
 ����:  ���ӳɹ�����XSUCC , δ���ӷ���XERROR
 ˵��:
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
 ������:Log_WriteConfigToXml
 ����:  �����ò���д�뵽xos.xml
 ����:  
 ���:
 ����:  д��ɹ�����XTRUE , д��ʧ�ܷ���XFALSE
 ˵��:
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


    /*�Ҹ��ڵ�*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    /*���ڵ�*/
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
    /*LOGCFG���ڵ�*/
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "LOGCFG" ) )
        {
            break;
        }
        cur = cur->next;
    }

    /*���û��LOGCFG����д���ӽڵ�*/
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

        /*û���ӽ��*/
        if ( cur == XNULL )
        {
            /*���������ӽ��*/
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
            /*�޸Ĵ��ڵĽ�㡣*/
            while(cur != XNULL )
            {
                if ( !XOS_StrCmp(cur->name, "ENABLEREMOTE" ) )
                {
                    XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", enable);
                    xmlSetNodeContent(cur, szBuf);
                    szFlag[0] = 1;
                }
                /*LOG������IP��ַ*/
                if ( !XOS_StrCmp(cur->name, "LOG_SERVER" ) )
                {
                    XOS_IptoStr(remoteAddr, szBuf);
                    xmlSetNodeContent(cur, szBuf);
                    szFlag[1] = 1;        
                }
                /*LOG�������˿�*/
                if ( !XOS_StrCmp(cur->name, "LOG_TCP_PORT" ) )
                {
                    XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", remotePort);
                    xmlSetNodeContent(cur, szBuf);
                    szFlag[2] = 1;           
                }
                /*���ض˿ں�*/
                if ( !XOS_StrCmp(cur->name, "LOCAL_TCP_PORT") )
                {
                    XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", localPort);
                    xmlSetNodeContent(cur, szBuf);
                    szFlag[3] = 1;            
                }
                /*��������*/
                if ( !XOS_StrCmp(cur->name, "LOG_FLOW" ) )
                {
                    XOS_Sprintf(szBuf, sizeof(szBuf)-1, "%d", flowCtrol);
                    xmlSetNodeContent(cur, szBuf);
                    szFlag[4] = 1;        
                }
                cur = cur->next;
            }

            /*���Ӳ����ڵĽ�㡣*/
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
    
    /*�������ĵ�*/
    newDoc = xmlCreateWriter("", NULL);
    if(NULL != newDoc) 
    {
        newRootNode = xmlGetRootNode(newDoc);
        if(NULL != newRootNode)
        {   /*�滻Ҫ�ڵ�*/
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
 ������:Log_SetReadSet
 ����:  ���ö���
 ����:  
 ���:
 ����:  
 ˵��:
************************************************************************/
XVOID Log_SetReadSet(void)
{
    XOS_INET_FD_SET(&(g_Log_Link.SockId),&(g_LogFdSet.readSet.fdSet));
}

/************************************************************************
 ������:Log_SetWriteSet
 ����:  ����д��
 ����:  
 ���:
 ����:  
 ˵��:
************************************************************************/
XVOID Log_SetWriteSet(void)
{
    XOS_INET_FD_SET(&(g_Log_Link.SockId),&(g_LogFdSet.writeSet.fdSet));

}

/************************************************************************
 ������:Log_SetRWSet
 ����:  ���ö�д��
 ����:  
 ���:
 ����:  
 ˵��:
************************************************************************/
XVOID Log_SetRWSet(void)
{
    XOS_INET_FD_SET(&(g_Log_Link.SockId),&(g_LogFdSet.readSet.fdSet));
    XOS_INET_FD_SET(&(g_Log_Link.SockId),&(g_LogFdSet.writeSet.fdSet));
}

/************************************************************************
 ������:Log_ClearWriteSet
 ����:  �������
 ����:  
 ���:
 ����:  
 ˵��:
************************************************************************/
XVOID Log_ClearReadSet(void)
{
    XOS_INET_FD_ZERO(&(g_LogFdSet.readSet.fdSet));
}

/************************************************************************
 ������:Log_ClearWriteSet
 ����:  ���д��
 ����:  
 ���:
 ����:  
 ˵��:
************************************************************************/
XVOID Log_ClearWriteSet(void)
{
    XOS_INET_FD_ZERO(&(g_LogFdSet.writeSet.fdSet));
}

/************************************************************************
 ������:Log_ClearRWSet
 ����:  �����д��
 ����:  
 ���:
 ����:  
 ˵��:
************************************************************************/
XVOID Log_ClearRWSet(void)
{
    XOS_INET_FD_ZERO(&(g_LogFdSet.readSet.fdSet));
    XOS_INET_FD_ZERO(&(g_LogFdSet.writeSet.fdSet));
}

/************************************************************************
 ������:Log_StartTcpReconnTimer
 ����:  ��ʼtcp�����Ӷ�ʱ��
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:Log_StopTcpReconnTimer
 ����:  ֹͣtcp������ʱ��
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS32 Log_StopTcpReconnTimer(void)
{
    XS32 ret = 0;

    
    /*ֹͣ��ʱ��*/
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
 ������:Log_TcpLinkStart
 ����:  ��ʼtcp��·����
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

        /*�����д��*/
        Log_SetWriteSet();
        /*���������Ӷ�ʱ��*/
        Log_StartTcpReconnTimer();
        return XERROR;
    }
    else
    {
        g_serverLinkStatus = LOG_LINK_CONNECTED;
        /*����ɶ���*/
        Log_SetReadSet();
    }
    
    return XSUCC;
}

/************************************************************************
 ������:Log_CloseTcp
 ����:  �ر�tcp
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS32 Log_CloseTcp(void)
{
    /*��ն�д��*/
    Log_ClearRWSet();

    g_serverLinkStatus = LOG_LINK_STOP;
    
    /*�ر�socket*/
    if ( XSUCC != XINET_CloseSock(&(g_Log_Link.SockId)))
    {
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
 ������:Log_StopLink
 ����:  ֹͣ��·
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_StopLink(void)
{
    XOS_MutexLock(&g_LogFdSet.mutex);
    /*ֹͣ���ֶ�ʱ��*/
    Log_StopShakeTimer();

    /*ֹͣ������ʱ��*/
    Log_StopTcpReconnTimer();

    /*ֹͣ��·*/    
    Log_CloseTcp();    

    XOS_MutexUnlock(&g_LogFdSet.mutex);

    return XSUCC;
}

/************************************************************************
 ������:Log_LinkStopAndInit
 ����:  ֹͣ����ʼ����·
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 Log_LinkStopAndInit(void)
{   

    XOS_MutexLock(&g_LogFdSet.mutex);
    /*ֹͣ���ֶ�ʱ��*/
    Log_StopShakeTimer();

    /*ֹͣ������ʱ��*/
    Log_StopTcpReconnTimer();

    /*ֹͣ��·*/    
    Log_CloseTcp();    
    
    /*���¿�����·*/
    Log_TcpLinkStart();

    XOS_MutexUnlock(&g_LogFdSet.mutex);

    return XSUCC;
}


/************************************************************************
 ������:Log_TcpReConnect
 ����:  ������·��ʱ������
 ����:  
 ���:
 ����:  
 ˵��: 
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
                /*�ر�tcp*/
                Log_CloseTcp();
                g_serverLinkStatus = LOG_LINK_INIT;
                g_LogexpireTimes = 0;

                /*�ȴ�4�������*/
            }
            else{} /*�����ȴ�*/
            break;
        case LOG_LINK_INIT:                
            {
                //printf("\r\nLog_TcpReConnect start reconnect\r\n");
                Log_StopTcpReconnTimer();
                Log_TcpLinkStart();
            }
            break;
        case LOG_LINK_CONNECTED:
            /*�رն�ʱ��*/
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
 ������:Log_TcpSelectWProc
 ����:  ��������д�¼�
 ����:  
        pReadSet--����
        pWriteSet--д��
 ���:
 ����:  
 ˵��: 
************************************************************************/
void Log_TcpSelectWProc(t_FDSET *pReadSet, t_FDSET *pWriteSet)
{
    /*ͬʱ�ɶ���д��˵���쳣*/
    if(XOS_INET_FD_ISSET(&(g_Log_Link.SockId),pReadSet) || g_Log_Link.SockId.fd <= 0)
    {
        printf("\r\nLog_TcpSelectWProc(),tcp client fd[port:%d] = %d disconnect,try reconnect!\r\n",
                                        g_Log_Link.SockId.fd,  g_Log_Link.LocalAddr.port);
        /*�ض�ʱ��*/
        Log_StopTcpReconnTimer();
        
        /*ֹͣ��·*/
        Log_CloseTcp();

        g_serverLinkStatus = LOG_LINK_INIT;

        /*���������Ӷ�ʱ��*/
        Log_StartTcpReconnTimer();                   
    }
    else
    {                            
        if(XSUCC != XINET_TcpConnectCheck(&(g_Log_Link.SockId)))/*����ʧ��*/
        {
            //printf("\r\nLog_tcpCliRcvFunc()->connect tcp server failed! myaddr.ip = 0x%08x,myaddr.port = %d;peeraddr.ip = 0x%08x,peeraddr.port = %d\r\n",
            //g_Log_Link.LocalAddr.ip, g_Log_Link.LocalAddr.port, 
            //g_Log_Link.ServerAddr.ip, g_Log_Link.ServerAddr.port);
                
            /*��write �������,�ȴ���ʱ�����ڽ��д���*/
            Log_ClearWriteSet(); 
        }
        else/*���ӳɹ�*/
        {
            Log_ClearWriteSet();/*�� д �������*/
            Log_SetReadSet();   /*��ӵ� �� ������*/
            
            /*�ͻ������ӳɹ�*/
            if(g_serverLinkStatus != LOG_LINK_CONNECTED)
            {
                g_serverLinkStatus = LOG_LINK_CONNECTED;
                
                //printf("\r\nLog_tcpCliRcvFunc(),connect tcp server successed!\r\n");
                /*�ر������Ӷ�ʱ��*/
                Log_StopTcpReconnTimer();
                /*�������ֶ�ʱ��*/
                Log_StartShakeTimer();
             }
        }
    }

}

/************************************************************************
 ������:Log_TcpSelectRProc
 ����:  ����������¼�
 ����:  
        pReadSet--����
        pWriteSet--д��
 ���:
 ����:  
 ˵��: 
************************************************************************/
XVOID Log_TcpSelectRProc(t_FDSET *pReadSet, t_FDSET *pWriteSet)
{
    XCHAR *pData = NULL;
    XS32 len = 0;
    XS32 ret = 0;
    
    /*�ɶ������쳣����ͨ�����case������*/
    if((pReadSet != XNULLP) && XOS_INET_FD_ISSET(&(g_Log_Link.SockId),pReadSet))
    {
        pData = (XCHAR*)XNULLP;
        len = XOS_INET_READ_ANY;

        /*��������������*/
        ret = XINET_RecvMsg(&(g_Log_Link.SockId),(t_IPADDR*)XNULLP,
            &pData,&len,XOS_INET_STREAM,XOS_INET_CTR_COMPATIBLE_TCPEEN);
        if(ret == XINET_CLOSE)/*���ӶϿ�*/
        {
            printf("\r\nLog_TcpSelectRProc(),tcp client disconnect,try reconnect!\r\n");

            /*ֹͣ���ֶ�ʱ��*/
            Log_StopShakeTimer();
            
            /*ֹͣ��·*/
            Log_CloseTcp();

            g_serverLinkStatus = LOG_LINK_INIT;

            /*���������Ӷ�ʱ��*/
            Log_StartTcpReconnTimer();             
        }
        else if(ret != XSUCC)
        {
            printf("\r\nLog_TcpSelectRProc()->receive tcp data error[ret = %d]!\r\n",ret);
        }
        if(pData != XNULL && len > 0)
        {
            /*��������*/
            Log_DealTcpStream((XU8*)pData, (XU16)len);

            /*�ͷ��ڴ�*/
            XOS_MemFree(FID_LOG, pData);
            
        }
    }
}

/************************************************************************
 ������:Log_TcpCliRcvFunc
 ����:  �����¼��������񣬲�׽��д�¼�
 ����:  
 ���:
 ����:  
 ˵��: 
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
            /*select ��ʱ*/
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
 ������:Log_SendTcpData
 ����:  �������ݸ�������
 ����:  
 ���:
 ����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��: Ŀǰ�����д���������ʧ������ж���
************************************************************************/
XS32 Log_SendTcpData(XCONST XCHAR *pData, XS32 dataLen)
{
    XS32 out_len = 0;
    
    XOS_MutexLock(&(g_LogFdSet.mutex));
    XINET_SendMsg(&g_Log_Link.SockId, eTCPClient, NULL, dataLen, (XCHAR*)pData, &out_len);
    XOS_MutexUnlock(&(g_LogFdSet.mutex));

    /*ʧ���ݲ�����*/
    return XSUCC;    
}


#ifdef __cplusplus
}
#endif
