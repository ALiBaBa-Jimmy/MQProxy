/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: clicmds.h
**
**  description:  XOS系统模块命令行头文件
**
**  author: zhanglei
**
**  date:   2006.3.7
**
***************************************************************
**                           history                     
**  
***************************************************************
**   author          date              modification            
**   zhanglei         2006.3.7              create  
**************************************************************/

#ifndef _CLIATT_H_
#define _CLIATT_H_

#ifdef  __cplusplus
extern  "C"
{
#endif


/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/ 
#include "cliconfig.h"

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                API 声明
-------------------------------------------------------------------------*/

XPUBLIC XVOID ATT_CliMsgSwitch( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv );
XPUBLIC XVOID ATT_CliAppFIDInfor( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv );
XPUBLIC XVOID ATT_CliAttInfor( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv );

#ifdef  __cplusplus
}
#endif

#endif /* _CLIATT_H_ */
