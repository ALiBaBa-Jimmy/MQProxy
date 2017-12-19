/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosmodule.c
**
**  description:  module management implent
**
**  author: wangzongyou
**
**  date:   2006.7.19
**
***************************************************************
**  history
**
***************************************************************
**   author  date  modification
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
  包含头文件
-------------------------------------------------------------------------*/

#include "xosmodule.h"
#include "xosencap.h"
#include "xoscfg.h"
#include "xosqueue.h"
#include "xosarray.h"
#include "xoshash.h"
#include "xosport.h"
#include "cmtimer.h"
#include "xosmem.h"
#include "xospub.h"
#include "trace_agent.h"
#ifdef XOS_UDT_MODULE
#include "udtlib.h"
#endif
#include "ha_interface.h"

/*-------------------------------------------------------------------------
   模块内部宏定义
-------------------------------------------------------------------------*/

/*临时用的宏*/

/*为了保证现有的平台注册接口不变, 实现从1.81 到2.0 的平滑
   过渡, 先将所有消息队列的大小写死, 做模块管理时将各任务
   的消息队列大小传入*/

/*-------------------------------------------------------------------------
   模块内部结构和枚举定义
-------------------------------------------------------------------------*/

/*tid 控制块,移动到xosqueue.h*/


typedef struct
{
    CLI_ENV* pCliEnv;
    XS32 index;
}t_FIDSHOW;

typedef struct
{
    setFidTraceInfo setTraceFunc;
    XVOID * param;                  /*trace level*/
    XVOID * param2;                 /*telnet session ID*/
}t_ALLFIDTRACE;

/*-------------------------------------------------------------------------
               模块内部全局变量
-------------------------------------------------------------------------*/
t_MODMNT  g_modMnt;
extern XU32 gMMFidTraceLel;
#ifdef MEM_FID_DEBUG
extern XBOOL bModifyFidFlg;
#endif

XU32 g_XOS_LocalPid = 10; /* change by wulei for mss */
XU32 g_Xos_LogicPid = 0;

/*临时引入的全局变量, 做读配置文件启动时应该删除*/
/*XEXTERN t_XOSLOGINLIST * g_xosLogin;*/

/*临时引入的注册函数, 做读配置文件启动时应该删除*/
XEXTERN XS32 XOS_cfgXosModule(t_XOSLOGINLIST *);
#ifndef XOS_MDLMGT
XEXTERN XS8 XOS_InfoReg(t_XOSLOGINLIST *);
#endif
XEXTERN  XBOOL MsgPflg[FID_MAX];

#ifdef XOS_MSGMEM_TRACE
XEXTERN XVOID XOS_RecordMsgBuf(XU32 fid, XVOID* pAddr);
XEXTERN XVOID XOS_UpdateMsgBuf(XVOID* pAddr);
XEXTERN XVOID XOS_ReleaseMsgBuf(XU32 fid, XVOID* pAddr);
#endif /* XOS_MSGMEM_TRACE */
XEXTERN  XCHAR* XOS_getPLName(e_PRINTLEVEL printLevel);
XEXTERN   XCHAR *XOS_getoutModeName(e_TRACEPRINTDES desmode);

XEXTERN XS32 write_to_syslog(const XS8* msg,...);

/*
XSTATIC XS32 trace_msgq_size(const XS8* msg,...);

static XS32 fid_stat[2048] = {FID_XOSMIN};
static XCHAR fid_name[2048][32] = {{0}};
static XS32 old_queue_nums = 0;
*/

/*-------------------------------------------------------------------------
                 模块内部函数
-------------------------------------------------------------------------*/
XS32 initFidTraceInfo(t_FIDTRACEINFO* traceInfo)
{
    XS32 i = 0;
    
    traceInfo->isNeedFileName=XTRUE;
    traceInfo->isNeedLine=XTRUE;
    traceInfo->isNeedTime=XTRUE;
    traceInfo->isPrintInTraceTsk=XFALSE;
    traceInfo->traceLevel=PL_ERR;
    /*初始化所有telnet终端的trace打印级别*/
    for(i = 0;i < kCLI_MAX_CLI_TASK;i++)
    {
        traceInfo->sessionLevel[i] = PL_ERR;
    }
    traceInfo->traceOutputMode=TOFILEANDCOS;
     /*创建临界区*/
    if ( XSUCC != XOS_MutexCreate( &traceInfo->xosFidTraceMutex) )
    {
        XOS_Trace(MD(FID_ROOT, PL_EXP), "XOS_MutexCreate xosFidMutex failed!");
        return XERROR;
    }

    return XSUCC;
}


/************************************************************************
函数名: MOD_getFidCb
功能：  获取功能块的控制块
输入：  fid   功能模块号
输出：  N/A
返回：  功能控制块指针,失败返回空
说明：
************************************************************************/
t_FIDCB* MOD_getFidCb(XU32 fid)
{
    /*有效性判断*/
    if(!XOS_isValidFid( fid))
    {
        return XNULLP;
    }
    return (t_FIDCB*)XOS_HashElemFind(g_modMnt.fidHash, (XVOID*)&fid);
}


/************************************************************************
函数名: MOD_getTidCbByFid
功能：  通过功能块号获取任务的控制块信息
输入：  fid   功能模块号
输出：  N/A
返回：  任务控制块指针,失败返回空
说明：
************************************************************************/
XPUBLIC t_TIDCB* MOD_getTidCbByFid(XU32 fid)
{
    t_FIDCB * pFidCb = (t_FIDCB*)XNULLP;
    
    pFidCb = MOD_getFidCb( fid);
    if(pFidCb == XNULLP)
    {
        return (t_TIDCB*)XNULLP;
    }

    return  (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);
}


/************************************************************************
函数名: MOD_TidNumCompare
功能： 
输入：  fid   功能模块号
输出：  N/A
返回：  任务控制块指针,失败返回空
说明：
************************************************************************/
XBOOL MOD_TidNumCompare(t_TIDCB *ptidCb , XU32 *ptidNum)
{
   if(ptidCb == XNULLP || ptidNum == XNULL)
   {
       XOS_Trace(MD(FID_ROOT, PL_ERR), " MOD_TidCbCompare: para invalid!");
       return XFALSE;
   }

   if(ptidCb->tidNum == *ptidNum)
   {
       return XTRUE;
   }
   return XFALSE;
}


/************************************************************************
函数名: MOD_getTidCbByTidNum
功能：  通过任务号获取任务的控制块信息
输入：  tidNum   任务号
输出：  N/A
返回：  任务控制块指针,失败返回空
说明：
************************************************************************/
XPUBLIC t_TIDCB* MOD_getTidCbByTidNum(XU32 tidNum)
{
    XS32 tidIndex = -1;

    tidIndex = XOS_ArrayElemCompare(g_modMnt.tidArray, (XOS_ArrayCompare)MOD_TidNumCompare, &tidNum);
    return  (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
}


/************************************************************************
函数名: MOD_isSameTid
功能: 任务控制块的比较函数

  输入:element1  任务控制块首地址
  param   - 比较参数
  输出:
  返回: 相等返回XTRUE, 不等返回XFALSE
  说明:
************************************************************************/
XBOOL MOD_isSameTid(XOS_ArrayElement element1, XVOID *param)
{
    t_TIDCB *pTidCb = (t_TIDCB*)XNULLP;

    pTidCb = (t_TIDCB*)element1;
    if(pTidCb->tidNum == (XU32)(XPOINT)param)
    {
        return XTRUE;
    }
    else
    {
        return XFALSE;
    }
}


/************************************************************************
* 输入 :
* param      - 元素
* paramSize  - 元素的大小(  单位字节)
* hashSize   - hash表的大小
* 返回 : hash 结果
************************************************************************/
XSTATIC  XU32 MOD_hashFunc(  XVOID*  param,  XS32  paramSize,  XS32  hashSize)
{
    XS32 hash;
    hash = *((XU32*)param);

    return (hash % hashSize);
}


/************************************************************************
* MOD_showFid
* 功能: 定义对每个hash元素通用的函数
* 输入:
* hHash   - 对象句柄
* elem    - 元素
* param   - 参数
* 输出:
* 返回: 指向一个参数，给下次调用XOS_HASHFunc使用
************************************************************************/
XVOID* MOD_showFid(XOS_HHASH hHash, XVOID* elem, XVOID *param)
{
    t_FIDCB *pFidCb;
    t_FIDSHOW *pFidShow;
    t_TIDCB *pTidCb;

    XCHAR* freeMsgType[] =
    {
        "xosMode",
            "userMode",
            "Max"
    };
    XCHAR *attSwitch[] =
    {
        "off",
        "on"
    };

    pFidCb = (t_FIDCB*)elem;
    pFidShow = (t_FIDSHOW*)param;
    pTidCb = (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);

    XOS_CliExtPrintf( pFidShow->pCliEnv,
        "%-6d%-6d%-18s%-18s%-10s%-10s%-10s\r\n",
        pFidShow->index,
        pFidCb->fidNum,
        pFidCb->fidName,
        (
        pTidCb !=XNULLP)?pTidCb->tskName:"noName",
        attSwitch[pFidCb->attSwitch],
        XOS_getPLName((e_PRINTLEVEL)(pFidCb->traceInfo.traceLevel)),
        freeMsgType[pFidCb->msgFreeType]
        );

    pFidShow->index++;
    return param;
}


/************************************************************************
*  MOD_showFidTraceInfo
* 功能: 定义对每个hash元素通用的函数，遍历并输出fid对应的trace标志
* 输入:
* hHash   - 对象句柄
* elem  - 元素
* param   - 参数
* 输出:
* 返回: 指向一个参数，给下次调用XOS_HASHFunc使用
************************************************************************/
XVOID* MOD_showFidTraceInfo(XOS_HHASH hHash, XVOID* elem, XVOID *param)
{
#if 1 /* wulei del for B07 2006.11.20 */
    t_FIDCB *pFidCb;
    t_FIDSHOW *pFidShow;
    /*  t_TIDCB *pTidCb;*/

    XS32 session = 0;
    e_PRINTLEVEL level = PL_MIN;
    
    XCHAR *attSwitch[] =
    {
        "off",
        "on"
    };

    pFidCb = (t_FIDCB*)elem;
    pFidShow = (t_FIDSHOW*)param;
    if ( pFidCb == XNULL || pFidShow == XNULL )
    {
        return XNULL;
    }
    /*
    pTidCb = (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);
    */
    session = MMISC_GetFsmId(pFidShow->pCliEnv);
    level = (e_PRINTLEVEL)pFidCb->traceInfo.sessionLevel[session];
    
    XOS_CliExtPrintf( pFidShow->pCliEnv,
        "%-6d%-18s%-8s%-8s%-8s%-8s%-8s%-18s\r\n",
        pFidCb->fidNum,
        pFidCb->fidName,
        XOS_getPLName(level),
        attSwitch[pFidCb->traceInfo.isNeedFileName],
        attSwitch[pFidCb->traceInfo.isNeedLine],
        attSwitch[pFidCb->traceInfo.isNeedTime],
        attSwitch[pFidCb->traceInfo.isPrintInTraceTsk],
        XOS_getoutModeName((pFidCb->traceInfo.traceOutputMode))
        );

    pFidShow->index++;
    return param;
#endif

    return XNULLP;
}


/************************************************************************
* MOD_setFidTrace
* 功能:设置功能模块的trace 信息
* 输入:
*  hHash   - 对象句柄
*  elem  - 元素
*  param   - 参数
* 输出:
* 返回: 指向一个参数，给下次调用XOS_HASHFunc使用
************************************************************************/
XVOID* MOD_setFidTrace (XOS_HHASH hHash, XVOID* elem, XVOID *param)
{
    t_FIDCB *pFidCb = XNULLP;
    t_ALLFIDTRACE *pFidTraceInfo = XNULLP;

    pFidCb = (t_FIDCB*)elem;
    pFidTraceInfo = (t_ALLFIDTRACE*)param;
    if(pFidCb == XNULLP || pFidTraceInfo == XNULLP)
    {
        return param;
    }
    pFidTraceInfo->setTraceFunc(&(pFidCb->traceInfo), pFidTraceInfo->param,pFidTraceInfo->param2);
    return param;
}


/************************************************************************
函数名: MOD_allTaskEntry
功能:  所有模块的任务入口函数
输入:pTid  任务号
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 在消息驱动的松耦合设计中,平台对所有的任务
      入口做一个封装, 截获定时器时钟源驱动消息,
      并且对其他消息做一个分发.
************************************************************************/
XSTATIC XVOID MOD_allTaskEntry( XVOID * tidCb)
{
    t_TIDCB* pTidCb   = (t_TIDCB*)XNULLP;
    t_XOSCOMMHEAD *pRQMsg = (t_XOSCOMMHEAD*)XNULLP;
    XS32 ret  = 0;
    /*
    XS32 queue_nums = 0;    
    XS32 listIndex = 0;
    XCHAR fid_trace_msg[255] = {0};*/
    t_FIDCB *pFidCb   = (t_FIDCB*)XNULLP;
    
    pTidCb = (t_TIDCB*)tidCb;
#ifdef XOS_LINUX
    pTidCb->tskpid = (XU32)gettid();  /* 线程 pid */
#endif
    
    while( XTRUE )
    {
        /*首先从消息队列中收消息*/
        pRQMsg = (t_XOSCOMMHEAD*)XNULLP;
        ret = QUE_MsgQRecv( &(pTidCb->recvMsgQue), &pRQMsg );
        if(  ret != XSUCC  || pRQMsg == XNULLP
            || pRQMsg->message != (XCHAR*)pRQMsg+sizeof(t_XOSCOMMHEAD) )
        {
            /*to do 出现严重错误*/
            XOS_Trace(MD(FID_ROOT, PL_ERR), "MOD_allTaskEntry()->recieve msg failed !");
            /* 暂时不作处理*/
            continue;
        }
        /*
        if(pMsg->datadest.FID == FID_NTL || pMsg->datasrc.FID == FID_NTL)
        {
        XOS_Trace(MD(FID_ROOT, PL_WARN), "QUE_MsgQRecv:NTL detail----------------begin");
        XOS_Trace(MD(FID_ROOT, PL_WARN), "pMsg->datadest.FID=%d",pMsg->datadest.FID);
        XOS_Trace(MD(FID_ROOT, PL_WARN), "pMsg->datasrc.FID=%d",pMsg->datasrc.FID);
        XOS_Trace(MD(FID_ROOT, PL_WARN), "pMsg->length=%d",pMsg->length);
        XOS_Trace(MD(FID_ROOT, PL_WARN), "pMsg->msgID=%d",pMsg->msgID);
        XOS_Trace(MD(FID_ROOT, PL_WARN), "QUE_MsgQRecv:NTL detail----------------end");
        }
        */
        /*消息的分发, 平台会处理定时器的时钟源驱动消息*/
        /*获取fid 的信息*/
        pFidCb = (t_FIDCB*)XNULLP;
        pFidCb = MOD_getFidCb(pRQMsg->datadest.FID);
        if ( pFidCb == XNULLP )
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), "MOD_allTaskEntry()->get fid cb failed !");
            /* 暂时不作处理,只释放消息*/
            XOS_MsgMemFree((XU32)FID_ROOT, pRQMsg);
            continue;
        }

        /*时钟源的驱动消息*/
        if ( pRQMsg->datasrc.FID == FID_TIME
            &&(pRQMsg->msgID == eTimeHigClock || pRQMsg->msgID == eTimeLowClock))
        {
#ifndef XOS_NEED_OLDTIMER
            TIM_ClckProc((XVOID*)pRQMsg);
#else
            /*时钟源消息的处理函数*/
            if( pRQMsg->msgID == eTimeHigClock )
            {
                TIM_ClckProc(MOD_getTimMntByFid(TIMER_PRE_HIGH,pRQMsg->datadest.FID));
            }
            else
            {
                TIM_ClckProc(MOD_getTimMntByFid(TIMER_PRE_LOW,pRQMsg->datadest.FID));
            }
#endif
            /*所有的定时器时钟驱动消息都是由平台释放*/
            XOS_MsgMemFree(pRQMsg->datasrc.FID, pRQMsg);
            continue;
        }

        /* 如果是NTL模块时打印队列数量到SYSLOG*/
        /*代码调试时为了跟踪需要打印，正式发布版本可注释掉*/
        /*
        if(pRQMsg->datadest.FID == FID_NTL) {
            
            queue_nums = XOS_listMaxUsage(pTidCb->recvMsgQue.queueList);
            
            if(fid_stat[pRQMsg->datasrc.FID] < 0) {
                fid_stat[pRQMsg->datasrc.FID] = 0;
            }
            fid_stat[pRQMsg->datasrc.FID]++;
            XOS_StrCpy(fid_name[pRQMsg->datasrc.FID], XOS_getFidName(pRQMsg->datasrc.FID));
            
            if((queue_nums - old_queue_nums) >= 200 ) {
                
                old_queue_nums = queue_nums;
                
                trace_msgq_size("Ntl module msgq maxUsage: %d\n", queue_nums);
                
                for(listIndex = 0; listIndex < 2048; listIndex++) {
                    if(fid_stat[listIndex] > 0) {
                        trace_msgq_size("Ntl module msgq detail: srcFid=%d, name=%s, used=%d\n",
                            listIndex,
                            fid_name[listIndex],
                            fid_stat[listIndex]);
                    }
                }
                
            }
        }*/
        
        /*高精度定时器的超时消息*/
        /* todo 高精度定时器暂时没有实现*/

#ifdef XOS_LINUX
        /* HA的线程监控 直接返回消息 BEGIN: Added by liujun, 2015/1/15 */
        if (FID_HA == pRQMsg->datasrc.FID && HA_WATCH_MSG_REQ == pRQMsg->msgID)
        {
            HA_XOSHelloProcess(pRQMsg->datadest.FID,pRQMsg->message,pRQMsg->length);
            if (eXOSMode == pFidCb->msgFreeType)
            {
                XOS_MsgMemFree(pRQMsg->datasrc.FID, pRQMsg);
            }
            continue;
        }
#endif
        /* END:   Added by liujun, 2015/1/15 */

        /*模块间的消息*/
        if(pFidCb->MsgFunc != XNULLP)
        {
#ifdef XOS_ModMem_Check
            if(XSUCC != XOS_MemCheck(FID_ROOT,pRQMsg))
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR), "check mem error.srcFid(%d),dstFid(%d),msgid(%d)",
                                                pRQMsg->datasrc.FID,
                                                pRQMsg->datadest.FID,
                                                pRQMsg->msgID);
#ifdef XOS_DEBUG
                /*调试用，如出现内存错误，则直接挂起*/
                XOS_SusPend();
#endif
                continue;
            }
#endif

#ifdef XOS_TRACE_AGENT
            XOS_TaTrace(pRQMsg->datadest.FID,e_TA_RECV,pRQMsg);
#endif
            pFidCb->MsgFunc((XVOID*)pRQMsg, &(pFidCb->fidNum));
        }
        /*释放消息内存*/
        if(pFidCb->msgFreeType == eXOSMode)
        {
            XOS_MsgMemFree(pRQMsg->datadest.FID, pRQMsg);
        }
    }
}


/*-------------------------------------------------------------------------
               模块接口函数
-------------------------------------------------------------------------*/
/************************************************************************
函数名: MOD_init
功能:  此模块的初始化
输入:
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明:
************************************************************************/
XS32 MOD_init(void)
{
    /* 没有初始化时初始化*/
    if(g_modMnt.isInit)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "MOD_init()->module managment is initlized !");
        return XSUCC;
    }

    /*创建hash表*/
    g_modMnt.fidHash =
        XOS_HashConstruct(MAX_FID_NUMS+1, MAX_FID_NUMS, sizeof(XU32), sizeof(t_FIDCB), "fidHash");
    if(!XOS_HashHandleIsValid(g_modMnt.fidHash))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "MOD_init()->construct hash failed  !");
        return XERROR;
    }
    /*设置hash 函数*/
    XOS_HashSetHashFunc(g_modMnt.fidHash, MOD_hashFunc);

    /*创建tid的管理结构*/
    g_modMnt.tidArray =
        XOS_ArrayConstruct(sizeof(t_TIDCB),  MAX_TID_NUMS, "tidArray");
    if(!XOS_ArrayHandleIsValid(g_modMnt.tidArray))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "MOD_init()->construct tid array failed  !");
        XOS_HashDestruct(g_modMnt.fidHash);
        return XERROR;
    }

    /*设置数组的比较函数*/
    XOS_ArraySetCompareFunc(g_modMnt.tidArray, MOD_isSameTid);

    g_modMnt.isInit = XTRUE;
    return XSUCC;
}


/*----------------------------------------------------------------------
             以下是模块的注册和启动部分
------------------------------------------------------------------------*/
#ifndef XOS_MDLMGT
/************************************************************************
函数名: MOD_readXosFids
功能:  读取平台配置部分
输入:
headFid   开始读的fid
tailFid  读结束的fid
ctrlFlag  控制字, xfalse 是平台的,  xtrue 是业务用的
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 这是一个临时的函数, 在作模块管理的时候应该
    修改,目前通过读.c文件中的配置, 作模块管理时
    改成读xml 文件的配置就可以了.完成的功能应该
    是一样的.
    此处有一个bug :
    当多个fid注册到同一个tid 的时候,tid的信息以第
    一个fid注册到该tid 时为主.
    因为这只是一个临时函数,并且暂不影响使用,
    只要注册时注意就可以了.所以暂不修改
************************************************************************/
XS32 MOD_readCfgFids(XU32 headFid, XU32 tailFid, XBOOL ctrlFlag)
{
    XS32 ret;
    XU32 i;
    XU32 fid;
    t_FIDCB fidCb;
    t_TIDCB tidCb;
    XS32 tidIndex;
    XVOID *pLocation;
    t_XOSLOGINLIST *pLog;

    /*执行.c 配置中的配置函数*/
    if(!ctrlFlag)
    {
        pLog = XOS_MemMalloc(FID_ROOT, FID_XOSMAX * sizeof(t_XOSLOGINLIST));
        
        if(XNULLP == pLog )
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), " XOS_cfgXosModule()-> malloc error!");
            return XERROR;
        }
        XOS_MemSet(pLog, XNULL, FID_XOSMAX * sizeof(t_XOSLOGINLIST));

        ret = XOS_cfgXosModule(pLog);
    }
    else
    {
        pLog = XOS_MemMalloc(FID_ROOT, (FID_MAX-FID_USERMIN) * sizeof(t_XOSLOGINLIST));
        if(XNULLP ==  pLog)
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), " XOS_cfgXosModule()-> malloc error!");
            return XERROR;
        }
        XOS_MemSet(pLog, XNULL, (FID_MAX-FID_USERMIN ) * sizeof(t_XOSLOGINLIST));
        ret = XOS_InfoReg(pLog);
    }

    if( ret != XSUCC)
    {
        return XERROR;
    }

    /*先读功能块相关的信息*/
    for(i = 0; i< tailFid-headFid; i++)
    {
        if(pLog[i].stack == XNULLP)
        {
            continue;
        }
        XOS_MemSet(&fidCb, 0, sizeof(t_FIDCB));

        /*填写fid 控制块元素*/
        fid = pLog[i].stack->head.FID;
        fidCb.attSwitch = XFALSE;
        XOS_MemCpy(fidCb.fidName, pLog[i].stack->head.FIDName, MAX_FID_NAME_LEN);
        fidCb.fidNum = fid;
        fidCb.initFunc = pLog[i].stack->init.init;
        fidCb.noticeFunc = pLog[i].stack->init.notice;
        fidCb.MsgFunc = pLog[i].stack->handle.message;
        fidCb.timerFunc = pLog[i].stack->handle.timer;
        fidCb.pidIndex  = XOS_GetLocalPID();
        fidCb.msgFreeType = pLog[i].stack->memfreetype;
        fidCb.attSwitch = XFALSE;   /*暂时设置所有的模块起来时，
                                    消息转接开关为关闭状态，做
                                    配置文件启动时，这个字段可配*/
        fidCb.noticeFlag = XFALSE;
        fidCb.logLevel = PL_ERR;

        initFidTraceInfo((&(fidCb.traceInfo))); /*初始化trace 的信息*/
#ifdef XOS_IPC_MGNT
        if(FID_IPCMGNT == fidCb.fidNum)
        {
            fidCb.traceInfo.traceLevel = PL_WARN;
        }
#endif

        /*业务的模块不容许注册到平台的任务上*/
        if(ctrlFlag && pLog[i].TID <= FID_USERMIN)
        {
            XOS_Trace(MD(FID_ROOT, PL_WARN),
                "fid[%d] try to reg in plat task [%d] failed!", fid, pLog[i].TID);
            continue;
        }

        /*tid index 需要查找后确定*/
        tidIndex = XOS_ArrayFind(g_modMnt.tidArray, (XVOID*)pLog[i].TID);
        if(tidIndex >= 0 || pLog[i].TID == 0) /*找到或不需要任务的fid*/
        {
            fidCb.tidIndex = tidIndex;
        }
        else /*没有找到*/
        {
            XOS_MemSet(&tidCb, 0, sizeof(t_TIDCB));
            tidCb.prio = pLog[i].prio;
            tidCb.stackSize = pLog[i].stacksize;
            tidCb.tidNum = pLog[i].TID;
            XOS_MemCpy(&(tidCb.tskName), pLog[i].taskname, MAX_TID_NAME_LEN);
            /* tidCb.timerNum = pLog[i].timenum; */
            /* tidCb.maxMsgsInQue = MAX_MSGS_IN_QUE;   */   /*暂时写死*/

            if ( MIN_MSGS_IN_QUE  > pLog[i].quenum  ||  MAX_MSGSNUM_IN_QUE <= pLog[i].quenum)
            {
                tidCb.maxMsgsInQue = MAX_MSGS_IN_QUE;
            }
            else
            {
                tidCb.maxMsgsInQue = pLog[i].quenum;
            }

            /*增加tid 的控制块*/
            tidIndex = XOS_ArrayAdd(g_modMnt.tidArray, (XOS_ArrayElement)&tidCb);
            if(tidIndex < 0)
            {
                return XERROR;
            }
            fidCb.tidIndex = tidIndex;
        }

        /*增加一个功能控制块*/
        pLocation = XNULLP;
        pLocation = XOS_HashElemAdd(g_modMnt.fidHash, (XVOID*)&fid, (XVOID*)&fidCb, XFALSE);
        if(pLocation == XNULLP)
        {
            return XERROR;
        }
    }

    XOS_MemFree(FID_ROOT, pLog);
    return XSUCC;
}


/************************************************************************
函数名: MOD_startXosFids
功能:  启动平台的相关模块
输入:
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明:  此函数在做模块管理时应该修改.
       主要修改点, 模块的启动顺序可控.
************************************************************************/
XS32 MOD_startXosFids(void)
{
    XS32 ret, retval;
    XS32 i;
    XS32 tskIndex;
    t_TIDCB *pTidCb;
    t_FIDCB *pFidCb;

    /*首先读取平台的模块配置*/
    ret = MOD_readCfgFids((XU32)FID_XOSMIN, (XU32)FID_XOSMAX, XFALSE);
    if(ret != XSUCC)
    {
        return XERROR;
    }

    /*创建消息队列*/
    for(i=FID_XOSMIN; i<FID_XOSMAX; i++)
    {
        pFidCb = (t_FIDCB*)XNULLP;
        pFidCb = MOD_getFidCb(i);
        if(pFidCb == XNULLP)
        {
            continue;
        }

        tskIndex = XOS_ArrayFind(g_modMnt.tidArray, (XVOID*)i);
        if(tskIndex >= 0)
        {
            pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, tskIndex);
            /*xos 中,定时器任务任务入口函数比较特殊*/
            if (pTidCb->tidNum== FID_TIME)
            {
                /*定时器必须先执行初始化函数*/
                pFidCb->initFunc(XNULLP, XNULLP);
#ifndef XOS_NEED_OLDTIMER                        
                /*创建定时器工作处理任务*/
                ret = XOS_TaskCreate((pTidCb->tskName), pTidCb->prio, pTidCb->stackSize,
                    (os_taskfunc)tm_excuteLowProc, XNULLP, &(pTidCb->tskId));
                if(XSUCC != ret)
                {
                    MMErr("FID_TIME:XOS_MMStartFid()-> can not create task[tsk_dealtimer]!");
                    return XERROR;
                }
#else
                /*低精度定时器*/
                ret = XOS_TaskCreate((pTidCb->tskName), (e_TSKPRIO)pTidCb->prio, pTidCb->stackSize,
                    (os_taskfunc)Low_TimerTask, XNULLP, &(pTidCb->tskId));
                if(XSUCC != ret)
                {
                    XOS_Trace(MD(FID_TIME,PL_ERR),"MOD_startXosFids()-> TIME is wrong!");
                    return XERROR;
                }
#ifdef XOS_HIGHTIMER
                /*高精度定时器任务*/
                ret = XOS_TaskCreate("tsk_HTIM", (e_TSKPRIO)pTidCb->prio, pTidCb->stackSize,
                    (os_taskfunc)High_TimerTask, XNULLP, &(pTidCb->tskId));
                if(XSUCC != ret)
                {
                    XOS_Trace(MD(FID_TIME,PL_ERR),"MOD_startXosFids()-> HIGHTIME is wrong!");
                    return XERROR;
                }
#endif
#endif
                continue;
            }
            else
            {
                pTidCb->recvMsgQue.tskIndex = pTidCb->tidNum;
                retval = QUE_MsgQCreate(&(pTidCb->recvMsgQue), pTidCb->maxMsgsInQue);
                if(retval != XSUCC)
                {
                    XOS_Trace(MD(FID_ROOT, PL_ERR),
                        "MOD_startXosFids()->create msg que failed ! tid: %d, tskName: %s",
                        pTidCb->tidNum, pTidCb->tskName);
                    return XERROR;
                }
                ret = XOS_TaskCreate((pTidCb->tskName), (e_TSKPRIO)pTidCb->prio, pTidCb->stackSize,
                    (os_taskfunc)MOD_allTaskEntry, (XVOID*)pTidCb, &(pTidCb->tskId));

                /* 注册死锁监控回调 BEGIN: Added by liujun, 2015/1/15 */
                HA_AddToDeadWatch(pFidCb->fidNum,pFidCb->tidIndex);
                
            }
            /*确定任务启动成功*/
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR),
                    "MOD_startXosFids()->create task failed ! tid: %d, tskName: %s",
                    pTidCb->tidNum, pTidCb->tskName);
                return XERROR;
            }
        }

        /*执行各模块的初始化函数*/
        if(pFidCb->initFunc == XNULLP)
        {
            continue;
        }
        ret = pFidCb->initFunc(XNULLP, XNULLP);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_ROOT, PL_WARN),
                "MOD_startXosFids()->excute fid[%s] init func failed!",
                pFidCb->fidName);
            return XERROR;
        }
    }
    return XSUCC;
}


/************************************************************************
函数名: MOD_startUserFids
功能:  启动业务的相关模块
输入:
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明:
************************************************************************/
XS32 MOD_startUserFids(void)
{
    XS32 ret;
    XS32 i;
    XS32 tskIndex;
    t_TIDCB *pTidCb;
    t_FIDCB *pFidCb;

    /*读取应用模块的信息*/
    ret = MOD_readCfgFids((XU32)FID_USERMIN, (XU32)FID_MAX, XTRUE);
    if(ret != XSUCC)
    {
        return XERROR;
    }

    /*创建消息队列*/
    for(i=FID_USERMIN; i<FID_MAX; i++)
    {
        /*创建消息队列并起任务*/
        tskIndex = XOS_ArrayFind(g_modMnt.tidArray, (XVOID*)i);
        if(tskIndex >= 0)
        {
            pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, tskIndex);
            pTidCb->recvMsgQue.tskIndex = pTidCb->tidNum;
            ret = QUE_MsgQCreate(&(pTidCb->recvMsgQue), pTidCb->maxMsgsInQue);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR),
                    "MOD_startXosFids()->create msg que failed ! tid: %d, tskName: %s",
                    pTidCb->tidNum, pTidCb->tskName);
                return XERROR;
            }

            /*启动任务*/
            ret = XOS_TaskCreate(pTidCb->tskName, (e_TSKPRIO)pTidCb->prio, pTidCb->stackSize,
                (os_taskfunc)MOD_allTaskEntry, (XVOID*)pTidCb, &(pTidCb->tskId));
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR),
                    "MOD_startXosFids()->create task failed ! tid: %d, tskName: %s",
                    pTidCb->tidNum, pTidCb->tskName);
                return XERROR;
            }
        }

        /*执行初始化函数*/
        pFidCb = (t_FIDCB*)XNULLP;
        pFidCb = MOD_getFidCb(i);
        if(pFidCb == XNULLP ||pFidCb->initFunc == XNULLP)
        {
            continue;
        }
        ret = pFidCb->initFunc(XNULLP, XNULLP);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_ROOT, PL_WARN),
                "MOD_startXosFids()->excute fid[%s] init func failed!",
                pFidCb->fidName);
            return XERROR;
        }
    }

    return XSUCC;
}
#endif


/************************************************************************
函数名: MOD_startNotice
功能:  启动通知函数
输入:
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 为了兼容1.81版，提供临时函数
************************************************************************/
XS32 MOD_StartNotice(XVOID)
{
    XS32 ret;
    XS32 i;
    t_FIDCB *pFidCb;

    for(i = FID_XOSMIN; i<FID_MAX; i++)
    {
        pFidCb = (t_FIDCB*)XNULLP;
        pFidCb = MOD_getFidCb(i);
        if(XNULL == pFidCb)
        {
            continue;
        }

        /*设置日志打印级别,各个模块可在notice模块里修改默认日志级别*/
        pFidCb->logLevel = PL_ERR;

        if(XNULL == pFidCb->noticeFunc)
        {
            continue;
        }

        if(pFidCb->noticeFlag == XFALSE)
        {
            ret = pFidCb->noticeFunc(XNULLP, XNULLP);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR),
                    "MOD_StartNotice()->excute fid[%s] init func failed!",
                    pFidCb->fidName);
                return XERROR;
            }
            pFidCb->noticeFlag = XTRUE;
        }
    }
#ifdef MEM_FID_DEBUG
    bModifyFidFlg = XTRUE;/*等待所有模块初始化完成，打开fid修正开关*/
#endif
    
    return XSUCC;
}


/*----------------------------------------------------------------------
                  以下是获取和设置模块相关的信息函数
------------------------------------------------------------------------*/
/************************************************************************
函数名: XOS_isValidFid
功能：  判断fid是否有效
输入：  fid   功能模块号
输出：  N/A
返回：  XFALSE OR XTRUE
说明：
************************************************************************/
XPUBLIC XBOOL XOS_isValidFid(XU32 fid)
{
    if(FID_XOSMIN < fid && fid < FID_MAX)
    {
        return XTRUE;
    }
    else
    {
        return XFALSE;
    }
}


/************************************************************************
函数名: XOS_getFidName
功能：  获取功能块消息转接开关的值
输入：  fid   功能模块号
输出：  N/A
返回：  名字或空指针
说明：
************************************************************************/
XPUBLIC XCHAR* XOS_getFidName(XU32 fid)
{
    t_FIDCB* pFidCb = (t_FIDCB*)XNULLP;

    pFidCb = MOD_getFidCb( fid);
    if(pFidCb != XNULLP)
    {
        return pFidCb->fidName;
    }
    else
    {
        return (XCHAR*)XNULLP;
    }
}


/************************************************************************
函数名: MOD_getFidAttSwitch
功能：  获取功能块消息转接开关的值
输入：  fid   功能模块号
输出：  N/A
返回：  XFALSE OR XTRUE
说明：
************************************************************************/
XPUBLIC XBOOL MOD_getFidAttSwitch(XU32 fid)
{
    t_FIDCB* pFidCb = (t_FIDCB*)XNULLP;

    pFidCb = MOD_getFidCb( fid);
    if(pFidCb != XNULLP)
    {
        return pFidCb->attSwitch;
    }
    else
    {
        /*错误比较严重*/
        XOS_CpsTrace(MD(FID_ROOT, PL_INFO), "MOD_getFidAttSwitch()-> fid [%d] is not in fid hash,", fid);
        return XFALSE;
    }
}


/************************************************************************
函数名: MOD_setFidAttSwitch
功能：  set功能块消息转接开关的值
输入：  fid   功能模块号
valve 开关状态，打开XTRUE , 关闭XFALSE
输出：  N/A
返回：  XFALSE OR XTRUE
说明：
************************************************************************/
XPUBLIC XS16 MOD_setFidAttSwitch(XU32 fid, XBOOL valve)
{
    t_FIDCB* pFidCb = (t_FIDCB*)XNULLP;

    pFidCb = MOD_getFidCb( fid);
    if(pFidCb != XNULLP)
    {
        pFidCb->attSwitch = valve;
        return XSUCC;
    }
    else
    {
        /*错误比较严重*/
        XOS_Trace(MD(FID_ROOT, PL_INFO), "MOD_setFidAttSwitch()-> fid [%d] is not in fid hash,", fid);
        return XERROR;
    }
}


/************************************************************************
函数名: MOD_getTimMntByFid
功能：  通过功能块号定时器管理结构指针
输入：  fid   功能模块号
输出：  N/A
返回：  任务控制块指针,失败返回空
说明：
************************************************************************/
#ifdef XOS_NEED_OLDTIMER
XPUBLIC XVOID * MOD_getTimMntByFid(e_TIMERPRE pre,XU32 fid)
{
    t_FIDCB *pFidCb;

    pFidCb = MOD_getFidCb(fid);
    if(pFidCb == XNULLP)
    {
        return XNULLP;
    }
    if(pre ==  TIMER_PRE_LOW)
        return &(pFidCb->timerMngerLow);
    else if(pre ==  TIMER_PRE_HIGH)
        return &(pFidCb->timerMngerHig);
    else
        return XNULLP;
}
#endif

/************************************************************************
函数名: MOD_getTimProcFunc
功能：  通过功能块号获取功能块的超时处理函数
输入：  fid   功能模块号
输出：  N/A
返回：  任务控制块指针,失败返回空
说明：
************************************************************************/
XPUBLIC  modTimerProcFunc MOD_getTimProcFunc(XU32 fid)
{
    t_FIDCB *pFidCb;
    pFidCb = MOD_getFidCb( fid);
    if(pFidCb == XNULLP)
    {
        return XNULLP;
    }
    return (modTimerProcFunc)pFidCb->timerFunc;
}


/*----------------------------------------------------------------------
                   以下是模块间通信的部分
------------------------------------------------------------------------*/
/************************************************************************
函数名: XOS_GetLocalPID( XVOID )
功能:   获取当前平台的 PID
输入:   无
输出:   当前平台的 PID
返回:
说明:
************************************************************************/
XPUBLIC XU32 XOS_GetLocalPID( XVOID )
{
    return g_XOS_LocalPid;
}


/************************************************************************
函数名: XOS_SetLocalPid(  )
功能:   设置当前平台的 PID
输入:   无
输出:
返回:
说明:
************************************************************************/
XVOID XOS_SetLocalPID(XU32 localpid)
{
    g_XOS_LocalPid = localpid;
}


/************************************************************************
函数名: XOS_MsgMemAlloc
功能：  给予需传输消息分配一个内存块
输入：
fid   - 功能块id
nbytes  - 消息的长度
输出：  N/A
返回：  t_XOSCOMMHEAD * － 分配的消息内存指针
说明：  给现在的消息队列临时用
************************************************************************/
t_XOSCOMMHEAD *XOS_MsgMemMalloc(XU32 fid, XU32 nbytes)
{
    t_XOSCOMMHEAD *pMsg ;
    XVOID *pTemp = XNULLP;

    pTemp = XOS_MemMalloc( fid, nbytes+sizeof(t_XOSCOMMHEAD));
    if(pTemp == XNULLP)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MsgMemMalloc()->mem malloc failed !");
        return (t_XOSCOMMHEAD*)XNULLP;
    }
    pMsg = (t_XOSCOMMHEAD*)pTemp;
    pMsg->length = nbytes;   /*消息的长度平台自动补全*/
    pMsg->message = (XVOID*)(((XCHAR*)pMsg)+sizeof(t_XOSCOMMHEAD));
    pMsg->traceID = 0;
    pMsg->logID = 0;

    /* add by wulei 2006.12.08 */
#ifdef XOS_MSGMEM_TRACE
    /* 加入消息内存监控部分 */
    XOS_RecordMsgBuf(fid, pMsg);
#endif /* XOS_MSGMEM_TRACE */

    return pMsg;
}


/************************************************************************
函数名: XOS_MsgMemFree
功能：  释放一个消息内存块
输入：
fid   - 功能块id
t_XOSCOMMHEAD * - 消息内存块指针
输出：  N/A
返回：  t_XOSCOMMHEAD * － 分配的消息内存指针
说明：
************************************************************************/
XVOID XOS_MsgMemFree(XU32 fid, t_XOSCOMMHEAD *ptr)
{
#ifdef XOS_MSGMEM_TRACE
    /* 加入消息内存监控部分 */
    XOS_ReleaseMsgBuf(fid, ptr);
#endif /* XOS_MSGMEM_TRACE */

    XOS_MemFree(fid, (XVOID*)ptr );
    return;
}

XS32 xos_msgHead2Host(t_XOSCOMMHEAD *pHead)
{
    pHead->datadest.PID = ntohl(pHead->datadest.PID);
    pHead->datadest.FID = ntohl(pHead->datadest.FID);
    pHead->datadest.FsmId = ntohl(pHead->datadest.FsmId);
    
    pHead->datasrc.PID = ntohl(pHead->datasrc.PID);
    pHead->datasrc.FID = ntohl(pHead->datasrc.FID);
    pHead->datasrc.FsmId = ntohl(pHead->datasrc.FsmId);

    pHead->msgID = ntohs(pHead->msgID);
    pHead->subID = ntohs(pHead->subID);
    pHead->prio  = ntohl(pHead->prio);
    pHead->length = ntohl(pHead->length);

    return 0;
}

XS32 xos_msgHead2Net(t_XOSCOMMHEAD *pHead)
{
//    t_XOSCOMMHEAD *pHead;

    
    pHead->datadest.PID = htonl(pHead->datadest.PID);
    pHead->datadest.FID = htonl(pHead->datadest.FID);
    pHead->datadest.FsmId = htonl(pHead->datadest.FsmId);
    
    pHead->datasrc.PID = htonl(pHead->datasrc.PID);
    pHead->datasrc.FID = htonl(pHead->datasrc.FID);
    pHead->datasrc.FsmId = htonl(pHead->datasrc.FsmId);

    pHead->msgID = htons(pHead->msgID);
    pHead->subID = htons(pHead->subID);
    pHead->prio  = htonl(pHead->prio);
    pHead->length = htonl(pHead->length);

    return 0;
}



/************************************************************************
函数名: XOS_MsgSend
功能：  模块间通信的消息发送函数
输入：  pMsg: 消息公共头信息
输出：  N/A
返回：  XSUCC OR XERROR
说明：  依据用户填写的信息头，将消息发送到目的地
************************************************************************/
extern XU32   g_ulMailBoxKey;
XPUBLIC XS32 XOS_MsgSend(t_XOSCOMMHEAD *pMsg)
{
    t_TIDCB *pTidCb;

    /*函数入口安全性检查*/
    if(pMsg == XNULLP || !XOS_isValidFid(pMsg->datadest.FID)
        ||!XOS_isValidFid(pMsg->datasrc.FID)
        ||pMsg->message != (XCHAR*)pMsg+sizeof(t_XOSCOMMHEAD))
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MsgSend()-> invalid Msg!pMsg: 0x%x,", pMsg);
        return XERROR;
    }

    /*获取接收任务tid 的信息*/

    pTidCb = (t_TIDCB*)XNULLP;
    pTidCb = MOD_getTidCbByFid(pMsg->datadest.FID);
    if(pTidCb == XNULLP)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR),
            "XOS_MsgSend()->srcFid %d,get Tid cb by dst fid[%d] failed ", pMsg->datasrc.FID,pMsg->datadest.FID);
        return XERROR;
    }

    /*发送消息到接收者的消息队列*/
#ifdef XOS_MSGMEM_TRACE
    XOS_UpdateMsgBuf(pMsg);
#endif /* XOS_MSGMEM_TRACE */

    /*发送消息到接收者的消息队列*/    
    return QUE_MsgQSend(&(pTidCb->recvMsgQue), (XVOID*)pMsg, pMsg->prio);
}





XS32 XOS_MsgDistribution(XU32 ipcIdx, XU32 remoteIdx, XU8 *p, XS32 len)
{
    t_TIDCB *pTidCb;
    t_XOSCOMMHEAD* pMsg      = XNULLP;


    if(len < sizeof(t_XOSCOMMHEAD))
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MsgDistribution err len < sizeof(t_XOSCOMMHEAD)");
        return XERROR;
    }

    pMsg = XOS_MsgMemMalloc( FID_IPC, len-sizeof(t_XOSCOMMHEAD) );
    if ( XNULLP == pMsg )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MsgDistribution err in XOS_MsgMemMalloc");
        return XERROR;
    }
    memcpy(pMsg, p, len);
    xos_msgHead2Host(pMsg);
    pMsg->message = (XVOID*)(((XCHAR*)pMsg)+sizeof(t_XOSCOMMHEAD));

    /*函数入口安全性检查*/
    if(pMsg == XNULLP || !XOS_isValidFid(pMsg->datadest.FID)
        ||!XOS_isValidFid(pMsg->datasrc.FID)
        ||pMsg->message != (XCHAR*)pMsg+sizeof(t_XOSCOMMHEAD))
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MsgSend()-> invalid Msg!pMsg: 0x%x,", pMsg);
        return XERROR;
    }
    
    /*获取接收任务tid 的信息*/
    pTidCb = (t_TIDCB*)XNULLP;
    pTidCb = MOD_getTidCbByFid(pMsg->datadest.FID);
    if(pTidCb == XNULLP)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR),
            "XOS_MsgSend()->srcFid %d,get Tid cb by dst fid[%d] failed ", pMsg->datasrc.FID,pMsg->datadest.FID);
        return XERROR;
    }

    /*发送消息到接收者的消息队列*/
#ifdef XOS_MSGMEM_TRACE
    XOS_UpdateMsgBuf(pMsg);
#endif /* XOS_MSGMEM_TRACE */

    /*发送消息到接收者的消息队列*/
    return QUE_MsgQSend(&(pTidCb->recvMsgQue), (XVOID*)pMsg, pMsg->prio);

    return XSUCC;
}



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
XPUBLIC XS32 XOS_MsgBroadcast(t_XOSCOMMHEAD *pMsg, brdcstFilterFunc brdcstFunc)
{
    XS32 ret;
    XS32 tidIndex;
    t_TIDCB *pTidCb;
    t_XOSCOMMHEAD *pMsgTemp;

    if(pMsg == XNULLP || brdcstFunc == XNULLP)
    {
        return XERROR;
    }

    tidIndex = XOS_ArrayGetFirstPos(g_modMnt.tidArray);
    while (tidIndex >= 0)
    {
        pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
        if(brdcstFunc (pTidCb->tidNum))
        {
            /*拷贝一条消息*/
            pMsgTemp = (t_XOSCOMMHEAD*)XNULLP;
            pMsgTemp = XOS_MsgMemMalloc(pMsg->datasrc.FID, pMsg->length);
            if(pMsgTemp == XNULLP)
            {
                continue;
            }
            pMsgTemp->datasrc = pMsg->datasrc;
            pMsgTemp->msgID = pMsg->msgID;
            pMsgTemp->prio = pMsg->prio;
            pMsgTemp->subID = pMsg->subID;
            pMsgTemp->datadest.PID = pMsg->datadest.PID;
            pMsgTemp->datadest.FsmId = pMsg->datadest.FsmId;
            pMsgTemp->datadest.FID = pTidCb->tidNum;
            if(pMsgTemp->length > 0)
            {
                XOS_MemCpy(pMsgTemp->message, pMsg->message, pMsg->length);
            }
            ret = XOS_MsgSend(pMsgTemp);
            if(ret != XSUCC)
            {
                XOS_MsgMemFree(pMsgTemp->datasrc.FID, pMsgTemp);
            }
        }

        tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
    }
    /*释放消息*/
    XOS_MsgMemFree((XU32)FID_ROOT, pMsg);
    return XSUCC;
}


/*----------------------------------------------------------------------
                     与模块相关的调试命令函数
------------------------------------------------------------------------*/
/************************************************************************
函数名: MOD_getFidTraceInfo
功能：  fid 相关的trce信息
输入：  fid   功能模块号
输出：  N/A
返回：  成功返回fid 相关的trace信息，失败返回XNULLP
说明：
************************************************************************/
t_FIDTRACEINFO* MOD_getFidTraceInfo(XU32 fid)
{
    t_FIDCB* pFidCb = (t_FIDCB*)XNULLP;

    pFidCb = MOD_getFidCb( fid);
    if(pFidCb != XNULLP)
    {
        return &(pFidCb->traceInfo);
    }
    else
    {
        return (t_FIDTRACEINFO*)XNULLP;
    }
}


/************************************************************************
函数名: MOD_setAllTraceInfo
功能：  设置所有的fid 相关的trace信息
输入：
setFunc   设置函数
param  设置函数的输入参数
输出：  N/A
返回：  成功返回XSUCC,  失败返回XERROR
说明：
************************************************************************/
XS32 MOD_setAllTraceInfo(setFidTraceInfo setFunc, XVOID* param,XVOID* param2)
{
    t_ALLFIDTRACE allFidTraceInfo;

    if(setFunc == XNULLP)
    {
        return XERROR;
    }
    XOS_MemSet(&allFidTraceInfo, 0, sizeof(t_ALLFIDTRACE));
    allFidTraceInfo.setTraceFunc = setFunc;
    allFidTraceInfo.param = param;
    allFidTraceInfo.param2 = param2;

    return XOS_HashWalk(g_modMnt.fidHash, MOD_setFidTrace, (XVOID*)&allFidTraceInfo);
}


/************************************************************************
函数名: MOD_fidInfoShow
功能：显示所有的fid 信息
输入：
输出：  N/A
返回：  XSUCC OR XERROR
说明：fidshow 的命令解析函数
************************************************************************/
XVOID MOD_fidInfoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    t_FIDSHOW fidShow;

    /*fid list show  */
    XOS_CliExtPrintf(pCliEnv,
        "fid list \r\n---------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-6s%-18s%-18s%-10s%-10s%-10s\r\n",
        "index",
        "fid",
        "fidName",
        "tidName",
        "attSwitch",
        "traceLev",
        "frMsgType"
        );
    XOS_MemSet(&fidShow, 0, sizeof(t_FIDSHOW));
    fidShow.pCliEnv = pCliEnv;
    fidShow.index = 0;
    /*遍历hash*/
    XOS_HashWalk(g_modMnt.fidHash, MOD_showFid, &fidShow);

    /*end of fid list */
    XOS_CliExtPrintf(pCliEnv,
        "---------------------------------------------------------------------------\r\n");

    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n" , fidShow.index);
    return;
}

/************************************************************************
函数名: MOD_GetTskPid
功能：获取的tid的pid
输入：
输出：  N/A
返回：得到返回0，没找到返回-1
说明：
************************************************************************/
XS32 MOD_GetTskPid(XU32 *pid,XU32 fid)
{    
    t_TIDCB *pTidCb = NULL;
    t_FIDCB *pFidCb = NULL;

    *pid = 0;

    pFidCb = MOD_getFidCb(fid);
    if(XNULL != pFidCb)
    {
        pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);
        if(pTidCb != (t_TIDCB*)XNULLP)
        {
            if (0 != pTidCb->tskpid)
            {
                *pid = pTidCb->tskpid;
                return XSUCC;
            }
        }
    }

    return XERROR;
}

/************************************************************************
函数名: MOD_tidInfoShow
功能：显示所有的tid 信息
输入：
输出：  N/A
返回：
说明：tidshow 的命令解析函数
************************************************************************/
XVOID MOD_tidInfoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    t_TIDCB* pTidCb;
    XS32 tidIndex;
    XS32 j;

    /*tid list show  */
    XOS_CliExtPrintf(pCliEnv,
        "tid list \r\n-------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-6s%-18s%-8s%-12s%-6s%-10s%-6s\r\n",
        "index",
        "fid",
        "tidName",
        "pid",
        "tid",
        "prio",
        "stkSize",
        "msgs"
        );

    tidIndex = XOS_ArrayGetFirstPos(g_modMnt.tidArray);
    j = 0;
    while(tidIndex >= 0)
    {
        pTidCb = (t_TIDCB*)XNULLP;
        pTidCb = (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
        if(pTidCb == XNULLP)
        {
            continue;
        }
        XOS_CliExtPrintf(pCliEnv,
            "%-6d%-6d%-18s%-8u0x%-10x%-6d%-10d%-6d\r\n",
            j,
            pTidCb->tidNum,
            pTidCb->tskName,
            pTidCb->tskpid,
            pTidCb->tskId,
            pTidCb->prio,
            pTidCb->stackSize,
            pTidCb->maxMsgsInQue
            );
        tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
        j++;
    }
    /*end of tid list */
    XOS_CliExtPrintf(pCliEnv,
        "-------------------------------------------------------\r\n");
    return ;
}


/************************************************************************
函数名: MOD_msgqInfoShow
功能：显示所有的tid 信息
输入：
输出：  N/A
返回：
说明：msgqshow 的命令解析函数
************************************************************************/
XVOID MOD_msgqInfoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    t_TIDCB* pTidCb;
    t_XOSMSGQ *pMsgQ;

    XS32 tidIndex;
    XS32 maxMsgs;
    XS32 maxUsage;
    XS32 curUsage;
    XS32 j;

    /*msgq list show  */
    XOS_CliExtPrintf(pCliEnv,
        "queue list \r\n-------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-18s%-10s%-10s%-10s\r\n",
        "index",
        "tidName",
        "maxMsgs",
        "maxUsage",
        "curUsage"
        );

    tidIndex = XOS_ArrayGetFirstPos(g_modMnt.tidArray);
    j = 0;
    while(tidIndex >= 0)
    {
        pTidCb = (t_TIDCB*)XNULLP;
        pTidCb = (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
        if(pTidCb == XNULLP ||pTidCb->recvMsgQue.queueList == XNULLP)
        {
            tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
            continue;
        }
        pMsgQ = &(pTidCb->recvMsgQue);
        maxMsgs = XOS_listMaxSize(pMsgQ->queueList);
        maxUsage = XOS_listMaxUsage(pMsgQ->queueList);
        curUsage = XOS_listCurSize(pMsgQ->queueList);

        XOS_CliExtPrintf(pCliEnv,
            "%-6d%-18s%-10d%-10d%-10d\r\n",
            j,
            pTidCb->tskName,
            (maxMsgs == -1)? 0:pTidCb->maxMsgsInQue,
            (maxUsage == -1)? 0:maxUsage-(XS32)eMAXPrio-1,
            (curUsage == -1)? 0:curUsage-(XS32)eMAXPrio-1
            );
        tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
        j++;
    }
    /*end of msgq list */
    XOS_CliExtPrintf(pCliEnv,
        "-------------------------------------------------------\r\n");
    return ;
}


/**********************************
函数名称  : fid_msgq
作者    : liu.da
设计日期  : 2007年12月01日
功能描述  : 增加线程队列的串口函数
参数    : void
返回值    : XVOID
************************************/
XPUBLIC XS32 msgqshow()
{
    t_TIDCB* pTidCb;
    t_XOSMSGQ *pMsgQ;

    XS32 tidIndex;
    XS32 maxMsgs;
    XS32 maxUsage;
    XS32 curUsage;
    XS32 j;
    /*msgq list show  */
    printf(
        "queue list \r\n-------------------------------------------------------\r\n");
    printf(
        "%-6s%-18s%-10s%-10s%-10s\r\n",
        "index",
        "tidName",
        "maxMsgs",
        "maxUsage",
        "curUsage"
        );

    tidIndex = XOS_ArrayGetFirstPos(g_modMnt.tidArray);
    j = 0;
    while(tidIndex >= 0)
    {
        pTidCb = (t_TIDCB*)XNULLP;
        pTidCb = (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
        if(pTidCb == XNULLP ||pTidCb->recvMsgQue.queueList == XNULLP)
        {
            tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
            continue;
        }
        pMsgQ = &(pTidCb->recvMsgQue);
        maxMsgs = XOS_listMaxSize(pMsgQ->queueList);
        maxUsage = XOS_listMaxUsage(pMsgQ->queueList);
        curUsage = XOS_listCurSize(pMsgQ->queueList);

        printf(
            "%-6d%-18s%-10d%-10d%-10d\r\n",
            j,
            pTidCb->tskName,
            (maxMsgs == -1)? 0:pTidCb->maxMsgsInQue,
            (maxUsage == -1)? 0:maxUsage-(XS32)eMAXPrio-1,
            (curUsage == -1)? 0:curUsage-(XS32)eMAXPrio-1
            );
        tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
        j++;
    }
    /*end of msgq list */
    return XSUCC;
}


XPUBLIC XS32 msgq_fidshow(int i_FID,int i_ShowCount)
{
    /*获取接收任务tid 的信息*/
    t_XOSMSGQ fid_msgq;
    t_TIDCB *pTidCb = (t_TIDCB*)XNULLP;
    XS32 listIndex;
    XU32 inum = 0;
    XU32 msg_count=i_ShowCount;
    t_QUEELEM*pQueElem;
    XCHAR  fid_name[32]={0};
    t_XOSCOMMHEAD *ptMsg;
    XCHAR* fidName = XNULL;
    
    if(i_FID <= 0)
    {
        printf("msgq_fidshow,usage:msgq_fidshow 4.\r\n");
        return XERROR;
    }
    if(msg_count < 10 )
    {
        msg_count =10;
    }

    /*得到模块名称*/
    fidName = XOS_getFidName(i_FID);
    if ( XNULLP !=  fidName)
    {
        XOS_Sprintf(fid_name,32,"%s", fidName);
    }else
    {
        XOS_Sprintf(fid_name,32,"Unknown_FID[%d]",i_FID);
    }

    pTidCb = MOD_getTidCbByFid(i_FID);
    if(pTidCb == XNULLP)
    {
        printf("msgq_fidshow,get Tid cb by fid[%d] failed.\r\n",i_FID);
        return XERROR;
    }

    if(pTidCb->recvMsgQue.queueList == XNULLP)
    {
        printf("msgq_fidshow,fid[%s] msgq current is null.\r\n",fid_name);
        return XERROR;
    }

    /*msgq list show  */
    fid_msgq=pTidCb->recvMsgQue;
    printf("fid %d queue list current has %d msgs.\r\n",
        fid_msgq.tskIndex,(XOS_listCurSize(fid_msgq.queueList)-eMAXPrio-1));

    printf("%-10s%-18s%-18s%-10s%-10s\r\n","No","srcfid","destfid","msgID","prio");
    listIndex = XOS_listHead(fid_msgq.queueList);
    while(listIndex != XERROR)
    {
        pQueElem = (t_QUEELEM*)XNULLP;
        pQueElem = (t_QUEELEM*)XOS_listGetElem(fid_msgq.queueList, listIndex);
        if ((pQueElem != XNULLP) && ((XPOINT)(pQueElem->pMsg) >  eMAXPrio)) //why?
        {
            ptMsg = (t_XOSCOMMHEAD*)(pQueElem->pMsg);
            printf( "%-10d%-18d%-18d%-10d%-10d\r\n",
                inum++,
                ptMsg->datasrc.FID,
                ptMsg->datadest.FID,
                ptMsg->msgID,
                ptMsg->prio
                );
        }
        listIndex =  XOS_listNext(fid_msgq.queueList,listIndex);
        if(inum >msg_count)
        {
            printf("msgq_fidshow,recent %d msg display finished.\r\n", msg_count);
            break;
        }
    }
    printf(  "------------------------------------------\r\n");

    return XSUCC;
}


/************************************************************************
函数名: MOD_mqPrtCtrl
功能：消息队列的消息打印控制开关.
输入：
输出：  N/A
返回：
说明：MOD_mqPrtCtrl 的命令解析函数
************************************************************************/
XVOID MOD_mqPrtCtrl(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 i =0;
    XBOOL ctrl = XFALSE ;

    if ( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
        return;
    }

    ctrl = (XBOOL )atol(ppArgv[1]);

    for(i=FID_XOSMIN;i <FID_MAX; i++)
        MsgPflg[i] = ctrl;
}


#ifdef XOS_MDLMGT
XPUBLIC XS32 XOS_MMStartFid(t_XOSLOGINLIST *LoginInfo,XVOID *Para1,XVOID *Para2)
{
    t_FIDCB fidCb;
    t_FIDCB* pFidCb;
    t_TIDCB tidCb;
    t_TIDCB *pTidCb;
    t_XOSLOGINLIST pLog;
    XS32 tidIndex;
    XS32 CurModuleId = (XS32)XosMMGetCurrentLoadModule();
    XVOID *pLocation;
    XS32 ret;
    XU32 fid;
    XS32 i = 0;

    if(XNULLP == LoginInfo)
    {
        return XERROR;
    }
    if(XNULLP == LoginInfo->stack)
    {
        return XERROR;
    }
    XOS_MemCpy(&pLog,LoginInfo,sizeof(t_XOSLOGINLIST));

    XOS_MemSet(&fidCb, 0, sizeof(t_FIDCB));
    fid = pLog.stack->head.FID;
    fidCb.attSwitch = XFALSE;
    XOS_MemCpy(fidCb.fidName, pLog.stack->head.FIDName, MAX_FID_NAME_LEN);
    fidCb.fidNum = fid;
    fidCb.initFunc = pLog.stack->init.init;
    fidCb.noticeFunc = pLog.stack->init.notice;
    fidCb.MsgFunc = pLog.stack->handle.message;
    fidCb.timerFunc = pLog.stack->handle.timer;
    fidCb.pidIndex  = XOS_GetLocalPID();
    fidCb.msgFreeType = pLog.stack->memfreetype;
    fidCb.attSwitch = XFALSE;
    fidCb.noticeFlag = XFALSE;
    fidCb.logLevel = PL_ERR;
    /*启动加载模块管理时，初始化各个模块的trace输出标志,modified (20061108)*/
    initFidTraceInfo((&(fidCb.traceInfo))); /*初始化trace 的信息*/
    if(gMMFidTraceLel != 0xffffffff && gMMFidTraceLel < PL_MAX)
    {
        fidCb.traceInfo.traceLevel = gMMFidTraceLel;
        for(i = 0;i < kCLI_MAX_CLI_TASK;i++)
        {
            fidCb.traceInfo.sessionLevel[i] = gMMFidTraceLel;
        }
    }
    
    pLocation = XNULLP;

    /*添加到hash表*/
    pLocation = XOS_HashElemAdd(g_modMnt.fidHash, (XVOID*)&fid, (XVOID*)&fidCb, XFALSE);
    if(pLocation == XNULLP)
    {
        return XERROR;
    }
    pFidCb = (t_FIDCB*)XNULLP;
    pFidCb = XOS_HashGetElem(g_modMnt.fidHash, pLocation);
    if(pFidCb == XNULLP)
    {
        return XERROR;
    }
    tidIndex = XOS_ArrayFind(g_modMnt.tidArray, (XVOID*)(XPOINT)pLog.TID);
    /*找到任务，任务已经起来*/
    if( tidIndex >= 0 || pLog.TID == 0 )
    {
        pFidCb->tidIndex = tidIndex;
    }
    else  /*没有找到，需要起任务*/
    {
        XOS_MemSet(&tidCb, 0, sizeof(t_TIDCB));
        tidCb.prio = pLog.prio;
        tidCb.stackSize = pLog.stacksize;
        tidCb.tidNum = pLog.TID;
        XOS_MemCpy(&(tidCb.tskName), pLog.taskname, MAX_TID_NAME_LEN);

        /* 当配置文件中有配置时，重置模块消息队列的大小为从配置文件中读取的值 */
        if (0 != gMMModuleList[CurModuleId].wsModInitPara.nMsgQueNum)
        {
            pLog.quenum = gMMModuleList[CurModuleId].wsModInitPara.nMsgQueNum;
        }
        
        /*windows下没有配置，或者配置较小的情况下*/
        if ( MIN_MSGS_IN_QUE  > pLog.quenum   || MAX_MSGSNUM_IN_QUE <= pLog.quenum)
        {
            tidCb.maxMsgsInQue = MAX_MSGS_IN_QUE;
        }
        else
        {
            tidCb.maxMsgsInQue =pLog.quenum;
        }

        tidIndex = XOS_ArrayAdd(g_modMnt.tidArray, (XOS_ArrayElement)&tidCb);
        if(tidIndex < 0)
        {
            return XERROR;
        }
        pFidCb->tidIndex = tidIndex;

        pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
        if(pTidCb == (t_TIDCB*)XNULLP)
        {
            return XSUCC;
        }
        /*xos 中,定时器任务任务入口函数比较特殊*/
        if (pTidCb->tidNum== FID_TIME)
        {
            ret = fidCb.initFunc(Para1, Para2);
            if(XSUCC != ret)
            {
                MMErr("FID_TIME:XOS_MMStartFid()-> Timer initFunc is wrong!");
                return XERROR;
            }
            
#ifndef XOS_NEED_OLDTIMER        
            /*定时器先执行初始化函数*/
            
            /*创建定时器工作处理任务*/
            ret = XOS_TaskCreate_Ex((pTidCb->tskName), pTidCb->prio, pTidCb->stackSize,
                (os_taskfunc)tm_excuteLowProc, XNULLP, &(pTidCb->tskId),
                 &gMMModuleList[CurModuleId].module_cpu_bind);
                
                
            if(XSUCC != ret)
            {
                MMErr("FID_TIME:XOS_MMStartFid()-> can not create task[tsk_dealtimer]!");
                return XERROR;
            }
#else
             /*低精度定时器*/
            ret = XOS_TaskCreate_Ex((pTidCb->tskName), pTidCb->prio, pTidCb->stackSize,
                (os_taskfunc)Low_TimerTask, XNULLP, &(pTidCb->tskId),
                &gMMModuleList[CurModuleId].module_cpu_bind);
            if(XSUCC != ret)
            {
                XOS_Trace(MD(FID_TIME,PL_ERR),"XOS_MMStartFid()-> TIME is wrong!");
                return XERROR;
            }
#ifdef XOS_HIGHTIMER
            /*高精度定时器任务*/
            ret = XOS_TaskCreate_Ex("tsk_HTIM", pTidCb->prio, pTidCb->stackSize,
                (os_taskfunc)High_TimerTask, XNULLP, &(pTidCb->tskId),             
                &gMMModuleList[CurModuleId].module_cpu_bind);
            if(XSUCC != ret)
            {
                XOS_Trace(MD(FID_TIME,PL_ERR),"XOS_MMStartFid()-> HIGHTIME is wrong!");
                return XERROR;
            }
#endif
            return XSUCC;
#endif            
        }
        else if (fidCb.MsgFunc == NULL && fidCb.timerFunc == NULL)
        {
            pTidCb->recvMsgQue.init = XFALSE;
            ret = XSUCC;
        }
        else
        {
            ret = QUE_MsgQCreate(&(pTidCb->recvMsgQue), pTidCb->maxMsgsInQue);
            if(ret != XSUCC)
            {
                pTidCb->recvMsgQue.init = XFALSE;
                XOS_Trace(MD(FID_ROOT, PL_ERR),
                    "XOS_MMStartFid()->create msg que failed ! tid: %d, tskName: %s",
                    pTidCb->tidNum, pTidCb->tskName);
                return XERROR;
            }
            pTidCb->recvMsgQue.init = XTRUE;
            pTidCb->recvMsgQue.tskIndex = pTidCb->tidNum;
            ret = XOS_TaskCreate_Ex((pTidCb->tskName), pTidCb->prio, pTidCb->stackSize,
                (os_taskfunc)MOD_allTaskEntry, (XVOID*)pTidCb, &(pTidCb->tskId),            
                &gMMModuleList[CurModuleId].module_cpu_bind);
        }

        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR),
                "XOS_MMStartFid()->create task failed ! tid: %d, tskName: %s",
                pTidCb->tidNum, pTidCb->tskName);
            return XERROR;
        }
    }

    /*执行初始化函数*/
    if(XNULLP == fidCb.initFunc || fidCb.fidNum == FID_TIME)
    {
        XOS_Trace(MD(FID_ROOT, PL_WARN),
            "XOS_MMStartFid()->initFunc is NULLP !");
        
        return XSUCC;
    }

    ret = fidCb.initFunc(Para1, Para2);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_ROOT, PL_WARN),
            "XOS_MMStartFid()->excute fid[%s] init func failed!",
            fidCb.fidName);
        return XERROR;
    }

    return XSUCC;
}
#endif /* XOS_MDLMGT */


/************************************************************************
函数名: TRC_CMDShow
功能：显示所有的fid 的trace输出标志
显示格式为:

------------------------------------------
fid  fidname   traclevel  filename   linenum   timeflag   trstotskflag   outmode
------------------------------------------

输入：
输出：  N/A
返回：  XSUCC OR XERROR
说明：fidshow 的命令解析函数
************************************************************************/
XVOID  TRC_CMDShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
#if 1 /* wulei del for B07 2006.11.20 */
    t_FIDSHOW fidShow;

    /*fid list show  */
    XOS_CliExtPrintf(pCliEnv,
        "fid list \r\n---------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-18s%-8s%-8s%-8s%-8s%-8s%-18s\r\n",
        "fid",
        "fidName",
        "trclev",
        "fnF",
        "lnumF",
        "timeF",
        "totaskF",
        "outmode"
        );
    XOS_MemSet(&fidShow, 0, sizeof(t_FIDSHOW));
    fidShow.pCliEnv = pCliEnv;
    fidShow.index = 0;
    /*遍历hash*/
    XOS_HashWalk(g_modMnt.fidHash, MOD_showFidTraceInfo, &fidShow);

    /*end of fid list */
    XOS_CliExtPrintf(pCliEnv,
        "---------------------------------------------------------------------------\r\n");

    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n" , fidShow.index);
#endif

    return;
}

RESET_PROC_FUNC pRestProcFunc = NULL;
/************************************************************************
函数名: XOS_ResetProcFuncReg
功能：  注册回调函数，用于系统重启前进行善后处理
输入：  ulFuncAddr 欲注册的函数
输出：
返回：
说明：
************************************************************************/
XS32 XOS_ResetProcFuncReg(void * ulFuncAddr)
{
     pRestProcFunc = (RESET_PROC_FUNC)ulFuncAddr;
     return XSUCC;
}


extern XPUBLIC XS32 LOG_TskStackInfo(t_XOSTASKID *tskID,XCHAR *pReason);
/************************************************************************
函数名: XOS_ResetByQueFull
功能：  消息队列满时重启平台
输入：  tidNum 消息队列满的任务号
        exitFlag 0:平台调用业务注册的回调函数；1:平台退出进程
输出：
返回：
说明：
************************************************************************/
#define STACK_HEAD_LEN 256
XPUBLIC XVOID XOS_ResetByQueFull(XS32 tidNum,XS32 exitFlag)
{
    XCHAR buf[STACK_HEAD_LEN] = {0};
    t_TIDCB *pTidCb = (t_TIDCB *)XNULLP;

    pTidCb = MOD_getTidCbByTidNum(tidNum);
    if(pTidCb == XNULLP)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR),
            "MOD_getTidCbByTidNum()-> get tidCB fail! ");
        return ;
    }
    
    XOS_Sprintf(buf, STACK_HEAD_LEN, 
        "task(tskid = 0x%x(%d), tskname = %s) recvmsgQue is full!\r\n", 
        pTidCb->tskId, (XS32)pTidCb->tskId, pTidCb->tskName);
    LOG_TskStackInfo(&pTidCb->tskId,buf);

    /*不再强制杀掉接收线程*/
#if 0
    XOS_TaskDel(pTidCb->tskId);
#endif

    if(exitFlag == 0)
    {
        if(NULL != pRestProcFunc)
        {
            XOS_Sprintf(buf, STACK_HEAD_LEN, 
            "reboot: task(tskid = 0x%x(%d), tskname = %s) recvmsgQue is full,system will reboot!!\r\n", 
            pTidCb->tskId, (XS32)pTidCb->tskId, pTidCb->tskName);

            write_to_syslog("task [%s] msg queue exceed 90% and call callback function!!!",pTidCb->tskName);
            pRestProcFunc();
        }
        else
        {
            /*平台或业务未注册，则默认不处理，直到95%退出*/
        }
    }
#ifdef XOS_VXWORKS
    else
    {
        /*vx平台不做处理，需要HA来退出进程并重启*/
    }
#else
    else
    {
        printf("task [%s] msg queue exceed 95 percent and exit...\n",pTidCb->tskName);
        write_to_syslog("task [%s] msg queue exceed 95 percent and exit!!!",pTidCb->tskName);
        _exit(0);
    }
#endif
/*此调用修改为由应用层根据具体需求在pRestProcFunc中选择调用*/
#if 0
    XOS_Reset(1);
#endif
    return;
}
/*
XS32 trace_msgq_size(const XS8* msg,...)
{
    XCHAR msg_format[1024]  = {0}; 
    XU32  msg_curlen  = 0;
#ifdef XOS_VXWORKS        
    XS32  len = 0;
    XS8*  ptr = NULL;
    FILE * fpw = NULL;
#endif
    va_list ap;
    
    
    va_start(ap, msg);
    msg_curlen = XOS_VsPrintf(msg_format, sizeof(msg_format)-1, msg, ap);
    va_end(ap);
    if (msg_curlen <= 0) {
        return XERROR;
    }
#ifdef XOS_VXWORKS    
    fpw = fopen( "/ata0a/syslog.txt", "a+" );
    if(fpw != NULL) {
        ptr = msg_format;
        while(msg_curlen > 0) {
            len = fwrite(ptr, 1, msg_curlen, fpw);
            msg_curlen -= len;
            ptr += len;
        }
        fclose(fpw);
        return XSUCC;
    }
#endif    
    return XERROR;
}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */


