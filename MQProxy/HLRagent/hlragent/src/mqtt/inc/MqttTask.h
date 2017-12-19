#pragma once




#include "agentinclude.h"
#include "xosshell.h"
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"


#ifdef __cplusplus
extern "C" {
#endif
#define MQTT_TIMEOUT_TYPE_LINK_STATUS    0X01
#define MQTT_TIMEOUT_TYPE_SUBINFO_LOAD   0X02


#define MQTT_DEFAULT_STRLEN    31  //ͨ�õ��ַ�������
#define MQTT_DEFAULT_SUBSCRIBE_NUM    16  //�����ļ� �̶��Ķ����������

#define MQTT_MAX_SUBSCRIBE_NUM    256
#pragma pack(1)

typedef struct
{
	char* clientid;
	int nodelimiter;
	char delimiter;
	int qos;
	char* username;
	char* password;
	char* host;
	char* port;
	int showtopics;
	int retained;
} szopts ;


typedef struct  
{
	XU8 topicName[MQTT_DEFAULT_STRLEN + 1];
	XU8 callBackFunc[MQTT_DEFAULT_STRLEN + 1];
}SubscriberInfo;

typedef struct
{
	XU8 hostIp[MQTT_DEFAULT_STRLEN + 1];
	XU8 hostPort[MQTT_DEFAULT_STRLEN + 1];
	XU8 userName[MQTT_DEFAULT_STRLEN + 1];
	XU8	password[MQTT_DEFAULT_STRLEN + 1];
    XU8 ucClientID[MQTT_DEFAULT_STRLEN + 1];
	XU32 subscriberNum;
	SubscriberInfo subscriberInfo[MQTT_DEFAULT_SUBSCRIBE_NUM];
}MqttInfo;



#pragma pack()
/**************************************************************************
�� �� ��: SPRDb_InitProc
��������: ��ʼ���ӿ�,��ƽ̨ע��
��    ��:
�� �� ֵ:
**************************************************************************/
 XS8 MQTT_InitProc(XVOID *pPara1, XVOID *pPara2 );

/**************************************************************************
�� �� ��: SPRDb_XosMsgProc
��������: ��Ϣ����ӿ�,��ƽ̨ע��
��    ��:
�� �� ֵ:
**************************************************************************/
 XS8 MQTT_XosMsgProc(XVOID *msg, XVOID *pPara);

/**************************************************************************
�� �� ��: SPRDb_TimeoutProc
��������: ��ʱ����ӿ�,��ƽ̨ע��
��    ��:
�� �� ֵ:
**************************************************************************/
 XS8 MQTT_TimeoutProc(t_BACKPARA  *pstPara);

/**************************************************************************
�� �� ��: SPRDb_NoticeProc
��������: 
��    ��:
�� �� ֵ:
**************************************************************************/
 XS8 MQTT_NoticeProc(XVOID* pLVoid, XVOID* pRVoid);



/***************************************************************
������GetDbCfgInfo
���ܣ������Ƿ�֧���������ñ�ʾ���������ļ�����oam�ڴ��ж�ȡ���ݿ�������Ϣ
���룺SDbConfigInfo *pDbCfgInfo���������ݿ�������Ϣ
�������
���أ�XSUCC/XERROR                    
***************************************************************/

int MQTTClient_INIT(char * hostIp , char * hostPort,char * username,char * password);

extern MqttInfo szMqttInfo;
extern MQTTAsync g_client;
extern szopts opts;

typedef int (*callfunbytopic)(char *topicName, int topicLen, MQTTAsync_message *message);
#ifdef __cplusplus
}
#endif


