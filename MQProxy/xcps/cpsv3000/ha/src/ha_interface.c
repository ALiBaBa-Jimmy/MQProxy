/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, ��������ͨ�ż������޹�˾

 ******************************************************************************
  �� �� ��   : ha_interface.c
  �� �� ��   : ����
  ��    ��   : liujun
  ��������   : 2014��12��17��
  ����޸�   :
  ��������   : HA�Ķ���ӿ� �������ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��12��17��
    ��    ��   : liujun
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
 * �궨��                                       *
 *----------------------------------------------*/



/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
#ifdef XOS_LINUX
extern XS32 HA_SetPeerOamInfo(ST_HA_PEER_OAM_INFO *pInfo, XS32 len);
extern XS32 HA_StatusChangeCallAdd(ST_HA_STATUS_CALL_INFO *info);
extern XS32 HA_WatchReportAdd(ST_HA_WATCH_REPORT *info);
#endif /* XOS_LINUX */
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/



/*****************************************************************************
 �� �� ��  : XOS_HA_VirtureIpAdd 
 ��������  : ����IP��ӽӿ�
 �������  : ST_HA_VRIP_INFO * VitrualIp  ip��ַΪ������
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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
 �� �� ��  : XOS_HA_DeleteVirIp
 ��������  : ɾ��һ������IP
 �������  : XU32 ip  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��9��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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
 �� �� ��  : XOS_HA_ChangeToStandby
 ��������  : ���������ӿ�
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XVOID XOS_HA_ChangeToStandby(void)
{
    #ifdef XOS_LINUX

    HA_ChangeToStandby();

    #endif /* XOS_LINUX */

    return ;
}

/*****************************************************************************
 �� �� ��  : XOS_HA_WorkModeSet
 ��������  : ���õ�ǰHAģʽ
 �������  : u8 status  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��30��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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
 �� �� ��  : XOS_HA_StatusChangeRegister
 ��������  : ״̬�л��ص�ע��
 �������  : u32 fid                             
             XOS_HA_STATUS_CHANGE_CALL callback  
             void *param                         
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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
 �� �� ��  : XOS_HA_GetCurrentStatus
 ��������  : ��ȡ��ǰ״̬
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
             XOS_HA_STATUS_INIT      = 1,
             XOS_HA_STATUS_ACTIVE    = 2,
             XOS_HA_STATUS_STANDBY   = 3,
             XOS_HA_STATUS_CLOSE     = 4
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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
 �� �� ��  : XOS_HA_DeadLockReg
 ��������  : �������ע��
 �������  : XU32 fid                        
             XU32 tid                        
             XOS_HA_DEAD_LOCK_CALL CallBack  
             void *param                     
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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
 �� �� ��  : XOS_HA_ThreadWatchInit
 ��������  : �߳�״̬��س�ʼ�� 
 �������  : XU32 fid                        
             XU32 tid                        
             XOS_HA_DEAD_LOCK_CALL CallBack  
             void *param                     
 �������  : ��
 �� �� ֵ  : ʧ�ܷ���XERROR 
             �ɹ�����socket fd,�û���Ҫ�������׽����յ�����Ϣ
                     ������Ϣ �ύ�� XOS_HA_HelloProcess ����
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��23��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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
 �� �� ��  : XOS_HA_ThreadHelloProcess
 ��������  : �߳���������
 �������  : XU32 fid      
             XU32 tid      
             XS32 *sockfd  
             XU32 TimeOut  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��9��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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
 �� �� ��  : XOS_HA_HelloProcess
 ��������  : �߳���������
 �������  : XU32 fid     
             XVOID *msg   
             XS32 msglen  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��9��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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
 �� �� ��  : XOS_HA_ResWatchReg
 ��������  : ��Դ���ע��
 �������  : XU32 fid                        
             XU64 Resource                   
             XOS_HA_RES_WATCH_CALL callback  
             void *param                     
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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
 �� �� ��  : XOS_HA_Init
 ��������  : HA��ʼ��.��Ҫʹ��HA���� ��Ҫ�ȵ��øú���
 �������  : ST_HA_PEER_OAM_INFO *pstInfo  
             ���Ϊ:����OAM��ģ��OAM_HaInfoGet��� һ������
             XS32 InfoLen ΪpstInfo�ĳ���       
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

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

