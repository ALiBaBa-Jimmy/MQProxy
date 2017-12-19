/***************************************************************
 **
 **  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
 **
 **  Core Network Department  platform team
 **
 **  filename: cmtimer.h
 **
 **  description: 
 **  �Ľӿ�: ָʹ��һ����ʱ����Ҫʹ���ĸ�API(create,begin,end,delete)
 **  ���ӿ�: ָʹ��һ����ʱ����Ҫʹ������API(start,stop)
 **  ƽ̨�ṩ���׽ӿڸ�ҵ����е��á�
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

#define  TIMER_CHECK_NUM                   (0xbb)   /*�Ľӿ�ģ�͵�У����*/
#define  TIMER_CHECK_DNUM                  (0xdd)   /*���ӿ�ģ�͵�У����*/
#define  MAX_FID_NUMBER                    (2000)   /*ҵ��ģ��fidֵ�ķ�Χ������ͳ�Ƹ���ģ�鶨ʱ������*/

//�¶�ʱ���ľ��ȶ�Ϊ10���룬���ֵ͸߾��ȣ��˺궨��ֻΪ���ϴ���ͳһ
/*�;��ȶ�ʱ��ʱ�Ӿ��ȶ���(100 millisecond)*/
#define LOC_LOWTIMER_CLCUNIT   100
/*�߾��ȶ�ʱ��ʱ�Ӿ��ȶ���(10 millisecond)*/
#define LOC_HIGHTIMER_CLCUNIT 10
/*ʱ�ӿ̶ȼ����ʱ��ÿ�涨ʱ������һ��*/
#define TM_SCALE 10

/* ��ʱ���洢���� */
typedef struct tag_TIMERNODE
{
    PTIMER       timerHandler;   /*��ǰ��ʱ�����,��Ҫ��״̬һ�����ʹ��*/
    t_PARA       para;           /*��ʱ��������,fid,�ߵ;���,��ʱ�����ȵ�*/
    t_BACKPARA   backpara;       /* ��ʱ����ʱ�ش����� */
    e_TIMESTATE  tmnodest;       /*��ʱ��״̬*/
    XBOOL flag;                  /*���ӿڵĶ�ʱ�����ĽӿڵĶ�ʱ�����,1Ϊ�Ľӿڶ�ʱ��.*/
}t_TIMERNODE;

/************************************************************************
������: tm_excuteHighProc
���ܣ�  ���ж��д�����
���룺  
�����  
���أ�  
˵����
************************************************************************/
void tm_excuteHighProc();
void tm_excuteLowProc();

/************************************************************************
 ������:TIM_MsgSnd
 ����: ��ʱ����ʱ����Ϣ���ͳ���
 ����: t_TIMERNODE ���͵Ĳ���
 ���:
 ����: 
 ˵��:
************************************************************************/
XVOID  TIM_MsgSnd(const t_TIMERNODE *data);

/************************************************************************
 ������: ��ʱ��ģ���ʼ������.
 ����:
 ����:
 ���:
 ����:
 ˵��:
************************************************************************/
XS8 TIM_InitTime(XVOID *t, XVOID *v);

/************************************************************************
 ������: ��ʱ��ģ���ʼ������.
 ����:
 ����:
 ���:
 ����:
 ˵��:
************************************************************************/
XS8 TIM_NoticeTime(XVOID *t, XVOID *v);

/************************************************************************
 ������: ����ģ���ʱ�Ӵ�����.
 ����:
 ����:
 ���:
 ����:
 ˵��:
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
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/

#include "xostimer.h"
#include "xosencap.h"
#include "xosport.h"
#include "clishell.h"

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
/*�;��ȶ�ʱ��ʱ�Ӿ��ȶ���(100 millisecond)*/
#define LOC_LOWTIMER_CLCUNIT   100
/*�߾��ȶ�ʱ��ʱ�Ӿ��ȶ���(10 millisecond)*/
#define LOC_HIGHTIMER_CLCUNIT 20

/*��ʱ������С*/
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
                �ṹ��ö������
-------------------------------------------------------------------------*/

/* ˫������ */
typedef struct _t_LISTENT
{
    struct _t_LISTENT  *prev;
    struct _t_LISTENT  *next;
}t_LISTENT;

/* ��ʱ��˫������ڵ�ṹ */
typedef struct tag_TIMERNODE
{
    t_LISTENT    stLe;     /* ˫������ͷ */
    PTIMER *      pTimer;  /* ��ʱ����� */
    t_PARA       para;    /*��ʱ��������,fid,�ߵ;���,��ʱ�����ȵ�*/
    t_BACKPARA   backpara; /* ��ʱ����ʱ�ش����� */
    XU32         walktimes;/* �ڵ㱻�����Ĵ��� */
    e_TIMESTATE  tmnodest; /*��ʱ��״̬*/
    XBOOL flag;   /*���ӿڵĶ�ʱ�����ĽӿڵĶ�ʱ�����,1Ϊ�Ľӿڶ�ʱ��.*/

}t_TIMERNODE;

/* ��ʱ������ṹ */
typedef struct tagTimeMngt
{
    t_LISTENT       stRunList[LOC_TIMER_LINKLEN];   /* ��ʱ���������� */
    t_TIMERNODE     *pstTimerPool;                  /* ��ʱ����ͷָ�� */
    XU32            maxtimernum;                    /* ���ʱ������ */
    t_LISTENT       idleheader;                     /* ��ʱ���ؿ��������ͷ�ڵ� */
    XU32            nowclock;                       /* ��ǰʱ�����ڿ̶� */
    XU32            timeruint;                      /*������ʱ���ľ���*/
    XU32            curNumOfElements;               /*ʹ��ֵ*/
    XU32            maxUsage;                       /*��ʱ���ķ�ֵ*/
    t_XOSMUTEXID    timerMutex;                     /*�ٽ���*/
}t_TIMERMNGT;

typedef struct tag_ParTimerNode
{
    t_LISTENT    stLe;
    XU32 fid;
    XU32 TimeLen;
    XU32 NowTime;
}t_ParTimerNode;

typedef struct tag_ParManage   /*һ����ʱ���Ĺ���ṹ*/
{
    t_ParTimerNode   ParNoArray[FID_MAX];
    t_LISTENT    HeadList;    /*�;��ȶ�ʱ��������ͷ*/
    XU32 HiTmFid[HiTm_FIDNum];
    XU8  HiTmindex;       /*  �߾��ȶ�ʱ��������*/
    t_XOSSEMID hitsem;    /*�߾��ȶ�ʱ���õ��ź���*/
    t_XOSSEMID lotsem;    /*�;��ȶ�ʱ���õ��ź���*/
#ifdef XOS_VXWORKS
    WDOG_ID  wdId;
    XU32   TmrMultiplier;
    WDOG_ID  wdIdlot;           /*�;��ȶ�ʱ����id��*/
    XU32   TmrMultiplierlot;    /*�;��ȶ�ʱ���ı���*/
#endif
    volatile  XBOOL TIME_INTIALIZED;    /*��ʱ��ģ����������б�־*/
}t_ParManage;

/*-------------------------------------------------------------------------
                ȫ�ֱ���
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                API ����
-------------------------------------------------------------------------*/

/************************************************************************
 ������:Low_TmerTask
 ����:   �;�������ʱ����ڴ�����,�߳���ں���
 ����:
 ���:
 ����:
 ˵��: �;��ȶ�ʱ�����ڷ�����Ϣ��������.
************************************************************************/
XPUBLIC XVOID Low_TimerTask( XVOID* ptr);

/************************************************************************
������  : High_TimerTask
����    : �߾�������ʱ����ڴ�����,�߳���ں���
����    :

���    : none
����    :
˵��    :�߾��ȶ�ʱ�����ڷ�����Ϣ��������.
************************************************************************/
XPUBLIC XVOID High_TimerTask(XVOID *ptr);

/************************************************************************
������  : TIM_ClckProc
����    : �������յ�ʱ��������Ϣ��ͳһ������
����    : management - �������ʱ�����Ľṹָ��
���    : none
����    : XSUCC, ��������ʧ�ܷ���XERROR
˵��    :
************************************************************************/
XPUBLIC XS32 TIM_ClckProc(t_TIMERMNGT *management);

/************************************************************************
 ������: ��ʱ��ģ���ʼ������.
 ����:
 ����:
 ���:
 ����:
 ˵��:
************************************************************************/
XPUBLIC XS8 TIM_InitTime(XVOID *t, XVOID *v);

/************************************************************************
 ������: ��ʱ��ģ���ʼ������.
 ����:
 ����:
 ���:
 ����:
 ˵��:
************************************************************************/
XPUBLIC XS8 TIM_NoticeTime(XVOID *t, XVOID *v);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif //XOS_NEED_OLDTIMER
#endif /*_CMROOT_H_*/


