/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosnetinf.h
**
**  description:  网口管理模块的头文件 
**
**  author: spj
**
**  date:   2014.12.22
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   spj             20014.12.22       create  
**************************************************************/
#ifndef _XOS_NET_INF_H_
#define _XOS_NET_INF_H_
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
/* 网卡接口的逻辑命名，由各网元业务使用 */
#define BASE1     "BASE1"
#define BASE2     "BASE2"
#define FABRIC1   "FABRIC1"
#define FABRIC2   "FABRIC2"
#define FRONT1    "FRONT1"
#define FRONT2    "FRONT2"
#define BACK1     "BACK1"
#define BACK2     "BACK2"
#define BACK3     "BACK3"
#define BACK4     "BACK4"
#define BACK5     "BACK5"
#define BACK6     "BACK6"

typedef enum
{
    E_BASE1 = 0,
    E_BASE2,
    E_FABRIC1,
    E_FABRIC2,
    E_FRONT1,
    E_FRONT2,
    E_BACK1,
    E_BACK2,
    E_BACK3,
    E_BACK4,
    E_BACK5,
    E_BACK6,
    E_PORT_UNKNOWN
}e_NetPort;
 
#define IP_EXIST        1  /* ip已经存在 */
#define IP_NOT_EXIST    0  /* ip不存在  */


#define  MAX_BOARD_TYPE_NUM 2  /* 前插板种类 */
#define  MAX_RTM_TYPE_NUM   2  /* 后插板种类 */
#define  MAX_BOARD_NAME_LEN 16 /* 板卡型号名称长度 */

#define  MAX_NETPORT_NUM    14 /* 最大网口数量 */
#define  MAX_DEV_NAME_LEN   17 /* 网口名称长度 */

/*-------------------------------------------------------------------------
                  结构和枚举声明
-------------------------------------------------------------------------*/
/* 网口名称映射关系 */
typedef struct
{
    XBOOL valid;  /* 有效标志位 */
    XS8  DevName[MAX_DEV_NAME_LEN];        /* 网口设备名称: 如eth0, eth1:1234 */
    XS8  LogicName[MAX_DEV_NAME_LEN];      /* 网口逻辑名称，我们根据其所在位置命名的名称,如BASE1 */
}t_NetDeviceMap;

/* 针对当前板卡类型和后卡类型，设备名和逻辑设备名的映射关系 */
typedef struct
{
    XBOOL init;           /* 初始化标志 */
    t_NetDeviceMap tNetDevMap[MAX_NETPORT_NUM]; /* 设备名称映射关系, 设备的枚举作为数组下标 枚举见 e_NetPort */
    XS8 BoardName[MAX_BOARD_NAME_LEN];    /* 板卡名称 */
    XS8 RtmName[MAX_BOARD_NAME_LEN];      /* 后卡名称 */
}t_BoardNeMaptInfo;

/*-------------------------------------------------------------------------
                      API 声明
-------------------------------------------------------------------------*/

XS32 XOS_NetInfInit(XVOID);
XS8* XOS_GetNetMap(XVOID);

/************************************************************************
函数名: XOS_LogicIfConvToDevIf
功能：逻辑设备名转化为实际设备名称
输入：ptLogicName : 逻辑设备名称
      DevNamelen  : 输出的实际设备名称buf的长度
输出：pDevName :    输出的实际设备名称
返回：XSUCC - 成功   XERROR - 失败
说明: 目前只支持linux
************************************************************************/
XS32 XOS_LogicIfConvToDevIf(const XS8 *ptLogicName,XS8 *pDevName,XS32 DevNamelen);

/************************************************************************
函数名: XOS_AddVirIp
功能：根据逻辑设备名称和传入的ip和netmask，添加虚拟设备
输入：pLogicIfName : 逻辑设备名称
      ip           : ip
      netmask      : 掩码
输出：
返回：成功返回0，失败返回-1
说明: 目前只支持linux
如果ip已在当前设备上存在，直接返回0
************************************************************************/
XS32 XOS_AddVirIp(const XS8* pLogicIfName, XU32 ip, XU32 netmask);

/************************************************************************
函数名: XOS_AddVirIp_Ex
功能：根据逻辑设备名称和传入的ip和netmask，添加虚拟设备,并传出添加的虚拟设备名
输入：pLogicIfName : 逻辑设备名称
      pGetIfName   : 带出添加的虚拟设备名称  ------ pGetIfName[]至少17个字节 ( XOS_IFNAMESIZE )
      ip           : ip
      netmask      : 掩码
输出：
返回：成功返回0，失败返回-1
************************************************************************/
XS32 XOS_AddVirIp_Ex(const XS8* pLogicIfName,XS8* pGetIfName, XU32 ip, XU32 netmask);

/************************************************************************
函数名: XOS_ModifyVirIp
功能：根据逻辑设备名称和传入的ip和netmask，修改虚拟设备ip为新ip
输入：pLogicIfName : 逻辑设备名称
      oldip        : 要修改的源ip  --- 本地序
      ip           : 设置的新的ip  --- 本地序
      netmask      : 掩码
输出：
返回：成功返回0，失败返回-1
说明: 目前只支持linux
如果oldip在当前设备上不存在,则相当于add接口
如果oldip存在，ip也存在，且ip与oldip不是同一个设备上，则删除旧设备，更新ip的设备为ip，netmask
如果oldip或ip其中一个存在，则找出此设备，并设置设备为ip，netmask
************************************************************************/
XS32 XOS_ModifyVirIp(const XS8* pLogicIfName, XU32 oldip, XU32 ip, XU32 netmask);

/************************************************************************
函数名: XOS_DeleteVirIp
功能：根据逻辑设备名称和传入ip，删除虚拟设备
输入：pLogicIfName : 逻辑设备名称
      ip           : ip
输出：
返回：成功返回0，失败返回-1
说明: 目前只支持linux
此接口不允许删除BASE1,BASE2,FABRIC1,FABRIC2的物理ip
************************************************************************/
XS32 XOS_DeleteVirIp(const XS8* pLogicIfName, XU32 ip);

/************************************************************************
函数名: XOS_CheckVirIp
功能：检查ip在传入的逻辑设备上是否存在
输入：pLogicIfName : 逻辑设备名称
      ip           : ip
输出：
返回：1: ip存在  (IP_EXIST)
      0: ip不存在 (IP_NOT_EXIST)
     -1: 返回错误
说明: 目前只支持linux
判断ip是否存在于当前逻辑设备
************************************************************************/
XS32 XOS_CheckVirIp(const XS8* pLogicIfName, XU32 ip);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*_XOS_NET_INF_H_*/
