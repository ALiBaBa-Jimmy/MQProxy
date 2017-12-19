#pragma once



#include "xostype.h"
#include "smu_sj3_type.h"
#include "MqttTask.h"
#include <map>
using  std::map;
#include <string>
using  std::string;

using namespace std;

#pragma pack(1)
typedef struct  
{
    XU32 type;
	XU8 topicName[MQTT_DEFAULT_STRLEN + 1];
	
}tSubInfo;


typedef struct
{
   XU8 operTyp;
   XU8 topicName[MQTT_DEFAULT_STRLEN + 1]; 


}tMQTTContext;
#pragma pack()
extern map<string,tSubInfo*> g_Subscribe;
extern t_XOSMUTEXID g_Subscribe_Mutext;




XS32 MQTT_InsertSubInfo(XU32 type, XCONST XCHAR *topic, XU8 initFlg);
XS32 MQTT_DelSubInfo(XU32 type, XCONST XCHAR *topic);
int  publishtopic(char * topic,char * publishbuf);
XS32 MQTT_PushTopic(XCHAR * destinationName, XVOID * buf, XU32 bufflen);
XS8  MQTT_DispatchMsg(XCHAR *topicName, XS32 topicLen, MQTTAsync_message *message);
XS32 GetMqttInfo(MqttInfo *pMqttInfo);

XS32 MQTT_GetConfig(XU32 fid, MqttInfo *pOut, XS8* pFile);
XS32 MQTT_ReSubscribefromMem(XVOID);





