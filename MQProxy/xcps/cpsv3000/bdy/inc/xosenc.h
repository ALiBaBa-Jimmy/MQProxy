/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: osenc.h
**
**  description:  ��IP����ģ���ͷ�ļ� 
**
**  author: wentao
**
**  date:   2006.12.4
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   wentao         2006.12.4              create  
**************************************************************/
#ifndef _OSENC_H_
#define _OSENC_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*------------------------------------------------------------------------
               ����ͷ�ļ�
-------------------------------------------------------------------------*/   
#include "xostype.h"
#include "xosencap.h"

#ifdef XOS_LINUX
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif

/*-------------------------------------------------------------------------
                  �궨��
-------------------------------------------------------------------------*/

#define XOS_MAXCMDLEN   128  /* ����buf��󳤶� */
#define XOS_IPSTRLEN    16   /* a.b.c.d ���ip�ַ����ĳ��� */
#define XOS_IFNAMESIZE  17   /* �����豸������󳤶� */
#define XOS_MAXIPNUM    128  /* ��������豸���� (���������豸) */

/*-------------------------------------------------------------------------
                  �ṹ��ö������
-------------------------------------------------------------------------*/

/*IP��Ϣ�б�ṹ��*/
typedef struct _IP_LIST
{
    XS8   xinterface[XOS_IFNAMESIZE];    /*IP�����Ľӿڣ�������NAMESIZE =16*/
    XU32  LocalIPaddr;    /*��ȡ����IP��ַ*/
    XU32  LocalNetMask;   /* ���� */
}t_IPLIST;

typedef struct  _LOCALIP_INFO
{
    XU32  nIPNum;    /*��ȡ��IP�ĸ���*/
    t_IPLIST        localIP[XOS_MAXIPNUM];    /*���֧��? ��*/
} t_LOCALIPINFO;


#ifdef XOS_WIN32
typedef SOCKET XOS_SOCKET_ID;
#endif
#ifdef XOS_LINUX
typedef XS32 XOS_SOCKET_ID;
#endif
#ifdef XOS_SOLARIS
typedef XS32 XOS_SOCKET_ID;
#endif
#ifdef XOS_VXWORKS
typedef XS32 XOS_SOCKET_ID;
#endif

/*-------------------------------------------------------------------------
                      API ����
-------------------------------------------------------------------------*/
/************************************************************************
������:    XOS_GetLocalIPList
���ܣ���ȡ�����豸������ӿں�IP��ַ�б�    
���룺 pLocalIPList        -        ��������IP��Ϣ�б�Ľṹ��
����� 
pLocalIPList.nIPNum    -    ��ȡ��IP��ַ�ĸ���, 
pLocalIPList. LocalIP[i]. Interface        -    IP�����Ľӿڣ�
pLocalIPList. LocalIP[i]. LocalIPaddr    -    32 λ��IP��ַ
�磺 pLocalIPList.localIP[0].LocalIPaddr ���б��е�һ������IP��ַ.
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����Ŀǰ��windows���޷�ȡ�ö�Ӧ��IP��ַ
************************************************************************/
XS32 XOS_GetLocalIPList ( t_LOCALIPINFO * pLocalIPList );

/************************************************************************
������:    XOS_LocalIPAdd
���ܣ� �ڱ��ؽӿ��м���һ��IP��ַ    
���룺 
interface    -    ָ������ӿڵ����ƣ�һ���Ӧһ���������磺windows��Ϊ"��������"��
linux��Ϊһ������"eth0:x"������'eth0'Ϊ������ʶ��
'x'��ʾ�ڼ���������"eth0:1"��ʾ��һ�������ĵ�һ��������
ipaddress    -    Ҫ��ӵ��˽ӿڵ�IP��ַ�����ʮ���ƣ��ַ�������ʽ���磺"192.168.0.1"��
subnetmask    -    ָ��IP���������룬�磺"255.255.255.0"
broadcast    -    ָ��IP�Ĺ㲥��ַ���磺"192.168.0.255"��windows����Ĭ�ϵ�,������XNULL��
�����N/A
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����ͬһ�ӿڣ��������¶�������ͬ��IP������XERROR��
��������Ľӿ�����������ʹ����Ҫ��֤������ԣ�LINUX�¿���ͨ��XOS_GetLocalIPList() 
����ȡ��ǰ�Ľӿ�������ӦIP�����ο�����Ҫע�����뼰�㲥��ַ�ĸ�ʽ����Ч�ԡ�
************************************************************************/
XS32 XOS_LocalIPAdd ( XS8 *xinterface , XS8 *ipaddress , XS8 *subnetmask , XS8 *broadcast );

/************************************************************************
������:    XOS_LocalIPDelete
���ܣ�ɾ��ָ���ӿ��µ�һ��IP��ַ    
���룺
interface    -    ָ������ӿڵ����ƣ�һ���Ӧһ���������磺windows��Ϊ"��������"��
linux��Ϊһ������"eth0:x"������'eth0'Ϊ������ʶ��'x'��ʾ�ڼ���������eth0:1��ʾ��һ�������ĵ�һ��������
ipaddress    -    Ҫɾ����IP��ַ�����ʮ���ƣ��ַ�������ʽ���磺"192.168.0.1"��
�����N/A
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵�������ɾ��ͬһIP��ַ������XERROR��
��������Ľӿ���ʹ����Ҫ��֤������ԣ�LINUX�¿���ͨ����XOS_GetLocalIPList() 
����ȡ��ǰ�Ľӿ�������ӦIP�����ο���
ע��ֻ��ɾ����̬IP��ÿ���ӿڵ����һ����̬IP����ɾ����LINUX�������ӿڣ��Ǳ���,��:"eth0"����IP����ɾ����
************************************************************************/
XS32 XOS_LocalIPDelete (XS8 *xinterface , XS8 *ipaddress );

/************************************************************************
������:    XOS_LocalIPModify
���ܣ���ָ���ӿ��½�ipaddress1 ��Ϊipaddresss2 ��ָ�����������뼰�㲥��ַ    
���룺
interface    -    ָ������ӿڵ����ƣ�һ���Ӧһ���������磺windows��Ϊ"��������"��
linux��Ϊһ������"eth0:x"������'eth0'Ϊ������ʶ��'x'��ʾ�ڼ���������eth0:1��ʾ��һ�������ĵ�һ��������
ipaddress1    -    Ҫ�޸ĵ�IP��ַ�����ʮ���ƣ��ַ�������ʽ���磺"192.168.0.1"��
ipaddress2    -    �޸ĺ��IP��ַ���磺"192.168.0.2"
subnetmask2    -    ָ��IP���������룬�磺"255.255.255.0"
broadcast2    -    ָ��IP�Ĺ㲥��ַ���磺"192.168.0.255"��windows����Ĭ�ϵ�,������XNULL��
�����N/A
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵������������Ľӿ�����IPʹ����Ҫ��֤������ԣ�LINUX�¿���ͨ��XOS_GetLocalIPList()
����ȡ��ǰ���õĽӿ�������Ӧ��IP��
************************************************************************/
XS32 XOS_LocalIPModify (XS8 *xinterface , XS8 *ipaddress1 , 
    XS8 *ipaddress2 , XS8 *subnetmask2 , XS8 *broadcast2 );


XS32 XOS_GetPhysicIP(XU32 *ipaddress);

XS32 XOS_ModifyPhysicIP (XU32 *ipaddress);

XS32 XOS_GetLogicIP (XU32 * ipaddress);

XS32 XOS_AddLogicIP (XU32* ipaddress);

XS32 XOS_DeleteLogicIP(XU32 *ipaddress);

XS32 XOS_ModifyLogicIP(XU32 *ipaddress);

XS32 XOS_GetMv2IP(XU32 *ipaddress);

XS32 XOS_GetMv1IP(XU32 *ipaddress);

#ifdef XOS_TSP
extern int Drv_GetMV0PhysicalIP (char *ipaddress , int *subnetmask , char *broadcast );

extern int Drv_MV0PhysicalIPModify (char *ipaddress , int subnetmask , char *broadcast );

extern int Drv_MV0LogicIPAdd (char *ipaddress , int subnetmask , char *broadcast );

extern int Drv_MV0LogicIPDelete(char *ipaddress );

extern int Drv_MV0LogicIPModify (char *ipaddress_old , char *ipaddress_new ,int subnetmask , char *broadcast );

extern int getLogicIp(XU32* ip, XU32* mask);

extern int Drv_GetMV2IP (char *ipaddress ,int *subnetmask , char *broadcast );

extern int Drv_GetMV1IP (char *ipaddress ,int *subnetmask , char *broadcast );
#endif

#ifdef XOS_LINUX
XS32 XOS_ModifyIfcfgOneLine(XS8 *newip);

XS32 XOS_GetPhysicIPByName(XS8 *name,XU32 *ipaddress,XU32 *mask,XU32 *broadcast);
XS32 XOS_ModifyPhysicIPByName (XS8 *name,XU32 *ipaddress);
XS32 XOS_ModifyIfcfgByName(XS8 *name,XS8 *newip);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*osenc.h*/
