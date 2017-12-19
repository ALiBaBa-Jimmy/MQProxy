/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssDbCliUserInfo.h
* Author:       luhaiyan
* Date:         09-07-2014
* OverView:     
*
* History:      最新历史修改在最前面
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*******************************************************************************/
#ifndef __EHSS_DB_CLI_USERINFO_H
#define __EHSS_DB_CLI_USERINFO_H


#include "xostype.h"
#include "clishell.h"
//#include "eHssMDbPviStruct.h"
//#include "eHssMAndHDbPviPui.h"
//#include "SmuPui_pvi_relation.h"
//#include "smudbdb_common.h"

#ifdef __cplusplus
extern "C" {
#endif

XVOID   RegUserDataCliCmd(XVOID);

// 命令行注册函数
//XS32  RegUserDataCmd(void);
//XVOID ShowMemUserInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID ShowImsiInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//
//XVOID ShowMsisdnInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//
//XVOID LoadImsiInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//
//XVOID LoadMsisdnInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//
//XVOID DestoryAllConn(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//
////显示APN组信息
//XVOID ShowApnGroupInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID ShowBizInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID ShowAmfInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID ShowIfcInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID ShowOpInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID LoadDefInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID ShowApnInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID CleanImsiInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID CleanMsisdnInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID GetPrintUserAllBizInfo(XS8* pShowMsg, XU8* pvi,XU8 pviType, SDbContext *pContext);
//XVOID GetPrintDefImsiStaticInfo(XS8* pShowMsg,SImsiStaticInfo* pImsiStaticInfo);
//XVOID GetPrintDefPviPuiStatic(XS8* pShowMsg,SPviPuiStatic* pPviPuiStatic);
//XVOID GetPrintUserSpecialBizInfo(XS8* pShowMsg, XU8* pvi,XU8 pviType,XU8* pCode, SDbContext *pContext);
////根据pvi、pvitype打印imsi相关信息
//XVOID GetPrintUserImsiInfo(XS8* pShowMsg, SDbPubPriRel *pDbPubPriRel, SDbContext *pContext);
//XVOID PrintDefInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID GetPrintDefInfo(XS8* pShowMsg,SImsiStaticInfo* pImsiStaticInfo,SPviPuiStatic* pPviPuiStatic);
////根据pvi、pvitype打印电话号码相关信息
//XVOID GetPrintUserMsidnInfo(XS8* pShowMsg, SDbPubPriRel *pDbPubPriRel, SDbContext *pContext);
//XVOID ShowTasReposInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID GetPrintUserTasInfo(XS8* pShowMsg, SDbPubPriRelListRsp* pPubPriRelQueRsp, SDbContext *pContext);
////装载电话号码
//XVOID BatchLoadTel(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
////批量装载IMSI
//XVOID BatchLoadImsi(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
