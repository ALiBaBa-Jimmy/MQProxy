/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年10月17日
**************************************************************************/

#include "agentHlrCom.h"
/*****************************************************************************
 Prototype    : agentHLR_EncodeTcpeTail
 Description  : 封装tcpe的尾巴两个字节 7E 0D
 Input        : t_AGENTUA* pInerMsg  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/6/26
    Author       : 处理MQTT推送上来的请求消息
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_EncodeTcpe(t_AGENTUA* pInerMsg)
{
    XU16 bufLen = 0;
    XU8 *pData = NULL;

    bufLen = pInerMsg->msgLenth;

    pData = (XU8 *)pInerMsg->pData;


    pData[bufLen - 2] = 0X7E;
    pData[bufLen - 1] = 0X0D;

    pData[0] = 0X7E;
    pData[1] = 0XA5;
    *(XU16*)(pData+2) = XOS_HtoNs(bufLen - 4);

    return XSUCC;
}
XS32 agentHLR_SendHlrMsg(t_AGENTUA* pInerMsg, XU32 destFID)
{

    t_XOSCOMMHEAD* ptCommHead;


    if(0 != pInerMsg->msgLenth)
    {
        agentHLR_EncodeTcpe(pInerMsg);
    }

	/*将消息透传给MQTT模块发送*/
	ptCommHead = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_HLR, sizeof(t_AGENTUA));
	if (XNULLP == ptCommHead)
	{
		XOS_Trace(MD(FID_HLR,PL_ERR), "smppUA_XosMsgHandler MsgMemMalloc is Err!");
		return XERROR;
	}
	ptCommHead->datasrc.FID = FID_HLR;
	ptCommHead->datadest.FID = destFID;
	ptCommHead->datadest.PID = XOS_GetLocalPID();
	ptCommHead->datasrc.PID = XOS_GetLocalPID();
	ptCommHead->msgID = pInerMsg->msgID;
	ptCommHead->prio = eNormalMsgPrio;
	

    XOS_MemCpy(ptCommHead->message, pInerMsg, sizeof(t_AGENTUA));
    
	if(XSUCC != XOS_MsgSend(ptCommHead))
	{
		XOS_Trace(MD(FID_HLR, PL_ERR)," send  hlr to MQTT msg failed.");
		XOS_MsgMemFree(FID_HLR, ptCommHead);
		return XERROR;
	}
    XOS_Trace(MD(FID_HLR, PL_DBG), " Send msg to destFID = %u successfully!", destFID);
    return XSUCC;

}
//直接透传给UA
XS32 OperType_TransmittoUA(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    t_AGENTUA AgentUaMsg = {0};
    XS32 ret;
    /*查询用户详细信息*/
    /*填充模块间通信结构头*/
    AgentUaMsg.msgLenth = agentUaMsgLen;
    AgentUaMsg.pData = (XCHAR *)XOS_MemMalloc(FID_HLR, agentUaMsgLen);
    if(NULL == AgentUaMsg.pData)
    {
        XOS_Trace(MD(fid,PL_ERR), "XOS_MemMalloc failed.");
        return ERROR;
    }
    XOS_MemCpy(AgentUaMsg.pData, pHlrMsg, agentUaMsgLen);
    

    ret = agentHLR_SendHlrMsg(&AgentUaMsg, FID_UA);
    if(XERROR == ret)
    {
        
        XOS_MemFree(fid, AgentUaMsg.pData);
        AgentUaMsg.pData = NULL;
        XOS_Trace(MD(fid,PL_ERR), "ret = %u.", ret);
    }
    return ret;

}


XS32 OperType_SubscribeorNot(XU32 fid,  XCHAR *topic, XU16 msgID)
{
    t_AGENTUA AgentUaMsg = {0};
    XS32 ret;
    XOS_Trace(MD(fid,PL_DBG), "Enter ");

    AgentUaMsg.msgLenth = 0;
    
    AgentUaMsg.msgID = msgID;
    XOS_MemCpy(AgentUaMsg.topic, topic, XOS_StrLen(topic));

    ret = agentHLR_SendHlrMsg(&AgentUaMsg, FID_MQTT);
    if(XERROR == ret)
    {
        
        XOS_MemFree(fid, AgentUaMsg.pData);
        AgentUaMsg.pData = NULL;
        XOS_Trace(MD(fid,PL_ERR), "ret = %u.", ret);
    }
    XOS_Trace(MD(fid,PL_DBG), "Exit ");
    return ret;
}


XS32 OperType_Push(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen, XCHAR *topic)
{
    t_AGENTUA AgentUaMsg = {0};
    XS32 ret;
    if(pHlrMsg == NULL)
    {
        XOS_Trace(MD(fid,PL_ERR), "pHlrMsg is NULL.");
        return ERROR;

    }


    AgentUaMsg.msgLenth = agentUaMsgLen;
    AgentUaMsg.pData = (XCHAR *)XOS_MemMalloc(fid, agentUaMsgLen);
    if(NULL == AgentUaMsg.pData)
    {
        XOS_Trace(MD(fid,PL_ERR), "XOS_MemMalloc failed.");
        return ERROR;
    }
    XOS_MemCpy(AgentUaMsg.pData, pHlrMsg, agentUaMsgLen);
    
    AgentUaMsg.msgID = MQTT_MSG_ID_PUSH;
    XOS_MemCpy(AgentUaMsg.topic, topic, XOS_StrLen(topic));

    ret = agentHLR_SendHlrMsg(&AgentUaMsg, FID_MQTT);
    if(XERROR == ret)
    {
        
        XOS_MemFree(fid, AgentUaMsg.pData);
        AgentUaMsg.pData = NULL;
        XOS_Trace(MD(fid,PL_ERR), "ret = %u.", ret);
    }
    return ret;
}

/************************************************************************
 函数名:BCDReverse
 功能:BCD码反转
 输入:
 输出:
 返回:
 说明:
************************************************************************/
XVOID agentHLR_BCDReverse(XU8 *bcd,XU32 len)
{
	XU32 i = 0;
	XU8  tmpbcd;
	for(i = 0;i<len;i++)
	{
		tmpbcd = bcd[i]<<4;
		bcd[i] = bcd[i]>>4;
		bcd[i] = bcd[i]|tmpbcd;
	}

}

XS32 agentHLR_EncodeTLV(XU8 *buf, XU8 tag, XVOID *Value, XU32 valueLen)
{
	XU16 len = 0;
	// tag 
	*(XU8*)(buf + len) = tag;
	len += 1;
	// len
	*(XU16*)(buf + len) = XOS_HtoNs(valueLen) ;
	len += 2;
	// v
	XOS_MemCpy((XU8 *)(buf + len),Value,valueLen);
	len += valueLen;
	return len;
    

}



/*有点危险*/
XS32 agentHLR_DecodeTLV(XU8 *buf, XU32 buflen,  XU8 tag, XVOID *value)
{
    XU32 i;
    XU8 Title;
    XU32 tmplen = 0;
    XU16 len = 0;
    for(i = 0; i < buflen; )
    {
        Title = buf[i];
        i += sizeof(XU8);
        
        len = XOS_NtoHs(*(XU16*)(buf + i));
        i += sizeof(XU16);
        if(Title == tag)
        {
            
            XOS_MemCpy(value , buf + i, len);
            return XSUCC;
        }
        else 
        {
	        XOS_Trace(MD(FID_HLR,PL_DBG), "Title = %u, len = %u.", Title, len);

        }
        /*将 value 域偏移出去*/
        i += len;
    }

    return XERROR;

}



XS32 agentHLR_DecodeULreg(XU8 *buf, XU32 buflen, SUpdateUserInfoReq *pRegInfo)
{
    XU32 i;
    XU8 Title;
    XU32 tmplen = 0;
    XU16 len = 0;
    for(i = 0; i < buflen; )
    {
        Title = buf[i];
        switch(Title)
        {
            case e_AGENT_RegInfo:
                i += sizeof(XU8);
                
                len = XOS_NtoHs(*(XU16*)(buf + i));
                i += sizeof(XU16);
                
                XOS_MemCpy(pRegInfo , buf + i, len);
                i += len;
                return XSUCC;

            default:
                i += sizeof(XU8);
                
                len = XOS_NtoHs(*(XU16*)(buf + i));
                i += sizeof(XU16);
                
                i += len;
		        XOS_Trace(MD(FID_HLR,PL_DBG), "Title = %u, len = %u.", Title, len);



        }

    }

    return XERROR;
}

XS32 agentHLR_DecodeSmcReport(XU8 *buf, XU32 buflen, tRep2Agent *pReportInfo)
{
    XU32 i;
    XU8 Title;
    XU32 tmplen = 0;
    XU16 len = 0;
    for(i = 0; i < buflen; )
    {
        Title = buf[i];
        switch(Title)
        {
            case e_AGENT_Report:
                i += sizeof(XU8);
                
                len = XOS_NtoHs(*(XU16*)(buf + i));
                i += sizeof(XU16);
                
                XOS_MemCpy(pReportInfo , buf + i, len);
                i += len;
                return XSUCC;

            default:
                i += sizeof(XU8);
                
                len = XOS_NtoHs(*(XU16*)(buf + i));
                i += sizeof(XU16);
                
                i += len;
		        XOS_Trace(MD(FID_HLR,PL_DBG), "Title = %u, len = %u.", Title, len);



        }

    }

    return XERROR;
}

XS32 agentHLR_GetValue(XU8* pOut, XU8* pInMsg)
{

	XU16 paraLen = 0;
	paraLen = XOS_NtoHs(*(XU16*)pInMsg);
	XOS_MemCpy(pOut,&pInMsg[2],paraLen);
	return (2 + paraLen);

}

/***************************************************************
函数：GetPresentTime
功能：获得当前的时间
输入：
输出：
返回：当前的时间    
/**************************************************************/
XU64 GetPresentTime(XVOID)
{
	XU64 sysTime = 0;
	//struct tm fTimes;
	XU64 longTime = 0;
	XU64 diffTime = 0;
	struct timeb timeBuffer;

	longTime = 946656000;//将该秒数写为固定值否则，每次调用该函数取得的时间会不一致
	
	//获得当前系统时间
	ftime( &timeBuffer );
	
	//当前时间的秒数-2000年的秒数
	diffTime = timeBuffer.time - longTime;

	//精确毫秒数
	sysTime = diffTime*1000 + timeBuffer.millitm;//秒+毫秒

	return sysTime;
}
/**************************************************************************
函 数 名: agentHLR_GetCurSysTime
函数功能:  获取系统时间
参    数:  XS8 *pTimeStr 要返回的时间串
返 回 值:  XS32 结果
**************************************************************************/
XS32 agentHLR_GetCurSysTime(XS8 *pTimeStr)
{
    t_XOSTT            timeT;
    t_XOSTD            timeTM;
    XS32               procRes;
    XU8                tempBuffer[256];

    if (XNULLP == pTimeStr)
    {
        return XERROR;
    }
    
    XOS_Time(&timeT);
    procRes = XOS_LocalTime(&timeT, &timeTM);
    if (XSUCC != procRes)
    {
        return XERROR;
    }
    XOS_Sprintf((XCHAR*)tempBuffer, 256, "%04d-%02d-%02d %02d:%02d:%02d", 
                timeTM.dt_year + 1900,
                timeTM.dt_mon + 1,
                timeTM.dt_mday,
                timeTM.dt_hour,
                timeTM.dt_min,
                timeTM.dt_sec);
    XOS_MemCpy(pTimeStr, tempBuffer, LENGTH_OF_DATE);
    return XSUCC;
}
/**************************************************************************
函 数 名: agentHLR_TimeMm2Str
函数功能:  将时间转换成字符串
参    数:  XS8 *pTimeStr 要返回的时间串
返 回 值:  XS32 结果
**************************************************************************/
XS32 agentHLR_TimeMm2Str(XU64 time, XS8 *pTimeStr)
{
    XU64 longTime = 946656000;
    time_t tmp = 0;
     
    tmp = time/1000;
    tmp += longTime;
    XOS_MemCpy(pTimeStr, ctime(&tmp), XOS_StrLen(ctime(&tmp)+1));

    return XSUCC;
}
/************************************************************************
   功能描述：把 十六进制 转为 字符串
示例：
    输入：0x61，0x62，0x63，0x64
	输出：ab，cd
                                                                  
/************************************************************************/
XS32 agentHLR_HexToStr(XU8 *pHex,XU8 * pStr,XU64 ulLen)
{
	XU64 i;

	for (i=0; i< ulLen;i++)
	{
		pStr[i] = agentHLR_ChrToHex(pHex[2*i]) * 16 + agentHLR_ChrToHex(pHex[2*i+1]);
	}

	return XSUCC;
}

/***********************************
功能描述：把ASCII字符转为十六进制
示例：
     输入 0x61
	 输出 0x0a
************************************/
XS32 agentHLR_ChrToHex(XU8 chr)
{
	XU8 temp1[17]="0123456789abcdef";
	XU8 temp2[7]="ABCDEF";
	XU8 i;

	for (i=0; i< 16;i++)
	{
		if (chr == temp1[i])
			return i;
	}
	for (i=0; i< 6;i++)
	{
		if (chr == temp2[i])
			return 10+i;
	}

	return -1;
}

/************************************************************************
   功能描述：把 字符串 转为 十六进制
示例：
    输入：ab，cd
	输出：0x61，0x62，0x63，0x64
                                                                  
/************************************************************************/
XS32 agentHLR_StrToHex(XU8 * pStr,XU8 *pHex,XU64 ulLen)
{
	XU64 i;

	for (i=0; i< ulLen;i++)
	{
		pHex[2*i]   = ( pStr[i] / 16 > 9) ?(87 + pStr[i] / 16 ):(48 + pStr[i] / 16 );
		pHex[2*i+1] = ( pStr[i] % 16 > 9) ?(87 + pStr[i] % 16 ):(48 + pStr[i] % 16 );
	}

	return XSUCC;
}

