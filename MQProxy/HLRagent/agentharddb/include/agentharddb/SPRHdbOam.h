/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssHdbOam.h
* Author:       luhaiyan
* Date:        10-04-2014
* OverView:     数据库模块的配置信息
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
#ifndef __SPR_HDB_OAM_H__
#define __SPR_HDB_OAM_H__

#include "agentHardDb.h"
#include "agentinclude.h"
#include "Getmysqldbcfg.h"

#ifdef __cplusplus
extern "C" {
#endif


/**********************************************************************
*
*  NAME:          RegistHDbCallbackTraceFun
*  FUNTION:       注册trace函数，主要为了解决动态库log输出不了的问题,
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
AGENTHARDDB_API  XS32 RegistHDbCallbackTraceFun(callbackTraceFun  traceFun);

/**********************************************************************
*
*  NAME:          InitDb
*  FUNTION:       初始化DB
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
AGENTHARDDB_API  XS32 InitDb(SDbConfigInfo *pDbCfgInfo);
//AGENTHARDDB_API XS32 InitAliasDb(SDbConfigInfo *pDbCfgInfo);

/**********************************************************************
*
*  NAME:          SetDbIp
*  FUNTION:       设置DBIP
*  INPUT:         XU8* pDbIP
*  OUTPUT:        成功失败
*  OTHERS:        其他需说明的问题
**********************************************************************/


AGENTHARDDB_API  XBOOL GetDbLinkStatus();

AGENTHARDDB_API  XS32 DestroyDb();
/**********************************************************************
*
*  NAME:          RegistHDbCallbackTraceFun
*  FUNTION:       注册trace函数，主要为了解决动态库log输出不了的问题,
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
AGENTHARDDB_API  XS32 RegistHDBMySqlDbCallbackTraceFun(callbackTraceFun  traceFun);


#ifdef __cplusplus
}
#endif 

#endif


