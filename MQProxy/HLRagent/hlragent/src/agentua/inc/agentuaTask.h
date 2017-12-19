/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��: smppuaApi.h
��    ��: smppUA��ƽ̨�ӿ�
ʱ    ��: 2012��8��29��
**************************************************************************/
#pragma once

#include "xosshell.h"
//#include "smcinterface.h"
#include "oam_main.h"


#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
typedef struct
{
	XU8 LocalhostIp[15 + 1];
	XU8 LocalhostPort[15 + 1];

	XU8 PeerhostIp[15 + 1];
	XU8 PeerhostPort[15 + 1];

}LinkInfo;

#pragma pack()
XS8 agentUA_InitProc(XVOID *pPara1, XVOID *Para2);
XS8 agentUA_XosMsgProc(XVOID *msg, XVOID *pPara);
XS8 agentUA_TimeoutProc(t_BACKPARA *tBackPara);
XS8 agentUA_NoticeProc(XVOID* pLVoid, XVOID* pRVoid);




#ifdef __cplusplus
}
#endif /* __cplusplus */




