/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xoscfg.c
**
**  description:  临时文件,做模块管理时,模块都通过读
                          配置文件启动, 不需要这个.c 文件
**
**  author: wangzongyou
**
**  date:   2006.07.19
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            

**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/ 
#include "xosmodule.h"
#include "cmtimer.h"
#include "xosntl.h"
#include "xosftp.h"
#include "xosmem.h"
#include "xosencap.h"
#include "xostrace.h"
#include "xoslog.h"
#include "xosmmgt.h"
#include "xosntpc.h"
#include "trace_agent.h"

#ifdef XOS_TELNETD
#include "clitelnetd.h"
#endif

#include "clishell.h"
#include "xoscfg.h"

/* 添加HA头文件: Added by liujun, 2014/12/29 */
#include "ha_resource.h"

/* t_XOSLOGINLIST * g_xosLogin;*/
#if defined(_MSC_VER) && (_MSC_VER > 1200)
    void XOS_CfgForSym(void) {}
#endif
    
/*-------------------------------------------------------------------------
                 模块内部宏定义
-------------------------------------------------------------------------*/
#ifdef XOS_MDLMGT
XS32  XOS_FIDROOT(HANDLE hDir,XS32 argc, XS8** argv);
XS32  XOS_FIDCLI(HANDLE hDir,XS32 argc, XS8** argv);
XS32  XOS_FIDNTL(HANDLE hDir,XS32 argc, XS8** argv);
#ifdef XOS_FTP_CLIENT
XS32  XOS_FIDFTP(HANDLE hDir,XS32 argc, XS8** argv);
#endif
#ifdef XOS_UDT_MODULE
XS32  XOS_FIDUDT(HANDLE hDir,XS32 argc, XS8** argv);
#endif
XS32  XOS_FIDIPC(HANDLE hDir,XS32 argc, XS8** argv);
#ifdef XOS_TELNETD
XS32  XOS_FIDTELNETD(HANDLE hDir,XS32 argc, XS8** argv);
#endif
#ifdef XOS_TELNETS
XS32  XOS_FIDTELNETS(HANDLE hDir,XS32 argc, XS8** argv);
#endif
XS32  XOS_FIDTRACE(HANDLE hDir,XS32 argc, XS8** argv);
XS32  XOS_FIDLOG(HANDLE hDir,XS32 argc, XS8** argv);
XS32  XOS_FIDFILE(HANDLE hDir,XS32 argc, XS8** argv);
XS32  XOS_FIDTIME(HANDLE hDir,XS32 argc, XS8** argv);

#ifdef XOS_TRACE_AGENT
XS32  XOS_FIDTA(HANDLE hDir,XS32 argc, XS8** argv);
#endif
#endif

/*-------------------------------------------------------------------------
                 模块内部结构和枚举定义
-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/
t_XOSFIDLIST XOS_ROOT =
{
    {
        "FID_ROOT",
        XNULL,
        FID_ROOT,
    },

    {
        XNULLP,
        XNULLP,
        XNULLP,
    },

    {
        XNULLP,
        XNULLP,
    },

    eXOSMode,
    XNULLP
};

t_XOSFIDLIST XOS_TIM =
{
    {
        "FID_TIME",
        XNULL,
        FID_TIME,
    },

    {
        TIM_InitTime,
        TIM_NoticeTime,
        XNULLP,
    },

    {
        XNULLP,
        XNULLP,
    },

    eXOSMode,
    XNULLP
};

t_XOSFIDLIST XOS_CLI =
{
    {
        "FID_CLI",
            XNULL,
            FID_CLI,
    },

    {
        XOS_CLIIniProc,
        XNULLP,
        XOS_CLICloseProc,
    },

    {
        XOS_CLIMsgProc,
        XNULLP,
    },

    eXOSMode,
    XNULLP
};

t_XOSFIDLIST XOS_NTL =
{
    {
        "FID_NTL",
            XNULL,
            FID_NTL,
    },

    {
        NTL_Init,
        XNULLP, 
        XNULL,
    },

    {
        NTL_msgProc,
        NTL_timerProc,
    },

        eXOSMode,
        XNULLP
};

#ifdef XOS_FTP_CLIENT
t_XOSFIDLIST XOS_FTP =
{
    {
        "FID_FTP",
        XNULL,
        FID_FTP,
    },

    {
        XOS_FtpInit,
        XNULLP, 
        XNULL,
    },

    {
        FTP_MsgProc,
        FTP_TimerProc,
    },

    eXOSMode,
    XNULLP
};
#endif

t_XOSFIDLIST XOS_TRACE =
{
    {
        "FID_TRACE",
        XNULL,
        FID_TRACE,
    },

    {
        Trace_Init,
            XNULLP,
            XNULL,
    },

    {
        Trace_msgProc,
        XNULLP,
    },

    eXOSMode,
    XNULLP
};

t_XOSFIDLIST XOS_FILE=
{
    {
        "FID_FILE",
        XNULL,
        FID_FILE,
    },

    {
        XNULL,
        XNULLP,
        XNULL,
    },

    {
        XNULL,
        XNULLP,
    },

    eXOSMode,
    XNULLP
};

t_XOSFIDLIST XOS_NTPC =
{
    {
        "FID_NTPC",
        XNULL,
        FID_NTPC,
    },

    {
        NTPC_Init,
        XNULLP, 
        XNULL,
    },

    {
        NTPC_msgProc,
        NTPC_timerProc,
    },

    eXOSMode,
    XNULLP
};

/*log */
t_XOSFIDLIST XOS_LOG =
{
    {
        "FID_LOG",
        XNULL,
        FID_LOG,
    },

    {
        Log_Init,
        Log_NoticeProc,
        XNULL,
    },

    {
        Log_MsgProc,
        Log_TimerProc,
    },

    eXOSMode,
    XNULLP
};


#ifdef XOS_TELNETD
t_XOSFIDLIST XOS_TelnetD =
{
    {
        "FID_TELNETD",
        XNULL,
        FID_TELNETD,
    },

    {
        TelnetD_Init,
            TelnetD_notice,
            XNULLP,
    },

    {
        TelnetD_msgProc,
        XNULLP,
    },

    eXOSMode,
    XNULLP
};
#endif

#ifdef XOS_IPC_MGNT
t_XOSFIDLIST XOS_IPCMGNT=
{
    {
        "FID_IPCMGNT",
        XNULL,
        FID_IPCMGNT,
    },

    {
        XOS_IPCInit,
            XOS_IPCNotice,
            XNULLP,
    },

    {
        XOS_IPCChanMgntFunc,
        XOS_IPCTimerCallBack,/*timer func*/
    },

    eXOSMode,
    XNULLP
};
#endif

#ifdef XOS_TRACE_AGENT
t_XOSFIDLIST XOS_TA =
{
    {
        "FID_TA",
        XNULL,
        FID_TA,
    },

    {
        TA_Init,
        XNULLP,
        XNULLP,
    },

    {
        TA_msgProc,
        TA_timerProc,
    },

    eXOSMode,
    XNULLP
};
#endif

/* 添加HA模块 BEGIN: Added by liujun, 2014/12/25 */
#ifdef XOS_LINUX
t_XOSFIDLIST XOS_HA =
{
    {
        "FID_HA",
        XNULL,
        FID_HA,
    },

    {
        HA_ResourceInit,
        XNULLP,
        HA_ResourceDestroy,
    },

    {
        HA_MsgProc,
        HA_TimerProc,
    },

    eXOSMode,
    XNULLP
};
#endif
/* END:   Added by liujun, 2014/12/25   */
/*-------------------------------------------------------------------------
模块内部函数
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
模块接口函数
-------------------------------------------------------------------------*/

/************************************************************************
函数名:
功能:
输入:
输出:
返回:
说明: 
************************************************************************/
XPUBLIC XS32 XOS_cfgXosModule(t_XOSLOGINLIST *g_xosLogin)
{
#if 0
    g_xosLogin = (t_XOSLOGINLIST *)XOS_MemMalloc(FID_ROOT, FID_XOSMAX * sizeof(t_XOSLOGINLIST));

    if(g_xosLogin == XNULLP)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), " XOS_cfgXosModule()-> malloc error!");
        return XERROR;
    }
    XOS_MemSet(g_xosLogin, 0, FID_XOSMAX * sizeof(t_XOSLOGINLIST));
#endif

    g_xosLogin[FID_TIME].stack      = &XOS_TIM;
    XOS_StrNcpy(g_xosLogin[FID_TIME].taskname , "Tsk_time", MAX_TID_NAME_LEN);
    g_xosLogin[FID_TIME].TID        = FID_TIME;
    g_xosLogin[FID_TIME].prio       = TSK_PRIO_HIGHEST;
    g_xosLogin[FID_TIME].stacksize  = 64000;
    g_xosLogin[FID_TIME].quenum = MAX_MSGS_IN_QUE;

    /* g_xosLogin[FID_TIME].timenum = 0; */

    g_xosLogin[FID_CLI].stack       = &XOS_CLI;
    XOS_StrNcpy(g_xosLogin[FID_CLI].taskname    ,"Tsk_cli", MAX_TID_NAME_LEN);
    g_xosLogin[FID_CLI].TID         = FID_CLI;
    g_xosLogin[FID_CLI].prio        = TSK_PRIO_NORMAL;
    g_xosLogin[FID_CLI].stacksize   = XNULL;
    g_xosLogin[FID_CLI].quenum = MAX_MSGS_IN_QUE;

    /* g_xosLogin[FID_CLI].timenum      = 0; */

#ifdef XOS_TELNETD
    g_xosLogin[FID_TELNETD].stack       = &XOS_TelnetD;
    XOS_StrNcpy(g_xosLogin[FID_TELNETD].taskname, "Tsk_telnetd", MAX_TID_NAME_LEN);
    g_xosLogin[FID_TELNETD].TID         = FID_TELNETD;
    g_xosLogin[FID_TELNETD].prio        = TSK_PRIO_NORMAL;  /*只能填写平台定义优先级的枚举*/
    g_xosLogin[FID_TELNETD].stacksize   = (XU16)XNULL;
    g_xosLogin[FID_TELNETD].quenum = MAX_MSGS_IN_QUE;
    /* g_xosLogin[FID_TELNETD].timenum      = 0; */
#endif
    
    g_xosLogin[FID_NTL].stack       = &XOS_NTL;
    XOS_StrNcpy(g_xosLogin[FID_NTL].taskname , "Tsk_ntl", MAX_TID_NAME_LEN);
    g_xosLogin[FID_NTL].TID         = FID_NTL;
    g_xosLogin[FID_NTL].prio        = TSK_PRIO_HIGHER;
    g_xosLogin[FID_NTL].stacksize   = XNULL;
    g_xosLogin[FID_NTL].quenum = MAX_MSGS_IN_QUE;
    /* g_xosLogin[FID_NTL].timenum      = 30; */
#ifdef XOS_FTP_CLIENT
    g_xosLogin[FID_FTP].stack       = &XOS_FTP;
    XOS_StrNcpy(g_xosLogin[FID_FTP].taskname , "Tsk_ftp", MAX_TID_NAME_LEN);
    g_xosLogin[FID_FTP].TID         = FID_FTP;
    g_xosLogin[FID_FTP].prio        = TSK_PRIO_NORMAL;
    g_xosLogin[FID_FTP].stacksize   = XNULL;
    g_xosLogin[FID_FTP].quenum = MAX_MSGS_IN_QUE;
    /* g_xosLogin[FID_NTL].timenum      = 30; */
#endif
    /* g_xosLogin[FID_IPC].timenum      = 0; */

    g_xosLogin[FID_TRACE].stack      = &XOS_TRACE;
    XOS_StrNcpy(g_xosLogin[FID_TRACE].taskname, "Tsk_trace", MAX_TID_NAME_LEN);
    g_xosLogin[FID_TRACE].TID           = FID_TRACE;
    g_xosLogin[FID_TRACE].prio      = TSK_PRIO_NORMAL;
    g_xosLogin[FID_TRACE].stacksize = XNULL;
    g_xosLogin[FID_TRACE].quenum    = MAX_MSGS_IN_QUE;

    /* g_xosLogin[FID_TRACE].timenum        = 0; */

#if 0
    g_xosLogin[FID_FILE].stack      = &XOS_FILE;
    XOS_StrNcpy(g_xosLogin[FID_FILE].taskname, "Tsk_FILE", MAX_TID_NAME_LEN);
    g_xosLogin[FID_FILE].TID            = FID_FILE;
    g_xosLogin[FID_FILE].prio       = TSK_PRIO_NORMAL;
    g_xosLogin[FID_FILE].stacksize  = XNULL;
    /* g_xosLogin[FID_TRACE].timenum        = 0; */
#endif

    g_xosLogin[FID_LOG].stack       = &XOS_LOG;
    XOS_StrNcpy(g_xosLogin[FID_LOG].taskname, "Tsk_log", MAX_TID_NAME_LEN);
    g_xosLogin[FID_LOG].TID         = FID_LOG;
    g_xosLogin[FID_LOG].prio        = TSK_PRIO_NORMAL;
    g_xosLogin[FID_LOG].stacksize   = XNULL;
    g_xosLogin[FID_LOG].quenum  = MAX_MSGS_IN_QUE;
    /* g_xosLogin[FID_TRACE].timenum        = 0; */

#ifdef XOS_IPC_MGNT
    g_xosLogin[FID_IPCMGNT].stack       = &XOS_IPCMGNT;
    XOS_StrNcpy(g_xosLogin[FID_IPCMGNT].taskname, "Tsk_ipcmgnt", MAX_TID_NAME_LEN);
    g_xosLogin[FID_IPCMGNT].TID         = FID_IPCMGNT;
    g_xosLogin[FID_IPCMGNT].prio        = TSK_PRIO_NORMAL;
    g_xosLogin[FID_IPCMGNT].stacksize   = XNULL;
    g_xosLogin[FID_IPCMGNT].quenum  = MAX_MSGS_IN_QUE;
    /* g_xosLogin[FID_IPC].timenum      = 0; */
#endif

    return XSUCC;
}


#ifdef XOS_MDLMGT
XS32  XOS_FIDROOT(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST RootLoginList;
    XOS_MemSet( &RootLoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    RootLoginList.stack     = &XOS_ROOT;
    XOS_StrNcpy(RootLoginList.taskname , "Tsk_root", MAX_TID_NAME_LEN);
    RootLoginList.TID       = FID_ROOT;
    RootLoginList.prio      = TSK_PRIO_LOWEST;    
    RootLoginList.quenum    = MIN_MSGS_IN_QUE;

    return XOS_MMStartFid(&RootLoginList, XNULLP, XNULLP);
}


XS32  XOS_FIDCLI(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST CLILoginList;
    XS32 ret = XSUCC;

    XOS_MemSet( &CLILoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    CLILoginList.stack     = &XOS_CLI;
    XOS_StrNcpy(CLILoginList.taskname , "Tsk_cli", MAX_TID_NAME_LEN);
    CLILoginList.TID       = FID_CLI;
    CLILoginList.prio      = TSK_PRIO_NORMAL;    
    CLILoginList.quenum    = MAX_MSGS_IN_QUE;

    ret = XOS_MMStartFid(&CLILoginList, XNULLP, XNULLP);

    return ret;
}


XS32  XOS_FIDNTL(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST NTLLoginList;
    XS32 ret = XSUCC;

    XOS_MemSet( &NTLLoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    NTLLoginList.stack     = &XOS_NTL;
    XOS_StrNcpy(NTLLoginList.taskname , "Tsk_ntl", MAX_TID_NAME_LEN);
    NTLLoginList.TID        = FID_NTL;
    NTLLoginList.prio      = TSK_PRIO_HIGHER;    
    NTLLoginList.quenum = MAX_MSGS_IN_QUE;

    ret = XOS_MMStartFid(&NTLLoginList,XNULLP, XNULLP);

    return ret;
}


#ifdef XOS_FTP_CLIENT
extern int gFtpMsgDealThread;
XS32  XOS_FIDFTP(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST FTPLoginList;
    XS32 ret = XSUCC;
    /*增加ftp任务数定制*/
    if(argc >= 1)
    {
        gFtpMsgDealThread = atoi(argv[0]);        
    }
    else
    {
        gFtpMsgDealThread = 1;
    }

    XOS_MemSet( &FTPLoginList, 0x00, sizeof(t_XOSLOGINLIST) );
    FTPLoginList.stack     = &XOS_FTP;
    XOS_StrNcpy(FTPLoginList.taskname , "Tsk_ftp", MAX_TID_NAME_LEN);
    FTPLoginList.TID        = FID_FTP;
    FTPLoginList.prio      = TSK_PRIO_NORMAL;
    FTPLoginList.quenum = MAX_MSGS_IN_QUE;
    ret = XOS_MMStartFid(&FTPLoginList,XNULLP, XNULLP);

    
    return ret;
}
#endif

XS32  XOS_FIDTELNETD(HANDLE hDir,XS32 argc, XS8** argv)
{
    XS32 ret = XSUCC;
#ifdef XOS_TELNETD
    t_XOSLOGINLIST TELNETDLoginList;

    XOS_MemSet( &TELNETDLoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    TELNETDLoginList.stack     = &XOS_TelnetD;
    XOS_StrNcpy(TELNETDLoginList.taskname , "Tsk_telnetd", MAX_TID_NAME_LEN);
    TELNETDLoginList.TID        = FID_TELNETD;
    TELNETDLoginList.prio      = TSK_PRIO_NORMAL;    
    TELNETDLoginList.quenum = MAX_MSGS_IN_QUE;
    TELNETDLoginList.stacksize = 204800;
    
    ret = XOS_MMStartFid(&TELNETDLoginList,XNULLP, XNULLP);
#endif
    return ret;
}


XS32  XOS_FIDTRACE(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST TRACELoginList;
    XS32 ret = XSUCC;
    
    XOS_MemSet( &TRACELoginList, 0x00, sizeof(t_XOSLOGINLIST) );
    
    TRACELoginList.stack     = &XOS_TRACE;
    XOS_StrNcpy(TRACELoginList.taskname , "Tsk_trace", MAX_TID_NAME_LEN);
    TRACELoginList.TID      = FID_TRACE;
    TRACELoginList.prio      = TSK_PRIO_NORMAL;    
    TRACELoginList.quenum = MAX_MSGS_IN_QUE;
    TRACELoginList.stacksize = 204800;

    ret = XOS_MMStartFid(&TRACELoginList,XNULLP, XNULLP);

    return ret;
}


XS32  XOS_FIDLOG(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST LOGLoginList;
    XS32 ret = XSUCC;

    XOS_MemSet( &LOGLoginList, 0x00, sizeof(t_XOSLOGINLIST) );
    LOGLoginList.stack     = &XOS_LOG;
    XOS_StrNcpy(LOGLoginList.taskname , "Tsk_log", MAX_TID_NAME_LEN);
    LOGLoginList.TID        = FID_LOG;
    LOGLoginList.prio      = TSK_PRIO_NORMAL;    
    LOGLoginList.quenum = MAX_MSGS_IN_QUE;

    ret = XOS_MMStartFid(&LOGLoginList,XNULLP, XNULLP);

    return ret;
}


XS32  XOS_FIDFILE(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST FILELoginList;
    XS32 ret = XSUCC;

    XOS_MemSet( &FILELoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    FILELoginList.stack     = &XOS_FILE;
    XOS_StrNcpy(FILELoginList.taskname , "Tsk_file", MAX_TID_NAME_LEN);
    FILELoginList.TID       = FID_FILE;
    FILELoginList.prio      = TSK_PRIO_NORMAL;    
    FILELoginList.quenum = MAX_MSGS_IN_QUE;

    ret = XOS_MMStartFid(&FILELoginList,XNULLP, XNULLP);

    return ret;
}


XS32  XOS_FIDTIME(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST TIMELoginList;
    XS32 ret = XSUCC;
    
    XOS_MemSet( &TIMELoginList, 0x00, sizeof(t_XOSLOGINLIST) );
    TIMELoginList.stack     = &XOS_TIM;
    XOS_StrNcpy(TIMELoginList.taskname , "Tsk_time", MAX_TID_NAME_LEN);
    TIMELoginList.TID       = FID_TIME;
    TIMELoginList.prio      = TSK_PRIO_HIGHEST;    
    TIMELoginList.quenum = MAX_MSGSNUM_IN_QUE;
    TIMELoginList.stacksize = 131072;
    
    ret = XOS_MMStartFid(&TIMELoginList,XNULLP, XNULLP);

    return ret;
}

XS32  XOS_FIDTA(HANDLE hDir,XS32 argc, XS8** argv)
{
    XS32 ret = XSUCC;
#ifdef XOS_TRACE_AGENT
    t_XOSLOGINLIST TALoginList;

    XOS_MemSet( &TALoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    TALoginList.stack     = &XOS_TA;
    XOS_StrNcpy(TALoginList.taskname , "Tsk_ta", MAX_TID_NAME_LEN);
    TALoginList.TID        = FID_TA;
    TALoginList.prio      = TSK_PRIO_NORMAL;    
    TALoginList.quenum = MAX_MSGS_IN_QUE;
    TALoginList.stacksize = 204800;
    
    ret = XOS_MMStartFid(&TALoginList,XNULLP, XNULLP);
#endif
    return ret;
}

#endif /*XOS_MDLMGT*/


#ifdef XOS_ST_TEST
/************************************************************************
函数名:    XOS_exeCliCmd
功能: 直接执行的函数
输入:
输出:
返回:
说明:  用户自己保证字符串尾部有个结束符
************************************************************************/
XVOID XOS_exeCliCmd(XCHAR* pStr)
{
    XCHAR *pChar;
    t_XOSCOMMHEAD *pLocalConsoleMsg;

    if(pStr == XNULLP)
    {
        return;
    }

    pLocalConsoleMsg = XOS_MsgMemMalloc( FID_CLI, 2 );
    if ( XNULLP == pLocalConsoleMsg )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "XNULLP == pLocalConsoleMsg!");
        return;
    }

    pChar = pStr;
    do
    {
        pLocalConsoleMsg = XOS_MsgMemMalloc( FID_CLI, 2 );
        if ( XNULLP == pLocalConsoleMsg )
        {
            XOS_Trace(MD(FID_CLI, PL_ERR), "XNULLP == pLocalConsoleMsg!");
            return;
        }
        
        XOS_MemSet( pLocalConsoleMsg, 0x00, 2 );
        
        pLocalConsoleMsg->datasrc.FID    = FID_CLI;
        pLocalConsoleMsg->datasrc.FsmId  = 0;
        pLocalConsoleMsg->datasrc.PID    = 0;
        pLocalConsoleMsg->datadest.FID   = FID_CLI;
        pLocalConsoleMsg->datadest.FsmId = 0;
        pLocalConsoleMsg->datadest.PID   = 0;
        pLocalConsoleMsg->msgID          = MSG_IP_DATAIND;
        pLocalConsoleMsg->subID          = 0;
        
        pLocalConsoleMsg->prio   = eAdnMsgPrio;
        sprintf( (XCHAR*)pLocalConsoleMsg->message, "%c", (*(pChar+1)=='\0')?'\r':*pChar);
        
        //       XOS_MsgSend(pLocalConsoleMsg);
        XOS_CLIMsgProc( (XVOID*)pLocalConsoleMsg, XNULL );
        XOS_MsgMemFree( FID_CLI, pLocalConsoleMsg );
        pChar++; 
    }
    while(*pChar != '\0');
}
#endif /*XOS_ST_TEST*/


#ifdef __cplusplus
}
#endif /* __cplusplus */


