#pragma once


#include "agentuaCommon.h"
#include "xoslist.h"
#include "xostrace.h"
#include "oam_main.h"
#include "smu_sj3_type.h"

#pragma pack(1)

typedef enum
{

    MML_UA_LINK_TABLE_ID = 2007,
    MML_UA_LINK_TOUDC_TABLE_ID,

    TABLE_ID_TYPE_BUTT

}TABEL_ID_TYPE;
#define MML_PE_TCF_TABLE_ID 601
#define MML_PE_TAS_TABLE_ID 612
#define MML_PE_SMPP_TABLE_ID 2005
#define MML_PE_SMCROAM_TABLE_ID 2006
#define MAX_LINK_DESC_DEF 128
#define MAX_SMPP_LINK_COUNT 32
#define MAX_HEART_BEAT_TIMES 5

#define MAX_HOST_DEF              16

//PE 链路告警中序列号合成用
#define SMPP_BEGIN_INDEX    0
#define TCF_BEGIN_INDEX     0x1000000
#define INNERGW_BEGIN_INDEX 0x2000000
#define TAS_BEGIN_INDEX     0x3000000
//网元类型
typedef enum
{
    NODE_SMPP = 1,
    NODE_TCF,   
	NODE_INNERGW,
	NODE_TAS,
	PEER_AGENT,
	PEER_HLR,
	PEER_UDC
}PEER_TYPE;

//链路状态
typedef enum
{
	LINKSTATUS_OK = 1,
    LINKSTATUS_ERROR
}LINK_STATUS;

typedef struct
{
	XU32 index;                                 //索引
	XU8  smcMasterIp[MAX_HOST_DEF];				//smc master ip
	XU8  smcSlaveIp[MAX_HOST_DEF];              //smc slave ip
	XU16 smcPort;								//smc port
	XU8  peerMasterIp[MAX_HOST_DEF];			//peer master ip
	XU8  peerSlaveIp[MAX_HOST_DEF];             //peer slave ip
	XU16 peerPort;								//peer port
	XU8  linkDesc[MAX_LINK_DESC_DEF];			//链路描述
	XU8  linkStatus;							//smc侧链路状态
}tPeTcfCfgOamTable;

typedef struct
{
	XU32 index;                                 //索引
	XU32 linkType;
	XU8  smcIp[MAX_HOST_DEF];					//smc ip
	XU16 smcPort;								//smc port
	XU8  peerIp[MAX_HOST_DEF];					//peer ip
	XU16 peerPort;								//peer port
	XU8  linkDesc[MAX_LINK_DESC_DEF];			//链路描述
	XU8  linkStatus;							//smc侧链路状态
}tPeSmppCfgOamTable;

typedef struct
{
	XU32 index;                                 //索引
	XU8  smcIp[MAX_HOST_DEF];					//smc ip
	XU16 smcPort;								//smc port
	XU8  smcSystemId[SYSTEMID_MAX_DEF];         //smc侧SystemID
	XU8  smcPassword[PASSWORD_MAX_DEF];         //smc侧Password
	XU8  peerIp[MAX_HOST_DEF];					//peer ip
	XU16 peerPort;								//peer port
	XU8  peerSystemId[SYSTEMID_MAX_DEF];		//peer侧SystemID
	XU8  peerPassword[PASSWORD_MAX_DEF];		//peer侧Password
	XU8  linkDesc[MAX_LINK_DESC_DEF];			//链路描述
}tPeSmppFMTReceiveMsg;

typedef struct
{
	XU32 index;                                 //索引
	XU8  smcMasterIp[MAX_HOST_DEF];				//smc master ip
	XU8  smcSlaveIp[MAX_HOST_DEF];              //smc slave ip
	XU16 smcPort;								//smc port
	XU8  peerMasterIp[MAX_HOST_DEF];			//peer master ip
	XU8  peerSlaveIp[MAX_HOST_DEF];             //peer slave ip
	XU16 peerPort;								//peer port
	XU8  linkDesc[MAX_LINK_DESC_DEF];			//链路描述
}tPeTcfFMTReceiveMsg;

typedef struct
{
	XU32 index;                                 //索引
	XU8  linkStatus;							//smc侧链路状态
}tResponsePeCfg;

typedef struct
{
	XU32 index;                                 //索引
	XU8  peerType;					            //对端类型
	XU8  linkType;                              //连接类型
	XU8  smcMasterIp[MAX_HOST_DEF];				//smc master ip
	XU16 smcPort;								//smc port
	XU8  smcSystemId[SYSTEMID_MAX_DEF];         //smc侧SystemID
	XU8  smcPassword[PASSWORD_MAX_DEF];         //smc侧Password
    XU8  peerMasterIp[MAX_HOST_DEF];			//peer master ip
    XU16 peerPort;								//peer port
	XU8  peerSystemId[SYSTEMID_MAX_DEF];		//peer侧SystemID
	XU8  peerPassword[PASSWORD_MAX_DEF];		//peer侧Password
	XU8  linkStatus;							//smc侧链路状态
	XU8  linkDesc[MAX_LINK_DESC_DEF];			//链路描述
	PTIMER hTimerHandle;						//smpp链路定时器
	PTIMER TimerCheck;
	HLINKHANDLE linkHandle;						//smpp链路linkHandle
	XS32 usRepeat;								//心跳响应失败次数
	XS32 isFmtMsg;                              //是否已格式化
}tSmppLink;
typedef struct
{
	XU32 index;                                 //索引
	XU8  peerType;					            //对端类型
	XU8  linkType;                              //连接类型
	XU8  LocalIp[MAX_HOST_DEF];				    // ip
	XU16 LocalPort;								// port
    XU8  peerIp[MAX_HOST_DEF];			       //peer  ip
    XU16 peerPort;								//peer port
	XU8  linkStatus;							//链路状态
	XU8  linkDesc[MAX_LINK_DESC_DEF];			//链路描述
	PTIMER hTimerHandle;						//定时器
	HLINKHANDLE linkHandle;						//链路linkHandle
	XS32 usRepeat;								//心跳响应失败次数
	XS32 isFmtMsg;                              //是否已格式化
}tAgentLink;


#pragma pack( )



AGENTUA_API extern XS32 agentUA_LinkHandler(AGT_OAM_CFG_REQ_T * pRecvMsg);
AGENTUA_API extern XOS_HLIST gSmppLinkList;




