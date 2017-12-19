/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: clicmds.c
**
**  description:  系统模块命令行c文件
**
**  author: zhanglei
**
**  date:   2006.3.7
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   zhanglei         2006.3.7              create
**************************************************************/

#ifdef  __cplusplus
extern  "C"
{
#endif

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include "clishell.h"
#include "clicmds.h"
#include "xostrace.h"
#include "xoscfg.h"
#include "xosmodule.h"
#include "xosos.h"
#include "xostl.h"
#ifdef XOS_ATT
#include "cliatt.h"
#endif

XEXTERN XVOID MOD_fidInfoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID MOD_tidInfoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID MOD_msgqInfoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);


/*内存管理调试命令*/
XEXTERN  XVOID MEM_infoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);

#ifdef MEM_FID_DEBUG
XEXTERN XVOID MEM_fidUsage(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID MEM_blocksSwill(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID MEM_fidBlockSwill(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);

XEXTERN XVOID MEM_allSwill(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID MEM_StackShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
#endif

XEXTERN XVOID XOS_CliGetSysTime(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliSetSysTime(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliGetPhyIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliModPhyIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliGetMv2IP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliGetMv1IP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliGetLogicIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliModLogicIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliAddLogicIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliDelLogicIP(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliGetRunTime(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliGetSysTicks(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliGetCpuRate(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
XEXTERN XVOID XOS_CliGetLogTelnet(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);

XEXTERN XVOID XOS_ShowIpList(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv );

/*-------------------------------------------------------------------------
                 模块内部宏定义
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                 模块内部结构和枚举定义
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                模块内部函数
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                模块接口函数
-------------------------------------------------------------------------*/
#ifdef XOS_ENABLE_GCOV
/************************************************************************
 函数名: Cli_XosExit( XVOID )
 功能:   平台退出命令
 输入:   无
 输出:   无
 返回:     成功返回XTRUE/失败返回XFALSE,或者错误
 说明:
************************************************************************/

XVOID Cli_XosExit(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    printf("############XOS EXIT! ###################\n\n");
    exit(0);
}
#endif

/************************************************************************
 函数名: XOS_CLIRegCommand( XVOID )
 功能:   注册命令函数.
 输入:   无
 输出:   无
 返回:     成功返回XTRUE/失败返回XFALSE,或者错误
 说明:
************************************************************************/
XBOOL XOS_CLIRegCommand( int cmdMode )
{
    XS32 ret = 0;
    
    if ( 0 > XOS_RegistCommandX( cmdMode,
        cli_printf_switch_cmd,
        "print_direct",
        "open print direct to telnet client ID",
        "print_direct           -- display print direction list\r\nprint_direct on/off  -- open/close print direct to this telnet client SID\r\nprint_direct <SID> on/off  -- open/close print direct to given telnet client SID\r\n",
        eUserAdmin ) )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "register command XOS>PLAT>print_direct failed!" );
    }

#ifdef XOS_ENABLE_GCOV
        ret = XOS_RegistCommandX(cmdMode, Cli_XosExit, 
        "xos_exit",
        "exit xos platform",
        "xos_exit\n",
        eUserAdmin);
    if (0 > ret)
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "register command xos_exit failed!" );
    }
#endif
    /*平台提示符*/
    ret = XOS_RegistCmdPromptX( cmdMode/*SYSTEM_MODE*/, "plat", "plat", "no parameter", eUserNormal );
    if ( 0 > ret )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "register command xos>plat failed!" );
        return XFALSE;
    }

    XOS_RegistCommand(ret, XOS_ShowRunMode,"showrunmode", "show cur xos run mode(32 or 64)", "no para");  
     
#ifdef CLI_LOG
    /*CLI LOG*/
    if ( 0 > XOS_RegistCommand( ret,
        cli_log_debug_cmd,
        "clilog",
                                "CLI debug log switch.",
                                "on:open debug function.\r\noff:close debug function.\r\nstate:debug state)." ) )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "register command xos>plat>clilog failed!" );
    }
#endif
    
#ifdef XOS_ATT /*## */
    /*ATT命令注册*/
    if ( 0 > XOS_RegistCommand( ret, ATT_CliAttInfor, "att",
                                "ATT msg trans info command",
                                "no parameter" ) )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "register command xos>plat>att failed!" );
    }
    if ( 0 > XOS_RegistCommand( ret,
                                ATT_CliMsgSwitch,
                                "msgswitch",
                                "ATT msg switch command",
                                "fid       -- XOS app FID\r\nIP(ATT)   -- ATT testtoolIP \r\nPort(ATT) -- ATT testtoolport\r\non[off]   -- msgswitchID" ) )
    {
        XOS_Trace( MD(FID_CLI, PL_ERR), "register command xos>plat>msgswitch failed!" );
    }
#endif
    
    XOS_RegistCommand(ret, MOD_fidInfoShow,"fidshow", "display fid information", "no parameter");
    XOS_RegistCommand(ret, MOD_tidInfoShow,"tskshow", "display tid information", "no parameter");
    XOS_RegistCommand(ret, MOD_msgqInfoShow,"msgqshow", "display all fid msgq information", "no parameter");
    
    /*内存管理命令*/
    XOS_RegistCommand(ret, MEM_infoShow,"memshow", "display general memory information", "no parameter");
#ifdef MEM_FID_DEBUG
    XOS_RegistCommand(ret, MEM_fidUsage,"memfidinfo", "display given fid memory informaion", "fid -- moduleID");
    XOS_RegistCommand(ret, MEM_blocksSwill,"memblockswill", "display given memory block usage", "blockSize -- such as 1024");
    XOS_RegistCommand(ret, MEM_fidBlockSwill,"memfidblockswill", "display given memory block usage of special fid", 
                           "display given memory block usage of special fid -- such as:memfidblockswill 4 1024");
    XOS_RegistCommand(ret, MEM_allSwill,"memallswill", "display all memory usage", "no parameter");
    
    XOS_RegistCommand(ret, MEM_StackShow,"memstackshow", "display given address memory stack", "memory address head pointer");
    
#endif

    XOS_RegistCommand(ret, XOS_ShowIpList, "showiplist", "show ip list", "-p ip, -i intface, -a");

    
    XOS_RegistCommand(ret, XOS_CliSetSysTime,"setsystime", "set the system time", "no parameter");
    XOS_RegistCommand(ret, XOS_CliGetPhyIP,"getphyIP", "get the physic IP", "no parameter");
    XOS_RegistCommand(ret, XOS_CliModPhyIP,"modphyIP", "modify the physic IP", "no parameter");
    XOS_RegistCommand(ret, XOS_CliGetLogicIP,"getlogicIP", "get the logic IP", "no parameter");
    XOS_RegistCommand(ret, XOS_CliAddLogicIP,"addlogicIP", "add the logic IP", "no parameter");
    XOS_RegistCommand(ret, XOS_CliDelLogicIP,"dellogicIP", "del the logic IP", "no parameter");
    XOS_RegistCommand(ret, XOS_CliModLogicIP,"modlogicIP", "mod the logic IP", "no parameter");
    XOS_RegistCommand(ret, XOS_CliGetMv2IP,"getmv2IP", "get the mv2 IP", "no parameter");
    XOS_RegistCommand(ret, XOS_CliGetMv1IP,"getmv1IP", "get the mv1 IP", "no parameter");
    XOS_RegistCommand(ret, XOS_CliGetRunTime,"getsysruntime", "get system running time", "no parameter");
    XOS_RegistCommand(ret, XOS_CliGetSysTicks,"getsystick", "get system ticks", "no parameter");
     XOS_RegistCommand(ret, XOS_CliGetCpuRate,"getcpurate", "get cpu rate", "no parameter");
     XOS_RegistCommand(ret, XOS_CliGetLogTelnet,"logtelnetfullshow", "get discard message of log and telnet module", "no parameter");
    return XTRUE;
}

#ifdef  __cplusplus
}
#endif


