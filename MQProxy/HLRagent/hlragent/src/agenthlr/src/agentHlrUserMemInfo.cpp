/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��10��17��
**************************************************************************/
#include "xosshell.h"
#include "agentHlrCom.h"
#include "agentHlrUserMemInfo.h"
#include "agentHlrUserInfoQryProc.h"

#define   AGENTHLR_USRMEM_TIMERLEN    0*60*1000
map<string, tContext*> g_UserInfo;
t_XOSMUTEXID g_UserInfoMutext;

tClearContextTime g_ClearUserInfoTime = {0};/*Ĭ��Ϊ��ҹ12��*/

PTIMER  g_UsrInfotimerHandle = NULL;
XOS_HHASH     g_QryUserInfoHashTab = XNULL;
XOS_HHASH     g_CcbindexUidMapTab = XNULL;
XOS_HHASH     gHashTableDn = XNULL;
XU32 gHlrAgentCcbIndex = 1;

XU64 g_SriRequestTimeOut = 3*60*1000;/*�������������û����ݳ�ʱ Ĭ��3����*/
XU64 g_UpdateRequestTimeOut = AGENTHLR_USRMEM_TIMERLEN; /*������³�ʱ, Ĭ��Ϊ��Ҫ����,������<cfg.xml>�ļ���*/

/*****************************************************************************
 Prototype    : agentHLR_UserInfoInit
 Description  : �û��ڴ滺���ʼ��
 Input        : XVOID  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_UserInfoInit(XVOID)
{
    XS32 ret = 0;
    if (XSUCC != XOS_MutexCreate(&g_UserInfoMutext))
	{
		XOS_Trace(MD(FID_HLR, PL_EXP),"create  g_UserInfoMutext mutex failed!");
		return XERROR;	
	}

    g_QryUserInfoHashTab = XOS_HashConstruct(2* SCU_MAX_NUM_OF_QRYFSM,
        SCU_MAX_NUM_OF_QRYFSM, sizeof(XU16), sizeof(tUSM), "g_QryUserInfoHashTab");

    if(XNULLP == g_QryUserInfoHashTab)
    {
        XOS_Trace(MD(FID_HLR,PL_EXP),"g_QryUserInfoHashTab init fail");
        return XERROR;
    }



    /*tel ��uid��hash��ϵ*/
    gHashTableDn = XOS_HashConstruct(SCU_MAX_NUM_OF_TELMAP, SCU_MAX_NUM_OF_TELMAP, 
		                          LENGTH_OF_TEL , LENGTH_OF_UID , "gHashTableDn");
    if(XNULLP == gHashTableDn)
    {
        XOS_Trace(MD(FID_HLR,PL_EXP),"gHashTableDn init failed");
        return XERROR;
    }


    /*�˶�ʱ��Ϊ���ƿ�ȴ���ʱ��ʱ��*/
    if(XSUCC !=XOS_TimerReg(FID_HLR, 100, SCU_MAX_NUM_OF_QRYFSM, 0))
    {
        XOS_Trace(MD(FID_HLR,PL_EXP),"timer init fail");
        return XERROR;
    }

    /*�����ѭ����ʱ���� �û�������Ϣ�˲鶨ʱ��*/
    t_BACKPARA tBackPara = {0};

    tBackPara.para1 = TIMER_TYPE_AGENT_USERCHECK;
    
	ret = XOS_TimerCreate(FID_HLR, &g_UsrInfotimerHandle, TIMER_TYPE_LOOP, TIMER_PRE_LOW, &tBackPara);
	if (XERROR == ret)
	{
		XOS_Trace(MD(FID_HLR,PL_ERR), "XOS_TimerCreate  Err!");
		return XERROR;
	}

	ret = XOS_TimerBegin(FID_HLR, g_UsrInfotimerHandle, TIMER_LENGTH_AGENT_MEM);/*ʱ��Ϊһ��Сʱ*/
	if (XERROR == ret)
	{
		XOS_Trace(MD(FID_HLR, PL_ERR), "TimerBegin is Err!");
		return XERROR;
	}
    
	return XSUCC;
}
/*****************************************************************************
 Prototype    : agentHLR_TimerUserInfoPro
 Description  : ��ʱ��ɾ����ѯ���ƿ����
 Input        : t_BACKPARA *tBackPara  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/25
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_TimerUserInfoPro(t_BACKPARA *tBackPara)
{
    
    XS32 retValue = 0;
    XU16 ccbIndex = 0;
    tUSM *pFind = XNULL;

    ccbIndex = tBackPara->para2;
    pFind = (tUSM*)XOS_HashElemFind(g_QryUserInfoHashTab, &ccbIndex);
    if(XNULL != pFind)
    {
        XOS_HashDelByElem(g_QryUserInfoHashTab, pFind);
        return XSUCC;
    }
    retValue = UserHash_DelbyccbIndex(ccbIndex);
    if(XSUCC != retValue)
    {

        
    }

    XOS_Trace(MD(FID_HLR,PL_EXP),"Delete Ctrl Block for time out ccbIndex = %u, retValue=%u,state=%u", 
        ccbIndex, retValue,pFind->state);

    return XSUCC;
}
/*****************************************************************************
 Prototype    : agentHLR_UserInfoCheckPro
 Description  : �û�������Ϣ�˲鶨ʱ����פ��ʱ�䳬ʱ��ɾ�� 
 Input        : t_BACKPARA *tBackPara  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/25
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_UserInfoCheckPro(t_BACKPARA *tBackPara)
{
    
    XS32 retValue = 0;
    XU64 cur_time = GetPresentTime();
    t_XOSTD TM_Struct = {0};

    XOS_GetTmTime(&TM_Struct);

    if(!(TM_Struct.dt_hour == g_ClearUserInfoTime.hour && TM_Struct.dt_min == g_ClearUserInfoTime.minitus))/*����24�� ��������ֱ�ӷ���*/
    {
        
        return XSUCC;
    }
    XOS_Trace(MD(FID_HLR, PL_LOG),"Del UserInfo %u  by timer", UserInfo_size());
    map<string,tContext*>::iterator it;
    
 
    for(it = g_UserInfo.begin(); it != g_UserInfo.end(); it++)
    {
        if(cur_time >= g_UpdateRequestTimeOut + it->second->updatetime)
        {
            
            if(XSUCC != UserInfo_Deletebyuid(it->second->uid))
            {
                XOS_Trace(MD(FID_HLR, PL_ERR),"Del UserInfo failed by uid");
            }
            else
            {
                XOS_Trace(MD(FID_HLR, PL_LOG),"Del UserInfo succefull by uid");           
            }
        }
  
    }
 

    return XSUCC;
}

/*****************************************************************************
 Prototype    : UserInfo_size
 Description  : ��ѯ���� ���ػ���Ĵ�С
 Input        : None
 Output       : None
 Return Value : XU32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XU32 UserInfo_size(XVOID)
{
    
    XU32 tmp = 0;
     
    XOS_MutexLock(&g_UserInfoMutext);
    tmp = g_UserInfo.size();
    XOS_MutexUnlock(&g_UserInfoMutext);
 
    
    return tmp;

}

/*****************************************************************************
 Prototype    : UserInfo_Insert
 Description  : �������
 Input        : None
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 UserInfo_Insert(XU8* uid, tContext* pContext)
{

    XS32 ret = XSUCC;
    XU8 uidStr[2*LENGTH_OF_UID+1] = {0};

    if(XNULL == pContext)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR),"pContext is NULL!");
        return XERROR;
    }

    DB_StrToHex(uid, uidStr, LENGTH_OF_UID);
    string tmp((XS8*)uidStr);
	
     
    XOS_MutexLock(&g_UserInfoMutext);
    map<string,tContext*>::iterator it = g_UserInfo.find(tmp);
    if(g_UserInfo.end() != it)
    {
        /*�� �����Ѵ���, ֱ�Ӹ��ǵ�*/
        DelTelUidHashTelLst(&it->second->tHashStat.subTelnoInfoList, uid);
        XOS_MemCpy(it->second, pContext, sizeof(tContext));
        XOS_MutexUnlock(&g_UserInfoMutext);
        XOS_Trace(MD(FID_HLR, PL_LOG),"uidStr=%s is exit when insert", uidStr);


        AddTelUidHashTelLst(&pContext->tHashStat.subTelnoInfoList, uid);
        return XSUCC;
    }

    tContext* ptIns = new tContext();
    
    XOS_MemCpy(ptIns, pContext, sizeof(tContext));
    g_UserInfo.insert(make_pair(tmp, ptIns));
    XOS_MutexUnlock(&g_UserInfoMutext);


    AddTelUidHashTelLst(&pContext->tHashStat.subTelnoInfoList, uid);
    
    return ret;
}

/*****************************************************************************
 Prototype    : UserInfo_Delete
 Description  : ɾ������
 Input        : None
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 UserInfo_Deletebyuid(XU8* uid)
{


    XU8 uidStr[2*LENGTH_OF_UID+1] = {0};
    
    DB_StrToHex(uid, uidStr, LENGTH_OF_UID);
    string tmp((XS8*)uidStr);

   
    XOS_Trace(MD(FID_HLR, PL_DBG),"Enter delete uid = %s", uidStr); 
    XOS_MutexLock(&g_UserInfoMutext);
    map<string,tContext*>::iterator it = g_UserInfo.find(tmp);
    if(g_UserInfo.end() != it)
    {
        /*ɾ���� uid��tel��map��ϵ*/
        DelTelUidHashTelLst(&it->second->tHashStat.subTelnoInfoList, uid);
        
        delete (it->second);
        it->second = XNULL;
        g_UserInfo.erase(tmp);
        
        XOS_Trace(MD(FID_HLR, PL_DBG),"Delete uid = %s succefully", uidStr); 
        XOS_MutexUnlock(&g_UserInfoMutext);
        return XSUCC;
    }
    XOS_Trace(MD(FID_HLR, PL_ERR),"Enter delete uid = %s failed", uidStr); 
    XOS_MutexUnlock(&g_UserInfoMutext);
    return XERROR;
}

/*****************************************************************************
 Prototype    : UserInfo_Qrybyuid
 Description  : ��ѯ���� ֱ�ӷ��ػ����ַ
 Input        : None
 Output       : None
 Return Value : tContext*
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
tContext*  UserInfo_Qrybyuid_Ext(XU8* uid)
{

    XS32 ret = XERROR;
    XU8 uidStr[2*LENGTH_OF_UID+1] = {0};



    DB_StrToHex(uid, uidStr, LENGTH_OF_UID);
    string tmp((XS8*)uidStr);
	
     
    XOS_MutexLock(&g_UserInfoMutext);
    map<string,tContext*>::iterator it = g_UserInfo.find(tmp);
    XOS_MutexUnlock(&g_UserInfoMutext);
    if(g_UserInfo.end() != it)
    {
        XOS_Trace(MD(FID_HLR, PL_DBG),"uidStr=%s is exit", uidStr);
        
        
        return (it->second);
    }
    XOS_Trace(MD(FID_HLR, PL_ERR),"uidStr=%s is not exit", uidStr);
    return XNULL;

}

/*****************************************************************************
 Prototype    : UserInfo_Qrybyuid
 Description  : ��ѯ����
 Input        : None
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 UserInfo_Qrybyuid(XU8* uid, tContext* pResult)
{

    XS32 ret = XERROR;
    XU8 uidStr[2*LENGTH_OF_UID+1] = {0};

    if(XNULL == pResult)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR),"pResult is NULL!");
        return XERROR;
    }

    DB_StrToHex(uid, uidStr, LENGTH_OF_UID);
    string tmp((XS8*)uidStr);
	
     
    XOS_MutexLock(&g_UserInfoMutext);
    map<string,tContext*>::iterator it = g_UserInfo.find(tmp);
    if(g_UserInfo.end() != it)
    {
        XOS_Trace(MD(FID_HLR, PL_DBG),"uidStr=%s is exit", uidStr);

        XOS_MemCpy(pResult, it->second, sizeof(tContext));
        ret = XSUCC;
    }


    XOS_MutexUnlock(&g_UserInfoMutext);
    return ret;

}
XS32 UserInfo_Qry(tUSM *pQryInfo, tContext* pResult)
{
    XS32 ret = 0;
    XU8* uid = NULL;
    XU64 cur_time = 0;
    tContext *pstContext = XNULL;
    
    if(XNULL == pQryInfo || XNULL == pResult)
    {

        return XERROR;
    }
    switch(pQryInfo->qrytype)
    {
        case e_QrybyUID:
            ret = UserInfo_Qrybyuid(pQryInfo->uid, pResult);
            break;
        case e_QrybyTEL:
            uid = (XU8*)XOS_HashElemFind(gHashTableDn, pQryInfo->tel);
            if(uid == XNULL)
            {
                XOS_Trace(MD(FID_HLR, PL_ERR),"Qry uid by tel failed on Local Agent");
                return XERROR;
            }
            XOS_MemCpy(pQryInfo->uid, uid, LENGTH_OF_UID);
            ret = UserInfo_Qrybyuid(pQryInfo->uid, pResult);
            break;
        default:
            ret = XERROR;
            XOS_Trace(MD(FID_HLR, PL_ERR),"Unknow qrytype=%u", pQryInfo->qrytype);
            break;



    }
    if(XSUCC != ret)
    {

        XOS_Trace(MD(FID_HLR, PL_ERR),"Qry UserInfo failed qrytype=%u", pQryInfo->qrytype);
    }
    else 
    {
        /*�û��������Ч�� ���*/
        XOS_Trace(MD(FID_HLR, PL_DBG),"Qry UserInfo succefully qrytype=%u", pQryInfo->qrytype);
        
        cur_time = GetPresentTime();


        if(cur_time >= g_UpdateRequestTimeOut + pResult->updatetime )
        {
            XOS_Trace(MD(FID_HLR, PL_ERR),"Qry UserInfo succefully qrytype=%u,but updatetime TimeOut", pQryInfo->qrytype);
            if(XSUCC != UserInfo_Deletebyuid(pQryInfo->uid))
            {
                XOS_Trace(MD(FID_HLR, PL_ERR),"Del UserInfo failed by uid=%08X", XOS_NtoHl(*(XU32*)pQryInfo->uid));
            }

            ret = XERROR;
        }
        
    }
    return ret;
}

#define ___QRY_HASH_CONSTRUCT____


XS32 UserHash_DelbyccbIndex(XU16 ccbIndex)
{
    XVOID *pFind = XNULL;
    
    pFind = XOS_HashElemFind(g_QryUserInfoHashTab, &ccbIndex);
    if(XNULL != pFind)
    {
        XOS_HashDelByElem(g_QryUserInfoHashTab, pFind);
        return XSUCC;
    }

    return XERROR;
}




/**********************************************************************
*
*  NAME:		GetHlrAgentCcbIndex
*  FUNTION:		���ccbindex
*  INPUT:		
*  OUTPUT:		XU32 :����ccbindex
*  OTHERS:      ������˵��������
**********************************************************************/
XU32 GetHlrAgentCcbIndex()
{

	XU32 ccbindex =0;
	
	ccbindex = gHlrAgentCcbIndex;
	gHlrAgentCcbIndex++;
	if(gHlrAgentCcbIndex > 0XFFFF)
	{
	 	gHlrAgentCcbIndex = 1;
	}

	return ccbindex;
}
#define ___TEL2UID____    
/*uid��tel��ӳ��*/

XS32 DelTelUidHash(XU8 *pDelTelno,XU8 *pDelUid)
{
	XU8*   uid = XNULL;
	XS32  ret= 0;
	XU8 hexTelno[LENGTH_OF_TEL*2+1]			= {0};
	DB_StrToHex(pDelTelno, hexTelno, LENGTH_OF_TEL);

   
	/*�ҵ���ϣ���иü�¼��λ��*/
	uid = (XU8* )XOS_HashElemFind(gHashTableDn,  pDelTelno);
	if (uid != XNULLP)
	{
		//�Ƚ�һ��hash�е�UID�ʹ���UID�Ƿ���ͬ������ͬ���򷵻�ʧ��
		if (XOS_MemCmp(uid,pDelUid,LENGTH_OF_UID) != 0)
		{
			ret = XERROR;

		}
		else
		{
			/*ɾ��ָ���ļ�¼*/
			ret = XOS_HashDelByElem(gHashTableDn, uid);
			
			
			ret = XSUCC;
			
		}

	}
	else
	{
		
		ret = XERROR;
	}
	

	return ret;

}

XS32 AddTelUidHash(XU8 *pAddTelno,XU8 *pAddUid)
{
	XU8*   uid = XNULL;
	XS32  ret= 0;
	XU8 hexTelno[LENGTH_OF_TEL*2+1]			= {0};
    XVOID *pHashMap = XNULL;
	DB_StrToHex(pAddTelno, hexTelno, LENGTH_OF_TEL);

	
   
	/*�ҵ���ϣ���иü�¼��λ��*/
	uid = (XU8* )XOS_HashElemFind(gHashTableDn, pAddTelno);
	if (uid != XNULLP)
	{
		//�Ƚ�һ��hash�е�UID�ʹ���UID�Ƿ���ͬ������ͬ����ɾ���ɵ�
		if (XOS_MemCmp(uid,pAddUid,LENGTH_OF_UID) != 0)
		{
			//ɾ��telno---uid�Ĺ�ϵ
	       XOS_HashDelByElem(gHashTableDn, uid);

		}
		else
		{
			return  XSUCC;
			
		}

	}
	
	/*Ҫ�����¼*/
	pHashMap = XOS_HashElemAdd(gHashTableDn, pAddTelno, pAddUid, XFALSE);
    if(XNULL == pHashMap)
    {
        /*�˷�֧��ʾhash��Դ�ľ�*/
        XOS_Trace(MD(FID_HLR, PL_ERR),"XOS_HashElemAdd is Err");
        return XERROR;
    }
	return  XSUCC;
	
}


XS32 AddTelUidHashTelLst(SSubTelnoInfoList* pTelLst, XU8* uid)
{
    

    for(XU32 index = 0; index < pTelLst->subTelnoNum; index ++)
    {
        AddTelUidHash(pTelLst->subTelnoList[index].totalTelno, uid);
    }
	
   return XSUCC; 
}

XS32 DelTelUidHashTelLst(SSubTelnoInfoList* pTelLst, XU8* uid)
{

    for(XU32 index = 0; index < pTelLst->subTelnoNum; index ++)
    {
        DelTelUidHash(pTelLst->subTelnoList[index].totalTelno, uid);
    }
	return XSUCC;
}
