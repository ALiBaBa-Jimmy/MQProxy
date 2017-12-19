/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: cmqueue.h
**
**  description:  
**
**  author: wulei
**
**  date:   2006.4.7
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   wulei         2006.4.7              create  
**************************************************************/
#ifndef _XOSQUEUE_H_
#define _XOSQUEUE_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/                  
#include "xostype.h"
#include "xoslist.h"
#include "xosencap.h"
#include "xosmodule.h"
#include "xoshash.h"

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/


typedef struct XOSMSGQ
{
    XOS_HLIST     queueList; 
    t_XOSSEMID   sem;        /* 信号量 */
    t_XOSMUTEXID  queueLock;  /* 一个队列需要一个琐 */
    XU8 prio[eMAXPrio+1]; /*消息队列优先级*/
    XU8 prioCursor;   /*消息游标，用来提高效率*/
    XS32 tskIndex;
    XU8 isFull;         /*用于判断消息队列容量是否持续超过90%，如果是则不会再次调用XOS_ResetByQueFull函数*/
    XBOOL init;           /*用于判断消息队列是否初始化，0:未初始化；1:已初始化*/
} t_XOSMSGQ;

typedef struct
{
   XCHAR *pMsg;
}t_QUEELEM;


/*tid 控制块*/
typedef struct
{
    XU32 tidNum;  /*用户注册的任务号*/
    XCHAR tskName[MAX_TID_NAME_LEN+1];  /*用户注册的任务名字*/
    XU32 prio;   /*任务优先级*/
    XU32 stackSize;  /*任务栈空间大小*/
    t_XOSTASKID tskId;  /*任务句柄,调试时用*/
    XU32 tskpid;        /* 任务pid */
    t_XOSMSGQ recvMsgQue;   /*接收消息的队列*/
    XU32  maxMsgsInQue;   /*队列容许消息的最大长度*/

    /*临时用*/
    XU32  timerNum;   /*定时器的个数*/
}t_TIDCB;

typedef struct
{
    XBOOL isInit;  /*模块是否初始化*/
    XOS_HHASH fidHash;  /*管理fid的hash表*/
    XOS_HARRAY tidArray;   /*管理tid 控制块的数组*/
}t_MODMNT;

/************************************************************************
函数名: QUE_MsgQCreate
功能：  创建消息队列
输入：  maxMsgs：消息队列中最大的消息数目
                   maxMsgLength：每条消息的最大长度

输出：  msgq 消息队列标识 
返回： 函数操作成功返回XSUCC, 函数操作失败返回XERROR
说明： 
************************************************************************/
XPUBLIC XS32   QUE_MsgQCreate(t_XOSMSGQ *pMsgQ,XU32 maxMsgs) ;


/************************************************************************
函数名: QUE_MsgQDelete
功能：  删除一个消息队列
输入：  pMsgQ  消息队列标识 
输出：  N/A
返回： 函数操作成功返回XSUCC, 函数操作失败返回XERROR
说明：
************************************************************************/
XPUBLIC XS32  QUE_MsgQDelete(t_XOSMSGQ *pMsgQ);



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
XPUBLIC XS32  QUE_MsgQSend(
                                t_XOSMSGQ *pQue, 
                                t_XOSCOMMHEAD*pMsg, 
                                e_MSGPRIO prio);


/************************************************************************
函数名: QUE_MsgQRecv
功能：  从一个消息队列接收一条消息
输入：  pQue  消息队列标识
                  ppMsg  指向消息的缓冲区结构地址的指针
输出： 
返回：函数操作成功返回XSUCC, 函数操作失败返回XERROR
说明：
************************************************************************/
XPUBLIC XS32  QUE_MsgQRecv(
                                t_XOSMSGQ *pQue, 
                                t_XOSCOMMHEAD **ppMsg);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*cmqueue.h*/

