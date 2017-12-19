/******************************************************************************

                  版权所有 (C), 2001-2014, 深圳信威通信技术有限公司

 ******************************************************************************
  文 件 名   : ha_interface.h
  版 本 号   : 初稿
  作    者   : liujun
  生成日期   : 2014年12月17日
  最近修改   :
  功能描述   : ha_interface.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月17日
    作    者   : liujun
    修改内容   : 创建文件

******************************************************************************/

#ifndef __HA_INTERFACE_H__
#define __HA_INTERFACE_H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "xospub.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define XOS_HA_DEAD_WATCH_UNIX_PATH             "/tmp/.XosHaDeadWatchSocket"
#define HA_IFNAME_LEN                           65
/*----------------------------------------------*
 * 数据结构说明                                 *
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
	XOS_HA_WATCH_CPU = 1,					/* CPU使用率 暂时不支持 */
	XOS_HA_WATCH_MEM = 2,					/* 内存使用情况   */
	XOS_HA_WATCH_MSG_QUE = 4,				/* 消息队列       */
	XOS_HA_WATCH_ALL = 0xffff
};

typedef struct xos_ha_msg
{
    XU16 MsgType;                   /* 消息类型 */
    XU16 MsgMethod;                 /* 消息方法 */
    XU32 MsgIndent;                 /* 识别码 */
    XU32 Fid;                       /* 模块ID */
    XU32 ThreadId;                  /* 线程ID */
    XU32 TimeStamp;                 /* 时间戳 */
    XU32 Status;                    /* 当前状态 */
}ST_XOS_HA_MSG;


typedef enum xos_ha_work_mode
{
    XOS_HA_WORK_SINGLE,				/* 单机模式 */
    XOS_HA_WORK_HA					/* 热备模式 */
}ENUM_XOS_HA_WORK_MODE;

typedef struct ha_status_call_info
{
    XU32 Fid;								/* 模块ID */
    XOS_HA_STATUS_CHANGE_CALL CallBack;		/* 回调函数 */
    void *param;							/* 参数 */
}ST_HA_STATUS_CALL_INFO;


enum enum_dead_watch_type
{
    HA_WATCH_BASE_XOS,                  /* 基于XOS平台的监控 */
    HA_WATCH_THREAD,                    /* 其他线程监控      */
    HA_WATCH_END
};

typedef enum ENUM_WATCH_MSG_TYPE
{
    HA_WATCH_MSG_REG,                   /* 注册消息 */
    HA_WATCH_MSG_REQ,                   /* 心跳请求 */
    HA_WATCH_MSG_RES,                   /* 心跳回复 */
    HA_WATCH_MSG_STATUS                 /* 状态切换 */
}ENUM_WATCH_MSG_TYPE_ENUM;

typedef struct ha_vrip_v4_info
{
    XU32 ipaddr;                         /* ipv4地址 */
    XU32 mask;                           /* 子网掩码 */
    XU32 GateWay;                        /* 网关     */
    XS8 LogicIFName[HA_IFNAME_LEN];      /* 逻辑接口名   */
}ST_HA_VRIP_INFO;

typedef struct ha_watch_report
{
    XU32 Fid;                           /* 模块ID  */
    XU64 WatchInfo;                     /* 监控tag */
    void *param;
    XOS_HA_RES_WATCH_CALL ReportFunc;   /* 回调函数*/
}ST_HA_WATCH_REPORT;

/*oam 获取peer IP API*/
typedef struct ha_peer_oam_info
{
    XU16 usSlotId;         /* 槽位号 */
    t_IPADDR stIpAddr[2];  /* IP信息 */
}ST_HA_PEER_OAM_INFO;

/*----------------------------------------------*
 * 接口函数原型说明                             *
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
