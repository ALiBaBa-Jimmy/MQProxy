/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, ��������ͨ�ż������޹�˾

 ******************************************************************************
  �� �� ��   : ha_deadwatch.c
  �� �� ��   : ����
  ��    ��   : liujun
  ��������   : 2014��12��26��
  ����޸�   :
  ��������   : ���� ��ѭ�����ʵ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��12��26��
    ��    ��   : liujun
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
 #ifdef XOS_LINUX
 
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/types.h>
#include <err.h>
#include <sys/epoll.h>

#include "xostype.h"
#include "xosencap.h"
#include "xostl.h"
#include "xosarray.h"
#include "xosmem.h"
#include "xosipmi.h"
#include "xoslist.h"

#include "ha_resource.h"
#include "ha_interface.h"

#include "ha_status_control.h"
#include "ha_deadwatch.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HA_DEAD_RECV_TIMEOUT                 2
#define HA_DEAD_MSG_COUNT                    1
#define HA_DEAD_WATCH_MAX                    255
#define HA_READ_ERROR                        -1
#define HA_KEEP_ALIVE_SEND_TIME              2000
#define HA_KEEP_ALIVE_RECV_TIME              1500
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
extern XS32 HA_SendMsgToModule(XU32 DstFid,XU16 MsgId, ST_XOS_HA_MSG *pstStatusMsg);
extern XS32 HA_SendXosMsg(XU32 DstFid, XU32 SrcFid, XU16 MsgId, ST_XOS_HA_MSG *pstStatusMsg);
extern XS32 HA_SocketInit(const XCHAR *path);

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

enum ENUM_EPOLL_EVT_TYPE
{
    HA_EPOLL_EVT_ACCEPT,                /* �������ӵ��¼� */
    HA_EPOLL_EVT_REG,                   /* ע������ */
    HA_EPOLL_EVT_HEART,                 /* �����ظ� */
};

typedef struct ha_epoll_info
{
    XS32 fd;                                    /* epoll ������������    */
    XS32 EventType;                             /* �¼�����              */
    ST_HA_DEAD_LOCK_INFO *pWatchInfo;           /* fd ��Ӧ���̼߳����Ϣ */
}ST_HA_EPOLL_INFO;
/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

static XOS_HLIST g_DealLockWatchList = XNULL;
static PTIMER g_TimerDeadWatch;
static XU32 g_DeadWatchMsgCount = 0;
/* Ĭ�ϴ�������صĿ��� */
static XBOOL g_DeadWatchDefaultOpen = XFALSE;
/* static PTIMER g_TimerDeadRecv;*/
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
XVOID HA_DeadWatchDefaultClose(XVOID)
{
    g_DeadWatchDefaultOpen = XFALSE;
}

XVOID HA_DeadWatchDefaultOpen(XVOID)
{
    g_DeadWatchDefaultOpen = XTRUE;
}

XBOOL HA_IsDefaultOpenDeadWatch(XVOID)
{
    return g_DeadWatchDefaultOpen;
}

static XU32 HA_GetMsgCount(XVOID)
{
    return g_DeadWatchMsgCount;
}

static XU32 HA_GetMsgCountPlus(XVOID)
{
    return g_DeadWatchMsgCount++;
}

/*****************************************************************************
 �� �� ��  : HA_DeadWatchCmp
 ��������  : ��������ȽϺ���
 �������  : nodeType element1  
             XVOID *param       
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
static XBOOL HA_DeadWatchCmp(nodeType element1, XVOID *param)
{
    ST_HA_DEAD_LOCK_INFO *node = element1;
    ST_HA_DEAD_LOCK_INFO *info = param;

    if (NULL == node || NULL == info)
    {
        return XFALSE;
    }

    if ((node->Fid == info->Fid) 
        && (node->Type == info->Type)
        && (node->Tid  == info->Tid))
    {
        return XTRUE;
    }

    return XFALSE;
}

/*****************************************************************************
 �� �� ��  : HA_DeadWatchFindInfo
 ��������  : ��ȡ�ڵ�
 �������  : XU32 Fid                       
             XU32 Tid                       
             ST_HA_DEAD_LOCK_INFO **result  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��8��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
static XS32 HA_DeadWatchFindInfo(XU32 Fid, XU32 Tid, 
                                            ST_HA_DEAD_LOCK_INFO **result)
{
    XS32 nodeIndex = 0;

    ST_HA_DEAD_LOCK_INFO stDeadLockInfo = {0};
    ST_HA_DEAD_LOCK_INFO *pElement = NULL;

    if (NULL == result)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "No space to recv result.");
        return XERROR;
    }

    stDeadLockInfo.Fid = Fid;
    stDeadLockInfo.Tid = Tid;
    stDeadLockInfo.Type = HA_WATCH_THREAD;

    nodeIndex = XOS_listHead(g_DealLockWatchList);
    nodeIndex = XOS_listFind(g_DealLockWatchList, nodeIndex, &stDeadLockInfo);
    if (XERROR == nodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Fid[%d] Tid[%d] Type[%d] have not registered!",
                                                    Fid, Tid, HA_WATCH_THREAD);
        return XERROR;
    }

    pElement = XOS_listGetElem(g_DealLockWatchList, nodeIndex);
    if (NULL == pElement)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Can't get element from index:%d!",nodeIndex);
        return XERROR;
    }

    *result = pElement;
    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_DeadWatchUpdateMsgId
 ��������  : ���յ�������Ϣ�� �����̵߳�msgid
 �������  : XS32 Fid    
             XS32 Tid    
             XS32 MsgId  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_DeadWatchUpdateMsgId(XU32 Fid, XU32 Tid, XU32 MsgId)
{
    XS32 nodeIndex = 0;

    ST_HA_DEAD_LOCK_INFO stDeadLockInfo = {0};
    ST_HA_DEAD_LOCK_INFO *pElement = NULL;

    stDeadLockInfo.Fid = Fid;
    stDeadLockInfo.Tid = Tid;
    stDeadLockInfo.Type = HA_WATCH_BASE_XOS;

    nodeIndex = XOS_listHead(g_DealLockWatchList);
    nodeIndex = XOS_listFind(g_DealLockWatchList, nodeIndex, &stDeadLockInfo);
    if (XERROR == nodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Fid[%d] Tid[%d] Type[%d] have not registered!",
                                                    Fid, Tid, HA_WATCH_THREAD);
        return XERROR;
    }

    pElement = XOS_listGetElem(g_DealLockWatchList, nodeIndex);
    if (NULL == pElement)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Can't get element from index:%d!",nodeIndex);
        return XERROR;
    }

    pElement->MsgIndent = MsgId;

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_DeadWatchUpdateSock
 ��������  : �����̼߳�ص�ͨ���׽���
 �������  : XS32 Fid     
             XS32 Tid     
             XS32 SockFd  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_DeadWatchUpdateSock(XU32 Fid, XU32 Tid, XS32 SockFd)
{
    XS32 nodeIndex = 0;

    ST_HA_DEAD_LOCK_INFO stDeadLockInfo = {0};
    ST_HA_DEAD_LOCK_INFO *pElement = NULL;

    stDeadLockInfo.Fid = Fid;
    stDeadLockInfo.Tid = Tid;
    stDeadLockInfo.Type = HA_WATCH_THREAD;

    nodeIndex = XOS_listHead(g_DealLockWatchList);
    nodeIndex = XOS_listFind(g_DealLockWatchList, nodeIndex, &stDeadLockInfo);
    if (XERROR == nodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Fid[%d] Tid[%d] Type[%d] have not registered!",
                                                    Fid, Tid, HA_WATCH_THREAD);
        return XERROR;
    }

    pElement = XOS_listGetElem(g_DealLockWatchList, nodeIndex);
    if (NULL == pElement)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Can't get element from index:%d!",nodeIndex);
        return XERROR;
    }

    pElement->Sockfd = SockFd;

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_DeadWatchDel
 ��������  : ɾ��һ�����
 �������  : ST_HA_DEAD_LOCK_INFO *info  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_DeadWatchDel(ST_HA_DEAD_LOCK_INFO *info)
{
    XS32 nodeIndex = 0;
    XS32 result = XSUCC; 

    if (XNULL == info)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Dead Watch Add Failed:NULL param!");
        return XERROR;
    }

    nodeIndex = XOS_listHead(g_DealLockWatchList);
    nodeIndex = XOS_listFind(g_DealLockWatchList, nodeIndex, info);
    if (XERROR != nodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Fid[%d] Tid[%d] Type[%d] Have registered!",
                                                    info->Fid,info->Tid,info->Type);
        return XERROR;
    }

    if (XTRUE != XOS_listDelete(g_DealLockWatchList, nodeIndex))
    {
        result = XERROR;
    }

    return result;
}

/*****************************************************************************
 �� �� ��  : HA_DeadWatchAdd
 ��������  : ��Ӽ��
 �������  : ST_HA_DEAD_LOCK_INFO *info  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_DeadWatchAdd(ST_HA_DEAD_LOCK_INFO *info)
{
    XS32 nodeIndex = 0;
    XS32 result = XSUCC; 
    ST_HA_DEAD_LOCK_INFO *pWatch = NULL;

    if (XNULL == info)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Dead Watch Add Failed:NULL param!");
        return XERROR;
    }

    /* ���Ȳ����Ƿ������ͬ�Ľڵ� */
    nodeIndex = XOS_listHead(g_DealLockWatchList);
    nodeIndex = XOS_listFind(g_DealLockWatchList, nodeIndex, info);
    if (XERROR != nodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_WARN), "Fid[%d] Tid[%d] Type[%d] Have registered!",
                                                    info->Fid,info->Tid,info->Type);
        pWatch = XOS_listGetElem(g_DealLockWatchList, nodeIndex);
        if (NULL != pWatch)
        {
            pWatch->MsgIndent = HA_GetMsgCount();
        }
        
        return XSUCC;
    }

    /* ��ʼ����Ϣ��ʶ�� */
    info->MsgIndent = HA_GetMsgCount();
    /* ��ӵ�β�� */
    nodeIndex = XOS_listAddTail(g_DealLockWatchList, info);
    if (XERROR == nodeIndex)
    {
        result = XERROR;
        XOS_Trace(MD(FID_HA, PL_ERR), "Add Dead Watch to List failed!");
    }

    return result;
}

/*****************************************************************************
 �� �� ��  : HA_AddToDeadWatch
 ��������  : ����߳�������أ�XOS�ڲ�ʵ��
 �������  : XU32 fid  
             XU32 tid  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��15��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XVOID HA_AddToDeadWatch(XU32 fid, XU32 tid)
{
    XS32 ret = 0;
    ST_HA_DEAD_LOCK_INFO stWatchInfo = {0};

    /* �������� �Ƿ�����Ĭ�ϼ�������߳� */
    if (HA_IsDefaultOpenDeadWatch())
    {
        stWatchInfo.Fid      = fid;
        stWatchInfo.Tid      = tid;
        stWatchInfo.Type     = HA_WATCH_BASE_XOS;
        stWatchInfo.param    = NULL;
        stWatchInfo.CallBack = NULL;
        stWatchInfo.MsgIndent = 0;
    	
        ret = HA_DeadWatchAdd(&stWatchInfo);
        if (XSUCC != ret)
        {
            XOS_Trace(MD(FID_HA, PL_ERR), "Add Dead Watch failed!");
        }
    }
    return;
}

/*****************************************************************************
 �� �� ��  : HA_DeadWatchLstInit
 ��������  : ������� �����Դ��ʼ��
 �������  : XOS_HLIST *pListHead  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_DeadWatchLstInit(XOS_HLIST *pListHead)
{
    if (NULL == pListHead)
    {
        return XERROR;
    }
    /* ��ʼ������������� */
    *pListHead = XOS_listConstruct(sizeof(ST_HA_DEAD_LOCK_INFO), 
                                                HA_DEAD_WATCH_MAX, "DEAD WATCH LIST");
    if (XNULLP == *pListHead)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "ha Dead Watch report list create failed!");
        return XERROR;
    }

    XOS_listClear(*pListHead);

    XOS_listSetCompareFunc(*pListHead, HA_DeadWatchCmp);

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_SendMsgToWatchMod
 ��������  : ������Ϣ��������߳�
 �������  : XOS_HLIST ListHead  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_SendMsgToWatchMod(XOS_HLIST ListHead)
{
    ST_XOS_HA_MSG stStatusMsg = {0};
    ST_HA_DEAD_LOCK_INFO *pDeadWatch = NULL;
    XS32 nodeIndex = 0 ,len = 0;
    /* ��Ϣ���  */
    XU32 MsgCount = 0;

    MsgCount = HA_GetMsgCountPlus();
    
    stStatusMsg.MsgType   = HA_WATCH_MSG_REQ;
    stStatusMsg.Status    = HA_GetCurrentStatus();
    
    nodeIndex = XOS_listHead(ListHead);
    
    for (; XERROR != nodeIndex;
        nodeIndex = XOS_listNext(ListHead, nodeIndex))
    {
        pDeadWatch = XOS_listGetElem(ListHead, nodeIndex);
        if (NULL == pDeadWatch)
        {
            continue;
        }
        
        /* ����2�����ڵ���Ϣ��ʧ ����Ϊ����/��ѭ���������ص� */
        if (MsgCount - pDeadWatch->MsgIndent > HA_DEAD_MSG_COUNT)
        {
            pDeadWatch->CallBack(pDeadWatch->param);

            /* ����ģʽ��״̬����ģ���л�����״̬ */
            if (HA_STATUS_CLOSE != ha_get_current_status())
            {
                ha_change_status_cmd(HA_STATUS_STANDBY);
            }
            else
            {
                /* ����ģʽ ֱ���л�*/
                HA_ChangeStatusCallBack(HA_STATUS_STANDBY);
            }
            
            XOS_Trace(MD(FID_HA, PL_WARN), "Mod[%d] Tid[%d] Maybe in dead loop!",
                                                pDeadWatch->Fid,pDeadWatch->Tid);
            /* �رո��̼߳�ص��׽��� �ͷ�epoll��Դ*/
            if (-1 != pDeadWatch->Sockfd)
            {
                close (pDeadWatch->Sockfd);
                pDeadWatch->Sockfd = -1;
                
                XOS_Free(pDeadWatch->pEpollBuf);
                pDeadWatch->pEpollBuf = NULL;
            }
        }
        
        stStatusMsg.Fid = pDeadWatch->Fid;
        stStatusMsg.ThreadId  = pDeadWatch->Tid;
        stStatusMsg.TimeStamp = time(NULL);
        stStatusMsg.MsgIndent = MsgCount;
        
        if (HA_WATCH_BASE_XOS == pDeadWatch->Type)
        {
            /* ����XOSƽ̨����Ϣ���� */
            stStatusMsg.MsgMethod = HA_WATCH_BASE_XOS;
            HA_SendMsgToModule(pDeadWatch->Fid, HA_WATCH_MSG_REQ, &stStatusMsg);
        }
        else if (HA_WATCH_THREAD == pDeadWatch->Type
            && -1 != pDeadWatch->Sockfd)
        {
            /* UNIX���׽��� */
            stStatusMsg.MsgMethod = HA_WATCH_THREAD;
            len = send(pDeadWatch->Sockfd, &stStatusMsg,sizeof(stStatusMsg),0);
            if (len < 0)
            {
                close(pDeadWatch->Sockfd);
                pDeadWatch->Sockfd = -1;
                XOS_Trace(FILI, FID_HA, PL_ERR,"ERROR:Send to fid[%d] tid[%d]!",
                                            pDeadWatch->Fid, pDeadWatch->Tid);
            }
        }
        else
        {
            XOS_Trace(FILI, FID_HA, PL_ERR,"ERROR:wrong info fid[%d] tid[%d]!",
                                                pDeadWatch->Fid, pDeadWatch->Tid);
        }
    }
}

/*****************************************************************************
 �� �� ��  : HA_RecvRegMsg
 ��������  : �������������Ϣ�ظ�������
 �������  : XS32 Sockfd  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
static int HA_RecvRegMsg(XS32 Sockfd,ST_XOS_HA_MSG *pRegMsg)
{
    XS8 buf[1024] = {0};
    XS32 RecvLen = 0;

    /* ���׽��ֽ�����Ϣ */
    RecvLen = recv(Sockfd, buf, sizeof(buf), 0);
    if (RecvLen <= 0)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Recv failed: %s", strerror(errno));
        goto FAILED;
    }

    if (RecvLen < sizeof(ST_XOS_HA_MSG))
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Msg len wrong: %d",RecvLen);
        goto FAILED;
    }

    /* �̼߳��ע�� */
    memcpy(pRegMsg, buf, sizeof(ST_XOS_HA_MSG));
    if (HA_WATCH_THREAD != pRegMsg->MsgMethod 
        || (HA_WATCH_MSG_REG != pRegMsg->MsgType
        &&  HA_WATCH_MSG_RES != pRegMsg->MsgType))
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Msg Type[%d] or Method[%d] not match!",
                                            pRegMsg->MsgType,pRegMsg->MsgMethod);
        goto FAILED;
    }
    
    return XSUCC;
FAILED:

    return XERROR;
}

/*****************************************************************************
 �� �� ��  : HA_ParseHeartBeatMsgFromQue
 ��������  : �����յ�����Ϣ
 �������  : t_XOSCOMMHEAD *pMsg  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��26��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_ParseHeartBeatMsgFromQue(t_XOSCOMMHEAD *pMsg)
{
    ST_XOS_HA_MSG *pRes = NULL;
    XS32 ret = 0;
    
    if (NULL == pMsg)
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"msg is NULL!");
        return ;
    }

    if(NULL == (pMsg->message))
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"msg has no data");
        return ;
    }

    if (pMsg->length < sizeof(ST_XOS_HA_MSG))
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"data len is too less:%d",pMsg->length);
        return ;
    }

    if (HA_WATCH_MSG_RES != pMsg->msgID)
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"wrong msg id:%d,need be:%d",
                                                   pMsg->msgID, HA_WATCH_MSG_RES);
        return ;
    }

    pRes = pMsg->message;
    if (HA_WATCH_BASE_XOS != pRes->MsgMethod || HA_WATCH_MSG_RES != pRes->MsgType)
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"wrong msg ,method[%d] type[%d]",
                                                   pRes->MsgMethod, pRes->MsgType);
        return;
    }

    ret = HA_DeadWatchUpdateMsgId(pRes->Fid, pRes->ThreadId, pRes->MsgIndent);
    if (XERROR == ret)
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"Update Msg id failed");
    }
}

/*****************************************************************************
 �� �� ��  : HA_RecvHeartBeatMsg
 ��������  : ����������Ϣ
 �������  : XOS_HLIST ListHead  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_RecvHeartBeatMsg(XOS_HLIST ListHead)
{
    XS32 nodeIndex = 0;
    XS32 readlen = 0;
    ST_XOS_HA_MSG stRecvBuf = {0};
    
    ST_HA_DEAD_LOCK_INFO *pDeadWatch = NULL;
    nodeIndex = XOS_listHead(ListHead);

    /* ���������׽��� ��ȡ�����ظ� */
    for (; XERROR != nodeIndex;
         nodeIndex = XOS_listNext(ListHead, nodeIndex))
    {
        pDeadWatch = XOS_listGetElem(ListHead, nodeIndex);
        if (NULL == pDeadWatch)
        {
            continue;
        }
        
        if ((HA_WATCH_THREAD == pDeadWatch->Type) && (pDeadWatch->Sockfd > 0))
        {
            readlen = read(pDeadWatch->Sockfd, &stRecvBuf, sizeof(stRecvBuf));
            if (readlen < sizeof(stRecvBuf) && HA_READ_ERROR != readlen)
            {
                continue;
            }
            else if (HA_READ_ERROR == readlen 
                && ((EWOULDBLOCK == errno) || (EAGAIN == errno)))
            {
                /* ������ģʽ����Ҫ�´��ٶ� */
                continue;
            }
            else if (HA_READ_ERROR == readlen)
            {
                /* ������� ����Ҫ�ر��׽��� */
                close(pDeadWatch->Sockfd);
                pDeadWatch->Sockfd = 0;
                continue;
            }
            
            if (HA_WATCH_MSG_RES != stRecvBuf.MsgType
                || HA_WATCH_THREAD !=stRecvBuf.MsgMethod
                || stRecvBuf.Fid != pDeadWatch->Fid
                || stRecvBuf.ThreadId != pDeadWatch->Tid)
            {
                /* ��ϢУ�� ʧ�� */
                close(pDeadWatch->Sockfd);
                pDeadWatch->Sockfd = 0;
                continue;
            }
            
            pDeadWatch->MsgIndent = stRecvBuf.MsgIndent;
        }
    }
}

/*****************************************************************************
 �� �� ��  : HA_ThreadAccept
 ��������  : ���������߳�
 �������  : void *arg  
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void *HA_ThreadAccept(void *arg)
{
    fd_set readfd ;
    XS32 NewConn = 0;
    XS32 ret = 0,val = 0;
    socklen_t len = 0;
    XS32 Sockfd = *(XS32 *)arg;
    struct sockaddr_un addr = {0};
    
    struct timeval timeout = {0};
    ST_XOS_HA_MSG RegMsg = {0};

    while (1)
    {
        /* �߳��˳� */
        if (Sockfd <= 0)
        {
            XOS_Trace(MD(FID_HA, PL_ERR),"Socket is invalid");
            break;
        }
        
        len = sizeof(addr);
        memset(&addr, 0, sizeof(addr));

        /* �ȴ��µ����� */
        NewConn = accept(Sockfd, (struct sockaddr *)&addr, &len);
        if (NewConn < 0)
        {
            XOS_Trace(MD(FID_HA, PL_ERR),"Accept failed: %s", strerror(errno));
            
            continue;
        }
        
        timeout.tv_sec  = HA_DEAD_RECV_TIMEOUT;
        timeout.tv_usec = 0;

        FD_ZERO(&readfd);
        FD_SET(NewConn, &readfd);
        ret = select (NewConn + 1, &readfd, 0, 0, &timeout);
        if (ret < 0)
        {
            continue;
        }
        else if (0 == ret)
        {
            /* ��ʱ ��ر� */
            close(NewConn);
        }
        else
        {
            /* ���ղ� ������Ϣ */
            if (FD_ISSET(NewConn,&readfd))
            {
                memset(&RegMsg, 0, sizeof(RegMsg));
                ret = HA_RecvRegMsg(NewConn, &RegMsg);
                if (XSUCC != ret)
                {
                    /* ����ȷ��Ϣ �ر� */
                    close(NewConn);
                    
                }

                /* ���׽��ָ��µ�����ڵ� */
                ret = HA_DeadWatchUpdateSock(RegMsg.Fid,RegMsg.ThreadId, NewConn);
                if (XSUCC != ret)
                {
                    XOS_Trace(MD(FID_HA, PL_ERR),"Update sockfd failed!");
                    close(NewConn);
                    continue;
                }

                /*  �����µ�����Ϊ������  */
                val = fcntl (NewConn, F_GETFL, 0);
                fcntl (NewConn, F_SETFL, (val | O_NONBLOCK));
            }
        }
    }

    return NULL;
}

/*****************************************************************************
 �� �� ��  : HA_UnixServInit
 ��������  : ����˳�ʼ��
 �������  : const char *path  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
static XS32 HA_UnixServInit(const char *path)
{
    XS32 ret;
    XS32 sock, len;
    struct sockaddr_un serv;
    mode_t old_mask;

    if (NULL == path)
    {
        return XERROR;
    }

    unlink (path);

    old_mask = umask (0077);

    sock = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        XOS_Trace(MD(FID_HA, PL_INFO), "socket:%s",strerror(errno));
        return XERROR;
    }

    /* server socket. */
    memset (&serv, 0, sizeof (struct sockaddr_un));
    serv.sun_family = AF_UNIX;
    strncpy (serv.sun_path, path, strlen (path));
#ifdef HAVE_SUN_LEN
    len = serv.sun_len = SUN_LEN(&serv);
#else
    len = sizeof (serv.sun_family) + strlen (serv.sun_path);
#endif /* HAVE_SUN_LEN */

    ret = bind (sock, (struct sockaddr *) &serv, len);
    if (ret < 0)
    {
        XOS_Trace(MD(FID_HA, PL_INFO), "bind:%s",strerror(errno));
        close (sock); 
        return XERROR;
    }

    ret = listen (sock, 5);
    if (ret < 0)
    {
        perror ("listen");
        close (sock); 
        return XERROR;
    }

    umask (old_mask);
    XOS_Trace(MD(FID_HA, PL_INFO),"ready to recv new connect!\n");

    return sock;
}

static XS32 HA_ParseMsgByEpoll(XS32 EpollFd, ST_HA_EPOLL_INFO *pReadInfo)
{
    XS32 result = 0;
    ST_XOS_HA_MSG RegMsg = {0};
    ST_HA_DEAD_LOCK_INFO *pInfo = NULL;
    struct epoll_event evnt = {0};

    if (NULL == pReadInfo)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"pReadInfo is NULL");
        return XERROR;
    }
    /* ���ղ�������Ϣ */
    result = HA_RecvRegMsg(pReadInfo->fd, &RegMsg);
    if (XERROR == result)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Recv msg error");
        goto FAILED;
    }

    /* Ϊ�� ��˵���ǵ�һ�����ӵ�ע����Ϣ */
    if (NULL == pReadInfo->pWatchInfo)
    {
        /* ��Ҫ����sockfd������ڵ�Ķ�Ӧ��ϵ */
        result = HA_DeadWatchFindInfo(RegMsg.Fid, RegMsg.ThreadId, &pInfo);
        if (XERROR == result)
        {
            XOS_Trace(MD(FID_HA, PL_ERR),"Find watch info failed");
            goto FAILED;
        }

        /* ���½ڵ���Ϣ */
        pInfo->MsgIndent      = HA_GetMsgCount();
        pInfo->pEpollBuf      = pReadInfo;
        
        pReadInfo->pWatchInfo = pInfo;
        pInfo->Sockfd = pReadInfo->fd;

        /* ����epoll�ж�Ӧ���¼�����Ϣ�ṹ */
        pReadInfo->EventType = HA_EPOLL_EVT_HEART;
        evnt.data.ptr = pReadInfo;
        evnt.events   = EPOLLIN ;
        epoll_ctl(EpollFd, EPOLL_CTL_MOD, pReadInfo->fd, &evnt);
    }
    else
    {
        /* ��ε������ظ�������Ҫ����msgid */
        pInfo = pReadInfo->pWatchInfo;
        pInfo->MsgIndent = RegMsg.MsgIndent;
    }

    return XSUCC;
FAILED:

    /* ʧ�ܵ���� ��Ҫ��epoll��ɾ�� ���ر����� �ͷŽڵ��ڴ�*/
    evnt.data.ptr = pReadInfo;
    evnt.events   = EPOLLIN ;
    epoll_ctl(EpollFd, EPOLL_CTL_DEL, pReadInfo->fd, &evnt);
    close (pReadInfo->fd);
    free(pReadInfo);
    
    return XERROR;
}

static XS32 HA_EpollProcessAccept(XS32 EpollFd, XS32 AcceptFd)
{
    XS32 NewConn  = 0;
    socklen_t len = 0;
    struct sockaddr_un addr = {0};
    struct epoll_event evnt = {0};
    ST_HA_EPOLL_INFO *pData = NULL;
    
    len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    /* �ȴ��µ����� */
    NewConn = accept(AcceptFd, (struct sockaddr *)&addr, &len);
    if (NewConn < 0)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Accept failed: %s", strerror(errno));

        return HA_DEAD_ERR_ACCEPT;
    }

    pData = XOS_Malloc(sizeof(ST_HA_EPOLL_INFO));
    if (NULL == pData)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"malloc failed");
        
        return XERROR;
    }
    
    pData->fd         = NewConn;
    pData->EventType  = HA_EPOLL_EVT_REG;
    pData->pWatchInfo = NULL;

    evnt.data.ptr = pData;
    evnt.events   = EPOLLIN ;
    epoll_ctl(EpollFd, EPOLL_CTL_ADD, NewConn, &evnt);
    
    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_CreateAccptFdWithEpoll
 ��������  : ��������̵߳��׽��ַ���� ����ӵ�epoll����
 �������  : XS32 epfd                     
             ST_HA_EPOLL_INFO *pAccptInfo  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��15��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
static XS32 HA_CreateAccptFdWithEpoll(XS32 epfd, ST_HA_EPOLL_INFO *pAccptInfo)
{
    XS32 ret = 0;
    XS32 AcceptFd = 0;
    struct epoll_event ev;

    AcceptFd = HA_UnixServInit(XOS_HA_DEAD_WATCH_UNIX_PATH);
    if (XERROR == AcceptFd)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"accept fd init failed");
        return XERROR;
    }
    
    pAccptInfo->fd = AcceptFd;
    pAccptInfo->EventType  = HA_EPOLL_EVT_ACCEPT;
    pAccptInfo->pWatchInfo = NULL;
    
    ev.data.ptr = pAccptInfo;
    ev.events = EPOLLIN ;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, AcceptFd, &ev);
    if (-1 == ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"add accept to epoll fail: %s", strerror(errno));
    }

    return AcceptFd;
}

/*****************************************************************************
 �� �� ��  : HA_ThreadRecv
 ��������  : epollʵ����������߳�
 �������  : void *arg  
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��8��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void *HA_ThreadRecv(void *arg)
{
    XS32 epfd = -1,nfds = 0,idx = 0,ret = 0, acceptfd;
    ST_HA_EPOLL_INFO AccptInfo = {0};
    ST_HA_EPOLL_INFO *pData = NULL;
    
    struct epoll_event events[HA_DEAD_WATCH_MAX];

    epfd = epoll_create(HA_DEAD_WATCH_MAX);
    if (-1 == epfd)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"epoll create fail: %s", strerror(errno));
        return NULL;
    }

    acceptfd = HA_CreateAccptFdWithEpoll(epfd, &AccptInfo);
    if (XERROR == acceptfd)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Accept fd init failed");
        return NULL;
    }

    while (1)
    {
        nfds = epoll_wait(epfd, events, HA_DEAD_WATCH_MAX, -1);
        if (nfds <= 0)
        {
            XOS_Trace(MD(FID_HA, PL_ERR),"epoll wait fail: %s", strerror(errno));
            sleep(1);
            continue;
        }

        for (idx = 0; idx < nfds; idx++)
        {
            pData = events[idx].data.ptr;
            if (HA_EPOLL_EVT_ACCEPT == pData->EventType)
            {
                /* ������ */
                ret = HA_EpollProcessAccept( epfd, acceptfd);
                if (HA_DEAD_ERR_ACCEPT == ret)
                {
                    close (acceptfd);
                    acceptfd = HA_CreateAccptFdWithEpoll(epfd, &AccptInfo);
                    if (XERROR == acceptfd)
                    {
                        XOS_Trace(MD(FID_HA, PL_ERR),"Accept fd init failed");
                        sleep(2);
                    }
                }
            }
            else
            {
                HA_ParseMsgByEpoll(epfd, pData);
            }
        }
    }
    
}

/*****************************************************************************
 �� �� ��  : HA_DeadWatchCreateThread
 ��������  : ������������߳�
 �������  : XS32 sockfd  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS8 HA_DeadWatchCreateThread(void)
{
    pthread_t thread;
    pthread_attr_t attr;
    XS32 ret = 0;
    
    /* ����Ϊ�����߳� */
    ret = pthread_attr_init(&attr);
    if (SUCCESS != ret)
    {
        XOS_Trace(MD(FID_HA, PL_INFO),"init thread attr failed:%d\r\n",ret);
        return XERROR;
    }
    
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (SUCCESS != ret)
    {
        XOS_Trace(MD(FID_HA, PL_INFO),"set thread to detached failed:%d\r\n",ret);
        return XERROR;
    }
    
    ret = pthread_create(&thread, &attr, HA_ThreadRecv, NULL);
    if (SUCCESS != ret)
    {
        XOS_Trace(MD(FID_HA, PL_INFO),"create thread failed:%d\r\n",ret);
        return XERROR;
    }

    ret = pthread_attr_destroy(&attr);
    if (SUCCESS != ret)
    {
        XOS_Trace(MD(FID_HA, PL_INFO),"destroy attr failed:%d.\r\n",ret);
    }

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_DeadWatchInit
 ��������  : ������س�ʼ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS8 HA_DeadWatchInit(void)
{
    t_BACKPARA backpara;
    XS32 ret = 0;
    t_PARA timerpara;

    /* ��������߳� */
    if (XERROR == HA_DeadWatchCreateThread())
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Dead Watch thread Create failed");
        return XERROR;
    }
    
    if (XERROR == HA_DeadWatchLstInit(&g_DealLockWatchList))
    {
        goto FAILED;
    }

    /* ����������ط��Ͷ�ʱ�� */
    backpara.para1 = HA_TIMER_TYPE_DEAD_WATCH;
    backpara.para2 = (XU64)g_DealLockWatchList;

    timerpara.fid = FID_HA;
    timerpara.len = HA_KEEP_ALIVE_SEND_TIME;
    timerpara.mode = TIMER_TYPE_LOOP;
    timerpara.pre  = TIMER_PRE_LOW;
    ret = XOS_TimerStart(&g_TimerDeadWatch, &timerpara, &backpara);
    if (XERROR == ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Timer Create failed");
        goto FAILED;
    }

    /* ���������������ն�ʱ�� 
    timerpara.len = HA_KEEP_ALIVE_RECV_TIME;
    
    backpara.para1 = HA_TIMER_TYPE_DEAD_RECV;
    ret = XOS_TimerStart(&g_TimerDeadRecv, &timerpara, &backpara);
    if (XERROR == ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Timer Create failed");
        goto FAILED;
    }*/

    return XSUCC;
    
FAILED:

    if (NULL != g_DealLockWatchList)
    {
        XOS_listDestruct(g_DealLockWatchList);
    }

    XOS_TimerStop(FID_HA, g_TimerDeadWatch);
    /*XOS_TimerStop(FID_HA, g_TimerDeadRecv);*/

    return XERROR;
}

/*****************************************************************************
 �� �� ��  : HA_ClientReConnect
 ��������  : ���´����׽��ֲ�����
 �������  : XU32 fid  
             XU32 tid  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��26��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_ClientReConnect(XU32 fid, XU32 tid)
{
    ST_XOS_HA_MSG stRegMsg = {0};
    XS32 ret = 0;
    XS32 socket = 0;

    socket = HA_SocketInit(XOS_HA_DEAD_WATCH_UNIX_PATH);
    if (XERROR == socket)
    {
        return XERROR;
    }

    stRegMsg.Fid       = fid;
    stRegMsg.ThreadId  = tid;
    stRegMsg.MsgIndent = 0;
    stRegMsg.MsgMethod = HA_WATCH_THREAD;
    stRegMsg.MsgType   = HA_WATCH_MSG_REG;
    stRegMsg.TimeStamp = time(NULL);
    stRegMsg.Status    = HA_GetCurrentStatus();

    ret = send(socket, &stRegMsg, sizeof(stRegMsg), 0);
    if (ret < 0)
    {
        XOS_Trace(MD(fid, PL_ERR), "Send Init Dead watch failed!");
        close (socket);
        return XERROR;
    }

    return socket;
}

/*****************************************************************************
 �� �� ��  : HA_ThreadHelloProcess
 ��������  : ������̴߳�����������
 �������  : XU32 fid      
             XU32 tid      
             XS32 *sockfd  ֮ǰXOS_HA_ThreadWatchInit ����ֵ
             XU32 TimeOut  ��λ ��,���Ϊ2
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��7��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_ThreadHelloProcess(XU32 fid, XU32 tid, XS32 *sockfd, XU32 TimeOut)
{
    ST_XOS_HA_MSG stMsg = {0};
    ST_XOS_HA_MSG stRes = {0};
    XS32 fd = 0;
    XS32 ret = 0;
    fd_set readfds ;
    struct timeval Wati = {0};

    /* ��ʱ���ܴ���2s */
    if (TimeOut > HA_DEAD_RECV_TIMEOUT)
    {
        TimeOut = HA_DEAD_RECV_TIMEOUT;
    }
    
    if (NULL == sockfd)
    {
        XOS_Trace(MD(fid,PL_ERR),"sockfd invalid!");

        return XERROR;
    }

    fd = *sockfd;
    if (fd < 0)
    {
        XOS_Trace(MD(fid,PL_ERR),"sockfd invalid reconnect!");
        goto RECONNECT;;
    }
    
    FD_ZERO (&readfds);
    FD_SET(fd,&readfds);

    Wati.tv_sec = TimeOut;
    Wati.tv_usec = 0;

    ret = select (fd + 1, &readfds, 0, 0, &Wati);
    if (ret <= 0)
    {
        XOS_Trace(MD(fid,PL_WARN),"select failed :%d!" ,ret);
        return XERROR;
    }

    /* ��ȡ��Ϣ */
    ret = read(fd, &stMsg, sizeof(stMsg));
    if (ret < sizeof(stMsg) && ret > 0)
    {
        XOS_Trace(MD(fid,PL_ERR),"read msg len wrong :%d!" ,ret);
        
        return XERROR;
    }

    /* ��ȡ���� */
    if (0 >= ret)
    {
        XOS_Trace(MD(fid,PL_ERR),"read failed[%s] reconnect!",strerror(errno));
        goto RECONNECT;;
    }

    /* ������Ϣ���� */
    if (HA_WATCH_THREAD != stMsg.MsgMethod)
    {
        XOS_Trace(MD(fid,PL_ERR),"Wrong msg method :%d",stMsg.MsgMethod);
        return XERROR;
    }

    /* �ظ���Ϣ */
    stRes.Fid = fid;
    stRes.MsgIndent = stMsg.MsgIndent;
    stRes.MsgMethod = stMsg.MsgMethod;
    stRes.MsgType   = HA_WATCH_MSG_RES;
    stRes.ThreadId  = stMsg.ThreadId;
    stRes.TimeStamp = time(NULL);

    ret = write(fd, &stRes, sizeof(stRes));
    if (ret <= 0)
    {
        XOS_Trace(MD(fid,PL_ERR),"send hello res failed:%d",ret);
        goto RECONNECT;
    }

    XOS_Trace(MD(fid,PL_DBG),"send hello res len:%d",ret);

    return XSUCC;
    
RECONNECT:
    close (fd);
    *sockfd = HA_ClientReConnect(fid, tid);
    
    return XERROR;
}

/*****************************************************************************
 �� �� ��  : HA_XOSHelloProcess
 ��������  : �������Ϣ�߳� �ظ���������
             �˴�������Ϣ���е��ڴ�����ͷ� ��Ҫҵ��ģ������ͷ�
 �������  : XU32 fid    ҵ��ģ��fid
             void *msg   �յ�����Ϣʵ��
             int msglen  ��Ϣʵ�峤��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_XOSHelloProcess(XU32 fid, void *msg, int msglen)
{
    ST_XOS_HA_MSG *pMsg = msg;
    ST_XOS_HA_MSG stRes = {0};
    XS32 ret = XERROR;
        
    if (NULL == pMsg || msglen < sizeof(ST_XOS_HA_MSG))
    {
        XOS_Trace(MD(fid,PL_ERR),"msg is NULL or len[%d] is less than:%d!",
                                                    msglen,sizeof(ST_XOS_HA_MSG));

        return XERROR;
    }

    if (fid != pMsg->Fid)
    {
        XOS_Trace(MD(fid,PL_ERR),"fid[%d] is not the same:%d!",pMsg->Fid,fid);
        return XERROR;
    }

    if (HA_WATCH_MSG_REQ != pMsg->MsgType)
    {
        XOS_Trace(MD(fid,PL_ERR),"MsgType is wrong:%d",pMsg->MsgType);
        return XERROR;
    }

    if (HA_WATCH_BASE_XOS != pMsg->MsgMethod)
    {
        XOS_Trace(MD(fid,PL_ERR),"MsgMethod is wrong:%d",pMsg->MsgMethod);
        return XERROR;
    }

    stRes.Fid = fid;
    stRes.MsgIndent = pMsg->MsgIndent;
    stRes.MsgMethod = pMsg->MsgMethod;
    stRes.MsgType   = HA_WATCH_MSG_RES;
    stRes.ThreadId  = pMsg->ThreadId;
    stRes.TimeStamp = time(NULL);

    ret = HA_SendXosMsg(FID_HA, fid, HA_WATCH_MSG_RES, &stRes);
    XOS_Trace(MD(fid,PL_DBG),"send based xos hello res:%d",ret);

    return ret;
}

/*****************************************************************************
 �� �� ��  : HA_DeadWatchDestroy
 ��������  : ���������Դ����
 �������  : void  
 �������  : ��
 �� �� ֵ  : inline
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_DeadWatchDestroy(void)
{
    if (XNULL != g_DealLockWatchList)
    {
        XOS_listDestruct(g_DealLockWatchList);
        g_DealLockWatchList = XNULL;
    }
}

/*****************************************************************************
 �� �� ��  : HA_ShowDeadWatchInfo
 ��������  : ��ӡ������ص���Ϣ
 �������  : CLI_ENV* pCliEnv  
             XS32 siArgc       
             XCHAR** ppArgv    
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��6��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XVOID HA_ShowDeadWatchInfo(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XS32 indx = 0;
    XS32 num = 0;
    ST_HA_DEAD_LOCK_INFO *pWatchInfo = NULL;

    XOS_CliExtPrintf(pCliEnv,"---------------------------------------------\r\n");
    indx = XOS_listHead(g_DealLockWatchList);

    while (XERROR != indx)
    {
        /* ����ÿһ���ڵ� ��ӡ */
        pWatchInfo = XOS_listGetElem(g_DealLockWatchList, indx);
        if (NULL != pWatchInfo)
        {
            XOS_CliExtPrintf(pCliEnv,"Fid:%d,Tid:%d, Type:%d MsgId:%d Sockfd:%d\r\n",
                                pWatchInfo->Fid,pWatchInfo->Tid,pWatchInfo->Type,
                                pWatchInfo->MsgIndent,pWatchInfo->Sockfd);
            num++;
        }
        indx = XOS_listNext(g_DealLockWatchList, indx);
    }
    
    XOS_CliExtPrintf(pCliEnv,"All watch num:%4d\r\n",num);
    XOS_CliExtPrintf(pCliEnv,"---------------------------------------------\r\n");
}


 #endif /* XOS_LINUX */