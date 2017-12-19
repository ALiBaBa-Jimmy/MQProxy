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
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xosqueue.h"
#include "xostrace.h"
#include "xoscfg.h"

/*-------------------------------------------------------------------------
                ģ���ڲ����ݽṹ����
-------------------------------------------------------------------------*/
typedef struct
{
   XU32 msg_count;
   XU32 chk_time;/*second*/
}t_FIDCHK;

/*-------------------------------------------------------------------------
                ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/
XU32 g_msgqMax=10;

XBOOL MsgPflg[FID_MAX];
t_FIDCHK fid_chk[FID_MAX];
XU32 log_msg_discard = 0;
XU32 telnet_msg_discard = 0;

/*-------------------------------------------------------------------------
                ģ���ڲ���̬����
-------------------------------------------------------------------------*/

/************************************************************************
������: QUE_QuikCheck
���ܣ�  ����쳣��ϢCrash
���룺  chk_Fid,����

�����
���أ���
˵����
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
        //���ڷ�ֵ����1����,����������,����ǰʱ��
        fid_chk[chk_Fid].msg_count=0;
        fid_chk[chk_Fid].chk_time=l_now;
        return XSUCC;
    }
    if(chk_Fid == FID_TIME)
    {
       //����Ƕ�ʱ���쳣,���⴦��
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
            //δ��ʱ,����
            return XSUCC;
        }
        else
        {
            //δ��ʱ,����
            return XERROR;
        }
    }
    else
    {
        //��ʱ
        if(fid_chk[chk_Fid].msg_count < g_msgqMax)
        {
            //��ʱ,����
            //����������,����ǰʱ��
            fid_chk[chk_Fid].msg_count=0;
            fid_chk[chk_Fid].chk_time=l_now;
            return XSUCC;
        }
        else
        {
            //δ��ʱ,����
            return XERROR;
        }
    }

    return XSUCC;
}

/************************************************************************
������: QUE_MsgQWalk
���ܣ�  ��ӡ��Ϣ�����е���Ϣ.
���룺  pMsgQ����Ϣ����

�����
���أ���
˵����
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
������: QUE_MsgQCreate
���ܣ�  ������Ϣ����
���룺  maxMsgs����Ϣ������������Ϣ��Ŀ
                   maxMsgLength��ÿ����Ϣ����󳤶�

�����  msgq ��Ϣ���б�ʶ
���أ� ���������ɹ�����XSUCC, ��������ʧ�ܷ���XERROR
˵����
************************************************************************/
XPUBLIC XS32   QUE_MsgQCreate(t_XOSMSGQ *pMsgQ,XU32 maxMsgs)
{
    XS32 i;
    XS32 nodeIndex;
    t_QUEELEM queElem;

    XOS_Trace(MD(FID_ROOT, PL_MIN), "QUE_MsgQCreate()");
    /*��ڵİ�ȫ�Լ��*/
    if(pMsgQ == XNULLP
    || maxMsgs == 0 ||maxMsgs > 0xffff)/*��󳤶Ȳ��ܳ���u16�����ֵ*/
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQCreate()->bad input params !max msgs: %d", maxMsgs);
        return XERROR;
    }

    /*����list*/
    pMsgQ->queueList = (XOS_HLIST)XNULLP;

    pMsgQ->queueList = XOS_listConstruct(sizeof(t_QUEELEM), maxMsgs+eMAXPrio+1, " ");
    if(pMsgQ->queueList == XNULLP)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQCreate()->create msg queue failed !max msgs: %d", maxMsgs);
        return XERROR;
    }
    /*��д�����ȼ��Ĺ��޽ڵ�*/
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
    /*������*/
    if(XSUCC != XOS_MutexCreate(&(pMsgQ->queueLock)))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQCreate()->create queueLock failed !");
        XOS_listDestruct(pMsgQ->queueList);
        return XERROR;
    }

    /*�����ź���*/
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
������: QUE_MsgQDelete
���ܣ�  ɾ��һ����Ϣ����
���룺  pMsgQ  ��Ϣ���б�ʶ
�����  N/A
���أ� ���������ɹ�����XSUCC, ��������ʧ�ܷ���XERROR
˵����
************************************************************************/
XPUBLIC XS32  QUE_MsgQDelete(t_XOSMSGQ *pMsgQ)
{
    XU32 i;
    t_QUEELEM *pQueElem;

    XOS_MutexLock(&(pMsgQ->queueLock));
    XOS_SemDelete(&pMsgQ->sem);
    /*�ͷŶ����е���Ϣ�ڴ�*/
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
    /*�ͷ�����*/
    XOS_listDestruct(pMsgQ->queueList);
    XOS_MutexUnlock(&(pMsgQ->queueLock));
    XOS_MutexDelete(&(pMsgQ->queueLock));
    return XSUCC;

}

/************************************************************************
������: QUE_MsgQSend
���ܣ� ��һ����Ϣ���з�����Ϣ
���룺 pQue  ��Ϣ���б�ʶ
                 pMsg ��Ϣ�������Ľṹָ��
                 prio ������Ϣ�����ȼ�
����� N/A
���أ����������ɹ�����XSUCC, ��������ʧ�ܷ���XERROR
˵����
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

    /*��д��Ϣ*/
    XOS_MemSet(&queElem, 0, sizeof(t_QUEELEM));
    queElem.pMsg = (XCHAR*) pMsg;

    /* ������Ϣ*/
    XOS_MutexLock(&(pQue->queueLock));    
    curMsgs = XOS_listCurSize(pQue->queueList);

    /* ����95% */
    if(XOS_listMaxSize(pQue->queueList) -10 <= curMsgs)
    {        
        /*��logģ���telnetģ�����Ϣ������ʱ��������дtrace�������п������޵ݹ顣Ҳ������������*/
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
    
    /* ����90% */
    if(XOS_listMaxSize(pQue->queueList)*9 <  curMsgs*10)
    {        
        /*��logģ���telnetģ�����Ϣ������ʱ��������дtrace�������п������޵ݹ顣*/
        if(QUE_QuickCheck(pMsg->datadest.FID) == XSUCC && pMsg->datadest.FID != FID_LOG && pMsg->datadest.FID != FID_TELNETD)
        {
            XOS_CpsTrace(MD(FID_ROOT, PL_WARN), "QUE_MsgQSend()->que is full, discard msg FID[%d] to FID[%d] prio[%d] !",
                 pMsg->datasrc.FID,pMsg->datadest.FID,pMsg->prio);
        }

        /*��̬������,logģ���telnetģ�����Ϣ������ʱ,����������*/
        if (0 == pQue->isFull && pMsg->datadest.FID != FID_LOG && pMsg->datadest.FID != FID_TELNETD)
        {
            XOS_ResetByQueFull(pQue->tskIndex,0);
            pQue->isFull++;
        }
    }
    else
    {
        /* �ָ���90%���£������� */
        pQue->isFull = 0;
    }
    /*��Ϣ�ŵ���Ϣ������*/
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

    /*�����α�*/
    pQue->prioCursor = XOS_MAX(pQue->prioCursor, (pMsg->prio+1));

    /*����*/
    XOS_MutexUnlock(&(pQue->queueLock));

    /*�ͷ��ź���*/
    XOS_SemPut(&(pQue->sem));

    return XSUCC;

}

/************************************************************************
������: QUE_MsgQRecv
���ܣ�  ��һ����Ϣ���н���һ����Ϣ
���룺  pQue  ��Ϣ���б�ʶ
                  ppMsg  ָ����Ϣ�Ļ������ṹ��ַ��ָ��
�����
���أ����������ɹ�����XSUCC, ��������ʧ�ܷ���XERROR
˵����
************************************************************************/
XPUBLIC XS32  QUE_MsgQRecv(t_XOSMSGQ *pQue, t_XOSCOMMHEAD **ppMsg)
{
    XU32   i;
    XBOOL isRecieve;
    t_QUEELEM *pQueElem;
    XS32 listIndex;

    /*��ڰ�ȫ�Լ��*/
    if (pQue == XNULLP || ppMsg == XNULLP)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "QUE_MsgQRecv()-> bad input param!");
        return XERROR;
    }

    /*�ȴ��ź���*/
    XOS_SemGet(&(pQue->sem));
/*
   if(pQue->tskIndex == FID_NTL)
   {
     fid_ntl_sem_get++;
   }
*/
    /*������Ϣ*/
    /*�Ƚ������ȼ��ߵ���Ϣ*/
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
    /*����*/
    XOS_MutexUnlock(&(pQue->queueLock));

    /* û���յ���Ϣ, �϶��ǳ��˴�*/
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


