/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssDbCliUserInfo.h
* Author:       luhaiyan
* Date:         09-07-2014
* OverView:     
*
* History:      ������ʷ�޸�����ǰ��
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
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

// ������ע�ắ��
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
////��ʾAPN����Ϣ
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
////����pvi��pvitype��ӡimsi�����Ϣ
//XVOID GetPrintUserImsiInfo(XS8* pShowMsg, SDbPubPriRel *pDbPubPriRel, SDbContext *pContext);
//XVOID PrintDefInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID GetPrintDefInfo(XS8* pShowMsg,SImsiStaticInfo* pImsiStaticInfo,SPviPuiStatic* pPviPuiStatic);
////����pvi��pvitype��ӡ�绰���������Ϣ
//XVOID GetPrintUserMsidnInfo(XS8* pShowMsg, SDbPubPriRel *pDbPubPriRel, SDbContext *pContext);
//XVOID ShowTasReposInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
//XVOID GetPrintUserTasInfo(XS8* pShowMsg, SDbPubPriRelListRsp* pPubPriRelQueRsp, SDbContext *pContext);
////װ�ص绰����
//XVOID BatchLoadTel(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);
////����װ��IMSI
//XVOID BatchLoadImsi(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
