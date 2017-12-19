/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     oam_link.h
* Author:       weizhao
* Date��        2014-09-26
* OverView:     oam link managemnet
*
* History:      create
* Revisor:      
* Date:         2014-09-26
* Description:  create the file
*******************************************************************************/
#ifndef __OAM_LINK_H__
#define __OAM_LINK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "oam_main.h"

#pragma pack(1)

#define LINK_HS_TIMEOUT_LEN     (3000)  //Unit: ms
#define LINK_TIMEOUT_LEN        (2000)  //Unit: ms
#define LINK_SYNC_TIMEOUT_LEN   (1000)  //Unit: ms
#define MAX_RETRY_NUM           (3)     //������Դ���
#define OAM_LINK_APPHANDLE      (1)     //OAM��·appHandle
#define MAX_LINK_NUM            (2)     //����������·
#define MAX_IPSTR_LEN           (16)    //ip��󳤶�
    
#define NE_PORT_BASE            (15000) //������Ԫ���̶˿�ʱ����ʼֵ
#define NE_PORT_NEID_OFFSET     (7)     //������Ԫ���̶˿�ʱ��ԪIDƫ��ֵ
#define NE_PORT_PID_OFFSET      (1)     //������Ԫ���̶˿�ʱPIDƫ��ֵ

#define AGT_PORT                (9999)  //agent�����˿�
#define OAM_BM_PORT             (9991)  //BM�����˿�
#define OAM_TS_PORT             (9992)  //TS�����˿�

#define OAM_BM_PID              (128)   //BM Pid
#define OAM_TS_PID              (129)   //TS Pid


//��·״̬
typedef enum link_state_e{
    LINK_STATE_NULL = 0,
    LINK_STATE_LINKINIT,
    LINK_STATE_LINKSTART,    
    LINK_STATE_CONNIND,
    LINK_STATE_REG,
    LINK_STATE_CONNECT,
    LINK_STATE_DISCONNECT,
}LINK_STATE_E;

//��ʱ�����
typedef enum tm_type_e{
    TM_TYPE_NULL = 0,
    TM_TYPE_LINKINIT,
    TM_TYPE_LINKSTART,
    TM_TYPE_REG,
    TM_TYPE_HS,
    TM_TYPE_HS_ACK,
    TM_TYPE_SYNC,
}TM_TYPE_E;

//msg type
typedef enum link_msg_type_e{
    LINK_MSG_REG = 1,
    LINK_MSG_REG_ACK,
    LINK_MSG_HS,
    LINK_MSG_HS_ACK,
    LINK_MSG_OTHER,
}LINK_MSG_TYPE_E;

//�ͻ��˿��ƿ�
typedef struct oam_link_cb_t
{
    XU32                uiAppHandle;        //��·��ʶ
    HLINKHANDLE         hLinkHandle;        //�ײ���·���

    XS32                siFrameId;          //���
    XS32                siSlotId;           //�ۺ�
    XU16                usNeId;             //��ԪID
    XU16                usPid;              //ģ��ID
    
    XU32                uiIpAddr[MAX_LINK_NUM];  //ip��ַ�б�
    XU16                usPort;                  //�����˿�

    XU32                uiPeerIp[MAX_LINK_NUM];  //�����IP��ַ
    XU16                usPeerPort;              //����˶˿�

    XU32                uiState;            //��·״̬
    XU32                uiRegSeq;           //ע�����
    XU32                uiHsSeq;            //�������
    XU32                uiTimeOutCnt;       //��ʱ����
    PTIMER              pTmHandle;          //��ʱ�����
}OAM_LINK_CB_T;

//reg msg struct
typedef struct link_reg_t{
    XU32    uiRegSeq;   //�Զ�ע����Ϣ���к�
    XU32    uiPID;      //����ID
    XU32    uiNeID;     //��ԪID
    XU32    uiFrameId;  //���
    XU32    uiSlotId;   //��λ��
}LINK_REG_T;

//reg ack msg struct
typedef struct link_regack_t{
    XU32    uiRegAckSeq;//ע����Ϣ���к�
}LINK_REGACK_T;

//hs msg struct
typedef struct link_hs_t{
    XU32    uiHsSeq;    //������Ϣ���к�

    /*�����ֶ��ǿͻ�����Ҫ��д��,����˲���*/
    XU32    uiPID;      //����ID
    XU32    uiNeID;     //��ԪID
    XU32    uiFrameId;  //���
    XU32    uiSlotId;   //��λ��
}LINK_HS_T;

//hs ack msg struct
typedef struct link_hsack_t{
    XU32    uiHsAckSeq; //���ֻ�Ӧ��Ϣ���
}LINK_HSACK_T;

typedef struct oam_link_cfg_t{
    XU32    peerIp;                    //�Զ�IP
    XU32    localIp[MAX_LINK_NUM];     //����IP, base1, base2
    XS32    siNeID;                    //��ԪID
    XS32    siPID;                     //����ID
    XS32    siFrameId;                 //���
    XS32    siSlotId;                  //�ۺ�
}OAM_LINK_CFG_T;

typedef struct oam_link_t{
    t_IPADDR    stLocalAddr;     //���˵�ַ
    HLINKHANDLE hLinkHandle;    //�ײ���·����
}OAM_LINK_T;

#pragma pack()


/******************   API   ******************/
XVOID OAM_LinkCBInit(OAM_LINK_CFG_T oamCfg);
XS8  OAM_LinkInit();
XS32 OAM_LinkInitAckProc(t_XOSCOMMHEAD* pMsg);
XS32 OAM_LinkStart();
XS32 OAM_LinkStartAckProc(t_XOSCOMMHEAD* pMsg);
XS32 OAM_LinkDataIndProc(t_XOSCOMMHEAD* pMsg);
XS32 OAM_LinkStopIndProc(t_XOSCOMMHEAD* pMsg);
XS32 OAM_LinkRelease();
XS8  OAM_LinkRegSend();
XS32 OAM_LinkRegAckProc(XVOID* pRegMsg);
XS8  OAM_LinkHsSend();
XS32 OAM_LinkHsProc(XVOID* pHsMsg);
XS32 OAM_LinkHsAckSend(XVOID* pHsMsg);
XS32 OAM_LinkHsAckProc(XVOID* pHsAckMsg);

XS8 OAM_LinkTimeOutProc(t_BACKPARA* para);
XS32 OAM_LinkTimerStart(PTIMER* ptHandle, XU32 len, e_TIMERTYPE mode, 
                               XU32 backpara1, XU32 backpara2, XU32 backpara3, XU32 backpara4);
XS32 OAM_LinkTimerStop(PTIMER* ptHandle);

XS32 OAM_LinkGet(OAM_LINK_T* pOamLink);

XVOID OAM_LinkAgtBaseIpAddrGet(XU32 uiIp, XU32 *puiBase1Ip, XU32 *puiBase2Ip);

#ifdef __cplusplus
}
#endif

#endif//__OAM_LINK_H__
