/******************************************************************************

                  版权所有 (C), 2001-2014, 深圳信威通信技术有限公司

 ******************************************************************************
  文 件 名   : ha_deadwatch.c
  版 本 号   : 初稿
  作    者   : liujun
  生成日期   : 2014年12月26日
  最近修改   :
  功能描述   : 死锁 死循环监控实现
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月26日
    作    者   : liujun
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
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
 * 宏定义                                       *
 *----------------------------------------------*/
#define HA_DEAD_RECV_TIMEOUT                 2
#define HA_DEAD_MSG_COUNT                    1
#define HA_DEAD_WATCH_MAX                    255
#define HA_READ_ERROR                        -1
#define HA_KEEP_ALIVE_SEND_TIME              2000
#define HA_KEEP_ALIVE_RECV_TIME              1500
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern XS32 HA_SendMsgToModule(XU32 DstFid,XU16 MsgId, ST_XOS_HA_MSG *pstStatusMsg);
extern XS32 HA_SendXosMsg(XU32 DstFid, XU32 SrcFid, XU16 MsgId, ST_XOS_HA_MSG *pstStatusMsg);
extern XS32 HA_SocketInit(const XCHAR *path);

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

enum ENUM_EPOLL_EVT_TYPE
{
    HA_EPOLL_EVT_ACCEPT,                /* 接收连接的事件 */
    HA_EPOLL_EVT_REG,                   /* 注册请求 */
    HA_EPOLL_EVT_HEART,                 /* 心跳回复 */
};

typedef struct ha_epoll_info
{
    XS32 fd;                                    /* epoll 监听的描述符    */
    XS32 EventType;                             /* 事件类型              */
    ST_HA_DEAD_LOCK_INFO *pWatchInfo;           /* fd 对应的线程监控信息 */
}ST_HA_EPOLL_INFO;
/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

static XOS_HLIST g_DealLockWatchList = XNULL;
static PTIMER g_TimerDeadWatch;
static XU32 g_DeadWatchMsgCount = 0;
/* 默认打开死锁监控的开关 */
static XBOOL g_DeadWatchDefaultOpen = XFALSE;
/* static PTIMER g_TimerDeadRecv;*/
/*----------------------------------------------*
 * 内部函数原型说明                             *
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
 函 数 名  : HA_DeadWatchCmp
 功能描述  : 死锁链表比较函数
 输入参数  : nodeType element1  
             XVOID *param       
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月4日
    作    者   : liujun
    修改内容   : 新生成函数

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
 函 数 名  : HA_DeadWatchFindInfo
 功能描述  : 获取节点
 输入参数  : XU32 Fid                       
             XU32 Tid                       
             ST_HA_DEAD_LOCK_INFO **result  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月8日
    作    者   : liujun
    修改内容   : 新生成函数

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
 函 数 名  : HA_DeadWatchUpdateMsgId
 功能描述  : 接收到心跳消息后 更新线程的msgid
 输入参数  : XS32 Fid    
             XS32 Tid    
             XS32 MsgId  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

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
 函 数 名  : HA_DeadWatchUpdateSock
 功能描述  : 更新线程监控的通信套接字
 输入参数  : XS32 Fid     
             XS32 Tid     
             XS32 SockFd  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

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
 函 数 名  : HA_DeadWatchDel
 功能描述  : 删除一个监控
 输入参数  : ST_HA_DEAD_LOCK_INFO *info  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

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
 函 数 名  : HA_DeadWatchAdd
 功能描述  : 添加监控
 输入参数  : ST_HA_DEAD_LOCK_INFO *info  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

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

    /* 首先查找是否存在相同的节点 */
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

    /* 初始化消息标识符 */
    info->MsgIndent = HA_GetMsgCount();
    /* 添加到尾部 */
    nodeIndex = XOS_listAddTail(g_DealLockWatchList, info);
    if (XERROR == nodeIndex)
    {
        result = XERROR;
        XOS_Trace(MD(FID_HA, PL_ERR), "Add Dead Watch to List failed!");
    }

    return result;
}

/*****************************************************************************
 函 数 名  : HA_AddToDeadWatch
 功能描述  : 添加线程死锁监控，XOS内部实现
 输入参数  : XU32 fid  
             XU32 tid  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月15日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XVOID HA_AddToDeadWatch(XU32 fid, XU32 tid)
{
    XS32 ret = 0;
    ST_HA_DEAD_LOCK_INFO stWatchInfo = {0};

    /* 根据配置 是否启用默认监控所有线程 */
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
 函 数 名  : HA_DeadWatchLstInit
 功能描述  : 死锁监控 相关资源初始化
 输入参数  : XOS_HLIST *pListHead  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月4日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 HA_DeadWatchLstInit(XOS_HLIST *pListHead)
{
    if (NULL == pListHead)
    {
        return XERROR;
    }
    /* 初始化死锁监控链表 */
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
 函 数 名  : HA_SendMsgToWatchMod
 功能描述  : 发送消息到被监控线程
 输入参数  : XOS_HLIST ListHead  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月4日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
void HA_SendMsgToWatchMod(XOS_HLIST ListHead)
{
    ST_XOS_HA_MSG stStatusMsg = {0};
    ST_HA_DEAD_LOCK_INFO *pDeadWatch = NULL;
    XS32 nodeIndex = 0 ,len = 0;
    /* 消息编号  */
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
        
        /* 若有2个周期的消息丢失 则认为死锁/死循环产生，回调 */
        if (MsgCount - pDeadWatch->MsgIndent > HA_DEAD_MSG_COUNT)
        {
            pDeadWatch->CallBack(pDeadWatch->param);

            /* 主备模式由状态管理模块切换到备状态 */
            if (HA_STATUS_CLOSE != ha_get_current_status())
            {
                ha_change_status_cmd(HA_STATUS_STANDBY);
            }
            else
            {
                /* 单机模式 直接切换*/
                HA_ChangeStatusCallBack(HA_STATUS_STANDBY);
            }
            
            XOS_Trace(MD(FID_HA, PL_WARN), "Mod[%d] Tid[%d] Maybe in dead loop!",
                                                pDeadWatch->Fid,pDeadWatch->Tid);
            /* 关闭该线程监控的套接字 释放epoll资源*/
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
            /* 基于XOS平台的消息队列 */
            stStatusMsg.MsgMethod = HA_WATCH_BASE_XOS;
            HA_SendMsgToModule(pDeadWatch->Fid, HA_WATCH_MSG_REQ, &stStatusMsg);
        }
        else if (HA_WATCH_THREAD == pDeadWatch->Type
            && -1 != pDeadWatch->Sockfd)
        {
            /* UNIX域套接字 */
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
 函 数 名  : HA_RecvRegMsg
 功能描述  : 死锁监控心跳消息回复处理函数
 输入参数  : XS32 Sockfd  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月4日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
static int HA_RecvRegMsg(XS32 Sockfd,ST_XOS_HA_MSG *pRegMsg)
{
    XS8 buf[1024] = {0};
    XS32 RecvLen = 0;

    /* 从套接字接收消息 */
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

    /* 线程监控注册 */
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
 函 数 名  : HA_ParseHeartBeatMsgFromQue
 功能描述  : 解析收到的消息
 输入参数  : t_XOSCOMMHEAD *pMsg  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月26日
    作    者   : liujun
    修改内容   : 新生成函数

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
 函 数 名  : HA_RecvHeartBeatMsg
 功能描述  : 接收心跳消息
 输入参数  : XOS_HLIST ListHead  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
void HA_RecvHeartBeatMsg(XOS_HLIST ListHead)
{
    XS32 nodeIndex = 0;
    XS32 readlen = 0;
    ST_XOS_HA_MSG stRecvBuf = {0};
    
    ST_HA_DEAD_LOCK_INFO *pDeadWatch = NULL;
    nodeIndex = XOS_listHead(ListHead);

    /* 遍历所有套接字 读取心跳回复 */
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
                /* 非阻塞模式，需要下次再读 */
                continue;
            }
            else if (HA_READ_ERROR == readlen)
            {
                /* 如果报错 则需要关闭套接字 */
                close(pDeadWatch->Sockfd);
                pDeadWatch->Sockfd = 0;
                continue;
            }
            
            if (HA_WATCH_MSG_RES != stRecvBuf.MsgType
                || HA_WATCH_THREAD !=stRecvBuf.MsgMethod
                || stRecvBuf.Fid != pDeadWatch->Fid
                || stRecvBuf.ThreadId != pDeadWatch->Tid)
            {
                /* 消息校验 失败 */
                close(pDeadWatch->Sockfd);
                pDeadWatch->Sockfd = 0;
                continue;
            }
            
            pDeadWatch->MsgIndent = stRecvBuf.MsgIndent;
        }
    }
}

/*****************************************************************************
 函 数 名  : HA_ThreadAccept
 功能描述  : 接受连接线程
 输入参数  : void *arg  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

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
        /* 线程退出 */
        if (Sockfd <= 0)
        {
            XOS_Trace(MD(FID_HA, PL_ERR),"Socket is invalid");
            break;
        }
        
        len = sizeof(addr);
        memset(&addr, 0, sizeof(addr));

        /* 等待新的连接 */
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
            /* 超时 则关闭 */
            close(NewConn);
        }
        else
        {
            /* 接收并 解析消息 */
            if (FD_ISSET(NewConn,&readfd))
            {
                memset(&RegMsg, 0, sizeof(RegMsg));
                ret = HA_RecvRegMsg(NewConn, &RegMsg);
                if (XSUCC != ret)
                {
                    /* 非正确消息 关闭 */
                    close(NewConn);
                    
                }

                /* 将套接字更新到链表节点 */
                ret = HA_DeadWatchUpdateSock(RegMsg.Fid,RegMsg.ThreadId, NewConn);
                if (XSUCC != ret)
                {
                    XOS_Trace(MD(FID_HA, PL_ERR),"Update sockfd failed!");
                    close(NewConn);
                    continue;
                }

                /*  设置新的连接为非阻塞  */
                val = fcntl (NewConn, F_GETFL, 0);
                fcntl (NewConn, F_SETFL, (val | O_NONBLOCK));
            }
        }
    }

    return NULL;
}

/*****************************************************************************
 函 数 名  : HA_UnixServInit
 功能描述  : 服务端初始化
 输入参数  : const char *path  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

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
    /* 接收并解析消息 */
    result = HA_RecvRegMsg(pReadInfo->fd, &RegMsg);
    if (XERROR == result)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Recv msg error");
        goto FAILED;
    }

    /* 为空 则说明是第一次连接的注册消息 */
    if (NULL == pReadInfo->pWatchInfo)
    {
        /* 需要更新sockfd与链表节点的对应关系 */
        result = HA_DeadWatchFindInfo(RegMsg.Fid, RegMsg.ThreadId, &pInfo);
        if (XERROR == result)
        {
            XOS_Trace(MD(FID_HA, PL_ERR),"Find watch info failed");
            goto FAILED;
        }

        /* 更新节点信息 */
        pInfo->MsgIndent      = HA_GetMsgCount();
        pInfo->pEpollBuf      = pReadInfo;
        
        pReadInfo->pWatchInfo = pInfo;
        pInfo->Sockfd = pReadInfo->fd;

        /* 更改epoll中对应的事件的消息结构 */
        pReadInfo->EventType = HA_EPOLL_EVT_HEART;
        evnt.data.ptr = pReadInfo;
        evnt.events   = EPOLLIN ;
        epoll_ctl(EpollFd, EPOLL_CTL_MOD, pReadInfo->fd, &evnt);
    }
    else
    {
        /* 多次的心跳回复，仅需要更新msgid */
        pInfo = pReadInfo->pWatchInfo;
        pInfo->MsgIndent = RegMsg.MsgIndent;
    }

    return XSUCC;
FAILED:

    /* 失败的情况 需要从epoll中删除 并关闭连接 释放节点内存*/
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

    /* 等待新的连接 */
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
 函 数 名  : HA_CreateAccptFdWithEpoll
 功能描述  : 创建监控线程的套接字服务端 并添加到epoll监听
 输入参数  : XS32 epfd                     
             ST_HA_EPOLL_INFO *pAccptInfo  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月15日
    作    者   : liujun
    修改内容   : 新生成函数

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
 函 数 名  : HA_ThreadRecv
 功能描述  : epoll实现死锁监控线程
 输入参数  : void *arg  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月8日
    作    者   : liujun
    修改内容   : 新生成函数

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
                /* 新连接 */
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
 函 数 名  : HA_DeadWatchCreateThread
 功能描述  : 创建服务监听线程
 输入参数  : XS32 sockfd  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS8 HA_DeadWatchCreateThread(void)
{
    pthread_t thread;
    pthread_attr_t attr;
    XS32 ret = 0;
    
    /* 以下为创建线程 */
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
 函 数 名  : HA_DeadWatchInit
 功能描述  : 死锁监控初始化
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS8 HA_DeadWatchInit(void)
{
    t_BACKPARA backpara;
    XS32 ret = 0;
    t_PARA timerpara;

    /* 创建监控线程 */
    if (XERROR == HA_DeadWatchCreateThread())
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Dead Watch thread Create failed");
        return XERROR;
    }
    
    if (XERROR == HA_DeadWatchLstInit(&g_DealLockWatchList))
    {
        goto FAILED;
    }

    /* 启动死锁监控发送定时器 */
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

    /* 启动死锁心跳接收定时器 
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
 函 数 名  : HA_ClientReConnect
 功能描述  : 重新创建套接字并连接
 输入参数  : XU32 fid  
             XU32 tid  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月26日
    作    者   : liujun
    修改内容   : 新生成函数

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
 函 数 名  : HA_ThreadHelloProcess
 功能描述  : 被监控线程处理心跳报文
 输入参数  : XU32 fid      
             XU32 tid      
             XS32 *sockfd  之前XOS_HA_ThreadWatchInit 返回值
             XU32 TimeOut  单位 秒,最大为2
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月7日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 HA_ThreadHelloProcess(XU32 fid, XU32 tid, XS32 *sockfd, XU32 TimeOut)
{
    ST_XOS_HA_MSG stMsg = {0};
    ST_XOS_HA_MSG stRes = {0};
    XS32 fd = 0;
    XS32 ret = 0;
    fd_set readfds ;
    struct timeval Wati = {0};

    /* 超时不能大于2s */
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

    /* 读取消息 */
    ret = read(fd, &stMsg, sizeof(stMsg));
    if (ret < sizeof(stMsg) && ret > 0)
    {
        XOS_Trace(MD(fid,PL_ERR),"read msg len wrong :%d!" ,ret);
        
        return XERROR;
    }

    /* 读取报错 */
    if (0 >= ret)
    {
        XOS_Trace(MD(fid,PL_ERR),"read failed[%s] reconnect!",strerror(errno));
        goto RECONNECT;;
    }

    /* 心跳消息解析 */
    if (HA_WATCH_THREAD != stMsg.MsgMethod)
    {
        XOS_Trace(MD(fid,PL_ERR),"Wrong msg method :%d",stMsg.MsgMethod);
        return XERROR;
    }

    /* 回复消息 */
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
 函 数 名  : HA_XOSHelloProcess
 功能描述  : 被监控消息线程 回复心跳报文
             此处不对消息队列的内存进行释放 需要业务模块进行释放
 输入参数  : XU32 fid    业务模块fid
             void *msg   收到的消息实体
             int msglen  消息实体长度
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

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
 函 数 名  : HA_DeadWatchDestroy
 功能描述  : 死锁监控资源回收
 输入参数  : void  
 输出参数  : 无
 返 回 值  : inline
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月4日
    作    者   : liujun
    修改内容   : 新生成函数

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
 函 数 名  : HA_ShowDeadWatchInfo
 功能描述  : 打印死锁监控的信息
 输入参数  : CLI_ENV* pCliEnv  
             XS32 siArgc       
             XCHAR** ppArgv    
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月6日
    作    者   : liujun
    修改内容   : 新生成函数

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
        /* 遍历每一个节点 打印 */
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