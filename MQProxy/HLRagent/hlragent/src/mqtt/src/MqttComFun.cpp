/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssDbTask.cpp
* Author:       luhaiyan
* Date��        10-15-2014
* OverView:     ���ݿ�Ĵ���
*
* History:      ������ʷ�޸�����ǰ��
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*******************************************************************************/


#include "MqttTask.h"
#include "MqttComFun.h"
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"
#include "agentDB_API.h"



map<string,tSubInfo*> g_Subscribe;
 t_XOSMUTEXID g_Subscribe_Mutext;
static int published = 0;

/*��Ԫ����*/
XU8 g_HLRagent_Type = 0;


/*****************************************************************************
 �� �� ��  : MQTT_InsertSubInfo
 ��������  : ���涩�ĵ�������Ϣ
 �������  : XU32 type            
             XCONST XCHAR *topic  
             XU8 initFlg          
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��7��21��
    ��    ��   : wangdanfeng
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 MQTT_InsertSubInfo(XU32 type, XCONST XCHAR *topic, XU8 initFlg)
{

    XS32 ret = XSUCC;
    string tmp = string(topic);
     
    XOS_MutexLock(&g_Subscribe_Mutext);
    map<string,tSubInfo*>::iterator it = g_Subscribe.find(tmp);
    if(g_Subscribe.end() != it)
    {
        XOS_MutexUnlock(&g_Subscribe_Mutext);
        return XERROR;
    }

    tSubInfo* ptIns = new tSubInfo();
    ptIns->type = type;
    XOS_MemCpy(ptIns->topicName, topic, XOS_StrLen(topic));
    g_Subscribe.insert(make_pair(tmp,ptIns));
    XOS_MutexUnlock(&g_Subscribe_Mutext);
    

    return ret;
    
}
/*****************************************************************************
 �� �� ��  : MQTT_DelSubInfo
 ��������  : ɾ�����ĵ�����
 �������  : XU32 type            
             XCONST XCHAR *topic  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��7��21��
    ��    ��   : wangdanfeng
    �޸�����   : �����ɺ���

*****************************************************************************/
XS32 MQTT_DelSubInfo(XU32 type, XCONST XCHAR *topic)
{

    XS32 ret;
    string tmp = string(topic);
     
    XOS_MutexLock(&g_Subscribe_Mutext);
    map<string,tSubInfo*>::iterator it = g_Subscribe.find(tmp);
    if(g_Subscribe.end() != it)
    {
        delete (it->second);
        it->second = NULL;
        
        g_Subscribe.erase(tmp);
        

        XOS_MutexUnlock(&g_Subscribe_Mutext);
        return XSUCC;
    }

    XOS_MutexUnlock(&g_Subscribe_Mutext);
    return XERROR;
    
}
/*****************************************************************************
 Prototype    : MQTT_ReSubscribefromMem
 Description  : ���ڶ�����������Ҫ���¶����ڴ��б������Ϣ
 Input        : XVOID  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/7/2
    Author       : ����MQTT����������������Ϣ
    Modification : Created function

*****************************************************************************/
XS32 MQTT_ReSubscribefromMem(XVOID)
{
    
    map<string,tSubInfo*>::iterator it;

    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
 
    for(it = g_Subscribe.begin(); it != g_Subscribe.end(); it++)
    {
        //XOS_CliExtPrintf(pCliEnv,"    %s\r\n", it->second->topicName);

        

        MQTTAsync_subscribe(g_client, (const char*)it->second->topicName, opts.qos, &ropts);
        XOS_Trace(MD(FID_MQTT, PL_LOG), "Mq Server maybe restart,Resub Hlr's topic = %s.", it->second->topicName);
    }



    return XSUCC;
}

/**************************************************************************
�� �� ��: MQTT_DispatchMsg
��������: ������XOSƽ̨ע��֪ͨ�����ص��ӿ�
��    ��:
�� �� ֵ: XSUCC
**************************************************************************/
XS8 MQTT_DispatchMsg(XCHAR *topicName, XS32 topicLen, MQTTAsync_message *message)
{
    
    
    t_Sj3_Oper_Rsp *pHlrMsg = (t_Sj3_Oper_Rsp *)message->payload;
    XU32 hlrmsg_len = message->payloadlen;
    XU32 fid = FID_MQTT;

    if(NULL == pHlrMsg)
    {

        XOS_Trace(MD(fid, PL_ERR), " pHlrMsg is NULL");
        return XERROR;
    }
    XOS_Trace(MD(fid, PL_DBG), " Enter hlrmsg_len = %u.", hlrmsg_len);
    XOS_Trace(MD(fid, PL_DBG), " ucOperateObjId = %u", pHlrMsg->OpHead.ucOperateObjId);
    XOS_Trace(MD(fid, PL_DBG), " ucOperateTypeId = %u", pHlrMsg->OpHead.ucOperateTypeId);
    XOS_Trace(MD(fid, PL_DBG), " usLen = %u", XOS_NtoHs(pHlrMsg->OpHead.usLen));
    
	t_XOSCOMMHEAD* ptCommHead = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(fid, sizeof(t_AGENTUA));
	if (XNULLP == ptCommHead)
	{
		XOS_Trace(MD(fid,PL_ERR), " MsgMemMalloc is Err!");
		return XERROR;
	}
	ptCommHead->datasrc.FID = fid;
	ptCommHead->datadest.FID = FID_HLR;
	ptCommHead->datadest.PID = XOS_GetLocalPID();
	ptCommHead->datasrc.PID = XOS_GetLocalPID();
	//ptCommHead->msgID = ucPackType;
	ptCommHead->prio = eNormalMsgPrio;
	
    t_AGENTUA* pAgentUaMsg = (t_AGENTUA*)ptCommHead->message;

    XOS_MemCpy(pAgentUaMsg->topic, topicName, XOS_StrLen(topicName));
    pAgentUaMsg->msgLenth = hlrmsg_len;
    
    /*������ڴ����Ҫhlr��agent�ͷŵ�*/
    pAgentUaMsg->pData = (XCHAR *)XOS_MemMalloc(fid, hlrmsg_len);
    if(NULL == pAgentUaMsg->pData)
    {
		XOS_Trace(MD(fid,PL_ERR), " MsgMemMalloc is Err!");
		return XERROR;
    }
    
    XOS_MemCpy(pAgentUaMsg->pData, pHlrMsg, hlrmsg_len);
	if(XSUCC != XOS_MsgSend(ptCommHead))
	{
		
        XOS_MemFree(fid, pAgentUaMsg->pData);
		XOS_MsgMemFree(fid, ptCommHead);
        XOS_Trace(MD(fid, PL_ERR)," FID_MQTT send msg to HLR failed.");
		return XERROR;
	}
    XOS_Trace(MD(fid, PL_DBG), " Send msg to FID_HLR successfully!");	


    return XSUCC;
}


/***************************************************************
������GetDbCfgInfo
���ܣ������Ƿ�֧���������ñ�ʾ���������ļ�����oam�ڴ��ж�ȡ���ݿ�������Ϣ
���룺SDbConfigInfo *pDbCfgInfo���������ݿ�������Ϣ
�������
���أ�XSUCC/XERROR                    
***************************************************************/
XS32 GetMqttInfo(MqttInfo *pMqttInfo)
{
	XS32 ret = XERROR;
	
	//�������ļ��ж�ȡ���ݿ�������Ϣ
	ret = MQTT_GetConfig(FID_MQTT, pMqttInfo, "./mqttcfg.xml");
	if (ret != XSUCC)
	{
		XOS_Trace(MD(FID_MQTT,PL_ERR)," Read mqttcfg.xml Err!.");

        
	}

	return ret;
}



/************************************************************************
������:  MQTT_GetConfig
���ܣ�   ��XML�ļ��õ����ݿ��ʼ��ģ���������Ϣ
���룺   pFile�� xml�ļ���
�����   pOut������ַ���
���أ�   �ɹ� XSUCC
         ʧ�� XERROR
˵����
************************************************************************/
XS32 MQTT_GetConfig(XU32 fid, MqttInfo *pOut, XS8* pFile)
{
	//XS32 siRet;
	xmlDocPtr  doc   = XNULL;
	xmlNodePtr cur      = NULL;
	xmlChar* pTempStr   = NULL;
	xmlNodePtr tmpcur = NULL;
	XU32 index = 0;

	if (XNULLP == pOut || XNULLP == pFile)
	{
		XOS_Trace(MD(fid, PL_EXP), "input point is null");
		return XERROR;
	}

	doc =  xmlParseFile( pFile );
	if (doc == XNULL) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		return XERROR;
	} 
	cur = xmlDocGetRootElement( doc );
	if (cur == XNULL) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return XERROR;
	}
	if ( XOS_StrCmp( cur->name, "MODULES") ) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return( XERROR );
	}
	cur = cur->xmlChildrenNode;
	while ( cur && xmlIsBlankNode ( cur ) )
	{
		cur = cur->next;
	}
	if ( cur == XNULL )
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return( XERROR );
	}
	tmpcur = cur;
    
	while ( cur != XNULL ) 
	{
		if ( !XOS_StrNcmp(cur->name, "ConnectServerCfg" ,XOS_StrLen( "ConnectServerCfg")))
		{
			break;
		}
		cur = cur->next;
	}
	if ( XNULL == cur )
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return( XERROR ); 
	}
	cur = cur->xmlChildrenNode;
	while ( cur && xmlIsBlankNode ( cur ) )
	{
		cur = cur->next;
	}
	if ( cur == XNULL )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}

	while(XNULLP != cur)
	{
		if ( !XOS_StrCmp(cur->name, "HostIp") )
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->hostIp, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}

		else if ( !XOS_StrCmp(cur->name, "HostPort") )
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->hostPort, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}

		else if ( !XOS_StrCmp(cur->name, "UserName") )
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->userName, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}
		else if ( !XOS_StrCmp(cur->name, "Password") )
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->password, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}
		else
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->ucClientID, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}
		cur = cur->next;
	}
	cur=tmpcur->next;
	

	while ( cur != XNULL ) 
	{
		if ( !XOS_StrNcmp(cur->name, "AllSubscriberInfo" ,XOS_StrLen( "AllSubscriberInfo")))
		{
			break;
		}
		cur = cur->next;
	}
	if ( XNULL == cur )
	{
		XOS_Trace(MD(fid, PL_LOG), "parse mqttcfg.xml file End");
		xmlFreeDoc(doc);
		return XSUCC; 
	}
	cur = cur->xmlChildrenNode;
	while ( cur && xmlIsBlankNode ( cur ) )
	{
		cur = cur->next;
	}
	if ( cur == XNULL )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}
	tmpcur = cur;
	do
	{
		while ( cur != XNULL ) 
		{
			if ( !XOS_StrNcmp(cur->name, "SubscriberInfo" ,XOS_StrLen( "SubscriberInfo")))
			{
				break;
			}
			cur = cur->next;
		}
		if ( XNULL == cur )
		{
			XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
			xmlFreeDoc(doc);
			return( XERROR ); 
		}

		cur = cur->xmlChildrenNode;
		while ( cur && xmlIsBlankNode ( cur ) )
		{
			cur = cur->next;
		}
		if ( cur == XNULL )
		{
			xmlFreeDoc(doc);
			return( XERROR );
		}

		while(XNULLP != cur)
		{
			if ( !XOS_StrCmp(cur->name, "TopicName") )
			{
				pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
				if ( XNULL != pTempStr )
				{
				    if(!XOS_MemCmp(pTempStr, "QryAllInfoMsg", XOS_StrLen("QryAllInfoMsg")))
			        {
                        g_HLRagent_Type = e_TYPE_UDC;
                    }
					XOS_StrCpy(pOut->subscriberInfo[index].topicName, pTempStr);
					xmlFree( pTempStr );
					pTempStr = XNULL;
				}
			}

			cur = cur->next;
            index++;
		}
		cur = tmpcur->next;
		tmpcur = cur;
		//index++;
        
	}while(cur != NULL);
    
	pOut->subscriberNum = index;

	xmlFreeDoc(doc);

	return XSUCC;
}
void onPublishFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Publish failed, rc\n");
	published = -1; 
}


void onPublish(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;

	published = 1;
}


int publishtopic(char * topic,char * publishbuf)
{
	MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
	int rc=0;
	pub_opts.onSuccess = onPublish;

	pub_opts.onFailure = onPublishFailure;

	rc = MQTTAsync_send(g_client, topic, strlen(publishbuf), publishbuf, opts.qos, opts.retained, &pub_opts);
	return 0;
}

XS32 MQTT_PushTopic(XCHAR * destinationName, XVOID * buf, XU32 bufflen)
{
    
	MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
	pub_opts.onSuccess = onPublish;
	pub_opts.onFailure = onPublishFailure;


    MQTTAsync_message message = {0};
    message.qos = opts.qos;
    message.retained = opts.retained;
    message.payloadlen = bufflen;
    message.payload = buf;

    int rc = MQTTAsync_send(g_client, destinationName, bufflen, buf, opts.qos, opts.retained, &pub_opts);
    //return MQTTAsync_sendMessage(g_client, destinationName, &message, &pub_opts);

    return rc;
}


