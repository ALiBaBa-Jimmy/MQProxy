/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMAndHDbComFun.cpp
* Author:       luhaiyan
* Date：        09-29-2014
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

#include "SPRMAndHDbComFun.h"
callbackTraceFun g_callbackTraceFunOfSPRDb;

/**********************************************************************
*
*  NAME:          RegistDbCallbackTraceFun
*  FUNTION:       注册trace函数，主要为了解决动态库log输出不了的问题
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 RegistDbCallbackTraceFun(callbackTraceFun  traceFun)
{ 
    g_callbackTraceFunOfSPRDb = traceFun;
	return XSUCC;
}
