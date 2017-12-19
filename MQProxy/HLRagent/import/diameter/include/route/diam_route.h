/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年8月23日
**************************************************************************/
#ifndef __DIAM_ROUTE_H__
#define __DIAM_ROUTE_H__

#include <route/diam_route_define.h>
#include <route/diam_route_node.h>
#include <util/diam_sharedptr.h>
#include <peer/diam_peer.h>
#include <api/diam_message.h>

/* 消息路由链路 */
class DiamRouteChain
{
public:
    DiamRouteChain();
    //析构
    virtual ~DiamRouteChain();
    //
    RouteResult Route(SharedPtr<DiamMsg> msg, DiamPeer *source);
    //操作路由链路链表
    int Add(DiamRouteNode *n);
    //在链路表中查找路由节点
    bool Lookup(DiamRouteNode *n);
    //从链路表中移除路由节点
    int Remove(DiamRouteNode *n);

private:
    DiamRouteNode* route_head_;  //路由链路起始路由节点
};

class DiamRoute
{
public:
    DiamRoute();

    RouteResult Route(SharedPtr<DiamMsg> msg, DiamPeer *source = NULL);

private:
    DiamRouteChain        route_chain_;

    DiamMsgRouteHandler*  handlers_[HANDER_MAX];
    //路由节点
    DiamRouteNode*        route_node_[ROUTENODE_MAX];
    //
    ACE_RW_Mutex          lock_;
};

typedef ACE_Singleton<DiamRoute, ACE_Recursive_Thread_Mutex> DiamRoute_S;
#define DiamMsgRoute() DiamRoute_S::instance()

#endif //__DIAM_ROUTE_H__
