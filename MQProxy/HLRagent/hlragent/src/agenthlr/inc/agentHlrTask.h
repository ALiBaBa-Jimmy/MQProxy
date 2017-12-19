/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: smppuaApi.h
功    能: smppUA跟平台接口
时    间: 2012年8月29日
**************************************************************************/
#pragma once

#include "xosshell.h"
#include "oam_main.h"
#include "taskcommon.h"


#ifdef __cplusplus
extern "C" {
#endif




XS8 agentHLR_InitProc(XVOID *pPara1, XVOID *Para2);
XS8 agentHLR_XosMsgProc(XVOID *msg, XVOID *pPara);
XS8 agentHLR_TimeoutProc(t_BACKPARA *tBackPara);
XS8 agentHLR_NoticeProc(XVOID* pLVoid, XVOID* pRVoid);




#ifdef __cplusplus
}
#endif /* __cplusplus */




