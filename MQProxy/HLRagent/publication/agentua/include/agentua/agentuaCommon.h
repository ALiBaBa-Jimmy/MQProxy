#pragma once


#include "HlrAgentUa.h"
#include "xosshell.h"

/*monitor 外部监视宏*/
#define MONITOR_EXTERNAL_DEF

/*接收消息的缓冲区最大长度*/
#define MSGBUFFER_MAX_DEF        4096

/*内部接口消息参数宏的定义*/
#define SYSTEMID_MAX_DEF            (16)
#define PASSWORD_MAX_DEF           (9)

#define TIMER_HEART_REQ_SEND    1
#define TIMER_HEART_WATCHER       2


#pragma pack(1)

typedef struct
{
	XU32 uiCommandLength;            /*命令长度*/
	XU32 uiCommandID;                /*命令ID*/
	XU32 uiCommandStatus;            /*命令状态*/ 
	XU32 uiSequenceNumber;           /*消息序列号*/ 
}tHLRMSG_HEAD;

typedef enum
{
    E_AGENTUA_HEART_REQ = 1,
    E_AGENTUA_HEART_RSP

}E_HEART;
/*heart req*/
typedef struct 
{
	
	tHLRMSG_HEAD heart;
	
}tHLRMSG_HEART;


/***************************************************************************************
*缓冲区数据结构的定义
*
*此数据结构主要负责对TCP Stream的切分，当收到的数据流的长度超过一个消息的长度时，先处理
*一个完整的消息，将剩余的消息保存下来，小于时将所有的数据保存下来，等到收到完整的消息后再
*处理，等于就直接进行处理
***********************************************************************************/
typedef struct MsgBuffer
{
	HLINKHANDLE linkHandle;
	XU32 uiLength;
	char cTemBuffer[MSGBUFFER_MAX_DEF];
} MSG_BUFFER_T,*pMSG_BUFFER_T;







/* 缓冲区变量表的定义*/
AGENTUA_API extern XOS_HLIST smppUA_htMsgBufferList;


/*函数声明*/

AGENTUA_API XS32 agentUA_FindPeerAppHandle(XU32 linkID, XU8 peerType);
AGENTUA_API XS32 Fuction64To32(XS64 appTempHandle);

AGENTUA_API XS32 agentUA_LinkStartFill(t_XOSCOMMHEAD *pMsgHead, XS32 appHandle);

AGENTUA_API XVOID agentUA_DataReqFill(t_XOSCOMMHEAD *pMsgHead,XS32 appHandle,XU32 uiLength,XU8 *pData);








