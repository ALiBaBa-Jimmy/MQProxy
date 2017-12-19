/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssHdbOam.cpp
* Author:       luhaiyan
* Date:        10-04-2014
* OverView:     数据库模块的配置信息,这里的数据库的启动等都是有OAMTASK触发，
*				不需要单独建立dbTAsk，否则多线程会导致这里定义的全局变量需要
*               增加锁；（网管需求要增加mysqlIP配置）
* History:      最新历史修改在最前面
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*******************************************************************************/
#include "SPRHdbOam.h"
#include "DbFactory.h"
#include "Getmysqldbcfg.h"
#include "DbFactoryMysql.h"
#include "ace/Init_ACE.h"
#include "DbCommon.h"
//#include "eHssMdbDbFun.h"
callbackTraceFun g_callbackTraceFunOfHssHDb;


//是否初始化数据库的配置信息，主要是因为数据库的配置信息特别是IP信息需要配置下来
XBOOL g_bInitDbConfig = XFALSE;
//是否已经初始过数据库
XBOOL g_bInitDb = XFALSE;
SDbConfigInfo g_szDbConfigInfo={0};

CDbService *g_PDbService = NULL;
CDbService *g_PDbAlias = NULL;/*别名库*/

/**********************************************************************
*
*  NAME:          RegistHDbCallbackTraceFun
*  FUNTION:       注册trace函数，主要为了解决动态库log输出不了的问题
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 RegistHDbCallbackTraceFun(callbackTraceFun  traceFun)
{ 
    g_callbackTraceFunOfHssHDb = traceFun;
    //g_callbackTraceFunOfMySqlDb = traceFun;
	return XSUCC;
}



/**********************************************************************
*
*  NAME:          RegistMySqlDbCallbackTraceFun
*  FUNTION:       注册trace函数，主要为了解决动态库log输出不了的问题
*  INPUT:         无
*  OUTPUT:        本地的host信息
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 RegistHDBMySqlDbCallbackTraceFun(callbackTraceFun  traceFun)
{
	//g_callbackTraceFunOfMySqlDb = traceFun;
    RegistMySqlDbCallbackTraceFun(traceFun);
	return 0;
}

#if 0

/*oracle 数据库的连接函数*/
/**********************************************************************
*
*  NAME:          InitDb
*  FUNTION:       初始化DB
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 InitDb(SDbConfigInfo *pDbCfgInfo)
{
	if(g_bInitDb == XTRUE)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"DB has inited");
		return XSUCC;
	}

	ACE::init();
	g_PDbService = CDbFactory::getInstance()->CreateDbService();
	if (g_PDbService == NULL)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"CDbService* is NULL");
		return XERROR;
	}	
	//初始化环境
	SDllEnv szDllEnv;
	szDllEnv.mod = THREADED_MUTEXED;
	g_PDbService->DbInit((XU8 *)&szDllEnv);
	
	
	XS32 nRet = XERROR;
	XOS_MemSet(&g_szDbConfigInfo,0,sizeof(SDbConfigInfo));

	XOS_MemCpy(&g_szDbConfigInfo,pDbCfgInfo,sizeof(SDbConfigInfo));

	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"connstring=%s",g_szDbConfigInfo.connstring);


	//初始化连接池	
	nRet = g_PDbService->DbInitConnection((XS8*)g_szDbConfigInfo.user, (XS8*)g_szDbConfigInfo.pwd,
		(XS8*)g_szDbConfigInfo.connstring,0,XTRUE);
	if (nRet != XSUCC)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"DbInitConnection error,nRet=%d,user=%s,connstring=%s",
			nRet,g_szDbConfigInfo.user,g_szDbConfigInfo.connstring);
		return XERROR;
	}



//	InitDefaultBusiness(FID_AGENTDB);

	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"leaving InitDb success");

	g_bInitDb = XTRUE;
	return XSUCC;

}


/**********************************************************************
*
*  NAME:          InitAliasDb
*  FUNTION:       初始化DB
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 InitAliasDb(SDbConfigInfo *pDbCfgInfo)
{

	g_PDbAlias= CDbFactory::getInstance()->CreateDbService();
	if (g_PDbAlias == NULL)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"g_PDbAlias is NULL");
		return XERROR;
	}	
	//初始化环境
	SDllEnv szDllEnv;
	szDllEnv.mod = THREADED_MUTEXED;
	g_PDbAlias->DbInit((XU8 *)&szDllEnv);
	
	
	XS32 nRet = XERROR;
	XOS_MemSet(&g_szDbConfigInfo,0,sizeof(SDbConfigInfo));

	XOS_MemCpy(&g_szDbConfigInfo,pDbCfgInfo,sizeof(SDbConfigInfo));

	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"connstring=%s",g_szDbConfigInfo.connstring);


	//初始化连接池	
	nRet = g_PDbAlias->DbInitConnection((XS8*)g_szDbConfigInfo.user_ali, (XS8*)g_szDbConfigInfo.pwd_ali,
		(XS8*)g_szDbConfigInfo.connstring_ali,0,XTRUE);
	if (nRet != XSUCC)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"DbInitConnection error,nRet=%d,user=%s,connstring=%s",
			nRet,g_szDbConfigInfo.user_ali,g_szDbConfigInfo.connstring_ali);
		return XERROR;
	}


	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"Exit");

	return XSUCC;

}

XBOOL GetDbLinkStatus()
{
	return g_bInitDb;	
}

/**********************************************************************
*
*  NAME:          DestroyDb
*  FUNTION:       断开到数据库的连接
*  INPUT:         
*  OUTPUT:        成功失败
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 DestroyDb()
{
	XS32 nRet = XERROR;

	if (g_PDbService == NULL)
	{	
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"g_PDbService1* is NULL");
		return XERROR;
	}
	//断开数据库连接	
	nRet = g_PDbService->DbDestroyConnection();
	if (nRet != XSUCC)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"DbDestroyConnection error,nRet=%d,user=%s,connstring=%s",
			nRet,g_szDbConfigInfo.user[0],g_szDbConfigInfo.connstring[0]);
		return XERROR;
	}

	g_bInitDb = XFALSE;	 //将全局变量设为false	
	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"leaving DestroyDb success,g_bInitDb=%d",g_bInitDb);
	return XSUCC;
	
}
#endif


/**********************************************************************
*
*  NAME:          InitDb
*  FUNTION:       初始化DB
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 InitDb(SDbConfigInfo *pDbCfgInfo)
{
	if(g_bInitDb == XTRUE)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"DB has inited");
		return XSUCC;
	}
	XS32 nRet = XERROR;
	XOS_MemSet(&g_szDbConfigInfo,0,sizeof(SDbConfigInfo));

	XOS_MemCpy(&g_szDbConfigInfo,pDbCfgInfo,sizeof(SDbConfigInfo));

	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"ip=%s",g_szDbConfigInfo.ip);

	CDbService* pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
	if (pDbService == NULL)
	{	
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"CDbService* is NULL ip=%s",g_szDbConfigInfo.ip);
		return XERROR;
	}


	//初始化连接池	
	nRet = pDbService->DbInitConnection((XS8*)g_szDbConfigInfo.user, (XS8*)g_szDbConfigInfo.pwd,
		(XS8*)g_szDbConfigInfo.dbName, (XS8*)g_szDbConfigInfo.ip, XTRUE);
	if (nRet != XSUCC)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"DbInitConnection error,nRet=%d,ip=%s",nRet,g_szDbConfigInfo.ip);
		return XERROR;
	}

//	InitDefaultBusiness(FID_AGENTDB);

	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"leaving InitDb success");

	g_bInitDb = XTRUE;
	return XSUCC;

}

/**********************************************************************
*
*  NAME:          SetDbIp
*  FUNTION:       设置DBIP
*  INPUT:         XU8* pDbIP
*  OUTPUT:        成功失败
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 SetDbIp(XU8* pDbIP)
{

	return XSUCC;
}

XBOOL GetDbLinkStatus()
{
	return g_bInitDb;	
}

/**********************************************************************
*
*  NAME:          DestroyDb
*  FUNTION:       断开到数据库的连接
*  INPUT:         
*  OUTPUT:        成功失败
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 DestroyDb()
{
	XS32 nRet = XERROR;
	CDbService* pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
	if (pDbService == NULL)
	{	
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"CDbService* is NULL");
		return XERROR;
	}

	//断开数据库连接	
	nRet = pDbService->DbDestroyConnection();
	if (nRet != XSUCC)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"DbDestroyConnection error,nRet=%d",nRet);
		return XERROR;
	}

	g_bInitDb = XFALSE;	 //将全局变量设为false	
	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"leaving DestroyDb success,g_bInitDb=%d",g_bInitDb);
	return XSUCC;
	
}

