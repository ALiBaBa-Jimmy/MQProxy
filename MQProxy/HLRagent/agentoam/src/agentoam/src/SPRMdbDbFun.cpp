/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMdb.h
* Author:       luhaiyan
* Date：        10-12-2014
* OverView:     HSS用户内存信息
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
#include "SPRMdbDbFun.h"
callbackTraceFun g_callbackTraceFunOfHssMDb;



callbackNotifyUpdateMdbToHdbFun g_callbackNotifyUpdateMdbToHdbFun;
/**********************************************************************
*
*  NAME:          RegistMDbCallbackTraceFun
*  FUNTION:       注册trace函数，主要为了解决动态库log输出不了的问题
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
XS32 RegistMDbCallbackTraceFun(callbackTraceFun  traceFun)
{ 
    g_callbackTraceFunOfHssMDb = traceFun;
	return XSUCC;
}

