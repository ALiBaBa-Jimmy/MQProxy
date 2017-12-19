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

#if defined(XOS_SCTP) && defined(XOS_LINUX)

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
#include "xossctp.h"


/*-------------------------------------------------------------------------
                ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/
static t_SCTPGLOAB g_sctpCb;
extern XBOOL g_sctp_timer;
extern XBOOL g_ntl_timer;
XSTATIC char xos_sctp_hb_interval[64] = "net.sctp.hb_interval = %d\r\n\r\n";
XSTATIC char xos_sctp_rto_min[64] = "net.sctp.rto_min = %d\r\n\r\n";
XSTATIC char xos_sctp_rto_initial[64] = "net.sctp.rto_initial = %d\r\n\r\n";
XSTATIC char xos_sctp_sack_timeout[64] = "net.sctp.sack_timeout = %d\r\n\r\n";

/*�洢sctpϵͳ��������ʱ�ļ���*/
#define XOS_SCTP_KERNEL_FILENAME "xos_sctp_kernel.conf"

/**************************************************************
������: XOS_SetSctpKernelParas
����: ����sctpЭ����ں˲������Ż���·�����ļ��ʱ��
����: 
���: ��
����: 
˵��: 
***************************************************************/
XVOID XOS_SetSctpKernelParas(t_NTLGENCFG* pGenCfg)
{
    FILE *fStream = NULL;
    XCHAR szCmdBuf[MAX_ORDERLEN] = {0};
    t_SOCKFD init_fd = XOS_INET_INV_SOCKFD;

    if(pGenCfg == XNULLP)
    {
        return;
    }
    /*linux����ϵͳ����ʱ��/proc/sys/net/sctp�ļ��ǲ����ڣ�
    ��Ҫ����sctp socket����������ļ������򱾺�������Ч*/
    init_fd = socket(AF_INET, XOS_INET_STREAM, IPPROTO_SCTP);
    if(init_fd != XOS_INET_INV_SOCKFD)
    {
        close(init_fd);
    }
    
    if(NULL != (fStream = XOS_OpenFile(XOS_SCTP_KERNEL_FILENAME, XF_WTMODE)))
    {
        XOS_Sprintf(szCmdBuf, sizeof(szCmdBuf) - 1,xos_sctp_hb_interval,pGenCfg->hb_interval);
        XOS_WriteFile(szCmdBuf,strlen(szCmdBuf),1,fStream);
        
        XOS_Sprintf(szCmdBuf, sizeof(szCmdBuf) - 1,xos_sctp_rto_min,pGenCfg->rto_min);
        XOS_WriteFile(szCmdBuf,strlen(szCmdBuf),1,fStream);
        
        XOS_Sprintf(szCmdBuf, sizeof(szCmdBuf) - 1,xos_sctp_rto_initial,pGenCfg->rto_init);
        XOS_WriteFile(szCmdBuf,strlen(szCmdBuf),1,fStream);

        XOS_Sprintf(szCmdBuf, sizeof(szCmdBuf) - 1,xos_sctp_sack_timeout,pGenCfg->sack_timeout);
        XOS_WriteFile(szCmdBuf,strlen(szCmdBuf),1,fStream);

        XOS_CloseFile(&fStream);

        XOS_Sprintf(szCmdBuf, sizeof(szCmdBuf)-1, "/sbin/sysctl -p %s", XOS_SCTP_KERNEL_FILENAME);

        XOS_ExeCmd(szCmdBuf,NULL,0);
        XOS_DeleteFile(XOS_SCTP_KERNEL_FILENAME);
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
    pServCliCb = (t_SSCLI*)XOS_HashElemFind(g_sctpCb.tSctpCliH,(XVOID*)pClientAddr);
    return pServCliCb;
}

/*******************************************************
��������    : XINET_GetSctpSockName
����        : liukai
�������    : 2013��9��22��
��������    : ��ȡsctp����������ı��ص�ַ
�������    : t_XINETFD *sockFd -������
�������    : t_SCTPIPADDR* locAddr��   -���ص�ַ(�����ֽ���)
����ֵ      : XPUBLIC XS16
XOS_SUCC  - �ɹ�
XOS_ERROR - ��Ҫָ��������
*********************************************************/
XPUBLIC XS16 XINET_GetSctpSockName(t_XINETFD *sockFd, t_SCTPIPADDR* locAddr)
{
    struct sockaddr_in *sockAddr = NULL;
    t_INETSOCKADDR *lclSockAddr = NULL;
    XS32  i = 0;
    XS32  nAddrNum = 0;

    if ((sockFd == XNULLP) || XOS_INET_INV_SOCK_FD(sockFd) ||
        (locAddr == XNULLP))
    {
        return(XERROR);
    }

    nAddrNum = sctp_getladdrs(sockFd->fd, 0,&lclSockAddr);

    if( nAddrNum <= 0 )
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"XINET_GetSctpSockName()->socket doesnt bind any address!");
        return(XERROR);
    }

    sockAddr = (struct sockaddr_in *)lclSockAddr;
    for(i = 0;i < nAddrNum && i < SCTP_ADDR_NUM; i++)
    {
        locAddr->ip[i] = XOS_INET_NTOH_U32(sockAddr[i].sin_addr.s_addr);
    }
    locAddr->port = XOS_INET_NTOH_U16(sockAddr[0].sin_port);
    locAddr->ipNum = (XU16)i;
    sctp_freeladdrs(lclSockAddr);

    return(XSUCC);
}

/************************************************************************
������:SCTP_noticeCloseCli
����:  ֪ͨ�ر�sctp�Ŀͻ���
����: pCliCb   �ͻ��˿��ƿ�ָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_noticeCloseCli(t_SCCB* pCliCb)
{
    t_XOSCOMMHEAD *pCloseMsg = XNULL;
    t_LINKCLOSEREQ *pCloseReq = XNULL;
    XS32 ret = 0;

    if(!pCliCb)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_noticeCloseCli()->pCliCb is null!");
        return XERROR;
    }
    pCloseMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL,sizeof(t_LINKCLOSEREQ));
    if(pCloseMsg == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_noticeCloseCli()->malloc close msg failed!");
        return XERROR;
    }
    pCloseMsg->datasrc.FID = FID_NTL;
    pCloseMsg->datasrc.PID = XOS_GetLocalPID();
    pCloseMsg->datadest.FID = FID_NTL;
    pCloseMsg->datadest.PID =  XOS_GetLocalPID();
    pCloseMsg->length = sizeof(t_LINKCLOSEREQ);
    pCloseMsg->msgID = eLinkStop;
    pCloseMsg->prio = eHAPrio;
    pCloseReq = (t_LINKCLOSEREQ*)(pCloseMsg->message);
    pCloseReq->linkHandle = pCliCb->linkHandle;

    ret = XOS_MsgSend(pCloseMsg);
    if(ret != XSUCC)
    {
        XOS_MsgMemFree(FID_NTL,pCloseMsg);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_noticeCloseCli()->send close msg failed!");
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
������:
����:  �ر�sctp�ͻ���
����:   taskNo       �����
        pCliCb   �ͻ��˿��ƿ�ָ��
        XS32 close_type  �رյ����Ͷ���
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: �ù��ܽӿ���Ҫ����������,�����ر�;�����ر�.
************************************************************************/
XS32 SCTP_closeCli(XU32 taskNo, t_SCCB *pCliCb,XS32 close_type)
{
    if(taskNo >= (XU32)g_sctpCb.sctpCliTskNo)
    {
        return XERROR;
    }

    if(pCliCb == XNULLP)
    {
        return (XERROR);
    }
    if(XOS_INET_INV_SOCK_FD(&(pCliCb->sockFd)))
    {
        return (XERROR);
    }

    /*��տͻ��˵�����*/
    SCTP_dataReqClear(&(pCliCb->packetlist));
    switch(close_type)
    {
    case NTL_SHTDWN_RECV:
        /*����read set */
        XOS_INET_FD_CLR(&(pCliCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet));
        break;
    case NTL_SHTDWN_SEND:
        /*����read set */
        XOS_INET_FD_CLR(&(pCliCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet));
        /*����write set */
        XOS_INET_FD_CLR(&(pCliCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.writeSet.fdSet));
        break;
    default:
        XOS_Trace(MD(FID_NTL,PL_MIN),"SCTP closeTCli unsupport close type %d",close_type);
        return (XERROR);
    } 

    if(--g_sctpCb.pSctpCliTsk[taskNo].setInfo.sockNum == 0)
    {
        /*���ͻ�������Ϊ0ʱ������ͻ�������������ź������������´��������ȴ��´��пͻ�������ʱ������*/
        XOS_SemGet(&(g_sctpCb.pSctpCliTsk[taskNo].taskSemp));
    }

    if(g_sctpCb.pSctpCliTsk[taskNo].setInfo.sockNum  == 0xffff)
    {
        g_sctpCb.pSctpCliTsk[taskNo].setInfo.sockNum = 0;
    }

    /*�ر�socket*/
    if ( XSUCC != XINET_CloseSock(&(pCliCb->sockFd)))
    {
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
������:SCTP_ResetAllFd
����:  �������е�sctp��д��
����:  taskNo  �����
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_ResetAllFd(XU32 taskNo)
{
    /*����read set */
    XOS_INET_FD_ZERO(&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet));
    /*����write set */
    XOS_INET_FD_ZERO(&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.writeSet.fdSet));

    g_sctpCb.pSctpCliTsk[taskNo].setInfo.sockNum = 0;

    return XSUCC;
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
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp server close client begin:serverClis sockNum=%d",g_sctpCb.sctpServTsk.setInfo.sockNum);
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp server close client begin:closing client sock=%d",pSctpServCli->sockFd.fd);

    /*�ر�sock ��������Դ*/

#ifdef XOS_NEED_CHK
    SCTP_dataReqClear(&(pSctpServCli->packetlist));

    if(XOS_INET_INV_SOCK_FD(&(pSctpServCli->sockFd)))
    {
        return (XERROR);
    }
#endif

    XOS_INET_FD_CLR(&(pSctpServCli->sockFd),&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));

    if(--g_sctpCb.sctpServTsk.setInfo.sockNum == 0)
    {
        XOS_SemGet(&(g_sctpCb.sctpServTsk.taskSemp));
    }
    if(g_sctpCb.sctpServTsk.setInfo.sockNum == 0xffff)
    {
        g_sctpCb.sctpServTsk.setInfo.sockNum=0;
    }
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp server close client end: serverClis  sockNum=%d",g_sctpCb.sctpServTsk.setInfo.sockNum);
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp server close client end: closed  client sock=%d",pSctpServCli->sockFd.fd);
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp server close client end: current client num=%d.\r\n",(pSctpServCli->pServerElem)->usageNum);

    XINET_CloseSock(&(pSctpServCli->sockFd));

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
    //XOS_MutexLock(&(g_sctpCb.hashMutex));
    XOS_HashDelByElem(g_sctpCb.tSctpCliH, pSctpServCli);
    //XOS_MutexUnlock(&(g_sctpCb.hashMutex));

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
    t_PARA  timerParam;
    t_BACKPARA backPara;
    t_SCCB *pCliCb = NULL;
    t_SSCB *pServCb = NULL;
    XU32 taskNo = 0;
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

        /*��ȡ��·���������*/
        taskNo = (XU32)NTL_getLinkIndex(pCloseReq->linkHandle)/(g_sctpCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_sctpCb.sctpCliTskNo)
        {
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_closeReqProc()->get the sctp client taskno failed!");
            return XERROR;
        }

        /*���������رգ�Ҫ��ʱ����*/
        if(pMsg->datasrc.FID == FID_NTL)
        {
            /*��ʼ��*/
            pCliCb->linkState = eStateInited;
            pCliCb->expireTimes = 0;

            /*������ʱ������ʱ��*/            
            XOS_MemSet(&timerParam,0,sizeof(t_PARA));
            timerParam.fid = FID_NTL;
            timerParam.len = SCTP_CLI_RECONNECT_INTERVAL;
            timerParam.mode = TIMER_TYPE_LOOP;
            timerParam.pre = TIMER_PRE_LOW;

            XOS_MemSet(&backPara,0,sizeof(t_BACKPARA));
            backPara.para1 = (XPOINT)pCloseReq->linkHandle;
            backPara.para2 = (XPOINT)taskNo;   
            backPara.para3 = (XPOINT)eSCTPReconnect;

            /*�����ʱ����������յ��µķ���������*/
            SCTP_dataReqClear(&(pCliCb->packetlist));/*�����·֮ǰ���յ������ݰ�*/

            XOS_INIT_THDLE (pCliCb->timerId);
            XOS_TimerStart(&(pCliCb->timerId),&timerParam, &backPara);
        }
        else /*�û�����ر�*/
        {
            SCTP_StopClientTimer(pCliCb);          

            /*�������������رպ��û������رգ�����Ҫ�ٹرգ��ر�fd, ������ݣ����ö�д��*/
            if(pCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
            {
                SCTP_closeCli(taskNo, pCliCb, NTL_SHTDWN_SEND);
            }
            else
            {
                /*�����ʱ����������յ��µķ���������*/
                SCTP_dataReqClear(&(pCliCb->packetlist));/*�����·֮ǰ���յ������ݰ�*/
            }

            pCliCb->linkState = eStateInited;
            pCliCb->expireTimes = 0;
            /*���Զ˵�ַ�ÿ�*/
            memset(pCliCb->peerAddr.ip,0,sizeof(pCliCb->peerAddr.ip[0]) * SCTP_ADDR_NUM);//XU32��Ϊ����ʵ������
            pCliCb->peerAddr.ipNum = 0;
            pCliCb->peerAddr.port = 0;            

            memset(pCliCb->myAddr.ip,0,sizeof(pCliCb->myAddr.ip[0]) * SCTP_ADDR_NUM);
            pCliCb->myAddr.ipNum = 0;
            pCliCb->myAddr.port = 0;

        }
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
       
        if(0x00 == pCloseReq->cliAddr.ip && 0x00 == pCloseReq->cliAddr.port)
        {
            /*���ȹر����н���Ŀͻ���*/     

            while(pServCb->pLatestCli != XNULLP)
            {
                SCTP_closeTsCli(pServCb->pLatestCli);
                if(pServCb->usageNum == 0)
                {
                    break;
                }
            }

            /*�ر�listen ��fd*/
            XOS_INET_FD_CLR(&(pServCb->sockFd),&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));
            if(--g_sctpCb.sctpServTsk.setInfo.sockNum == 0)
            {
                XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
                XOS_SemGet(&(g_sctpCb.sctpServTsk.taskSemp));
                XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
            }
            if(g_sctpCb.sctpServTsk.setInfo.sockNum == 0xffff)
            {
                g_sctpCb.sctpServTsk.setInfo.sockNum =0;
            }

            /*�ر�fd*/
            XINET_CloseSock(&(pServCb->sockFd));
            
            /*�ı���Ӧ��cb ����,��ʼ��*/
            pServCb->linkState = eStateInited;
            pServCb->maxCliNum = 0;
            pServCb->usageNum = 0;
            pServCb->pLatestCli = (t_SSCLI*)XNULLP;
            pServCb->authFunc = NULL;
            pServCb->pParam = NULL;
            
        }
        else
        {   
            /*����ָ���Ŀͻ��˿��ƿ�*/
            pSctpServerClient = (t_SSCLI*)XOS_HashElemFind(g_sctpCb.tSctpCliH, (XVOID*)&(pCloseReq->cliAddr));
            if(NULL != pSctpServerClient)
            {
                XOS_Trace(MD(FID_NTL, PL_INFO),"close sctp client ip=0x%08x, port=%d", pCloseReq->cliAddr.ip, pCloseReq->cliAddr.port);
                SCTP_closeTsCli(pSctpServerClient);
            }
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
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
    XU32 taskNo = 0;
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

        /*��ȡ��·���������*/
        taskNo = (XU32)NTL_getLinkIndex(pReleaseReq->linkHandle)/(g_sctpCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_sctpCb.sctpCliTskNo)
        {
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_ReleaseLink()->get the sctp client taskno failed!");
            return XERROR;
        }

        SCTP_StopClientTimer(pCliCb);          

        /*�������������رպ��û������رգ�����Ҫ�ٹرգ��ر�fd, ������ݣ����ö�д��*/
        if(pCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            SCTP_closeCli(taskNo, pCliCb, NTL_SHTDWN_SEND);
        }
        else
        {
            /*�����ʱ����������յ��µķ���������*/
            SCTP_dataReqClear(&(pCliCb->packetlist));/*�����·֮ǰ���յ������ݰ�*/
        }

        pCliCb->linkState = eStateInited;
        pCliCb->expireTimes = 0;
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
        while(pServCb->pLatestCli != XNULLP)
        {
            SCTP_closeTsCli(pServCb->pLatestCli);
            if(pServCb->usageNum == 0)
            {
                break;
            }
        }

        /*�ر�listen ��fd*/
        XOS_INET_FD_CLR(&(pServCb->sockFd),&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));
        if(--g_sctpCb.sctpServTsk.setInfo.sockNum == 0)
        {
            XOS_SemGet(&(g_sctpCb.sctpServTsk.taskSemp));
        }
        if(g_sctpCb.sctpServTsk.setInfo.sockNum == 0xffff)
        {
            g_sctpCb.sctpServTsk.setInfo.sockNum =0;
        }

        /*�ر�fd*/
        XINET_CloseSock(&(pServCb->sockFd));

        /*�ı���Ӧ��cb ����,��ʼ��*/
        pServCb->linkState = eStateInited;
        pServCb->maxCliNum = 0;
        pServCb->usageNum = 0;
        pServCb->pLatestCli = (t_SSCLI*)XNULLP;
        pServCb->authFunc = NULL;
        pServCb->pParam = NULL;
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));

        break;

    default:
        break;
    }

    return XSUCC;
}

/************************************************************************
������:SCTP_Opt
����:  ����������������������·��ż����·���Ķ�����ֵⷥ
����:  pCliCb ��sctp�ͻ��˿��ƿ�ָ��
        peerAddrNum - �Զ˰󶨵�ַ����
���:
����:  �ɹ�����XSUCC,���򷵻�XERROR
˵��:  
************************************************************************/
XS32 SCTP_Opt(t_SCCB *pCliCb,XS32 peerAddrNum)
{
    t_SCTPSTARTACK sctpStartAck;
    XU32 optVal = 0;
    XU32 peerInstream = 0;
    XS32 ret = XSUCC;

    if(pCliCb == NULL)
    {
        return XERROR;
    }
    ret = XINET_GetOpt(&(pCliCb->sockFd), XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_PEER_INSTREAM, &peerInstream);
    if( ret != XSUCC || pCliCb->maxStream > peerInstream )
    {
        /*�ر��½����sock*/
        XINET_CloseSock(&(pCliCb->sockFd));
        XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_Opt,server (ip[0x%x],port[%d]) instream is %d,local outstream is %d!",
            XOS_INET_NTOH_U32(pCliCb->peerAddr.ip[0]),XOS_INET_NTOH_U16(pCliCb->peerAddr.port),peerInstream,pCliCb->maxStream);
        pCliCb->linkState = eStateInited;
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_Opt(),connect sctp server failed,peer instream too small!");

        /*��������ʧ����Ϣ���ϲ�*/
        sctpStartAck.appHandle = pCliCb->userHandle;
        sctpStartAck.linkStartResult = eInvalidInstrm;
        XOS_MemCpy(&(sctpStartAck.localAddr),&(pCliCb->myAddr),sizeof(t_SCTPIPADDR));

        NTL_msgToUser((XVOID*)&sctpStartAck,&(pCliCb->linkUser),sizeof(t_SCTPSTARTACK),eSctpStartAck);
        return ret;
    }
    
    optVal = pCliCb->pathmaxrxt;
    ret = XINET_SetOpt(&(pCliCb->sockFd),XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_PATHMAXRXT,&optVal);
    if( ret != XSUCC )
    {
        /*�ر��½����sock*/
        XINET_CloseSock(&(pCliCb->sockFd));
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_Opt,set sctp_path_max_retrans new client(ip[0x%x],port[%d]) !",
            XOS_INET_NTOH_U32(pCliCb->myAddr.ip[0]),XOS_INET_NTOH_U16(pCliCb->myAddr.port));
        /*��������ʧ����Ϣ���ϲ�*/
        sctpStartAck.appHandle = pCliCb->userHandle;
        sctpStartAck.linkStartResult = eOtherResult;
        XOS_MemCpy(&(sctpStartAck.localAddr),&(pCliCb->myAddr),sizeof(t_SCTPIPADDR));

        NTL_msgToUser((XVOID*)&sctpStartAck,&(pCliCb->linkUser),sizeof(t_SCTPSTARTACK),eSctpStartAck);
        return ret;
    }

    optVal = pCliCb->pathmaxrxt * peerAddrNum;
    ret = XINET_SetOpt(&(pCliCb->sockFd),XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_ASSOCINFO,&optVal);
    if( ret != XSUCC )
    {
        /*�ر��½����sock*/
        XINET_CloseSock(&(pCliCb->sockFd));
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_Opt,set sctp_assoc_max_retrans new client(ip[0x%x],port[%d]) !",
            XOS_INET_NTOH_U32(pCliCb->myAddr.ip[0]),XOS_INET_NTOH_U16(pCliCb->myAddr.port));
        /*��������ʧ����Ϣ���ϲ�*/
        sctpStartAck.appHandle = pCliCb->userHandle;
        sctpStartAck.linkStartResult = eOtherResult;
        XOS_MemCpy(&(sctpStartAck.localAddr),&(pCliCb->myAddr),sizeof(t_SCTPIPADDR));

        NTL_msgToUser((XVOID*)&sctpStartAck,&(pCliCb->linkUser),sizeof(t_SCTPSTARTACK),eSctpStartAck);
        return ret;
    }
    return XSUCC;
}

/************************************************************************
������:SCTP_AcceptClient
����:  ����һ���ͻ������ӣ�����ӵ�����
����:  pServCb ��sctp����˿��ƿ�ָ��
���:
����:  �ɹ�����XSUCC,���򷵻�XERROR
˵��:  ֻ��SCTP_servTsk�е���
************************************************************************/
XS32 SCTP_AcceptClient(t_SSCB  *pServCb)
{
    t_SSCLI    servCliCb;
    t_SSCLI *pServCliCb = NULL;
    t_SCTPIPADDR fromAddr;
    t_XINETFD servCliFd;
    XS32 ret = 0;
    XU32 peerInstream;
    XU32 optVal = 0;
    t_IPADDR keyAddr;
    XVOID *pLocation = NULL;
    t_CONNIND connectInd;

    if(pServCb == NULL)
    {
        return XERROR;
    }
    XOS_MemSet(&fromAddr,0,sizeof(t_SCTPIPADDR));
    /*�Ƚ���*/
    ret = XINET_SctpAccept(&(pServCb->sockFd),&fromAddr,pServCb->hbInterval,&servCliFd);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_AcceptClient(),accept fd %d failed!",pServCb->sockFd.fd);
        return XERROR;
    }

    /*������֤*/
    if(pServCb->authFunc != XNULL
        && !(pServCb->authFunc(pServCb->userHandle,&fromAddr,pServCb->pParam)))
    {
        /*�ر��½����sock*/
        XINET_CloseSock(&servCliFd);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient(),get client(ip[0x%x],port[%d]) auth failed!",
            XOS_INET_NTOH_U32(fromAddr.ip[0]),XOS_INET_NTOH_U16(fromAddr.port));
        return XERROR;
    }

    ret = XINET_GetOpt(&servCliFd, XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_PEER_INSTREAM, &peerInstream);
    if( ret != XSUCC || pServCb->maxStream > peerInstream )
    {
        /*�ر��½����sock*/
        XINET_CloseSock(&servCliFd);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient(),new client's instream too small,(ip[0x%x],port[%d]) instream is %d,local outstream is %d!",
            XOS_INET_NTOH_U32(fromAddr.ip[0]),XOS_INET_NTOH_U16(fromAddr.port),peerInstream,pServCb->maxStream);
        return XERROR;
    }

    optVal = pServCb->pathmaxrxt;
    ret = XINET_SetOpt(&servCliFd,XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_PATHMAXRXT,&optVal);
    if( ret != XSUCC )
    {
        /*�ر��½����sock*/
        XINET_CloseSock(&servCliFd);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient() set sctp_path_max_retrans for socket failed!");
        return XERROR;
    }

    optVal = pServCb->pathmaxrxt * fromAddr.ipNum;
    ret = XINET_SetOpt(&servCliFd,XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_ASSOCINFO,&optVal);
    if( ret != XSUCC )
    {
        /*�ر��½����sock*/
        XINET_CloseSock(&servCliFd);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient() set sctp_assoc_max_retrans for socket failed!");
        return XERROR;
    }

    XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_AcceptClient(),accept fd %d ,ip:%x,port:%d,hb:%d!",pServCb->sockFd.fd,
        fromAddr.ip[0],fromAddr.port,pServCb->hbInterval);

    /*����ͻ����������ܳ�����������������*/
    if(pServCb->maxCliNum < pServCb->usageNum +1)
    {
        /*�ر��½����sock*/
        XINET_CloseSock(&servCliFd);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient(),new accept client(ip[0x%x],port[%d]) is overstep maxclis %d allowned!",
            XOS_INET_NTOH_U32(fromAddr.ip[0]),XOS_INET_NTOH_U16(fromAddr.port),pServCb->maxCliNum);
        return XERROR;
    }
    /*������Ŀͻ��˼ӵ�hash����*/
    XOS_MemSet(&servCliCb,0,sizeof(t_SSCLI));
    XOS_MemCpy(&(servCliCb.sockFd),&servCliFd,sizeof(t_XINETFD));
    /*����ҲӦ�ñ���*/
    servCliCb.pServerElem = pServCb;
    servCliCb.pPreCli = pServCb->pLatestCli;
    servCliCb.pNextCli = (t_SSCLI*)XNULLP;

    /*���۶Զ����Ӷ���ip��������ȡ��һ��ip��Ϊhash��key*/
    pLocation = XNULLP;
    keyAddr.ip = fromAddr.ip[0];
    keyAddr.port = fromAddr.port;
    XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_AcceptClient() client ip:%x,port:%d!",keyAddr.ip,keyAddr.port);
    pLocation = XOS_HashElemAdd(g_sctpCb.tSctpCliH,&keyAddr,(XVOID*)&servCliCb,XTRUE);

    if(pLocation == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient(),add new accept client to hash failed!");
        XINET_CloseSock(&servCliFd);
        return XERROR;
    }

    /*������*/
    pServCliCb = (t_SSCLI*)XNULLP;
    pServCliCb = (t_SSCLI*)XOS_HashGetElem(g_sctpCb.tSctpCliH, pLocation);

    if(pServCb->pLatestCli != XNULLP && pServCliCb != XNULLP)
    {
        pServCb->pLatestCli->pNextCli = pServCliCb;
    }
    pServCb->pLatestCli= pServCliCb;
    pServCb->usageNum++; /*����һ���µĿͻ�����*/

    /*���뵽read ����*/
    g_sctpCb.sctpServTsk.setInfo.sockNum++;
    XOS_INET_FD_SET(&servCliFd, &( g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));

    /*��������ָʾ��Ϣ���ϲ�*/
    connectInd.appHandle = pServCb->userHandle;
    connectInd.peerAddr.ip = fromAddr.ip[0];
    connectInd.peerAddr.port = fromAddr.port;
    //XOS_MemCpy(&(connectInd.peerAddr),&fromAddr,sizeof(t_SCTPIPADDR));
    ret = NTL_msgToUser(&connectInd,&(pServCb->linkUser),sizeof(t_CONNIND),eSctpConnInd);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_AcceptClient(),indcate new client ip[0x%x],port[%d] connecttion to user failed!",
            XOS_INET_NTOH_U32(fromAddr.ip[0]),XOS_INET_NTOH_U16(fromAddr.port));
        return XERROR;
    }
    return XSUCC;
}
/************************************************************************
* SCTP_pollingHash
* ����: �����ÿ��hashԪ��ͨ�õĺ���
* ����  :
* hHash   - ������
* elem    - Ԫ��
* param   - ����
************************************************************************/
XVOID* SCTP_pollingHash(XOS_HHASH hHash,XVOID *elem,XVOID *param)
{
    t_SSCLI *pTservCli = NULL;
    t_FDSET *pReadSet = NULL;
    XCHAR *pData = NULL;
    XCHAR *pBufData = NULL;
    XS32 len = 0;
    XS32 ret = 0;
    t_SCTPDATAIND dataInd;
    t_IPADDR *pAddr = NULL;
    t_LINKCLOSEIND tscCloseInd;
    struct sctp_sndrcvinfo sinfo;

    if( elem == XNULLP || !param)
    {
        return param;
    }

    pTservCli = (t_SSCLI*)elem;
    pReadSet = (t_FDSET*)param;
    /* ���*/
    if(XOS_INET_FD_ISSET(&(pTservCli->sockFd),pReadSet))
    {
        /*��������������*/
        len = XOS_INET_READ_ANY;
        ret = XINET_SctpRecvMsg(&(pTservCli->sockFd),&pData,&len,NULL,&sinfo);
        if(ret != XSUCC)
        {
            if(ret == XINET_CLOSE)
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_pollHash()-> sctp server close client link sock=[%d]!",pTservCli->sockFd.fd);
                XOS_MemSet(&tscCloseInd,0,sizeof(t_LINKCLOSEIND));
                tscCloseInd.appHandle = pTservCli->pServerElem->userHandle;
                tscCloseInd.closeReason = ePeerReq;
                pAddr = (t_IPADDR*)XNULLP;
                pAddr = (t_IPADDR*)XOS_HashGetKeyByElem(hHash,pTservCli);
                if(pAddr == XNULLP)
                {
                    XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_pollHash()->connect indication get peer address failed!");
                    SCTP_closeTsCli(pTservCli);
                    return param;
                }
                tscCloseInd.peerAddr.ip = pAddr->ip;
                tscCloseInd.peerAddr.port = pAddr->port;
                NTL_msgToUser(&tscCloseInd,&(pTservCli->pServerElem->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);
                SCTP_closeTsCli(pTservCli);
                return param;
            }
            else if(ret == XINET_TIMEOUT)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_pollHash()->remote sctp clinet disconnect!");
                XOS_MemSet(&tscCloseInd,0,sizeof(t_LINKCLOSEIND));
                tscCloseInd.appHandle = pTservCli->pServerElem->userHandle;
                tscCloseInd.closeReason = eNetError;
                pAddr = (t_IPADDR*)XNULLP;
                pAddr = (t_IPADDR*)XOS_HashGetKeyByElem(hHash,pTservCli);
                if(pAddr == XNULLP)
                {
                    XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_pollHash()->connect indication get peer address failed!");
                    SCTP_closeTsCli(pTservCli);
                    return param;
                }
                tscCloseInd.peerAddr.ip = pAddr->ip;
                tscCloseInd.peerAddr.port = pAddr->port;
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_pollHash()->sctp client ip=0x%08x, port=%d!",pAddr->ip,pAddr->port);
                NTL_msgToUser(&tscCloseInd,&(pTservCli->pServerElem->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);
                SCTP_closeTsCli(pTservCli);
                return param;
            }
            else
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_pollHash()-> sctp server client sock=[%d] receive msg error!",pTservCli->sockFd.fd);
                return param;
            }
        }

        /*�������ݵ��ϲ��û�*/
        dataInd.appHandle = pTservCli->pServerElem->userHandle;
        dataInd.dataLenth = (XU32)len;

        /*ʵ�����ݶ���С��ת����С�ڴ�洢���յ�����*/
        pBufData = XOS_MemMalloc(FID_NTL, len);    
        if( XNULLP == pBufData )
        {
            XOS_MemFree(FID_NTL, pData);
            return param;
        }
        XOS_MemCpy(pBufData, pData, len);
        XOS_MemFree(FID_NTL, pData);

        dataInd.pData = pBufData;
        dataInd.attr.stream = sinfo.sinfo_stream;
        dataInd.attr.context = sinfo.sinfo_context;
        dataInd.attr.ppid = sinfo.sinfo_ppid;
        pAddr = (t_IPADDR*)XNULLP;
        pAddr = (t_IPADDR*)XOS_HashGetKeyByElem(hHash,pTservCli);
        if( pAddr != XNULLP)
        {
            XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_pollHash()-> server recv %d Byte data from [0x%x,%d]!",len,pAddr->ip,pAddr->port );
            dataInd.peerAddr.ip = pAddr->ip;
            dataInd.peerAddr.port = pAddr->port;
        }
        else
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_pollHash()-> get hash key error!" );
        }

        NTL_msgToUser(&dataInd,&(pTservCli->pServerElem->linkUser),sizeof(t_SCTPDATAIND),eSctpDataInd);
    }

    return param;
}

/************************************************************************
������:SCTP_sctpServTsk
����:  sctp server listening function
����:taskNo  �����
���:
����:
˵��:
************************************************************************/
XVOID SCTP_servTsk(XVOID* taskNo)
{
    t_FDSET  readSet;
    XS32 ret = 0;
    t_SSCB  *pServCb = NULL;
    XS16 setNum = 0;
    XS32 i = 0;
    XU32 pollAbideTime = 0;

    XOS_UNUSED(taskNo);
    pollAbideTime = POLL_FD_TIME_OUT;

    while(1)
    {
        /*�̸߳�����ʱ����û��sockfd��fdΪ��ʱselectֱ�ӷ���*/
        g_sctpCb.sctpServTsk.activeFlag = XFALSE;
        XOS_SemGet(&(g_sctpCb.sctpServTsk.taskSemp));
        g_sctpCb.sctpServTsk.activeFlag = XTRUE;
        XOS_SemPut(&(g_sctpCb.sctpServTsk.taskSemp));

        /*�������ֲ������У���ֹ�ƻ�ȫ�ֵĶ���*/
        XOS_MemSet(&readSet,0,sizeof(t_FDSET));
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        XOS_MemCpy(&readSet,&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet),sizeof(t_FDSET));
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));

        /*select ����*/
        setNum = 0;
        ret = XINET_Select(&readSet,(t_FDSET*) XNULLP,(XU32*)&pollAbideTime,(XS16*)&setNum);
        if(ret != XSUCC)
        {
            /*select ��ʱ*/
            if(ret == XINET_TIMEOUT)
            {
                continue;
            }
            else /*��������*/
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"sctpServer task,select readset failed, setNum =%d.",setNum);
                continue;
            }
        }
        
        if(setNum > 0)
        {
            XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
            /*�пͻ������ӽ���*/
            for(i = 0; i<g_sctpCb.genCfg.maxSctpServLink; i++)
            {
                pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,i);

                if(pServCb == XNULL)
                {
                    continue;
                }
                if(pServCb->sockFd.fd == XOS_INET_INV_SOCKFD)
                {
                    continue;
                }

                if(XOS_INET_FD_ISSET(&(pServCb->sockFd),&(readSet)))
                {
                    setNum--;
                    ret = SCTP_AcceptClient(pServCb);
                    if(ret == XERROR)
                    {
                        continue;
                    }
                }
            }

            /*�����ݽ���*/
            XOS_HashWalk(g_sctpCb.tSctpCliH,SCTP_pollingHash,&readSet);
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        }       
    }
}

/************************************************************************
������:SCTP_cliRcvFunc
����:  sctp�ͻ��˵���ں���
����:taskNo  �����
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XVOID  SCTP_cliRcvFunc(XVOID* taskPara)
{
    XU32 taskNo = 0;
    t_FDSET sctpCliRead;
    t_FDSET sctpCliWrite;
    t_FDSET *pReadSet = NULL;
    t_FDSET *pWriteSet = NULL;
    t_SCTPDATAIND dataInd;
    t_SCCB *pCliCb = NULL;
    t_LINKCLOSEIND closeInd;
    t_SCTPSTARTACK sctpStartAck;
    XS32 ret = 0;
    XS16 setNum = 0;
    XCHAR *pData = NULL;
    XCHAR *pBufData = NULL;
    XS32 len = 0;
    XU32 i = 0;
    XU32 pollAbideTime = 0;
    XU32 nNoStatus = 0;
    XU32 nDealFlg = 0;
    struct sctp_sndrcvinfo sinfo;
    XS32 peerAddrNum = 0;

    /*����*/
    pollAbideTime = POLL_FD_TIME_OUT;
    taskNo = (XU32)(XPOINT)taskPara;

    while(1)
    {
        /*�̸߳�����ʱ����û��sockfd��fdΪ��ʱselectֱ�ӷ���*/
        g_sctpCb.pSctpCliTsk[taskNo].activeFlag = XFALSE;

        XOS_SemGet(&(g_sctpCb.pSctpCliTsk[taskNo].taskSemp));
        g_sctpCb.pSctpCliTsk[taskNo].activeFlag = XTRUE;
        XOS_SemPut(&(g_sctpCb.pSctpCliTsk[taskNo].taskSemp));

        /*�������ֲ�������*/
        XOS_MemSet(&sctpCliRead,0,sizeof(t_FDSET));
        XOS_MemSet(&sctpCliWrite,0,sizeof(t_FDSET));
        pReadSet = (t_FDSET*)XNULLP;
        pWriteSet = (t_FDSET*)XNULLP;

        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
        /*������ƿ�Ϊ��*/
        if(XOS_ArrayGetCurElemNum(g_sctpCb.sctpClientLinkH) == 0)
        {
            SCTP_ResetAllFd(taskNo);
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Sleep(1);
            continue;
        }
        
#ifdef XOS_NEED_CHK

        if(g_sctpCb.pSctpCliTsk[taskNo].setInfo.sockNum > 0)
        {
            /*copy read set */
            XOS_INET_FD_ZERO(&(sctpCliRead));
            XOS_MemCpy(&sctpCliRead,&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet),sizeof(t_FDSET));
            pReadSet = &sctpCliRead;
            
            /*copy write set*/
            XOS_INET_FD_ZERO(&(sctpCliWrite));
            XOS_MemCpy(&sctpCliWrite,&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.writeSet.fdSet),sizeof(t_FDSET));
            pWriteSet = &sctpCliWrite;
        }
        else
        {
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            continue;
        }
#endif
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        /*poll the socket */
        setNum = 0;
        ret = XINET_Select(pReadSet,pWriteSet,(XU32*)&pollAbideTime,(XS16*)&setNum);
        if(ret != XSUCC)
        {
            /*select ��ʱ*/
            if(ret == XINET_TIMEOUT)
            {
                continue;
            }
            else  /*select ����Ӧ�����������صģ������continue�������������أ�������ѭ��*/
            {    /*��������û��ʹ�û��⣬���п���fd���ر��ˣ�ȴ�ڲ���select�������쳣*/
                XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_cliRcvFunc,select readset/writeset failed,return setNum =%d.",setNum);
                continue;
            }
        }

        nNoStatus = 0;
        nDealFlg = 0;
        if(setNum > 0)
        {
            XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
            for(i=(taskNo*(g_sctpCb.genCfg.fdsPerThrPolling)); i<(taskNo+1)*(g_sctpCb.genCfg.fdsPerThrPolling); i++)
            {
                /*����Ϊȫ�� 2013.10.24*/
                pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,(XS32)i);

                if(pCliCb != XNULLP)
                {
                    nDealFlg++;
                    /*�ͻ���δ������·����·��������*/
                    if( ((pCliCb->linkState != eStateConnected)&&(pCliCb->linkState != eStateConnecting))
                        ||(pCliCb->sockFd.fd == XOS_INET_INV_SOCKFD))
                    {
                        continue;
                    }

                    /*��д*/
                    if((pWriteSet != XNULLP) && XOS_INET_FD_ISSET(&(pCliCb->sockFd),pWriteSet))
                    {
                        setNum--;

                        /*ͬʱ�ɶ���д��˵���쳣*/
                        if(XOS_INET_FD_ISSET(&(pCliCb->sockFd),pReadSet) || pCliCb->sockFd.fd <= 0)
                        {
                            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_cliRcvFunc(),sctp client fd = %d disconnect,try reconnect!",
                            pCliCb->sockFd.fd);
                            /*�ض�ʱ��*/
                            XOS_TimerStop(FID_NTL,pCliCb->timerId);
                            /*ֹͣ��·*/
                            SCTP_closeCli(taskNo,pCliCb,NTL_SHTDWN_SEND);
                            /*֪ͨ״̬�ı�*/
                            SCTP_noticeCloseCli( pCliCb);
                            /*���͹ر�ָʾ*/
                            closeInd.appHandle = pCliCb->userHandle;
                            closeInd.closeReason = ePeerReq;
                            NTL_msgToUser((XVOID*)&closeInd,&(pCliCb->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);

                            continue;
                        }
                        peerAddrNum = XINET_SctpConnectCheck(&(pCliCb->sockFd));

                        /*��write �������,�ȴ���ʱ�����ڽ��д���*/
                        XOS_INET_FD_CLR(&(pCliCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.writeSet.fdSet));

                        if(peerAddrNum <= 0)
                        {
                            XOS_Trace(MD(FID_NTL,PL_WARN),
                            "SCTP_cliRcvFunc()->connect sctp server failed! myaddr.ip = 0x%08x,myaddr.port = %d;peeraddr.ip = 0x%08x,peeraddr.port = %d",
                            pCliCb->myAddr.ip[0], pCliCb->myAddr.port, pCliCb->peerAddr.ip[0], pCliCb->peerAddr.port);

                            continue;
                        }

                        /*����������������������·��ż����·���Ķ�����ֵⷥ*/
                        ret = SCTP_Opt(pCliCb,peerAddrNum);
                        if(ret == XERROR)
                        {
                            continue;
                        }

                        /*��ӵ� �� ������*/
                        XOS_INET_FD_SET(&(pCliCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet));

                        /*�ͻ������ӳɹ�*/
                        if(pCliCb->linkState != eStateConnected)
                        {
                            pCliCb->linkState = eStateConnected;
                            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_cliRcvFunc(),connect sctp server successed!");
                            SCTP_StopClientTimer(pCliCb);

                            /*���������ɹ���Ϣ���ϲ�*/
                            sctpStartAck.appHandle = pCliCb->userHandle;
                            sctpStartAck.linkStartResult = eSUCC;
                            XOS_MemCpy(&(sctpStartAck.localAddr),&(pCliCb->myAddr),sizeof(t_SCTPIPADDR));

                            NTL_msgToUser((XVOID*)&sctpStartAck,&(pCliCb->linkUser),sizeof(t_SCTPSTARTACK),eSctpStartAck);
                        }
                    }
                    if(pCliCb->sockFd.fd == XOS_INET_INV_SOCKFD)
                    {
                        continue;
                    }
                    /*�ɶ������쳣����ͨ�����case������*/
                    if((pReadSet != XNULLP) && XOS_INET_FD_ISSET(&(pCliCb->sockFd),pReadSet))
                    {
                        setNum--;
                        pData = (XCHAR*)XNULLP;
                        len = XOS_INET_READ_ANY;

                        /*��������������*/
                        ret = XINET_SctpRecvMsg(&(pCliCb->sockFd),&pData,&len,NULL,&sinfo);
                        if(ret == XINET_CLOSE)
                        {
                            /* if sctp server closed then notify all this client*/
                            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_cliRcvFunc(),sctp client disconnect,try reconnect!");
                            
                            pCliCb->linkState = eStateWaitClose;
                            //SCTP_StopClientTimer(pCliCb); /*�����������ʱ��ʵ���Ѿ��رգ������ٴιر�*/

                            SCTP_closeCli(taskNo, pCliCb, NTL_SHTDWN_SEND);
                            /*֪ͨ״̬�ı�*/
                            SCTP_noticeCloseCli( pCliCb);
                            /*���͹ر�ָʾ*/
                            closeInd.appHandle = pCliCb->userHandle;
                            closeInd.closeReason = ePeerReq;
                            NTL_msgToUser((XVOID*)&closeInd,&(pCliCb->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);
                        }
                        else if(ret == XINET_TIMEOUT)
                        {
                            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_cliRcvFunc()-> sctp client detects remote server disconnect!");
                            
                            pCliCb->linkState = eStateWaitClose;

                            SCTP_closeCli(taskNo, pCliCb, NTL_SHTDWN_SEND);
                            /*֪ͨ״̬�ı�*/
                            SCTP_noticeCloseCli( pCliCb);
                            /*���͹ر�ָʾ*/
                            closeInd.appHandle = pCliCb->userHandle;
                            closeInd.closeReason = eNetError;
                            NTL_msgToUser((XVOID*)&closeInd,&(pCliCb->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);
                        }
                        else if(ret != XSUCC)
                        {
                            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_cliRcvFunc()->receive sctp data error[ret = %d]!",ret);
                        }
                        if(pData != XNULLP && len > 0)
                        {
                            /*�������ݵ��ϲ�*/
                            XOS_MemSet(&dataInd,0,sizeof(t_SCTPDATAIND));
                            dataInd.appHandle = pCliCb->userHandle;
                            dataInd.dataLenth = (XU32)len;

                            /*ʵ�����ݶ���С��ת����С�ڴ�洢���յ�����*/
                            pBufData = XOS_MemMalloc(FID_NTL, len); 
                            if( XNULLP == pBufData)
                            {
                                XOS_MemFree(FID_NTL, pData);
                                continue;
                            }

                            XOS_MemCpy(pBufData, pData, len);
                            XOS_MemFree(FID_NTL, pData);

                            dataInd.pData = pBufData;
                            dataInd.attr.stream = sinfo.sinfo_stream;
                            dataInd.attr.context = sinfo.sinfo_context;
                            dataInd.attr.ppid = sinfo.sinfo_ppid;
                            dataInd.peerAddr.ip = pCliCb->peerAddr.ip[0];
                            dataInd.peerAddr.port= pCliCb->peerAddr.port;

                            XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_cliRcvFunc()-> client recv %d Byte data from [0x%x,%d]!",len,pCliCb->peerAddr.ip[0],pCliCb->peerAddr.port );
                            ret = NTL_msgToUser((XVOID*)&dataInd,&(pCliCb->linkUser),sizeof(t_SCTPDATAIND),eSctpDataInd);
                            if(ret != XSUCC)
                            {
                                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_cliRcvFunc()->send data to user error[ret = %d]!",ret);
                                continue;
                            }
                        }
                    }

                    /* �Ѿ�������еı���λ��socket*/
                    if(setNum <= 0)
                    {
                        break;
                    }
                }
            }
            if(setNum > 0) /*������δ��д������������һ��select��������*/
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_cliRcvFunc()->deal data error! curFdNum = %d, nDealFlg = %d, nNoStatus = %d",
                          setNum, nDealFlg, nNoStatus);
            }
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        }
    }
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
    XU16 pollingNum = 0;
    XCHAR taskName[NTL_TSK_NAME_LEN] = {0};
    XS32 i = 0;
    XS32 ret = 0;
    XU32 hashElems = 0;

    if(pGenCfg == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->bad input param!");
        return XERROR;
    }

    /*����sctpϵͳ���������ڿ��ƶ������ʱ��*/
    XOS_SetSctpKernelParas(pGenCfg);

    /*����������Ϣ*/
    XOS_MemCpy(&(g_sctpCb.genCfg),pGenCfg,sizeof(t_NTLGENCFG));

    /*ȷ��һ���̼߳��ӵ�����������*/
    if(g_sctpCb.genCfg.fdsPerThrPolling > 0 && g_sctpCb.genCfg.fdsPerThrPolling < FDS_MAX_THREAD_POLLING)
    {
        pollingNum = g_sctpCb.genCfg.fdsPerThrPolling;
    }
    else
    {
        pollingNum = FDS_PER_THREAD_POLLING;  /*Ĭ��һ���̼߳���256 ��������*/
        g_sctpCb.genCfg.fdsPerThrPolling = pollingNum;
    }

    /*ȷ�������񼯵�����*/
    /*sctp cli tasks */
    g_sctpCb.sctpCliTskNo = (g_sctpCb.genCfg.maxSctpCliLink%pollingNum)
        ?(g_sctpCb.genCfg.maxSctpCliLink/pollingNum + 1)
        :(g_sctpCb.genCfg.maxSctpCliLink/pollingNum);
    /*��ȫ�Լ��:sctpcli  ���õ����񳬹����������,˵��ͨ�����ò�����*/
    if(g_sctpCb.sctpCliTskNo > MAX_SCTP_CLI_POLL_THREAD)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_genCfgProc()->gen cfg is not reasonable!");
        return XERROR;
    }

    if(g_sctpCb.sctpCliTskNo > 0)
    {
        g_sctpCb.pSctpCliTsk =
            (t_NTLTSKINFO*)XOS_MemMalloc(FID_NTL,sizeof(t_NTLTSKINFO)*(g_sctpCb.sctpCliTskNo));

        if(g_sctpCb.pSctpCliTsk)
        {
            memset((char*)g_sctpCb.pSctpCliTsk, 0, (XS32)sizeof(t_NTLTSKINFO)*(g_sctpCb.sctpCliTskNo));    
        }
        else
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->malloc SctpCliTsk space failed!");
            goto genCfgError;
        }
    }
    
    /* sctp client  link �����������*/
    if (g_sctpCb.sctpCliTskNo > 0 )
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

        /*��������*/
        for (i=0; i<g_sctpCb.sctpCliTskNo; i++)
        {
            /*����������ź���*/
            ret = XOS_SemCreate(&(g_sctpCb.pSctpCliTsk[i].taskSemp), 0);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctpCli thread[%d]  semaphore  failed!",i);
                goto genCfgError;
            }           

            /* fd read write set ������*/
            ret = XOS_MutexCreate(&(g_sctpCb.pSctpCliTsk[i].setInfo.fdSetMutex));
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctp cli  thread fdset mutex[%d]  failed!",i);
                goto genCfgError;
            }

            /*��ʼ�� set*/
            XOS_INET_FD_ZERO(&(g_sctpCb.pSctpCliTsk[i].setInfo.readSet.fdSet));
            XOS_INET_FD_ZERO(&(g_sctpCb.pSctpCliTsk[i].setInfo.writeSet.fdSet));

            /*����sctp client����*/
            XOS_MemSet(taskName,0,NTL_TSK_NAME_LEN);
            XOS_Sprintf(taskName, sizeof(taskName)-1, "Tsk_sctpc%d", i);
            ret = XOS_TaskCreate(taskName,TSK_PRIO_NORMAL,NTL_RECV_TSK_STACK_SIZE,(os_taskfunc)SCTP_cliRcvFunc,
                (XVOID *)(XPOINT)i,&(g_sctpCb.pSctpCliTsk[i].taskId));
            if (ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctpCli thread[%d]  failed!",i);
                goto genCfgError;
            }
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
        hashElems = (g_sctpCb.genCfg.maxSctpServLink)*g_sctpCb.genCfg.sctpClientsPerServ;
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

        /*����������ź���*/
        ret = XOS_SemCreate(&(g_sctpCb.sctpServTsk.taskSemp),0);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctpServTsk thread entry semaphore  failed!");
            goto genCfgError;
        }        

        /*read set ������*/
        ret = XOS_MutexCreate(&(g_sctpCb.sctpServTsk.setInfo.fdSetMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctp serv thread mutex failed!");
            goto genCfgError;
        }

        /*��ʼ�� set*/
        XOS_INET_FD_ZERO(&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));
        XOS_INET_FD_ZERO(&(g_sctpCb.sctpServTsk.setInfo.writeSet.fdSet));

        /*����sctp server ������*/
        XOS_Sprintf(taskName, sizeof(taskName)-1, "Tsk_sctps");
        ret = XOS_TaskCreate(taskName,TSK_PRIO_NORMAL,NTL_RECV_TSK_STACK_SIZE,(os_taskfunc)SCTP_servTsk,
            (XVOID *)0,&(g_sctpCb.sctpServTsk.taskId));
    }
    
    g_sctpCb.isGenCfg = XTRUE;
    return XSUCC;

genCfgError:
    /*�ͷ���Դ*/
    XOS_ArrayDestruct(g_sctpCb.sctpClientLinkH);
    XOS_ArrayDestruct(g_sctpCb.sctpServerLinkH);
    XOS_HashDestruct(g_sctpCb.tSctpCliH);

    for(i=0; i<g_sctpCb.sctpCliTskNo; i++)
    {
        XOS_TaskDel(g_sctpCb.pSctpCliTsk[i].taskId);
        XOS_SemDelete(&(g_sctpCb.pSctpCliTsk[i].taskSemp));
        XOS_MutexDelete(&(g_sctpCb.pSctpCliTsk[i].setInfo.fdSetMutex));
    }
    
    for(i=0; i<g_sctpCb.sctpCliTskNo; i++)
    {
        XOS_TaskDel(g_sctpCb.pSctpCliTsk[i].taskId);
        XOS_SemDelete(&(g_sctpCb.pSctpCliTsk[i].taskSemp));
        XOS_MutexDelete(&(g_sctpCb.pSctpCliTsk[i].setInfo.fdSetMutex));
    }
    
    XOS_TaskDel(g_sctpCb.sctpServTsk.taskId);
    XOS_MutexDelete(&(g_sctpCb.hashMutex));
    XOS_SemDelete(&(g_sctpCb.sctpServTsk.taskSemp));
    XOS_MutexDelete(&(g_sctpCb.sctpServTsk.setInfo.fdSetMutex));

    if(g_sctpCb.pSctpCliTsk != XNULLP)
    {
        XOS_MemFree(FID_NTL, g_sctpCb.pSctpCliTsk);
    }
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
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_DeleteCB()->unknown link type coming!");
        break;
    }

    if(XSUCC != nRst)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_DeleteCB()->delete failed");
    }
    return nRst;
}

/************************************************************************
������:SCTP_servStart
����:  ������·������Ϣ
����:pMsg ����Ϣָ��
���:pStartAck ������ȷ����Ϣ
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_servStart(t_XOSCOMMHEAD* pMsg,t_SCTPSTARTACK* pStartAck)
{
    t_SCTPSERSTART *pSctpServStart = NULL;
    t_LINKSTART *pLinkStart = NULL;
    t_SSCB *pServCb = NULL;
    XCHAR szLinkStateName[TEMP_STRING_LEN] = {0};
    XS32 ret = 0;
    XU32 stream = 0;
    if(!pMsg || !pStartAck)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart() pMsg or startAck is null!");
        return XERROR;
    }
    
    pLinkStart = (t_LINKSTART*)(pMsg->message);
    if(!pLinkStart)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->pLinkStart is null!");
        return XERROR;
    }
    
    pSctpServStart = &(pLinkStart->linkStart.sctpServerStart);

    /*��ȡsctp server ���ƿ�*/
    pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
    
    if(pServCb == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->get sctp serv control block failed!");
        return XERROR;
    }

    /*�����·״̬����ز���*/
    if((pServCb->linkState != eStateInited )
        ||(pSctpServStart->allownClients == 0)
        ||(pSctpServStart->allownClients > 
        (XU32)((g_sctpCb.genCfg.maxSctpServLink)*g_sctpCb.genCfg.sctpClientsPerServ)))
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->link state [%s]  error or allownClients[%d] error!",
            NTL_getLinkStateName(pServCb->linkState,szLinkStateName,sizeof(szLinkStateName)-1),pSctpServStart->allownClients);
        goto errorProc;
    }

    /*���״̬��ȷ����������ϴ����ǹر���*/
    SCTP_CloseServerSocket(pServCb);    

    if(pSctpServStart->streamNum > DEFAULT_SCTP_STREAM && pSctpServStart->streamNum <= SCTP_MAX_STREAM)
    {
        stream = pSctpServStart->streamNum;
    }
    else
    {
        stream = DEFAULT_SCTP_STREAM;
    }
    /*����sockect*/
    ret = XINET_SctpSocket(&(pServCb->sockFd),stream);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->create sockFd failed");
        goto errorProc;
    }

    /*�󶨶˿�*/
    ret =  XINET_SctpBind(&(pServCb->sockFd),&(pSctpServStart->myAddr),SCTP_BIND_ADD);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->XINET Bind sockFd failed");
        goto errorProc;
    }
    /*ȷ�����˵�ַ*/
    XINET_GetSctpSockName(&(pServCb->sockFd),&(pServCb->myAddr));

    /*listen*/
    ret = XINET_Listen(&(pServCb->sockFd),MAX_BACK_LOG);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->XINET_Listen sockFd %d failed",pServCb->sockFd.fd);
        goto errorProc;
    }

    /*��ӵ�read ����*/
    if(g_sctpCb.sctpServTsk.setInfo.sockNum++ == 0)
    {
        XOS_INET_FD_SET(&(pServCb->sockFd),&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));
        XOS_SemPut(&(g_sctpCb.sctpServTsk.taskSemp));
    }
    else
    {
        XOS_INET_FD_SET(&(pServCb->sockFd),&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));
    }

    /*��д����*/
    pServCb->maxCliNum = pSctpServStart->allownClients;
    pServCb->usageNum = 0;
    pServCb->linkState = eStateListening;
    pServCb->pLatestCli = (t_SSCLI*)XNULLP;
    pServCb->maxStream = (XU16)stream;
    pServCb->authFunc = pSctpServStart->authenFunc;
    pServCb->pParam = pSctpServStart->pParam;
    if(pSctpServStart->hbInterval >= SCTP_MIN_HB_INTERVAL )
    {
        pServCb->hbInterval = pSctpServStart->hbInterval;
    }
    else
    {
        pServCb->hbInterval = SCTP_DEFAULT_HB_INTERVAL;
    }
    if(pSctpServStart->pathmaxrxt >= SCTP_PATH_RETRANS_MIN && pSctpServStart->pathmaxrxt <= SCTP_PATH_RETRANS_MAX)
    {
        pServCb->pathmaxrxt = pSctpServStart->pathmaxrxt;
    }
    else
    {
        pServCb->pathmaxrxt = SCTP_PATH_RETRANS_DEFAULT;
    }
    /*��д�������*/
    pStartAck->appHandle = pServCb->userHandle;
    pStartAck->linkStartResult = eSUCC;
    XOS_MemCpy(&(pStartAck->localAddr),&(pServCb->myAddr),sizeof(t_SCTPIPADDR));
    return XSUCC;

errorProc:
    {
        /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
        pStartAck->appHandle = pServCb->userHandle;
        pStartAck->linkStartResult = eFAIL;
        return XSUCC;
    }
}

/************************************************************************
������:SCTP_StartClientTimer
����:  ����sctp�ͻ��˵�������ʱ��
����:  pSctpCb ��sctp���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_StartClientTimer(t_SCCB *pSctpCb, XU32 taskNo)
{
    t_PARA timerPara;
    t_BACKPARA backPara;
    XS32 ret = 0;

    if(!pSctpCb)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_StartClientTimer()->pSctpCb is null");
        return XERROR;
    }
    
    XOS_MemSet(&timerPara,0,sizeof(t_PARA));
    XOS_MemSet(&backPara,0,sizeof(t_BACKPARA));

    timerPara.fid  = FID_NTL;
    timerPara.len  = SCTP_CLI_RECONNECT_INTERVAL;
    timerPara.mode = TIMER_TYPE_LOOP;
    timerPara.pre  = TIMER_PRE_LOW;
    backPara.para1 = (XPOINT)pSctpCb->linkHandle;
    backPara.para2 = (XPOINT)taskNo;
    backPara.para3 = (XPOINT)eSCTPReconnect;
    XOS_INIT_THDLE (pSctpCb->timerId);    

    pSctpCb->expireTimes = 0;

    ret =  XOS_TimerStart(&(pSctpCb->timerId), &timerPara, &backPara);

    return ret;
}

          
/************************************************************************
������:SCTP_SetClientFd
����:  sctp client select ��λ����
����:  pSctpCb ����Ϣָ��
       fdFlg  --0:��,1:д, 2:����д
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/            
void SCTP_SetClientFd(t_SCCB *pSctpCb, XU32 taskNo, e_ADDSETFLAG fdFlg)
{
    if(!pSctpCb)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_SetClientFd()->pSctpCb is null");
        return;
    }
    
    switch(fdFlg)
    {
    case eRead:
        XOS_INET_FD_SET(&(pSctpCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet));
        break;
        
    case eWrite:
        XOS_INET_FD_SET(&(pSctpCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.writeSet.fdSet));
        break;
        
    case eReadWrite:
        XOS_INET_FD_SET(&(pSctpCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet));
        XOS_INET_FD_SET(&(pSctpCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.writeSet.fdSet));            
        break;
        
    default:
        break;
    }

    if(g_sctpCb.pSctpCliTsk[taskNo].setInfo.sockNum++ == 0)
    {
        XOS_SemPut(&(g_sctpCb.pSctpCliTsk[taskNo].taskSemp));
    }
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
                pCliCb->sockFd.fd = XOS_INET_INV_SOCKFD;    
                memset((char*)&(pCliCb->packetlist), 0, sizeof(t_SctpResndPacket));
                XOS_INIT_THDLE(pCliCb->timerId);
                pCliCb->expireTimes = 0;
                
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
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_linkInitProc()->add the sctp server cb was reset!");
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
                pSctpSrvCb->sockFd.fd = XOS_INET_INV_SOCKFD;    
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
    XU32 taskNo = 0;
    XU16 stream = 0;
    XU32 hbInterval = 0;
    XU16 pathmaxrxt = 0;
    XCHAR szTemp[TEMP_STRING_LEN] = {0};
    XCHAR szLinkStateName[TEMP_STRING_LEN] = {0};
    XU32 optVal;

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

    /*��ȡ��·����*/
    linkType = NTL_getLinkType(pLinkStart->linkHandle);
    XOS_MemSet(&startAck,0,sizeof(t_SCTPSTARTACK));
    switch (linkType)
    {
    case eSCTPClient:
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

        /*��ȡ��·���������*/
        taskNo = (XU32)NTL_getLinkIndex(pLinkStart->linkHandle)/(g_sctpCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_sctpCb.sctpCliTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] sctp client taskNo %d is invalid!",pSctpCb->linkUser.FID, taskNo);
            startAck.appHandle = pSctpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
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

           /*״̬��ȷ��fdȴ�򿪣���Ҫ�رգ��������ϴ����ǹر���*/
        if (pSctpCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] sctp client forget close old fd!",pSctpCb->linkUser.FID);
            SCTP_closeCli(taskNo, pSctpCb, NTL_SHTDWN_SEND);
        }     

        if(pSctpCliStart->streamNum > DEFAULT_SCTP_STREAM && pSctpCliStart->streamNum <= SCTP_MAX_STREAM)
        {
            stream = pSctpCliStart->streamNum;
        }
        else
        {
            stream = DEFAULT_SCTP_STREAM;
        }
        pSctpCb->maxStream = stream;
        /*��������sock*/
        ret = XINET_SctpSocket(&(pSctpCb->sockFd),stream);
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] sctp client socket failed!",pSctpCb->linkUser.FID);
            /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
            startAck.appHandle = pSctpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
        }

        optVal = XOS_INET_OPT_ENABLE;
        XINET_SetOpt(&(pSctpCb->sockFd), XOS_INET_LEVEL_SOCKET, XOS_INET_OPT_LINGER, &optVal);

        if(pSctpCliStart->hbInterval >= SCTP_MIN_HB_INTERVAL )
        {
            hbInterval = pSctpCliStart->hbInterval;
        }
        else
        {
            hbInterval = SCTP_DEFAULT_HB_INTERVAL;
        }
        pSctpCb->hbInterval = hbInterval;
        
        XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_linkStartProc()->set heartbeat:%d!",hbInterval);

        ret = XINET_SetOpt( &(pSctpCb->sockFd), XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_HB,&hbInterval);
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] sctp client set heartbeat failed!",pSctpCb->linkUser.FID);
            startAck.appHandle = pSctpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
        }
        
        if(pSctpCliStart->pathmaxrxt >= SCTP_PATH_RETRANS_MIN && pSctpCliStart->pathmaxrxt <= SCTP_PATH_RETRANS_MAX)
        {
            pathmaxrxt = pSctpCliStart->pathmaxrxt;
        }
        else
        {
            pathmaxrxt = SCTP_PATH_RETRANS_DEFAULT;
        }
        pSctpCb->pathmaxrxt = pathmaxrxt;

        /*����sctp �ͻ���,��Ҫ����bind*/
        ret =  XINET_SctpBind(&(pSctpCb->sockFd),&(pSctpCliStart->myAddr),SCTP_BIND_ADD);
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_linkStartProc()->FID[%d] sctp client bind failed!",pSctpCb->linkUser.FID);
            startAck.appHandle = pSctpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
        }

        XINET_GetSctpSockName(&(pSctpCb->sockFd),&(pSctpCb->myAddr));
        XOS_MemCpy(&(pSctpCb->peerAddr),&(pSctpCliStart->peerAddr),sizeof(t_SCTPIPADDR));        

        /*���ӵ��Զ�*/
        ret = XINET_SctpConnect(&(pSctpCb->sockFd),&(pSctpCliStart->peerAddr),NULL);
        if(ret == XSUCC)
        {
            ret = SCTP_Opt(pSctpCb, pSctpCliStart->peerAddr.ipNum);
            if(ret == XERROR)
            {
                XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
                return XERROR;
            }
            /*���������ӳɹ��Ŀ����Ժ�С��Ҫ�����ӱ����Ķ˿ڣ�һ���ǿ������ӳɹ���*/
            pSctpCb->linkState = eStateConnected;
            SCTP_StopClientTimer(pSctpCb);

            /*��ӵ�readset��*/
            SCTP_SetClientFd(pSctpCb, taskNo, eRead);

            /*��дstartAck �ظ���Ϣ,���ؽ��Ϊ�ɹ�*/
            startAck.appHandle = pSctpCb->userHandle;
            XOS_MemCpy(&(startAck.localAddr), &(pSctpCb->myAddr), sizeof(t_SCTPIPADDR));
            startAck.linkStartResult = eSUCC;
        }
        else
        {
            pSctpCb->linkState = eStateConnecting;
            XOS_IptoStr(pSctpCliStart->peerAddr.ip[0],szTemp);
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_linkStartProc(),FID[%d] sctp client the %dth connect[%s:%d] failed ,start timer for pSctpCb[0x%x] reconnect!",
                pSctpCb->linkUser.FID,pSctpCb->expireTimes ,szTemp,pSctpCliStart->peerAddr.port,pSctpCb);

            /*������Ӳ��ɹ�,��������,������ʱ��,��֤��·����*/            
            if(XSUCC != SCTP_StartClientTimer(pSctpCb, taskNo))
            {
                /*��ʱ������ʧ�ܴ�������*/
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] sctp client start timer failed!",pSctpCb->linkUser.FID);
                startAck.appHandle = pSctpCb->userHandle;
                startAck.linkStartResult = eFAIL;
                XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
                break;
            }

            /*��ӵ�writeset��*/
            SCTP_SetClientFd(pSctpCb, taskNo, eWrite);
            
            startAck.appHandle = pSctpCb->userHandle;
            XOS_MemCpy(&(startAck.localAddr),&(pSctpCb->myAddr),sizeof(t_SCTPIPADDR));
            startAck.linkStartResult = eBlockWait;            
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        break;

    case eSCTPServer:
        /*sctp server �������Ƚϸ��ӣ�����һ����������*/
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        ret = SCTP_servStart( pMsg,&startAck);
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        if(ret == XERROR)
        {
            /*���ش���Ͳ��ûظ�start Ack ��Ϣ*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->start sctp server  failed!");
            return XERROR;
        }
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
������:SCTP_dataReqTimerProc
����:  ��ʱ����Ϣ�е�sctp��֧�����������ڶ�ʱ�ط�����
����:  t_BACKPARA* pParam  ��ʱ��Ϣ��ַָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS8 SCTP_timerProc(t_BACKPARA* pParam)
{
    t_SCCB *pSctpCb = NULL;
    XS32 ret = 0;
    XU32 taskNo = 0;
    XCHAR szTemp[TEMP_STRING_LEN] = {0};
    XCHAR szLocalTemp[TEMP_STRING_LEN] = {0};
    XPOINT timer_src[2] = {0};
    t_STARTACK sctpStartAck;
    XU32 optVal= 0;     
    XU32 peerInstream;

    if(pParam == (t_BACKPARA*)XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc()->timer PTIMER is null ,bad input param!");
        return XERROR;
    }

    timer_src[0] = (XPOINT)pParam->para3;
    timer_src[1] = (XPOINT)pParam->para4;

    /*���Ϳͻ��˻������е�����*/
    if((timer_src[0] ==  eSCTPCliResendTimer) && (timer_src[1] == eSCTPCliResendCheck))
    {
        SCTP_dataReqTimerSend(eSCTPClient);
        return XSUCC;
    }

    /*���ͷ���˻������е�����*/
    if((timer_src[0] ==  eSCTPSerResendTimer) && (timer_src[1] == eSCTPSerResendCheck))
    {
        SCTP_dataReqTimerSend(eSCTPServer);
        return XSUCC;
    }

    /*����sctp�ͻ��˵�����*/
    pSctpCb = (t_SCCB*)XNULLP;

    pSctpCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,NTL_getLinkIndex((HLINKHANDLE)pParam->para1));
    if((t_SCCB*)XNULLP == pSctpCb)
    {
        return XERROR;
    }
    XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
    /*�жϴ�Ԫ���ڶ������Ƿ����ڿ��У���������ٴ���*/
    if(XFALSE == XOS_ArrayIsUesd(g_sctpCb.sctpClientLinkH, pSctpCb)) 
    {
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc()->pSctpCb is not used again");
        return XERROR;
    }
    XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
    
    taskNo = (XU32)(XPOINT)pParam->para2;
    if(taskNo >= (XU32)g_sctpCb.sctpCliTskNo)
    {
        return XERROR;
    }   

    /*�����·״̬*/
    switch(pSctpCb->linkState)
    {
    case  eStateInited:
        if (pSctpCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc()->pSctpCb->sockFd.fd is %d,state is eStateInited!",pSctpCb->sockFd.fd);
            if (XSUCC != SCTP_closeCli(taskNo, pSctpCb, NTL_SHTDWN_SEND))
            {
                return XERROR;
            }
        }    
         
        /*rebuild a new socket*/
        ret = XINET_SctpSocket(&(pSctpCb->sockFd),pSctpCb->maxStream);
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc()->XINET_Socket failed");
            break;
        }
        
        optVal = XOS_INET_OPT_ENABLE;
        ret = XINET_SetOpt(&pSctpCb->sockFd, XOS_INET_LEVEL_SOCKET, XOS_INET_OPT_LINGER, &optVal);            
        if( ret != XSUCC )
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc()->FID[%d] set SO_LINGER for socket failed!",pSctpCb->linkUser.FID);
            break;
        }

        ret = XINET_SetOpt( &(pSctpCb->sockFd), XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_HB,&(pSctpCb->hbInterval));
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc()->FID[%d] sctp client set heartbeat failed!",pSctpCb->linkUser.FID);
            break;
        }
        /*���԰󶨣�������ñ��˵�ַ���Ҷ˿ڲ���ͻ��Ӧ�ð󶨳ɹ�*/
        ret = XINET_SctpBind(&(pSctpCb->sockFd),&(pSctpCb->myAddr),SCTP_BIND_ADD);
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc()->XINET_Bind[%d] failed", pSctpCb->sockFd.fd);
            break;
        }
        XINET_GetSctpSockName(&(pSctpCb->sockFd),&(pSctpCb->myAddr));
        if(g_ntltraceswitch)
        {
            XOS_IptoStr(pSctpCb->peerAddr.ip[0],szTemp);
            XOS_IptoStr(pSctpCb->myAddr.ip[0],szLocalTemp);
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_timerProc(),FID[%d] pSctpCb[0x%x] sctp client sock %d connectting.",
                pSctpCb->linkUser.FID,pSctpCb,pSctpCb->sockFd.fd);
            XOS_Trace(MD(FID_NTL,PL_INFO),"the %dth connecting local[%s:%d] => remote[%s:%d].",
                pSctpCb->expireTimes,szLocalTemp,pSctpCb->myAddr.port,szTemp,pSctpCb->peerAddr.port);
        }
        pSctpCb->expireTimes++;
        ret = XINET_SctpConnect(&(pSctpCb->sockFd),&(pSctpCb->peerAddr),NULL);
        if(ret != XSUCC)
        {
           pSctpCb->linkState = eStateConnecting;
            
            SCTP_SetClientFd(pSctpCb, taskNo, eWrite); 
        }
        else
        {
            ret = XINET_GetOpt(&(pSctpCb->sockFd), XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_PEER_INSTREAM, &peerInstream);
            if( ret != XSUCC || pSctpCb->maxStream > peerInstream )
            {
                /*�ر��½����sock*/
                XINET_CloseSock(&(pSctpCb->sockFd));
                
                /*ֹͣ��ʱ��*/
                SCTP_StopClientTimer(pSctpCb);
                
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc,new client(ip[0x%x],port[%d]) instream is %d,local outstream is %d!",
                    XOS_INET_NTOH_U32(pSctpCb->peerAddr.ip[0]),XOS_INET_NTOH_U16(pSctpCb->peerAddr.port),peerInstream,pSctpCb->maxStream);
                break;
            }

            pSctpCb->linkState = eStateConnected;

            /*ֹͣ��ʱ��*/
            SCTP_StopClientTimer(pSctpCb);
                            
            /*��ӵ�readset��*/
            SCTP_SetClientFd(pSctpCb, taskNo, eRead);

            /*���������ɹ���Ϣ���ϲ�*/
            sctpStartAck.appHandle = pSctpCb->userHandle;
            sctpStartAck.linkStartResult = eSUCC;
            XOS_MemCpy(&(sctpStartAck.localAddr),&(pSctpCb->myAddr),sizeof(t_IPADDR));
            NTL_msgToUser((XVOID*)&sctpStartAck,&(pSctpCb->linkUser),sizeof(t_STARTACK),eSctpStartAck);
        }
        break;

    case eStateConnecting:
        pSctpCb->expireTimes ++;
        if(pSctpCb->expireTimes >= SCTP_CLI_RECONNEC_TIMES)
        {
            SCTP_closeCli((taskNo), pSctpCb, NTL_SHTDWN_SEND);
            pSctpCb->linkState = eStateInited;
            pSctpCb->expireTimes = 0;
        }
        break;

    case eStateConnected:
        /*�Ѿ������ϣ��ص���ʱ��*/
        XOS_IptoStr(pSctpCb->peerAddr.ip[0], szTemp);
        XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_timerProc(),FID[%d] sctp client  pSctpCb[0x%x] the %dth connect[%s:%d] successed!",
            pSctpCb->linkUser.FID,pSctpCb,pSctpCb->expireTimes ,szTemp,pSctpCb->peerAddr.port);

        /*ֹͣ��ʱ��*/
        SCTP_StopClientTimer(pSctpCb);

        break;

    default:
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc()->expire msg in bad state!");
        break;
    }
    return XSUCC;
}

/************************************************************************
������:SCTP_dataReqSendProc
����:  �����Ŷ�����
����:  t_XINETFD *pSockFd  - socket���
        t_SctpResndPacket *pPacklist  - ӵ���Ŷ�����
        t_SCTPIPADDR *pDstAddr      - ���ݷ���Ŀ�ĵ�ַ
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_dataReqSendProc(t_XINETFD *pSockFd,t_SctpResndPacket *pPacklist,t_SCTPIPADDR *pDstAddr)
{
    t_SctpResnd *loopPacket = XNULL;
    t_SctpResnd *pdelPacket = XNULL;
    XU32 total_packet=0,success_packet=0,fail_packet=0;
    XS32 ret = 0;

    if ((pSockFd == XNULLP) || XOS_INET_INV_SOCK_FD(pSockFd) || XNULL == pPacklist )
    {
        return(XERROR);
    }
    loopPacket   = pPacklist->pFirstDataReq;
    total_packet = pPacklist->rsnd_size;
    if((total_packet >0) && (loopPacket == XNULL))
    {
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL DataReq ReSendProc(),unbelievable error,packlist is destroyed.");
        }
        return XERROR;
    }
    while(loopPacket != XNULL)
    {
        pdelPacket= loopPacket;
        loopPacket = loopPacket->pNextPacket;
        if(XNULL == pdelPacket->pData || pdelPacket->msgLenth ==0 )
        {
            if(g_ntltraceswitch)
            {
                if(XNULL == pdelPacket->pData)
                {
                    XOS_Trace(MD(FID_NTL,PL_INFO),"NTL DataReq ReSendProc() msg is null.");
                }
                if(pdelPacket->msgLenth ==0)
                {
                    XOS_Trace(MD(FID_NTL,PL_INFO),"NTL DataReq ReSendProc() msg len is 0.");
                }
            }
            if(pdelPacket->pData)
            {
                XOS_MemFree(FID_NTL,(XVOID*)(pdelPacket->pData));
            }
            XOS_MemFree((XU32)FID_NTL,pdelPacket);
            pPacklist->rsnd_size--;
            pPacklist->rsnd_delete++;
            pPacklist->pFirstDataReq=loopPacket;
            continue;
        }
        ret =XINET_SctpSendMsg(pSockFd,pdelPacket->pData,(XS32)pdelPacket->msgLenth,pDstAddr,pdelPacket->attr);
        if(ret != XSUCC)
        {
            pPacklist->rsnd_fail++;
            fail_packet++;

            if(g_ntltraceswitch)
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"resend packet size %d,success %d,failed %d,unsend %d",
                    total_packet,success_packet,fail_packet,pPacklist->rsnd_size);
            }
            return XERROR;/*send failed should return.*/
        }
        else
        {
            /*success clean up packet*/
            XOS_MemFree(FID_NTL,(XVOID*)(pdelPacket->pData));
            XOS_MemFree(FID_NTL,pdelPacket);

            pPacklist->rsnd_size--;
            pPacklist->rsnd_success++;
            success_packet++;
            pPacklist->pFirstDataReq=loopPacket;
        }
    }
    if(g_ntltraceswitch)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"resend packet size %d,success %d,failed %d,unsend %d\r\n",
            total_packet,success_packet,fail_packet,pPacklist->rsnd_size);
    }

    return XSUCC;
}

/************************************************************************
������:SCTP_dataReqTimerSend
����:  ��ʱ����Ϣ�е�sctp��֧�����������ڶ�ʱ�ط�����
����:  
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_dataReqTimerSend(e_LINKTYPE type)
{
    t_XOSCOMMHEAD *msgToNtl = XNULL;
    XU32 len = 4;
    XS32 ret = 0;

    if(XFALSE==g_ntl_timer)
    {
        return XSUCC;
    }
    msgToNtl = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_NTL,len);
    if ( XNULL == msgToNtl )
    {
        return XERROR;
    }
    msgToNtl->datasrc.PID = XOS_GetLocalPID();
    msgToNtl->datasrc.FID = (XU32)FID_NTL;
    msgToNtl->length = len;
    if(type == eSCTPClient)
    {
        msgToNtl->msgID = eSCTPCliResendTimer;
    }
    else
    {
        msgToNtl->msgID = eSCTPSerResendTimer;
    }
    msgToNtl->subID= type;

    msgToNtl->prio = eNormalMsgPrio;
    msgToNtl->datadest.PID = XOS_GetLocalPID();
    msgToNtl->datadest.FID = (XU32)FID_NTL;
    XOS_MemCpy(msgToNtl->message, &len, (XS32)len);
    /*��������*/
    ret = XOS_MsgSend(msgToNtl);
    if(ret != XSUCC)
    {
        /*������Ϣ����Ӧ�ý���Ϣ�ڴ��ͷ�*/
        XOS_MsgMemFree((XU32)FID_NTL, msgToNtl);
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
������:SCTP_dataReqTimerProc
����:  ��ʱ����Ϣ�е�sctp��֧�����������ڶ�ʱ�ط�����
����:  t_XOSCOMMHEAD *pMsg  ��Ϣ��ַָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_dataReqTimerProc(t_XOSCOMMHEAD *pMsg)
{
    t_SCCB *pCliCb = XNULL;
    t_SSCB *pSctpSrvCb = XNULL;
    t_SSCLI *pTsClient = XNULL;
    XS32 i = 0;  
    XS32 j = 0;
    XS32 nIndex = 0;

    if(XFALSE==g_sctp_timer)
    {
        return XSUCC;
    }
    if(!pMsg)
    {
        return XERROR;
    }
    
    switch(pMsg->subID)
    {
    case eSCTPClient:
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqProcTimerProc eSCTPClient comming");
        }
        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
        nIndex = XOS_ArrayGetFirstPos(g_sctpCb.sctpClientLinkH);
        for(i=nIndex; i>=0; i=XOS_ArrayGetNextPos(g_sctpCb.sctpClientLinkH,i))
        {
            pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,i);
            if(pCliCb == XNULLP)
            {
                XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
                return XERROR;
            }
            /*�ȼ����·��״̬*/
            if (pCliCb->linkState != eStateConnected)
            {
                XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
                return XERROR;
            }
            if(pCliCb->packetlist.rsnd_size >0)
            {
                SCTP_dataReqSendProc(&(pCliCb->sockFd),&(pCliCb->packetlist),NULL);
            }
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        break;

    case eSCTPServer:
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqProcTimerProc eSCTPServer comming");
        }
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        nIndex = XOS_ArrayGetFirstPos(g_sctpCb.sctpServerLinkH);
        for(i=nIndex; i>=0; i=XOS_ArrayGetNextPos(g_sctpCb.sctpServerLinkH,i))
        {
            pSctpSrvCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,i);
            if(pSctpSrvCb == XNULLP)
            {
                XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
                return XERROR;
            }

            if(pSctpSrvCb->usageNum > 0)
            {
                pTsClient = pSctpSrvCb->pLatestCli;
                if(XNULLP == pTsClient )
                {
                    XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
                    return XERROR;
                }
                for(j=0; j<pSctpSrvCb->usageNum; j++)
                {
                    XOS_HashGetKeyByElem(g_sctpCb.tSctpCliH,pTsClient);
                    if(pTsClient == XNULLP)
                    {
                        break;
                    }

                    if(pTsClient->packetlist.rsnd_size >0)
                    {
                        SCTP_dataReqSendProc(&(pTsClient->sockFd),&(pTsClient->packetlist),NULL);
                    }
                    pTsClient = pTsClient->pPreCli;
                }
            }
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        break;

    default:
        break;
    }

    return XSUCC;
}


/************************************************************************
������:SCTP_dataReqTimerProc
����:  ���ڷ���sctp����
����:  t_XINETFD *pSockFd          - socket���
    :  t_SctpResndPacket* pklist    -ӵ������ָ��
    :  t_SCTPIPADDR *pIpAddr        - ��ϢĿ�ĵ�ַ
    :  t_SCTPDATAREQ *pDataReq      -��Ϣ��������
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_dataReqtpktProc(t_XINETFD *pSockFd,t_SctpResndPacket* pklist,t_SCTPIPADDR *pIpAddr,t_SCTPDATAREQ *pDataReq)
{
    XS32 ret = 0;
    if(pSockFd == XNULL || pklist == XNULL || pDataReq == XNULL)
    {
        return XERROR;
    }
    if(XNULL == pDataReq->pData || pDataReq->msgLenth == 0)
    {
        return XERROR;
    }
#ifdef XOS_NEED_CHK
    pklist->rsnd_total++;
    if(pklist->rsnd_size ==0)
    {
        /*û���Ŷ�,��������*/
        ret = XINET_SctpSendMsg(pSockFd, pDataReq->pData, (XS32)pDataReq->msgLenth, NULL,pDataReq->attr);
        if(ret != XSUCC)
        {
            /*��������������������,�Ŷ�*/
            if(XSUCC == SCTP_dataReqSaveProc(pklist,pDataReq))
            {
                if(g_ntltraceswitch)
                {
                    XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqtpktProc,resend wait packet size %d",pklist->rsnd_size);
                }
                /*�Ŷӳɹ�*/
                return XSUCC;
            }
            /*����*/
            pklist->rsnd_delete++;
            /*��Ϊ��ͳ�Ƽ���,���в�֪ͨ�û�����ʧ��*/
            /*�Ŷ�ʧ��,�����ݱ�����*/
            if(pDataReq->pData != XNULLP)
            {
                XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
            }
            pDataReq->pData=NULL;
            pDataReq->msgLenth=0;
            return XERROR;
        }
        else
        {
            pklist->rsnd_success++;
            /*���ͳɹ���������ݰ�*/
            if(pDataReq->pData != XNULLP)
            {
                XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
            }
            pDataReq->pData=NULL;
            pDataReq->msgLenth=0;
            return XSUCC;
        }
    }
    else
    {
        /*���Ŷ�*/
        if(XSUCC == SCTP_dataReqSaveProc(pklist,pDataReq))
        {
            if(g_ntltraceswitch)
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqtpktProc,resend wait packet size %d",pklist->rsnd_size);
            }
            /*�ط������ɹ��ȴ�,ֱ���ɹ�ɾ��*/
            SCTP_dataReqSendProc(pSockFd,pklist,NULL);
            return XSUCC;
        }
        /*����*/
        pklist->rsnd_delete++;
        /*�Ŷ�ʧ��,��Ϊ��ͳ�Ƽ���,���в�֪ͨ�û�����ʧ��*/
        /*�Ŷ�ʧ��,��pDataReq���ݰ�����*/
        if(pDataReq->pData != XNULLP)
        {
            XOS_MemFree(FID_NTL,(XVOID*)(pDataReq->pData));
        }
        pDataReq->pData=NULL;
        pDataReq->msgLenth=0;
        SCTP_dataReqSendProc(pSockFd,pklist,pIpAddr);
        return XERROR;
    }
#endif
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
    XU32 taskNo = 0;

#ifdef INPUT_PAR_CHECK
    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_dataReqProc()->input param pMsg is Null!");
        return XERROR;        
    }

    pDataReq = (t_SCTPDATAREQ*)(pMsg->message);
    if(XNULLP == pDataReq)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_dataReqProc()->input param pMsg is Null!");
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
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            return XERROR;
        }
        /*�ȼ����·��״̬*/
        if (pCliCb->linkState != eStateConnected)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorLinkState;
            sendError.userHandle = pCliCb->userHandle;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
        }
        
        /*��ȡ��·���������*/
        taskNo = (XU32)NTL_getLinkIndex(pDataReq->linkHandle)/(g_sctpCb.genCfg.fdsPerThrPolling);
        if(taskNo > (XU32)g_sctpCb.sctpCliTskNo)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eOtherErrorReason;
            sendError.userHandle = pCliCb->userHandle;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
        }
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_dataReqProc() stream:%d,maxstream:%d!",pDataReq->attr.stream , pCliCb->maxStream);
        if(pDataReq->attr.stream >= pCliCb->maxStream)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorStreamNum;
            sendError.userHandle = pCliCb->userHandle;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
        }
        /*��������*/
        ret = SCTP_dataReqtpktProc(&(pCliCb->sockFd), &(pCliCb->packetlist), NULL, pDataReq);
        if(ret != XSUCC && g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc sctp client call SCTP_dataReqProctpktProc failed!");
        }       
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        return XSUCC;

    case eSCTPServer:
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        pSctpSrvCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,linkIndex);
        
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
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
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
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            break;
        }
        XOS_Trace(MD(FID_NTL,PL_DBG),"SCTP_dataReqProc() stream:%d,maxstream:%d!",pDataReq->attr.stream , pSctpSrvCb->maxStream);
        if(pDataReq->attr.stream >= pSctpSrvCb->maxStream)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorStreamNum;
            sendError.userHandle = pSctpSrvCb->userHandle;
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
            break;
        }

        /*sctp ��װ*/
        ret=SCTP_dataReqtpktProc(&(pTsClient->sockFd),&(pTsClient->packetlist),NULL,pDataReq);
        if ( ret != XSUCC && g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_dataReqProc sctp server call SCTP_dataReqProctpktProc failed!");
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
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
������:SCTP_dataReqSaveProc
����: �洢����ʧ�ܵ����ݵ�ӵ������
����: t_SctpResndPacket *pPacklist  - ӵ������ָ��
        t_SCTPDATAREQ *pDataReq     - �������ݽṹ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��:
************************************************************************/
#ifdef XOS_NEED_CHK
XS32 SCTP_dataReqSaveProc(t_SctpResndPacket *pPacklist,t_SCTPDATAREQ *pDataReq)
{
    t_SctpResnd *ptmpPacket = NULL;
    t_SctpResnd *ploopPacket = NULL;
    if(XNULL == pDataReq)
    {
        return XERROR;
    }
    if((XNULL == (pDataReq->pData)) || (pDataReq->msgLenth ==0))
    {
        return XERROR;
    }

    if(pPacklist == XNULLP)
    {
        return XERROR;
    }
    if(pPacklist->rsnd_size >= XOS_RESEND_BUF_SIZE)
    {
        /*����ŶӶ�����,��������Ϣ������*/
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqSaveProc()-> resend size %d,queue is full!");
        }
        return XERROR;
    }
    if (XNULL ==  (ptmpPacket = (t_SctpResnd *)XOS_MemMalloc((XU32)FID_NTL,sizeof(t_SctpResnd))))
    {
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_dataReqSaveProc() XOS_MemMalloc failed!");
        }
        return XERROR;
    }
    /*���µ����ݰ��ӵ�������*/
    XOS_MemSet(ptmpPacket,0x0,sizeof(t_SctpResnd));
    ptmpPacket->pNextPacket=XNULL;
    /*�����ݰ���ַ������������*/
    ptmpPacket->pData=pDataReq->pData;
    ptmpPacket->msgLenth = pDataReq->msgLenth;
    ptmpPacket->attr.stream = pDataReq->attr.stream;
    ptmpPacket->attr.context = pDataReq->attr.context;
    ptmpPacket->attr.ppid = pDataReq->attr.ppid;

    /*������*/
    pPacklist->rsnd_wait++;
    if(pPacklist->pFirstDataReq == XNULL)
    {
        /*��Ϊͷ�ڵ�*/
        pPacklist->pFirstDataReq=ptmpPacket;
        pPacklist->rsnd_size++;
        return XSUCC;
    }

    ploopPacket = pPacklist->pFirstDataReq;
    while(ploopPacket ->pNextPacket != XNULL)
    {
        ploopPacket = ploopPacket->pNextPacket;
    }
    ploopPacket ->pNextPacket =ptmpPacket;
    pPacklist->rsnd_size++;
    return XSUCC;
}
#endif

/************************************************************************
������:SCTP_dataReqClear
����:  ���sctpӵ������
����:  t_SctpResndPacket* pklist    -ӵ������ָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:���ŶӶ��в����,ֱ�����ͳɹ�Ϊֹ
************************************************************************/
XS32 SCTP_dataReqClear(t_SctpResndPacket *pPacklist)
{
    t_SctpResnd *ploopPacket = XNULL;
    t_SctpResnd *pdelPacket = XNULL;

    if(pPacklist == XNULL)
    {
        return (XERROR);
    }
    ploopPacket = pPacklist->pFirstDataReq;

    if(g_ntltraceswitch)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqClear Clear begin size = %d",pPacklist->rsnd_size);
    }
    while(ploopPacket!= XNULLP)
    {
        pdelPacket= ploopPacket;
        ploopPacket = ploopPacket->pNextPacket;
        if(XNULL != pdelPacket->pData)
        {
            XOS_MemFree(FID_NTL,(XVOID*)(pdelPacket->pData));
        }
        pdelPacket->msgLenth=0;
        pdelPacket->attr.context = 0;
        pdelPacket->attr.ppid = 0;
        pdelPacket->attr.stream = 0;
        XOS_MemFree((XU32)FID_NTL,pdelPacket);
        pPacklist->rsnd_size--;
        pPacklist->rsnd_delete++;
        pPacklist->pFirstDataReq=ploopPacket;
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqClear Clear del one size = %d",pPacklist->rsnd_size);
        }
    }
    if(g_ntltraceswitch)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqClear Clear end size = %d",pPacklist->rsnd_size);
    }
    pPacklist->rsnd_size=0;
    pPacklist->rsnd_total=0;
    pPacklist->rsnd_wait=0;
    pPacklist->rsnd_success=0;
    pPacklist->rsnd_delete=0;
    pPacklist->rsnd_fail=0;
    pPacklist->pFirstDataReq = XNULL;
    return XSUCC;
}

/************************************************************************
������:SCTP_StopClientTimer
����:  ֹͣsctp�ͻ��˵Ķ�ʱ��
����:  pSctpCb ��sctp���ƿ�ָ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 SCTP_StopClientTimer(t_SCCB *pSctpCb)
{
    XS32 ret = 0;

    if(!pSctpCb)
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),"SCTP_StopClientTimer()->pSctpCb is null!");
        return XERROR;
    }
    
    /*ֹͣ��ʱ��*/
    if(pSctpCb->timerId == 0)
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"SCTP_StopClientTimer()->XOS_TimerStop pSctpCb->timerId 0!");
        return XERROR;    
    }
    
    ret = XOS_TimerStop(FID_NTL, pSctpCb->timerId);

    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),"SCTP_StopClientTimer()->XOS_TimerStop failed!");
    }

    XOS_INIT_THDLE(pSctpCb->timerId);
    
    pSctpCb->expireTimes = 0;
    
    return ret;    
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
    XU32 taskNo = 0;

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

        taskNo = (XU32)NTL_getLinkIndex(pCloseReq->linkHandle)/(g_sctpCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_sctpCb.sctpCliTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ResetLinkByReapply()->sctp client taskNo is %d", taskNo);
            return XERROR;    
        }

        /*�رն�ʱ��*/
        SCTP_StopClientTimer(pCliCb);

           /*���δ���͵����ݣ��ر�fd*/
        if (pCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            SCTP_closeCli(taskNo, pCliCb, NTL_SHTDWN_SEND);
        }

        /*�رպ��óɳ�ʼ��״̬*/
        memset(pCliCb->peerAddr.ip,0,sizeof(pCliCb->peerAddr.ip[0]) * SCTP_ADDR_NUM);
        pCliCb->peerAddr.port = 0;
        pCliCb->peerAddr.ipNum = 0;
        memset(pCliCb->myAddr.ip,0,sizeof(pCliCb->myAddr.ip[0]) * SCTP_ADDR_NUM);
        pCliCb->myAddr.port = 0;
        pCliCb->myAddr.ipNum = 0;
        pCliCb->linkState = eStateInited;
        pCliCb->sockFd.fd = XOS_INET_INV_SOCKFD;
        pCliCb->expireTimes = 0;

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
        pServCb->authFunc = NULL;
        pServCb->pParam = NULL;
        pServCb->pLatestCli = (t_SSCLI*)XNULLP;
        memset(pServCb->myAddr.ip,0,sizeof(pServCb->myAddr.ip[0]) * SCTP_ADDR_NUM);
        pServCb->myAddr.port = 0;
        pServCb->myAddr.ipNum = 0;
        pServCb->sockFd.fd = XOS_INET_INV_SOCKFD;
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
    while(pServCb->pLatestCli != XNULLP)
    {
        SCTP_closeTsCli(pServCb->pLatestCli);
        if(pServCb->usageNum == 0)
        {
            break;
        }
    }

    /*�ر�fd*/
    if (pServCb->sockFd.fd != XOS_INET_INV_SOCKFD)
    {
        XOS_INET_FD_CLR(&(pServCb->sockFd),&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));
        if(--g_sctpCb.sctpServTsk.setInfo.sockNum == 0)
        {
            XOS_SemGet(&(g_sctpCb.sctpServTsk.taskSemp));
        }
        if(g_sctpCb.sctpServTsk.setInfo.sockNum == 0xffff)
        {
            g_sctpCb.sctpServTsk.setInfo.sockNum =0;
        }

        XINET_CloseSock(&(pServCb->sockFd));
    }        

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
    XS32 timeIndex = 0;
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
        "%-6s%-12s%-22s%-10s%-10s%-6s%-10s%-6s%-6s\r\n",
        "index",
        "userHander",
        "userFid",
        "linkstate",
        "myIP",
        "mPort",
        "peerIP",
        "pPort",
        "timId"
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
            timeIndex = 0;
            
            if(sctpCliCb->timerId != XNULL)
            {
                timeIndex =(XS32)((XPOINT)(sctpCliCb->timerId)&0x0fffff);
            }

            XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-22s%-10s%-10x%-6d%-10x%-6d%-6x\r\n",
                i,
                sctpCliCb->userHandle,
                XOS_getFidName(sctpCliCb->linkUser.FID),
                state[sctpCliCb->linkState],
                sctpCliCb->myAddr.ip[0],
                sctpCliCb->myAddr.port,
                sctpCliCb->peerAddr.ip[0],
                sctpCliCb->peerAddr.port,
                timeIndex);
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
������:SCTP_CliMsgShow
����: ��ʾsctp client ÿ��·ͳ����Ϣ��Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
************************************************************************/
XVOID SCTP_CliMsgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 nIndex = -1;
    XS32 i =0 ;
    XS32 j = 0;
    t_SCCB* sctpCliCb = NULL;

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
        
    if(XOS_StrCmp(ppArgv[1], "5G") !=0 )
    {
        XOS_CliExtPrintf(pCliEnv,"pls input para:5G\r\n");
        return;
    }
    
    /*sctp client cfg list start */
    XOS_CliExtPrintf(pCliEnv,
        "sctp client statistic list \r\n---------------------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-18s%-10s%-10s%-8s%-10s%-10s%-10s%-4s\r\n",
        "index",
        "userHander",
        "userFid",
        "linkstate",
        "total",
        "wait",
        "success",
        "delete",
        "fail",
        "curr"
        );

    nIndex = XOS_ArrayGetFirstPos(g_sctpCb.sctpClientLinkH);
    for(j=0,i=nIndex; i>=0; i=XOS_ArrayGetNextPos(g_sctpCb.sctpClientLinkH,i))
    {
        sctpCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,i);
        if(sctpCliCb)
        {
            XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-18s%-10s%-10d%-8d%-10d%-10d%-10d%-4d\r\n",
                i,sctpCliCb->userHandle,XOS_getFidName(sctpCliCb->linkUser.FID),
                state[sctpCliCb->linkState],
                sctpCliCb->packetlist.rsnd_total,
                sctpCliCb->packetlist.rsnd_wait,
                sctpCliCb->packetlist.rsnd_success,
                sctpCliCb->packetlist.rsnd_delete,
                sctpCliCb->packetlist.rsnd_fail,
                sctpCliCb->packetlist.rsnd_size
                );

        }
        j++;
        NTL_SHOW_CMD_CTL(j);
    }

    /*end of sctp client list */
    XOS_CliExtPrintf(pCliEnv,
        "---------------------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n",j);
    return ;
}


/************************************************************************
������:SCTP_ServMsgShow
����: ��ʾsctp server's client ÿ��·ͳ����Ϣ��Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
************************************************************************/
XVOID SCTP_ServMsgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 i = 0, j = 0, sum = 0;
    XS32 nIndex = 0;
    t_SSCB* sctpServCb = NULL;
    t_SSCLI* pSctpServCli = NULL;
    t_IPADDR* pPeerAddr = NULL;
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
    
    if(XOS_StrCmp(ppArgv[1], "5G") !=0 )
    {
        XOS_CliExtPrintf(pCliEnv,"pls input para:5G\r\n");
        return;
    }

    XOS_CliExtPrintf(pCliEnv,"sctp server configuration list\r\n--------------------------------------------------------------------------------------\r\n");
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
    for(sum = 0, i = nIndex; i >= 0; i = XOS_ArrayGetNextPos(g_sctpCb.sctpServerLinkH,i))
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
                sctpServCb->myAddr.ip,
                sctpServCb->myAddr.port,
                sctpServCb->maxCliNum,
                sctpServCb->usageNum);
            if(sctpServCb->usageNum > 0)
            {
                XOS_CliExtPrintf(pCliEnv,"      sctp client connect list\r\n");
                XOS_CliExtPrintf(pCliEnv,"      --------------------------------------------------------------------------------\r\n");
                XOS_CliExtPrintf(pCliEnv,
                    "      %-6s%-10s%-8s%-10s%-8s%-10s%-10s%-10s%-4s\r\n",
                    "index",
                    "peerip",
                    "prport",
                    "total",
                    "wait",
                    "success",
                    "delete",
                    "fail",
                    "curr"
                    );
                pSctpServCli = sctpServCb->pLatestCli;
                if(pSctpServCli)
                {
                    for(j=0; j<sctpServCb->usageNum; j++)
                    {
                        pPeerAddr = (t_IPADDR*)XOS_HashGetKeyByElem(g_sctpCb.tSctpCliH,pSctpServCli);
                        XOS_CliExtPrintf(pCliEnv,
                            "      %-6d%-10x%-8d%-10d%-8d%-10d%-10d%-10d%-4d\r\n",
                            j,pPeerAddr->ip,pPeerAddr->port,
                            pSctpServCli->packetlist.rsnd_total,
                            pSctpServCli->packetlist.rsnd_wait,
                            pSctpServCli->packetlist.rsnd_success,
                            pSctpServCli->packetlist.rsnd_delete,
                            pSctpServCli->packetlist.rsnd_fail,
                            pSctpServCli->packetlist.rsnd_size
                            );

                        pSctpServCli = pSctpServCli->pPreCli;
                        if(pSctpServCli == XNULLP)
                        {
                            break;
                        }
                        NTL_SHOW_CMD_CTL(j);
                    }
                    XOS_CliExtPrintf(pCliEnv,"      --------------------------------------------------------------------------------\r\n");
                }
            }
            sum++;
            NTL_SHOW_CMD_CTL(sum);
        }
    }

    /*end of sctp serv clients list */
    XOS_CliExtPrintf(pCliEnv,"--------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,"total lists : %d\r\n",sum);
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
        "%-6s%-12s%-22s%-10s%-10s%-6s%-6s%-6s%-8s\r\n",
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

#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */


