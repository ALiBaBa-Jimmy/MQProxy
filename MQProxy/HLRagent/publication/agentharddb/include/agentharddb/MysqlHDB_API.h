/************************************************************************
* �ļ�����SmuTemplate.h
* ˵  �������ݿ�ӿڣ����ڿ��Կ���һ��ҵ����ͬһ���ӿڣ���ҵ��ӿڸ���ͬ
*			��ֵ
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


