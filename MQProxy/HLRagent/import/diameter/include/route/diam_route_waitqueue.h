/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��8��23��
**************************************************************************/
#ifndef __DIAM_ROUTE_MSG_WAITQUEUE_H__
#define __DIAM_ROUTE_MSG_WAITQUEUE_H__

#include <peer/diam_peer.h>
#include <util/diam_queue.h>

/**************************************************************************
��    ��: CDiamRouterPendingReq
�� �� ��: δ���������ݽṹ
ʱ    ��: 2012��8��23��
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
