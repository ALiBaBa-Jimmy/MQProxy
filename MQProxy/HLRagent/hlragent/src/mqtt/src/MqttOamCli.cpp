
#include "MqttOamCli.h"
#include "MqttComFun.h"
#include "agentHlrCom.h"
#include "MQTTAsync.h"

#ifdef __cplusplus
extern "C" {
#endif

extern szopts opts;
extern MQTTAsync g_client;
extern MQTTAsync_connectOptions g_conn_opts;

XVOID Push_test(XCHAR *topic, XU8 opertype_id)
{
    XU8 buf[2048] = {0};
    XU32 buf_len = 0;
    XU16 len = 0;
    XU8 tel[16] = {0};
    XU8 uid[4] = {0};
    XU8 networkid[11] = {0};
    XOS_MemCpy(networkid, "111", 3);

    uid[0] = 0x62;
    uid[1] = 0x04;
    uid[2] = 0x97;
    uid[3] = 0xB2;

    tel[0] = 0X00;
    tel[1] = 0X85;
    tel[2] = 0X53;
    tel[3] = 0X83;
    tel[4] = 0X00;
    tel[5] = 0X20;
    tel[6] = 0X93;
    tel[7] = 0XFF;
    tel[8] = 0XFF;
    tel[9] = 0XFF;
    tel[10] = 0XFF;
    tel[11] = 0XFF;
    tel[12] = 0XFF;
    tel[13] = 0XFF;
    tel[14] = 0XFF;
    tel[15] = 0XFF;


    t_Sj3_Oper_Rsp *Hlrmsg = (t_Sj3_Oper_Rsp *)buf;

    Hlrmsg->Ph.ucPackType = SJ_REQ;
    Hlrmsg->Ph.ossId = 0;
    Hlrmsg->Ph.usDlgId = 0x01;

    Hlrmsg->OpHead.ucOperateObjId = 0x8f;
    Hlrmsg->OpHead.ucOperateTypeId = opertype_id;
    Hlrmsg->OpHead.ucIsEnd = XTRUE;
    XOS_MemCpy(Hlrmsg->OpHead.uctopic, networkid, XOS_StrLen(networkid));

    len += agentHLR_EncodeTLV(Hlrmsg->usBuf, e_UID_TAG, uid, 4); 
    len += agentHLR_EncodeTLV(Hlrmsg->usBuf+len, e_TELNO_TAG, tel, 16); 
    len += agentHLR_EncodeTLV(Hlrmsg->usBuf+len, e_NETWORKING_TAG, networkid, XOS_StrLen(networkid));
    
    
    Hlrmsg->OpHead.usLen = XOS_HtoNs(len);

    buf_len = len+LENGTH_HEAD+2;

    MQTT_PushTopic(topic, buf, buf_len);    
    return;
}
void Subscribe_onSuccessCallback(void* context, MQTTAsync_successData* response)
{
    tMQTTContext *pContext = (tMQTTContext *)context;
    



}

void Subscribe_onFailurCallback(void* context, MQTTAsync_failureData* response)
{

    tMQTTContext *pContext = (tMQTTContext *)context;



}

XVOID CLIOam_ShowMqttLinkInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    if(siArgc != 1)
    {
        XOS_CliExtPrintf(pCliEnv,"Usage: showlinker :回显client的连接信息");
        return;
    }
    XS32 LinkStatus = MQTTAsync_isConnected(g_client);
    XOS_CliExtPrintf(pCliEnv,"  %10s = %s\r\n", "clientid", opts.clientid);
    XOS_CliExtPrintf(pCliEnv,"  %10s = %s\r\n", "username", opts.username);
    XOS_CliExtPrintf(pCliEnv,"  %10s = %s\r\n", "username", opts.password);
    XOS_CliExtPrintf(pCliEnv,"  %10s = %s\r\n", "host", opts.host);
    XOS_CliExtPrintf(pCliEnv,"  %10s = %s\r\n", "port", opts.port);
    XOS_CliExtPrintf(pCliEnv,"  %10s = %u(%s)\r\n", "LinkStatus", LinkStatus,(LinkStatus == XTRUE ? "OK" : "ERR"));
    XOS_CliExtPrintf(pCliEnv,"  %10s = %u\r\n", "qos",  opts.qos);
    return ;
}


XVOID CLIOam_SetLogLevel(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    if(siArgc < 2)
    {
        XOS_CliExtPrintf(pCliEnv,"Usage: loglevel level");
        return;
    }
    XU32 level = atoi(ppArgv[1]);
    MQTTAsync_setTraceLevel((enum MQTTASYNC_TRACE_LEVELS)level);
    XOS_CliExtPrintf(pCliEnv,"Set loglevel to %u.", level);
    return ;
}

XVOID CLIOam_PushTopicCfg(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
	XU8 topic[100]={0};
	XU8 publishbuf[100]={0};
	if (siArgc < 3)
	{
		XOS_CliExtPrintf(pCliEnv,"Usage: push topic publishbuf");
        return;
	}
    if(!XOS_MemCmp("test", ppArgv[1], 4))
    {

        Push_test(MQTT_USER_REGISTER_SUCC_INFO, atoi(ppArgv[2]));
        return;

    }
	memcpy(topic,ppArgv[1],strlen(ppArgv[1]));
	memcpy(publishbuf,ppArgv[2],strlen(ppArgv[2]));

	publishtopic((char*)topic,(char*)publishbuf);
	return ;
}



XVOID CLIOam_SubcribeTopicCfg(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 ret;
	XU8 topic[100]={0};
    tMQTTContext context = {0};
	
    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	if (siArgc < 2)
	{
		XOS_CliExtPrintf(pCliEnv,"Usage: subscribe topic");
        return;
	}
	memcpy(topic,ppArgv[1],strlen(ppArgv[1]));
    memcpy(context.topicName, ppArgv[1],strlen(ppArgv[1]));
    context.operTyp = 11;
	ropts.onSuccess = Subscribe_onSuccessCallback;
    ropts.onFailure = Subscribe_onFailurCallback;
    
    ropts.context = &context;

	ret = MQTTAsync_subscribe(g_client, (const char*)topic, opts.qos, &ropts);
    if(MQTTASYNC_SUCCESS != ret)
    {

        XOS_CliExtPrintf(pCliEnv,"subscribe failed!");
        return;
    }
    
    MQTT_InsertSubInfo(1, (const char*)topic, XFALSE);
	return ;
}



XVOID CLIOam_UNSubcribeTopicCfg(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 ret;
	XU8 topic[100]={0};
	
    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	if (siArgc < 2)
	{
		XOS_CliExtPrintf(pCliEnv,"Usage: unsubscribe topic");
        return;
	}
	memcpy(topic,ppArgv[1],strlen(ppArgv[1]));
	

	ret = MQTTAsync_unsubscribe(g_client, (const char *)topic, &ropts);
    if(MQTTASYNC_SUCCESS != ret)
    {

        XOS_CliExtPrintf(pCliEnv,"unsubscribe failed!");
        return;
    }
    MQTT_DelSubInfo(1, (const char *)topic);
    //MQTT_DelSubInfo_plus((const char *)topic);
	return ;
}



XVOID CLIOam_showSubcribeTopicCfg(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
//    XU32 index;
    XU32 numbers = 0;
	XU8 topic[100]={0};

     XOS_CliExtPrintf(pCliEnv," Subcribe Info:\r\n");
     XOS_CliExtPrintf(pCliEnv," ===========================\r\n");


    map<string,tSubInfo*>::iterator it;
    
    for(it = g_Subscribe.begin(); it != g_Subscribe.end(); it++)
    {
        XOS_CliExtPrintf(pCliEnv,"    %s\r\n", it->second->topicName);

    }
	
    XOS_CliExtPrintf(pCliEnv,"(Totle Num = %u)\r\n", g_Subscribe.size());

	return ;
}





XVOID CLIOam_Linkmqtt(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    if(siArgc < 6)
    {
        XOS_CliExtPrintf(pCliEnv,"Usage: linker ip port username password clientid");
        return;
    }
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

	disc_opts.context = g_client;

    MQTTAsync_disconnect(g_client, &disc_opts);
    MQTTAsync_destroy(&g_client);
    
    XOS_MemCpy(szMqttInfo.hostIp, ppArgv[1],   XOS_StrLen(ppArgv[1]) + 1);
    XOS_MemCpy(szMqttInfo.hostPort, ppArgv[2], XOS_StrLen(ppArgv[2]) + 1);
    XOS_MemCpy(szMqttInfo.userName, ppArgv[3], XOS_StrLen(ppArgv[3]) + 1);
    XOS_MemCpy(szMqttInfo.password, ppArgv[4], XOS_StrLen(ppArgv[4]) + 1);
    XOS_MemCpy(szMqttInfo.ucClientID, ppArgv[5], XOS_StrLen(ppArgv[5]) + 1);
   
        
	MQTTClient_INIT((char *)szMqttInfo.hostIp,(char *)szMqttInfo.hostPort,
		(char *)szMqttInfo.userName,(char *)szMqttInfo.password);
    return ;
}

// 命令行注册函数
XS32  RegMQTTCliCmd(void)
{
	XS32 promptID;
    
    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "mqtt", "mqtt", " ");	
  

    if (0 > XOS_RegistCommand(promptID, CLIOam_PushTopicCfg, "push","push message", "HELP"))
	{
        
        return XERROR;
    } 
    if (0 > XOS_RegistCommand(promptID, CLIOam_SubcribeTopicCfg, "subscribe","subscribe message", "HELP"))
	{
        
        return XERROR;
    }	
    if (0 > XOS_RegistCommand(promptID, CLIOam_UNSubcribeTopicCfg, "unsubscribe","unsubscribe message", "HELP"))
	{
        
        return XERROR;
    }
    if (0 > XOS_RegistCommand(promptID, CLIOam_showSubcribeTopicCfg, "showsubscribe","showsubscribe", "HELP"))
	{
        
        return XERROR;
    }	
    if (0 > XOS_RegistCommand(promptID, CLIOam_SetLogLevel, "loglevel","set  loglevel", "HELP"))
	{
        
        return XERROR;
    }	
    if (0 > XOS_RegistCommand(promptID, CLIOam_Linkmqtt, "linker","linker", "HELP"))
	{
        
        return XERROR;
    }	
    if (0 > XOS_RegistCommand(promptID, CLIOam_ShowMqttLinkInfo, "showlinker","showlinker", "HELP"))
	{
        
        return XERROR;
    }	
    return XSUCC;
}







#ifdef __cplusplus
}
#endif





