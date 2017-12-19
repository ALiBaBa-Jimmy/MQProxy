/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��8��17��
**************************************************************************/
#ifndef __DIAM_ROUTE_NODE_H__
#define __DIAM_ROUTE_NODE_H__

#include <util/diam_sharedptr.h>
#include <api/diam_message.h>
#include <peer/diam_peer.h>
#include <route/diam_route_define.h>
#include <route/diam_route_waitqueue.h>
#include <route/diam_route_handler.h>

class DiamRouteNode
{
public:
    DiamRouteNode(DiamMsgRouteHandler* (&handers)[HANDER_MAX]);
    virtual ~DiamRouteNode();

public:
    RouteResult Route(SharedPtr<DiamMsg> &msg, DiamPeer* sour);

    DiamRouteNode *next();

    void next(DiamRouteNode *next);

protected:
    //������Ϣ·��
    virtual RouteResult RequestMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest) = 0;
    //Ӧ����Ϣ·��
    virtual RouteResult AnswerMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest);
    //������Ϣ����
    virtual DiamRetCode RequestMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour,DiamPeer* dest) = 0;
    //Ӧ����Ϣ����
    virtual DiamRetCode AnswerMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour,DiamPeer* dest) = 0;
    //��·���
    int LoopDetection(SharedPtr<DiamMsg> &msg);
    //������Ϣ����
    DiamRetCode ErrorMsgHandling(SharedPtr<DiamMsg> msg, DiamPeer *sour, DiamPeer *dest);

public:
    //��������Ϣ���뵽��Ӧ�ȴ������У�û�н���Ϣ����������
    int Add(int localh2h, SharedPtr<DiamMsg> &msg, DiamPeer *sour, DiamPeer *dest);
    //��ѯ������Ϣ
    DiamRouterPendingReqPtr LookupQueuedMessage(int h2hId);
    //�洢��Ϣ��
    int StoreRequestMessage(int h2hId, SharedPtr<DiamMsg> &msg);
    //ɾ����Ϣ��
    int DeleteQueuedMessage(int h2hId);
    //�������л������Ϣ
    void ClearQueuedMessages();
    //δ������
    DiamProtectedMap<int, DiamRouterPendingReqPtr> req_map_;

protected:
    //��Ӧ����Ϣ������
    DiamMsgRouteHandler* (&route_handler_)[HANDER_MAX];
    //�ض�������
    RedirectMsgHandler    redirAgent_;

private:
    DiamRouteNode* next_;
};

#endif //__DIAM_ROUTE_NODE_H__

