/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssTraceApi.h
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
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "trace_agent.h"
#include "SPRTraceStruct.h"

/**************************************************************************
* �� �� ���� agentTrace
* �������ܣ� HSS��trace����
* ��    �룺 ulFid           - ���룬���ܿ�ID
*			 ulLevel         - ���룬��ӡ����
*            const t_InfectId *userID��Ҫ���˵���Ϣ
*			 ucFormat        - ���룬��ӡ��ʽ���ַ���
*			 ...             - ���룬��ӡ����
* ��    ����
* �� �� ֵ��
* ˵    ����
**************************************************************************/
XVOID agentTrace( SSPRTracePre* pSprTracePre, const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName, XU32 ulFid, e_PRINTLEVEL eLevel, 				
				const XCHAR *cFormat, ... );

/**************************************************************************
* �� �� ���� SYS_MsgSend
* �������ܣ�  SYS_MsgSend -��װƽ̨����Ϣ���ͺ�����Ϊ�˼���ʹ��
* ��    �룺
* ��    ����
* �� �� ֵ��
* ˵    ����
**************************************************************************/
XS32 SYS_MsgSend(t_XOSCOMMHEAD *pMsg);

#ifdef __cplusplus
}
#endif /* __cplusplus */




