/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  filename:       diam_datatype.h
**  description:
**  author:         panjian
**  data:           2014.07.16
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   panjian         2014.07.16        create
**************************************************************/
#ifndef __DIAM_DIFINE_H__
#define __DIAM_DIFINE_H__

#include <api/diam_datatype.h>

typedef enum
{
    LOG_TRACE   = 0,
    LOG_DEBUG   = 1,
    LOG_INFO    = 2,
    LOG_WARNING = 3,
    LOG_ERROR   = 4,
    LOG_OFF     = 5
} ELogLevel;

typedef enum
{
    DIAM_HSS    = 0,
    DIAM_MME    = 1,
    DIAM_TCF    = 2,
    DIAM_SMC    = 3,
    DIAM_TAS    = 4,
    DIAM_SPGW   = 5,
    DIAM_PCRF   = 6
} EDiamNEType;

typedef enum
{
    DIAM_RET_NOT_FOUND =     -2,
    DIAM_RET_FAILURE =       -1,
    DIAM_RET_SUCCESS =        0,
    DIAM_RET_NOMEM,
    DIAM_RET_PROTO,
    DIAM_RET_SECURITY,
    DIAM_RET_PARAMETER,
    DIAM_RET_CONFIG,
    DIAM_RET_UNKNOWN_CMD,
    DIAM_RET_MISSING_AVP,
    DIAM_RET_ALREADY_INIT,
    DIAM_RET_TIMED_OUT,
    DIAM_RET_CANNOT_SEND_MSG,
    DIAM_RET_ALREADY_REGISTERED,
    DIAM_RET_CANNOT_REGISTER,
    DIAM_RET_NOT_INITIALIZED,
    DIAM_RET_NETWORK_ERROR,
    DIAM_RET_MSG_UNPROCESSED,
    DIAM_RET_INVALID_STATE,
    DIAM_RET_PARSING_FAILED,
    DIAM_RET_UNKNOWN_SESSION,
    DIAM_RET_PARSING_ERROR,
    DIAM_RET_INCOMPLETE,
    DIAM_RET_NOSERVICE,
    DIAM_RET_PARAM_ERR,
    DIAM_RET_PEER_ERR,
    DIAM_RET_PEER_NULL,
    DIAM_RET_PORT_NULL,
    DIAM_RET_IDENT_NULL,
    DIAM_RET_REALM_NULL
} DiamRetCode;

typedef enum
{
    PARSE_TYPE_FIXED_HEAD = 0,
    PARSE_TYPE_REQUIRED,
    PARSE_TYPE_OPTIONAL
} DiamAvpParseType;

typedef enum
{
    PROTOCOL_TCP  = 0,
    PROTOCOL_SCTP = 1
} PROTOCOL_TYPE;

typedef enum
{
    DIAM_LINK_EV_CONN  = 0, //链路成功事件
    DIAM_LINK_EV_DISC  = 1  //链路断连事件
} DiamLinkEvent;

#define HEADFLAG_REQ     1
#define HEADFLAG_ANS     0
#define HEADFLAG_P       0
#define HEADFLAG_E       0
#define HEADFLAG_T       0
#define HEADFLAG_U       0
#define HEADFLAG_RSVD    0
#define DIAMETER_VERSION 1
#define INIT_HOPBYHOP    0
#define INIT_ENDTOEND    0
#define INIT_COMMANDCODE 0

#define  VENDOR_ID                         10000
//
// avp定义
//
#define DIAM_AVPNAME_SESSIONID             "Session-Id"
#define DIAM_AVPNAME_RESULTCODE            "Result-Code"
#define DIAM_AVPNAME_EXPERIMENTALRESULT    "Experimental-Result"
#define DIAM_AVPNAME_EXPERIMENTALRESULTCODE "Experimental-Result-Code"
#define DIAM_AVPNAME_ORIGINHOST            "Origin-Host"
#define DIAM_AVPNAME_ORIGINREALM           "Origin-Realm"
#define DIAM_AVPNAME_ORIGINSTATEID         "Origin-State-Id"
//
#define DIAM_AVPNAME_ORIGIN_IP             "Origin-IP"
//
#define DIAM_AVPNAME_DESTHOST              "Destination-Host"
#define DIAM_AVPNAME_DESTREALM             "Destination-Realm"
#define DIAM_AVPNAME_AUTHSESSIONSTATE      "Auth-Session-State"
#define DIAM_AVPNAME_AUTHAPPID             "Auth-Application-Id"
#define DIAM_AVPNAME_ACCTAPPID             "Acct-Application-Id"
#define DIAM_AVPNAME_VENDORAPPID           "Vendor-Specific-Application-Id"
#define DIAM_AVPNAME_REAUTHREQTYPE         "Re-Auth-Request-Type"
#define DIAM_AVPNAME_TERMINATION           "Termination-Cause"
#define DIAM_AVPNAME_ERRRORREPORTINGHOST   "Error-Reporting-Host"
#define DIAM_AVPNAME_AUTHLIFETIME          "Authorization-Lifetime"
#define DIAM_AVPNAME_AUTHGRACE             "Auth-Grace-Period"
#define DIAM_AVPNAME_SESSIONTIMEOUT        "Session-Timeout"
#define DIAM_AVPNAME_HOSTIP                "Host-IP-Address"
#define DIAM_AVPNAME_VENDORID              "Vendor-Id"
#define DIAM_AVPNAME_PRODUCTNAME           "Product-Name"
#define DIAM_AVPNAME_ROUTERECORD           "Route-Record"
#define DIAM_AVPNAME_REDIRECTHOST          "Redirect-Host"
#define DIAM_AVPNAME_REDIRECTHOSTUSAGE     "Redirect-Host-Usage"
#define DIAM_AVPNAME_USERNAME              "SUID"
#define DIAM_AVPNAME_SUID                  "SUID"
#define DIAM_AVPNAME_FIRMWAREREV           "Firmware-Revision"
#define DIAM_AVPNAME_INBANDSECID           "Inband-Security-Id"
#define DIAM_AVPNAME_SUPPORTEDVENDORID     "Supported-Vendor-Id"
#define DIAM_AVPNAME_ERRORMESSAGE          "Error-Message"
#define DIAM_AVPNAME_ERRORREPORTINGHOST    "Error-Reporting-Host"
#define DIAM_AVPNAME_DISCONNECT_CAUSE      "Disconnect-Cause"
#define DIAM_AVPNAME_ACCTREC_TYPE          "Accounting-Record-Type"
#define DIAM_AVPNAME_ACCTREC_NUM           "Accounting-Record-Number"
#define DIAM_AVPNAME_ACCTSUBSID            "Accounting-Sub-Session-Id"
#define DIAM_AVPNAME_ACCTREALTIME          "Accounting-Realtime-Required"
#define DIAM_AVPNAME_ACCTSID               "Acct-Session-Id"
#define DIAM_AVPNAME_ACCTMULTISID          "Acct-Multi-Session-Id"
#define DIAM_AVPNAME_ACCTINTERVAL          "Acct-Interim-Interval"
#define DIAM_AVPNAME_CLASS                 "Class"
#define DIAM_AVPNAME_PROXYINFO             "Proxy-Info"
#define DIAM_AVPNAME_PROXYHOST             "Proxy-Host"
#define DIAM_AVPNAME_PROXYSTATE            "Proxy-State"
#define DIAM_AVPNAME_WILDCARD              "AVP"
#define DIAM_MSGASSOCIATEID                "AssociateId"
#define DIAM_MSGSEQUENCEID                 "SequenceId"

#define DIAM_SESSION_STATE_MAINTAINED       0
#define DIAM_SESSION_NO_STATE_MAINTAINED    1

#define DIAM_MSGCODE_ABORTSESSION          274
#define DIAM_MSGCODE_SESSIONTERMINATION    275
#define DIAM_MSGCODE_CAPABILITIES_EXCHG    257
#define DIAM_MSGCODE_WATCHDOG              280
#define DIAM_MSGCODE_DISCONNECT_PEER       282
#define DIAM_MSGCODE_ACCOUNTING            271
#define DIAM_MSGCODE_REAUTH                258


typedef struct _PeerInfo
{
    DiamUINT32          linkId;
    DiamUINT32          serverId;
    DiamChar            localMasterIp[16];
    DiamChar            localSlaveIp[16];
    DiamUINT32          localPort;
    DiamChar            remoteMasterIp[16];
    DiamChar            remoteSlaveIp[16];
    DiamUINT32          remotePort;
    DiamUINT32          linkStatus;        //链路状态(链路状态表示双方协商通过)
    DiamUINT32          connectStatus;     //TCP/SCTP连接状态
    DiamChar            protocol[5];
    DiamUINT32          sendMsgCnt;
    DiamUINT32          recvMsgCnt;
} PeerInfo;

typedef struct
{
    DiamUINT32          size;
    PeerInfo            peerInfo[16];
} ConnectInfo;

#endif

