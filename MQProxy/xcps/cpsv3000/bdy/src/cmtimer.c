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
 **   wangzongyou     2006.7.25           �޸���Ӧģ�����      
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
#define TVN_MASK (TVN_SIZE - 1)     /*2,3,4,5��ת�̶̿�����*/
#define TVR_MASK (TVR_SIZE - 1)     /*�����ת�̶̿�����*/

#define TIMER_MIN_CN        (1000)     /*��С��ʱ������*/
#define TIMER_MAX_CN        (0x1ffff) /*���ʱ������*/

XU32 g_TimerMaxCnt = TIMER_MAX_CN;    /*���ʱ������*/

XU32 *g_pTimerFidInfo = NULL;    /*����ͳ�Ƹ���ģ�鵱ǰʹ�õĶ�ʱ��������*/

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
 * ʱ��̶ȶ���
 */
typedef struct tvec_scale_s
{
    XU32 tick_per_sec;     //ÿ��ticks    
    XU32 scale_jiffies;    //ʱ��ָ��,ÿ����һʱ�̣�ָ���1
}tvec_scale_s;

//work�������У�����ר�ŷ�����Ϣ
typedef struct tvec_work_s
{
    t_XOSMUTEXID lock;       //ִ�ж�����
    t_XOSSEMID   sem;        //�������е��ź���
    sys_list_st work_list;   //ִ�ж���
}tvec_work_s;

typedef struct tvec_t_base_s 
{
    t_XOSMUTEXID lock;
    t_XOSSEMID sem;    //�̶��ź���
    XU32 timer_jiffies;    //ʱ��,��ͬ���ȶ���ʱ��������ͬ
    XU32 bActive:1;
    XU32 usedCn:31;/*��¼��ʱ������������*/
    XU32 idelCn;/*��¼��ǰ����������*/
    sys_list_st   idleList;         /* ��������װ���б�         */
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
������: tm_tickProc
���ܣ�  ϵͳ����ʱ����ʱ�ص�����
���룺  
�����  
���أ�  
˵����
************************************************************************/
#ifdef WIN32
void PASCAL tm_tickProc(XU32 wTimerID, XU32 msg, XU32 dwUser, XU32 dwl, XU32 dw2);
#endif
#if defined LINUX || defined VXWORKS
void tm_tickProc();
#endif

/*
 * ���ж�ʱ��������
 *
 */
void tm_Run(tvec_base_t *base);
/*
 * ִ�����ڿ�ִ�еĶ�ʱ������
 */
XSTATIC XVOID XOS_ExecWorkList(void *);

/*
 * ��ʼ���߾��Ⱥ͵;����������
 *
 */
XSTATIC XS32 Init_TimerTask(tvec_base_t* baseCpu);


XEXTERN XVOID XOS_TimerSet(XBOOL openflag);

#ifdef XOS_VXWORKS
WDOG_ID g_wdId = 0x00;//VXWORKS �¶�ʱ���Ź�ID
#endif

tvec_scale_s g_scale;   //ʱ��
tvec_base_t g_preHighCpu;  //�߾��ȶ���
tvec_base_t g_preLowCpu;  //�;��ȶ���

/************************************************************************
 ������:TIM_buildHandle
 ����: ���춨ʱ�����
 ����: linkType �� ��ʱ������(����λ)
       linkIndex ����ʱ������
 ���:
 ����:  ���ض�ʱ�����
 ˵��:  ��ʱ�������֯��ͼ��

            pre     checkcoder                  Index
          32     28          20                     1
           -----------------------------------------
           |     |           |                      |
           |     |           |                      |
           -----------------------------------------

************************************************************************/
XSTATIC PTIMER  TIM_buildHandle(e_TIMERPRE pre, XU32 Index)
{
     /*LINK_CHECK_NUM, Ϊ�˶㿪vc�в���ʼ���ľֲ�������Ϊ0xcccc �����*/
     PTIMER  timerhandle;
     XU32 checkNum = TIMER_CHECK_NUM; /*0xbb*/
     timerhandle = (PTIMER)(XPOINT)(((pre&0xf)<<28) |(checkNum<<20) | ((Index&0xfffff)));

     return timerhandle;
}

/************************************************************************
 ������:TIM_buildDHandle
 ����: ���춨ʱ�����
 ����: linkType �� ��ʱ������(����λ)
       linkIndex ����ʱ������
 ���:
 ����:  ���ض�ʱ�����
 ˵��:  ��ʱ�������֯��ͼ��

            pre     checkcoder                  Index
          32     28          20                     1
           -----------------------------------------
           |     |           |                      |
           |     |           |                      |
           -----------------------------------------

************************************************************************/
XSTATIC PTIMER  TIM_buildDHandle(e_TIMERPRE pre, XU32 Index)
{
     /*LINK_CHECK_NUM, Ϊ�˶㿪vc�в���ʼ���ľֲ�������Ϊ0xcccc �����*/
     PTIMER  timerhandle;
     XU32 checkNum = TIMER_CHECK_DNUM; /*0xDD*/
     timerhandle = (PTIMER)(XPOINT)(((pre&0xf)<<28) |(checkNum<<20) | ((Index&0xfffff)));

     return timerhandle;
}

/************************************************************************
 ������:TIM_isValidTHdle
 ����: ��֤��ʱ���������Ч��
 ����: ��ʱ�����
 ���:
 ����: ��Ч����XTURE, ���򷵻�XFALSE
 ˵��:
************************************************************************/
XSTATIC  XBOOL TIM_isValidTHdle( PTIMER timerhandle)
{
     return (TIMER_CHECK_NUM == (((XPOINT)timerhandle>>20)&0xff)? XTRUE:XFALSE);
}

/************************************************************************
 ������:TIM_isValidDTHdle
 ����: ��֤��ʱ���������Ч��
 ����: ��ʱ�����
 ���:
 ����: ��Ч����XTURE, ���򷵻�XFALSE
 ˵��:
************************************************************************/
XSTATIC  XBOOL TIM_isValidDTHdle( PTIMER timerhandle)
{
     return (TIMER_CHECK_DNUM == (((XPOINT)timerhandle>>20)&0xff)? XTRUE:XFALSE);
}

/************************************************************************
 ������:TIM_getTimerIndex
 ����: ͨ����ʱ�������ȡ��ʱ��Index
 ����: ��ʱ�����
 ���:
 ����: ��ʱ������
 ˵��:
************************************************************************/
XSTATIC  XS32 TIM_getTimerIndex( PTIMER timerhandle)
{
     return (XS32)((XPOINT)timerhandle&0x0fffff);
}

/************************************************************************
 ������:TIM_getTimerPre
 ����: ͨ����ʱ�������ȡ��ʱ������
 ����: ��ʱ�����
 ���:
 ����: ��ʱ������(�߾��Ȼ�;���)
 ˵��:
************************************************************************/
XSTATIC  e_TIMERPRE TIM_getTimerPre( PTIMER timerhandle)
{
     return (e_TIMERPRE)((XPOINT)timerhandle>>28);
}


/************************************************************************
������  : TIM_XmlReadCfg
����    : get XOS timer configure informations
����    : filename   XOS �����ļ���
���    :
����    : XS32
˵����    ��ȡ�����ļ�ʧ�ܷ���XERROR �ɹ�����XSUCC
************************************************************************/
static XS32 TIM_XmlReadCfg(XU32 *pTimerCnt, XCHAR * filename)
{
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    XU32  CpsTempVal = TIMER_MAX_CN/2;
    XU32  OtherTempVal = TIMER_MAX_CN/2;

    /*�������*/
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
    
    /*�Ҹ��ڵ�*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XERROR ;
    }
    /*���ڵ�*/
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
    /*TIMER���ڵ�*/
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
    
    /*����TIMER�ӽڵ�*/
    while ( cur != XNULL )
    {
        /*timer�������*/
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
������: tm_internalAdd
���ܣ���ʱ�������뵽��Ӧ��ʱ��������
���룺
�����  N/A
���أ�
˵����
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
    
    /*ͨ����Խ�����������ҽӵ�����*/
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

    /*�˴����ڿ����밲ȫ���⣬��������Ҫ��֤������*/
    sys_listAdd(vec->prev, &timer->entry);

}
/************************************************************************
������: tm_cascade
���ܣ�����ʱ��̶Ȱ���Ӧ�Ķ�ʱ�����뵽ִ�ж�����
����: index -ָ���̶��̵Ŀ̶�ֵ
�����  N/A
���أ�
˵����
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
        
        //����ʱ��̶� ���¼��뵽tv1��
        tm_internalAdd(base, tmp);
    }
//    INIT_LIST_HEAD(head);

    return index;
}
/************************************************************************
������: XOS_CliGetTimerInfo
���ܣ�ͳ�Ƹ���ģ��ʹ�ö�ʱ��������
���룺
�����  N/A
���أ�
˵����MAX_FID_NUMBERֵ�����Fidֵ���������ģ��ʹ��Fidֵ����2000����Ҫ�޸Ĵ˺�
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
������: linux_tm_tickProc
���ܣ�  linux�µ�ʱ����������
���룺  
�����  
���أ�  
˵����ÿ��10ms����һ�������ź���
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

        t1.tv_usec += 10000;  /*10ms���*/
        /* 1s = 1000000us */
        if (t1.tv_usec > 1000000)
        {
            t1.tv_sec++;
            t1.tv_usec -= 1000000;
        }

        t3.tv_sec = 0;
        /*ÿ������̵߳��ȵ�ʱ������10ms������߾��ȶ�ʱ�����᲻׼*/
        t3.tv_usec = ((t1.tv_usec > t2.tv_usec)? t1.tv_usec - t2.tv_usec
                :t1.tv_usec + 1000000 - t2.tv_usec);
        select(1, NULL, NULL, NULL, &t3);
#endif
      /*�����ź���*/
       if(g_scale.scale_jiffies % (LOC_HIGHTIMER_CLCUNIT / TM_SCALE) == 0) 
       {
            XOS_SemPut(&g_preHighCpu.sem);
       }
    
       if(g_scale.scale_jiffies % (LOC_LOWTIMER_CLCUNIT / TM_SCALE) == 0) 
       {
           XOS_SemPut(&g_preLowCpu.sem);
       }    

       g_scale.scale_jiffies++;  //ʱ���1        
    }
}

/************************************************************************
������: tm_tickProc
���ܣ�  ʱ����������
���룺  
�����  
���أ�  
˵����
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

    g_scale.scale_jiffies++;  //ʱ���1
}


/************************************************************************
������: tm_excuteHighProc
���ܣ�  ��ʱ��������.
        (��Ҫ�����������tick����-��������ʱ��ָ��ģ�鷢�Ͷ�ʱ��Ϣ)
���룺  
�����  
���أ�  
˵����
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

    /*����������ת��ָ��*/
    index = base->timer_jiffies & TVR_MASK;

    /*
     * Cascade timers:
     ����̶�
     */
    XOS_MutexLock(&base->lock);
    
    /*���ڵ��⣬����ת��ת��ָ��, index==0,˵���������ת��ת��һ��255��, ֻ���ڲ�ת��ת��һ�ܣ��ſ��ܵ�����
      ��ת����Ҫ�ƶ�ת��ָ��*/

    /*�����ܵ��ڵĽڵ�ҽӵ�������0�Žڵ�����*/
    if  (!index &&
            (!tm_cascade(base, &base->tv2, INDEX(0))) &&
            (!tm_cascade(base, &base->tv3, INDEX(1))) &&
             !tm_cascade(base, &base->tv4, INDEX(2)))
             tm_cascade(base, &base->tv5, INDEX(3));
    
    /*�����ڽڵ������ƶ�����������*/
    sys_listSpliceInit(base->tv1.vec + index, &worklst);
    XOS_MutexUnlock(&base->lock);

    //����tv1����
    while (!sys_listEmpty(head)) 
    {
        work_timer = NULL;
        timer = (struct timer_list* )head->next;//list_entry(head->next,struct timer_list,entry);
        sys_listDel(&timer->entry);
        
        //XOS_Trace(MD(FID_TIME,PL_ERR),"tm_run: add running timer into worklist");
        //���ӵ�worklist����
        //�ӿ��ж�����ȡ��ʱ�ڵ�

        XOS_MutexLock(&base->lock);
        timer->data.backpara.count++;  /* ��¼��ʱ�����д��� */
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
            /*�Ӿֲ������м��뵽worklist��������*/
            XOS_MutexLock(&base->tv_work_list.lock);
            sys_listAdd(base->tv_work_list.work_list.prev, &work_timer->entry);
            XOS_MutexUnlock(&base->tv_work_list.lock);
        }
        
        /*����ѭ����ʱ�������¼��빤������*/
        XOS_MutexLock(&base->lock);
        if(TIMER_TYPE_LOOP == timer->data.para.mode)
        {
            //���ݾ��ȼ���λ��
            if(timer->data.para.pre == TIMER_PRE_HIGH) {
                timer->expires  = timer->data.para.len / LOC_HIGHTIMER_CLCUNIT + base->timer_jiffies;
            } else {
                timer->expires  = timer->data.para.len / LOC_LOWTIMER_CLCUNIT + base->timer_jiffies;
            }
            tm_internalAdd(base, timer);
        }
        else
        {
            //����һ�ζ�ʱ�������������ж�����
            if(XFALSE==timer->data.flag)
            {
                timer->data.tmnodest = TIMER_STATE_NULL;
                sys_listAdd(&base->idleList, &timer->entry);
                ++base->idelCn;

                /*���ζ�ʱ����XOS_TimerStopʱ����tmnodest�жϣ�������Ҫ���������*/
                if(g_pTimerFidInfo != NULL && timer->data.para.fid < MAX_FID_NUMBER)
                {
                    g_pTimerFidInfo[timer->data.para.fid]--;
                }
            }
            else
            {
                //�Ľӿ�ģʽ���������XOS_TimerDelete���ͷţ���������ж���
                timer->data.tmnodest = TIMER_STATE_FREE;
            }
            --base->usedCn;
        }
        XOS_MutexUnlock(&base->lock);

        
    }
    //��worklist�����źţ�֪ͨ��ִ��
    if(!sys_listEmpty(&base->tv_work_list.work_list)) 
    {
        XOS_SemPut(&base->tv_work_list.sem);
    }
    //����ָ���1
    XOS_MutexLock(&base->lock);
    base->timer_jiffies++;
    XOS_MutexUnlock(&base->lock);
}
/************************************************************************
 ������:TIM_MsgSnd
 ����: ��ʱ����ʱ����Ϣ���ͳ���
 ����: t_TIMERNODE ���͵Ĳ���
 ���:
 ����: 
 ˵��:
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
        
        XOS_SemGet(&baseCpu->tv_work_list.sem);/*��ȡ�ź���*/

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
 ������:TIM_MsgSnd
 ����: ��ʱ����ʱ����Ϣ���ͳ���
 ����: t_TIMERNODE ���͵Ĳ���
 ���:
 ����: 
 ˵��:
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
         * ���ڸ߾��ȶ�ʱ�����������ȼ�����һ��
         * ���ڵ;��ȶ�ʱ�����������ȼ�����һ��
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
������  : TIM_ClckProc
����    : �������յ�ʱ��������Ϣ��ͳһ������
����    : management - �������ʱ�����Ľṹָ��
���    : none
����    : XSUCC, ��������ʧ�ܷ���XERROR
˵��    :
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
    /* �ص���Ӧ������ */
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
������:TimerShowAll
���ܣ���ʾ��ʱ����ͳ����Ϣ
���룺
�����
���أ�
˵����
*******************************************************************/
XVOID TimerShowAll(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XOS_CliExtPrintf(pCliEnv,"%-12s%-12s%-12s\r\n","MaxListCnt","WorkListCnt","IdelListCnt");
    XOS_CliExtPrintf(pCliEnv,"HighTimer:%-12ld%-12ld%-12ld\r\n",g_TimerMaxCnt,g_preHighCpu.usedCn,g_preHighCpu.idelCn);
    XOS_CliExtPrintf(pCliEnv,"LowTimer:%-12ld%-12ld%-12ld\r\n",g_TimerMaxCnt,g_preLowCpu.usedCn,g_preLowCpu.idelCn);
}

/*******************************************************************
������:TimerShowJiff
���ܣ���ʾ��ʱ����������ͳ����Ϣ
���룺
�����
���أ�
˵����
*******************************************************************/
XVOID TimerShowJiff(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XOS_CliExtPrintf(pCliEnv, "scale_jiffies = %d\r\n", g_scale.scale_jiffies);
    XOS_CliExtPrintf(pCliEnv, "&g_preLowCpu.sem = %0x8\r\n", &g_preLowCpu.sem);

    return;
}

/*******************************************************************
������:TimerShowSignal
���ܣ���ʾʱ���źŵ�ǰ��׽������ַ
���룺
�����
���أ�
˵����
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
������:TimerShowSignal
���ܣ�����ʱ���źŲ�׽����
���룺
�����
���أ�
˵����
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
������:TimerShowLowStatus
���ܣ����ݾ����ѯ�;��ȶ�ʱ���ڵ�ĵ�ǰ״̬
���룺
�����
���أ�
˵����
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
������:TimerCommandInit
���ܣ�ע�ᶨʱ�������ͳ����Ϣ��ѯ����
���룺
�����
���أ�
˵����
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
������:
���ܣ�
���룺
�����
���أ�
˵����
*******************************************************************/
XS8 TIM_InitTime(XVOID *t, XVOID *v)
{
    XS32 ret = 0x00;
    t_XOSTASKID taskWork;

    /* �������ļ�xos.xml��ȡtimer���� */
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
    /*��ʱ������*/
    ret = XOS_TaskCreate("Tsk_TimerSignal", TSK_PRIO_HIGHER, 10000,
        (os_taskfunc)linux_tm_tickProc, XNULLP, &taskWork);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_TIME,PL_ERR),"ID_TIME:TIM_InitTime()-> can not create Tsk_TimerSignal!");
        return XERROR;
    }
#endif

/*ȡ��linux���ź�������ʱ��*/
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

    /*�����߾��ȶ�ʱ��������������*/
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
    
    baseCpu->bActive = 0x01; //�������
    baseCpu->timer_jiffies = 0;
    
    //������ʱ��������Դ
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
������:
���ܣ�
���룺
�����
���أ�
˵����
*******************************************************************/
XPUBLIC XS8 TIM_NoticeTime(XVOID *t, XVOID *v)
{
   //SysTM.TIME_INTIALIZED = XTRUE;
   return XSUCC;
}

/************************************************************************
������: XOS_TimerSet
���ܣ�  �˺�����������ֻ��ҵ�������ˣ��������Ӹ�ʵ��
���룺
        fid            - ���ܿ�ID��
        
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
************************************************************************/
XVOID XOS_TimerSet(XBOOL openflag)
{
    return;
}

/************************************************************************
������: XOS_TimerNumReg
���ܣ�  �¶�ʱ��ģ������ģ�鹲��һ�����ж��У����Դ˺�����������SUCC
���룺
        fid            - ���ܿ�ID��
        
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
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
������: XOS_TimerCreate
���ܣ�  ��ʱ����������
���룺  fid      - ���ܿ�ID��
        tHandle  - ��ʱ�����
        type     - ��ʱ����������ʽ�ͼ���ѡ��
        para     - ��ʱ����ʱ�ش�����
�����  tHandle
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
*******************************************************************/
XS32  XOS_TimerCreate(XU32 fid, PTIMER *ptHandle, e_TIMERTYPE timertype, e_TIMERPRE  timerpre, t_BACKPARA *backpara)
{
    struct timer_list *ptimer = NULL;
    /*��ȡ�߾��Ȼ�;��ȿ��ƿ�ָ��*/
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
        /*���ﲻ֧��tHandle��������Ϊ���������*/
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

    //�ӿ��ж�����ȡ��ʱ�ڵ�
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
    index = (XU32)(ptimer - pbase->idle_timer);  /* �����ȶ�ʧ*/
    *ptHandle = TIM_buildHandle(timerpre,(XU32)index);
    ptimer->data.timerHandler = *ptHandle;

    /*��ʱ������ͳ��*/
    if(NULL != g_pTimerFidInfo && fid < MAX_FID_NUMBER)
    {
        g_pTimerFidInfo[fid]++;
    }

    XOS_MutexUnlock(&pbase->lock);
    
    return XSUCC;
}

/*******************************************************************
������: XOS_TimerBegin
���ܣ�  ��ʱ����������
���룺  tHandle  - ��ʱ�����
        len      - ��ʱ������ʱ���ȣ���λms��
�����  tHandle
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
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
    
    /*��ȡ��Դ��*/
    XOS_MutexLock(&pbase->lock);
    
    /*ɾ��ԭ�ж�ʱ�����ڵ���������ϵ*/
    ptimer = pbase->idle_timer + TIM_getTimerIndex(tHandle);
    {
        /*������������ɾ��*/
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

    /*���㶨ʱ���ڵ����*/
    ptimer->data.para.len   = len;
    ptimer->data.para.fid   = fid;
    ptimer->base = pbase;

    /*����ʱʱ��ת��Ϊ������*/
    if(pre == TIMER_PRE_HIGH) 
    {
        ptimer->expires  = len / LOC_HIGHTIMER_CLCUNIT + pbase->timer_jiffies;
    } 
    else 
    {
        ptimer->expires  = len / LOC_LOWTIMER_CLCUNIT + pbase->timer_jiffies;
    }

    ptimer->data.tmnodest   = TIMER_STATE_RUN;

    /*������������*/
    tm_internalAdd(pbase, ptimer);
    ++pbase->usedCn;

    XOS_MutexUnlock(&pbase->lock);
    
    return XSUCC;
}

/*******************************************************************
������: XOS_TimerEnd
���ܣ�  ֹͣ��ʱ��
���룺  ptimer      - ��ʱ�����
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
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
         //������������ɾ��
        sys_listDel(&ptimer->entry);
        ptimer->base = NULL;
        --pbase->usedCn;
    }
    ptimer->data.tmnodest    = TIMER_STATE_FREE;
    XOS_MutexUnlock(&pbase->lock);
    return XSUCC;
}

/*******************************************************************
������: XOS_TimerDelete
���ܣ�  ɾ����ʱ�����
���룺  ptimer      - ��ʱ�����
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����  �뺯��XOS_TimerEnd�Ĳ������,����������ɾ����������
һ������ǿ�������,���黹����;
һ���ǹ黹��������
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

    //��ֹ���Ѿ�ֹͣ�ľ���ظ�ֹͣ����
    //��ֹͣ������ڿ��ж��ж�Ӧ�Ĵ洢λ�ÿ��ܸ�������ʱ��ʹ����.����ɶ�����ģ�鶨ʱ����Ӱ��
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
       //������������ɾ��
        sys_listDel(&ptimer->entry);
        ptimer->base = NULL;
        --pbase->usedCn;
    }

     /*���뵽����������*/
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
������: XOS_TimerGetState
���ܣ�  ��ʱ���Ƿ��������жϺ���
���룺  ptimer      - ��ʱ����� 
�����  none
���أ�  ����ö������
˵����
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
������: XOS_TimerRunning
���ܣ�
���룺  ptimer      - ��ʱ�����
�����
���أ�  �����з���XSUCC, �������з���XERROR
˵����
*******************************************************************/
e_TIMESTATE   XOS_TimerRunning(XU32 fid, PTIMER tHandle)
{
    return XOS_TimerGetState(fid, tHandle);
}

//--------------------------����Ϊ���ӿ�ģʽ�ӿ�-----------------------//
/************************************************************************
������: XOS_TimerStart
���ܣ�  ��ʱ����������
���룺  tHandle     - ��ʱ�����
        timerpara   - ��ʱ������
        backpara    - ��ʱ����ʱ�ش�����

�����  tHandle
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
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
            /*��ֹͣ��ʱ��*/
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
    
    //�ӿ��ж�����ȡ��ʱ�ڵ�
    ptimer = (struct timer_list* )pbase->idleList.next;
    sys_listDel(&ptimer->entry);
    sys_listInit(&ptimer->entry);
    TIMER_INITIALIZER(ptimer, pbase);
    --pbase->idelCn;

    //����ptimer�������е������±�
    index = (XU32)(ptimer - pbase->idle_timer);/* �����ȶ�ʧ*/
    
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
    //��ӵ�����������
    /*
    ��Ϊ��С���ȵ�ԪΪ10���룬expires������ʼ��Ϊ10�ı���
    */
    if(pre == TIMER_PRE_HIGH) {
        ptimer->expires  = timerpara->len / LOC_HIGHTIMER_CLCUNIT + pbase->timer_jiffies;
    } else {
        ptimer->expires  = timerpara->len / LOC_LOWTIMER_CLCUNIT + pbase->timer_jiffies;
    }
    
    /*ptimerʵ��������ӵ�æ������*/
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
������: XOS_TimerStop
���ܣ�  ��ʱ��ֹͣ����
���룺  tHandle     - ��ʱ�����

�����
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����  �˺�����Ȼ���ڴ���Ĺرձ��˵Ķ�ʱ���ķ���
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
    
    //��ֹ���Ѿ�ֹͣ�ľ���ظ�ֹͣ����
    //��ֹͣ������ڿ��ж��ж�Ӧ�Ĵ洢λ�ÿ��ܸ�������ʱ��ʹ����.����ɶ�����ģ�鶨ʱ����Ӱ��
    if(fid != ptimer->data.para.fid || ptimer->data.para.pre != TIM_getTimerPre(tHandle))
    {
        XOS_Trace(MD(FID_TIME, PL_INFO), "\r\nFID %d the timer has stoped early,timerpre:%d\n",fid,TIM_getTimerPre(tHandle));
        XOS_MutexUnlock(&pbase->lock);
        return XSUCC;
    }
    
    if(TIMER_STATE_RUN == ptimer->data.tmnodest)
    {
        //������������ɾ��
        sys_listDel(&ptimer->entry);
        ptimer->base = NULL;
        /*���뵽����������*/
        ptimer->data.tmnodest    = TIMER_STATE_NULL;

        sys_listAdd(pbase->idleList.prev, &ptimer->entry);

        --pbase->usedCn;
        ++pbase->idelCn;

        /*��ÿ��fid�Ķ�ʱ����������ͳ��*/
        if(NULL != g_pTimerFidInfo && fid < MAX_FID_NUMBER)
        {
            g_pTimerFidInfo[fid]--;
        }
       
        XOS_MutexUnlock(&pbase->lock);
        return XSUCC;
    }
    else if(TIMER_STATE_NULL == ptimer->data.tmnodest)
    {
        /* ��ʱ������ÿ�*/
        ptimer->base = NULL;
        XOS_MutexUnlock(&pbase->lock);
        return XSUCC;
    }

    XOS_MutexUnlock(&pbase->lock);
    
    /*���ӿڵĶ�ʱ��ֻ������״̬���������*/
    XOS_Trace(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStop,its timer state is wrong\n",fid);
    return XERROR;
}

/************************************************************************
������: XOS_TimerGetParam
���ܣ�  ��ȡ��ʱ����param�ṹ��
���룺  tHandle     - ��ʱ�����
�����  ptBackPara  - ��Ҫ��ȡ�ṹ������ָ��
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵��:
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
                  ����ͷ�ļ�
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
                �궨��
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
 ������:TIM_buildHandle
 ����: ���춨ʱ�����
 ����: linkType �� ��ʱ������(����λ)
       linkIndex ����ʱ������
 ���:
 ����:  ���ض�ʱ�����
 ˵��:  ��ʱ�������֯��ͼ��

            pre     checkcoder                  Index
          32     28          20                     1
           -----------------------------------------
           |     |           |                      |
           |     |           |                      |
           -----------------------------------------

************************************************************************/
XSTATIC PTIMER  TIM_buildHandle(e_TIMERPRE pre, XU32 Index)
{
     /*LINK_CHECK_NUM, Ϊ�˶㿪vc�в���ʼ���ľֲ�������Ϊ0xcccc �����*/
     PTIMER  timerhandle;
     XU32 checkNum = TIMER_CHECK_NUM; /*0xbb*/
     timerhandle = (PTIMER)(XPOINT)(((pre&0xf)<<28) |(checkNum<<20) | ((Index&0xfffff)));

     return timerhandle;
}

/************************************************************************
 ������:TIM_buildDHandle
 ����: ���춨ʱ�����
 ����: linkType �� ��ʱ������(����λ)
       linkIndex ����ʱ������
 ���:
 ����:  ���ض�ʱ�����
 ˵��:  ��ʱ�������֯��ͼ��

            pre     checkcoder                  Index
          32     28          20                     1
           -----------------------------------------
           |     |           |                      |
           |     |           |                      |
           -----------------------------------------

************************************************************************/
XSTATIC PTIMER  TIM_buildDHandle(e_TIMERPRE pre, XU32 Index)
{
     /*LINK_CHECK_NUM, Ϊ�˶㿪vc�в���ʼ���ľֲ�������Ϊ0xcccc �����*/
     PTIMER  timerhandle;
     XU32 checkNum = TIMER_CHECK_DNUM; /*0xDD*/
     timerhandle = (PTIMER)(XPOINT)(((pre&0xf)<<28) |(checkNum<<20) | ((Index&0xfffff)));

     return timerhandle;
}

/************************************************************************
 ������:TIM_isValidTHdle
 ����: ��֤��ʱ���������Ч��
 ����: ��ʱ�����
 ���:
 ����: ��Ч����XTURE, ���򷵻�XFALSE
 ˵��:
************************************************************************/
XSTATIC  XBOOL TIM_isValidTHdle( PTIMER timerhandle)
{
     return (TIMER_CHECK_NUM == (((XPOINT)timerhandle>>20)&0xff)? XTRUE:XFALSE);
}

/************************************************************************
 ������:TIM_isValidDTHdle
 ����: ��֤��ʱ���������Ч��
 ����: ��ʱ�����
 ���:
 ����: ��Ч����XTURE, ���򷵻�XFALSE
 ˵��:
************************************************************************/
XSTATIC  XBOOL TIM_isValidDTHdle( PTIMER timerhandle)
{
     return (TIMER_CHECK_DNUM == (((XPOINT)timerhandle>>20)&0xff)? XTRUE:XFALSE);
}

/************************************************************************
 ������:TIM_getTimerIndex
 ����: ͨ����ʱ�������ȡ��ʱ��Index
 ����: ��ʱ�����
 ���:
 ����: ��ʱ������
 ˵��:
************************************************************************/
XSTATIC  XS32 TIM_getTimerIndex( PTIMER timerhandle)
{
     return (XS32)((XPOINT)timerhandle&0x0fffff);
}

/************************************************************************
 ������:TIM_getTimerPre
 ����: ͨ����ʱ�������ȡ��ʱ������
 ����: ��ʱ�����
 ���:
 ����: ��ʱ������(�߾��Ȼ�;���)
 ˵��:
************************************************************************/
XSTATIC  e_TIMERPRE TIM_getTimerPre( PTIMER timerhandle)
{
     return (e_TIMERPRE)((XPOINT)timerhandle>>28);
}

/************************************************************************
������  : LowClock_MsgSnd
����    : �;��ȶ�ʱ����һ����ʱ����ʱ����
����    :
���    : none
����    : XSUCC, ��������ʧ�ܷ���XERROR
˵��    :
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
        /*��ʱ���ڵ��ѵ���*/
        if( pstTmp->NowTime ==0)
        {
            /*ѭ����λ�ڵ�ļ�ʱֵ*/
            pstTmp->NowTime = pstTmp->TimeLen;

            /*����Ӧ��fid����ʱ����Ϣ,��ʾע��ľ��������ѵ�*/
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
         pstTmp = pstTmpNext;    /*ָ������*/
    }
}

/************************************************************************
������  : HighClock_Timeout
����    : �߾��ȶ�ʱ����һ����ʱ����ʱ����
����    :
���    : none
����    : XSUCC, ��������ʧ�ܷ���XERROR ,������ϵͳ�ĸ�ʽ����ͬ
˵��    :
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
     XOS_SemPut(&SysTM.lotsem);/*PC-LINT��ʱ�������жϷ���ֵ������.*/

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
������  : HighClock_Timeout
����    : �߾��ȶ�ʱ����һ����ʱ����ʱ����
����    :
���    : none
����    : XSUCC, ��������ʧ�ܷ���XERROR ,������ϵͳ�ĸ�ʽ����ͬ
˵��    :
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
     XOS_SemPut(&SysTM.hitsem);/*PC-LINT��ʱ�������жϷ���ֵ������.*/

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
������  : XOS_StartSysTimer
����    : ��װÿ������ϵͳ���Դ���ϵͳTIMER,API
����    :
���    : none
����    :
˵��    :��������ϵͳ�漰��timerģ��ı���SysTMÿ��ϵͳ�Լ��ĳ�ʱ������ͬ��
         ��timer������أ����Է�������
         millisecond ����,ǧ��֮һ��
         microsecond ΢��,ǧ��֮һ����(10-3)��
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
    /*ÿ���ticks���ж�*/
    tmr_ticks = sysClkRateGet();

    if(pre == TIMER_PRE_HIGH)
    {
        if (tmr_ticks < 1000/LOC_HIGHTIMER_CLCUNIT)
        {
            XOS_Trace(MD(FID_TIME, PL_DBG), "Clock rate too slow for high presion timer,the high presion may be not accurate!\n");
            return XERROR;
        }
        /*�߾���ÿperiod(ms)��tick��*/
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
        /*�;���ÿperiod(ms)��tick��*/
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
������  : High_TimerTask
����    : �߾��ȶ�ʱ�����ڷ���һ����ʱ��ʱ����Ϣ�����������.
����    :

���    : none
����    :
˵��    :�߾��ȶ�ʱ�����ڷ�����Ϣ������.
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

    /*�ȴ�����ģ�鶼�������ٿ�ʼ����.*/
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
        /*ÿ������̵߳��ȵ�ʱ������20ms������߾��ȶ�ʱ�����᲻׼*/
        t3.tv_usec = ((t1.tv_usec > t2.tv_usec)? t1.tv_usec - t2.tv_usec
                :t1.tv_usec + 1000000 - t2.tv_usec);
        select(1, NULL, NULL, NULL, &t3);
#endif

        for(i=0;i<SysTM.HiTmindex;i++)
        {
            /*����Ӧ��fid����ʱ����Ϣ,��ʾ�߾��ȶ�ʱ���������ѵ�*/
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
 ������:Low_TmerTask
 ����:
 ����:
 ���:
 ����:
 ˵��: �;��ȶ�ʱ�����ڷ���һ����ʱ��ʱ����Ϣ�����������.
************************************************************************/
XPUBLIC XVOID Low_TimerTask( XVOID* ptr)
{

#if(defined(XOS_SOLARIS) ||defined(XOS_LINUX))
    struct timeval t1,t2,t3;
    gettimeofday(&t1, NULL);
#endif

    XOS_UNUSED(ptr);

    /*�ȴ�����ģ�鶼�������ٿ�ʼ����.*/
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
        /*ÿ������̵߳��ȵ�ʱ������100ms������߾��ȶ�ʱ�����᲻׼*/
        t3.tv_usec = ((t1.tv_usec > t2.tv_usec)? t1.tv_usec - t2.tv_usec
                :t1.tv_usec + 1000000 - t2.tv_usec);
        select(1, NULL, NULL, NULL, &t3);
#endif
        LowClock_MsgSnd();

    }

}
/************************************************************************
������  : TIM_LinkCreate
����    : ������ע�ᶨʱ����
����    : pstTimerMngt    -    �������ʱ�����Ľṹָ��
          ulMaxTimerNum   -    ���ܿ�����ʱ����Ŀ
          id              -    ��ʶ
���    : none
����    :
˵��    :
************************************************************************/
XSTATIC XS32 TIM_LinkCreate(XU32 fid, t_TIMERMNGT *pstTimerMngt, XU32 ulMaxTimerNum)
{
    XU32 i;
    if(0 == ulMaxTimerNum ||  XNULLP== pstTimerMngt)
    return XERROR;

    /* ���붨ʱ���ص��ڴ� */
    i = sizeof(t_TIMERNODE) * ulMaxTimerNum;

    //2008-01-21���ڶ�ʱ��ʹ�ø���,�޸�����
    //pstTimerMngt->pstTimerPool = (t_TIMERNODE *)XOS_MemMalloc(FID_TIME, i);
    pstTimerMngt->pstTimerPool = (t_TIMERNODE *)XOS_MemMalloc(fid, i);
    if(XNULL == pstTimerMngt->pstTimerPool)
    {
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nThe memory allocate for timer is failed");
        return XERROR;
    }

    pstTimerMngt->maxtimernum = ulMaxTimerNum;
    pstTimerMngt->nowclock = 0;

    /* ��ʼ���������� */
    for(i = 0; i < LOC_TIMER_LINKLEN; i++)
    {
        CM_INIT_TQ(&pstTimerMngt->stRunList[i]);

    }
    /* ��ʼ���������� */
    CM_INIT_TQ(&pstTimerMngt->idleheader);

    /*�ѿ��еĽڵ��������������*/
    for (i = 0; i < ulMaxTimerNum; i++)
    {
        pstTimerMngt->pstTimerPool[i].tmnodest = TIMER_STATE_NULL;
        CM_INIT_TQ(&pstTimerMngt->pstTimerPool[i].stLe);
        CM_PLC_TQ(pstTimerMngt->idleheader.prev, &pstTimerMngt->pstTimerPool[i].stLe);
    }

    /* ��ʼ���� */
    if ( XSUCC != XOS_MutexCreate(&pstTimerMngt->timerMutex) )
    {
        XOS_Trace(MD(FID_TIME, PL_WARN), "XOS_MutexCreate timerMutex failed! will not be thread-safed .");
    }

    return XSUCC;

}

/************************************************************************
������: XOS_TimerNumReg
���ܣ�  �����ܿ�ע�����ʱ������(�����ڳ�ʼ�������е���)
���룺
        fid            - ���ܿ�ID��
        tmaxtimernum   - ���ܿ�����ʱ����Ŀ
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
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
������: XOS_TimerCreate
���ܣ�  ��ʱ����������
���룺  fid      - ���ܿ�ID��
        tHandle  - ��ʱ�����
        type     - ��ʱ����������ʽ�ͼ���ѡ��
        para     - ��ʱ����ʱ�ش�����
�����  tHandle
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
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
      /*���ﲻ֧��tHandle��������Ϊ���������*/
        XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerCreate,invalid input para ptHandle,it isn't null.\n",fid);
        return XERROR;
    }
    pTimerMngt = MOD_getTimMntByFid(timerpre,fid);
    if(pTimerMngt == XNULLP)
    {
         return XERROR;
    }
    XOS_TIMER_LOCK(&pTimerMngt->timerMutex);
    /*�ӿ��������ҵ�һ���ڵ�*/
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
    /* �ӿ�������ɾ�� */
    CM_RMV_TQ(&pstTmp->stLe);

    TimerpoolIndex = pstTmp - pTimerMngt->pstTimerPool;
    /* ��д��ʱ����Ϣ */
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
������: XOS_TimerBegin
���ܣ�  ��ʱ����������
���룺  tHandle  - ��ʱ�����
        len      - ��ʱ������ʱ���ȣ���λms��
�����  tHandle
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
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
      CM_RMV_TQ(&pstTmp->stLe);/*������������ɾ��*/
      /*20090419 add stat below 8888*/
      pTimerMngt->curNumOfElements--;
      /*20090419 add stat above*/

    }
    pstTmp->para.len   = len;
    pstTmp->para.fid   = fid;
    pstTmp->tmnodest   = TIMER_STATE_RUN;
    pstTmp->walktimes  = 0; /*�Ѿ����Ϲ�����������,�ֱ����������������Ҫ����Ϊ0*/

    /*���ýڵ�ӵ���ʱ������������ */
    ulTimerLinkIndex = (len / pTimerMngt->timeruint + pTimerMngt->nowclock)% LOC_TIMER_LINKLEN;
    CM_PLC_TQ(pTimerMngt->stRunList[ulTimerLinkIndex].prev,&pstTmp->stLe);

    /*20090419 add stat below*/
    /* ����ͳ������ */
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
������: XOS_TimerEnd
���ܣ�  ֹͣ��ʱ��
���룺  ptimer      - ��ʱ�����
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
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
      CM_RMV_TQ(&pstTmp->stLe);/*������������ɾ��*/
      /*20090419 add stat below 8888*/
      tmmanager->curNumOfElements--;
      /*20090419 add stat above*/
    }
    pstTmp->tmnodest    = TIMER_STATE_FREE;
    XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
    return XSUCC;
}

/*******************************************************************
������: XOS_TimerDelete
���ܣ�  ɾ����ʱ�����
���룺  ptimer      - ��ʱ�����
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����  �뺯��XOS_TimerEnd�Ĳ������,����������ɾ����������
һ������ǿ�������,���黹����;
һ���ǹ黹��������
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
      CM_RMV_TQ(&pstTmp->stLe);/*������������ɾ��*/
      /*20090419 add stat below*/
      tmmanager->curNumOfElements--;
      /*20090419 add stat above*/
    }

     /*���뵽����������*/
    pstTmp->tmnodest    = TIMER_STATE_NULL;
    CM_PLC_TQ(tmmanager->idleheader.prev,&pstTmp->stLe);

    XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
    return XSUCC;
}

/*******************************************************************
������: XOS_TimerGetState
���ܣ�  ��ʱ���Ƿ��������жϺ���
���룺  ptimer      - ��ʱ����� 
�����  none
���أ�  ����ö������
˵����
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
������: XOS_TimerStart
���ܣ�  ��ʱ����������
���룺  tHandle     - ��ʱ�����
        timerpara   - ��ʱ������
        backpara    - ��ʱ����ʱ�ش�����

�����  tHandle
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
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
            /*��ֹͣ��ʱ��*/
            XOS_TimerStop(timerpara->fid,ptHandle[0]);
        }
        else
        {
            XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStart,input ptHandle isn't null,but it is invalid.\n",timerpara->fid);
            return XERROR;
        }
    }
#else
    // ������ָ��һƬδ֪�ڴ棬��ô���������أ�
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
    /* �ӿ�������ɾ�� */
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

    /* ���ýڵ�ӵ���ʱ������������ */
    ulTimerLinkIndex = (timerpara->len / tmmanager->timeruint + tmmanager->nowclock)% LOC_TIMER_LINKLEN;
    CM_PLC_TQ(tmmanager->stRunList[ulTimerLinkIndex].prev,&pstTmp->stLe);

    /*20090419 add stat below*/
    /* ����ͳ������ */
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
������: XOS_TimerStop
���ܣ�  ��ʱ��ֹͣ����
���룺  tHandle     - ��ʱ�����

�����
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵����
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
        /* ��ʱ������ÿ�*/
        *(pstTmp->pTimer) = XNULL;
        /*������������ɾ��*/
        CM_RMV_TQ(&pstTmp->stLe);
        pstTmp->tmnodest    = TIMER_STATE_NULL;
        /*���뵽����������*/
        CM_PLC_TQ(tmmanager->idleheader.prev,&pstTmp->stLe);
        /* �������뵽������ */
        /*20090419 add stat below*/
        tmmanager->curNumOfElements--;
        /*20090419 add stat above*/

        XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
        return XSUCC;
    }
    else if(TIMER_STATE_NULL == pstTmp->tmnodest)
    {
        /* ��ʱ������ÿ�*/
        *(pstTmp->pTimer) = XNULL;
        XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
        return XSUCC;
    }

    /*���ӿڵĶ�ʱ��ֻ������״̬���������*/
    XOS_PRINT(MD(FID_TIME, PL_ERR), "\r\nFID %d call XOS_TimerStop,its timer state is wrong\n",fid);

    XOS_TIMER_UNLOCK(&tmmanager->timerMutex);
    return XERROR;

}

/*******************************************************************
������: XOS_TimerRunning
���ܣ�
���룺  ptimer      - ��ʱ�����
�����
���أ�  �����з���XSUCC, �������з���XERROR
˵����
*******************************************************************/
e_TIMESTATE   XOS_TimerRunning(XU32 fid, PTIMER tHandle)
{
    return XOS_TimerGetState(fid, tHandle);
}

/************************************************************************
������  : TIM_ClckProc
����    : �������յ�ʱ��������Ϣ��ͳһ������
����    : management - �������ʱ�����Ľṹָ��
���    : none
����    : XSUCC, ��������ʧ�ܷ���XERROR
˵��    :
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

    /* �̶���ǰ��һ�� */
    management->nowclock = (management->nowclock + 1) % LOC_TIMER_LINKLEN;
    head = &(management->stRunList[management->nowclock]);
    /*��ʼ�����ڵ�����*/
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
        /*��ʱ���ڵ��ѵ���*/
        if (pstTmp->para.len <= i)
        {
            /* �Ӷ�ʱ��������ɾ�� */
            CM_RMV_TQ(&pstTmp->stLe);
            /* ���뵽�������� */
            CM_PLC_TQ(list.prev, &pstTmp->stLe);
        }
        else
        {
            pstTmp->walktimes++;
        }
        pstTmp = pstTmpNext;    /*ָ������*/
    }
    /* ���������� */
    for(pstTmp=(t_TIMERNODE *)list.next; &pstTmp->stLe!=&list; pstTmp=(t_TIMERNODE *)list.next)
    {

        /*�ӵ���������ɾ��*/
        CM_RMV_TQ(&pstTmp->stLe);

        if(pstTmp->para.mode == TIMER_TYPE_LOOP)
        {
            /* �����ѭ����ʱ�� ,���뵽����������*/
            ulTimerLinkIndex = (pstTmp->para.len / management->timeruint + management->nowclock)% LOC_TIMER_LINKLEN;
            pstTmp->walktimes = 0;
            CM_PLC_TQ(management->stRunList[ulTimerLinkIndex].prev,&pstTmp->stLe);
        }
        else
        {
            if(!pstTmp->flag)/*���ӿڵĶ�ʱ������*/
            {
                *(pstTmp->pTimer) = XNULL;  /*һ���Զ�ʱ������ÿ�*/
                CM_PLC_TQ(&(management->idleheader), &(pstTmp->stLe));
                pstTmp->tmnodest = TIMER_STATE_NULL;
            }
            else /*�ĽӿڵĶ�ʱ������*/
            {
                pstTmp->tmnodest = TIMER_STATE_FREE;
            }
            /*20090419 add stat below 8888*/
            management->curNumOfElements--;
            /*20090419 add stat above*/
        }

        /* �ص���Ӧ������ */
        timerExpFunc = MOD_getTimProcFunc(pstTmp->para.fid);
        if(!timerExpFunc )
        {
            XOS_TIMER_UNLOCK(&management->timerMutex);
            return XERROR;
        }
        // ��ֹ�ص�����������ֹͣ��ʱ��
        XOS_TIMER_UNLOCK(&management->timerMutex);
        timerExpFunc( &pstTmp->backpara);
        XOS_TIMER_LOCK(&management->timerMutex);
    }
    XOS_TIMER_UNLOCK(&management->timerMutex);
    return XSUCC;
}

/*******************************************************************
������:
���ܣ�
���룺
�����
���أ�
˵����
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
������:
���ܣ�
���룺
�����
���أ�
˵����
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


