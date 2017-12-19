/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosmodule.h
**
**  description: module managment defination 
**
**  author: wangzongyou
**
**  date:   2006.7.13
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   wangzongyou         2006.6.13              create  
**************************************************************/
#ifndef _XOS_MODULE_H_
#define _XOS_MODULE_H_
#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/*-------------------------------------------------------------
                  包含头文件
--------------------------------------------------------------*/ 
#include "xostype.h"
#include "cmtimer.h"
#include "xostrace.h"

/*-------------------------------------------------------------
                  宏定义
--------------------------------------------------------------*/
#define MAX_FID_NAME_LEN      (50)
#define MAX_TID_NAME_LEN      (50)
#define MAX_FID_NUMS          (100)                 /*最大fid控制块数量*/
#define MAX_TID_NUMS          (MAX_FID_NUMS)        /*最大tid控制块数量*/

#define XOS_MODULE_STACK_SIZE (1024)  /* 模块stack大小定义 */

#define MAX_MSGS_IN_QUE       (2048)
#define MIN_MSGS_IN_QUE       (1024)
#define MAX_MSGSNUM_IN_QUE    (MIN_MSGS_IN_QUE * 64)


/*-------------------------------------------------------------
                 为了保证1.81版的外部注册接口不变
                 临时 结构和枚举声明
--------------------------------------------------------------*/
/* 内存释放类型 */
typedef enum
{
    eXOSMode=0,  /*XOS回收释放，在调用完消息处理函数后，平台释放消息内存*/
    eUserMode,   /*用户模式，平台不对消息内存进行释放*/
    MAXMode
}e_MEMMODE;


/* FID的内部结构 */
typedef struct _XOSFIDLIST
{
    struct
    {
        XCHAR   FIDName[MAX_FID_NAME_LEN + 1];         /*FID的名称*/
        XU32    PID;                            /*FID对应的PID*/
        XU32    FID;                            /*FID序列号*/
    }head;
    
    struct
    {
        XS8 (*init) (XVOID*, XVOID*);           /*FID的初始化函数入口*/
        XS8(*notice)(XVOID*, XVOID*);          /*FID的通知函数入口*/
        XS8 (*close)(XVOID*, XVOID*);           /*FID的关闭函数入口*/
    }init;
    
    struct
    {
        XS8 (*message)(XVOID*, XVOID*);         /*FID的消息处理函数入口*/
        XS8 (*timer)  (t_BACKPARA*);         /*FID的定时器超时处理函数入口*/
    }handle;
    
    e_MEMMODE   memfreetype;
    XU32 *standby;                              /*备用参数指针*/
}t_XOSFIDLIST;


/* 注册的结构块 */
typedef struct _XOSLOGINLIST
{
    t_XOSFIDLIST  *stack;       /*FID的结构*/
    XCHAR         taskname[MAX_TID_NAME_LEN + 1];    /*TID 的名称*/
    XU32          TID;           /*FID对应的TID,可以多个FID对应一个TID*/
    XU16          prio;          /*TID 的优先级  为NULL则为系统分配*/
    XU32          stacksize;    /*TID的堆栈大小 为NULL则为默认值modified by lixn 20070606 for vxworks*/
    /*XU32          timenum;     */ /*此TID上需注册的定时器个数*/
    XU32          quenum;      /*此TID上需注册的消息队列大小*/
}t_XOSLOGINLIST;

/*-------------------------------------------------------------
                     结构和枚举声明
--------------------------------------------------------------*/

/************************************************************************
函数名:modInitFunc
功能:  FID的初始化函数
输入:  没有意义的输入参数

输出:
返回: 成功返回XSUCC , 失败返回XERROR 
说明: 为了保持现有的接口不变,写成这样的.
************************************************************************/
typedef XS8 (*modInitFunc) (XVOID* sb1 , XVOID* sb2 );


/************************************************************************
函数名:modMsgProcFunc
功能:  FID的消息处理函数
输入:
pMsg   消息头指针
sb 没有意义的输入参数
输出:
返回: 成功返回XSUCC , 失败返回XERROR 
说明: 为了保持现有的接口不变,写成这样的.
************************************************************************/
typedef XS8 (*modMsgProcFunc) (XVOID* pMsg, XVOID* sb);


/************************************************************************
函数名:modTimerProcFunc
功能:  FID的定时器超时处理函数
输入:  
输出:
返回: 成功返回XSUCC , 失败返回XERROR 
说明: 为了保持现有的接口不变,写成这样的.
************************************************************************/
typedef XS8 (*modTimerProcFunc)  (t_BACKPARA*);


/************************************************************************
函数名:broadcastFilter
功能:  广播过滤函数
输入:  
输出:
返回: 容许广播返回XTRUE, 不容许广播XFALSE
说明: 为了现有的定时器设计,提供广播的机制
************************************************************************/
typedef XBOOL (*brdcstFilterFunc)  (XU32 fid);


/************************************************************************
函数名:setFidTraceInfo
功能:  遍历fid控制块，设置功能块相关的trace信息
输入:  
输出:
返回: 前一个fid 设置信息后的param
说明: 
************************************************************************/
typedef XVOID* (*setFidTraceInfo)  ( t_FIDTRACEINFO* pFidTraceInfo, XVOID* param,XVOID* param2);


/*-------------------------------------------------------------------------
                          接口函数
-------------------------------------------------------------------------*/
/************************************************************************
函数名: MOD_init
功能:  此模块的初始化

输入:
输出:
返回:成功返回XSUCC , 失败返回XERROR  
说明: 
************************************************************************/
XS32 MOD_init(void);


/************************************************************************
函数名: MOD_startXosFids
功能:  启动平台的相关模块

输入:
输出:
返回:成功返回XSUCC , 失败返回XERROR  
说明:  此函数在做模块管理时应该修改.
主要修改点, 模块的启动顺序可控.
************************************************************************/
XS32 MOD_startXosFids(void);

/************************************************************************
函数名: MOD_startUserFids
功能:  启动业务的相关模块

输入:
输出:
返回:成功返回XSUCC , 失败返回XERROR  
说明: 
************************************************************************/
XS32 MOD_startUserFids(void);


/************************************************************************
函数名: MOD_startNotice
功能:  启动通知函数 
输入:
输出:
返回:成功返回XSUCC , 失败返回XERROR  
说明: 为了兼容1.81版，提供临时函数
************************************************************************/
XS32 MOD_StartNotice(XVOID);


/************************************************************************
函数名: MOD_getTimMntByFid
功能：  通过功能块号获取任务的控制块信息
输入：  fid   功能模块号
输出：  N/A
返回：  任务控制块指针,失败返回空
说明：  
************************************************************************/
XVOID* MOD_getTimMntByFid(e_TIMERPRE pre,XU32 fid);


/************************************************************************
函数名: MOD_getTimProcFunc
功能：  通过功能块号获取功能块的超时处理函数
输入：  fid   功能模块号
输出：  N/A
返回：  任务控制块指针,失败返回空
说明：  
************************************************************************/
modTimerProcFunc MOD_getTimProcFunc(XU32 fid);


/************************************************************************
函数名: XOS_isValidFid
功能：  判断fid是否有效
输入：  fid   功能模块号
输出：  N/A
返回：  XFALSE OR XTRUE
说明：  
************************************************************************/
XBOOL XOS_isValidFid(XU32 fid);


/************************************************************************
函数名: XOS_getFidName
功能：  获取功能块消息转接开关的值
输入：  fid   功能模块号
输出：  N/A
返回：  名字或空指针
说明：  
************************************************************************/
XPUBLIC XCHAR* XOS_getFidName(XU32 fid);


/************************************************************************
函数名: MOD_getFidAttSwitch
功能：  获取功能块消息转接开关的值
输入：  fid   功能模块号
输出：  N/A
返回：  XFALSE OR XTRUE
说明：  
************************************************************************/
XPUBLIC XBOOL MOD_getFidAttSwitch(XU32 fid);


/************************************************************************
函数名: MOD_setFidAttSwitch
功能：  set功能块消息转接开关的值
输入：
fid   功能模块号
valve 开关状态，打开XTRUE , 关闭XFALSE
输出：  N/A
返回：  XFALSE OR XTRUE
说明：  
************************************************************************/
XPUBLIC XS16 MOD_setFidAttSwitch(XU32 fid, XBOOL valve);


/************************************************************************
函数名: MOD_getFidTraceInfo
功能：  fid 相关的trce信息

输入：  fid   功能模块号
输出：  N/A
返回：  成功返回fid 相关的trace信息，失败返回XNULLP
说明：  
************************************************************************/
XPUBLIC t_FIDTRACEINFO* MOD_getFidTraceInfo(XU32 fid);


/************************************************************************
函数名: MOD_setAllTraceInfo
功能：  设置所有的fid 相关的trace信息

输入：
setFunc   设置函数
param    设置函数的输入参数
输出：  N/A
返回：  成功返回XSUCC,  失败返回XERROR
说明：  
************************************************************************/
XPUBLIC XS32 MOD_setAllTraceInfo(setFidTraceInfo setFunc, XVOID* param,XVOID* param2);


/************************************************************************
函数名: XOS_MsgMemAlloc
功能：  给予需传输消息分配一个内存块        
输入：  fid           - 功能块id
nbytes        - 消息的长度
输出：  N/A
返回：  t_XOSCOMMHEAD * － 分配的消息内存指针
说明：  给现在的消息队列临时用
************************************************************************/
t_XOSCOMMHEAD *XOS_MsgMemMalloc(XU32 fid, XU32 nbytes);


/************************************************************************
函数名: XOS_MsgMemFree
功能：  释放一个消息内存块   
输入：
fid             - 功能块id
t_XOSCOMMHEAD * - 消息内存块指针
输出：  N/A
返回：  t_XOSCOMMHEAD * － 分配的消息内存指针
说明： 
************************************************************************/
XVOID XOS_MsgMemFree(XU32 fid, t_XOSCOMMHEAD *ptr);


/************************************************************************
函数名: XOS_MsgSend
功能：  模块间通信的消息发送函数
输入：  pMsg: 消息公共头信息
输出：  N/A
返回：  XSUCC OR XERROR
说明：  依据用户填写的信息头，将消息发送到目的地  
************************************************************************/
XPUBLIC XS32 XOS_MsgSend(t_XOSCOMMHEAD *pMsg);


/************************************************************************
函数名: XOS_MsgBroadcast
功能：  消息广播,暂时提供给低精度定时器用
输入：
pMsg: 消息公共头信息
brdcstFunc 广播函数,由用户编写,用来限制广播的fid
输出：  N/A
返回：  XSUCC OR XERROR
说明：  依据用户填写的信息头，将消息发送到目的地  
************************************************************************/
XPUBLIC XS32 XOS_MsgBroadcast(t_XOSCOMMHEAD *pMsg, brdcstFilterFunc brdcstFunc);


/************************************************************************
函数名: XOS_MMStartFid
功能：  
输入：
输出：  N/A
返回：  XSUCC OR XERROR
说明：  
************************************************************************/
XPUBLIC XS32 XOS_MMStartFid(t_XOSLOGINLIST *,XVOID *,XVOID *);

XS32 XOS_MsgDistribution(XU32 ipcIdx, XU32 remoteIdx, XU8 *p, XS32 len);

XS32 MOD_GetTskPid(XU32 *pid,XU32 fid);

#ifdef XOS_ST_TEST
/************************************************************************
函数名:    XOS_exeCliCmd

功能: 直接执行的函数
输入:
输出:
返回:
说明:  用户自己保证字符串尾部有个结束符
************************************************************************/
XVOID XOS_exeCliCmd(XCHAR* pStr);
#endif 


#ifndef XOS_NEED_OLDTIMER
/*fid 控制块*/
typedef struct
{
    XCHAR fidName[MAX_FID_NAME_LEN+1];  /*功能块的名字*/
    XU32  fidNum;   /*fid 号*/
    XU32  pidIndex;   /*功能块的所在pid 的索引*/
    XU32  tidIndex;   /*tid  的索引*/
    modInitFunc initFunc;   /*模块初始化函数*/
    modInitFunc noticeFunc;       /*模块通知函数*/
    modMsgProcFunc MsgFunc;   /*模块的消息处理函数*/
    modTimerProcFunc timerFunc;   /*定时器超时处理函数*/
    e_MEMMODE   msgFreeType;  /* 释放消息的类型*/
    XBOOL  attSwitch;   /* 消息转接开关*/
    XBOOL  noticeFlag;  /*Notice是否通知过*/
    t_FIDTRACEINFO   traceInfo;   /*trace 的相关信息*/
    e_PRINTLEVEL logLevel;  /*模块的日志级别*/
}t_FIDCB;
#else
/*fid 控制块*/
typedef struct
{
    XCHAR fidName[MAX_FID_NAME_LEN+1];  /*功能块的名字*/
    XU32  fidNum;   /*fid 号*/
    XU32  pidIndex;   /*功能块的所在pid 的索引*/
    XU32  tidIndex;   /*tid  的索引*/
    modInitFunc initFunc;   /*模块初始化函数*/
    modInitFunc noticeFunc;       /*模块通知函数*/
    modMsgProcFunc MsgFunc;   /*模块的消息处理函数*/
    modTimerProcFunc timerFunc;   /*定时器超时处理函数*/
    e_MEMMODE   msgFreeType;  /* 释放消息的类型*/
    XBOOL  attSwitch;   /* 消息转接开关*/
    XBOOL  noticeFlag;  /*Notice是否通知过*/
    t_FIDTRACEINFO   traceInfo;   /*trace 的相关信息*/
    t_TIMERMNGT timerMngerLow;  /*低精度定时器管理结构*/
    t_TIMERMNGT timerMngerHig;      /*高精度定时器管理结构*/
    e_PRINTLEVEL logLevel;  /*模块的日志级别*/
}t_FIDCB;
#endif

t_FIDCB* MOD_getFidCb(XU32 fid);
#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _CLISHELL_H_ */
