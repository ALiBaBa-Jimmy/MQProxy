/***************************************************************
 **                                                             
 **  Xinwei Telecom Technology co., ltd. ShenZhen R&D center    
 **                                                             
 **  Core Network Department  platform team                     
 **                                                             
 **  filename: xostimer.h                                       
 **                                                             
 **  description:                                               
 **                                                             
 **  author: chenwanli                                          
 **                                                             
 **  date:   2006.3.8                                           
 **                                                             
 ***************************************************************
 **                          history                            
 **                                                             
 ***************************************************************
 **   author          date              modification            
 **   chenwanli      2006.3.8              create               
 **************************************************************/

#ifndef _XOSTIMER_H_
#define _XOSTIMER_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/                  

#include "xostype.h"
/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
#define XOS_INIT_THDLE(handle)   handle = XNULL

XOS_DECLARE_HANDLE(PTIMER); /*定时器句柄*/



/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/
/*定时器向外发的消息类型*/
typedef enum
{
    eMinMsgType = 0,
    eTimeLowClock,
    eTimeHigClock,
    eMaxType
}e_messagetype;


/* 定时器类型定义 */
typedef enum
{
    TIMER_TYPE_LOOP = 0, /* 循环定时器 */
    TIMER_TYPE_ONCE,    /* 单次定时器 */
    TIMER_TYPE_END
}e_TIMERTYPE ;

/* 定时器精度定义 */
 typedef enum
{
    TIMER_PRE_HIGH = 0, /* 一级精度定时器 */
    TIMER_PRE_LOW,     /* 二级精度定时器 */
    TIMER_PRE_END
}e_TIMERPRE;

typedef enum
{
    TIMER_STATE_NULL = 0, /* 定时器句柄未分配     */
    TIMER_STATE_FREE,     /* 定时器句柄可以被利用 */
    TIMER_STATE_RUN,      /* 定时器句柄正在运行   */
    
    TIMER_STATE_ERROR
}e_TIMESTATE;

typedef struct 
{
    XU32        fid;    /* 功能块ID   */
    XU32        len;    /* 定时器长度 */
    e_TIMERTYPE  mode;   /* 启动定时器方式: 循环或非循环 */
    e_TIMERPRE   pre;    /* 需要定时器精度: 一级或二级（高精度与低精度） */

}t_PARA;

typedef struct 
{
#ifdef XOS_ARCH_64
    XU64 para1;
    XU64 para2;
    XU64 para3;
    XU64 para4;
    XU64 count;  /* 记录定时器运行次数，不允许做其他用途 */
#else
    XU32 para1;
    XU32 para2;
    XU32 para3;
    XU32 para4;
    XU64 count;  /* 记录定时器运行次数，不允许做其他用途 */
#endif
}t_BACKPARA;

/*-------------------------------------------------------------------------
                API 声明
-------------------------------------------------------------------------*/


/************************************************************************
函数名: XOS_TimerNumReg
功能：  各功能块注册最大定时器个数(必须在初始化过程中调用)
输入：
        fid            - 功能块ID号
        tmaxtimernum   - 功能块的最大定时器数目
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR 
说明：
************************************************************************/
XS32 XOS_TimerReg(XU32 fid, 
                     XU32 msecnd ,
                     XU32 lowPrecNum, 
                     XU32 highPrecNum);

//#ifdef XOS_TIMER_FOURFUNC

/*******************************************************************
函数名: XOS_TimerGetState
功能：  定时器是否在运行判断函数
输入：  ptimer      - 定时器句柄 
输出：  none
返回：  如上枚举类型
说明： 
*******************************************************************/
e_TIMESTATE XOS_TimerGetState(XU32 fid,PTIMER tHandle);    

/*******************************************************************
函数名: XOS_TimerCreate
功能：  定时器创建函数
输入：  fid      - 功能块ID号
        tHandle  - 定时器句柄
        type     - 定时器的启动方式和级别选择
        para     - 定时器超时回传参数
输出：  tHandle
返回：  XSUCC, 函数操作失败返回XERROR 
说明：
*******************************************************************/
XS32  XOS_TimerCreate(XU32 fid, 
                        PTIMER* tHandle,  
                        e_TIMERTYPE  timertype, 
                        e_TIMERPRE  timerpre, 
                        t_BACKPARA *backpara);


/*******************************************************************
函数名: XOS_TimerBegin
功能：  定时器启动函数
输入：  tHandle  - 定时器句柄
        len      - 定时器的延时长度（单位ms）
输出：  tHandle
返回：  XSUCC, 函数操作失败返回XERROR 
说明：
*******************************************************************/
XS32 XOS_TimerBegin(XU32 fid,PTIMER tHandle, XU32 len);


/*******************************************************************
函数名: XOS_TimerEnd
功能：  停止定时器
输入：  ptimer      - 定时器句柄 
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR 
说明：
*******************************************************************/
XS32 XOS_TimerEnd(XU32 fid,PTIMER tHandle);

/*******************************************************************
函数名: XOS_TimerDelete
功能：  删除定时器句柄
输入：  ptimer      - 定时器句柄 
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR 
说明：
*******************************************************************/
XS32 XOS_TimerDelete(XU32 fid,PTIMER tHandle);
//#endif
/************************************************************************
函数名: XOS_TimerIsRuning
功能：  定时器是否在运行判断函数
输入：  ptimer      - 定时器句柄 
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR 
说明：
************************************************************************/
e_TIMESTATE   XOS_TimerRunning(XU32 fid, PTIMER tHandle);


/************************************************************************
函数名: XOS_TimerStart
功能：  定时器启动函数
输入：  tHandle     - 定时器句柄
        timerpara   - 定时器参数
        backpara    - 定时器超时回传参数
        
输出：  tHandle
返回：  XSUCC, 函数操作失败返回XERROR 
说明：
************************************************************************/
XS32 XOS_TimerStart(PTIMER *tHandle, 
                      t_PARA *timerpara, 
                      t_BACKPARA *backpara);

/************************************************************************
函数名: XOS_TimerStop
功能：  停止定时器
输入：  ptimer      - 定时器句柄 
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR 
说明：
************************************************************************/
XS32 XOS_TimerStop(XU32 fid, PTIMER tHandle);

/************************************************************************
函数名: XOS_TimerGetParam
功能：  获取定时器的param结构体
输入：  tHandle     - 定时器句柄
输出：  ptBackPara  - 需要获取结构体的输出指针
返回：  XSUCC, 函数操作失败返回XERROR
说明:
************************************************************************/
XS32 XOS_TimerGetParam(PTIMER tHandle,t_BACKPARA *ptBackPara);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*demon.h*/

