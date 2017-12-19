/************************************************************************
* 文件名：SmuTemplate.h
* 说  明：数据库接口，后期可以考虑一类业务用同一个接口，按业务接口赋不同
*			的值
************************************************************************/
#pragma once
#include "xosshell.h"
#include "SPRHardDbCommon.h"
#include "agentHardDb.h"
#include "agentHDB_API.h"
#include "smu_sj3_type.h"
#include "agentinclude.h"


#pragma pack(1)

#pragma pack()







AGENTHARDDB_API XS32 MysqlHDB_QryLocalNetWorkingbyUid(XS32 connid, SDbReqSql *req, XU8 *pNetworkId);
AGENTHARDDB_API XS32 MysqlHDB_QryuidbyTel(XS32 connid, SDbReqSql *req, XU8 *uid);
AGENTHARDDB_API XS32 MysqlHDB_QryuidbyAlias(XS32 connid, SDbReqSql *req, XU8 *uid);
AGENTHARDDB_API XS32 MysqlHDB_LoadUtInfo(XS32 connid, SDbReqSql *req);
AGENTHARDDB_API XS32 MysqlHDB_Update(XS32 connid, SDbReqSql *req);


