/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssTrace.cpp
* Author:       luhaiyan
* Date:         09-07-2014
* OverView:     eHSS TRACE接口
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


#include "agentTrace.h"



/**************************************************************************
* 函 数 名： agentTrace
* 函数功能： 超时处理接口,向平台注册
* 输    入： ulFid           - 输入，功能块ID
*			 ulLevel         - 输入，打印级别
*            const t_InfectId *userID：要过滤的信息
*			 ucFormat        - 输入，打印格式化字符串
*			 ...             - 输入，打印参数
* 输    出：
* 返 回 值：
* 说    明：SHssTracePre中的logId=0表示要重新申请logid，否则不需要
**************************************************************************/
XVOID agentTrace( SSPRTracePre* pSprTracePre, const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName, XU32 ulFid, e_PRINTLEVEL eLevel,
				const XCHAR *cFormat, ... )
{
	//XS32    msg_curlen  = 0;
	XU8   msgFormat[MAX_TRACE_INFO_LEN+1]  = {0}; 
	XU8   msgPre[MAX_TRACE_INFO_LEN+1]  = {0}; 
	XS8   msgTemp[30] ={0};
	va_list ap;
	XU32 i = 0;
	if(XNULL == cFormat)
	{
		return;
	}
	va_start(ap, cFormat);
	XOS_VsPrintf((XS8*)msgFormat, sizeof(msgFormat)-1, cFormat, ap);
	va_end(ap);

	//如果没有过滤条件，调用原来的输出
	if(pSprTracePre == NULL )
	{
		XOS_Trace(FileName,ulLineNum,FunName, ulFid, eLevel, (XS8*)msgFormat);
	}
	else
	{		
		XU8   msg[MAX_TRACE_INFO_LEN+1]  = {0}; 

		for(i = 0 ; i < pSprTracePre->szInfectId.imsiNum ; i++)
		{
			sprintf((XS8*)msgTemp,"IMSI[%d] = %s; ",i,pSprTracePre->szInfectId.imsi[i]);
			strcat((XS8*)msgPre,msgTemp);
			XOS_MemSet(msgTemp,0,strlen(msgTemp)+1);
		}
		for(i = 0 ; i < pSprTracePre->szInfectId.idNum ; i++)
		{
			sprintf((XS8*)msgTemp, "MSISDN[%d]=%s; ",i,pSprTracePre->szInfectId.id[i]);
			strcat((XS8*)msgPre,msgTemp);
			XOS_MemSet(msgTemp,0,strlen(msgTemp)+1);
		}
	
		
		sprintf((XS8*)msg, "%s %s", msgPre, msgFormat);
#ifndef WIN32
	#ifdef XOS_TRACE_AGENT
		//避免每次都去申请，这里可以保证在一个函数内logid不用去申请多次
		if(pSprTracePre->logId == 0)
		{
			pSprTracePre->logId = XOS_LogInfect(&pSprTracePre->szInfectId);	
		}

		XOS_LogTrace(FileName, ulLineNum, FunName, ulFid, eLevel, pSprTracePre->logId , (XCHAR*)msg);
	#else
		XOS_Trace(FileName, ulLineNum, FunName, ulFid, eLevel, (XCHAR*)msg);
	#endif
#else

	XOS_Trace(FileName, ulLineNum, FunName, ulFid, eLevel, (XCHAR*)msg);
#endif
	}
		
}




/**************************************************************************
* 函 数 名： SYS_MsgSend
* 函数功能：  SYS_MsgSend -封装平台的消息发送函数，为了监视使用
* 输    入：
* 输    出：
* 返 回 值：
* 说    明：
**************************************************************************/
XS32 SYS_MsgSend(t_XOSCOMMHEAD *pMsg)
{
    /*MNT监控- 先简单做,直接发到MNT上去*/ 
	//将来做消息输出用	

	pMsg->prio = eNormalMsgPrio;
    if(XSUCC != XOS_MsgSend(pMsg))
    {
        return XERROR;
    }
        
    return XSUCC;
}


