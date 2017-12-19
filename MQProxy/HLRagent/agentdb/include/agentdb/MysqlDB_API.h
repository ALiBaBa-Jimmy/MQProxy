
#pragma once


#include "agentDb.h"
#include "SPRDbErrorCode.h"
#include "smu_sj3_type.h"
#include "agentHDB_API.h"



AGENTDB_API XS32 MysqlDB_QryLocalNetWorkIdbyUID(XU32 fid, XU8 * pUid, XU8 *pNetWorkId);
AGENTDB_API XS32 MysqlDB_QryuidbyTel(XU32 fid, XU8 * pUid, XU8 *Tel);
AGENTDB_API XS32 MysqlDB_QryuidbyAlias(XU32 fid, XU8 * pUid, XU8 *Alias);

AGENTDB_API XS32 MysqlDB_QryAllUtBindInfo(XU32 fid);

AGENTDB_API XS32 MysqlDB_UptUtBindInfo(XU32 fid, SLeaseHoldCtlTable* pUtInfo);

