/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMdbAndHardDb.cpp
* Author:       luhaiyan
* Date：        08-27-2014
* OverView:     
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


#include "agentDB_API.h"
#include "DbFactory.h"
#include "SPRCommFun.h"
#include "agentHDB_API.h"
#include "DbErrorCode.h"
#include "SPRMAndHDbComFun.h"
extern callbackTraceFun g_callbackTraceFunOfSPRDb;



XS32 agentDB_QryNationCodebyUID(XU32 fid, XU8 * pNation, XU8 *puid)
{
	XS32 nRet = XSUCC;
	XU32 index = 0;
    SDbReqSql szDbReq={0};
    
    sprintf((XS8 *)szDbReq.conditionSql, 
        "select NATIONAL_CODE from UDC_UID_LOCATION where USERID = '%02X%02X%02X%02X'", 
             puid[0], puid[1],puid[2],puid[3]);
	nRet = agentHDB_QryNationCodebyTelNo(&szDbReq, pNation);
	
	if(nRet == ERROR_CODE_DB_NO_DATA_EFFECT)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition  not find data %d",nRet);
		return ERR_CODE_PUI_DATA_NOT_EXIST;
	}
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %d",nRet);
		return ERR_CODE_DB_EXCEPTION;
	}


	return XSUCC;
}



XS32 agentDB_QryLocalNetWorkIdbyUID(XU32 fid, XU8 * pUid, XU8 *pNetWorkId)
{
	XS32 nRet = XSUCC;
	XU32 index = 0;
    XU8  uidStr[9]={0};
    SDbReqSql szDbReq={0};
    
    DB_StrToHex(pUid, uidStr, 4);
    sprintf((XS8 *)szDbReq.conditionSql, "select NETWORKID from UDC_UID_LOCATION where USERID = '%02X%02X%02X%02X'", 
             pUid[0], pUid[1],pUid[2],pUid[3]);
	nRet = agentHDB_QryLocalNetWorkingbyUid(&szDbReq, pNetWorkId);
	
	if(nRet == ERROR_CODE_DB_NO_DATA_EFFECT)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition  not find data %d",nRet);
		return ERR_CODE_PUI_DATA_NOT_EXIST;
	}
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %d",nRet);
		return ERR_CODE_DB_EXCEPTION;
	}


	return XSUCC;
}




XS32 agentDB_QryuidbyTel(XU32 fid, XU8 * pUid, XU8 *Tel)
{
	XS32 nRet = XSUCC;
	XU32 index = 0;
    XU8  telStr[33]={0};
    SDbReqSql szDbReq={0};
    
    DB_StrToHex(Tel, telStr, 16);
    sprintf((XS8 *)szDbReq.conditionSql, "SELECT USERID FROM HLR_USERID_TELNO_REL WHERE TOTAL_TELNO = '%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X'", 
             Tel[0],Tel[1],Tel[2],Tel[3],Tel[4],Tel[5],Tel[6],Tel[7],Tel[8],Tel[9],Tel[10],Tel[11],Tel[12],Tel[13],Tel[14],Tel[15]);
	nRet = agentHDB_QryuidbyTel(&szDbReq, pUid);
	
	if(nRet == ERROR_CODE_DB_NO_DATA_EFFECT)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"not find data %d",nRet);
		return ERR_CODE_PUI_DATA_NOT_EXIST;
	}
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR)," return %d",nRet);
		return ERR_CODE_DB_EXCEPTION;
	}


	return XSUCC;
}



/*****************************************************************************
 Prototype    : agentDB_QryuidbyAlias
 Description  : 根据别名查询uid
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
XS32 agentDB_QryuidbyAlias(XU32 fid, XU8 * pUid, XU8 *Alias)
{
	XS32 nRet = XSUCC;
	XU32 index = 0;
    XU8  telStr[33]={0};
    SDbReqSql szDbReq={0};
    
    
    sprintf((XS8 *)szDbReq.conditionSql, "SELECT USERID FROM HLR_USERBM_INFO WHERE USERBM = '%s'", Alias);
	nRet = agentHDB_QryuidbyAlias(&szDbReq, pUid);
	
	if(nRet == ERROR_CODE_DB_NO_DATA_EFFECT)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"not find data %d",nRet);
		return ERR_CODE_PUI_DATA_NOT_EXIST;
	}
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR)," return %d",nRet);
		return ERR_CODE_DB_EXCEPTION;
	}


	return XSUCC;
}


/*****************************************************************************
 Prototype    : agentDB_QryDynInfobyUID
 Description  : 查询用户的动态信息
 Input        : XU32 fid             
                XU8 * pUid           
                TXdbHdbDyn *pResult  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/8/25
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentDB_QryDynInfobyUID(XU32 fid, XU8 * pUid, TXdbHdbDyn *pResult)
{



    XS32 nRet = XSUCC;
	XU32 index = 0;
    XU8  uidStr[9] = {0};
    SDbReqSql szDbReq={0};
    
    DB_StrToHex(pUid, uidStr, 4);

   

    sprintf((XS8 *)szDbReq.conditionSql, \
            "SELECT PID,BSID,BSCID,LAIID,VSSID,HSSTATUS,XWIPV4,XWIPPORT,BSID_VOICE,\
            BSCID_VOICE,LAIID_VOICE,VSSID_VOICE,PORT,SAGIPV4,NETWORKID,AUTHNET,SAGIP_VOICE,SAGIP_DATA \
             FROM SUB_DYN_INFO WHERE USERID = '%02X%02X%02X%02X'", \
                            pUid[0], pUid[1],pUid[2],pUid[3]);
	nRet = agentHDB_QryDynInfobyUid(&szDbReq, pResult);
            
	if(nRet == ERROR_CODE_DB_NO_DATA_EFFECT)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition  not find data %d,uidStr = %s",nRet,uidStr);
		return ERR_CODE_PUI_DATA_NOT_EXIST;
	}
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %d,uidStr=%s",nRet,uidStr);
		return ERR_CODE_DB_EXCEPTION;
	}

    sprintf((XS8 *)szDbReq.conditionSql, "SELECT CAMTALK_TELNO, CSS_PUBLIC_IP, CSS_PRIVATE_IP,CSSID ,XWIPV4,XWIPPORT,CAMTALK_STATUS,\
        to_char(NVL(PREV_REG_DATE,to_date('19700101000000','YYYYMMDDHH24MISS')),'YYYYMMDDHH24MISS'), \
        to_char(NVL(CUR_REG_DATE,to_date('19700101000000','YYYYMMDDHH24MISS')), 'YYYYMMDDHH24MISS')  FROM HLR_CAMTALK_DYN_INFO where USERID = '%02X%02X%02X%02X'", 
             pUid[0], pUid[1],pUid[2],pUid[3]);

    nRet = agentHDB_QryCamTalkInfo(&szDbReq, pResult);
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %d,uidStr=%s",nRet,uidStr);
		return ERR_CODE_DB_EXCEPTION;
	}
	return nRet;

}

/*****************************************************************************
 Prototype    : agentDB_UpdateDynInfo
 Description  : 更新用户的动态信息（需要注意扩展）
 Input        : XU32 fid              
                XU8 * pUid            
                TXdbHdbDyn *pDynInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/8/25
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentDB_UpdateDynInfo(XU32 fid, XU8 * pUid, TXdbHdbDyn *pDynInfo)
{



    XS32 nRet = XSUCC;
    SDbReqSql szDbReq={0};
    XU8  Telstr[33] = {0};


    XU8 tmpString[512]={0};
    
    

    sprintf((XS8*)tmpString, "%s","UPDATE SUB_DYN_INFO SET ");
    strcat((XS8*)szDbReq.conditionSql, (XS8*)tmpString);
    
    sprintf((XS8*)tmpString, "PID='%02X%02X%02X%02X',BSID='%02X%02X',BSCID='%02X%02X%02X%02X',LAIID='%02X%02X%02X%02X%02X',\
        VSSID='%02X%02X%02X%02X' ", 
        pDynInfo->pid[0],pDynInfo->pid[1],pDynInfo->pid[2],pDynInfo->pid[3],
        pDynInfo->bsId[0],pDynInfo->bsId[1],
        pDynInfo->bscId[0],pDynInfo->bscId[1],pDynInfo->bscId[2],pDynInfo->bscId[3],
        pDynInfo->laiId[0],pDynInfo->laiId[1],pDynInfo->laiId[2],pDynInfo->laiId[3],pDynInfo->laiId[4],
        pDynInfo->vssId[0],pDynInfo->vssId[1],pDynInfo->vssId[2],pDynInfo->vssId[3]);
    strcat((XS8*)szDbReq.conditionSql, (XS8*)tmpString);

    sprintf((XS8*)tmpString, ",HSSTATUS='%02X',XWIPV4=%u,XWIPPORT=%u ", pDynInfo->hsStatus,pDynInfo->xwipv4,pDynInfo->xwipport);
    strcat((XS8*)szDbReq.conditionSql, (XS8*)tmpString);


    sprintf((XS8*)tmpString, ",BSID_VOICE='%02X%02X',BSCID_VOICE='%02X%02X%02X%02X',LAIID_VOICE='%02X%02X%02X%02X%02X',\
        VSSID_VOICE='%02X%02X%02X%02X' ", 
        pDynInfo->voiceAnchor.bsID[0],pDynInfo->voiceAnchor.bsID[1],
        pDynInfo->voiceAnchor.bscID[0],pDynInfo->voiceAnchor.bscID[1],pDynInfo->voiceAnchor.bscID[2],pDynInfo->voiceAnchor.bscID[3],
        pDynInfo->voiceAnchor.laiID[0],pDynInfo->voiceAnchor.laiID[1],pDynInfo->voiceAnchor.laiID[2],pDynInfo->voiceAnchor.laiID[3],pDynInfo->voiceAnchor.laiID[4],
        pDynInfo->voiceAnchor.vssID[0],pDynInfo->voiceAnchor.vssID[1],pDynInfo->voiceAnchor.vssID[2],pDynInfo->voiceAnchor.vssID[3]);
    strcat((XS8*)szDbReq.conditionSql, (XS8*)tmpString);

    sprintf((XS8*)tmpString, ",PORT='%02X',SAGIPV4=%u,NETWORKID='%s',SAGIP_VOICE=%u,SAGIP_DATA=%u ", 
        pDynInfo->pidPort,
        pDynInfo->sagIpv4,
        pDynInfo->cNetworkID,
        pDynInfo->sagIp_voice,
        pDynInfo->sagIp_data);
    strcat((XS8*)szDbReq.conditionSql, (XS8*)tmpString);

    sprintf((XS8*)tmpString, "WHERE USERID='%02X%02X%02X%02X'",pUid[0], pUid[1],pUid[2],pUid[3]);
    strcat((XS8*)szDbReq.conditionSql, (XS8*)tmpString);
    
	nRet = agentHDB_Update(&szDbReq);


            
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %d",nRet);
		return ERROR;
	}
    DB_StrToHex(pDynInfo->camtalkDynInfo.camtalkTelno, Telstr, LENGTH_OF_TEL);
    sprintf((XS8 *)szDbReq.conditionSql, \
        "UPDATE HLR_CAMTALK_DYN_INFO SET CAMTALK_TELNO='%s', XWIPV4=%u,XWIPPORT=%u,CAMTALK_STATUS=%u,CSSID='%02X%02X%02X%02X', CSS_PUBLIC_IP=%u, CSS_PRIVATE_IP=%u, PREV_REG_DATE=to_date('%s','YYYYMMDDHH24MISS'),CUR_REG_DATE=to_date('%s','YYYYMMDDHH24MISS') where USERID='%02X%02X%02X%02X'", 
        Telstr,
        pDynInfo->camtalkDynInfo.iXWIpV4,
        pDynInfo->camtalkDynInfo.sXWIpPort,
        pDynInfo->camtalkDynInfo.cCamtalkStatus,
        pDynInfo->camtalkDynInfo.cssId[0],pDynInfo->camtalkDynInfo.cssId[1],pDynInfo->camtalkDynInfo.cssId[2],pDynInfo->camtalkDynInfo.cssId[3],
        pDynInfo->camtalkDynInfo.iCssPublicIp, pDynInfo->camtalkDynInfo.iCssPrivateIp,
        pDynInfo->camtalkDynInfo.privRegDate,
        pDynInfo->camtalkDynInfo.curRegDate,
        pUid[0], pUid[1],pUid[2],pUid[3]);
    

    nRet = agentHDB_Update(&szDbReq);
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %d",nRet);
		return ERROR;
	}


    return XSUCC;

}

#define   __AGENTDB_API__SUBSCRIBE_INTERFACE___





XS32 agentDB_SubInsert(XU32 fid, XU32 type, XU8 *topic)
{
	XS32 nRet = XSUCC;
	
    
    
    
	SDbReqSql szDbReq={0};

	sprintf((XS8 *)szDbReq.conditionSql, "insert into SUBSCRIBE_INFO(SUBSTYPE, TOPICNAME) values(%u,'%s')", type, topic);
	nRet = agentHDB_SubOper(&szDbReq);
	
	if(nRet == ERROR_CODE_DB_NO_DATA_EFFECT)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition  not find data %d",nRet);
		return ERR_CODE_PUI_DATA_NOT_EXIST;
	}
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %d",nRet);
		return ERR_CODE_DB_EXCEPTION;
	}


	return XSUCC;
}





XS32 agentDB_SubDelete(XU32 fid, XU8 *topic)
{
	XS32 nRet = XSUCC;
	
    
    
    
	SDbReqSql szDbReq={0};

	sprintf((XS8 *)szDbReq.conditionSql, "delete from SUBSCRIBE_INFO where TOPICNAME = '%s' ", topic);
	nRet = agentHDB_SubOper(&szDbReq);
	
	if(nRet == ERROR_CODE_DB_NO_DATA_EFFECT)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition  not find data %d",nRet);
		return ERR_CODE_PUI_DATA_NOT_EXIST;
	}
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %d",nRet);
		return ERR_CODE_DB_EXCEPTION;
	}


	return XSUCC;
}





XS32 agentDB_SubQryAll(XU32 fid)
{
	XS32 nRet = XSUCC;
	
    
    
    
	SDbReqSql szDbReq={0};

	sprintf((XS8 *)szDbReq.conditionSql, "select SUBSTYPE, TOPICNAME from SUBSCRIBE_INFO");
	nRet = agentHDB_SubQryOper(&szDbReq);
	
	if(nRet == ERROR_CODE_DB_NO_DATA_EFFECT)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition  not find data %d",nRet);
		return ERR_CODE_PUI_DATA_NOT_EXIST;
	}
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %d",nRet);
		return ERR_CODE_DB_EXCEPTION;
	}


	return XSUCC;
}




XS32 agentDB_QryAllUtBindInfo(XU32 fid)
{
	XS32 nRet = XSUCC;
	
    SDbReqSql szDbReq={0};
    
    sprintf((XS8 *)szDbReq.conditionSql, "select PID,PORT,CREDIT_USERID,BIND_USERID,TERMINAL_STATUS,CREDIT_STATUS, NETID from LEASEHOLD_CONTROL");
	nRet = agentHDB_LoadUtInfo(&szDbReq);
	
	if(nRet == ERROR_CODE_DB_NO_DATA_EFFECT)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"  not find data %d",nRet);
		return ERR_CODE_PUI_DATA_NOT_EXIST;
	}
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR)," return %d",nRet);
		return ERR_CODE_DB_EXCEPTION;
	}


	return XSUCC;
}




XS32 agentDB_UptUtBindInfo(XU32 fid, SLeaseHoldCtlTable* pUtInfo)
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


    nRet = agentHDB_Update(&szDbReq);
	if(nRet != XSUCC)
	{
		g_callbackTraceFunOfSPRDb(NULL, MD(fid,PL_ERR),"DbCmSubscriber0QueryByCondition return %u",nRet);
		return ERROR;
	}

	return XSUCC;
}

