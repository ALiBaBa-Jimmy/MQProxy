/**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
 **
 **  Core Network Department  platform team
 **
 **  filename: timer.c
 **
 **  description:
 **
 **  author: chenwanli
 **
 **  date:   2006.3.28
 **
 ***************************************************************
 **                          history
 **
 ***************************************************************
 **   author          date              modification
 **   chenwanli       2006.3.28             create
 **   wangzongyou     2006.7.25           修改适应模块管理      
 **************************************************************/
#ifndef XOS_NEED_OLDTIMER

#include "xoscfg.h"
#include "xosencap.h"
#include "cmtimer.h"
#include "xosmodule.h"
#include "xostrace.h"
#include "xosmem.h"
#include "xospub.h"
#include "clishell.h"
#include "xmlparser.h"
#ifdef XOS_WIN32
#pragma comment(lib,"winmm.lib")
#endif

/*
 * per-CPU timer vector definitions:
 */
#define TVN_BITS (6)
#define TVR_BITS (8)
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)     /*2,3,4,5层转盘刻度数量*/
#define TVR_MASK (TVR_SIZE - 1)     /*最里层转盘刻度数量*/

#define TIMER_MIN_CN        (1000)     /*最小定时器数量*/
#define TIMER_MAX_CN        (0x1ffff) /*最大定时器数量*/

XU32 g_TimerMaxCnt = TIMER_MAX_CN;    /*最大定时器数量*/

XU32 *g_pTimerFidInfo = NULL;    /*用于统计各个模块当前使用的定时器的数量*/

#define TIMER_MAGIC    0x4b87ad6e
#define TIMER_INITIALIZER(timer, cpu) { \
        timer->expires  = 0x00;         \
        XOS_MemSet(&timer->data, 0x00, sizeof(timer->data));         \
        timer->base     = cpu;            \
        timer->magic    = TIMER_MAGIC;  \
    }
    
/**
 * list_entry - get the struct for this entry
 * @ptr:    the &struct list_head pointer.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)
    
/**
 * container_of - cast a member of a structure out to the containing structure
 *
 * @ptr:    the pointer to the member.
 * @type:    the type of the container struct this is embedded in.
 * @member:    the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({            \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
        
#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define INDEX(N) ((base->timer_jiffies >> (TVR_BITS + N * TVN_BITS)) & TVN_MASK)

#define LIST_HEAD_INIT(name) { &(name), &(name) }


typedef struct tvec_s 
{
    sys_list_st vec[TVN_SIZE];
} tvec_t;

typedef struct tvec_root_s 
{
    sys_list_st vec[TVR_SIZE];
} tvec_root_t;
/*
 * 时间刻度定义
 */
typedef struct tvec_scale_s
{
    XU32 tick_per_sec;     //每秒ticks    
    XU32 scale_jiffies;    //时刻指针,每跳动一时刻，指针加1
}tvec_scale_s;

//work工作队列，用来专门发送消息
typedef struct tvec_work_s
{
    t_XOSMUTEXID lock;       //执行队列锁
    t_XOSSEMID   sem;        //工作队列的信号量
    sys_list_st work_list;   //执行队列
}tvec_work_s;

typedef struct tvec_t_base_s 
{
    t_XOSMUTEXID lock;
    t_XOSSEMID sem;    //刻度信号量
    XU32 timer_jiffies;    //时针,不同精度队列时针跳动不同
    XU32 bActive:1;
    XU32 usedCn:31;/*记录定时器运行链表数*/
    XU32 idelCn;/*记录当前空闲链表数*/
    sys_list_st   idleList;         /* 空闲数据装载列表         */
    struct timer_list *idle_timer;
    tvec_work_s tv_work_list;
    tvec_root_t tv1;
    tvec_t tv2;
    tvec_t tv3;
    tvec_t tv4;
    tvec_t tv5;
} tvec_base_t;

//typedef struct tvec_t_base_s tvec_base_t;

struct timer_list 
{
    sys_list_st entry;
    XU32 expires;
    XU32 magic;
    t_TIMERNODE data;
    struct tvec_t_base_s *base;
};

/************************************************************************
函数名: tm_tickProc
功能：  系统级定时器超时回调函数
输入：  
输出：  
返回：  
说明：
************************************************************************/
#ifdef WIN32
void PASCAL tm_tickProc(XU32 wTimerID, XU32 msg, XU32 dwUser, XU32 dwl, XU32 dw2);
#endif
#if defined LINUX || defined VXWORKS
void tm_tickProc();
#endif

/*
 * 运行定时器函数，
 *
 */
void tm_Run(tvec_base_t *base);
/*
 * 执行属于可执行的定时器队列
 */
XSTATIC XVOID XOS_ExecWorkList(void *);

/*
 * 初始化高精度和低精度任务队列
 *
 */
XSTATIC XS32 Init_TimerTask(tvec_base_t* baseCpu);


XEXTERN XVOID XOS_TimerSet(XBOOL openflag);

#ifdef XOS_VXWORKS
WDOG_ID g_wdId = 0x00;//VXWORKS 下定时看门狗ID
#endif

tvec_scale_s g_scale;   //时钟
tvec_base_t g_preHighCpu;  //高精度队列
tvec_base_t g_preLowCpu;  //低精度队列

/************************************************************************
 函数名:TIM_buildHandle
 功能: 构造定时器句柄
 输入: linkType － 定时器类型(高四位)
       linkIndex －定时器索引
 输出:
 返回:  返回定时器句柄
 说明:  定时器句柄组织如图，

            pre     checkcoder                  Index
          32     28          20                     1
           -----------------------------------------
           |     |           |                      |
           |     |           |                      |
           -----------------------------------------

************************************************************************/
XSTATIC PTIMER  TIM_buildHandle(e_TIMERPRE pre, XU32 Index)
{
     /*LINK_CHECK_NUM, 为了躲开vc中不初始化的局部变量总为0xcccc 的情况*/
     PTIMER  timerhandle;
     XU32 checkNum = TIMER_CHECK_NUM; /*0xbb*/
     timerhandle = (PTIMER)(XPOINT)(((pre&0xf)<<28) |(checkNum<<20) | ((Index&0xfffff)));

     return timerhandle;
}

/************************************************************************
 函数名:TIM_buildDHandle
 功能: 构造定时器句柄
 输入: linkType － 定时器类型(高四位)
       linkIndex －定时器索引
 输出:
 返回:  返回定时器句柄
 说明:  定时器句柄组织如图，

            pre     checkcoder                  Index
          32     28          20                     1
           -----------------------------------------
           |     |           |                      |
           |     |           |                      |
           -----------------------------------------

************************************************************************/
XSTATIC PTIMER  TIM_buildDHandle(e_TIMERPRE pre, XU32 Index)
{
     /*LINK_CHECK_NUM, 为了躲开vc中不初始化的局部变量总为0xcccc 的情况*/
     PTIMER  timerhandle;
     XU32 checkNum = TIMER_CHECK_DNUM; /*0xDD*/
     timerhandle = (PTIMER)(XPOINT)(((pre&0xf)<<28) |(checkNum<<20) | ((Index&0xfffff)));

     return timerhandle;
}

/************************************************************************
 函数名:TIM_isValidTHdle
 功能: 验证定时器句柄的有效性
 输入: 定时器句柄
 输出:
 返回: 有效返回XTURE, 否则返回XFALSE
 说明:
************************************************************************/
XSTATIC  XBOOL TIM_isValidTHdle( PTIMER timerhandle)
{
     return (TIMER_CHECK_NUM == (((XPOINT)timerhandle>>20)&0xff)? XTRUE:XFALSE);
}

/************************************************************************
 函数名:TIM_isValidDTHdle
 功能: 验证定时器句柄的有效性
 输入: 定时器句柄
 输出:
 返回: 有效返回XTURE, 否则返回XFALSE
 说明:
************************************************************************/
XSTATIC  XBOOL TIM_isValidDTHdle( PTIMER timerhandle)
{
     return (TIMER_CHECK_DNUM == (((XPOINT)timerhandle>>20)&0xff)? XTRUE:XFALSE);
}

/************************************************************************
 函数名:TIM_getTimerIndex
 功能: 通过定时器句柄获取定时器Index
 输入: 定时器句柄
 输出:
 返回: 定时器索引
 说明:
************************************************************************/
XSTATIC  XS32 TIM_getTimerIndex( PTIMER timerhandle)
{
     return (XS32)((XPOINT)timerhandle&0x0fffff);
}

/************************************************************************
 函数名:TIM_getTimerPre
 功能: 通过定时器句柄获取定时器精度
 输入: 定时器句柄
 输出:
 返回: 定时器精度(高精度或低精度)
 说明:
************************************************************************/
XSTATIC  e_TIMERPRE TIM_getTimerPre( PTIMER timerhandle)
{
     return (e_TIMERPRE)((XPOINT)timerhandle>>28);
}


/************************************************************************
函数名  : TIM_XmlReadCfg
功能    : get XOS timer configure informations
输入    : filename   XOS 配置文件名
输出    :
返回    : XS32
说明：    读取配置文件失败返回XERROR 成功返回XSUCC
************************************************************************/
static XS32 TIM_XmlReadCfg(XU32 *pTimerCnt, XCHAR * filename)
{
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    XU32  CpsTempVal = TIMER_MAX_CN/2;
    XU32  OtherTempVal = TIMER_MAX_CN/2;

    /*规则参数*/
    if(!pTimerCnt || !filename)
    {
        return XERROR;
    }

    *pTimerCnt = 0;

    doc = xmlParseFile(filename);
    if (doc == XNULL)
    {
        return (XERROR);
    }
    
    /*找根节点*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XERROR ;
    }
    /*根节点*/
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XERROR ;
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XERROR ;
    }
    /*TIMER主节点*/
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "TIMER" ) )
        {
            break;
        }
        cur = cur->next;
    }
    
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XERROR ;
    }
    
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XERROR ;
    }
    
    /*遍历TIMER子节点*/
    while ( cur != XNULL )
    {
        /*timer最大数量*/
        if ( !XOS_StrCmp(cur->name, "CPS_MAXNUM" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                CpsTempVal = atoi(pTempStr);

                if (CpsTempVal < TIMER_MIN_CN)
                {
                    CpsTempVal = TIMER_MIN_CN;
                }
                else if (CpsTempVal > TIMER_MAX_CN)
                {
                    CpsTempVal = TIMER_MAX_CN;
                }
  
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            } 
            else
            {
                CpsTempVal = TIMER_MAX_CN/2;
            }
        }
        else if (!XOS_StrCmp(cur->name, "OTHER_MAXNUM" ))
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                OtherTempVal = atoi(pTempStr);

                if (OtherTempVal < TIMER_MIN_CN)
                {
                    OtherTempVal = TIMER_MIN_CN;
                }
                else if (OtherTempVal > TIMER_MAX_CN)
                {
                    OtherTempVal = TIMER_MAX_CN;
                }
  
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }              
            else
            {
                OtherTempVal = TIMER_MAX_CN/2;
            }
        }
        
        cur = cur->next;
    }

    *pTimerCnt = OtherTempVal+CpsTempVal;
    
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XSUCC; 
}


/************************************************************************
函数名: tm_internalAdd
功能：把时间对象加入到对应定时器队列中
输入：
输出：  N/A
返回：
说明：
************************************************************************/
static void tm_internalAdd(tvec_base_t *base, struct timer_list *timer)
{
    XU32 expires = 0;
    XU32 idx = 0;
    sys_list_st *vec = NULL;

    if(NULL == base || NULL == timer)
    {
        return ;
    }
    
    /*通过相对节拍数，计算挂接的链表*/
    expires = timer->expires;
    idx = expires - base->timer_jiffies;    

    if (idx < TVR_SIZE)  /*2^8*/
    {
        XS32 i = expires & TVR_MASK;
        vec = base->tv1.vec + i;
    } 
    else if (idx < 1 << (TVR_BITS + TVN_BITS)) /*2^14*/ 
    {
        XS32 i = (expires >> TVR_BITS) & TVN_MASK;
        vec = base->tv2.vec + i;
    } 
    else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) /*2^20*/
    {
        XS32 i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
        vec = base->tv3.vec + i;
    } 
    else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) /*2^26*/
    {
        XS32 i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
        vec = base->tv4.vec + i;
    } 
    else if ((signed long) idx < 0) 
    {
        /*
         * Can happen if you add a timer with expires == jiffies,
         * or you set a timer to go off in the past
         */
        vec = base->tv1.vec + (base->timer_jiffies & TVR_MASK);
    } 
    else /*2^32*/
    {
        XS32 i;
        /* If the timeout is larger than 0xffffffff on 64-bit
         * architectures then we use the maximum timeout:
         */
        if (idx > 0xffffffffUL) 
        {
            idx = 0xffffffffUL;
            expires = idx + base->timer_jiffies;
        }
        i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
        vec = base->tv5.vec + i;
    }
    /*
     * Timers are FIFO:
     */

    /*此处存在可重入安全问题，调用者需要保证可重入*/
    sys_listAdd(vec->prev, &timer->entry);

}
/************************************************************************
函数名: tm_cascade
功能：根据时间刻度把相应的定时器放入到执行队列中
输入: index -指定刻度盘的刻度值
输出：  N/A
返回：
说明：
************************************************************************/
static XS32 tm_cascade(tvec_base_t *base, tvec_t *tv, XS32 index)
{
    /* cascade all the timers from tv up one level */
    sys_list_st *head, *curr;
    struct timer_list* tl;

    head = tv->vec + index;
    curr = head->next;
    /*
     * We are removing _all_ timers from the list, so we don't  have to
     * detach them individually, just clear the list afterwards.
     */
    while (curr != head) 
    {
        struct timer_list *tmp;

//      tmp = list_entry(curr, struct timer_list, entry);
        tl = (struct timer_list* )curr;
        curr = curr->next;
        sys_listDel(&tl->entry);
        sys_listInit(&tl->entry);
        tmp = (struct timer_list* )&tl->entry;
        
        //根据时间刻度 重新加入到tv1中
        tm_internalAdd(base, tmp);
    }
//    INIT_LIST_HEAD(head);

    return index;
}
/************************************************************************
函数名: XOS_CliGetTimerInfo
功能：统计各个模块使用定时器的数量
输入：
输出：  N/A
返回：
说明：MAX_FID_NUMBER值由最大Fid值，如果后续模块使用Fid值大于2000，需要修改此宏
************************************************************************/
XVOID XOS_CliGetTimerInfo(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    int i;
    XOS_CpsTrace(MD(FID_TIME, PL_INFO), "XOS_CliGetTimerInfo test");
    XOS_CliExtPrintf(pCliEnv,
        "timer info list \r\n-----------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%\r\n",
        "fid",
        "timerNumber");

    for(i = 0;i < MAX_FID_NUMBER; i++)
    {
        if(g_pTimerFidInfo[i] > 0)
        {
            XOS_CliExtPrintf(pCliEnv,"%d    %d\r\n",i,g_pTimerFidInfo[i]);
        }
    }

}

/************************************************************************
函数名: linux_tm_tickProc
功能：  linux下的时钟驱动任务
输入：  
输出：  
返回：  
说明：每隔10ms发送一个驱动信号量
************************************************************************/
XPUBLIC XVOID linux_tm_tickProc( XVOID* ptr)
{

#if(defined(XOS_SOLARIS) ||defined(XOS_LINUX))
    struct timeval t1,t2,t3;
    gettimeofday(&t1, NULL);
#endif

    while(1)
    {

#if(defined(XOS_SOLARIS) ||defined(XOS_LINUX))
        gettimeofday(&t2, NULL);

        t1.tv_usec += 10000;  /*10ms间隔*/
        /* 1s = 1000000us */
        if (t1.tv_usec > 1000000)
        {
            t1.tv_sec++;
            t1.tv_usec -= 1000000;
        }

        t3.tv_sec = 0;
        /*每次这个线程调度的时间差不超过10ms，否则高精度定时器将会不准*/
        t3.tv_usec = ((t1.tv_usec > t2.tv_usec)? t1.tv_usec - t2.tv_usec
                :t1.tv_usec + 1000000 - t2.tv_usec);
        select(1, NULL, NULL, NULL, &t3);
#endif
      /*发送信号量*/
       if(g_scale.scale_jiffies % (LOC_HIGHTIMER_CLCUNIT / TM_SCALE) == 0) 
       {
            XOS_SemPut(&g_preHighCpu.sem);
       }
    
       if(g_scale.scale_jiffies % (LOC_LOWTIMER_CLCUNIT / TM_SCALE) == 0) 
       {
           XOS_SemPut(&g_preLowCpu.sem);
       }    

       g_scale.scale_jiffies++;  //时针加1        
    }
}

/************************************************************************
函数名: tm_tickProc
功能：  时钟跳动函数
输入：  
输出：  
返回：  
说明：
************************************************************************/
#ifdef XOS_WIN32
void PASCAL tm_tickProc(XU32 wTimerID, XU32 msg, XU32 dwUser, XU32 dwl, XU32 dw2)
#else
void tm_tickProc()
#endif
{    
#ifdef XOS_VXWORKS 
    wdStart(g_wdId, TM_SCALE * g_scale.tick_per_sec / 1000, (FUNCPTR)tm_tickProc, 0);
#endif

    if(g_scale.scale_jiffies % (LOC_HIGHTIMER_CLCUNIT / TM_SCALE) == 0) {
        XOS_SemPut(&g_preHighCpu.sem);
    }
    
    if(g_scale.scale_jiffies % (LOC_LOWTIMER_CLCUNIT / TM_SCALE) == 0) {
        XOS_SemPut(&g_preLowCpu.sem);
    }

    g_scale.scale_jiffies++;  //时针加1
}


/************************************************************************
函数名: tm_excuteHighProc
功能：  定时处理任务.
        (主要对运行链表的tick进行-操作，超时向指定模块发送定时消息)
输入：  
输出：  
返回：  
说明：
************************************************************************/
void tm_excuteHighProc()
{
    while(g_preHighCpu.bActive)
    {
        XOS_SemGet(&g_preHighCpu.sem);
        tm_Run(&g_preHighCpu);
    }
}
void tm_excuteLowProc()
{
    while(g_preLowCpu.bActive)
    {
        XOS_SemGet(&g_preLowCpu.sem);
        tm_Run(&g_preLowCpu);
    }
}


/***
 * timer_run - run all expired timers (if any) on this CPU.
 * @base: the timer vector to be processed.
 *
 * This function cascades all vectors and executes all expired timer
 * vectors.
 */
void tm_Run(tvec_base_t *base)
{
    sys_list_st worklst = LIST_HEAD_INIT(worklst);
    sys_list_st *head = &worklst;
    struct timer_list *timer = 0x00;
    struct timer_list *work_timer = 0x00;
    int index = 0;

    if(NULL == base)
    {
        return ;
    }

    /*计算最里面转盘指针*/
    index = base->timer_jiffies & TVR_MASK;

    /*
     * Cascade timers:
     计算刻度
     */
    XOS_MutexLock(&base->lock);
    
    /*从内到外，依次转动转盘指针, index==0,说明最里面的转盘转了一周255次, 只有内层转盘转了一周，才可能导致外
      层转盘需要移动转盘指针*/

    /*外层可能到期的节点挂接到最里层的0号节点链表*/
    if  (!index &&
            (!tm_cascade(base, &base->tv2, INDEX(0))) &&
            (!tm_cascade(base, &base->tv3, INDEX(1))) &&
             !tm_cascade(base, &base->tv4, INDEX(2)))
             tm_cascade(base, &base->tv5, INDEX(3));
    
    /*将到期节点链表移动到工作链表*/
    sys_listSpliceInit(base->tv1.vec + index, &worklst);
    XOS_MutexUnlock(&base->lock);

    //处理tv1队列
    while (!sys_listEmpty(head)) 
    {
        work_timer = NULL;
        timer = (struct timer_list* )head->next;//list_entry(head->next,struct timer_list,entry);
        sys_listDel(&timer->entry);
        
        //XOS_Trace(MD(FID_TIME,PL_ERR),"tm_run: add running timer into worklist");
        //增加到worklist队列
        //从空闲队列中取定时节点

        XOS_MutexLock(&base->lock);
        timer->data.backpara.count++;  /* 记录定时器运行次数 */
        if(&base->idleList == base->idleList.next) 
        {
            XOS_Trace(MD(FID_TIME,PL_ERR),"XOS_TimerStart:timer list is full");
        } 
        else 
        {
            work_timer = (struct timer_list* )base->idleList.next;
            sys_listDel(&work_timer->entry);
            memcpy(work_timer, timer, sizeof(struct timer_list));
            sys_listInit(&work_timer->entry);
            --base->idelCn;
        }
        XOS_MutexUnlock(&base->lock);
        if(work_timer != NULL)
        {
            /*从局部队列中加入到worklist工作队列*/
            XOS_MutexLock(&base->tv_work_list.lock);
            sys_listAdd(base->tv_work_list.work_list.prev, &work_timer->entry);
            XOS_MutexUnlock(&base->tv_work_list.lock);
        }
        
        /*对于循环定时器，重新加入工作队列*/
        XOS_MutexLock(&base->lock);
        if(TIMER_TYPE_LOOP == timer->data.para.mode)
        {
            //根据精度计算位置
            if(timer->data.para.pre == TIMER_PRE_HIGH) {
                timer->expires  = timer->data.para.len / LOC_HIGHTIMER_CLCUNIT + base->timer_jiffies;
            } else {
                timer->expires  = timer->data.para.len / LOC_LOWTIMER_CLCUNIT + base->timer_jiffies;
            }
            tm_internalAdd(base, timer);
        }
        else
        {
            //对于一次定时器，将其加入空闲队列中
            if(XFALSE==timer->data.flag)
            {
                timer->data.tmnodest = TIMER_STATE_NULL;
                sys_listAdd(&base->idleList, &timer->entry);
                ++base->idelCn;

                /*单次定时器在XOS_TimerStop时，对tmnodest判断，计数需要在这里加上*/
                if(g_pTimerFidInfo != NULL && timer->data.para.fid < MAX_FID_NUMBER)
                {
                    g_pTimerFidInfo[timer->data.para.fid]--;
                }
            }
            else
            {
                //四接口模式，必须调用XOS_TimerDelete来释放，并加入空闲队列
                timer->data.tmnodest = TIMER_STATE_FREE;
            }
            --base->usedCn;
        }
        XOS_MutexUnlock(&base->lock);

        
    }
    //向worklist发送信号，通知其执行
    if(!sys_listEmpty(&base->tv_work_list.work_list)) 
    {
        XOS_SemPut(&base->tv_work_list.sem);
    }
    //队列指针加1
    XOS_MutexLock(&base->lock);
    base->timer_jiffies++;
    XOS_MutexUnlock(&base->lock);
}
/************************************************************************
 函数名:TIM_MsgSnd
 功能: 定时器到时，消息发送程序
 输入: t_TIMERNODE 发送的参数
 输出:
 返回: 
 说明:
************************************************************************/
XVOID XOS_ExecWorkList(void * param)
{
    sys_list_st *head = XNULL;
    struct timer_list *timer = 0x00;
    int empty = 0;
    tvec_base_t * baseCpu = NULL;


    if(NULL == param)
    {
        return ;
    }
    
    baseCpu = (tvec_base_t*)param;

    while(baseCpu && baseCpu->bActive) 
    {
        
        XOS_SemGet(&baseCpu->tv_work_list.sem);/*获取信号量*/

        XOS_MutexLock(&baseCpu->tv_work_list.lock);

        head = &(baseCpu->tv_work_list.work_list);
        
        empty = sys_listEmpty(head) ? 1 : 0;
        
        while (!sys_listEmpty(head)) {
            timer = (struct timer_list* )head->next;//list_entry(head->next,struct timer_list,entry);
            sys_listDel(&timer->entry);
            
            TIM_MsgSnd(&timer->data);

            XOS_MutexLock(&baseCpu->lock);
            sys_listAdd(&baseCpu->idleList, &timer->entry);
            ++baseCpu->idelCn;
            XOS_MutexUnlock(&baseCpu->lock);

            //XOS_Trace(MD(FID_TIME,PL_ERR),"XOS_ExecWorkList: pre=%d,expires=%d,len=%d", timer->data.para.pre, timer->expires, timer->data.para.len);
        }

        XOS_MutexUnlock(&baseCpu->tv_work_list.lock);

        XOS_Sleep(empty ? 10 : 0);
    }
}

/************************************************************************
 函数名:TIM_MsgSnd
 功能: 定时器到时，消息发送程序
 输入: t_TIMERNODE 发送的参数
 输出:
 返回: 
 说明:
************************************************************************/
XVOID  TIM_MsgSnd(const t_TIMERNODE *data)
{
    t_XOSCOMMHEAD* ptr_timer_sndmsg = XNULLP;
    t_BACKPARA*   ptr_data = XNULL;
    XU8*          ptr_buff = XNULL;
    XS32 ret = XSUCC;
    XS32 i = 0;
    
    ptr_buff = (XU8*)XOS_MsgMemMalloc( FID_TIME, sizeof(t_BACKPARA));
    if(XNULLP == ptr_buff)
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "Timer Clock message allocation failed.\n");
        return ;
    }
    ptr_timer_sndmsg = (t_XOSCOMMHEAD*)ptr_buff;
    ptr_timer_sndmsg->datasrc.FID  = FID_TIME;
    ptr_timer_sndmsg->datadest.FID = data->para.fid;
    ptr_timer_sndmsg->datasrc.PID  = XOS_GetLocalPID();
    ptr_timer_sndmsg->datadest.PID = XOS_GetLocalPID();

    ptr_timer_sndmsg->prio         = eTimePrio;
    if(data->para.pre == TIMER_PRE_HIGH) {
        ptr_timer_sndmsg->msgID        = eTimeHigClock;
    } else {
        ptr_timer_sndmsg->msgID        = eTimeLowClock;
    }
    ptr_timer_sndmsg->length        = sizeof(t_BACKPARA);
    ptr_timer_sndmsg->message       = ptr_buff + sizeof(t_XOSCOMMHEAD);
    ptr_data = (t_BACKPARA*)(ptr_buff+ sizeof(t_XOSCOMMHEAD));
    XOS_MemSet(ptr_data, 0, sizeof(t_BACKPARA));
    XOS_MemCpy(ptr_data, &(data->backpara), sizeof(t_BACKPARA));
    
    for(i = 0, ret = XSUCC; i < 2; i++) {
        if(XSUCC == (ret = XOS_MsgSend(ptr_timer_sndmsg))) {
            break;
        }
        
        /*
         * 对于高精度定时器，保持优先级重试一次
         * 对于低精度定时器，降低优先级重试一次
         */
        if(data->para.pre == TIMER_PRE_LOW) {
            ptr_timer_sndmsg->prio = eNormalMsgPrio;
        }
        XOS_Sleep(0);
    }
    if(ret == XERROR) 
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "TIM_MsgSnd->XOS_MsgSend() failed");
        XOS_MsgMemFree(FID_TIME, (t_XOSCOMMHEAD*)ptr_buff);
    }
    return;
}

/************************************************************************
函数名  : TIM_ClckProc
功能    : 各任务收到时钟任务消息的统一处理函数
输入    : management - 任务管理定时器链的结构指针
输出    : none
返回    : XSUCC, 函数操作失败返回XERROR
说明    :
************************************************************************/
XS32 TIM_ClckProc(XVOID* ptr_in_msg)
{
    modTimerProcFunc timerExpFunc;
    t_XOSCOMMHEAD* ptr_msg = NULL;

    if(NULL==ptr_in_msg)
    {
        return ERROR;
    }
    ptr_msg = (t_XOSCOMMHEAD*)ptr_in_msg;
    /* 回调相应处理函数 */
    timerExpFunc = MOD_getTimProcFunc(ptr_msg->datadest.FID);
    if(!timerExpFunc )
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "Timer Clock message allocation failed.\n");
        return XERROR;
    }
    timerExpFunc((t_BACKPARA*)ptr_msg->message);
    return XSUCC;
}
/*******************************************************************
函数名:TimerShowAll
功能：显示定时器的统计信息
输入：
输出：
返回：
说明：
*******************************************************************/
XVOID TimerShowAll(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XOS_CliExtPrintf(pCliEnv,"%-12s%-12s%-12s\r\n","MaxListCnt","WorkListCnt","IdelListCnt");
    XOS_CliExtPrintf(pCliEnv,"HighTimer:%-12ld%-12ld%-12ld\r\n",g_TimerMaxCnt,g_preHighCpu.usedCn,g_preHighCpu.idelCn);
    XOS_CliExtPrintf(pCliEnv,"LowTimer:%-12ld%-12ld%-12ld\r\n",g_TimerMaxCnt,g_preLowCpu.usedCn,g_preLowCpu.idelCn);
}

/*******************************************************************
函数名:TimerShowJiff
功能：显示定时器驱动脉冲统计信息
输入：
输出：
返回：
说明：
*******************************************************************/
XVOID TimerShowJiff(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XOS_CliExtPrintf(pCliEnv, "scale_jiffies = %d\r\n", g_scale.scale_jiffies);
    XOS_CliExtPrintf(pCliEnv, "&g_preLowCpu.sem = %0x8\r\n", &g_preLowCpu.sem);

    return;
}

/*******************************************************************
函数名:TimerShowSignal
功能：显示时钟信号当前捕捉函数地址
输入：
输出：
返回：
说明：
*******************************************************************/
XVOID TimerShowSignalStatus(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
#ifdef XOS_LINUX  
    int ret = 0;
    struct itimerval value;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;

    ret = getitimer(ITIMER_REAL, &value);

    XOS_CliExtPrintf(pCliEnv,"[ITIMER_REAL]\r\nvalue.it_interval = %d:%d\r\n", value.it_interval.tv_sec, value.it_interval.tv_usec);

    if(0 != ret)
    {
        XOS_CliExtPrintf(pCliEnv,"---------ITIMER_REAL signal is lose---------------\n");
        return;
    }
    
    return ;
#endif
}


/*******************************************************************
函数名:TimerShowSignal
功能：重置时钟信号捕捉函数
输入：
输出：
返回：
说明：
*******************************************************************/
XVOID TimerSignalReset(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
#if 0
#ifdef XOS_LINUX 
    int sigFunAddr =  signal(SIGALRM, tm_tickProc);
    XOS_CliExtPrintf(pCliEnv,"old signal alarm callback addr = 0x%08x\r\nnew signal alarm callback addr = 0x%08x", 
                         sigFunAddr, tm_tickProc);

    return;
#endif
#endif
}


/*******************************************************************
函数名:TimerShowLowStatus
功能：根据句柄查询低精度定时器节点的当前状态
输入：
输出：
返回：
说明：
*******************************************************************/
XVOID TimerShowLowStatus(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    PTIMER tHandle = 0;
#ifdef XOS_ARCH_64
    XU64 memAddr = 0;
#endif
    struct timer_list *ptimer = NULL;

    if(siArgc >= 2 )
    {
       #ifdef XOS_ARCH_64
       XOS_StrToLongNum(ppArgv[1], &memAddr);
       tHandle = (PTIMER)memAddr;
       #else
       tHandle = (PTIMER)atoi(ppArgv[1]);
       #endif
       if(!TIM_isValidTHdle(tHandle))
       {
            XOS_CliExtPrintf(pCliEnv,"timer handler is unvalid %d", tHandle);
            return ;
       }

        ptimer = g_preLowCpu.idle_timer + TIM_getTimerIndex(tHandle);
        if(NULL != ptimer)
        {
            XOS_CliExtPrintf(pCliEnv,"status   fid    mode     pre     len  entry    next     prev\r\n");
            XOS_CliExtPrintf(pCliEnv,"%d       %d        %d       %d     %d,    %08x,    %08x,    %08x\r\n",
                ptimer->data.tmnodest,
                ptimer->data.para.fid,
                ptimer->data.para.mode,
                ptimer->data.para.pre,
                ptimer->data.para.len,
                &ptimer->entry,
                ptimer->entry.next,
                ptimer->entry.prev
               );
        }
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"usage: timershowlowstatus tHandler");
    }
 }


/*******************************************************************
函数名:TimerCommandInit
功能：注册定时器的相关统计信息查询命令
输入：
输出：
返回：
说明：
*******************************************************************/
XS32 TimerCommandInit(int cmdMode)
{
    int ret=0,reg_result=0;
    ret = XOS_RegistCmdPrompt( cmdMode, "plat", "plat", "no parameter" );
    if ( XERROR >= ret )
    {
        XOS_CliInforPrintf("call TimerCommandInit failed,error num=%d\r\n",ret);
        return XERROR;
    }

    reg_result = XOS_RegistCommand(ret,TimerShowAll,
                    "timershowall", "show all timer info",
                    "example:timershowall");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("call TimerCommandInit for timershowall failed,error num=%d\r\n",reg_result);
        return XERROR;
    }

    reg_result = XOS_RegistCommand(ret,TimerShowLowStatus,
                        "timershowlowstatus", "show low timer status info",
                        "example:timershowlowstatus 1");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("call TimerCommandInit for TimerShowLowStatus failed,error num=%d\r\n",reg_result);
        return XERROR;
    }


    reg_result = XOS_RegistCommand(ret,TimerShowJiff,
                            "timershowjiff", "show timer jiff",
                            "example:timershowjiff");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("call TimerCommandInit for TimerShowJiff failed,error num=%d\r\n",reg_result);
        return XERROR;
    }

    reg_result = XOS_RegistCommand(ret,TimerShowSignalStatus,
                            "timershowsignalstatus", "timershowsignalstatus",
                            "example:timershowsignalstatus");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("call TimerCommandInit for TimerShowSignal failed,error num=%d\r\n",reg_result);
        return XERROR;
    }

    reg_result = XOS_RegistCommand(ret,TimerSignalReset,
                                "timersignalreset", "timerSignalReset",
                                "example:timerSignalReset");
    if ( XERROR >=reg_result)
    {
        XOS_CliInforPrintf("call TimerCommandInit for TimerSignalReset failed,error num=%d\r\n",reg_result);
        return XERROR;
    }

    reg_result = XOS_RegistCommand(ret, XOS_CliGetTimerInfo,
                     "gettimerinfo", "get the all timer information", "no parameter");
    
    if ( XERROR >=reg_result)
    {
       XOS_CliInforPrintf("call TimerCommandInit for XOS_CliGetTimerInfo failed,error num=%d\r\n",reg_result);
       return XERROR;
    }

    return XSUCC;
}

/*******************************************************************
函数名:
功能：
输入：
输出：
返回：
说明：
*******************************************************************/
XS8 TIM_InitTime(XVOID *t, XVOID *v)
{
    XS32 ret = 0x00;
    t_XOSTASKID taskWork;

    /* 从配置文件xos.xml读取timer个数 */
    if (0 != TIM_XmlReadCfg(&g_TimerMaxCnt, XOS_CliGetXmlName())) 
    {
        g_TimerMaxCnt = TIMER_MAX_CN;
        MMErr("TIM_InitTime->read xos.xml file for timer cfg failed, will use default cfg:0x%x!\r\n",g_TimerMaxCnt);
    }

#if 0
#ifdef XOS_LINUX
    struct itimerval value, ovalue;    
    g_scale.tick_per_sec = sysconf(_SC_CLK_TCK);
#endif
#endif

#ifdef XOS_WIN32
    g_scale.tick_per_sec = 1000;
#endif

#ifdef XOS_VXWORKS
    g_scale.tick_per_sec = sysClkRateGet();
#endif

    g_scale.scale_jiffies = 0x00;

    if(XERROR != (ret = Init_TimerTask(&g_preHighCpu))) {
        ret = Init_TimerTask(&g_preLowCpu);
    }
    if(ret == XERROR) {
        return ret;
    }

#ifdef XOS_WIN32
    ret = timeSetEvent(TM_SCALE, 1, (LPTIMECALLBACK)tm_tickProc, 0, TIME_PERIODIC || TIME_CALLBACK_FUNCTION);//
    if(0 == ret)
    {
        MMErr("[TIM_InitTime]:settimer failed\r\n");
        return XERROR;
    }
#endif

#ifdef XOS_VXWORKS 
    g_wdId = wdCreate();
    ret = wdStart(g_wdId, TM_SCALE * g_scale.tick_per_sec / 1000, (FUNCPTR)tm_tickProc, 0);
    if(OK != ret)
    {
        MMErr("[TIM_InitTime]:settimer failed\r\n");
        return XERROR;
    }
#endif
    

#if(defined(XOS_SOLARIS) ||defined(XOS_LINUX))
    /*定时器驱动*/
    ret = XOS_TaskCreate("Tsk_TimerSignal", TSK_PRIO_HIGHER, 10000,
        (os_taskfunc)linux_tm_tickProc, XNULLP, &taskWork);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_TIME,PL_ERR),"ID_TIME:TIM_InitTime()-> can not create Tsk_TimerSignal!");
        return XERROR;
    }
#endif

/*取消linux下信号驱动定时器*/
#if 0
#ifdef XOS_LINUX  
    signal(SIGALRM, tm_tickProc);
    value.it_value.tv_sec       = 0;
    value.it_value.tv_usec      = TM_SCALE* 1000;
    value.it_interval.tv_sec    = 0;
    value.it_interval.tv_usec   = TM_SCALE * 1000;
    ret = setitimer(ITIMER_REAL, &value, &ovalue);
    if(0 != ret)
    {
        MMErr("[TIM_InitTime]:settimer failed\r\n");
        return XERROR;
    }
#endif
#endif

    TimerCommandInit(SYSTEM_MODE);

    /*创建高精度定时器工作处理任务*/
    ret = XOS_TaskCreate("Tsk_HighTm", TSK_PRIO_HIGHER, 10000, (os_taskfunc)tm_excuteHighProc, XNULLP, &taskWork);
    if(XSUCC != ret)
    {
        MMErr("FID_TIME:TIM_InitTime()-> can not create High timer!");
        return XERROR;
    }
    ret = XOS_TaskCreate("Tsk_HighWorkList", TSK_PRIO_NORMAL,10000,(os_taskfunc)XOS_ExecWorkList, &g_preHighCpu, &taskWork);
    if(XSUCC != ret) {
        return XERROR;
    }
    ret = XOS_TaskCreate("Tsk_LowWorkList", TSK_PRIO_NORMAL,10000,(os_taskfunc)XOS_ExecWorkList, &g_preLowCpu, &taskWork);
    if(XSUCC != ret) {
        return XERROR;
    }    
    
    g_pTimerFidInfo = (XU32 *)XOS_MemMalloc(FID_TIME, MAX_FID_NUMBER * sizeof(int));
    if(NULL == g_pTimerFidInfo)
    {
        MMErr("FID_TIME:TIM_InitTime()->XOS_MemMalloc() for g_pTimerFidInfo failed!");
        return XERROR;
    }
    XOS_MemSet(g_pTimerFidInfo, 0, MAX_FID_NUMBER * sizeof(int));
    
    return ret;
}

XS32 Init_TimerTask(tvec_base_t* baseCpu)
{
    XU32 cir = 0x00;    
    
    baseCpu->usedCn = 0;
    baseCpu->idelCn = g_TimerMaxCnt;

    sys_listInit(&baseCpu->idleList);
    
    baseCpu->idle_timer = (struct timer_list* )XOS_MemMalloc(FID_TIME,sizeof(struct timer_list)*g_TimerMaxCnt);
    
    if(XNULL== baseCpu->idle_timer)
    {
        MMErr("[TIM_InitTime]:malloc timer_list failed\r\n");
        return XERROR;
    }

    for(cir=0x00; cir<g_TimerMaxCnt; cir++)
    {
        sys_listInit(&baseCpu->idle_timer[cir].entry);
        sys_listAdd(&baseCpu->idleList, &baseCpu->idle_timer[cir].entry);
    }
    
    if ( XSUCC != XOS_MutexCreate( &baseCpu->lock) )
    {
        MMErr("[TIM_InitTime]:XOS_MutexCreate g_perCpu.lock failed\r\n");
        return XERROR;
    }
    
    if (XSUCC != XOS_SemCreate(&baseCpu->sem, 0))
    {
        MMErr("[TIM_InitTime]:XOS_SemCreate g_perCpu.sem failed\r\n");
        return XERROR;
    }

    for(cir=0x00; cir<TVR_SIZE; cir++)
    {
        sys_listInit(&baseCpu->tv1.vec[cir]);
    }
    
    for(cir=0x00; cir<TVN_SIZE; cir++)
    {
        sys_listInit(&baseCpu->tv2.vec[cir]);
        sys_listInit(&baseCpu->tv3.vec[cir]);
        sys_listInit(&baseCpu->tv4.vec[cir]);
        sys_listInit(&baseCpu->tv5.vec[cir]);
    }
    
    baseCpu->bActive = 0x01; //激活队列
    baseCpu->timer_jiffies = 0;
    
    //创建定时器所需资源
     if ( XSUCC != XOS_MutexCreate( &baseCpu->tv_work_list.lock) )
    {
        MMErr("[TIM_InitTime]:XOS_MutexCreate g_perCpu.tv_work_list.lock failed\r\n");
        return XERROR;
    }
    
    if (XSUCC != XOS_SemCreate(&baseCpu->tv_work_list.sem, 0))
    {
        MMErr("[TIM_InitTime]:XOS_SemCreate g_perCpu.tv_work_list.sem failed\r\n");
        return XERROR;
    }
    sys_listInit(&baseCpu->tv_work_list.work_list);

    return XSUCC;    
}


/*******************************************************************
函数名:
功能：
输入：
输出：
返回：
说明：
*******************************************************************/
XPUBLIC XS8 TIM_NoticeTime(XVOID *t, XVOID *v)
{
   //SysTM.TIME_INTIALIZED = XTRUE;
   return XSUCC;
}

/************************************************************************
函数名: XOS_TimerSet
功能：  此函数不做处理，只因业务层调用了，所以增加该实现
输入：
        fid            - 功能块ID号
        
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR
说明：
************************************************************************/
XVOID XOS_TimerSet(XBOOL openflag)
{
    return;
}

/************************************************************************
函数名: XOS_TimerNumReg
功能：  新定时器模块所有模块共用一个空闲队列，所以此函数基本返回SUCC
输入：
        fid            - 功能块ID号
        
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR
说明：
************************************************************************/
XS32 XOS_TimerReg(XU32 fid, XU32 msecnd ,XU32 lowPrecNum, XU32 highPrecNum)
{
    XU32 lowcount=0;
    XU32 highcount=0;
    if(!XOS_isValidFid( fid))
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerReg,invalid parameter fid.\n",fid);
        return XERROR;
    }
    if(0 < lowPrecNum)
    {
        lowcount = lowPrecNum;
    }
    if(0 < highPrecNum)
    {
        highcount = highPrecNum;
    }
    if(g_TimerMaxCnt<(lowcount + highcount))
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerReg,the timercount out of limit.\n",fid);
        return XERROR;
    }
    return XSUCC;
}

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
XS32  XOS_TimerCreate(XU32 fid, PTIMER *ptHandle, e_TIMERTYPE timertype, e_TIMERPRE  timerpre, t_BACKPARA *backpara)
{
    struct timer_list *ptimer = NULL;
    /*获取高精度或低精度控制块指针*/
    tvec_base_t *pbase = (timerpre == TIMER_PRE_HIGH ? &g_preHighCpu : &g_preLowCpu);
    XU32 index = 0;

    if(!XOS_isValidFid( fid) ||!ptHandle
        ||timertype >= TIMER_TYPE_END || timerpre >= TIMER_PRE_END )
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerCreate,invalid parameter.\n",fid);
        return XERROR;
    }

    if(*ptHandle)
    {
        /*这里不支持tHandle的内容作为输入参数用*/
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerCreate,invalid input para ptHandle,it isn't null.\n",fid);
        return XERROR;
    }

    XOS_MutexLock(&pbase->lock);
    /* add timer list       */
    
    if(&pbase->idleList == pbase->idleList.next)
    {
        XOS_MutexUnlock(&pbase->lock);
        XOS_Trace(MD(FID_TIME,PL_ERR),"XOS_TimerCreate:timer list is full");
        return XERROR;
    }

    //从空闲队列中取定时节点
    ptimer = (struct timer_list* )pbase->idleList.next;
    sys_listDel(&ptimer->entry);
    sys_listInit(&ptimer->entry);
    
    TIMER_INITIALIZER(ptimer, pbase);

    ptimer->data.para.fid = fid;
    ptimer->data.para.len = 0;
    ptimer->data.para.mode = timertype;
    ptimer->data.para.pre = timerpre;
    ptimer->data.tmnodest = TIMER_STATE_FREE;
    ptimer->data.flag = XTRUE;
    --pbase->idelCn;

    if(NULL != backpara)
    {
        XOS_MemCpy(&ptimer->data.backpara, backpara, sizeof(t_BACKPARA));
    }
    ptimer->data.backpara.count = 0;

    ptimer->magic = TIMER_MAGIC;
    index = (XU32)(ptimer - pbase->idle_timer);  /* 允许精度丢失*/
    *ptHandle = TIM_buildHandle(timerpre,(XU32)index);
    ptimer->data.timerHandler = *ptHandle;

    /*定时器数量统计*/
    if(NULL != g_pTimerFidInfo && fid < MAX_FID_NUMBER)
    {
        g_pTimerFidInfo[fid]++;
    }

    XOS_MutexUnlock(&pbase->lock);
    
    return XSUCC;
}

/*******************************************************************
函数名: XOS_TimerBegin
功能：  定时器启动函数
输入：  tHandle  - 定时器句柄
        len      - 定时器的延时长度（单位ms）
输出：  tHandle
返回：  XSUCC, 函数操作失败返回XERROR
说明：
*******************************************************************/
XS32 XOS_TimerBegin(XU32 fid, PTIMER tHandle, XU32 len)
{
    struct timer_list *ptimer = NULL;
    e_TIMERPRE pre = TIM_getTimerPre(tHandle);
    tvec_base_t *pbase = (pre == TIMER_PRE_HIGH ? &g_preHighCpu : &g_preLowCpu);

    if(!TIM_isValidTHdle(tHandle)||!XOS_isValidFid(fid)||(0 == len))
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerBegin,input parameter is illegal.\n",fid);
        return XERROR;
    }
    
    /*获取资源锁*/
    XOS_MutexLock(&pbase->lock);
    
    /*删除原有定时器所在的运行链关系*/
    ptimer = pbase->idle_timer + TIM_getTimerIndex(tHandle);
    {
        /*从运行链表中删除*/
        XOS_Trace(MD(FID_TIME, PL_INFO), "FID %d call XOS_TimerBegin, timer status is RUN,"\
                                          "delete from list,len=%d,handle=%d",
                                          fid, len, tHandle);

        if(TIMER_STATE_RUN == ptimer->data.tmnodest)
        {
          --pbase->usedCn;
        }

        sys_listDel(&ptimer->entry);
        ptimer->base = NULL;

    }

    /*计算定时器节点参数*/
    ptimer->data.para.len   = len;
    ptimer->data.para.fid   = fid;
    ptimer->base = pbase;

    /*将超时时间转换为节拍数*/
    if(pre == TIMER_PRE_HIGH) 
    {
        ptimer->expires  = len / LOC_HIGHTIMER_CLCUNIT + pbase->timer_jiffies;
    } 
    else 
    {
        ptimer->expires  = len / LOC_LOWTIMER_CLCUNIT + pbase->timer_jiffies;
    }

    ptimer->data.tmnodest   = TIMER_STATE_RUN;

    /*加入运行链表*/
    tm_internalAdd(pbase, ptimer);
    ++pbase->usedCn;

    XOS_MutexUnlock(&pbase->lock);
    
    return XSUCC;
}

/*******************************************************************
函数名: XOS_TimerEnd
功能：  停止定时器
输入：  ptimer      - 定时器句柄
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR
说明：
*******************************************************************/
XS32 XOS_TimerEnd(XU32 fid ,PTIMER tHandle)
{
    struct timer_list *ptimer = NULL;
    e_TIMERPRE pre = TIM_getTimerPre(tHandle);
    tvec_base_t *pbase = (pre == TIMER_PRE_HIGH ? &g_preHighCpu : &g_preLowCpu);

    if(!TIM_isValidTHdle(tHandle)||!XOS_isValidFid(fid))
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerEnd,input parameter is illegal.\n",fid);
        return XERROR;
    }

    XOS_MutexLock(&pbase->lock);
    ptimer = pbase->idle_timer+TIM_getTimerIndex(tHandle);
    
    if(TIMER_STATE_NULL == ptimer->data.tmnodest)
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerEnd,its timer state is wrong\n",fid);
        XOS_MutexUnlock(&pbase->lock);
        return XERROR;
    }

    if(TIMER_STATE_RUN == ptimer->data.tmnodest)
    {
         //从运行链表中删除
        sys_listDel(&ptimer->entry);
        ptimer->base = NULL;
        --pbase->usedCn;
    }
    ptimer->data.tmnodest    = TIMER_STATE_FREE;
    XOS_MutexUnlock(&pbase->lock);
    return XSUCC;
}

/*******************************************************************
函数名: XOS_TimerDelete
功能：  删除定时器句柄
输入：  ptimer      - 定时器句柄
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR
说明：  与函数XOS_TimerEnd的差别在于,从运行链表删除后差别如下
一种情况是可以重用,不归还空闲;
一种是归还给闲链表
*******************************************************************/
XS32 XOS_TimerDelete(XU32 fid, PTIMER tHandle)
{
    struct timer_list *ptimer = NULL;
    e_TIMERPRE pre = TIM_getTimerPre(tHandle);
    tvec_base_t *pbase = (pre == TIMER_PRE_HIGH ? &g_preHighCpu : &g_preLowCpu);

    if(!TIM_isValidTHdle(tHandle)||!XOS_isValidFid(fid))
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerDelete,input parameter is illegal.\n",fid);
        return XERROR;
    }
    
    XOS_MutexLock(&pbase->lock);
    ptimer = pbase->idle_timer+TIM_getTimerIndex(tHandle);

    //防止对已经停止的句柄重复停止操作
    //当停止句柄后，在空闲队列对应的存储位置可能给其他定时器使用了.而造成对其他模块定时器的影响
    if(fid != ptimer->data.para.fid || ptimer->data.para.pre != TIM_getTimerPre(tHandle))
    {
        XOS_MutexUnlock(&pbase->lock);
        return XSUCC;
    }
    
    if(TIMER_STATE_NULL == ptimer->data.tmnodest)
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerDelete,its timer state is wrong\n",fid);
        XOS_MutexUnlock(&pbase->lock);
        return XERROR;
    }
    
    if(TIMER_STATE_RUN == ptimer->data.tmnodest)
    {
       //从运行链表中删除
        sys_listDel(&ptimer->entry);
        ptimer->base = NULL;
        --pbase->usedCn;
    }

     /*加入到空闲链表中*/
    ptimer->data.tmnodest    = TIMER_STATE_NULL;
    sys_listAdd(&pbase->idleList, &ptimer->entry);
    ++pbase->idelCn;

    if(NULL != g_pTimerFidInfo && fid < MAX_FID_NUMBER)
    {
        g_pTimerFidInfo[fid]--;
    }

    XOS_MutexUnlock(&pbase->lock);
    return XSUCC;
}

/*******************************************************************
函数名: XOS_TimerGetState
功能：  定时器是否在运行判断函数
输入：  ptimer      - 定时器句柄 
输出：  none
返回：  如上枚举类型
说明：
*******************************************************************/
e_TIMESTATE XOS_TimerGetState(XU32 fid,PTIMER tHandle)
{
    e_TIMESTATE state = TIMER_STATE_NULL;
    struct timer_list *ptimer = NULL;
    e_TIMERPRE pre = TIM_getTimerPre(tHandle);
    tvec_base_t *pbase = (pre == TIMER_PRE_HIGH ? &g_preHighCpu : &g_preLowCpu);
    
    if((!TIM_isValidDTHdle(tHandle)&&!TIM_isValidTHdle(tHandle))||!XOS_isValidFid(fid))
    {
        XOS_Trace(MD(FID_TIME, PL_WARN), "FID %d call XOS_TimerGetState,invalid handle:0x%x.\n",fid,tHandle);
        return TIMER_STATE_ERROR;
    }
    
    XOS_MutexLock(&pbase->lock);
    ptimer = pbase->idle_timer+TIM_getTimerIndex(tHandle);
    state = ptimer->data.tmnodest;
    XOS_MutexUnlock(&pbase->lock);
    return state;
}

/*******************************************************************
函数名: XOS_TimerRunning
功能：
输入：  ptimer      - 定时器句柄
输出：
返回：  在运行返回XSUCC, 不在运行返回XERROR
说明：
*******************************************************************/
e_TIMESTATE   XOS_TimerRunning(XU32 fid, PTIMER tHandle)
{
    return XOS_TimerGetState(fid, tHandle);
}

//--------------------------以下为二接口模式接口-----------------------//
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
XS32 XOS_TimerStart(PTIMER *ptHandle, t_PARA *timerpara, t_BACKPARA *backpara)
{
    struct timer_list *ptimer = NULL;
    XU32 index;
    e_TIMERPRE pre;
    tvec_base_t *pbase;
    
    if(!timerpara
        ||!XOS_isValidFid( timerpara->fid)
        ||!ptHandle
        ||timerpara->mode >= TIMER_TYPE_END
        || timerpara->pre >= TIMER_PRE_END )
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nXOS_TimerStart input parameters is illegal");
        return XERROR;
    }
#ifdef XOS_NEED_CHK
    if(*ptHandle)
    {
        if(TIM_isValidDTHdle(*ptHandle))
        {
            /*先停止定时器*/
            XOS_TimerStop(timerpara->fid, ptHandle[0]);
        }
        else
        {
            XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStart,input ptHandle isn't null,but it is invalid.\n",timerpara->fid);
            return XERROR;
        }
    }
#endif
    pre = timerpara->pre;
    pbase = (pre == TIMER_PRE_HIGH ? &g_preHighCpu : &g_preLowCpu);
    
    XOS_MutexLock(&pbase->lock);
    
    if(pre == TIMER_PRE_HIGH) {
        if(timerpara->len < LOC_HIGHTIMER_CLCUNIT || timerpara->len % LOC_HIGHTIMER_CLCUNIT != 0)
        {
            XOS_MutexUnlock(&pbase->lock);
            XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStart,input timerpara len %d is wrong.\n",timerpara->fid,timerpara->len);
            return XERROR;
        }
    } else {
        if(timerpara->len < LOC_LOWTIMER_CLCUNIT || timerpara->len % LOC_LOWTIMER_CLCUNIT != 0)
        {
            XOS_MutexUnlock(&pbase->lock);
            XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStart,input timerpara len %d is wrong.",timerpara->fid,timerpara->len);
            return XERROR;
        }
    }

    if(&pbase->idleList == pbase->idleList.next)
    {
        XOS_MutexUnlock(&pbase->lock);
        XOS_Trace(MD(FID_TIME,PL_ERR),"XOS_TimerStart:timer list is full");
        return XERROR;
    }
    
    //从空闲队列中取定时节点
    ptimer = (struct timer_list* )pbase->idleList.next;
    sys_listDel(&ptimer->entry);
    sys_listInit(&ptimer->entry);
    TIMER_INITIALIZER(ptimer, pbase);
    --pbase->idelCn;

    //计算ptimer在链表中的数组下标
    index = (XU32)(ptimer - pbase->idle_timer);/* 允许精度丢失*/
    
    *ptHandle = TIM_buildDHandle(timerpara->pre,(XU32)index);
    ptimer->data.timerHandler = *ptHandle;

    ptimer->data.para.fid   = timerpara->fid;
    ptimer->data.para.len   = timerpara->len;
    ptimer->data.para.pre  = timerpara->pre;
    ptimer->data.para.mode  = timerpara->mode;
    ptimer->data.flag = XFALSE;
    if(backpara)
    {
        XOS_MemCpy(&(ptimer->data.backpara), backpara, sizeof(t_BACKPARA));
    }
    ptimer->data.backpara.count = 0;
    
    ptimer->data.tmnodest   = TIMER_STATE_RUN;
    //添加到工作队列中
    /*
    因为最小精度单元为10毫秒，expires参数初始化为10的倍数
    */
    if(pre == TIMER_PRE_HIGH) {
        ptimer->expires  = timerpara->len / LOC_HIGHTIMER_CLCUNIT + pbase->timer_jiffies;
    } else {
        ptimer->expires  = timerpara->len / LOC_LOWTIMER_CLCUNIT + pbase->timer_jiffies;
    }
    
    /*ptimer实体对象增加到忙链表中*/
    tm_internalAdd(pbase, ptimer);
    ++pbase->usedCn;

    if(NULL != g_pTimerFidInfo && timerpara->fid < MAX_FID_NUMBER)
    {
        g_pTimerFidInfo[timerpara->fid]++;
    }

    XOS_MutexUnlock(&pbase->lock);
    
    return XSUCC;
}

/************************************************************************
函数名: XOS_TimerStop
功能：  定时器停止函数
输入：  tHandle     - 定时器句柄

输出：
返回：  XSUCC, 函数操作失败返回XERROR
说明：  此函数仍然存在错误的关闭别人的定时器的风险
************************************************************************/
XS32 XOS_TimerStop(XU32 fid, PTIMER tHandle)
{
    struct timer_list *ptimer = NULL;
    e_TIMERPRE pre = TIM_getTimerPre(tHandle);
    tvec_base_t *pbase = (pre == TIMER_PRE_HIGH ? &g_preHighCpu : &g_preLowCpu);
    
    if(!TIM_isValidDTHdle(tHandle)||!XOS_isValidFid(fid))
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "handle 0x%x or fid %d is illegal",tHandle,fid);
        return XERROR;
    }
    
    XOS_MutexLock(&pbase->lock);
    
    ptimer = pbase->idle_timer + TIM_getTimerIndex(tHandle);
    
    //防止对已经停止的句柄重复停止操作
    //当停止句柄后，在空闲队列对应的存储位置可能给其他定时器使用了.而造成对其他模块定时器的影响
    if(fid != ptimer->data.para.fid || ptimer->data.para.pre != TIM_getTimerPre(tHandle))
    {
        XOS_Trace(MD(FID_TIME, PL_INFO), "\r\nFID %d the timer has stoped early,timerpre:%d\n",fid,TIM_getTimerPre(tHandle));
        XOS_MutexUnlock(&pbase->lock);
        return XSUCC;
    }
    
    if(TIMER_STATE_RUN == ptimer->data.tmnodest)
    {
        //从运行链表中删除
        sys_listDel(&ptimer->entry);
        ptimer->base = NULL;
        /*加入到空闲链表中*/
        ptimer->data.tmnodest    = TIMER_STATE_NULL;

        sys_listAdd(pbase->idleList.prev, &ptimer->entry);

        --pbase->usedCn;
        ++pbase->idelCn;

        /*对每个fid的定时器数量进行统计*/
        if(NULL != g_pTimerFidInfo && fid < MAX_FID_NUMBER)
        {
            g_pTimerFidInfo[fid]--;
        }
       
        XOS_MutexUnlock(&pbase->lock);
        return XSUCC;
    }
    else if(TIMER_STATE_NULL == ptimer->data.tmnodest)
    {
        /* 定时器句柄置空*/
        ptimer->base = NULL;
        XOS_MutexUnlock(&pbase->lock);
        return XSUCC;
    }

    XOS_MutexUnlock(&pbase->lock);
    
    /*两接口的定时器只有两种状态，否则出错。*/
    XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStop,its timer state is wrong\n",fid);
    return XERROR;
}

/************************************************************************
函数名: XOS_TimerGetParam
功能：  获取定时器的param结构体
输入：  tHandle     - 定时器句柄
输出：  ptBackPara  - 需要获取结构体的输出指针
返回：  XSUCC, 函数操作失败返回XERROR
说明:
************************************************************************/
XS32 XOS_TimerGetParam(PTIMER tHandle,t_BACKPARA *ptBackPara)
{
    struct timer_list *ptimer = NULL;
    e_TIMERPRE pre = TIM_getTimerPre(tHandle);
    tvec_base_t *pbase = (pre == TIMER_PRE_HIGH ? &g_preHighCpu : &g_preLowCpu);

    if (NULL == ptBackPara)
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "param is null\n");
        return XERROR;
    }
    
    if(!TIM_isValidDTHdle(tHandle)&&!TIM_isValidTHdle(tHandle))
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "handle 0x%x is illegal",tHandle);
        return XERROR;
    }
    
    XOS_MutexLock(&pbase->lock);
    
    ptimer = pbase->idle_timer + TIM_getTimerIndex(tHandle);
    if(TIMER_STATE_RUN == ptimer->data.tmnodest)
    {
        XOS_MemCpy(ptBackPara, &ptimer->data.backpara, sizeof(t_BACKPARA));
    }
    else
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nTimer state is wrong,can't get timer param!\n");
		XOS_MutexUnlock(&pbase->lock);
        return XERROR;
    }

    XOS_MutexUnlock(&pbase->lock);

    return XSUCC;
}

#else
#ifdef  __cplusplus
extern  "C"
{
#endif

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include "xoscfg.h"
#include "xosencap.h"
#include "cmtimer.h"
#include "xosmodule.h"
#include "xostrace.h"
#include "xosmem.h"
#include "xospub.h"
#include "clishell.h"
/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
#if 0
XSTATIC FILE * fp = XNULL;
XSTATIC char buf[120];
#endif

static int g_timer_multi_thread = 1;

#define XOS_TIMER_LOCK(lock) if (g_timer_multi_thread) XOS_MutexLock(lock)
#define XOS_TIMER_UNLOCK(lock) if (g_timer_multi_thread) XOS_MutexUnlock(lock)

XSTATIC t_ParManage    SysTM ;
XEXTERN XVOID XOS_TimerSet(XBOOL openflag);

XS32 TIM_GetNodeNum(XU32 fid,e_TIMERPRE timerpre,XU32 *curusage,XU32 *maxusage,XU32 *freenum)
{
    t_TIMERNODE *pstTmp = XNULLP;
    t_TIMERMNGT *pTimerMngt = XNULLP;

    XU32 num =0;

    if(!XOS_isValidFid( fid)  || timerpre >= TIMER_PRE_END )
    {
        //XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call TIM_GetNodeNum,invalid parameter.\n",fid);
        return XERROR;
    }

    pTimerMngt = MOD_getTimMntByFid(timerpre,fid);
    if(pTimerMngt == XNULLP)
    return XERROR;

    pstTmp = (t_TIMERNODE *)pTimerMngt->idleheader.next;

    if(XNULLP == pstTmp)
    return XERROR;

    while(&pTimerMngt->idleheader != &pstTmp->stLe)
    {
        num++;
        pstTmp = (t_TIMERNODE *)pstTmp->stLe.next ;
    }
    *maxusage=pTimerMngt->maxUsage;
    *curusage = pTimerMngt->curNumOfElements;
    *freenum=num;
    return XSUCC;

}

XVOID XOS_TimerSet(XBOOL openflag)
{
#ifdef XOS_NEED_CHK
   SysTM.TIME_INTIALIZED = openflag;
#endif
}
/************************************************************************
 函数名:TIM_buildHandle
 功能: 构造定时器句柄
 输入: linkType － 定时器类型(高四位)
       linkIndex －定时器索引
 输出:
 返回:  返回定时器句柄
 说明:  定时器句柄组织如图，

            pre     checkcoder                  Index
          32     28          20                     1
           -----------------------------------------
           |     |           |                      |
           |     |           |                      |
           -----------------------------------------

************************************************************************/
XSTATIC PTIMER  TIM_buildHandle(e_TIMERPRE pre, XU32 Index)
{
     /*LINK_CHECK_NUM, 为了躲开vc中不初始化的局部变量总为0xcccc 的情况*/
     PTIMER  timerhandle;
     XU32 checkNum = TIMER_CHECK_NUM; /*0xbb*/
     timerhandle = (PTIMER)(XPOINT)(((pre&0xf)<<28) |(checkNum<<20) | ((Index&0xfffff)));

     return timerhandle;
}

/************************************************************************
 函数名:TIM_buildDHandle
 功能: 构造定时器句柄
 输入: linkType － 定时器类型(高四位)
       linkIndex －定时器索引
 输出:
 返回:  返回定时器句柄
 说明:  定时器句柄组织如图，

            pre     checkcoder                  Index
          32     28          20                     1
           -----------------------------------------
           |     |           |                      |
           |     |           |                      |
           -----------------------------------------

************************************************************************/
XSTATIC PTIMER  TIM_buildDHandle(e_TIMERPRE pre, XU32 Index)
{
     /*LINK_CHECK_NUM, 为了躲开vc中不初始化的局部变量总为0xcccc 的情况*/
     PTIMER  timerhandle;
     XU32 checkNum = TIMER_CHECK_DNUM; /*0xDD*/
     timerhandle = (PTIMER)(XPOINT)(((pre&0xf)<<28) |(checkNum<<20) | ((Index&0xfffff)));

     return timerhandle;
}

/************************************************************************
 函数名:TIM_isValidTHdle
 功能: 验证定时器句柄的有效性
 输入: 定时器句柄
 输出:
 返回: 有效返回XTURE, 否则返回XFALSE
 说明:
************************************************************************/
XSTATIC  XBOOL TIM_isValidTHdle( PTIMER timerhandle)
{
     return (TIMER_CHECK_NUM == (((XPOINT)timerhandle>>20)&0xff)? XTRUE:XFALSE);
}

/************************************************************************
 函数名:TIM_isValidDTHdle
 功能: 验证定时器句柄的有效性
 输入: 定时器句柄
 输出:
 返回: 有效返回XTURE, 否则返回XFALSE
 说明:
************************************************************************/
XSTATIC  XBOOL TIM_isValidDTHdle( PTIMER timerhandle)
{
     return (TIMER_CHECK_DNUM == (((XPOINT)timerhandle>>20)&0xff)? XTRUE:XFALSE);
}

/************************************************************************
 函数名:TIM_getTimerIndex
 功能: 通过定时器句柄获取定时器Index
 输入: 定时器句柄
 输出:
 返回: 定时器索引
 说明:
************************************************************************/
XSTATIC  XS32 TIM_getTimerIndex( PTIMER timerhandle)
{
     return (XS32)((XPOINT)timerhandle&0x0fffff);
}

/************************************************************************
 函数名:TIM_getTimerPre
 功能: 通过定时器句柄获取定时器精度
 输入: 定时器句柄
 输出:
 返回: 定时器精度(高精度或低精度)
 说明:
************************************************************************/
XSTATIC  e_TIMERPRE TIM_getTimerPre( PTIMER timerhandle)
{
     return (e_TIMERPRE)((XPOINT)timerhandle>>28);
}

/************************************************************************
函数名  : LowClock_MsgSnd
功能    : 低精度定时器的一级定时器超时函数
输入    :
输出    : none
返回    : XSUCC, 函数操作失败返回XERROR
说明    :
************************************************************************/
XSTATIC XVOID  LowClock_MsgSnd( )
{
    t_XOSCOMMHEAD *timer_temp;
    t_LISTENT    *head= &SysTM.HeadList;
    t_ParTimerNode  *pstTmp=XNULLP, *pstTmpNext=XNULLP;

    if(head->next == XNULL)
    {
         XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe timer module haven't been initialized\n");
         return ;
    }

    if(head->next == head)
    {
        return;
    }

    for(pstTmp=(t_ParTimerNode *)head->next; &pstTmp->stLe!=head;)
    {
        pstTmpNext = (t_ParTimerNode *)pstTmp->stLe.next;
        if(pstTmp->NowTime < LOC_LOWTIMER_CLCUNIT)
        {
            pstTmp->NowTime=0;
        }else
        {
            pstTmp->NowTime -= LOC_LOWTIMER_CLCUNIT;
        }
        /*定时器节点已到期*/
        if( pstTmp->NowTime ==0)
        {
            /*循环复位节点的计时值*/
            pstTmp->NowTime = pstTmp->TimeLen;

            /*向相应的fid发送时钟消息,表示注册的精度周期已到*/
            timer_temp = (t_XOSCOMMHEAD*)XNULLP;
            timer_temp = XOS_MsgMemMalloc( FID_TIME, XNULL );
            if(XNULLP == timer_temp)
            {
                XOS_PRINT(MD(FID_TIME, PL_ERR), "Timer Clock message allocation failed.\n");
                return ;
            }
            timer_temp->datasrc.FID  = FID_TIME;
            timer_temp->datadest.FID = pstTmp->fid;
            timer_temp->datasrc.PID  = XOS_GetLocalPID();
            timer_temp->datadest.PID = XOS_GetLocalPID();
            timer_temp->prio         = eTimePrio;
            timer_temp->msgID        = eTimeLowClock;
            if( XOS_MsgSend(timer_temp) != XSUCC)
            {
                  XOS_MsgMemFree(timer_temp->datasrc.FID, timer_temp);
            }
        }
         pstTmp = pstTmpNext;    /*指针下移*/
    }
}

/************************************************************************
函数名  : HighClock_Timeout
功能    : 高精度定时器的一级定时器超时函数
输入    :
输出    : none
返回    : XSUCC, 函数操作失败返回XERROR ,各操作系统的格式都不同
说明    :
************************************************************************/
#ifdef  XOS_WIN32
XSTATIC XVOID CALLBACK LowClock_Timeout(XU32 wTimerID, XU32 msg, XS32 dwUser,
                                            XS32 dw1, XS32 dw2)
#endif
#ifdef XOS_VXWORKS
XSTATIC XVOID    LowClock_Timeout( )
#endif
#if(defined(XOS_WIN32) || defined(XOS_VXWORKS))
{
     XOS_SemPut(&SysTM.lotsem);/*PC-LINT超时处理函数判断返回值无意义.*/

        /*restart wdtimer*/
#ifdef XOS_VXWORKS
        /* restart the timer */
        if(wdStart(SysTM.wdIdlot, SysTM.TmrMultiplierlot , (FUNCPTR)LowClock_Timeout, 0) != XSUCC)
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe watch dog timer START is failed");

        }
#endif
}
#endif

/************************************************************************
函数名  : HighClock_Timeout
功能    : 高精度定时器的一级定时器超时函数
输入    :
输出    : none
返回    : XSUCC, 函数操作失败返回XERROR ,各操作系统的格式都不同
说明    :
************************************************************************/
#ifdef  XOS_WIN32
XSTATIC XVOID CALLBACK HighClock_Timeout(XU32 wTimerID, XU32 msg, XS32 dwUser,
                                            XS32 dw1, XS32 dw2)
#endif
#ifdef XOS_VXWORKS
XSTATIC XVOID    HighClock_Timeout( )
#endif
#if(defined(XOS_WIN32) || defined(XOS_VXWORKS))
{
     XOS_SemPut(&SysTM.hitsem);/*PC-LINT超时处理函数判断返回值无意义.*/

        /*restart wdtimer*/
#ifdef XOS_VXWORKS
        /* restart the timer */
        if(wdStart(SysTM.wdId, SysTM.TmrMultiplier , (FUNCPTR)HighClock_Timeout, 0) != XSUCC)
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe watch dog timer START is failed");

        }
#endif
    
}
#endif

/************************************************************************
函数名  : XOS_StartSysTimer
功能    : 封装每个操作系统下自带的系统TIMER,API
输入    :
输出    : none
返回    :
说明    :由于其它系统涉及到timer模块的变量SysTM每个系统自己的超时函数不同，
         与timer紧密相关，所以放在这里
         millisecond 毫秒,千分之一秒
         microsecond 微秒,千分之一毫秒(10-3)秒
************************************************************************/
XSTATIC XS32 XOS_StartSysTimer(e_TIMERPRE pre)
{
#ifdef XOS_WIN32
    if(pre == TIMER_PRE_HIGH)
    {
        if(XNULL == timeSetEvent(LOC_HIGHTIMER_CLCUNIT, 0, (LPTIMECALLBACK)HighClock_Timeout,
                                0, TIME_PERIODIC|TIME_CALLBACK_FUNCTION))

        return XERROR;

    }
    else if(pre == TIMER_PRE_LOW)
    {

        if(XNULL == timeSetEvent(LOC_LOWTIMER_CLCUNIT, 0, (LPTIMECALLBACK)LowClock_Timeout,
                                    0, TIME_PERIODIC|TIME_CALLBACK_FUNCTION))
        return XERROR;
    }
#endif

#ifdef XOS_VXWORKS
    extern int sysClkRateGet (void);

    XU32 tmr_ticks;
    /*每秒的ticks次中断*/
    tmr_ticks = sysClkRateGet();

    if(pre == TIMER_PRE_HIGH)
    {
        if (tmr_ticks < 1000/LOC_HIGHTIMER_CLCUNIT)
        {
            XOS_Trace(MD(FID_TIME, PL_DBG), "Clock rate too slow for high presion timer,the high presion may be not accurate!\n");
            return XERROR;
        }
        /*高精度每period(ms)的tick数*/
        SysTM.TmrMultiplier  = tmr_ticks/(1000/LOC_HIGHTIMER_CLCUNIT);
        SysTM.wdId = wdCreate();

        if (SysTM.wdId == NULL)
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe watch dog timer create is failed");
            return XERROR;
        }
        /* start the timer */
        if (wdStart(SysTM.wdId, SysTM.TmrMultiplier , (FUNCPTR)HighClock_Timeout, 0) != XSUCC)
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe watch dog timer START is failed");
            return XERROR;
        }
    }
    else if(pre == TIMER_PRE_LOW)
    {
        if (tmr_ticks < 1000/LOC_LOWTIMER_CLCUNIT)
        {
            XOS_Trace(MD(FID_TIME, PL_DBG), "Clock rate too slow for low presion timer,the low presion may be not accurate!\n");
            return XERROR;
        }
        /*低精度每period(ms)的tick数*/
        SysTM.TmrMultiplierlot = tmr_ticks/(1000/LOC_LOWTIMER_CLCUNIT);
        SysTM.wdIdlot = wdCreate();

        if (SysTM.wdIdlot == NULL)
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe watch dog timer create is failed");
            return XERROR;
        }
        /* start the timer */
        if (wdStart(SysTM.wdIdlot, SysTM.TmrMultiplierlot , (FUNCPTR)LowClock_Timeout, 0) != XSUCC)
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe watch dog timer START is failed");
            return XERROR;
        }
    }

#endif

    return  XSUCC;
    }

/************************************************************************
函数名  : High_TimerTask
功能    : 高精度定时器用于发送一级定时器时钟消息的任务函数入口.
输入    :

输出    : none
返回    :
说明    :高精度定时器用于发送消息的任务.
************************************************************************/
XPUBLIC XVOID High_TimerTask( XVOID* ptr)
{
    t_XOSCOMMHEAD *timer_temp;
    XU8 i=0;

#if(defined(XOS_SOLARIS) ||defined(XOS_LINUX))
    struct timeval t1,t2,t3;
    gettimeofday(&t1, NULL);
#endif

    XOS_UNUSED(ptr);

    /*等待所有模块都启动后再开始工作.*/
    while(!SysTM.TIME_INTIALIZED)
    XOS_Sleep(100);

    if(XOS_StartSysTimer(TIMER_PRE_HIGH)!= XSUCC)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe system timer for high pre creation is failed");
        return;
    }

    while(1)
    {

#if(defined(XOS_WIN32) || defined(XOS_VXWORKS))
        if(XOS_SemGet(&SysTM.hitsem)!= XSUCC)
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR)," semget error in high_timer task \n");
            return ;
        }
#endif

#if(defined(XOS_SOLARIS) ||defined(XOS_LINUX))
        gettimeofday(&t2, NULL);

        t1.tv_usec += 20000;
        /* 1s = 1000000us */
        if (t1.tv_usec > 1000000)
        {
        t1.tv_sec++;
        t1.tv_usec -= 1000000;
        }

        t3.tv_sec = 0;
        /*每次这个线程调度的时间差不超过20ms，否则高精度定时器将会不准*/
        t3.tv_usec = ((t1.tv_usec > t2.tv_usec)? t1.tv_usec - t2.tv_usec
                :t1.tv_usec + 1000000 - t2.tv_usec);
        select(1, NULL, NULL, NULL, &t3);
#endif

        for(i=0;i<SysTM.HiTmindex;i++)
        {
            /*向相应的fid发送时钟消息,表示高精度定时器的周期已到*/
            timer_temp = (t_XOSCOMMHEAD*)XNULLP;
            timer_temp = XOS_MsgMemMalloc( FID_TIME, XNULL );
            if(XNULLP == timer_temp)
            {
                XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe TIMER clock message allocated failed");
                return;
            }
            timer_temp->datasrc.FID  = FID_TIME;
            timer_temp->datadest.FID = SysTM.HiTmFid[i];
            timer_temp->datasrc.PID  = XOS_GetLocalPID();
            timer_temp->datadest.PID = XOS_GetLocalPID();
            timer_temp->prio         = eTimePrio;
            timer_temp->msgID        = eTimeHigClock;

            if( XOS_MsgSend(timer_temp) != XSUCC)
            {
                XOS_MsgMemFree(timer_temp->datasrc.FID, timer_temp);
            }

        }
}
}

/************************************************************************
 函数名:Low_TmerTask
 功能:
 输入:
 输出:
 返回:
 说明: 低精度定时器用于发送一级定时器时钟消息的任务函数入口.
************************************************************************/
XPUBLIC XVOID Low_TimerTask( XVOID* ptr)
{

#if(defined(XOS_SOLARIS) ||defined(XOS_LINUX))
    struct timeval t1,t2,t3;
    gettimeofday(&t1, NULL);
#endif

    XOS_UNUSED(ptr);

    /*等待所有模块都启动后再开始工作.*/
    while(!SysTM.TIME_INTIALIZED)
    XOS_Sleep(100);

    if(XOS_StartSysTimer(TIMER_PRE_LOW)!= XSUCC)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe system timer for low pre creation is failed");
        return;
    }

    while(1)
    {

#if(defined(XOS_WIN32) || defined(XOS_VXWORKS))
        if(XOS_SemGet(&SysTM.lotsem)!= XSUCC)
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR)," semget error in low_timer task \n");
            return ;
        }
#endif

#if(defined(XOS_SOLARIS) ||defined(XOS_LINUX))
        gettimeofday(&t2, NULL);

        t1.tv_usec += 100000;
        /* 1s = 1000000us */
        if (t1.tv_usec > 1000000)
        {
            t1.tv_sec++;
            t1.tv_usec -= 1000000;
        }

        t3.tv_sec = 0;
        /*每次这个线程调度的时间差不超过100ms，否则高精度定时器将会不准*/
        t3.tv_usec = ((t1.tv_usec > t2.tv_usec)? t1.tv_usec - t2.tv_usec
                :t1.tv_usec + 1000000 - t2.tv_usec);
        select(1, NULL, NULL, NULL, &t3);
#endif
        LowClock_MsgSnd();

    }

}
/************************************************************************
函数名  : TIM_LinkCreate
功能    : 各任务注册定时器链
输入    : pstTimerMngt    -    任务管理定时器链的结构指针
          ulMaxTimerNum   -    功能块的最大定时器数目
          id              -    标识
输出    : none
返回    :
说明    :
************************************************************************/
XSTATIC XS32 TIM_LinkCreate(XU32 fid, t_TIMERMNGT *pstTimerMngt, XU32 ulMaxTimerNum)
{
    XU32 i;
    if(0 == ulMaxTimerNum ||  XNULLP== pstTimerMngt)
    return XERROR;

    /* 申请定时器池的内存 */
    i = sizeof(t_TIMERNODE) * ulMaxTimerNum;

    //2008-01-21便于定时器使用跟踪,修改如下
    //pstTimerMngt->pstTimerPool = (t_TIMERNODE *)XOS_MemMalloc(FID_TIME, i);
    pstTimerMngt->pstTimerPool = (t_TIMERNODE *)XOS_MemMalloc(fid, i);
    if(XNULL == pstTimerMngt->pstTimerPool)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe memory allocate for timer is failed");
        return XERROR;
    }

    pstTimerMngt->maxtimernum = ulMaxTimerNum;
    pstTimerMngt->nowclock = 0;

    /* 初始化运行链表 */
    for(i = 0; i < LOC_TIMER_LINKLEN; i++)
    {
        CM_INIT_TQ(&pstTimerMngt->stRunList[i]);

    }
    /* 初始化空闲链表 */
    CM_INIT_TQ(&pstTimerMngt->idleheader);

    /*把空闲的节点链入空闲链表中*/
    for (i = 0; i < ulMaxTimerNum; i++)
    {
        pstTimerMngt->pstTimerPool[i].tmnodest = TIMER_STATE_NULL;
        CM_INIT_TQ(&pstTimerMngt->pstTimerPool[i].stLe);
        CM_PLC_TQ(pstTimerMngt->idleheader.prev, &pstTimerMngt->pstTimerPool[i].stLe);
    }

    /* 初始化锁 */
    if ( XSUCC != XOS_MutexCreate(&pstTimerMngt->timerMutex) )
    {
        XOS_Trace(MD(FID_TIME, PL_WARN), "XOS_MutexCreate timerMutex failed! will not be thread-safed .");
    }

    return XSUCC;

}

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
XS32 XOS_TimerReg(XU32 fid, XU32 msecnd ,XU32 lowPrecNum, XU32 highPrecNum)
{
    t_TIMERMNGT* tm =XNULLP;

    if(!XOS_isValidFid( fid))
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerReg,invalid parameter fid.\n",fid);
        return XERROR;
    }
    if(lowPrecNum > 200000)
    {
       XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerReg,para lowPrecNum %d extend limit.\n",fid,lowPrecNum);
       return XERROR;
    }
    if(highPrecNum >2000)
    {
       XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerReg,para highPrecNum %d extend limit.\n",fid,highPrecNum);
       return XERROR;
    }
    if(lowPrecNum)
    {
        SysTM.ParNoArray[fid].fid =fid;
        if( msecnd % LOC_LOWTIMER_CLCUNIT != 0)
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerReg,para msecnd is not n 100ms format.\n",fid);
            return XERROR;
        }
        if(msecnd )
        {
            SysTM.ParNoArray[fid].TimeLen = msecnd;
            SysTM.ParNoArray[fid].NowTime = msecnd;
        }
        else
        {
            SysTM.ParNoArray[fid].TimeLen = LOC_LOWTIMER_CLCUNIT;
            SysTM.ParNoArray[fid].NowTime = LOC_LOWTIMER_CLCUNIT;
        }
        CM_PLC_TQ(SysTM.HeadList.prev, &SysTM.ParNoArray[fid].stLe);
        tm = MOD_getTimMntByFid(TIMER_PRE_LOW,fid);
        if(!tm)
        {
            return XERROR;
        }
        tm->timeruint = SysTM.ParNoArray[fid].TimeLen;
        if(TIM_LinkCreate(fid, tm, lowPrecNum)!= XSUCC)
        {
            return XERROR;
        }

    }
    if(highPrecNum)
    {
#ifdef XOS_HIGHTIMER
        if(SysTM.HiTmindex <= HiTm_FIDNum-1)
           SysTM.HiTmFid[SysTM.HiTmindex++] = fid;
        else
            return XERROR;
        tm = MOD_getTimMntByFid(TIMER_PRE_HIGH,fid);
        if(!tm)
            return XERROR;
        tm->timeruint = LOC_HIGHTIMER_CLCUNIT;
        if(TIM_LinkCreate(fid, tm, highPrecNum)!= XSUCC)
            return XERROR;
#else
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nhigh timer used must defined  XOS_HIGHTIMER");
#endif
    }

    return XSUCC;
}

//#ifdef  XOS_TIMER_FOURFUNC
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
XS32  XOS_TimerCreate(XU32 fid, PTIMER *ptHandle, e_TIMERTYPE timertype, e_TIMERPRE  timerpre, t_BACKPARA *backpara)
{
    t_TIMERNODE *pstTmp = XNULLP;
    t_TIMERMNGT *pTimerMngt = XNULLP;
    XU32 TimerpoolIndex =0;

    if(!XOS_isValidFid( fid) ||!ptHandle
        ||timertype >= TIMER_TYPE_END || timerpre >= TIMER_PRE_END )
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerCreate,invalid parameter.\n",fid);
        return XERROR;
    }

    if(*ptHandle)
    {
      /*这里不支持tHandle的内容作为输入参数用*/
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerCreate,invalid input para ptHandle,it isn't null.\n",fid);
        return XERROR;
    }
    pTimerMngt = MOD_getTimMntByFid(timerpre,fid);
    if(pTimerMngt == XNULLP)
    {
         return XERROR;
    }
    XOS_TIMER_LOCK(&pTimerMngt->timerMutex);
    /*从空闲链中找到一个节点*/
    pstTmp = (t_TIMERNODE *)pTimerMngt->idleheader.next;
    if((XNULLP == pstTmp) || (&pTimerMngt->idleheader == &pstTmp->stLe))
    {
         XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerCreate,there's no free node in its timer manager.\n",fid);
         XOS_TIMER_UNLOCK(&pTimerMngt->timerMutex);
         return XERROR;
    }

    if(TIMER_STATE_NULL!= pstTmp->tmnodest)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerCreate,its timer status is wrong",fid);
        XOS_TIMER_UNLOCK(&pTimerMngt->timerMutex);
        return XERROR;
    }
    /* 从空闲链中删除 */
    CM_RMV_TQ(&pstTmp->stLe);

    TimerpoolIndex = pstTmp - pTimerMngt->pstTimerPool;
    /* 填写定时器信息 */
    *ptHandle = TIM_buildHandle(timerpre,(XU16)TimerpoolIndex);

    pstTmp->stLe.prev   = XNULL;
    pstTmp->stLe.next   = XNULL;
    pstTmp->pTimer      = ptHandle;
    pstTmp->para.fid    = fid;
    pstTmp->para.len    = 0;
    pstTmp->para.mode   = timertype;
    pstTmp->para.pre    = timerpre;
    pstTmp->walktimes       = 0;
    pstTmp->tmnodest    = TIMER_STATE_FREE;
    pstTmp->flag = XTRUE;
    if(XNULLP != backpara)
    {
        XOS_MemCpy(&pstTmp->backpara, backpara, backparalen);
    }
    XOS_TIMER_UNLOCK(&pTimerMngt->timerMutex);
    return XSUCC;
}

/*******************************************************************
函数名: XOS_TimerBegin
功能：  定时器启动函数
输入：  tHandle  - 定时器句柄
        len      - 定时器的延时长度（单位ms）
输出：  tHandle
返回：  XSUCC, 函数操作失败返回XERROR
说明：
*******************************************************************/
XS32 XOS_TimerBegin(XU32 fid,PTIMER tHandle, XU32 len)
{
    XU32        ulTimerLinkIndex;
    t_TIMERMNGT *pTimerMngt = XNULLP;
    t_TIMERNODE *pstTmp = XNULLP;

    if(!TIM_isValidTHdle(tHandle)||!XOS_isValidFid(fid))
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerBegin,input parameter is illegal.\n",fid);
        return XERROR;
    }

    pTimerMngt  = MOD_getTimMntByFid(TIM_getTimerPre(tHandle),fid);
    if(pTimerMngt == XNULLP)
    {
         return XERROR;
    }
    XOS_TIMER_LOCK(&pTimerMngt->timerMutex);

    if(0 == len  ||pTimerMngt->timeruint == 0
        || len%(pTimerMngt->timeruint) != 0  || len <(pTimerMngt->timeruint))
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerBegin,input para timer len %d is wrong\n",fid,len);
        XOS_TIMER_UNLOCK(&pTimerMngt->timerMutex);
        return XERROR;
    }

    pstTmp = pTimerMngt->pstTimerPool + TIM_getTimerIndex(tHandle);
    if(TIMER_STATE_NULL == pstTmp->tmnodest)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerBegin,its timer state is wrong\n",fid);
        XOS_TIMER_UNLOCK(&pTimerMngt->timerMutex);
        return XERROR;
    }
    if(TIMER_STATE_RUN == pstTmp->tmnodest)
    {
      CM_RMV_TQ(&pstTmp->stLe);/*从运行链表中删除*/
      /*20090419 add stat below 8888*/
      pTimerMngt->curNumOfElements--;
      /*20090419 add stat above*/

    }
    pstTmp->para.len   = len;
    pstTmp->para.fid   = fid;
    pstTmp->tmnodest   = TIMER_STATE_RUN;
    pstTmp->walktimes  = 0; /*已经挂上过运行链表中,又被重新启动的情况需要先置为0*/

    /*将该节点加到定时器运行链表中 */
    ulTimerLinkIndex = (len / pTimerMngt->timeruint + pTimerMngt->nowclock)% LOC_TIMER_LINKLEN;
    CM_PLC_TQ(pTimerMngt->stRunList[ulTimerLinkIndex].prev,&pstTmp->stLe);

    /*20090419 add stat below*/
    /* 设置统计数据 */
    pTimerMngt->curNumOfElements++;
    if (pTimerMngt->curNumOfElements > pTimerMngt->maxUsage)
    {
        pTimerMngt->maxUsage = pTimerMngt->curNumOfElements;
    }
    /*20090419 add stat above*/

    XOS_TIMER_UNLOCK(&pTimerMngt->timerMutex);
    return XSUCC;
}

/*******************************************************************
函数名: XOS_TimerEnd
功能：  停止定时器
输入：  ptimer      - 定时器句柄
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR
说明：
*******************************************************************/
XS32 XOS_TimerEnd(XU32 fid ,PTIMER tHandle)
{
    t_TIMERNODE *pstTmp = XNULLP;
    t_TIMERMNGT *tmmanager = XNULLP ;

    if(!TIM_isValidTHdle(tHandle)||!XOS_isValidFid(fid))
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerEnd,input parameter is illegal.\n",fid);
        return XERROR;
    }

    tmmanager   = MOD_getTimMntByFid(TIM_getTimerPre(tHandle),fid);
    XOS_TIMER_LOCK(&tmmanager->timerMutex);

    pstTmp = tmmanager->pstTimerPool + TIM_getTimerIndex(tHandle);
    if(TIMER_STATE_NULL == pstTmp->tmnodest)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerEnd,its timer state is wrong\n",fid);
        XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
        return XERROR;
    }

    if(TIMER_STATE_RUN == pstTmp->tmnodest)
    {
      CM_RMV_TQ(&pstTmp->stLe);/*从运行链表中删除*/
      /*20090419 add stat below 8888*/
      tmmanager->curNumOfElements--;
      /*20090419 add stat above*/
    }
    pstTmp->tmnodest    = TIMER_STATE_FREE;
    XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
    return XSUCC;
}

/*******************************************************************
函数名: XOS_TimerDelete
功能：  删除定时器句柄
输入：  ptimer      - 定时器句柄
输出：  none
返回：  XSUCC, 函数操作失败返回XERROR
说明：  与函数XOS_TimerEnd的差别在于,从运行链表删除后差别如下
一种情况是可以重用,不归还空闲;
一种是归还给闲链表
*******************************************************************/
XS32 XOS_TimerDelete(XU32 fid, PTIMER tHandle)
{

    t_TIMERNODE *pstTmp = XNULLP;
    t_TIMERMNGT *tmmanager = XNULLP ;

    if(!TIM_isValidTHdle(tHandle)||!XOS_isValidFid(fid))
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerDelete,input parameter is illegal.\n",fid);
        return XERROR;
    }

    tmmanager   = MOD_getTimMntByFid(TIM_getTimerPre(tHandle),fid);
    if(!tmmanager)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerDelete,get is timer manager failed,it is null.\n",fid);
        return XERROR;
    }
    XOS_TIMER_LOCK(&tmmanager->timerMutex);

    pstTmp = tmmanager->pstTimerPool + TIM_getTimerIndex(tHandle);
    if(TIMER_STATE_NULL == pstTmp->tmnodest)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerDelete,its timer state is wrong\n",fid);
        XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
        return XERROR;
    }

    if(TIMER_STATE_RUN == pstTmp->tmnodest)
    {
      CM_RMV_TQ(&pstTmp->stLe);/*从运行链表中删除*/
      /*20090419 add stat below*/
      tmmanager->curNumOfElements--;
      /*20090419 add stat above*/
    }

     /*加入到空闲链表中*/
    pstTmp->tmnodest    = TIMER_STATE_NULL;
    CM_PLC_TQ(tmmanager->idleheader.prev,&pstTmp->stLe);

    XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
    return XSUCC;
}

/*******************************************************************
函数名: XOS_TimerGetState
功能：  定时器是否在运行判断函数
输入：  ptimer      - 定时器句柄 
输出：  none
返回：  如上枚举类型
说明：
*******************************************************************/
e_TIMESTATE XOS_TimerGetState(XU32 fid,PTIMER tHandle)
{
    t_TIMERMNGT *tmmanager = XNULLP;
    e_TIMESTATE state = TIMER_STATE_NULL;
    if((!TIM_isValidDTHdle(tHandle)&&!TIM_isValidTHdle(tHandle))||!XOS_isValidFid(fid))
    {
        XOS_PRINT(MD(FID_TIME, PL_WARN), "FID %d call XOS_TimerGetState,invalid handle:0x%x.\n",fid,tHandle);
        return TIMER_STATE_ERROR;
    }
    tmmanager   =  MOD_getTimMntByFid(TIM_getTimerPre(tHandle),fid);
    if(!tmmanager)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerGetState,get is timer manager failed,it is null.\n",fid);
        return TIMER_STATE_ERROR;
    }

    XOS_TIMER_LOCK(&tmmanager->timerMutex);
    state = (tmmanager->pstTimerPool + TIM_getTimerIndex(tHandle))->tmnodest;
    XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
    return state;
}

//#endif
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
XS32 XOS_TimerStart(PTIMER *ptHandle, t_PARA *timerpara, t_BACKPARA *backpara)
{
    t_TIMERNODE *pstTmp = XNULLP;
    XU32        ulTimerLinkIndex  ;
    t_TIMERMNGT *tmmanager = XNULLP;
    XS32  TimerpoolIndex =-1;

    if(!timerpara ||!XOS_isValidFid( timerpara->fid) ||!ptHandle
    ||timerpara->mode >= TIMER_TYPE_END || timerpara->pre >= TIMER_PRE_END )
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nXOS_TimerStart input parameters is illegal");
        return XERROR;
    }

#ifdef XOS_NEED_CHK
    if(*ptHandle)
    {
        if(TIM_isValidDTHdle(*ptHandle))
        {
            /*先停止定时器*/
            XOS_TimerStop(timerpara->fid,ptHandle[0]);
        }
        else
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStart,input ptHandle isn't null,but it is invalid.\n",timerpara->fid);
            return XERROR;
        }
    }
#else
    // 自增后指向一片未知内存，怎么可以这样呢？
    //ptHandle++;
#endif

    tmmanager   = MOD_getTimMntByFid(timerpara->pre,timerpara->fid);

    if(!tmmanager)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStart,get its manager failed.\n",timerpara->fid);
        return XERROR;
    }
    XOS_TIMER_LOCK(&tmmanager->timerMutex);

    if(tmmanager->timeruint == 0 ||timerpara->len <(tmmanager->timeruint)
    ||timerpara->len % (tmmanager->timeruint) != 0)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStart,input timerpara len %d is wrong.\n",timerpara->fid,timerpara->len);
        XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
        return XERROR;
    }
    pstTmp = (t_TIMERNODE *)tmmanager->idleheader.next;
    /*if there's no free node in the list*/
    if((XNULLP == pstTmp) || (&tmmanager->idleheader == &pstTmp->stLe))
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStart,there's no free node in its manager list\n",timerpara->fid);
        XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
        return XERROR;
    }
    /* 从空闲链中删除 */
    CM_RMV_TQ(&pstTmp->stLe);
    pstTmp->tmnodest= TIMER_STATE_RUN;
    TimerpoolIndex = pstTmp - tmmanager->pstTimerPool;
    *ptHandle = TIM_buildDHandle(timerpara->pre,(XU16)TimerpoolIndex);
    pstTmp->pTimer      = ptHandle;
    pstTmp->stLe.next  = XNULL;
    pstTmp->stLe.prev  = XNULL;
    pstTmp->para.fid   = timerpara->fid;
    pstTmp->para.len   = timerpara->len;
    pstTmp->para.pre  = timerpara->pre;
    pstTmp->para.mode  = timerpara->mode;
    pstTmp->flag = XFALSE;
    if(backpara)
    {
        XOS_MemCpy(&pstTmp->backpara, backpara, backparalen);
    }
    pstTmp->walktimes = 0;

    /* 将该节点加到定时器运行链表中 */
    ulTimerLinkIndex = (timerpara->len / tmmanager->timeruint + tmmanager->nowclock)% LOC_TIMER_LINKLEN;
    CM_PLC_TQ(tmmanager->stRunList[ulTimerLinkIndex].prev,&pstTmp->stLe);

    /*20090419 add stat below*/
    /* 设置统计数据 */
    tmmanager->curNumOfElements++;
    if (tmmanager->curNumOfElements > tmmanager->maxUsage)
    {
        tmmanager->maxUsage = tmmanager->curNumOfElements;
    }
    /*20090419 add stat above*/

    XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
    return XSUCC;
}

/************************************************************************
函数名: XOS_TimerStop
功能：  定时器停止函数
输入：  tHandle     - 定时器句柄

输出：
返回：  XSUCC, 函数操作失败返回XERROR
说明：
************************************************************************/
XS32 XOS_TimerStop(XU32 fid, PTIMER tHandle)
{
    t_TIMERNODE *pstTmp = XNULLP;
    t_TIMERMNGT *tmmanager = XNULLP;

    if(!TIM_isValidDTHdle(tHandle)||!XOS_isValidFid(fid))
    {
        XOS_PRINT(MD(FID_TIME, PL_INFO), "\r\nThe argument in XOS_TimerStop by fid %d is illegal may be result of repeated stop",fid);
        return XERROR;
    }

    tmmanager   = MOD_getTimMntByFid(TIM_getTimerPre(tHandle),fid);
    if(!tmmanager)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStop,get is timer manager failed,it is null.\n",fid);
        return XERROR;
    }
    XOS_TIMER_LOCK(&tmmanager->timerMutex);
    pstTmp = tmmanager->pstTimerPool + TIM_getTimerIndex(tHandle);

    if(TIMER_STATE_RUN == pstTmp->tmnodest)
    {
        /* 定时器句柄置空*/
        *(pstTmp->pTimer) = XNULL;
        /*从运行链表中删除*/
        CM_RMV_TQ(&pstTmp->stLe);
        pstTmp->tmnodest    = TIMER_STATE_NULL;
        /*加入到空闲链表中*/
        CM_PLC_TQ(tmmanager->idleheader.prev,&pstTmp->stLe);
        /* 将它加入到闲链中 */
        /*20090419 add stat below*/
        tmmanager->curNumOfElements--;
        /*20090419 add stat above*/

        XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
        return XSUCC;
    }
    else if(TIMER_STATE_NULL == pstTmp->tmnodest)
    {
        /* 定时器句柄置空*/
        *(pstTmp->pTimer) = XNULL;
        XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
        return XSUCC;
    }

    /*两接口的定时器只有两种状态，否则出错。*/
    XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStop,its timer state is wrong\n",fid);

    XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
    return XERROR;

}

/*******************************************************************
函数名: XOS_TimerRunning
功能：
输入：  ptimer      - 定时器句柄
输出：
返回：  在运行返回XSUCC, 不在运行返回XERROR
说明：
*******************************************************************/
e_TIMESTATE   XOS_TimerRunning(XU32 fid, PTIMER tHandle)
{
    return XOS_TimerGetState(fid, tHandle);
}

/************************************************************************
函数名  : TIM_ClckProc
功能    : 各任务收到时钟任务消息的统一处理函数
输入    : management - 任务管理定时器链的结构指针
输出    : none
返回    : XSUCC, 函数操作失败返回XERROR
说明    :
************************************************************************/
XPUBLIC XS32 TIM_ClckProc(t_TIMERMNGT *management)
{
    XU32 i=0,timermaxscale=0,ulTimerLinkIndex=0;
    t_LISTENT    *head , list ;
    t_TIMERNODE  *pstTmp= XNULLP, *pstTmpNext= XNULLP;
    modTimerProcFunc timerExpFunc;
#if 0
    sprintf(buf,"The  TIM_ClckProc @ %d(ms) work \n", timeGetTime());
    fputs(buf,fp);
#endif
    if(XNULL == management)
    {
        return XERROR;
    }
    XOS_TIMER_LOCK(&management->timerMutex);

    /* 刻度往前走一步 */
    management->nowclock = (management->nowclock + 1) % LOC_TIMER_LINKLEN;
    head = &(management->stRunList[management->nowclock]);
    /*初始化到期的链表*/
    CM_INIT_TQ(&list);
    timermaxscale =LOC_TIMER_LINKLEN * (management->timeruint);
    for(pstTmp=(t_TIMERNODE *)head->next; &pstTmp->stLe!=head;)
    {
        if(!pstTmp)
        {
            XOS_TIMER_UNLOCK(&management->timerMutex);
            return XERROR;
        }

        pstTmpNext = (t_TIMERNODE *)pstTmp->stLe.next;
        i = ((pstTmp->walktimes + 1) * timermaxscale);
        /*定时器节点已到期*/
        if (pstTmp->para.len <= i)
        {
            /* 从定时器链表中删除 */
            CM_RMV_TQ(&pstTmp->stLe);
            /* 加入到期链表中 */
            CM_PLC_TQ(list.prev, &pstTmp->stLe);
        }
        else
        {
            pstTmp->walktimes++;
        }
        pstTmp = pstTmpNext;    /*指针下移*/
    }
    /* 处理到期链表 */
    for(pstTmp=(t_TIMERNODE *)list.next; &pstTmp->stLe!=&list; pstTmp=(t_TIMERNODE *)list.next)
    {

        /*从到期链表中删除*/
        CM_RMV_TQ(&pstTmp->stLe);

        if(pstTmp->para.mode == TIMER_TYPE_LOOP)
        {
            /* 如果是循环定时器 ,加入到运行链表中*/
            ulTimerLinkIndex = (pstTmp->para.len / management->timeruint + management->nowclock)% LOC_TIMER_LINKLEN;
            pstTmp->walktimes = 0;
            CM_PLC_TQ(management->stRunList[ulTimerLinkIndex].prev,&pstTmp->stLe);
        }
        else
        {
            if(!pstTmp->flag)/*两接口的定时器类型*/
            {
                *(pstTmp->pTimer) = XNULL;  /*一次性定时器句柄置空*/
                CM_PLC_TQ(&(management->idleheader), &(pstTmp->stLe));
                pstTmp->tmnodest = TIMER_STATE_NULL;
            }
            else /*四接口的定时器类型*/
            {
                pstTmp->tmnodest = TIMER_STATE_FREE;
            }
            /*20090419 add stat below 8888*/
            management->curNumOfElements--;
            /*20090419 add stat above*/
        }

        /* 回调相应处理函数 */
        timerExpFunc = MOD_getTimProcFunc(pstTmp->para.fid);
        if(!timerExpFunc )
        {
            XOS_TIMER_UNLOCK(&management->timerMutex);
            return XERROR;
        }
        // 防止回调函数中启动停止定时器
        XOS_TIMER_UNLOCK(&management->timerMutex);
        timerExpFunc( &pstTmp->backpara);
        XOS_TIMER_LOCK(&management->timerMutex);
    }
    XOS_TIMER_UNLOCK(&management->timerMutex);
    return XSUCC;
}

/*******************************************************************
函数名:
功能：
输入：
输出：
返回：
说明：
*******************************************************************/
XPUBLIC XS8 TIM_InitTime(XVOID *t, XVOID *v)
{

    //XU32 ret;
    CM_INIT_TQ(&SysTM.HeadList);
    SysTM.HiTmindex = 0;
#ifdef XOS_HIGHTIMER
    if(XSUCC != XOS_SemCreate(&(SysTM.hitsem), 0))
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "timer create semphore failed !\n");
        return XERROR;
    }
#endif

    if(XSUCC != XOS_SemCreate(&(SysTM.lotsem), 0))
    {
        XOS_Trace(MD(FID_TIME, PL_ERR), "timer create semphore failed !\n");
        return XERROR;
    }

    SysTM.TIME_INTIALIZED = XFALSE;

    return XSUCC;
}

/*******************************************************************
函数名:
功能：
输入：
输出：
返回：
说明：
*******************************************************************/
XPUBLIC XS8 TIM_NoticeTime(XVOID *t, XVOID *v)
{
   SysTM.TIME_INTIALIZED = XTRUE;
   return XSUCC;
}

#ifdef  __cplusplus
}
#endif
#endif //XOS_NEED_OLDTIMER


