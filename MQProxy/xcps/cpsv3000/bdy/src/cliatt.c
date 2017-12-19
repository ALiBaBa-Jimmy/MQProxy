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
**  data:   2006.3.7
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
#include "cliatt.h"
#include "xoscfg.h"
#include "xosencap.h"

#ifdef XOS_ATT
#include "xostl.h"
#endif

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

#ifdef XOS_ATT
/************************************************************************
 函数名: ATT_CliTestSeitch( XVOID )
 功能:   ATT 消息开关命令函数.
 输入:   无
 输出:   无
 返回:     无
 说明:
************************************************************************/
XVOID ATT_CliMsgSwitch( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv )
{
    XCHAR *pHelpStr = "msgswitch fid IP(ATT) Port(ATT) on[off]\r\nexample:\
                       \r\n        msgswitch 1 192.168.0.1 1028 on\
                       \r\n        msgswitch 1 192.168.0.1 1028 off";
    XU32 dstFid;
    t_IPADDR attAddr;

    if ( siArgc == 5 )
    {
        dstFid       = (XU32)atol( ppArgv[1] );
        if ( XSUCC != XOS_StrtoIp( ppArgv[2], &(attAddr.ip) ) )
        {
            goto OUT_ERR;
        }
        attAddr.port = (XU16)atol( ppArgv[3] );

        if ( !XOS_StrCmp( ppArgv[4], "on" ) )
        {

            if ( XSUCC == IPC_attOn( dstFid, &attAddr) )
            {
                XOS_CliExtPrintf(pCliEnv,
                                  "\r\n  fid:%d <-> %s:%d online.\r\n",
                                  dstFid,
                                  ppArgv[2],
                                  attAddr.port );
            }
            else
            {
                XOS_CliExtPrintf(pCliEnv,
                                  "msgswitch error\r\n");
            }

            return;
        }
        else if ( !XOS_StrCmp( ppArgv[4], "off" ) )
        {
            if ( XSUCC == IPC_attOff( dstFid ) )
            {
                XOS_CliExtPrintf(pCliEnv,
                                  "\r\n  fid:%d -- %s:%d offline.\r\n",
                                  dstFid,
                                  ppArgv[2],
                                  attAddr.port );
            }
            else
            {
                XOS_CliExtPrintf(pCliEnv,
                                  "msgswitch error\r\n");
            }

            return;
        }
        else
        {
            goto OUT_ERR;
        }
    }

OUT_ERR:
    XOS_CliExtPrintf(pCliEnv,"%s", pHelpStr );

    return ;
}

/************************************************************************
 函数名: ATT_CliAppInfor( XVOID )
 功能:   ATT 消息开关命令函数.
 输入:   无
 输出:   无
 返回:     无
 说明:
************************************************************************/
XVOID ATT_CliAppFIDInfor( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv )
{
#if 0
    XS32 i = 0;

    XOS_CliExtPrintf(pCliEnv,"---------------------------------------------\n");
    XOS_CliExtPrintf(pCliEnv,"%3s    %8s    %3s\r\n", "fid", "fid Name", "pid");
    for( i = 0 ; i < MaxFIDNum; i++ )
    {
        if ( XNULL != ModuleList[i].FID
          && XNULL != ModuleList[i].FIDStack
          && XNULL != ModuleList[i].FIDStack->head.FIDName )
        {
                XOS_CliExtPrintf(pCliEnv,"%3d    %8s    %3d\r\n",
                    ModuleList[i].FID,
                    ModuleList[i].FIDStack->head.FIDName,
                    ModuleList[i].FIDStack->head.PID );
        }
    }

    XOS_CliExtPrintf(pCliEnv,"---------------------------------------------\n");
#endif
    return ;
}

/************************************************************************
 函数名: ATT_CliAppInfor( XVOID )
 功能:   ATT 消息开关命令函数.
 输入:   无
 输出:   无
 返回:     无
 说明:
************************************************************************/
XVOID ATT_CliAttInfor( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv )
{
    IPC_attShow( pCliEnv );

    return ;
}

#endif

#ifdef  __cplusplus
}
#endif


