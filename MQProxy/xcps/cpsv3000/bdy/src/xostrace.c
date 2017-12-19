/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: 
**
**  description:
**
**  author: 
**
**  date:   2006.3.30
**
**************************************************************
**                          history
**
***************************************************************
**   author          date                   modification
**   guolili         2006.3.30              create
**   lxn             2006.8.3               modify
**   lxn             2006.8.17              modify  主要完成win32 和linux下的不同版本以及写记录文件
**   lxn             2006.9.6               modify  完成将trace作为单独的功能块设计，增加注册功能块，处理消息队列中消息函数
                                                    调用log功能写日志文件等、以及不同系统版本的兼容性改进
**   lixn            2006.11.20             完成三期设计
**   zgq             2007.11.10             代码检视及功能清理
**   zgq             2009.09.04             功能更新,清理内存消息,内存trace功能
**************************************************************/

#ifdef  __cplusplus
extern  "C"
{
#endif

#include "xostrace.h"
#include "xoslog.h"

#include "xosmodule.h"
#include "xosencap.h"
#include "xosport.h"
#include "xoshash.h"

////////////////////////////////////VxWorks/////////////////////////////////////////
#ifdef   XOS_VXWORKS
XEXTERN t_XOSTT gMsec; /*单板时钟调整变量*/
#endif
////////////////////////////////////VxWorks/////////////////////////////////////////

/*Trace模块全局变量定义*/
XEXTERN e_PRINTLEVEL g_lPrintToLogLevel;
XEXTERN char gszPrefixInfo[CLI_PRE_FIX_LEN];
XSTATIC t_XOSMUTEXID g_no_fid_trace_mutext; 


XSTATIC XCHAR*   g_PrintLevelName[PL_MAX]   = {"PL_MIN" ,"PL_DBG", "PL_INFO","PL_WARN","PL_ERR","PL_EXP","PL_LOG"};
XSTATIC XCHAR*   g_PrintDevName[MAX_DEV]    = {"COSOLE","LOGFILE","COSOLE_AND_FILE","TELNET","SERVER"};

/*所有功能块关闭标志,[SWITCH_OFF]表示关闭*/
XSTATIC XU32     g_ulTraceAllFidCloseFlg    = SWITCH_ON;

/*trace功能块还没有启动*/
XSTATIC XU16     g_ulTraceIsSetup           = SWITCH_OFF;

//add by gcg
//控制是否将打印信息输出的主控板开关
XEXTERN XU32     g_ulPrinToMcbFlg;
XEXTERN t_FIDCB * MOD_getFidCb(XU32 fid);
//!add
XVOID TRC_CMDSetPrinToMcbFlg(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
XS32 Trace_regCliCmd(int cmdMode);

XS32 XOS_TraceCheckLevel(t_FIDCB *pFidCb, e_PRINTLEVEL level);
XS32 XOS_TraceCheckSessionLevel(t_FIDCB *pFidCb, e_PRINTLEVEL level,e_PRINTLEVEL minLevel);


/*内部函数说明*/
/********************************内部函数*************************************/
/*-----------------------------------------------
函数名：TR_sendmsg
功  能：trace内对消息内容进行填充并发送
输  入: fid   -要填充msg体的FID功能块号
        level -信息级别
        str   -消息中message体
输  出:
返  回：成功-XSUCC;失败-XERROR
------------------------------------------------*/
XVOID TRC_sendmsg(XU32 fid,e_PRINTLEVEL level,XCHAR *str)
{
    t_XOSCOMMHEAD* sendmsg = XNULL;/*发送到trace模块的消息*/
    XU32           msglen  = 0;    /*记录消息长度*/
    
    msglen  = (XU32)XOS_StrLen( str )+1;
    sendmsg = XOS_MsgMemMalloc( FID_TRACE, msglen );
    
    if ( XNULLP == sendmsg )
    {
        XOS_CliInforPrintf("XOS_Trace()->malloc msg failed  !\r\n");
        return;
    }
    
    /*填写消息数据*/
    sendmsg->datasrc.FID    = fid;
    sendmsg->datasrc.FsmId  = 0;
    sendmsg->datasrc.PID    = XOS_GetLocalPID();
    sendmsg->datadest.FID   = (XU32)FID_TRACE;
    sendmsg->datadest.FsmId = 0;
    sendmsg->datadest.PID   = XOS_GetLocalPID();
    sendmsg->msgID          = 0;
    sendmsg->msgID          = (XU16)level;
    sendmsg->subID          = 0;
    sendmsg->prio           = eNormalMsgPrio;
    sendmsg->length         = msglen;
    
    XOS_MemCpy(sendmsg->message, str, msglen);
    
    /*发送消息*/
    if ( XSUCC != XOS_MsgSend( sendmsg ) )
    {
        XOS_CliInforPrintf("TRC_sendmsg,FID %d msg level [%d] failed!\r\n",
            fid,level);
        XOS_MsgMemFree(fid,sendmsg );
        return;
    }
}

/*-----------------------------------------------
函数名：formateloctime
功  能：取系统时间并处理为年-月-日- 时间的格式
输  入:datet--记录时间串的指针
输  出:
返  回：成功-实际格式化长度;失败-XERROR
------------------------------------------------*/
XS32 TRC_formateloctime(XCHAR *datet)
{
    time_t        lt;
    t_trc_systime tmptime;
    
    if ( XNULLP == datet )
    {
        return XERROR;
    }
    
    time( &lt );
#ifdef XOS_VXWORKS
    lt = lt + gMsec;
#endif
    
    XOS_MemCpy(&tmptime, localtime(&lt), sizeof(t_trc_systime));
    return XOS_Sprintf(datet,
        MAX_FILE_NAME_LEN,
        "%04d-%02d-%02d %02d:%02d:%02d",
        tmptime.dt_year+1900,
        tmptime.dt_mon+1,
        tmptime.dt_mday,
        tmptime.dt_hour,
        tmptime.dt_min,
        tmptime.dt_sec);    
}

/************************************************************************
函数名:  
功  能:  需要给模块管理模块提供的改变fid中trace信息结构体中各个控制开关的回调函数
输  入:
输  出:
返  回: 前一个fid 设置信息后的param
说  明:
************************************************************************/
XVOID *TRC_cbsetfidlevel(t_FIDTRACEINFO *pFidTraceInfo, XVOID *param,XVOID *param2)
{
    pFidTraceInfo->traceLevel =*(e_PRINTLEVEL*)param;

    /*设置当前telnet终端的trace级别*/
    pFidTraceInfo->sessionLevel[*(XS32*)param2] = *(e_PRINTLEVEL*)param;
    return param;
}

////////////////////////////////////Trace Cli/////////////////////////////////////////
/*---------------------------------------------------------------------
函数名：  TRC_settotask
功  能：  开启对应fid功能块，将打印到终端的信息发送给trace模块消息队列标志，
          trace模块将根据消息统一处理打印信息。
输  入：  fid -功能块ID
输  出:
返回值:
说  明：
------------------------------------------------------------------------*/
XVOID TRC_settotask (CLI_ENV *pCliEnv,XU32 fid,XBOOL openflag)
{
    t_FIDTRACEINFO* fidtrace = XNULL;
    if ( !XOS_isValidFid(fid) )
    {
        XOS_CliExtPrintf(pCliEnv,"input para fid %d is wrong\r\n",fid);
        return;
    }

    fidtrace = MOD_getFidTraceInfo( fid );
    if ( XNULL == fidtrace )
    {
        XOS_CliExtPrintf(pCliEnv,"get fid %d traceinfo return null!\r\n",fid);
        return;
    }
    
    fidtrace->isPrintInTraceTsk = openflag;
    if(XTRUE == openflag)
    {
        XOS_CliExtPrintf(pCliEnv,"set fid %d trace msg output to trace queue\r\n",fid);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"set fid %d trace msg output to terminal(telnet|console)\r\n",fid);
    }
    return;
    
}
/*---------------------------------------------------------------------
函数名：  
功  能：  打开[所有功能块的打印开关]函数
输  入：  pCliEnv     - 输入，命令行终端号
输  出：  打印的字符个数
返回值:   XSUCC  -  成功          XERROR -  失败
说  明：
------------------------------------------------------------------------*/
XS32 TRC_settraceopenall (CLI_ENV *pCliEnv,XBOOL openflag)
{
    
    g_ulTraceAllFidCloseFlg = openflag;
    if(XTRUE == openflag)
    {
        XOS_CliExtPrintf(pCliEnv,"SWITCH_ON all module trace\r\n");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"SWITCH_OFF all module trace\r\n");
    }
    return  XSUCC;
}

/*---------------------------------------------------------------------
函数名：  XOS_TraceSetFidLevel
功  能：  打开[某功能块的打印开关]函数
输  入：  ulFid           - 输入，功能ID
          ePrintLevel     - 输入，打印级别
输  出：  
返回值:   XSUCC  -  成功          XERROR -  失败
说  明：
------------------------------------------------------------------------*/
XS32 XOS_TraceSetFidLevel (XU32  ulFid, e_PRINTLEVEL ulLevel)
{
    t_FIDTRACEINFO* tmp  = XNULL; /*fid中trace信息的结构体*/
    XS32 i =0;
    
    if ( !XOS_isValidFid(ulFid) )
    {
        printf("input para module FID %d is wrong\r\n",ulFid);
        return XERROR;
    }
    
    if ( ulLevel >= PL_MAX||ulLevel < PL_MIN )
    {
        printf("input module FID %d print level %d is wrong\r\n",ulFid,ulLevel);
        return XERROR;
    }
    
    /*MOD_setFidTraceLevel(ulFid, ulLevel );*/
    tmp = MOD_getFidTraceInfo(ulFid);
    XOS_LogSetFidLevel(ulFid,ulLevel);
    
    if ( XNULL != tmp )
    {
        tmp->traceLevel = ulLevel;
        /*设置所有telnet终端的trace级别*/
        for(i = 0;i < kCLI_MAX_CLI_TASK;i++)
        {
            tmp->sessionLevel[i] = ulLevel;
        }
        
        printf("set module FID [%d] print level to %s\r\n",
                ulFid,
                g_PrintLevelName[ulLevel]);
    }
    else
    {
        printf("get FID %d traceinfo return null!\r\n",ulFid);
    }
    
    return  XSUCC;
    
}

/*---------------------------------------------------------------------
函数名：  
功  能：  打开[某功能块的打印开关]函数
输  入：  ulFid           - 输入，功能ID
          ePrintLevel     - 输入，打印级别
输  出： 
返回值:   XSUCC  -  成功          XERROR -  失败
说  明：
------------------------------------------------------------------------*/
XS32 TRC_setfidlevel (CLI_ENV *pCliEnv,XU32  ulFid, e_PRINTLEVEL ulLevel)
{
    t_FIDTRACEINFO* tmp  = XNULL; /*fid中trace信息的结构体*/
    
    if ( !XOS_isValidFid(ulFid) )
    {
        XOS_CliExtPrintf(pCliEnv,"input para module FID %d is wrong\r\n",ulFid);
        return XERROR;
    }
    
    if ( ulLevel >= PL_MAX||ulLevel < PL_MIN )
    {
        XOS_CliExtPrintf(pCliEnv,"input module FID %d print level %d is wrong\r\n",ulFid,ulLevel);
        return XERROR;
    }
    
    /*MOD_setFidTraceLevel(ulFid, ulLevel );*/
    tmp = MOD_getFidTraceInfo(ulFid);
    XOS_LogSetFidLevel(ulFid,ulLevel);

    if ( XNULL != tmp )
    {
        tmp->traceLevel = ulLevel;
        /*设置当前telnet终端的trace级别*/
        tmp->sessionLevel[MMISC_GetFsmId(pCliEnv)] = ulLevel;
        
        XOS_CliExtPrintf(pCliEnv,
            "set module FID [%d] print level to %s\r\n",
            ulFid,
            g_PrintLevelName[ulLevel]);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"get FID %d traceinfo return null!\r\n",ulFid);
    }
    
    return  XSUCC;
    
}
/***********************************************************************
函数名: 
功  能: 设置所有fid功能块的tracelevel打印级别
输  入: ulLevel --打印级别
        pCliEnv --命令行终端号
输  出:
返回值: 成功返回XSUCC， 失败返回XERROR
*************************************************************************/
XS32 TRC_setalllevel(CLI_ENV *pCliEnv,e_PRINTLEVEL ulLevel)
{
    XU32 level = (XU32) ulLevel;
    XS32 session = 0;
    
    if ( ulLevel < PL_MIN
        || ulLevel >= PL_MAX )
    {
        XOS_CliExtPrintf(pCliEnv,
            "input print level %d is wrong\r\n",
            ulLevel);
        return XERROR;
    }

    session = MMISC_GetFsmId(pCliEnv);
    /*调用xosmodule.h中提供的回调函数设置所有的fid的相关信息*/
    if ( XSUCC != MOD_setAllTraceInfo(TRC_cbsetfidlevel,&level,&session) )
    {
        XOS_CliExtPrintf(pCliEnv,"set print tracelevel failed!\r\n");
        return XERROR;
    }
    
    XOS_CliExtPrintf(pCliEnv,"all FID tracelevel have set to %s\r\n",g_PrintLevelName[ulLevel]);
    
    return XSUCC;
    
}
/*---------------------------------------------------------------------
函数名：  XOS_TraceCheckLevel
功  能：  检查trace与log打印级别
输  入：
输  出：
返回值:
说  明：
------------------------------------------------------------------------*/
XS32 XOS_TraceCheckLevel(t_FIDCB *pFidCb, e_PRINTLEVEL elevel)
{
    t_FIDTRACEINFO* traceInfo = NULL;

    if(NULL == pFidCb) 
    {
        return XSUCC;
    }

    traceInfo = &(pFidCb->traceInfo);

    if(NULL == traceInfo) 
    {
        if(elevel < pFidCb->logLevel)
        {
            return XERROR;
        }
    }   
    else 
    {
        if (elevel >= traceInfo->traceLevel ) 
        {
            return XSUCC;
        } 
        else 
        {
            if (elevel >= pFidCb->logLevel)
            {
                return XSUCC;
            }
        }
        return XERROR;
    }
    
    return XSUCC;
}
/*---------------------------------------------------------------------
函数名：  XOS_TraceCheckSessionLevel
功  能：  检查trace与log打印级别
输  入：  pFidCb:模块控制块
        elevel:日志打印级别
        minLevel:日志过滤级别
输  出：
返回值:XSUCC 成功；XERROR 失败
说  明：用于telnet终端打印trace时调用
------------------------------------------------------------------------*/
XS32 XOS_TraceCheckSessionLevel(t_FIDCB *pFidCb, e_PRINTLEVEL elevel,e_PRINTLEVEL minLevel)
{
    t_FIDTRACEINFO* traceInfo = NULL;

    if(NULL == pFidCb) 
    {
        return XSUCC;
    }

    traceInfo = &(pFidCb->traceInfo);

    if(NULL == traceInfo) 
    {
        if(elevel < pFidCb->logLevel)
        {
            return XERROR;
        }
    }   
    else 
    {
        if (elevel >= minLevel ) 
        {
            return XSUCC;
        } 
        else 
        {
            if (elevel >= pFidCb->logLevel)
            {
                return XSUCC;
            }
        }
        return XERROR;
    }
    
    return XSUCC;
}
/***********************************************************************
函数名: 
功  能: 设置对应fid功能块输出信息时是否打印时间
输  入: ulFid   -功能块ID
        ulCtrl  -控制开关0,1
        pCliEnv -命令行终端号
输  出:
返回值: 成功返回XSUCC， 失败返回XERROR
*************************************************************************/
XS32 TRC_setfidtime(CLI_ENV *pCliEnv,XU32 ulFid,XBOOL ulCtrl)
{
    t_FIDTRACEINFO* fidtrace = XNULL;
    
    if ( !XOS_isValidFid(ulFid) )
    {
        XOS_CliExtPrintf(pCliEnv,"fid %d parameter wrong\n",ulFid);
        return XERROR;
    }
    
    /*判断devid是否合法*/
    if ( XTRUE != ulCtrl && XFALSE != ulCtrl )
    {
        XOS_CliExtPrintf(pCliEnv,"switch para %d wrong\r\n",ulCtrl);
        return XERROR;
    }
    fidtrace = MOD_getFidTraceInfo(ulFid);
    if ( XNULL == fidtrace )
    {
        XOS_CliExtPrintf(pCliEnv,"set FID %d traceinfo null!\r\n",ulFid);
        return XERROR;
    }
    
    fidtrace->isNeedTime = ulCtrl;
    
    if ( SWITCH_ON == ulCtrl )
    {
        XOS_CliExtPrintf(pCliEnv,"fid %d time switch is open\r\n",ulFid);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"fid %d time switch is close\r\n",ulFid);
    }
    return XSUCC;
}
/*---------------------------------------------------------------------
函数名：  
功  能：  打开对应fid功能块[打印文件名和代码行数的打印开关]函数
输  入：  pCliEnv   -命令行终端号
          ulCtrl    -打开或关闭
          fid       -功能块ID
输  出：  打印的字符个数
返回值:   XSUCC  -  成功          XERROR -  失败
说  明：
------------------------------------------------------------------------*/
XS32 TRC_setfidline (CLI_ENV *pCliEnv,XU32 fid,XBOOL ulCtrl)
{
    
    t_FIDTRACEINFO* tmpfidtrace = XNULL;
    
    tmpfidtrace = MOD_getFidTraceInfo(fid);
    
    if ( XNULL == tmpfidtrace )
    {
        XOS_CliExtPrintf(pCliEnv,"get FID %d traceinfo return null!\r\n",fid);
        return XERROR;
    }
    
    tmpfidtrace->isNeedLine     = ulCtrl;
    tmpfidtrace->isNeedFileName = ulCtrl;
    
    if ( SWITCH_ON == ulCtrl )
    {
        XOS_CliExtPrintf(pCliEnv,"SWITCH_ON FID %d for filename and line trace\r\n",fid);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"SWITCH_OFF FID %d for filename and line trace\r\n",fid);
    }
    
    return XSUCC;
    
}

/***********************************************************************
函数名: 
功  能: 设置对应fid功能块打印信息输出设备号(终端还是记录文件)函数
输  入: pCliEnv -命令行终端号
        devid   -设备记录号0,1,2
        fid     -功能块ID

输  出:
返回值: 成功返回XSUCC， 失败返回XERROR
*************************************************************************/
XS32 TRC_setoutdev (CLI_ENV *pCliEnv,XU32 fid,e_TRACEPRINTDES devid)
{
    t_FIDTRACEINFO* fidtrace   = XNULL;
    
    if ( !XOS_isValidFid(fid) )
    {
        XOS_CliExtPrintf(pCliEnv,"input para module FID %d is wrong\r\n",fid);
        return XERROR;
    }
    
    /*判断devid是否合法*/
    if ( devid > TOSERVER )
    {
        XOS_CliExtPrintf(pCliEnv,"input para devid %d is wrong\r\n",devid);
        return XERROR;
    }
    
    fidtrace = MOD_getFidTraceInfo(fid);
    
    if ( XNULL == fidtrace )
    {
        XOS_CliExtPrintf(pCliEnv,"get FID %d traceinfo return null!\r\n",fid);
        return XERROR;
    }
    
    fidtrace->traceOutputMode = devid;
    
    XOS_CliExtPrintf(pCliEnv,"set FID %d print output deviceID to %s\r\n",
        fid,g_PrintDevName[devid]);
    
    return XSUCC;

}

XEXTERN XVOID  TRC_CMDShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
/*---------------------------------------------------------------------
函数名:  
功  能:  设置对应FID的trace输出信息是否打印时间命令
输  入:  pCliEnv         - 输入，终端号
         siArgc          - 输入，命令行输入的参数个数
         ppArgv          - 输入，命令行输入的参数列表
输  出:
返  回:
说  明:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetfidtime(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*参数为FID和控制开关*/
    XU32      ulFid;
    XU32      lflag;
    
    if ( 3 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter number is wrong\r\n");
        return;
    }
    
    ulFid = atoi(ppArgv[1]);
    lflag = atoi(ppArgv[2]);
    
    if ( 1 != lflag && 0 != lflag )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter %d is wrong!\r\n",lflag);
        return;
    }
    
    /*调用设置fid输出信息打印时间的函数*/
    TRC_setfidtime(pCliEnv,ulFid,(XBOOL)lflag);
    
    return;
}

/*命令行注册使用函数*/
/*---------------------------------------------------------------------
函数名:  TRC_CMDSettotask
功  能:  设置对应FID模块输出到终端的信息，是否要发送消息给trace模块命令
输  入:  pCliEnv         - 输入，终端号
         siArgc          - 输入，命令行输入的参数个数
         ppArgv          - 输入，命令行输入的参数列表
输  出:
返  回:
说  明:
------------------------------------------------------------------------*/
XVOID TRC_CMDSettotask(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*参数分别为FID和控制开关*/
    XU32      ulFid;
    XU32      lflag;
    
    if ( 3 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
        return;
    }
    
    ulFid = atoi(ppArgv[1]);
    lflag = atoi(ppArgv[2]);
    
    if (1 != lflag && 0 != lflag )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter %d is wrong\r\n",lflag);
        return;
    }
    
    /*调用设置fid对应lflag函数*/
    if ( 1 == lflag )
    {
        TRC_settotask(pCliEnv,ulFid,XTRUE);
    }
    
    if ( 0 == lflag )
    {
        TRC_settotask(pCliEnv,ulFid,XFALSE);
    }
    
    return;
    
}
/*---------------------------------------------------------------------
函数名:  
功  能:  设置打开/关闭所有模块的打印开关命令
输  入:  pCliEnv         - 输入，终端号
         siArgc          - 输入，命令行输入的参数个数
         ppArgv          - 输入，命令行输入的参数列表
输  出:
返  回:
说  明:
------------------------------------------------------------------------*/
XVOID TRC_CMDSettraceopenall(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*参数为控制开关*/
    XU32      lflag;
    
    if ( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
        if(g_ulTraceAllFidCloseFlg)
        {
            XOS_CliExtPrintf(pCliEnv,"current all module trace switch is open\r\n");
        }
        else
        {
            XOS_CliExtPrintf(pCliEnv,"current all module trace switch is close\r\n");
        }
        return;
    }
    
    lflag = atoi(ppArgv[1]);
    
    if ( 1 != lflag && 0 != lflag )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter switch %d is wrong\r\n",lflag);
        return;
    }
    
    /*调用设置allfidprintflag函数*/
    if ( 1 == lflag )
    {
        TRC_settraceopenall(pCliEnv,XTRUE);
    }
    
    if ( 0 == lflag )
    {
        TRC_settraceopenall(pCliEnv,XFALSE);
    }
    
    return;
}
/*---------------------------------------------------------------------
函数名:  
功  能:  打开一个功能块的打印开关命令
输  入:  pCliEnv         - 输入，终端号
         siArgc          - 输入，命令行输入的参数个数
         ppArgv          - 输入，命令行输入的参数列表
输  出:
返  回:
说  明:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetfidlevel(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*参数为FID和输出级别*/
    
    XU32  ulFid;
    e_PRINTLEVEL ePrintLevel;
    
    if ( 3 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"setfidlevel <fid> <level>  --set fid printf level.\r\n");

        return;
    }
    
    ulFid       = atoi(ppArgv[1]);
    ePrintLevel = (e_PRINTLEVEL) atoi(ppArgv[2]);
    
    TRC_setfidlevel(pCliEnv, ulFid, ePrintLevel);
    
    return;
}


/*---------------------------------------------------------------------
函数名:  
功  能:  设置所有模块的trace输出信息级别命令
输  入:  pCliEnv         - 输入，终端号
         siArgc          - 输入，命令行输入的参数个数
         ppArgv          - 输入，命令行输入的参数列表
输  出:
返  回:
说  明:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetalllevel(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*参数为输出级别*/
    XU32      level;
    
    if ( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"setalllevel  <level>  --set all fid printf level.\r\n");

        return;
    }
    
    level = atoi(ppArgv[1]);
    
    /*调用设置所有fid输出级别的函数*/
    TRC_setalllevel(pCliEnv,(e_PRINTLEVEL)level);
    return;
}

/*---------------------------------------------------------------------
函数名:  
功  能:  设置对应FID的trace输出信息是否文件名和行号命令
输  入:  pCliEnv         - 输入，终端号
         siArgc          - 输入，命令行输入的参数个数
         ppArgv          - 输入，命令行输入的参数列表
输  出:
返  回:
说  明:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetfidline(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*参数为FID和控制开关*/
    
    XU32      ulFid;
    XU32      lflag;
    
    if ( 3 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter number is wrong\r\n");
        return;
    }
    
    ulFid = atoi(ppArgv[1]);
    lflag = atoi(ppArgv[2]);
    
    if ( 1 != lflag && 0 != lflag )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter %d is wrong\r\n",lflag);
        return;
    }
    
    /*调用设置相应fid打印文件名和行号的函数*/
    if ( 1 == lflag )
    {
        TRC_setfidline(pCliEnv,ulFid,(XBOOL)SWITCH_ON);
    }
    
    if ( 0 == lflag )
    {
        TRC_setfidline(pCliEnv,ulFid,(XBOOL)SWITCH_OFF);
    }
    
    return;
    
}
/*---------------------------------------------------------------------
函数名:  
功  能:  设置打印信息输出目标设备
输  入:  pCliEnv         - 输入，终端号
         siArgc          - 输入，命令行输入的参数个数
         ppArgv          - 输入，命令行输入的参数列表
输  出:
返  回:
说  明:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetoutdev(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32            lfid;
    e_TRACEPRINTDES devtype;
    
    if ( 3 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter number is wrong\r\n");
        return;
    }
    
    lfid    = atoi(ppArgv[1]);
    devtype = (e_TRACEPRINTDES)atoi(ppArgv[2]);
    
    TRC_setoutdev(pCliEnv,lfid,devtype);
    
    return;
    
}
////////////////////////////////////Trace Cli/////////////////////////////////////////

/*---------------------------------------------------------------------
函数名:  XOS_PrintInit
功  能:  打印模块初始化函数初始化trace模块，同时调用初始化log模块(作为子模块)
输  入:  pCliEnv         - 输入，终端号
         siArgc          - 输入，命令行输入的参数个数
         ppArgv          - 输入，命令行输入的参数列表
输  出:
返  回: XSUCC, 函数操作失败返回XERROR
说  明:
------------------------------------------------------------------------*/
XS8 Trace_Init(XVOID * t, XVOID *V)
{    
    /*init mutex*/
    if ( XSUCC != XOS_MutexCreate( &g_no_fid_trace_mutext) )
    {
        XOS_CliInforPrintf("XOS_MutexCreate g_no_fid_mutext failed!");
        return XERROR;
    }
    
    Trace_regCliCmd(SYSTEM_MODE);
    return(XSUCC);
}

XS32 Trace_regCliCmd(int cmdMode)
{    
    XS32 ret;
    XS32 backvalue;

    ret = XOS_RegistCmdPrompt( cmdMode, "plat", "plat", "no parameter" );
    if ( XERROR >= ret )
    {
        XOS_CliInforPrintf("trace init failed,return %d\r\n",ret);
        return XERROR;
    }
    
    if ( XERROR >=  ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetfidlevel,
        "setfidlevel", "set fid trace level",
        "para:fid 0/1 \r\n\tPL_MIN=0；\r\n\tPL_DBG=1；\r\n\tPL_INFO=2；\r\n\tPL_WARN=3；\r\n\tPL_ERR=4；\r\n\tPL_EXP=5；\r\n\tPL_LOG=6. \r\nexample:setfidlevel 1 2") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd setfidtrclev failed,return %d\r\n",backvalue);
    }
    
    if( XERROR >=  ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetalllevel,
        "setalllevel", "set all fid trace level",
        "para:\r\n\tPL_MIN=0；\r\n\tPL_DBG=1；\r\n\tPL_INFO=2；\r\n\tPL_WARN=3；\r\n\tPL_ERR=4； \r\n\tPL_EXP=5；\r\n\tPL_LOG=6.\r\nexample:setalltrclev 5\r\n" ) ))
    {
        XOS_CliInforPrintf("trace init,reg cmd setallfidtrslev failed,return %d\r\n",backvalue);
    }
    
    if ( XERROR >=  ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetfidtime,
        "setptime","set fid trace time flag",
        "para:fid 0/1\r\n\t0-close;\r\n\t1-open.\r\nexample: to close fid 1 time switch; usage:setptime 1 0") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd setptime failed,return %d\r\n",backvalue);
    }
    
    if ( XERROR >=   ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetfidline,
        "setpfileline","set fid trace filename and line flag",
        "para:fid 0/1\r\n\t0-close;\r\n\t1-open.\r\nexample:setpfileline 1 0") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd setpfileline failed,return %d\r\n",backvalue);
    }
    
    if ( XERROR >=   ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetoutdev,
        "setoutdev",
        "set fid trace output device",
        "para:fid dev \r\ndev:\r\n\t0-only console;\r\n\t1-only to logfile;\r\n\t2-both console and logfile\r\nexample:setoutdev 1 0") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd setoutdev failed,return %d\r\n",backvalue);
    }
    
    if ( XERROR >= ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSettotask,
        "settotask", "set fid trace output mode",
        "output to terminal or task queue\r\npara:fid 0/1 \r\nexample:settotask 201 1") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd settotask failed,return %d\r\n",backvalue);
    }
    
    if ( XERROR >=   ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSettraceopenall,
        "settraceflag", "set all fid trace flag(close/open)",
        "para:0/1\r\n\t0-close,1-open.\r\nexample:setprintflag 0") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd setprintflag failed,return %d\r\n",backvalue);
    }

//add by gcg
    if ( XERROR >= ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetPrinToMcbFlg,
        "setprint2mcb", "set print info to main control bord",
        "set print info to main control bord\r\npara:flag 0/1 \r\nexample:setprint2mcb 1") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd settotask failed,return %d\r\n",backvalue);
    }
//!add
    
    if ( XERROR >=   ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDShow,
        "showalltrcinfo",
        "display all fid trace information",
        "no parameter") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd showallfidtrcinfo failed,return %d\r\n",backvalue);
    }

    /*设置全局变量,trace控制输出开关以及trace功能块启动标志*/
    /*所有功能块关闭标志,[SWITCH_OFF]表示关闭*/
    g_ulTraceAllFidCloseFlg = SWITCH_ON;
    
    g_ulTraceIsSetup  = SWITCH_ON;
    
    return(XSUCC);
    
}

//begin added for cpu efficiency zenghaixin 20100802
/*---------------------------------------------------------------------
函数名：  XOS_JudgeMsgFormat
功  能：  判断格式化字符串长度正确性
输  入：  FileName      - 文件名(宏__FILE__)
          out_filename  - 用于保存由绝对路径FileName解析得到的文件名
          prt_time      - 用于保存事件发生时间
          ulFid         - 功能块ID
          fid_name      - 用于保存线程名称
          format_len    - 信息描述串长度
输  出：  out_filename  - 记录由绝对路径FileName解析得到的文件名
          prt_time      - 记录事件发生时间
          fid_name      - 线程名称

返回：    1 - 长度正确  0 - 长度非法
说  明：  该函数被XOS_Trace()函数在需要时调用，以减少无效的内存拷贝，提高CPU效率
------------------------------------------------------------------------*/
XS32 XOS_JudgeMsgFormat( const XCHAR* FileName,       /*文件名(宏__FILE__)*/
                        XCHAR    *out_filename,    /*记录由绝对路径FileName解析得到的文件名*/ 
                        XCHAR    *prt_time,        /*记录事件发生时间*/
                        XU32 ulFid,            /*功能块ID*/
                        XCHAR    *fid_name,        /*线程名称*/  
                        XS32    format_len        /*信息描述串长度*/  )
{
    XS32   msg_curlen = 0;   /*总信息串的长度记录*/
    XCHAR* fidName = NULL;
    
    /*得到文件名称串*/
    if ( XSUCC != Trace_abFileName(FileName,out_filename,MAX_FILE_NAME_LEN) )
    {
        return 0;
    }
    msg_curlen = format_len + (XU32)XOS_StrLen(out_filename);
    
    /*处理系统时间，得到对应的时间字符串*/
    if ( XSUCC >= TRC_formateloctime(prt_time) )
    {
        prt_time[0] = '\0';
    }
    msg_curlen = msg_curlen + (XU32)XOS_StrLen(prt_time);
    
    /*得到模块名称*/
    fidName = XOS_getFidName(ulFid);
    if ( XNULLP != fidName)
    {
        XOS_Sprintf(fid_name,MAX_FILE_NAME_LEN,"%s", fidName);
    }
    else
    {
        XOS_Sprintf(fid_name,MAX_FILE_NAME_LEN,"Unknown_FID[%d]",ulFid);
    }
    msg_curlen = msg_curlen + (XU32)XOS_StrLen(fid_name);
    
    if(msg_curlen > MAX_TRACE_INFO_LEN-16)
    {
        return 0;
    }
    
    return 1;
}

/*---------------------------------------------------------------------
函数名：  XOS_FillMsgFilter
功  能：  填充过滤信息串格式
输  入：  msg_filter    - 用于保存过滤信息串格式 
          eLevel        - 打印级别
          fidtrace      - 记录由fid得到的相应的trace信息控制开关结构体
          prt_time      - 记录事件发生时间
          fid_name      - 线程名称
          out_filename    - 记录绝对路径文件名 
          ulLineNum     - 行号
          msg_format    - 记录信息描述串内容，由cformat,...得到

输  出：  msg_filter  -  过滤信息串格式
说  明：  该函数被XOS_Trace()函数在需要时调用，以减少无效的内存拷贝，提高CPU效率
------------------------------------------------------------------------*/
XVOID XOS_FillMsgFilter( XCHAR *msg_filter,                     /*过滤信息串*/
                        const int               size,           /*过滤信息串大小*/
                        const e_PRINTLEVEL      eLevel,         /*打印级别*/
                        const t_FIDTRACEINFO*   fidtrace,       /*记录由fid得到的相应的trace信息控制开关结构体*/
                        const XCHAR*            prt_time,       /*记录事件发生时间*/
                        const XCHAR*            fid_name,       /*线程名称*/  
                        const XCHAR*            out_filename,   /*记录由绝对路径FileName解析得到的文件名*/ 
                        const XU32              ulLineNum,      /*行号*/
                        const XCHAR*            msg_format      /*记录信息描述串内容，由cformat,...得到*/ )
{
    XU32 msg_curlen = 0;   /*总信息串的长度记录*/
    XU32 msg_seglen = 0;    /*记录每次串的增加长度*/
    
    /*根据配置的cli开关,格式化输出字符串*/
    /*级别：{时间：文件名 ：行号}  FID名：信息描述*/
    XOS_MemSet(msg_filter,0x0,size);
    msg_curlen = XOS_Sprintf(msg_filter,size,
        "%s:{",g_PrintLevelName[eLevel]);
    if ( XTRUE == fidtrace->isNeedTime )
    {
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            size - msg_curlen,
            "%s,",prt_time);
        msg_curlen  += msg_seglen;
    }
    
    if ( XTRUE ==  fidtrace->isNeedFileName )
    {
        /*打印文件名*/
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            size - msg_curlen,
            "%s,",out_filename);
        msg_curlen  +=msg_seglen;
    }
    
    if ( XTRUE == fidtrace->isNeedLine )
    {
        /*打印行号*/
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            size - msg_curlen,
            "%d",ulLineNum);
        msg_curlen  +=msg_seglen;
    }
    
    msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
        size - msg_curlen,
        "}%s: %s\r\n",fid_name,msg_format);
    msg_curlen  +=msg_seglen;
    
    /*保证串的结束*/
    msg_filter[size -1] = '\0';
}
//end added for cpu efficiency zenghaixin 20100802

#if 0
extern int XOS_ShouldTrace(const XCHAR* file_name, XU32 line, XU32 fid, e_PRINTLEVEL level);
#endif

/*---------------------------------------------------------------------
函数名：  XOS_Trace
功  能：  信息打印函数
输  入：  FileName       - 输入，文件名(宏__FILE__)
          ulLineNum      - 输入，行号   (宏__LINE__)
          ulFid          - 输入，功能块ID
          ulLevel        - 输入，打印级别
          ucFormat       - 输入，打印格式化字符串
          ...            - 输入，打印参数

输  出：  XSUCC  -  成功          XERROR -  失败
说  明：该函数为用户输出信息接口
        如果g_ulTraceAllToTraceTsk标志关闭，则将输出到终端的信息直接打印
        如果g_ulTraceAllToTraceTsk开启，则将信息发送给trace功能模块
        对于打印到其他目标设备的信息都发送到trace功能模块进行处理
------------------------------------------------------------------------*/
#if 0
XVOID XOS_Trace( const XCHAR* FileName, XU32 ulLineNum, XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat, ... )
{
    //modified for cpu efficiency zenghaixin 20100802
    //XU32   msg_curlen                     = 0;     /*总信息串的长度记录*/
    XS32   format_len                       = 0;     /*信息描述串长度*/
    XS32   callFlag                         = 0;     
    //end modified for cpu efficiency zenghaixin 20100802
    //XU32   msg_seglen                        = 0;     /*记录每次串的增加长度*/
    XCHAR  msg_filter[MAX_TRACE_INFO_LEN ]   = {0};   /*记录过滤信息串格式*/
    XCHAR  msg_format[MAX_TRACE_INFO_LEN ]   = {0};   /*记录信息描述串内容，由cformat,...得到*/
    XCHAR  out_filename[MAX_FILE_NAME_LEN]   = {0};   /*记录由绝对路径FileName解析得到的文件名*/
    XCHAR  prt_time[MAX_FILE_NAME_LEN]       = {0};   /*记录事件发生时间*/
    XCHAR  fid_name[MAX_FILE_NAME_LEN]       = {0};   /*线程名称*/
    //XU32   test; 
    
    //#ifdef XOS_NEED_CHK
    va_list ap;
    //#endif
    t_FIDTRACEINFO* fidtrace            = XNULL; /*记录由fid得到的相应的trace信息控制开关结构体*/
    if ( SWITCH_OFF == g_ulTraceIsSetup || XNULL == cFormat)
    {
        return ;
    }

#ifdef XOS_NEED_CHK
    
    //格式化输出的各参数
    /*得到信息描述串*/
    /*级别不对*/
    if ( eLevel >= PL_MAX|| eLevel < PL_MIN )
    {
        return;
    }
 
    if ( XNULL == cFormat )
    {
        return;
    }
#if 0 
        va_start( ap, cFormat );
        //msg_curlen=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
        format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);//modified for cpu efficiency zenghaixin 20100802
        va_end( ap );
        //if(msg_curlen<=0)
        if(format_len<=0)//modified for cpu efficiency zenghaixin 20100802
        {
            return;
        }
    }
    
//#if 0 //deleteded for cpu efficiency zenghaixin 20100802
    /*得到文件名称串*/
    if ( XSUCC !=  Trace_abFileName(FileName,out_filename,MAX_FILE_NAME_LEN) )
    {
        return;
    }
    msg_curlen=msg_curlen+XOS_StrLen(out_filename);
    
    /*处理系统时间，得到对应的时间字符串*/
    if ( XERROR == TRC_formateloctime(prt_time) )
    {
        prt_time[0] = '\0';
    }
    msg_curlen=msg_curlen+XOS_StrLen(prt_time);
    
    /*得到模块名称*/
    if ( XNULLP != XOS_getFidName(ulFid) )
    {
        XOS_Sprintf(fid_name,MAX_FILE_NAME_LEN,"%s",XOS_getFidName(ulFid));
    }else
    {
        XOS_Sprintf(fid_name,MAX_FILE_NAME_LEN,"Unknown_FID[%d]",ulFid);
    }
    msg_curlen=msg_curlen+XOS_StrLen(fid_name);
    
    if(msg_curlen > MAX_TRACE_INFO_LEN-16)
    {
        return;
    }
#endif //end deleteded for cpu efficiency zenghaixin 20100802   
#else
    return;
#endif
    ///////////////
    /*得到该fid的trace开关*/
    fidtrace = MOD_getFidTraceInfo(ulFid);
    if ( XNULL == fidtrace )
    {
        va_start( ap, cFormat );
        format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
        va_end( ap );
        if(format_len<=0)
        {
            return;
        }
        //未注册模块的Trace信息且满足打印输出条件或日志条件
        if ( XOS_UNREG_FID_TRACE_LEV <= eLevel )
        {
            //added for cpu efficiency zenghaixin 20100802
            //判断格式化字符串长度正确性
            if( 0 == XOS_JudgeMsgFormat( FileName,      /*文件名(宏__FILE__)*/
                out_filename,    /*记录由绝对路径FileName解析得到的文件名*/ 
                prt_time,        /*记录事件发生时间*/
                ulFid,            /*功能块ID*/
                fid_name,        /*线程名称*/  
                format_len        /*信息描述串长度*/ ) )
            {
                return;
            }
            callFlag = 1;
            //end added for cpu efficiency zenghaixin 20100802
            XOS_PrinToMcb("%s",msg_format);
            XOS_CliInforPrintf("%s",msg_format);
        }
        
        if ( eLevel >= g_lPrintToLogLevel )
        {
            //added for cpu efficiency zenghaixin 20100802
            if(0 == callFlag)
            {
                if( 0 == XOS_JudgeMsgFormat( FileName,      /*文件名(宏__FILE__)*/
                    out_filename,    /*记录由绝对路径FileName解析得到的文件名*/ 
                    prt_time,        /*记录事件发生时间*/
                    ulFid,            /*功能块ID*/
                    fid_name,        /*线程名称*/  
                    format_len        /*信息描述串长度*/ ) )
                {
                    return;
                }
            }
            //end added for cpu efficiency zenghaixin 20100802
            //满足日志级别的必须输出
            Log_Write(ulFid,eLevel,msg_format);
        }
        return;
    }
    
#if 0 //deleteded for cpu efficiency zenghaixin 20100802
    /*根据配置的cli开关,格式化输出字符串*/
    /*级别：{时间：文件名 ：行号}  FID名：信息描述*/
    XOS_MemSet(msg_filter,0x0,sizeof(msg_filter));
    msg_curlen=0;
    msg_curlen = XOS_Sprintf(msg_filter,MAX_TRACE_INFO_LEN,
        "%s:{",g_PrintLevelName[eLevel]);
    if ( XTRUE == fidtrace->isNeedTime )
    {
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            MAX_TRACE_INFO_LEN - msg_curlen,
            "%s,",prt_time);
        msg_curlen  += msg_seglen;
    }
    
    if ( XTRUE ==  fidtrace->isNeedFileName )
    {
        /*打印文件名*/
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            MAX_TRACE_INFO_LEN - msg_curlen,
            "%s,",out_filename);
        msg_curlen  +=msg_seglen;
    }
    
    if ( XTRUE == fidtrace->isNeedLine )
    {
        /*打印行号*/
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            MAX_TRACE_INFO_LEN - msg_curlen,
            "%d",ulLineNum);
        msg_curlen  +=msg_seglen;
    }
    
    msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
        MAX_TRACE_INFO_LEN - msg_curlen,
        "}%s: %s\r\n",fid_name,msg_format);
    msg_curlen  +=msg_seglen;
    
    /*保证串的结束*/
    msg_filter[MAX_TRACE_INFO_LEN -1] = '\0';
#endif //end deleteded for cpu efficiency zenghaixin 20100802   
    ///////////////
    
    /*如果所有功能块打印控制开关都关闭，则不输出到屏幕任何信息*/
    if ( SWITCH_OFF == g_ulTraceAllFidCloseFlg )
    {
        /*日志文件还是得写*/
        if ( eLevel >= g_lPrintToLogLevel )
        {
            va_start( ap, cFormat );
            format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
            va_end( ap );
            if(format_len<=0)
            {
                return;
            }
            //added for cpu efficiency zenghaixin 20100802
            //判断格式化字符串长度正确性
            if( 0 == XOS_JudgeMsgFormat( FileName,      /*文件名(宏__FILE__)*/
                out_filename,    /*记录由绝对路径FileName解析得到的文件名*/ 
                prt_time,        /*记录事件发生时间*/
                ulFid,            /*功能块ID*/
                fid_name,        /*线程名称*/  
                format_len     /*信息描述串长度*/ ) )
            {
                return;
            }
            //填充过滤信息串格式
            XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
                prt_time, fid_name, out_filename, ulLineNum, msg_format);
            //end added for cpu efficiency zenghaixin 20100802
            Log_Write(ulFid,eLevel,msg_filter);
        }
        return;
    }
    
    /*如果此时是平台的输出信息，则转向cpstrace,减少trace造成的依赖关系*/
    if ( FID_XOSMIN <= ulFid && ulFid < FID_XOSMAX )
    {
#ifdef XOS_NEED_CHK
        //added for cpu efficiency zenghaixin 20100802
        va_start( ap, cFormat );
        format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
        va_end( ap );
        if(format_len<=0)
        {
            return;
        }
        //判断格式化字符串长度正确性
        if( 0 == XOS_JudgeMsgFormat( FileName,      /*文件名(宏__FILE__)*/
            out_filename,    /*记录由绝对路径FileName解析得到的文件名*/ 
            prt_time,        /*记录事件发生时间*/
            ulFid,            /*功能块ID*/
            fid_name,        /*线程名称*/  
            format_len     /*信息描述串长度*/ ) )
        {
            return;
        }

        XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
            prt_time, fid_name, out_filename, ulLineNum, msg_format);
        
        //end added for cpu efficiency zenghaixin 20100802
        XOS_PrinToMcb("%s",msg_filter);
        XOS_CpsTrace(FileName, ulLineNum, ulFid, eLevel, msg_format);
        return ;
#endif
    }
    
    /*FID不合理*/
    if ( XFALSE== XOS_isValidFid(ulFid))
    {
        return;
    }

#if 0
    if (XOS_ShouldTrace(FileName, ulLineNum, ulFid, eLevel) != XSUCC)
    {
        return;
    }
#endif

    if(eLevel <  fidtrace->traceLevel)
    {
        //将满足日志级别但小于Cli配置级别的Trace直接输出到日志文件中
        //一般情况下将异常的输出到logfile
        if(eLevel >= g_lPrintToLogLevel)
        {
            va_start( ap, cFormat );
            format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
            va_end( ap );
            if(format_len<=0)
            {
                return;
            }
            //added for cpu efficiency zenghaixin 20100802
            //判断格式化字符串长度正确性
            if( 0 == XOS_JudgeMsgFormat( FileName,      /*文件名(宏__FILE__)*/
                out_filename,    /*记录由绝对路径FileName解析得到的文件名*/ 
                prt_time,        /*记录事件发生时间*/
                ulFid,            /*功能块ID*/
                fid_name,        /*线程名称*/  
                format_len     /*信息描述串长度*/ ) )
            {
                return;
            }
            //填充过滤信息串格式
            XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
                prt_time, fid_name, out_filename, ulLineNum, msg_format);
            //end added for cpu efficiency zenghaixin 20100802
            Log_Write(ulFid,eLevel,msg_filter);
        }
        return ;
    }
    /*下面输出的条件*/
    /*a.Cli配置级别小于日志级别
    b.trace大于cli配置级别*/
    switch ( fidtrace->traceOutputMode )
    {
    case TOCOSOLE:
        //added for cpu efficiency zenghaixin 20100802
        va_start( ap, cFormat );
        format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
        va_end( ap );
        if(format_len<=0)
        {
            return;
        }
        //判断格式化字符串长度正确性
        if( 0 == XOS_JudgeMsgFormat( FileName,      /*文件名(宏__FILE__)*/
            out_filename,    /*记录由绝对路径FileName解析得到的文件名*/ 
            prt_time,        /*记录事件发生时间*/
            ulFid,            /*功能块ID*/
            fid_name,        /*线程名称*/  
            format_len     /*信息描述串长度*/ ) )
        {
            return;
        }
        //填充过滤信息串格式
        XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
            prt_time, fid_name, out_filename, ulLineNum, msg_format);
        //end added for cpu efficiency zenghaixin 20100802
        if ( XFALSE == fidtrace->isPrintInTraceTsk )
        {
            XOS_PrinToMcb("%s",msg_filter);
            XOS_CliInforPrintf("%s",msg_filter);
        }
        else
        {
            TRC_sendmsg(ulFid,eLevel,msg_filter);
        }
        break;
        
    case TOLOGFILE:
        XOS_CpsTrace()
        if ( eLevel >= g_lPrintToLogLevel )
        {
            va_start( ap, cFormat );
            format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
            va_end( ap );
            if(format_len<=0)
            {
                return;
            }
            //added for cpu efficiency zenghaixin 20100802
            //判断格式化字符串长度正确性
            if( 0 == XOS_JudgeMsgFormat( FileName,      /*文件名(宏__FILE__)*/
                out_filename,    /*记录由绝对路径FileName解析得到的文件名*/ 
                prt_time,        /*记录事件发生时间*/
                ulFid,            /*功能块ID*/
                fid_name,        /*线程名称*/  
                format_len     /*信息描述串长度*/ ) )
            {
                return;
            }
            //填充过滤信息串格式
            XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
                prt_time, fid_name, out_filename, ulLineNum, msg_format);
            //end added for cpu efficiency zenghaixin 20100802
            TRC_sendmsg(ulFid,eLevel,msg_filter);
        }
        break;
    case TOFILEANDCOS:
        //added for cpu efficiency zenghaixin 20100802
        va_start( ap, cFormat );
        format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
        va_end( ap );
        if(format_len<=0)
        {
            return;
        }
        //判断格式化字符串长度正确性
        if( 0 == XOS_JudgeMsgFormat( FileName,      /*文件名(宏__FILE__)*/
            out_filename,    /*记录由绝对路径FileName解析得到的文件名*/ 
            prt_time,        /*记录事件发生时间*/
            ulFid,            /*功能块ID*/
            fid_name,        /*线程名称*/  
            format_len     /*信息描述串长度*/ ) )
        {
            return;
        }
        //填充过滤信息串格式
        XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
            prt_time, fid_name, out_filename, ulLineNum, msg_format);
        //end added for cpu efficiency zenghaixin 20100802
        if ( XFALSE == fidtrace->isPrintInTraceTsk )
        {
            XOS_PrinToMcb("%s",msg_filter);
            XOS_CliInforPrintf("%s\r\n",msg_filter);
        }
        TRC_sendmsg(ulFid,eLevel,msg_filter);
        break;
    default:
        break;
        
    }
    return;
    /*当信息级别小于过滤级别时，如果过滤级别设置过大，可能导致日志文件也不能写入*/
}
#endif 

static int XOS_TraceGetParam(const char* file_name,
        XU32 *pMsg_curlen,
        const XCHAR* fid_name,
        const XU32 line,
        const e_PRINTLEVEL eLevel,
        char* out_filename,
        char* prt_time)
{
    XU32 uCurlen = 0;
    XU32 uTmpLen = 0;

    if(XNULL == pMsg_curlen ||XNULL == file_name || XNULL == out_filename ||XNULL == prt_time ||XNULL == fid_name)
    {
        return XERROR;
    }

    Trace_abFileName(file_name, out_filename, MAX_FILE_NAME_LEN);
    TRC_formateloctime(prt_time);

    uTmpLen = (XU32)XOS_StrLen(out_filename) + (XU32)XOS_StrLen(prt_time) + (XU32)XOS_StrLen(fid_name);

    uCurlen = *pMsg_curlen + uTmpLen;

    if(uCurlen > (MAX_TRACE_INFO_LEN - 16)) 
    {
        uCurlen = (MAX_TRACE_INFO_LEN - 16);

        *pMsg_curlen = uCurlen - uTmpLen;
    }

    return XSUCC;
}

static void XOS_TraceSetOutput(const e_PRINTLEVEL eLevel, const char* prt_time, const char* out_filename, const XU32 ulLineNum, 
                               const char* fid_name, const char* msg_format,
                               char *msg_output)
{
    if(XNULL == prt_time || XNULL == out_filename || XNULL == fid_name || XNULL == msg_format 
        || XNULL == msg_output || 0 > eLevel ||PL_MAX <= eLevel)
    {
        return ;
    }
    
    XOS_MemSet(msg_output, 0, MAX_TRACE_INFO_LEN);
    XOS_Sprintf(msg_output,MAX_TRACE_INFO_LEN,
        "%s:{%s,%s,%d}%s:%s",
        g_PrintLevelName[eLevel],
        prt_time,out_filename,ulLineNum,
        fid_name,msg_format);
}

static int XOS_TraceCheckFid(const t_FIDCB *pFidCb, const XU32 ulFid, const e_PRINTLEVEL eLevel, const char* prt_time, const char* out_filename, const XU32 ulLineNum, 
                             const char* fid_name, const char* msg_format,const e_PRINTLEVEL minLevel)
{
    XCHAR   msg_output[MAX_TRACE_INFO_LEN]   = {0};

    /*模块未注册*/
    if ( XNULL == pFidCb )
    {
        XOS_TraceSetOutput(eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format, msg_output);
        if ( eLevel >=  XOS_UNREG_FID_TRACE_LEV)
        {
            Log_Write(pFidCb, ulFid, eLevel, msg_output);

        }
        return XERROR;
    }

    if ( SWITCH_OFF == g_ulTraceAllFidCloseFlg || eLevel <  minLevel)
    {
        if (eLevel >= pFidCb->logLevel)
        {
            XOS_TraceSetOutput(eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format, msg_output);
            Log_Write(pFidCb, ulFid, eLevel, msg_output);
        }
        return XERROR;
    }

    return XSUCC;
}

static void XOS_TraceSetMsg(const char* fun_name,const e_PRINTLEVEL eLevel, const char* prt_time, const char* out_filename, const XU32 ulLineNum, 
                               const char* fid_name, const char* msg_format, const t_FIDTRACEINFO* fidtrace,
                               char *msg_filter)
{
    XU32    msg_curlen  = 0;
    XU32    msg_seglen = 0;

    if(XNULL ==  fidtrace)
    {
        return;
    }
    
    XOS_MemSet(msg_filter, 0x0, MAX_TRACE_INFO_LEN);
    msg_curlen = XOS_Sprintf(msg_filter,MAX_TRACE_INFO_LEN,
        "%s:{",g_PrintLevelName[eLevel]);
    if ( XTRUE == fidtrace->isNeedTime )
    {
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            MAX_TRACE_INFO_LEN - msg_curlen,
            "%s", prt_time);
        msg_curlen  += msg_seglen;
    }

    if ( XTRUE ==  fidtrace->isNeedFileName )
    {
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            MAX_TRACE_INFO_LEN - msg_curlen,
            ",%s", out_filename);
        msg_curlen  += msg_seglen;
    }
    
    msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
        MAX_TRACE_INFO_LEN - msg_curlen,
        ",%s", fun_name);
    msg_curlen  += msg_seglen;

    if ( XTRUE == fidtrace->isNeedLine )
    {
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen, MAX_TRACE_INFO_LEN - msg_curlen,
            ",%d",ulLineNum);
        msg_curlen  += msg_seglen;
    }

    // auto add \r\n
    XOS_Sprintf(msg_filter+msg_curlen, MAX_TRACE_INFO_LEN + 4 - msg_curlen,
        "}%s: %s\r\n", fid_name, msg_format);
}
/*---------------------------------------------------------------------
函数名：  XOS_CpsTrace
功  能：  针对平台内部的trace打印消息，负责打印到终端并且记录到日志文件中
输  入：      FileName  -文件名
        ulFid           - 输入，功能块ID
          ulLineNum     - 行号
          ulLevel         - 输入，打印级别
          ucFormat        - 输入，打印格式化字符串
          ...             - 输入，打印参数

输  出：
返回值:  XSUCC  -   成功； XERROR - 失败
说  明： 如果输入的长度大于MAX_TRACE_INFO_LEN-1，则会失败，且不进行提示
------------------------------------------------------------------------*/
XVOID XOS_CpsTrace( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat, ... )
{
    XS32    msg_curlen  = 0;
    XCHAR   msg_format[MAX_TRACE_INFO_LEN+1]  = {0}; 
    XCHAR   msg_filter[MAX_TRACE_INFO_LEN+5]  = {0}; /*5 bytes for \r\n*/
    XCHAR   out_filename[MAX_FILE_NAME_LEN] = {0};
    XCHAR   fid_name[MAX_FILE_NAME_LEN]     = {0};
    XCHAR   prt_time[MAX_FILE_NAME_LEN]     = {0};
    t_FIDTRACEINFO* fidtrace = NULL;
    t_FIDCB *pFidCb = NULL;
    va_list ap;
    e_PRINTLEVEL min_level = PL_MAX;    /*所有telnet终端中设置最低的打印级别，用于控制log打印*/
    XS32 i = 0;

    if (SWITCH_OFF == g_ulTraceIsSetup || XNULL == cFormat || eLevel >= PL_MAX || eLevel < PL_MIN)
    {
        return ;
    }
#if 0
    if (XOS_ShouldTrace(FileName, ulLineNum, ulFid, eLevel) != XSUCC)
    {
        return;
    }
#endif
    pFidCb = MOD_getFidCb( ulFid);

    if(XNULLP != pFidCb) 
    {
        fidtrace = &(pFidCb->traceInfo);

        min_level = pFidCb->logLevel;
        
        for(i = 0;i < kCLI_MAX_CLI_TASK;i++)
        {
            if(g_paUserSIDLogined[i].xs32SID > 0 && g_paUserSIDLogined[i].xbSwitch)
            {
                min_level = min_level < pFidCb->traceInfo.sessionLevel[g_paUserSIDLogined[i].xs32SID] ? min_level :pFidCb->traceInfo.sessionLevel[g_paUserSIDLogined[i].xs32SID];
            }
        }
    }
    else
    {
        min_level = XOS_UNREG_FID_TRACE_LEV;
    }

    /*如果输出日志的级别小于配置的级别，就直接返回*/
    if(XOS_TraceCheckSessionLevel(pFidCb, eLevel,min_level) == XERROR) 
    {
        return;
    }

    va_start(ap, cFormat);
    msg_curlen = XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat, ap);
    va_end(ap);

    if (msg_curlen <= 0)
    {
        return;
    }

    if(XNULLP != pFidCb) 
    {
        if(NULL == pFidCb->fidName) 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
        } 
        else 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "%s", pFidCb->fidName);
        }
    } 
    else 
    {
        XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
    }

    if (XOS_TraceGetParam(FileName, (XU32 *)&msg_curlen, fid_name, ulLineNum, eLevel, out_filename, prt_time) == XERROR)
    {
        return ;
    }

    msg_format[msg_curlen] = '\0';
    
    if(XOS_TraceCheckFid(pFidCb, ulFid, eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format,min_level) == XERROR)
    {
        return;
    }

    XOS_TraceSetMsg(FunName,eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format, fidtrace, msg_filter);

    switch ( fidtrace->traceOutputMode )
    {
    case TOCOSOLE: /*打印到终端*/
        if (XFALSE == fidtrace->isPrintInTraceTsk)
        {
            XOS_PrintToMcbStr(msg_filter);
        }
        XOS_CliInforPrintfSessionStr(pFidCb->traceInfo.sessionLevel,eLevel,msg_filter);

        break ;
    case TOLOGFILE: /*打印到文件*/
        Log_Write(pFidCb, ulFid,eLevel,msg_filter);
        break ;

    case TOFILEANDCOS:/*打印到终端和文件*/
        if (XFALSE == fidtrace->isPrintInTraceTsk)
        {
            XOS_PrintToMcbStr(msg_filter);
        }
        XOS_CliInforPrintfSessionStr(pFidCb->traceInfo.sessionLevel,eLevel,msg_filter);

        Log_SessionWrite(pFidCb, ulFid,eLevel,min_level,msg_filter);
        break ;
    default:
        break ;
    }
}

/*---------------------------------------------------------------------
函数名：  XOS_TraceTa
功  能：  针对平台内部的trace打印消息，负责打印到终端并且记录到日志文件中
输  入： FileName       - 文件名
        ulLineNum       - 行号
        ulFid           - 输入，功能块ID
          ulLevel         - 输入，打印级别
          ucFormat        - 输入，打印格式化字符串
          ...             - 输入，打印参数
输  出：
返回值:  XSUCC  -   成功； XERROR - 失败
说  明： 如果输入的长度大于MAX_TRACE_INFO_LEN-1，则会失败，且不进行提示
------------------------------------------------------------------------*/
XVOID XOS_TraceTa( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat, va_list ap )
{
    XS32    msg_curlen  = 0;
    XCHAR   msg_format[MAX_TRACE_INFO_LEN+1]  = {0}; 
    XCHAR   msg_filter[MAX_TRACE_INFO_LEN+5]  = {0}; /*5 bytes for \r\n*/
    XCHAR   out_filename[MAX_FILE_NAME_LEN] = {0};
    XCHAR   fid_name[MAX_FILE_NAME_LEN]     = {0};
    XCHAR   prt_time[MAX_FILE_NAME_LEN]     = {0};
    t_FIDTRACEINFO* fidtrace = NULL;
    t_FIDCB *pFidCb = NULL;
    e_PRINTLEVEL min_level = PL_MAX;    /*所有telnet终端中设置最低的打印级别，用于控制log打印*/
    XS32 i = 0;

    if (SWITCH_OFF == g_ulTraceIsSetup || XNULL == cFormat || eLevel >= PL_MAX || eLevel < PL_MIN)
    {
        return ;
    }
#if 0
    if (XOS_ShouldTrace(FileName, ulLineNum, ulFid, eLevel) != XSUCC)
    {
        return;
    }
#endif
    pFidCb = MOD_getFidCb( ulFid);

    if(XNULLP != pFidCb) 
    {
        fidtrace = &(pFidCb->traceInfo);

        min_level = pFidCb->logLevel;
        
        for(i = 0;i < kCLI_MAX_CLI_TASK;i++)
        {
            if(g_paUserSIDLogined[i].xs32SID > 0 && g_paUserSIDLogined[i].xbSwitch)
            {
                min_level = min_level < pFidCb->traceInfo.sessionLevel[g_paUserSIDLogined[i].xs32SID] ? min_level :pFidCb->traceInfo.sessionLevel[g_paUserSIDLogined[i].xs32SID];
            }
        }
    }
    else
    {
        min_level = XOS_UNREG_FID_TRACE_LEV;
    }

    /*如果输出日志的级别小于配置的级别，就直接返回*/
    if(XOS_TraceCheckSessionLevel(pFidCb, eLevel,min_level) == XERROR) 
    {
        return;
    }

    msg_curlen = XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat, ap);

    if (msg_curlen <= 0)
    {
        return;
    }

    if(XNULLP != pFidCb) 
    {
        if(NULL == pFidCb->fidName) 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
        } 
        else 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "%s", pFidCb->fidName);
        }
    } 
    else 
    {
        XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
    }

    if (XOS_TraceGetParam(FileName, (XU32 *)&msg_curlen, fid_name, ulLineNum, eLevel, out_filename, prt_time) == XERROR)
    {
        return ;
    }

    msg_format[msg_curlen] = '\0';
    
    if(XOS_TraceCheckFid(pFidCb, ulFid, eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format,min_level) == XERROR)
    {
        return;
    }

    XOS_TraceSetMsg(FunName,eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format, fidtrace, msg_filter);

    switch ( fidtrace->traceOutputMode )
    {
    case TOCOSOLE: /*打印到终端*/
        if (XFALSE == fidtrace->isPrintInTraceTsk)
        {
            XOS_PrintToMcbStr(msg_filter);
        }
        XOS_CliInforPrintfSessionStr(pFidCb->traceInfo.sessionLevel,eLevel,msg_filter);

        break ;
    case TOLOGFILE: /*打印到文件*/
        Log_Write(pFidCb, ulFid,eLevel,msg_filter);
        break ;

    case TOFILEANDCOS:/*打印到终端和文件*/
        if (XFALSE == fidtrace->isPrintInTraceTsk)
        {
            XOS_PrintToMcbStr(msg_filter);
        }
        XOS_CliInforPrintfSessionStr(pFidCb->traceInfo.sessionLevel,eLevel,msg_filter);

        Log_SessionWrite(pFidCb, ulFid,eLevel,min_level,msg_filter);
        break ;
    default:
        break ;
    }
}
/*---------------------------------------------------------------------
函数名：  XOS_TraceInfo
功  能：  log跟踪接口内调用，用于生成log字符串
输  入：  FileName       - 文件名
            ulLineNum   - 行号
          ulFid           - 输入，功能块ID
          ulLevel         - 输入，打印级别
          cFormat       - 打印格式化字符串
          ap             - 输入，va_list处理字符串类型
          msgLen             - 输入，打印的字符串的长度

输  出：
返回值:  XSUCC  -   成功； XERROR - 失败
说  明： 用于在TA模块日志过滤时调用；
        如果输入的长度大于MAX_TRACE_INFO_LEN-1，则会失败，且不进行提示；
------------------------------------------------------------------------*/
XVOID XOS_TraceInfo( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat,va_list ap ,XCHAR *buf)
{
    XS32    msg_curlen = 0;
    XCHAR   msg_format[MAX_TRACE_INFO_LEN+1]  = {0}; 
    XCHAR   out_filename[MAX_FILE_NAME_LEN] = {0};
    XCHAR   fid_name[MAX_FILE_NAME_LEN]     = {0};
    XCHAR   prt_time[MAX_FILE_NAME_LEN]     = {0};
    t_FIDCB *pFidCb = NULL;
    t_FIDTRACEINFO* fidtrace = NULL;

    if (SWITCH_OFF == g_ulTraceIsSetup || eLevel >= PL_MAX || eLevel < PL_MIN)
    {
        return ;
    }

    pFidCb = MOD_getFidCb( ulFid);
    if(XNULLP != pFidCb) 
    {
        fidtrace = &(pFidCb->traceInfo);
    }
    
    msg_curlen = XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat, ap);

    if (msg_curlen <= 0)
    {
        return;
    }

    if(XNULLP != pFidCb) 
    {
        if(NULL == pFidCb->fidName) 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
        } 
        else 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "%s", pFidCb->fidName);
        }
    }
    else 
    {
        XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
    }

    if (XOS_TraceGetParam(FileName, (XU32 *)&msg_curlen, fid_name, ulLineNum, eLevel, out_filename, prt_time) == XERROR)
    {
        return ;
    }

    msg_format[msg_curlen] = '\0';
    
    XOS_TraceSetMsg(FunName,eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format, fidtrace, buf);

}

/*---------------------------------------------------------------------
函数名：  XOS_CpsPrintf
功  能：  针对平台内部的trace打印消息，负责打印到终端并且记录到日志文件中
输  入：  ulFid            - 输入，功能块ID
          ulLevel          - 输入，打印级别
          etraceOutputMode - 输入, 输出到终端或文件 
          ucFormat         - 输入，打印格式化字符串
          ap               - 输入, 变参的栈起始地址

输  出：
返回值:  XSUCC  -   成功； XERROR - 失败
说  明： 如果输入的长度大于MAX_TRACE_INFO_LEN，则会失败，且不进行提示
------------------------------------------------------------------------*/
XVOID XOS_CpsPrintf(XU32 ulFid, e_PRINTLEVEL eLevel, e_TRACEPRINTDES etraceOutputMode, const XCHAR *cFormat, va_list ap)
{
    XCHAR   msg_format[MAX_TRACE_INFO_LEN+1]  = {0}; 
    XS32    msg_curlen  = 0;
    t_FIDCB *pFidCb = NULL;

    pFidCb = MOD_getFidCb( ulFid);

    /*如果输出日志的级别小于配置的级别，就直接返回*/
    if(XOS_TraceCheckLevel(pFidCb, eLevel) == XERROR) 
    {
        return;
    }   

#ifdef XOS_WIN32
    msg_curlen =  _vsnprintf(msg_format,sizeof(msg_format)-1, cFormat, ap);
#endif
    //#if ( XOS_LINUX  || XOS_SOLARIS )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) )
    msg_curlen = vsnprintf(msg_format, sizeof(msg_format)-1, cFormat, ap);
#endif
    //#if ( XOS_VXWORKS || XOS_VTA )
#if ( defined(XOS_VXWORKS) || defined(XOS_VTA) )
    msg_curlen = vx_vsnprintf(msg_format,sizeof(msg_format)-1,cFormat,ap);
#endif  


    if (msg_curlen <= 0)
    {
       return;
    }

    switch ( etraceOutputMode )
    {
    case TOCOSOLE: /*打印到终端*/
        //XOS_CliInforPrintfStr(msg_format);
        break ;
    case TOLOGFILE: /*打印到文件*/
        Log_Write(pFidCb, ulFid, eLevel, msg_format);
         break ;
    case TOFILEANDCOS:/*打印到终端和文件*/
        break ;
    default:
        break ;
    }
}


/*---------------------------------------------------------------------
函数名：  XOS_getPLName
功  能：  获取打印级别的名称
输  入：
输  出：
返回值:
说  明：
------------------------------------------------------------------------*/
XCHAR* XOS_getPLName(e_PRINTLEVEL printLevel)
{
    if( printLevel >= PL_MAX|| printLevel <PL_MIN )
    {
        return (XCHAR*)XNULLP;
    }
    else
    {
        return g_PrintLevelName[printLevel];
    }
}

/*---------------------------------------------------------------------
函数名：  XOS_getoutModeName
功  能：  获取打印目的设备名称
输  入：
输  出：
返回值:
说  明：
------------------------------------------------------------------------*/
XCHAR* XOS_getoutModeName(e_TRACEPRINTDES desmode)
{
    if ( desmode > MAX_DEV || desmode < TOCOSOLE )
    {
        return (XCHAR*)XNULLP;
        
    }
    else
    {
        return  g_PrintDevName[desmode];
    }
    
}

/*----------------------------------------------
打印控制命令行回调函数
-------------------------------------------------*/
/*---------------------------------------------------------------------
函数名：  XOS_Sprintf
功  能：  格式化字符串函数.与sprintf功能相同
输  入：  Buffer    -存放格式化后的字符串的缓冲区.
          charnum   -缓冲区的大小.
          cFormat   -格式化的字符串
          ...       -可以有任意个跟前面的格式化字符串匹配
输  出：
返回值: 成功返回写入的实际字节数失败返回0值
说明：
------------------------------------------------------------------------*/
XS32 XOS_Sprintf( XCHAR *Buffer, XU32 charnum, const XCHAR *cFormat, ... )
{
    XS32   length                      = 0;
    va_list ap;
    
    va_start( ap, cFormat );
    length = XOS_VsPrintf(Buffer,charnum,cFormat,ap);
    va_end(ap);

    return length;
}

/*------------------------------------------------------------------
函数名：  Trace_msgProc
功  能：  消息处理函数
输  入：
输  出：
返回值:
说  明：
------------------------------------------------------------------------*/
XS8 Trace_msgProc(XVOID* pMsP, XVOID*sb )
{
    
    t_XOSCOMMHEAD*   msg      = XNULL;
    t_FIDTRACEINFO*  fidtrace = XNULL;
    e_TRACEPRINTDES  outputmode;
    t_FIDCB* pFidCb = (t_FIDCB*)XNULLP;

    
    
    if ( XNULL == pMsP )
    {
        return XERROR;
    }
    
    msg  = (t_XOSCOMMHEAD *) pMsP;

//add by gcg
//打印业务板信息 
    if(FID_TRACE == msg->datasrc.FID
       && MSGID_PRINTO_MCB == msg->msgID
       && msg->datasrc.PID != msg->datadest.PID)
    {
        XOS_CliInforPrintf( "%s\r\n",msg->message );
        return XSUCC;
    }
//!add
    pFidCb = MOD_getFidCb( msg->datasrc.FID);
    
    /*未注册模块*/
    if ( XNULL == pFidCb )
    {
        return XERROR;
    }

    fidtrace = &(pFidCb->traceInfo);
    outputmode = fidtrace->traceOutputMode;
    
    /*根据trace模块中的信息输出目标设备开关，决定调用输出到那个设备文件中*/
    switch ( outputmode )
    {
    case TOCOSOLE:
        XOS_CliInforPrintf( "%s\r\n",msg->message );
        break;
        
    case TOFILEANDCOS:
        if ( XTRUE == fidtrace->isPrintInTraceTsk )
        {
            /*if cli msg output in thread,ignore*/
            XOS_CliInforPrintf("%s\r\n",msg->message);
        }
        Log_Write(pFidCb, msg->datasrc.FID,(e_PRINTLEVEL)msg->msgID,msg->message);
        break;
        
    case TOLOGFILE:
        Log_Write(pFidCb, msg->datasrc.FID,(e_PRINTLEVEL)msg->msgID,msg->message);
        break;
        
    case TOSERVER:
        /*待扩展*/
        break;
        
    case TOTELNET:
        /*待扩展*/
        break;
        
    default :
        break;
    }
    return XSUCC;
    
}

/******************************************************
函数名：  Trace_abFileName
功  能：  从绝对路径中解析文件名函数
输  入：  FileName        -绝对路径
          fnstr           -最终的文件名
          Max_FileNameLen -文件名的最大长度
输  出：
返回值:   成功返回xsucc;失败返回xerror;
说  明：
*******************************************************/
XS32 Trace_abFileName(XCONST XCHAR *FileName,XCHAR *fnstr,XU32 Max_FileNameLen)
{
    XU32   filelen = 0; /*记录绝对路径FileName中相对filename的长度*/
    XCHAR* pSrc    = XNULLP;
    int nameLen = 0;
    
    if ( XNULL == fnstr ||  XNULL == FileName)
    {
        return XERROR;
    }

    nameLen = (XU32)XOS_StrLen(FileName);
    
    pSrc = (XCHAR*)FileName + nameLen;
    
    /*逆序找到绝对路径中文件的上层文件夹*/
    for(filelen = 0; '\\' != *pSrc && '/' != *pSrc; pSrc--)
    {
        filelen++;
        if('.' == *pSrc)
        {
            filelen = 0;
        }
        if ( filelen > (XU32)nameLen )
        {
            break;
        }
    }
    
    filelen = XOS_MIN(filelen, Max_FileNameLen);
    
    XOS_StrNcpy(fnstr, pSrc+1, filelen);
    
    return XSUCC;
    
}

/*--------------------------------------------------------------------
函数名：  XOS_GetFidTraceFlag
功  能：  获取FID当前trace信息输出标志位
输  入：
          ulFid          -功能块ID
          ulFidTraceInfo -负责记录trace的各个标志位结构
输  出：
返回值:  成功返回XSUCC；失败返回XERROR
说  明：
------------------------------------------------------------------------*/
XS32 XOS_GetFidTraceFlag(XU32 ulFid, t_FIDTRACEINFO *ulFidTraceInfo)
{
    t_FIDTRACEINFO* fidtrace = XNULL;
    
    if ( XNULL == ulFidTraceInfo )
    {
        XOS_Trace(MD(ulFid,PL_ERR),"XOS_GetFidTraceFlag()-> input param is null!");
        return XERROR;
    }
    
    if ( !XOS_isValidFid(ulFid) )
    {
        XOS_Trace(MD(ulFid,PL_ERR),"XOS_GetFidTraceFlag()-> input ulFid is illegal!");
        return XERROR;
    }
    
    fidtrace = MOD_getFidTraceInfo(ulFid);
    
    if ( XNULL == fidtrace )
    {
        XOS_Trace(MD(ulFid,PL_ERR),"XOS_GetFidTraceFlag()-> can not get fid-trace flag!");
        return XERROR;
    }
    
    ulFidTraceInfo->isNeedFileName    = fidtrace->isNeedFileName;
    ulFidTraceInfo->isNeedLine        = fidtrace->isNeedLine;
    ulFidTraceInfo->isNeedTime        = fidtrace->isNeedTime;
    ulFidTraceInfo->isPrintInTraceTsk = fidtrace->isPrintInTraceTsk;
    ulFidTraceInfo->traceLevel        = fidtrace->traceLevel;
    ulFidTraceInfo->traceOutputMode   = fidtrace->traceOutputMode;
    
    return XSUCC;
    
}

/*--------------------------------------------------------------------
函数名：  XOS_SetFidTraceFlag
功  能：  设置FID对应trace信息输出标志位
输  入：
          ulFid           -功能块ID
          FilenameFlag    -设置是否打印文件名标志（TRUE-打印；FALSE-不打印）
          LinenumFlag     -设置是否打印行号标志（TRUE-打印；FALSE-不打印）
          TimeFlag        -设置是否打印时间标志（TRUE-打印；FALSE-不打印）
          TransToTaskFlag -设置是否传输到消息队列标志（TRUE-传；FALSE-不传）
          OutputMode      -设置输出目标设备标志（见e_TRACEPRINTDES 结构）
          OutputLevel     -设置该FID的信息打印过滤级别，输出信息级别低于该级别时将不输出（见e_PRINTLEVEL结构

输  出：
返回值:  成功返回XSUCC；失败返回XERROR
说  明：
------------------------------------------------------------------------*/
XS32 XOS_SetFidTraceFlag(XU32 ulFid,XBOOL FilenameFlag,XBOOL LinenumFlag,XBOOL TimeFlag,
                         XBOOL TransToTaskFlag, e_TRACEPRINTDES  OutputMode, e_PRINTLEVEL OutputLevel)
{
    t_FIDTRACEINFO* fidtrace = XNULL;
    
    if ( !XOS_isValidFid(ulFid) )
    {
        XOS_Trace(MD(ulFid,PL_ERR),"XOS_SetFidTraceFlag()-> input ulFid is illegal!\r\n");
        return XERROR;
    }
    
    if ( ( FilenameFlag !=  XTRUE && FilenameFlag != XFALSE )
        ||( LinenumFlag !=  XTRUE && LinenumFlag != XFALSE )
        ||( TimeFlag !=  XTRUE && TimeFlag != XFALSE )
        ||( TransToTaskFlag !=  XTRUE && TransToTaskFlag != XFALSE )
        ||( OutputMode < TOCOSOLE || OutputMode > TOSERVER )
        ||( OutputLevel < PL_MIN || OutputLevel >= PL_MAX )  )
    {
        XOS_Trace(MD(ulFid,PL_ERR),"XOS_SetFidTraceFlag()-> input TRACE FLAGS are illegal!\r\n");
        return XERROR;
    }
    
    fidtrace = MOD_getFidTraceInfo(ulFid);
    
    if ( XNULL == fidtrace )
    {
        XOS_Trace(MD(ulFid,PL_WARN), "XOS_SetFidTraceFlag()->set fid trace flag error!\r\n");
        return XERROR;
    }
    
    fidtrace->isNeedFileName = FilenameFlag;
    fidtrace->isNeedLine = LinenumFlag;
    fidtrace->isNeedTime = TimeFlag;
    fidtrace->isPrintInTraceTsk = TransToTaskFlag;
    fidtrace->traceLevel = OutputLevel;
    fidtrace ->traceOutputMode = OutputMode;
    
    return XSUCC;
}

XS8 XOS_TraceClose(XU32 switchType)
{
    if(SWITCH_OFF == switchType)
    {
        g_ulTraceIsSetup =SWITCH_OFF;
    }
    if(SWITCH_ON == switchType)
    {
        g_ulTraceIsSetup =SWITCH_ON;
    }
    return XSUCC;
}

#define gcg
/*---------------------------------------------------------------------
函数名:  
功  能:  设置是否将业务板打印信息输出到主控板
输  入:  pCliEnv         - 输入，终端号
         siArgc          - 输入，命令行输入的参数个数
         ppArgv          - 输入，命令行输入的参数列表
输  出:
返  回:
说  明:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetPrinToMcbFlg(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32      ulFlag;
    
    if ( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter number is wrong\r\n");
        return;
    }

    ulFlag = atoi(ppArgv[1]);
    
    if ( 1 != ulFlag && 0 != ulFlag )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter %d is wrong!\r\n",ulFlag);
        return;
    }
    
    XOS_CliExtPrintf(pCliEnv,"set print to mcb success(%d)!\r\n",ulFlag);
    g_ulPrinToMcbFlg = ulFlag;
    
    return;
}


XS32 XOS_PrintToMcbStr(const XCHAR* buff)
{
    if( XNULL == buff )
    {
        return XERROR;
    }

    if(g_ulPrinToMcbFlg == 1)
    {
        XCHAR szBuf[MAX_BUFF_LEN] = {0};
        t_XOSCOMMHEAD* sendmsg = XNULL;
        XU32    msglen  = 0;
        XU32    ulRemotePid = 0;

        ulRemotePid = cli_getMcbPid();
        if(0xFFFFFFFF == ulRemotePid)
        {
            return XERROR;
        }

        memset(szBuf, 0, MAX_BUFF_LEN);
        //打印信息前加上PID和IP前缀
        sprintf(szBuf,"%s %s", gszPrefixInfo, buff);

        msglen  = (XU32)XOS_StrLen(szBuf)+1;
        sendmsg = XOS_MsgMemMalloc( FID_TRACE, msglen );

        if ( XNULLP == sendmsg )
        {
            //XOS_PRINT(MD(FID_TRACE,PL_ERR),"XOS_MsgMemMalloc fail.");
            return XERROR;
        }

        /*填写消息数据*/
        sendmsg->datasrc.FID    = FID_TRACE;
        sendmsg->datasrc.FsmId  = 0;
        sendmsg->datasrc.PID    = XOS_GetLocalPID();
        sendmsg->datadest.FID   = (XU32)FID_TRACE;
        sendmsg->datadest.FsmId = 0;
        sendmsg->datadest.PID   = ulRemotePid;
        sendmsg->msgID          = MSGID_PRINTO_MCB;
        sendmsg->subID          = 0;
        sendmsg->prio           = eNormalMsgPrio;
        sendmsg->length         = msglen;

        XOS_MemCpy(sendmsg->message, szBuf, msglen);

        if( XOS_MsgSend(sendmsg) != XSUCC)
        {
            XOS_MsgMemFree(sendmsg->datasrc.FID, sendmsg);
            return XERROR;
        }
    }
    return XSUCC;
}

/************************************************************************
 函数名: XOS_PrinToMcb(  )
 功能:   带格式的输出函数,输出信息可以重定向
 输入:   pFmt 格式话字符串
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XS32 XOS_PrinToMcb( XCHAR* pFmt, ... )
{
    XCHAR   buf[MAX_BUFF_LEN] = {0};
    va_list pvar;

    if ( XNULL == pFmt )
    {
        return XERROR;
    }

    va_start( pvar, pFmt );
    XOS_VsPrintf( buf, MAX_BUFF_LEN, pFmt, pvar );
    va_end( pvar );
    buf[MAX_BUFF_LEN-1] = '\0';
    
    return XOS_PrintToMcbStr(buf);
}

#ifdef  __cplusplus
}
#endif


