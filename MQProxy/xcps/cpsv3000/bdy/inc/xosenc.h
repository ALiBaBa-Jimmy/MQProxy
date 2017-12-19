/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: osenc.h
**
**  description:  多IP管理模块的头文件 
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
               包含头文件
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
                  宏定义
-------------------------------------------------------------------------*/

#define XOS_MAXCMDLEN   128  /* 命令buf最大长度 */
#define XOS_IPSTRLEN    16   /* a.b.c.d 点分ip字符串的长度 */
#define XOS_IFNAMESIZE  17   /* 网口设备名称最大长度 */
#define XOS_MAXIPNUM    128  /* 最大网口设备数量 (包括虚拟设备) */

/*-------------------------------------------------------------------------
                  结构和枚举声明
-------------------------------------------------------------------------*/

/*IP信息列表结构体*/
typedef struct _IP_LIST
{
    XS8   xinterface[XOS_IFNAMESIZE];    /*IP所属的接口（名），NAMESIZE =16*/
    XU32  LocalIPaddr;    /*获取到的IP地址*/
    XU32  LocalNetMask;   /* 掩码 */
}t_IPLIST;

typedef struct  _LOCALIP_INFO
{
    XU32  nIPNum;    /*获取到IP的个数*/
    t_IPLIST        localIP[XOS_MAXIPNUM];    /*最多支持? 个*/
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
                      API 声明
-------------------------------------------------------------------------*/
/************************************************************************
函数名:    XOS_GetLocalIPList
功能：获取本地设备的网络接口和IP地址列表    
输入： pLocalIPList        -        用来保存IP信息列表的结构体
输出： 
pLocalIPList.nIPNum    -    获取到IP地址的个数, 
pLocalIPList. LocalIP[i]. Interface        -    IP所属的接口，
pLocalIPList. LocalIP[i]. LocalIPaddr    -    32 位的IP地址
如： pLocalIPList.localIP[0].LocalIPaddr 是列表中第一个本机IP地址.
返回：
XSUCC    -    成功
XERROR    -    失败
说明：目前在windows下无法取得对应的IP地址
************************************************************************/
XS32 XOS_GetLocalIPList ( t_LOCALIPINFO * pLocalIPList );

/************************************************************************
函数名:    XOS_LocalIPAdd
功能： 在本地接口中加入一个IP地址    
输入： 
interface    -    指定网络接口的名称（一般对应一个网卡，如：windows下为"本地连接"；
linux下为一个别名"eth0:x"，其中'eth0'为网卡标识，
'x'表示第几个别名，"eth0:1"表示第一块网卡的第一个别名）
ipaddress    -    要添加到此接口的IP地址，点分十进制（字符串的形式，如："192.168.0.1"）
subnetmask    -    指定IP的子网掩码，如："255.255.255.0"
broadcast    -    指定IP的广播地址，如："192.168.0.255"（windows下是默认的,可以填XNULL）
输出：N/A
返回：
XSUCC    -    成功
XERROR    -    失败
说明：同一接口（别名）下多次添加相同的IP将返回XERROR；
对于输入的接口名（别名）使用者要保证其可用性，LINUX下可以通过XOS_GetLocalIPList() 
来获取当前的接口名及对应IP来做参考，还要注意掩码及广播地址的格式及有效性。
************************************************************************/
XS32 XOS_LocalIPAdd ( XS8 *xinterface , XS8 *ipaddress , XS8 *subnetmask , XS8 *broadcast );

/************************************************************************
函数名:    XOS_LocalIPDelete
功能：删除指定接口下的一个IP地址    
输入：
interface    -    指定网络接口的名称（一般对应一个网卡，如：windows下为"本地连接"；
linux下为一个别名"eth0:x"，其中'eth0'为网卡标识，'x'表示第几个别名，eth0:1表示第一块网卡的第一个别名）
ipaddress    -    要删除的IP地址，点分十进制（字符串的形式，如："192.168.0.1"）
输出：N/A
返回：
XSUCC    -    成功
XERROR    -    失败
说明：多次删除同一IP地址将返回XERROR；
对于输入的接口名使用者要保证其可用性，LINUX下可以通过的XOS_GetLocalIPList() 
来获取当前的接口名及对应IP来做参考；
注：只能删除静态IP，每个接口的最后一个静态IP不能删除，LINUX下是主接口（非别名,如:"eth0"）的IP不能删除；
************************************************************************/
XS32 XOS_LocalIPDelete (XS8 *xinterface , XS8 *ipaddress );

/************************************************************************
函数名:    XOS_LocalIPModify
功能：在指定接口下将ipaddress1 改为ipaddresss2 并指定其子网掩码及广播地址    
输入：
interface    -    指定网络接口的名称（一般对应一个网卡，如：windows下为"本地连接"；
linux下为一个别名"eth0:x"，其中'eth0'为网卡标识，'x'表示第几个别名，eth0:1表示第一块网卡的第一个别名）
ipaddress1    -    要修改的IP地址，点分十进制（字符串的形式，如："192.168.0.1"）
ipaddress2    -    修改后的IP地址，如："192.168.0.2"
subnetmask2    -    指定IP的子网掩码，如："255.255.255.0"
broadcast2    -    指定IP的广播地址，如："192.168.0.255"（windows下是默认的,可以填XNULL）
输出：N/A
返回：
XSUCC    -    成功
XERROR    -    失败
说明：对于输入的接口名及IP使用者要保证其可用性，LINUX下可以通过XOS_GetLocalIPList()
来获取当前可用的接口名及对应的IP；
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
