/******************************************************************************

                  ��Ȩ���� (C), xinwei, xinwei

 ******************************************************************************
  �� �� ��   : MysqlDB_API.cpp
  �� �� ��   : ����
  ��    ��   : wangdanfeng
  ��������   : 2015��12��4��
  ����޸�   :
  ��������   : Mysql���ݿ�ӿ�
  �����б�   :
              MysqlDB_QryAllUtBindInfo
              MysqlDB_QryLocalNetWorkIdbyUID
              MysqlDB_QryuidbyAlias
              MysqlDB_QryuidbyTel
  �޸���ʷ   :
  1.��    ��   : 2015��12��4��
    ��    ��   : wangdanfeng
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
 
#include "MysqlDB_API.h"
#include "DbFactoryMysql.h"
#include "DbFactory.h"
#include "SPRCommFun.h"
#include "agentHDB_API.h"
#include "MysqlHDB_API.h"
#include "DbErrorCode.h"
#include "SPRMAndHDbComFun.h"

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
extern callbackTraceFun g_callbackTraceFunOfSPRDb;

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/




/*****************************************************************************
 �� �� ��  : MysqlDB_QryLocalNetWorkIdbyUID
 ��������  : ����UID��ѯ�û���������
 �������  : XU32 fid         
             XU8 * pUid       
             XU8 *pNetWorkId  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��12��4��
    ��    ��   : wangdanfeng
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 MysqlDB_QryLocalNetWorkIdbyUID(XU32 fid, XU8 * pUid, XU8 *pNetWorkId)
{
	XS32 nRet = XSUCC;

    XU8  uidStr[9]={0};
    SDbReqSql szDbReq={0};

	//��ʼ�����ӣ����һ��connid���Ӻ�
	CDbService *pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
	if (pDbService == NULL)
	{
		g_callbackTraceFunOfSPRDb(XNULL, MD(fid,PL_ERR),"CDbService* is NULL");			
		return XERROR;
	}	
	XS32 connid = pDbService->DbCreateConn();
	if(XSUCC != pDbService->DBCheckConnection(connid))
	{
		g_callbackTraceFunOfSPRDb(XNULL, MD(fid,PL_ERR),"CDbService create connid error.");
		return XERROR;
	}
    
    DB_StrToHex(pUid, uidStr, 4);
    sprintf((XS8 *)szDbReq.conditionSql, "select NETWORKID from UDC_UID_LOCATION where USERID = '%02X%02X%02X%02X'", 
             pUid[0], pUid[1],pUid[2],pUid[3]);
    
	nRet = MysqlHDB_QryLocalNetWorkingbyUid(connid, &szDbReq, pNetWorkId);   

	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %d",nRet);
		
	}

    pDbService->DbDestroyConn(connid);
	return nRet;
}



/*****************************************************************************
 �� �� ��  : MysqlDB_QryuidbyTel
 ��������  : ���ݵ绰�����ѯUID
 �������  : XU32 fid    
             XU8 * pUid  
             XU8 *Tel    
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��12��4��
    ��    ��   : wangdanfeng
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 MysqlDB_QryuidbyTel(XU32 fid, XU8 * pUid, XU8 *Tel)
{
	XS32 nRet = XSUCC;

    XU8  telStr[33]={0};
    SDbReqSql szDbReq={0};
    
    DB_StrToHex(Tel, telStr, 16);
    sprintf((XS8 *)szDbReq.conditionSql, "SELECT USERID FROM HLR_USERID_TELNO_REL WHERE TOTAL_TELNO = '%s'", telStr);


	//��ʼ�����ӣ����һ��connid���Ӻ�
	CDbService *pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
	if (pDbService == NULL)
	{
		g_callbackTraceFunOfSPRDb(XNULL, MD(fid,PL_ERR),"CDbService* is NULL");			
		return XERROR;
	}	
	XS32 connid = pDbService->DbCreateConn();
	if(XSUCC != pDbService->DBCheckConnection(connid))
	{
		g_callbackTraceFunOfSPRDb(XNULL, MD(fid,PL_ERR),"CDbService create connid error.");
		return XERROR;
	}


    nRet = MysqlHDB_QryuidbyTel(connid, &szDbReq, pUid);
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR)," return %d",nRet);
		
	}
    
    pDbService->DbDestroyConn(connid);
	return nRet;
}



/*****************************************************************************
 Prototype    : agentDB_QryuidbyAlias
 Description  : ���ݱ�����ѯuid
 Input        : XU32 fid    
                XU8 * pUid  
                XU8 *Alias  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/21
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 MysqlDB_QryuidbyAlias(XU32 fid, XU8 * pUid, XU8 *Alias)
{
	XS32 nRet = XSUCC;
	XU32 index = 0;
    XU8  telStr[33]={0};
    SDbReqSql szDbReq={0};
    
    
    sprintf((XS8 *)szDbReq.conditionSql, "SELECT USERID FROM HLR_USERBM_INFO WHERE USERBM = '%s'", Alias);

	//��ʼ�����ӣ����һ��connid���Ӻ�
	CDbService *pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
	if (pDbService == NULL)
	{
		g_callbackTraceFunOfSPRDb(XNULL, MD(fid,PL_ERR),"CDbService* is NULL");			
		return XERROR;
	}	
	XS32 connid = pDbService->DbCreateConn();
	if(XSUCC != pDbService->DBCheckConnection(connid))
	{
		g_callbackTraceFunOfSPRDb(XNULL, MD(fid,PL_ERR),"CDbService create connid error.");
		return XERROR;
	}
    nRet = MysqlHDB_QryuidbyAlias(connid, &szDbReq, pUid);
	

	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR)," return %d",nRet);
		
	}

    pDbService->DbDestroyConn(connid);
	return nRet;
}





/*****************************************************************************
 �� �� ��  : MysqlDB_QryAllUtBindInfo
 ��������  : �����ް���Ϣȫ��װ�����ڴ�
 �������  : XU32 fid  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��12��4��
    ��    ��   : wangdanfeng
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 MysqlDB_QryAllUtBindInfo(XU32 fid)
{
	XS32 nRet = XSUCC;
	
    SDbReqSql szDbReq={0};
    
    sprintf((XS8 *)szDbReq.conditionSql, "select PID,PORT,CREDIT_USERID,BIND_USERID,TERMINAL_STATUS,CREDIT_STATUS, NETID from LEASEHOLD_CONTROL");


	//��ʼ�����ӣ����һ��connid���Ӻ�
	CDbService *pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
	if (pDbService == NULL)
	{
		g_callbackTraceFunOfSPRDb(XNULL, MD(fid,PL_ERR),"CDbService* is NULL");			
		return XERROR;
	}	
	XS32 connid = pDbService->DbCreateConn();
	if(XSUCC != pDbService->DBCheckConnection(connid))
	{
		g_callbackTraceFunOfSPRDb(XNULL, MD(fid,PL_ERR),"CDbService create connid error.");
		return XERROR;
	}
    nRet = MysqlHDB_LoadUtInfo(connid, &szDbReq);
	
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR)," return %d",nRet);
		
	}

    pDbService->DbDestroyConn(connid);
    
	return nRet;
}





XS32 MysqlDB_UptUtBindInfo(XU32 fid, SLeaseHoldCtlTable* pUtInfo)
{
	XS32 nRet = XSUCC;
	
    SDbReqSql szDbReq={0};
    
    sprintf((XS8 *)szDbReq.conditionSql, "UPDATE LEASEHOLD_CONTROL SET CREDIT_USERID='%02X%02X%02X%02X',BIND_USERID='%02X%02X%02X%02X',TERMINAL_STATUS=%u,CREDIT_STATUS=%u WHERE PID='%02X%02X%02X%02X' AND PORT=%u ",
        pUtInfo->creditUid[0],pUtInfo->creditUid[1],pUtInfo->creditUid[2],pUtInfo->creditUid[3],
        pUtInfo->bindUid[0],pUtInfo->bindUid[1],pUtInfo->bindUid[2],pUtInfo->bindUid[3],
        pUtInfo->terminalStatus,
        pUtInfo->creditStatus,
        pUtInfo->pid[0],pUtInfo->pid[1],pUtInfo->pid[2],pUtInfo->pid[3],
        pUtInfo->port);

	//��ʼ�����ӣ����һ��connid���Ӻ�
	CDbService *pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
	if (pDbService == NULL)
	{
		g_callbackTraceFunOfSPRDb(XNULL, MD(fid,PL_ERR),"CDbService* is NULL");			
		return XERROR;
	}	
	XS32 connid = pDbService->DbCreateConn();
	if(XSUCC != pDbService->DBCheckConnection(connid))
	{
		g_callbackTraceFunOfSPRDb(XNULL, MD(fid,PL_ERR),"CDbService create connid error.");
		return XERROR;
	}


    nRet = MysqlHDB_Update(connid, &szDbReq);
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"MysqlHDB_Update return %u",nRet);
		
	}
    
    pDbService->DbDestroyConn(connid);
    
	return nRet;
}

