/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��8��23��
**************************************************************************/
#ifndef __DIAM_ROUTE_H__
#define __DIAM_ROUTE_H__

#include <route/diam_route_define.h>
#include <route/diam_route_node.h>
#include <util/diam_sharedptr.h>
#include <peer/diam_peer.h>
#include <api/diam_message.h>

/* ��Ϣ·����· */
class DiamRouteChain
{
public:
    DiamRouteChain();
    //����
    virtual ~DiamRouteChain();
    //
    RouteResult Route(SharedPtr<DiamMsg> msg, DiamPeer *source);
    //����·����·����
    int Add(DiamRouteNode *n);
    //����·���в���·�ɽڵ�
    bool Lookup(DiamRouteNode *n);
    //����·�����Ƴ�·�ɽڵ�
    int Remove(DiamRouteNode *n);

private:
    DiamRouteNode* route_head_;  //·����·��ʼ·�ɽڵ�
};

class DiamRoute
{
public:
    DiamRoute();

    RouteResult Route(SharedPtr<DiamMsg> msg, DiamPeer *source = NULL);

private:
    DiamRouteChain        route_chain_;

    DiamMsgRouteHandler*  handlers_[HANDER_MAX];
    //·�ɽڵ�
    DiamRouteNode*        route_node_[ROUTENODE_MAX];
    //
    ACE_RW_Mutex          lock_;
};

typedef ACE_Singleton<DiamRoute, ACE_Recursive_Thread_Mutex> DiamRoute_S;
#define DiamMsgRoute() DiamRoute_S::instance()

#endif //__DIAM_ROUTE_H__
