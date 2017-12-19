/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: agentHlrCom.h
功    能: agentHlr 消息处理相关
时    间: 2012年8月29日
**************************************************************************/
#pragma once

#include "xostype.h"
#include "smu_sj3_type.h"




#ifdef __cplusplus
extern "C" {
#endif
#define MAX_OSS_MSG_LEN 1800
#define TIMER_TYPE_AGENT_QRYUSM   0X01
#define TIMER_TYPE_AGENT_USERCHECK   0X02


#define TIMER_LENGTH_AGENT_QRYUSM  1*60*1000  /* 查询控制块定时器时长初步定为 1 分钟*/
#define TIMER_LENGTH_AGENT_MEM  30*1000  /* 用户缓存检查定时器*/


#pragma pack(1)



typedef enum
{

    e_QrybyUID = 1,
    e_QrybyTEL,
    e_QrybyGid,
    e_QryUserBM,
    e_QryUserPidPort,
    
    e_QryBUTT

}e_QRYALL_TYPE;
typedef enum
{

    e_WaitRoutingFromDns = 1,
    e_WaitUserInfoFromHomeHlr,


    e_State_BUTT

}e_QRY_STATE;





#pragma pack()


#define  ENCODEOSSREQ(ossReq, OperType, nLen, ccbIndex) \
    do{ \
    	ossReq.OpHead.ucIsSucc = 0; \
    	ossReq.OpHead.ucPackId = 0; \
    	ossReq.OpHead.ucOperateObjId = e_OperObj; \
    	ossReq.OpHead.ucOperateTypeId = OperType; \
    	ossReq.OpHead.ucIsEnd = 1; \
    	ossReq.OpHead.usLen = XOS_HtoNs(nLen); \
    	ossReq.Ph.ucPackType = SJ_REQ; \
    	ossReq.Ph.usDlgId = XOS_HtoNs(ccbIndex); \
    }while(0)

    
#define  ENCODEOSSRSP(ossReq, OperType, nLen, ccbIndex) \
    do{ \
    	ossReq.OpHead.ucIsSucc = 0; \
    	ossReq.OpHead.ucPackId = 0; \
    	ossReq.OpHead.ucOperateObjId = e_OperObj; \
    	ossReq.OpHead.ucOperateTypeId = OperType; \
    	ossReq.OpHead.ucIsEnd = 1; \
    	ossReq.OpHead.usLen = XOS_HtoNs(nLen); \
    	ossReq.Ph.ucPackType = SJ_RSP; \
    	ossReq.Ph.usDlgId = ccbIndex; \
    }while(0)
XS32 agentHLR_QryMsgProc(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 agentHLR_QryNetRspMsgPro(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 agentHLR_QryNetIDRequestMsg(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 agentHLR_QryUserInfoRspMsgPro(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);

//XS32 agentHLR_QryUserNetIdbyDNS(tUSM *pQryUSM, t_Sj3_Oper_Rsp *pMsgOper);
XS32 ProcessDecodeQryUserAllInfoReq(XU8 *pOutMsg, XU8 *pInMsg, XU16 InLen);
#ifdef __cplusplus
}
#endif 




