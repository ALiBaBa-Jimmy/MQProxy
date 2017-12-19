#ifndef __DIAM_PEER_DEFINE_H__
#define __DIAM_PEER_DEFINE_H__

#include <api/diam_datatype.h>
#include <api/diam_define.h>
#include <entity/diam_app_config.h>
#include <util/diam_queue.h>

#define DIAM_RELAY_APPLICATION_ID          0xffffffff

typedef enum
{
    DIAM_PEER_ST_CLOSED,
    DIAM_PEER_ST_WAIT_CONN_ACK,
    DIAM_PEER_ST_WAIT_I_CEA,
    DIAM_PEER_ST_WAIT_CONN_ACK_ELECT,
    DIAM_PEER_ST_WAIT_RETURNS,
    DIAM_PEER_ST_I_OPEN,
    DIAM_PEER_ST_R_OPEN,
    DIAM_PEER_ST_CLOSING
} EDiamPeerState;

typedef enum
{
    DIAM_PEER_OPEN,
    DIAM_PEER_CLOSE
    /*
    DIAM_PEER_EV_START,
    DIAM_PEER_EV_STOP,
    DIAM_PEER_EV_TIMEOUT,
    DIAM_PEER_EV_CONN_RETRY,
    DIAM_PEER_EV_R_CONN_CER,
    DIAM_PEER_EV_I_RCV_CONN_ACK,
    DIAM_PEER_EV_I_RCV_CONN_NACK,
    DIAM_PEER_EV_R_RCV_CEA,
    DIAM_PEER_EV_I_RCV_CEA,
    DIAM_PEER_EV_I_PEER_DISC,
    DIAM_PEER_EV_R_PEER_DISC,
    DIAM_PEER_EV_I_RCV_NON_CEA,
    DIAM_PEER_EV_WIN_ELECTION,
    DIAM_PEER_EV_SEND_MESSAGE,
    DIAM_PEER_EV_R_RCV_MESSAGE,
    DIAM_PEER_EV_I_RCV_MESSAGE,
    DIAM_PEER_EV_R_RCV_DWR,
    DIAM_PEER_EV_I_RCV_DWR,
    DIAM_PEER_EV_R_RCV_DWA,
    DIAM_PEER_EV_I_RCV_DWA,
    DIAM_PEER_EV_R_RCV_DPR,
    DIAM_PEER_EV_I_RCV_DPR,
    DIAM_PEER_EV_R_RCV_CER,
    DIAM_PEER_EV_I_RCV_CER,
    DIAM_PEER_EV_R_RCV_DPA,
    DIAM_PEER_EV_I_RCV_DPA,
    DIAM_PEER_EV_WATCHDOG,
    DIAM_PEER_CONNECT_ATTEMPT_TOUT,
    DIAM_PEER_EV_WATCHDOG_TIMEOUT  //看门狗响应超时定时器事件
    */
} EDiamPeerEvent;

typedef enum
{
    DIAM_MODE_CLIENT     = 1,
    DIAM_MODE_SERVER     = 2
} EDiamServMode;

typedef enum
{
    DIAM_DISCONNECT_REBOOTING       = 0,
    DIAM_DISCONNECT_BUSY            = 1,
    DIAM_DISCONNECT_DONTWANTTOTALK  = 2,
    DIAM_DISCONNECT_UNKNOWN         = 1000,
    DIAM_DISCONNECT_TRANSPORT       = 10001,
    DIAM_DISCONNECT_TIMEOUT         = 10002
} EDiamDisconnectCause;

typedef struct
{
    DiamProtectedMap<DiamUINT32, DiamUINT32> m_LastTxHopId;
    DiamProtectedMap<DiamUINT32, DiamUINT32> m_LastTxEndId;
    DiamUINT32                               m_LastRxHopId;
    DiamUINT32                               m_LastRxEndId;
} MsgId;

//对等端能力数据
typedef struct
{
    DiamUTF8String              m_Host;                //主机名
    DiamUTF8String              m_Realm;               //域名
    DiamHostIpList              m_HostIpLst;           //IP地址列表
    DiamUINT32                  m_VendorId;            //
    DiamUTF8String              m_ProductName;         //产品名称
    DiamUINT32                  m_OriginStateId;       //
    DiamSupportedVendorIdLst    m_SupportedVendorIdLst;//
    DiamApplicationIdLst        m_AuthAppIdLst;
    DiamApplicationIdLst        m_AcctAppIdLst;
    DiamUINT32                  m_InbandSecurityId;    //
    DiamAppId                   m_ApplicationId;       //
    DiamVendorSpecificIdLst     m_VendorSpecificId;    //
    DiamUINT32                  m_FirmwareRevision;    //
    MsgId                       m_MsgId;
} DiamPeerCapabilities;

#endif //__DIAM_PEER_DEFINE_H__

