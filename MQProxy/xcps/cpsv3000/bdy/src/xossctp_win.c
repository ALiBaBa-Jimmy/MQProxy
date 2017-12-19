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
                  包含头文件
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
                模块内部全局变量
-------------------------------------------------------------------------*/
static t_SCTPGLOAB g_sctpCb;

/************************************************************************
函数名:sctp_dataArrive
功能: 数据接收的回调函数
输入:
输出:assocID - 接收数据的偶联的ID
    streamID - 接收数据的流号
    len - 接收数据的长度
    streamSN - 接收数据的流顺序号
    TSN - 接收数据的传输序列号
    protoID - 净荷协议标识符
    unordered - 接收数据是否无序，TRUE==1==unordered, FALSE==0==normal
    ulpDataPtr - 用户定义数据的指针，这里没有使用，为NULL
返回: void
说明:
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
    XCHAR *pBuf;       /*分配消息的指针*/
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

    /*上报数据到服务端*/
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
            /*发送数据到上层用户*/
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

    /*上报数据到客户端*/
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
            /*发送数据到上层用户*/
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
函数名:sctp_sendFailure
功能: 数据发送失败时的回调函数
输入:
输出:assocID - 发送数据的偶联的ID
    unsent_data - 未发送的数据的流号
    dataLength - 数据的长度
    context - 数据的上下文标识符
    ulpDataPtr - 用户定义数据的指针，这里没有使用，为NULL
返回: void
说明:
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
    /*先判断是否是服务端发送数据出错*/
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

    /*判断是否客户端发送数据出错*/
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
函数名:sctp_networkStatusChange
功能: 网络连接状态有变化时的回调函数
输入:
输出:assocID - 发送数据的偶联的ID
    destAddrIndex - 地址索引，没有使用
    newState - 变化后的状态值，0表示正常,1表示断开
    ulpDataPtr - 用户定义数据的指针，这里没有使用，为NULL
返回: void
说明:这里只处理了连接成功的情况，用于判断创建连接是否成功
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
        return;        /*除【连接成功】外的其他状态变迁*/
    }
    result = xsctp_getAssocStatus(assocID, &assocStatus);
    if( result < 0)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"sctp_networkStatusChange()-> get peer status failed!");
        return;
    }
    XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_networkStatusChange():state:%d,numOfAdd:%d,primDest:%s,sPort:%d,dPort:%d,priIndex:%d!",
        assocStatus.state,assocStatus.numberOfAddresses,assocStatus.primaryDestinationAddress,assocStatus.sourcePort,assocStatus.destPort,assocStatus.primaryAddressIndex);

    /*服务端状态变迁处理*/
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
                /*关闭新接入的sock*/
                xsctp_abort(assocID);
                XOS_Trace(MD(FID_NTL,PL_ERR),"new client's instream too small,(ip[0x%x],port[%d]) instream is %d,local outstream is %d!",
                    assocStatus.primaryDestinationAddress,assocStatus.destPort,assocStatus.inStreams,pServCb->maxStream);
                XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
                return;
            }

            /*接入客户的数量不能超过最大容许接入数量*/
            if((XU16)(pServCb->maxCliNum) < (XU16)(pServCb->usageNum +1))
            {
                /*关闭新接入的sock*/
                xsctp_abort(assocID);
                XOS_Trace(MD(FID_NTL,PL_ERR),"new client(ip[0x%x],port[%d]) is overstep maxclis %d allowned!",
                    assocStatus.primaryDestinationAddress,assocStatus.destPort,pServCb->maxCliNum);
                XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
                return;
            }
            /*将接入的客户端加到hash表中*/
            XOS_MemSet(&servCliCb,0,sizeof(t_SSCLI));
            servCliCb.assocID = assocID;
            XOS_StrtoIp((XCHAR *)assocStatus.primaryDestinationAddress, &(servCliCb.destAddr.ip));
            servCliCb.destAddr.port = assocStatus.destPort;
            /*链表也应该保护*/
            servCliCb.pServerElem = pServCb;
            servCliCb.pPreCli = pServCb->pLatestCli;
            servCliCb.pNextCli = (t_SSCLI*)XNULLP;

            /*不论对端连接多少ip过来，都取第一个ip作为hash的key*/
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

            /*构成链*/
            XOS_MutexLock(&(g_sctpCb.hashMutex));
            pServCliCb = (t_SSCLI*)XNULLP;
            pServCliCb = (t_SSCLI*)XOS_HashGetElem(g_sctpCb.tSctpCliH, pLocation);

            if(pServCb->pLatestCli != XNULLP && pServCliCb != XNULLP)
            {
                pServCb->pLatestCli->pNextCli = pServCliCb;
            }
            pServCb->pLatestCli= pServCliCb;
            pServCb->usageNum++; /*接入一个新的客户进来*/
            XOS_MutexUnlock(&(g_sctpCb.hashMutex));

            /*发送连接指示消息到上层*/
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

    /*客户端状态变迁处理*/
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
                /*关闭新接入的sock*/
                xsctp_abort(pSctpCb->assocID);
                XOS_Trace(MD(FID_NTL,PL_DBG),"sctp_networkStatusChange,new client(ip[0x%x],port[%d]) instream is %d,local outstream is %d!",
                    assocStatus.primaryDestinationAddress,assocStatus.destPort,assocStatus.inStreams,pSctpCb->maxStream);
                pSctpCb->linkState = eStateInited;
                XOS_Trace(MD(FID_NTL,PL_ERR),"sctp_networkStatusChange(),connect sctp server failed,peer instream too small!");

                /*发送启动成功消息到上层*/
                sctpStartAck.appHandle = pSctpCb->userHandle;
                sctpStartAck.linkStartResult = eInvalidInstrm;
                XOS_MemCpy(&(sctpStartAck.localAddr),&(pSctpCb->myAddr),sizeof(t_SCTPIPADDR));

                NTL_msgToUser((XVOID*)&sctpStartAck,&(pSctpCb->linkUser),sizeof(t_SCTPSTARTACK),eSctpStartAck);
                XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
                return;
            }

            /*客户端连接成功*/
            if(pSctpCb->linkState != eStateConnected)
            {
                pSctpCb->linkState = eStateConnected;
                XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_networkStatusChange(),connect sctp server successed!");

                /*发送启动成功消息到上层*/
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
函数名:sctp_communicationUp
功能: 网络连接状态有变化时的回调函数
输入:
输出:assocID - 即将创建的偶联的ID
    status - 地址索引，没有使用
    noOfDestinations - 对端的地址数
    noOfInStreams - 对端的入流数
    noOfOutStreams - 对端的出流数
    associationSupportsPRSCTP
    dummy
返回: NULL
说明:接收到第三和第四次握手报文时调用，无需处理
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
        case SCTP_COMM_UP_RECEIVED_VALID_COOKIE:  /*服务端收到cookie_echo时*/
        XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_communicationUp()-> SCTP_COMM_UP_RECEIVED_VALID_COOKIE!");
            break;

        case SCTP_COMM_UP_RECEIVED_COOKIE_ACK:  /*客户端收到cookie_ack时*/
        XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_communicationUp()-> SCTP_COMM_UP_RECEIVED_COOKIE_ACK!");
            break;
        default:
            break;
        
    }
    return NULL;
}

/************************************************************************
函数名:sctp_communicationLost
功能: 网络连接关闭或断开时的回调函数
输入:
输出:assocID - 即将创建的偶联的ID
    status - 网络关闭的状态值
    ulpDataPtr - 用户定义数据的指针，这里没有使用，为NULL
返回: void
说明:
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
    /*communicationUpNotif返回空指针，这里不需free*/
    //free((struct ulp_data *) ulpDataPtr);

    /* delete the association */
    xsctp_deleteAssociation(assocID);
    
    /*先判断是否是服务端断开连接*/
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

    /*判断是否客户端断开连接*/
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
函数名:sctp_communicationError
功能: 网络连接出错时的回调函数
输入:
输出:assocID - 出错的偶联的ID
    status - 网络错误的状态值
    dummy
返回: void
说明:平台没有处理这个回调函数
************************************************************************/
void sctp_communicationError(unsigned int assocID, unsigned short status, void* dummy)
{
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_communicationError() assocID:%d,!",assocID);
}

/************************************************************************
函数名:sctp_restart
功能: 重置网络连接时的回调函数
输入:
输出:assocID - 要重置的偶联的ID
    ulpDataPtr - 用户定义数据的指针，这里没有使用，为NULL
返回: void
说明:平台没有处理这个回调函数
************************************************************************/
void sctp_restart(unsigned int assocID, void* ulpDataPtr)
{
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_restart()! assocID:%d",assocID);
}

/************************************************************************
函数名:sctp_shutdownComplete
功能: 网络连接关闭完成时的回调函数
输入:
输出:assocID - 关闭的偶联的ID
    ulpDataPtr - 用户定义数据的指针，这里没有使用，为NULL
返回: void
说明:
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

    /*先判断是否是服务端关闭*/
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

    /*判断是否客户端关闭*/
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
函数名:sctp_shutdownReceived
功能: 接收到对端发送的关闭三次分手第一个报文时的回调函数
输入:
输出:assocID - 关闭的偶联的ID
    ulpDataPtr - 用户定义数据的指针，这里没有使用，为NULL
返回: void
说明:平台没有处理这个回调函数
************************************************************************/
void sctp_shutdownReceived(unsigned int assocID, void* ulpDataPtr)
{
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp_shutdownReceived() assocID:%d!",assocID);
}

/************************************************************************
函数名:SCTP_init
功能:  sctp动态库初始化函数
输入:
输出:
返回: 
说明:
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
函数名:event_loop
功能:  sctp事件轮询线程
输入:
输出:
返回: 
说明:客户端与服务端都通过这个轮询上报事件
************************************************************************/
XVOID  event_loop()
{
    while(1)
    {
        sctp_eventLoop();
    }
}

/************************************************************************
函数名:sctp_msg
功能:  sctp消息处理线程
输入:
输出:
返回: 
说明:sctplib库发送的消息都通过这个线程进行处理，然后调用相应的处理函数
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
函数名:SCTP_findClient
功能:  查找sctp 连接的客户
输入:
pTserverCb －server cb的指针
pClientAddr  － 接进来客户的地址指针
输出:
返回: 成功返回控制块指针,否则返回xnullp
说明:
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
函数名:SCTP_closeTsCli
功能: 关闭一个sctp server 接入的客户端
输入:pSctpServCli －server client 的控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 SCTP_closeTsCli(t_SSCLI* pSctpServCli)
{
    if(pSctpServCli == XNULLP)
    {
        return XERROR;
    }

    xsctp_abort(pSctpServCli->assocID);  
    xsctp_deleteAssociation(pSctpServCli->assocID);
    
    /*从链表中断开,修改server cb 的数据*/
    (pSctpServCli->pServerElem)->usageNum--;
    /*是链表尾部的节点*/
    if(pSctpServCli->pNextCli == XNULLP)
    {
        (pSctpServCli->pServerElem)->pLatestCli = pSctpServCli->pPreCli;
    }
    else
    {
        pSctpServCli->pNextCli->pPreCli = pSctpServCli->pPreCli;
    }
    
    /*不是头节点*/
    if( pSctpServCli->pPreCli != XNULLP)
    {
        pSctpServCli->pPreCli->pNextCli = pSctpServCli->pNextCli;
    }
    
    /*从hash 中删除*/
    XOS_HashDelByElem(g_sctpCb.tSctpCliH, pSctpServCli);
    return XSUCC;
}

/************************************************************************
函数名:SCTP_closeReqProc
功能:  处理链路关闭请求消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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

    /*下行的消息都要检查句柄的的有效性*/
    if(!NTL_isValidLinkH(pCloseReq->linkHandle))
    {
        /*链路句柄无效的消息，直接丢弃*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_closeReqProc()->FID[%d] linkHandle invalid!", pMsg->datasrc.FID);
        return XERROR;
    }
    /*获取链路类型*/
    linkType = NTL_getLinkType(pCloseReq->linkHandle);

    switch(linkType)
    {
    case eSCTPClient:
        /*获取sctp 控制块*/
        pCliCb = (t_SCCB*)XNULLP;

        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));        
        pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));

        if(pCliCb == XNULLP)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_closeReqProc()->get the sctp client control block failed!");
            return XERROR;
        }

        xsctp_abort(pCliCb->assocID);
        xsctp_deleteAssociation(pCliCb->assocID);
        xsctp_unregisterInstance(pCliCb->instance);
        pCliCb->linkState = eStateInited;

        /*将对端地址置空*/
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
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_closeReqProc()->get the sctp server control block failed!");
            return XERROR;
        }
        if(pServCb->linkState != eStateListening)
        {
            /*链路状态不对,消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_closeReqProc()->sctp serv  link state is wrong!");
            ret = (pServCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            return ret;
        }           
       
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        if(0x00 == pCloseReq->cliAddr.ip && 0x00 == pCloseReq->cliAddr.port)
        {
            XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
            /*首先关闭所有接入的客户端*/     
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

            /*关闭fd*/
            ret = xsctp_unregisterInstance(pServCb->instance);
            /*改变相应的cb 数据,初始化*/
            pServCb->linkState = eStateInited;
            pServCb->maxCliNum = 0;
            pServCb->usageNum = 0;
            pServCb->pLatestCli = (t_SSCLI*)XNULLP;
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        }
        else
        {   
            XOS_MutexLock(&(g_sctpCb.hashMutex));

            /*查找指定的客户端控制块*/
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
函数名:SCTP_closeReqForRelease
功能:  处理链路释放请求消息--只能由SCTP_linkReleaseProc调用
输入:  pMsg －消息指针
输出:
返回:  成功返回XSUCC,否则返回XERROR
说明:  此函数与SCTP_closeReqProc函数在sctpserver上处理不同
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

    /*下行的消息都要检查句柄的的有效性*/
    if(!NTL_isValidLinkH(pReleaseReq->linkHandle))
    {
        /*链路句柄无效的消息，直接丢弃*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ReleaseLink()->FID[%d] linkHandle invalid!", pMsg->datasrc.FID);
        return XERROR;
    }
    /*获取链路类型*/
    linkType = NTL_getLinkType(pReleaseReq->linkHandle);

    switch(linkType)
    {
    case eSCTPClient:
        /*获取sctp 控制块*/
        pCliCb = (t_SCCB*)XNULLP;

        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));        
        pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,NTL_getLinkIndex(pReleaseReq->linkHandle));
        
        if(pCliCb == XNULLP)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_ReleaseLink()->get the sctp client control block failed!");
            return XERROR;
        }

        /*网络关闭*/
        xsctp_abort(pCliCb->assocID);
        xsctp_deleteAssociation(pCliCb->assocID);
        xsctp_unregisterInstance(pCliCb->instance);

        pCliCb->linkState = eStateInited;

        /*将对端地址置空*/
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
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_ReleaseLink()->get the sctp server control block failed!");
            return XERROR;
        }
        if(pServCb->linkState != eStateListening)
        {
            /*链路状态不对,消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_ReleaseLink()->sctp serv  link state is wrong!");
            ret = (pServCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            return ret;
        }               

        /*首先关闭所有接入的客户端*/     
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

        /*关闭链路*/
        ret = xsctp_unregisterInstance(pServCb->instance);

        /*改变相应的cb 数据,初始化*/
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
函数名:SCTP_genCfgProc
功能:  处理通用配置消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: SCTP 在收到通用配置消息后启动各子任务，完成各
子任务的初始化。
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

    /*保存配置信息*/
    XOS_MemCpy(&(g_sctpCb.genCfg),pGenCfg,sizeof(t_NTLGENCFG));

    /* sctp client  link 任务相关启动*/
    if (g_sctpCb.genCfg.maxSctpCliLink > 0 )
    {
        /*分配sctp client控制块资源*/
        g_sctpCb.sctpClientLinkH = XNULL;
        g_sctpCb.sctpClientLinkH =  XOS_ArrayConstruct(sizeof(t_SCCB), g_sctpCb.genCfg.maxSctpCliLink, "sctpClilinkH");
        if(XNULL == g_sctpCb.sctpClientLinkH)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->construct sctpCli array failed!");
            goto genCfgError;
        }
        XOS_ArraySetCompareFunc(g_sctpCb.sctpClientLinkH, SCTP_channel_find_function);

        /*sctp客户端链路控制块互斥变量*/
        ret = XOS_MutexCreate(&(g_sctpCb.sctpClientLinkMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctpclient thread[%d]  semaphore  failed!",i);
            goto genCfgError;
        }         
    }

    /*sctp server 相关的配置,sctp server目前采用单任务处理,以后可以扩展*/
    if(g_sctpCb.genCfg.maxSctpServLink > 0)
    {
        /*创建sctp server 控制块数组*/
        g_sctpCb.sctpServerLinkH = XNULL;
        g_sctpCb.sctpServerLinkH = XOS_ArrayConstruct( sizeof(t_SSCB),g_sctpCb.genCfg.maxSctpServLink,"sctpServH");
        if(XNULL == g_sctpCb.sctpServerLinkH )
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->construct sctpServ array failed!");
            goto genCfgError;
        }
 
        XOS_ArraySetCompareFunc(g_sctpCb.sctpServerLinkH, SCTP_channel_find_function);

        /*sctp服务端链路控制块互斥变量*/
        ret = XOS_MutexCreate(&(g_sctpCb.sctpServerLinkMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctpserver thread[%d]  semaphore  failed!",i);
            goto genCfgError;
        }

        /*创建接入客户端的hash*/
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

        /*初始化hash 的互斥琐*/
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
    /*释放资源*/
    XOS_ArrayDestruct(g_sctpCb.sctpClientLinkH);
    XOS_ArrayDestruct(g_sctpCb.sctpServerLinkH);
    XOS_HashDestruct(g_sctpCb.tSctpCliH);

        
    XOS_MutexDelete(&(g_sctpCb.hashMutex));

    /*清空*/
    XOS_MemSet(&g_sctpCb,0,sizeof(t_SCTPGLOAB));
    /*设置成没有初始化*/
    g_sctpCb.isGenCfg = XFALSE;
    return XERROR;
}

/************************************************************************
函数名:SCTP_DeleteCB
功能:  释放控制块
输入:  linkType －链路类型
       linkIndex--链路索引
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
函数名:SCTP_linkInitProc
功能:  处理链路初始化消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
            /*重置链路*/
            SCTP_ResetLinkByReapplyEntry(eSCTPClient, &linkInitAck, &(pLinkInit->appHandle), &nRtnFind);
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkInitProc()->add the sctp client cb was reset!");
        }
        else
        {
            /*添加到忙闲链中*/
            linkIndex = XOS_ArrayAddExt(g_sctpCb.sctpClientLinkH,(XOS_ArrayElement *) &pCliCb);
            
            if((linkIndex >= 0) && (pCliCb != XNULLP))
            {
                /*初始化控制块参数*/
                pCliCb->userHandle = pLinkInit->appHandle;
                XOS_MemCpy(&(pCliCb->linkUser),&(pMsg->datasrc),sizeof(t_XOSUSERID));
                pCliCb->linkHandle = NTL_buildLinkH(eSCTPClient,(XU16)linkIndex);
                pCliCb->linkState = eStateInited;
                memset(pCliCb->myAddr.ip,0,sizeof(pCliCb->myAddr.ip[0])*SCTP_ADDR_NUM);
                pCliCb->myAddr.port = 0;
                memset(pCliCb->peerAddr.ip,0,sizeof(pCliCb->myAddr.ip[0])*SCTP_ADDR_NUM);
                pCliCb->peerAddr.port = 0;
                
                /*回复eLinkInitAck的参数*/
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eSUCC;
                linkInitAck.linkHandle= pCliCb->linkHandle;
            }
            else
            {
                /*回复链路确认失败*/
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
            /*重置链路*/
            SCTP_ResetLinkByReapplyEntry(eSCTPServer, &linkInitAck, &(pLinkInit->appHandle), &nRtnFind);
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkInitProc()->add the sctp server cb was reset!");
        }
        else
        {
            /*添加到忙闲链中*/
            linkIndex = XOS_ArrayAddExt(g_sctpCb.sctpServerLinkH,(XOS_ArrayElement *)&pSctpSrvCb);
            
            if((linkIndex >= 0) && (pSctpSrvCb != XNULLP))
            {
                /*填写控制块参数*/
                pSctpSrvCb->userHandle = pLinkInit->appHandle;
                XOS_MemCpy(&(pSctpSrvCb->linkUser),&(pMsg->datasrc),sizeof(t_XOSUSERID));
                pSctpSrvCb->linkHandle = NTL_buildLinkH(eSCTPServer,(XU16)linkIndex);
                pSctpSrvCb->linkState = eStateInited;
                memset(pSctpSrvCb->myAddr.ip,0,sizeof(pSctpSrvCb->myAddr.ip[0]) * SCTP_ADDR_NUM);
                pSctpSrvCb->myAddr.port = 0;
                pSctpSrvCb->maxCliNum = 0;
                pSctpSrvCb->usageNum = 0;
                pSctpSrvCb->pLatestCli = NULL;

                /*回复eLinkInitAck的参数*/
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eSUCC;
                linkInitAck.linkHandle= pSctpSrvCb->linkHandle;
            }
            else
            {
                /*回复链路确认失败*/
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkInitProc()->add the sctp server cb to array failed!");
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eFAIL;
            }
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        break;

    default:
        /*回复链路确认失败*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkInitProc()->not support link type!");
        linkInitAck.appHandle = pLinkInit->appHandle;
        linkInitAck.lnitAckResult = eFAIL;
        break;
    }

    /*回复initAck 消息*/
    ret = NTL_msgToUser(&linkInitAck,&(pMsg->datasrc),sizeof(t_LINKINITACK),eSctpInitAck);

    /*回复消息失败，该元素将不再使用，应该清除*/
    if((ret != XSUCC) && (linkInitAck.lnitAckResult == eSUCC))
    {
        /*清空资源*/
        SCTP_DeleteCB(pLinkInit->linkType, linkIndex);        
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
函数名:SCTP_linkStarttProc
功能:  处理链路启动消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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

    /*所有的下行消息(elinkInit 除外)都要验证链路句柄的有效性，
    以防止误操作修改到其他的数据*/
    if(!NTL_isValidLinkH(pLinkStart->linkHandle))
    {
        /*链路句柄无效的消息，直接丢弃*/
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

    /*获取链路类型*/
    linkType = NTL_getLinkType(pLinkStart->linkHandle);
    XOS_MemSet(&startAck,0,sizeof(t_SCTPSTARTACK));
    switch (linkType)
    {
    case eSCTPClient:
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_linkStartProc()->start a sctp client!");
        pSctpCliStart = &(pLinkStart->linkStart.sctpClientStart);

        /*获取sctp client  控制块*/
        pSctpCb = (t_SCCB*)XNULLP;
        
        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
        pSctpCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
        
        if(pSctpCb == XNULLP)
        {
            /*这种错误很严重，没有办法回复startAck 消息，所以将消息丢弃*/
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->get the sctp client control block failed!");
            return XERROR;
        }

        /*检查链路的状态，看是否为重复启动*/
        if(pSctpCb->linkState != eStateInited)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] the sctp client link state [%s] is wrong!",
                                          pSctpCb->linkUser.FID, NTL_getLinkStateName(pSctpCb->linkState, szLinkStateName, sizeof(szLinkStateName)-1));
            /*填写startAck 回复消息,返回结果为efailed*/
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
                /*填写startAck 回复消息,返回结果为efailed*/
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
            /*填写startAck 回复消息,返回结果为efailed*/
            startAck.appHandle = pSctpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
        }

        /*填写startAck 回复消息,返回结果为成功*/
        startAck.appHandle = pSctpCb->userHandle;
        XOS_MemCpy(&(startAck.localAddr), &(pSctpCb->myAddr), sizeof(t_SCTPIPADDR));
        startAck.linkStartResult = eBlockWait;
        
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        break;

    case eSCTPServer:
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_linkStartProc()->start a sctp server!");
        /*sctp server 的启动比较复杂，单独一个函数处理*/
        pSctpServStart = &(pLinkStart->linkStart.sctpServerStart);

        /*获取sctp server 控制块*/
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
        
        if(pServCb == XNULLP)
        {
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->get sctp serv control block failed!");
            return XERROR;
        }

        /*检查链路状态和相关参数*/
        if((pServCb->linkState != eStateInited )
            ||(pSctpServStart->allownClients == 0)
            ||(pSctpServStart->allownClients > (XU32)((g_sctpCb.genCfg.maxSctpServLink)*SCTP_CLIENTS_PER_SERV)))
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->link state [%s]  error or allownClients[%d] error!",NTL_getLinkStateName(pServCb->linkState,szLinkStateName,sizeof(szLinkStateName)-1),pSctpServStart->allownClients);
            /*填写startAck 回复消息,返回结果为efailed*/
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
            /*填写参数*/
            pServCb->maxCliNum = pSctpServStart->allownClients;
            pServCb->usageNum = 0;
            pServCb->linkState = eStateListening;
            pServCb->pLatestCli = (t_SSCLI*)XNULLP;
            pServCb->maxStream = (XU16)stream;
            pServCb->hbInterval = pSctpServStart->hbInterval;
            pServCb->instance = (XU16)instance;
            /*填写输出参数*/
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

    /*发送startAck 消息到上层*/
    ret = NTL_msgToUser(&startAck,&(pMsg->datasrc),sizeof(t_SCTPSTARTACK),eSctpStartAck);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_linkStartProc()->send  msg startAck to user failed!");
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
函数名:SCTP_linkReleaseProc
功能:  处理链路释放消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
        /*链路句柄无效的消息，直接丢弃*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkReleaseProc()->linkHandle invalid!");
        return XERROR;
    }
    /*获取链路类型*/
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
            /*先删除所有接入的客户*/
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

            /*再清除server对应的控制块*/
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
函数名:SCTP_dataReqProcProc
功能: 将数据发送到网络
输入: pDataReq  －数据发送的指针
输出:
返回: 成功返回XOS_XSUCC,失败返回XERROR
说明:
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
    /*首先验证链路句柄的有效性*/
    if(!NTL_isValidLinkH(pDataReq->linkHandle))
    {/*如果消息内容指针不为空，应该释放内容*/
        if(XNULLP != pDataReq->pData)
        {
            XOS_MemFree(FID_NTL,(XVOID*)(pDataReq->pData));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc()->linkHandle is invalid ,free the data pointer!");
        }
        return XERROR;
    }
    /*发送错误的标志置false*/
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
            /*内部处理错误应该告警*/
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
        /*先检查链路的状态*/
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
        /*发送数据*/
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
        { /*内部处理错误应该告警*/
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

        /*查找对应的客户端cb*/
        pTsClient = SCTP_findClient(pSctpSrvCb, &(pDataReq->dstAddr));
        if(XNULLP == pTsClient )
        {
            /*没找到对应的客户端*/
            errorFlag = XTRUE;
            /*有可能是连接的客户端关闭*/
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

        /*发送数据*/
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

    /*发送过程中出错,发送error send消息到上层*/
    if(errorFlag)
    {
        /*关闭后在向上层发送错误信息*/
        XOS_Trace(MD(FID_NTL,PL_INFO),"srdFID[%d] to destFId[%d] with msgid[%d] and pri[%d]",
        pMsg->datasrc.FID,pMsg->datadest.FID,pMsg->msgID,pMsg->prio);
        ret = NTL_msgToUser((XVOID*)&sendError,&(pMsg->datasrc),sizeof(t_SENDSCTPERROR),eSctpErrorSend);
        if ( ret != XSUCC )
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqProc()-> send msg sendError to user failed!");
        }
    }

    /*释放数据*/
    if(pDataReq->pData!= XNULLP)
    {
        XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
    }

    return XSUCC;
}


/************************************************************************
函数名:SCTP_channel_find_function
功能: 根据用户参数查找通道是否已分配,如果已分配，则重置为init
输出:
返回: 成功返回XTRUE,失败返回XERROR
说明:
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
函数名:SCTP_ResetLinkByReapply
功能:  因重复申请链路而停止原来的链路
输入:  pCloseReq －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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

    /*下行的消息都要检查句柄的的有效性*/
    if(!NTL_isValidLinkH(pCloseReq->linkHandle))
    {
        /*链路句柄无效的消息，直接返回*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ResetLinkByReapply()->linkHandle invalid!");
        return XERROR;
    }
    /*获取链路类型*/
    linkType = NTL_getLinkType(pCloseReq->linkHandle);

    switch(linkType)
    {
    case eSCTPClient:
        /*获取sctp 控制块*/
        pCliCb = (t_SCCB*)XNULLP;
        pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        if(XNULLP == pCliCb)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ResetLinkByReapply()->get the sctp client control block failed!");
            return XERROR;
        }        

        /*关闭链路*/
        
        /*关闭后置成初始化状态*/
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
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_ResetLinkByReapply()->get the sctp server control block failed!");
            return XERROR;
        }

        /*关闭sctpserver*/
        SCTP_CloseServerSocket(pServCb);        

        /*改变相应的cb 数据,初始化*/
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
函数名:SCTP_CloseServerSocket
功能:  关闭sctpserver
输入:  pServCb －sctpserver控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: sctpserver需要调用方的全局保护
************************************************************************/
void SCTP_CloseServerSocket(t_SSCB *pServCb)
{
    if(NULL == pServCb)
    {
        return ;
    }

    /*首先关闭所有接入的客户端*/
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

    /*关闭fd*/

    return;
}

/************************************************************************
函数名:SCTP_ResetLinkByReapplyEntry
功能:  因重复申请链路而停止原来的链路
输入:  e_LINKTYPE linkType      -连接类型
        HAPPUSER *pUserHandle   -连接的用户句柄
        XS32 *pnRtnFind         -控制块的索引值
输出:   t_LINKINITACK *linkInitAck  -处理结果
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
函数名:SCTP_RestartLink
功能:  重新连接sctp链接
输入:   t_SCCB* sctpCliCb
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
    /*分配消息内存*/
    pMsg = (t_XOSCOMMHEAD*)XNULL;
    pMsg = XOS_MsgMemMalloc(FID_NTL,sizeof(t_LINKSTART));
    if(pMsg == XNULL)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"ERROR: link msg to ntl: can not malloc memory.");
        return XERROR;
    }

    /*填写消息数据*/
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datasrc.FID = FID_NTL;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = FID_NTL;
    pMsg->prio = eFastMsgPrio;
    pMsg->msgID = eLinkStart;
    XOS_MemCpy(pMsg->message, &startLnk, sizeof(t_LINKSTART));

    /*发送数据*/
    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        /*数据指示消息，应该先释放收到数据*/
        XOS_MsgMemFree(FID_NTL, pMsg);
        XOS_Trace(FILI, FID_NTL, PL_ERR, "ERROR: NTL_RestartSctpLink send msg failed.");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
函数名:SCTP_CliCfgShow
功能: 显示sctp client 配置信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
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
函数名:SCTP_ServCfgShow
功能: 显示sctp server 配置信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
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


