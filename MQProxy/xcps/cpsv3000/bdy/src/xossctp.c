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
                  包含头文件
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
                模块内部全局变量
-------------------------------------------------------------------------*/
static t_SCTPGLOAB g_sctpCb;
extern XBOOL g_sctp_timer;
extern XBOOL g_ntl_timer;
XSTATIC char xos_sctp_hb_interval[64] = "net.sctp.hb_interval = %d\r\n\r\n";
XSTATIC char xos_sctp_rto_min[64] = "net.sctp.rto_min = %d\r\n\r\n";
XSTATIC char xos_sctp_rto_initial[64] = "net.sctp.rto_initial = %d\r\n\r\n";
XSTATIC char xos_sctp_sack_timeout[64] = "net.sctp.sack_timeout = %d\r\n\r\n";

/*存储sctp系统参数的临时文件名*/
#define XOS_SCTP_KERNEL_FILENAME "xos_sctp_kernel.conf"

/**************************************************************
函数名: XOS_SetSctpKernelParas
功能: 调整sctp协议的内核参数，优化链路断连的检测时间
输入: 
输出: 无
返回: 
说明: 
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
    /*linux操作系统启动时，/proc/sys/net/sctp文件是不存在，
    需要创建sctp socket来生成这个文件，否则本函数不生效*/
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
    pServCliCb = (t_SSCLI*)XOS_HashElemFind(g_sctpCb.tSctpCliH,(XVOID*)pClientAddr);
    return pServCliCb;
}

/*******************************************************
函数名称    : XINET_GetSctpSockName
作者        : liukai
设计日期    : 2013年9月22日
功能描述    : 获取sctp连接相关联的本地地址
输入参数    : t_XINETFD *sockFd -描述符
输出参数    : t_SCTPIPADDR* locAddr号   -本地地址(主机字节序)
返回值      : XPUBLIC XS16
XOS_SUCC  - 成功
XOS_ERROR - 主要指参数错误
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
函数名:SCTP_noticeCloseCli
功能:  通知关闭sctp的客户端
输入: pCliCb   客户端控制块指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
函数名:
功能:  关闭sctp客户端
输入:   taskNo       任务号
        pCliCb   客户端控制块指针
        XS32 close_type  关闭的类型定义
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: 该功能接口需要扩充参数最好,主动关闭;被动关闭.
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

    /*清空客户端的数据*/
    SCTP_dataReqClear(&(pCliCb->packetlist));
    switch(close_type)
    {
    case NTL_SHTDWN_RECV:
        /*清理read set */
        XOS_INET_FD_CLR(&(pCliCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet));
        break;
    case NTL_SHTDWN_SEND:
        /*清理read set */
        XOS_INET_FD_CLR(&(pCliCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet));
        /*清理write set */
        XOS_INET_FD_CLR(&(pCliCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.writeSet.fdSet));
        break;
    default:
        XOS_Trace(MD(FID_NTL,PL_MIN),"SCTP closeTCli unsupport close type %d",close_type);
        return (XERROR);
    } 

    if(--g_sctpCb.pSctpCliTsk[taskNo].setInfo.sockNum == 0)
    {
        /*当客户端数量为0时，捕获客户端任务的驱动信号量，导致其下次阻塞，等待下次有客户端任务时再启动*/
        XOS_SemGet(&(g_sctpCb.pSctpCliTsk[taskNo].taskSemp));
    }

    if(g_sctpCb.pSctpCliTsk[taskNo].setInfo.sockNum  == 0xffff)
    {
        g_sctpCb.pSctpCliTsk[taskNo].setInfo.sockNum = 0;
    }

    /*关闭socket*/
    if ( XSUCC != XINET_CloseSock(&(pCliCb->sockFd)))
    {
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
函数名:SCTP_ResetAllFd
功能:  清理所有的sctp读写集
输入:  taskNo  任务号
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 SCTP_ResetAllFd(XU32 taskNo)
{
    /*清理read set */
    XOS_INET_FD_ZERO(&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet));
    /*清理write set */
    XOS_INET_FD_ZERO(&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.writeSet.fdSet));

    g_sctpCb.pSctpCliTsk[taskNo].setInfo.sockNum = 0;

    return XSUCC;
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
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp server close client begin:serverClis sockNum=%d",g_sctpCb.sctpServTsk.setInfo.sockNum);
    XOS_Trace(MD(FID_NTL,PL_INFO),"sctp server close client begin:closing client sock=%d",pSctpServCli->sockFd.fd);

    /*关闭sock ，清理资源*/

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
    //XOS_MutexLock(&(g_sctpCb.hashMutex));
    XOS_HashDelByElem(g_sctpCb.tSctpCliH, pSctpServCli);
    //XOS_MutexUnlock(&(g_sctpCb.hashMutex));

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

        /*获取链路所在任务号*/
        taskNo = (XU32)NTL_getLinkIndex(pCloseReq->linkHandle)/(g_sctpCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_sctpCb.sctpCliTskNo)
        {
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_closeReqProc()->get the sctp client taskno failed!");
            return XERROR;
        }

        /*如果是网络关闭，要超时重联*/
        if(pMsg->datasrc.FID == FID_NTL)
        {
            /*初始化*/
            pCliCb->linkState = eStateInited;
            pCliCb->expireTimes = 0;

            /*启动超时重联定时器*/            
            XOS_MemSet(&timerParam,0,sizeof(t_PARA));
            timerParam.fid = FID_NTL;
            timerParam.len = SCTP_CLI_RECONNECT_INTERVAL;
            timerParam.mode = TIMER_TYPE_LOOP;
            timerParam.pre = TIMER_PRE_LOW;

            XOS_MemSet(&backPara,0,sizeof(t_BACKPARA));
            backPara.para1 = (XPOINT)pCloseReq->linkHandle;
            backPara.para2 = (XPOINT)taskNo;   
            backPara.para3 = (XPOINT)eSCTPReconnect;

            /*在这段时间里可能又收到新的发送数据了*/
            SCTP_dataReqClear(&(pCliCb->packetlist));/*清除链路之前接收到的数据包*/

            XOS_INIT_THDLE (pCliCb->timerId);
            XOS_TimerStart(&(pCliCb->timerId),&timerParam, &backPara);
        }
        else /*用户请求关闭*/
        {
            SCTP_StopClientTimer(pCliCb);          

            /*如果正处于网络关闭后，用户再来关闭，则不需要再关闭，关闭fd, 清空数据，重置读写集*/
            if(pCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
            {
                SCTP_closeCli(taskNo, pCliCb, NTL_SHTDWN_SEND);
            }
            else
            {
                /*在这段时间里可能又收到新的发送数据了*/
                SCTP_dataReqClear(&(pCliCb->packetlist));/*清除链路之前接收到的数据包*/
            }

            pCliCb->linkState = eStateInited;
            pCliCb->expireTimes = 0;
            /*将对端地址置空*/
            memset(pCliCb->peerAddr.ip,0,sizeof(pCliCb->peerAddr.ip[0]) * SCTP_ADDR_NUM);//XU32改为数据实际类型
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
       
        if(0x00 == pCloseReq->cliAddr.ip && 0x00 == pCloseReq->cliAddr.port)
        {
            /*首先关闭所有接入的客户端*/     

            while(pServCb->pLatestCli != XNULLP)
            {
                SCTP_closeTsCli(pServCb->pLatestCli);
                if(pServCb->usageNum == 0)
                {
                    break;
                }
            }

            /*关闭listen 的fd*/
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

            /*关闭fd*/
            XINET_CloseSock(&(pServCb->sockFd));
            
            /*改变相应的cb 数据,初始化*/
            pServCb->linkState = eStateInited;
            pServCb->maxCliNum = 0;
            pServCb->usageNum = 0;
            pServCb->pLatestCli = (t_SSCLI*)XNULLP;
            pServCb->authFunc = NULL;
            pServCb->pParam = NULL;
            
        }
        else
        {   
            /*查找指定的客户端控制块*/
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

        /*获取链路所在任务号*/
        taskNo = (XU32)NTL_getLinkIndex(pReleaseReq->linkHandle)/(g_sctpCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_sctpCb.sctpCliTskNo)
        {
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_ReleaseLink()->get the sctp client taskno failed!");
            return XERROR;
        }

        SCTP_StopClientTimer(pCliCb);          

        /*如果正处于网络关闭后，用户再来关闭，则不需要再关闭，关闭fd, 清空数据，重置读写集*/
        if(pCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            SCTP_closeCli(taskNo, pCliCb, NTL_SHTDWN_SEND);
        }
        else
        {
            /*在这段时间里可能又收到新的发送数据了*/
            SCTP_dataReqClear(&(pCliCb->packetlist));/*清除链路之前接收到的数据包*/
        }

        pCliCb->linkState = eStateInited;
        pCliCb->expireTimes = 0;
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
        while(pServCb->pLatestCli != XNULLP)
        {
            SCTP_closeTsCli(pServCb->pLatestCli);
            if(pServCb->usageNum == 0)
            {
                break;
            }
        }

        /*关闭listen 的fd*/
        XOS_INET_FD_CLR(&(pServCb->sockFd),&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));
        if(--g_sctpCb.sctpServTsk.setInfo.sockNum == 0)
        {
            XOS_SemGet(&(g_sctpCb.sctpServTsk.taskSemp));
        }
        if(g_sctpCb.sctpServTsk.setInfo.sockNum == 0xffff)
        {
            g_sctpCb.sctpServTsk.setInfo.sockNum =0;
        }

        /*关闭fd*/
        XINET_CloseSock(&(pServCb->sockFd));

        /*改变相应的cb 数据,初始化*/
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
函数名:SCTP_Opt
功能:  检查输入输出流，并设置链路的偶联及路径的断链检测伐值
输入:  pCliCb －sctp客户端控制块指针
        peerAddrNum - 对端绑定地址数量
输出:
返回:  成功返回XSUCC,否则返回XERROR
说明:  
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
        /*关闭新接入的sock*/
        XINET_CloseSock(&(pCliCb->sockFd));
        XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_Opt,server (ip[0x%x],port[%d]) instream is %d,local outstream is %d!",
            XOS_INET_NTOH_U32(pCliCb->peerAddr.ip[0]),XOS_INET_NTOH_U16(pCliCb->peerAddr.port),peerInstream,pCliCb->maxStream);
        pCliCb->linkState = eStateInited;
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_Opt(),connect sctp server failed,peer instream too small!");

        /*发送启动失败消息到上层*/
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
        /*关闭新接入的sock*/
        XINET_CloseSock(&(pCliCb->sockFd));
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_Opt,set sctp_path_max_retrans new client(ip[0x%x],port[%d]) !",
            XOS_INET_NTOH_U32(pCliCb->myAddr.ip[0]),XOS_INET_NTOH_U16(pCliCb->myAddr.port));
        /*发送启动失败消息到上层*/
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
        /*关闭新接入的sock*/
        XINET_CloseSock(&(pCliCb->sockFd));
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_Opt,set sctp_assoc_max_retrans new client(ip[0x%x],port[%d]) !",
            XOS_INET_NTOH_U32(pCliCb->myAddr.ip[0]),XOS_INET_NTOH_U16(pCliCb->myAddr.port));
        /*发送启动失败消息到上层*/
        sctpStartAck.appHandle = pCliCb->userHandle;
        sctpStartAck.linkStartResult = eOtherResult;
        XOS_MemCpy(&(sctpStartAck.localAddr),&(pCliCb->myAddr),sizeof(t_SCTPIPADDR));

        NTL_msgToUser((XVOID*)&sctpStartAck,&(pCliCb->linkUser),sizeof(t_SCTPSTARTACK),eSctpStartAck);
        return ret;
    }
    return XSUCC;
}

/************************************************************************
函数名:SCTP_AcceptClient
功能:  接收一个客户端连接，并添加到链表
输入:  pServCb －sctp服务端控制块指针
输出:
返回:  成功返回XSUCC,否则返回XERROR
说明:  只在SCTP_servTsk中调用
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
    /*先接收*/
    ret = XINET_SctpAccept(&(pServCb->sockFd),&fromAddr,pServCb->hbInterval,&servCliFd);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_AcceptClient(),accept fd %d failed!",pServCb->sockFd.fd);
        return XERROR;
    }

    /*接入认证*/
    if(pServCb->authFunc != XNULL
        && !(pServCb->authFunc(pServCb->userHandle,&fromAddr,pServCb->pParam)))
    {
        /*关闭新接入的sock*/
        XINET_CloseSock(&servCliFd);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient(),get client(ip[0x%x],port[%d]) auth failed!",
            XOS_INET_NTOH_U32(fromAddr.ip[0]),XOS_INET_NTOH_U16(fromAddr.port));
        return XERROR;
    }

    ret = XINET_GetOpt(&servCliFd, XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_PEER_INSTREAM, &peerInstream);
    if( ret != XSUCC || pServCb->maxStream > peerInstream )
    {
        /*关闭新接入的sock*/
        XINET_CloseSock(&servCliFd);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient(),new client's instream too small,(ip[0x%x],port[%d]) instream is %d,local outstream is %d!",
            XOS_INET_NTOH_U32(fromAddr.ip[0]),XOS_INET_NTOH_U16(fromAddr.port),peerInstream,pServCb->maxStream);
        return XERROR;
    }

    optVal = pServCb->pathmaxrxt;
    ret = XINET_SetOpt(&servCliFd,XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_PATHMAXRXT,&optVal);
    if( ret != XSUCC )
    {
        /*关闭新接入的sock*/
        XINET_CloseSock(&servCliFd);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient() set sctp_path_max_retrans for socket failed!");
        return XERROR;
    }

    optVal = pServCb->pathmaxrxt * fromAddr.ipNum;
    ret = XINET_SetOpt(&servCliFd,XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_ASSOCINFO,&optVal);
    if( ret != XSUCC )
    {
        /*关闭新接入的sock*/
        XINET_CloseSock(&servCliFd);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient() set sctp_assoc_max_retrans for socket failed!");
        return XERROR;
    }

    XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_AcceptClient(),accept fd %d ,ip:%x,port:%d,hb:%d!",pServCb->sockFd.fd,
        fromAddr.ip[0],fromAddr.port,pServCb->hbInterval);

    /*接入客户的数量不能超过最大容许接入数量*/
    if(pServCb->maxCliNum < pServCb->usageNum +1)
    {
        /*关闭新接入的sock*/
        XINET_CloseSock(&servCliFd);
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_AcceptClient(),new accept client(ip[0x%x],port[%d]) is overstep maxclis %d allowned!",
            XOS_INET_NTOH_U32(fromAddr.ip[0]),XOS_INET_NTOH_U16(fromAddr.port),pServCb->maxCliNum);
        return XERROR;
    }
    /*将接入的客户端加到hash表中*/
    XOS_MemSet(&servCliCb,0,sizeof(t_SSCLI));
    XOS_MemCpy(&(servCliCb.sockFd),&servCliFd,sizeof(t_XINETFD));
    /*链表也应该保护*/
    servCliCb.pServerElem = pServCb;
    servCliCb.pPreCli = pServCb->pLatestCli;
    servCliCb.pNextCli = (t_SSCLI*)XNULLP;

    /*不论对端连接多少ip过来，都取第一个ip作为hash的key*/
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

    /*构成链*/
    pServCliCb = (t_SSCLI*)XNULLP;
    pServCliCb = (t_SSCLI*)XOS_HashGetElem(g_sctpCb.tSctpCliH, pLocation);

    if(pServCb->pLatestCli != XNULLP && pServCliCb != XNULLP)
    {
        pServCb->pLatestCli->pNextCli = pServCliCb;
    }
    pServCb->pLatestCli= pServCliCb;
    pServCb->usageNum++; /*接入一个新的客户进来*/

    /*加入到read 集中*/
    g_sctpCb.sctpServTsk.setInfo.sockNum++;
    XOS_INET_FD_SET(&servCliFd, &( g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));

    /*发送连接指示消息到上层*/
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
* 功能: 定义对每个hash元素通用的函数
* 输入  :
* hHash   - 对象句柄
* elem    - 元素
* param   - 参数
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
    /* 检查*/
    if(XOS_INET_FD_ISSET(&(pTservCli->sockFd),pReadSet))
    {
        /*从网络上收数据*/
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

        /*发送数据到上层用户*/
        dataInd.appHandle = pTservCli->pServerElem->userHandle;
        dataInd.dataLenth = (XU32)len;

        /*实际数据都较小，转换成小内存存储接收的数据*/
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
函数名:SCTP_sctpServTsk
功能:  sctp server listening function
输入:taskNo  任务号
输出:
返回:
说明:
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
        /*线程刚起来时，并没有sockfd，fd为空时select直接返回*/
        g_sctpCb.sctpServTsk.activeFlag = XFALSE;
        XOS_SemGet(&(g_sctpCb.sctpServTsk.taskSemp));
        g_sctpCb.sctpServTsk.activeFlag = XTRUE;
        XOS_SemPut(&(g_sctpCb.sctpServTsk.taskSemp));

        /*拷贝到局部变量中，防止破坏全局的读集*/
        XOS_MemSet(&readSet,0,sizeof(t_FDSET));
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        XOS_MemCpy(&readSet,&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet),sizeof(t_FDSET));
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));

        /*select 监视*/
        setNum = 0;
        ret = XINET_Select(&readSet,(t_FDSET*) XNULLP,(XU32*)&pollAbideTime,(XS16*)&setNum);
        if(ret != XSUCC)
        {
            /*select 超时*/
            if(ret == XINET_TIMEOUT)
            {
                continue;
            }
            else /*其他错误*/
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"sctpServer task,select readset failed, setNum =%d.",setNum);
                continue;
            }
        }
        
        if(setNum > 0)
        {
            XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
            /*有客户端连接进来*/
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

            /*有数据接收*/
            XOS_HashWalk(g_sctpCb.tSctpCliH,SCTP_pollingHash,&readSet);
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        }       
    }
}

/************************************************************************
函数名:SCTP_cliRcvFunc
功能:  sctp客户端的入口函数
输入:taskNo  任务号
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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

    /*毫秒*/
    pollAbideTime = POLL_FD_TIME_OUT;
    taskNo = (XU32)(XPOINT)taskPara;

    while(1)
    {
        /*线程刚起来时，并没有sockfd，fd为空时select直接返回*/
        g_sctpCb.pSctpCliTsk[taskNo].activeFlag = XFALSE;

        XOS_SemGet(&(g_sctpCb.pSctpCliTsk[taskNo].taskSemp));
        g_sctpCb.pSctpCliTsk[taskNo].activeFlag = XTRUE;
        XOS_SemPut(&(g_sctpCb.pSctpCliTsk[taskNo].taskSemp));

        /*拷贝到局部变量中*/
        XOS_MemSet(&sctpCliRead,0,sizeof(t_FDSET));
        XOS_MemSet(&sctpCliWrite,0,sizeof(t_FDSET));
        pReadSet = (t_FDSET*)XNULLP;
        pWriteSet = (t_FDSET*)XNULLP;

        XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
        /*如果控制块为空*/
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
            /*select 超时*/
            if(ret == XINET_TIMEOUT)
            {
                continue;
            }
            else  /*select 出错，应该是立即返回的，如果再continue，还是立即返回，可能死循环*/
            {    /*由于这里没有使用互斥，很有可能fd被关闭了，却在参与select，导致异常*/
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
                /*扩大为全锁 2013.10.24*/
                pCliCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,(XS32)i);

                if(pCliCb != XNULLP)
                {
                    nDealFlg++;
                    /*客户端未启动链路或链路被重置了*/
                    if( ((pCliCb->linkState != eStateConnected)&&(pCliCb->linkState != eStateConnecting))
                        ||(pCliCb->sockFd.fd == XOS_INET_INV_SOCKFD))
                    {
                        continue;
                    }

                    /*可写*/
                    if((pWriteSet != XNULLP) && XOS_INET_FD_ISSET(&(pCliCb->sockFd),pWriteSet))
                    {
                        setNum--;

                        /*同时可读可写，说明异常*/
                        if(XOS_INET_FD_ISSET(&(pCliCb->sockFd),pReadSet) || pCliCb->sockFd.fd <= 0)
                        {
                            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_cliRcvFunc(),sctp client fd = %d disconnect,try reconnect!",
                            pCliCb->sockFd.fd);
                            /*关定时器*/
                            XOS_TimerStop(FID_NTL,pCliCb->timerId);
                            /*停止链路*/
                            SCTP_closeCli(taskNo,pCliCb,NTL_SHTDWN_SEND);
                            /*通知状态改变*/
                            SCTP_noticeCloseCli( pCliCb);
                            /*发送关闭指示*/
                            closeInd.appHandle = pCliCb->userHandle;
                            closeInd.closeReason = ePeerReq;
                            NTL_msgToUser((XVOID*)&closeInd,&(pCliCb->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);

                            continue;
                        }
                        peerAddrNum = XINET_SctpConnectCheck(&(pCliCb->sockFd));

                        /*从write 集中清除,等待定时器到期进行处理*/
                        XOS_INET_FD_CLR(&(pCliCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.writeSet.fdSet));

                        if(peerAddrNum <= 0)
                        {
                            XOS_Trace(MD(FID_NTL,PL_WARN),
                            "SCTP_cliRcvFunc()->connect sctp server failed! myaddr.ip = 0x%08x,myaddr.port = %d;peeraddr.ip = 0x%08x,peeraddr.port = %d",
                            pCliCb->myAddr.ip[0], pCliCb->myAddr.port, pCliCb->peerAddr.ip[0], pCliCb->peerAddr.port);

                            continue;
                        }

                        /*检查输入输出流，并设置链路的偶联及路径的断链检测伐值*/
                        ret = SCTP_Opt(pCliCb,peerAddrNum);
                        if(ret == XERROR)
                        {
                            continue;
                        }

                        /*添加到 读 集合中*/
                        XOS_INET_FD_SET(&(pCliCb->sockFd),&(g_sctpCb.pSctpCliTsk[taskNo].setInfo.readSet.fdSet));

                        /*客户端连接成功*/
                        if(pCliCb->linkState != eStateConnected)
                        {
                            pCliCb->linkState = eStateConnected;
                            XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_cliRcvFunc(),connect sctp server successed!");
                            SCTP_StopClientTimer(pCliCb);

                            /*发送启动成功消息到上层*/
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
                    /*可读或有异常，都通过这个case来处理*/
                    if((pReadSet != XNULLP) && XOS_INET_FD_ISSET(&(pCliCb->sockFd),pReadSet))
                    {
                        setNum--;
                        pData = (XCHAR*)XNULLP;
                        len = XOS_INET_READ_ANY;

                        /*从网络上收数据*/
                        ret = XINET_SctpRecvMsg(&(pCliCb->sockFd),&pData,&len,NULL,&sinfo);
                        if(ret == XINET_CLOSE)
                        {
                            /* if sctp server closed then notify all this client*/
                            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_cliRcvFunc(),sctp client disconnect,try reconnect!");
                            
                            pCliCb->linkState = eStateWaitClose;
                            //SCTP_StopClientTimer(pCliCb); /*这种情况，定时器实际已经关闭，不需再次关闭*/

                            SCTP_closeCli(taskNo, pCliCb, NTL_SHTDWN_SEND);
                            /*通知状态改变*/
                            SCTP_noticeCloseCli( pCliCb);
                            /*发送关闭指示*/
                            closeInd.appHandle = pCliCb->userHandle;
                            closeInd.closeReason = ePeerReq;
                            NTL_msgToUser((XVOID*)&closeInd,&(pCliCb->linkUser),sizeof(t_LINKCLOSEIND),eSctpStopInd);
                        }
                        else if(ret == XINET_TIMEOUT)
                        {
                            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_cliRcvFunc()-> sctp client detects remote server disconnect!");
                            
                            pCliCb->linkState = eStateWaitClose;

                            SCTP_closeCli(taskNo, pCliCb, NTL_SHTDWN_SEND);
                            /*通知状态改变*/
                            SCTP_noticeCloseCli( pCliCb);
                            /*发送关闭指示*/
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
                            /*发送数据到上层*/
                            XOS_MemSet(&dataInd,0,sizeof(t_SCTPDATAIND));
                            dataInd.appHandle = pCliCb->userHandle;
                            dataInd.dataLenth = (XU32)len;

                            /*实际数据都较小，转换成小内存存储接收的数据*/
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

                    /* 已经检查所有的被置位的socket*/
                    if(setNum <= 0)
                    {
                        break;
                    }
                }
            }
            if(setNum > 0) /*有数据未读写，即将导致下一次select立即返回*/
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_cliRcvFunc()->deal data error! curFdNum = %d, nDealFlg = %d, nNoStatus = %d",
                          setNum, nDealFlg, nNoStatus);
            }
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        }
    }
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

    /*设置sctp系统参数，用于控制断链检测时间*/
    XOS_SetSctpKernelParas(pGenCfg);

    /*保存配置信息*/
    XOS_MemCpy(&(g_sctpCb.genCfg),pGenCfg,sizeof(t_NTLGENCFG));

    /*确定一个线程监视的描述符数量*/
    if(g_sctpCb.genCfg.fdsPerThrPolling > 0 && g_sctpCb.genCfg.fdsPerThrPolling < FDS_MAX_THREAD_POLLING)
    {
        pollingNum = g_sctpCb.genCfg.fdsPerThrPolling;
    }
    else
    {
        pollingNum = FDS_PER_THREAD_POLLING;  /*默认一个线程监视256 个描述符*/
        g_sctpCb.genCfg.fdsPerThrPolling = pollingNum;
    }

    /*确定各任务集的数量*/
    /*sctp cli tasks */
    g_sctpCb.sctpCliTskNo = (g_sctpCb.genCfg.maxSctpCliLink%pollingNum)
        ?(g_sctpCb.genCfg.maxSctpCliLink/pollingNum + 1)
        :(g_sctpCb.genCfg.maxSctpCliLink/pollingNum);
    /*安全性检查:sctpcli  配置的任务超过最大任务数,说明通用配置不合理*/
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
    
    /* sctp client  link 任务相关启动*/
    if (g_sctpCb.sctpCliTskNo > 0 )
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

        /*启动任务集*/
        for (i=0; i<g_sctpCb.sctpCliTskNo; i++)
        {
            /*任务的驱动信号量*/
            ret = XOS_SemCreate(&(g_sctpCb.pSctpCliTsk[i].taskSemp), 0);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctpCli thread[%d]  semaphore  failed!",i);
                goto genCfgError;
            }           

            /* fd read write set 互斥琐*/
            ret = XOS_MutexCreate(&(g_sctpCb.pSctpCliTsk[i].setInfo.fdSetMutex));
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctp cli  thread fdset mutex[%d]  failed!",i);
                goto genCfgError;
            }

            /*初始化 set*/
            XOS_INET_FD_ZERO(&(g_sctpCb.pSctpCliTsk[i].setInfo.readSet.fdSet));
            XOS_INET_FD_ZERO(&(g_sctpCb.pSctpCliTsk[i].setInfo.writeSet.fdSet));

            /*创建sctp client任务*/
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

        /*初始化hash 的互斥琐*/
        ret = XOS_MutexCreate(&(g_sctpCb.hashMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()->create  mutex for sctphash failed!");
            goto genCfgError;
        }

        /*任务的驱动信号量*/
        ret = XOS_SemCreate(&(g_sctpCb.sctpServTsk.taskSemp),0);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctpServTsk thread entry semaphore  failed!");
            goto genCfgError;
        }        

        /*read set 互斥琐*/
        ret = XOS_MutexCreate(&(g_sctpCb.sctpServTsk.setInfo.fdSetMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_genCfgProc()-> sctp serv thread mutex failed!");
            goto genCfgError;
        }

        /*初始化 set*/
        XOS_INET_FD_ZERO(&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));
        XOS_INET_FD_ZERO(&(g_sctpCb.sctpServTsk.setInfo.writeSet.fdSet));

        /*创建sctp server 的任务*/
        XOS_Sprintf(taskName, sizeof(taskName)-1, "Tsk_sctps");
        ret = XOS_TaskCreate(taskName,TSK_PRIO_NORMAL,NTL_RECV_TSK_STACK_SIZE,(os_taskfunc)SCTP_servTsk,
            (XVOID *)0,&(g_sctpCb.sctpServTsk.taskId));
    }
    
    g_sctpCb.isGenCfg = XTRUE;
    return XSUCC;

genCfgError:
    /*释放资源*/
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
函数名:SCTP_servStart
功能:  处理链路启动消息
输入:pMsg －消息指针
输出:pStartAck －启动确认消息
返回: 成功返回XSUCC,否则返回XERROR
说明:
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

    /*获取sctp server 控制块*/
    pServCb = (t_SSCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpServerLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
    
    if(pServCb == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->get sctp serv control block failed!");
        return XERROR;
    }

    /*检查链路状态和相关参数*/
    if((pServCb->linkState != eStateInited )
        ||(pSctpServStart->allownClients == 0)
        ||(pSctpServStart->allownClients > 
        (XU32)((g_sctpCb.genCfg.maxSctpServLink)*g_sctpCb.genCfg.sctpClientsPerServ)))
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->link state [%s]  error or allownClients[%d] error!",
            NTL_getLinkStateName(pServCb->linkState,szLinkStateName,sizeof(szLinkStateName)-1),pSctpServStart->allownClients);
        goto errorProc;
    }

    /*如果状态正确，则可能是上次忘记关闭了*/
    SCTP_CloseServerSocket(pServCb);    

    if(pSctpServStart->streamNum > DEFAULT_SCTP_STREAM && pSctpServStart->streamNum <= SCTP_MAX_STREAM)
    {
        stream = pSctpServStart->streamNum;
    }
    else
    {
        stream = DEFAULT_SCTP_STREAM;
    }
    /*启动sockect*/
    ret = XINET_SctpSocket(&(pServCb->sockFd),stream);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->create sockFd failed");
        goto errorProc;
    }

    /*绑定端口*/
    ret =  XINET_SctpBind(&(pServCb->sockFd),&(pSctpServStart->myAddr),SCTP_BIND_ADD);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->XINET Bind sockFd failed");
        goto errorProc;
    }
    /*确定本端地址*/
    XINET_GetSctpSockName(&(pServCb->sockFd),&(pServCb->myAddr));

    /*listen*/
    ret = XINET_Listen(&(pServCb->sockFd),MAX_BACK_LOG);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_servStart()->XINET_Listen sockFd %d failed",pServCb->sockFd.fd);
        goto errorProc;
    }

    /*添加到read 集中*/
    if(g_sctpCb.sctpServTsk.setInfo.sockNum++ == 0)
    {
        XOS_INET_FD_SET(&(pServCb->sockFd),&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));
        XOS_SemPut(&(g_sctpCb.sctpServTsk.taskSemp));
    }
    else
    {
        XOS_INET_FD_SET(&(pServCb->sockFd),&(g_sctpCb.sctpServTsk.setInfo.readSet.fdSet));
    }

    /*填写参数*/
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
    /*填写输出参数*/
    pStartAck->appHandle = pServCb->userHandle;
    pStartAck->linkStartResult = eSUCC;
    XOS_MemCpy(&(pStartAck->localAddr),&(pServCb->myAddr),sizeof(t_SCTPIPADDR));
    return XSUCC;

errorProc:
    {
        /*填写startAck 回复消息,返回结果为efailed*/
        pStartAck->appHandle = pServCb->userHandle;
        pStartAck->linkStartResult = eFAIL;
        return XSUCC;
    }
}

/************************************************************************
函数名:SCTP_StartClientTimer
功能:  启动sctp客户端的重启定时器
输入:  pSctpCb －sctp控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
函数名:SCTP_SetClientFd
功能:  sctp client select 置位操作
输入:  pSctpCb －消息指针
       fdFlg  --0:读,1:写, 2:读和写
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
                pCliCb->sockFd.fd = XOS_INET_INV_SOCKFD;    
                memset((char*)&(pCliCb->packetlist), 0, sizeof(t_SctpResndPacket));
                XOS_INIT_THDLE(pCliCb->timerId);
                pCliCb->expireTimes = 0;
                
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
            XOS_Trace(MD(FID_NTL,PL_WARN),"SCTP_linkInitProc()->add the sctp server cb was reset!");
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
                pSctpSrvCb->sockFd.fd = XOS_INET_INV_SOCKFD;    
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

    /*所有的下行消息(elinkInit 除外)都要验证链路句柄的有效性，
    以防止误操作修改到其他的数据*/
    if(!NTL_isValidLinkH(pLinkStart->linkHandle))
    {
        /*链路句柄无效的消息，直接丢弃*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStarttProc()->FID[%d] linkHandle[%d] invalid!", pMsg->datasrc.FID,
                                     pLinkStart->linkHandle);
        return XERROR;
    }

    /*获取链路类型*/
    linkType = NTL_getLinkType(pLinkStart->linkHandle);
    XOS_MemSet(&startAck,0,sizeof(t_SCTPSTARTACK));
    switch (linkType)
    {
    case eSCTPClient:
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

        /*获取链路所在任务号*/
        taskNo = (XU32)NTL_getLinkIndex(pLinkStart->linkHandle)/(g_sctpCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_sctpCb.sctpCliTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] sctp client taskNo %d is invalid!",pSctpCb->linkUser.FID, taskNo);
            startAck.appHandle = pSctpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
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

           /*状态正确，fd却打开，须要关闭，可能是上次忘记关闭了*/
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
        /*启动所有sock*/
        ret = XINET_SctpSocket(&(pSctpCb->sockFd),stream);
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] sctp client socket failed!",pSctpCb->linkUser.FID);
            /*填写startAck 回复消息,返回结果为efailed*/
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

        /*对于sctp 客户端,需要进行bind*/
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

        /*连接到对端*/
        ret = XINET_SctpConnect(&(pSctpCb->sockFd),&(pSctpCliStart->peerAddr),NULL);
        if(ret == XSUCC)
        {
            ret = SCTP_Opt(pSctpCb, pSctpCliStart->peerAddr.ipNum);
            if(ret == XERROR)
            {
                XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
                return XERROR;
            }
            /*非阻塞连接成功的可能性很小，要是连接本机的端口，一般是可以连接成功的*/
            pSctpCb->linkState = eStateConnected;
            SCTP_StopClientTimer(pSctpCb);

            /*添加到readset中*/
            SCTP_SetClientFd(pSctpCb, taskNo, eRead);

            /*填写startAck 回复消息,返回结果为成功*/
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

            /*如果连接不成功,尝试重连,启动定时器,保证链路连接*/            
            if(XSUCC != SCTP_StartClientTimer(pSctpCb, taskNo))
            {
                /*定时器启动失败处理，待定*/
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->FID[%d] sctp client start timer failed!",pSctpCb->linkUser.FID);
                startAck.appHandle = pSctpCb->userHandle;
                startAck.linkStartResult = eFAIL;
                XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
                break;
            }

            /*添加到writeset中*/
            SCTP_SetClientFd(pSctpCb, taskNo, eWrite);
            
            startAck.appHandle = pSctpCb->userHandle;
            XOS_MemCpy(&(startAck.localAddr),&(pSctpCb->myAddr),sizeof(t_SCTPIPADDR));
            startAck.linkStartResult = eBlockWait;            
        }
        XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
        break;

    case eSCTPServer:
        /*sctp server 的启动比较复杂，单独一个函数处理*/
        XOS_MutexLock(&(g_sctpCb.sctpServerLinkMutex));
        ret = SCTP_servStart( pMsg,&startAck);
        XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
        if(ret == XERROR)
        {
            /*返回错误就不用回复start Ack 消息*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_linkStartProc()->start sctp server  failed!");
            return XERROR;
        }
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
函数名:SCTP_dataReqTimerProc
功能:  定时器消息中的sctp分支处理函数，用于定时重发数据
输入:  t_BACKPARA* pParam  定时消息地址指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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

    /*发送客户端缓冲区中的数据*/
    if((timer_src[0] ==  eSCTPCliResendTimer) && (timer_src[1] == eSCTPCliResendCheck))
    {
        SCTP_dataReqTimerSend(eSCTPClient);
        return XSUCC;
    }

    /*发送服务端缓冲区中的数据*/
    if((timer_src[0] ==  eSCTPSerResendTimer) && (timer_src[1] == eSCTPSerResendCheck))
    {
        SCTP_dataReqTimerSend(eSCTPServer);
        return XSUCC;
    }

    /*处理sctp客户端的重连*/
    pSctpCb = (t_SCCB*)XNULLP;

    pSctpCb = (t_SCCB*)XOS_ArrayGetElemByPos(g_sctpCb.sctpClientLinkH,NTL_getLinkIndex((HLINKHANDLE)pParam->para1));
    if((t_SCCB*)XNULLP == pSctpCb)
    {
        return XERROR;
    }
    XOS_MutexLock(&(g_sctpCb.sctpClientLinkMutex));
    /*判断此元素在队列中是否属于空闲，如空闲则不再处理*/
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

    /*检查链路状态*/
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
        /*尝试绑定，如果配置本端地址并且端口不冲突，应该绑定成功*/
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
                /*关闭新接入的sock*/
                XINET_CloseSock(&(pSctpCb->sockFd));
                
                /*停止定时器*/
                SCTP_StopClientTimer(pSctpCb);
                
                XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc,new client(ip[0x%x],port[%d]) instream is %d,local outstream is %d!",
                    XOS_INET_NTOH_U32(pSctpCb->peerAddr.ip[0]),XOS_INET_NTOH_U16(pSctpCb->peerAddr.port),peerInstream,pSctpCb->maxStream);
                break;
            }

            pSctpCb->linkState = eStateConnected;

            /*停止定时器*/
            SCTP_StopClientTimer(pSctpCb);
                            
            /*添加到readset中*/
            SCTP_SetClientFd(pSctpCb, taskNo, eRead);

            /*发送启动成功消息到上层*/
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
        /*已经连接上，关掉定时器*/
        XOS_IptoStr(pSctpCb->peerAddr.ip[0], szTemp);
        XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_timerProc(),FID[%d] sctp client  pSctpCb[0x%x] the %dth connect[%s:%d] successed!",
            pSctpCb->linkUser.FID,pSctpCb,pSctpCb->expireTimes ,szTemp,pSctpCb->peerAddr.port);

        /*停止定时器*/
        SCTP_StopClientTimer(pSctpCb);

        break;

    default:
        XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_timerProc()->expire msg in bad state!");
        break;
    }
    return XSUCC;
}

/************************************************************************
函数名:SCTP_dataReqSendProc
功能:  发送排队数据
输入:  t_XINETFD *pSockFd  - socket句柄
        t_SctpResndPacket *pPacklist  - 拥塞排队链表
        t_SCTPIPADDR *pDstAddr      - 数据发送目的地址
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
函数名:SCTP_dataReqTimerSend
功能:  定时器消息中的sctp分支处理函数，用于定时重发数据
输入:  
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
    /*发送数据*/
    ret = XOS_MsgSend(msgToNtl);
    if(ret != XSUCC)
    {
        /*其他消息类型应该将消息内存释放*/
        XOS_MsgMemFree((XU32)FID_NTL, msgToNtl);
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
函数名:SCTP_dataReqTimerProc
功能:  定时器消息中的sctp分支处理函数，用于定时重发数据
输入:  t_XOSCOMMHEAD *pMsg  消息地址指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
            /*先检查链路的状态*/
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
函数名:SCTP_dataReqTimerProc
功能:  用于发送sctp数据
输入:  t_XINETFD *pSockFd          - socket句柄
    :  t_SctpResndPacket* pklist    -拥塞队列指针
    :  t_SCTPIPADDR *pIpAddr        - 消息目的地址
    :  t_SCTPDATAREQ *pDataReq      -消息发送请求
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
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
        /*没有排队,发送数据*/
        ret = XINET_SctpSendMsg(pSockFd, pDataReq->pData, (XS32)pDataReq->msgLenth, NULL,pDataReq->attr);
        if(ret != XSUCC)
        {
            /*网络阻塞控制冗塞控制,排队*/
            if(XSUCC == SCTP_dataReqSaveProc(pklist,pDataReq))
            {
                if(g_ntltraceswitch)
                {
                    XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqtpktProc,resend wait packet size %d",pklist->rsnd_size);
                }
                /*排队成功*/
                return XSUCC;
            }
            /*计数*/
            pklist->rsnd_delete++;
            /*因为有统计计数,所有不通知用户发送失败*/
            /*排队失败,将数据报丢掉*/
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
            /*发送成功，清除数据包*/
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
        /*先排队*/
        if(XSUCC == SCTP_dataReqSaveProc(pklist,pDataReq))
        {
            if(g_ntltraceswitch)
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"SCTP_dataReqtpktProc,resend wait packet size %d",pklist->rsnd_size);
            }
            /*重发包不成功等待,直到成功删除*/
            SCTP_dataReqSendProc(pSockFd,pklist,NULL);
            return XSUCC;
        }
        /*计数*/
        pklist->rsnd_delete++;
        /*排队失败,因为有统计计数,所有不通知用户发送失败*/
        /*排队失败,将pDataReq数据包丢掉*/
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
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            return XERROR;
        }
        /*先检查链路的状态*/
        if (pCliCb->linkState != eStateConnected)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorLinkState;
            sendError.userHandle = pCliCb->userHandle;
            XOS_MutexUnlock(&(g_sctpCb.sctpClientLinkMutex));
            break;
        }
        
        /*获取链路所在任务号*/
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
        /*发送数据*/
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
            XOS_MutexUnlock(&(g_sctpCb.sctpServerLinkMutex));
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

        /*sctp 封装*/
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
函数名:SCTP_dataReqSaveProc
功能: 存储发送失败的数据到拥塞队列
输入: t_SctpResndPacket *pPacklist  - 拥塞队列指针
        t_SCTPDATAREQ *pDataReq     - 发送数据结构体
输出:
返回: 成功返回XOS_XSUCC,失败返回XERROR
说明:
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
        /*如果排队队列满,将后续消息包丢掉*/
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
    /*将新的数据包加到链表中*/
    XOS_MemSet(ptmpPacket,0x0,sizeof(t_SctpResnd));
    ptmpPacket->pNextPacket=XNULL;
    /*将数据包地址拷贝到链表中*/
    ptmpPacket->pData=pDataReq->pData;
    ptmpPacket->msgLenth = pDataReq->msgLenth;
    ptmpPacket->attr.stream = pDataReq->attr.stream;
    ptmpPacket->attr.context = pDataReq->attr.context;
    ptmpPacket->attr.ppid = pDataReq->attr.ppid;

    /*空链表*/
    pPacklist->rsnd_wait++;
    if(pPacklist->pFirstDataReq == XNULL)
    {
        /*作为头节点*/
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
函数名:SCTP_dataReqClear
功能:  清空sctp拥塞队列
输入:  t_SctpResndPacket* pklist    -拥塞队列指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:已排队队列不清除,直到发送成功为止
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
函数名:SCTP_StopClientTimer
功能:  停止sctp客户端的定时器
输入:  pSctpCb －sctp控制块指针
输出:
返回: 成功返回XOS_XSUCC,失败返回XERROR
说明:
************************************************************************/
XS32 SCTP_StopClientTimer(t_SCCB *pSctpCb)
{
    XS32 ret = 0;

    if(!pSctpCb)
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),"SCTP_StopClientTimer()->pSctpCb is null!");
        return XERROR;
    }
    
    /*停止定时器*/
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
    XU32 taskNo = 0;

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

        taskNo = (XU32)NTL_getLinkIndex(pCloseReq->linkHandle)/(g_sctpCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_sctpCb.sctpCliTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"SCTP_ResetLinkByReapply()->sctp client taskNo is %d", taskNo);
            return XERROR;    
        }

        /*关闭定时器*/
        SCTP_StopClientTimer(pCliCb);

           /*清空未发送的数据，关闭fd*/
        if (pCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            SCTP_closeCli(taskNo, pCliCb, NTL_SHTDWN_SEND);
        }

        /*关闭后置成初始化状态*/
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
    while(pServCb->pLatestCli != XNULLP)
    {
        SCTP_closeTsCli(pServCb->pLatestCli);
        if(pServCb->usageNum == 0)
        {
            break;
        }
    }

    /*关闭fd*/
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
函数名:SCTP_CliMsgShow
功能: 显示sctp client 每链路统计信息信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
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
函数名:SCTP_ServMsgShow
功能: 显示sctp server's client 每链路统计信息信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
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


