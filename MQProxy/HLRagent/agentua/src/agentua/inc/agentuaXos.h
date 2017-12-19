/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: agentuaXos.h
功    能: 向上适配XOS平台的消息,包括向平台注册
时    间: 2012年9月14日
**************************************************************************/
#pragma once

#include "xosshell.h"
#include "agentuaOam.h"
#include "trace_agent.h"

/**************************************************************************
函 数 名: agentUA_XosMsgHandler
函数功能:
参    数:
返 回 值:
**************************************************************************/
AGENTUA_API XS8 agentUA_XosMsgHandler(t_XOSCOMMHEAD* pXosMsg, XVOID *Para);

