/***************************************************************
**
**  Xinwei Telecom Technology co.,ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xossctp.c
**
**  description:  sctp  implement
**
**  author: liukai
**
**  date:   2013.9.9
**
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(XOS_SCTP) && defined(XOS_WIN32)

/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xosencap.h"
#include "xosntl.h"
#include "xosarray.h"
#include "xoshash.h"
#include "xosxml.h"
#include "xosmem.h"
#include "xosha.h"
#include "xossctp_win.h"


/*-------------------------------------------------------------------------
                ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/
static t_SCTPGLOAB g_sctpCb;

/************************************************************************
������:sctp_dataArrive
����: ���ݽ��յĻص�����
����:
���:assocID - �������ݵ�ż����ID
    streamID - �������ݵ�����
    len - �������ݵĳ���
    streamSN - �������ݵ���˳���
    TSN - �������ݵĴ������к�
    protoID - ����Э���ʶ��
    unordered - ���������Ƿ�����TRUE==1==unordered, FALSE==0==normal
    ulpDataPtr - �û��������ݵ�ָ�룬����û��ʹ�ã�ΪNULL
����: void
˵��:
************************************************************************/
void sctp_dataArrive(unsigned int assocID, unsigned short streamID, unsigned int len,
                     unsigned short streamSN,unsigned int TSN, unsigned int protoID,
                     unsigned int unordered, void* ulpDataPtr)
{
    SCTP_AssociationStatus assocStatus;
    XS32 result = -1;
    XS32 i = 0;
    t_SSCB  *pServCb = NULL;
    t_SCCB *pCliCb = NULL;
    t_SCTPDATAIND dataInd;
    XU32 length;
    XU16 ssn;
    XU32 tsn;
    XCHAR *pBuf;       /*������Ϣ��ָ��*/
    XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_dataArrive(),assocID:%d,sID:%d,len:%d,SSN:%d,TSN:%d,proID:%d,unord:%d,%d!",
        assocID,streamID,len,streamSN,TSN,protoID,unordered,(int *)ulpDataPtr);
    result = xsctp_getAssocStatus(assocID, &assocStatus);
    if( result < 0)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"sctp_dataArrive()-> get peer status failed!");
        return;
    }

    pBuf = (XCHAR *)XOS_MemMalloc(FID_NTL,len);
    if (pBuf == XNULLP)
    {
        XOS_Trace(MD(FID_NTL, PL_EXP),  "sctp_dataArrive()-> malloc memory failed !");
        return;
    }
    
    length = len;
    result = xsctp_receive(assocID, streamID, (XU8 *)pBuf, &length,&ssn, &tsn, SCTP_MSG_DEFAULT);
    if( SCTP_SPECIFIC_FUNCTION_ERROR == result || result < 0)
    {
        XOS_Trace(MD(FID_NTL, PL_EXP),  "sctp_dataArrive() recv res:%d - error or no data !",result);
        XOS_MemFree(FID_NTL,pBuf);
        return;
    }

    /*�ϱ����ݵ������*/
    XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
    for(i = 0; i<g_sctpCb.genCfg.maxSctpServLink; i++)
    {
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,i);
        if( pServCb == XNULL )
        {
            continue;
        }
        if( pServCb->linkState != eStateListening)
        {
            continue;
        }
        if( pServCb->myAddr.port == assocStatus.sourcePort)
        {
            /*�������ݵ��ϲ��û�*/
            dataInd.appHandle = pServCb->userHandle;
            dataInd.dataLenth = (XU32)len;
            dataInd.pData = pBuf;
            dataInd.attr.stream = streamID;
            dataInd.attr.context = 0;
            dataInd.attr.ppid = protoID;
                        
            XOS_StrtoIp((XCHAR *)assocStatus.primaryDestinationAddress, &(dataInd.peerAddr.ip));
            dataInd.peerAddr.port = assocStatus.destPort;

            NTL_msgToUser(&dataInd,&(pServCb->linkUser),sizeof(t_SCTPDATAIND),eSctpDataInd);
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            return;
        }
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));

    /*�ϱ����ݵ��ͻ���*/
    XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
    for(i = 0; i<g_sctpCb.genCfg.maxSctpCliLink; i++)
    {
        pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,i);
        if( pCliCb == XNULL )
        {
            continue;
        }
        if( pCliCb->linkState != eStateConnected)
        {
            continue;
        }
        if( assocID == pCliCb->assocID )
        {
            /*�������ݵ��ϲ��û�*/
            dataInd.appHandle = pCliCb->userHandle;
            dataInd.dataLenth = (XU32)len;
            dataInd.pData = pBuf;
            dataInd.attr.stream = streamID;
            dataInd.attr.context = 0;
            dataInd.attr.ppid = protoID;
            XOS_StrtoIp((XCHAR *)assocStatus.primaryDestinationAddress, &(dataInd.peerAddr.ip));
            dataInd.peerAddr.port = assocStatus.destPort;

            NTL_msgToUser(&dataInd,&(pCliCb->linkUser),sizeof(t_SCTPDATAIND),eSctpDataInd);
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            return;
        }
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));

    XOS_MemFree(FID_NTL,pBuf);
    return;
}

/************************************************************************
������:sctp_sendFailure
����: ���ݷ���ʧ��ʱ�Ļص�����
����:
���:assocID - �������ݵ�ż����ID
    unsent_data - δ���͵����ݵ�����
    dataLength - ���ݵĳ���
    context - ���ݵ������ı�ʶ��
    ulpDataPtr - �û��������ݵ�ָ�룬����û��ʹ�ã�ΪNULL
����: void
˵��:
************************************************************************/
void sctp_sendFailure(unsigned int assocID,
                      unsigned char *unsent_data, unsigned int dataLength, unsigned int *context, void* ulpDataPtr)
{
    SCTP_AssociationStatus assocStatus;
    XS32 result = -1;
    XS32 ret = 0;
    t_SSCB  *pServCb = NULL;
    t_SSCLI *pServCliCb = NULL;
    t_IPADDR keyAddr;
    XS32 i = 0;
    t_SCCB *pSctpCb = XNULL;
    t_SENDSCTPERROR sendError;

    XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_sendFailure() assocID:%d,!",assocID);
    result = xsctp_getAssocStatus(assocID, &assocStatus);
    if( result < 0)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"sctp_sendFailure()-> get peer status failed!");
        return;
    }
    /*���ж��Ƿ��Ƿ���˷������ݳ���*/
    XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
    for(i = 0; i<g_sctpCb.genCfg.maxSctpServLink; i++)
    {
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,i);
        if( pServCb == XNULL )
        {
            continue;
        }

        if( pServCb->linkState != eStateListening)
        {
            continue;
        }
        XOS_StrtoIp((XCHAR *)assocStatus.primaryDestinationAddress, &(keyAddr.ip));
        keyAddr.port = assocStatus.destPort;
        XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_sendFailure()-> peer addr:%s,port:%d!",
            assocStatus.primaryDestinationAddress,keyAddr.port);
        pServCliCb = SCTP_findClient(pServCb, &(keyAddr));

        if( pServCliCb == XNULLP )
        {
            continue;
        }
        sendError.errorReson = eOtherErrorReason;
        sendError.userHandle = pServCb->userHandle;
        XOS_MemCpy(&sendError.peerIp, &(keyAddr), sizeof(t_IPADDR));
        ret = NTL_msgToUser((XVOID*)&sendError,&(pServCb->linkUser),sizeof(t_SENDSCTPERROR),eSctpErrorSend);
        if ( ret != XSUCC )
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_sendFailure()-> send msg sendError to user failed!");
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        return;
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));

    /*�ж��Ƿ�ͻ��˷������ݳ���*/
    XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
    for(i = 0; i<g_sctpCb.genCfg.maxSctpCliLink; i++)
    {
        pSctpCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,i);
        if( pSctpCb == XNULL )
        {
            continue;
        }

        if( pSctpCb->linkState != eStateConnected)
        {
            continue;
        }
        if( pSctpCb->assocID == assocID)
        {
            sendError.errorReson = eOtherErrorReason;
            sendError.userHandle = pSctpCb->userHandle;
            sendError.peerIp.ip = pSctpCb->peerAddr.ip[0];
            sendError.peerIp.port = pSctpCb->peerAddr.port;
            ret = NTL_msgToUser((XVOID*)&sendError,&(pSctpCb->linkUser),sizeof(t_SENDSCTPERROR),eSctpErrorSend);
            if ( ret != XSUCC )
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_sendFailure()-> send msg sendError to user failed!");
            }
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            return;
        }
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
    return;
}

/************************************************************************
������:sctp_networkStatusChange
����: ��������״̬�б仯ʱ�Ļص�����
����:
���:assocID - �������ݵ�ż����ID
    destAddrIndex - ��ַ������û��ʹ��
    newState - �仯���״ֵ̬��0��ʾ����,1��ʾ�Ͽ�
    ulpDataPtr - �û��������ݵ�ָ�룬����û��ʹ�ã�ΪNULL
����: void
˵��:����ֻ���������ӳɹ�������������жϴ��������Ƿ�ɹ�
************************************************************************/
void sctp_networkStatusChange(unsigned int assocID, short destAddrIndex, unsigned short newState, void* ulpDataPtr)
{
    SCTP_AssociationStatus assocStatus;
    XS32 result = -1;
    XS32 ret = 0;
    t_SSCB  *pServCb = NULL;
    t_SSCLI    servCliCb;
    t_SSCLI *pServCliCb = NULL;
    t_CONNIND connectInd;
    XS32 i = 0;
    XVOID *pLocation = NULL;
    t_SCCB *pSctpCb = XNULL;
    t_SCTPSTARTACK sctpStartAck;

    XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_networkStatusChange() assocID:%d,index:%d,status:%d,%x!",
        assocID,destAddrIndex,newState,ulpDataPtr);
    if( XSUCC != newState )
    {
        return;        /*�������ӳɹ����������״̬��Ǩ*/
    }
    result = xsctp_getAssocStatus(assocID, &assocStatus);
    if( result < 0)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"sctp_networkStatusChange()-> get peer status failed!");
        return;
    }
    XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_networkStatusChange():state:%d,numOfAdd:%d,primDest:%s,sPort:%d,dPort:%d,priIndex:%d!",
        assocStatus.state,assocStatus.numberOfAddresses,assocStatus.primaryDestinationAddress,assocStatus.sourcePort,assocStatus.destPort,assocStatus.primaryAddressIndex);

    /*�����״̬��Ǩ����*/
    XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
    for(i = 0; i<g_sctpCb.genCfg.maxSctpServLink; i++)
    {
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,i);
        if( pServCb == XNULL )
        {
            continue;
        }

        if( pServCb->linkState != eStateListening)
        {
            continue;
        }
        if( pServCb->myAddr.port == assocStatus.sourcePort)
        {
            if( pServCb->maxStream > assocStatus.outStreams )
            {
                /*�ر��½����sock*/
                xsctp_abort(assocID);
                XOS_Trace(MD(FID_NTL,PL_ERR),"new client's instream too small,(ip[0x%x],port[%d]) instream is %d,local outstream is %d!",
                    assocStatus.primaryDestinationAddress,assocStatus.destPort,assocStatus.inStreams,pServCb->maxStream);
                XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
                return;
            }

            /*����ͻ����������ܳ�����������������*/
            if((XU16)(pServCb->maxCliNum) < (XU16)(pServCb->usageNum +1))
            {
                /*�ر��½����sock*/
                xsctp_abort(assocID);
                XOS_Trace(MD(FID_NTL,PL_ERR),"new client(ip[0x%x],port[%d]) is overstep maxclis %d allowned!",
                    assocStatus.primaryDestinationAddress,assocStatus.destPort,pServCb->maxCliNum);
                XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
                return;
            }
            /*������Ŀͻ��˼ӵ�hash����*/
            XOS_MemSet(&servCliCb,0,sizeof(t_SSCLI));
            servCliCb.assocID = assocID;
            XOS_StrtoIp((XCHAR *)assocStatus.primaryDestinationAddress, &(servCliCb.destAddr.ip));
            servCliCb.destAddr.port = assocStatus.destPort;
            /*����ҲӦ�ñ���*/
            servCliCb.pServerElem = pServCb;
            servCliCb.pPreCli = pServCb->pLatestCli;
            servCliCb.pNextCli = (t_SSCLI*)XNULLP;

            /*���۶Զ����Ӷ���ip��������ȡ��һ��ip��Ϊhash��key*/
            pLocation = XNULLP;
            XOS_Trace(MD(FID_NTL,PL_DBG),"sctpServer client ip:%x,port:%d!",servCliCb.destAddr.ip,servCliCb.destAddr.port);
            XOS_MutexLock(&(g_sctpCb.hashMutex));
            pLocation = XOS_HashElemAdd(g_sctpCb.tSctpCliH,&(servCliCb.destAddr),(XVOID*)&servCliCb,XTRUE);
            XOS_MutexUnlock(&(g_sctpCb.hashMutex));

            if(pLocation == XNULLP)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"add new client to hash failed!");
                xsctp_abort(assocID);
                XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
                return;
            }

            /*������*/
            XOS_MutexLock(&(g_sctpCb.hashMutex));
            pServCliCb = (t_SSCLI*)XNULLP;
            pServCliCb = (t_SSCLI*)XOS_HashGetElem(g_sctpCb.tSctpCliH, pLocation);

            if(pServCb->pLatestCli != XNULLP && pServCliCb != XNULLP)
            {
                pServCb->pLatestCli->pNextCli = pServCliCb;
            }
            pServCb->pLatestCli= pServCliCb;
            pServCb->usageNum++; /*����һ���µĿͻ�����*/
            XOS_MutexUnlock(&(g_sctpCb.hashMutex));

            /*��������ָʾ��Ϣ���ϲ�*/
            connectInd.appHandle = pServCb->userHandle;
            connectInd.peerAddr.ip = pServCliCb->destAddr.ip;
            connectInd.peerAddr.port = pServCliCb->destAddr.port;
            ret = NTL_msgToUser(&connectInd,&(pServCb->linkUser),sizeof(t_CONNIND),eSctpConnInd);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_WARN),"indcate new client ip[0x%x],port[%d] connecttion to user failed!",
                    assocStatus.primaryDestinationAddress,assocStatus.destPort);
            }
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            return;
        }
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));

    /*�ͻ���״̬��Ǩ����*/
    XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
    for(i = 0; i<g_sctpCb.genCfg.maxSctpCliLink; i++)
    {
        pSctpCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,i);
        if( pSctpCb == XNULL )
        {
            continue;
        }

        if( pSctpCb->linkState != eStateConnecting)
        {
            continue;
        }
        if( pSctpCb->assocID == assocID)
        {
            if( pSctpCb->maxStream > assocStatus.outStreams)
            {
                /*�ر��½����sock*/
                xsctp_abort(pSctpCb->assocID);
                XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_networkStatusChange,new client(ip[0x%x],port[%d]) instream is %d,local outstream is %d!",
                    assocStatus.primaryDestinationAddress,assocStatus.destPort,assocStatus.inStreams,pSctpCb->maxStream);
                pSctpCb->linkState = eStateInited;
                XOS_Trace(MD(FID_NTL,PL_ERR),"sctp_networkStatusChange(),connect sctp server failed,peer instream too small!");

                /*���������ɹ���Ϣ���ϲ�*/
                sctpStartAck.appHandle = pSctpCb->userHandle;
                sctpStartAck.linkStartResult = eInvalidInstrm;
                XOS_MemCpy(&(sctpStartAck.localAddr),&(pSctpCb->myAddr),sizeof(t_SCTPIPADDR));

                NTL_msgToUser((XVOID*)&sctpStartAck,&(pSctpCb->linkUser),sizeof(t_SCTPSTARTACK),eSctpStartAck);
                XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
                return;
            }

            /*�ͻ������ӳɹ�*/
            if(pSctpCb->linkState != eStateConnected)
            {
                pSctpCb->linkState = eStateConnected;
                XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_networkStatusChange(),connect sctp server successed!");

                /*���������ɹ���Ϣ���ϲ�*/
                sctpStartAck.appHandle = pSctpCb->userHandle;
                sctpStartAck.linkStartResult = eSUCC;
                XOS_MemCpy(&(sctpStartAck.localAddr),&(pSctpCb->myAddr),sizeof(t_SCTPIPADDR));

                NTL_msgToUser((XVOID*)&sctpStartAck,&(pSctpCb->linkUser),sizeof(t_SCTPSTARTACK),eSctpStartAck);
            }
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            return;
        }
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
    
    return;
}

/************************************************************************
������:sctp_communicationUp
����: ��������״̬�б仯ʱ�Ļص�����
����:
���:assocID - ����������ż����ID
    status - ��ַ������û��ʹ��
    noOfDestinations - �Զ˵ĵ�ַ��
    noOfInStreams - �Զ˵�������
    noOfOutStreams - �Զ˵ĳ�����
    associationSupportsPRSCTP
    dummy
����: NULL
˵��:���յ������͵��Ĵ����ֱ���ʱ���ã����账��
************************************************************************/
void* sctp_communicationUp(unsigned int assocID, int status,
                           unsigned int noOfDestinations,
                           unsigned short noOfInStreams, unsigned short noOfOutStreams,
                           int associationSupportsPRSCTP, void* dummy)
{
    SCTP_AssociationStatus assocStatus;
    XS32 result = -1;

    XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_communicationUp():assocID:%d,status:%d,dest no:%d,in:%d,out:%d,sup:%d,%x!",
        assocID,status,noOfDestinations,noOfInStreams,noOfOutStreams,associationSupportsPRSCTP,dummy);

    result = xsctp_getAssocStatus(assocID, &assocStatus);
    if( result < 0)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"sctp_communicationUp()-> get peer status failed!");
        return NULL;
    }
    XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_communicationUp():state:%d,numOfAdd:%d,primDest:%s,sPort:%d,dPort:%d,priIndex:%d!",
        assocStatus.state,assocStatus.numberOfAddresses,assocStatus.primaryDestinationAddress,assocStatus.sourcePort,assocStatus.destPort,assocStatus.primaryAddressIndex);

    switch(status)
    {
        case SCTP_COMM_UP_RECEIVED_VALID_COOKIE:  /*������յ�cookie_echoʱ*/
        XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_communicationUp()-> SCTP_COMM_UP_RECEIVED_VALID_COOKIE!");
            break;

        case SCTP_COMM_UP_RECEIVED_COOKIE_ACK:  /*�ͻ����յ�cookie_ackʱ*/
        XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_communicationUp()-> SCTP_COMM_UP_RECEIVED_COOKIE_ACK!");
            break;
        default:
            break;
        
    }
    return NULL;
}

/************************************************************************
������:sctp_communicationLost
����: �������ӹرջ�Ͽ�ʱ�Ļص�����
����:
���:assocID - ����������ż����ID
    status - ����رյ�״ֵ̬
    ulpDataPtr - �û��������ݵ�ָ�룬����û��ʹ�ã�ΪNULL
����: void
˵��:
************************************************************************/
void sctp_communicationLost(unsigned int assocID, unsigned short status, void* ulpDataPtr)
{
    XU8 buffer[XOS_INET_MAX_UDPRAW_MSGSIZE];
    XU32 bufferLength;
    XU16 streamID, streamSN;
    XU32 protoID;
    XU32 tsn;
    XU8 flags;
    void* ctx;

    XS32 ret = 0;
    t_SSCB  *pServCb = NULL;
    t_SSCLI *pServCliCb = NULL;
    XS32 i = 0;
    t_SCCB *pSctpCb = XNULL;
    t_LINKCLOSEIND closeInd;

    XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_communicationLost() assocID:%d,status:%d,ptr:%x!",
        assocID,status,ulpDataPtr);
    /* retrieve data */
    bufferLength = sizeof(buffer);
    while (xsctp_receiveUnsent(assocID, buffer, &bufferLength,  &tsn,
                               &streamID, &streamSN, &protoID, &flags, &ctx) >= 0){
        /* do something with the retrieved data */
        /* after that, reset bufferLength */
        bufferLength = sizeof(buffer);
    }

    bufferLength = sizeof(buffer);
    while (xsctp_receiveUnacked(assocID, buffer, &bufferLength, &tsn,
                &streamID, &streamSN, &protoID, &flags, &ctx) >= 0){
        /* do something with the retrieved data */
        /* after that, reset bufferLength */
        bufferLength = sizeof(buffer);
    }

    /* free ULP data */
    /*communicationUpNotif���ؿ�ָ�룬���ﲻ��free*/
    //free((struct ulp_data *) ulpDataPtr);

    /* delete the association */
    xsctp_deleteAssociation(assocID);
    
    /*���ж��Ƿ��Ƿ���˶Ͽ�����*/
    XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
    for(i = 0; i<g_sctpCb.genCfg.maxSctpServLink; i++)
    {
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,i);
        if( pServCb == XNULL )
        {
            continue;
        }

        if( pServCb->linkState != eStateListening)
        {
            continue;
        }

        pServCliCb = pServCb->pLatestCli;
        while( pServCliCb != XNULLP )
        {
            if( assocID == pServCliCb->assocID )
            {
                closeInd.closeReason= eNetError;
                closeInd.appHandle = pServCb->userHandle;
                XOS_MemCpy(&closeInd.peerAddr, &(pServCliCb->destAddr), sizeof(t_IPADDR));
                ret = NTL_msgToUser((XVOID*)&closeInd,&(pServCb->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);
                if ( ret != XSUCC )
                {
                    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_communicationLost()-> send msg eSctpStopInd to user failed!");
                }
                SCTP_closeTsCli(pServCliCb);
                XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
                return;
            }
            pServCliCb = pServCliCb->pPreCli;
        }
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));

    /*�ж��Ƿ�ͻ��˶Ͽ�����*/
    XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
    for(i = 0; i<g_sctpCb.genCfg.maxSctpCliLink; i++)
    {
        pSctpCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,i);
        if( pSctpCb == XNULL )
        {
            continue;
        }

        if( pSctpCb->linkState != eStateConnected)
        {
            continue;
        }
        if( pSctpCb->assocID == assocID)
        {
            xsctp_unregisterInstance(pSctpCb->instance);

            pSctpCb->linkState = eStateInited;

            closeInd.closeReason = eNetError;
            closeInd.appHandle = pSctpCb->userHandle;
            closeInd.peerAddr.ip = pSctpCb->peerAddr.ip[0];
            closeInd.peerAddr.port = pSctpCb->peerAddr.port;
            ret = NTL_msgToUser((XVOID*)&closeInd,&(pSctpCb->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);
            if ( ret != XSUCC )
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_communicationLost()-> send msg eSctpStopInd to user failed!");
            }
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            return;
        }
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
    return;
}

/************************************************************************
������:sctp_communicationError
����: �������ӳ���ʱ�Ļص�����
����:
���:assocID - �����ż����ID
    status - ��������״ֵ̬
    dummy
����: void
˵��:ƽ̨û�д�������ص�����
************************************************************************/
void sctp_communicationError(unsigned int assocID, unsigned short status, void* dummy)
{
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_communicationError() assocID:%d,!",assocID);
}

/************************************************************************
������:sctp_restart
����: ������������ʱ�Ļص�����
����:
���:assocID - Ҫ���õ�ż����ID
    ulpDataPtr - �û��������ݵ�ָ�룬����û��ʹ�ã�ΪNULL
����: void
˵��:ƽ̨û�д�������ص�����
************************************************************************/
void sctp_restart(unsigned int assocID, void* ulpDataPtr)
{
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_restart()! assocID:%d",assocID);
}

/************************************************************************
������:sctp_shutdownComplete
����: �������ӹر����ʱ�Ļص�����
����:
���:assocID - �رյ�ż����ID
    ulpDataPtr - �û��������ݵ�ָ�룬����û��ʹ�ã�ΪNULL
����: void
˵��:
************************************************************************/
void sctp_shutdownComplete(unsigned int assocID, void* ulpDataPtr)
{
    XS32 ret = 0;
    t_SSCB  *pServCb = NULL;
    t_SSCLI *pServCliCb = NULL;
    XS32 i = 0;
    t_SCCB *pSctpCb = XNULL;
    t_LINKCLOSEIND closeInd;

    XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_shutdownComplete() assocID:%d!",assocID);
    xsctp_deleteAssociation(assocID);

    /*���ж��Ƿ��Ƿ���˹ر�*/
    XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
    for(i = 0; i<g_sctpCb.genCfg.maxSctpServLink; i++)
    {
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,i);
        if( pServCb == XNULL )
        {
            continue;
        }

        if( pServCb->linkState != eStateListening)
        {
            continue;
        }

        pServCliCb = pServCb->pLatestCli;
        while( pServCliCb != XNULLP )
        {
            if( assocID == pServCliCb->assocID )
            {
                closeInd.closeReason= ePeerReq;
                closeInd.appHandle= pServCb->userHandle;
                XOS_MemCpy(&closeInd.peerAddr, &(pServCliCb->destAddr), sizeof(t_IPADDR));
                ret = NTL_msgToUser((XVOID*)&closeInd,&(pServCb->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);
                if ( ret != XSUCC )
                {
                    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_communicationLost()-> send msg eSctpStopInd to user failed!");
                }
                SCTP_closeTsCli(pServCliCb);
                XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
                return;
            }
            pServCliCb = pServCliCb->pPreCli;
        }
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));

    /*�ж��Ƿ�ͻ��˹ر�*/
    XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
    for(i = 0; i<g_sctpCb.genCfg.maxSctpCliLink; i++)
    {
        pSctpCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,i);
        if( pSctpCb == XNULL )
        {
            continue;
        }

        if( pSctpCb->linkState != eStateConnected)
        {
            continue;
        }
        if( pSctpCb->assocID == assocID)
        {
            xsctp_unregisterInstance(pSctpCb->instance);
            closeInd.closeReason = ePeerReq;
            closeInd.appHandle = pSctpCb->userHandle;
            closeInd.peerAddr.ip = pSctpCb->peerAddr.ip[0];
            closeInd.peerAddr.port = pSctpCb->peerAddr.port;
            ret = NTL_msgToUser((XVOID*)&closeInd,&(pSctpCb->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);
            if ( ret != XSUCC )
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_communicationLost()-> send msg eSctpStopInd to user failed!");
            }
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            return;
        }
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
    return;
}

/************************************************************************
������:sctp_shutdownReceived
����: ���յ��Զ˷��͵Ĺر����η��ֵ�һ������ʱ�Ļص�����
����:
���:assocID - �رյ�ż����ID
    ulpDataPtr - �û��������ݵ�ָ�룬����û��ʹ�ã�ΪNULL
����: void
˵��:ƽ̨û�д�������ص�����
************************************************************************/
void sctp_shutdownReceived(unsigned int assocID, void* ulpDataPtr)
{
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_shutdownReceived() assocID:%d!",assocID);
}

/************************************************************************
������:SCTP_init
����:  sctp��̬���ʼ������
����:
���:
����: 
˵��:
************************************************************************/
XS32 SCTP_init()
{
    SCTP_LibraryParameters params;
    XS32 ret = -1;
    /* initialize the echo_ulp variable */

    ret = sctp_initLibrary();
    if( XSUCC != ret )
    {
        return XERROR;
    }
    ret = sctp_getLibraryParameters(&params);
    if( XSUCC != ret )
    {
        return XERROR;
    }
    params.sendOotbAborts = 0;          /*0:ignore ootb packets,1:do not ignore*/
    /* params.checksumAlgorithm = SCTP_CHECKSUM_ALGORITHM_ADLER32; */
    params.checksumAlgorithm = SCTP_CHECKSUM_ALGORITHM_CRC32C;
    ret = sctp_setLibraryParameters(&params);
    if( XSUCC != ret )
    {
        return XERROR;
    }
    setInitHB(2000);     /*scope:100 - 600000(ms)*/
    setAssocMaxRetrans(2);     /*scope:1-50*/
    setRtoInit(1500);     /*scope:100 - 600000(ms)*/
    setRtoMin(1000);     /*scope:100 - 600000(ms)*/
    setRtoMax(10000);     /*scope:100 - 600000(ms)*/

    return XSUCC;
}

/************************************************************************
������:event_loop
����:  sctp�¼���ѯ�߳�
����:
���:
����: 
˵��:�ͻ��������˶�ͨ�������ѯ�ϱ��¼�
************************************************************************/
XVOID  event_loop()
{
    while(1)
    {
        sctp_eventLoop();
    }
}

/************************************************************************
������:sctp_msg
����:  sctp��Ϣ�����߳�
����:
���:
����: 
˵��:sctplib�ⷢ�͵���Ϣ��ͨ������߳̽��д���Ȼ�������Ӧ�Ĵ�����
************************************************************************/
XVOID  sctp_msg()
{
    msgNode msg;
    msg_dataArrive msgDA;
    msg_sendFailure msgSF;
    msg_networkStatusChange msgNSC;
    msg_communicationUp msgCU;
    msg_communicationLost msgCL;
    msg_communicationError msgCE;
    msg_restart msgRE;
    msg_shutdownComplete msgSC;
    msg_shutdownReceived msgSR;
    
    while(1)
    {
        msg = getMsg();

        switch(msg.type)
        {
            case dataArrive:
                msgDA = msg.msg.dataArrive;
                sctp_dataArrive(msgDA.assocID, msgDA.streamID, msgDA.len, msgDA.streamSN, 
                    msgDA.tsn, msgDA.protoID, msgDA.unordered, msgDA.ulpDataPtr);
                break;
                
            case sendFailure:
                msgSF = msg.msg.sendFailure;
                sctp_sendFailure(msgSF.assocID, msgSF.unsent_data, msgSF.dataLength, 
                    msgSF.context, msgSF.ulpDataPtr);
                break;
                
            case networkStatusChange:
                msgNSC = msg.msg.networkStatusChange;
                sctp_networkStatusChange(msgNSC.assocID, msgNSC.destAddrIndex, msgNSC.newState,
                   msgNSC.ulpDataPtr);
                break;
                
            case communicationUp:
                msgCU = msg.msg.communicationUp;
                sctp_communicationUp(msgCU.assocID, msgCU.status,msgCU.noOfDestinations, 
                    msgCU.noOfInStreams, msgCU.noOfOutStreams, msgCU.associationSupportsPRSCTP, msgCU.dummy);
                break;
                
            case communicationLost:
                msgCL = msg.msg.communicationLost;
                sctp_communicationLost(msgCL.assocID, msgCL.status, msgCL.ulpDataPtr);
                break;
                
            case communicationError:
                msgCE = msg.msg.communicationError;
                sctp_communicationError(msgCE.assocID, msgCE.status, msgCE.dummy);
                break;
                
            case restart:
                msgRE = msg.msg.restart;
                sctp_restart(msgRE.assocID, msgRE.ulpDataPtr);
                break;
                
            case shutdownComplete:
                msgSC = msg.msg.shutdownComplete;
                sctp_shutdownComplete(msgSC.assocID, msgSC.ulpDataPtr);
                break;
                
            case shutdownReceived:
                msgSR = msg.msg.shutdownReceived;
                sctp_shutdownReceived(msgSR.assocID, msgSR.ulpDataPtr);
                break;
                
            default:
                XOS_Trace(MD(FID_NTL,PL_WARN),"sctp_msg()->unknown message type!");
                break;
        }
    }
}


/************************************************************************
������:SCTP_findClient
����:  ����sctp ���ӵĿͻ�
����:
pTserverCb ��server cb��ָ��
pClientAddr  �� �ӽ����ͻ��ĵ�ַָ��
���:
����: �ɹ����ؿ��ƿ�ָ��,���򷵻�xnullp
˵��:
************************************************************************/
t_SSCLI * SCTP_findClient(t_SSCB *pTserverCb,t_IPADDR *pClientAddr)
{
    t_SSCLI *pServCliCb = NULL;

#ifdef INPUT_PAR_CHECK
    if(pTserverCb == XNULLP || pClientAddr == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_findClient()->input param error!");
        return (t_SSCLI*)XNULLP;
    }

#endif
    XOS_MutexLock(&(g_sctpCb.hashMutex));
    pServCliCb = (t_SSCLI*)XOS_HashElemFind(g_sctpCb.tSctpCliH,(XVOID*)pClientAddr);
    XOS_MutexUnlock(&(g_sctpCb.hashMutex));
    return pServCliCb;
}

/************************************************************************
������:SCTP_closeTsCli
����: �ر�һ��sctp server ����Ŀͻ���
����:pSctpServCli ��server client �Ŀ��ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_closeTsCli(t_SSCLI* pSctpServCli)
{
    if(pSctpServCli == XNULLP)
    {
        return XERROR;
    }

    xsctp_abort(pSctpServCli->assocID);  
    xsctp_deleteAssociation(pSctpServCli->assocID);
    
    /*�������жϿ�,�޸�server cb ������*/
    (pSctpServCli->pServerElem)->usageNum--;
    /*������β���Ľڵ�*/
    if(pSctpServCli->pNextCli == XNULLP)
    {
        (pSctpServCli->pServerElem)->pLatestCli = pSctpServCli->pPreCli;
    }
    else
    {
        pSctpServCli->pNextCli->pPreCli = pSctpServCli->pPreCli;
    }
    
    /*����ͷ�ڵ�*/
    if( pSctpServCli->pPreCli != XNULLP)
    {
        pSctpServCli->pPreCli->pNextCli = pSctpServCli->pNextCli;
    }
    
    /*��hash ��ɾ��*/
    XOS_HashDelByElem(g_sctpCb.tSctpCliH, pSctpServCli);
    return XSUCC;
}

/************************************************************************
������:SCTP_closeReqProc
����:  ������·�ر�������Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_closeReqProc(t_XOSCOMMHEAD* pMsg)
{
    t_LINKCLOSEREQ *pCloseReq = NULL;
    e_LINKTYPE linkType;
    t_SCCB *pCliCb = NULL;
    t_SSCB *pServCb = NULL;
    XS32 ret  = 0;
    t_SSCLI *pSctpServerClient = NULL;

    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_closeReqProc()->pMsg invalid!");
        return XERROR;
    }
    pCloseReq = (t_LINKCLOSEREQ*)(pMsg->message);
    if(!pCloseReq)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_closeReqProc()->pCloseReq invalid!");
        return XERROR;
    }

    /*���е���Ϣ��Ҫ������ĵ���Ч��*/
    if(!NTL_isValidLinkH(pCloseReq->linkHandle))
    {
        /*��·�����Ч����Ϣ��ֱ�Ӷ���*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_closeReqProc()->FID[%d] linkHandle invalid!", pMsg->datasrc.FID);
        return XERROR;
    }
    /*��ȡ��·����*/
    linkType = NTL_getLinkType(pCloseReq->linkHandle);

    switch(linkType)
    {
    case eSCTPClient:
        /*��ȡsctp ���ƿ�*/
        pCliCb = (t_SCCB*)XNULLP;

        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));        
        pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));

        if(pCliCb == XNULLP)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_closeReqProc()->get the sctp client control block failed!");
            return XERROR;
        }

        xsctp_abort(pCliCb->assocID);
        xsctp_deleteAssociation(pCliCb->assocID);
        xsctp_unregisterInstance(pCliCb->instance);
        pCliCb->linkState = eStateInited;

        /*���Զ˵�ַ�ÿ�*/
        memset(pCliCb->peerAddr.ip,0,sizeof(pCliCb->peerAddr.ip[0]) * SCTP_ADDR_NUM);
        pCliCb->peerAddr.ipNum = 0;
        pCliCb->peerAddr.port = 0;            

        memset(pCliCb->myAddr.ip,0,sizeof(pCliCb->myAddr.ip[0]) * SCTP_ADDR_NUM);
        pCliCb->myAddr.ipNum = 0;
        pCliCb->myAddr.port = 0;

        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        break;
    case eSCTPServer:
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        
        if(pServCb == XNULLP)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_closeReqProc()->get the sctp server control block failed!");
            return XERROR;
        }
        if(pServCb->linkState != eStateListening)
        {
            /*��·״̬����,��Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_closeReqProc()->sctp serv  link state is wrong!");
            ret = (pServCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            return ret;
        }           
       
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        if(0x00 == pCloseReq->cliAddr.ip && 0x00 == pCloseReq->cliAddr.port)
        {
            XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
            /*���ȹر����н���Ŀͻ���*/     
            XOS_MutexLock(&(g_sctpCb.hashMutex));               

            while(pServCb->pLatestCli != XNULLP)
            {
                SCTP_closeTsCli(pServCb->pLatestCli);
                if(pServCb->usageNum == 0)
                {
                    break;
                }
            }

            XOS_MutexUnlock(&(g_sctpCb.hashMutex));

            /*�ر�fd*/
            ret = xsctp_unregisterInstance(pServCb->instance);
            /*�ı���Ӧ��cb ����,��ʼ��*/
            pServCb->linkState = eStateInited;
            pServCb->maxCliNum = 0;
            pServCb->usageNum = 0;
            pServCb->pLatestCli = (t_SSCLI*)XNULLP;
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        }
        else
        {   
            XOS_MutexLock(&(g_sctpCb.hashMutex));

            /*����ָ���Ŀͻ��˿��ƿ�*/
            pSctpServerClient = (t_SSCLI*)XOS_HashElemFind(g_sctpCb.tSctpCliH, (XVOID*)&(pCloseReq->cliAddr));
            if(NULL != pSctpServerClient)
            {
                XOS_Trace(MD(FID_NTL, PL_INFO),"close sctp client ip=0x%08x, port=%d", pCloseReq->cliAddr.ip, pCloseReq->cliAddr.port);
                SCTP_closeTsCli(pSctpServerClient);
            }

            XOS_MutexUnlock(&(g_sctpCb.hashMutex));
        }
        break;
    default:
        break;
    }

    return XSUCC;
}

/************************************************************************
������:SCTP_closeReqForRelease
����:  ������·�ͷ�������Ϣ--ֻ����SCTP_linkReleaseProc����
����:  pMsg ����Ϣָ��
���:
����:  �ɹ�����XSUCC,���򷵻�XERROR
˵��:  �˺�����SCTP_closeReqProc������sctpserver�ϴ���ͬ
************************************************************************/
XS32 SCTP_ReleaseLink(t_XOSCOMMHEAD* pMsg)
{
    t_LINKRELEASE *pReleaseReq = NULL;
    e_LINKTYPE linkType;
    t_SCCB *pCliCb = NULL;
    t_SSCB *pServCb = NULL;
    XS32 ret  = 0;

    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ReleaseLink()->pMsg invalid!");
        return XERROR;
    }
    pReleaseReq = (t_LINKRELEASE*)(pMsg->message);
    if(!pReleaseReq)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ReleaseLink()->pCloseReq invalid!");
        return XERROR;
    }

    /*���е���Ϣ��Ҫ������ĵ���Ч��*/
    if(!NTL_isValidLinkH(pReleaseReq->linkHandle))
    {
        /*��·�����Ч����Ϣ��ֱ�Ӷ���*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ReleaseLink()->FID[%d] linkHandle invalid!", pMsg->datasrc.FID);
        return XERROR;
    }
    /*��ȡ��·����*/
    linkType = NTL_getLinkType(pReleaseReq->linkHandle);

    switch(linkType)
    {
    case eSCTPClient:
        /*��ȡsctp ���ƿ�*/
        pCliCb = (t_SCCB*)XNULLP;

        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));        
        pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,NTL_getLinkIndex(pReleaseReq->linkHandle));
        
        if(pCliCb == XNULLP)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_ReleaseLink()->get the sctp client control block failed!");
            return XERROR;
        }

        /*����ر�*/
        xsctp_abort(pCliCb->assocID);
        xsctp_deleteAssociation(pCliCb->assocID);
        xsctp_unregisterInstance(pCliCb->instance);

        pCliCb->linkState = eStateInited;

        /*���Զ˵�ַ�ÿ�*/
        memset(pCliCb->peerAddr.ip,0,sizeof(pCliCb->peerAddr.ip[0]) * SCTP_ADDR_NUM);
        pCliCb->peerAddr.port = 0;
        pCliCb->peerAddr.ipNum = 0;
        
        memset(pCliCb->myAddr.ip,0,sizeof(pCliCb->myAddr.ip[0]) * SCTP_ADDR_NUM);
        pCliCb->myAddr.port = 0;
        pCliCb->myAddr.ipNum = 0;
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        break;
    case eSCTPServer:
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,NTL_getLinkIndex(pReleaseReq->linkHandle));
        
        if(pServCb == XNULLP)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_ReleaseLink()->get the sctp server control block failed!");
            return XERROR;
        }
        if(pServCb->linkState != eStateListening)
        {
            /*��·״̬����,��Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_ReleaseLink()->sctp serv  link state is wrong!");
            ret = (pServCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            return ret;
        }               

        /*���ȹر����н���Ŀͻ���*/     
        XOS_MutexLock(&(g_sctpCb.hashMutex));               

        while(pServCb->pLatestCli != XNULLP)
        {
            SCTP_closeTsCli(pServCb->pLatestCli);
            if(pServCb->usageNum == 0)
            {
                break;
            }
        }

        XOS_MutexUnlock(&(g_sctpCb.hashMutex));

        /*�ر���·*/
        ret = xsctp_unregisterInstance(pServCb->instance);

        /*�ı���Ӧ��cb ����,��ʼ��*/
        pServCb->linkState = eStateInited;
        pServCb->maxCliNum = 0;
        pServCb->usageNum = 0;
        pServCb->pLatestCli = (t_SSCLI*)XNULLP;
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));

        break;

    default:
        break;
    }

    return XSUCC;
}

/************************************************************************
������:SCTP_genCfgProc
����:  ����ͨ��������Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: SCTP ���յ�ͨ��������Ϣ����������������ɸ�
������ĳ�ʼ����
************************************************************************/
XS32 SCTP_genCfgProc(t_NTLGENCFG* pGenCfg)
{
    XS32 i = 0;
    XS32 ret = 0;
    XU32 hashElems = 0;
    t_XOSTASKID eventTaskId;
    XS32 eventTaskIndex = 0;
    t_XOSTASKID msgTaskId;
    XS32 msgTaskIndex = 0;
        
    if(pGenCfg == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->bad input param!");
        return XERROR;
    }

    /*����������Ϣ*/
    XOS_MemCpy(&(g_sctpCb.genCfg),pGenCfg,sizeof(t_NTLGENCFG));

    /* sctp client  link �����������*/
    if (g_sctpCb.genCfg.maxSctpCliLink > 0 )
    {
        /*����sctp client���ƿ���Դ*/
        g_sctpCb.sctpClientLinkH = XNULL;
        g_sctpCb.sctpClientLinkH =  XOS_ArrayConstruct(sizeof(t_SCCB), g_sctpCb.genCfg.maxSctpCliLink, "sctpClilinkH");
        if(XNULL == g_sctpCb.sctpClientLinkH)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->construct sctpCli array failed!");
            goto genCfgError;
        }
        XOS_ArraySetCompareFunc(g_sctpCb.sctpClientLinkH, SCTP_channel_find_function);

        /*sctp�ͻ�����·���ƿ黥�����*/
        ret = XOS_MutexCreate(&(g_sctpCb.sctpClientLinkMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctpclient thread[%d]  semaphore  failed!",i);
            goto genCfgError;
        }         
    }

    /*sctp server ��ص�����,sctp serverĿǰ���õ�������,�Ժ������չ*/
    if(g_sctpCb.genCfg.maxSctpServLink > 0)
    {
        /*����sctp server ���ƿ�����*/
        g_sctpCb.sctpServerLinkH = XNULL;
        g_sctpCb.sctpServerLinkH = XOS_ArrayConstruct( sizeof(t_SSCB),g_sctpCb.genCfg.maxSctpServLink,"sctpServH");
        if(XNULL == g_sctpCb.sctpServerLinkH )
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->construct sctpServ array failed!");
            goto genCfgError;
        }
 
        XOS_ArraySetCompareFunc(g_sctpCb.sctpServerLinkH, SCTP_channel_find_function);

        /*sctp�������·���ƿ黥�����*/
        ret = XOS_MutexCreate(&(g_sctpCb.sctpServerLinkMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctpserver thread[%d]  semaphore  failed!",i);
            goto genCfgError;
        }

        /*��������ͻ��˵�hash*/
        hashElems = (g_sctpCb.genCfg.maxSctpServLink)*SCTP_CLIENTS_PER_SERV;
        g_sctpCb.tSctpCliH = XOS_HashConstruct(hashElems, hashElems, sizeof(t_IPADDR),
            sizeof(t_SSCLI),"sctpServCliH");

        if(g_sctpCb.tSctpCliH == XNULL)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->construct SctpCli hash failed!");
            goto genCfgError;
        }

        /*set the hash func */
        ret = XOS_HashSetHashFunc(g_sctpCb.tSctpCliH, NTL_tcliHashFunc);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->set  SctpCli hashfunc failed!");
            goto genCfgError;
        }

        /*set the hash key compare func*/
        ret = XOS_HashSetKeyCompareFunc(g_sctpCb.tSctpCliH, NTL_cmpIpAddr);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->set hash key compare failed!");
            goto genCfgError;
        }

        /*��ʼ��hash �Ļ�����*/
        ret = XOS_MutexCreate(&(g_sctpCb.hashMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->create  mutex for sctphash failed!");
            goto genCfgError;
        }
    }
    ret = XOS_TaskCreate("win_sctp_event",TSK_PRIO_NORMAL,NTL_RECV_TSK_STACK_SIZE,(os_taskfunc)event_loop,
    (XVOID *)(XPOINT)eventTaskIndex,&eventTaskId);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc() create win_sctp_svent thread failed!");
        goto genCfgError;
    }
    ret = XOS_TaskCreate("win_sctp_msg",TSK_PRIO_NORMAL,NTL_RECV_TSK_STACK_SIZE,(os_taskfunc)sctp_msg,
    (XVOID *)(XPOINT)msgTaskIndex,&msgTaskId);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc() create win_sctp_msg thread failed!");
        goto genCfgError;
    }

    g_sctpCb.isGenCfg = XTRUE;
    return XSUCC;

genCfgError:
    /*�ͷ���Դ*/
    XOS_ArrayDestruct(g_sctpCb.sctpClientLinkH);
    XOS_ArrayDestruct(g_sctpCb.sctpServerLinkH);
    XOS_HashDestruct(g_sctpCb.tSctpCliH);

        
    XOS_MutexDelete(&(g_sctpCb.hashMutex));

    /*���*/
    XOS_MemSet(&g_sctpCb,0,sizeof(t_SCTPGLOAB));
    /*���ó�û�г�ʼ��*/
    g_sctpCb.isGenCfg = XFALSE;
    return XERROR;
}

/************************************************************************
������:SCTP_DeleteCB
����:  �ͷſ��ƿ�
����:  linkType ����·����
       linkIndex--��·����
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_DeleteCB(e_LINKTYPE linkType, XS32 linkIndex)
{
    XS32 nRst = 0;
    switch (linkType)
    {
    case eSCTPClient:
        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
        nRst = XOS_ArrayDeleteByPos(g_sctpCb.sctpClientLinkH, linkIndex);
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        break;

    case eSCTPServer:
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        nRst = XOS_ArrayDeleteByPos(g_sctpCb.sctpServerLinkH, linkIndex);
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        break;
            
    default:
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_DeleteCB()->unknown link type coming!");
        break;
    }

    if(XSUCC != nRst)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_DeleteCB()->delete failed");
    }
    return nRst;
}

/************************************************************************
������:SCTP_linkInitProc
����:  ������·��ʼ����Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_linkInitProc(t_XOSCOMMHEAD* pMsg)
{
    t_LINKINIT *pLinkInit = XNULL;
    t_SCCB *pCliCb = XNULL;
    t_SSCB *pSctpSrvCb = XNULL;
    t_LINKINITACK linkInitAck;
    t_Link_Index LinkIndex;
    XS32 linkIndex = -1;
    XS32 ret = 0;
    XS32 nRtnFind = XERROR;
    
    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_linkInitProc()->pMsg is null!");
        return XERROR;
    }
    XOS_MemSet(&linkInitAck,0,sizeof(t_LINKINITACK));
    pLinkInit = (t_LINKINIT *)(pMsg->message);
    if(!pLinkInit)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_linkInitProc()->pLinkInit is null!");
        return XERROR;
    }
    
    LinkIndex.linkUser = (t_XOSUSERID*)&pMsg->datasrc;
    LinkIndex.userHandle =  pLinkInit->appHandle;
    LinkIndex.linkType = pLinkInit->linkType;

    switch (pLinkInit->linkType)
    {
    case eSCTPClient:
        
        pCliCb = (t_SCCB*)XNULLP;
        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
        nRtnFind = XOS_ArrayFind(g_sctpCb.sctpClientLinkH, &LinkIndex);
        
        if(XERROR != nRtnFind)
        {
            /*������·*/
            SCTP_ResetLinkByReapplyEntry(eSCTPClient, &linkInitAck, &(pLinkInit->appHandle), &nRtnFind);
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkInitProc()->add the sctp client cb was reset!");
        }
        else
        {
            /*��ӵ�æ������*/
            linkIndex = XOS_ArrayAddExt(g_sctpCb.sctpClientLinkH,(XOS_ArrayElement *) &pCliCb);
            
            if((linkIndex >= 0) && (pCliCb != XNULLP))
            {
                /*��ʼ�����ƿ����*/
                pCliCb->userHandle = pLinkInit->appHandle;
                XOS_MemCpy(&(pCliCb->linkUser),&(pMsg->datasrc),sizeof(t_XOSUSERID));
                pCliCb->linkHandle = NTL_buildLinkH(eSCTPClient,(XU16)linkIndex);
                pCliCb->linkState = eStateInited;
                memset(pCliCb->myAddr.ip,0,sizeof(pCliCb->myAddr.ip[0])*SCTP_ADDR_NUM);
                pCliCb->myAddr.port = 0;
                memset(pCliCb->peerAddr.ip,0,sizeof(pCliCb->myAddr.ip[0])*SCTP_ADDR_NUM);
                pCliCb->peerAddr.port = 0;
                
                /*�ظ�eLinkInitAck�Ĳ���*/
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eSUCC;
                linkInitAck.linkHandle= pCliCb->linkHandle;
            }
            else
            {
                /*�ظ���·ȷ��ʧ��*/
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkInitProc()->add the sctp client cb to array failed!");
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eFAIL;
            }
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        break;

    case eSCTPServer:
        pSctpSrvCb = (t_SSCB*)XNULLP;
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        nRtnFind = XOS_ArrayFind(g_sctpCb.sctpServerLinkH, &LinkIndex);
        
        if(XERROR != nRtnFind)
        {
            /*������·*/
            SCTP_ResetLinkByReapplyEntry(eSCTPServer, &linkInitAck, &(pLinkInit->appHandle), &nRtnFind);
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkInitProc()->add the sctp server cb was reset!");
        }
        else
        {
            /*��ӵ�æ������*/
            linkIndex = XOS_ArrayAddExt(g_sctpCb.sctpServerLinkH,(XOS_ArrayElement *)&pSctpSrvCb);
            
            if((linkIndex >= 0) && (pSctpSrvCb != XNULLP))
            {
                /*��д���ƿ����*/
                pSctpSrvCb->userHandle = pLinkInit->appHandle;
                XOS_MemCpy(&(pSctpSrvCb->linkUser),&(pMsg->datasrc),sizeof(t_XOSUSERID));
                pSctpSrvCb->linkHandle = NTL_buildLinkH(eSCTPServer,(XU16)linkIndex);
                pSctpSrvCb->linkState = eStateInited;
                memset(pSctpSrvCb->myAddr.ip,0,sizeof(pSctpSrvCb->myAddr.ip[0]) * SCTP_ADDR_NUM);
                pSctpSrvCb->myAddr.port = 0;
                pSctpSrvCb->maxCliNum = 0;
                pSctpSrvCb->usageNum = 0;
                pSctpSrvCb->pLatestCli = NULL;

                /*�ظ�eLinkInitAck�Ĳ���*/
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eSUCC;
                linkInitAck.linkHandle= pSctpSrvCb->linkHandle;
            }
            else
            {
                /*�ظ���·ȷ��ʧ��*/
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkInitProc()->add the sctp server cb to array failed!");
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eFAIL;
            }
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        break;

    default:
        /*�ظ���·ȷ��ʧ��*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkInitProc()->not support link type!");
        linkInitAck.appHandle = pLinkInit->appHandle;
        linkInitAck.lnitAckResult = eFAIL;
        break;
    }

    /*�ظ�initAck ��Ϣ*/
    ret = NTL_msgToUser(&linkInitAck,&(pMsg->datasrc),sizeof(t_LINKINITACK),eSctpInitAck);

    /*�ظ���Ϣʧ�ܣ���Ԫ�ؽ�����ʹ�ã�Ӧ�����*/
    if((ret != XSUCC) && (linkInitAck.lnitAckResult == eSUCC))
    {
        /*�����Դ*/
        SCTP_DeleteCB(pLinkInit->linkType, linkIndex);        
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
������:SCTP_linkStarttProc
����:  ������·������Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_linkStartProc(t_XOSCOMMHEAD* pMsg)
{
    t_LINKSTART *pLinkStart = XNULL;
    t_SCTPCLISTART *pSctpCliStart = XNULL;
    t_SCCB *pSctpCb = XNULL;
    e_LINKTYPE linkType;
    t_SCTPSTARTACK startAck;
    XS32 ret = 0;
    XU16 stream = 0;
    XCHAR szLinkStateName[TEMP_STRING_LEN] = {0};

    t_SCTPSERSTART *pSctpServStart = NULL;
    t_SSCB *pServCb = NULL;
    XS32 instance;
    XU32 i;
    XU8 localAddrList[SCTP_ADDR_NUM][SCTP_MAX_IP_LEN];
    SCTP_InstanceParameters instanceParameters;
    XU8 destinationAddress[SCTP_MAX_IP_LEN];
    XS32 assocID;
    SCTP_ulpCallbacks echoUlp;

    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStarttProc()->pMsg invalid!");
        return XERROR;
    }
    pLinkStart = (t_LINKSTART*)(pMsg->message);
    if(!pLinkStart)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStarttProc()->pLinkStart invalid!");
        return XERROR;
    }

    /*���е�������Ϣ(elinkInit ����)��Ҫ��֤��·�������Ч�ԣ�
    �Է�ֹ������޸ĵ�����������*/
    if(!NTL_isValidLinkH(pLinkStart->linkHandle))
    {
        /*��·�����Ч����Ϣ��ֱ�Ӷ���*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStarttProc()->FID[%d] linkHandle[%d] invalid!", pMsg->datasrc.FID,
                                     pLinkStart->linkHandle);
        return XERROR;
    }
    echoUlp.dataArriveNotif= NULL;
    echoUlp.sendFailureNotif          = NULL;
    echoUlp.networkStatusChangeNotif  = NULL;
    echoUlp.communicationUpNotif      = NULL;
    echoUlp.communicationLostNotif    = NULL;
    echoUlp.communicationErrorNotif   = NULL;
    echoUlp.restartNotif              = NULL;
    echoUlp.shutdownCompleteNotif     = NULL;
    echoUlp.peerShutdownReceivedNotif = NULL;

    /*��ȡ��·����*/
    linkType = NTL_getLinkType(pLinkStart->linkHandle);
    XOS_MemSet(&startAck,0,sizeof(t_SCTPSTARTACK));
    switch (linkType)
    {
    case eSCTPClient:
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_linkStartProc()->start a sctp client!");
        pSctpCliStart = &(pLinkStart->linkStart.sctpClientStart);

        /*��ȡsctp client  ���ƿ�*/
        pSctpCb = (t_SCCB*)XNULLP;
        
        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
        pSctpCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
        
        if(pSctpCb == XNULLP)
        {
            /*���ִ�������أ�û�а취�ظ�startAck ��Ϣ�����Խ���Ϣ����*/
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->get the sctp client control block failed!");
            return XERROR;
        }

        /*�����·��״̬�����Ƿ�Ϊ�ظ�����*/
        if(pSctpCb->linkState != eStateInited)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] the sctp client link state [%s] is wrong!",
                                          pSctpCb->linkUser.FID, NTL_getLinkStateName(pSctpCb->linkState, szLinkStateName, sizeof(szLinkStateName)-1));
            /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
            startAck.appHandle = pSctpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
        }

        if(pSctpCliStart->streamNum >= DEFAULT_SCTP_STREAM && pSctpCliStart->streamNum <= SCTP_MAX_STREAM)
        {
            pSctpCb->maxStream = pSctpCliStart->streamNum;
        }
        else
        {
            pSctpCb->maxStream = DEFAULT_SCTP_STREAM;
        }
        
        /*start the sctp client*/
        for(i = 0;i < pSctpCliStart->myAddr.ipNum;i++)
        {
            XOS_IptoStr(pSctpCliStart->myAddr.ip[i], (XCHAR *)localAddrList[i]);
        }
        XOS_IptoStr(pSctpCliStart->peerAddr.ip[0], (XCHAR *)destinationAddress);
        instance = xsctp_registerInstance(pSctpCliStart->myAddr.port,
                              SCTP_MAX_STREAM,pSctpCb->maxStream, 
                              pSctpCliStart->myAddr.ipNum, localAddrList,
                              echoUlp);
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_linkStartProc() port:%d,stream:%d,ipnum:%d instace:%d!",
            pSctpCliStart->myAddr.port,
            pSctpCb->maxStream,
            pSctpCliStart->myAddr.ipNum,
            instance);
        if( instance > 0)
        {
            XOS_MemCpy(&(pSctpCb->myAddr), &(pSctpCliStart->myAddr), sizeof(t_SCTPIPADDR));
            XOS_MemCpy(&(pSctpCb->peerAddr), &(pSctpCliStart->peerAddr), sizeof(t_SCTPIPADDR));
            
            xsctp_getAssocDefaults((unsigned short)instance, &instanceParameters);
            instanceParameters.maxSendQueue = 0;
            instanceParameters.ipTos = 0x10;
            xsctp_setAssocDefaults((XU16)instance, &instanceParameters);
            
            assocID = (XS32)xsctp_associate((XU32)instance, pSctpCb->maxStream, destinationAddress, pSctpCliStart->peerAddr.port, NULL);
            XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_linkStartProc() stream:%d,peer:%s,port:%d,assocID:%d!",
                pSctpCb->maxStream,
                destinationAddress,
                pSctpCliStart->peerAddr.port,
                assocID);
            if( assocID <= 0)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] start sctp client association failed!",pSctpCb->linkUser.FID);
                /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
                startAck.appHandle = pSctpCb->userHandle;
                startAck.linkStartResult = eFAIL;
                XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
                break;
            }
            XOS_MemCpy(&(pSctpCb->myAddr), &(pSctpCliStart->myAddr), sizeof(t_SCTPIPADDR));
            pSctpCb->instance = (XU16)instance;
            pSctpCb->assocID = (XU32)assocID;
            pSctpCb->linkState = eStateConnecting;
            
        }
        else
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] register sctp client failed!",pSctpCb->linkUser.FID);
            /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
            startAck.appHandle = pSctpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
        }

        /*��дstartAck �ظ���Ϣ,���ؽ��Ϊ�ɹ�*/
        startAck.appHandle = pSctpCb->userHandle;
        XOS_MemCpy(&(startAck.localAddr), &(pSctpCb->myAddr), sizeof(t_SCTPIPADDR));
        startAck.linkStartResult = eBlockWait;
        
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        break;

    case eSCTPServer:
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_linkStartProc()->start a sctp server!");
        /*sctp server �������Ƚϸ��ӣ�����һ����������*/
        pSctpServStart = &(pLinkStart->linkStart.sctpServerStart);

        /*��ȡsctp server ���ƿ�*/
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
        
        if(pServCb == XNULLP)
        {
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->get sctp serv control block failed!");
            return XERROR;
        }

        /*�����·״̬����ز���*/
        if((pServCb->linkState != eStateInited )
            ||(pSctpServStart->allownClients == 0)
            ||(pSctpServStart->allownClients > (XU32)((g_sctpCb.genCfg.maxSctpServLink)*SCTP_CLIENTS_PER_SERV)))
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->link state [%s]  error or allownClients[%d] error!",NTL_getLinkStateName(pServCb->linkState,szLinkStateName,sizeof(szLinkStateName)-1),pSctpServStart->allownClients);
            /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
            startAck.appHandle = pServCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            break;
        }

        if(pSctpServStart->streamNum > DEFAULT_SCTP_STREAM && pSctpServStart->streamNum <= SCTP_MAX_STREAM)
        {
            stream = pSctpServStart->streamNum;
        }
        else
        {
            stream = DEFAULT_SCTP_STREAM;
        }
        /* set up the "server" */
        for(i = 0;i < pSctpServStart->myAddr.ipNum;i++)
        {
            XOS_IptoStr(pSctpServStart->myAddr.ip[i], (XCHAR *)localAddrList[i]);
        }
        instance = xsctp_registerInstance(pSctpServStart->myAddr.port,
                              SCTP_MAX_STREAM,stream, 
                              pSctpServStart->myAddr.ipNum, localAddrList,
                              echoUlp);
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_linkStartProc() instance:%d!",instance);
        /* run the event handler forever */
        if( instance > 0)
        {
            /*��д����*/
            pServCb->maxCliNum = pSctpServStart->allownClients;
            pServCb->usageNum = 0;
            pServCb->linkState = eStateListening;
            pServCb->pLatestCli = (t_SSCLI*)XNULLP;
            pServCb->maxStream = (XU16)stream;
            pServCb->hbInterval = pSctpServStart->hbInterval;
            pServCb->instance = (XU16)instance;
            /*��д�������*/
            startAck.appHandle = pServCb->userHandle;
            startAck.linkStartResult = eSUCC;
            XOS_MemCpy(&(pServCb->myAddr),&(pSctpServStart->myAddr), sizeof(t_SCTPIPADDR));
            XOS_MemCpy(&(startAck.localAddr),&(pServCb->myAddr),sizeof(t_SCTPIPADDR));

            startAck.appHandle = pServCb->userHandle;
            startAck.linkStartResult = eSUCC;
        }
        else
        {
            startAck.appHandle = pServCb->userHandle;
            startAck.linkStartResult = eFAIL;
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        break;

    default:
        XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_linkStartProc()->not support type!");
        return XERROR;
    }

    /*����startAck ��Ϣ���ϲ�*/
    ret = NTL_msgToUser(&startAck,&(pMsg->datasrc),sizeof(t_SCTPSTARTACK),eSctpStartAck);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_linkStartProc()->send  msg startAck to user failed!");
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
������:SCTP_linkReleaseProc
����:  ������·�ͷ���Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_linkReleaseProc(t_XOSCOMMHEAD* pMsg)
{
    t_LINKRELEASE *pLinkRelease = NULL;
    t_SSCB *pServCb = NULL;
    e_LINKTYPE linkType;
    XS32 linkIndex = 0;

    if(NULL == pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_linkReleaseProc()->pMsg is null!");
        return XERROR;
    }    

    pLinkRelease = (t_LINKRELEASE*)(pMsg->message);
    
    if(NULL == pLinkRelease)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_linkReleaseProc()->pLinkRelease is null!");
        return XERROR;
    }

    if(!NTL_isValidLinkH(pLinkRelease->linkHandle))
    {
        /*��·�����Ч����Ϣ��ֱ�Ӷ���*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkReleaseProc()->linkHandle invalid!");
        return XERROR;
    }
    /*��ȡ��·����*/
    linkType = NTL_getLinkType(pLinkRelease->linkHandle);
    linkIndex = NTL_getLinkIndex(pLinkRelease->linkHandle);
    switch (linkType)
    {
    case eSCTPClient:
        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));        
        XOS_ArrayDeleteByPos(g_sctpCb.sctpClientLinkH,linkIndex);
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        break;

    case eSCTPServer:
        pServCb = (t_SSCB*)XNULLP;
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,linkIndex);
        
        if(pServCb != XNULLP)
        {
            /*��ɾ�����н���Ŀͻ�*/
            XOS_MutexLock(&(g_sctpCb.hashMutex));
            if( pServCb->pLatestCli != XNULLP)
            {
                while(pServCb->pLatestCli != XNULLP)
                {
                    SCTP_closeTsCli(pServCb->pLatestCli);
                    if(pServCb->usageNum == 0)
                    {
                        break;
                    }
                }
            }
            XOS_MutexUnlock(&(g_sctpCb.hashMutex));

            /*�����server��Ӧ�Ŀ��ƿ�*/
            XOS_ArrayDeleteByPos(g_sctpCb.sctpServerLinkH,linkIndex);
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        
        break;

    case ePCI:
        break;

    default:
        break;
    }

    return XSUCC;
}

/************************************************************************
������:SCTP_dataReqProcProc
����: �����ݷ��͵�����
����: pDataReq  �����ݷ��͵�ָ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 SCTP_dataReqProc(t_XOSCOMMHEAD *pMsg)
{
    XS32  ret = 0;
    XS32  linkIndex = 0;
    XBOOL errorFlag = XFALSE;
    t_SCTPDATAREQ *pDataReq = NULL;
    t_SENDSCTPERROR sendError;
    t_SCCB *pCliCb = XNULL;
    t_SSCB *pSctpSrvCb = XNULL;
    t_SSCLI *pTsClient = XNULL;
    t_LINKCLOSEIND closeInd;

#ifdef INPUT_PAR_CHECK
    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_dataReqProc()->input param pMsg is Null!");
        return XERROR;        
    }

    pDataReq = (t_SCTPDATAREQ*)(pMsg->message);
    if(XNULLP == pDataReq )
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_dataReqProc()->input param pMsg is Null!");
        return XERROR;
    }
    if( XNULLP == pDataReq->pData )
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_dataReqProc()-> pMsg data is Null!");
        return XERROR;
    }
#endif

    XOS_MemSet(&sendError,0,sizeof(t_SENDSCTPERROR));
    XOS_MemCpy(&(sendError.peerIp), &(pDataReq->dstAddr), sizeof(t_IPADDR));
    /*������֤��·�������Ч��*/
    if(!NTL_isValidLinkH(pDataReq->linkHandle))
    {/*�����Ϣ����ָ�벻Ϊ�գ�Ӧ���ͷ�����*/
        if(XNULLP != pDataReq->pData)
        {
            XOS_MemFree(FID_NTL,(XVOID*)(pDataReq->pData));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc()->linkHandle is invalid ,free the data pointer!");
        }
        return XERROR;
    }
    /*���ʹ���ı�־��false*/
    errorFlag = XFALSE;
    linkIndex = NTL_getLinkIndex(pDataReq->linkHandle);
    switch(NTL_getLinkType( pDataReq->linkHandle))
    {
    case eSCTPClient:
        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
        pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,linkIndex);
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        if(pCliCb == XNULLP)
        {
            /*�ڲ��������Ӧ�ø澯*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc()->can't get the sctp client link control block data,msg from FID [%d] to dest ip[0x%x]\r\n",pMsg->datasrc.FID,pDataReq->dstAddr.ip);

            errorFlag = XTRUE;
            closeInd.appHandle = (HAPPUSER)0x00000000;
            closeInd.closeReason = eNetError;
            ret = NTL_msgToUser((XVOID*)&closeInd,&(pMsg->datasrc),sizeof(t_LINKCLOSEIND),eSctpStopInd);
            if ( ret != XSUCC )
            {
                XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc()-> send msg closeind to user failed!");
            }

            /*clean up */
            XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
            return XERROR;
        }
        /*�ȼ����·��״̬*/
        if (pCliCb->linkState != eStateConnected)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorLinkState;
            sendError.userHandle = pCliCb->userHandle;
            break;
        }
        
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_dataReqProc() stream:%d,maxstream:%d!",pDataReq->attr.stream , pCliCb->maxStream);
        if(pDataReq->attr.stream >= pCliCb->maxStream)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorStreamNum;
            sendError.userHandle = pCliCb->userHandle;
            break;
        }
        /*��������*/
        ret = sctp_send(pCliCb->assocID,
              pDataReq->attr.stream,
              (XU8 *)pDataReq->pData, pDataReq->msgLenth,
              pDataReq->attr.ppid,
              SCTP_USE_PRIMARY, &(pDataReq->attr.context), 0, 0, SCTP_BUNDLING_DISABLED);
        if(ret != XSUCC && g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc sctp client send data to server failed!");
        }
        if(pDataReq->pData!= XNULLP)
        {
            XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
        }
        return XSUCC;

    case eSCTPServer:
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        pSctpSrvCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,linkIndex);
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        
        if(pSctpSrvCb == XNULLP)
        { /*�ڲ��������Ӧ�ø澯*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc()->can't get the sctp server link control block data !msg from FID [%d] to dest ip[0x%x] \r\n",pMsg->datasrc.FID,pDataReq->dstAddr.ip);

            errorFlag = XTRUE;
            closeInd.appHandle = (HAPPUSER)0x00000000;
            closeInd.closeReason = eNetError;
            ret = NTL_msgToUser((XVOID*)&closeInd,&(pMsg->datasrc),sizeof(t_LINKCLOSEIND),eSctpStopInd);
            if ( ret != XSUCC )
            {
                XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc()-> send msg closeind to user failed!");
            }

            /*clean up */
            XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
            return XERROR;
        }

        /*���Ҷ�Ӧ�Ŀͻ���cb*/
        pTsClient = SCTP_findClient(pSctpSrvCb, &(pDataReq->dstAddr));
        if(XNULLP == pTsClient )
        {
            /*û�ҵ���Ӧ�Ŀͻ���*/
            errorFlag = XTRUE;
            /*�п��������ӵĿͻ��˹ر�*/
            sendError.errorReson = eErrorDstAddr;
            sendError.userHandle = pSctpSrvCb->userHandle;
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc(),get sctp client ip[0x%x],port[%d] ctrlBlock failed!",pDataReq->dstAddr.ip,pDataReq->dstAddr.port);
            break;
        }
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_dataReqProc() stream:%d,maxstream:%d!",pDataReq->attr.stream , pSctpSrvCb->maxStream);
        if(pDataReq->attr.stream >= pSctpSrvCb->maxStream)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorStreamNum;
            sendError.userHandle = pSctpSrvCb->userHandle;
            break;
        }

        /*��������*/
        ret = sctp_send(pTsClient->assocID,
              pDataReq->attr.stream,
              (XU8 *)pDataReq->pData, pDataReq->msgLenth,
              pDataReq->attr.ppid,
              SCTP_USE_PRIMARY, &(pDataReq->attr.context), 0, 0, SCTP_BUNDLING_DISABLED);
        if ( ret != XSUCC && g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc sctp server call SCTP_dataReqProctpktProc failed!");
        }
        if(pDataReq->pData!= XNULLP)
        {
            XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
        }
        return XSUCC;

    default:
        break;
    }

    /*���͹����г���,����error send��Ϣ���ϲ�*/
    if(errorFlag)
    {
        /*�رպ������ϲ㷢�ʹ�����Ϣ*/
        XOS_Trace(MD(FID_NTL,PL_INFO),"srdFID[%d] to destFId[%d] with msgid[%d] and pri[%d]",
        pMsg->datasrc.FID,pMsg->datadest.FID,pMsg->msgID,pMsg->prio);
        ret = NTL_msgToUser((XVOID*)&sendError,&(pMsg->datasrc),sizeof(t_SENDSCTPERROR),eSctpErrorSend);
        if ( ret != XSUCC )
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqProc()-> send msg sendError to user failed!");
        }
    }

    /*�ͷ�����*/
    if(pDataReq->pData!= XNULLP)
    {
        XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
    }

    return XSUCC;
}


/************************************************************************
������:SCTP_channel_find_function
����: �����û���������ͨ���Ƿ��ѷ���,����ѷ��䣬������Ϊinit
���:
����: �ɹ�����XTRUE,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XBOOL SCTP_channel_find_function(XOS_ArrayElement element1, XVOID *param)
{
    t_SCCB *pCliCb = XNULL;
    t_SCCB *pSctpSrvCb = XNULL;
    
    t_Link_Index *trap_target2 = (t_Link_Index *)param;
    
    if(XNULL == trap_target2)
    {
        return XFALSE;
    }

    switch(trap_target2->linkType)
    {
        case eSCTPClient:
            pCliCb = (t_SCCB *)element1;
            if(XNULL == pCliCb)
            {
                return XFALSE;
            }
            if(pCliCb->linkUser.FID == trap_target2->linkUser->FID &&
                pCliCb->userHandle == trap_target2->userHandle)
            {
                return XTRUE;
            }        
            break;
        case eSCTPServer:
            pSctpSrvCb = (t_SCCB *)element1;
            if(XNULL == pSctpSrvCb)
            {
                return XFALSE;
            }
            if(pSctpSrvCb->linkUser.FID == trap_target2->linkUser->FID &&
                pSctpSrvCb->userHandle == trap_target2->userHandle)
            {
                return XTRUE;
            }
            break;
        default:
            return XFALSE;
    }        

    return XFALSE;
}

/************************************************************************
������:SCTP_ResetLinkByReapply
����:  ���ظ�������·��ֹͣԭ������·
����:  pCloseReq ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_ResetLinkByReapply(t_LINKCLOSEREQ* pCloseReq)
{    
    e_LINKTYPE linkType;
    t_SCCB *pCliCb = NULL;
    t_SSCB *pServCb = NULL;

    if(NULL == pCloseReq)
    {
         XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ResetLinkByReapply()->pCloseReq invalid!");
        return XERROR;
    }

    /*���е���Ϣ��Ҫ������ĵ���Ч��*/
    if(!NTL_isValidLinkH(pCloseReq->linkHandle))
    {
        /*��·�����Ч����Ϣ��ֱ�ӷ���*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ResetLinkByReapply()->linkHandle invalid!");
        return XERROR;
    }
    /*��ȡ��·����*/
    linkType = NTL_getLinkType(pCloseReq->linkHandle);

    switch(linkType)
    {
    case eSCTPClient:
        /*��ȡsctp ���ƿ�*/
        pCliCb = (t_SCCB*)XNULLP;
        pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        if(XNULLP == pCliCb)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ResetLinkByReapply()->get the sctp client control block failed!");
            return XERROR;
        }        

        /*�ر���·*/
        
        /*�رպ��óɳ�ʼ��״̬*/
        memset(pCliCb->peerAddr.ip,0,sizeof(pCliCb->peerAddr.ip[0]) * SCTP_ADDR_NUM);
        pCliCb->peerAddr.port = 0;
        pCliCb->peerAddr.ipNum = 0;
        memset(pCliCb->myAddr.ip,0,sizeof(pCliCb->myAddr.ip[0]) * SCTP_ADDR_NUM);
        pCliCb->myAddr.port = 0;
        pCliCb->myAddr.ipNum = 0;
        pCliCb->linkState = eStateInited;


        break;

    case eSCTPServer:
        pServCb = (t_SSCB*)XNULLP;
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        if(pServCb == XNULLP)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_ResetLinkByReapply()->get the sctp server control block failed!");
            return XERROR;
        }

        /*�ر�sctpserver*/
        SCTP_CloseServerSocket(pServCb);        

        /*�ı���Ӧ��cb ����,��ʼ��*/
        pServCb->linkState = eStateInited;
        pServCb->maxCliNum = 0;
        pServCb->usageNum = 0;
        pServCb->pLatestCli = (t_SSCLI*)XNULLP;
        memset(pServCb->myAddr.ip,0,sizeof(pServCb->myAddr.ip[0]) * SCTP_ADDR_NUM);
        pServCb->myAddr.port = 0;
        pServCb->myAddr.ipNum = 0;
        break;

    default:
        return XERROR;
    }

    return XSUCC;
    
}

/************************************************************************
������:SCTP_CloseServerSocket
����:  �ر�sctpserver
����:  pServCb ��sctpserver���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: sctpserver��Ҫ���÷���ȫ�ֱ���
************************************************************************/
void SCTP_CloseServerSocket(t_SSCB *pServCb)
{
    if(NULL == pServCb)
    {
        return ;
    }

    /*���ȹر����н���Ŀͻ���*/
    XOS_MutexLock(&(g_sctpCb.hashMutex));
    while(pServCb->pLatestCli != XNULLP)
    {
        SCTP_closeTsCli(pServCb->pLatestCli);
        if(pServCb->usageNum == 0)
        {
            break;
        }
    }
    XOS_MutexUnlock(&(g_sctpCb.hashMutex));

    /*�ر�fd*/

    return;
}

/************************************************************************
������:SCTP_ResetLinkByReapplyEntry
����:  ���ظ�������·��ֹͣԭ������·
����:  e_LINKTYPE linkType      -��������
        HAPPUSER *pUserHandle   -���ӵ��û����
        XS32 *pnRtnFind         -���ƿ������ֵ
���:   t_LINKINITACK *linkInitAck  -������
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_ResetLinkByReapplyEntry(e_LINKTYPE linkType, t_LINKINITACK *linkInitAck, HAPPUSER *pUserHandle, XS32 *pnRtnFind)
{
    t_LINKCLOSEREQ stopReq;

    if(NULL == linkInitAck ||  NULL == pUserHandle || NULL == pnRtnFind)
    {
         XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ResetLinkByReapplyEntry()->add para is error!");
         return XERROR;
    }
    
    stopReq.cliAddr.ip = 0;
    stopReq.cliAddr.port = 0;
    stopReq.linkHandle = (HLINKHANDLE)NTL_buildLinkH(linkType,(XU16)(*pnRtnFind));
    
    SCTP_ResetLinkByReapply(&stopReq);    
       
    linkInitAck->linkHandle = stopReq.linkHandle;
    linkInitAck->appHandle = *pUserHandle;
    linkInitAck->lnitAckResult = eSUCC;

return XSUCC;
}

/************************************************************************
������:SCTP_RestartLink
����:  ��������sctp����
����:   t_SCCB* sctpCliCb
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_RestartLink(t_SCCB* sctpCliCb)
{
    t_LINKSTART startLnk;
    XS32 ret = 0;
    t_XOSCOMMHEAD *pMsg = XNULL;
    if(XNULL == sctpCliCb)
    {
        return XERROR;
    }
    
    XOS_MemSet((char*)&startLnk, 0x0, sizeof(t_LINKSTART));
    startLnk.linkHandle = sctpCliCb->linkHandle;
    XOS_MemCpy(&(startLnk.linkStart.sctpClientStart.myAddr), &(sctpCliCb->myAddr), sizeof(t_SCTPIPADDR));
    XOS_MemCpy(&(startLnk.linkStart.sctpClientStart.peerAddr), &(sctpCliCb->peerAddr), sizeof(t_SCTPIPADDR));
    /*������Ϣ�ڴ�*/
    pMsg = (t_XOSCOMMHEAD*)XNULL;
    pMsg = XOS_MsgMemMalloc(FID_NTL,sizeof(t_LINKSTART));
    if(pMsg == XNULL)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"ERROR: link msg to ntl: can not malloc memory.");
        return XERROR;
    }

    /*��д��Ϣ����*/
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datasrc.FID = FID_NTL;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = FID_NTL;
    pMsg->prio = eFastMsgPrio;
    pMsg->msgID = eLinkStart;
    XOS_MemCpy(pMsg->message, &startLnk, sizeof(t_LINKSTART));

    /*��������*/
    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        /*����ָʾ��Ϣ��Ӧ�����ͷ��յ�����*/
        XOS_MsgMemFree(FID_NTL, pMsg);
        XOS_Trace(FILI, FID_NTL, PL_ERR, "ERROR: NTL_RestartSctpLink send msg failed.");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
������:SCTP_CliCfgShow
����: ��ʾsctp client ������Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
************************************************************************/
XVOID SCTP_CliCfgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 nIndex = -1;
    XS32 i = 0;
    XS32 j = 0;
    XS32 link_index = -1;
    t_SCCB *sctpCliCb = NULL;

    XCHAR* state[] =
    {
        "Null",
        "Inited",
        "Started",
        "Cncting",
        "Cncted",
        "Listen",
        "Wait",
        "Max"
    };

    if(!pCliEnv || !ppArgv)
    {
        return;
    }

    /*sctp client cfg list start */
    XOS_CliExtPrintf(pCliEnv,
        "sctp client  config list \r\n-------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-22s%-10s%-10s%-6s%-10s%-6s\r\n",
        "index",
        "userHander",
        "userFid",
        "linkstate",
        "myIP",
        "mPort",
        "peerIP",
        "pPort"
        );
    if((3==siArgc))
    {
        link_index = atoi(ppArgv[1]);
    }
    
    nIndex = XOS_ArrayGetFirstPos(g_sctpCb.sctpClientLinkH);
    for(j=0,i=nIndex; i>=0; i=XOS_ArrayGetNextPos(g_sctpCb.sctpClientLinkH,i))
    {
        sctpCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,i);
        if(sctpCliCb)
        {
            XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-22s%-10s%-10x%-6d%-10x%-6d\r\n",
                i,
                sctpCliCb->userHandle,
                XOS_getFidName(sctpCliCb->linkUser.FID),
                state[sctpCliCb->linkState],
                sctpCliCb->myAddr.ip[0],
                sctpCliCb->myAddr.port,
                sctpCliCb->peerAddr.ip[0],
                sctpCliCb->peerAddr.port
                );
            if(link_index == i)
            {
                if((3==siArgc))
                {
                    if(0 == XOS_StrCmp(ppArgv[2],"restart"))
                    {
                        XOS_CliExtPrintf(pCliEnv,"restart link %i\r\n",i);
                        SCTP_RestartLink(sctpCliCb);
                    }
                }
            }
        }
        j++;
        NTL_SHOW_CMD_CTL(j);
    }

    /*end of sctp client list */
    XOS_CliExtPrintf(pCliEnv,
        "-------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n",j);
    return ;
}

/************************************************************************
������:SCTP_ServCfgShow
����: ��ʾsctp server ������Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
************************************************************************/
XVOID SCTP_ServCfgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 i = 0, j = 0, sum = 0;
    XS32 nIndex = 0;
    t_SSCB *sctpServCb = NULL;
    t_SSCLI *pSctpServCli = NULL;
    t_IPADDR *pPeerAddr = NULL;
    XCHAR* state[] =
    {
        "Null",
        "Inited",
        "Started",
        "Cncting",
        "Cncted",
        "Listen",
        "Wait",
        "Max"
    };    

    XOS_UNUSED(siArgc);

    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    
    XOS_CliExtPrintf(pCliEnv,
        "sctp server configuration list\r\n-------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-22s%-10s%-10s%-6s%-6s%-6s\r\n",
        "index",
        "userHander",
        "userFid",
        "linkstate",
        "myIP",
        "mPort",
        "mClis",
        "cClis"
        );

    nIndex = XOS_ArrayGetFirstPos(g_sctpCb.sctpServerLinkH);
    for(sum=0,i=nIndex; i>=0; i=XOS_ArrayGetNextPos(g_sctpCb.sctpServerLinkH,i))
    {
        sctpServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,i);
        if(sctpServCb)
        {
            XOS_CliExtPrintf(pCliEnv,
            "%-6d%-12x%-22s%-10s%-10x%-6d%-6d%-6d\r\n",
            i,
            sctpServCb->userHandle,
            XOS_getFidName(sctpServCb->linkUser.FID),
            state[sctpServCb->linkState],
            sctpServCb->myAddr.ip[0],
            sctpServCb->myAddr.port,
            sctpServCb->maxCliNum,
            sctpServCb->usageNum);
            if(sctpServCb->usageNum > 0)
            {
                XOS_CliExtPrintf(pCliEnv,
                    "      ----------------------------\r\n");
                XOS_CliExtPrintf(pCliEnv,
                    "      %-12s%-10s%-8s\r\n",
                    "clino",
                    "peerIp",
                    "pPort"
                    );
                pSctpServCli = sctpServCb->pLatestCli;
                for(j=0; j<sctpServCb->usageNum; j++)
                {
                    pPeerAddr =
                        (t_IPADDR*)XOS_HashGetKeyByElem(g_sctpCb.tSctpCliH,pSctpServCli);
                    XOS_CliExtPrintf(pCliEnv,
                        "      %-12d%-10x%-8d\r\n",
                        j,
                        pPeerAddr->ip,
                        pPeerAddr->port);

                    pSctpServCli = pSctpServCli->pPreCli;
                    if(pSctpServCli == XNULLP)
                    {
                        break;
                    }
                    NTL_SHOW_CMD_CTL(j);
                }
                XOS_CliExtPrintf(pCliEnv, "      ----------------------------\r\n");
            }
        }
        sum++;
        NTL_SHOW_CMD_CTL(sum);
    }

    /*end of sctp serv clients list */
    XOS_CliExtPrintf(pCliEnv,
        "-------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n",sum);
    return ;
}

XVOID SCTP_msgQueueShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    int curSize = 0;
    int queueSize = 0;
    
    getMsgQueueInfo(&curSize,&queueSize);
    XOS_CliExtPrintf(pCliEnv,
    "sctplib msg queue info \r\n--------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
    "%-10s%-10s\r\n",
    "maxMsgs",
    "curUsage"
    );
    XOS_CliExtPrintf(pCliEnv,
    "%-10d%-10d\r\n",
    queueSize,
    curSize
    );
    XOS_CliExtPrintf(pCliEnv,
    "--------------------------\r\n");
}
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */


