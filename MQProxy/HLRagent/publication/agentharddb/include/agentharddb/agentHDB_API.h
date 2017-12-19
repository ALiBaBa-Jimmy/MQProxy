/************************************************************************
* �ļ�����SmuTemplate.h
* ˵  �������ݿ�ӿڣ����ڿ��Կ���һ��ҵ����ͬһ���ӿڣ���ҵ��ӿڸ���ͬ
*			��ֵ
************************************************************************/
#pragma once
#include "xosshell.h"
#include "SPRHardDbCommon.h"
#include "agentHardDb.h"
#include "smu_sj3_type.h"
#include "agentinclude.h"


#pragma pack(1)


typedef struct  
{
	XU8     conditionSql[DB_MAX_SQL_LENGTH];
}SDbReqSql;

typedef struct 
{
	XU8 pid[LENGTH_OF_PID];
	XU8 port;
	XU8 creditUid[LENGTH_OF_UID];//�ſ�UID��������ѵ�UID
	XU8  bindUid[LENGTH_OF_UID]; //Ҫ��ʹ�õ�UID��Ŀǰ����ֻ�Ǽ�¼��û��ʵ��ʹ�á�
	XU16 terminalStatus;//�ն˳��⼤�����δ����״̬ 2��δ���� 1������
	XU8 creditStatus;	/*�ſ�״̬*/
    XU8 NetId[LENGTH_OF_NETWORK_ID];
}SLeaseHoldCtlTable;

typedef struct 
{
    XU32 count;
    SLeaseHoldCtlTable UtLeaseInfo[20];
}SLeaseHoldCtlPack;

#pragma pack()

typedef XS32 (*SubLoadCallBack)(XU32 type, XCONST XCHAR *topic, XU8 initFlg);
//XS32 MQTT_InsertSubInfo(XU32 type, XCONST XCHAR *topic, XU8 initFlg)
typedef XVOID (*SubLoadUtBindCallBack)(SLeaseHoldCtlTable *pIns);
extern SubLoadUtBindCallBack   g_LoadUtBindInfoCallback;

AGENTHARDDB_API XU64 DB_HexToStr(XU8 *pHex,XU8 * pStr,XU64 ulLen);
AGENTHARDDB_API XU64 DB_StrToHex(XU8 * pStr,XU8 *pHex,XU64 ulLen);

AGENTHARDDB_API XS32 agentHDB_QryLocalNetWorkingbyUid(SDbReqSql *req, XU8 *pNetworkId);
AGENTHARDDB_API XS32 agentHDB_QryuidbyTel(SDbReqSql *req, XU8 *uid);
AGENTHARDDB_API XS32 agentHDB_QryuidbyAlias(SDbReqSql *req, XU8 *uid);
AGENTHARDDB_API XS32 agentHDB_QryNationCodebyTelNo(SDbReqSql *req, XU8 *pNation);
AGENTHARDDB_API XS32 agentHDB_SubOper(SDbReqSql *req);
AGENTHARDDB_API XS32 agentHDB_SubQryOper(SDbReqSql *req);
AGENTHARDDB_API XS32 agentHDB_SubLoadCallback(SubLoadCallBack func);
AGENTHARDDB_API XS32 agentHDB_SubLoadUtBindCallback(SubLoadUtBindCallBack func);
AGENTHARDDB_API XS32 agentHDB_QryCamTalkInfo(SDbReqSql *req, TXdbHdbDyn *pResult);
AGENTHARDDB_API XS32 agentHDB_QryDynInfobyUid(SDbReqSql *req, TXdbHdbDyn *pResult);
AGENTHARDDB_API XS32 agentHDB_Update(SDbReqSql *req);
AGENTHARDDB_API XS32 agentHDB_LoadUtInfo(SDbReqSql *req);


