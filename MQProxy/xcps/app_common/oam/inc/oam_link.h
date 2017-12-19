/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     oam_link.h
* Author:       weizhao
* Date：        2014-09-26
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
#define MAX_RETRY_NUM           (3)     //最大重试次数
#define OAM_LINK_APPHANDLE      (1)     //OAM链路appHandle
#define MAX_LINK_NUM            (2)     //主备两条链路
#define MAX_IPSTR_LEN           (16)    //ip最大长度
    
#define NE_PORT_BASE            (15000) //计算网元进程端口时的起始值
#define NE_PORT_NEID_OFFSET     (7)     //计算网元进程端口时网元ID偏移值
#define NE_PORT_PID_OFFSET      (1)     //计算网元进程端口时PID偏移值

#define AGT_PORT                (9999)  //agent工作端口
#define OAM_BM_PORT             (9991)  //BM工作端口
#define OAM_TS_PORT             (9992)  //TS工作端口

#define OAM_BM_PID              (128)   //BM Pid
#define OAM_TS_PID              (129)   //TS Pid


//链路状态
typedef enum link_state_e{
    LINK_STATE_NULL = 0,
    LINK_STATE_LINKINIT,
    LINK_STATE_LINKSTART,    
    LINK_STATE_CONNIND,
    LINK_STATE_REG,
    LINK_STATE_CONNECT,
    LINK_STATE_DISCONNECT,
}LINK_STATE_E;

//定时器类别
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

//客户端控制块
typedef struct oam_link_cb_t
{
    XU32                uiAppHandle;        //链路标识
    HLINKHANDLE         hLinkHandle;        //底层链路句柄

    XS32                siFrameId;          //框号
    XS32                siSlotId;           //槽号
    XU16                usNeId;             //网元ID
    XU16                usPid;              //模块ID
    
    XU32                uiIpAddr[MAX_LINK_NUM];  //ip地址列表
    XU16                usPort;                  //监听端口

    XU32                uiPeerIp[MAX_LINK_NUM];  //服务端IP地址
    XU16                usPeerPort;              //服务端端口

    XU32                uiState;            //链路状态
    XU32                uiRegSeq;           //注册序号
    XU32                uiHsSeq;            //握手序号
    XU32                uiTimeOutCnt;       //超时次数
    PTIMER              pTmHandle;          //定时器句柄
}OAM_LINK_CB_T;

//reg msg struct
typedef struct link_reg_t{
    XU32    uiRegSeq;   //对端注册消息序列号
    XU32    uiPID;      //进程ID
    XU32    uiNeID;     //网元ID
    XU32    uiFrameId;  //框号
    XU32    uiSlotId;   //槽位号
}LINK_REG_T;

//reg ack msg struct
typedef struct link_regack_t{
    XU32    uiRegAckSeq;//注册消息序列号
}LINK_REGACK_T;

//hs msg struct
typedef struct link_hs_t{
    XU32    uiHsSeq;    //握手消息序列号

    /*以下字段是客户端需要填写的,服务端不用*/
    XU32    uiPID;      //进程ID
    XU32    uiNeID;     //网元ID
    XU32    uiFrameId;  //框号
    XU32    uiSlotId;   //槽位号
}LINK_HS_T;

//hs ack msg struct
typedef struct link_hsack_t{
    XU32    uiHsAckSeq; //握手回应消息序号
}LINK_HSACK_T;

typedef struct oam_link_cfg_t{
    XU32    peerIp;                    //对端IP
    XU32    localIp[MAX_LINK_NUM];     //本端IP, base1, base2
    XS32    siNeID;                    //网元ID
    XS32    siPID;                     //进程ID
    XS32    siFrameId;                 //框号
    XS32    siSlotId;                  //槽号
}OAM_LINK_CFG_T;

typedef struct oam_link_t{
    t_IPADDR    stLocalAddr;     //本端地址
    HLINKHANDLE hLinkHandle;    //底层链路索引
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
