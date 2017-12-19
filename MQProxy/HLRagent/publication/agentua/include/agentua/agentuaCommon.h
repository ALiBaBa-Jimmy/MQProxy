#pragma once


#include "HlrAgentUa.h"
#include "xosshell.h"

/*monitor �ⲿ���Ӻ�*/
#define MONITOR_EXTERNAL_DEF

/*������Ϣ�Ļ�������󳤶�*/
#define MSGBUFFER_MAX_DEF        4096

/*�ڲ��ӿ���Ϣ������Ķ���*/
#define SYSTEMID_MAX_DEF            (16)
#define PASSWORD_MAX_DEF           (9)

#define TIMER_HEART_REQ_SEND    1
#define TIMER_HEART_WATCHER       2


#pragma pack(1)

typedef struct
{
	XU32 uiCommandLength;            /*�����*/
	XU32 uiCommandID;                /*����ID*/
	XU32 uiCommandStatus;            /*����״̬*/ 
	XU32 uiSequenceNumber;           /*��Ϣ���к�*/ 
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
*���������ݽṹ�Ķ���
*
*�����ݽṹ��Ҫ�����TCP Stream���з֣����յ����������ĳ��ȳ���һ����Ϣ�ĳ���ʱ���ȴ���
*һ����������Ϣ����ʣ�����Ϣ����������С��ʱ�����е����ݱ����������ȵ��յ���������Ϣ����
*�������ھ�ֱ�ӽ��д���
***********************************************************************************/
typedef struct MsgBuffer
{
	HLINKHANDLE linkHandle;
	XU32 uiLength;
	char cTemBuffer[MSGBUFFER_MAX_DEF];
} MSG_BUFFER_T,*pMSG_BUFFER_T;







/* ������������Ķ���*/
AGENTUA_API extern XOS_HLIST smppUA_htMsgBufferList;


/*��������*/

AGENTUA_API XS32 agentUA_FindPeerAppHandle(XU32 linkID, XU8 peerType);
AGENTUA_API XS32 Fuction64To32(XS64 appTempHandle);

AGENTUA_API XS32 agentUA_LinkStartFill(t_XOSCOMMHEAD *pMsgHead, XS32 appHandle);

AGENTUA_API XVOID agentUA_DataReqFill(t_XOSCOMMHEAD *pMsgHead,XS32 appHandle,XU32 uiLength,XU8 *pData);








