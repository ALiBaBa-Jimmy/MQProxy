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
                  ����ͷ�ļ�
--------------------------------------------------------------*/ 
#include "xostype.h"
#include "cmtimer.h"
#include "xostrace.h"

/*-------------------------------------------------------------
                  �궨��
--------------------------------------------------------------*/
#define MAX_FID_NAME_LEN      (50)
#define MAX_TID_NAME_LEN      (50)
#define MAX_FID_NUMS          (100)                 /*���fid���ƿ�����*/
#define MAX_TID_NUMS          (MAX_FID_NUMS)        /*���tid���ƿ�����*/

#define XOS_MODULE_STACK_SIZE (1024)  /* ģ��stack��С���� */

#define MAX_MSGS_IN_QUE       (2048)
#define MIN_MSGS_IN_QUE       (1024)
#define MAX_MSGSNUM_IN_QUE    (MIN_MSGS_IN_QUE * 64)


/*-------------------------------------------------------------
                 Ϊ�˱�֤1.81����ⲿע��ӿڲ���
                 ��ʱ �ṹ��ö������
--------------------------------------------------------------*/
/* �ڴ��ͷ����� */
typedef enum
{
    eXOSMode=0,  /*XOS�����ͷţ��ڵ�������Ϣ��������ƽ̨�ͷ���Ϣ�ڴ�*/
    eUserMode,   /*�û�ģʽ��ƽ̨������Ϣ�ڴ�����ͷ�*/
    MAXMode
}e_MEMMODE;


/* FID���ڲ��ṹ */
typedef struct _XOSFIDLIST
{
    struct
    {
        XCHAR   FIDName[MAX_FID_NAME_LEN + 1];         /*FID������*/
        XU32    PID;                            /*FID��Ӧ��PID*/
        XU32    FID;                            /*FID���к�*/
    }head;
    
    struct
    {
        XS8 (*init) (XVOID*, XVOID*);           /*FID�ĳ�ʼ���������*/
        XS8(*notice)(XVOID*, XVOID*);          /*FID��֪ͨ�������*/
        XS8 (*close)(XVOID*, XVOID*);           /*FID�Ĺرպ������*/
    }init;
    
    struct
    {
        XS8 (*message)(XVOID*, XVOID*);         /*FID����Ϣ���������*/
        XS8 (*timer)  (t_BACKPARA*);         /*FID�Ķ�ʱ����ʱ���������*/
    }handle;
    
    e_MEMMODE   memfreetype;
    XU32 *standby;                              /*���ò���ָ��*/
}t_XOSFIDLIST;


/* ע��Ľṹ�� */
typedef struct _XOSLOGINLIST
{
    t_XOSFIDLIST  *stack;       /*FID�Ľṹ*/
    XCHAR         taskname[MAX_TID_NAME_LEN + 1];    /*TID ������*/
    XU32          TID;           /*FID��Ӧ��TID,���Զ��FID��Ӧһ��TID*/
    XU16          prio;          /*TID �����ȼ�  ΪNULL��Ϊϵͳ����*/
    XU32          stacksize;    /*TID�Ķ�ջ��С ΪNULL��ΪĬ��ֵmodified by lixn 20070606 for vxworks*/
    /*XU32          timenum;     */ /*��TID����ע��Ķ�ʱ������*/
    XU32          quenum;      /*��TID����ע�����Ϣ���д�С*/
}t_XOSLOGINLIST;

/*-------------------------------------------------------------
                     �ṹ��ö������
--------------------------------------------------------------*/

/************************************************************************
������:modInitFunc
����:  FID�ĳ�ʼ������
����:  û��������������

���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR 
˵��: Ϊ�˱������еĽӿڲ���,д��������.
************************************************************************/
typedef XS8 (*modInitFunc) (XVOID* sb1 , XVOID* sb2 );


/************************************************************************
������:modMsgProcFunc
����:  FID����Ϣ������
����:
pMsg   ��Ϣͷָ��
sb û��������������
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR 
˵��: Ϊ�˱������еĽӿڲ���,д��������.
************************************************************************/
typedef XS8 (*modMsgProcFunc) (XVOID* pMsg, XVOID* sb);


/************************************************************************
������:modTimerProcFunc
����:  FID�Ķ�ʱ����ʱ������
����:  
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR 
˵��: Ϊ�˱������еĽӿڲ���,д��������.
************************************************************************/
typedef XS8 (*modTimerProcFunc)  (t_BACKPARA*);


/************************************************************************
������:broadcastFilter
����:  �㲥���˺���
����:  
���:
����: ����㲥����XTRUE, ������㲥XFALSE
˵��: Ϊ�����еĶ�ʱ�����,�ṩ�㲥�Ļ���
************************************************************************/
typedef XBOOL (*brdcstFilterFunc)  (XU32 fid);


/************************************************************************
������:setFidTraceInfo
����:  ����fid���ƿ飬���ù��ܿ���ص�trace��Ϣ
����:  
���:
����: ǰһ��fid ������Ϣ���param
˵��: 
************************************************************************/
typedef XVOID* (*setFidTraceInfo)  ( t_FIDTRACEINFO* pFidTraceInfo, XVOID* param,XVOID* param2);


/*-------------------------------------------------------------------------
                          �ӿں���
-------------------------------------------------------------------------*/
/************************************************************************
������: MOD_init
����:  ��ģ��ĳ�ʼ��

����:
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR  
˵��: 
************************************************************************/
XS32 MOD_init(void);


/************************************************************************
������: MOD_startXosFids
����:  ����ƽ̨�����ģ��

����:
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR  
˵��:  �˺�������ģ�����ʱӦ���޸�.
��Ҫ�޸ĵ�, ģ�������˳��ɿ�.
************************************************************************/
XS32 MOD_startXosFids(void);

/************************************************************************
������: MOD_startUserFids
����:  ����ҵ������ģ��

����:
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR  
˵��: 
************************************************************************/
XS32 MOD_startUserFids(void);


/************************************************************************
������: MOD_startNotice
����:  ����֪ͨ���� 
����:
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR  
˵��: Ϊ�˼���1.81�棬�ṩ��ʱ����
************************************************************************/
XS32 MOD_StartNotice(XVOID);


/************************************************************************
������: MOD_getTimMntByFid
���ܣ�  ͨ�����ܿ�Ż�ȡ����Ŀ��ƿ���Ϣ
���룺  fid   ����ģ���
�����  N/A
���أ�  ������ƿ�ָ��,ʧ�ܷ��ؿ�
˵����  
************************************************************************/
XVOID* MOD_getTimMntByFid(e_TIMERPRE pre,XU32 fid);


/************************************************************************
������: MOD_getTimProcFunc
���ܣ�  ͨ�����ܿ�Ż�ȡ���ܿ�ĳ�ʱ������
���룺  fid   ����ģ���
�����  N/A
���أ�  ������ƿ�ָ��,ʧ�ܷ��ؿ�
˵����  
************************************************************************/
modTimerProcFunc MOD_getTimProcFunc(XU32 fid);


/************************************************************************
������: XOS_isValidFid
���ܣ�  �ж�fid�Ƿ���Ч
���룺  fid   ����ģ���
�����  N/A
���أ�  XFALSE OR XTRUE
˵����  
************************************************************************/
XBOOL XOS_isValidFid(XU32 fid);


/************************************************************************
������: XOS_getFidName
���ܣ�  ��ȡ���ܿ���Ϣת�ӿ��ص�ֵ
���룺  fid   ����ģ���
�����  N/A
���أ�  ���ֻ��ָ��
˵����  
************************************************************************/
XPUBLIC XCHAR* XOS_getFidName(XU32 fid);


/************************************************************************
������: MOD_getFidAttSwitch
���ܣ�  ��ȡ���ܿ���Ϣת�ӿ��ص�ֵ
���룺  fid   ����ģ���
�����  N/A
���أ�  XFALSE OR XTRUE
˵����  
************************************************************************/
XPUBLIC XBOOL MOD_getFidAttSwitch(XU32 fid);


/************************************************************************
������: MOD_setFidAttSwitch
���ܣ�  set���ܿ���Ϣת�ӿ��ص�ֵ
���룺
fid   ����ģ���
valve ����״̬����XTRUE , �ر�XFALSE
�����  N/A
���أ�  XFALSE OR XTRUE
˵����  
************************************************************************/
XPUBLIC XS16 MOD_setFidAttSwitch(XU32 fid, XBOOL valve);


/************************************************************************
������: MOD_getFidTraceInfo
���ܣ�  fid ��ص�trce��Ϣ

���룺  fid   ����ģ���
�����  N/A
���أ�  �ɹ�����fid ��ص�trace��Ϣ��ʧ�ܷ���XNULLP
˵����  
************************************************************************/
XPUBLIC t_FIDTRACEINFO* MOD_getFidTraceInfo(XU32 fid);


/************************************************************************
������: MOD_setAllTraceInfo
���ܣ�  �������е�fid ��ص�trace��Ϣ

���룺
setFunc   ���ú���
param    ���ú������������
�����  N/A
���أ�  �ɹ�����XSUCC,  ʧ�ܷ���XERROR
˵����  
************************************************************************/
XPUBLIC XS32 MOD_setAllTraceInfo(setFidTraceInfo setFunc, XVOID* param,XVOID* param2);


/************************************************************************
������: XOS_MsgMemAlloc
���ܣ�  �����贫����Ϣ����һ���ڴ��        
���룺  fid           - ���ܿ�id
nbytes        - ��Ϣ�ĳ���
�����  N/A
���أ�  t_XOSCOMMHEAD * �� �������Ϣ�ڴ�ָ��
˵����  �����ڵ���Ϣ������ʱ��
************************************************************************/
t_XOSCOMMHEAD *XOS_MsgMemMalloc(XU32 fid, XU32 nbytes);


/************************************************************************
������: XOS_MsgMemFree
���ܣ�  �ͷ�һ����Ϣ�ڴ��   
���룺
fid             - ���ܿ�id
t_XOSCOMMHEAD * - ��Ϣ�ڴ��ָ��
�����  N/A
���أ�  t_XOSCOMMHEAD * �� �������Ϣ�ڴ�ָ��
˵���� 
************************************************************************/
XVOID XOS_MsgMemFree(XU32 fid, t_XOSCOMMHEAD *ptr);


/************************************************************************
������: XOS_MsgSend
���ܣ�  ģ���ͨ�ŵ���Ϣ���ͺ���
���룺  pMsg: ��Ϣ����ͷ��Ϣ
�����  N/A
���أ�  XSUCC OR XERROR
˵����  �����û���д����Ϣͷ������Ϣ���͵�Ŀ�ĵ�  
************************************************************************/
XPUBLIC XS32 XOS_MsgSend(t_XOSCOMMHEAD *pMsg);


/************************************************************************
������: XOS_MsgBroadcast
���ܣ�  ��Ϣ�㲥,��ʱ�ṩ���;��ȶ�ʱ����
���룺
pMsg: ��Ϣ����ͷ��Ϣ
brdcstFunc �㲥����,���û���д,�������ƹ㲥��fid
�����  N/A
���أ�  XSUCC OR XERROR
˵����  �����û���д����Ϣͷ������Ϣ���͵�Ŀ�ĵ�  
************************************************************************/
XPUBLIC XS32 XOS_MsgBroadcast(t_XOSCOMMHEAD *pMsg, brdcstFilterFunc brdcstFunc);


/************************************************************************
������: XOS_MMStartFid
���ܣ�  
���룺
�����  N/A
���أ�  XSUCC OR XERROR
˵����  
************************************************************************/
XPUBLIC XS32 XOS_MMStartFid(t_XOSLOGINLIST *,XVOID *,XVOID *);

XS32 XOS_MsgDistribution(XU32 ipcIdx, XU32 remoteIdx, XU8 *p, XS32 len);

XS32 MOD_GetTskPid(XU32 *pid,XU32 fid);

#ifdef XOS_ST_TEST
/************************************************************************
������:    XOS_exeCliCmd

����: ֱ��ִ�еĺ���
����:
���:
����:
˵��:  �û��Լ���֤�ַ���β���и�������
************************************************************************/
XVOID XOS_exeCliCmd(XCHAR* pStr);
#endif 


#ifndef XOS_NEED_OLDTIMER
/*fid ���ƿ�*/
typedef struct
{
    XCHAR fidName[MAX_FID_NAME_LEN+1];  /*���ܿ������*/
    XU32  fidNum;   /*fid ��*/
    XU32  pidIndex;   /*���ܿ������pid ������*/
    XU32  tidIndex;   /*tid  ������*/
    modInitFunc initFunc;   /*ģ���ʼ������*/
    modInitFunc noticeFunc;       /*ģ��֪ͨ����*/
    modMsgProcFunc MsgFunc;   /*ģ�����Ϣ������*/
    modTimerProcFunc timerFunc;   /*��ʱ����ʱ������*/
    e_MEMMODE   msgFreeType;  /* �ͷ���Ϣ������*/
    XBOOL  attSwitch;   /* ��Ϣת�ӿ���*/
    XBOOL  noticeFlag;  /*Notice�Ƿ�֪ͨ��*/
    t_FIDTRACEINFO   traceInfo;   /*trace �������Ϣ*/
    e_PRINTLEVEL logLevel;  /*ģ�����־����*/
}t_FIDCB;
#else
/*fid ���ƿ�*/
typedef struct
{
    XCHAR fidName[MAX_FID_NAME_LEN+1];  /*���ܿ������*/
    XU32  fidNum;   /*fid ��*/
    XU32  pidIndex;   /*���ܿ������pid ������*/
    XU32  tidIndex;   /*tid  ������*/
    modInitFunc initFunc;   /*ģ���ʼ������*/
    modInitFunc noticeFunc;       /*ģ��֪ͨ����*/
    modMsgProcFunc MsgFunc;   /*ģ�����Ϣ������*/
    modTimerProcFunc timerFunc;   /*��ʱ����ʱ������*/
    e_MEMMODE   msgFreeType;  /* �ͷ���Ϣ������*/
    XBOOL  attSwitch;   /* ��Ϣת�ӿ���*/
    XBOOL  noticeFlag;  /*Notice�Ƿ�֪ͨ��*/
    t_FIDTRACEINFO   traceInfo;   /*trace �������Ϣ*/
    t_TIMERMNGT timerMngerLow;  /*�;��ȶ�ʱ������ṹ*/
    t_TIMERMNGT timerMngerHig;      /*�߾��ȶ�ʱ������ṹ*/
    e_PRINTLEVEL logLevel;  /*ģ�����־����*/
}t_FIDCB;
#endif

t_FIDCB* MOD_getFidCb(XU32 fid);
#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _CLISHELL_H_ */
