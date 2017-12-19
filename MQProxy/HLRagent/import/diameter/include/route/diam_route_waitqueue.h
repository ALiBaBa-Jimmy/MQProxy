/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年8月23日
**************************************************************************/
#ifndef __DIAM_ROUTE_MSG_WAITQUEUE_H__
#define __DIAM_ROUTE_MSG_WAITQUEUE_H__

#include <peer/diam_peer.h>
#include <util/diam_queue.h>

/**************************************************************************
类    名: CDiamRouterPendingReq
类 功 能: 未决队列数据结构
时    间: 2012年8月23日
**************************************************************************/
typedef struct
{
    int                 m_OrigHH;
    DiamPeer           *m_Source;
    DiamPeer           *m_Dest;
    ACE_Time_Value      m_ReTxExpireTime;
    DiamUINT32          m_ReTxCount;
    DiamUINT32          m_AssociateId;
    //SharedPtr<DiamMsg>  m_ReqMessage;
} DiamRouterPendingReq;

typedef DiamRouterPendingReq* DiamRouterPendingReqPtr;

class PendingReqCleanup : public DiamIterAction<DiamRouterPendingReqPtr>
{
public:
    virtual bool operator()(DiamRouterPendingReqPtr &r)
    {
        delete r;
        return true;
    }
};

#endif //__DIAM_ROUTE_MSG_WAITQUEUE_H__
