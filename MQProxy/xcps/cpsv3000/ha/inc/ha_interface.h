/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, ��������ͨ�ż������޹�˾

 ******************************************************************************
  �� �� ��   : ha_interface.h
  �� �� ��   : ����
  ��    ��   : liujun
  ��������   : 2014��12��17��
  ����޸�   :
  ��������   : ha_interface.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��12��17��
    ��    ��   : liujun
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __HA_INTERFACE_H__
#define __HA_INTERFACE_H__

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "xospub.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define XOS_HA_DEAD_WATCH_UNIX_PATH             "/tmp/.XosHaDeadWatchSocket"
#define HA_IFNAME_LEN                           65
/*----------------------------------------------*
 * ���ݽṹ˵��                                 *
 *----------------------------------------------*/

typedef XS32 (*XOS_HA_DEAD_LOCK_CALL)(void *param);
typedef XS32 (*XOS_HA_STATUS_CHANGE_CALL)(XU8 ExStatus, XU8 NewStatus, void *param);

typedef XS32 (*XOS_HA_RES_WATCH_CALL)(XU32 Resource, void *param);

typedef enum XOS_HA_STATUS
{
    XOS_HA_STATUS_INIT      = 1,
    XOS_HA_STATUS_STANDBY   = 2,            
    XOS_HA_STATUS_ACTIVE    = 3,
    XOS_HA_STATUS_CLOSE     = 4
}ENUM_XOS_HA_STATUS;

enum xos_ha_watch_resource_type
{
	XOS_HA_WATCH_CPU = 1,					/* CPUʹ���� ��ʱ��֧�� */
	XOS_HA_WATCH_MEM = 2,					/* �ڴ�ʹ�����   */
	XOS_HA_WATCH_MSG_QUE = 4,				/* ��Ϣ����       */
	XOS_HA_WATCH_ALL = 0xffff
};

typedef struct xos_ha_msg
{
    XU16 MsgType;                   /* ��Ϣ���� */
    XU16 MsgMethod;                 /* ��Ϣ���� */
    XU32 MsgIndent;                 /* ʶ���� */
    XU32 Fid;                       /* ģ��ID */
    XU32 ThreadId;                  /* �߳�ID */
    XU32 TimeStamp;                 /* ʱ��� */
    XU32 Status;                    /* ��ǰ״̬ */
}ST_XOS_HA_MSG;


typedef enum xos_ha_work_mode
{
    XOS_HA_WORK_SINGLE,				/* ����ģʽ */
    XOS_HA_WORK_HA					/* �ȱ�ģʽ */
}ENUM_XOS_HA_WORK_MODE;

typedef struct ha_status_call_info
{
    XU32 Fid;								/* ģ��ID */
    XOS_HA_STATUS_CHANGE_CALL CallBack;		/* �ص����� */
    void *param;							/* ���� */
}ST_HA_STATUS_CALL_INFO;


enum enum_dead_watch_type
{
    HA_WATCH_BASE_XOS,                  /* ����XOSƽ̨�ļ�� */
    HA_WATCH_THREAD,                    /* �����̼߳��      */
    HA_WATCH_END
};

typedef enum ENUM_WATCH_MSG_TYPE
{
    HA_WATCH_MSG_REG,                   /* ע����Ϣ */
    HA_WATCH_MSG_REQ,                   /* �������� */
    HA_WATCH_MSG_RES,                   /* �����ظ� */
    HA_WATCH_MSG_STATUS                 /* ״̬�л� */
}ENUM_WATCH_MSG_TYPE_ENUM;

typedef struct ha_vrip_v4_info
{
    XU32 ipaddr;                         /* ipv4��ַ */
    XU32 mask;                           /* �������� */
    XU32 GateWay;                        /* ����     */
    XS8 LogicIFName[HA_IFNAME_LEN];      /* �߼��ӿ���   */
}ST_HA_VRIP_INFO;

typedef struct ha_watch_report
{
    XU32 Fid;                           /* ģ��ID  */
    XU64 WatchInfo;                     /* ���tag */
    void *param;
    XOS_HA_RES_WATCH_CALL ReportFunc;   /* �ص�����*/
}ST_HA_WATCH_REPORT;

/*oam ��ȡpeer IP API*/
typedef struct ha_peer_oam_info
{
    XU16 usSlotId;         /* ��λ�� */
    t_IPADDR stIpAddr[2];  /* IP��Ϣ */
}ST_HA_PEER_OAM_INFO;

/*----------------------------------------------*
 * �ӿں���ԭ��˵��                             *
 *----------------------------------------------*/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern XU8 XOS_HA_GetCurrentStatus(void);
extern XVOID XOS_HA_ChangeToStandby(void);
extern XS32 XOS_HA_VirtureIpAdd(ST_HA_VRIP_INFO * VitrualIp);
extern XS32 XOS_HA_WorkModeSet(XU8 status);
extern XS32 XOS_HA_StatusChangeRegister(XU32 fid, 
                                    XOS_HA_STATUS_CHANGE_CALL callback, void *param);
extern XS32 XOS_HA_DeadLockReg(XU32 fid, XU32 tid, 
                                    XOS_HA_DEAD_LOCK_CALL CallBack, void *param);
extern XS32 XOS_HA_ResWatchReg(XU32 fid, XU64 Resource, 
                                    XOS_HA_RES_WATCH_CALL callback, void *param);
extern XS32 XOS_HA_ThreadWatchInit(XU32 fid, XU32 tid,
                                    XOS_HA_DEAD_LOCK_CALL CallBack, void *param);
extern XS32 XOS_HA_Init(ST_HA_PEER_OAM_INFO *pstInfo,XS32 InfoLen);
extern XS32 HA_XOSHelloProcess(XU32 fid, void *msg, int msglen);
extern XS32 XOS_HA_DeleteVirIp(XU32 ip);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __HA_INTERFACE_H__ */
