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


#define MQTT_DEFAULT_STRLEN    31  //通用的字符串长度
#define MQTT_DEFAULT_SUBSCRIBE_NUM    16  //配置文件 固定的订阅最大限制

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
函 数 名: SPRDb_InitProc
函数功能: 初始化接口,向平台注册
参    数:
返 回 值:
**************************************************************************/
 XS8 MQTT_InitProc(XVOID *pPara1, XVOID *pPara2 );

/**************************************************************************
函 数 名: SPRDb_XosMsgProc
函数功能: 消息处理接口,向平台注册
参    数:
返 回 值:
**************************************************************************/
 XS8 MQTT_XosMsgProc(XVOID *msg, XVOID *pPara);

/**************************************************************************
函 数 名: SPRDb_TimeoutProc
函数功能: 超时处理接口,向平台注册
参    数:
返 回 值:
**************************************************************************/
 XS8 MQTT_TimeoutProc(t_BACKPARA  *pstPara);

/**************************************************************************
函 数 名: SPRDb_NoticeProc
函数功能: 
参    数:
返 回 值:
**************************************************************************/
 XS8 MQTT_NoticeProc(XVOID* pLVoid, XVOID* pRVoid);



/***************************************************************
函数：GetDbCfgInfo
功能：根据是否支持网管配置标示，从配置文件或者oam内存中读取数据库配置信息
输入：SDbConfigInfo *pDbCfgInfo：返回数据库配置信息
输出：无
返回：XSUCC/XERROR                    
***************************************************************/

int MQTTClient_INIT(char * hostIp , char * hostPort,char * username,char * password);

extern MqttInfo szMqttInfo;
extern MQTTAsync g_client;
extern szopts opts;

typedef int (*callfunbytopic)(char *topicName, int topicLen, MQTTAsync_message *message);
#ifdef __cplusplus
}
#endif


