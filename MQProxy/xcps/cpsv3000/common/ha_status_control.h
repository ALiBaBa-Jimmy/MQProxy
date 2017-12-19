/******************************************************************************

                  版权所有 (C), 2001-2014, 深圳信威通信技术有限公司

 ******************************************************************************
  文 件 名   : ha_status_control.h
  版 本 号   : 初稿
  作    者   : liujun
  生成日期   : 2014年12月4日
  最近修改   :
  功能描述   : ha_status_control.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月4日
    作    者   : liujun
    修改内容   : 创建文件

******************************************************************************/

#ifndef __HA_STATUS_CONTROL_H__
#define __HA_STATUS_CONTROL_H__ 

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "pub_types.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HA_TUNNEL_NUM               2

/*----------------------------------------------*
 * 数据结构说明                                 *
 *----------------------------------------------*/
/* 状态改变回调函数指针 */
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
    /* 优先级  */
    u8 priority;
    /* 配置状态值 */
    u8 conf_num;
    /* 心跳ip地址对 最多支持2组 网络序 */
    ST_HA_PEER_IP ipinfo[HA_TUNNEL_NUM];
}ST_HA_INIT_PARAM;

typedef struct ha_heart_beat
{
    u8  version;            /*  版本号      */
	u8  type;			    /*  消息类型    */
	u8  status;			    /*  当前状态    */
    u8  conf_num;           /*  配置数据    */
	u16 priority;		    /*  优先级值    */
	u32 ipaddr;			    /*  IP          */
    u32 msg_identy;         /*  消息识别码  */

}ST_HA_HEART_BEAT;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*----------------------------------------------*
 * 对外接口申明                                 *
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
