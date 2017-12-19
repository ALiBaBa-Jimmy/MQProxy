/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     SPRTraceStruct.h
* Author:       luhaiyan
* Date:         09-07-2014
* OverView:     SPR TRACE�ӿ�
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

#ifndef __SPR_TRACE_STRUCT_H__
#define __SPR_TRACE_STRUCT_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "trace_agent.h"
#pragma pack(1)


typedef struct
{
	XU16 paraType; //0:imsi;1:MSISDN,2:ALL  ƽ̨t_InfectId�޸�����������û������
	t_InfectId szInfectId;
	XU32 logId;
}SSPRTracePre;

#pragma pack()

//traceע�ắ��,��ҪΪ�˽��windows�¶�̬��trace������˵�����
typedef XVOID (*callbackTraceFun) (SSPRTracePre* pSprTracePre, const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName, XU32 ulFid, e_PRINTLEVEL eLevel,
								 const XCHAR *cFormat, ... );

#ifdef __cplusplus
}
#endif 

#endif 


