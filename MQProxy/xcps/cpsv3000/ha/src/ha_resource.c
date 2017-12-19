/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, ��������ͨ�ż������޹�˾

 ******************************************************************************
  �� �� ��   : ha_resource.c
  �� �� ��   : ����
  ��    ��   : liujun
  ��������   : 2014��12��17��
  ����޸�   :
  ��������   : ��Դ����ģ��ʵ��
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

#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>

#include "xostype.h"
#include "xosencap.h"
#include "xostl.h"
#include "xosarray.h"
#include "xosmem.h"
#include "trace_agent.h"
#include "xosipmi.h"
#include "xoslist.h"
#include "xosmodule.h"
#include "xosnetinf.h"
#include "xosxml.h"
#include "xmlparser.h"

#include "ha_resource.h"
#include "ha_interface.h"

#include "ha_status_control.h"
#include "ha_deadwatch.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HA_VITURE_IP_MAX                     255
#define HA_WATCH_REPORT_MAX                  255

#define HA_CHANGE_CALL_MAX                   255

#define HA_TIMER_NUMBER                      4
#define HA_ENABLE_VIP                        0
#define HA_DISABLE_VIP                       1
#define HA_MAX_CMD_LEN                       (128)
#define HA_STATUS_PRIO_ACTIVE                100
#define HA_STATUS_PRIO_STANDBY               50
#define HA_PEER_IP_MAX_NUM                   HA_TUNNEL_NUM
/* ״̬���� ����ʱ�� ���� */
#define HA_KEEP_ALIVE_TIME                   10000
/* arp �㲥Ϊ2��1��  */
#define HA_FREE_ARP_SEND_TIME                2000
#define HA_PEER_NUM                          2
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

typedef struct ha_peer_ip_info
{
    XU32 LocalIp;
    XU32 PeerIp;
}ST_HA_PEER_IP_INFO;

/*arp Э���ֶ�*/
typedef struct
{
   XU8  dst_mac[6];
   XU8  src_mac[6];   
   XU16 proto;
   XU16 hw_type;
   XU16 proto_type;
   XU8  hw_len;
   XU8  proto_len;
   XU16 option;
   XU8  sender_hdr[6];
   XU32 sender_ip;
   XU8  target_hdr[6];
   XU32 target_ip;
}ST_ARP_PACKET;
/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
extern RESET_PROC_FUNC pMemProcFunc;
extern RESET_PROC_FUNC pRestProcFunc;
extern t_XOSFIDLIST XOS_HA;
/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static XOS_HLIST g_VirtualIpList     = XNULL;
static XOS_HLIST g_WatchReportList   = XNULL;
static XOS_HLIST g_StatusChangeCall  = XNULL;

static XU8  g_HaCurrentStatus    = HA_STATUS_INITAL;
static PTIMER g_pArpSendTimer    = XNULL;
/*static PTIMER g_pStatusControlTimer = XNULL;*/

static ST_HA_INIT_PARAM g_stConfig;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

XU8 HA_SubMask2Prefix(XU32 netmask) 
{
    XU8 prefix = 32;
    
    netmask = ~netmask;
    while (0 != (netmask & 0x01)) 
    {
        netmask >>= 1;
        prefix--;
    }
    
    return prefix;
}

XU32 HA_Prefix2SubMask(XU8 prefix) 
{
    XU32 mask = 0xffffffff;
    
    prefix = 32 - prefix;
    for (;prefix > 0; prefix--) 
    {
        mask <<= 1;
    }
    
    return mask;
}

void HA_SetCurrentStatus(XU8 Status)
{
    if (Status <= HA_STATUS_CLOSE)
    {
        g_HaCurrentStatus = Status;
    }
}

XU8 HA_GetCurrentStatus(void)
{
    return g_HaCurrentStatus;
}

/*****************************************************************************
 �� �� ��  : HA_SendXosMsg
 ��������  : ������ģ�鷢��״̬��Ϣ
 �������  : XU32 DstFid                  
             XU32 SrcFid                  
             XU16 MsgId                   
             ST_XOS_HA_MSG *pstStatusMsg  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��6��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_SendXosMsg(XU32 DstFid, XU32 SrcFid, XU16 MsgId, 
                                                    ST_XOS_HA_MSG *pstStatusMsg)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS32 ret = XERROR;

    if (NULL == pstStatusMsg)
    {
        XOS_Trace(FILI, FID_HA, PL_ERR, "pstStatusMsg is null");
        return XERROR;
    }
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_HA, sizeof(ST_XOS_HA_MSG));
    if(XNULL == pMsg) 
    {
        XOS_Trace(FILI, FID_HA, PL_ERR, "msg is null");
        return XERROR;
    }
    
    pMsg->datasrc.PID  = XOS_GetLocalPID();
    pMsg->datasrc.FID  = SrcFid;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = DstFid;
    pMsg->prio  = eAdnMsgPrio;
    pMsg->msgID = MsgId;
    pMsg->message = (XCHAR *)(pMsg) + sizeof(t_XOSCOMMHEAD);
    XOS_MemCpy(pMsg->message, pstStatusMsg, sizeof(ST_XOS_HA_MSG));

    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        /*����ָʾ��Ϣ��Ӧ�����ͷ��յ�����*/
        XOS_MsgMemFree(FID_HA, pMsg);
        XOS_Trace(FILI, FID_HA, PL_ERR, "ERROR: status change msg send to %d failed.",DstFid);
        return XERROR;
    }
    
    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_SendMsgToModule
 ��������  : ��ģ�鷢��״̬��Ϣ
 �������  : XU32 DstFid                  
             XU16 MsgId                   
             ST_XOS_HA_MSG *pstStatusMsg  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��6��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_SendMsgToModule(XU32 DstFid,XU16 MsgId, ST_XOS_HA_MSG *pstStatusMsg)
{    
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    if (NULL == pstStatusMsg)
    {
        XOS_Trace(FILI, FID_HA, PL_ERR, "pstStatusMsg is null");
        return XERROR;
    }

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_HA, sizeof(ST_XOS_HA_MSG));
    
    if(XNULL != pMsg) 
    {
        pMsg->datasrc.PID  = XOS_GetLocalPID();
        pMsg->datasrc.FID  = FID_HA;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = DstFid;
        pMsg->msgID = HA_WATCH_MSG_REQ;
        pMsg->prio  = eAdnMsgPrio;
        pMsg->msgID = MsgId;
        pMsg->message = ((XCHAR *)pMsg) + sizeof(t_XOSCOMMHEAD);
        
        XOS_MemCpy(pMsg->message, pstStatusMsg, sizeof(ST_XOS_HA_MSG));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*����ָʾ��Ϣ��Ӧ�����ͷ��յ�����*/
            XOS_MsgMemFree(FID_HA, pMsg);
            XOS_Trace(FILI, FID_HA, PL_ERR, "ERROR: status change msg send to %d failed.",DstFid);
            return XERROR;
        }
        return XSUCC;
    }

    XOS_Trace(FILI, FID_HA, PL_ERR,"ERROR:msg mem malloc failed!");
    return ret;
}

/*****************************************************************************
 �� �� ��  : HA_SendStatusToAllMod
 ��������  : ������ģ�鷢��״̬�л���Ϣ
 �������  : XU8 status  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��25��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_SendStatusToAllMod(XU8 status)
{
    ST_XOS_HA_MSG stStatusMsg = {0};
    ST_HA_STATUS_CALL_INFO *pCallInfo = NULL;
    XS32 nodeIndex = 0;

    stStatusMsg.MsgMethod = HA_WATCH_BASE_XOS;
    stStatusMsg.MsgType   = HA_WATCH_MSG_STATUS;
    stStatusMsg.Status    = status;

    nodeIndex = XOS_listHead(g_StatusChangeCall);
    while (XERROR != nodeIndex)
    {
        pCallInfo = XOS_listGetElem(g_StatusChangeCall, nodeIndex);
        if (NULL != pCallInfo)
        {
            stStatusMsg.Fid = pCallInfo->Fid;
            HA_SendMsgToModule(pCallInfo->Fid, HA_WATCH_MSG_STATUS, &stStatusMsg);
        }
        nodeIndex = XOS_listNext(g_StatusChangeCall, nodeIndex);
    }
}


/*****************************************************************************
 �� �� ��  : HA_CallAllModStatusChangeFunc
 ��������  : �ص�ע��ģ���״̬�л�����
 �������  : XU8 status  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��25��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_CallAllModStatusChange(XU8 ExStatus, XU8 NewStatus)
{
    ST_HA_STATUS_CALL_INFO *pCallInfo = NULL;
    XS32 nodeIndex = 0;
    
    nodeIndex = XOS_listHead(g_StatusChangeCall);
    while (XERROR != nodeIndex)
    {
        pCallInfo = XOS_listGetElem(g_StatusChangeCall, nodeIndex);
        if (NULL != pCallInfo)
        {
            pCallInfo->CallBack(ExStatus, NewStatus, pCallInfo->param);
        }
        nodeIndex = XOS_listNext(g_StatusChangeCall, nodeIndex);
    }
}

/*****************************************************************************
 �� �� ��  : HA_MakeArpPacket
 ��������  : ����arp����
 �������  : XU8 *buf           
             const XU8 *ifName  
             XU16 option        
             XU32 ip            
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS8 HA_MakeArpPacket(XU8 *buf, const XU8 *ifName, XU16 option, XU32 ip)
{
    ST_ARP_PACKET *pArpMsg = NULL;
    XU8 mac[6] = {0};

    if(NULL == buf)
    {
        return XERROR;    
    }

    pArpMsg = (ST_ARP_PACKET *)buf;

    XINET_GetMac((XU8*)ifName, (XU8*)mac);
    XOS_MemCpy(pArpMsg->src_mac, mac, 6);
    XOS_MemSet(pArpMsg->dst_mac, 0xff, 6);
    pArpMsg->proto      = XOS_HtoNs(ETH_P_ARP);
    pArpMsg->hw_type    = XOS_HtoNs(ARPHRD_ETHER);
    pArpMsg->proto_type = XOS_HtoNs(ETH_P_IP);
    pArpMsg->hw_len     = 6;
    pArpMsg->proto_len  = 4;
    pArpMsg->option     = XOS_HtoNs(option);
    pArpMsg->sender_ip  = XOS_HtoNl(ip);
    pArpMsg->target_ip  = XOS_HtoNl(ip);
    XOS_MemCpy(pArpMsg->sender_hdr, mac, 6);

    if (ARPOP_REQUEST == option)
    {
        XOS_MemSet(pArpMsg->target_hdr, 0, 6);
    }
    else
    {
        XOS_MemCpy(pArpMsg->target_hdr, mac, 6);
    }

    return sizeof(ST_ARP_PACKET);
}

/*****************************************************************************
 �� �� ��  : HA_BroadcastFreeArp
 ��������  : ����ip�������arp
 �������  : const XCHAR* pIfName  
             XU32 ulIp             
             XU8 count             
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS8 HA_BroadcastFreeArp(const XCHAR* pIfName, XU32 ulIp, XU8 count)
{
    int sockfd;
    struct sockaddr_ll sll;
    unsigned char buf[64] = {0};
    XS32 len = 0;
    XS32 n = 0;
    XU32 i = 0;
    struct ifreq ifinfo;

    if (NULL == pIfName || 0 == ulIp || 0 == count)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Invalid Input Param");
        return XERROR;
    }

    sockfd = socket(PF_PACKET, SOCK_RAW, ETH_P_ARP);
    if (XERROR == sockfd)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "socket failed:%s",strerror(errno));
        return XERROR;
    }
    
    strcpy(ifinfo.ifr_name, pIfName);
    ioctl(sockfd, SIOCGIFINDEX, &ifinfo);

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifinfo.ifr_ifindex;
    sll.sll_protocol = (ETH_P_ARP);
    if (XERROR == bind(sockfd, (struct sockaddr *)&sll, sizeof(sll)))
    {
        close(sockfd);
        XOS_Trace(MD(FID_HA, PL_ERR), "%s %d bind failed:%s",  __FUNCTION__,
                                                        __LINE__,strerror(errno));
        return XERROR;
    }

    /*�㲥arp����*/
    len = HA_MakeArpPacket(buf, (XU8*)pIfName, ARPOP_REQUEST, ulIp);
    for (i = 0; i < count; i++)
    {
        XOS_Trace(MD(FID_HA, PL_INFO), "send arp request on %s for ip[0x%X]", pIfName, ulIp);             
        n = send(sockfd, buf, len, 0);
        if (n <= 0)
        {
            XOS_Trace(MD(FID_HA, PL_ERR),"%s %d sendARPOP_REQUEST failed:%s, dest.ip=0x%0x", 
                                                        __FUNCTION__,__LINE__,strerror(errno), ulIp);
        }
        /* 10ms���ٷ� */
        usleep(10000);
    }

    /*�㲥arp��Ӧ*/
    len = HA_MakeArpPacket(buf, (XU8*)pIfName, ARPOP_REPLY, ulIp);	
    for (i=0; i < count; i++)
    {
        XOS_Trace(MD(FID_HA, PL_INFO), "send arp reply on %s for ip[0x%X]", pIfName, ulIp);             
        n = send(sockfd, buf, len, 0);
        if (n <= 0)
        {
            XOS_Trace(MD(FID_HA, PL_ERR),"%s %d ARPOP_REPLY failed errno = %d, dest.ip=0x%0x", 
                                                     __FUNCTION__,__LINE__,strerror(errno), ulIp);
        }
        /* 10ms���ٷ� */
        usleep(10000);
    }

    close(sockfd);

    return XSUCC;
}
/*****************************************************************************
 �� �� ��  : HA_SendAllVipFreeArp
 ��������  : �������еĸ���IP���ARP
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_SendAllVipFreeArp(void)
{
    ST_HA_VRIF_INFO *pVipInfo = NULL;
    XS32 nodeIndex = 0;
    
    nodeIndex = XOS_listHead(g_VirtualIpList);
    while (XERROR != nodeIndex)
    {
        pVipInfo = XOS_listGetElem(g_VirtualIpList, nodeIndex);
        if (NULL != pVipInfo)
        {
            HA_BroadcastFreeArp(pVipInfo->IFName, pVipInfo->ipaddr, 1);
        }
        
        nodeIndex = XOS_listNext(g_VirtualIpList, nodeIndex);
    }
}

XS32 HA_ChangeToStandby(void)
{
    /* ����״̬�л� */
    return ha_change_status_cmd(HA_STATUS_STANDBY);
}

/*****************************************************************************
 �� �� ��  : HA_ConfigVirtualIP
 ��������  : ����vip��״̬
 �������  : XU32 ipaddr  : ip��ַ ������
             XS32 status  : 1 ʹ��
                            0 �ر�
 �������  : ��
 �� �� ֵ  : inline
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��31��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_ConfigVirtualIP(XU32 ipaddr,XS32 status)
{
    XS32 ret = XSUCC;
    XCHAR szCmd[256] = {0};
    XU8 pIp[16]    = {0};
    XCHAR ucIp[32] = {0};
    
    XOS_IptoStr(ipaddr, (XCHAR*)pIp);
    
    XOS_Sprintf(ucIp, sizeof(ucIp)-1, "\"%s=%d\"", pIp, status);
    
    XOS_Sprintf(szCmd, sizeof(szCmd)-1, "echo %s > /proc/ha/agent ", ucIp);
    ret = XOS_ExeCmd(szCmd, NULL, 0);
    if (XSUCC != ret)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"failed to excute:%s!\n",szCmd);
    }

    return ret ;
}

/*****************************************************************************
 �� �� ��  : HA_EnableAllVirtualIp
 ��������  : �������е���IP��ַ
 �������  : XS32 status  : 1 ʹ��
                            0 �ر�
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��18��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
static XS32 HA_EnableAllVirtualIp(XVOID)
{
    XS32 listIndex = 0;
    XS32 result = 0;
    ST_HA_VRIF_INFO *VipInfo = NULL;

    listIndex = XOS_listHead(g_VirtualIpList);
    while (XERROR != listIndex)
    {
        VipInfo = XOS_listGetElem(g_VirtualIpList, listIndex);
        if (NULL != VipInfo)
        {           
            result = HA_CreateVipInterface(VipInfo);
            if (XSUCC != result)
            {
                XOS_Trace(MD(FID_HA, PL_ERR), "Create Vip failed.");
                continue;
            }

            /* �㲥���arp */
            HA_BroadcastFreeArp(VipInfo->IFName, VipInfo->ipaddr, 1);

        }
        listIndex = XOS_listNext(g_VirtualIpList,listIndex);
    }

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_DisableAllVirtualIp
 ��������  : ж�����и���IP
 �������  : XVOID  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��14��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
static XVOID HA_DisableAllVirtualIp(XVOID)
{
    XS32 listIndex = 0;
    ST_HA_VRIF_INFO *VipInfo = NULL;
    XCHAR szCmd[HA_MAX_CMD_LEN*2] = {0};
    XCHAR pIp[HA_MAX_CMD_LEN] = {0};
    XCHAR pGw[HA_MAX_CMD_LEN] = {0};
		
    listIndex = XOS_listHead(g_VirtualIpList);
    while (XERROR != listIndex)
    {
        VipInfo = XOS_listGetElem(g_VirtualIpList, listIndex);
        if (NULL != VipInfo)
        {
            if (0 != VipInfo->GateWay)
            {
                XOS_IptoStr(VipInfo->ipaddr, (XCHAR*)pIp);
                XOS_IptoStr(VipInfo->GateWay, (XCHAR*)pGw);
                
                /* ȡ��·�� */
                XOS_Sprintf(szCmd, sizeof(szCmd)-1, 
                                    "route del -net %s/%d gw %s 2> /dev/null", 
                                    pIp, VipInfo->prefix, pGw);

                XOS_Trace(MD(FID_HA, PL_LOG), "%s", szCmd);
                XOS_ExeCmd((XCHAR*)szCmd, NULL, 0);  
            }
            /* ɾ���ӿ� */
            XOS_DeleteVirIp(VipInfo->LogicName, VipInfo->ipaddr);
        }
    }
}
/*****************************************************************************
 �� �� ��  : HA_ChangeStatusCallBack
 ��������  : ״̬�л��ص�
 �������  : XU8 status  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_ChangeStatusCallBack(XU8 status)
{
    t_PARA timerpara;
    t_BACKPARA backpara;
    
    if (status > HA_STATUS_CLOSE || HA_STATUS_START == status)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Wrong Status type:%d!\n",status);
        return ;
    }

    /* ����ģʽ ����Ҫ����ARP�ķ��� */
    if (HA_STATUS_CLOSE != ha_get_current_status())
    {
        /*��״̬ ��Ҫ�������ARP �����÷���arp��ʱ�� */
        if (HA_STATUS_ACTIVE == status)
        {
            /* ������ARP�Ķ�ʱ������ */
            backpara.para1 = HA_TIMER_TYPE_ARP;
            
            timerpara.fid = FID_HA;
            /* arp 2s����һ�� */
            timerpara.len = HA_FREE_ARP_SEND_TIME;
            timerpara.mode = TIMER_TYPE_LOOP;
            timerpara.pre  = TIMER_PRE_LOW;
            XOS_TimerStart(&g_pArpSendTimer, &timerpara, &backpara);

            /* ʹ�����и���IP */
            HA_EnableAllVirtualIp();
        }
        else
        {
            /* ȡ��ARP���Ͷ�ʱ�� */
            XOS_TimerStop(FID_HA, g_pArpSendTimer);

            /* ж�����и���ip�ӿ� */
            HA_DisableAllVirtualIp();
        }
    }
    /* ͬ�� �������� �ص�״̬�л����� */
    HA_CallAllModStatusChange(HA_GetCurrentStatus(),status);

    /* �������״̬�л�����״̬ �˳����� */
    if (HA_STATUS_ACTIVE == HA_GetCurrentStatus()
        && HA_STATUS_STANDBY == status)
    {
        XOS_Trace(MD(FID_HA, PL_DBG),"Process exiting!\n");
        exit(0);
    }

    HA_SetCurrentStatus(status);
}


/*****************************************************************************
 �� �� ��  : HA_StatusChangeCallAdd
 ��������  : ״̬�л��ص���ӵ�����
 �������  : ST_HA_STATUS_CALL_INFO *info  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��30��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_StatusChangeCallAdd(ST_HA_STATUS_CALL_INFO *info)
{
    XS32 nodeIndex = 0;
    XS32 result = XSUCC;

    if (XNULL == info)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"Status List Add Failed:NULL param!");
        return XERROR;
    }

    nodeIndex = XOS_listHead(g_StatusChangeCall);
    nodeIndex = XOS_listFind(g_StatusChangeCall, nodeIndex, info);
    if (XERROR != nodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Fid[%d] Have registered!",info->Fid);
                                                    
        return XERROR;
    }
    
    nodeIndex = XOS_listAddTail(g_StatusChangeCall, info);
    if (XERROR == nodeIndex)
    {
        result = XERROR;
        XOS_Trace(MD(FID_HA, PL_ERR), "Add Status List failed!");
    }

    return result;
}

/*****************************************************************************
 �� �� ��  : HA_StatusChangeCmp
 ��������  : ���ұȽϺ���
 �������  : nodeType element1  
             XVOID *param       
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��30��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
static XBOOL HA_StatusChangeCmp(nodeType element1, XVOID *param)
{
    ST_HA_STATUS_CALL_INFO *node = element1;
    ST_HA_STATUS_CALL_INFO *info = param;
    
    if (XNULL == node || XNULL == info)
    {
        return XFALSE;
    }

    if (info->Fid == node->Fid)
    {
        return XTRUE;
    }

    return XFALSE;
}

/*****************************************************************************
 �� �� ��  : HA_StatusChangeCallLstInit
 ��������  : ״̬�л��ص������ʼ��
 
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_StatusChangeCallLstInit(void)
{
    /* ��ʼ��״̬�л��ص����� */
    g_StatusChangeCall = XOS_listConstruct(sizeof(ST_HA_STATUS_CALL_INFO), 
                                        HA_CHANGE_CALL_MAX, "STATUS CHANGE CALL");
    if (XNULLP == g_StatusChangeCall)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "ha status change call list create failed!");
        return XERROR;
    }

    XOS_listClear(g_StatusChangeCall);

    XOS_listSetCompareFunc(g_StatusChangeCall, HA_StatusChangeCmp);

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_WatchReportAdd
 ��������  : ��Դ��ػص����
 �������  : ST_HA_WATCH_REPORT *info  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��9��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_WatchReportAdd(ST_HA_WATCH_REPORT *info)
{
    XS32 nodeIndex = 0;
    XS32 result = XSUCC;

    if (NULL == info)
    {
        XOS_Trace(MD(FID_HA, PL_ERR),"info is null");
        return XERROR;
    }

    nodeIndex = XOS_listHead(g_WatchReportList);
    nodeIndex = XOS_listFind(g_WatchReportList, nodeIndex, info);
    if (XERROR != nodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Fid[%d] have registered watch call back.",info->Fid);
        return XERROR;
    }
    
    nodeIndex = XOS_listAddTail(g_WatchReportList, info);
    if (XERROR == nodeIndex)
    {
        result = XERROR;
        XOS_Trace(MD(FID_HA, PL_ERR), "ha add watch call back failed!");
    }

    return result;
}

XBOOL HA_WatchReportCmp(nodeType element1, XVOID *param)
{
    ST_HA_WATCH_REPORT *node = element1;
    ST_HA_WATCH_REPORT *info = param;

    if (NULL == node || NULL == info)
    {
        return XFALSE;
    }

    if (node->Fid == info->Fid)
    {
        return XTRUE;
    }

    return XFALSE;
}

/*****************************************************************************
 �� �� ��  : HA_WatchReportLstInit
 ��������  : ��Դ��زþ��ص������ʼ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_WatchReportLstInit(void)
{
    /* ��ʼ�������Դ�þ��ص����� */
    g_WatchReportList = XOS_listConstruct(sizeof(ST_HA_WATCH_REPORT), 
                                        HA_WATCH_REPORT_MAX, "WATCH REPORT LIST");
    if (XNULLP == g_WatchReportList)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "ha Watch report list create failed!");
        return XERROR;
    }

    XOS_listClear(g_WatchReportList);

    XOS_listSetCompareFunc(g_WatchReportList, HA_WatchReportCmp);

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_ListCmpFunc
 ��������  : ��IP�ȽϺ���
 �������  : nodeType element1  
             XVOID *param       
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��22��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XBOOL HA_VipListCmpFunc(nodeType element1, XVOID *param)
{
    ST_HA_VRIF_INFO *VipInfo = element1;
    XU32 *ip = param;

    if (NULL == VipInfo || NULL == ip)
    {
        return XFALSE;
    }

    if (*ip == VipInfo->ipaddr)
    {
        return XTRUE;
    }

    return XFALSE;
}

/*****************************************************************************
 �� �� ��  : HA_VirtureIpv4Add
 ��������  : ����IP��Ϣ��ӵ�����
 �������  : ST_HA_VRIP_INFO *VipInfo  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��18��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_VirtualIpv4AddToList(ST_HA_VRIF_INFO *VipInfo)
{
    XS32 nodeIndex = 0;

    nodeIndex = XOS_listHead(g_VirtualIpList);
    nodeIndex = XOS_listFind(g_VirtualIpList, nodeIndex, VipInfo);
    if (XERROR != nodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "vip[0x%x] have added.",VipInfo->ipaddr);
        return XERROR;
    }
    
    nodeIndex = XOS_listAddTail(g_VirtualIpList, VipInfo);
    if (XERROR == nodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "ha add vrip list failed!");
    }

    return nodeIndex;
}

/*****************************************************************************
 �� �� ��  : HA_VirtualIpUpdateSubName
 ��������  : ��������ӿ�
 �������  : XS32 Index      
             XS8 *pViIfName  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��14��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
static XVOID HA_VirtualIpUpdateSubName(XS32 Index, XS8 *pViIfName)
{
    ST_HA_VRIF_INFO *pUpDateInfo = NULL;
    
    pUpDateInfo = XOS_listGetElem(g_VirtualIpList, Index);
    if (NULL != pUpDateInfo)
    {
        XOS_StrCpy(pUpDateInfo->ViIfName, pViIfName);
    }

    return;
}

/*****************************************************************************
 �� �� ��  : HA_CreateVipInterface
 ��������  : ��������IP�Ľӿ�
 �������  : ST_HA_VRIF_INFO *pInterFace  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_CreateVipInterface(ST_HA_VRIF_INFO *pInterFace)
{
    /*
    XU8 pMask[HA_MAX_CMD_LEN] = {0};
    XU8 pNet[HA_MAX_CMD_LEN] = {0};
    static XU8 VipIfIndex = 1;*/
    XU32 result = 0,mask = 0;
    XU8 cmd[HA_MAX_CMD_LEN*2] = {0};
    XU8 pIp[HA_MAX_CMD_LEN] = {0};
    XU8 pGw[HA_MAX_CMD_LEN] = {0};

    if(NULL == pInterFace)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Param is NULL!");
        return XERROR;
    }

    XOS_IptoStr(XOS_HtoNl(pInterFace->ipaddr), (XCHAR*)pIp);
    XOS_IptoStr(XOS_HtoNl(pInterFace->GateWay), (XCHAR*)pGw);

    mask = HA_Prefix2SubMask(pInterFace->prefix);
#if 0
    XOS_IptoStr(pInterFace->mask, (XCHAR*)pMask);
    XOS_IptoStr((pInterFace->ipaddr & pInterFace->mask), (XCHAR*)pNet);

    /* �����ӽӿ��� */
    XOS_Sprintf(pInterFace->ViIfName, sizeof(pInterFace->ViIfName - 1), "%s:%d",  
                                                pInterFace->IFName, VipIfIndex);
    
    /*���ø���IP  */
    XOS_Sprintf((XCHAR*)cmd, sizeof(cmd)-1,
                        "ifconfig %s %s netmask %s up 2> /dev/null", 
                        pInterFace->ViIfName, pIp, pMask);
    XOS_Trace(MD(FID_HA, PL_LOG), "%s", cmd);
    XOS_ExeCmd((XCHAR*)cmd, NULL, 0);  
    VipIfIndex++;
    XOS_Sleep(1);
    
    /*Ϊ����IP��ӱ�����·��*/
    XOS_Sprintf((XCHAR*)cmd, sizeof(cmd)-1, 
                    "ip route add %s/%d proto kernel src %s dev %s 2> /dev/null", 
                    pNet, prefix, pIp, pInterFace->IFName);
    XOS_Trace(MD(FID_HA, PL_LOG), "%s", cmd);
    XOS_ExeCmd((XCHAR*)cmd, NULL, 0); 
    XOS_Sleep(1);    
#endif
    result = XOS_AddVirIp_Ex(pInterFace->LogicName, pInterFace->ViIfName,
                                                    pInterFace->ipaddr, mask);
    if (XSUCC != result)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Create Vip failed.");
        return XERROR;
    }

    /*Ϊ����IP��ӿ�����·��(����)*/
    if (0 != pInterFace->GateWay)
    {
        XOS_Sprintf((XCHAR*)cmd, sizeof(cmd)-1, 
           "route add -net %s/%d gw %s 2> /dev/null", pIp, pInterFace->prefix, pGw);
                            
        XOS_Trace(MD(FID_HA, PL_LOG), "%s", cmd);
        XOS_ExeCmd((XCHAR*)cmd, NULL, 0);
    }
    
    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_VirtualIpv4Add
 ��������  : ����IP���
 �������  : ST_HA_VRIF_INFO *VipInfo  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_VirtualIpv4Add(ST_HA_VRIF_INFO *VipInfo)
{

    XS32 result = XSUCC;
    XS32 SaveIndx = 0;
    if (NULL == VipInfo)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Input param is NULL");
        return XERROR;
    }

    result = XOS_LogicIfConvToDevIf(VipInfo->LogicName, VipInfo->IFName, 
                                                       sizeof(VipInfo->IFName));
    if (XERROR == result)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Get device name error");
        return XERROR;
    }

    /* ��ӵ����� */
    SaveIndx = HA_VirtualIpv4AddToList(VipInfo);
    if (XERROR == SaveIndx)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Add vip to list failed");
        return XERROR;
    }

    if (HA_STATUS_ACTIVE == HA_GetCurrentStatus())
    {
        /* ��������IP�ӿ� */
        result = HA_CreateVipInterface(VipInfo);
        if (XSUCC != result)
        {
            XOS_Trace(MD(FID_HA, PL_ERR), "Create vitrual interface failed");
            return XERROR;
        }

        /* �����ӽӿ��� */
        HA_VirtualIpUpdateSubName(SaveIndx, VipInfo->ViIfName);
        
        HA_BroadcastFreeArp(VipInfo->IFName, VipInfo->ipaddr, 3);
        
    }

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_VirtualIpDel
 ��������  : ɾ������ip
 �������  : XU32 ipaddr  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��9��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_VirtualIpDel(XU32 ipaddr)
{
    XS32 NodeIndex = 0;
    ST_HA_VRIF_INFO *pstVipInfo = NULL;
    XCHAR szCmd[HA_MAX_CMD_LEN*2] = {0};
    XU8 pIp[HA_MAX_CMD_LEN] = {0};
    XU8 pGw[HA_MAX_CMD_LEN] = {0};
    
    NodeIndex = XOS_listHead(g_VirtualIpList);
    if (XERROR == NodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "get first index failed");
        return XERROR;
    }
    
    NodeIndex = XOS_listFind(g_VirtualIpList, NodeIndex, &ipaddr);
    if (XERROR == NodeIndex)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "cannot find info of 0x%x",ipaddr);
        return XERROR;
    }

    pstVipInfo = XOS_listGetElem(g_VirtualIpList, NodeIndex);
    if (NULL != pstVipInfo)
    {
        if (0 != pstVipInfo->GateWay)
        {
            XOS_IptoStr(pstVipInfo->ipaddr, (XCHAR*)pIp);
            XOS_IptoStr(pstVipInfo->GateWay, (XCHAR*)pGw);
            
            /* ȡ��·�� */
            XOS_Sprintf(szCmd, sizeof(szCmd)-1, 
                                        "route del -net %s/%d gw %s 2> /dev/null", 
                                        pIp, pstVipInfo->prefix, pGw);
            XOS_Trace(MD(FID_HA, PL_LOG), "%s", szCmd);
            XOS_ExeCmd((XCHAR*)szCmd, NULL, 0);  
        }
        /* ɾ���ӿ� */
        XOS_DeleteVirIp(pstVipInfo->LogicName, ipaddr);
    }

    /* ������ɾ�� */
    XOS_listDelete(g_VirtualIpList, NodeIndex);

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_VirtualIpDelAll
 ��������  : ɾ�����е�vip�ӿ�
 �������  : XVOID  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��5��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XVOID HA_VirtualIpDelAll(XVOID)
{
    XS32 NodeIndex = 0;
    ST_HA_VRIF_INFO *pstVipInfo = NULL;
    XCHAR szCmd[256] = {0};

    NodeIndex = XOS_listHead(g_VirtualIpList);
    while (XERROR != NodeIndex)
    {
        pstVipInfo = XOS_listGetElem(g_VirtualIpList, NodeIndex);
        if (NULL != pstVipInfo)
        {
            XOS_Sprintf(szCmd, sizeof(szCmd)-1, "ifconfig %s down",
                                                            pstVipInfo->ViIfName);
            XOS_Trace(MD(FID_HA, PL_LOG), "%s", szCmd);
            XOS_ExeCmd((XCHAR*)szCmd, NULL, 0);  
        }

        XOS_listDeleteHead(g_VirtualIpList);
        NodeIndex = XOS_listHead(g_VirtualIpList);
    }
    
    return ;
}

/*****************************************************************************
 �� �� ��  : HA_VipLstInit
 ��������  : ��IP�����ʼ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��23��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_VipLstInit(void)
{
    /* ��ʼ������IP���� */
    g_VirtualIpList = XOS_listConstruct(sizeof(ST_HA_VRIF_INFO), 
                                                HA_VITURE_IP_MAX, "VITURE IP");
    if (XNULLP == g_VirtualIpList)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "ha resource create vrip list failed!");
        return XERROR;
    }

    XOS_listClear(g_VirtualIpList);

    /* ���ñȽϺ��� */
    return XOS_listSetCompareFunc(g_VirtualIpList, HA_VipListCmpFunc);
}

/*****************************************************************************
 �� �� ��  : HA_ResourceDestroy
 ��������  : ��Դ����
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_ResourceDestroy(void)
{
    /* ����ɾ������IP�ӿ� */
    HA_VirtualIpDelAll();
    if (XNULL != g_VirtualIpList)
    {
        XOS_listDestruct(g_VirtualIpList);
        g_VirtualIpList = XNULL;
    }

    if (XNULL != g_WatchReportList)
    {
        XOS_listDestruct(g_WatchReportList);
        g_WatchReportList = XNULL;
    }

    if (XNULL != g_StatusChangeCall)
    {
        XOS_listDestruct(g_StatusChangeCall);
        g_StatusChangeCall = XNULL;
    }

    HA_DeadWatchDestroy();

    ha_cancel_thread();

    pMemProcFunc = XNULL;
    pRestProcFunc = XNULL;
}

/*****************************************************************************
 �� �� ��  : HA_JusticeToChange
 ��������  : ����ע���ҵ��ص� ���вþ� �Ƿ���Ҫ����״̬�л�
 �������  : XU32 ResourceType  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XBOOL HA_JusticeToChange(XU32 ResourceType)
{
    XS32 indx = 0;
    ST_HA_WATCH_REPORT *pReportInfo = NULL;
    XBOOL result = XTRUE;
        
    indx = XOS_listHead(g_WatchReportList);

    while (XERROR != indx)
    {
        /* ����ÿһ���ڵ㣬���вþ� :�Ƿ�����л�״̬ */
        pReportInfo = XOS_listGetElem(g_WatchReportList, indx);
        if (NULL != pReportInfo && (ResourceType & pReportInfo->WatchInfo))
        {
            /* ʵ��1Ʊ����� */
            result = pReportInfo->ReportFunc(ResourceType, pReportInfo->param);
            if (XFALSE == result)
            {
                return XFALSE;
            }
        }
        indx = XOS_listNext(g_WatchReportList, indx);
    }

    return XTRUE;
}

/*****************************************************************************
 �� �� ��  : HA_MemRunOutCall
 ��������  : �ڴ��ػص�
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_MemRunOutCall(void)
{
    XBOOL result = XFALSE;
    
    result = HA_JusticeToChange(XOS_HA_WATCH_MEM);
    if (XTRUE == result)
    {
        HA_ChangeToStandby();
    }
}

/*****************************************************************************
 �� �� ��  : HA_MsgFullCall
 ��������  : ��Ϣ���лص�
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_MsgFullCall(void)
{
    XBOOL result = XFALSE;
    
    result = HA_JusticeToChange(XOS_HA_WATCH_MSG_QUE);
    if (XTRUE == result)
    {
        HA_ChangeToStandby();
    }
}


/*****************************************************************************
 �� �� ��  : HA_ShowWatchReport
 ��������  : ��ӡ���ע��ص�
 �������  : CLI_ENV* pCliEnv  
             XS32 siArgc       
             XCHAR** ppArgv    
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XVOID HA_ShowWatchReport(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XS32 indx = 0;
    XS32 num = 0;
    ST_HA_WATCH_REPORT *pReportInfo = NULL;

    XOS_CliExtPrintf(pCliEnv,"---------------------------------------------\r\n");
    indx = XOS_listHead(g_WatchReportList);

    while (XERROR != indx)
    {
        /* ����ÿһ���ڵ� ��ӡ */
        pReportInfo = XOS_listGetElem(g_WatchReportList, indx);
        if (NULL != pReportInfo)
        {
            XOS_CliExtPrintf(pCliEnv,"Fid:%d, Watch Info:0x%x\r\n",
                                pReportInfo->Fid,pReportInfo->WatchInfo);
            num++;
        }
        indx = XOS_listNext(g_WatchReportList, indx);
    }
    
    XOS_CliExtPrintf(pCliEnv,"All watch num:%4d\r\n",num);
    XOS_CliExtPrintf(pCliEnv,"---------------------------------------------\r\n");
}

/*****************************************************************************
 �� �� ��  : HA_ShowVitualIp
 ��������  : ��ӡ���е�����ӿ�
 �������  : CLI_ENV* pCliEnv  
             XS32 siArgc       
             XCHAR** ppArgv    
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XVOID HA_ShowVitualIp(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XS32 indx = 0;
    XS32 num = 0;
    ST_HA_VRIF_INFO *pVitualIP = NULL;
    XCHAR ipstr[32] = {0};

    XOS_CliExtPrintf(pCliEnv,"---------------------------------------------\r\n");
    indx = XOS_listHead(g_VirtualIpList);

    while (XERROR != indx)
    {
        /* ����ÿһ���ڵ㣬���вþ��Ƿ�����л�״̬ */
        pVitualIP = XOS_listGetElem(g_VirtualIpList, indx);
        if (NULL != pVitualIP)
        {
            memset(ipstr, 0, sizeof(ipstr));
            XOS_IptoStr(pVitualIP->ipaddr, ipstr);
            XOS_CliExtPrintf(pCliEnv,"ifname:%-20s logic:%-20s ssubIf:%-20s ip:%-16s\r\n",
                     pVitualIP->IFName,pVitualIP->LogicName,pVitualIP->ViIfName,ipstr);
            num++;
        }
        indx = XOS_listNext(g_VirtualIpList, indx);
    }
    
    XOS_CliExtPrintf(pCliEnv,"All VIP num:%4d\r\n",num);
    XOS_CliExtPrintf(pCliEnv,"---------------------------------------------\r\n");
}

/*****************************************************************************
 �� �� ��  : HA_ShowCurrentConfig
 ��������  : ��ӡHA����������
 �������  : CLI_ENV* pCliEnv  
             XS32 siArgc       
             XCHAR** ppArgv    
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XVOID HA_ShowCurrentConfig(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XS8 *pstatus = NULL;
    XCHAR Ipstr[32] = {0};
    XS8 i = 0;
    ST_HA_PEER_IP *ptr = NULL;
    
    XOS_CliExtPrintf(pCliEnv,"---------------------------------------------\r\n");
    
    switch (g_HaCurrentStatus)
    {
        case HA_STATUS_START:
            pstatus = "START";
            break;
        case HA_STATUS_INITAL:
            pstatus = "INITAL";
            break;
        case HA_STATUS_STANDBY:
            pstatus = "STANDBY";
            break;
        case HA_STATUS_ACTIVE:
            pstatus = "ACTIVE";
            break;
        case HA_STATUS_CLOSE:
            pstatus = "CLOSE";
            break;
        default:
            pstatus = "INIALID";
    }
        
    XOS_CliExtPrintf(pCliEnv,"Current Status:%s\r\n",pstatus);
    XOS_CliExtPrintf(pCliEnv,"Local prio:%2d\r\n", g_stConfig.priority);
    XOS_CliExtPrintf(pCliEnv,"Local Slot:%2d\r\n", g_stConfig.conf_num);

    ptr = &g_stConfig.ipinfo[0];
    for (; i < HA_PEER_IP_MAX_NUM; i++)
    {
        ptr += i;
        memset(Ipstr, 0, sizeof(Ipstr));
        XOS_IptoStr(ptr->local_ip, Ipstr);
        XOS_CliExtPrintf(pCliEnv,"Local Ip[%d]:%-16s:%-4d\r\n",i, Ipstr, 
                                                                ptr->local_port);

        memset(Ipstr, 0, sizeof(Ipstr));
        XOS_IptoStr(ptr->peer_ip, Ipstr);
        XOS_CliExtPrintf(pCliEnv,"Peer  Ip[%d]:%-16s:%-4d\r\n",i, Ipstr, 
                                                                ptr->peer_port);
    }
}

/*****************************************************************************
 �� �� ��  : HA_CommandReg
 ��������  : ������ע��
 �������  : XVOID  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_CommandReg(XVOID)
{
    XS32 promptID = 0;

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "ha", "Ha module", "");

    /* ������ʾ���� */
    if (0 > XOS_RegistCommand(promptID, HA_ShowCurrentConfig, "showconfig", 
                                            "show ha running config", "�޲���"))
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Failed to register command"  "XOS>HA>showconfig!");

        return XERROR;
    }

    /* ���ע����ʾ���� */
    if (0 > XOS_RegistCommand(promptID, HA_ShowWatchReport, "watchinfo", 
                                            "show ha watch info", "�޲���"))
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Failed to register command"  "XOS>HA>watchinfo!");

        return XERROR;
    }

    /* ����IP��ʾ���� */
    if (0 > XOS_RegistCommand(promptID, HA_ShowVitualIp, "vipinfo", 
                                            "show vip info", "�޲���"))
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Failed to register command"  "XOS>HA>vip!");

        return XERROR;
    }

    if (0 > XOS_RegistCommand(promptID, HA_ShowDeadWatchInfo, "deadwatch", 
                                            "show dead watch info", "�޲���"))
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Failed to register command"  "XOS>HA>vip!");

        return XERROR;
    }
#if 0
    /* �ֶ��޸����� */
    if (0 > XOS_RegistCommand(promptID, HA_TestConfig, "config", 
                                "config the board info", "0 9 192.168.0.1 3389"))
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Failed to register command"  "XOS>HA>config!");

        return XERROR;
    }

    /* ���ù���ģʽ */
    if (0 > XOS_RegistCommand(promptID, HA_TestSetWorkMode, "workmod", 
                                "set work mod", "0:single, 1:ha"))
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Failed to register command"  "XOS>HA>workmod!");

        return XERROR;
    }

    /* �������ip */
    if (0 > XOS_RegistCommand(promptID, HA_TestVipAdd, "vipadd", 
                                "add vip ", "vipadd eth0 10.10.10.8 10.10.10.1"))
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Failed to register command"  "XOS>HA>workmod!");

        return XERROR;
    }

    /* ����״̬�л��ص� */
    if (0 > XOS_RegistCommand(promptID, HA_TestStatusCallBack, "statuscall", 
                                "register status change call", " number "))
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Failed to register command"  "XOS>HA>statuscall!");

        return XERROR;
    }

    /* �����̼߳�� */
    if (0 > XOS_RegistCommand(promptID, HA_TestWatchThread, "watchtest", 
                                "test thread watch", "  "))
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Failed to register command"  "XOS>HA>watchtest!");

        return XERROR;
    }

    /* �����̹߳ҵ� */
    if (0 > XOS_RegistCommand(promptID, HA_CloseWatchThread, "threadclose", 
                                "close thread", "  "))
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "Failed to register command"  "XOS>HA>threadclose!");

        return XERROR;
    }
#endif
    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : Ha_AgentNfLoad
 ��������  : ���ر��Ĺ����ں�ģ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS8 Ha_AgentNfLoad(void)
{
    
    XCHAR szBuf[HA_MAX_CMD_LEN] = {0};
    XCHAR szCmd[HA_MAX_CMD_LEN] = {0};
    
    /*��ѯnf*/
    XOS_Sprintf(szCmd, sizeof(szCmd)-1, "lsmod | grep \"nf \"");
    XOS_ExeCmd(szCmd, szBuf, sizeof(szBuf)-1);
    
    if(strstr(szBuf, "nf ") == NULL) /*δ����nf*/
    {
        /*��ʼ��nf*/
        XOS_Sprintf(szCmd, sizeof(szCmd)-1, "insmod nf.ko");
        XOS_ExeCmd(szCmd, NULL, 0);   
    }

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_GetConfigValue
 ��������  : ����xos.xml��ȡHA��������Ϣ
 �������  : XVOID  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��15��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
static XBOOL HA_GetConfigValue(XVOID)
{
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    XCHAR *pFileName = NULL;

    
    pFileName = XOS_CliGetXmlName();
    if (NULL == pFileName)
    {
        XOS_Trace(MD(FID_HA, PL_ERR), "");
        return XFALSE;
    }

    doc = xmlParseFile(pFileName);
    if (XNULL == doc)
    {
        return (XFALSE);
    }
    
    /*�Ҹ��ڵ�*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    
    /*���ڵ�*/
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur->next;
    }
    
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    
    /*HACFG ���ڵ�*/
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "HACFG" ) )
        {
            break;
        }
        cur = cur->next;
    }
    
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XTRUE ;
    }
    
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XTRUE ;
    }
    
    /*����HACFG�ӽڵ�*/
    while ( cur != XNULL )
    {
        /* ��ȡĬ���Ƿ����̼߳�صĹ��� */
        if ( !XOS_StrCmp(cur->name, "DEFAULT_WATCH_DEAD" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                
                if (0 == XOS_StrCmp(pTempStr, "ON"))
                {
                    HA_DeadWatchDefaultOpen();
                }
                else
                {
                    HA_DeadWatchDefaultClose();
                }
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }            
        }
        cur = cur->next;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XTRUE;
    
}

/*****************************************************************************
 �� �� ��  : HA_ResourceInit
 ��������  : ��ʼ��
 �������  : XVOID *p1  
             XVOID *p2  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS8 HA_ResourceInit(XVOID *p1, XVOID *p2)
{
    XS32 ret = 0;

    /* ��ȡxml������ */
    HA_GetConfigValue();

    /* ��IP�����ʼ�� */
    ret = HA_VipLstInit();
    if (XSUCC != ret)
    {
        goto FINISHED;
    }

    /* ��Դ��������ʼ�� */
    ret = HA_WatchReportLstInit();
    if (XSUCC != ret)
    {
        goto FINISHED;
    }

    /* ������� ��ʼ�� */
    ret = HA_DeadWatchInit();
    if (XSUCC != ret)
    {
        goto FINISHED;
    }

    /* ״̬�л��ص������ʼ�� */
    ret = HA_StatusChangeCallLstInit();
    if (XSUCC != ret)
    {
        goto FINISHED;
    }

    /* ���붨ʱ�� */
    ret = XOS_TimerReg(FID_HA, 0, HA_TIMER_NUMBER, 0);
    if (XSUCC != ret)
    {
        goto FINISHED;
    }

    ha_status_change_reg(HA_ChangeStatusCallBack);
    pMemProcFunc  = HA_MemRunOutCall;
    pRestProcFunc = HA_MsgFullCall;

    /* ����״̬���� */
    if (0 != g_stConfig.priority)
    {
        ret = HA_StartStatusControl();
    }
    else /* ����ģʽ�л�Ϊ��״̬ */
    {
        ha_status_init();
        HA_ChangeStatusCallBack(HA_STATUS_ACTIVE);
    }
    /* ע�������� */
    HA_CommandReg();

    return XSUCC;

FINISHED:
    if (XSUCC != ret)
    {
        HA_ResourceDestroy();
    }

    return ret;
}


XS32  XOS_FIDHA(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST HaList;
    XS32 ret = XSUCC;

    XOS_MemSet( &HaList, 0x00, sizeof(t_XOSLOGINLIST) );

    HaList.stack      = &XOS_HA;
    XOS_StrNcpy(HaList.taskname , "Tsk_ha", MAX_TID_NAME_LEN);
    HaList.TID        = FID_HA;
    HaList.prio       = TSK_PRIO_HIGHER;    
    HaList.quenum = MAX_MSGS_IN_QUE;

    ret = XOS_MMStartFid(&HaList, XNULLP, XNULLP);

    return ret;
}

XS8 HA_TimerProc( t_BACKPARA* pParam)
{
    if(NULL == pParam)
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"timer PTIMER is null ,bad input param!");
        return XERROR;
    }

    switch ( pParam->para1 )
    {
        case HA_TIMER_TYPE_ARP :
            /* ���ARP */
            HA_SendAllVipFreeArp();
            break;
        case HA_TIMER_TYPE_STATUS :
            /* ״̬���� */
            ha_time_out_process();
            break;
        case HA_TIMER_TYPE_DEAD_WATCH:
            /* ��ʱ������������ */
            HA_SendMsgToWatchMod((XOS_HLIST)pParam->para2);
            break;
        case HA_TIMER_TYPE_DEAD_RECV:
            /* �����������Ļظ� */
            HA_RecvHeartBeatMsg((XOS_HLIST)pParam->para2);
            break;
        default:
            XOS_Trace(MD(FID_HA,PL_ERR),"timer bad input param!");
    }

    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_MsgProc
 ��������  : ��Ϣ������ ������ģ�鷢�͹�������Ϣ����Ҫ�������ظ�
 �������  : XVOID* pMsgP  
             XVOID* para   
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS8 HA_MsgProc(XVOID* pMsgP, XVOID* para)
{
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)pMsgP;

    if(NULL == pMsg)
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"msg is NULL!");
        return XERROR;
    }
    
    if(NULL == (pMsg->message))
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"msg has no data");
        return XERROR;
    }

    if(FID_HA == pMsg->datadest.FID)
    {
        HA_ParseHeartBeatMsgFromQue(pMsg);
    }

    return XSUCC;
}


/*****************************************************************************
 �� �� ��  : HA_StartStatusControl
 ��������  : ����״̬����
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��30��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_StartStatusControl(void)
{

    /* t_PARA timerpara;
     t_BACKPARA backpara;*/
     
    ha_status_control_init(&g_stConfig);

    /* ������״̬��ʱ������ 
    backpara.para1 = HA_TIMER_TYPE_STATUS;
    
    timerpara.fid  = FID_HA;
    timerpara.len  = HA_KEEP_ALIVE_TIME;
    timerpara.mode = TIMER_TYPE_LOOP;
    timerpara.pre  = TIMER_PRE_LOW;
    XOS_TimerStart(&g_pStatusControlTimer, &timerpara, &backpara);
    */
    return XSUCC;
}

/*****************************************************************************
 �� �� ��  : HA_CloseStatusControl
 ��������  : �ر�״̬����ʱ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��12��30��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
void HA_CloseStatusControl(void)
{
    /*XOS_TimerStop(FID_HA, g_pStatusControlTimer);*/
    
    ha_cancel_thread();
}

/*****************************************************************************
 �� �� ��  : HA_SetPeerOamInfo
 ��������  : IP�Ե�����
 �������  : ST_HA_PEER_OAM_INFO *pInfo  
             XS32 len                    
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��1��4��
    ��    ��   : liujun
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 HA_SetPeerOamInfo(ST_HA_PEER_OAM_INFO *pstPeerInfo, XS32 len)
{
    XS32 slot = 0;
    
    if (len < (HA_PEER_NUM * sizeof(ST_HA_PEER_OAM_INFO))
        || (NULL == pstPeerInfo))
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"peerinfo is null or len is wrong:%d",len);
        return XERROR;
    }
        
    /* ��ȡ��ǰ��λ�� */
    slot = XOS_GetSlotNum();
    if (slot < 0)
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"Slot num is invalid:%d",slot);
        return XERROR;
    }
    
    g_stConfig.conf_num = slot;
    /* ���õĲ�λ���� ����Ҫ��һ��Ϊ��ǰ��λ�� ���������Ǵ���� */
    if (slot != pstPeerInfo[0].usSlotId && slot != pstPeerInfo[1].usSlotId)
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"Get info from oam is not matched");
        return XERROR;
    }

    /* ��������IP��Ϊ0 */
    if ((0 == pstPeerInfo[0].stIpAddr[0].ip && 0 == pstPeerInfo[0].stIpAddr[1].ip)
        || (0 == pstPeerInfo[1].stIpAddr[0].ip && 0 == pstPeerInfo[1].stIpAddr[1].ip))
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"Can not all the ipaddr are zero.");
        return XERROR;
    }

    /* ��������port��Ϊ0 */
    if ((0 == pstPeerInfo[0].stIpAddr[0].port && 0 == pstPeerInfo[0].stIpAddr[1].port)
        || (0 == pstPeerInfo[1].stIpAddr[0].port && 0 == pstPeerInfo[1].stIpAddr[1].port))
    {
        XOS_Trace(MD(FID_HA,PL_ERR),"Can not all the ports are zero.");
        return XERROR;
    }

    /* 0����Ϊ��״̬����Ϣ */
    if (slot == pstPeerInfo[0].usSlotId)
    {
        /* ��ǰ����ΪACTIVE �����ȼ����ø� */
        g_stConfig.priority = HA_STATUS_PRIO_ACTIVE;

        /* 0����ip�� */
        g_stConfig.ipinfo[0].local_ip   = pstPeerInfo[0].stIpAddr[0].ip;
        g_stConfig.ipinfo[0].local_port = pstPeerInfo[0].stIpAddr[0].port;
        g_stConfig.ipinfo[0].peer_ip   = pstPeerInfo[1].stIpAddr[0].ip;
        g_stConfig.ipinfo[0].peer_port = pstPeerInfo[1].stIpAddr[0].port;
        /* 1����ip�� */
        g_stConfig.ipinfo[1].local_ip   = pstPeerInfo[0].stIpAddr[1].ip;
        g_stConfig.ipinfo[1].local_port = pstPeerInfo[0].stIpAddr[1].port;
        g_stConfig.ipinfo[1].peer_ip   = pstPeerInfo[1].stIpAddr[1].ip;
        g_stConfig.ipinfo[1].peer_port = pstPeerInfo[1].stIpAddr[1].port;
    }
    else
    {
        /* ��ǰ����ΪSTANDBY */
        g_stConfig.priority = HA_STATUS_PRIO_STANDBY;
        /* 0����ip�� */
        g_stConfig.ipinfo[0].local_ip   = pstPeerInfo[1].stIpAddr[0].ip;
        g_stConfig.ipinfo[0].local_port = pstPeerInfo[1].stIpAddr[0].port;
        g_stConfig.ipinfo[0].peer_ip   = pstPeerInfo[0].stIpAddr[0].ip;
        g_stConfig.ipinfo[0].peer_port = pstPeerInfo[0].stIpAddr[0].port;   
        /* 1����ip�� */
        g_stConfig.ipinfo[1].local_ip   = pstPeerInfo[1].stIpAddr[1].ip;
        g_stConfig.ipinfo[1].local_port = pstPeerInfo[1].stIpAddr[1].port;
        g_stConfig.ipinfo[1].peer_ip   = pstPeerInfo[0].stIpAddr[1].ip;
        g_stConfig.ipinfo[1].peer_port = pstPeerInfo[0].stIpAddr[1].port;
    }

    HA_StartStatusControl();

    return XSUCC;
}

 #endif /* XOS_LINUX */