/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��8��20��
**************************************************************************/
#ifndef __DIAM_ROUTE_NODE_FORWARD_H__
#define __DIAM_ROUTE_NODE_FORWARD_H__

#include <route/diam_route_node.h>

//·��ת������ڵ�
class DiamRouteNodeForward : public DiamRouteNode
{
public:
    DiamRouteNodeForward(DiamMsgRouteHandler* (&handers)[HANDER_MAX]) : DiamRouteNode(handers) {}

private:
    //������Ϣ·��
    RouteResult RequestMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest);
    //������Ϣ����
    DiamRetCode RequestMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour, DiamPeer* dest);
    //Ӧ����Ϣ����
    DiamRetCode AnswerMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour, DiamPeer* dest);

};

#endif //__DIAM_ROUTE_NODE_FORWARD_H__

