/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     MqttTask.cpp
* Author:       wangdanfeng
* Date：        10-15-2014
* OverView:     数据库的处理
*
* History:      最新历史修改在最前面
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*******************************************************************************/


#include "MqttTask.h"
#include "MqttComFun.h"
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"
#include "agentHDB_API.h"
#include "agentDB_API.h"

#include "xosmmgt.h"
//#include <stdio.h>
#include <signal.h>
//#include <memory.h>
#include "MqttOamCli.h"
#include "xostrace.h"
#include "taskcommon.h"
#include "agentuaOam.h"

#if defined(WIN32)
#include <Windows.h>
#define sleep Sleep
#else
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#include "xosxml.h"
#include "xmlparser.h"
#include "fid_def.h"
#include "xwriter.h"

#ifdef __cplusplus
extern "C" {
#endif



volatile int finished = 0;
//char* topic = NULL;
int subscribed = 0;
int disconnected = 0;
MqttInfo szMqttInfo = {0};
MQTTAsync g_client;

void cfinish(int sig)
{
	signal(SIGINT, NULL);
	finished = 1;
}


/*Qos: 0:最多一次有可能丢失，1:至少一次有可能重复，2:只有一次到达，用户比较严格地系统*/
szopts opts = 
{
	"AGENT", 1, '\n', 2, NULL, NULL, "localhost", "1883", 0 ,0
};

PTIMER  g_timerHandle = NULL; 
MQTTAsync_connectOptions g_conn_opts = MQTTAsync_connectOptions_initializer;


/*****************************************************************************
 Prototype    : messageArrived
 Description  : 接收推送消息回调
 Input        : void *context               
                char *topicName             
                int topicLen                
                MQTTAsync_message *message  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/7/6
    Author       : 处理MQTT推送上来的请求消息
    Modification : Created function

*****************************************************************************/
int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{


    XOS_Trace(MD(FID_MQTT, PL_DBG), "Enter topic = %s", topicName);



    MQTT_DispatchMsg(topicName, topicLen, message);
    
    
    
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
   
    
    
	return 1;
}
/*****************************************************************************
 Prototype    : deliveryComplete
 Description  : 推送消息回调
 Input        : void* context          
                MQTTAsync_token token  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/7/6
    Author       : 处理MQTT推送上来的请求消息
    Modification : Created function

*****************************************************************************/
void deliveryComplete(void* context, MQTTAsync_token token)
{

    XOS_Trace(MD(FID_MQTT, PL_DBG), "token = %u", token);
}
void onDisconnect(void* context, MQTTAsync_successData* response)
{
	disconnected = 1;
}


void onSubscribe(void* context, MQTTAsync_successData* response)
{
	subscribed = 1;
}


void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{

    XOS_Trace(MD(FID_MQTT, PL_ERR), "Subscribe failed, rc %d", response->code);
	finished = 1;
}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	if(response != NULL)
	{

        XOS_Trace(MD(FID_MQTT, PL_ERR), "Connect failed, rc %d", response->code);
	}
	
	finished = 1;
}



/*
* 断线回调
*
*
*
*/
void connectionLost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync)context;

    XOS_Trace(MD(FID_MQTT, PL_ERR), "connectionLost called, cause = %s", cause);

}


void onConnect(void* context, MQTTAsync_successData* response)
{
	
    MQTT_ReSubscribefromMem();
}

XVOID MQTT_TraceCallback(enum MQTTASYNC_TRACE_LEVELS level, XCHAR *message)
{
    

    XOS_Trace(MD(FID_MQTT, ((e_PRINTLEVEL)(level+1))), "%s", message);

}


/**************************************************************************
函 数 名: eHssDb_InitProc
函数功能: 数据库线程初始化函数
参    数:
返 回 值: XSUCC 成功  XERROR 失败
**************************************************************************/
XS8 MQTT_InitProc(XVOID *pPara1, XVOID *Para2)
{	
	
	XS32 ret;
    XU32 i = 0;
    t_BACKPARA tBackPara = {0};

    agentHDB_SubLoadCallback(MQTT_InsertSubInfo);

    if (XSUCC != XOS_MutexCreate(&g_Subscribe_Mutext))
	{
		XOS_Trace(MD(FID_MQTT, PL_EXP),"create  g_ScuMsgStatLocked mutex failed!");
		return XERROR;	
	}


	/*MQTT相关命令行注册*/
	RegMQTTCliCmd();
    
    /*MQTT模块注册trace 函数*/
    MQTTAsync_setTraceCallback(MQTT_TraceCallback);
    
    /*读取配置文件 获取连接信息和订阅信息*/
	GetMqttInfo(&szMqttInfo);
    for(i = 0 ; i < szMqttInfo.subscriberNum; i++)
	{
        MQTT_InsertSubInfo(1, (XCONST XCHAR*)szMqttInfo.subscriberInfo[i].topicName, XFALSE);
	}
    
    /*注册定向推送回调函数，建立和MQTT server的连接*/ 
	MQTTClient_INIT((char *)szMqttInfo.hostIp,(char *)szMqttInfo.hostPort,
		(char *)szMqttInfo.userName,(char *)szMqttInfo.password);

    tBackPara.para1 = MQTT_TIMEOUT_TYPE_LINK_STATUS;
    
	ret = XOS_TimerCreate(FID_MQTT, &g_timerHandle, TIMER_TYPE_LOOP, TIMER_PRE_LOW, &tBackPara);
	if (XERROR == ret)
	{
		XOS_Trace(MD(FID_MQTT,PL_ERR), "XOS_TimerCreate  Err!");
		return XERROR;
	}

	ret = XOS_TimerBegin(FID_MQTT, g_timerHandle, 5000);
	if (XERROR == ret)
	{
		XOS_Trace(MD(FID_MQTT, PL_ERR), "TimerBegin is Err!");
		return XERROR;
	}

    return XSUCC;
}

/**************************************************************************
函 数 名: SPRDb_XosMsgProc
函数功能: 数据库线程消息处理函数
参    数:
返 回 值:
**************************************************************************/
XS8 MQTT_XosMsgProc(XVOID *msg, XVOID *pPara)
{

    t_XOSCOMMHEAD* pxosMsg = (t_XOSCOMMHEAD*)msg;
    t_AGENTUA* pAgentUaMsg = XNULL;
    t_Sj3_Oper_Rsp *pHlrMsg = NULL;
    XU32 srcFID;
    XU32 msgID = 0;
    XU32 ret;
    XU32 fid = FID_MQTT;
    XU8 tmpBuf[1024 *3] = {0};
    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;

	if(XNULL == msg)
    {
        XOS_Trace(MD(FID_MQTT,PL_ERR),"agentHLR_XosMsgProc input is NULL");
		return XERROR;
    }
    srcFID = pxosMsg->datasrc.FID;
    msgID = pxosMsg->msgID;
    
    XOS_Trace(MD(FID_MQTT,PL_DBG),"Enter: srcFID = %u", srcFID);
    pAgentUaMsg = (t_AGENTUA*)pxosMsg->message;
	if(XNULL == pAgentUaMsg)
    {
        XOS_Trace(MD(FID_MQTT,PL_ERR),"pAgentUaMsg  is NULL");
		return XERROR;
    }
    
    XOS_Trace(MD(FID_HLR,PL_DBG), " topicname = %s , msgID = %u.", pAgentUaMsg->topic, msgID);
    
    
    if(msgID == MQTT_MSG_ID_PUSH)
    {
        pHlrMsg = (t_Sj3_Oper_Rsp *)pAgentUaMsg->pData;
        if(XNULL == pHlrMsg)
        {
            XOS_Trace(MD(FID_MQTT,PL_ERR),"pHlrMsg  is NULL");
    		return XERROR;

        }
        XOS_Trace(MD(fid, PL_DBG), " ucOperateObjId = %u", pHlrMsg->OpHead.ucOperateObjId);
        XOS_Trace(MD(fid, PL_DBG), " ucOperateTypeId = %u", pHlrMsg->OpHead.ucOperateTypeId);
        XOS_Trace(MD(fid, PL_DBG), " usLen = %u", XOS_NtoHs(pHlrMsg->OpHead.usLen));
        XOS_MemCpy(tmpBuf, pHlrMsg, pAgentUaMsg->msgLenth);
        if(MQTTASYNC_SUCCESS != MQTT_PushTopic(pAgentUaMsg->topic, tmpBuf, pAgentUaMsg->msgLenth))
        {
            XOS_Trace(MD(FID_MQTT,PL_ERR),"Push failed.");
        }
        
        
        XOS_MemFree(FID_MQTT, pHlrMsg);
    }
    else if(msgID == MQTT_MSG_ID_SUBSCRI)
    {
        
        if(0 == pAgentUaMsg->topic[0])
        {
            XOS_Trace(MD(FID_MQTT,PL_ERR),"pAgentUaMsg->topic is Null");
            return XERROR;
        }
        ret = MQTTAsync_subscribe(g_client, (const char*)pAgentUaMsg->topic, opts.qos, &ropts);
        if(MQTTASYNC_SUCCESS != ret)
        {

            XOS_Trace(MD(FID_MQTT,PL_ERR),"subscribe failed!ret = %u.", ret);
            
        }
        /*将订阅的主题插入到内存和数据库中*/
        MQTT_InsertSubInfo(1, (const char*)pAgentUaMsg->topic, XFALSE);
        
    }
    else if(msgID == MQTT_MSG_ID_unSUBSCRI)
    {
        
        ret = MQTTAsync_unsubscribe(g_client, (const char*)pAgentUaMsg->topic,  &ropts);
        if(MQTTASYNC_SUCCESS != ret)
        {

            XOS_Trace(MD(FID_MQTT,PL_ERR),"unsubscribe failed!ret = %u.", ret);
            
        }
        /*将订阅的主题从内存和数据库中删除*/
        MQTT_DelSubInfo(1, (const char*)pAgentUaMsg->topic);
    }
    return XSUCC;
}

/**************************************************************************
函 数 名: SPRDb_TimeoutProc
函数功能: 用于向XOS平台注册超时回调接口,暂时不需要进行处理
参    数:
返 回 值: XSUCC
**************************************************************************/
XS8 MQTT_TimeoutProc(t_BACKPARA  *pstPara)
{
    XU32 TimeTyp = pstPara->para1;
    int rc = 0;
    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
    XS32 LinkStatus;
    
    LinkStatus = MQTTAsync_isConnected(g_client);
    
    switch(TimeTyp)
    {
        case MQTT_TIMEOUT_TYPE_LINK_STATUS:
            if(XTRUE != LinkStatus)
            {

                
                XOS_Trace(MD(FID_MQTT, PL_ERR), "Agent try to connect to MQTT Server");
                XOS_Trace(MD(FID_MQTT, PL_DBG), "username=%s, password=%s",g_conn_opts.username,g_conn_opts.password);
                /*建链*/
            	g_conn_opts.username = (char *)szMqttInfo.userName;
	            g_conn_opts.password = (char *)szMqttInfo.password;
                ssl_opts.enableServerCertAuth = 0;
                g_conn_opts.ssl = &ssl_opts;
            	if ((rc = MQTTAsync_connect(g_client, &g_conn_opts)) != MQTTASYNC_SUCCESS)
            	{
            	    XOS_Trace(MD(FID_MQTT, PL_ERR), "Failed to start connect, return code %d", rc);
            		return XERROR;
            	}

                
            }
            break;
            
        default:
            break;

    }
    
    
    return XSUCC;
}

/**************************************************************************
函 数 名: SPRDb_NoticeProc
函数功能: 用于向XOS平台注册通知函数回调接口
参    数:
返 回 值: XSUCC
**************************************************************************/
XS8 MQTT_NoticeProc(XVOID* pLVoid, XVOID* pRVoid)
{
    
	return XSUCC;
}


/*****************************************************************************
 函 数 名  : MQTTClient_INIT
 功能描述  : 初始化mqtt连接程式
 输入参数  : char * hostIp    
             char * hostPort  
             char * username  
             char * password  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月8日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
int MQTTClient_INIT(char * hostIp , char * hostPort,char * username,char * password)
{

	MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;

	
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	int rc = 0;
	char url[100] = {0};

    opts.clientid = (char *)szMqttInfo.ucClientID;
	opts.host = hostIp;
	opts.port = hostPort;
	opts.username = username;
	opts.password = password;
	opts.showtopics = 1;
	sprintf(url, "%s:%s", opts.host, opts.port);

	rc = MQTTAsync_create(&g_client, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTAsync_setCallbacks(g_client, g_client, connectionLost, messageArrived, deliveryComplete);


	g_conn_opts.keepAliveInterval = 5;
	g_conn_opts.cleansession = 1;/*0:可以接受离线消息，断链后不需要再订阅 1:不接受离线消息，断开连接后需要重新订阅*/
	g_conn_opts.username = opts.username;
	g_conn_opts.password = opts.password;
	g_conn_opts.onSuccess = onConnect;
	g_conn_opts.onFailure = onConnectFailure;
	g_conn_opts.context = g_client;

	//MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
	ssl_opts.enableServerCertAuth = 0;
	g_conn_opts.ssl = &ssl_opts;
	if ((rc = MQTTAsync_connect(g_client, &g_conn_opts)) != MQTTASYNC_SUCCESS)
	{
	    XOS_Trace(MD(FID_MQTT, PL_ERR), "Failed to start connect, return code %d", rc);
		return XERROR;
	}

    //XOS_Trace(MD(FID_MQTT, PL_LOG), "MQTTClient_INIT  successfully!");


    

	return 0;

}



#ifdef __cplusplus
}
#endif

