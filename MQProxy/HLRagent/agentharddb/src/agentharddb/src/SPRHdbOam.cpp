/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssHdbOam.cpp
* Author:       luhaiyan
* Date:        10-04-2014
* OverView:     ���ݿ�ģ���������Ϣ,��������ݿ�������ȶ�����OAMTASK������
*				����Ҫ��������dbTAsk��������̻߳ᵼ�����ﶨ���ȫ�ֱ�����Ҫ
*               ������������������Ҫ����mysqlIP���ã�
* History:      ������ʷ�޸�����ǰ��
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*******************************************************************************/
#include "SPRHdbOam.h"
#include "DbFactory.h"
#include "Getmysqldbcfg.h"
#include "DbFactoryMysql.h"
#include "ace/Init_ACE.h"
#include "DbCommon.h"
//#include "eHssMdbDbFun.h"
callbackTraceFun g_callbackTraceFunOfHssHDb;


//�Ƿ��ʼ�����ݿ��������Ϣ����Ҫ����Ϊ���ݿ��������Ϣ�ر���IP��Ϣ��Ҫ��������
XBOOL g_bInitDbConfig = XFALSE;
//�Ƿ��Ѿ���ʼ�����ݿ�
XBOOL g_bInitDb = XFALSE;
SDbConfigInfo g_szDbConfigInfo={0};

CDbService *g_PDbService = NULL;
CDbService *g_PDbAlias = NULL;/*������*/

/**********************************************************************
*
*  NAME:          RegistHDbCallbackTraceFun
*  FUNTION:       ע��trace��������ҪΪ�˽����̬��log������˵�����
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
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
*  FUNTION:       ע��trace��������ҪΪ�˽����̬��log������˵�����
*  INPUT:         ��
*  OUTPUT:        ���ص�host��Ϣ
*  OTHERS:        ������˵��������
**********************************************************************/
XS32 RegistHDBMySqlDbCallbackTraceFun(callbackTraceFun  traceFun)
{
	//g_callbackTraceFunOfMySqlDb = traceFun;
    RegistMySqlDbCallbackTraceFun(traceFun);
	return 0;
}

#if 0

/*oracle ���ݿ�����Ӻ���*/
/**********************************************************************
*
*  NAME:          InitDb
*  FUNTION:       ��ʼ��DB
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
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
	//��ʼ������
	SDllEnv szDllEnv;
	szDllEnv.mod = THREADED_MUTEXED;
	g_PDbService->DbInit((XU8 *)&szDllEnv);
	
	
	XS32 nRet = XERROR;
	XOS_MemSet(&g_szDbConfigInfo,0,sizeof(SDbConfigInfo));

	XOS_MemCpy(&g_szDbConfigInfo,pDbCfgInfo,sizeof(SDbConfigInfo));

	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"connstring=%s",g_szDbConfigInfo.connstring);


	//��ʼ�����ӳ�	
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
*  FUNTION:       ��ʼ��DB
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
**********************************************************************/
XS32 InitAliasDb(SDbConfigInfo *pDbCfgInfo)
{

	g_PDbAlias= CDbFactory::getInstance()->CreateDbService();
	if (g_PDbAlias == NULL)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"g_PDbAlias is NULL");
		return XERROR;
	}	
	//��ʼ������
	SDllEnv szDllEnv;
	szDllEnv.mod = THREADED_MUTEXED;
	g_PDbAlias->DbInit((XU8 *)&szDllEnv);
	
	
	XS32 nRet = XERROR;
	XOS_MemSet(&g_szDbConfigInfo,0,sizeof(SDbConfigInfo));

	XOS_MemCpy(&g_szDbConfigInfo,pDbCfgInfo,sizeof(SDbConfigInfo));

	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"connstring=%s",g_szDbConfigInfo.connstring);


	//��ʼ�����ӳ�	
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
*  FUNTION:       �Ͽ������ݿ������
*  INPUT:         
*  OUTPUT:        �ɹ�ʧ��
*  OTHERS:        ������˵��������
**********************************************************************/
XS32 DestroyDb()
{
	XS32 nRet = XERROR;

	if (g_PDbService == NULL)
	{	
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"g_PDbService1* is NULL");
		return XERROR;
	}
	//�Ͽ����ݿ�����	
	nRet = g_PDbService->DbDestroyConnection();
	if (nRet != XSUCC)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"DbDestroyConnection error,nRet=%d,user=%s,connstring=%s",
			nRet,g_szDbConfigInfo.user[0],g_szDbConfigInfo.connstring[0]);
		return XERROR;
	}

	g_bInitDb = XFALSE;	 //��ȫ�ֱ�����Ϊfalse	
	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"leaving DestroyDb success,g_bInitDb=%d",g_bInitDb);
	return XSUCC;
	
}
#endif


/**********************************************************************
*
*  NAME:          InitDb
*  FUNTION:       ��ʼ��DB
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
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


	//��ʼ�����ӳ�	
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
*  FUNTION:       ����DBIP
*  INPUT:         XU8* pDbIP
*  OUTPUT:        �ɹ�ʧ��
*  OTHERS:        ������˵��������
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
*  FUNTION:       �Ͽ������ݿ������
*  INPUT:         
*  OUTPUT:        �ɹ�ʧ��
*  OTHERS:        ������˵��������
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

	//�Ͽ����ݿ�����	
	nRet = pDbService->DbDestroyConnection();
	if (nRet != XSUCC)
	{
		g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_ERR),"DbDestroyConnection error,nRet=%d",nRet);
		return XERROR;
	}

	g_bInitDb = XFALSE;	 //��ȫ�ֱ�����Ϊfalse	
	g_callbackTraceFunOfHssHDb(XNULL, MD(FID_AGENTDB,PL_DBG),"leaving DestroyDb success,g_bInitDb=%d",g_bInitDb);
	return XSUCC;
	
}

