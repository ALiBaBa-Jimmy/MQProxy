/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     HlrAgentUa.h
* Author:       luhaiyan
* Date：        08-27-2014
* OverView:     SmcSmppUa接口头文件
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



