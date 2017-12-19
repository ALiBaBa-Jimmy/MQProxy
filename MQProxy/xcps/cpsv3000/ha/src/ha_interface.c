/******************************************************************************

                  版权所有 (C), 2001-2014, 深圳信威通信技术有限公司

 ******************************************************************************
  文 件 名   : ha_interface.c
  版 本 号   : 初稿
  作    者   : liujun
  生成日期   : 2014年12月17日
  最近修改   :
  功能描述   : HA的对外接口 主程序文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月17日
    作    者   : liujun
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#ifdef XOS_LINUX

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#endif /* XOS_LINUX */
#include "xostype.h"
#include "xosencap.h"
#include "xostl.h"
#include "xosarray.h"
#include "xosmem.h"
#include "xosipmi.h"
#include "xoslist.h"

#include "ha_resource.h"
#include "ha_interface.h"
#include "ha_deadwatch.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/



/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
#ifdef XOS_LINUX
extern XS32 HA_SetPeerOamInfo(ST_HA_PEER_OAM_INFO *pInfo, XS32 len);
extern XS32 HA_StatusChangeCallAdd(ST_HA_STATUS_CALL_INFO *info);
extern XS32 HA_WatchReportAdd(ST_HA_WATCH_REPORT *info);
#endif /* XOS_LINUX */
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/



/*****************************************************************************
 函 数 名  : XOS_HA_VirtureIpAdd 
 功能描述  : 浮动IP添加接口
 输入参数  : ST_HA_VRIP_INFO * VitrualIp  ip地址为主机序
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 XOS_HA_VirtureIpAdd(ST_HA_VRIP_INFO * VitrualIp)
{
    XS32 ret = 0;
	#ifdef XOS_LINUX
    ST_HA_VRIF_INFO stVitrualIf = {0};
    
    if (NULL == VitrualIp)
    { 
        XOS_Trace(MD(FID_HA, PL_ERR), "Input param is NULL");
        return XERROR;
    }

    stVitrualIf.GateWay = VitrualIp->GateWay;
    stVitrualIf.ipaddr  = VitrualIp->ipaddr;
    stVitrualIf.prefix  = HA_SubMask2Prefix(VitrualIp->mask);
    XOS_StrCpy(stVitrualIf.LogicName, VitrualIp->LogicIFName);

    ret = HA_VirtualIpv4Add(&stVitrualIf);
    if (XSUCC != ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Add vip failed!");
    }
    
    #endif /* XOS_LINUX */

    return ret;

    
}

/*****************************************************************************
 函 数 名  : XOS_HA_DeleteVirIp
 功能描述  : 删除一个浮动IP
 输入参数  : XU32 ip  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月9日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 XOS_HA_DeleteVirIp(XU32 ip)
{
    XS32 ret = 0;
    #ifdef XOS_LINUX
    ret = HA_VirtualIpDel(ip);
    if (XSUCC != ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "del vip failed!");
    }
    #endif /* XOS_LINUX */

    return ret;
}

/*****************************************************************************
 函 数 名  : XOS_HA_ChangeToStandby
 功能描述  : 主动降备接口
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XVOID XOS_HA_ChangeToStandby(void)
{
    #ifdef XOS_LINUX

    HA_ChangeToStandby();

    #endif /* XOS_LINUX */

    return ;
}

/*****************************************************************************
 函 数 名  : XOS_HA_WorkModeSet
 功能描述  : 设置当前HA模式
 输入参数  : u8 status  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月30日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 XOS_HA_WorkModeSet(XU8 status)
{
    #ifdef XOS_LINUX
    if (XOS_HA_WORK_SINGLE == status)
    
    {
        HA_CloseStatusControl();
    }
    else if (XOS_HA_WORK_HA == status)
    {
        HA_StartStatusControl();
    }

    #endif /* XOS_LINUX */
    return XSUCC;
}

/*****************************************************************************
 函 数 名  : XOS_HA_StatusChangeRegister
 功能描述  : 状态切换回调注册
 输入参数  : u32 fid                             
             XOS_HA_STATUS_CHANGE_CALL callback  
             void *param                         
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 XOS_HA_StatusChangeRegister(XU32 fid, 
                                    XOS_HA_STATUS_CHANGE_CALL callback, void *param)
{
    XS32 ret = XSUCC;
    #ifdef XOS_LINUX
    ST_HA_STATUS_CALL_INFO stStatusInfo = {0};
    stStatusInfo.Fid      = fid;
    stStatusInfo.param    = param;
    stStatusInfo.CallBack = callback;
    
    ret = HA_StatusChangeCallAdd(&stStatusInfo);
    if (XSUCC != ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Add status call failed!");
    }
    #endif /* XOS_LINUX */
    return ret; 
}

/*****************************************************************************
 函 数 名  : XOS_HA_GetCurrentStatus
 功能描述  : 获取当前状态
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
             XOS_HA_STATUS_INIT      = 1,
             XOS_HA_STATUS_ACTIVE    = 2,
             XOS_HA_STATUS_STANDBY   = 3,
             XOS_HA_STATUS_CLOSE     = 4
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XU8 XOS_HA_GetCurrentStatus(void)
{
    #ifdef  XOS_LINUX
    return HA_GetCurrentStatus();
    #else
    return 0;
    #endif /*  XOS_LINUX */
}

/*****************************************************************************
 函 数 名  : XOS_HA_DeadLockReg
 功能描述  : 死锁监控注册
 输入参数  : XU32 fid                        
             XU32 tid                        
             XOS_HA_DEAD_LOCK_CALL CallBack  
             void *param                     
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月31日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 XOS_HA_DeadLockReg(XU32 fid, XU32 tid, 
                                    XOS_HA_DEAD_LOCK_CALL CallBack, void *param)
{
    XS32 ret = 0;
    #ifdef XOS_LINUX
    ST_HA_DEAD_LOCK_INFO stWatchInfo = {0};
    
    if (NULL == CallBack)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Call back is NULL!");
        return XERROR;
    }

    stWatchInfo.Fid      = fid;
    stWatchInfo.Tid      = tid;
    stWatchInfo.Type     = HA_WATCH_BASE_XOS;
    stWatchInfo.param    = param;
    stWatchInfo.CallBack = CallBack;
    stWatchInfo.MsgIndent = 0;
	
    ret = HA_DeadWatchAdd(&stWatchInfo);
    if (XSUCC != ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Add Dead Watch failed!");
    }
    #endif /* XOS_LINUX */
    return ret;
}

#ifdef XOS_LINUX
XS32 HA_SocketInit(const XCHAR *path)
{
    XS32 ret = 0;
    XS32 sock = 0;
    XU32 len = 0;
    struct sockaddr_un addr;

    if (NULL == path)
    {
        return XERROR;
    }

    sock = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Init Socket failed:%s!",strerror(errno));
        return XERROR;
    }
    
    /* Make server socket. */ 
    memset (&addr, 0, sizeof (struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy (addr.sun_path, path, strlen (path));
#ifdef HAVE_SUN_LEN
    len = addr.sun_len = SUN_LEN(&addr);
#else
    len = sizeof (addr.sun_family) + strlen (addr.sun_path);
#endif /* HAVE_SUN_LEN */

    ret = connect (sock, (struct sockaddr *) &addr, len);
    if (ret < 0)
    {
        close (sock);
        XOS_Trace(MD(FID_HA, PL_ERR), "Init Socket failed:%s!",strerror(errno));
        return XERROR;
    }
    
    return sock;
}
#endif /* XOS_LINUX */
/*****************************************************************************
 函 数 名  : XOS_HA_ThreadWatchInit
 功能描述  : 线程状态监控初始化 
 输入参数  : XU32 fid                        
             XU32 tid                        
             XOS_HA_DEAD_LOCK_CALL CallBack  
             void *param                     
 输出参数  : 无
 返 回 值  : 失败返回XERROR 
             成功返回socket fd,用户需要监听该套接字收到的消息
                     并将消息 提交给 XOS_HA_HelloProcess 处理
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年12月23日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 XOS_HA_ThreadWatchInit(XU32 fid, XU32 tid,
                                    XOS_HA_DEAD_LOCK_CALL CallBack, void *param)
{
    #ifdef XOS_LINUX
    XS32 ret = 0;
    ST_HA_DEAD_LOCK_INFO stWatchInfo = {0};
    ST_XOS_HA_MSG stRegMsg = {0};
    XS32 socket = 0;

    if (NULL == CallBack)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Call back is NULL!");
        return XERROR;
    }


    stWatchInfo.Fid      = fid;
    stWatchInfo.Tid      = tid;
    stWatchInfo.Type     = HA_WATCH_THREAD;
    stWatchInfo.param    = param;
    stWatchInfo.CallBack = CallBack;
    stWatchInfo.MsgIndent = 0;
    
    ret = HA_DeadWatchAdd(&stWatchInfo);
    if (XSUCC != ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Add Dead Watch failed!");
        goto FAILED;
    }

    socket = HA_SocketInit(XOS_HA_DEAD_WATCH_UNIX_PATH);
    if (XERROR == socket)
    {
        goto FAILED;
    }

    stRegMsg.Fid       = fid;
    stRegMsg.ThreadId  = tid;
    stRegMsg.MsgIndent = 0;
    stRegMsg.MsgMethod = HA_WATCH_THREAD;
    stRegMsg.MsgType   = HA_WATCH_MSG_REG;
    stRegMsg.TimeStamp = time(NULL);
    stRegMsg.Status    = HA_GetCurrentStatus();

    ret = send(socket, &stRegMsg, sizeof(stRegMsg), 0);
    if (ret < 0)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Send Init Dead watch failed!");
        goto FAILED;
    }

    return socket;

FAILED:
    HA_DeadWatchDel(&stWatchInfo);
    if (socket > 0)
    {
        close(socket);
    }
    
    return XERROR;
    #else

    return XSUCC;
    #endif /* XOS_LINUX */
}

/*****************************************************************************
 函 数 名  : XOS_HA_ThreadHelloProcess
 功能描述  : 线程心跳处理
 输入参数  : XU32 fid      
             XU32 tid      
             XS32 *sockfd  
             XU32 TimeOut  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月9日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 XOS_HA_ThreadHelloProcess(XU32 fid, XU32 tid, XS32 *sockfd, XU32 TimeOut)
{
    XS32 ret = XSUCC;
    #ifdef XOS_LINUX
    ret = HA_ThreadHelloProcess(fid,tid,sockfd,TimeOut);
    if (XSUCC != ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Thread Hello Process Failed");
    }
    #endif /* XOS_LINUX */
    return ret;
}

/*****************************************************************************
 函 数 名  : XOS_HA_HelloProcess
 功能描述  : 线程心跳处理
 输入参数  : XU32 fid     
             XVOID *msg   
             XS32 msglen  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月9日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 XOS_HA_HelloProcess(XU32 fid, XVOID *msg,  XS32 msglen)
{
    XS32 ret = XSUCC;
    #ifdef XOS_LINUX
    ret = HA_XOSHelloProcess(fid, msg, msglen);
    if (XSUCC != ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Thread Hello Process Failed");
    }
    #endif /* XOS_LINUX */
    return ret;
}

/*****************************************************************************
 函 数 名  : XOS_HA_ResWatchReg
 功能描述  : 资源监控注册
 输入参数  : XU32 fid                        
             XU64 Resource                   
             XOS_HA_RES_WATCH_CALL callback  
             void *param                     
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月4日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 XOS_HA_ResWatchReg(XU32 fid, XU64 Resource, 
                                    XOS_HA_RES_WATCH_CALL callback, void *param)
{
    XS32 ret = XSUCC;
    #ifdef XOS_LINUX
    ST_HA_WATCH_REPORT stWatchInfo = {0};
    
    stWatchInfo.Fid        = fid;
    stWatchInfo.param      = param;
    stWatchInfo.WatchInfo  = Resource;
    stWatchInfo.ReportFunc = callback;

    ret = HA_WatchReportAdd(&stWatchInfo);
    if (XSUCC != ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Watch report add Failed");
    }
    #endif /* XOS_LINUX */
    return ret;
}

/*****************************************************************************
 函 数 名  : XOS_HA_Init
 功能描述  : HA初始化.若要使用HA功能 需要先调用该函数
 输入参数  : ST_HA_PEER_OAM_INFO *pstInfo  
             入参为:调用OAM的模块OAM_HaInfoGet获得 一个数组
             XS32 InfoLen 为pstInfo的长度       
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年1月4日
    作    者   : liujun
    修改内容   : 新生成函数

*****************************************************************************/
XS32 XOS_HA_Init(ST_HA_PEER_OAM_INFO *pstInfo,XS32 InfoLen)
{
    XS32 ret = XSUCC;
    #ifdef XOS_LINUX
    ret = HA_SetPeerOamInfo(pstInfo, InfoLen);
    if (XSUCC != ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "config set Failed");
    }
    
    #endif /* XOS_LINUX */
    return ret; 
}

