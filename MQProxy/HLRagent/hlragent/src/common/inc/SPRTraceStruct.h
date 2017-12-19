/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     SPRTraceStruct.h
* Author:       luhaiyan
* Date:         09-07-2014
* OverView:     SPR TRACE接口
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

#ifndef __SPR_TRACE_STRUCT_H__
#define __SPR_TRACE_STRUCT_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "trace_agent.h"
#pragma pack(1)


typedef struct
{
	XU16 paraType; //0:imsi;1:MSISDN,2:ALL  平台t_InfectId修改完后这个参数没有用了
	t_InfectId szInfectId;
	XU32 logId;
}SSPRTracePre;

#pragma pack()

//trace注册函数,主要为了解决windows下动态库trace输出不了的问题
typedef XVOID (*callbackTraceFun) (SSPRTracePre* pSprTracePre, const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName, XU32 ulFid, e_PRINTLEVEL eLevel,
								 const XCHAR *cFormat, ... );

#ifdef __cplusplus
}
#endif 

#endif 


