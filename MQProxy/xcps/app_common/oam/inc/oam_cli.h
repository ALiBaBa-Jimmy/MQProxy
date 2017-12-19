/*************************************************************
文件名       :oam_cli.h
文件描述     :
作者         :xiaohuiming
创建日期     :2014-07-15
修改记录     :
***************************************************************/

#ifndef _OAM_CLI_H_
#define _OAM_CLI_H_

#include "xosshell.h"

extern XVOID OAM_CliRegister();
extern XVOID OAM_xmlFileRead(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv);
extern XVOID OAM_xmlFileWrite(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv);

extern XVOID OAM_CfgMsgTest(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv);
extern XVOID OAM_ShowInfo(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv);
extern XVOID OAM_ShowMTInfo(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv);
extern XVOID OAM_FmMsgTest(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv);
#endif