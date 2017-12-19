/*----------------------------------------------------------------------
    created:    2004/09/29
    created:    29:9:2004   9:27
    filename:
    file path:
    file base:
    file ext:   c
    author:

    purpose:
----------------------------------------------------------------------*/

#ifdef  __cplusplus
    extern  "C"{
#endif

#include "xosshell.h"
//#include "saap_def.h"
#include "tcpmain.h"
#include "tcpoamproc.h"
#include "tcproutertable.h"
#include "syshash.h"
#include "../../saap/inc/saappublichead.h"

XS32 gTcpeReouteEnable  =0;
XS32 gtestInOneHost = 0;
/*20090722 add below*/
TCPE_USER_FID_T g_tcpe_UserFID[MAX_TCPE_USER_NUM];
/*20090722 add above*/
t_XOSFIDLIST g_TcpeFidInfo = { {"FID_TCPE",  XNULL, FID_TCPE}, {TCP_InitProc , XNULL, XNULL},  {TCP_MsgProc,  TCP_TimeoutProc},  eXOSMode, XNULL};

/*TCP�շ���Ϣͳ��*/
// �ⲿ�����������˱���ר�õ��������ļ��ж���
extern TCP_DSTID_INDEX_IP_CCB   *gTCPDstIDToIPTbl;
extern HASH_TABLE                gTCPDstIDToIPHashTbl;

// �ⲿ������·��
extern TCP_LINT_CCB_INDEX       *g_pstTCP ;

//���ʱ���м����ʱ����
XU8  *g_pDatainTmpBuff = XNULL ;

/* ��ʱ��ṹ */
XU8 g_tcpemsgbuff[SAAP_MAX_MSG_LEN];
int gMaxTcpeLinkTimes = 3; // ����ִ���
/**********************************
��������    : Tcp_Init
����        : codeReview
�������    : 2008��10��8��
��������    :
����ֵ      : XS32
************************************/
XS32 Tcp_Init( )
{
    XS32 i = 0;
    // ��ʼ������ģʽ
    //g_ucTCPMonitorMondel = 0 ;

    // Ĭ�Ϸ�����·������Ϣ
    g_Tcp_LinkHand = 1;

    //����trace��ϢĬ�Ϲر�
    g_Tcp_Trace =0;

    // �����ڴ�
    g_pDatainTmpBuff = (XU8 *)XOS_MemMalloc(FID_TCPE, 2 * MAX_IP_DATA_LEN);
    if(XNULL == g_pDatainTmpBuff)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe Init failed.");
        return XERROR;
    }
    //20100817 cxf add
    for(i =0; i < MAX_TCPE_USER_NUM; i++)
    {
        g_tcpe_UserFID[i].userType = 0;
        g_tcpe_UserFID[i].userFid = 0;
    }
    g_tcpe_UserFID[0].userType = SAAP_USER_TYPE;
    g_tcpe_UserFID[0].userFid = FID_SAAP;
    return XSUCC;

}

/**********************************
��������    : TCP_InitProc
����        : codeReview
�������    : 2008��10��8��
��������    : ���ܿ��ʼ������
����        : XVOID *Para1
����        : XVOID *Para2
����ֵ      : XS8
************************************/
XS8 TCP_InitProc(XVOID *Para1, XVOID *Para2 )
{
    int i  = 0;
    
    if(XSUCC != tcpe_link_inittable())
    {
         return XERROR;
    }
    if(XSUCC !=TCP_init_DstID_table())
    {
        return XERROR;
    }

    //������ע��
    TCP_command_init();

    if(XSUCC !=XOS_TimerReg(FID_TCPE,500,TCP_REG_TIMER_NUM*2 + 2,0))
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe init,reg timer fail.");
    }

    for(i = 0; i< MAX_TCPEN_LINK_NUM; i++)
    {
        SAAP_StartTimer(&g_pstTCP->lintCcb[i].htTm,FID_TCPE, TCP_TWO_SECOND, TCP_HAND_TIMER,i,TIMER_TYPE_LOOP);
    }
        
    //��ʼ��ȫ�ֱ���
    if(XSUCC != Tcp_Init())
    {
       return XERROR;
    }

    //ע��OAM
    TcpTblRegToOAM(FID_TCPE);
    return XSUCC;
}

/**********************************
��������    : TCP_MsgProc
����        : codeReview
�������    : 2008��10��8��
��������    :
   tcpe �������Ϣ������ֻ������������
   1.��NTL����ͨѶ��Ϣ
   2.��saap����ҵ������������Ϣ
   3.����������Ϣ������.
   ��ע:���ݽṹt_XOSCOMMHEAD �� COMMON_HEADER_SAAP �ȼ�

����        : t_XOSCOMMHEAD *pxosMsg
����        : XVOID *Para
����ֵ      : XS8
************************************/
XS8 TCP_MsgProc(XVOID *msg, XVOID *Para)
{
     t_XOSCOMMHEAD *pxosMsg=XNULL;
     COMMON_HEADER_SAAP *pstMsg = XNULL;
     if(XNULL == msg)
     {
         return XERROR;
     }
     pxosMsg=(t_XOSCOMMHEAD*)msg;
     pstMsg = (COMMON_HEADER_SAAP *)g_tcpemsgbuff;

     //�����NLT����������ָʾ,��Ҫ����Ϣ����copy,����NTL��Ϣ��Ԥ�������
     if(FID_NTL == pxosMsg->datasrc.FID && eDataInd == pxosMsg->msgID)
     {
         if(XSUCC != SYS_XOSMsg2CpsMsg(pxosMsg, pstMsg) )
         {
            XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe msg proc,trans from xos ntl data in msg to cps msg failed.");
            return XERROR;
         }
     }
     else
     {
         //���XOSƽ̨�޸�����Ϣͷ,COMMON_HEADER�ṹӦ����Ӧ�޸�
         pstMsg = (COMMON_HEADER_SAAP *)pxosMsg;
     }

     //ntl��Ϣ�Ĵ���
     if(FID_NTL == pstMsg->stSender.ucFId)
     {
         Tcp_ntlmsg_proc(pstMsg);
         return 0;
     }

     //����ҵ����ϢӦ��ֻ��DATA REQ��CFG����Ϣ
     switch(pstMsg->usMsgId)
     {
        case SAAP_DATA_REQ:
            TCP_Svc2Tcp_DataReqProc( pstMsg , TCP_INVIKE_LOCAL);
            break;

//20080928�����߼��޸�,�������洦���߼�,û�д˹���,������ƽ̨
//        case MSG_IP_CONFIG: //������Ϣ������������������Ϣ
//            pstMsg->usMsgId = pxosMsg->subID;
//            tcp_OAM_proc( pstMsg );
//            break;
        default:
           if(g_Tcp_Trace)
           {
             XOS_Trace(MD(FID_TCPE,PL_MIN),"tcpe msg proc,unsupport msg type %d coming.",pstMsg->usMsgId);
           }
           break;
     }

     return 0;
}
void Tcp_PacketStat(XU16 link_Index,XU32 stattype)
{
#ifdef SAG_PERFORM_STAT_INCLUDE
    if( link_Index < TCP_LINT_MAX )
    {
       switch(stattype)
       {
         case eSendMsg:
              gTcpeStaData.msgStat[link_Index].ulSendMsgCnt++;
              break;
         case eRecvErrMsg:
              gTcpeStaData.msgStat[link_Index].ulRecvErrMsgCnt++;
              break;
         case eSendErrMsg:
              gTcpeStaData.msgStat[link_Index].ulSendErrMsgCnt++;
              break;
         case eRecvMsg:
              gTcpeStaData.msgStat[link_Index].ulRecvMsgCnt++;
              break;
         case eLinkTry:
              gTcpeStaData.msgStat[link_Index].ulLinkTryCnt++;
              break;
         case eLinkHand:
              gTcpeStaData.msgStat[link_Index].ulLinkHand++;
              break;
         case eLinkHandAck:
              gTcpeStaData.msgStat[link_Index].ulLinkHandAck++;
              break;
         case eLinkLose:
              gTcpeStaData.msgStat[link_Index].ulLinkLoseCnt++;
              break;
         case eLinkShutStop:
              gTcpeStaData.msgStat[link_Index].ulLinkShutStop++;
              break;
         default:
              gTcpeStaData.msgStat[link_Index].ulOther++;
              break;
       }
    }else
    {
       switch(stattype)
       {
         case eSendMsg:
              gTcpeStaData.msgIn.ulSendMsgCnt++;
              break;
         case eRecvErrMsg:
              gTcpeStaData.msgIn.ulRecvErrMsgCnt++;
              break;
         case eSendErrMsg:
              gTcpeStaData.msgIn.ulSendErrMsgCnt++;
              break;
         case eRecvMsg:
              gTcpeStaData.msgIn.ulRecvMsgCnt++;
              break;
         case eLinkTry:
              gTcpeStaData.msgIn.ulLinkTryCnt++;
              break;
         case eLinkHand:
              gTcpeStaData.msgIn.ulLinkHand++;
              break;
         case eLinkHandAck:
              gTcpeStaData.msgIn.ulLinkHandAck++;
              break;
         case eLinkLose:
              gTcpeStaData.msgIn.ulLinkLoseCnt++;
              break;
         case eLinkShutStop:
              gTcpeStaData.msgIn.ulLinkShutStop++;
              break;
         default:
              gTcpeStaData.msgIn.ulOther++;
              break;
       }
    }
#endif
}
/*20090722 add below*/
/********************************** 
��������    : tcpe_setfid_byusertype
����        : Jeff.Zeng
�������    : 2009��7��22��
��������    : 
�������    : XS32 userType �û�������Ϣ����
����        : XS32 userFid  �û�������Ϣ����·��FID
����ֵ        : XS32 
            : XSUCC:�ɹ�/XERROR:ʧ��
************************************/
XS32 tcpe_setfid_byusertype(XS32 userType,XS32 userFid)
{
    XS32 i  =0;
    if(0 == userFid)
    {
        return XERROR;
    }
    for(i=0; i< MAX_TCPE_USER_NUM; i++)
    {
        if(0 == g_tcpe_UserFID[i].userFid) //cb not use ,20100817 cxf modify
        {
            g_tcpe_UserFID[i].userType = userType;
            g_tcpe_UserFID[i].userFid = userFid;
            return XSUCC;
        }
    }     
    return XERROR;
}


/********************************** 
��������    : tcpe_getfid_byusertype
����        : Jeff.Zeng
�������    : 2009��7��22��
��������    : 
����        : XS32 UserType ֱ��ģ�͵���Ϣ���͵õ�FID
����ֵ        : XS32 XSUCC:�ɹ�/XERROR:ʧ��
************************************/
XS32 tcpe_getfid_byusertype(XS32 UserType)
{
    XS32 i  =0;

    for(i =0; i < MAX_TCPE_USER_NUM; i++)
    {
        if(UserType == g_tcpe_UserFID[i].userType)
        {
            return g_tcpe_UserFID[i].userFid;
        }
    }
    return 0;
}
/*20090722 add above*/
/**********************************
��������    : Tcp_ntlmsg_proc
����        : codeReview
�������    : 2008��10��8��
��������    : ���ܿ��ͨѶ��Ϣ������ں���
����        : COMMON_HEADER_SAAP *pstMsg
����ֵ      : XS32
************************************/
XS32 Tcp_ntlmsg_proc(COMMON_HEADER_SAAP *pstMsg)
{
    XS32 ulLinkIndex = 0;
    t_LINKINITACK  * pLinkInitAck  = XNULL;
    t_STARTACK     * pLinkStartAck = XNULL;
    t_SENDERROR    * pSendErrMsg   = XNULL;
    t_LINKCLOSEIND * pLinkStopInd  = XNULL;
    t_CONNIND      * pLinkConnInd  = XNULL;
    if(g_Tcp_Trace)
    {
       XOS_Trace(MD(FID_TCPE,PL_MIN),"enter tcpe ntlmsg_proc.");
    }
    switch(pstMsg->usMsgId)
    {
        case eInitAck:
            pLinkInitAck = (t_LINKINITACK *)(pstMsg+1);
            return tcpe_link_initack(pLinkInitAck);
            //break;

        case eStartAck:
            pLinkStartAck = (t_STARTACK *)(pstMsg+1);
            return tcpe_link_startack(pLinkStartAck);
            //break;

        case eErrorSend:
            pSendErrMsg = (t_SENDERROR *)(pstMsg+1);
            return tcpe_link_errorsend(pSendErrMsg);
            //break;

        case eStopInd:
            pLinkStopInd = (t_LINKCLOSEIND *)(pstMsg +1);
            ulLinkIndex = (XS32)pLinkStopInd->appHandle;
            //ͳ����·���رմ���
            Tcp_PacketStat((XU16)ulLinkIndex,eLinkShutStop);
            if(ulLinkIndex >= MAX_TCPEN_LINK_NUM)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"link index err when rev eStopInd.");
                return XERROR;
            }
            XOS_Trace(MD(FID_TCPE,PL_WARN),"link(%d) rev eStopInd from ntl.",ulLinkIndex);

            //�澯
            if(TCP_LINK_OK == g_pstTCP->lintCcb[ulLinkIndex].ucHandState)
            {
                //�����ǰ״̬���Ѿ�����,�յ���·ָֹͣʾ,�����ܸ澯
                tcpe_link_alarm(TCPE_TCP_ALARM_STATE_DISC,ulLinkIndex);
            }
            //������·״̬
            tcpe_link_setstatus( ulLinkIndex,TCPE_TCP_LINK_STATE_STOP);
            g_pstTCP->lintCcb[ulLinkIndex].ucHandState = TCP_LINK_FAIL;

            //���tcpe����
            tcpe_link_clearbuff(ulLinkIndex);

            //ֹͣ��ʱ��
            //XOS_TimerStop(FID_TCPE,g_pstTCP->lintCcb[ulLinkIndex].htTm); //20110909 cxf del
            break;

        case eConnInd:
            pLinkConnInd = (t_CONNIND *)(pstMsg +1);
            ulLinkIndex = (XS32)pLinkConnInd->appHandle;
            if(ulLinkIndex >= MAX_TCPEN_LINK_NUM)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"link index err when rev eConInd.");
                return XERROR;
            }
            XOS_Trace(MD(FID_TCPE,PL_INFO),"link(%d) rev eConnInd from Ntl.",ulLinkIndex);
            tcpe_link_setstatus( ulLinkIndex, TCPE_TCP_LINK_STATE_BUILD);

            //ֻ�пͻ��˲��������ֶ�ʱ��
            //20081205 modify below
            //g_pstTCP->lintCcb[ulLinkIndex].ucHandState   =  TCP_LINK_OK;
            //20081205 modify above

            /*20090911 modify below,������,�ͻ��˶��������ֳ���*/
            //if(g_pstTCP->lintCcb[ulLinkIndex].ucModel == eTCPClient)
            //{
                //SAAP_StartTimer(&g_pstTCP->lintCcb[ulLinkIndex].htTm,FID_TCPE, TCP_TWO_SECOND, TCP_HAND_TIMER,ulLinkIndex,TIMER_TYPE_LOOP); // ??? cxf 20110908 del
            //}
            /*20090911 modify above*/
            XOS_Trace(MD(FID_TCPE,PL_INFO),"rev eConnInd of link(%d).",ulLinkIndex);
            break;

        case eDataInd:
            pstMsg->usMsgId = SAAP_DATA_IND;
            Tcp_ntlmsg_packetdecodeproc(pstMsg);
            break;//return XSUCC;

        default:
            return XSUCC;

    }
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN),"out tcpe ntlmsg_proc.");
    }
    return XSUCC;
}

/**********************************
��������    :
����        : codeReview
�������    : 2008��10��8��
��������    : �˺�ʽ����ƽ̨���е�����ָʾ
����        :  COMMON_HEADER_SAAP *pstMsg
����        : XU8 *MsgBuffer
����ֵ      : void
************************************/
void TCP_Tcp2Svc_DataIndProc( COMMON_HEADER_SAAP *pstMsg, XU8 *MsgBuffer )
{
    // �������е���ϢBUF
    XU8    ucOutMsg[SAAP_MAX_MSG_LEN + sizeof(COMMON_HEADER_SAAP)];
    TCP_AND_NEX_LAYER_INTERFACE     *pMsg ;     // �յ��İ��ṹ
    TCP_AND_PRO_LAYER_MSG_BUF       *pstTmp;    // ��ȥ�İ��ṹ
    XU8   handMsg[sizeof(TCP_hand_msg)];
    XU8 *pBuf ;
    XU16  usccno, lintNo ;
    if(g_Tcp_Trace)
    {
        XOS_Trace(MD(FID_TCPE,PL_MIN),"enter TCP_Tcp2Svc_DataIndProc.");
    }
    pBuf   = (XU8*)MsgBuffer;
    pMsg   = (TCP_AND_NEX_LAYER_INTERFACE*)MsgBuffer ; // ת����TCP/IP��װ�ṹ
    pstTmp = (TCP_AND_PRO_LAYER_MSG_BUF*)ucOutMsg ;

    // ·�ɽڵ�����ȶ�
    pstTmp->tcpHead.RouterNum = XOS_NtoHs(pMsg->RouterNum ) ;

//#ifdef SAG_PERFORM_STAT_INCLUDE
    //ͨ��Դ������ҵ�ӳ�����±�,Ȼ��õ���·��
    usccno = TCP_SearchDstIDToIpTable( XOS_NtoHs(pMsg->GuestDeviceID) );
    if(usccno < TCP_MAX_IPROUTER_NUM)
    {
        lintNo = gTCPDstIDToIPTbl[usccno].usLintIndex;
    }
//#endif

    // 0 �� -- ��Ϊ 0xfffff
    pstTmp->tcpHead.RouterNum -- ;
    if ( pstTmp->tcpHead.RouterNum  <= 0
        || pstTmp->tcpHead.RouterNum == BLANK_USHORT )
    {
        // �����޶�����������
        Tcp_PacketStat(lintNo,eRecvErrMsg);
        XOS_Trace(MD(FID_TCPE,PL_ERR),"router num become 0,discard the msg.");
        return ;
    }

    // ���²�����ɰ�ͷת��
    pstTmp->tcpHead.DstDeviceID = XOS_NtoHs(pMsg->DstDeviceID) ;
    pstTmp->tcpHead.GuestDeviceID = XOS_NtoHs(pMsg->GuestDeviceID) ;
    pstTmp->tcpHead.UserType = XOS_NtoHs(pMsg->UserType);

    // �˴�Ҫ��ȥ TCP/IP ��װ�������ֶεĳ���
    pstTmp->tcpHead.MsgLen = XOS_NtoHs(pMsg->MsgLen) - SAAPANDTCP_HEAD_LEN;

    // �ڴ汣�������ȼ��
    if ( pstTmp->tcpHead.MsgLen > SAAP_MAX_MSG_LEN )
    {
        //20081016�޸����¹��ܵ�,һ��ͳ��
        //if( lintNo < TCP_LINT_MAX - 2 )
        //{
           //Tcp_PacketStat(lintNo,eRecvErrMsg);
        //}
        Tcp_PacketStat(lintNo,eRecvErrMsg);
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe msg len(%x) is too long.",pstTmp->tcpHead.MsgLen );
        return ;
    }
    XOS_MemCpy( pstTmp->ucBuffer,  pBuf + sizeof(TCP_AND_NEX_LAYER_INTERFACE),  pstTmp->tcpHead.MsgLen  );

    pstTmp->com.usMsgLen =  pstTmp->tcpHead.MsgLen + sizeof(TCP_AND_PRO_LAYER_INTERFACE) ;
    pstTmp->com.usMsgId  =  pstMsg->usMsgId ; // SAAP_DATA_IND tcpmain.c line 414

    pstTmp->com.stReceiver =  pstMsg->stReceiver ;
    pstTmp->com.stSender   =  pstMsg->stSender ;

    //ͨ��dpid��ȡ��վ����
    if(usccno < TCP_LINT_MAX)
    {
        lintNo = gTCPDstIDToIPTbl[usccno].usLintIndex;
    }

    // �ȶ��Ƿ������ѵ�����㣬������ǣ���Ϊת�ӵ㣬��ֱ�ӳ���
    // �����������Ϊ ffff , ��˵����ģ��Ϊ���ܵ�û��������ģ�飬�Ǳ����������ת��·��
    if(gTcpeReouteEnable) // �Ƿ�����TCPE��·�ɹ���
    {
        //��ע:8888�÷�֧������
        if (  g_pstTCP->GlobalDstID  != pstTmp->tcpHead.DstDeviceID
                &&  BLANK_USHORT !=  pstTmp->tcpHead.DstDeviceID
                 &&  0xfffd != pstTmp->tcpHead.DstDeviceID  )
        {
            // �����·��������ֱ�ӷ���, �����Ǻ�������, ID ��Ϊ��������
            TCP_Svc2Tcp_DataReqProc( (COMMON_HEADER_SAAP*)pstTmp , TCP_INVIKE_ROUTE );
            return ;
        }
    }

    // �����������Ϣ�����������
    if ( TCP_USERTYPE_HANDSHAKE == XOS_NtoHs(pMsg->UserType) )
    {
        XOS_MemSet( handMsg, BLANK_UCHAR, sizeof(TCP_hand_msg) );
        XOS_MemCpy( handMsg, pstMsg, sizeof(COMMON_HEADER_SAAP) );
        XOS_MemCpy( handMsg + sizeof(COMMON_HEADER_SAAP), MsgBuffer,  sizeof(TCP_hand_msg) - sizeof(COMMON_HEADER_SAAP) );
        TCP_hand_msg_proc( (COMMON_HEADER_SAAP*)handMsg );  //TCP����
        return  ;
    }
    // �����û�����,���ｫ���ȫ��ID���ڲ�ID��һ��ת��

    // �����ҵ���ķ���

    // 20100817 cxf modify
    pstTmp->com.stReceiver.ucFId = tcpe_getfid_byusertype(pstTmp->tcpHead.UserType);
    if(0 == pstTmp->com.stReceiver.ucFId) // not find dstFid by userType
    {
        if(g_Tcp_Trace)
        {
            XOS_Trace(MD(FID_TCPE,PL_INFO),"unsupport msg for tcpe link data in usertype(0x%x).",pstTmp->tcpHead.UserType);
        }
        Tcp_PacketStat(lintNo,eRecvErrMsg);
        return;
    }
    pstTmp->com.stSender.ucFId  =  FID_TCPE ;
    SYS_MSGSEND((COMMON_HEADER_SAAP *)pstTmp );
    Tcp_PacketStat(lintNo,eRecvMsg);
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN),"out TCP_Tcp2Svc_DataIndProc FID_TCPE => %d.",pstTmp->com.stReceiver.ucFId);
    }
    return  ;

}

//review part ok
/**********************************
��������    : TCP_Svc2Tcp_DataReqProc
����        : codeReview
�������    : 2008��10��8��
��������    :  ҵ������ƽ̨���������������
����        :  COMMON_HEADER_SAAP *pstMsg
����        : XU8 flag
����ֵ      : void
************************************/
void TCP_Svc2Tcp_DataReqProc( COMMON_HEADER_SAAP *pstMsg , XU8 flag )
{
    XU16 shLen ;
    XU16 usccno=TCP_ERROR_NUM, lintNo=TCP_ERROR_NUM;
    XU16  msgout_sumidx=BLANK_USHORT;
    // �������е���ϢBUF
    XU8    ucOutMsg[SAAP_MAX_MSG_LEN + sizeof(COMMON_HEADER_SAAP)];

    TCP_AND_PRO_LAYER_MSG_BUF       *pMsg ;
    TCP_AND_NEX_LAYER_MSG_BUF       *pstTmp;

    COMMON_MESSAGE_SAAP *pstReturnMsg ;  // ��·��������û��㷵��
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN), "Enter TCP Svc2Tcp_DataReqProc");
    }
    Tcp_PacketStat(msgout_sumidx,eSendMsg);
    pMsg = ( TCP_AND_PRO_LAYER_MSG_BUF* )pstMsg ;  // ���յ��İ�ת���ɱ����Ӧ��Э��

    // �����������
    pstTmp = (TCP_AND_NEX_LAYER_MSG_BUF *)ucOutMsg;

    // ��ͷ����
    pstTmp->tcpHead.ucHeadFlag7E  = TCP_CURR_FLAG ;
    pstTmp->tcpHead.ucHeadFlagA5  = TCP_HEAD_FLAG ;

    // ��һ������û����ݵķ�װ
    // �û����ݵĳ���
    shLen = pMsg->tcpHead.MsgLen;
    usccno = TCP_SearchDstIDToIpTable(pMsg->tcpHead.DstDeviceID);//8888 �ֽ������ղ�ͬ
    if(usccno < TCP_MAX_IPROUTER_NUM)
    {
        lintNo = gTCPDstIDToIPTbl[usccno].usLintIndex;
    }

    // �ڴ汣��
    if ( shLen > SAAP_MAX_MSG_LEN )
    {
        Tcp_PacketStat(lintNo,eSendErrMsg);
        XOS_Trace(MD(FID_TCPE,PL_WARN),"TCP Svc2Tcp_DataReqProc,msg len err.");
        return ;
    }
    XOS_MemCpy(pstTmp->ucBuffer,  pMsg->ucBuffer, shLen );

    if( TCP_INVIKE_LOCAL == flag)
    {
        // ����Ǳ��ص���, �������ѵ���������
        pstTmp->tcpHead.GuestDeviceID = XOS_HtoNs( g_pstTCP->GlobalDstID );
    }
    else
    {
        // ·�ɵ��ã�Դ����㲻��
        pstTmp->tcpHead.GuestDeviceID = XOS_HtoNs( pMsg->tcpHead.GuestDeviceID );
        pstTmp->com.usMsgId = SAAP_DATA_REQ ;// ��Ϣ��Ϊ��������
    }

    // �����û�����,���ｫ����ڲ�ID��ȫ��ID��һ��ת��
    pstTmp->tcpHead.UserType =  XOS_HtoNs(pMsg->tcpHead.UserType) ;

    // ����·�ɴ���,��ʼ��ʱΪ���ֵ
    pstTmp->tcpHead.RouterNum  =  XOS_HtoNs( pMsg->tcpHead.RouterNum ) ;

    shLen = shLen + sizeof(TCP_AND_NEX_LAYER_INTERFACE);

    // ������, ����ĳ���Ҫ��ȥ head falg len
    pstTmp->tcpHead.MsgLen = XOS_HtoNs( (XU16)(shLen - 2) );

    // �����β
    ucOutMsg[shLen + sizeof(COMMON_HEADER_SAAP)] = TCP_CURR_FLAG ;
    ucOutMsg[shLen + sizeof(COMMON_HEADER_SAAP) + 1] = TCP_END_FLAG ;
    shLen = shLen + 2 ;  // buf �ܳ�

    // ����Ŀ��������������·��ϵ�����ƽ̨�ķ���
    // ������������, ����ʹ�������ֽڷ���
    if ( 0 ==  pMsg->tcpHead.DstDeviceID)
    {

        // ���Ĭ����һ���ڵ��Ƿ�Ϸ�
        usccno = g_pstTCP->ProIndex;
        if ( BLANK_USHORT == g_pstTCP->ProIndex  )
        {

            /*2009/05/19 add below for saap_senderr_rsp process,submsg is src fid*/
            pstReturnMsg = (COMMON_MESSAGE_SAAP *)ucOutMsg;
            pstReturnMsg->stHeader.stReceiver =   pstMsg->stSender ;
            pstReturnMsg->stHeader.stSender   =   pstMsg->stReceiver ;
            pstReturnMsg->stHeader.usMsgId   =    TCP_HAND_ERR+1 ;
            pstReturnMsg->stHeader.subID   =       pstMsg->subID;
            pstReturnMsg->stHeader.usMsgLen   =   0x01 ;
            pstReturnMsg->ucBuffer[0]         =   TCP_LINK_FAIL ;
            SYS_MSGSEND( (COMMON_HEADER_SAAP *)pstReturnMsg );
            /*2009/05/19  add above for saap_senderr_rsp process*/

            //9999 �Ƿ�ͳ�Ʒ���ʧ��,20081016��ӷ���ʧ��ͳ��
            Tcp_PacketStat(msgout_sumidx,eSendErrMsg);
            XOS_Trace(MD(FID_TCPE,PL_ERR),"DstDeviceId =0,but can not find default link,can not send data.");
            return ;
        }
        // ���� ��·�ž��� Ĭ��ֵ
        lintNo = usccno ;

        // 0 ����GTҪʹ��Ĭ����·
        pstTmp->tcpHead.DstDeviceID = XOS_HtoNs(g_pstTCP->lintCcb[usccno].DstDeviceID) ;

    }
    else
    {
        pstTmp->tcpHead.DstDeviceID = XOS_HtoNs(pMsg->tcpHead.DstDeviceID);
        usccno = TCP_SearchDstIDToIpTable(pMsg->tcpHead.DstDeviceID);
        if ( TCP_ERROR_NUM != usccno )
        {
            // ��� HASH ������ DPID ����ȡ�����������·������ֵ
            lintNo = gTCPDstIDToIPTbl[usccno].usLintIndex ;
        }
        else
        {
            // ���� ��·�ž��� Ĭ��ֵ
            usccno = g_pstTCP->ProIndex ;
            lintNo = usccno ;
            if ( BLANK_USHORT == g_pstTCP->ProIndex  )
            {
                /*2009/05/19 add below for saap_senderr_rsp process,submsg is src fid*/
                pstReturnMsg = (COMMON_MESSAGE_SAAP *)ucOutMsg;
                pstReturnMsg->stHeader.stReceiver =   pstMsg->stSender ;
                pstReturnMsg->stHeader.stSender   =   pstMsg->stReceiver ;
                pstReturnMsg->stHeader.usMsgId   =    TCP_HAND_ERR+1 ;
                pstReturnMsg->stHeader.subID   =       pstMsg->subID;
                pstReturnMsg->stHeader.usMsgLen   =   0x01 ;
                pstReturnMsg->ucBuffer[0]         =   TCP_LINK_FAIL ;
                SYS_MSGSEND( (COMMON_HEADER_SAAP *)pstReturnMsg );
                /*2009/05/19  add above for saap_senderr_rsp process*/

                //9999 �Ƿ�ͳ�Ʒ���ʧ��,20081016��ӷ���ʧ��ͳ��
                Tcp_PacketStat(msgout_sumidx,eSendErrMsg);
                XOS_Trace(MD(FID_TCPE,PL_ERR),"can not find link of DstIp(%x) and  without default link,can not send data.",pMsg->tcpHead.DstDeviceID);
                return ;
            }
        }
    }

    // ������ִ�,���Ǳ��ذ�,������·����ʧ�ܣ��򱾵�Ӧ�ò㷵�ش���
    if( 1 == g_Tcp_LinkHand  && TCP_INVIKE_LOCAL == flag
                             &&  TCP_LINK_OK != g_pstTCP->lintCcb[lintNo].ucHandState  )
    {
        pstReturnMsg = (COMMON_MESSAGE_SAAP *)ucOutMsg;
        pstReturnMsg->stHeader.stReceiver =   pstMsg->stSender ;
        pstReturnMsg->stHeader.stSender   =   pstMsg->stReceiver ;

        pstReturnMsg->stHeader.usMsgId   =    TCP_HAND_ERR ;
        /*2009/04/28 add below for saap_senderr_rsp process,submsg is src fid*/
        pstReturnMsg->stHeader.subID   =       pstMsg->subID;
        XOS_Trace(MD(FID_TCPE,PL_INFO),"tcpe linkerr send rsp subID %d.fsmid %d",pstReturnMsg->stHeader.subID,pstReturnMsg->stHeader.stReceiver.usFsmId);
        /*2009/04/28 add above for saap_senderr_rsp process*/
        pstReturnMsg->stHeader.usMsgLen   =   0x01 ;
        pstReturnMsg->ucBuffer[0]         =   TCP_LINK_FAIL ;    // ��·������ֵ����

        // ��·���󣬻�Ӧʧ��
        SYS_MSGSEND( (COMMON_HEADER_SAAP *)pstReturnMsg );
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe's hand shake state is fail,can not send data.");
        Tcp_PacketStat(lintNo,eSendErrMsg);
        return ;
    }
    if(g_Tcp_Trace)
    {
       XOS_Trace(MD(FID_TCPE,PL_DBG),"use link(%d) to send data.",lintNo);
    }
    if ( lintNo > TCP_LINT_MAX-1 )
    {
        /* �˴��޷�����·��ͳ��,��Ϊ��·���Ѿ������˷�Χ�� */
        XOS_Trace(MD(FID_TCPE,PL_ERR),"link index (%d) err.",lintNo);
        //9999 �Ƿ�ͳ�Ʒ���ʧ��,20081016��ӷ���ʧ��ͳ��
        Tcp_PacketStat(msgout_sumidx,eSendErrMsg);
        return ;
    }

    // ��ɰ�ͷ��װ

    pstTmp->com.stReceiver.ucModId = XOS_GetLocalPID();
    pstTmp->com.stReceiver.ucFId= FID_NTL;
    pstTmp->com.stReceiver.usFsmId =(XU16)g_pstTCP->lintCcb[lintNo].ulLinkHandle;

    pstTmp->com.stSender.ucModId = XOS_GetLocalPID();
    pstTmp->com.stSender.ucFId = FID_TCPE;
    pstTmp->com.stSender.usFsmId = lintNo;

    pstTmp->com.usMsgId = pstMsg->usMsgId;

    pstTmp->com.usMsgLen  =  shLen ;

    SYS_MSGSEND((COMMON_HEADER_SAAP *)pstTmp );
    Tcp_PacketStat(lintNo,eSendMsg);
    return ;
}

////////////////////////////////////////////////////////////////////////
//  ����Ϊ����������
////////////////////////////////////////////////////////////////////////

// ���������������˺�����IP�������ҵ����ķ���
// �˺���ע��� error ���������ڵ�Ԫ���Լ�����ĵ�
/**********************************
��������    :
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : COMMON_HEADER_SAAP *pstMsg
����ֵ      : void
************************************/
void Tcp_ntlmsg_packetdecodeproc( COMMON_HEADER_SAAP *pstMsg )
{
    COMMON_MESSAGE_SAAP * pMsg ;
    XU16 usNo  , usLen ,  bufLen  ;
    XU16  i , usbufLen;
    XU16  msgin_sumidx=TCP_ERROR_NUM;
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN),"enter tcpe ntlmsg_packetdecodeproc.");
    }

    Tcp_PacketStat(msgin_sumidx,eRecvMsg);
    //��ע:tcpe �ڴ���ͨѶ��Ϣʱ,����·��usNo��Ϊ����Ϣ�Ĳ���ֵ
    usNo =  pstMsg->stReceiver.usFsmId ;
    if ( usNo > TCP_LINT_MAX  )
    {
        //����ͳ��888
        Tcp_PacketStat(msgin_sumidx,eRecvErrMsg);
        //XOS_Trace(MD(FID_TCPE,PL_ERR),"TCPE ntlDataInd_DecodeProc,link index(%d) > max(%d) ",usNo,TCP_LINT_MAX);
        return ;
    }

    pMsg = (COMMON_MESSAGE_SAAP * )pstMsg;

    // �õ�����������鳤��
    usLen = pMsg->stHeader.usMsgLen ;
    if( usLen > MAX_IP_DATA_LEN || usLen <= 0 )
    {
        Tcp_PacketStat(usNo,eRecvErrMsg);
        return ;
    }

    // ���ȫ�ֻ���
    XOS_MemSet( g_pDatainTmpBuff ,0x0, MAX_IP_DATA_LEN * 2 );

    // �õ������а��ĳ���,��ƴ��
    bufLen = g_pstTCP->lintCcb[usNo].bufLen;
    if( bufLen > 0 )
    {
        if ( bufLen + usLen >=  MAX_IP_DATA_LEN * 2 )  // �ڴ汣��
        {
           Tcp_PacketStat(usNo,eRecvErrMsg);
           return ;
        }
        // ����м�BUF
        XOS_MemCpy( g_pDatainTmpBuff,&g_pstTCP->lintCcb[usNo].ucBuffer[0], bufLen );
        XOS_MemCpy(&g_pDatainTmpBuff[bufLen], pMsg->ucBuffer, usLen );

        // �õ��µĳ�����Ϣ
        usLen = usLen +  bufLen;
    }
    else
    {
        // ֱ�� copy BUF
        XOS_MemCpy( g_pDatainTmpBuff, pMsg->ucBuffer, usLen );
    }

    // �����Դ
    //ֻҪ������������ 0
    g_pstTCP->lintCcb[usNo].bufLen = 0 ;
    XOS_MemSet( &g_pstTCP->lintCcb[usNo].ucBuffer[0],0x0,MAX_IP_DATA_LEN );

    // ��ͷ��λʵ�����ض�λ����, �˴�ʧ���ض�λǰ�İ���������
    for( i = 0 ;  i  < usLen ; i++  )
    {
        if ( 0 == i )
        {
            if ( usLen - i <= SAAPANDTCP_HEAD_LEN)//9999 �㷨��ǰ,����ÿ�αȽ�
            {
                // �ȶ�һ�³��ȣ��������С�ڰ�ͷ����ֱ�ӵȴ���һ���ĵ���
                XOS_MemCpy( &g_pstTCP->lintCcb[usNo].ucBuffer[0], &g_pDatainTmpBuff[i], usLen - i );
                g_pstTCP->lintCcb[usNo].bufLen = usLen - i ;
                Tcp_PacketStat(usNo,eOtherStat);
                return ;
            }
        }

        if(  TCP_CURR_FLAG == g_pDatainTmpBuff[ i ]  &&
             TCP_HEAD_FLAG == g_pDatainTmpBuff[ i + 1 ] )
        {
            usbufLen = XOS_NtoHs(*(XU16*)&g_pDatainTmpBuff[ i + 2 ] );

            // �ڴ汣��,������
            if( usbufLen > MAX_IP_DATA_LEN - 100 )
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe's bufLen(%d) too long,now support max %d len,ingore it.",usbufLen,MAX_IP_DATA_LEN - 100 );
                Tcp_PacketStat(usNo,eRecvErrMsg);
                return ;
            }

            //ȫ�ֻ�������������һ�����������ݰ�
            if( i + 4 + usbufLen <= usLen   )   //usbufLenӦ�ð�����2�ֽ�
            {
                if(  TCP_CURR_FLAG == g_pDatainTmpBuff[ i + 2 + usbufLen ]  &&
                     TCP_END_FLAG  == g_pDatainTmpBuff[ i + 2 + usbufLen + 1 ] )
                {
                   // ҵ�����ݰ��ҵ���ת��ҵ����
                    TCP_Tcp2Svc_DataIndProc(pstMsg, (XU8*)&g_pDatainTmpBuff[i] );
                    //�����3��TCP ip_to_tcp_DataInd�Ĵ����й�,�����ݽṹ�а���3�������ֽ�
                    i = i  +  usbufLen + 3 ;   // ѭ���� i ����һ�Σ�����ֻ�� 3
                }
                else
                {
                    //8888 ��������߼�����,�����ղ���������ʶ,��Ϊ����ͳ����?
                    Tcp_PacketStat(usNo,eRecvErrMsg);
                }

            }
            else
            {
                XOS_MemCpy( &g_pstTCP->lintCcb[usNo].ucBuffer[0], &g_pDatainTmpBuff[i], usLen - i );
                g_pstTCP->lintCcb[usNo].bufLen = usLen - i ;
                Tcp_PacketStat(usNo,eOtherStat);
                //���ݲ�����,��������,
                return ;
            }

        }
        else
        {
            Tcp_PacketStat(usNo,eRecvErrMsg);
        }
    }
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN),"out tcpe ntlmsg_packetdecodeproc.");
    }
}

/**********************************
��������    :
����        : codeReview
�������    : 2008��10��8��
��������    : ��ʱ����Ϣ������
����        : t_BACKPARA  *pstPara
����ֵ      : XS8 TCP_TimeoutProc
************************************/
XS8 TCP_TimeoutProc(t_BACKPARA  *pstPara)
{
    XS32 tmrName = 0;
    XS32 ulPara = 0;
    //XS32 ulLinkIndex  = 0;

    tmrName = pstPara->para2;
    ulPara = pstPara->para3;

    switch(tmrName)
    {
        case TCP_HAND_TIMER: // ���ֶ�ʱ��
            {
                Tcp_lint_scan( (XU16)ulPara );
            }
            break;
        default:
            if(g_Tcp_Trace)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"TCP_TimeoutProc,unknown timer msg.");
            }
            break;
    }
    return XSUCC;
}

/**********************************
��������    : Tcp_lint_scan
����        : codeReview
�������    : 2008��10��8��
��������    : ���ֶ�ʱ������������������
����        : XU16 Index
����ֵ      : void
************************************/
void Tcp_lint_scan(XU16 Index)
{
    XU8   ucOutMsg[sizeof(TCP_hand_msg) + 1];
    // �������ְ��ṹ
    TCP_hand_msg  *pstTcpHand ;

    if ( g_Tcp_LinkHand == 0 )
    {
        return ;
    }

    if(Index >= TCP_LINT_MAX )
    {
       XOS_Trace(MD(FID_TCPE,PL_WARN),"tcpe hand time out msg proc,input link index para %d is error",Index);
       return;
    }

    // ??? 20110908 cxf add
    if( LINT_CCB_NULL ==g_pstTCP->lintCcb[Index].ucFlag) // not config it ,return;
    {
        return;
    }
    //
    // ���ֿ���
    if( g_pstTCP->lintCcb[Index].hankCount >= gMaxTcpeLinkTimes/*MAX_TCPE_HAND_TIMES*/  )
    {
        //�澯
        /*������ִ�������10��,���Ҵ������ӵ�״̬ʱ,�澯�ϱ�*/
        if( TCP_LINK_OK == g_pstTCP->lintCcb[Index].ucHandState)
        {
            tcpe_link_alarm(TCPE_TCP_ALARM_STATE_DISC,Index);
        }

        // ��ͨ
        g_pstTCP->lintCcb[Index].ucHandState  = TCP_LINK_FAIL ;
        g_pstTCP->lintCcb[Index].hankCount    = 0 ;
        Tcp_PacketStat(Index,eLinkLose);

        //20081205 add below
        //����ײ㴦����ͨ״̬������ʧ��,���ؽ���·,
        //ͬʱ�޸����ֻ���,��Ҫ�ȵ�������Ӧ�ɹ������Ϊ����TCP_LINK_OK,��Ӧ������·������TCP_LINK_OK
        // if( TCPE_TCP_LINK_STATE_BUILD == g_pstTCP->lintCcb[Index].ulLinkStatus  ) // ??? 20110908 cxf del
        {
           XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe link[%d] hand time out,rebuild it.",Index);

           //ֹͣ��ʱ��
           //XOS_TimerStop(FID_TCPE,g_pstTCP->lintCcb[Index].htTm); //??? 20110909 cxf del

           //�ؽ���·
           tcpe_link_release(Index);

           tcpe_link_init(Index);           
           return;
        }
        //20081205 add above
    }

    /*20110908 cxf add below */
    if( TCPE_TCP_LINK_STATE_BUILD != g_pstTCP->lintCcb[Index].ulLinkStatus) // not connected,no send shake hand msg
    {
        g_pstTCP->lintCcb[Index].hankCount ++;
        return;
    }
    /*20110908 cxf add above*/
    
    pstTcpHand = (TCP_hand_msg*) ucOutMsg ;
    //��װ��ͷ
    pstTcpHand->tcpHead.ucHeadFlag7E  = TCP_CURR_FLAG;
    pstTcpHand->tcpHead.ucHeadFlagA5  = TCP_HEAD_FLAG;

    pstTcpHand->tcpHead.MsgLen        = XOS_HtoNs(sizeof(XU16) * 5 + sizeof(XU8) );
    pstTcpHand->tcpHead.DstDeviceID   = XOS_HtoNs(g_pstTCP->lintCcb[Index].DstDeviceID);
    pstTcpHand->tcpHead.GuestDeviceID = XOS_HtoNs(g_pstTCP->GlobalDstID);
    pstTcpHand->tcpHead.UserType      = XOS_HtoNs(TCP_USERTYPE_HANDSHAKE);
    pstTcpHand->tcpHead.RouterNum     = XOS_HtoNs(TCP_ROUTER_MAX_NUM);

    //��Ϣ����,��������
    pstTcpHand->msgType               = TCP_hand_reg ;
    //��װ��β
    pstTcpHand->Tailbuff[0]           = TCP_CURR_FLAG;
    pstTcpHand->Tailbuff[1]           = TCP_END_FLAG;

    if( DEFAULT_TCPEN_MEDIA_LINKID == Index )
    {
        pstTcpHand->tcpHead.GuestDeviceID = XOS_HtoNs(0xfffd);
    }

    // ��ɰ�ͷ��װ
    pstTcpHand->comHead.stSender.ucFId     = FID_TCPE;
    pstTcpHand->comHead.stSender.usFsmId   = Index;
    pstTcpHand->comHead.stReceiver.ucFId   = FID_NTL;
    pstTcpHand->comHead.stReceiver.usFsmId = g_pstTCP->lintCcb[Index].ulLinkHandle;
    pstTcpHand->comHead.usMsgId            = SAAP_DATA_REQ ;

    pstTcpHand->comHead.usMsgLen           = sizeof(TCP_hand_msg) - sizeof(COMMON_HEADER_SAAP); //��������֮��
    if(g_Tcp_Trace)
    {
       XOS_Trace(MD(FID_TCPE,PL_MIN),"tcp linkindex = %d,dstId = 0x%x,hand msg send out",(XU16)Index,pstTcpHand->tcpHead.DstDeviceID);
    }
    SYS_MSGSEND( (COMMON_HEADER_SAAP *)pstTcpHand );

    g_pstTCP->lintCcb[Index].hankCount++ ;

}

/**********************************
��������    : TCP_hand_msg_proc
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : COMMON_HEADER_SAAP *pstMsg
����ֵ      : void
************************************/
void TCP_hand_msg_proc( COMMON_HEADER_SAAP *pstMsg )
{
    XU16  srcLinkPos=TCP_ERROR_NUM , lintno=TCP_ERROR_NUM;
    XU16  srcDpId = 0;
    XU16  msghand_sumidx=TCP_ERROR_NUM;

    TCP_hand_msg  *pMsg ;
    TCP_hand_msg  *pstTcpHand ;
    XU8  ucOutMsg[sizeof(TCP_hand_msg) + 1];

    pMsg = (TCP_hand_msg *)pstMsg;

    switch( pMsg->msgType )
    {
        case TCP_hand_reg:
            {
                //tcp server process tcp client hand reg msg
                pstTcpHand = (TCP_hand_msg*)ucOutMsg ;

                pstTcpHand->tcpHead.ucHeadFlag7E  = TCP_CURR_FLAG;  //��װ��ͷ
                pstTcpHand->tcpHead.ucHeadFlagA5  = TCP_HEAD_FLAG;

                pstTcpHand->tcpHead.MsgLen        = XOS_HtoNs(sizeof(XU16) * 5 + sizeof(XU8));
                pstTcpHand->tcpHead.DstDeviceID   = pMsg->tcpHead.GuestDeviceID;

                pstTcpHand->tcpHead.GuestDeviceID = XOS_HtoNs(g_pstTCP->GlobalDstID);
                if(gtestInOneHost) // Ϊ�����ڱ����ϲ���,
                {
                    pstTcpHand->tcpHead.DstDeviceID = pMsg->tcpHead.DstDeviceID;
                    pstTcpHand->tcpHead.GuestDeviceID = pMsg->tcpHead.DstDeviceID;
                }
                pstTcpHand->tcpHead.UserType      = XOS_HtoNs(TCP_USERTYPE_HANDSHAKE);
                pstTcpHand->tcpHead.RouterNum     = XOS_HtoNs(TCP_ROUTER_MAX_NUM);

                pstTcpHand->msgType               = TCP_hand_ack ;  //����Ӧ��

                pstTcpHand->Tailbuff[0]           = TCP_CURR_FLAG;   // ��β
                pstTcpHand->Tailbuff[1]           = TCP_END_FLAG;   // ��β

                //����Դ������������·��ϵ�����ƽ̨�ķ���
                //���,ÿ��Ŀ�������ֻ������һ����·888,������·���һ��λ
                srcDpId = XOS_NtoHs(pMsg->tcpHead.GuestDeviceID);
                srcLinkPos = TCP_SearchDstIDToIpTable(srcDpId);
                if ( TCP_ERROR_NUM == srcLinkPos)
                {
                    XOS_Trace(MD(FID_TCPE,PL_ERR),"rev hand msg,but can't find the SrcDpid(0x%x) link.",srcDpId);
                    Tcp_PacketStat(msghand_sumidx,eLinkHand);
                    return ; // error
                }
                if(srcLinkPos < TCP_MAX_IPROUTER_NUM)
                {
                    lintno = gTCPDstIDToIPTbl[srcLinkPos].usLintIndex;
                }

                if ( lintno > TCP_LINT_MAX-1)
                {
                    XOS_Trace(MD(FID_TCPE,PL_ERR),"link index(%d) err of dp(%x)",lintno,srcDpId);
                    Tcp_PacketStat(msghand_sumidx,eLinkHand);
                    return;
                }

                //�澯�ָ�
                if( TCP_LINK_FAIL == g_pstTCP->lintCcb[lintno].ucHandState)
                {
                    tcpe_link_alarm(TCPE_TCP_ALARM_STATE_CON,lintno);
                }
                /*20090911 comment below,tcp client and tcp server hand both needed*/
                //g_pstTCP->lintCcb[lintno].hankCount =  0 ;
                //g_pstTCP->lintCcb[lintno].ucHandState = TCP_LINK_OK ;
                /*20090911 comment above,tcp client and tcp server hand both needed*/

                if( DEFAULT_TCPEN_MEDIA_LINKID == lintno )
                {
                    pstTcpHand->tcpHead.GuestDeviceID = XOS_HtoNs(0xfffd);
                }

                Tcp_PacketStat(lintno,eLinkHand);
                // ��ɰ�ͷ��װ
                pstTcpHand->comHead.stReceiver.ucFId = FID_NTL;
                pstTcpHand->comHead.stSender.ucFId = FID_TCPE;
                pstTcpHand->comHead.stReceiver.usFsmId = g_pstTCP->lintCcb[lintno].ulLinkHandle;
                pstTcpHand->comHead.stSender.usFsmId = lintno;
                pstTcpHand->comHead.usMsgId            = SAAP_DATA_REQ ;
                //��������֮��
                pstTcpHand->comHead.usMsgLen           = sizeof(TCP_hand_msg) - sizeof(COMMON_HEADER_SAAP);

                SYS_MSGSEND((COMMON_HEADER_SAAP *)pstTcpHand );

            }
            break;
        case TCP_hand_ack:  // ��������ACK
            {
                pstTcpHand = (TCP_hand_msg*)pstMsg ;

                //������
                srcDpId = XOS_NtoHs(pMsg->tcpHead.GuestDeviceID);
                srcLinkPos  =  TCP_SearchDstIDToIpTable( XOS_NtoHs(pstTcpHand->tcpHead.GuestDeviceID) );

                if ( TCP_ERROR_NUM == srcLinkPos)
                {
                    Tcp_PacketStat(msghand_sumidx,eLinkHandAck);
                    XOS_Trace(MD(FID_TCPE,PL_ERR),"rev hand ack,but can not find the link of srcDpId(%x)",srcDpId);
                    return ; // error
                }
                // �õ���·��
                if(srcLinkPos < TCP_MAX_IPROUTER_NUM)
                {
                    lintno = gTCPDstIDToIPTbl[srcLinkPos].usLintIndex;
                }
                if ( lintno > TCP_LINT_MAX-1 )
                {
                    XOS_Trace(MD(FID_TCPE,PL_ERR),"link index(%d) err of dp(%x)",lintno,srcDpId);
                    Tcp_PacketStat(msghand_sumidx,eLinkHandAck);
                    return;
                }

                if ( TCP_LINK_FAIL == g_pstTCP->lintCcb[lintno].ucHandState )
                {
                    Tcp_PacketStat(lintno,eLinkTry);
                }else
                {
                    Tcp_PacketStat(lintno,eLinkHandAck);
                }
                //�澯�ָ�
                if( TCP_LINK_FAIL == g_pstTCP->lintCcb[lintno].ucHandState)
                {
                    tcpe_link_alarm(TCPE_TCP_ALARM_STATE_CON,lintno);
                }
                g_pstTCP->lintCcb[lintno].hankCount =  0 ;    // ��·��
                g_pstTCP->lintCcb[lintno].ucHandState = TCP_LINK_OK ;   // ͨ��ok

            }
            break;
        default:
            Tcp_PacketStat(msghand_sumidx,eOtherStat);
            break;
    }
}

// �˺����������������·�ŵ���Ч��
XS32 tcp_check_lintIndex(XU16 index)
{
    if( 0 == index )
    {
        return  0 ;
    }

    if( 0 < index &&  TCP_LINT_MAX > index )
    {
        return  0 ;
    }
    else
    {
        return  1 ;
    }

}
     
XS32  XOS_FIDTCPE(HANDLE hDir,XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST NTLLoginList;
    XS32 ret = XSUCC;

    XOS_MemSet( &NTLLoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    NTLLoginList.stack     = &g_TcpeFidInfo;
    XOS_StrNcpy(NTLLoginList.taskname , "Tsk_TCPE", MAX_TID_NAME_LEN);

#ifdef MAKE_SXC_LINUX    
    NTLLoginList.TID        = 221;  
#else
    NTLLoginList.TID        = FID_TCPE;  
#endif  

    NTLLoginList.prio      = TSK_PRIO_NORMAL;
    NTLLoginList.quenum = MAX_MSGS_IN_QUE;
    NTLLoginList.stacksize = 102400;
    ret = XOS_MMStartFid(&NTLLoginList,XNULLP,XNULLP);

    return ret;
}

XU16 tcpe_getDefaultLink()
{
    return g_pstTCP->ProIndex;
}

#ifdef __cplusplus
    }
#endif


