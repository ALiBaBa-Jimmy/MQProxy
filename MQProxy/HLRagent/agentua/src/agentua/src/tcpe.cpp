/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年9月13日
**************************************************************************/
#include "agentProtocol.h"
#include "agentuaXos.h"
#include "agentuaCommon.h"
#include "xosenc.h"
#include "agentuaOam.h"
#include "fid_def.h"
#include "smu_sj3_type.h"
#include "tcpe.h"
#include "xostype.h"


/*****************************************************************************
 Prototype    : SendMsg2Ntl_StopReq
 Description  : 向ntl发送断链请求
 Input        : XU32 msgID      
                XU32 appHandle  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/7/7
    Author       : 处理MQTT推送上来的请求消息
    Modification : Created function

*****************************************************************************/
XS32 SendMsg2Ntl_StopReq(HLINKHANDLE appHandle)
{
   
    t_LINKCLOSEREQ tLinkClose = {0};
    XS32 ret;


    t_XOSCOMMHEAD * pMsgHead = XOS_MsgMemMalloc(FID_UA, sizeof(t_LINKCLOSEREQ));
	if (XNULLP == pMsgHead)
	{
		XOS_Trace(MD(FID_UA,PL_ERR), "XOS_MsgMemMalloc Err!");
		return XERROR;
	}
	pMsgHead->datasrc.FID =  FID_UA;
	pMsgHead->datadest.FID = FID_NTL;
	pMsgHead->msgID = eStopInd;
	pMsgHead->prio = eNormalMsgPrio;

	tLinkClose.linkHandle = appHandle;

	XOS_MemCpy(pMsgHead->message, &tLinkClose, sizeof(t_LINKCLOSEREQ));

	ret=XOS_MsgSend(pMsgHead);
	if (XERROR == ret)
	{
		/*释放掉内存空间*/                
		XOS_MemFree(FID_UA, pMsgHead);
		XOS_Trace(MD(FID_UA, PL_ERR), "XOS_MsgSend to NTL is Err!");

	}  
    
    return ret;
}
