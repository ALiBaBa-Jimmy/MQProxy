/******************************************************************************

                  版权所有 (C), 2001-2014, 深圳信威通信技术有限公司

 ******************************************************************************
  文 件 名   : ha_deadwatch.h
  版 本 号   : 初稿
  作    者   : liujun
  生成日期   : 2014年12月29日
  最近修改   :
  功能描述   : ha_deadwatch.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月29日
    作    者   : liujun
    修改内容   : 创建文件

******************************************************************************/
#ifdef XOS_LINUX

#ifndef __HA_DEADWATCH_H__
#define __HA_DEADWATCH_H__
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "xoslist.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HA_DEAD_SUCCECC                     0
#define HA_DEAD_ERR                         -1
#define HA_DEAD_ERR_ACCEPT                  -2

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
 
typedef struct ha_dead_lock_info
{
    XU32 Fid;
    XU32 Tid;
    XU32 Type;
    XU32 MsgIndent;
    XS32 Sockfd;
    void *param;
    void *pEpollBuf;
    XOS_HA_DEAD_LOCK_CALL CallBack;
}ST_HA_DEAD_LOCK_INFO;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/




#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern XS32 HA_ClientReConnect(XU32 fid, XU32 tid);
extern XS32 HA_DeadWatchAdd(ST_HA_DEAD_LOCK_INFO *info);

extern XS32 HA_DeadWatchDel(ST_HA_DEAD_LOCK_INFO *info);
extern XS8 HA_DeadWatchInit(void);

extern XS32 HA_DeadWatchUpdateMsgId(XU32 Fid, XU32 Tid, XU32 MsgId);
extern XS32 HA_DeadWatchUpdateSock(XU32 Fid, XU32 Tid, XS32 SockFd);
extern XS32 HA_GetIndexFromHashKey(XU32 ulHashKey, XU32 *Index);
extern void HA_ParseHeartBeatMsgFromQue(t_XOSCOMMHEAD *pMsg);
extern XS32 HA_ThreadHelloProcess(XU32 fid, XU32 tid, XS32 *sockfd, XU32 TimeOut);
extern XS32 HA_XOSHelloProcess(XU32 fid, void *msg, int msglen);
extern void HA_DeadWatchDestroy(void);
extern XVOID HA_DeadWatchDefaultOpen(XVOID);
extern XVOID HA_DeadWatchDefaultClose(XVOID);
extern void HA_RecvHeartBeatMsg(XOS_HLIST ListHead);
extern void HA_SendMsgToWatchMod(XOS_HLIST ListHead);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __HA_DEADWATCH_H__ */

#endif /* XOS_LINUX */