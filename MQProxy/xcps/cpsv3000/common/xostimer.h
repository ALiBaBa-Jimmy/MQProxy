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
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/                  

#include "xostype.h"
/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
#define XOS_INIT_THDLE(handle)   handle = XNULL

XOS_DECLARE_HANDLE(PTIMER); /*��ʱ�����*/



/*-------------------------------------------------------------------------
                �ṹ��ö������
-------------------------------------------------------------------------*/
/*��ʱ�����ⷢ����Ϣ����*/
typedef enum
{
    eMinMsgType = 0,
    eTimeLowClock,
    eTimeHigClock,
    eMaxType
}e_messagetype;


/* ��ʱ�����Ͷ��� */
typedef enum
{
    TIMER_TYPE_LOOP = 0, /* ѭ����ʱ�� */
    TIMER_TYPE_ONCE,    /* ���ζ�ʱ�� */
    TIMER_TYPE_END
}e_TIMERTYPE ;

/* ��ʱ�����ȶ��� */
 typedef enum
{
    TIMER_PRE_HIGH = 0, /* һ�����ȶ�ʱ�� */
    TIMER_PRE_LOW,     /* �������ȶ�ʱ�� */
    TIMER_PRE_END
}e_TIMERPRE;

typedef enum
{
    TIMER_STATE_NULL = 0, /* ��ʱ�����δ����     */
    TIMER_STATE_FREE,     /* ��ʱ��������Ա����� */
    TIMER_STATE_RUN,      /* ��ʱ�������������   */
    
    TIMER_STATE_ERROR
}e_TIMESTATE;

typedef struct 
{
    XU32        fid;    /* ���ܿ�ID   */
    XU32        len;    /* ��ʱ������ */
    e_TIMERTYPE  mode;   /* ������ʱ����ʽ: ѭ�����ѭ�� */
    e_TIMERPRE   pre;    /* ��Ҫ��ʱ������: һ����������߾�����;��ȣ� */

}t_PARA;

typedef struct 
{
#ifdef XOS_ARCH_64
    XU64 para1;
    XU64 para2;
    XU64 para3;
    XU64 para4;
    XU64 count;  /* ��¼��ʱ�����д�������������������; */
#else
    XU32 para1;
    XU32 para2;
    XU32 para3;
    XU32 para4;
    XU64 count;  /* ��¼��ʱ�����д�������������������; */
#endif
}t_BACKPARA;

/*-------------------------------------------------------------------------
                API ����
-------------------------------------------------------------------------*/


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
XS32 XOS_TimerReg(XU32 fid, 
                     XU32 msecnd ,
                     XU32 lowPrecNum, 
                     XU32 highPrecNum);

//#ifdef XOS_TIMER_FOURFUNC

/*******************************************************************
������: XOS_TimerGetState
���ܣ�  ��ʱ���Ƿ��������жϺ���
���룺  ptimer      - ��ʱ����� 
�����  none
���أ�  ����ö������
˵���� 
*******************************************************************/
e_TIMESTATE XOS_TimerGetState(XU32 fid,PTIMER tHandle);    

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
XS32  XOS_TimerCreate(XU32 fid, 
                        PTIMER* tHandle,  
                        e_TIMERTYPE  timertype, 
                        e_TIMERPRE  timerpre, 
                        t_BACKPARA *backpara);


/*******************************************************************
������: XOS_TimerBegin
���ܣ�  ��ʱ����������
���룺  tHandle  - ��ʱ�����
        len      - ��ʱ������ʱ���ȣ���λms��
�����  tHandle
���أ�  XSUCC, ��������ʧ�ܷ���XERROR 
˵����
*******************************************************************/
XS32 XOS_TimerBegin(XU32 fid,PTIMER tHandle, XU32 len);


/*******************************************************************
������: XOS_TimerEnd
���ܣ�  ֹͣ��ʱ��
���룺  ptimer      - ��ʱ����� 
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR 
˵����
*******************************************************************/
XS32 XOS_TimerEnd(XU32 fid,PTIMER tHandle);

/*******************************************************************
������: XOS_TimerDelete
���ܣ�  ɾ����ʱ�����
���룺  ptimer      - ��ʱ����� 
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR 
˵����
*******************************************************************/
XS32 XOS_TimerDelete(XU32 fid,PTIMER tHandle);
//#endif
/************************************************************************
������: XOS_TimerIsRuning
���ܣ�  ��ʱ���Ƿ��������жϺ���
���룺  ptimer      - ��ʱ����� 
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR 
˵����
************************************************************************/
e_TIMESTATE   XOS_TimerRunning(XU32 fid, PTIMER tHandle);


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
XS32 XOS_TimerStart(PTIMER *tHandle, 
                      t_PARA *timerpara, 
                      t_BACKPARA *backpara);

/************************************************************************
������: XOS_TimerStop
���ܣ�  ֹͣ��ʱ��
���룺  ptimer      - ��ʱ����� 
�����  none
���أ�  XSUCC, ��������ʧ�ܷ���XERROR 
˵����
************************************************************************/
XS32 XOS_TimerStop(XU32 fid, PTIMER tHandle);

/************************************************************************
������: XOS_TimerGetParam
���ܣ�  ��ȡ��ʱ����param�ṹ��
���룺  tHandle     - ��ʱ�����
�����  ptBackPara  - ��Ҫ��ȡ�ṹ������ָ��
���أ�  XSUCC, ��������ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 XOS_TimerGetParam(PTIMER tHandle,t_BACKPARA *ptBackPara);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*demon.h*/

