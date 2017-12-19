/************************************************************************
* 文件名：SmuTemplate.h
* 说  明：数据库接口，后期可以考虑一类业务用同一个接口，按业务接口赋不同
*			的值
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
	XU8 creditUid[LENGTH_OF_UID];//信控UID，交月租费的UID
	XU8  bindUid[LENGTH_OF_UID]; //要绑定使用的UID，目前这里只是记录，没有实际使用。
	XU16 terminalStatus;//终端出租激活或者未激活状态 2：未激活 1：激活
	XU8 creditStatus;	/*信控状态*/
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


