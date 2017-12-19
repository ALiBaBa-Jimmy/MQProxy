/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, ��������ͨ�ż������޹�˾

 ******************************************************************************
  �� �� ��   : ha_status_control.h
  �� �� ��   : ����
  ��    ��   : liujun
  ��������   : 2014��12��4��
  ����޸�   :
  ��������   : ha_status_control.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��12��4��
    ��    ��   : liujun
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __HA_STATUS_CONTROL_H__
#define __HA_STATUS_CONTROL_H__ 

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "pub_types.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HA_TUNNEL_NUM               2

/*----------------------------------------------*
 * ���ݽṹ˵��                                 *
 *----------------------------------------------*/
/* ״̬�ı�ص�����ָ�� */
typedef s32 (*status_change_call_back) (u32 status, void *param);

typedef enum ha_status
{
    HA_STATUS_START  = 0,
    HA_STATUS_INITAL = 1,
    HA_STATUS_STANDBY= 2,
    HA_STATUS_ACTIVE = 3,
    HA_STATUS_CLOSE  = 4
}EN_HA_STATUS;

typedef enum ha_msg_type
{
    HA_MSG_START         = 0,
    HA_MSG_CHANGE_STATUS = 1,
    HA_MSG_KEEP_ALIVE    = 2,
    HA_MSG_ACK           = 3
}EN_HA_MSG_TYPE;

typedef struct ha_peer_ip
{
    u32 local_ip;
    u32 local_port;
    u32 peer_ip;
    u32 peer_port;
}ST_HA_PEER_IP;

typedef struct ha_init_param
{
    /* ���ȼ�  */
    u8 priority;
    /* ����״ֵ̬ */
    u8 conf_num;
    /* ����ip��ַ�� ���֧��2�� ������ */
    ST_HA_PEER_IP ipinfo[HA_TUNNEL_NUM];
}ST_HA_INIT_PARAM;

typedef struct ha_heart_beat
{
    u8  version;            /*  �汾��      */
	u8  type;			    /*  ��Ϣ����    */
	u8  status;			    /*  ��ǰ״̬    */
    u8  conf_num;           /*  ��������    */
	u16 priority;		    /*  ���ȼ�ֵ    */
	u32 ipaddr;			    /*  IP          */
    u32 msg_identy;         /*  ��Ϣʶ����  */

}ST_HA_HEART_BEAT;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*----------------------------------------------*
 * ����ӿ�����                                 *
 *----------------------------------------------*/
typedef void (* STATUS_CHANGE_CALL)(u8 status) ;
extern u8 ha_get_current_status(void);
extern void ha_status_init(void);
extern s32 ha_status_control_init(const ST_HA_INIT_PARAM *param);
extern s32 ha_change_status_cmd(EN_HA_STATUS status);
extern void ha_time_out_process(void);
extern s32 ha_status_change_call_register(status_change_call_back call_func,void *param);
extern inline void ha_status_change_reg(STATUS_CHANGE_CALL callback);
extern void ha_cancel_thread(void);
extern void ha_log_level_set(s32 level);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __HA_STATUS_CONTROL_H__ */
