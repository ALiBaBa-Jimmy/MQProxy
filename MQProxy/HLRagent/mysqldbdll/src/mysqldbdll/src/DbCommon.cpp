/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     DbCommon.cpp
* Author:       luhaiyan
* Date:         10-15-2014
* OverView:     
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
#include "SPRTraceStruct.h"
#include "DbCommon.h"

callbackTraceFun g_callbackTraceFunOfMySqlDb;
/**********************************************************************
*
*  NAME:          RegistMySqlDbCallbackTraceFun
*  FUNTION:       注册trace函数，主要为了解决动态库log输出不了的问题
*  INPUT:         无
*  OUTPUT:        本地的host信息
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 RegistMySqlDbCallbackTraceFun(callbackTraceFun  traceFun)
{
	g_callbackTraceFunOfMySqlDb = traceFun;
	return 0;
}

