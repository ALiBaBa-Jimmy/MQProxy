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
*  FUNTION:       ע��trace��������ҪΪ�˽����̬��log������˵�����,
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
**********************************************************************/
AGENTDB_API  XS32 RegistDbCallbackTraceFun(callbackTraceFun  traceFun);

#ifdef __cplusplus
}
#endif

