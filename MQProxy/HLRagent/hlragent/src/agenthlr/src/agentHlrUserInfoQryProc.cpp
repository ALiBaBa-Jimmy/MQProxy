/******************************************************************************
*******************************************************************************
***Copyright (C), Xinwei Telecom Technology Inc.******************
*******************************************************************************
*******************************************************************************
***File Name     : agentHlrUserInfoQryProc.cpp
** Version       : Initial Draft
** Author        : wangdanfeng
** Created       : 2015/9/14
** Last Modified :
  Description   : ���ļ���Ҫ�����ѯ�û���ϸ��Ϣ������
** History       :
** 1.Date        : 2015/9/14
**   Author      : wangdanfeng
***  Modification: Created file

******************************************************************************/


#include "agentHlrCom.h"
#include "agentHlrUserMemInfo.h"
#include "agentHlrUserInfoQryProc.h"

extern XOS_HHASH     g_QryUserInfoHashTab;
extern XOS_HHASH     gHashTableDn;



XS32 ProcessDecodeQryUserAllInfoReq(XU8 *pOutMsg, XU8 *pInMsg, XU16 InLen)
{
	XS8 nRes = XSUCC ;  // ���ر�־	
	XU16 charNum = 0;
	XU8 tag = 0;
	tUSM *pQryUserInfoReq = (tUSM *) pOutMsg ;

	for( charNum = 0 ;  charNum < InLen ;   )  // ����� charNum ���Լӿ����ں�������
	{
		tag  = pInMsg[charNum]; 
		charNum  +=  1 ; 
		switch( tag )  // case ҵ������
		{
		case e_UID_TAG:
			{
				charNum	 +=  agentHLR_GetValue(pQryUserInfoReq->uid, &pInMsg[charNum]);  
                pQryUserInfoReq->qrytype = e_QrybyUID;
                return XSUCC;
                
			}
			break;
		case e_TELNO_TAG:
			{
				charNum	 +=  agentHLR_GetValue(pQryUserInfoReq->tel, &pInMsg[charNum]);  
                pQryUserInfoReq->qrytype = e_QrybyTEL;
                return XSUCC;
			}
			break;
		case e_AGENT_CTRX:
			{
				charNum	 +=  agentHLR_GetValue((XU8*)&pQryUserInfoReq->ctrxreq, &pInMsg[charNum]);  
                pQryUserInfoReq->qrytype = e_QrybyGid;
                return XSUCC;
			}
			break;
		case e_Agent_USERBM:
			{
				charNum	 +=  agentHLR_GetValue(pQryUserInfoReq->UserBM, &pInMsg[charNum]);  
                pQryUserInfoReq->qrytype = e_QryUserBM;
                return XSUCC;
			}
			break;
		case e_Agent_Pid:
			{
				charNum	 +=  agentHLR_GetValue((XU8*)&pQryUserInfoReq->pidport, &pInMsg[charNum]);  
                pQryUserInfoReq->qrytype = e_QryUserPidPort;
                return XSUCC;
			}
			break;
        default :
			{
				charNum++ ;
				nRes = XERROR ; 					
			}
			break;	
		}
		
		if ( XERROR  ==  nRes ) 
		{
			break ;   // �˳�ѭ��
		}		
	}				
	
	return  nRes ;  

}
/*****************************************************************************
 Prototype    : ProcessEncodeQryUserAllInfoRsp
 Description  : ��hlr������ϸ��Ϣ
 Input        : tContext* pQryUserInfoRsp  
                t_Sj3_Oper_Rsp *pMsgOper   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 ProcessEncodeQryUserAllInfoRsp(tContext* pQryUserInfoRsp,t_Sj3_Oper_Rsp *pMsgOper)
{
	int i = 0;
    XU16 len = 0;
	t_Sj3_Oper_Rsp ossMsgReq = {0};	

	XU8 buf[4*1024] = {0};
    XU16 tmpLen = 0;

    len += agentHLR_EncodeTLV(buf, e_FLAG_TAG, &pQryUserInfoRsp->result, 1);
    len += agentHLR_EncodeTLV(buf+len, e_UID_TAG, pQryUserInfoRsp->uid, LENGTH_OF_UID);
    len += agentHLR_EncodeTLV(buf+len, e_STATIC_TAG, &pQryUserInfoRsp->tHashStat, sizeof(TXdbHashStat));
    len += agentHLR_EncodeTLV(buf+len, e_DYN_TAG, &pQryUserInfoRsp->tHashDyn, sizeof(TXdbHashDyn));
    len += agentHLR_EncodeTLV(buf+len, e_SS_TAG, &pQryUserInfoRsp->tHashSS, sizeof(TXdbHashSS));

    
	while(len > MAX_OSS_MSG_LEN)
	{
		tmpLen = MAX_OSS_MSG_LEN;
		XOS_MemSet(&ossMsgReq,0,sizeof(t_Sj3_Oper_Rsp));
		//����OSS��Ϣ����
		ossMsgReq.OpHead.ucIsSucc = 0;
		ossMsgReq.OpHead.ucPackId = i;//����ţ���0��ʼ
		ossMsgReq.OpHead.ucOperateObjId = pMsgOper->OpHead.ucOperateObjId;//��Ҫ���ظ��ݷõ�HLR
		ossMsgReq.OpHead.ucOperateTypeId = pMsgOper->OpHead.ucOperateTypeId;//��Ҫ���ظ��ݷõ�HLR
		ossMsgReq.OpHead.ucIsEnd = 0;//�Ƿ�Ϊ���һ����
		ossMsgReq.OpHead.usLen = XOS_HtoNs(tmpLen);
		ossMsgReq.Ph.ucPackType = SJ_RSP;
		ossMsgReq.Ph.usDlgId = pMsgOper->Ph.usDlgId;//��Ҫ���ظ��ݷõ�HLR
		XOS_MemCpy(ossMsgReq.usBuf, &buf[i*tmpLen], tmpLen);
		XOS_MemCpy(ossMsgReq.OpHead.uctopic,pMsgOper->OpHead.uctopic,XOS_StrLen(pMsgOper->OpHead.uctopic));//��Ҫ���ظ��ݷõ�HLR

		
		//HlrAgentSendOssMsg((XU8*)&ossMsgReq,sizeof(t_Sj3_Oper_Rsp)- 2000 + tmpLen);
		OperType_TransmittoUA(FID_HLR, &ossMsgReq, tmpLen+LENGTH_HEAD+LENGTH_TAIL);
		i++;
		len = len - tmpLen;
	}
	

	
	XOS_MemSet(&ossMsgReq,0,sizeof(t_Sj3_Oper_Rsp));
	//����OSS��Ϣ����
	ossMsgReq.OpHead.ucIsSucc = 0;
	ossMsgReq.OpHead.ucPackId = i;//����ţ���0��ʼ
	ossMsgReq.OpHead.ucOperateObjId = pMsgOper->OpHead.ucOperateObjId;//��Ҫ���ظ��ݷõ�HLR
	ossMsgReq.OpHead.ucOperateTypeId = pMsgOper->OpHead.ucOperateTypeId;//��Ҫ���ظ��ݷõ�HLR
	ossMsgReq.OpHead.ucIsEnd = 1;//�Ƿ�Ϊ���һ����
	ossMsgReq.OpHead.usLen = XOS_HtoNs(len);
	ossMsgReq.Ph.ucPackType = SJ_RSP;
	ossMsgReq.Ph.usDlgId = pMsgOper->Ph.usDlgId;//��Ҫ���ظ��ݷõ�HLR
	XOS_MemCpy(ossMsgReq.usBuf,&buf[i*MAX_OSS_MSG_LEN], len);
	XOS_MemCpy(ossMsgReq.OpHead.uctopic,pMsgOper->OpHead.uctopic,XOS_StrLen(pMsgOper->OpHead.uctopic));//��Ҫ���ظ��ݷõ�HLR
	
	
	//HlrAgentSendOssMsg((XU8*)&ossMsgReq,sizeof(t_Sj3_Oper_Rsp)- 2000 + len);	
    OperType_TransmittoUA(FID_HLR, &ossMsgReq, len+LENGTH_HEAD+LENGTH_TAIL);

	return XSUCC;

}

XS32 agent_ProcQryUsm(tUSM *pQryUSM)
{
//    XS32 RetValue;
    //tUSM tmp = {0};
    if(XNULL == pQryUSM)
    {
        XOS_Trace(MD(FID_HLR,PL_ERR), "pQryUSM is NULL");
        return XERROR;
    }

    /*��ѯ�ýṹ*/
    XVOID *pFind = XOS_HashElemFind(g_QryUserInfoHashTab, &pQryUSM->ccbIndex);
    if(XNULL == pFind)
    {
        if(XNULL == XOS_HashElemAdd(g_QryUserInfoHashTab, &pQryUSM->ccbIndex, pQryUSM, XFALSE))
        {
            /*HASH��Դ�ľ�*/
            XOS_Trace(MD(FID_HLR, PL_ERR), "g_QryUserInfoHashTab add Err ");
            XOS_HashClear(g_QryUserInfoHashTab);
            return XERROR;
        }
        return XSUCC;
    }
    else
    {
        /*�����ѯ�������д���*/
        
    }

    return XSUCC;
}

XS32 agent_StartTimer(XU16 ccbIndex)
{
    tUSM *pFind = XNULL;
	t_PARA      tmpPara = {0};
	t_BACKPARA  tmpBack = {0};

    pFind = (tUSM *)XOS_HashElemFind(g_QryUserInfoHashTab, &ccbIndex);
    if(XNULL == pFind)
    {
        XOS_Trace(MD(FID_HLR,PL_ERR), "pFind is NULL");
        return XERROR;
    }
	/*������ʱ��*/
	tmpPara.fid         = FID_HLR;
	tmpPara.len         = TIMER_LENGTH_AGENT_QRYUSM;
	tmpPara.mode        = TIMER_TYPE_ONCE;
	tmpPara.pre         = TIMER_PRE_LOW;

    tmpBack.para1 = TIMER_TYPE_AGENT_QRYUSM;
	tmpBack.para2 = pFind->ccbIndex;
		
    XOS_TimerStart(&pFind->Timer,&tmpPara,&tmpBack);
    XOS_Trace(MD(FID_HLR,PL_DBG), "timer start : ccbIndex=%u", ccbIndex);
    return XSUCC;
}

XS32 agentHLR_QryUserNetIdbyDNS(tUSM *pQryUSM, t_Sj3_Oper_Rsp *pMsgOper)
{
    t_Sj3_Oper_Rsp ossMsgReq = {0};	
    XU16 len = 0;
    XS32 ret = 0;
    XU8 buf[1024] = {0};

    /*������Ϣ�Ƿ�Ϊ�ظ���ѯ*/
    pQryUSM->ccbIndex = XOS_NtoHs(pMsgOper->Ph.usDlgId);
    pQryUSM->qrytime = GetPresentTime();
    pQryUSM->state = e_WaitRoutingFromDns;
	XOS_MemCpy(pQryUSM->srcNetID, pMsgOper->OpHead.uctopic, XOS_StrLen(pMsgOper->OpHead.uctopic));
    agent_ProcQryUsm(pQryUSM);

    //����OSS��Ϣ����
    if(pQryUSM->qrytype == e_QrybyUID)
    {
        len += agentHLR_EncodeTLV(buf, e_UID_TAG, pQryUSM->uid, LENGTH_OF_UID);
    }
    else if(pQryUSM->qrytype == e_QrybyTEL)
    {
        len += agentHLR_EncodeTLV(buf, e_TELNO_TAG, pQryUSM->tel, LENGTH_OF_TEL);

    }
    else if(pQryUSM->qrytype == e_QryUserBM)
    {
        len += agentHLR_EncodeTLV(buf, e_Agent_USERBM, pQryUSM->UserBM, LENGTH_OF_NICK_NAME);

    }
    ENCODEOSSREQ(ossMsgReq, e_QryNetWorkId_OperType, len, pQryUSM->ccbIndex);
    XOS_MemCpy(ossMsgReq.usBuf, buf, len);

    /*���ֶ�Ӧ��Ϊ����hlr������id*/
	XOS_MemCpy(ossMsgReq.OpHead.uctopic, pMsgOper->OpHead.uctopic, XOS_StrLen(pMsgOper->OpHead.uctopic));

    /*��udc���Ͳ�ѯnetworkid ������*/
    ret = OperType_Push(FID_HLR, &ossMsgReq, len+LENGTH_HEAD+LENGTH_TAIL, MQTT_CALLING_REQ_QryNetWork);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), "PUSH TO UDC Qry networkid error!ret = %u", ret);
    }
    
    /*������ѯ��ʱ��ʱ��*/
    agent_StartTimer(pQryUSM->ccbIndex);

    
    XOS_Trace(MD(FID_HLR,PL_DBG), "Exit : len = %u, ccbindex=%u", len, pQryUSM->ccbIndex);
    return ret;

}
#define  ___STEP__ONE_____
/*****************************************************************************
 Prototype    : agentHLR_QryMsgProc
 Description  : ��ѯ�û���Ϣ��Ϣ���������
 Input        : XU32 fid                 
                t_Sj3_Oper_Rsp *pHlrMsg  
                XU32 agentUaMsgLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_QryMsgProc(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    XU16 bufLen = 0;
    XS32 ret;
    tUSM tQryInfo = {0};
    tContext tUserInfo = {0};
    


    XOS_Trace(MD(fid, PL_DBG),"Enter ");
    bufLen = XOS_NtoHs(pHlrMsg->OpHead.usLen);



    ret = ProcessDecodeQryUserAllInfoReq((XU8*)&tQryInfo, pHlrMsg->usBuf, bufLen);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(fid,PL_ERR), "Qryinfo msg decode failed.");
        return XERROR;
    }
    tQryInfo.operatorTyp = e_Qry_OperType;

    /*��agent �����в�ѯ�û���������Ϣ*/
    ret = UserInfo_Qry(&tQryInfo, &tUserInfo);
    if(XSUCC == ret)
    {
        /*��ѯ�ɹ�ֱ�Ӹ�hlr����*/
        XOS_Trace(MD(fid,PL_DBG), "Qry User info on agent Succefully.");
        
        return ProcessEncodeQryUserAllInfoRsp(&tUserInfo, pHlrMsg);
    }
    
    /*agent���ڴ���û�в�ѯ���û�����Ϣ�� ��Ҫ������ز�ѯ*/
    /*��������DNS����uid�����ص� NETID*/
    

    /*��DNS/UDC����uid������·��*/
    return agentHLR_QryUserNetIdbyDNS(&tQryInfo, pHlrMsg);
}




XS32 ProcessDecodeQryNetIDRsp(XU8 *uid,XU8* pNetID,XU8* res, XU8 *pInMsg, XU16 InLen)
{
	XS8 nRes = XSUCC ;  // ���ر�־	
	XU16 charNum = 0;
	XU8 tag = 0;
	

	for( charNum = 0 ;  charNum < InLen ;   )  
	{
		tag  = pInMsg[charNum]; 
		charNum  +=  1 ; 
		switch( tag )  // case ҵ������
		{
		case e_FLAG_TAG:
          charNum	 +=  agentHLR_GetValue(res, &pInMsg[charNum]);
          break;
		case e_UID_TAG:
          
			charNum	 +=  agentHLR_GetValue(uid, &pInMsg[charNum]);  
			break;
		case e_NETWORKING_TAG:
			
			charNum	 +=  agentHLR_GetValue(pNetID, &pInMsg[charNum]);  
			break;

        default :
			{
				charNum++ ;
				nRes = XERROR ; 					
			}
			break;	
		}
		
		if ( XERROR  ==  nRes ) 
		{
			break ;   // �˳�ѭ��
		}		
	}				
	
	return  nRes ;  

}


#define  ___STEP__TWO_____
/*****************************************************************************
 Prototype    : agentHLR_QryNetRspMsgPro
 Description  : �����ѯ·����Ӧ��Ϣ
 Input        : XU32 fid                 
                t_Sj3_Oper_Rsp *pHlrMsg  
                XU32 agentUaMsgLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/15
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_QryNetRspMsgPro(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    XU16 bufLen = 0;
    tUSM *pFind = XNULL;
    XS32 ret = XSUCC;
    XU8 uid[LENGTH_OF_UID] = {0};
    XU8 NetID[LENGTH_OF_NETWORK_ID] = {0};
    t_Sj3_Oper_Rsp ossMsgReq = {0};
    XU16 len = 0;
    XU8 buf[512] = {0};
    XU8 result = 0;


    
    bufLen = XOS_NtoHs(pHlrMsg->OpHead.usLen);


    /*���� udc���ص�·����Ӧ��Ϣ*/
    ret = ProcessDecodeQryNetIDRsp(uid, NetID, &result, pHlrMsg->usBuf, bufLen);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(fid,PL_ERR), "Decode Err");
        return XERROR;
    }
    XU16 ccbIndex = XOS_NtoHs(pHlrMsg->Ph.usDlgId);
    XOS_Trace(MD(fid,PL_DBG), "Decode : result=%u,uid=%08X, NetID=%s,ccbIndex=%u.", result,XOS_NtoHl(*(XU32*)uid), NetID,ccbIndex);

    
    
    pFind = (tUSM *)XOS_HashElemFind(g_QryUserInfoHashTab, &ccbIndex);
    if(XNULL == pFind)
    {
        /*�޷��ҵ����ƿ飬����ʧ��*/
        XOS_Trace(MD(fid,PL_ERR), "pFind is NULL");
        return XERROR;
    }
    /*�رն�ʱ��*/
    XOS_TimerStop(fid, pFind->Timer);

    /*���״̬��*/
    if(e_WaitRoutingFromDns != pFind->state)
    {
        XOS_Trace(MD(fid,PL_ERR), "state Err %u", pFind->state);
        XOS_HashDelByElem(g_QryUserInfoHashTab, pFind);
        return XERROR;
    }
    
    if(result != XSUCC)
    {
        /*��ѯ�û��Ĺ�������ʧ��*/
        XOS_Trace(MD(fid,PL_ERR), "result Err %u", result);

        t_Sj3_Oper_Rsp ossMsgReq = {0};
        tContext userInfoRsp = {0};
        userInfoRsp.result = result;
        ENCODEOSSRSP(ossMsgReq, e_Qry_OperType, 0, XOS_HtoNs(pFind->ccbIndex));
        ProcessEncodeQryUserAllInfoRsp(&userInfoRsp, &ossMsgReq);

        XOS_HashDelByElem(g_QryUserInfoHashTab, pFind);
        return XERROR;
    }
    /*�л�״̬*/
    pFind->state = e_WaitUserInfoFromHomeHlr;
    XOS_MemCpy(pFind->uid, uid, LENGTH_OF_UID);

    
    //����OSS��Ϣ����
    len += agentHLR_EncodeTLV(buf, e_UID_TAG, pFind->uid, LENGTH_OF_UID);
    ENCODEOSSREQ(ossMsgReq, pFind->operatorTyp, len, pFind->ccbIndex);
    XOS_MemCpy(ossMsgReq.usBuf, buf, len);
	XOS_MemCpy(ossMsgReq.OpHead.uctopic, pFind->srcNetID, LENGTH_OF_NETWORK_ID);

    /*��udc���Ͳ�ѯnetworkid ������*/
    ret = OperType_Push(FID_HLR, &ossMsgReq, len+LENGTH_HEAD+LENGTH_TAIL, (XCHAR*)NetID);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), "PUSH TO UDC Qry networkid error!ret = %u", ret);
        XOS_HashDelByElem(g_QryUserInfoHashTab, pFind);
        return XERROR;
    }
  
    /*������ѯ��ʱ��ʱ��*/
    agent_StartTimer(pFind->ccbIndex);
    XOS_Trace(MD(fid, PL_DBG),"Exit");
    return XSUCC;
}
/*****************************************************************************
 Prototype    : agentHLR_QryNetIDRequestMsg
 Description  : �����ѯ·��������Ϣ
 Input        : XU32 fid                 
                t_Sj3_Oper_Rsp *pHlrMsg  
                XU32 agentUaMsgLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/15
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_QryNetIDRequestMsg(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    XU16 bufLen = 0;
    XS32 ret = XSUCC;
    
    XU8 NetID[LENGTH_OF_NETWORK_ID] = {0};
    t_Sj3_Oper_Rsp ossMsgReq = {0};
    XU16 len = 0;
    XU16 ccbIndex = 0;
    XU8 buf[512] = {0};
    tUSM tQryInfo = {0};
    XU8 telStr[33] = {0};
    XU8 result = e_Res_XSUCC;


    
    bufLen = XOS_NtoHs(pHlrMsg->OpHead.usLen);
    

    /*���� ��Ϣ*/
    ret = ProcessDecodeQryUserAllInfoReq((XU8*)&tQryInfo, pHlrMsg->usBuf, bufLen);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(fid,PL_ERR), "Decode Err: bufLen=%u.", bufLen);
        return XERROR;
    }



    
    XOS_Trace(MD(fid,PL_DBG), "Decode : uid=%08X.", XOS_NtoHl(*(XU32*)tQryInfo.uid));


    switch(tQryInfo.qrytype)
    {
        case e_QrybyUID:
            //ret = agentDB_QryLocalNetWorkIdbyUID(fid, tQryInfo.uid, NetID);
            break;
        case e_QrybyTEL:
            DB_StrToHex(tQryInfo.tel, telStr, LENGTH_OF_TEL);
            ret = MysqlDB_QryuidbyTel(fid, tQryInfo.uid, tQryInfo.tel);
            if(XSUCC != ret)
            {
                XOS_Trace(MD(fid, PL_ERR), " qry UID BY tel=%s failed", telStr);
                result = e_Res_QryRouteFail;
                //return XERROR;
            }
            break;
        case e_QryUserBM:
            ret = MysqlDB_QryuidbyAlias(fid, tQryInfo.uid, tQryInfo.UserBM);
            if(XSUCC != ret)
            {
                XOS_Trace(MD(fid, PL_ERR), " qry UID BY UserBM=%s failed",tQryInfo.UserBM);
                result = e_Res_QryRouteFail;
            }
            break;
       default:
            XOS_Trace(MD(fid, PL_ERR), "unknow qry type = %u", tQryInfo.qrytype);
            break;

    }

    if(result == e_Res_XSUCC)
    {
        /*��ѯ�����������*/
        ret = MysqlDB_QryLocalNetWorkIdbyUID(fid, tQryInfo.uid, NetID);
        if(XSUCC != ret)
        {
            XOS_Trace(MD(fid, PL_ERR), "Qry networkid Err: bufLen=%u, uid = %08X.", bufLen, XOS_NtoHl(*(XU32*)tQryInfo.uid));
            result = e_Res_QryRouteFail;
        }
    }

    XOS_Trace(MD(fid,PL_DBG), "uid=%08X, NetID=%s.", XOS_NtoHl(*(XU32*)tQryInfo.uid), NetID);

    
    //����OSS��Ϣ����
    len += agentHLR_EncodeTLV(buf, e_FLAG_TAG, &result, 1);
    len += agentHLR_EncodeTLV(buf+len, e_UID_TAG, tQryInfo.uid, LENGTH_OF_UID);
    len += agentHLR_EncodeTLV(buf+len, e_NETWORKING_TAG, NetID, LENGTH_OF_NETWORK_ID);
    ENCODEOSSRSP(ossMsgReq, e_QryNetWorkId_OperType, len, pHlrMsg->Ph.usDlgId);
    
    XOS_MemCpy(ossMsgReq.usBuf, buf, len);
	XOS_MemCpy(ossMsgReq.OpHead.uctopic, pHlrMsg->OpHead.uctopic, LENGTH_OF_NETWORK_ID);

    /*��udc���Ͳ�ѯnetworkid ��Rsp*/
    ret = OperType_Push(FID_HLR, &ossMsgReq, len+LENGTH_HEAD+LENGTH_TAIL, (XCHAR*)pHlrMsg->OpHead.uctopic);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), "PUSH TO UDC Qry networkid error!ret = %u", ret);
        
        return XERROR;
    }
    
    XOS_Trace(MD(fid, PL_DBG),"Exit");
    return XSUCC;
}
/*****************************************************************************
 Prototype    : Agent_UserInfoPackCollected
 Description  : �ռ��û���Ϣ�ķְ�
 Input        : tUSM *pFind  
                XU8 *buf     
                XU16 bufLen  
                XU8 packId   
                XU8 IsEnd    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/17
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 Agent_UserInfoPackCollected(tUSM *pFind, XU8 *buf, XU16 bufLen, XU8 packId, XU8 IsEnd)
{
    

    if(packId >= AGENT_TCPE_MAX_PACKNUM)
    {
        XOS_Trace(MD(FID_HLR, PL_DBG),"packId is Err %u", packId);
        return XERROR;
    }


    XOS_MemCpy(pFind->buf.singleBuf[packId].buf, buf, bufLen);
    pFind->buf.singleBuf[packId].bufLen = bufLen;

    if(IsEnd == XTRUE)
    {
        /*������ ���سɹ�*/
        pFind->buf.endPackId = packId;
        return XSUCC;
    }
    return XERROR;
}
/*****************************************************************************
 Prototype    : Agent_UserInfoPackedJionedAll
 Description  : ƴ���û���Ϣ
 Input        : tUSM *pFind    
                XU8 *pOut      
                XU32 * bufLen  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/17
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 Agent_UserInfoPackedJionedAll(tUSM *pFind, XU8 *pOut, XU32 * bufLen)
{
    XU32 i = 0;

    XU32 tmpLen = 0;
    for(i = 0; i <= pFind->buf.endPackId; i++)
    {

        XOS_MemCpy(pOut+tmpLen, pFind->buf.singleBuf[i].buf, pFind->buf.singleBuf[i].bufLen);
        tmpLen += pFind->buf.singleBuf[i].bufLen;

    }
    XOS_MemSet(&pFind->buf, 0, sizeof(tUserInfoBuf));
    *bufLen = tmpLen;
    
    return XSUCC;
}


XS8 ProcessDecodeUserInfo(XU8 *pOutMsg,XU8 *pInMsg,XU16 InLen)
{
	XS8 nRes = XSUCC ;  // ���ر�־	
	XU16 charNum  ;
	XU16 type = 0  ;
	XU32 tmp = 0;
	XU8 tag = 0;
	tContext *pUserInfoRsp = (tContext *) pOutMsg ;
	
	charNum = 0 ;   // ������ʼ��
	for( charNum = 0 ;  charNum < InLen ;   )  // ����� charNum ���Լӿ����ں�������
	{
		tag  = pInMsg[charNum]; 
		charNum  +=  1 ; 
		switch( tag )  // case ҵ������
		{
		case e_FLAG_TAG:
			{
				charNum	 +=  agentHLR_GetValue(&(pUserInfoRsp->result), &pInMsg[charNum]);  						
			}
			break;
		case e_UID_TAG :     // uid
			{
				charNum	 +=  agentHLR_GetValue(pUserInfoRsp->uid, &pInMsg[charNum]);  						
			}
			break ;
		case e_STATIC_TAG:
			{
				charNum += agentHLR_GetValue((XU8*)&(pUserInfoRsp->tHashStat), &pInMsg[charNum]);
			}
			break;
		case e_DYN_TAG:
			{
				charNum += agentHLR_GetValue((XU8*)&(pUserInfoRsp->tHashDyn), &pInMsg[charNum]);
			}
			break;
		case e_SS_TAG:
			{
				charNum += agentHLR_GetValue((XU8*)&(pUserInfoRsp->tHashSS), &pInMsg[charNum]);
			}
			break;
		default :
			{
				charNum++ ;
				nRes = XERROR ; 					
			}
			break;	
		}
		
		if ( XERROR  ==  nRes ) 
		{
			break ;   // �˳�ѭ��
		}		
	}				
	
	return  nRes ;  

}

/**********************************************************************
*
*  NAME:		AgentQryUserAllInfoRspMsgProc
*  FUNTION:		����֪ͨ��Ϣ��scu_auth������ִ���û���Ȩ��Ϣ�Ĵ���
*  INPUT:		TScuAuthInfoArg *pMsg
*  OUTPUT:		XS32 
*  OTHERS:      ������˵��������
**********************************************************************/
XS8 AgentQryUserAllInfoRspMsgProc(t_Sj3_Oper_Rsp *pMsgOper,XU8 *pMsg,XU16 msgLen, tUSM *pFind)
{
	XS32 nRet = XERROR;
	
	tContext userInfoRsp = {0};

    XU8 uidStr[9]={0};
    XOS_Trace(MD(FID_HLR, PL_DBG),"Enter ");
    
	
	nRet = ProcessDecodeUserInfo((XU8 *)&userInfoRsp,pMsg,msgLen);
	if (nRet != XERROR)
	{
	    DB_StrToHex(userInfoRsp.uid, uidStr, LENGTH_OF_UID);
        if (userInfoRsp.result == XSUCC)
        {
            /*��Ҫ�������ڴ� ����ṹ*/
            userInfoRsp.updatetime = GetPresentTime();
            UserInfo_Insert(userInfoRsp.uid, &userInfoRsp);
            XOS_Trace(MD(FID_HLR,PL_DBG), "succeful result=%u,uidStr=%s.", userInfoRsp.result,uidStr);

            
        }
        else 
        {
            /*��ѯʧ��*/
            XOS_Trace(MD(FID_HLR,PL_ERR), "Err result=%u, uidStr=%s.", userInfoRsp.result, uidStr);
        }
        t_Sj3_Oper_Rsp ossMsgReq = {0};

        /*��hlr���� �û���Ϣ��Ӧ*/
        ENCODEOSSRSP(ossMsgReq, e_Qry_OperType, 0, XOS_HtoNs(pFind->ccbIndex));
        if(e_QryUserBM == pFind->qrytype)
        {
            /* ? */
            XOS_MemCpy(userInfoRsp.tHashStat.userBm, pFind->UserBM, XOS_StrLen(pFind->UserBM));
        }
        ProcessEncodeQryUserAllInfoRsp(&userInfoRsp, &ossMsgReq);
	}
    else
    {
        XOS_Trace(MD(FID_HLR,PL_ERR), "Decode Msg Err");
    }

	

	return nRet;
}

#define  ___STEP__THREE_____
XS32 agentHLR_QryUserInfoRspMsgPro(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    /*�����û�����ϸ��Ϣ�����ڰ�̫��Ӧ�ò��Ϊ2�������գ���Ҫ�������*/
    XU16 ccbIndex = 0;
 
    


    ccbIndex = XOS_NtoHs(pHlrMsg->Ph.usDlgId);

    tUSM *pFind = (tUSM *)XOS_HashElemFind(g_QryUserInfoHashTab, &ccbIndex);
    if(XNULL == pFind)
    {
        /*�޷��ҵ����ƿ飬����ʧ��*/
        XOS_Trace(MD(fid,PL_ERR), "pFind is NULL,ccbindex=%u", ccbIndex);
        
        return XERROR;
    }



    /*���״̬��*/
    if(e_WaitUserInfoFromHomeHlr != pFind->state)
    {
        XOS_Trace(MD(fid,PL_ERR), "state Err %u", pFind->state);
        
        return XERROR;
    }

    XU16 usBufLen = XOS_NtoHs(pHlrMsg->OpHead.usLen);

    XOS_Trace(MD(fid, PL_LOG),"packid = %u.", pHlrMsg->OpHead.ucPackId);
    /*�ռ��û��ְ�*/
    XS32 ret = Agent_UserInfoPackCollected(pFind, pHlrMsg->usBuf, XOS_NtoHs(pHlrMsg->OpHead.usLen),
                         pHlrMsg->OpHead.ucPackId, pHlrMsg->OpHead.ucIsEnd);
    if(XSUCC != ret)
    {
        /*δ�յ��յ�������سɹ������ȹ�������һ����*/
        XOS_Trace(MD(fid, PL_DBG),"Wait for Next packid = %u.", pHlrMsg->OpHead.ucPackId+1);
        return XSUCC;
    }
    /*����ִ�����ˣ���ʾӦ�ò��յ�һ�������İ�*/


    /*�رն�ʱ��*/
    XOS_TimerStop(fid, pFind->Timer);
    
    /*ƴ���ݰ�����*/
    XU8 tmpBuf[4000] = {0};
    XU32 tmpLen = 0;
    Agent_UserInfoPackedJionedAll(pFind, tmpBuf, &tmpLen);


    
    /*���û�����Ϣװ�����ڴ棬�����͸�hlr��Ԫ*/
    AgentQryUserAllInfoRspMsgProc(pHlrMsg, tmpBuf, tmpLen, pFind);
    /*ɾ����uid ��ѯ���ƿ�*/
    XOS_HashDelByElem(g_QryUserInfoHashTab, pFind);
    XOS_Trace(MD(fid, PL_DBG),"Exit");
    return XSUCC;
}
