/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssTrace.cpp
* Author:       luhaiyan
* Date:         09-07-2014
* OverView:     eHSS TRACE�ӿ�
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


#include "agentTrace.h"



/**************************************************************************
* �� �� ���� agentTrace
* �������ܣ� ��ʱ����ӿ�,��ƽ̨ע��
* ��    �룺 ulFid           - ���룬���ܿ�ID
*			 ulLevel         - ���룬��ӡ����
*            const t_InfectId *userID��Ҫ���˵���Ϣ
*			 ucFormat        - ���룬��ӡ��ʽ���ַ���
*			 ...             - ���룬��ӡ����
* ��    ����
* �� �� ֵ��
* ˵    ����SHssTracePre�е�logId=0��ʾҪ��������logid��������Ҫ
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

	//���û�й�������������ԭ�������
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
		//����ÿ�ζ�ȥ���룬������Ա�֤��һ��������logid����ȥ������
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
* �� �� ���� SYS_MsgSend
* �������ܣ�  SYS_MsgSend -��װƽ̨����Ϣ���ͺ�����Ϊ�˼���ʹ��
* ��    �룺
* ��    ����
* �� �� ֵ��
* ˵    ����
**************************************************************************/
XS32 SYS_MsgSend(t_XOSCOMMHEAD *pMsg)
{
    /*MNT���- �ȼ���,ֱ�ӷ���MNT��ȥ*/ 
	//��������Ϣ�����	

	pMsg->prio = eNormalMsgPrio;
    if(XSUCC != XOS_MsgSend(pMsg))
    {
        return XERROR;
    }
        
    return XSUCC;
}


