/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     HlrAgentUa.h
* Author:       luhaiyan
* Date��        08-27-2014
* OverView:     SmcSmppUa�ӿ�ͷ�ļ�
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


#ifdef WIN32
	#ifdef AGENTUA_EXPORTS
	#define AGENTUA_API __declspec(dllexport)
	#else
	#define AGENTUA_API __declspec(dllimport)
	#endif
#else
	#define AGENTUA_API
#endif

#include "xosshell.h"
#include "xostimer.h"



