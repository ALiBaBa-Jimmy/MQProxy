/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosnetinf.h
**
**  description:  ���ڹ���ģ���ͷ�ļ� 
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
/* �����ӿڵ��߼��������ɸ���Ԫҵ��ʹ�� */
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
 
#define IP_EXIST        1  /* ip�Ѿ����� */
#define IP_NOT_EXIST    0  /* ip������  */


#define  MAX_BOARD_TYPE_NUM 2  /* ǰ������� */
#define  MAX_RTM_TYPE_NUM   2  /* �������� */
#define  MAX_BOARD_NAME_LEN 16 /* �忨�ͺ����Ƴ��� */

#define  MAX_NETPORT_NUM    14 /* ����������� */
#define  MAX_DEV_NAME_LEN   17 /* �������Ƴ��� */

/*-------------------------------------------------------------------------
                  �ṹ��ö������
-------------------------------------------------------------------------*/
/* ��������ӳ���ϵ */
typedef struct
{
    XBOOL valid;  /* ��Ч��־λ */
    XS8  DevName[MAX_DEV_NAME_LEN];        /* �����豸����: ��eth0, eth1:1234 */
    XS8  LogicName[MAX_DEV_NAME_LEN];      /* �����߼����ƣ����Ǹ���������λ������������,��BASE1 */
}t_NetDeviceMap;

/* ��Ե�ǰ�忨���ͺͺ����ͣ��豸�����߼��豸����ӳ���ϵ */
typedef struct
{
    XBOOL init;           /* ��ʼ����־ */
    t_NetDeviceMap tNetDevMap[MAX_NETPORT_NUM]; /* �豸����ӳ���ϵ, �豸��ö����Ϊ�����±� ö�ټ� e_NetPort */
    XS8 BoardName[MAX_BOARD_NAME_LEN];    /* �忨���� */
    XS8 RtmName[MAX_BOARD_NAME_LEN];      /* ������ */
}t_BoardNeMaptInfo;

/*-------------------------------------------------------------------------
                      API ����
-------------------------------------------------------------------------*/

XS32 XOS_NetInfInit(XVOID);
XS8* XOS_GetNetMap(XVOID);

/************************************************************************
������: XOS_LogicIfConvToDevIf
���ܣ��߼��豸��ת��Ϊʵ���豸����
���룺ptLogicName : �߼��豸����
      DevNamelen  : �����ʵ���豸����buf�ĳ���
�����pDevName :    �����ʵ���豸����
���أ�XSUCC - �ɹ�   XERROR - ʧ��
˵��: Ŀǰֻ֧��linux
************************************************************************/
XS32 XOS_LogicIfConvToDevIf(const XS8 *ptLogicName,XS8 *pDevName,XS32 DevNamelen);

/************************************************************************
������: XOS_AddVirIp
���ܣ������߼��豸���ƺʹ����ip��netmask����������豸
���룺pLogicIfName : �߼��豸����
      ip           : ip
      netmask      : ����
�����
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
���ip���ڵ�ǰ�豸�ϴ��ڣ�ֱ�ӷ���0
************************************************************************/
XS32 XOS_AddVirIp(const XS8* pLogicIfName, XU32 ip, XU32 netmask);

/************************************************************************
������: XOS_AddVirIp_Ex
���ܣ������߼��豸���ƺʹ����ip��netmask����������豸,��������ӵ������豸��
���룺pLogicIfName : �߼��豸����
      pGetIfName   : ������ӵ������豸����  ------ pGetIfName[]����17���ֽ� ( XOS_IFNAMESIZE )
      ip           : ip
      netmask      : ����
�����
���أ��ɹ�����0��ʧ�ܷ���-1
************************************************************************/
XS32 XOS_AddVirIp_Ex(const XS8* pLogicIfName,XS8* pGetIfName, XU32 ip, XU32 netmask);

/************************************************************************
������: XOS_ModifyVirIp
���ܣ������߼��豸���ƺʹ����ip��netmask���޸������豸ipΪ��ip
���룺pLogicIfName : �߼��豸����
      oldip        : Ҫ�޸ĵ�Դip  --- ������
      ip           : ���õ��µ�ip  --- ������
      netmask      : ����
�����
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
���oldip�ڵ�ǰ�豸�ϲ�����,���൱��add�ӿ�
���oldip���ڣ�ipҲ���ڣ���ip��oldip����ͬһ���豸�ϣ���ɾ�����豸������ip���豸Ϊip��netmask
���oldip��ip����һ�����ڣ����ҳ����豸���������豸Ϊip��netmask
************************************************************************/
XS32 XOS_ModifyVirIp(const XS8* pLogicIfName, XU32 oldip, XU32 ip, XU32 netmask);

/************************************************************************
������: XOS_DeleteVirIp
���ܣ������߼��豸���ƺʹ���ip��ɾ�������豸
���룺pLogicIfName : �߼��豸����
      ip           : ip
�����
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
�˽ӿڲ�����ɾ��BASE1,BASE2,FABRIC1,FABRIC2������ip
************************************************************************/
XS32 XOS_DeleteVirIp(const XS8* pLogicIfName, XU32 ip);

/************************************************************************
������: XOS_CheckVirIp
���ܣ����ip�ڴ�����߼��豸���Ƿ����
���룺pLogicIfName : �߼��豸����
      ip           : ip
�����
���أ�1: ip����  (IP_EXIST)
      0: ip������ (IP_NOT_EXIST)
     -1: ���ش���
˵��: Ŀǰֻ֧��linux
�ж�ip�Ƿ�����ڵ�ǰ�߼��豸
************************************************************************/
XS32 XOS_CheckVirIp(const XS8* pLogicIfName, XU32 ip);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*_XOS_NET_INF_H_*/
