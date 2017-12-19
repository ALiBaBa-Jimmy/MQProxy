/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: clishell.c
**
**  description:  命令行模块用户输入解析:
                   该模块为解释用户在命令行上
                   所输入, 调用相应模块的处理函数
**
**  author: zl
**
**  date:   2006.3.7
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   zl         2006.3.7              create
**************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include <time.h>
#include <errno.h>
#include <stdarg.h>

#include "cliconfig.h"
#include "clishell.h"
#include "clitelnet.h"
#include "clicmds.h"
#include "xoscfg.h"
#include "xosmodule.h"
#include "xosencap.h"
#include "xosmem.h"
#include "xosxml.h"
#include "xosos.h"
#include "xosfilesys.h"
#include "clitelnetd.h"
#include "xosenc.h"
#include "xosnetinf.h"
//#if ( XOS_LINUX  )
#ifdef XOS_LINUX

#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>
#endif

#ifdef XOS_WIN32
#include <Windows.h>
#endif

/*-------------------------------------------------------------------------
                 模块内部宏定义
-------------------------------------------------------------------------*/


/*
*外部变量定义
*/

/*-------------------------------------------------------------------------
                 模块内部结构和枚举定义
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/
/*
#ifdef XSTATIC
#undef XSTATIC
#define XSTATIC
#endif
*/
#ifdef XOS_VXWORKS
/*定义全局变量，控制vxworks下的终端信息输出默认打开，可以在命令行设置*/
XSTATIC XBOOL g_OutputForVxworks = XTRUE;
#endif

XSTATIC XCHAR ne_Version[128]={"NULL"};
XSTATIC CLI_ENV                **gppCliSessions = XNULL;
XSTATIC t_XOS_COM_CHAN         gConsoleObject;
XSTATIC t_XOS_CLI_AP_CMDS_MODE g_paCliAppMode[CLI_MAX_CMD_CLASS];
XSTATIC t_XOS_CLI_USER_INFOR   g_paUserInfor[CLI_MAX_USER_NUM] =
{
    { eUserSuper,  "sys",    "sys",   -1 },
    { eUserAdmin,  "admin",  "admin", -1 },
    { eUserNormal, "user",   "user",  -1 },
    { eUserNormal, "debug",  "debug", -1 }
};
t_XOS_PIRNT_BORAD_SWITCH g_paUserSIDLogined[kCLI_MAX_CLI_TASK+1] =
{
    { 0, 0 }
};
XSTATIC t_XOS_CLI_VER g_paCliVer[CLI_REGVER_MAX];

//#if ( XOS_LINUX )
#ifdef XOS_LINUX
XSTATIC struct termios g_save_term;
#endif

XSTATIC t_XOSMUTEXID g_mutexCliPrt;
XSTATIC t_XOSMUTEXID  g_cliMutex;
XSTATIC XBOOL g_bDubugFlag = XFALSE;
XSTATIC XBOOL bClI_InitFlag  = XFALSE;

XEXTERN XU32 log_msg_discard;
XEXTERN XU32 telnet_msg_discard;

//gcg+ 
//控制是否将打印信息输出到主控板
XU32     g_ulPrinToMcbFlg    = 0; 
XU32     gMcbPid = 0; //主控板PID
XU32     gLocalIP = 0; //业务板IP
//!

#if defined (XOS_LINUX) || defined(XOS_SOLARIS)
XSTATIC XS32  g_s32Daemon_proc  = XFALSE;
#endif

#ifdef XOS_EW_START
XSTATIC XCHAR g_aszSysFPath[XOS_MAX_PATHLEN+1] ={0};    /*完整路径*/
XSTATIC XCHAR g_aszSysFName[XOS_MAX_PATHLEN+1] ={0};    /*可执行文件名(全路径)*/
XSTATIC XCHAR g_aszXmlFName[XOS_MAX_PATHLEN+1] ={0};    /*xml文件名(全路径)*/
XSTATIC XBOOL g_bIsFirstGeted = XFALSE;/*记录是否已经完成第一次的全路径操作*/
#endif

#ifdef XOS_EW_START
#ifdef XOS_VTA
XSTATIC XCHAR ObjProjectName[XOS_MAX_PATHLEN+1] = "ss.out";
#endif
#endif


static CLI_FUNC_REG_T gcliRegFunc;


char gszPrefixInfo[CLI_PRE_FIX_LEN];
XS32 gRemoteShowCli = 0;

#ifdef XOS_MDLMGT
/*保留main函数的入参，供业务调用*/
XU32 g_main_argc;   /*argc，入参的数量*/
XS8  **g_main_argv;   /*argv,入参列表*/
#endif
//
XVOID cli_RemoteExecCli(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
XS32 cli_SendCliRspMsg(CLI_ENV *pEnv, XCHAR *pBuf, XS32 BufLen);
XS32 cli_getRemoteCursor(const XCHAR *szLocalCursor,XCHAR *szRemoteCursor);
XS32 Cli_SendMsgByBuff(XVOID *buff,XU32 msg_len,XU32 msgId,XU32 srcFid,XU32 dstFid,XU32 dstFsmId,XU32 dstPid);
int cli_getRemotePidLst(int strLen, char* pcliPath, XU32* pidNum, XU32* pidList);
char * Cli_SetCliPrefixInfo(char *szPrefixInfo);


extern XS32 Trace_regCliCmd(int cmdMode);

/*************************************************************\

   modified by lixn 2007.6.12

   for mem check

\**************************************************************/
#ifdef MEM_ID_DEBUG

XEXTERN xosMemCheck();

#endif

/*-------------------------------------------------------------------------
                模块内部函数
-------------------------------------------------------------------------*/
XSTATIC XVOID cli_sendTelnetCloseReq( XS32 connId );
XSTATIC XS32  cli_telnetConsoleRead( CLI_ENV *pEnv, XU8 charIn1, XCHAR *pBuf, XS32 *bytesRead );
XSTATIC XS32  cli_telnetConsoleWrite( CLI_ENV *pEnv, XCHAR *pBuf, XS32 BufSize );
XSTATIC XBOOL cli_initCmdTree( XVOID );
XSTATIC XBOOL cli_initCliMem( XVOID );
XSTATIC XVOID cli_releaseCliMem( XVOID );
XSTATIC XBOOL cli_initCliApCmd( XVOID );
XSTATIC XVOID cli_initPrompt(CLI_ENV* pCliEnv);
XSTATIC XS32  cli_excuteCommand(CLI_ENV *pCliEnv);
XSTATIC XVOID cli_cmdExit(CLI_ENV* pCliEnv, XS32 siArgc , XCHAR **ppArgv);
XSTATIC XVOID cli_cmdQuit(CLI_ENV* pCliEnv, XS32 siArgc,XCHAR **ppArgv);
XSTATIC XVOID cli_cmdHelp(CLI_ENV* pCliEnv, XS32 siArgc,XCHAR **ppArgv);
XSTATIC XVOID cli_cmdCls(CLI_ENV* pCliEnv, XS32 siArgc,XCHAR **ppArgv);
XSTATIC XVOID cli_PromptSwitch(CLI_ENV* pCliEnv, XU32 siDstMode);
XSTATIC XS32  cli_parseCommand( CLI_ENV *pCliEnv );
XSTATIC XVOID cli_cmdSwitchPrompt(CLI_ENV* pCliEnv, XS32 siArgc , XCHAR **ppArgv);
XSTATIC XS16  cli_extGetCursorX( t_XOS_CMD_EDITINFO *pEdit );
XSTATIC XS16  cli_extGetCursorY( t_XOS_CMD_EDITINFO *pEdit );
XSTATIC XVOID cli_extSetCursorX(t_XOS_CMD_EDITINFO *pEdit, XS16 pos);
XSTATIC XVOID cli_extSetCursorY(t_XOS_CMD_EDITINFO *pEdit, XS16 pos);
XSTATIC XVOID cli_extCurrCoord(CLI_ENV *pCliEnv, XS16 *xPos, XS16 *yPos);
XSTATIC XVOID cli_extResetOutput(CLI_ENV *pCliEnv);
XSTATIC XVOID cli_extSetCursor(CLI_ENV *pCliEnv, XS16 position);
XSTATIC XVOID cli_listMoveCursor(CLI_ENV *pCliEnv, XS16 offset);
XSTATIC XVOID cli_extInitCommandLine(CLI_ENV *pCliEnv);
XSTATIC XS32  cli_histInitHistInfo(CLI_ENV *pCliEnv);
XSTATIC XS32  cli_histResetHistInfo(CLI_ENV *pCliEnv);
XSTATIC XS32  cli_dbInitEnvironment(CLI_ENV **ppCliEnv, t_XOS_COM_CHAN *pChannel);
XSTATIC XVOID cli_histAddHistLine (CLI_ENV *pCliEnv);
XSTATIC XVOID cli_taskPrintError(CLI_ENV *pCliEnv, XS32 status);
#ifdef XOS_WIN32
XSTATIC XCHAR cli_taskValidateLogin( XCHAR *pLogin, XCHAR *pPassword, Access *pAccLvl );
#endif
XSTATIC XS32  cli_extReadCh(CLI_ENV *pCliEnv, XU8* pOrgCh, XU16 orgLen);
XSTATIC XS32  cli_extPutStr(CLI_ENV *pCliEnv, const XCHAR *pBuf);
XSTATIC XS32  cli_extWrite(CLI_ENV *pCliEnv, const XCHAR *pBuf, XS32 bufLen);
#ifdef XOS_WIN32
XSTATIC XVOID cli_cmdDosClear(CLI_ENV *pCliEnv);
#endif
XSTATIC XS32  cli_cmdClear(CLI_ENV *pCliEnv);
XSTATIC XS32  cli_extPut(CLI_ENV *pCliEnv, const XCHAR *pBuf, XS32 bufLen);
XSTATIC XS32  cli_taskLogin(CLI_ENV *pCliEnv);
XSTATIC XS32  cli_cacheConstruct(t_XOS_CACHE_HANDLE **pph_cacHandle, Access AccessType);
XSTATIC XU32  cli_getPromptLayer( XS8 * pszPrompt );
XSTATIC XVOID cli_extInsertText(CLI_ENV *pCliEnv, XCHAR *pText, XS16 length);
XSTATIC XVOID cli_strToUp(XCHAR * s);
XSTATIC XS32  cli_environmentConstruct(t_XOS_GEN_ENVIRON **pp_envInit);
XSTATIC XS32  cli_environmentDestruct (t_XOS_GEN_ENVIRON **pp_envTemp);
#ifdef XOS_WIN32
XSTATIC XS32  cli_pciConsoleRead( CLI_ENV *pEnv, XU8 charIn1, XCHAR *pBuf, XS32 *bytesRead );
#endif
XSTATIC XS32  cli_pciConsoleWrite( CLI_ENV *pEnv, const XCHAR *pBuf, XS32 s32StrLen );
XSTATIC XBOOL cli_CheckBlanks( XS8 * pszPrompt, XS32 s32Len );
XSTATIC XS32  cli_insert_usid_logined( XS32 s32sid );
XSTATIC XBOOL cli_delete_usid_logined( XS32 s32sid );
XSTATIC XBOOL cli_open_direct_switch( XS32 s32sid, XBOOL bSwitchFlag  );
XSTATIC XS32  cli_InitSessionByIndex( XS32 s32Index, t_XOS_COM_CHAN *pChannel );

XSTATIC t_XOS_CLI_LIST*  cli_listConstruct(XVOID);
XSTATIC t_XOS_CMD_DATAS* cli_NewCmds( XVOID  );
XSTATIC CLI_ENV*         cli_telnetGetCliSession( XS32 index );
XSTATIC t_XOS_COM_CHAN*  cli_telnetConsoleChannel();

XS8 XOS_TelnetSInitProc( );

#ifdef XOS_VXWORKS
XSTATIC XVOID setOutputForVxworks(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
#endif

/*-------------------------------------------------------------------------
                模块接口函数
-------------------------------------------------------------------------*/
/************************************************************************
函数名: XOS_SystemExit(  )
功能:   
输入:
输出:   
返回:     
说明:
************************************************************************/
XVOID  XOS_SystemExit( XS32 s32ExitCode )
{
    exit( s32ExitCode );
}

/*## 获取可执行文件和xml文件的完全路径*/
#ifdef XOS_EW_START

XS32 CLI_GetFullPathInfo( XVOID)
{
    //#if ( XOS_WIN32 )
#ifdef XOS_WIN32
    XCHAR szDir[XOS_MAX_PATHLEN+1] ={0};
    XCHAR szTemp[XOS_MAX_PATHLEN+1] ={0};
    
    if( XOS_StrLen( g_aszSysFPath) != 0 )
    {
        return XSUCC;
    }
    
    if(XFALSE != g_bIsFirstGeted )
    {
        return XSUCC;
    }
    
    if(0 == (GetModuleFileName(XNULL, szTemp, XOS_MAX_PATHLEN)))
    {
        /*若获取完全路径失败，就在当前目录下用LOG记录*/
#ifdef CLI_LOG
        cli_log_write( eCLIError, "GetModuleFileName,get moudule fullpath filename failed!\n"  );
#endif
        return XERROR;
    }
    
    XOS_StrNcpy(g_aszSysFName,szTemp,XOS_MAX_PATHLEN);   /*获得可执行文件名(全路径)*/
    /*将全路径名拆分*/
    if(XOS_StrLen(szTemp)>0)
    //if(szTemp)
    {
        //_splitpath(lpszPathName,drive,dir,fname,ext)
        //lpszPathName not support file like a.out.data
        _splitpath(szTemp,g_aszSysFPath,szDir,XNULL,XNULL); /**/
        /*获得完整路径*/
        XOS_StrCat(g_aszSysFPath,szDir);
        XOS_StrNcpy(g_aszXmlFName,g_aszSysFPath, XOS_MAX_PATHLEN);
        XOS_StrCat(g_aszXmlFName,"xos.xml");  /*获得xml文件名(全路径)*/
    }
    else
    {
#ifdef CLI_LOG
        cli_log_write( eCLIError, "Get the SysFPath Failed!\n"  );
#endif
        return XERROR;
    }
    g_bIsFirstGeted = XTRUE ;
    
#endif
    
    //#if ( XOS_LINUX || XOS_SOLARIS )
#if defined (XOS_LINUX) || defined(XOS_SOLARIS)
    XU32 i=0;
    XCHAR *szEnv = "";
    struct stat statbuf;
    
    if( XFALSE != g_bIsFirstGeted )
    {
        return XSUCC;
    }
    
    if( XOS_StrLen( g_aszSysFPath) != 0 && stat( g_aszSysFPath, &statbuf ) != -1
        && S_ISREG( statbuf.st_mode ))
    {
        return XSUCC;
    }
    else
    {
        szEnv = getenv(XOS_PATH_ENV);   /*获得可执行文件名(全路径)*/
        if ( XNULL == szEnv )
        {
#ifdef CLI_LOG
            cli_log_write( eCLIError, "get environment variable XOS_PATH_ENV failed,the module filename do not exist!\n"  );
#endif
            return XERROR;
        }
        
        if ( stat(szEnv,&statbuf) == -1 )
        {
            /*### 环境变量中设置的路径错误*/
#ifdef CLI_LOG
            cli_log_write( eCLIError, "check get environment variable XOS_PATH_ENV return value for module filename failed,file do not exist!\n"  );
#endif
            return XERROR;
        }
        if ( !S_ISREG( statbuf.st_mode ) )
        {
#ifdef CLI_LOG
            cli_log_write( eCLIError, "check get environment variable XOS_PATH_ENV return value for module filename failed,file do not exist!\n"  );
#endif
            return XERROR;
        }
        
        XOS_StrNcpy( g_aszSysFName, szEnv, XOS_MAX_PATHLEN);
        if ( stat( g_aszSysFName, &statbuf ) == -1 )
        {
#ifdef CLI_LOG
            cli_log_write( eCLIError, "environment variable XOS_PATH_ENV settting file %s do not exist!\n",g_aszSysFName);
#endif
            return XERROR;
        }
        if ( !S_ISREG( statbuf.st_mode ) )
        {
#ifdef CLI_LOG
            cli_log_write( eCLIError, "environment variable XOS_PATH_ENV settting file %s do not exist!\n",g_aszSysFName);
#endif
            return XERROR;
        }
        
        /*获得完整路径*/
        for( i = 0; szEnv[i] !='\0' && i < XOS_MAX_PATHLEN ; i++ )
        {
            if( szEnv[i] == '/' )
            {
                XOS_StrNcpy( g_aszSysFPath, szEnv, i+1 );
                XOS_StrNCat( g_aszSysFPath, "",1);
            }
        }
        
        XOS_StrNcpy(g_aszXmlFName, g_aszSysFPath, XOS_MAX_PATHLEN);
        XOS_StrCat(g_aszXmlFName,  "xos.xml");  /*获得xml文件名(全路径)*/
        
        /*有效性检查*/
        if ( stat( g_aszXmlFName, &statbuf ) == -1 )
        {
#ifdef CLI_LOG
            cli_log_write( eCLIError, "XmlFName file %s do not exist!\n",g_aszXmlFName);
#endif
            return XERROR;
        }
        if ( !S_ISREG( statbuf.st_mode ) )
        {
#ifdef CLI_LOG
            cli_log_write( eCLIError, "XmlFName file %s do not exist!\n",g_aszXmlFName);
#endif
            return XERROR;
        }
    }
    
    g_bIsFirstGeted = XTRUE ;
#endif
    
#if defined (XOS_SOLARIS) || defined (XOS_LINUX) || defined (XOS_WIN32)
    /*### 切换当前工作路径到指定目录info*/
    if ( 0 != chdir(g_aszSysFPath) )
    {
        printf("chdir error\r\n");
#ifdef CLI_LOG
        cli_log_write( eCLIError, "Change dir to %s failed!\n" , g_aszSysFPath );
#endif
        return XERROR;
    }
#endif
#ifdef XOS_VTA
    if ( XFALSE != g_bIsFirstGeted )
    {
        return XSUCC;
    }
    XOS_StrNcpy(g_aszSysFPath,"./", XOS_MAX_PATHLEN);
    XOS_StrNcpy(g_aszXmlFName, "xos.xml", XOS_MAX_PATHLEN);
    XOS_StrNcpy( g_aszSysFName, ObjProjectName, XOS_MAX_PATHLEN);
    
    g_bIsFirstGeted = XTRUE ;
#endif
    /*log记录当前完整路径*/
#ifdef CLI_LOG
    cli_log_write( eCLINormal, "The system path is \"%s\"!\n" , g_aszSysFPath );
#endif
    return XSUCC;
    
}

/*获取完整路径*/
XS32 XOS_GetSysPath( XCHAR *szSysPath, XS32 nPathLen )
{
    if( XSUCC != CLI_GetFullPathInfo())
    {
#ifdef CLI_LOG
        cli_log_write( eCLIError, "Get SysFPath error, exit(0)!\n"  );
#endif
        XOS_SystemExit( XOS_EXIT_SUCCESS );
    }
    
    if ( XNULL == szSysPath || XOS_MAX_PATHLEN > nPathLen )
    {
#ifdef CLI_LOG
        cli_log_write( eCLIError, " XOS_GetSysPath( szSysPath, nPathLen) Para is wrong!\n" );
#endif
        return XERROR;
    }
    
    XOS_StrNcpy( szSysPath, g_aszSysFPath, XOS_MAX_PATHLEN );
    return XSUCC;
}

/*获取可执行文件名（带全路径的）*/
XCHAR* XOS_CliGetSysName( XVOID )
{
    if( XSUCC != CLI_GetFullPathInfo())
    {
#ifdef CLI_LOG
        cli_log_write( eCLIError, "Get SysFPath error, exit(0).\n"  );
#endif
        XOS_SystemExit( XOS_EXIT_SUCCESS );
        return NULL;
    }
    return g_aszSysFName;
}
#endif

/*获取xml 文件名（带全路径的）*/
XCHAR*  XOS_CliGetXmlName( XVOID)
{
#ifndef XOS_EW_START
    return "xos.xml";
#else
    if( XSUCC != CLI_GetFullPathInfo())
    {
#ifdef CLI_LOG
        cli_log_write( eCLIError, "Get SysFPath error, exit(0).\n"  );
#endif
        XOS_SystemExit( XOS_EXIT_SUCCESS );
        return NULL;
    }
    return g_aszXmlFName;
#endif
}

/************************************************************************
 函数名: XOS_DaemonInit()
 功能:  平台以守护进程方式启动
 输入:  启动命令: "xossol -D"
 输出:  N/A
 返回:  0/-1
 说明:  目前在 Linux 和 Solaris 使用该方式启动平台, Windows(使用 Service) 没有实现该功能
************************************************************************/
XS32 XOS_DaemonInit( XS32 argc, XCHAR* argv[] )
{
#if (defined (XOS_LINUX) || defined(XOS_SOLARIS))
    
    XS32 nret = 0;
    pid_t pid = -1;
    
    /*## 进程启动方式*/
    if ( argc == 2 && !XOS_StrCmp( argv[1], "-d" )  )
    {
        g_s32Daemon_proc = 1;
        return 0;
    }
    
    if ( !(argc == 2 && !XOS_StrCmp( argv[1], "-D" )) )
    {
        return 0;
    }
#ifdef CLI_LOG
    
    cli_log_write( eCLINormal, "XOS_DaemonInit." );
    
#endif
    
    pid = fork();
    if ( pid < 0 )
    {
#ifdef CLI_LOG
        
        cli_log_write( eCLIError, "fork()1 failed." );
#endif
        return XERROR;
    }
    else if( pid != 0 )
    {
        /* paraent terminates */
        exit(0);
    }
    
    nret = setsid();
    if ( nret < 0 )
    {
#ifdef CLI_LOG
        
        cli_log_write( eCLIError, "setsid() failed." );
#endif
        return XERROR;
    }
    
    pid = fork();
    if ( pid < 0 )
    {
#ifdef CLI_LOG
        
        cli_log_write( eCLIError, "fork()2 failed." );
#endif
        return XERROR;
    }
    else if( pid != 0 )
    {
        /* paraent terminates */
        exit(0);
    }
    
    /*chdir( "/" );*/
    
    umask( 0 );
    
    g_s32Daemon_proc = XTRUE;
    
#ifdef CLI_LOG
    
    cli_log_write( eCLINormal, "XOS_DaemonInit() daemoned succefully." );
#endif
    
#endif /*XOS_SOLARIS*/
    
    return XSUCC;    
}

XPUBLIC XVOID CLI_dealInput(XCHAR cInputChar)
{
    t_XOSCOMMHEAD *pLocalConsoleMsg = (t_XOSCOMMHEAD *)XNULLP;

    pLocalConsoleMsg = XOS_MsgMemMalloc( FID_CLI, SEND_STRING_LEN );
    if ( XNULLP == pLocalConsoleMsg )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "XNULLP == pLocalConsoleMsg!");
        return;
    }
    
    XOS_MemSet( pLocalConsoleMsg, 0x00, SEND_STRING_LEN );
    
    pLocalConsoleMsg->datasrc.FID    = FID_CLI;
    pLocalConsoleMsg->datasrc.FsmId  = 0;
    pLocalConsoleMsg->datasrc.PID    = XOS_GetLocalPID();
    pLocalConsoleMsg->datadest.FID   = FID_CLI;
    pLocalConsoleMsg->datadest.FsmId = 0;
    pLocalConsoleMsg->datadest.PID   = XOS_GetLocalPID();
    pLocalConsoleMsg->msgID          = MSG_IP_DATAIND;
    pLocalConsoleMsg->subID          = 0;
    
    pLocalConsoleMsg->prio   = eAdnMsgPrio;
    sprintf( (XCHAR*)pLocalConsoleMsg->message, "%c", cInputChar );
    
    XOS_CLIMsgProc( (XVOID*)pLocalConsoleMsg, XNULL );
    XOS_MsgMemFree( FID_CLI, pLocalConsoleMsg );
}
/************************************************************************
 函数名: CLI_locConsoleEntry()
 功能:  主输入处理函数，主要处理超级终端的命令行输入
 输入:  本地控制台输入
 输出:  N/A
 返回:  N/A
 说明:  只支持linux,solaris,windows
************************************************************************/
XPUBLIC XVOID CLI_locConsoleEntry(XVOID)
{
#ifndef XOS_VXWORKS
    XCHAR cInputChar                = 0;
    t_XOSCOMMHEAD *pLocalConsoleMsg = XNULLP;
    //#if ( XOS_LINUX )
#ifdef XOS_LINUX
    struct termios term;
#endif
    
#if (defined (XOS_LINUX) || defined(XOS_SOLARIS))
   
    /* 平台以守护进程方式运行, 主线程休眠 */
    if ( XTRUE == g_s32Daemon_proc )
    {
#ifdef CLI_LOG
        cli_log_write( eCLINormal, "xos main thread run." );
#endif
        while(1)
        {
            pause();
        }
    }
#endif /*XOS_SOLARIS*/
    
    /* 平台以 Term 进程方式运行, 主线程提取标准键盘I/O */
    pLocalConsoleMsg = XOS_MsgMemMalloc( FID_CLI, SEND_STRING_LEN );
    if ( XNULLP == pLocalConsoleMsg )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "ERROR: XNULLP == pLocalConsoleMsg !" );
#ifdef CLI_LOG
        cli_log_write( eCLINormal, "xos main thread exit." );
#endif
        return;
    }
    XOS_MemSet( pLocalConsoleMsg, 0x00, SEND_STRING_LEN );
    
    pLocalConsoleMsg->datasrc.FID    = FID_CLI;
    pLocalConsoleMsg->datasrc.FsmId  = 0;
    pLocalConsoleMsg->datasrc.PID    = XOS_GetLocalPID();
    pLocalConsoleMsg->datadest.FID   = FID_CLI;
    pLocalConsoleMsg->datadest.FsmId = 0;
    pLocalConsoleMsg->datadest.PID   = XOS_GetLocalPID();
    pLocalConsoleMsg->msgID          = MSG_IP_DATAIND;
    pLocalConsoleMsg->subID          = 0;
    pLocalConsoleMsg->prio           = eAdnMsgPrio;
    
    sprintf( (XCHAR*)pLocalConsoleMsg->message, "%c", '\r' );
    
    if ( XSUCC != XOS_CLIMsgProc( (XVOID*)pLocalConsoleMsg, XNULL ) )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: get msg header pointer failed!");
    }
    XOS_MsgMemFree( FID_CLI, pLocalConsoleMsg );
    pLocalConsoleMsg = XNULL;
    
    /*处理 term 版本*/
    //#if ( XOS_LINUX )
#ifdef XOS_LINUX
    if ( tcgetattr( STDIN_FILENO, &g_save_term ) < 0 )
    {
        return ;
    }
    
    term = g_save_term;
    
    term.c_lflag &= ~( ECHO);
    term.c_lflag &= ~(ICANON);
    tcsetattr( STDIN_FILENO, TCSANOW, &term );
#endif
    
    while( XTRUE )
    {
        cInputChar = (XCHAR)XOS_GetChar();
        
        CLI_dealInput(cInputChar);
    }
    
#endif
}

/************************************************************************
 函数名: CLI_IniProc(  )
 功能:   初始化消息分发
 输入:
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR,或者错误
 说明:
************************************************************************/
XS8 XOS_CLIIniProc( XVOID *t, XVOID *v )
{
    XS32               siIdx        = 0;
    t_XOS_COM_CHAN     *pTheConsole = XNULL;
    XS32               u32CliSessionNum = 0;
#ifdef XOS_VXWORKS
    XS32 ret = 0;
    XS32 backvalue = 0;
#endif
    t = t;
    v = v;
    
    if ( XSUCC != XOS_MutexCreate( &g_cliMutex ) )
    {
        XOS_Trace(MD(FID_CLI,PL_ERR),"XOS_MutexCreate g_cliMutex failed!");
        return XERROR;
    }
    if ( XSUCC != XOS_MutexCreate( &g_mutexCliPrt) )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "XOS_MutexCreate g_mutexCliPrt failed!");
        return XERROR;
    }
    
    pTheConsole      = cli_telnetConsoleChannel();
    u32CliSessionNum = kCLI_MAX_CLI_TASK + 1;
    
    /*申请内存,并填充命令树 */
    if( !cli_initCmdTree() )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: XOS_CLIIniProc(), init command tree failed!");
        return XERROR;
    }
    
    gppCliSessions = (CLI_ENV**)XOS_MemMalloc( FID_CLI, sizeof(gppCliSessions)*u32CliSessionNum );
    if ( gppCliSessions == XNULL )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "get command line session failed!");
        return XERROR;
    }
    XOS_MemSet( gppCliSessions, 0x00, sizeof(gppCliSessions) * u32CliSessionNum );
    
    for ( siIdx = 0; siIdx < u32CliSessionNum; siIdx++ )
    {
        if ( XSUCC != cli_InitSessionByIndex( siIdx, pTheConsole ) )
        {
            return XERROR;
        }
    }
    
    /* 命令行初始化成功 */
    bClI_InitFlag = XTRUE;
    
#ifdef CLI_LOG
    cli_log_write( eCLINormal, "Task CLI init successfully!" );
#endif /* end  CLI_LOG  */
    
#ifdef XOS_TELNETS
    if ( XSUCC != XOS_TelnetSInitProc( ) )
    {
#ifdef CLI_LOG
        cli_log_write( eCLINormal, "Task telnet Server init successfully!" );
#endif
        return XFALSE;
    }
#endif
    
#ifdef  MEM_ID_DEBUG
    
    ret = XOS_RegistCmdPrompt( SYSTEM_MODE, "plat", "plat", "no paratmeter" );
    
    if ( XERROR >= ret )
    {
        XOS_CliInforPrintf("XOS_CliInit()->xos_RegistCmdPrompt ERROR!.error num is %d\r\n",ret);
        return XERROR;
    }
    
    if ( XERROR >= ( backvalue = XOS_RegistCommand(ret,
        xosMemCheck,
        "checkallmem", "chck all memory margin whether overflow",
        " no parameter") ) )
    {
        XOS_CliInforPrintf("XOS_CliInit()->xos_Regist Command  (checkallmem) ERROR NUM IS %d\r\n",backvalue);
    }
    
#endif
    
#ifdef XOS_VXWORKS
    /*注册控制终端输出的开关命令*/
    
    ret = XOS_RegistCmdPrompt( SYSTEM_MODE, "plat", "plat", "no parameter" );
    
    if ( XERROR >= ret )
    {
        XOS_CliInforPrintf("XOS_CliInit()->xos_RegistCmdPrompt ERROR!.error num is %d\r\n",ret);
        return XERROR;
    }
    
    if ( XERROR >= ( backvalue = XOS_RegistCommand(ret,
        setOutputForVxworks,
        "setoutputforvx", "set output to vxWorks terminal switch",
        "parameter: on/off; exp:setoutputforvx on") ) )
    {
        XOS_CliInforPrintf("XOS_CliInit()->xos_Regist Command  (setoutputforvx) ERROR NUM IS %d\r\n",backvalue);
    }
    
#endif /*_VXWORKS_*/


//#define CXF_OAM_BRD_TEST 1    // test code ,normal undef this macro
    {
#ifdef CXF_OAM_BRD_TEST
        int ret = 0;
        char aucBrdInfo[CLI_PRE_FIX_LEN] = {0};
        //20110215 cxf add ???
        Cli_remoteCliFuncReg(CLI_FUNCID_GETPIDLIST,cli_getRemotePidLst);

        sprintf(aucBrdInfo,"cliMsg from brd[%d],IP[%s]",10,"169.0.199.19");
        Cli_SetCliPrefixInfo(aucBrdInfo);

        ret = XOS_RegistCmdPrompt( SYSTEM_MODE, "cxf", "cxf", "no paratmeter" );
        
        cli_setRemoteCliRegFlag(1);
        XOS_CLIRegCommand(ret);
        Trace_regCliCmd(ret);
//#else
        cli_setRemoteCliRegFlag(0);
#endif
    }
    return XSUCC;
}

/************************************************************************
 函数名: XOS_CLICloseProc(  )
 功能:   是初始化函数的反向操作
 输入:
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR,或者错误
 说明:
************************************************************************/
XS8 XOS_CLICloseProc( XVOID *t, XVOID *v )
{
    XOS_MutexDelete( &g_cliMutex );
    XOS_MutexDelete( &g_mutexCliPrt);
    
    return XSUCC;
}

XS32 cli_RecvRemoteCliReq(t_XOSCOMMHEAD *pMsg);
XS32 cli_RecvRemoteCliRsp(t_XOSCOMMHEAD *pMsg);
/************************************************************************
 函数名: CLI_MsgProc(  )
 功能:   根据用户输入的消息(对每个字符)进行分发处理
 输入:   pstMsgHeader: 消息头部指针,用户输入的字符
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR,或者错误
 说明:
************************************************************************/
XS8 XOS_CLIMsgProc( XVOID *t, XVOID *v )
{
    CLI_ENV*       pCliEnv   = XNULL;
    XS32           status    = 0;
    XS32           s32SId    = 0;
    t_XOSCOMMHEAD* pCh       = XNULL;
    XU32           MsgID     = 0;
    t_XOS_COM_CHAN *pChannel = XNULL;
    
    if ( XNULL == t )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: XOS_CLIMsgProc() XNULL == t failed!");
        return XERROR;
    }
        
    /* 如果命令行初始化,即内存资源分配失败的话,禁用消息接受函数 */
    if ( !bClI_InitFlag )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: bClI_InitFlag failed!");
        return XERROR;
    }
    
    pCh = (t_XOSCOMMHEAD*)t;
    
#ifdef XOS_ModMem_Check
    if((XNULL == pCh) || (pCh->message != (XCHAR*)pCh+sizeof(t_XOSCOMMHEAD)))
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: XOS_CLIMsgProc() get msg is wrong!");
#ifdef XOS_DEBUG
        /*调试用，如出现内存错误，则直接挂起*/
        XOS_SusPend();
#endif
        
        return XERROR;
        
    }
    
    if(XSUCC != XOS_MemCheck(FID_CLI,pCh))
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "check mem error.srcFid(%d),dstFid(%d),msgid(%d)",
                                        pCh->datasrc.FID,
                                        pCh->datadest.FID,
                                        pCh->msgID);
#ifdef XOS_DEBUG
        /*调试用，如出现内存错误，则直接挂起*/
        XOS_SusPend();
#endif
        
        return XERROR;
    }
    
#endif
    
    if ( XNULL == pCh  )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: get msg header pointer failed!");
        return XERROR;
    }
    
    s32SId  = pCh->subID;
    MsgID   = pCh->msgID;

    if(FID_CLI == pCh->datasrc.FID) // 20110107 cxf add
    {
        switch(MsgID)
        {
            case MSG_REMOTE_CLI_REQ:
                cli_RecvRemoteCliReq(pCh);
                return XSUCC;
            case MSG_REMOTE_CLI_RSP:
                cli_RecvRemoteCliRsp(pCh);
                return XSUCC;
            default:
                break;
        }
    }
    
    /*发送者功能块号*/
    pCliEnv = cli_telnetGetCliSession( s32SId );
    if( XNULL == pCliEnv )
    {
        cli_sendTelnetCloseReq( s32SId );
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR:  XNULL == pCliEnv !");
        return XERROR;
    }
    if( MMISC_GetFsmId(pCliEnv) != s32SId )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: MMISC_GetFsmId(pCliEnv) != s32SId !");
        return XERROR;
    }
    
    switch( MsgID )
    {
    case MSG_IP_DATAIND:
        {
            if(XSUCC != cli_extReadCh( pCliEnv, (XU8*)pCh->message, (XU16)pCh->length ))
            {
                XOS_Trace(MD(FID_CLI, PL_ERR), "XOS_CLIMsgProc: call cli_extReadCh failed");
            }
            
            /*是否登录系统*/
            if( MMISC_GetLoginStat(pCliEnv) != kTELNET_USR_PROCESS )
            {
                break;
            }
            
            if( MMISC_GetInput(pCliEnv) == 0 )
            {
                if ( 0 < MEDIT_GetLength(pCliEnv) )
                {
                    cli_histAddHistLine( pCliEnv );
                    
                    /*查找命令*/
                    status = cli_parseCommand( pCliEnv );
                    switch ( status )
                    {
                    case XSUCC:
                        cli_excuteCommand( pCliEnv );
                        break;
                    case ERROR_CLI_CONFLICT:
                        break;
                    case STATUS_CLI_INTERNAL_COMMAND:
                    case STATUS_CLI_NO_INTERMEDIATE:
                    case ERROR_CLI_NO_INPUT_DATA:
                        break;
                    default:
                        cli_taskPrintError( pCliEnv, status );
                        break;
                    }
                }
                cli_extResetOutput(pCliEnv);
                if( CLIENV(pCliEnv)->cliShellCmd.cliMode != ExitCliShellMd )
                {
                    XOS_CliExtWriteStr( pCliEnv, CLIENV(pCliEnv)->cliShellCmd.pscCursor );
                }
                cli_extInitCommandLine(pCliEnv);
                
                switch( s32SId )
                {
                    /* distinguish console and telnet */
                case LOCAL_CLI_SESSION:
                    {
                        /* local console 处理来自本地客户端的消息 */
                        if( CLIENV(pCliEnv)->cliShellCmd.cliMode == ExitCliShellMd )
                        {
                            /* loop again */
                            cli_UTILInit(pCliEnv);
                            
                            MMISC_SetLoginStat(pCliEnv, kTELNET_NOT_LOGIN);
                            MHIST_History(pCliEnv)->iCurHistCmd  = 0;
                            MHIST_History(pCliEnv)->iNumCmds     = 0;
                            MHIST_History(pCliEnv)->bufferIndex  = 0;
                        }
                    }
                    break;
                    
                default:
                    /* distinguish each client:  0 < usFsmId <= kCLI_MAX_CLI_TASK */
                    if( CLIENV(pCliEnv)->cliShellCmd.cliMode == ExitCliShellMd )
                    {
                        /* 关闭 Telnet 客户端 */
                        cli_sendTelnetCloseReq(MMISC_GetFsmId(pCliEnv));
                        
                        /* 命令树根节点 */
                        CLIENV(pCliEnv)->cliShellCmd.cliMode = SYSTEM_MODE;
                        CLIENV(pCliEnv)->cliShellCmd.pCmdLst = g_paCliAppMode[SYSTEM_MODE].pModePCmds;
                        
                        /* 修改提示符 */
                        cli_initPrompt(pCliEnv);
                        MMISC_SetLoginStat(pCliEnv, kTELNET_NOT_LOGIN);
                        MEDIT_SetKeyStat(pCliEnv, KEY_STATE_DATA);
                        MHIST_History(pCliEnv)->iCurHistCmd  = 0;
                        MHIST_History(pCliEnv)->iNumCmds     = 0;
                    }
                    break;
                }
            }
        }
        
        break;
        
    case MSG_IP_CLOSEIND:
        {
            /* 退出登录 */
            cli_delete_usid_logined(s32SId);
            /* close by remote client */
            /* 命令树根节点 */
            CLIENV(pCliEnv)->cliShellCmd.cliMode = SYSTEM_MODE;
            CLIENV(pCliEnv)->cliShellCmd.pCmdLst = g_paCliAppMode[SYSTEM_MODE].pModePCmds;
            
            /* 修改提示符 */
            cli_initPrompt( pCliEnv );
            MMISC_SetLoginStat( pCliEnv, kTELNET_NOT_LOGIN );
            MEDIT_SetKeyStat( pCliEnv, KEY_STATE_DATA );
            MHIST_History( pCliEnv )->iCurHistCmd  = 0;
            MHIST_History( pCliEnv )->iNumCmds     = 0;
            
            /*清除历史记录,20081210 added*/
            cli_histResetHistInfo(pCliEnv);
            //cli_delete_usid_logined( MMISC_GetFsmId( pCliEnv ) );
        }
        break;
        
    case MSG_IP_CONNECTIND:
        {
            if ( XSUCC != XOS_CliTelnetInit(pCliEnv) )
            {
                CLI_EnableFeature(  pCliEnv, kCLI_FLAG_ECHO );
                XOS_CliExtWriteStr( pCliEnv, kCLI_MSG_FAIL  );
                
                pChannel = MMISC_GetChannel(pCliEnv);
                if ( XNULL != pChannel )
                {
                    pChannel->ThreadState = kThreadDead;
                    pChannel->InUse       = XFALSE;
                }
                
                /* CLI_TASK_Cleanup(pCliEnv); */
                return XSUCC;
            }
            /* 打印提示符 */
            /*XOS_CliExtWriteStr( pCliEnv, CLIENV(pCliEnv)->cliShellCmd.pscCursor );*/
        }
        break;
    default:
        break;
    }
    
    return XSUCC;
}

/************************************************************************
 函数名: cli_InitSessionByIndex(  )
 功能:   初始化Session
 输入:
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR,或者错误
 说明:
************************************************************************/
XSTATIC XS32 cli_InitSessionByIndex( XS32 s32Index, t_XOS_COM_CHAN *pChannel )
{
    t_XOS_GEN_ENVIRON  *pCliEnv = XNULL;
    
    if ( XNULL == pChannel )
    {
        return XERROR;
    }
    
    if ( XSUCC != cli_dbInitEnvironment( &pCliEnv, pChannel ) )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: cli_dbInitEnvironment() failed!");
        return XERROR;
    }
    
    MMISC_SetFsmId( pCliEnv, s32Index );
    gppCliSessions[s32Index] = pCliEnv;
    
    cli_UTILInit( pCliEnv );
    
    /*分配给本地 Console*/
    if ( 0 == s32Index )
    {
        MCONN_SetReadHandle(  pCliEnv, (ReadHandle*)cli_telnetConsoleRead);
        MCONN_SetWriteHandle( pCliEnv, (WriteHandle*)cli_telnetConsoleWrite);
        
        MCONN_SetConnType( pCliEnv,  kCONSOLE_CONNECTION );
        MSCRN_SetWidth(    pCliEnv,  kCLI_DEFAULT_WIDTH  );
        MSCRN_SetHeight(   pCliEnv,  kCLI_DEFAULT_HEIGHT );
    }
    else /* 远程 TELNET cli */
    {
        MCONN_SetReadHandle(  pCliEnv, (ReadHandle*)TELNET_RECV_FN );
        MCONN_SetWriteHandle( pCliEnv, (WriteHandle*)TELNET_SEND_FN );
    }
    
    /* 命令树根节点*/
    CLIENV(pCliEnv)->cliShellCmd.cliMode = SYSTEM_MODE;
    CLIENV(pCliEnv)->cliShellCmd.pCmdLst = g_paCliAppMode[SYSTEM_MODE].pModePCmds;
    
    /* 修改提示符*/
    cli_initPrompt( pCliEnv );
    
    MMISC_SetLoginStat( pCliEnv, kTELNET_NOT_LOGIN );
    MEDIT_SetKeyStat(   pCliEnv, KEY_STATE_DATA    );
    
    if ( XSUCC != cli_histInitHistInfo( pCliEnv ) )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: cli_histInitHistInfo() failed!");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
 函数名: cli_parseCommand(  )
 功能:   从用户输入的一段完整字符串中分析命令
 输入:   pCliEnv
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:   该函数获取用户输入的每一个字符,发现有换行符将其
         放入命令字结构
************************************************************************/
XSTATIC XS32 cli_parseCommand( CLI_ENV *pCliEnv )
{
    XS32  i             = 0;
    XS32  j             = 0;
    XS32  sIndex        = 0;
    XCHAR *pscTmpName   = XNULL;
    
    if ( XNULL == pCliEnv )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: cli_parseCommand() failed: XNULL == pCliEnv.");
        return XERROR;
    }
    
    XOS_MemSet( &CLIENV(pCliEnv)->cliShellCmd.cmdArg, 0x00, sizeof(t_XOS_CMD_ARG) );
    pscTmpName = MEDIT_BufPtr(pCliEnv);
    
    /*保存字符串*/
    for ( i = 0, j = 0;  i < CLI_MAX_INPUT_LEN; i++ )
    {
        /*用户输入了空格或TAB*/
        if ( pscTmpName[i] == ' ' || pscTmpName[i] == '\t' )
        {
            if ( 0 == i )
            {
                return XERROR;
            }
            /* 第一个字符串不是参数*/
            sIndex = CLIENV(pCliEnv)->cliShellCmd.cmdArg.siArgcnt;
            if ( -1 < sIndex  && CLI_MAX_ARG_NUM > sIndex  )
            {
                CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[ sIndex ][j] = '\0';
            }
            
            /* 下个字符是有效字符或逗号*/
            if (   pscTmpName[i+1] != '\0'
                && pscTmpName[i+1] != ' '
                && pscTmpName[i+1] != '\t')
            {
                /* 下一个参数*/
                CLIENV(pCliEnv)->cliShellCmd.cmdArg.siArgcnt++;
                
                j = 0;
            }
            
            if ( CLIENV(pCliEnv)->cliShellCmd.cmdArg.siArgcnt >= CLI_MAX_ARG_NUM )
            {
                XOS_Trace( MD(FID_CLI, PL_ERR), "cli_parseCommand() siArgcnt >= CLI_MAX_ARG_NUM!" );
                return ERROR_CLI_AMBIGUOUS_PARAM;
            }
        }
        else
        {
            /* 按字符保存*/
            CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[
                CLIENV(pCliEnv)->cliShellCmd.cmdArg.siArgcnt
                ][j] = pscTmpName[i];
            
            /* 结尾*/
            if( pscTmpName[i] == '\0' )
            {
                break;
            }
            
            j++;
            if( j >= CLI_MAX_ARG_LEN )
            {
                XOS_Trace( MD(FID_CLI, PL_ERR), "cli_parseCommand() j >= CLI_MAX_ARG_LEN!" );
                return ERROR_CLI_INVALID_PARAM;
            }
        }
    }
    
    /*分析命令和参数*/
    for( i = 0; i < CLI_MAX_CMD_NUM; i++ )
    {
        if( XNULL == CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pCmdHandler )
        {
            break;
        }
        
        pscTmpName = CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pscCmdName;
        
        /* 完整命令比较查找符合*/
        if( !XOS_StrCmp( pscTmpName, CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[0] ) )
        {
            CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx = i;
            return XSUCC;
        }
    }
    
    pscTmpName    = XNULL;
    j             = 0;
    sIndex        = 0;
    /*分析命令和参数*/
    for( i = 0; i < CLI_MAX_CMD_NUM; i++ )
    {
        if( CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pCmdHandler == XNULL)
        {
            break;
        }
        
        pscTmpName = CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pscCmdName;
        /*不完整命令比较查找符合*/
        if( XOS_StrStr( pscTmpName,CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[0]) == pscTmpName )
        {
            /*是否具有查看权限*/
            if ( MMISC_GetAccess(pCliEnv) <= CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].eAccLvl )
            {
                if ( 0 == j++ )
                {
                    sIndex = i;
                }
                else
                {
                    break;
                }
            }
        }
    }
    
    /*找到只有一个的冲突命令*/
    if ( 1 == j )
    {
        CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx = sIndex;
        return XSUCC;
    }
    else if ( 1 < j )
    {
        /*打印所有冲突命令*/
        j = 0;
        for( i = 0; i < CLI_MAX_CMD_NUM; i++ )
        {
            if( CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pCmdHandler == XNULL)
            {
                break;
            }
            
            pscTmpName = CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pscCmdName;
            /*不完整命令比较查找符合*/
            if( XOS_StrStr( pscTmpName,CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[0]) == pscTmpName )
            {
                /*是否具有查看权限*/
                if ( MMISC_GetAccess(pCliEnv) <= CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].eAccLvl )
                {
                    if ( 0 == j++ )
                    {
                        if ( 0 == MMISC_GetFsmId( pCliEnv ) )
                        {
                            XOS_CliExtWriteStrLine( pCliEnv,"" );
                        }
                        else
                        {
                            XOS_CliExtWriteStr( pCliEnv, "\r\n" );
                        }
                    }
                    /*找到该命令, 打印该相似命令*/
                    XOS_CliExtWriteStrLine( pCliEnv, pscTmpName );
                }
            }
        }
        if ( j > 0 )
        {
            return ERROR_CLI_CONFLICT;
        }
    }
    
    return ERROR_CLI_BAD_COMMAND;
}

/************************************************************************
 函数名: cli_taskPrintError(  )
 功能:  打印错误
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XSTATIC XVOID cli_taskPrintError(CLI_ENV *pCliEnv, XS32 status)
{
    XCHAR *errorText = MMISC_GetErrorText(pCliEnv);
    XS32  ret = 0;
    
    ret = XOS_CliExtWriteStr( pCliEnv, kCLI_MSG_ERROR_PREFIX );
    
    switch ( status )
    {
    case ERROR_CLI_BAD_COMMAND:
        ret = XOS_CliExtWriteStr(pCliEnv, kMSG_ERROR_CLI_BAD_COMMAND);
        break;
    case ERROR_CLI_INVALID_PARAM:
        ret = XOS_CliExtWriteStr(pCliEnv, kMSG_ERROR_CLI_INVALID_PARAM);
        break;
    case ERROR_CLI_AMBIGUOUS_PARAM:
        ret = XOS_CliExtWriteStr(pCliEnv, kMSG_ERROR_CLI_AMBIGUOUS_PARAM);
        break;
    default:
        ret = XOS_CliExtWriteStr(pCliEnv, kMSG_ERROR_CLI_DEFAULT );
    }
    
    if ( !NULL_STRING(errorText) )
    {
        ret = XOS_CliExtWriteStr(pCliEnv, errorText);
        MMISC_ClearErrorText(pCliEnv);
    }
    
    ret = XOS_CliExtWriteStrLine(pCliEnv, "");
    
    ret = ret;
}

/*-------------------------------------------------------------------------
                模块内部函数
-------------------------------------------------------------------------*/
/************************************************************************
 函数名: cli_findUser( )
 功能:   从用户列表中查找用户属性ID
 输入:   会话ID
 输出:   无
 返回:   当前对应的当前客户端结构指针
 说明:
************************************************************************/
XBOOL cli_findUser( XCHAR* szUserName, XCHAR* szPassWd, Access* ps32UserID )
{
    XS32 i = 0;
    
    if ( XNULL == szUserName || XNULL == szPassWd || XNULL == ps32UserID )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR: cli_findUser() parameters invalidate.");
        return XFALSE;
    }
    
    for ( i = 0; i < CLI_MAX_USER_NUM; i++ )
    {
        if ( XNULL != g_paUserInfor[i].szUserName
            && XNULL != g_paUserInfor[i].szPassWord )
        {
            if( !XOS_StrCmp( szUserName, g_paUserInfor[i].szUserName )
                && !XOS_StrCmp( szPassWd, g_paUserInfor[i].szPassWord ) )
            {
                *ps32UserID = (Access)g_paUserInfor[i].eAccLvl;
                return XTRUE;
            }
        }
        else
        {
            break;
        }
    }
    
    return XFALSE;
}

/************************************************************************
 函数名: cli_telnetGetCliSession( XS32 index )
 功能:   从命令行查找会话
 输入:   会话ID
 输出:   无
 返回:   当前对应的当前客户端结构指针
 说明:
************************************************************************/
XSTATIC CLI_ENV * cli_telnetGetCliSession( XS32 index )
{
    if ( index < 0 || index > kCLI_MAX_CLI_TASK )
    {
        return XNULL;
    }
    
    return gppCliSessions[index];
}

/************************************************************************
 函数名: cli_telnetConsoleChannel(  )
 功能:   获取当前控制台
 输入:   无
 输出:   无
 返回:   当前控制台结构指针
 说明:
************************************************************************/
XSTATIC t_XOS_COM_CHAN *cli_telnetConsoleChannel()
{
    return &gConsoleObject;
}

/************************************************************************
 函数名: XOS_TelnetBroadcastMessage(  )
 功能:   向所有 Telnet 客户端广播消息
 输入:   pMessage: 用户输入的字符
         authLevel
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR,或者错误
 说明:
************************************************************************/
XPUBLIC XS32 XOS_TelnetBroadcastMessage( XCHAR *pMessage, Access authLevel )
{
    XS32           index     = 0;
    CLI_ENV        *pCliDest = NULL;
    t_XOS_COM_CHAN *pChannel = NULL;
    
    authLevel = authLevel;
    
    if ( pMessage == XNULL )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "XOS_TelnetBroadcastMessage(pMessage == XNULL)!");
        return XERROR;
    }
    
    /* 1 ~ kCLI_MAX_CLI_TASK 是 Telnet 客户端*/
    for ( index = 1; index <= kCLI_MAX_CLI_TASK; index++ )
    {
        pCliDest = cli_telnetGetCliSession(index);
        
        if ( XNULL == pCliDest )
        {
            continue;
        }
        
        pChannel = MMISC_GetChannel(pCliDest);
        
        if (kThreadWorking != pChannel->ThreadState)
        {
            continue;
        }
        
        if(kTELNET_USR_PROCESS != MMISC_GetLoginStat(pCliDest))
        {
            continue;
        }
        
        /*XOS_CliExtWriteStr(pCliDest, pMessage);*/
        if ( XSUCC != cli_extPutStr(pCliDest, pMessage) )
        {
            continue;
        }
    }
    return XSUCC;
}

/************************************************************************
 函数名: cli_pciConsoleRead(  )
 功能:   从pci控制台读取一个字符
 输入:   pEnv
         charIn1
         pBuf
         bytesRead
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:
************************************************************************/
#ifdef XOS_WIN32
XSTATIC  XS32 cli_pciConsoleRead( CLI_ENV *pEnv, XU8 charIn1, XCHAR *pBuf, XS32 *bytesRead )
{
    XCHAR charIn = charIn1;
    
    if (   XNULL == pEnv
        || XNULL == pBuf
        || XNULL == bytesRead )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_pciConsoleRead() param invalidate!");
        return XERROR;
    }
    
    *bytesRead = 0;
    
    if ( kCLI_DOSKEY_ESC == charIn )
    {
        charIn = (XS8)XOS_GetChar();
        switch(charIn)
        {
        case kCLI_DOSKEY_UP:
            charIn = kKEY_MOVE_UP;
            break;
        case kCLI_DOSKEY_DN:
            charIn = kKEY_MOVE_DOWN;
            break;
        case kCLI_DOSKEY_LT:
            charIn = kKEY_MOVE_LEFT;
            break;
        case kCLI_DOSKEY_RT:
            charIn = kKEY_MOVE_RIGHT;
            break;
        case kCLI_DOSKEY_DEL:
            charIn = kKEY_DELETE_CHAR;
            break;
        default:
            break;
        }
    }
    
    /* unix uses lf as end of line */
#ifndef XOS_WIN32
    if (kLF == charIn)
        charIn = kCR;
#endif
    
    pBuf[(*bytesRead)++] = charIn;
    
    return XSUCC;
}
#endif

/************************************************************************
 函数名: cli_telnetConsoleWrite(  )
 功能:   向pci控制台输出字符串
 输入:   pEnv
         pBuf
         BufSize

 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32 cli_pciConsoleWrite( CLI_ENV *pEnv, const XCHAR *pBuf, XS32 s32StrLen )
{
    /*为解决BM使用shell脚本拉起网元出现printf打印过多导致printf内部锁挂死，这里不打印printf*/
    return XSUCC;
    
    if ( XNULL == pBuf || 0 > s32StrLen )
    {
        return XERROR;
    }

    if(XOS_StrLen(pBuf))
    {
        printf("%s",pBuf);
    }
    
    return XSUCC;
}

/************************************************************************
 函数名: cli_telnetConsoleRead(  )
 功能:   从控制台读取一个字符
 输入:   pEnv
         charIn1
         pBuf
         bytesRead
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:
************************************************************************/
XSTATIC  XS32 cli_telnetConsoleRead( CLI_ENV *pEnv, XU8 charIn1, XCHAR *pBuf, XS32 *bytesRead )
{
    XCHAR charIn = charIn1;
    
    if ( XNULL == pEnv
        || XNULL == pBuf
        || XNULL == bytesRead )
    {
        return XERROR;
    }
    
    *bytesRead = 0;
    
    if ( kCLI_DOSKEY_ESC == charIn )
    {
        charIn = XOS_GetChar();
        switch(charIn)
        {
        case kCLI_DOSKEY_UP:
            charIn = kKEY_MOVE_UP;
            break;
        case kCLI_DOSKEY_DN:
            charIn = kKEY_MOVE_DOWN;
            break;
        case kCLI_DOSKEY_LT:
            charIn = kKEY_MOVE_LEFT;
            break;
        case kCLI_DOSKEY_RT:
            charIn = kKEY_MOVE_RIGHT;
            break;
        case kCLI_DOSKEY_DEL:
            charIn = kKEY_DELETE_CHAR;
            break;
        default:
            break;
        }
    }
    
    /* unix uses lf as end of line */
#ifndef XOS_WIN32
    if (kLF == charIn)
        charIn = kCR;
#endif
    
    pBuf[(*bytesRead)++] = charIn;
    
    return XSUCC;
}

/************************************************************************
 函数名: cli_telnetConsoleWrite(  )
 功能:   向控制台输出字符串
 输入:   pEnv
         pBuf
         BufSize

 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32 cli_telnetConsoleWrite( CLI_ENV *pEnv, XCHAR *pBuf, XS32 BufSize )
{
    if (   XNULL == pEnv
        || XNULL == pBuf
        || 0 > BufSize
        )
    {
        return XERROR;
    }
    XOS_Fwrite( pBuf, 1, (XU32)BufSize, stdout );
    fflush( stdout );
    
    return XSUCC;
}

/************************************************************************
 函数名: cli_excuteCommand(  )
 功能:   执行命令
 输入:
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32 cli_excuteCommand( CLI_ENV *pCliEnv )
{
    XS32  i                                  = 0;
    XCHAR cache[ 2*CLI_MAX_ARG_LEN  ]        = {0};
    XCHAR *pscArgument[CLI_MAX_ARG_NUM + 1]  = {0};
    XVOID (* func)(CLI_ENV *, XS32, XCHAR **);
    t_XOS_CMD_DATAS  *pCmdLst                = XNULL;
    func                                     = XNULL;
    
    for( i = 0; i < CLI_MAX_ARG_NUM; i++ )
    {
        if( XNULL != CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[i] )
        {
            /* 保存参数,从1开始,0保存的是命令字  */
            pscArgument[i] = &CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[i][0];
        }
        else
        {
            /* 参数结束 */
            break;
        }
    }
    
    i       = CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx;
    pCmdLst = CLIENV(pCliEnv)->cliShellCmd.pCmdLst;
    /* 参数帮助 */
    if( !XOS_StrCmp(pscArgument[1], "?") )
    {
        XOS_CliExtWriteStrLine(pCliEnv, "description");
        XOS_CliExtWriteStrLine(pCliEnv, "---------");
        
        if ( 0 > i )
        {
            return XERROR;
        }
        
        sprintf( cache, "%s", pCmdLst[i].pscParaHelpStr );
        XOS_CliExtWriteStrLine( pCliEnv, cache );
        
        return XSUCC;
    }
    
    if ( MMISC_GetAccess(pCliEnv) > pCmdLst[i].eAccLvl )
    {
        XOS_CliExtWriteStr( pCliEnv, kMSG_ERROR_CLI_BAD_COMMAND1 );
        return XSUCC;
    }
    
    func = (XVOID(*)(CLI_ENV *, XS32,XCHAR **))( pCmdLst[
        CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx
        ].pCmdHandler );
    if ( XNULL != func )
    {
        func(pCliEnv, CLIENV(pCliEnv)->cliShellCmd.cmdArg.siArgcnt + 1, pscArgument);
    }
    
    return XSUCC;
}

/************************************************************************
 函数名: cli_cmdVersion(  )
 功能:   XOS版本查询命令响应函数
 输入:   无
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_cmdVersion(CLI_ENV* pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XS32 i;
    ppArgv = ppArgv;
    siArgc = siArgc;
    for(i = 0; i < CLI_REGVER_MAX; i++)
    {
        if(g_paCliVer[i].verFlag==XTRUE)
        {
            XOS_CliExtPrintf(pCliEnv,"    %-16s:%s\r\n",g_paCliVer[i].szVerName,g_paCliVer[i].szVerValue);
        }
    }
    
}

/************************************************************************
 函数名: cli_cmdSID(  )
 功能:   Telnet Client SID 查询命令
 输入:   无
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_cmdSID( CLI_ENV* pCliEnv, XS32 siArgc,XCHAR **ppArgv )
{
    ppArgv = ppArgv;
    if( siArgc == 1 )
    {
        XOS_CliExtPrintf(pCliEnv,"SID: \"%d\".", MMISC_GetFsmId(pCliEnv) );
    }
}

/************************************************************************
 函数名: cli_initCmdTree(  )
 功能:   初始化用户命令结构
 输入:   无
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:   该函数获取用户输入的每一个字符,发现有换行符将其
         放入命令字结构
         Review OK
************************************************************************/
XSTATIC XBOOL cli_initCmdTree( XVOID )
{
    /* 申请动态内存 */
    if ( !cli_initCliMem() )
    {
        cli_releaseCliMem();
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_initCliMem failed!");
        return XFALSE;
    }
    /* 初始化动态内存区,只调用一次*/
    if ( !cli_initCliApCmd() )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_initCliApCmd failed!");
        return XFALSE;
    }
    
    return XTRUE;
}

/************************************************************************
 函数名: cli_initCliMem(  )
 功能:   初始化用户命令内存
 输入:   无
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:   Review OK
************************************************************************/
XSTATIC XBOOL cli_initCliMem( XVOID )
{
    XS32 i = 0;
    t_XOS_CMD_DATAS *pSysCmds = XNULL;
    
    /*命令集中最大数*/
    for( i = 0; i < CLI_MAX_CMD_CLASS; i++ )
    {
        g_paCliAppMode[i].pModePCmds = XNULL;
        XOS_MemSet( g_paCliAppMode[i].pscCurSor, 0x00, CLI_MAX_CURSOR_LEN + 1 );
    }
    
    for( i = 0; i < CLI_REGVER_MAX; i++ )
    {
        XOS_MemSet(&g_paCliVer[i],0x00,sizeof(t_XOS_CLI_VER));
    }
    
    /*每组命令集中的命令最大数*/
    i = CLI_MAX_CMD_NUM * sizeof( t_XOS_CMD_DATAS );
    pSysCmds = (t_XOS_CMD_DATAS*)XOS_MemMalloc( FID_CLI, i );
    if ( pSysCmds == XNULL )
    {
        return XFALSE;
    }
    XOS_MemSet( pSysCmds, 0x00, i );
    
    if ( CLI_MAX_CMD_NUM > CLI_INIT_CMDS_NUM )
    {
        /*初始化系统模式*/
        pSysCmds[INDEX_CMD_HELP].pCmdHandler    = cli_cmdHelp;
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP].pscCmdName,      "help", CLI_MAX_CMD_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP].pscCmdHelpStr,  "help", CLI_MAX_CMD_HELP_LEN  );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP].pscParaHelpStr, "no parameter", CLI_MAX_ARG_LEN );
        pSysCmds[INDEX_CMD_HELP].eAccLvl        = eUserNormal;
        
        pSysCmds[INDEX_CMD_HELP1].pCmdHandler    = cli_cmdHelp;
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP1].pscCmdName,      "?", CLI_MAX_CMD_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP1].pscCmdHelpStr,  "quick help", CLI_MAX_CMD_HELP_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP1].pscParaHelpStr, "no parameter", CLI_MAX_ARG_LEN );
        pSysCmds[INDEX_CMD_HELP1].eAccLvl        = eUserNormal;
        
        pSysCmds[INDEX_CMD_CLEAR].pCmdHandler    = cli_cmdCls;
        XOS_StrNcpy( pSysCmds[INDEX_CMD_CLEAR].pscCmdName,      "clear", CLI_MAX_CMD_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_CLEAR].pscCmdHelpStr,  "clear screen", CLI_MAX_CMD_HELP_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_CLEAR].pscParaHelpStr, "no parameter", CLI_MAX_ARG_LEN );
        pSysCmds[INDEX_CMD_CLEAR].eAccLvl        = eUserNormal;
        
        pSysCmds[INDEX_CMD_QUIT].pCmdHandler     = cli_cmdExit;
        XOS_StrNcpy( pSysCmds[INDEX_CMD_QUIT].pscCmdName,      "exit", CLI_MAX_CMD_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_QUIT].pscCmdHelpStr,  "exit", CLI_MAX_CMD_HELP_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_QUIT].pscParaHelpStr, "exit     -- exit cur login user\r\nexit xos -- exit xos command", CLI_MAX_ARG_LEN );
        pSysCmds[INDEX_CMD_QUIT].eAccLvl           = eUserNormal;
        
        pSysCmds[INDEX_CMD_VERSION].pCmdHandler    = cli_cmdVersion;
        XOS_StrNcpy( pSysCmds[INDEX_CMD_VERSION].pscCmdName,      "ver", CLI_MAX_CMD_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_VERSION].pscCmdHelpStr,  "display version", CLI_MAX_CMD_HELP_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_VERSION].pscParaHelpStr, "no parameter", CLI_MAX_ARG_LEN );
        pSysCmds[INDEX_CMD_VERSION].eAccLvl        = eUserNormal;
        
        pSysCmds[INDEX_CMD_SID].pCmdHandler        = cli_cmdSID;
        XOS_StrNcpy( pSysCmds[INDEX_CMD_SID].pscCmdName,      "sid", CLI_MAX_CMD_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_SID].pscCmdHelpStr,  "display telnet client ID", CLI_MAX_CMD_HELP_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_SID].pscParaHelpStr, "no parameter", CLI_MAX_ARG_LEN );
        pSysCmds[INDEX_CMD_SID].eAccLvl            = eUserNormal;
        
    }
    
    /*系统提示符 kCLI_DEFAULT_PROMPT*/
    sprintf( g_paCliAppMode[SYSTEM_MODE].pscCurSor, "\r\n%s>", kCLI_DEFAULT_PROMPT );
    g_paCliAppMode[SYSTEM_MODE].pModePCmds = pSysCmds;
    XOS_RegAppVer("Build",XOS_BUILD_TIME);
    XOS_RegAppVer("Xcps",XOS_OS_VERSION);
    return XTRUE;
}

/************************************************************************
 函数名: cli_releaseCliMem(  )
 功能:   释放用户命令内存, 是初始化用户命令内存的反向操作
 输入:   无
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:   Review OK
************************************************************************/
XSTATIC XVOID  cli_releaseCliMem( XVOID )
{
    XS32 i = 0;
    t_XOS_CMD_DATAS *pTemCmds = XNULL;
    
    for( i = 0; i < CLI_MAX_CMD_CLASS; i++ )
    {
        pTemCmds = g_paCliAppMode[i].pModePCmds;
        if ( XNULL != pTemCmds )
        {
            XOS_MemFree( FID_CLI, pTemCmds );
            XOS_MemSet( &g_paCliAppMode[i], 0x00, sizeof(t_XOS_CLI_AP_CMDS_MODE) );
        }
    }
    
    return ;
}

/************************************************************************
 函数名: cli_NewCmds(  )
 功能:   为一个提示符生成新的命令结构
 输入:   无
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:   Review OK
************************************************************************/
XSTATIC t_XOS_CMD_DATAS *cli_NewCmds( XVOID )
{
    t_XOS_CMD_DATAS *pSysCmds = XNULL;
    XU32 cmdSize              = 0;
    
    cmdSize = CLI_MAX_CMD_NUM;
    pSysCmds = (t_XOS_CMD_DATAS*)XOS_MemMalloc( FID_CLI, cmdSize*sizeof( t_XOS_CMD_DATAS ) );
    if ( pSysCmds == XNULL )
    {
        return XNULL;
    }
    XOS_MemSet( pSysCmds, 0x00, cmdSize*sizeof( t_XOS_CMD_DATAS ) );
    
    if ( cmdSize > CLI_INIT_CMDS_NUM )
    {
        /*初始化系统模式*/
        pSysCmds[INDEX_CMD_HELP].pCmdHandler    = cli_cmdHelp;
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP].pscCmdName,      "help", CLI_MAX_CMD_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP].pscCmdHelpStr,  "help", CLI_MAX_CMD_HELP_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP].pscParaHelpStr, "no parameter", CLI_MAX_ARG_LEN );
        pSysCmds[INDEX_CMD_HELP].eAccLvl        = eUserNormal;
        
        pSysCmds[INDEX_CMD_HELP1].pCmdHandler    = cli_cmdHelp;
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP1].pscCmdName,      "?", CLI_MAX_CMD_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP1].pscCmdHelpStr,  "quick help", CLI_MAX_CMD_HELP_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_HELP1].pscParaHelpStr, "no parameter", CLI_MAX_ARG_LEN );
        pSysCmds[INDEX_CMD_HELP1].eAccLvl        = eUserNormal;
        
        pSysCmds[INDEX_CMD_CLEAR].pCmdHandler    = cli_cmdCls;
        XOS_StrNcpy( pSysCmds[INDEX_CMD_CLEAR].pscCmdName,      "clear", CLI_MAX_CMD_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_CLEAR].pscCmdHelpStr,  "clear screen", CLI_MAX_CMD_HELP_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_CLEAR].pscParaHelpStr, "no parameter", CLI_MAX_ARG_LEN );
        pSysCmds[INDEX_CMD_CLEAR].eAccLvl        = eUserNormal;
        
        pSysCmds[INDEX_CMD_QUIT].pCmdHandler    = cli_cmdQuit;
        XOS_StrNcpy( pSysCmds[INDEX_CMD_QUIT].pscCmdName,      "quit", CLI_MAX_CMD_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_QUIT].pscCmdHelpStr,  "to up menu", CLI_MAX_CMD_HELP_LEN );
        XOS_StrNcpy( pSysCmds[INDEX_CMD_QUIT].pscParaHelpStr, "no parameter", CLI_MAX_ARG_LEN );
        pSysCmds[INDEX_CMD_QUIT].eAccLvl        = eUserNormal;
    }
    
    return pSysCmds;
}

/************************************************************************
 函数名: cli_initCliMem(  )
 功能:   初始化用户命令
 输入:   无
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:   Review OK
************************************************************************/
XSTATIC XBOOL cli_initCliApCmd( XVOID )
{
    return XOS_CLIRegCommand( SYSTEM_MODE);
}

/************************************************************************
 函数名: cli_initPrompt(  )
 功能:   根据模式修改提示符
 输入:   无
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:   该函数在每次模式跳转的时候调用
************************************************************************/
XSTATIC XVOID cli_initPrompt(CLI_ENV* pCliEnv)
{
    XCHAR* pscCmdNextCoursor;
    XS32 CurCliMode = CLIENV(pCliEnv)->cliShellCmd.cliMode;
    
    if ( ExitCliShellMd == CurCliMode )
    {
        return;
    }
    
    if ( CurCliMode < 0 )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "CurCliMode error!");
        return;
    }
    
    pscCmdNextCoursor = g_paCliAppMode[CurCliMode].pscCurSor;
    /* 根据模式, 找到提示提示符*/
    XOS_StrNcpy( CLIENV(pCliEnv)->cliShellCmd.pscCursor, pscCmdNextCoursor, CLI_MAX_ARG_LEN );
    return;
}

/*
* ----------------------------------------------------------------------
*   命令树处理函数
* ----------------------------------------------------------------------
*/

/************************************************************************
 函数名: cli_PromptSwitch(  )
 功能:   切换到对应的模式
 输入:   无
 输出:   无
 返回:   无
 说明:   该函数在每次模式切换的时候调用
************************************************************************/
XSTATIC XVOID cli_PromptSwitch( CLI_ENV* pCliEnv, XU32 siDstMode )
{
    CLIENV(pCliEnv)->cliShellCmd.cliMode = siDstMode;
    CLIENV(pCliEnv)->cliShellCmd.ulPara0 = 0;
    CLIENV(pCliEnv)->cliShellCmd.ulPara1 = 0;
    
    cli_initPrompt(pCliEnv);
    
    if ( ExitCliShellMd == siDstMode )
    {
        CLIENV(pCliEnv)->cliShellCmd.cliMode = ExitCliShellMd;
        return;
    }
    CLIENV(pCliEnv)->cliShellCmd.pCmdLst = g_paCliAppMode[siDstMode].pModePCmds;
    
    return;
}

/************************************************************************
 函数名: cli_cmdSwitchPrompt(  )
 功能:   模式切换命令响应函数
 输入:   无
 输出:   无
 返回:   无
 说明:   该函数在每次模式切换的时候调用
************************************************************************/
XSTATIC XVOID cli_cmdSwitchPrompt(CLI_ENV* pCliEnv, XS32 siArgc , XCHAR **ppArgv)
{
    XS32 nextMode = 0;
    XS32 sIndex   = 0;
    
    if ( XNULL == pCliEnv || 1 > siArgc || XNULL == ppArgv )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_cmdSwitchPrompt() parameter error!");
        return;
    }
    
    sIndex   = CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx;
    if ( sIndex > 0 )
    {
        nextMode = CLIENV(pCliEnv)->cliShellCmd.pCmdLst[ sIndex].m_nMode;
    }
    cli_PromptSwitch( pCliEnv, (XU32)nextMode );
    
    return;
}

/************************************************************************
 函数名: cli_cmdSwitchPrompt(  )
 功能:   退出命令行命令响应函数
 输入:   无
 输出:   无
 返回:   无
 说明:
************************************************************************/
XSTATIC XVOID cli_cmdExit(CLI_ENV* pCliEnv, XS32 siArgc , XCHAR **ppArgv)
{
    XS32 s32SID = 0;
    
    if ( XNULL == pCliEnv || 1 > siArgc || XNULL == ppArgv  )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_cmdExit() parameter error!");
        return;
    }
    
    s32SID = MMISC_GetFsmId(pCliEnv);
    
    if ( 2 == siArgc && !XOS_StrCmp( ppArgv[1], "xos" ) )
    {
        if ( MMISC_GetAccess( pCliEnv ) > eUserSuper )
        {
            XOS_CliExtWriteStrLine( pCliEnv, kMSG_ERROR_CLI_PER_COMMAND );
            return;
        }
        //#if ( XOS_LINUX )
#ifdef XOS_LINUX
        tcsetattr( STDIN_FILENO, TCSANOW, &g_save_term );
#endif
        //#if ( XOS_WIN32 || XOS_LINUX || XOS_SOLARIS )
#if defined (XOS_WIN32) || defined (XOS_LINUX) || defined (XOS_SOLARIS)
        exit( 0 );
#endif
    }
    else
    {
        cli_PromptSwitch( pCliEnv, ExitCliShellMd );
        /* 退出登录 */
        cli_delete_usid_logined( s32SID );
    }
    
    return;
}

/************************************************************************
 函数名: cli_cmdQuit(  )
 功能:   退出当前提示符命令响应函数
 输入:   无
 输出:   无
 返回:   无
 说明:   退出当前提示符,返回到上级提示符
************************************************************************/
XSTATIC XVOID cli_cmdQuit(CLI_ENV* pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*命令 顺序号*/
    XS32 parentMode = 0;
    XCHAR* pscCmdNextCoursor = XNULL;
    
    if ( XNULL == pCliEnv || 1 > siArgc || XNULL == ppArgv  )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_cmdQuit() parameter error!");
        return;
    }
    
    parentMode = CLIENV(pCliEnv)->cliShellCmd.pCmdLst[ INDEX_CMD_QUIT ].m_nParent;
    
    if ( parentMode < 0 )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "parentMode error!");
        return;
    }
    
    /*模式切换*/
    CLIENV(pCliEnv)->cliShellCmd.pCmdLst = g_paCliAppMode[parentMode].pModePCmds;
    pscCmdNextCoursor = g_paCliAppMode[parentMode].pscCurSor;
    /* 根据模式, 找到提示提示符*/
    XOS_StrNcpy( CLIENV(pCliEnv)->cliShellCmd.pscCursor, pscCmdNextCoursor, CLI_MAX_ARG_LEN );
    
    return;
}

/************************************************************************
 函数名: cli_cmdHelp(  )
 功能:   帮助命令响应函数
 输入:   无
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_cmdHelp(CLI_ENV* pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XS32 i = 0;
    XCHAR cache[ 2*CLI_MAX_ARG_LEN + 1 ]     = {0};
    XCHAR cacheExit[ 2*CLI_MAX_ARG_LEN + 1 ] = {0};
    XS32 ret = 0;
    t_XOS_CMD_DATAS  *pCmdLst                = XNULL;
    
    pCmdLst = CLIENV(pCliEnv)->cliShellCmd.pCmdLst;
    
    if( siArgc > 2 )
    {
        ret = XOS_CliExtWriteStrLine( pCliEnv,"invalid parameter");
    }
    
    if( siArgc == 1 )
    {
        ret = XOS_CliExtWriteStrLine( pCliEnv, "           command list           " );
        ret = XOS_CliExtWriteStrLine( pCliEnv, "----------------------------------" );
        
        for( i = 0; XOS_StrLen( pCmdLst[i].pscCmdName ) > 0; i++ )
        {
            if ( MMISC_GetAccess(pCliEnv) > pCmdLst[i].eAccLvl )
            {
                continue;
            }
            
            sprintf( cache,
                "%-20s%-2s%-6s\r\n",
                pCmdLst[i].pscCmdName,
                "- ",
                pCmdLst[i].pscCmdHelpStr);
            
            if ( i != SYSTEM_EXIT_QUIT_INDEX )
            {
                ret = XOS_CliExtWriteStr( pCliEnv, cache );
            }
            else
            {
                XOS_StrNcpy( cacheExit, cache, 2*CLI_MAX_CMD_HELP_LEN );
            }
        }
        ret = XOS_CliExtWriteStr( pCliEnv, cacheExit );
    }
    else if( siArgc == 2 )
    {
        ret = XOS_CliExtWriteStrLine( pCliEnv, "       parameter dispcription     " );
        ret = XOS_CliExtWriteStrLine( pCliEnv, "----------------------------------" );
        
        for( i = 0; XOS_StrLen( pCmdLst[i].pscCmdName) > 0; i++ )
        {
            if ( MMISC_GetAccess(pCliEnv) > pCmdLst[i].eAccLvl )
            {
                continue;
            }
            
            if( !XOS_StrCmp( pCmdLst[i].pscCmdName, ppArgv[1]) )
            {
                sprintf( cache, "%s", pCmdLst[i].pscParaHelpStr );
                ret = XOS_CliExtWriteStrLine( pCliEnv, cache );
            }
        }
    }
    
    ret = ret;
    
    return;
}

/************************************************************************
 函数名: cli_cmdCls(  )
 功能:   清屏函数
 输入:   无
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_cmdCls(CLI_ENV* pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    if ( XNULL == pCliEnv || 1 > siArgc || XNULL == ppArgv )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_cmdCls() error!");
        return;
    }
    cli_cmdClear(pCliEnv);
}

/* 输入输出函数---------------------------------------------------------*/
/************************************************************************
 函数名: cli_extGetCursorX(  )
 功能:   光标横向坐标
 输入:   无
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XS16 cli_extGetCursorX( t_XOS_CMD_EDITINFO *pEdit )
{
    if ( pEdit == XNULL )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_extGetCursorX() parameter error!");
        return 0;
    }
    return pEdit->termX;
}

/************************************************************************
 函数名: cli_extGetCursorY(  )
 功能:   光标纵向坐标
 输入:   无
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XS16 cli_extGetCursorY( t_XOS_CMD_EDITINFO *pEdit )
{
    if ( pEdit == XNULL )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_extGetCursorY() parameter error!");
        return 0;
    }
    return pEdit->termY;
}

/************************************************************************
 函数名: cli_extSetCursorX(  )
 功能:   设置光标坐标
 输入:   无
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extSetCursorX(t_XOS_CMD_EDITINFO *pEdit, XS16 pos)
{
    if ( pEdit == XNULL )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_extSetCursorX() parameter error!");
    }
    pEdit->termX = pos;
}

/************************************************************************
 函数名: cli_extSetCursorY(  )
 功能:   设置光标坐标
 输入:   无
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extSetCursorY(t_XOS_CMD_EDITINFO *pEdit, XS16 pos)
{
    if ( pEdit == XNULL )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_extSetCursorY() parameter error!");
    }
    pEdit->termY = pos;
}

/************************************************************************
 函数名: cli_extCurrCoord(  )
 功能:   设置当前坐标
 输入:   无
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extCurrCoord(CLI_ENV *pCliEnv, XS16 *xPos, XS16 *yPos)
{
    XCHAR *pBuf       = MEDIT_BufPtr(pCliEnv);
    XS16  cursorPos   = MEDIT_GetCursor(pCliEnv);
    XS16  width       = (XS16)MSCRN_GetWidth(pCliEnv);
    XS16  X           = (XS16)MEDIT_PromptLen(pCliEnv);
    XS16  Y           = 0;
    
    if ( 0 >= width )
    {
        width = kCLI_DEFAULT_WIDTH;
    }
    
    while ( 0 < cursorPos-- )
    {
        switch (*(pBuf++))
        {
        case kCR:
            X = 0;
            break;
        case kLF:
            Y++;
            break;
        default:
            if (++X >= width)
            {
                Y++;
                X = 0;
            }
            break;
        }
    }
    *xPos = X;
    *yPos = Y;
    
    return;
}

/************************************************************************
 函数名: cli_extLineStart(  )
 功能:   设置光标开始坐标
 输入:   无
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extLineStart(CLI_ENV *pCliEnv)
{
    cli_extSetCursor(pCliEnv, 0);
}

/************************************************************************
 函数名: cli_extEraseLine(  )
 功能:   行擦除
 输入:   pCliEnv
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extEraseLine(CLI_ENV *pCliEnv)
{
    XCHAR *pBuf      = MEDIT_BufPtr(pCliEnv);
    XS16  lineLength = MEDIT_GetLength(pCliEnv);
    XS16  index = 0;
    
#ifdef kKEY_CONTINUE
    for (index = 0; index < lineLength; index++)
    {
        if ((kCR == pBuf[index]) || (kLF == pBuf[index]))
        {
            continue;
        }
        
        pBuf[index] = ' ';
    }
#else
    for (index = 0; index < lineLength; index++)
    {
        pBuf[index] = ' ';
    }
#endif
    
    pBuf[index] = '\0';
    
    cli_extLineStart(pCliEnv);
    cli_extWrite(pCliEnv, pBuf, index);
    
    index     = -index;
    
    cli_listMoveCursor(pCliEnv, index);
    cli_extInitCommandLine(pCliEnv);
    
    return;
}

/************************************************************************
 函数名: cli_extMoveTTYCursor(  )
 功能:   行擦除
 输入:   pCliEnv
         xPos
         yPos
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extMoveTTYCursor(CLI_ENV *pCliEnv, XS16 xPos, XS16 yPos)
{
    XCHAR   buffer[32] = {0};
    
#ifdef __DISABLE_VT_ESCAPES__
    return;
#endif
    
    if ( 0 != xPos )
    {
        if (xPos < 0)
        {
            sprintf(buffer, kCLI_VTTERM_LT, -xPos);
        }
        else if (xPos > 0)
        {
            sprintf(buffer, kCLI_VTTERM_RT, xPos);
        }
        
        cli_extPutStr(pCliEnv, buffer);
    }
    
    if ( 0 != yPos )
    {
        if (yPos < 0)
        {
            sprintf(buffer, kCLI_VTTERM_UP, -yPos);
        }
        else if (yPos > 0)
        {
            sprintf(buffer, kCLI_VTTERM_DN, yPos);
        }
        
        cli_extPutStr(pCliEnv, buffer);
    }
    
    return;
}

/************************************************************************
 函数名: cli_extMoveDOSCursor(  )
 功能:   Windows 下移动 DOS Console 光标
 输入:   pCliEnv
         xPos
         yPos
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
#ifdef XOS_WIN32

XSTATIC XVOID cli_extMoveDOSCursor(CLI_ENV *pCliEnv, XS16 xPos, XS16 yPos)
{
    CONSOLE_SCREEN_BUFFER_INFO screenBuffer;
    HANDLE                     console;
    
    if ( xPos >= MSCRN_GetWidth(pCliEnv) )
    {
        xPos--;
    }
    
    console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE == console)
    {
        return;
    }
    
    if ( 0 == GetConsoleScreenBufferInfo(console, &screenBuffer) )
    {
        return;
    }
    
    screenBuffer.dwCursorPosition.X += xPos;
    screenBuffer.dwCursorPosition.Y += yPos;
    
    SetConsoleCursorPosition( console, screenBuffer.dwCursorPosition);
    
    return;
}

#endif /* XOS_WIN32 */

/************************************************************************
 函数名: cli_extSetCursor(  )
 功能:   设置 光标
 输入:   pCliEnv
         position
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extSetCursor(CLI_ENV *pCliEnv, XS16 position)
{
    XS16  lineLength = MEDIT_GetLength(pCliEnv);
    XS16  new_X      = 0;
    XS16  new_Y      = 0;
    XS16  old_X      = 0;
    XS16  old_Y      = 0;
    t_XOS_CMD_EDITINFO *pEdit   = MEDIT_EditInfoPtr(pCliEnv);
    
    if ( XNULL == pCliEnv || 0 > position )
    {
        return;
    }
    
    /* no point moving around text not on screen */
    if ( CLI_NotEnabled(pCliEnv, kCLI_FLAG_ECHO) )
        return;
    
    cli_extCurrCoord(pCliEnv, &old_X, &old_Y);
    
    if (position < 0)
        position = 0;
    
    if (position > lineLength)
        position = lineLength;
    
    MEDIT_SetCursor(pCliEnv, position);
    
    cli_extCurrCoord(pCliEnv, &new_X, &new_Y);
    
    /* save position */
    cli_extSetCursorX(pEdit, new_X);
    cli_extSetCursorY(pEdit, new_Y);
    
    new_X -= old_X;
    new_Y -= old_Y;
    
    /* don't bother moving if it's nowhere */
    if ((0 == new_X) && (0 == new_Y))
        return;
    
    if (kCONSOLE_CONNECTION == MCONN_GetConnType(pCliEnv))
    {
#ifdef XOS_WIN32
        cli_extMoveDOSCursor(pCliEnv, new_X, new_Y);
#else
        cli_extMoveTTYCursor(pCliEnv, new_X, new_Y);
#endif
    }
    else
    {
        cli_extMoveTTYCursor(pCliEnv, new_X, new_Y);
    }
}

/************************************************************************
 函数名: cli_listMoveCursor(  )
 功能:   设置 光标
 输入:   pCliEnv
         position
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_listMoveCursor( CLI_ENV *pCliEnv, XS16 offset )
{
    XS16  cursorPos = XNULL;
    XCHAR *pBuf     = XNULL;
    
    if ( XNULL == pCliEnv )
    {
        return;
    }
    
    /* 光标偏移的未知 在 [-80, 0) , (0,+80) */
    if ( 0 == offset )
    {
        return;
    }
    
    cursorPos = MEDIT_GetCursor( pCliEnv );
    pBuf      = MEDIT_BufPtr( pCliEnv );
    
    /* no point moving around text not on screen */
    if ( CLI_NotEnabled(pCliEnv, kCLI_FLAG_ECHO) )
    {
        return;
    }
    
    cursorPos += offset;
    pBuf      += cursorPos;
    
    /* if moving to a EOL XS8 we have to scoot
    past to the other side */
    while ((kCR == *pBuf) || (kLF == *pBuf))
    {
        if (0 > offset)
        {
            cursorPos--;
            pBuf--;
        }
        else
        {
            cursorPos++;
            pBuf++;
        }
    }
    
    cli_extSetCursor(pCliEnv, cursorPos);
}

/*-----------------------------------------------------------------------*/
/************************************************************************
 函数名: cli_extResetOutput(  )
 功能:
 输入:

 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extResetOutput(CLI_ENV *pCliEnv)
{
    t_XOS_LINE_OUT      *output = MMISC_OutputPtr(pCliEnv);
    
    output->flags      = 0;
    output->lineCount  = 0;
    output->length     = 0;
    
    if (XNULL != output->pOutput)
    {
        output->pOutput[0]  = '\0';
    }
}

/************************************************************************
 函数名: cli_extMore(  )
 功能:
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extMore(CLI_ENV *pEnv)
{
    if ( XNULL == pEnv )
    {
        return;
    }
#ifdef __BSC_TASK_CLI__
    XCHAR        pBuf           = 0;
    XCHAR        moreBuf[]      = kCLI_MORE_TEXT;
    XS32         moreSize       = sizeof(moreBuf);
    WriteHandle *Writer         = MCONN_GetWriteHandle(pEnv);
    t_XOS_COM_CHAN  *pChannel   = MMISC_GetChannel(pEnv);
    t_XOS_LINE_OUT  *output     = MMISC_OutputPtr(pEnv);
    XS32       time             = OS_SPECIFIC_GET_SECS();
    
    Writer(pEnv, moreBuf, moreSize);
    cli_extResetOutput(pEnv);
    
    switch (MCONN_GetConnType(pEnv))
    {
    case kCONSOLE_CONNECTION:
        pBuf = XOS_GetChar();
        break;
    case kTELNET_CONNECTION:
        while (0 == pBuf)
        {
            OS_SPECIFIC_SOCKET_READ(pChannel->sock, &pBuf, 1);
            if (0 == OS_SPECIFIC_GET_SECS() - time)
                pBuf = 0;
        }
        break;
    }
    /* clear more message */
    Writer(pEnv, "\r", 1);
    XOS_MemSet(moreBuf, ' ', moreSize);
    Writer(pEnv, moreBuf, moreSize);
    Writer(pEnv, "\r", 1);
    
#ifdef kCLI_PRINT_CANCEL
    if (kCLI_PRINT_CANCEL == TOUPPER(pBuf))
        SET_PRINT_FLAG(output, kPRINT_FLAG_NOPRINT);
#endif
    
#endif
}
/************************************************************************
 函数名: cli_extUpdate(  )
 功能:
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XS32 cli_extUpdate(CLI_ENV *pCliEnv, const XCHAR *pBuf, XS32 length, XCHAR *more)
{
    XS32        index    = 0;
    XCHAR       hardWrap = (XCHAR) CLI_IsEnabled(pCliEnv, kCLI_FLAG_HARDWRAP);
    t_XOS_CMD_EDITINFO  *pEdit    = MEDIT_EditInfoPtr(pCliEnv);
    t_XOS_LINE_OUT      *pOutput  = MMISC_OutputPtr(pCliEnv);
    XS16      xPos     = cli_extGetCursorX(pEdit);
    XS16      yPos     = cli_extGetCursorY(pEdit);
    XS16      maxCol   = MSCRN_GetWidth(pCliEnv);
    
#ifndef __DISABLE_PAGED_OUTPUT__
    XS16      maxRow   = MSCRN_GetHeight(pCliEnv);
#endif
    
    if ( XNULL == pCliEnv || XNULL == pBuf || 0 > length || XNULL == more )
    {
        return XERROR;
        
    }
    
    *more = XFALSE;
    
    while ( length-- > 0 )
    {
        if (GET_PRINT_FLAG(pOutput, kPRINT_FLAG_NOPRINT))
            return -1;
        
        index++;
        pEdit->cursorPos++;
        
        switch (*pBuf)
        {
        case kCR:
            xPos = 0;
            break;
        case kLF:
            yPos++;
            pOutput->lineCount++;
            break;
        default:
            xPos++;
            break;
        }
        
        if (maxCol == xPos)
        {
            if (hardWrap)
            {
                cli_extPut(pCliEnv, kEOL, kEOL_SIZE);
            }
            xPos = 0;
            yPos++;
            pOutput->lineCount++;
        }
        
        cli_extSetCursorX(pEdit, xPos);
        cli_extSetCursorY(pEdit, yPos);
        
#ifndef __DISABLE_PAGED_OUTPUT__
        if (CLI_IsEnabled(pCliEnv, kCLI_FLAG_MORE))
        {
            if (pOutput->lineCount >= maxRow - 1)
            {
                *more = XTRUE;
                break;
            }
        }
#endif /* __DISABLE_PAGED_OUTPUT__ */
        
        pBuf++;
    }
    
    return index;
}

/************************************************************************
 函数名: cli_extWrite(  )
 功能:
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XS32 cli_extWrite(CLI_ENV *pCliEnv, const XCHAR *pBuf, XS32 bufLen)
{
    XS32         status   = XSUCC;
    XS32         outSize  = 0;
    XCHAR        more     = XFALSE;
    WriteHandle  *Writer  = XNULL;
    
    if ( XNULL == pCliEnv )
    {
        return XERROR;
    }
    
    Writer  = MCONN_GetWriteHandle(pCliEnv);
    
    /* this should be an assert eventually */
    if ( 0 > bufLen || XNULL == pBuf || XNULL == Writer )
    {
        return CLI_ERROR;
    }
    
    /* skip it all if echo disabled */
    if (   CLI_IsEnabled(pCliEnv, kCLI_FLAG_INPUT)
        && CLI_NotEnabled(pCliEnv, kCLI_FLAG_ECHO))
    {
        return XSUCC;
    }
    
    XOS_MutexLock( &g_mutexCliPrt);
    while ( XTRUE )
    {
        outSize = cli_extUpdate(pCliEnv, pBuf, bufLen, &more);
        if (0 > outSize)
        {
            break;
        }
        
        status  = Writer( pCliEnv, pBuf, outSize );
        
#ifndef __DISABLE_PAGED_OUTPUT__
        if (more)
            cli_extMore(pCliEnv);
#endif
        
        bufLen -= outSize;
        
        if ( (0 >= bufLen)
            || (status != XSUCC)
            || (0 > outSize) )
        {
            break;
        }
    }
    
    XOS_MutexUnlock( &g_mutexCliPrt);
    return status;
}

/************************************************************************
 函数名: cli_extPut(  )
 功能:   Similar to cli_extWrite but doesn't update buffer
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XS32 cli_extPut(CLI_ENV *pCliEnv, const XCHAR *pBuf, XS32 bufLen)
{
    WriteHandle  *Writer = XNULL;
    if ( XNULL == pCliEnv || XNULL == pBuf || 0 > bufLen )
    {
        return XERROR;
    }
    
    Writer = MCONN_GetWriteHandle(pCliEnv);
    if ( XNULL == Writer )
    {
        return XERROR;
    }
    
    /* this should be an assert eventually */
    if (XNULL == pBuf)
    {
        return CLI_ERROR;
    }
    
    return Writer( pCliEnv, pBuf, bufLen );
}

/************************************************************************
 函数名: cli_extPutStr(  )
 功能:   Similar to XOS_CliExtWriteStr but doesn't update buffer
 输入:   pBuf 字符创输入指针
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XS32 cli_extPutStr(CLI_ENV *pCliEnv, const XCHAR *pBuf)
{
    /* this should be an assert eventually */
    if (XNULL == pBuf)
    {
        return CLI_ERROR;
    }
    
    return cli_extPut( pCliEnv, pBuf, (XS32)XOS_StrLen(pBuf) );
}

/************************************************************************
 函数名: XOS_CliExtWriteStr(  )
 功能:   字符串输出函数
 输入:   pBuf 字符创输入指针
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XS32 XOS_CliExtWriteStr(CLI_ENV *pCliEnv, const XCHAR *pBuf)
{
    /* this should be an assert eventually */
    if ( XNULL == pBuf || XNULL == pCliEnv )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "XOS_CliExtWriteStr() parameters error!");
        return XERROR;
    }
    
    return cli_extWrite( pCliEnv, pBuf, (XS32)XOS_StrLen(pBuf) );
}

/************************************************************************
 函数名: XOS_CliExtWriteStrLine(  )
 功能:   字符串输出函数
 输入:   pBuf 字符创输入指针
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XS32 XOS_CliExtWriteStrLine(CLI_ENV *pCliEnv, const XCHAR *pBuf)
{
    XS32 status = XSUCC;
    
    if ( XNULL == pBuf || XNULL == pCliEnv )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "XOS_CliExtWriteStrLine() parameters error!");
        return XERROR;
    }
    
    if ( XSUCC == (status = XOS_CliExtWriteStr(pCliEnv, pBuf)) )
    {
        return cli_extWrite( pCliEnv, kEOL, kEOL_SIZE );
    }
    
    return status;
}

/************************************************************************
 函数名: XOS_CliExtPrintf(  )
 功能:   带格式的输出函数
 输入:   pCliEnv
         pFmt 格式话字符串
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XS32 XOS_CliExtPrintf( CLI_ENV* pCliEnv, const XCHAR* pFmt, ...)
{
    //XCHAR *buf = XNULLP;
    XCHAR buf[MAX_BUFF_LEN+1] = {0};
    va_list ap;
    XS32 state = 0;
    XS32 fmt_len=0;
    
    if ( XNULL == pFmt)// || XNULL == pCliEnv )
    {
        return XERROR;
    }
    #if 0
    if(XNULL == (buf = XOS_MemMalloc(FID_CLI, MAX_BUFF_LEN+1)) )
    {
        return XERROR;
    }
    #endif
    va_start(ap,pFmt);
    fmt_len=XOS_VsPrintf(buf, MAX_BUFF_LEN+1, pFmt, ap);
    va_end(ap);
    if(fmt_len < 0)
    {
        //XOS_MemFree(FID_CLI, buf);
        return XERROR;
    }
    buf[MAX_BUFF_LEN] = '\0';
    if(pCliEnv != XNULLP)
    {
        state = cli_extWrite( pCliEnv, buf, (XS32)XOS_StrLen(buf) );
    }
    else
    {
        //printf("%s",buf);
    }
    #if 0
    if(XERROR == XOS_MemFree(FID_CLI, buf))
    {
        return XERROR;
    }
    #endif
    return state;
}

XS32 XOS_CliInforPrintfStr(const XCHAR* buff)
{
    XS32           index       = 0;
    CLI_ENV        *pCliDest   = NULL;
    t_XOS_COM_CHAN *pChannel   = NULL;
    XBOOL          bRirectFlag = XFALSE;
    
    if ( XNULL == buff)
    {
        return XERROR;
    }
    
    /*根据开关打印信息重定向到Telnet 客户端*/
    /* 1 ~ kCLI_MAX_CLI_TASK 是 Telnet 客户端*/
    for ( index = 0; index < kCLI_MAX_CLI_TASK; index++ )
    {
        if( 0 < g_paUserSIDLogined[index].xs32SID
            && g_paUserSIDLogined[index].xbSwitch  )
        {
            pCliDest = cli_telnetGetCliSession( g_paUserSIDLogined[index].xs32SID );
            if ( XNULL != pCliDest )
            {
                pChannel = MMISC_GetChannel( pCliDest );
                if (   XNULL != pChannel
                    && kThreadWorking == pChannel->ThreadState
                    && kTELNET_USR_PROCESS == MMISC_GetLoginStat( pCliDest ))
                {
                    if ( XSUCC != cli_extPutStr( pCliDest, buff ) )
                    {
                        return XERROR;
                    }

                    bRirectFlag = XTRUE;
                }
            }
        }
    }

    if ( !bRirectFlag )
    {
        cli_pciConsoleWrite( XNULL, buff, (XS32)XOS_StrLen(buff) );
        return XSUCC;
    }

    return XERROR;

}

/************************************************************************
 函数名: XOS_CliInforPrintfSessionStr(  )
 功能:   字符串输出到telnet终端
 输入:   pFidLevel:所有telnet终端的打印级别数组起始地址
         elevel 打印级别
         buff :打印内容
 输出:   无
 返回:   XSUCC 成功；XERROR 失败
 说明:   无
************************************************************************/
XPUBLIC XS32  XOS_CliInforPrintfSessionStr(const e_PRINTLEVEL *pFidLevel,e_PRINTLEVEL elevel,const XCHAR* buff )
{
    XS32           index       = 0;
    CLI_ENV        *pCliDest   = NULL;
    t_XOS_COM_CHAN *pChannel   = NULL;
    XBOOL          bRirectFlag = XFALSE;
    
    if ( XNULL == pFidLevel || XNULL == buff)
    {
        return XERROR;
    }
    
    /*根据开关打印信息重定向到Telnet 客户端*/
    /* 1 ~ kCLI_MAX_CLI_TASK 是 Telnet 客户端*/
    for ( index = 0; index < kCLI_MAX_CLI_TASK; index++ )
    {
        if( 0 < g_paUserSIDLogined[index].xs32SID
            && g_paUserSIDLogined[index].xbSwitch  
            && elevel >= pFidLevel[g_paUserSIDLogined[index].xs32SID])
        {
            pCliDest = cli_telnetGetCliSession( g_paUserSIDLogined[index].xs32SID );
            if ( XNULL != pCliDest )
            {
                pChannel = MMISC_GetChannel( pCliDest );
                if (   XNULL != pChannel
                    && kThreadWorking == pChannel->ThreadState
                    && kTELNET_USR_PROCESS == MMISC_GetLoginStat( pCliDest ))
                {
                    if ( XSUCC != cli_extPutStr( pCliDest, buff ) )
                    {
                        return XERROR;
                    }

                    bRirectFlag = XTRUE;
                }
            }
        }
    }

    if ( !bRirectFlag )
    {
#if 0
        cli_pciConsoleWrite( XNULL, buff, (XS32)XOS_StrLen(buff) );
#endif
        return XSUCC;
    }

    return XERROR;

}
/************************************************************************
 函数名: XOS_CliInforPrintf(  )
 功能:   带格式的输出函数,输出信息可以重定向
 输入:   pFmt 格式话字符串
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XS32 XOS_CliInforPrintf( const XCHAR* pFmt, ... )
{
#if 0
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
    return XOS_CliInforPrintfStr(buf);
#endif
    return XSUCC;
}

/************************************************************************
 函数名: cli_extPrintString(  )
 功能:   字符输出函数
 输入:   字符串
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extPrintString( CLI_ENV *pCliEnv, XCHAR *pString, XS32 length, XCHAR fill )
{
    XS32  limit = 0, size = 0;
    
    if ( XNULL == pCliEnv || XNULL == pString || 0 > length )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "cli_extPrintString() parameter error!" );
    }
    
    if (0 == length)
    {
        if( XSUCC != XOS_CliExtWriteStr(pCliEnv, pString) )
        {
            return;
        }
        return;
    }
    
    size  = (XS32) XOS_StrLen(pString);
    limit = CLI_MIN(length, size);
    if( XSUCC != cli_extWrite(pCliEnv, pString, limit))
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "XSUCC != cli_extWrite() error!");
    }
    
    limit = length - limit;
    while (0 < limit--)
    {
        cli_extWrite(pCliEnv, &fill, 1);
    }
    
    return;
}

/************************************************************************
 函数名: cli_extInsertText(  )
 功能:   Line editing functions
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extInsertText(CLI_ENV *pCliEnv, XCHAR *pText, XS16 length)
{
    XS16    offset = 0;
    XS16    cursorPos   = MEDIT_GetCursor(pCliEnv);
    XS16    lineLength  = MEDIT_GetLength(pCliEnv);
    XCHAR   *pBuf       = MEDIT_BufPtr(pCliEnv);
    XS32    index = 0;
    
    if ( XNULL == pCliEnv || XNULL == pText || 0 > length )
    {
        return;
    }
    
    if ( CLI_MAX_INPUT_LEN <= (lineLength + length) )
    {
        return;
    }
    if ( XNULL == pBuf )
    {
        return;
    }
    
    /* are we just appending text? */
    if (cursorPos == lineLength)
    {
        pBuf       += lineLength;
        cursorPos  += length;
        
        if ( CLI_MAX_INPUT_LEN >= length )
        {
            XOS_StrNcpy( pBuf, pText, length );
        }
        
        cli_extWrite(pCliEnv, pText, length);
        
        MEDIT_SetCursor(pCliEnv, cursorPos);
        MEDIT_SetLength(pCliEnv, cursorPos);
        
        return;
    }
    
    /* inserting in the middle... */
    for (index = lineLength; index >= cursorPos; index--)
    {
        pBuf[index + length] = pBuf[index];
    }
    
    pBuf += cursorPos;
    
    if ( CLI_MAX_INPUT_LEN >= length )
    {
        XOS_StrNcpy( pBuf, pText, length );
    }
    
    XOS_CliExtWriteStr(pCliEnv, pBuf);
    
    lineLength += length;
    offset      = (XS16)(cursorPos - lineLength + length);
    
    MEDIT_SetCursor(pCliEnv, lineLength);
    MEDIT_SetLength(pCliEnv, lineLength);
    cli_listMoveCursor(pCliEnv, offset);
    
    return;
}

/************************************************************************
 函数名: cli_extRefresh(  )
 功能:
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extRefresh(CLI_ENV *pCliEnv, XS16 offset, XS16 length)
{
    XCHAR     *pBuf       = MEDIT_BufPtr(pCliEnv);
    XS16   cursorPos = 0;
    
    if (0 > length)
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_extRefresh() 0 > length!");
        return;
    }
    
    cli_listMoveCursor(pCliEnv, offset);
    cursorPos = MEDIT_GetCursor(pCliEnv);
    pBuf += cursorPos;
    
    cli_extWrite(pCliEnv, pBuf, length);
    length = -length;
    cli_listMoveCursor(pCliEnv, length);
}

/************************************************************************
 函数名: cli_extDeleteText(  )
 功能:
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extDeleteText( CLI_ENV *pCliEnv, XS16 start, XS16 length )
{
    XCHAR  *pBuf        = MEDIT_BufPtr(pCliEnv);
    XS16   lineLength   = MEDIT_GetLength(pCliEnv);
    XS16   index        = 0;
    XS16   remainder    = 0;
    XS16   updated[kCLI_DEFAULT_HEIGHT] = {0};
    XS16   lineWidth    = 0;
    XS16   lineCount    = 0;
    XCHAR  linesDeleted = XFALSE;
    XCHAR  *pTemp       = XNULL;
    
    if (0 >= length)
    {
        return;
    }
    
    if (0 == XOS_StrNcmp(kEOL, &pBuf[start + 1], kEOL_SIZE))
    {
        if (1 < kEOL_SIZE)
        {
            length++;
        }
        
        start++;
    }
    
    /* delete mid string */
    if ( (start + length) < lineLength)
    {
        
        /* if shifting lines up we need to know what to erase */
        for (index = start; index < (start + length); index++)
        {
            if (kCR == pBuf[index])
            {
                linesDeleted = XTRUE;
                break;
            }
        }
        
        if (linesDeleted)
        {
            pTemp = &pBuf[start];
            for (index = start; index < lineLength; index++, pTemp++)
            {
                if (kCR == *pTemp)
                {
                    updated[lineCount] = lineWidth;
                    lineCount++;
                    lineWidth = 0;
                }
                if ((kCR != *pTemp) && (kLF != *pTemp))
                {
                    lineWidth++;
                }
            }
            if (0 < lineCount)
            {
                updated[lineCount] = lineWidth;
            }
            
            /* erase extra lines */
            for (index = 0; index <= lineCount; index++)
            {
                cli_extPrintString(pCliEnv, "", updated[index], ' ');
                if (index < lineCount)
                {
                    XOS_CliExtWriteStrLine(pCliEnv, "");
                }
            }
            cli_extSetCursor(pCliEnv, start);
        }
        
        /* shift remaining text left */
        for (index = start + length; index < lineLength; index++)
        {
            pBuf[index - length] = pBuf[index];
            pBuf[index] = ' ';
        }
        
        /* erase rest of line */
        for (index = lineLength - length; index < lineLength; index++)
        {
            pBuf[index] = ' ';
        }
    }
    else /* end of string */
    {
        for (index = start; index < lineLength; index++)
        {
            pBuf[index] = ' ';
        }
    }
    
    /* display revised text */
    remainder         = lineLength - start;
    start            -= MEDIT_GetCursor(pCliEnv);
    
    cli_extRefresh(pCliEnv, start, remainder);
    
    lineLength       -= length;
    pBuf[lineLength]  = kCHAR_NULL;
    
    MEDIT_SetLength(pCliEnv, lineLength);
}

/************************************************************************
 函数名: cli_extDeleteChar(  )
 功能:
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extDeleteChar(CLI_ENV *pCliEnv)
{
    XS16 lineLength = MEDIT_GetLength(pCliEnv);
    XS16 cursorPos  = MEDIT_GetCursor(pCliEnv);
    
    if (lineLength <= 0)
    {
        return;
    }
    
    /* trying to delete past end of line? */
    if (cursorPos >= lineLength)
    {
        return;
    }
    
    cli_extDeleteText(pCliEnv, cursorPos, 1);
}

/************************************************************************
 函数名: cli_extInitCommandLine(  )
 功能:
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extInitCommandLine(CLI_ENV *pCliEnv)
{
    XCHAR    *pBuf = MEDIT_BufPtr(pCliEnv);
    
    MEDIT_SetLength(pCliEnv, 0);
    MEDIT_SetCursor(pCliEnv, 0);
    XOS_MemSet( pBuf, kCHAR_NULL, CLI_MAX_INPUT_LEN + 1 );
}

/************************************************************************
 函数名: cli_extBackspace(  )
 功能:
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_extBackspace(CLI_ENV *pCliEnv)
{
    XCHAR    *pBuf            = MEDIT_BufPtr(pCliEnv);
    XS16     cursorPos        = MEDIT_GetCursor(pCliEnv);
    XS16     lineLength       = MEDIT_GetLength(pCliEnv);
    t_XOS_CMD_EDITINFO *pEdit = MEDIT_EditInfoPtr(pCliEnv);
    XS16     xPos             = cli_extGetCursorX(pEdit);
    
    if (0 >= cursorPos)
    {
        return;
    }
    
    /* just remove from buffer w/o screen update */
    if ( (CLI_NotEnabled(pCliEnv, kCLI_FLAG_ECHO)) &&
        (cursorPos == lineLength))
    {
        MEDIT_SetCursor(pCliEnv, --cursorPos);
        MEDIT_SetLength(pCliEnv, --lineLength);
        pBuf[lineLength] = kCHAR_NULL;
        
        return;
    }
    
    if (   (kTELNET_CONNECTION == MCONN_GetConnType(pCliEnv))
        && (cursorPos == lineLength) )
    {
        /* telnet is dumb and won't go to previous line */
        if ( 0 < xPos-- )
        {
            cli_extSetCursorX(pEdit, xPos);
            cli_extPutStr(pCliEnv, kCLI_TELNET_BACKSPACE);
            MEDIT_SetCursor(pCliEnv, --cursorPos);
            MEDIT_SetLength(pCliEnv, --lineLength);
            pBuf[lineLength] = kCHAR_NULL;
            
            return;
        }
        
#ifdef __DISABLE_VT_ESCAPES__
        return;
#endif
        
    }
    cli_listMoveCursor(pCliEnv, -1);
    cli_extDeleteChar(pCliEnv);
    
    return;
}
/************************************************************************
 函数名: cli_cmdDosClear(  )
 功能:   清屏
 输入:
 输出:
 返回:
 说明:
************************************************************************/
#ifdef XOS_WIN32
/* clear DOS screen */
XSTATIC XVOID cli_cmdDosClear(CLI_ENV *pCliEnv)
{
    CONSOLE_SCREEN_BUFFER_INFO lpScreenBufferInfo;
    DWORD                      nLength;
    COORD                      dwWriteCoord;
    DWORD                      NumberOfCharsWritten;
    
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if ( XNULL == pCliEnv )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "cli_cmdDosClear() XNULL == pCliEnv!");
        return;
    }
    if (INVALID_HANDLE_VALUE == console)
    {
        return;
    }
    
    if ( 0 == GetConsoleScreenBufferInfo(console, &lpScreenBufferInfo) )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "GetConsoleScreenBufferInfo() == 0!");
        return;
    }
    
    dwWriteCoord.X = 0;
    dwWriteCoord.Y = 0;
    nLength = (DWORD)(lpScreenBufferInfo.dwSize.X * lpScreenBufferInfo.dwSize.Y);
    if (! FillConsoleOutputCharacter(
        console,                /* handle to screen buffer*/
        ' ',                    /* character*/
        nLength,                /* number of cells*/
        dwWriteCoord,           /* first coordinates*/
        &NumberOfCharsWritten   /* number of cells written*/
        ))
        return;
    
    SetConsoleCursorPosition( console, dwWriteCoord );
}
#endif /* __WIN32_OS__ */

/************************************************************************
 函数名: cli_cmdClear(  )
 功能:   clear screen
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XS32 cli_cmdClear( CLI_ENV *pCliEnv )
{
    if ( XNULL == pCliEnv )
    {
        return XERROR;
    }
    
    if ( kTELNET_CONNECTION == MCONN_GetConnType(pCliEnv) )
    {
        XOS_CliExtWriteStr( pCliEnv, kCLI_TELNET_CLEAR );
    }
    else
    {
        /* Console */
#ifdef XOS_WIN32
        cli_cmdDosClear( pCliEnv );
#else
        /* assume that non-Windows console is really telnet */
        XOS_CliExtWriteStr( pCliEnv, kCLI_TELNET_CLEAR );
#endif /* __WIN32_OS__ */
    }
    return XSUCC;
}

/************************************************************************
 函数名: cli_histResetHistInfo(  )
 功能:   重置历史记录相关
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XSTATIC XS32 cli_histResetHistInfo(CLI_ENV *pCliEnv)
{
    t_XOS_HIST_INFO *history = XNULL;
    XU32            histSize = kCLI_HISTORY_BUFFER_SIZE * sizeof(t_XOS_CMD_HIST_BUFF);
    
    if ( 1 > histSize )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "cli_histInitHistInfo() 1 > histSize!" );
        return XERROR;
    }
    
    if ( XNULL == pCliEnv )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "cli_histInitHistInfo() XNULL == pCliEnv!" );
        return XERROR;
    }
    
    history = MHIST_History(pCliEnv);
    
    history->iMaxHistCmds = kCLI_HISTORY_BUFFER_SIZE;
    history->iCurHistCmd  = 0;
    history->iNumCmds     = 0;
    history->bufferIndex  = 0;
    if(history->pHistBuff)
    {
        XOS_MemSet( history->pHistBuff, 0x00, histSize );
        XOS_MemSet( history->pHistBuff->histCmd, 0x00, CLI_MAX_INPUT_LEN + 1 );
    }
    
    return XSUCC;
}
/************************************************************************
 函数名: cli_histInitHistInfo(  )
 功能:   历史记录相关
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XSTATIC XS32 cli_histInitHistInfo(CLI_ENV *pCliEnv)
{
    t_XOS_HIST_INFO *history = XNULL;
    XU32            histSize = kCLI_HISTORY_BUFFER_SIZE * sizeof(t_XOS_CMD_HIST_BUFF);
    
    if ( 1 > histSize )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "cli_histInitHistInfo() 1 > histSize!" );
        return XERROR;
    }
    
    if ( XNULL == pCliEnv )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "cli_histInitHistInfo() XNULL == pCliEnv!" );
        return XERROR;
    }
    
    history = MHIST_History(pCliEnv);
    
    history->iMaxHistCmds = kCLI_HISTORY_BUFFER_SIZE;
    history->iCurHistCmd  = 0;
    history->iNumCmds     = 0;
    history->bufferIndex  = 0;
    
    history->pHistBuff = (t_XOS_CMD_HIST_BUFF *)XOS_MemMalloc( FID_CLI, histSize);
    if ( XNULL == history->pHistBuff )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "cli_histInitHistInfo() XOS_MemMalloc( FID_CLI, histSize) error!" );
        return ERROR_MEMMGR_NO_MEMORY;
    }
    XOS_MemSet( history->pHistBuff, 0x00, histSize );
    XOS_MemSet( history->pHistBuff->histCmd, 0x00, CLI_MAX_INPUT_LEN + 1 );
    
    return XSUCC;
}

/************************************************************************
 函数名: cli_histAddHistLine(  )
 功能:   添加历史记录相关
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XSTATIC XVOID cli_histAddHistLine (CLI_ENV *pCliEnv)
{
    XCHAR       *pSrc  = XNULL;
    XCHAR       *pDest = XNULL;
    t_XOS_HIST_INFO    *pHistory;
    
    if ( XNULL == pCliEnv )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "cli_histAddHistLine() XNULL == pCliEnv!" );
        return;
    }
    
    pHistory = MHIST_History(pCliEnv);
    pSrc     = MEDIT_BufPtr(pCliEnv);
    
    if ( 0 < pHistory->iNumCmds )
    {
        if ( HIST_BUFFER_FULL(pHistory) )
        {
            pHistory->bufferIndex = 0;
        }
        else
        {
            pHistory->bufferIndex++;
        }
    }
    
    pDest = HistBuffPtr( pHistory, pHistory->bufferIndex );
    if ( XNULL != pDest && XNULL != pSrc )
    {
        XOS_StrNcpy( pDest, pSrc, CLI_MAX_INPUT_LEN );
        pHistory->iCurHistCmd = (++(pHistory->iNumCmds)) + 1;
        
#ifdef CLI_LOG
        cli_log_write( eCLIDebug, "insert history cmd: %s!", pDest );
#endif /* end  CLI_LOG  */
        
    }
}

/************************************************************************
 函数名: cli_histHist2Buff(  )
 功能:   查找历史
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XS32 cli_histHist2Buff( t_XOS_HIST_INFO *pHistory, XS32 index )
{
    XS32 offset = 0;
    XS32 least  = 0;
    
    if ( XNULL == pHistory )
    {
        return XERROR;
    }
    
    /* empty buffer? */
    if ( 0 >= pHistory->iNumCmds )
    {
        XOS_Trace( MD(FID_CLI, PL_WARN),"cli_histHist2Buff() 0 >= pHistory->iNumCmds,no history cmd");
        return XERROR;
    }
    
    least = LEAST_RECENT_HIST(pHistory);
    
    /* out of range? */
    if ((least > index) || (index > pHistory->iNumCmds))
    {
        return XERROR;
    }
    
    /* buffer not wrapped yet? */
    if (pHistory->iNumCmds < pHistory->iMaxHistCmds)
    {
        return (index - 1);
    }
    
    offset = pHistory->bufferIndex - (pHistory->iNumCmds - index);
    if (0 > offset)
    {
        offset += pHistory->iMaxHistCmds;
    }
    
    return offset;
}

/************************************************************************
 函数名: cli_getHistoryCmd(  )
 功能:   历史
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XCHAR * cli_getHistoryCmd(t_XOS_HIST_INFO *pHistory)
{
    XCHAR       *pBuf;
    XS32       offset;
    t_XOS_CMD_HIST_BUFF *histCmdBuff;
    XCHAR       *pTemp = MHIST_TempBuf(pHistory);
    
    if (pHistory->iCurHistCmd > pHistory->iNumCmds)
    {
        return pTemp;
    }
    
    offset = cli_histHist2Buff(pHistory, pHistory->iCurHistCmd);
    if (0 > offset)
    {
        return XNULL;
    }
    
    histCmdBuff = &(pHistory->pHistBuff[offset]);
    pBuf = histCmdBuff->histCmd;
    return pBuf;
}
/************************************************************************
 函数名: cli_changeHistory(  )
 功能:   历史
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XSTATIC XVOID cli_changeHistory(t_XOS_HIST_INFO *pHistory, XS32  offset)
{
    XS32 newHistory = 0;
    
    if ( XNULL == pHistory )
    {
        return;
    }
    
    newHistory = pHistory->iCurHistCmd + offset;
    
    if ( (1 > newHistory) || (newHistory > (pHistory->iNumCmds + 1)) )
    {
        return;
    }
    
    if (newHistory < LEAST_RECENT_HIST(pHistory))
    {
        return;
    }
    
    pHistory->iCurHistCmd = newHistory;
}

/************************************************************************
 函数名: cli_histScroll(  )
 功能:
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XSTATIC XVOID cli_histScroll(CLI_ENV *pCliEnv, XS32 offset)
{
    t_XOS_HIST_INFO  *pHistory     = MHIST_History(pCliEnv);
    XCHAR            *pBuf         = MEDIT_BufPtr(pCliEnv);
    XCHAR            *pTemp        = MHIST_TempBuf(pHistory);
    XCHAR            *pHistoryCmd  = XNULL;
    
    if ( XNULL == pHistory )
    {
        return;
    }
    
    if ( pHistory->iCurHistCmd > pHistory->iNumCmds )
    {
        XOS_StrNcpy( pTemp, pBuf, CLI_MAX_INPUT_LEN );
    }
    
    cli_changeHistory(pHistory, offset);
    pHistoryCmd = cli_getHistoryCmd( pHistory );
    if ( XNULL == pHistoryCmd )
    {
        return;
    }
    
    cli_extEraseLine( pCliEnv );
    /*RCC_DB_SetCommand(pCliEnv, pHistoryCmd);*/
    
    XOS_CliExtWriteStr( pCliEnv, pHistoryCmd );
    
    XOS_StrNcpy( pBuf, pHistoryCmd, CLI_MAX_INPUT_LEN );
    MEDIT_SetCursor( pCliEnv, (XS16)XOS_StrLen(pHistoryCmd) );
    MEDIT_SetLength( pCliEnv, (XS16)XOS_StrLen(pHistoryCmd) );
    cli_listMoveCursor( pCliEnv, (XS16) XOS_StrLen(pHistoryCmd) );
}

/************************************************************************
 函数名: cli_listConstruct(  )
 功能:
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XSTATIC t_XOS_CLI_LIST *cli_listConstruct(XVOID)
{
    t_XOS_CLI_LIST *pTemp = (t_XOS_CLI_LIST *)XOS_MemMalloc(FID_CLI, sizeof(t_XOS_CLI_LIST));
    if (XNULL == pTemp)
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "XOS_MemMalloc(FID_CLI, sizeof(t_XOS_CLI_LIST)) error!" );
        return XNULL;
    }
    
    XOS_MemSet( pTemp, 0x00, sizeof(t_XOS_CLI_LIST) );
    
    pTemp->pObject  = XNULL;
    pTemp->pNext    = XNULL;
    
    return pTemp;
}

XSTATIC void cli_extTab(CLI_ENV *pCliEnv)
{
    XS32 i = 0;
    XCHAR cache[ 2*CLI_MAX_ARG_LEN + 1 ]     = {0};
    
    XS32 mached[64] = {0};
    t_XOS_CMD_DATAS* pCmdLst = XNULL;
    XS32 end = 0;
    XS32 bufLen = 0;
    XS32 matchedCount = 0;
    XCHAR* input = XNULL;
    XBOOL isMatched = XFALSE;
    
    pCmdLst = CLIENV(pCliEnv)->cliShellCmd.pCmdLst;
    input = MEDIT_BufPtr(pCliEnv);
    bufLen = (XS32)XOS_StrLen(input);
    strncpy(cache, input, bufLen);

    // count matched item
    for (i=0; XOS_StrLen(pCmdLst[i].pscCmdName) > 0; ++i)
    {
        if ( MMISC_GetAccess(pCliEnv) > pCmdLst[i].eAccLvl )
        {
            continue;
        }
        
        if (XOS_StrNcmp(pCmdLst[i].pscCmdName, input, bufLen) == 0)
        {
            mached[matchedCount++] = i;
        }
    }

    if (matchedCount == 0)
    {
        if (pCliEnv == cli_telnetGetCliSession(0))
        {
            CLI_dealInput(kCR);
            cli_extReadCh(pCliEnv, (XU8*)cache, (XU16)bufLen);
        }
    }
    else if (matchedCount == 1)
    {
        cli_extReadCh(pCliEnv, (XU8*)(pCmdLst[mached[0]].pscCmdName + bufLen), 
            (XU16)(XOS_StrLen(pCmdLst[mached[0]].pscCmdName) - bufLen));
    }
    else
    {
        end = bufLen-1;
        isMatched = XTRUE;

        while(isMatched)
        {
            ++end;
            for (i=1; i<matchedCount; ++i)
            {
                if (pCmdLst[mached[i]].pscCmdName[end] == 0 ||
                    pCmdLst[mached[i]].pscCmdName[end] != pCmdLst[mached[0]].pscCmdName[end])
                {
                    isMatched = XFALSE;
                    break;
                }
            }
        }

        if (end != bufLen)
        {
            cli_extReadCh(pCliEnv, (XU8*)(pCmdLst[mached[0]].pscCmdName + bufLen), 
                (XU16)(end - bufLen));
        }
        else if (pCliEnv == cli_telnetGetCliSession(0))
        {
            CLI_dealInput(kCR);
            cli_extReadCh(pCliEnv, (XU8*)cache, (XU16)bufLen);
        }
    }
}
/************************************************************************
 函数名: cli_extReadCh(  )
 功能:  Read one XS8 and process
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XSTATIC XS32 cli_extReadCh(CLI_ENV *pCliEnv, XU8* pOrgCh, XU16 orgLen)
{
    e_XOS_KEY_STATE keyState;
    XS32        idx         = 0;
    XS32        bytesRead   = 0;
    XS32        status      = XSUCC;
    XCHAR       loop        = XTRUE;
    XU8         charIn      = 0;
    ReadHandle  *Reader     = XNULL;
    
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_INPUT);
    
    /*cli_extResetOutput(pCliEnv);*/
    
    if ( CLI_IsEnabled(pCliEnv, kCLI_FLAG_KILL) )
    {
        status = STATUS_CLI_KILL;
        loop   = XFALSE;
        MMISC_SetInput(pCliEnv, 0);
        
        return ERROR_CLI_WRONG_MODE;
    }
    
    /*TELNET 字符处理*/
    Reader      = MCONN_GetReadHandle(pCliEnv);
    while( idx < orgLen)
    {
        if ( XNULL != Reader )
        {
            Reader( pCliEnv, pOrgCh[idx], (XCHAR *)&charIn, &bytesRead );
        }
        
        if(bytesRead == 0)
        {
            idx++;
            continue;
        }
        
        keyState = MEDIT_GetKeyStat(pCliEnv);
        /* pre-process escape codes */
        switch (keyState)
        {
        case KEY_STATE_ESC:
            switch ( TOLOWER(charIn) )
            {
            case kCLI_ESC_CURSOR:
                MEDIT_SetKeyStat(pCliEnv, KEY_STATE_CURSOR);
                idx++;
                continue;
            case kCLI_WORD_PREV:
                charIn = (XS8)kKEY_WORD_PREV;
                break;
            case kCLI_WORD_NEXT:
                charIn = (XS8)kKEY_WORD_NEXT;
                break;
            case kCLI_WORD_UPPERCASE_TO_END:
                charIn = (XS8)kKEY_UPPERCASE;
                break;
            case kCLI_WORD_DELETE_TO_END:
                charIn = (XS8)kKEY_DELETE_WORD_END;
                break;
            case kCLI_WORD_LOWERCASE_TO_END:
                charIn = (XS8)kKEY_LOWERCASE;
                break;
            }
            MEDIT_SetKeyStat(pCliEnv, KEY_STATE_DATA);
            break;
            
            case KEY_STATE_CURSOR:
                {
                    switch(charIn)
                    {
                    case kCLI_CURSOR_UP:
                        charIn = kKEY_MOVE_UP;
                        break;
                    case kCLI_CURSOR_DOWN:
                        charIn = kKEY_MOVE_DOWN;
                        break;
                    case kCLI_CURSOR_LEFT:
                        charIn = kKEY_MOVE_LEFT;
                        break;
                    case kCLI_CURSOR_RIGHT:
                        charIn = kKEY_MOVE_RIGHT;
                        break;
                    default:
                        break;
                    }
                    MEDIT_SetKeyStat(pCliEnv, KEY_STATE_DATA);
                    break;
                }
                
            default:
                break;
                
        }
        
        switch ( charIn )
        {
        case kESC:
            {
                MEDIT_SetKeyStat(pCliEnv, KEY_STATE_ESC);
                break;
                
#ifndef __EOL_USES_LF__
        case kCR: /* 字符结尾? */
#endif
            
#ifdef __EOL_USES_LF__
        case kLF:
#endif
            if (XSUCC != XOS_CliExtWriteStr(pCliEnv, kEOL))
                return ERROR_CLI_WRITE;
            
            loop = XFALSE;
            }
            break;
            
        case kBS:
            cli_extBackspace(pCliEnv);
            break;
            
        case kKEY_BREAK:
            if (CLI_IsEnabled(pCliEnv, kCLI_FLAG_LOGON))
            {
                XOS_CliExtWriteStrLine(pCliEnv, "");
                cli_extInitCommandLine(pCliEnv);
                /*RCC_TASK_PrintPrompt(pCliEnv);*/
            }
            break;
            
        case kTAB:
            cli_extTab(pCliEnv);
            break;
        case kKEY_DELETE:
        case kKEY_DELETE_CHAR:
            cli_extDeleteChar(pCliEnv);
            break;
        case kKEY_DELETE_FROM_START:
            break;
        case kKEY_DELETE_TO_END:
            break;
        case kKEY_LINE_START:
            break;
        case kKEY_LINE_END:
            break;
        case kKEY_MOVE_UP:
            cli_histScroll(pCliEnv, -1);
            break;
        case kKEY_MOVE_DOWN:
            cli_histScroll(pCliEnv, 1);
            break;
        case kKEY_MOVE_LEFT:
            cli_listMoveCursor(pCliEnv, -1);
            break;
        case kKEY_MOVE_RIGHT:
            cli_listMoveCursor(pCliEnv, 1);
            break;
        case kKEY_WORD_PREV:
            break;
        case kKEY_WORD_NEXT:
            break;
        case kKEY_UPPERCASE:
            break;
        case kKEY_LOWERCASE:
            break;
        case kKEY_DELETE_WORD_END:
            break;
        case kKEY_DELETE_WORD_START:
            break;
        case kKEY_TRANSPOSE:
            break;
        case kKEY_END_OF_ENTRY:
            XOS_CliExtWriteStrLine( pCliEnv, kEOL );
            status = STATUS_CLI_EXIT_TO_ROOT;
            loop   = XFALSE;
            break;
        case 25: /* Ctrl-y */
            break;
        case kKEY_HELP:
            {
                /* 即时显示帮助*/
                cli_extInsertText(pCliEnv, (XCHAR *) &charIn, 1);
                
                if (XSUCC != XOS_CliExtWriteStr(pCliEnv, kEOL))
                    return ERROR_CLI_WRITE;
                
                loop = XFALSE;
            }
            
            break; /* 帮助 */
            
        default:
            /* 实体字符 */
            if ((' ' <= charIn) && (charIn <= 127))
                cli_extInsertText(pCliEnv, (XCHAR *) &charIn, 1);
            break;
        }  /* switch */
        
        idx++;
        
    }/*while( idx < orgLen)*/
    
    /*  CLI_DisableFeature(pCliEnv, kCLI_FLAG_INPUT);*/
    
    if(loop == XFALSE)
        MMISC_SetInput(pCliEnv, 0);
    else
        MMISC_SetInput(pCliEnv, 1);
    
    /*用户鉴权 t_XOS_GEN_ENVIRON*/
    if( kTELNET_USR_PROCESS != pCliEnv->pCli->loginStat
        /*MMISC_GetLoginStat(pCliEnv) != kTELNET_USR_PROCESS*/ )
    {
        if( XSUCC != cli_taskLogin(pCliEnv) )
        {
            cli_extResetOutput(pCliEnv);
            cli_extInitCommandLine(pCliEnv);
        }
    }
    
    return status;
}

/************************************************************************
 函数名: cli_extNEWLINE(  )
 功能:  在新的一行打印字符串
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XSTATIC XVOID cli_extNEWLINE(CLI_ENV* pCliEnv)
{
    cli_extWrite(pCliEnv, kEOL, kEOL_SIZE);
    cli_extResetOutput(pCliEnv);
    cli_extInitCommandLine(pCliEnv);
}

/************************************************************************
 函数名: cli_taskLogin(  )
 功能:  提示登陆界面
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XSTATIC XS32 cli_taskLogin(CLI_ENV *pCliEnv)
{
    if ( XNULL == pCliEnv )
    {
        return XERROR;
    }
    switch( MMISC_GetLoginStat(pCliEnv) )
    {
    case kTELNET_NOT_LOGIN: /*未登陆状态*/
        {
            XOS_MemSet( MMISC_Login(pCliEnv),  0,  kCLI_MAX_LOGIN_LEN + 1   );
            XOS_MemSet( MMISC_Passwd(pCliEnv), 0, kCLI_MAX_PASSWORD_LEN + 1 );
            
            CLI_DisableFeature(pCliEnv, kCLI_FLAG_LOGON);
            
            XOS_CliExtWriteStrLine(pCliEnv, XOS_OS_VERSION );
            XOS_CliExtWriteStrLine(pCliEnv, "");
            
            XOS_CliExtWriteStrLine(pCliEnv, "*--------------------------------------------------------------------*");
            XOS_CliExtWriteStrLine(pCliEnv, "*                                                                    *");
            XOS_CliExtWriteStrLine(pCliEnv, "*                        XOS COMMAND LINE SYSTEM                     *");
            XOS_CliExtWriteStrLine(pCliEnv, "*                                                                    *");
            XOS_CliExtWriteStrLine(pCliEnv, "*--------------------------------------------------------------------*");
            
            XOS_CliExtWriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
            
            cli_extResetOutput(pCliEnv);
            cli_extInitCommandLine(pCliEnv);
            
            /*CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO); */
            
            MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);
        }
        break;
    case kTELNET_USRNAME_LOGIN: /* 处理并输出 "Login:",等待用户输入*/
        {
            CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);
            XOS_CliExtWriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
            
            MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);
            
            cli_extResetOutput(pCliEnv);
            cli_extInitCommandLine(pCliEnv);
        }
        break;
    case kTELNET_USRNAME_INPUT:  /* 处理并输出 "Login: 如数的字符串 admin , 然后显示 password*/
        {
            /*决定是否输入,即 输入 admin 是否完成*/
            if(MMISC_GetInput(pCliEnv) == 0)
            {
                if (kCLI_MAX_LOGIN_LEN <= MEDIT_GetLength(pCliEnv))
                {
                    XOS_CliExtWriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
                    MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);
                    return ERROR_CLI_RETRY;
                }
                
                if ( 0 == MEDIT_GetLength(pCliEnv))
                {
                    XOS_CliExtWriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
                    MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);
                    
                    return ERROR_CLI_RETRY;
                }
                
                MEDIT_CopyFromInput(pCliEnv, CLIENV(pCliEnv)->login);
                
                XOS_CliExtWriteStr(pCliEnv, kCLI_PASSWORD_PROMPT);
                CLI_DisableFeature(pCliEnv, kCLI_FLAG_ECHO);
                
                MMISC_SetLoginStat(pCliEnv, kTELNET_LOGIN_PROCESS);
                
                /*cli_extNEWLINE (pCliEnv);*/
                cli_extResetOutput(pCliEnv);
                cli_extInitCommandLine(pCliEnv);
            }
        }
        break;
    case kTELNET_LOGIN_PROCESS: /* 处理并输出 "password: 输入的字符串 admin , 验证用户登陆信息, 然后显示系统提示符 SYS>*/
        {
            if(MMISC_GetInput(pCliEnv) == 0)
            {
                if (kCLI_MAX_PASSWORD_LEN <= MEDIT_GetLength(pCliEnv))
                {
                    CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);
                    
                    /*PASSWD DISABLE ECHO KEOL NOT DISPLAYED.*/
                    XOS_CliExtWriteStrLine(pCliEnv, "");
                    
                    XOS_CliExtWriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
                    MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);
                    
                    return ERROR_CLI_RETRY;
                }
                
                if ( 0 == MEDIT_GetLength(pCliEnv) )
                {
                    CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);
                    
                    /*PASSWD DISABLE ECHO KEOL NOT DISPLAYED.*/
                    XOS_CliExtWriteStrLine(pCliEnv, "");
                    
                    XOS_CliExtWriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
                    MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);
                    
                    return ERROR_CLI_RETRY;
                }
                
                MEDIT_CopyFromInput(pCliEnv, CLIENV(pCliEnv)->passwd);
                
                if ( !cli_findUser( MMISC_Login(pCliEnv),
                    MMISC_Passwd(pCliEnv),
                    &(MMISC_GetAccess(pCliEnv))) )
                {
                    CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);
                    XOS_CliExtWriteStrLine(pCliEnv, "");
                    
                    XOS_CliExtWriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
                    MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);
                    
                    return ERROR_CLI_RETRY;
                }
                
                CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);
                MMISC_SetLoginStat(pCliEnv, kTELNET_USR_PROCESS);
                
                cli_extNEWLINE(pCliEnv);
                
                /*登陆成功, 显示提供提示符*/
                pCliEnv->RemoteBid = 0xFFFFFFFF; // 20110701 cxf add
                CLIENV(pCliEnv)->cliShellCmd.cliMode = SYSTEM_MODE;
                CLIENV(pCliEnv)->cliShellCmd.pCmdLst = g_paCliAppMode[SYSTEM_MODE].pModePCmds;
                
                /* 修改提示符*/
                cli_initPrompt(pCliEnv);
                
                /*加入已登录用户组*/
                cli_insert_usid_logined( MMISC_GetFsmId( pCliEnv ) );
                
                /*如果是debug用户则加入重定向*/
                if( !XOS_StrCmp( MMISC_Login(pCliEnv), "debug" ) )
                {
                    cli_open_direct_switch( MMISC_GetFsmId( pCliEnv ), XTRUE );
                }
            }
        }
        break;
    default:
        break;
    }
    
    return XSUCC;
}

/************************************************************************
 函数名: cli_taskValidateLogin(  )
 功能:  登陆验证
 输入:
 输出:
 返回:
 说明:
************************************************************************/
#ifdef XOS_WIN32
XSTATIC XCHAR cli_taskValidateLogin( XCHAR *pLogin, XCHAR *pPassword, Access *pAccLvl /* 用户级别暂未使用*/)
{
    if ( XNULL == pAccLvl )
    {
        return XFALSE;
    }
    if ( !XOS_StrCmp( kCLI_DEFAULT_LOGIN,    pLogin) &&
        !XOS_StrCmp( kCLI_DEFAULT_PASSWORD, pPassword ) )
    {
        return XTRUE;
    }
    return XFALSE;
}
#endif
/************************************************************************
 函数名: cli_UTILInit(  )
 功能:  初始化
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XVOID cli_UTILInit(CLI_ENV *pCliEnv)
{
    /* enable intermediate mode */
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_MODE);
    /* expanded error warnings */
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_WARN);
    /* show help for child nodes */
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_HELPCHILD);
    /* write to output updates cursor position*/
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_UPDATE);
    /* enable paging of output */
    /* make sure output is initially visible */
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);
    
    return ;
}

/************************************************************************
 函数名: cli_dbInitEnvironment(  )
 功能:  为环境变量分配内存
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XSTATIC XS32 cli_dbInitEnvironment(CLI_ENV **ppCliEnv, t_XOS_COM_CHAN *pChannel)
{
    t_XOS_CLI_INFO     *pCliEnv = XNULL;
    t_XOS_GEN_ENVIRON  *pNewEnv = XNULL;
    CLI_ENV            **pCli   = &pNewEnv;
    XS32               si       = 0;
    
    /* 参数合法性检查*/
    if ( XNULL == ppCliEnv || XNULL == pChannel )
    {
        return XERROR;
    }
    if ( XSUCC != cli_environmentConstruct(&pNewEnv) )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "cli_environmentConstruct() error!" );
        return XERROR;
    }
    
    si = sizeof(t_XOS_CLI_INFO);
    pCliEnv = (t_XOS_CLI_INFO*)XOS_MemMalloc( (XU32)FID_CLI, (XU32)si );
    if ( XNULL == pCliEnv )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "XOS_MemMalloc( (XU32)FID_CLI, (XU32)si:%d ) error!", si );
        cli_environmentDestruct(&pNewEnv);
        
        return XERROR;
    }
    XOS_MemSet( pCliEnv, 0, si );
    
    pNewEnv->pCli         = pCliEnv;
    pCliEnv->pChannel     = pChannel;
    pCliEnv->pEnvironment = pNewEnv;
    pCliEnv->chMore       = 0;
    
#ifndef __DISABLE_STRUCTURES__
    si = cli_cacheConstruct(&pNewEnv->phCacheHandle, K_CACHE_READWRITE);
    if( XSUCC != si )
    {
        cli_environmentDestruct(&pNewEnv);
        XOS_MemFree( FID_CLI, pCliEnv);
        
        XOS_Trace( MD(FID_CLI, PL_ERR), "cli_cacheConstruct() error!" );
        return XERROR;
    }
#endif
    
    pChannel->env = pNewEnv;
    *ppCliEnv     = *pCli;
    
    return XSUCC;
}
/************************************************************************
 函数名: cli_environmentConstruct(  )
 功能:  环境变量申请分配内存
 输入:
 输出:
 返回:
 说明: Review POK
************************************************************************/
XSTATIC XS32 cli_environmentConstruct( t_XOS_GEN_ENVIRON **pp_envInit )
{
    if ( XNULL == pp_envInit )
    {
        return XERROR;
    }

    if ( XNULL == (*pp_envInit = (t_XOS_GEN_ENVIRON*)XOS_MemMalloc( FID_CLI, sizeof(t_XOS_GEN_ENVIRON))) )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "XOS_MemMalloc( FID_CLI, sizeof(t_XOS_GEN_ENVIRON)) error!" );
        return XERROR;
    }

    XOS_MemSet( *pp_envInit, 0x00, sizeof(t_XOS_GEN_ENVIRON) );

    (*pp_envInit)->PostValid            = XTRUE;
    (*pp_envInit)->PostBuffer           = XNULL;
    (*pp_envInit)->PostBufferLen        = 0;
    (*pp_envInit)->UserLevel            = 0;
    (*pp_envInit)->clientIndex          = -1;

/*  for (index = 0; index < K_MAX_ENV_SIZE; index++)
        (*pp_envInit)->variables[index] = XNULL;
*/

    return XSUCC;

}   /* cli_environmentConstruct */

/************************************************************************
 函数名: cli_environment Destruct(  )
 功能:  释放环境变量分配内存
 输入:
 输出:
 返回:
 说明: Review POK
************************************************************************/
XSTATIC XS32 cli_environmentDestruct(t_XOS_GEN_ENVIRON **pp_envTemp)
{
    t_XOS_GEN_ENVIRON *p_envTemp;

    if (XNULL == pp_envTemp)
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "cli_environment Destruct() parameter error!" );
        return ERROR_GENERAL_NO_DATA;
    }

    p_envTemp = *pp_envTemp;
    if ( XNULL == p_envTemp )
    {
        return ERROR_GENERAL_NO_DATA;
    }

    if ( XNULL != p_envTemp->phCacheHandle )
    {
        /*8888*/
        p_envTemp->phCacheHandle = XNULL;
    }

    if ( XNULL != p_envTemp->PostBuffer )
    {
        XOS_MemFree( FID_CLI, p_envTemp->PostBuffer);
        p_envTemp->PostBuffer = XNULL;
    }

    XOS_MemFree( FID_CLI, p_envTemp);
    *pp_envTemp = XNULL;

    return XSUCC;
}

/************************************************************************
 函数名: cli_cacheConstruct(  )
 功能:
 输入:   cp:
 输出:   无
 返回:
 说明:   Review POK
************************************************************************/
XSTATIC XS32 cli_cacheConstruct(t_XOS_CACHE_HANDLE **pph_cacHandle, Access AccessType)
{
    if (XNULL == pph_cacHandle)
    {
        return XERROR;
    }

    *pph_cacHandle = (t_XOS_CACHE_HANDLE *)XOS_MemMalloc( FID_CLI, sizeof(t_XOS_CACHE_HANDLE));
    if (XNULL == (*pph_cacHandle) )
    {
        return XERROR;
    }

    if ( XNULL == ((*pph_cacHandle)->p_lstCacheObjects = cli_listConstruct()) )
    {
        XOS_MemFree( FID_CLI, *pph_cacHandle);
        *pph_cacHandle = XNULL;

        return XERROR;
    }

    (*pph_cacHandle)->AccessType = AccessType;

    return XSUCC;
}

/************************************************************************
 函数名: cli_strToUp(  )
 功能:   转化大写字符串
 输入:   cp:
 输出:   无
 返回:
 说明:   Review OK
************************************************************************/
XSTATIC XVOID cli_strToUp(XCHAR * s)
{
    XS32 i=0;

    if ( XNULL == s )
    {
        return;
    }

    for(i = 0; i < (XS32)XOS_StrLen(s); i++)
    {
        if(s[i] >= 'a' && s[i] <= 'z' )
        {
            s[i] -= 0x20;
        }
    }

}

/************************************************************************
 函数名: cli_CheckBlanks(  )
 功能:   检查命令中的非法字符串,不能含有 " ","  ", "?" 字符
 输入:   pszPrompt
 输出:   无
 返回:   无
 说明:   无 Review OK
************************************************************************/
XSTATIC XBOOL cli_CheckBlanks( XS8 * pszPrompt, XS32 s32Len )
{
    XS32 i = 0;

    if ( XNULL == pszPrompt || 0 > s32Len )
    {
        return XFALSE;
    }

    for ( i = 0; i < s32Len; i++ )
    {
        if ( '?' == pszPrompt[i]
        || ' ' == pszPrompt[i]
        /*|| '    ' == pszPrompt[i]*/
        || '>' == pszPrompt[i])
        {
            return XTRUE;
        }
    }
    return XFALSE;
}

/************************************************************************
 函数名: cli_getPromptLayer(  )
 功能:   提示符光标的层数
 输入:   pszPrompt
 输出:   无
 返回:   无
 说明:   无 Review OK
************************************************************************/
XSTATIC XU32 cli_getPromptLayer( XS8 * pszPrompt )
{
    XS8 * pTemp = 0;
    XS8 * pDirTemp = pszPrompt;
    XU32  nLen = 0;

    if ( pszPrompt == XNULL )
    {
        return -1;
    }

    while ( (pTemp = XOS_StrStr(pDirTemp, ">")) != XNULL )
    {
        nLen++;

        pDirTemp = pTemp+1;
    }

    return nLen;
}

/************************************************************************
 函数名: cli_open_direct_switch(  )
 功能:   打开指定sid的从定向开关
 输入:   s32sid: XS32 类型
 输出:   无
 返回:   无
 说明:
************************************************************************/
XSTATIC XBOOL cli_open_direct_switch(XS32 s32sid, XBOOL bSwitchFlag)
{
    XS32 i = 0;

    if ( 0 > s32sid || kCLI_MAX_CLI_TASK < s32sid )
    {
        return XFALSE;
    }

    for ( i = 0; i <= kCLI_MAX_CLI_TASK; i++ )
    {
        if ( s32sid == g_paUserSIDLogined[i].xs32SID )
        {
            g_paUserSIDLogined[i].xbSwitch = bSwitchFlag;

            return XTRUE;
        }
    }

    return XFALSE;
}
/************************************************************************
 函数名: cli_insert_usid_logined(  )
 功能:   添加新的sid.
 输入:   s32sid: Telenet client sid

 输出:   无
 返回:   成功返回 SID/失败返回 0
 说明:
************************************************************************/
XSTATIC XS32 cli_insert_usid_logined( XS32 s32sid )
{
    XS32 i = 0;
    if ( 0 > s32sid || kCLI_MAX_CLI_TASK < s32sid )
    {
        return 0;
    }

    for ( i = 0; i <= kCLI_MAX_CLI_TASK; i++ )
    {
         if ( g_paUserSIDLogined[i].xs32SID == 0 )
         {
            g_paUserSIDLogined[i].xs32SID = s32sid;
            return s32sid;
         }
         else
         {
            continue;
         }
    }

    return 0;
}
/************************************************************************
 函数名: cli_delete_usid_logined(  )
 功能:   删除Sid.
 输入:   s32sid: Telenet client sid

 输出:   无
 返回:   成功返回XTRUED/失败返回XFALSE
 说明:
************************************************************************/
XSTATIC XBOOL cli_delete_usid_logined( XS32 s32sid )
{
    XS32 i = 0;
    for ( i = 0; i <= kCLI_MAX_CLI_TASK; i++ )
    {
         if ( g_paUserSIDLogined[i].xs32SID == s32sid )
         {
            g_paUserSIDLogined[i].xs32SID = 0;
            g_paUserSIDLogined[i].xbSwitch = XFALSE; /*## */
            return XTRUE;
         }
         else
         {
            continue;
         }
    }

    return XFALSE;
}

/************************************************************************
 函数名: cli_regist_cmdprompt(  )
 功能:   注册提示符命令.
 输入:   prarenPromptID:  上级父提示符 ID
         pscCmdName:      CMD 名称
         pscCmdHelpStr:   CMD 帮助字符串
         pscParaHelpStr:  CMD 参数说明
         pParentLocation: 在父提示符中的位置

 输出:   无
 返回:   成功返回提示符命令ID/失败返回XERROR,或者错误码
 说明:   Review OK
************************************************************************/
XS32 cli_regist_cmdprompt( XCONST XS32   prarenPromptID,
                           XCONST XCHAR* pscCmdName,
                           XCONST XCHAR* pscCmdHelpStr,
                           XCONST XCHAR* pscParaHelpStr,
                           XS32* pParentLocation )
{
    /*找到第 i 个提示符*/
    XU32  i                                       = prarenPromptID;
    /*放在第 j 个命令中*/
    XU32  j                                       = 0;
    XU32  nNewMode                                = 0;
    XCHAR szBuffer[ CLI_MAX_CURSOR_LEN + 1]       = {0};
    XU32  nLeng                                   = 0;
    XCHAR szCmdName[ CLI_MAX_CMD_LEN + 1 ]        = {0};
    XCHAR szCmdHelpStr[ CLI_MAX_CMD_HELP_LEN + 1] = {0};
    XCHAR szParaHelpStr[ CLI_MAX_ARG_LEN + 1]     = {0};

    if (   SYSTEM_MODE             >  prarenPromptID
        || (CLI_MAX_CMD_CLASS - 1) <  prarenPromptID
        || XNULL                   == pscCmdName
        || XNULL                   == pscCmdHelpStr
        || XNULL                   == pscParaHelpStr
        || XNULL                   == pParentLocation )
    {
        return XERROR;
    }

    XOS_StrNcpy( szCmdName,     pscCmdName,     CLI_MAX_CMD_LEN + 1      );
    XOS_StrNcpy( szCmdHelpStr,  pscCmdHelpStr,  CLI_MAX_CMD_HELP_LEN + 1 );
    XOS_StrNcpy( szParaHelpStr, pscParaHelpStr, CLI_MAX_ARG_LEN + 1      );

    /*命令超长*/
    if (   '\0' != szCmdName[CLI_MAX_CMD_LEN]
        || '\0' != szCmdHelpStr[CLI_MAX_CMD_HELP_LEN]
        || '\0' != szParaHelpStr[CLI_MAX_ARG_LEN] )
    {
        return (XERROR - 1);
    }

    if ( 0 >= XOS_StrLen( szCmdName ) )
    {
        return (XERROR - 2);
    }

    /*检查命令中的空格*/
    if ( cli_CheckBlanks( szCmdName, (XS32)XOS_StrLen( szCmdName ) ) )
    {
        return (XERROR - 3);
    }

    if ( XNULL == g_paCliAppMode[i].pModePCmds )
    {
        return XERROR;
    }

    /*在已有模式中查找*/
    for( j = 0; j < CLI_MAX_CMD_NUM; j++ )
    {
        if( 0 != XOS_StrLen( g_paCliAppMode[i].pModePCmds[j].pscCmdName ) )
        {
            /*找到已经注册的提示符*/
            if ( !XOS_StrCmp( pscCmdName, g_paCliAppMode[i].pModePCmds[j].pscCmdName ) )
            {
                return g_paCliAppMode[i].pModePCmds[j].m_nMode;
            }
        }
        else
        {
            break;
        }
    }

    /*已经满了*/
    if ( CLI_MAX_CMD_NUM <= j )
    {
        return (XERROR - 4);
    }

    /*分配空间*/
    for( nNewMode = 0; nNewMode < CLI_MAX_CMD_CLASS; nNewMode++ )
    {
        if( XNULL == g_paCliAppMode[nNewMode].pModePCmds )
        {
            break;
        }
    }
    /*已经满了*/
    if ( CLI_MAX_CMD_CLASS <= nNewMode )
    {
        return (XERROR - 5);
    }

    /*光标长度合法化验证,不超过 3 层*/
    XOS_MemCpy( szBuffer, g_paCliAppMode[i].pscCurSor, CLI_MAX_CURSOR_LEN );
    nLeng = cli_getPromptLayer( szBuffer );
    if  ( 6 < nLeng )//中继网关chendahong需求3-->6
    {
        return (XERROR - 6);
    }
    /*光标总长度验证*/
    nLeng = (XU32)XOS_StrLen( g_paCliAppMode[i].pscCurSor ) + (XU32)XOS_StrLen( szCmdName ) + 1;
    nLeng = nLeng + (XU32)XOS_StrLen( szCmdName );
    if ( kCLI_DEFAULT_WIDTH < nLeng )
    {
        return (XERROR - 7);
    }

    /*初始化新的模式*/
    g_paCliAppMode[nNewMode].pModePCmds = cli_NewCmds( );
    if ( XNULL == g_paCliAppMode[nNewMode].pModePCmds )
    {
        return (XERROR - 8);
    }
    /*说明
    i:为上一层提示符位置
    nNewMode:为新一层提示符位置
    j:为在上一层i提示符所在命令集中的命令序号*/
    /*加入新模式命令返回命令,SYSTEM_EXIT_QUIT_INDEX 必须一致INDEX_CMD_QUIT */
    g_paCliAppMode[nNewMode].pModePCmds[SYSTEM_EXIT_QUIT_INDEX].m_nMode   = nNewMode;
    g_paCliAppMode[nNewMode].pModePCmds[SYSTEM_EXIT_QUIT_INDEX].m_nParent = i;

    /*加入新模式跳转命令*/
    g_paCliAppMode[i].pModePCmds[j].m_nMode   = nNewMode;
    g_paCliAppMode[i].pModePCmds[j].m_nParent = i;

    g_paCliAppMode[i].pModePCmds[j].pCmdHandler = cli_cmdSwitchPrompt;
    XOS_MemCpy( g_paCliAppMode[i].pModePCmds[j].pscCmdName,     szCmdName,     CLI_MAX_CMD_LEN      );
    XOS_MemCpy( g_paCliAppMode[i].pModePCmds[j].pscCmdHelpStr,  szCmdHelpStr,  CLI_MAX_CMD_HELP_LEN );
    XOS_MemCpy( g_paCliAppMode[i].pModePCmds[j].pscParaHelpStr, szParaHelpStr, CLI_MAX_ARG_LEN      );

    /*计算光标,将上一层的光标合并到当前光标的一部分*/
    XOS_MemCpy(  g_paCliAppMode[nNewMode].pscCurSor, g_paCliAppMode[i].pscCurSor, CLI_MAX_CURSOR_LEN );
    XOS_StrNCat( g_paCliAppMode[nNewMode].pscCurSor, szCmdName,                   CLI_MAX_CURSOR_LEN );
    XOS_StrNCat( g_paCliAppMode[nNewMode].pscCurSor, ">",                         CLI_MAX_CURSOR_LEN );

    /*转换为大写*/
    cli_strToUp( g_paCliAppMode[nNewMode].pscCurSor );

    *pParentLocation = j;

    return (nNewMode);
}

/************************************************************************
 函数名: XOS_RegistCmdPromptX(  )
 功能:   注册提示符命令.
 输入:   prarenPromptID: 字符串类型,上级父提示符 ID.
         pscCmdName:     字符串类型,CMD 名称.
         pscCmdHelpStr:  字符串类型,CMD 帮助字符串.
         pscParaHelpStr: 字符串类型,CMD 参数说明.
         eAccess:        e_USERCALSS 类型, 命令权限级别.

 输出:   无
 返回:   成功返回提示符命令ID/失败返回XERROR,或者错误码
 说明:   Review OK
************************************************************************/
XS32 XOS_RegistCmdPromptX( XCONST XS32   prarenPromptID,
                           XCONST XCHAR* pscCmdName,
                           XCONST XCHAR* pscCmdHelpStr,
                           XCONST XCHAR* pscParaHelpStr,
                           e_USERCALSS   eAccess )
{
    XS32 s32cmdid = -1;
    XS32 Location = 0;

    if ( 0 > eAccess || eUserNormal  < eAccess )
    {
        return XERROR;
    }

    s32cmdid = cli_regist_cmdprompt( prarenPromptID,
                                     pscCmdName,
                                     pscCmdHelpStr,
                                     pscParaHelpStr,
                                     &Location );
    if ( SYSTEM_MODE > s32cmdid || (CLI_MAX_CMD_CLASS - 1) < s32cmdid )
    {
        return XERROR;
    }

    g_paCliAppMode[prarenPromptID].pModePCmds[Location].eAccLvl = eAccess;

    return s32cmdid;
}
/************************************************************************
 函数名: XOS_RegAppVer 2007/11/22加入本功能,用于现场版本管理和维护
 功能:   提供各模块注册版本管理信息的接口.
 输入:   AppVerName:       模块及应用程序名称
         strVerValue:      模块及应用程序版本字符串描述
 说明:   两个版本参数值最少输入一项如
         XOS_RegAppVer("BSP    version","01.00.01.00");
 输出:   无
 返回:   成功返回命令ID/失败返回XERROR,或者错误码
 说明:
************************************************************************/
XS32 XOS_RegAppVer(XCONST XCHAR*  AppVerName,XCONST XCHAR* strVerValue)
{
    XS32 i=0;
    XU32 Ver_Len_Max=8;
    XU32 Ver_Value_Max = CLI_MAX_INPUT_LEN-2;
    XCHAR szVerName[CLI_MAX_INPUT_LEN];
    XOS_MemSet(szVerName,0x0,CLI_MAX_INPUT_LEN);
    
    if(!AppVerName || !strVerValue)
    {
        return XERROR;
    }
    
    if((XOS_StrLen(AppVerName)==0)  || (XOS_StrLen(strVerValue)==0))
    {
        return XERROR;
    }
    
    if(XOS_StrLen(AppVerName) > Ver_Len_Max)
    {
        XOS_StrNcpy(szVerName,AppVerName,Ver_Len_Max);
        szVerName[Ver_Len_Max-1]=0x0;
    }
    else
    {
        XOS_StrCpy(szVerName,AppVerName);
    }

    /*在已有注册中中查找*/
    for( i = 0; i < CLI_REGVER_MAX; i++ )
    {
        if( 0 != XOS_StrLen( g_paCliVer[i].szVerName))
        {
            /*找到已经注册的提示符*/
            if ( !XOS_StrCmp(g_paCliVer[i].szVerName,szVerName))
            {
                return XSUCC;
            }
        }
        else if(g_paCliVer[i].verFlag==XTRUE)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    
    /*已经满了*/
    if ( CLI_REGVER_MAX <= i )
    {
        return XERROR;
    }

    g_paCliVer[i].index=i;
    XOS_StrCpy(g_paCliVer[i].szVerName,szVerName);

    if(XOS_StrLen(strVerValue) > Ver_Value_Max)
    {
        XOS_StrNcpy(g_paCliVer[i].szVerValue,strVerValue,Ver_Value_Max);
        g_paCliVer[i].szVerValue[Ver_Value_Max-1]=0x0;
    }
    else
    {
        XOS_StrCpy(g_paCliVer[i].szVerValue,strVerValue);
    }

    g_paCliVer[i].verFlag=XTRUE;
    
    return XSUCC;
}
XCHAR* XOS_GetVersion(XVOID)
{
    XS32 i;
    XS32 ipos=0;
    XS32 ilen=0;
    XCHAR szSeperator1[]=":";
    XCHAR szSeperator2[]="|";
    XSTATIC XCHAR strValue[255]={0};
    XOS_MemSet(strValue,0x0,255);
    for(i=1;i<CLI_REGVER_MAX;i++)
    {
        if(g_paCliVer[i].verFlag==XTRUE)
        {
            ilen = (XS32)XOS_StrLen(g_paCliVer[i].szVerName);
            if(ipos+ilen<250)
            {
                XOS_StrCat(strValue,g_paCliVer[i].szVerName);
            }
            ipos+=ilen;
            ilen = (XS32)XOS_StrLen(szSeperator1);
            if(ipos+ilen<250)
            {
                XOS_StrCat(strValue,szSeperator1);
            }
            ipos+=ilen;
            ilen = (XS32)XOS_StrLen(g_paCliVer[i].szVerValue);
            if(ipos+ilen<250)
            {
                XOS_StrCat(strValue,g_paCliVer[i].szVerValue);
            }
            ipos+=ilen;
            ilen = (XS32)XOS_StrLen(szSeperator2);
            if(ipos+ilen<250)
            {
                XOS_StrCat(strValue,szSeperator2);
            }
            ipos+=ilen;
            
        }
    }
    strValue[250]=0x0;
    //printf("%s\r\n",strValue);
    return strValue;
}

XS32 XOS_RegNeVer(XCONST XCHAR* strVerValue)
{
    XU32 len_max=128;
    if(!strVerValue)
    {
        return XERROR;
    }
    if(XOS_StrLen(strVerValue)==0)
    {
        return XERROR;
    }
    if(XOS_StrLen(strVerValue) > len_max)
    {
        XOS_StrNcpy(ne_Version,strVerValue,len_max);
        ne_Version[len_max-1]=0x0;
    }
    else
    {
        XOS_StrCpy(ne_Version,strVerValue);
    }
    return XSUCC;
}
XCHAR* XOS_GetNeVersion(XVOID)
{
    return (XCHAR*)ne_Version;
}

/************************************************************************
 函数名: XOS_RegistCmdPrompt(  )
 功能:   注册提示符命令.
 输入:   prarenPromptID: 上级父提示符 ID
         pscCmdName:     CMD 名称
         pscCmdHelpStr:  CMD 帮助字符串
         pscParaHelpStr: CMD 参数说明

 输出:   无
 返回:   成功返回提示符命令ID/失败返回XERROR,或者错误码
 说明:
************************************************************************/
XS32 XOS_RegistCmdPrompt( XCONST XS32   prarenPromptID,
                         XCONST XCHAR* pscCmdName,
                         XCONST XCHAR* pscCmdHelpStr,
                         XCONST XCHAR* pscParaHelpStr )
{
    XS32 s32cmdid = -1;
    XS32 Location = 0;
    
    s32cmdid = cli_regist_cmdprompt( prarenPromptID,
        pscCmdName,
        pscCmdHelpStr,
        pscParaHelpStr,
        &Location );
    if (   SYSTEM_MODE             >  s32cmdid
        || (CLI_MAX_CMD_CLASS - 1) <  s32cmdid
        || CLI_MAX_CMD_NUM         <= Location
        || SYSTEM_MODE             >  Location)
    {
        return s32cmdid;
    }
    
    g_paCliAppMode[prarenPromptID].pModePCmds[Location].eAccLvl = eUserNormal;
    
    return s32cmdid;
}
/************************************************************************
 函数名: XOS_RegistCommand(  )
 功能:   根据提示符 ID 注册子命令.
 输入:   promptID:       提示符 ID
         pCmdHandler:    CMD 处理函数
         pscCmdName:     CMD 名称
         pscCmdHelpStr:  CMD 帮助字符串
         pscParaHelpStr: CMD 参数说明

 输出:   无
 返回:   成功返回命令ID/失败返回XERROR,或者错误码
 说明:   Review OK
************************************************************************/
XS32 XOS_RegistCommand( XCONST XS32    promptID,
                        CmdHandlerFunc pCmdHandler,
                        XCONST XCHAR*  pscCmdName,
                        XCONST XCHAR*  pscCmdHelpStr,
                        XCONST XCHAR*  pscParaHelpStr )
{
    XU32 i                                        = 0;
    t_XOS_CMD_DATAS* cmdData                      = XNULL;
    XCHAR szCmdName[ CLI_MAX_CMD_LEN + 1 ]        = {0};
    XCHAR szCmdHelpStr[ CLI_MAX_CMD_HELP_LEN + 1] = {0};
    XCHAR szParaHelpStr[ CLI_MAX_ARG_LEN + 1]     = {0};

    if (   SYSTEM_MODE             >  promptID
        || (CLI_MAX_CMD_CLASS - 1) <  promptID
        || XNULL                   == pCmdHandler
        || XNULL                   == pscCmdName
        || XNULL                   == pscCmdHelpStr
        || XNULL                   == pscParaHelpStr )
    {
        return (XERROR);
    }

    XOS_StrNcpy( szCmdName,     pscCmdName,     CLI_MAX_CMD_LEN + 1      );
    XOS_StrNcpy( szCmdHelpStr,  pscCmdHelpStr,  CLI_MAX_CMD_HELP_LEN + 1 );
    XOS_StrNcpy( szParaHelpStr, pscParaHelpStr, CLI_MAX_ARG_LEN + 1      );

    /*命令超长*/
    if (   '\0' != szCmdName[CLI_MAX_CMD_LEN]
        || '\0' != szCmdHelpStr[CLI_MAX_CMD_HELP_LEN]
        || '\0' != szParaHelpStr[CLI_MAX_ARG_LEN] )
    {
        return (XERROR - 1);
    }

    if ( 0 >= XOS_StrLen( szCmdName ) )
    {
        return (XERROR - 2);
    }

    /*检查命令中的空格*/
    if ( cli_CheckBlanks( szCmdName, (XS32)XOS_StrLen( szCmdName ) ) )
    {
        return (XERROR - 3);
    }

    cmdData = g_paCliAppMode[promptID].pModePCmds;
    if ( XNULL == cmdData )
    {
        return XERROR;
    }

    i = 0;
    while( i < CLI_MAX_CMD_NUM
        && XOS_StrLen( cmdData[i].pscCmdName ) > 0 )
    {
        if ( !XOS_StrCmp( cmdData[i].pscCmdName, szCmdName ) )
        {
            return i;
        }

        i++;
    }
    if ( CLI_MAX_CMD_NUM <= i )
    {
        return (XERROR - 4);
    }

    cmdData[i].siFlag         = 0;
    if(0 == gRemoteShowCli)
    {
        cmdData[i].pCmdHandler    = pCmdHandler;
    }
    else
    {
        cmdData[i].pCmdHandler    = cli_RemoteExecCli;
    }
    
    XOS_MemCpy( cmdData[i].pscCmdName,     szCmdName,     CLI_MAX_CMD_LEN      );
    XOS_MemCpy( cmdData[i].pscCmdHelpStr,  szCmdHelpStr,  CLI_MAX_CMD_HELP_LEN );
    XOS_MemCpy( cmdData[i].pscParaHelpStr, szParaHelpStr, CLI_MAX_ARG_LEN      );
    cmdData[i].eAccLvl = eUserNormal;

    return i;
}
/************************************************************************
 函数名: XOS_RegistCommandX(  )
 功能:   根据提示符 ID 注册子命令.
 输入:   promptID:       提示符 ID
         pCmdHandler:    CMD 处理函数
         pscCmdName:     CMD 名称
         pscCmdHelpStr:  CMD 帮助字符串
         pscParaHelpStr: CMD 参数说明
         eAccess:        e_USERCALSS 类型, 命令权限级别.

 输出:   无
 返回:   成功返回命令ID/失败返回XERROR,或者错误码
 说明:
************************************************************************/
XS32 XOS_RegistCommandX( XCONST XS32    promptID,
                         CmdHandlerFunc pCmdHandler,
                         XCONST XCHAR*  pscCmdName,
                         XCONST XCHAR*  pscCmdHelpStr,
                         XCONST XCHAR*  pscParaHelpStr,
                         e_USERCALSS    eAccess )
{
    XS32  s32ret = -1;

    if ( 0 > eAccess || eUserNormal  < eAccess )
    {
        return XERROR;
    }

    s32ret = XOS_RegistCommand( promptID,
                                pCmdHandler,
                                pscCmdName,
                                pscCmdHelpStr,
                                pscParaHelpStr );

    if ( SYSTEM_MODE > s32ret || (CLI_MAX_CMD_NUM - 1) < s32ret )
    {
        return XERROR;
    }

    g_paCliAppMode[promptID].pModePCmds[s32ret].eAccLvl = eAccess;

    return s32ret;
}

/************************************************************************
 函数名: cli_log_debug(  )
 功能:
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XVOID cli_log_debug( XBOOL bFlag  )
{
    g_bDubugFlag = bFlag;
}

/************************************************************************
 函数名: cli_log_write(  )
 功能:   CLI日志
 输入:
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XVOID cli_log_write( e_LOGLEVEL nLevel, XCONST XCHAR* fmt, ... )
{
#ifndef XOS_VXWORKS
    FILE *log                    = NULL;
    XCHAR buf[MAX_BUFF_LEN + 1]  = {0};
    XS32  errno_save             = -1;
    struct tm *tmnow             = NULL;
    time_t Clock                 = 0;
    va_list var;
#ifdef XOS_EW_START
    XCHAR szLogFName[XOS_MAX_PATHLEN]={0};
    sprintf( szLogFName, "%s%s", g_aszSysFPath, "clilog.log"  );
#endif

    if ( eCLIDebug == nLevel )
    {
        if ( !g_bDubugFlag )
        {
            return;
        }
    }

    if ( bClI_InitFlag )
    {
        if ( XSUCC != XOS_MutexLock( &g_cliMutex ) )
        {
            return;
        }
    }

    errno_save = errno;

    /**/
#ifndef XOS_EW_START
    log = fopen( "clilog.log", "a" );
#else
    log = fopen( szLogFName, "a" );
#endif

    if ( log == NULL )
    {
        if ( XSUCC != XOS_MutexUnlock( &g_cliMutex ) )
        {
            return;
        }
        return ;
    }

    Clock = (time_t)time((time_t*)0);
    tmnow = (struct tm*)localtime(&Clock);
    sprintf( buf, "{%04d-%02d-%02d  %02d:%02d:%02d}",
                       tmnow->tm_year+1900, tmnow->tm_mon+1,
                       tmnow->tm_mday, tmnow->tm_hour,
                       tmnow->tm_min, tmnow->tm_sec);

    buf[MAX_BUFF_LEN] = '\0';
    fprintf( log, buf );
    fflush( log );
    memset( buf, 0x00, MAX_BUFF_LEN + 1 );

    if( nLevel == eCLINormal )
    {
        strncat( buf, "NORMAL: ", MAX_BUFF_LEN );
    }
    else if( nLevel == eCLIError )
    {
        strncat( buf, "ERROR: ", MAX_BUFF_LEN );
    }
    else if( nLevel == eCLIDebug )
    {
        strncat( buf, "DEBUG: ", MAX_BUFF_LEN );
    }
    else
    {
        return;
    }
    buf[MAX_BUFF_LEN] = '\0';
    fprintf( log, buf );
    fflush( log );

    va_start(var, fmt);
    vfprintf( log, fmt, var );
    fprintf( log, "\n" );
    va_end( var );
    fflush( log );

    if ( nLevel == eCLIError )
    {
        memset( buf, 0x00, MAX_BUFF_LEN );
        sprintf( buf, "    [os system description: %s]", strerror( errno_save ) );

        buf[MAX_BUFF_LEN] = '\0';
        fprintf( log, buf );
        fprintf( log, "\n" );
        fflush( log );
    }

    if ( log != NULL )
    {
        fclose( log );
    }

    if ( bClI_InitFlag )
    {
        if ( XSUCC != XOS_MutexUnlock( &g_cliMutex ) )
        {
            return;
        }
    }
#endif /*XOS_VXWORKS*/

    return;
}

/************************************************************************
 函数名: cli_log_debug_cmd(  )
 功能:   clilog 命令函数.
 输入:   无
 输出:   无
 返回:   无
 说明:
************************************************************************/
XVOID cli_log_debug_cmd( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv )
{
    XBOOL bDebug   = XFALSE;

    if ( siArgc != 2 )
    {
        XOS_CliExtPrintf(pCliEnv,"debug state:%s.", (bDebug?"on":"off") );
        return;
    }

    if (   0 != XOS_StrNcmp( ppArgv[1], "on", 2 )
        && 0 != XOS_StrNcmp( ppArgv[1], "off", 3 ) )
    {
        XOS_CliExtPrintf(pCliEnv,"debug state:%s.", (bDebug?"on":"off") );
        return;
    }

    if ( 0 == XOS_StrNcmp( ppArgv[1], "on", 2 ) )
    {
        bDebug = XTRUE;
    }
    else if ( 0 == XOS_StrNcmp( ppArgv[1], "off", 3 ) )
    {
        bDebug = XFALSE;
    }

    cli_log_debug( bDebug );

    XOS_CliExtPrintf(pCliEnv,"debug state:%s.", (bDebug?"on":"off") );

    return;
}

/************************************************************************
 函数名: cli_printf_switch_cmd(  )
 功能:   print_direct 命令事件响应函数.
 输入:   无
 输出:   无
 返回:   无
 说明:
************************************************************************/
XVOID cli_printf_switch_cmd( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv )
{
    XS32 i = 0, s32Value = 0;

    if ( 1 == siArgc )
    {
        XOS_CliExtWriteStrLine( pCliEnv, "---------------------------------------------");
        for ( i = 0; i < kCLI_MAX_CLI_TASK; i++ )
        {
            if ( 0 < g_paUserSIDLogined[i].xs32SID )
            {
                if ( g_paUserSIDLogined[i].xbSwitch )
                {
                    XOS_CliExtPrintf(pCliEnv,
                                      "Telnet Client (SID = %d) Print Direction on\r\n",
                                      g_paUserSIDLogined[i].xs32SID );

                    s32Value = 1;
                }
                else
                {
                    XOS_CliExtPrintf(pCliEnv,
                                      "Telnet Client (SID = %d) Print Direction off\r\n",
                                      g_paUserSIDLogined[i].xs32SID );

                    s32Value = 1;
                }
            }
        }

        if ( !s32Value )
        {
            XOS_CliExtPrintf(pCliEnv,"can't found telnet client login.\r\n" );
        }
        XOS_CliExtWriteStrLine( pCliEnv, "---------------------------------------------");

        return;
    }
    /*支持用户直接输入print_direct on/off命令来打开或关闭当前终端的trace打印*/
    else if ( 2 == siArgc )
    {
        if(MMISC_GetFsmId(pCliEnv) == 0)
        {
            XOS_CliExtPrintf(pCliEnv,"this command is for telnet terminal.",
                                      MMISC_GetFsmId(pCliEnv) );
            return;
        }
        for ( i = 0; i < kCLI_MAX_CLI_TASK; i++ )
        {
            if ( MMISC_GetFsmId(pCliEnv) == g_paUserSIDLogined[i].xs32SID )
            {
                if ( !XOS_StrNcmp( ppArgv[1], "on", 2 ) )
                {
                    g_paUserSIDLogined[i].xbSwitch = XTRUE;
                    XOS_CliExtPrintf(pCliEnv,
                                      "open print switch to telnet client(SID = %d).",
                                      MMISC_GetFsmId(pCliEnv) );
                }
                else if ( !XOS_StrNcmp( ppArgv[1], "off", 3 ) )
                {
                    g_paUserSIDLogined[i].xbSwitch = XFALSE;
                    XOS_CliExtPrintf(pCliEnv,
                                      "close print switch to telnet client(SID = %d)",
                                      MMISC_GetFsmId(pCliEnv) );
                }
                else
                {
                    XOS_CliExtWriteStrLine( pCliEnv, "print_direct           -- display print direction list\r\nprint_direct on/off  -- open/close print direct to this telnet client SID\r\nprint_direct <SID> on/off  -- open/close print direct to given telnet client SID\r\n" );
                }
            }
        }
        return;
    }
    else if ( 3 == siArgc )
    {
        s32Value = 0;

        XOS_StrToNum( ppArgv[ 1 ], (XU32 *)&s32Value );
        if ( 1 > s32Value || kCLI_MAX_CLI_TASK < s32Value )
        {
            XOS_CliExtPrintf(pCliEnv,"SID: %d error\r\n", s32Value );
            return ;
        }

        if ( !XOS_StrNcmp( ppArgv[2], "on", 2 ) )
        {
            for ( i = 0; i < kCLI_MAX_CLI_TASK; i++ )
            {
                if ( s32Value == g_paUserSIDLogined[i].xs32SID )
                {
                    g_paUserSIDLogined[i].xbSwitch = XTRUE;
                    XOS_CliExtPrintf(pCliEnv,
                                      "open print switch to telnet client(SID = %d).",
                                      s32Value );
                }
            }
        }
        else if ( !XOS_StrNcmp( ppArgv[2], "off", 2 ) )
        {
            for ( i = 0; i < kCLI_MAX_CLI_TASK; i++ )
            {
                if ( s32Value == g_paUserSIDLogined[i].xs32SID )
                {
                    g_paUserSIDLogined[i].xbSwitch = XFALSE;
                    XOS_CliExtPrintf(pCliEnv,
                                      "close print switch to telnet client(SID = %d)",
                                      s32Value );
                }
            }
        }
        else
        {
            XOS_CliExtWriteStrLine( pCliEnv, "print_direct           -- display print direction list\r\nprint_direct on/off  -- open/close print direct to this telnet client SID\r\nprint_direct <SID> on/off  -- open/close print direct to given telnet client SID\r\n" );
        }
        return;
    }
    else
    {
        XOS_CliExtWriteStrLine( pCliEnv, "print_direct           -- display print direction list\r\nprint_direct on/off  -- open/close print direct to this telnet client SID\r\nprint_direct <SID> on/off  -- open/close print direct to given telnet client SID\r\n" );
    }

    return;
}

/*****************************************************
 以下是技术原型的验证代码,通过编译宏屏蔽
******************************************************/
#ifdef XOS_TELNETS

#ifdef XOS_WIN32
#define close(s)  closesocket(s)
#endif

#define MAX_NAME_LEN  1024  /* Maximum string name length*/
#define FD_SETSIZE1   (kCLI_MAX_CLI_TASK + 1) /*为了使下标与SessionID一致*/

struct SERVER_DS
{
    XS32 maxi;
#ifdef XOS_WIN32
    SOCKET listenfd;
    SOCKET maxfd;
#else
    XS32 listenfd;
    XS32 maxfd;
#endif
    XS32  nready;

    XS32  client[ FD_SETSIZE1 ];

    XU16  maxTelClients;  /*支持的最大的Telnet Client Connection 数量*/
    XU32  serverip;       /*监听IP*/
    XU16  port;           /*监听端口*/

    fd_set read_set;
    fd_set all_set;

    char buf[MAX_NAME_LEN];
    struct sockaddr_in servaddr;
} tel_ds;

XVOID TelnetSendMsg2CLI( XS32 u32SubID, XU16 msgID, XCHAR* pData, XU32 u32Len );
XSTATIC XVOID TelnetServerFunc( XVOID* taskNo );

/************************************************************************
 函数名: XOS_TelnetInitProc(  )
 功能:
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XS8 XOS_TelnetSInitProc( )
{
    XS32 ret       = 0;
    t_XOSTASKID   taskid;
    t_TelnetDCfg  telCfg;

#ifndef XOS_EW_START
    if( !XML_GetTelDCfgFromXosXml( &telCfg , "xos.xml") )
#else
    if( !XML_GetTelDCfgFromXosXml( &telCfg , XOS_CliGetXmlName( )) )
#endif
    {
        #ifdef CLI_LOG
        cli_log_write( eCLIDebug, "ERROR: get telnet cfg error !");
    #endif
        return XERROR;
    }

    tel_ds.serverip      = telCfg.ip;
    tel_ds.maxTelClients = telCfg.maxTelClients ;
    tel_ds.port          = telCfg.port;

    if ( kCLI_MAX_CLI_TASK < tel_ds.maxTelClients )
    {
        #ifdef CLI_LOG
        cli_log_write( eCLIError,
                            "TELNETS_ERROR: xos.xml configure, client connection num %d is so bigger than %d!\r\n",
                            tel_ds.maxTelClients,
                            kCLI_MAX_CLI_TASK );
        #endif

        return XERROR;
    }

    /*创建任务*/
    ret = XOS_TaskCreate(   "tTelnetS" ,
                             TSK_PRIO_NORMAL,
                             60000,
                             TelnetServerFunc,
                             XNULL,
                             &taskid );

    if ( ret != XSUCC )
    {
        #ifdef CLI_LOG
        cli_log_write( eCLIError, "create task named \"CLI Telnet Server\" failed!" );
        #endif
        return XERROR;
    }
    #ifdef CLI_LOG
    cli_log_write( eCLINormal, "Task \"CLI Telnet Server\" init successfully!" );
    #endif

    return XSUCC;
}

/************************************************************************
 函数名: XOS_TelnetServerSend2Client(  )
 功能:
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XS8 TelnetServerSend2Client( XU16   uSubID, XCHAR* pData, XU32 u32Len )
{
#ifdef XOS_WIN32
    SOCKET    sockfd  = 0;
#else
    XS32      sockfd  = 0;
#endif
    XS32      nwriten = 0;
    XS32      nleft   = 0;
    XS8       *ptr    = XNULL;

    if ( 0 >= u32Len  || XNULL == pData )
    {
        #ifdef CLI_LOG
        cli_log_write( eCLIDebug, "XOS_TelnetServerSend2Client error!" );
        #endif
        return XERROR;
    }

    if ( 1 > uSubID || kCLI_MAX_CLI_TASK < uSubID  )
    {
        #ifdef CLI_LOG
        cli_log_write( eCLIDebug, "cli sessin id error!" );
        #endif
        return XERROR;
    }

    if ( ( sockfd = tel_ds.client[uSubID] ) < 0 )
    {
        #ifdef CLI_LOG
        cli_log_write( eCLIDebug, "get telnet client socket des from tel_ds.client[] error!" );
        #endif
        return XERROR;
    }

    /*要发送多次才能发送完毕*/
    ptr   = pData;
    nleft = u32Len;
    while ( nleft > 0 )
    {
        if ( ( nwriten = send( sockfd, ptr, nleft, 0 ) ) == 0 )
        {
            #ifdef CLI_LOG
            cli_log_write( eCLIDebug,
                           "send( sockfd:%d, pData: %s, nleft:%d, 0 ) error!",
                           sockfd,
                           ptr,
                           nleft );
            #endif

            nwriten = 0;

            return XERROR;
        }

        nleft -= nwriten;
        ptr   += nwriten;
    }

    return XSUCC;
}

XSTATIC XVOID TelnetServerFunc( XVOID* taskNo )
{
    struct sockaddr_in cliaddr;
#ifdef XOS_WIN32
    SOCKET connfd = 0;
    SOCKET sockfd = 0;
#else
    XS32 connfd = 0;
    XS32 sockfd = 0;
#endif
    XS32    i = 0;
    XS32    clilen = 0;
    XS32    n = 0;

#ifdef XOS_WIN32
    WORD wVersionRequested;
    WSADATA socket_data;

    wVersionRequested = MAKEWORD(1, 1);
    if( 0 != WSAStartup(wVersionRequested, &socket_data))
    {
        return ;
    }
#endif

    memset( tel_ds.buf, 0x00, MAX_NAME_LEN );
    tel_ds.listenfd = socket( AF_INET, SOCK_STREAM, 0 );

#ifdef XOS_WIN32
    /* The SOCKET type is unsigned in the WinSock library */
    if( tel_ds.listenfd == INVALID_SOCKET)
#else
    if( tel_ds.listenfd < 0 )
#endif
    {
        return ;
    }

    memset( &tel_ds.servaddr, 0x00, sizeof( tel_ds.servaddr ) );
    tel_ds.servaddr.sin_family      = AF_INET;
    tel_ds.servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
    tel_ds.servaddr.sin_port        = htons( tel_ds.port );

#ifdef XOS_WIN32
    if ( SOCKET_ERROR == bind( tel_ds.listenfd,
                               (struct sockaddr*)&tel_ds.servaddr,
                               sizeof(tel_ds.servaddr) ) )
#else
    if ( 0 > bind( tel_ds.listenfd,
                               (struct sockaddr*)&tel_ds.servaddr,
                               sizeof(tel_ds.servaddr) ) )
#endif
    {
        #ifdef CLI_LOG
        cli_log_write( eCLIError, "bind() failed. port(%d) is used.", tel_ds.port );
        #endif

        close( sockfd );
        return;
    }

#ifdef XOS_WIN32
    if ( SOCKET_ERROR == listen( tel_ds.listenfd, 8 ) )
#else
    if ( 0 > listen( tel_ds.listenfd, 8 ) )
#endif
    {
        #ifdef CLI_LOG
        cli_log_write( eCLIError, "listen() failed." );
        #endif
        close( sockfd );
        return;
    }

    tel_ds.maxfd = tel_ds.listenfd;
    tel_ds.maxi  = -1;
    for( i = 0; i < FD_SETSIZE1; i++ )
    {
        tel_ds.client[i] = -1;
    }

    FD_ZERO( &tel_ds.read_set );
    FD_SET( tel_ds.listenfd, &tel_ds.all_set );

    for( ; ; )
    {
        tel_ds.read_set = tel_ds.all_set;

        /*return the num of read*/
        tel_ds.nready = select( tel_ds.maxfd + 1,
                                &(tel_ds.read_set),
                                0,
                                0,
                                0 );

//#if XOS_WIN32
#ifdef XOS_WIN32
        Sleep( 1 );
#endif

        if ( FD_ISSET( tel_ds.listenfd, &tel_ds.read_set ) )
        {
            clilen = sizeof( cliaddr );
            connfd = accept( tel_ds.listenfd, (struct sockaddr*)&cliaddr, &clilen );
            if ( connfd == -1 )
            {
                /*if accept socket faild,then the whole cli service stoped,can not accept new clients*/
                break;
            }

            for( i = 1; i <= tel_ds.maxTelClients; i++ )
            {
                /*find a pos and set it for new connect*/
                if( tel_ds.client[i]  < 0 )
                {
                    tel_ds.client[i] = connfd;
                    break;
                }
            }

            if ( tel_ds.maxTelClients < i )
            {
                if ( 0 == send( connfd,
                                 "too many clients.",
                                 strlen( "too many clients." ) + 1,
                                 0
                               ) )
                {
                    #ifdef CLI_LOG
                    cli_log_write( eCLIDebug, "send  too many clients error!" );
                    #endif
                }

                close( connfd );
                continue;
            }
        printf("new tcp client accept connected %d\n",connfd);

    #ifdef CLI_LOG
            cli_log_write( eCLIDebug, "new client connected!" );
    #endif

            FD_SET( connfd, &tel_ds.all_set );
            TelnetSendMsg2CLI( i, MSG_IP_CONNECTIND, "", 1 );

            if ( connfd > tel_ds.maxfd )
            {
                tel_ds.maxfd = connfd;
            }

            if ( i > tel_ds.maxi )
            {
                tel_ds.maxi = i;
            }

            if ( --tel_ds.nready <= 0 )
            {
                continue;
            }
        }

        for( i = 1; i <= tel_ds.maxi; i++ )
        {
            if ( ( sockfd = tel_ds.client[i] ) < 0 )
            {
                continue;
            }

            if ( FD_ISSET( sockfd, &(tel_ds.read_set) ) )
            {
                memset( tel_ds.buf, 0x00, MAX_NAME_LEN );
                if ( ( n = recv( sockfd, tel_ds.buf, MAX_NAME_LEN, 0 ) ) <= 0 )
                {
                    TelnetSendMsg2CLI( i, MSG_IP_CLOSEIND, "", 1);

                    /* connection closed by client */
                    close( sockfd );

                    FD_CLR( sockfd, &(tel_ds.all_set) );
                    tel_ds.client[i] = -1;
    #ifdef CLI_LOG
                    cli_log_write( eCLIDebug, "client closed:sid:%d.", i );
    #endif
                }
                else
                {
                    TelnetSendMsg2CLI( i, MSG_IP_DATAIND, tel_ds.buf, n );
                }

                if ( --tel_ds.nready <= 0 )
                {
                    break;
                }
            }
        }
    }

    close( sockfd );
#ifdef XOS_WIN32
        WSACleanup();
#endif

    return ;
}

void TelnetSendMsg2CLI( XS32 u32SubID, XU16 msgID, XCHAR* pData, XU32 u32Len )
{
    t_XOSCOMMHEAD* pTelnetMsg      = XNULLP;
    XS32 i = 0;

    pTelnetMsg = XOS_MsgMemMalloc( FID_CLI, u32Len*sizeof(XCHAR) );
    if ( XNULLP == pTelnetMsg )
    {
        return ;
    }
    pTelnetMsg->datasrc.FID    = FID_CLI;
    pTelnetMsg->datasrc.FsmId  = 0;
    pTelnetMsg->datasrc.PID    = XOS_GetLocalPID();

    pTelnetMsg->datadest.FID   = FID_CLI;
    pTelnetMsg->datadest.FsmId = 0;
    pTelnetMsg->datadest.PID   = XOS_GetLocalPID();
    pTelnetMsg->prio           = eAdnMsgPrio;
    pTelnetMsg->msgID          = msgID;
    pTelnetMsg->subID          = u32SubID;

    if ( MSG_IP_DATAIND != msgID )
    {
        XOS_MemCpy( (char*)pTelnetMsg->message, "", u32Len );
    }
    else
    {
        XOS_MemCpy( (char*)pTelnetMsg->message, pData, u32Len );
    }

    if ( XSUCC != XOS_MsgSend( pTelnetMsg ) )
    {
        /*出错处理*/
        XOS_MsgMemFree( FID_CLI, pTelnetMsg );
    }

    return ;
}

#endif /**/

/************************************************************************
 函数名: cli_sendTelnetCloseReq(  )
 功能:   向 Telnet 客户端 发送关闭连接消息
 输入:
 输出:   无
 返回:   成功返回XSUCC/失败返回XERROR
 说明:
************************************************************************/
XSTATIC XVOID cli_sendTelnetCloseReq( XS32 connId )
{
#ifdef XOS_TELNETS
#ifdef XOS_WIN32
    SOCKET    sockfd  = 0;
#else
    XS32      sockfd  = 0;
#endif
#endif

    if ( 1 > connId || kCLI_MAX_CLI_TASK < connId )
    {
        #ifdef CLI_LOG
        cli_log_write( eCLIDebug, "cli_sendTelnetCloseReq() param invalidate!" );
        #endif
        return;
    }
    /*send close req*/

#ifdef XOS_TELNETS
    sockfd = tel_ds.client[connId];
    if ( -1 != sockfd )
    {
        close( sockfd );

        FD_CLR( sockfd, &(tel_ds.all_set) );
        tel_ds.client[connId] = -1;
    }
#endif

/*  COMMON_MESSAGE stMsg;*/

    /*
    stMsg.stHeader.stReceiver.ucModId = stSysData.ucModId;
    stMsg.stHeader.stReceiver.ucFId   = FID_NTL;
    stMsg.stHeader.stReceiver.usFsmId = connId + (IP_FSM_CLI_FIRST_CLIENT - 1);;

    stMsg.stHeader.stSender.ucModId   = stSysData.ucModId;
    stMsg.stHeader.stSender.ucFId     = FID_OAM;
    stMsg.stHeader.stSender.usFsmId   = BLANK_USHORT;

    stMsg.stHeader.usMsgId    = MSG_IP_CLOSEREQ;

    stMsg.stHeader.usMsgLen   = 0;
    memcpy(stMsg.ucBuffer, pBuf, BufLen);

    if(stMsg.stHeader.stReceiver.usFsmId)
    SYS_MSGSEND(&stMsg);*/

}

/***************************************************
                关闭所有重定向
                deleted by lixiangni 2007.1.6

****************************************************/

#if 0
XS32  XOS_TurnOffAllRedirection()
{
   XU32 i = 0;
 for ( i = 0; i <= kCLI_MAX_CLI_TASK ; i++ )
    {
    g_paUserSIDLogined[i].xbSwitch = XFALSE ;
    }
 return XSUCC ;
}

#endif
#ifdef XOS_VXWORKS
XVOID setOutputForVxworks(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{

    if ( 1 == siArgc)
    {
        /*没有参数输出则显示当前该开关的值*/
        XOS_CliExtPrintf(pCliEnv,"-----------------------------------------");
        XOS_CliExtPrintf(pCliEnv,"outputforvx:%d.",g_OutputForVxworks);
        XOS_CliExtPrintf(pCliEnv,"-----------------------------------------\r\n");
        return ;
    }
    if ( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter number is wrong\r\n");
        return;
    }

    if ( !XOS_StrNcmp( ppArgv[1], "on", 2 ) )
    {
        g_OutputForVxworks = XTRUE ;
        XOS_CliExtPrintf(pCliEnv,"open output to vxworks console terminal switch\r\n");
        return ;
    }
    else if ( !XOS_StrNcmp( ppArgv[1], "off", 2 ) )
    {    g_OutputForVxworks = XFALSE ;
        XOS_CliExtPrintf(pCliEnv,"close output to vxworks console terminal switch\r\n");
        return ;
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"usage: command ? for detail\r\n");
    }
    /*
    lflag = atoi(ppArgv[1]);
    if (1 != lflag && 0 != lflag )
    {
    XOS_CliExtPrintf(pCliEnv,"input parameter switch %d  is wrong",lflag);
    return;
    }
    */

    return ;
}
XS32 XOS_CloseVxPrint()
{
   g_OutputForVxworks = XFALSE ;
   return XTRUE;
}
XS32 XOS_OpenVxPrint()
{
    g_OutputForVxworks = XTRUE ;
    return XTRUE;
}
#endif /*_vxworks_*/

#define REMOTE_CLI_CXF 
/*
@brief
@para flag 0-local 1-remote
*/
int cli_setRemoteCliRegFlag(int flag) //设置是否为远端CLI
{
    gRemoteShowCli = flag;
    return 0;
}

XVOID cli_RemoteExecCli(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XCHAR szRemoteCursor[CLI_MAX_ARG_LEN]= {0};
    XU8 *pMsgBuf = NULL;
    XU8 *pMsgTmp = NULL;
    XS32 nextStrLen = 0;
    CliFunc_getRemoteCliPath pGetRemoteCliPahtFunc = NULL;
    XU32 ulDstPidCnt;
    XU32 aulDstPidLst[MAX_CLI_REMOTE_PID_CN];
    
    t_XOS_CLI_MSG_HEAD *pCliMsgHead = NULL;
    XU32 cliMsgLen = 0;
    
    XU32 i = 0;
    XS32 j = 0;
#if 0    
    XOS_CliExtPrintf(pCliEnv,"Have send cmd<%s> to Dst Board,please wait a moment.\r\n",ppArgv[0]);
    for(i = 1; i < siArgc; i++)
    {
        XOS_CliExtPrintf(pCliEnv,"arg[%d]=%s, ",i,ppArgv[i]);    
    }
#endif    
    //check the pid in current path if valid
    if( 0xFFFFFFFF != pCliEnv->RemoteBid && NULL != gcliRegFunc.pChkPathPidFunc)
    {
       if(XSUCC !=  gcliRegFunc.pChkPathPidFunc(pCliEnv->pCli->cliShellCmd.pscCursor,pCliEnv->RemoteBid) )
       {
            XOS_CliExtPrintf(pCliEnv,"CliFunc_checkPathPid(%s,%d) err.",pCliEnv->pCli->cliShellCmd.pscCursor,pCliEnv->RemoteBid);
            return;
       }
    }
    // 
    if(NULL != gcliRegFunc.pGetRemoteCliPathFunc)
    {
        pGetRemoteCliPahtFunc = gcliRegFunc.pGetRemoteCliPathFunc;
    }
    else
    {
        pGetRemoteCliPahtFunc = (CliFunc_getRemoteCliPath)cli_getRemoteCursor;
    }
    
    if(XSUCC != pGetRemoteCliPahtFunc(pCliEnv->pCli->cliShellCmd.pscCursor,szRemoteCursor) )
    {
        XOS_CliExtPrintf(pCliEnv,"tran [%s] to  remote cur fail by func[%p].",pCliEnv->pCli->cliShellCmd.pscCursor,pGetRemoteCliPahtFunc);
        return;
    }
            
    cliMsgLen += sizeof(t_XOS_CLI_MSG_HEAD);
    cliMsgLen += (XU32)strlen(szRemoteCursor) + 1; // 1 mean last byte of the string

    for(j = 0; j < siArgc; j++)
    {
        cliMsgLen += (XU32)strlen(ppArgv[j]) + 1; // 1 mean last byte of the string
    }
    pMsgBuf = XOS_MemMalloc(FID_CLI,cliMsgLen);
    if(NULL == pMsgBuf)
    {
        XOS_CliExtPrintf(pCliEnv,"malloc %d bytes fail.",cliMsgLen);
        return;
    }
    pCliMsgHead = (t_XOS_CLI_MSG_HEAD *)pMsgBuf;
    //pCliMsgHead->pCliEnv = (void*)XOS_HtoNl((XU32)pCliEnv);
    pCliMsgHead->cliSessionId =  XOS_HtoNl(pCliEnv->pCli->fsmId);
    
    pCliMsgHead->cmdSeqNo = XOS_HtoNl(++pCliEnv->stCliMsgHead.cmdSeqNo); //
    pCliMsgHead->telnetIp = XOS_HtoNl(pCliEnv->IpAddr);
    pCliMsgHead->siArgc = XOS_HtoNl(siArgc); //
    //copy the clipath ,cli_name ,argument
    pMsgTmp = pMsgBuf;
    pMsgTmp = pCliMsgHead->buff; // fisrt field is path
    nextStrLen = (XS32)strlen(szRemoteCursor) + 1;
    memcpy(pMsgTmp,szRemoteCursor,nextStrLen);
    pMsgTmp += nextStrLen;
    for(j=0; j < siArgc; j++)
    {
        nextStrLen = (XS32)strlen(ppArgv[j]) + 1;
        memcpy(pMsgTmp,ppArgv[j],nextStrLen);
        pMsgTmp += nextStrLen;
    }
    // get Dest Pid List
    if(pCliEnv->RemoteBid == 0xFFFFFFFF)
    {
    if(NULL != gcliRegFunc.pGetPidListFunc)
    {
        gcliRegFunc.pGetPidListFunc((XS32)strlen(pCliEnv->pCli->cliShellCmd.pscCursor)+1, pCliEnv->pCli->cliShellCmd.pscCursor
            , (int*)&ulDstPidCnt, aulDstPidLst);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"pGetPidListFunc did not reg.");
        return;
    }
    }
    else // use DstPid that user seted.
    {
        ulDstPidCnt  = 1;
        aulDstPidLst[0] = pCliEnv->RemoteBid;
    }
    if(0 == ulDstPidCnt)
    {
        XOS_CliExtPrintf(pCliEnv,"dstPidCnt =0.");
        return;
    }
    // send msg to remote dst pid
    for(i = 0; i <ulDstPidCnt; i++)
    {        
        XOS_CliExtPrintf(pCliEnv,"\r\nSend cmd<%s> to DstPID(0x%x),wait a moment please.\r\n",ppArgv[0],aulDstPidLst[i]);
        Cli_SendMsgByBuff(pMsgBuf,cliMsgLen,MSG_REMOTE_CLI_REQ,FID_CLI, FID_CLI, 0,aulDstPidLst[i] );
    }
    //cli_ExecuteFullPathCmd(0,szRemoteCursor,siArgc,ppArgv); // test code
    XOS_MemFree(FID_CLI,pMsgBuf);
    return;
}

XS32 cli_getRemoteCursor(const XCHAR *szLocalCursor,XCHAR *szRemoteCursor)
{
    XS32 i = 0;
    XS32 firstPos =0;
    XS32 secondPos = 0;
    XS32 lenTmp = 0;

    lenTmp = (XS32)strlen(szLocalCursor);
    for(i =0; i < lenTmp; i++)
    {
        if('>' == szLocalCursor[i])
        {
            firstPos = i+1;
            break;
        }
    }
    //
    if(0 != i)
    {
        for(i = firstPos; i < lenTmp; i++)
        {
            if('>' == szLocalCursor[i])
            {
                secondPos  = i+1;
                break;
            }
        }
    }
    else
    {
        return XERROR;
    }

    if(0 == secondPos)
    {
        return XERROR;
    }

    strncpy(szRemoteCursor,szLocalCursor,firstPos);
    strcat(szRemoteCursor,&szLocalCursor[secondPos]);
    return XSUCC;
}

XS32 cli_ExecuteFullPathCmd(CLI_ENV *pCliEnv,XCHAR *szCliPath,XS32 siArgc,XCHAR *argv[])
{
    XCHAR *szCmdName = argv[0];
    XCHAR *pscTmpName = NULL;

    XS32 i = 0;
    XU32 cliNameLen = 0;
    XS32 nNewMode = 0;
    t_XOS_CMD_DATAS *pCmdLst = NULL;
    
    //find cmdHander
    for( nNewMode = 0; nNewMode < CLI_MAX_CMD_CLASS; nNewMode++ )
    {
        if( XNULL == g_paCliAppMode[nNewMode].pModePCmds )
        {
            continue;
        }
        // parse the cursor
        if(0 == strcmp(szCliPath,g_paCliAppMode[nNewMode].pscCurSor) )
        {
            break;
        }
    }    

    if(nNewMode >= CLI_MAX_CMD_CLASS) // not find
    {
        XOS_Trace(MD(FID_CLI,PL_ERR),"not find %s cursor.",szCliPath);
        goto ErrProc;
    }
    // pointer the prompt's cmdList
    pCmdLst = g_paCliAppMode[nNewMode].pModePCmds;
    //find cmdName
    cliNameLen = (XU32)strlen(argv[0]);
    for( i = 0; i < CLI_MAX_CMD_NUM; i++ )
    {
        if( XNULL ==pCmdLst[i].pCmdHandler )
        {
            continue;
        }

        pscTmpName = pCmdLst[i].pscCmdName;

        /* 完整命令比较查找符合*/
        if( 0== strncmp( pscTmpName, argv[0],cliNameLen ) )
        {
            //CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx = i;
            break;
        }
    }    

    if(i >= CLI_MAX_CMD_NUM) // not find
    {
        XOS_Trace(MD(FID_CLI,PL_ERR),"not found %s%s cmd",szCliPath,szCmdName);
        goto ErrProc;
    }
    //execute cmd
    pCmdLst[i].pCmdHandler(pCliEnv,siArgc,argv);
    return XSUCC;
ErrProc:    
    return XERROR;
}

XS32 cli_SendCliRspMsg(t_XOS_GEN_ENVIRON *pEnv, XCHAR *pBuf, XS32 BufLen)
{
    XCHAR aucBrdInfo[CLI_PRE_FIX_LEN] = {0};
    XU8 *pmsgBuf = NULL;
    t_XOS_CLI_MSG_HEAD *pCliMsgHead = NULL;
    XU32 msgLen = 0;

    //sprintf(aucBrdInfo,"cmd'%s%s' from brd",pEnv->pCli->cliShellCmd.pscCursor+2,pEnv->pCli->cliShellCmd.cmdArg.pscArgval[0]);
    strncpy(aucBrdInfo,gszPrefixInfo,CLI_PRE_FIX_LEN);
    aucBrdInfo[CLI_PRE_FIX_LEN-1] = 0;
    //
    msgLen = sizeof(t_XOS_CLI_MSG_HEAD) + (XU32)strlen(aucBrdInfo) + BufLen + 1;

    pmsgBuf = XOS_MemMalloc(FID_CLI,msgLen);
    //
    if(NULL == pmsgBuf)
    {
        XOS_Trace(MD(FID_CLI,PL_ERR),"malloc %d bytes fail.",msgLen);
        return XERROR;
    }
    pCliMsgHead = (t_XOS_CLI_MSG_HEAD*)pmsgBuf;
    memcpy(pCliMsgHead,&pEnv->stCliMsgHead,sizeof(t_XOS_CLI_MSG_HEAD) );
    strcpy((char*)pCliMsgHead->buff,aucBrdInfo);
    strcat((char*)pCliMsgHead->buff,pBuf);
    //send cli msg rsp to Peer 
    Cli_SendMsgByBuff(pmsgBuf, msgLen, MSG_REMOTE_CLI_RSP, FID_CLI,FID_CLI, 0, pEnv->RemoteBid);
    //XOS_Trace(MD(FID_CLI,PL_LOG),"send msg<%s> to IPC.\r\n",pBuf);
    XOS_MemFree(FID_CLI,pmsgBuf);
    return 0;
}

//#define MAX_CLI_STR_BUF_LEN (1024*7)
#define MAX_CLI_ARG_CNT 100
XS32 cli_RecvRemoteCliReq(t_XOSCOMMHEAD *pMsg)
{
    static t_XOS_GEN_ENVIRON * pCliEnv = NULL;
    static t_XOS_CLI_INFO   *pCliTmp = NULL;    
    static XCHAR *pcliStrBuf = NULL;
    
    t_XOS_CLI_MSG_HEAD *pcliMsg = NULL;
    XS32 siArgc = 0;
    XCHAR * argv [MAX_CLI_ARG_CNT] = {0};
    XCHAR *pCharTmp = NULL;
    XCHAR *szCliPath = NULL;
    XS32 i = 0;
    XS32 strLenTmp = 0;
    char *szDefaultPara ="errpara"; //
    
    pcliMsg = (t_XOS_CLI_MSG_HEAD *)pMsg->message;
    siArgc = XOS_NtoHl(pcliMsg->siArgc);
    if(siArgc >= sizeof(argv))
    {
        XOS_PRINT(MD(FID_CLI,PL_ERR),"too many argument num(%d).",siArgc);
        return XERROR;
    }

    for(i = 0; i< MAX_CLI_ARG_CNT; i++) // 
    {
        argv[i] = szDefaultPara;
    }
    //create a nwe pCliEnv ,so use it to save the cli info
    if(NULL == pCliEnv)
    {
        pCliEnv = XOS_MemMalloc(FID_CLI, sizeof(CLI_ENV));
    }
    
    if(NULL  == pCliEnv)
    {
        XOS_Trace(MD(FID_CLI,PL_ERR),"malloc %d bytes fail.",sizeof(CLI_ENV));
        return XERROR;
    }    
    //
    if(NULL  == pCliTmp)
    {
        pCliTmp = XOS_MemMalloc(FID_CLI, sizeof(t_XOS_CLI_INFO) );
    }
    //
    if(NULL == pCliTmp)
    {
        XOS_Trace(MD(FID_CLI,PL_ERR),"malloc %d bytes fail.",sizeof(t_XOS_CLI_INFO) );
        return XERROR;
    }
    else
    {
        pCliEnv->pCli = pCliTmp;
        MCONN_SetWriteHandle(pCliEnv, (WriteHandle*)cli_SendCliRspMsg ); // set the output handler                
    }

    //
    pCliEnv->pStrBuf = NULL;
    if(NULL == pcliStrBuf)
    {
        pcliStrBuf = XOS_MemMalloc(FID_CLI,MAX_BUFF_LEN+1);
    }
    //
    if(NULL  == pcliStrBuf)
    {
        XOS_Trace(MD(FID_CLI,PL_ERR),"malloc %d bytes fail.",MAX_BUFF_LEN+1);
        return XERROR;
    }
    pCliEnv->pStrBuf = pcliStrBuf;
    // decode remote cli msg
    //...    
    //save srcBrd's
    pCliEnv->RemoteBid = pMsg->datasrc.PID; //
    // save mpu brd's cli info
    memcpy(&pCliEnv->stCliMsgHead,pcliMsg,sizeof(t_XOS_CLI_MSG_HEAD) );
    //
    //decode cliPath
    pCharTmp = (char*)pcliMsg->buff;
    szCliPath = pCharTmp;
    strLenTmp = (XS32)strlen(szCliPath) +  1;
    pCharTmp += strLenTmp;
    //save cursor
    strncpy(pCliEnv->pCli->cliShellCmd.pscCursor,szCliPath,CLI_MAX_ARG_LEN);
    pCliEnv->pCli->cliShellCmd.cmdArg.siArgcnt = siArgc;
    // decode argument
    for(i = 0;  i < siArgc; i++)
    {
        argv[i] = pCharTmp; 
        // save argument to pClienv
        strncpy(pCliEnv->pCli->cliShellCmd.cmdArg.pscArgval[i],argv[i],CLI_MAX_ARG_LEN);
        strLenTmp = (XS32)strlen(argv[i]) + 1;
        if(strLenTmp > 256)
        {
            XOS_PRINT(MD(FID_CLI,PL_ERR),"arg string too long.");
            return XERROR;
        }
        pCharTmp += strLenTmp;
    }
    XOS_Trace(MD(FID_CLI,PL_DBG),"recv cli req' %s%s ',execute it.",szCliPath+2,argv[0]); // 2 use for cut the \r\n
    cli_ExecuteFullPathCmd(pCliEnv, szCliPath, siArgc, argv);        
    return XSUCC;
}

XS32 cli_RecvRemoteCliRsp(t_XOSCOMMHEAD *pMsg)
{
    t_XOS_CLI_MSG_HEAD *pCliMsgHead = NULL;
    t_XOS_GEN_ENVIRON * pCliEnv = NULL;
    XCHAR *pBuf = NULL;
    XS32 s32SId  = 0;
    XU32 cmdSeqNo =0;
    
    XOS_Trace(MD(FID_CLI,PL_DBG),"recv cli rsp,print it.");
    pCliMsgHead = (t_XOS_CLI_MSG_HEAD *)pMsg->message;
    //pCliEnv = (t_XOS_GEN_ENVIRON *)pCliMsgHead->pCliEnv;
    s32SId = XOS_NtoHl(pCliMsgHead->cliSessionId);
    cmdSeqNo = XOS_NtoHl(pCliMsgHead->cmdSeqNo);
    
    pCliEnv = cli_telnetGetCliSession( s32SId );
    if( XNULL == pCliEnv )
    {
        XOS_Trace(MD(FID_CLI, PL_ERR), "ERROR:  XNULL == pCliEnv Sid=%d!",s32SId);
        return XERROR;
    }    
    //
    if(kTELNET_NOT_LOGIN == pCliEnv->pCli->loginStat)
    {
        XOS_Trace(MD(FID_CLI,PL_ERR),"rev cli rsp ,but telnet session not exists.");    
        return XSUCC;
    }
    // judget the cmdSeqNo
    if(pCliEnv->stCliMsgHead.cmdSeqNo != cmdSeqNo)
    {
        ;
    }
    pBuf = (char*)pCliMsgHead->buff;
    XOS_CliExtPrintf(pCliEnv,"%s",pBuf);
    return XSUCC;
}

XS32 XOS_CliExtFlush( CLI_ENV* pCliEnv);
/************************************************************************
 函数名: XOS_CliExtPrintf(  )
 功能:   带格式的输出函数到CLI BUFF
 输入:   pCliEnv
         pFmt 格式话字符串
 输出:   无
 返回:   无
 说明:   无
 20110107 cxf add
************************************************************************/
XS32 XOS_CliExtPrintf2Buff( CLI_ENV* pCliEnv, XCHAR* pFmt, ...)
{
    XCHAR *buf = XNULLP;
    va_list ap;
    XS32 state = 0;
    XS32 fmt_len=0;

    if ( XNULL == pFmt || XNULL == pCliEnv )
    {
        return XERROR;
    }
    if(XNULL == (buf = XOS_MemMalloc(FID_CLI, MAX_BUFF_LEN+1)) )
    {
        return XERROR;
    }
    va_start(ap,pFmt);
    fmt_len=XOS_VsPrintf(buf, MAX_BUFF_LEN+1, pFmt, ap);
    va_end(ap);
    if(fmt_len < 0)
    {
       XOS_Trace(MD(FID_CLI,PL_ERR),"print2buff err.");
       return XERROR;
    }
    if(fmt_len < 0)
    {
       XOS_MemFree(FID_CLI, buf);
       return XERROR;
    }
    buf[MAX_BUFF_LEN] = '\0';    
    // if the bufflen is overload ,flush it.
    if(strlen(pCliEnv->pStrBuf) + strlen(buf) > MAX_BUFF_LEN)
    {
        XOS_Trace(MD(FID_CLI,PL_LOG),"cli str buf overload,flust it.");
        XOS_CliExtFlush(pCliEnv);
    }
    strcat(pCliEnv->pStrBuf,buf);
    if(XERROR == XOS_MemFree(FID_CLI, buf))
    {
        return XERROR;
    }    
    pCliEnv->pStrBuf[MAX_BUFF_LEN] = '\0';
    //state = cli_extWrite( pCliEnv, buf, (XS32)XOS_StrLen(buf) );    
    return state;
}

/*
20110107 cxf add
*/
XS32 XOS_CliExtFlush( CLI_ENV* pCliEnv)
{
    XS32 state = XSUCC;
    if (XNULL == pCliEnv )
    {
        return XERROR;
    }
    state = cli_extWrite( pCliEnv, pCliEnv->pStrBuf, (XS32)XOS_StrLen(pCliEnv->pStrBuf) );    
    pCliEnv->pStrBuf[0] = 0; //
    return state;
}

/*
*20110215 cxf add
*@brief remote cli func register
*/
XS32 Cli_remoteCliFuncReg(XU32 funcId, void * funcAddr)
{
    switch(funcId)
    {
        case CLI_FUNCID_GETPIDLIST:
            gcliRegFunc.pGetPidListFunc = funcAddr;
            break;
        case CLI_FUNCID_GET_REMOTE_CLIPATH:
            gcliRegFunc.pGetRemoteCliPathFunc = funcAddr;
            break;
        case CLI_FUNCID_CHECK_PID_PATH:
            gcliRegFunc.pChkPathPidFunc = funcAddr;
            break;
        default:
            return XERROR;
    }
    return XSUCC;
}

/*
*20110215 cxf add
*@brief set cli prefix info
*/
char * Cli_SetCliPrefixInfo(char *szPrefixInfo)
{
    strncpy(gszPrefixInfo,szPrefixInfo,CLI_PRE_FIX_LEN);
    return gszPrefixInfo;
}

/*
*@brief send msg to remote dest Pid
*/
XS32 Cli_SendMsgByBuff(XVOID *buff,XU32 msg_len,XU32 msgId,XU32 srcFid,XU32 dstFid,XU32 dstFsmId,XU32 dstPid)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    
    pMsg = XOS_MsgMemMalloc(FID_CLI, msg_len );
    if ( pMsg == NULL )
    {
        XOS_PRINT(MD(FID_CLI,PL_ERR),"DCS_SendMsgByBuff failed,XOS_MsgMemMalloc error.");
        return XERROR;
    }
    pMsg->datasrc.FID = srcFid;
    pMsg->datadest.FID = dstFid;
    pMsg->datadest.FsmId = dstFsmId;
    
    pMsg->datasrc.PID  = XOS_GetLocalPID();
    pMsg->datadest.PID = dstPid;
    pMsg->prio            = eNormalMsgPrio;
    pMsg->msgID        = msgId;
    pMsg->length = msg_len;
    XOS_MemCpy(pMsg->message, buff, msg_len);
    //MNT_DcsMsgProc(pMsg,0xFFFFFFFF,NULL,0);

     if( XOS_MsgSend(pMsg) != XSUCC)
     {
         XOS_PRINT(MD(FID_CLI,PL_ERR),"SYS_SendMsgByBuff failed,XOS_MsgSend fail.");
          XOS_MsgMemFree(pMsg->datasrc.FID, pMsg);
        return 2;
     }
    
    return 0;
}

/*
*20110215 cxf add
*@brief test ccode
*/
int cli_getRemotePidLst(int strLen, char* pcliPath, XU32* pidNum, XU32* pidList)
{
    XU32 i = 0;
    *pidNum = 1; //3;
    for(i = 0; i < *pidNum; i++)
    {
        pidList[i] = 3 - XOS_GetLocalPID();
    }
    return XSUCC;
}

XVOID Cli_setPeerPid(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    if(siArgc < 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }
    //pCliEnv->RemoteBid = atoi(ppArgv[1]);
    XOS_StrToNum(ppArgv[1], &pCliEnv->RemoteBid);
    XOS_CliExtPrintf(pCliEnv,"set cli dst pid =0x%x.",pCliEnv->RemoteBid);
    return;
}

XVOID cli_setRemotePid(CLI_ENV* pCliEnv ,XU32 Pid)
{
    pCliEnv->RemoteBid = Pid;
    return;
}

XVOID cli_showDstPid(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{ 
    XOS_CliExtPrintf(pCliEnv,"cli dst pid =0x%x.",pCliEnv->RemoteBid);
    return;
}

#define gcg
XU32   cli_getMcbPid(XVOID)
{
    //gMcbPid = 1;
    return gMcbPid;
}

XVOID  cli_setMcbPid(XU32 ulMcbPid)
{
     gMcbPid = ulMcbPid;
}

extern XS32 XOS_GetSysTime( t_XOSTB *timex );
extern XS32 XOS_SetSysTime( t_XOSTB *timex );
extern XS32 XOS_GetPhysicIP(XU32 *ipaddress);
extern XS32 XOS_GetLogicIP (XU32 * ipaddress);
extern XS32 XOS_AddLogicIP (XU32* ipaddress);
extern XS32 XOS_DeleteLogicIP(XU32 *ipaddress);
extern XS32 XOS_ModifyLogicIP(XU32 *ipaddress);
extern XS32 XOS_GetMv2IP(XU32 *ipaddress);
extern XS32 XOS_GetMv1IP(XU32 *ipaddress);


XVOID XOS_CliGetSysTime(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    t_XOSTB timex;
    
    if(siArgc >= 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }
    
    
    if(XSUCC != XOS_GetSysTime(&timex ))
    {
        XOS_CliExtPrintf(pCliEnv,"get failed.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"get succ.now is %d sysget result=%d",time(0),timex.time);
    }

{
    struct tm *ptr;
    char datestr[100] = {0};

    if(0 == (ptr = (struct tm *)localtime((time_t*)&timex.time)))
    {
        return;
    }

    memcpy(datestr,asctime(ptr),strlen(asctime(ptr))+1);

    XOS_CliExtPrintf(pCliEnv,"now is=%s\r\n",datestr);

    XOS_CliExtPrintf(pCliEnv,"now is tm_min%d hour:%d mday:%d mon:%d year:%d wday:%d yday:%d isdst:%d\r\n",
            ptr->tm_min,    /* minutes after the hour   - [0, 59] */
            ptr->tm_hour,   /* hours after midnight     - [0, 23] */
            ptr->tm_mday,   /* day of the month     - [1, 31] */
            ptr->tm_mon,    /* months since January     - [0, 11] */
            ptr->tm_year,   /* years since 1900 */
            ptr->tm_wday,   /* days since Sunday        - [0, 6] */
            ptr->tm_yday,   /* days since January 1     - [0, 365] */
            ptr->tm_isdst   /* Daylight Saving Time flag */);

}

    return;
}


XVOID XOS_CliSetSysTime(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    t_XOSTB timex;
    time_t now;
    
    if(siArgc >= 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }

    now = time(0);
    timex.time = 1300962116;//2011-03-24 18:22:01
    
    if(XSUCC != XOS_SetSysTime(&timex ))
    {
        XOS_CliExtPrintf(pCliEnv,"set failed.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"set succ.now is %d sysget result=%d",now,timex.time);
    }

#ifdef XOS_VXWORKS
{
    struct tm *ptr;
    char datestr[100] = {0};
    
    XOS_GetSysTime(&timex );
    
    if(0 == (ptr = (struct tm *)localtime((time_t*)&timex.time)))
    {
        return;
    }

    memcpy(datestr,asctime(ptr),strlen(asctime(ptr))+1);

    XOS_CliExtPrintf(pCliEnv,"now is=%s\r\n",datestr);

    XOS_CliExtPrintf(pCliEnv,"now is tm_min%d hour:%d mday:%d mon:%d year:%d wday:%d yday:%d isdst:%d\r\n",
            ptr->tm_min,    /* minutes after the hour   - [0, 59] */
            ptr->tm_hour,   /* hours after midnight     - [0, 23] */
            ptr->tm_mday,   /* day of the month     - [1, 31] */
            ptr->tm_mon,    /* months since January     - [0, 11] */
            ptr->tm_year,   /* years since 1900 */
            ptr->tm_wday,   /* days since Sunday        - [0, 6] */
            ptr->tm_yday,   /* days since January 1     - [0, 365] */
            ptr->tm_isdst   /* Daylight Saving Time flag */);

}
#endif
    return;
}

extern XS32 XOS_GetDevIPList ( const XS8 *pdev, t_LOCALIPINFO * pLocalIPList );
extern XS32 XOS_GetAllIPList ( t_LOCALIPINFO * pLocalIPList );
extern XS32 XOS_LogicIfConvToDevIf(const XS8 *ptLogicName,XS8 *pDevName,XS32 DevNamelen);
#define Inet_Aton(addr_int)\
    inet_ntoa((struct in_addr)(XOS_HtoNl(addr_int)))

XVOID XOS_ShowIpList(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv )
{
    XS8* pLogicIfName = NULL;
    XS8* strSearchIp = NULL;
    XS8  strDev[XOS_IFNAMESIZE] = {0};
    XS8  strIp[XOS_IPSTRLEN] = {0};    
    XS8  strIpPrint[XOS_IPSTRLEN] = {0};    
    XS8  strMaskPrint[XOS_IPSTRLEN] = {0};    
    t_LOCALIPINFO llist;
    t_LOCALIPINFO lAllList;
    XBOOL printall = XFALSE;
    t_BoardNeMaptInfo *ptNetInfo;
    XS32 i;

    if (!pCliEnv || !ppArgv)
    {
        return;
    }

    XOS_CliExtPrintf(pCliEnv,"\r\n--------------------------------------------------------\r\n");

    if (siArgc == 1)
    {
        printall = XTRUE;
    }
    else if (siArgc == 2)
    {
        if (0 == XOS_StrCmp("-a",ppArgv[1]))
        {
            printall = XTRUE;
        }
        else
        {
            XOS_CliExtPrintf(pCliEnv,
                "showiplist\r\n"
                        "\t\t -a              \t--show all dev ip list\r\n"
                        "\t\t -i devlogicname \t--show dev ip list\r\n"
                        "\t\t -p a.b.c.d      \t--show dev info match to this ip\r\n");

            return;
        }
    }
    else if (siArgc == 3)
    {
        if (0 == XOS_StrCmp("-i",ppArgv[1]))
        {
            pLogicIfName = ppArgv[2];
        }
        else if (0 == XOS_StrCmp("-p",ppArgv[1]))
        {
            strSearchIp = ppArgv[2];
            //XOS_StrNcpy(strSearchIp, ppArgv[2], sizeof(strSearchIp));
        }
        else
        {
            XOS_CliExtPrintf(pCliEnv,
                "showiplist\r\n"
                        "\t\t -a              \t--show all dev ip list\r\n"
                        "\t\t -i devlogicname \t--show dev ip list\r\n"
                        "\t\t -p a.b.c.d      \t--show dev info match to this ip\r\n");

            return;
        }
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,
            "showiplist\r\n"
                    "\t\t -a              \t--show all dev ip list\r\n"
                    "\t\t -i devlogicname \t--show dev ip list\r\n"
                    "\t\t -p a.b.c.d      \t--show dev info match to this ip\r\n");

        return;
    }


    if (pLogicIfName)
    {
        if (XSUCC != XOS_LogicIfConvToDevIf(pLogicIfName, strDev, sizeof(strDev)))
        {
            XOS_CliExtPrintf(pCliEnv, "convert Logic name[%s] to real dev name fail!\r\n",pLogicIfName);

            /* 全局变量已经初始化 */
            if (NULL == (ptNetInfo = (t_BoardNeMaptInfo*)XOS_GetNetMap()))
            {
                return;
            }
            
            XOS_CliExtPrintf(pCliEnv, "\rArgument must be:\r\n");
            for (i = 0; i < MAX_NETPORT_NUM; i++)
            {
                if (!ptNetInfo->tNetDevMap[i].valid)
                    continue;
                    
                XOS_CliExtPrintf(pCliEnv, "                   %s\r\n",ptNetInfo->tNetDevMap[i].LogicName);
            }            
            return;
        }

        llist.nIPNum = 0;
        /* 遍历设备上所有ip */
        if( XSUCC != XOS_GetDevIPList(strDev, &llist))
        {
            XOS_CliExtPrintf(pCliEnv, "get ip list fail!\r\n");
            return;
        }

        if (0 == llist.nIPNum)
        {
            XOS_CliExtPrintf(pCliEnv, "no ip on dev %s -- %s!\r\n",pLogicIfName,strDev);
            return;
        }
        for( i=0; i < llist.nIPNum ; i++ )
        {    
            XOS_IptoStr(llist.localIP[i].LocalIPaddr,strIpPrint);
            XOS_IptoStr(llist.localIP[i].LocalNetMask,strMaskPrint);
        
            XOS_CliExtPrintf(pCliEnv, "\r\n%-16s  ip:%-16s netmask:%-16s\r\n",llist.localIP[i].xinterface,
                strIpPrint,strMaskPrint);
        } 

    }
    else
    {
        lAllList.nIPNum = 0;
        if (XSUCC != XOS_GetAllIPList(&lAllList))
        {
            XOS_CliExtPrintf(pCliEnv, "get ip list fail!\r\n");
            return ;
        }

        if (printall)
        {
            for( i=0; i < lAllList.nIPNum; i++ )
            {            
                XOS_IptoStr(lAllList.localIP[i].LocalIPaddr,strIpPrint);
                XOS_IptoStr(lAllList.localIP[i].LocalNetMask,strMaskPrint);
                XOS_CliExtPrintf(pCliEnv, "\r\n%-16s  ip:%-16s netmask:%-16s\r\n",lAllList.localIP[i].xinterface,
                    strIpPrint,strMaskPrint);
            }
            
            XOS_CliExtPrintf(pCliEnv,"\r\n--------------------------------------------------------\r\n");
            return;
        }

        if (strSearchIp)
        {
            XBOOL flag = XFALSE;
            for( i=0; i < lAllList.nIPNum; i++ )
            {
                XOS_IptoStr(lAllList.localIP[i].LocalIPaddr,strIp);
                if (0 == XOS_StrCmp(strSearchIp, strIp))
                {   
                    flag = XTRUE;
                    XOS_IptoStr(lAllList.localIP[i].LocalNetMask,strMaskPrint);
                    XOS_CliExtPrintf(pCliEnv, "\r\n%-16s  ip:%-16s netmask:%-16s\r\n",lAllList.localIP[i].xinterface,
                        strIp,strMaskPrint);

                }
            }

            if (!flag)
            {
                XOS_CliExtPrintf(pCliEnv,"\r\n not found!\r\n");
            }
        }
    }
    XOS_CliExtPrintf(pCliEnv,"\r\n--------------------------------------------------------\r\n");

}



XVOID XOS_CliGetPhyIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ip;
    
    if(siArgc >= 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }

    if(XSUCC != XOS_GetPhysicIP(&ip))
    {
        XOS_CliExtPrintf(pCliEnv,"get failed.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"get succ. IP=0x%x.",ip);
    }

    //XOS_Trace(MD(FID_CLI,PL_ERR),"test trace.");
    return;
}

extern XS32 XOS_ModifyPhysicIP(XU32 *ipaddress);
XVOID XOS_CliModPhyIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ip;
    
    if(siArgc >= 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }

    XOS_GetPhysicIP(&ip);
    if(XSUCC != XOS_ModifyPhysicIP(&ip))
    {
        XOS_CliExtPrintf(pCliEnv,"modify failed.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"modify succ. IP=0x%x.",ip);
    }
    
    return;
}

XVOID XOS_CliGetMv2IP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ip;
    
    if(siArgc >= 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }

    if(XSUCC != XOS_GetMv2IP(&ip))
    {
        XOS_CliExtPrintf(pCliEnv,"get failed.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"get succ. IP=0x%x.",ip);
    }
    
    return;
}

XVOID XOS_CliGetMv1IP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ip;
    
    if(siArgc >= 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }

    if(XSUCC != XOS_GetMv1IP(&ip))
    {
        XOS_CliExtPrintf(pCliEnv,"get failed.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"get succ. IP=0x%x.",ip);
    }
    
    return;
}

XVOID XOS_CliGetLogicIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ip=0;
    
    if(siArgc >= 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }

    if(XSUCC != XOS_GetLogicIP(&ip))
    {
        XOS_CliExtPrintf(pCliEnv,"get failed.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"get succ. IP=0x%x.",ip);
    }
    
    return;
}

XVOID XOS_CliModLogicIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ip=0xc0a80001;
    
    if(siArgc >= 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }

    if(XSUCC != XOS_ModifyLogicIP(&ip))
    {
        XOS_CliExtPrintf(pCliEnv,"modify failed.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"modify succ. IP=0x%x.",ip);
    }
    
    return;
}


XVOID XOS_CliAddLogicIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ip=0xc0a80002;
    
    if(siArgc >= 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }

    if(XSUCC != XOS_AddLogicIP(&ip))
    {
        XOS_CliExtPrintf(pCliEnv,"add failed.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"add succ. IP=0x%x.",ip);
    }
    
    return;
}


XVOID XOS_CliDelLogicIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ip;
    
    if(siArgc >= 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }
    
    if(XSUCC != XOS_GetLogicIP(&ip))
    {
        XOS_CliExtPrintf(pCliEnv,"no logic ip to del.");
        return;
    }

    if(XSUCC != XOS_DeleteLogicIP(&ip))
    {
        XOS_CliExtPrintf(pCliEnv,"delete failed.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"delete succ. IP=0x%x.",ip);
    }
    
    return;
}
extern XU32 XOS_GetRunTime(void);
XVOID XOS_CliGetRunTime(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 uiRunSecs = 0;

    uiRunSecs = XOS_GetRunTime();
    if(0 != uiRunSecs)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nSystem running for %d days %d hours %d mins %d secs",
            uiRunSecs/86400,
            uiRunSecs%86400/3600,
            uiRunSecs%3600/60,
            uiRunSecs%60);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nThe System time has been modified by other programe. no supported here.");
    }
    return;
}

XVOID XOS_CliGetSysTicks(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ulticks = 0;
    XU32 uiRunSecs = 0;
    ulticks = XOS_GetSysTicks();
    
    if(0 != ulticks)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nThe System ticks is %d",ulticks);
        uiRunSecs = XOS_TicksToSec(ulticks);
        XOS_CliExtPrintf(pCliEnv, "\r\nSystem running for %d days %d hours %d mins %d secs",
            uiRunSecs/86400,
            uiRunSecs%86400/3600,
            uiRunSecs%3600/60,
            uiRunSecs%60);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nThe System ticks no supported here.");
    }
    return;
}

XVOID XOS_CliGetCpuRate(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    int rate = 0;
    XS32 uiRet = 0;
    char strRate[32] = {0};
    uiRet = XOS_GetCpuRate(&rate);

    if(XSUCC == uiRet)
    {
        sprintf(strRate, "\r\nThe cpu rate is %d", rate); 
        XOS_CliExtPrintf(pCliEnv, strRate);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nGet cpu rate fail.");
    }
    return;
}

XVOID XOS_CliGetLogTelnet(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    char strRate[64] = {0};

    sprintf(strRate, "Log Module discard message number:  %d", log_msg_discard); 
    XOS_CliExtPrintf(pCliEnv, strRate);
    
    sprintf(strRate, "\r\nTelnet Module discard message number:  %d", telnet_msg_discard); 
    XOS_CliExtPrintf(pCliEnv, strRate);
    return;
}

/************************************************************************
 函数名: XOS_ShowRunMode()
 功能:   显示当前运行环境32 or 64
 输入:   pCliEnv
 输出:   无
 返回:   无
 说明:   无
 ************************************************************************/
XVOID XOS_ShowRunMode(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU8 ucPointLen = 0;

    XOS_UNUSED(siArgc);
    XOS_UNUSED(ppArgv);

    ucPointLen = sizeof(XVOID*);

    XOS_CliExtPrintf(pCliEnv, "The Cps run at the %d mode\r\n", (4 == ucPointLen ? 32 : 64)); 

    return ;
}

extern XS32 NTL_regCli(int cmdMode);

int XOS_RegAllCliCmd(int cmdMode)
{
    XOS_CLIRegCommand(cmdMode);
    Trace_regCliCmd(cmdMode);
    NTL_regCli(cmdMode);
    return XSUCC;
}

#ifdef XOS_MDLMGT
/************************************************************************
函数名:XOS_getMainArg
功能:  获取main函数参数个数和参数列表
输入:  
输出: argc - 入参数量
    argv - 入参地址
返回: main函数的参数个数
说明: 
************************************************************************/
XS8 **XOS_getMainArg(XU32 *argc)
{
    if(NULL == argc)
    {
        return NULL;
    }
    *argc = g_main_argc;
    return g_main_argv;
}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */


