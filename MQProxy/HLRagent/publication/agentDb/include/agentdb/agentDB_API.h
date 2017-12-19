
#pragma once


#include "agentDb.h"
#include "SPRDbErrorCode.h"
#include "smu_sj3_type.h"
#include "agentHDB_API.h"



AGENTDB_API XS32 agentDB_QryLocalNetWorkIdbyUID(XU32 fid, XU8 * pUid, XU8 *pNetWorkId);
AGENTDB_API XS32 agentDB_QryuidbyTel(XU32 fid, XU8 * pUid, XU8 *Tel);
AGENTDB_API XS32 agentDB_QryuidbyAlias(XU32 fid, XU8 * pUid, XU8 *Alias);
AGENTDB_API XS32 agentDB_QryNationCodebyUID(XU32 fid, XU8 * pNation, XU8 *pTel);
AGENTDB_API XS32  agentDB_SubInsert(XU32 fid, XU32 type, XU8 *topic);
AGENTDB_API XS32  agentDB_SubDelete(XU32 fid, XU8 *topic);
AGENTDB_API XS32 agentDB_SubQryAll(XU32 fid);
AGENTDB_API XS32 agentDB_QryDynInfobyUID(XU32 fid, XU8 * pUid, TXdbHdbDyn *pResult);
AGENTDB_API XS32 agentDB_UpdateDynInfo(XU32 fid, XU8 * pUid, TXdbHdbDyn *pDynInfo);
AGENTDB_API XS32 agentDB_QryAllUtBindInfo(XU32 fid);
AGENTDB_API XS32 agentDB_UptUtBindInfo(XU32 fid, SLeaseHoldCtlTable* pUtInfo);


