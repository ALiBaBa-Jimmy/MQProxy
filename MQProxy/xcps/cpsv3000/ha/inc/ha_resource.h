/******************************************************************************

                  版权所有 (C), 2001-2014, 深圳信威通信技术有限公司

 ******************************************************************************
  文 件 名   : ha_resource.h
  版 本 号   : 初稿
  作    者   : liujun
  生成日期   : 2014年12月29日
  最近修改   :
  功能描述   : ha_resource.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月29日
    作    者   : liujun
    修改内容   : 创建文件

******************************************************************************/
#ifdef XOS_LINUX
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "xostimer.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HA_NAME_LEN                             72
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 数据结构原型说明                             *
 *----------------------------------------------*/
typedef enum EN_TIMER_TYPE
{
    HA_TIMER_TYPE_ARP,
    HA_TIMER_TYPE_STATUS,
    HA_TIMER_TYPE_DEAD_WATCH,
    HA_TIMER_TYPE_DEAD_RECV
    
}ENUM_TIMER_TYPE;

typedef struct ha_vrif_info
{
    XU32 ipaddr;                         /* ipv4地址   */
    XU32 prefix;                         /* 子网掩码   */
    XU32 GateWay;                        /* 网关       */
    XS8 LogicName[HA_NAME_LEN];          /* 逻辑接口名 */
    XS8 IFName[HA_NAME_LEN];             /* 接口名     */
    XS8 ViIfName[HA_NAME_LEN];           /* 虚拟接口名 */
}ST_HA_VRIF_INFO;

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

#ifndef __HA_RESOURCE_H__
#define __HA_RESOURCE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern XS8 Ha_AgentNfLoad(void);
extern XS8 HA_BroadcastFreeArp(const XCHAR* pIfName, XU32 ulIp, XU8 count);
extern void HA_CallAllModStatusChange(XU8 ExStatus, XU8 NewStatus);
extern void HA_ChangeStautsCallBack(XU8 status);
extern XS32 HA_ChangeToStandby(void);
extern void HA_CloseStatusControl(void);
extern XS32 HA_CommandReg(XVOID);
extern XS32 HA_ConfigVirtualIP(XU32 ipaddr,XS32 status);
extern XS32 HA_CreateVipInterface(ST_HA_VRIF_INFO *pInterFace);
extern XU8 HA_GetCurrentStatus(void);
extern XBOOL HA_JusticeToChange(XU32 ResourceType);
extern XS8 HA_MakeArpPacket(XU8 *buf, const XU8 *ifName, XU16 option, XU32 ip);
extern void HA_MemRunOutCall(void);
extern void HA_MsgFullCall(void);
extern XS8 HA_MsgProc(XVOID* pMsgP, XVOID* para);
extern void HA_ResourceDestroy(void);
extern XS8 HA_ResourceInit(XVOID *p1, XVOID *p2);
extern void HA_SendAllVipFreeArp(void);
extern void HA_SendStatusToAllMod(XU8 status);
extern void HA_SetCurrentStatus(XU8 Status);
extern XVOID HA_ShowCurrentConfig(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
extern XVOID HA_ShowVitualIp(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
extern XVOID HA_ShowWatchReport(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
extern XS32 HA_StartStatusControl(void);
extern XS32 HA_StatusChangeCallLstInit(void);
extern XS8 HA_TimerProc( t_BACKPARA* pParam);
extern XBOOL HA_VipListCmpFunc(nodeType element1, XVOID *param);
extern XS32 HA_VipLstInit(void);
extern XVOID HA_VirtualIpDelAll(XVOID);
extern XS32 HA_VirtualIpv4Add(ST_HA_VRIF_INFO *VipInfo);
extern XS32 HA_VirtualIpv4AddToList(ST_HA_VRIF_INFO *VipInfo);
extern XBOOL HA_WatchReportCmp(nodeType element1, XVOID *param);
extern XS32 HA_WatchReportLstInit(void);
extern XS32  XOS_FIDHA(HANDLE hDir,XS32 argc, XS8** argv);
extern XVOID HA_ShowDeadWatchInfo(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
extern XS32 HA_VirtualIpDel(XU32 ipaddr);
extern XU8 HA_SubMask2Prefix(XU32 netmask);
extern XU32 HA_Prefix2SubMask(XU8 prefix) ;
extern void HA_ChangeStatusCallBack(XU8 status);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __HA_RESOURCE_H__ */
#endif /* XOS_LINUX */
