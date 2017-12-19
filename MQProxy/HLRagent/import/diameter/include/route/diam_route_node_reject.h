/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��: 
��    ��: 
ʱ    ��: 2012��8��20��
**************************************************************************/
#ifndef __DIAM_ROUTENODE_REJECT_H__
#define __DIAM_ROUTENODE_REJECT_H__

#include <route/diam_route_node.h>

//·�ɴ�����ڵ�
class DiamRouteNodeReject : public DiamRouteNode
{
public:
    DiamRouteNodeReject(DiamMsgRouteHandler* (&handers)[HANDER_MAX]):DiamRouteNode(handers){}
private:
    //������Ϣ·��
    RouteResult RequestMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest);
    //Ӧ����Ϣ·��
    RouteResult AnswerMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest);
    //������Ϣ����
    DiamRetCode RequestMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour, DiamPeer* dest);
    //Ӧ����Ϣ����
    DiamRetCode AnswerMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour, DiamPeer* dest);
};

#endif //__DIAM_ROUTENODE_REJECT_H__
