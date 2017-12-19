
#ifdef  __cplusplus
    extern  "C"{
#endif

#include "xosshell.h"
#include "../../hashlst/inc/saap_def.h"
#include "../../hashlst/inc/syshash.h"
#include "../../saap/inc/saappublichead.h"

SAAP_SPA_CLI_DATA_T gstSaapSpaCliData;
extern XU32 TcpeGetLinkHandle(XU32 ulLinkIndex);
extern t_IPADDR TcpeGetPeerIp(XU32 ulLinkIndex);
XS32 SAAP_NumCvt2Bcd(XCHAR *pStr, XU8 *pBcd, XU8 *pLen);
XEXTERN XU8 g_Tcp_Trace;

XU32 Saap_GetClock();//8888

/********************************** 
��������    : SYS MSGSEND
����        : 
�������    : 2008��9��24�� CodeReview
��������    : 
����        : COMMON_HEADER_SAAP *psCpsMsg
����ֵ      : int 
˵��:���͸�NTL����Ϣ����������Ϣ
=>NTL MSG_IP_CLOSEREQ
=>NTL MSG_IP_CONFIG
************************************/
int SYS_MSGSEND( COMMON_HEADER_SAAP *psCpsMsg) //�ڲ��ӿ� 8888
{
    //������·��ʼ����Ϣ��ntl
    t_XOSCOMMHEAD* pxosMsg = XNULL;
    XU16 usMsgLen  = 0;
    if(XNULL == psCpsMsg)
    {
       return XERROR;
    }
    if(FID_TCPE == psCpsMsg->stSender.ucFId)
    {
        if(g_Tcp_Trace)
        {
          XOS_Trace(MD(FID_TCPE,PL_MIN),"Enter SYS MSGSEND,srcFid=%d,dstFid=%d",psCpsMsg->stSender.ucFId,psCpsMsg->stReceiver.ucFId);
        }
    }
    else
    {
       if(g_Tcp_Trace)
       {
         XOS_Trace(MD(FID_SAAP,PL_MIN),"Enter SYS MSGSEND,srcFid=%d,dstFid=%d",psCpsMsg->stSender.ucFId,psCpsMsg->stReceiver.ucFId);
       }
    }
    usMsgLen = psCpsMsg->usMsgLen;
    // if datareq 2 ntl
    if(FID_NTL == psCpsMsg->stReceiver.ucFId && SAAP_DATA_REQ == psCpsMsg->usMsgId)
    {
        return TCPE_SendMsg2Ip( psCpsMsg);//tcpe = > NTL send data 8888
    }

    pxosMsg= (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(psCpsMsg->stSender.ucFId, psCpsMsg->usMsgLen);
    if(XNULL == pxosMsg)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"SYS MSGSEND malloc mem fail!");
        return XERROR;
    }

    //��Ϣͷת��
    pxosMsg->datasrc.PID    = XOS_GetLocalPID();
    pxosMsg->datasrc.FID    = psCpsMsg->stSender.ucFId;

    /*2009/04/28 modify below for saap_senderr_rsp process,submsg is src fid*/
    //pxosMsg->datasrc.FsmId  =0;
    pxosMsg->datasrc.FsmId  = psCpsMsg->stSender.usFsmId;
    /*2009/04/28 modify above for saap_senderr_rsp process*/

    pxosMsg->datadest.PID   = XOS_GetLocalPID();
    pxosMsg->datadest.FID   = psCpsMsg->stReceiver.ucFId;
    /*2009/04/28 modify below for saap_senderr_rsp process,submsg is src fid*/
    //pxosMsg->datadest.FsmId  =0;
    pxosMsg->datadest.FsmId  = psCpsMsg->stReceiver.usFsmId;
    /*2009/04/28 modify above for saap_senderr_rsp process*/
    pxosMsg->msgID          = psCpsMsg->usMsgId;
    /*2009/04/28 add below for saap_senderr_rsp process,submsg is src fid*/
    pxosMsg->subID= psCpsMsg->subID;
    /*2009/04/28 add above for saap_senderr_rsp process*/
    pxosMsg->prio           = eNormalMsgPrio;
    pxosMsg->length         = psCpsMsg->usMsgLen;
    //����NTL����Ϣ,��Ϣ��ҪתΪXOSƽ̨����Ϣ��
    if(FID_NTL == psCpsMsg->stReceiver.ucFId)
    {
        switch(pxosMsg->msgID)
        {
            case SAAP_DATA_CFG:
                pxosMsg->msgID = eLinkInit;//8888 delete
                break;
            case SAAP_DATA_REQ:
                pxosMsg->msgID = eSendData;//8888 ingnore 
                break;
            case SAAP_DATA_IND:
                pxosMsg->msgID = eDataInd; //8888 delete
                break;
        }
    }

    //��Ϣ���ݿ���
    XOS_MemCpy(pxosMsg->message,psCpsMsg + 1,psCpsMsg->usMsgLen);//9999

    //������Ϣ
    if(XSUCC != XOS_MsgSend(pxosMsg))
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR), "SYS MSGSEND XOS_MsgSend() fail!");
        XOS_MsgMemFree(pxosMsg->datasrc.FID,  pxosMsg);
        return XERROR;
    }
    if(FLAG_YES == SAAP_MsgNeedToMnt(psCpsMsg)) //������һ���ͻ�������
    {
        SAAP_SendMsgToMnt(psCpsMsg);
        Saap_SpaCliClsSendFlag();//9999
    }
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_SAAP,PL_MIN), "out SYS MSGSEND");
    }
    return XSUCC;
}

/********************************** 
��������    : TCPE_SendMsg2Ip
����        : 
�������    : 2008��9��24�� CodeReview
��������    : 
����        : COMMON_HEADER_SAAP *psCpsMsg
����ֵ      : int 
************************************/
int TCPE_SendMsg2Ip( COMMON_HEADER_SAAP *psCpsMsg)
{
//������·��ʼ����Ϣ��ntl
    t_XOSCOMMHEAD* pxosMsg = XNULL;
    t_DATAREQ  *pDataReq = XNULL;
    XCHAR      *pBearData = XNULL;
    XU16 usMsgLen  = 0;
    if(g_Tcp_Trace)
    {

       XOS_Trace(MD(FID_TCPE,PL_MIN),"Enter TCPE SendMsg2Ip,srcFid=%d,dstFid=%d,LinkIdx=%d",
       psCpsMsg->stSender.ucFId,psCpsMsg->stReceiver.ucFId,psCpsMsg->stSender.usFsmId);
    }

    usMsgLen = psCpsMsg->usMsgLen;

    //������һ��DATA_REQ���ڴ�
    pxosMsg= (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(psCpsMsg->stSender.ucFId, sizeof(t_DATAREQ));
    if(XNULL == pxosMsg)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"TCPE SendMsg2Ip XOS_MsgMemMalloc fail!r\n");
        return XERROR;
    }

    //��Ϣͷת��
    pxosMsg->datasrc.PID    = XOS_GetLocalPID();
    pxosMsg->datasrc.FID    = psCpsMsg->stSender.ucFId;
    pxosMsg->datasrc.FsmId  = 0;
    pxosMsg->datadest.PID   = XOS_GetLocalPID();
    pxosMsg->datadest.FID   = psCpsMsg->stReceiver.ucFId;
    pxosMsg->datadest.FsmId = 0;
    pxosMsg->msgID          = psCpsMsg->usMsgId;
    pxosMsg->prio           = eNormalMsgPrio;
    pxosMsg->length         = sizeof(t_DATAREQ);

    //����NTL����Ϣ,��Ϣ��ҪתΪXOSƽ̨����Ϣ��
    switch(psCpsMsg->usMsgId)
    {
        case MSG_IP_CONFIG:
            pxosMsg->msgID = eLinkInit; //8888 delete
            break;
        case SAAP_DATA_REQ:
            pxosMsg->msgID = eSendData;
            break;
        default:
            break;
    }

    pDataReq = (t_DATAREQ*)(pxosMsg->message);
    pxosMsg->length = sizeof(t_DATAREQ);
    pDataReq->msgLenth = usMsgLen;
    pDataReq->linkHandle = (HLINKHANDLE)TcpeGetLinkHandle(psCpsMsg->stSender.usFsmId); //psCpsMsg->stReceiver.usFsmId; //���Ӻ�,��Ӧ������
    pDataReq->dstAddr = TcpeGetPeerIp(psCpsMsg->stSender.usFsmId);
    pBearData = (XCHAR*)XOS_MemMalloc(psCpsMsg->stSender.ucFId, usMsgLen);
    if( XNULL == pBearData )
    {
        XOS_Trace(MD(FID_TCPE, PL_ERR),"TCPE SendMsg2Ip XOS_MemMalloc failed.");
        XOS_MsgMemFree(psCpsMsg->stSender.ucFId, (t_XOSCOMMHEAD *)pxosMsg);
        return XERROR;
    }
    XOS_MemCpy(pBearData, psCpsMsg + 1, usMsgLen);
    pDataReq->pData = pBearData;
    if(g_Tcp_Trace)
    {
        XOS_Trace(MD(FID_TCPE,PL_MIN), "pxosMsg->msgID=%d",pxosMsg->msgID);
        XOS_Trace(MD(FID_TCPE,PL_MIN), "pxosMsg->length=%d",pxosMsg->length);
        XOS_Trace(MD(FID_TCPE,PL_MIN), "pxosMsg->msg[pDataReq].linkHandle=%d",pDataReq->linkHandle);
        XOS_Trace(MD(FID_TCPE,PL_MIN), "pxosMsg->msg[pDataReq].msglen=%d",pDataReq->msgLenth);
    }

    //������Ϣ
    if(XSUCC != XOS_MsgSend(pxosMsg))
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR), "TCPE SendMsg2Ip fail!");
        XOS_MsgMemFree(pxosMsg->datasrc.FID, pxosMsg);
        XOS_MemFree(pxosMsg->datasrc.FID, pBearData);
        return XERROR;
    }
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN),"TCPE SendMsg2Ip XOS_MsgSend succ,srcFid=%d,dstFid=%d.",pxosMsg->datasrc.FID,pxosMsg->datadest.FID);
      XOS_Trace(MD(FID_TCPE,PL_MIN),"out TCPE SendMsg2Ip.");
    }
    return XSUCC;
}

/********************************** 
��������    : SAAP_StartTimer
����        : 
�������    : 2008��9��24�� CodeReview
��������    : 
����        : PTIMER *ptHandle
����        : XU32 fid
����        : XU32 len
����        : XU32 tmName
����        : XU32 para
����        : XU32 mode mode 0-ѭ��     1 ��ѭ��
����ֵ      : int 
************************************/
int SAAP_StartTimer(PTIMER *ptHandle,XU32 fid,XU32 len,XU32 tmName,XU32 para,XU32 mode )
{
    t_PARA tmrPara;
    t_BACKPARA tmrBackPara;

    tmrPara.fid = fid;
    tmrPara.len = len;
    tmrPara.mode= mode;       /* loop or once */
    tmrPara.pre = TIMER_PRE_LOW;  /* ����high or low */

    tmrBackPara.para1 = fid;
    tmrBackPara.para2 = tmName;
    tmrBackPara.para3 = para;
    tmrBackPara.para4 = 0;

    if(XSUCC !=XOS_TimerStart(ptHandle,&tmrPara, &tmrBackPara))
    {
        return XERROR;
    }
    return XSUCC;
}

/********************************** 
��������    : SAAP_SockGetV4Addr
����        : 
�������    : 2008��9��24�� CodeReview
��������    : 
����        : XCHAR *cIP
����        : XU32 *addr
����ֵ      : XS32 
************************************/
XS32 SAAP_SockGetV4Addr(XCHAR *cIP, XU32 *addr)
{
    *addr = inet_addr(cIP);
    return(XSUCC);
}

XS32 SYS_Str2Bcd(XCHAR * bcdstr) //9999
{
    XU32 ulLen  = 0;
    XU32 value  = 0;
    XU8 newBcdStr[100];

    ulLen = strlen(bcdstr);
    if(ulLen > 8)
    {
        return BLANK_ULONG;
    }
    strcpy(newBcdStr,"0x");
    strcat(newBcdStr,bcdstr);
    //8888,�����ת��Ϊ0xffffffff = 4294967295
    XOS_StrToNum(newBcdStr, &value);
    return value;
}

/********************************** 
��������    : SYS XOSMsg2CpsMsg
����        : 
�������    : 2008��9��24�� CodeReview
��������    : Ԥ����Datain����Ϣ����ΪCPS����Ϣ,
              ͬʱ����·��Ϊ����Ϣ�Ĳ���
              ��ʱͬʱ��������Ϣ���屣��.
����        : t_XOSCOMMHEAD *pxosMsg
����        : COMMON_HEADER_SAAP *pCpsMsg
����ֵ      : XS32 
************************************/
XS32 SYS_XOSMsg2CpsMsg(t_XOSCOMMHEAD *pxosMsg,COMMON_HEADER_SAAP *pCpsMsg)
{
    //��Ϣͷת��
    t_DATAIND *pDataInd = XNULL;

    pCpsMsg->stReceiver.ucModId = pxosMsg->datadest.PID;
    pCpsMsg->stReceiver.ucFId = pxosMsg->datadest.FID;
    pCpsMsg->stReceiver.usFsmId = pxosMsg->datadest.FsmId;

    pCpsMsg->stSender.ucModId = pxosMsg->datasrc.PID;
    pCpsMsg->stSender.ucFId = pxosMsg->datasrc.FID;
    pCpsMsg->stSender.usFsmId = pxosMsg->datasrc.FsmId;

    pCpsMsg->usMsgId        = pxosMsg->msgID;
    pCpsMsg->usMsgLen       =  pxosMsg->length;

    //��Ϣ���ݿ���,�����NTL����������Ϣ��Ҫ���⴦��
    if(FID_NTL == pxosMsg->datasrc.FID && eDataInd == pxosMsg->msgID)
    {
        pDataInd  = (t_DATAIND*)pxosMsg->message;
        pCpsMsg->stReceiver.usFsmId = (XU32)pDataInd->appHandle;//9999 tcpe ��·��
        pCpsMsg->usMsgLen = pDataInd->dataLenth;
        if(pCpsMsg->usMsgLen > ( SAAP_MAX_MSG_LEN - sizeof(COMMON_HEADER_SAAP) ) )
        {
            XOS_Trace(MD(FID_SAAP,PL_ERR),"SYS XOSMsg2CpsMsg msgLen(%d) is too long.",pCpsMsg->usMsgLen );
            XOS_MemFree(FID_TCPE, (XVOID*)pDataInd->pData);
            return XERROR;
        }
        XOS_MemCpy(pCpsMsg + 1,pDataInd->pData,pDataInd->dataLenth);
        //��ע:������ȫ�ֻ�����,��Ҫ������ջ�����
        XOS_MemFree(FID_TCPE, (XVOID*)pDataInd->pData);
    }
    else
    {
        //��ע:�����֧��ʵ�ǲ����ڵ�
        if(pCpsMsg->usMsgLen > ( SAAP_MAX_MSG_LEN - sizeof(COMMON_HEADER_SAAP) ) )
        {
            XOS_Trace(MD(FID_SAAP,PL_ERR),"SYS XOSMsg2CpsMsg msgLen(%d) is too long.",pCpsMsg->usMsgLen );
            return XERROR;
        }
        XOS_MemCpy(pCpsMsg + 1,pxosMsg->message,pCpsMsg->usMsgLen);
    }
    return XSUCC;
}

XCHAR * SAAP_IptoStr( XU32 inetAddress , char *pString )//8888��ƽ̨�Ľӿ�,������XOS_IptoStr
{
    XCHAR *temp = XNULLP;
    struct in_addr addr;
    addr.s_addr = XOS_HtoNl(inetAddress);
    if(XNULLP == (temp = inet_ntoa(addr )))
    {
        pString[0] = 0;
        return pString;
    }
    XOS_MemCpy(pString, temp, XOS_StrLen(temp)+1);
    return pString;

}

XS32 SAAP_StrTelToDn(XCHAR *pStrTel,XU8 *pDn)
{
    XU8 ucStrDn[SAAP_MAX_TELNO_LENGTH] = {0};
    XCHAR StrTmp[16] = {0};
    XU8 ucTelNum;

    if(XNULL==pStrTel || XNULL==pDn)//8888
    {
        XOS_Trace( MD(FID_SAAP,PL_ERR), "SAAP_StrTelToDn input null parameter");
        return XERROR;    
    }

    XOS_Trace(MD(FID_SAAP,PL_MIN),"Now Enter In SAAP_StrTelToDn!");

    if( XOS_StrLen(pStrTel) > 16 )//8888
    {
        XOS_Trace( MD(FID_SAAP,PL_ERR), "SAAP_StrTelToDn tel num [%d] too long,error!", XOS_StrLen(pStrTel));
        return XERROR;
    }

    /*�绰����*/
    ucTelNum = (XU8)XOS_StrLen(pStrTel);
    XOS_MemCpy(StrTmp, pStrTel, ucTelNum);
    XOS_MemSet(ucStrDn, 0xFF, SAAP_MAX_TELNO_LENGTH);

    SAAP_NumCvt2Bcd(StrTmp,ucStrDn,&ucTelNum);
    XOS_MemCpy(pDn,ucStrDn,SAAP_MAX_TELNO_LENGTH);
    return XSUCC;
}

/********************************** 
��������    : SAAP_NumCvt2Bcd
����        : 
�������    : 2008��9��24�� CodeReview
��������    : ���ַ�����ʽ�ĺ���ת����BCD���ʽ
����        : XCHAR *pStr ����, �ַ�����ʽ
����        : XU8 *pBcd ���, BCD���ʽ����
����        : XU8 *pLen ����, �������
����ֵ      : XS32 
����:
        "33" => 0x33 
        "34" => 0x43
        "37" => 0x73
        "3468" => 0x4386
        "3768" => 0x7386
************************************/
XS32 SAAP_NumCvt2Bcd(XCHAR *pStr, XU8 *pBcd, XU8 *pLen)
{
    XS32 retVal = XSUCC;
    XU8 ucIdx;
    XU8 ucBuf;       /*BCD����������*/
    XU8 ucNum;       /*ȡ���ĺ���*/
    XU8 ucMaxLen;    /*ʵ��ȡ�ĳ���*/

    /*�����Ϸ��Լ��*/
    if( XNULL == pStr || XNULL == pBcd )
    {
        return XERROR;
    }

    ucMaxLen = (XU8)(XOS_StrLen(pStr));
    if( MAX_DIGITAL_LEN <= ucMaxLen )
    {
        ucMaxLen = MAX_DIGITAL_LEN - 1;
    }

    ucBuf = 0;
    for( ucIdx=0; ucMaxLen>ucIdx; ++ucIdx )
    {
        if(('f' == pStr[ucIdx])||('F' == pStr[ucIdx]))
        {
            ucNum = 0x0F;
        }
        else
        {
            ucNum = (XU8)(pStr[ucIdx] - '0');
        }
        /*ucIdxΪż��ʱȡ����λ, Ϊ����ʱȡ����λ*/
        if( 0 == (ucIdx & 0x01) )
        {
            pBcd[ucBuf] = ((BCD_NUM_MASK << 4) | (ucNum & BCD_NUM_MASK));
        }
        else
        {
            pBcd[ucBuf] &= (BCD_NUM_MASK);
            pBcd[ucBuf] |= ((ucNum & BCD_NUM_MASK) << 4);
            XOS_Trace(MD(FID_SAAP,PL_DBG), "SAAP_NumCvt2Bcd>  %0x", pBcd[ucBuf]);
            ++ucBuf;
            if( DIGITAL_BUFF_LEN <= ucBuf )
            {
                break;
            }
        }
    }

    if( XNULL != pLen )
    {
        *pLen = ucIdx;
    }

    XOS_Trace(MD(FID_SAAP,PL_DBG), "exit SAAP_NumCvt2Bcd>");
    return retVal;
}

/********************************** 
��������	: SAAP_MsgNeedToMnt
����		: 
�������	: 2008��9��25��
��������	: 
����		: COMMON_HEADER_SAAP *psCpsMsg
����ֵ		: XU32 
************************************/
XU32 SAAP_MsgNeedToMnt(COMMON_HEADER_SAAP *psCpsMsg)
{
    //saap->tcpe
    if(FID_SAAP == psCpsMsg->stSender.ucFId && FID_TCPE == psCpsMsg->stReceiver.ucFId)
    {
        return  SAAP_FilterSaap2Tcpe(psCpsMsg);
    }
#if 0
    //TCPE -> SAAP
    if(FID_TCPE == psCpsMsg->stSender.ucFId && FID_SAAP == psCpsMsg->stReceiver.ucFId)
    {
        return TCPE_FilterTcpe2Saap(psCpsMsg);
    }
#endif
    //SAAP->SRV
    if(FID_SAAP == psCpsMsg->stSender.ucFId && FID_TCPE != psCpsMsg->stReceiver.ucFId)
    {
        return SAAP_FilterSaap2Srv(psCpsMsg);
    }
    return FLAG_NO;
}

/********************************** 
��������    : SAAP_SendMsgToMnt
����        : 
�������    : 2008��9��24�� CodeReview
��������    : �½ӿ�,��MOnitor������Ϣ���ٱ���
����        : COMMON_HEADER_SAAP *psCpsMsg
����ֵ      : XU32 
************************************/
XU32 SAAP_SendMsgToMnt(COMMON_HEADER_SAAP *psCpsMsg)
{
    t_XOSCOMMHEAD* pxosMsg = XNULL;
    t_DATAREQ  *pDataReq = XNULL;
    XCHAR      *pBearData = XNULL;
    SAAP_SPA_MSG_HEAD_T * pSaapTraceMsgHead = XNULL;
    static XU32 ulSpaMsgSerial = 0;
    XU16 usMsgLen  = 0;
    XU32 i =0;

    usMsgLen = psCpsMsg->usMsgLen + sizeof(t_XOSCOMMHEAD) + sizeof(SAAP_SPA_MSG_HEAD_T);
    for(i =0;i< SAAP_MAX_SPA_CLI_NUM;i++)
    {
        if(FLAG_NO ==  gstSaapSpaCliData.cliData[i].busyflag)
        {
            continue;
        }

        if(FLAG_NO  == gstSaapSpaCliData.cliData[i].ulSend2MntFlag) //û�����������
        {
            continue;
        }

        //������һ��DATA_REQ���ڴ�
        pxosMsg= (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SAAP, sizeof(t_DATAREQ));
        if(XNULL == pxosMsg)
        {
            XOS_Trace(MD(FID_TCPE,PL_ERR),"SAAP_SengMsgToMnt XOS_MsgMemMalloc failed!r\n");
            return XERROR;
        }

        //��Ϣͷ
        pxosMsg->datasrc.PID    = XOS_GetLocalPID();
        pxosMsg->datasrc.FID    = FID_SAAP;
        pxosMsg->datasrc.FsmId  = 0;
        pxosMsg->datadest.PID   = XOS_GetLocalPID();
        pxosMsg->datadest.FID   = FID_NTL;
        pxosMsg->datadest.FsmId = 0;
        pxosMsg->msgID          = psCpsMsg->usMsgId;
        pxosMsg->prio           = eNormalMsgPrio;
        pxosMsg->length         =  sizeof(t_DATAREQ);
        pxosMsg->msgID = eSendData;

        pDataReq = (t_DATAREQ*)(pxosMsg->message);
        pxosMsg->length = sizeof(t_DATAREQ);

        //������ƽ̨����Ϣͷ,������������ͨ��tcp ���͸� monitor
        pDataReq->msgLenth = usMsgLen; 
        pDataReq->linkHandle = ( HLINKHANDLE )gstSaapSpaCliData.linkHandle;
        pDataReq->dstAddr.ip = gstSaapSpaCliData.cliData[i].IpAddr;
        pDataReq->dstAddr.port =  gstSaapSpaCliData.cliData[i].port;
        pBearData = (XCHAR*)XOS_MemMalloc(FID_SAAP, usMsgLen );
        if( XNULL == pBearData )
        {
            XOS_Trace(MD(FID_TCPE, PL_ERR), "SAAP_SengMsgToMnt() call XOS_MemMalloc failed.");
            XOS_MsgMemFree(FID_SAAP, (t_XOSCOMMHEAD *)pxosMsg);
            return XERROR;
        }
        pSaapTraceMsgHead = (SAAP_SPA_MSG_HEAD_T *)pBearData;
        pSaapTraceMsgHead->msgType = XOS_HtoNs(0x15e0); //���ڸ���
        pSaapTraceMsgHead->msgLen =XOS_HtoNs(usMsgLen-8); // 8 Ϊtrace msgtype ,msgBdyLen,traceId
        pSaapTraceMsgHead->TraceId = XOS_HtoNl(gstSaapSpaCliData.ulTraceId);
        pSaapTraceMsgHead->SendSerial = XOS_HtoNl(ulSpaMsgSerial++);
        pSaapTraceMsgHead->TimeLabel = XOS_HtoNl(Saap_GetClock()); //XOS_HtoNl(0xFFFFFFFF);
        pSaapTraceMsgHead->SrvMsgType = XOS_HtoNl(SAAP_SPA_MSG_TYPE); //saap��Ϣ
        //pSaapTraceMsgHead->MsgByteOrder = 0x00; //  byte order host
        if(FID_SAAP == psCpsMsg->stSender.ucFId && FID_TCPE == psCpsMsg->stReceiver.ucFId)
        {
            pSaapTraceMsgHead->SubMsgType =  0x01; //saap2tcpe msg
        }
        else if(FID_SAAP == psCpsMsg->stSender.ucFId && FID_TCPE != psCpsMsg->stReceiver.ucFId)
        {
            pSaapTraceMsgHead->SubMsgType =  0x02; //saap2srv msg
        }
        else
        {   
            XOS_MsgMemFree(FID_SAAP,(t_XOSCOMMHEAD *)pxosMsg);
            XOS_MemFree(FID_SAAP,pBearData);           
            XOS_Trace(MD(FID_SAAP,PL_INFO),"saap spa unknow msg type[%d => %d].",
            psCpsMsg->stSender.ucFId,psCpsMsg->stReceiver.ucFId);
            return XERROR;
        }

        XOS_MemCpy(pBearData + sizeof(SAAP_SPA_MSG_HEAD_T), psCpsMsg,usMsgLen - sizeof(SAAP_SPA_MSG_HEAD_T));
        pDataReq->pData = pBearData;

        //������Ϣ
        if(XSUCC != XOS_MsgSend(pxosMsg))
        {
            XOS_Trace(MD(FID_SAAP,PL_ERR), "SAAP_SengMsgToMnt fail!");
            XOS_MsgMemFree(FID_SAAP,pxosMsg);
            XOS_MemFree(FID_SAAP,pBearData);
            return XERROR;
        }
    }
    return XSUCC;
}

/********************************** 
��������    : Saap_SpaCliFindCcbByIp
����        : 
�������    : 2008��9��24�� CodeReview
��������    : find the spa cli ccb by ip and port
����        : XU32 ip
����        : XU32 port
����ֵ      : XU32 
************************************/
XU32 Saap_SpaCliFindCcbByIp(XU32 ip,XU32 port)
{
    XU32 i  =0;
    XU32 idx  = BLANK_ULONG;
    for(i= 0;i< SAAP_MAX_SPA_CLI_NUM;i++)
    {
        if(FLAG_NO == gstSaapSpaCliData.cliData[i].busyflag)
        {
            continue;
        }
         if( gstSaapSpaCliData.cliData[i].IpAddr == ip
         && gstSaapSpaCliData.cliData[i].port   == port)
        {
            idx = i;
        }
    }
    return idx;
}

/* save filter data ,when it change*/
/********************************** 
��������	: Saap_SpaClisaveData
����		: 
�������	: 2008��9��25��
��������	: 
����		: XU32 idx
����		: SAAP_MON_FILTER_T * psaapFilter
����ֵ		: XU32 
************************************/
XU32 Saap_SpaClisaveData(XU32 idx,SAAP_MON_FILTER_T * psaapFilter)
{
    gstSaapSpaCliData.ulTraceFlag = FLAG_YES;
    gstSaapSpaCliData.linkHandle = psaapFilter->ullinkHandle;
    gstSaapSpaCliData.cliData[idx].ucCodePlan = psaapFilter->codePlan;
    gstSaapSpaCliData.cliData[idx].gtValue = psaapFilter->ulGt;
    return XSUCC;
}

/* find idle spa cli ccb*/
/********************************** 
��������	: Saap_SpaCliFindIdleCcb
����		: 
�������	: 2008��9��25��
��������	: 
����ֵ		: XU32 
************************************/
XU32 Saap_SpaCliFindIdleCcb()
{
    XU32 i  =0;
    XU32 idx  = BLANK_ULONG;
    for(i= 0;i< SAAP_MAX_SPA_CLI_NUM;i++)
    {
         if(FLAG_NO == gstSaapSpaCliData.cliData[i].busyflag)
        {
            idx = i;
            break;
        }
    }
    return idx;
}

/* save a new filter data*/
/********************************** 
��������	: Saap_SpaCliInsert
����		: 
�������	: 2008��9��25��
��������	: 
����		: XU32 idx
����		: SAAP_MON_FILTER_T * psaapFilter
����ֵ		: XU32 
************************************/
XU32 Saap_SpaCliInsert(XU32 idx,SAAP_MON_FILTER_T * psaapFilter)
{
    gstSaapSpaCliData.ulTraceFlag = FLAG_YES;
    gstSaapSpaCliData.linkHandle = psaapFilter->ullinkHandle;
    gstSaapSpaCliData.ulTraceId = psaapFilter->ulTraceId;
    gstSaapSpaCliData.cliNum++;
    gstSaapSpaCliData.cliData[idx].busyflag = FLAG_YES;
    gstSaapSpaCliData.cliData[idx].IpAddr = psaapFilter->ulSpaCliIpAddr;
    gstSaapSpaCliData.cliData[idx].port = psaapFilter->ulSpaCliPort;
    gstSaapSpaCliData.cliData[idx].ucCodePlan = psaapFilter->codePlan;
    gstSaapSpaCliData.cliData[idx].gtValue = psaapFilter->ulGt;
    return XSUCC;
}

/*cls a filter data*/
/********************************** 
��������	: Saap_SpaCliDelCcb
����		: 
�������	: 2008��9��25��
��������	: 
����		: XU32 idx
����ֵ		: XU32 
************************************/
XU32 Saap_SpaCliDelCcb(XU32 idx)
{
    //����Ҫ��������ͻ�������,��memset��
    gstSaapSpaCliData.cliData[idx].busyflag = FLAG_NO;
    gstSaapSpaCliData.cliData[idx].ulSend2MntFlag = FLAG_NO;
    gstSaapSpaCliData.cliNum--;
    if(0 == gstSaapSpaCliData.cliNum)
    {
        gstSaapSpaCliData.ulTraceFlag = FLAG_NO;
    }
    return XSUCC;
}

/********************************** 
��������	: Saap_SpaCliClsSendFlag
����		: 
�������	: 2008��9��25��
��������	: 
����ֵ		: XU32 
************************************/
XU32 Saap_SpaCliClsSendFlag()
{
    XU32 i  = 0;
    for(i =0; i < SAAP_MAX_SPA_CLI_NUM;i++)
    {
        gstSaapSpaCliData.cliData[i].ulSend2MntFlag = FLAG_NO; //������ͱ�ʶ
    }
    return XSUCC;
}

/********************************** 
��������	: Saap_GetClock
����		: 
�������	: 2008��9��25��
��������	: 8888
����ֵ		: XU32 
************************************/
XU32 Saap_GetClock()
{
    return (XU32)clock();
}


#ifdef __cplusplus
    }
#endif


