/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��: smppuaApi.h
��    ��: smppUA��ƽ̨�ӿ�
ʱ    ��: 2012��8��29��
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




