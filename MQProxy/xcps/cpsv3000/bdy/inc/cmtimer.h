/***************************************************************
 **
 **  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
 **
 **  Core Network Department  platform team
 **
 **  filename: cmtimer.h
 **
 **  description: 
 **  四接口: 指使用一个定时器需要使用四个API(create,begin,end,delete)
 **  二接口: 指使用一个定时器需要使用两个API(start,stop)
 **  平台提供两套接口给业务进行调用。
 **
 **  author: wulei
 **
 **  date:   2006.5.26
 **
 ***************************************************************
 **                          history
 **
 ***************************************************************
 **   author          date              modification
 **   wulei         2006.5.26              create
 **************************************************************/

#ifndef _CMTIMER_H_
#define _CMTIMER_H_

#ifndef XOS_NEED_OLDTIMER

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "xostimer.h"
#include "xosencap.h"
#include "xosport.h"
#include "clishell.h"

#pragma pack(1)

#define  TIMER_CHECK_NUM                   (0xbb)   /*四接口模型的校验码*/
#define  TIMER_CHECK_DNUM                  (0xdd)   /*二接口模型的校验码*/
#define  MAX_FID_NUMBER                    (2000)   /*业务模块fid值的范围，用于统计各个模块定时器数量*/

//新定时器的精度都为10豪秒，不分低高精度，此宏定义只为与老代码统一
/*低精度定时器时钟精度定义(100 millisecond)*/
#define LOC_LOWTIMER_CLCUNIT   100
/*高精度定时器时钟精度定义(10 millisecond)*/
#define LOC_HIGHTIMER_CLCUNIT 10
/*时钟刻度间隔，时钟每规定时刻跳动一次*/
#define TM_SCALE 10

/* 定时器存储数据 */
typedef struct tag_TIMERNODE
{
    PTIMER       timerHandler;   /*当前定时器句柄,需要与状态一起进行使用*/
    t_PARA       para;           /*定时器的类型,fid,高低精度,定时器长度等*/
    t_BACKPARA   backpara;       /* 定时器超时回传参数 */
    e_TIMESTATE  tmnodest;       /*定时器状态*/
    XBOOL flag;                  /*两接口的定时器和四接口的定时器标记,1为四接口定时器.*/
}t_TIMERNODE;

/************************************************************************
函数名: tm_excuteHighProc
功能：  运行队列处理函数
输入：  
输出：  
返回：  
说明：
************************************************************************/
void tm_excuteHighProc();
void tm_excuteLowProc();

/************************************************************************
 函数名:TIM_MsgSnd
 功能: 定时器到时，消息发送程序
 输入: t_TIMERNODE 发送的参数
 输出:
 返回: 
 说明:
************************************************************************/
XVOID  TIM_MsgSnd(const t_TIMERNODE *data);

/************************************************************************
 函数名: 定时器模块初始化函数.
 功能:
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XS8 TIM_InitTime(XVOID *t, XVOID *v);

/************************************************************************
 函数名: 定时器模块初始化函数.
 功能:
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XS8 TIM_NoticeTime(XVOID *t, XVOID *v);

/************************************************************************
 函数名: 其他模块的时钟处理函数.
 功能:
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XS32 TIM_ClckProc();


#pragma pack()

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/

#include "xostimer.h"
#include "xosencap.h"
#include "xosport.h"
#include "clishell.h"

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
/*低精度定时器时钟精度定义(100 millisecond)*/
#define LOC_LOWTIMER_CLCUNIT   100
/*高精度定时器时钟精度定义(10 millisecond)*/
#define LOC_HIGHTIMER_CLCUNIT 20

/*定时器链大小*/
#define LOC_TIMER_LINKLEN   100

#define backparalen  sizeof(t_BACKPARA)

#define  TIMER_CHECK_NUM                   0xbb
#define  TIMER_CHECK_DNUM                   0xdd
#define  CM_RMV_TQ(del)                      \
{                                            \
     (del)->prev->next = (del)->next;        \
     (del)->next->prev = (del)->prev;        \
     (del)->next = (del)->prev = del;        \
}

#define CM_PLC_TQ(old, add)          \
{    (add)->next = (old)->next;      \
     (add)->prev = old;              \
     (old)->next = add;              \
     (add)->next->prev = add;        \
}

#define CM_INIT_TQ(ent)            (ent)->next = (ent)->prev = ent

#define HiTm_FIDNum   10
/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/

/* 双向链表 */
typedef struct _t_LISTENT
{
    struct _t_LISTENT  *prev;
    struct _t_LISTENT  *next;
}t_LISTENT;

/* 定时器双向链表节点结构 */
typedef struct tag_TIMERNODE
{
    t_LISTENT    stLe;     /* 双向链表头 */
    PTIMER *      pTimer;  /* 定时器句柄 */
    t_PARA       para;    /*定时器的类型,fid,高低精度,定时器长度等*/
    t_BACKPARA   backpara; /* 定时器超时回传参数 */
    XU32         walktimes;/* 节点被遍历的次数 */
    e_TIMESTATE  tmnodest; /*定时器状态*/
    XBOOL flag;   /*两接口的定时器和四接口的定时器标记,1为四接口定时器.*/

}t_TIMERNODE;

/* 定时器管理结构 */
typedef struct tagTimeMngt
{
    t_LISTENT       stRunList[LOC_TIMER_LINKLEN];   /* 定时器运行链表 */
    t_TIMERNODE     *pstTimerPool;                  /* 定时器池头指针 */
    XU32            maxtimernum;                    /* 最大定时器个数 */
    t_LISTENT       idleheader;                     /* 定时器池空闲链表的头节点 */
    XU32            nowclock;                       /* 当前时钟所在刻度 */
    XU32            timeruint;                      /*二级定时器的精度*/
    XU32            curNumOfElements;               /*使用值*/
    XU32            maxUsage;                       /*定时器的峰值*/
    t_XOSMUTEXID    timerMutex;                     /*临界区*/
}t_TIMERMNGT;

typedef struct tag_ParTimerNode
{
    t_LISTENT    stLe;
    XU32 fid;
    XU32 TimeLen;
    XU32 NowTime;
}t_ParTimerNode;

typedef struct tag_ParManage   /*一级定时器的管理结构*/
{
    t_ParTimerNode   ParNoArray[FID_MAX];
    t_LISTENT    HeadList;    /*低精度定时器的链表头*/
    XU32 HiTmFid[HiTm_FIDNum];
    XU8  HiTmindex;       /*  高精度定时器的索引*/
    t_XOSSEMID hitsem;    /*高精度定时器用的信号量*/
    t_XOSSEMID lotsem;    /*低精度定时器用的信号量*/
#ifdef XOS_VXWORKS
    WDOG_ID  wdId;
    XU32   TmrMultiplier;
    WDOG_ID  wdIdlot;           /*低精度定时器的id号*/
    XU32   TmrMultiplierlot;    /*低精度定时器的倍数*/
#endif
    volatile  XBOOL TIME_INTIALIZED;    /*定时器模块的任务运行标志*/
}t_ParManage;

/*-------------------------------------------------------------------------
                全局变量
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                API 声明
-------------------------------------------------------------------------*/

/************************************************************************
 函数名:Low_TmerTask
 功能:   低精度任务定时器入口处理函数,线程入口函数
 输入:
 输出:
 返回:
 说明: 低精度定时器用于发送消息的任务函数.
************************************************************************/
XPUBLIC XVOID Low_TimerTask( XVOID* ptr);

/************************************************************************
函数名  : High_TimerTask
功能    : 高精度任务定时器入口处理函数,线程入口函数
输入    :

输出    : none
返回    :
说明    :高精度定时器用于发送消息的任务函数.
************************************************************************/
XPUBLIC XVOID High_TimerTask(XVOID *ptr);

/************************************************************************
函数名  : TIM_ClckProc
功能    : 各任务收到时钟任务消息的统一处理函数
输入    : management - 任务管理定时器链的结构指针
输出    : none
返回    : XSUCC, 函数操作失败返回XERROR
说明    :
************************************************************************/
XPUBLIC XS32 TIM_ClckProc(t_TIMERMNGT *management);

/************************************************************************
 函数名: 定时器模块初始化函数.
 功能:
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XPUBLIC XS8 TIM_InitTime(XVOID *t, XVOID *v);

/************************************************************************
 函数名: 定时器模块初始化函数.
 功能:
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XPUBLIC XS8 TIM_NoticeTime(XVOID *t, XVOID *v);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif //XOS_NEED_OLDTIMER
#endif /*_CMROOT_H_*/


