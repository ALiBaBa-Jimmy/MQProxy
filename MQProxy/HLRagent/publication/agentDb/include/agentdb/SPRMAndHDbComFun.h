#pragma once

#include "agentDb.h"
#include "SPRTraceStruct.h"

#ifdef __cplusplus
extern "C" {
#endif
extern callbackTraceFun g_callbackTraceFunOfSPRDb;
/**********************************************************************
*
*  NAME:          RegistDbCallbackTraceFun
*  FUNTION:       注册trace函数，主要为了解决动态库log输出不了的问题,
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
AGENTDB_API  XS32 RegistDbCallbackTraceFun(callbackTraceFun  traceFun);

#ifdef __cplusplus
}
#endif

