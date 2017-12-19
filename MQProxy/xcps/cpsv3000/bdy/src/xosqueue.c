/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosqueue.c
**
**  description:
**
**  author: wangzongyou
**
**  date:   2006.7.17
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification

**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include "xosqueue.h"
#include "xostrace.h"
#include "xoscfg.h"

/*-------------------------------------------------------------------------
                模块内部数据结构定义
-------------------------------------------------------------------------*/
typedef struct
{
   XU32 msg_count;
   XU32 chk_time;/*second*/
}t_FIDCHK;

/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/
XU32 g_msgqMax=10;

XBOOL MsgPflg[FID_MAX];
t_FIDCHK fid_chk[FID_MAX];
XU32 log_msg_discard = 0;
XU32 telnet_msg_discard = 0;

/*-------------------------------------------------------------------------
                模块内部静态函数
-------------------------------------------------------------------------*/

/************************************************************************
函数名: QUE_QuikCheck
功能：  检查异常消息Crash
输入：  chk_Fid,检查点

输出：
返回：无
说明：
************************************************************************/
XSTATIC XS32  QUE_QuickCheck(XU32 chk_Fid)
{
    long  l_now;
    XU32  time_delta=0;
    if(chk_Fid >=FID_MAX)
    {
      return XSUCC;
    }
    l_now = (long)time(XNULL);
    fid_chk[chk_Fid].msg_count++;
    time_delta = l_now - fid_chk[chk_Fid].chk_time;
    if(time_delta >60)
    {
        //短期峰值处理1分钟,计数器清零,赋当前时间
        fid_chk[chk_Fid].msg_count=0;
        fid_chk[chk_Fid].chk_time=l_now;
        return XSUCC;
    }
    if(chk_Fid == FID_TIME)
    {
       //如果是定时器异常,特殊处理
       if(fid_chk[chk_Fid].msg_count >g_msgqMax)
       {
          return XERROR;
       }
    }
    /*2 second check,limit 10 error packet*/
    if(time_delta <2)
    {
        if(fid_chk[chk_Fid].msg_count < g_msgqMax)
        {
            //未超时,低载
            return XSUCC;
        }
        else
        {
            //未超时,过载
            return XERROR;
        }
    }
    else
    {
        //超时
        if(fid_chk[chk_Fid].msg_count < g_msgqMax)
        {
            //超时,低载
            //计数器清零,赋当前时间
            fid_chk[chk_Fid].msg_count=0;
            fid_chk[chk_Fid].chk_time=l_now;
            return XSUCC;
        }
        else
        {
            //未超时,过载
            return XERROR;
        }
    }

    return XSUCC;
}

/************************************************************************
函数名: QUE_MsgQWalk
功能：  打印消息队列中的消息.
输入：  pMsgQ：消息队列

输出：
返回：无
说明：
************************************************************************/
XSTATIC XVOID   QUE_MsgQWalk(t_XOSMSGQ *pQue)
{
    XS32 listIndex;
    XU32 inum = 0;
    t_QUEELEM *pQueElem;
    t_XOSCOMMHEAD *ptMsg;

    if(!MsgPflg[pQue->tskIndex])
        return;

    /*msgq list show  */
    printf(  "tid %d queue list current has %d msgs.\r\n",
    pQue->tskIndex,XOS_listCurSize(pQue->queueList));

    printf( "%-18s%-18s%-10s%-10s%-10s\r\n",
    "srcfid","destfid","No.msg","msgID","prio");

    MsgPflg[pQue->tskIndex]  = XFALSE;

    listIndex =  XOS_listHead(pQue->queueList);
    while(listIndex != XERROR)
    {
        pQueElem = (t_QUEELEM*)XNULLP;
        pQueElem = (t_QUEELEM*)XOS_listGetElem(pQue->queueList, listIndex);

        if((pQueElem != XNULLP) && ((XPOINT)(pQueElem->pMsg) >  eMAXPrio)) // why?
        {
            ptMsg = (t_XOSCOMMHEAD*)(pQueElem->pMsg);
            /*printf("from %d to %d num %d msgid is %d and msgprio is %d and all msg is %d\n",
            ptMsg->datasrc.FID, ptMsg->datadest.FID,inum++,ptMsg->msgID,ptMsg->prio,);
            */
            printf( "%-18d%-18d%-10d%-10d%-10d\r\n",
            ptMsg->datasrc.FID,
            ptMsg->datadest.FID,
            inum++,
            ptMsg->msgID,
            ptMsg->prio
            );
        }
        listIndex =  XOS_listNext(pQue->queueList,listIndex);
    }
    printf(  "------------------------------------------\r\n");

}

/************************************************************************
函数名: QUE_MsgQCreate
功能：  创建消息队列
输入：  maxMsgs：消息队列中最大的消息数目
                   maxMsgLength：每条消息的最大长度

输出：  msgq 消息队列标识
返回： 函数操作成功返回XSUCC, 函数操作失败返回XERROR
说明：
************************************************************************/
XPUBLIC XS32   QUE_MsgQCreate(t_XOSMSGQ *pMsgQ,XU32 maxMsgs)
{
    XS32 i;
    XS32 nodeIndex;
    t_QUEELEM queElem;

    XOS_Trace(MD(FID_ROOT, PL_MIN), "QUE_MsgQCreate()");
    /*入口的安全性检查*/
    if(pMsgQ == XNULLP
    || maxMsgs == 0 ||maxMsgs > 0xffff)/*最大长度不能超过u16的最大值*/
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQCreate()->bad input params !max msgs: %d", maxMsgs);
        return XERROR;
    }

    /*创建list*/
    pMsgQ->queueList = (XOS_HLIST)XNULLP;

    pMsgQ->queueList = XOS_listConstruct(sizeof(t_QUEELEM), maxMsgs+eMAXPrio+1, " ");
    if(pMsgQ->queueList == XNULLP)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQCreate()->create msg queue failed !max msgs: %d", maxMsgs);
        return XERROR;
    }
    /*填写各优先级的归宿节点*/
    for(i = eMinPrio; i<=eMAXPrio; i++)
    {
        XOS_MemSet(&queElem, 0, sizeof(t_QUEELEM));
        queElem.pMsg = (XCHAR*)(XPOINT)i;

        nodeIndex = XOS_listAddTail(pMsgQ->queueList, &queElem);
        if(nodeIndex == XERROR)
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQCreate()->add the node [%d] failed", i);
            XOS_listDestruct(pMsgQ->queueList);
            return XERROR;
        }
        pMsgQ->prio[i] = nodeIndex;
    }

    pMsgQ->isFull = 0;
    /*创建琐*/
    if(XSUCC != XOS_MutexCreate(&(pMsgQ->queueLock)))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQCreate()->create queueLock failed !");
        XOS_listDestruct(pMsgQ->queueList);
        return XERROR;
    }

    /*创建信号量*/
    if(XSUCC != XOS_SemCreate(&(pMsgQ->sem), 0))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQCreate()->create queue semphore failed !");
        XOS_listDestruct(pMsgQ->queueList);
        XOS_MutexDelete(&(pMsgQ->queueLock));
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
函数名: QUE_MsgQDelete
功能：  删除一个消息队列
输入：  pMsgQ  消息队列标识
输出：  N/A
返回： 函数操作成功返回XSUCC, 函数操作失败返回XERROR
说明：
************************************************************************/
XPUBLIC XS32  QUE_MsgQDelete(t_XOSMSGQ *pMsgQ)
{
    XU32 i;
    t_QUEELEM *pQueElem;

    XOS_MutexLock(&(pMsgQ->queueLock));
    XOS_SemDelete(&pMsgQ->sem);
    /*释放队列中的消息内存*/
    if(XOS_listCurSize(pMsgQ->queueList) > (eMAXPrio+1))
    {
        for(i = eMinPrio; i<eMAXPrio; i++)
        {
            pQueElem = (t_QUEELEM*)XNULLP;
            pQueElem = (t_QUEELEM*)XOS_listGetElem(pMsgQ->queueList, XOS_listNext(pMsgQ->queueList, pMsgQ->prio[i]));
            if(pQueElem != XNULLP && (XPOINT)(pQueElem->pMsg) != i+1)
            {
                XOS_MsgMemFree(FID_ROOT, (t_XOSCOMMHEAD*)(pQueElem->pMsg));
            }
        }
    }
    /*释放链表*/
    XOS_listDestruct(pMsgQ->queueList);
    XOS_MutexUnlock(&(pMsgQ->queueLock));
    XOS_MutexDelete(&(pMsgQ->queueLock));
    return XSUCC;

}

/************************************************************************
函数名: QUE_MsgQSend
功能： 向一个消息队列发送消息
输入： pQue  消息队列标识
                 pMsg 消息缓冲区的结构指针
                 prio 发送消息的优先级
输出： N/A
返回：函数操作成功返回XSUCC, 函数操作失败返回XERROR
说明：
************************************************************************/
//static XU32 fid_ntl_sem_get=0;
//static XU32 fid_ntl_sem_put=0;
XPUBLIC XS32  QUE_MsgQSend(t_XOSMSGQ *pQue, t_XOSCOMMHEAD*pMsg, e_MSGPRIO prio)
{
    t_QUEELEM queElem;
    XS32 ret;
    XS32 curMsgs;

    if (XNULLP == pQue )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "QUE_MsgQSend(),bad input param,input que is null!");
        return XERROR;
    }

    if(XFALSE == pQue->init)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "QUE_MsgQSend(),msg queue not init!");
        return XERROR;
    }
        
    if ( XNULLP == pMsg)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "QUE_MsgQSend(),bad input param,input msg is null!");
        return XERROR;
    }

    if ( prio >=eMAXPrio )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "QUE_MsgQSend(),pri is illegal!srcFid[%d],dstFid[%d],msgid[%d],pri[%d]",
                                            pMsg->datasrc.FID,
                                            pMsg->datadest.FID,
                                            pMsg->msgID,pMsg->prio);
        return XERROR;
    }

    /*填写消息*/
    XOS_MemSet(&queElem, 0, sizeof(t_QUEELEM));
    queElem.pMsg = (XCHAR*) pMsg;

    /* 发送消息*/
    XOS_MutexLock(&(pQue->queueLock));    
    curMsgs = XOS_listCurSize(pQue->queueList);

    /* 超过95% */
    if(XOS_listMaxSize(pQue->queueList) -10 <= curMsgs)
    {        
        /*当log模块和telnet模块的消息队列满时，不能再写trace，否则，有可能无限递归。也不进行重启。*/
        if (pMsg->datadest.FID == FID_LOG)
        {
            log_msg_discard++;
            XOS_MutexUnlock(&(pQue->queueLock));

            return XERROR;
        }
        if (pMsg->datadest.FID == FID_TELNETD)
        {
            telnet_msg_discard++;
            XOS_MutexUnlock(&(pQue->queueLock));

            return XERROR;
        }
        
        if(QUE_QuickCheck(pMsg->datadest.FID) == XSUCC)
        {
            XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "QUE_MsgQSend()->que is full, discard msg FID[%d] to FID[%d] prio[%d] !",
                 pMsg->datasrc.FID,pMsg->datadest.FID,pMsg->prio);
        }

        XOS_MutexUnlock(&(pQue->queueLock));

        XOS_ResetByQueFull(pQue->tskIndex,1);

        return XERROR;
    }
    
    /* 超过90% */
    if(XOS_listMaxSize(pQue->queueList)*9 <  curMsgs*10)
    {        
        /*当log模块和telnet模块的消息队列满时，不能再写trace，否则，有可能无限递归。*/
        if(QUE_QuickCheck(pMsg->datadest.FID) == XSUCC && pMsg->datadest.FID != FID_LOG && pMsg->datadest.FID != FID_TELNETD)
        {
            XOS_CpsTrace(MD(FID_ROOT, PL_WARN), "QUE_MsgQSend()->que is full, discard msg FID[%d] to FID[%d] prio[%d] !",
                 pMsg->datasrc.FID,pMsg->datadest.FID,pMsg->prio);
        }

        /*静态计数器,log模块和telnet模块的消息队列满时,不进行重启*/
        if (0 == pQue->isFull && pMsg->datadest.FID != FID_LOG && pMsg->datadest.FID != FID_TELNETD)
        {
            XOS_ResetByQueFull(pQue->tskIndex,0);
            pQue->isFull++;
        }
    }
    else
    {
        /* 恢复到90%以下，则重置 */
        pQue->isFull = 0;
    }
    /*消息放到消息队列里*/
    ret = XOS_listAdd(pQue->queueList, pQue->prio[pMsg->prio], (nodeType)&queElem);
    if(ret == XERROR)
    {
        if(QUE_QuickCheck(pMsg->datasrc.FID) == XSUCC || QUE_QuickCheck(pMsg->datadest.FID) == XSUCC)
        {
            XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "QUE_MsgQSend()->add msg FID[%d] to FID[%d]  to que failed !, que cursize: %d",
            pMsg->datasrc.FID,pMsg->datadest.FID,XOS_listCurSize(pQue->queueList));
        }

        QUE_MsgQWalk(pQue);

        XOS_MutexUnlock(&(pQue->queueLock));
        return XERROR;
    }

    /*调整游标*/
    pQue->prioCursor = XOS_MAX(pQue->prioCursor, (pMsg->prio+1));

    /*解琐*/
    XOS_MutexUnlock(&(pQue->queueLock));

    /*释放信号量*/
    XOS_SemPut(&(pQue->sem));

    return XSUCC;

}

/************************************************************************
函数名: QUE_MsgQRecv
功能：  从一个消息队列接收一条消息
输入：  pQue  消息队列标识
                  ppMsg  指向消息的缓冲区结构地址的指针
输出：
返回：函数操作成功返回XSUCC, 函数操作失败返回XERROR
说明：
************************************************************************/
XPUBLIC XS32  QUE_MsgQRecv(t_XOSMSGQ *pQue, t_XOSCOMMHEAD **ppMsg)
{
    XU32   i;
    XBOOL isRecieve;
    t_QUEELEM *pQueElem;
    XS32 listIndex;

    /*入口安全性检查*/
    if (pQue == XNULLP || ppMsg == XNULLP)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQRecv()-> bad input param!");
        return XERROR;
    }

    /*等待信号量*/
    XOS_SemGet(&(pQue->sem));
/*
   if(pQue->tskIndex == FID_NTL)
   {
     fid_ntl_sem_get++;
   }
*/
    /*接收消息*/
    /*先接受优先级高的消息*/
    XOS_MutexLock(&(pQue->queueLock));
    isRecieve = XFALSE;
    for(i = pQue->prioCursor; i>eMinPrio; i--)
    {
        pQueElem = (t_QUEELEM*)XNULLP;
        listIndex = XOS_listPrev(pQue->queueList, pQue->prio[i]);
        pQueElem = (t_QUEELEM*)XOS_listGetElem(pQue->queueList, listIndex);
        if(pQueElem != XNULLP && (XPOINT)(pQueElem->pMsg) != (XPOINT)(i-1))
        {
            *ppMsg = (t_XOSCOMMHEAD*)(pQueElem->pMsg);
            XOS_listDelete(pQue->queueList, listIndex);
            isRecieve = XTRUE;
            pQue->prioCursor = i;
            break;
        }
    }
    /*解琐*/
    XOS_MutexUnlock(&(pQue->queueLock));

    /* 没有收到消息, 肯定是出了错*/
    if(!isRecieve)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQRecv()-> not recieve msg!que cursize:%d",
                   XOS_listCurSize(pQue->queueList));
        return XERROR;
    }
    return XSUCC;

}

XPUBLIC int msgq_maxset(int q_max)
{
   if(q_max <10 )
   {
     g_msgqMax = 10;
   }else
   {
     g_msgqMax = q_max;
   }
   printf("max msg count is set to %d.\r\n",g_msgqMax);
   return 0;
}

XPUBLIC int msgq_maxshow()
{
   int i=0;
   for(i=0;i<FID_MAX;i++)
   {
     if(fid_chk[i].msg_count >0)
     {
        printf("FID[%d],current max msg count is %d.\r\n",i,fid_chk[i].msg_count);
     }

   }
   return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */


