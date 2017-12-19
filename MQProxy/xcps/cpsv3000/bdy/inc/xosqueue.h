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
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/                  
#include "xostype.h"
#include "xoslist.h"
#include "xosencap.h"
#include "xosmodule.h"
#include "xoshash.h"

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                �ṹ��ö������
-------------------------------------------------------------------------*/


typedef struct XOSMSGQ
{
    XOS_HLIST     queueList; 
    t_XOSSEMID   sem;        /* �ź��� */
    t_XOSMUTEXID  queueLock;  /* һ��������Ҫһ���� */
    XU8 prio[eMAXPrio+1]; /*��Ϣ�������ȼ�*/
    XU8 prioCursor;   /*��Ϣ�α꣬�������Ч��*/
    XS32 tskIndex;
    XU8 isFull;         /*�����ж���Ϣ���������Ƿ��������90%��������򲻻��ٴε���XOS_ResetByQueFull����*/
    XBOOL init;           /*�����ж���Ϣ�����Ƿ��ʼ����0:δ��ʼ����1:�ѳ�ʼ��*/
} t_XOSMSGQ;

typedef struct
{
   XCHAR *pMsg;
}t_QUEELEM;


/*tid ���ƿ�*/
typedef struct
{
    XU32 tidNum;  /*�û�ע��������*/
    XCHAR tskName[MAX_TID_NAME_LEN+1];  /*�û�ע�����������*/
    XU32 prio;   /*�������ȼ�*/
    XU32 stackSize;  /*����ջ�ռ��С*/
    t_XOSTASKID tskId;  /*������,����ʱ��*/
    XU32 tskpid;        /* ����pid */
    t_XOSMSGQ recvMsgQue;   /*������Ϣ�Ķ���*/
    XU32  maxMsgsInQue;   /*����������Ϣ����󳤶�*/

    /*��ʱ��*/
    XU32  timerNum;   /*��ʱ���ĸ���*/
}t_TIDCB;

typedef struct
{
    XBOOL isInit;  /*ģ���Ƿ��ʼ��*/
    XOS_HHASH fidHash;  /*����fid��hash��*/
    XOS_HARRAY tidArray;   /*����tid ���ƿ������*/
}t_MODMNT;

/************************************************************************
������: QUE_MsgQCreate
���ܣ�  ������Ϣ����
���룺  maxMsgs����Ϣ������������Ϣ��Ŀ
                   maxMsgLength��ÿ����Ϣ����󳤶�

�����  msgq ��Ϣ���б�ʶ 
���أ� ���������ɹ�����XSUCC, ��������ʧ�ܷ���XERROR
˵���� 
************************************************************************/
XPUBLIC XS32   QUE_MsgQCreate(t_XOSMSGQ *pMsgQ,XU32 maxMsgs) ;


/************************************************************************
������: QUE_MsgQDelete
���ܣ�  ɾ��һ����Ϣ����
���룺  pMsgQ  ��Ϣ���б�ʶ 
�����  N/A
���أ� ���������ɹ�����XSUCC, ��������ʧ�ܷ���XERROR
˵����
************************************************************************/
XPUBLIC XS32  QUE_MsgQDelete(t_XOSMSGQ *pMsgQ);



/************************************************************************
������: QUE_MsgQSend
���ܣ� ��һ����Ϣ���з�����Ϣ
���룺 pQue  ��Ϣ���б�ʶ 
                 pMsg ��Ϣ�������Ľṹָ��
                 prio ������Ϣ�����ȼ�
����� N/A
���أ����������ɹ�����XSUCC, ��������ʧ�ܷ���XERROR
˵����
************************************************************************/
XPUBLIC XS32  QUE_MsgQSend(
                                t_XOSMSGQ *pQue, 
                                t_XOSCOMMHEAD*pMsg, 
                                e_MSGPRIO prio);


/************************************************************************
������: QUE_MsgQRecv
���ܣ�  ��һ����Ϣ���н���һ����Ϣ
���룺  pQue  ��Ϣ���б�ʶ
                  ppMsg  ָ����Ϣ�Ļ������ṹ��ַ��ָ��
����� 
���أ����������ɹ�����XSUCC, ��������ʧ�ܷ���XERROR
˵����
************************************************************************/
XPUBLIC XS32  QUE_MsgQRecv(
                                t_XOSMSGQ *pQue, 
                                t_XOSCOMMHEAD **ppMsg);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*cmqueue.h*/

