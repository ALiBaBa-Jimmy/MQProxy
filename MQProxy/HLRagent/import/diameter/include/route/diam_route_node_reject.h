/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: 
功    能: 
时    间: 2012年8月20日
**************************************************************************/
#ifndef __DIAM_ROUTENODE_REJECT_H__
#define __DIAM_ROUTENODE_REJECT_H__

#include <route/diam_route_node.h>

//路由错误处理节点
class DiamRouteNodeReject : public DiamRouteNode
{
public:
    DiamRouteNodeReject(DiamMsgRouteHandler* (&handers)[HANDER_MAX]):DiamRouteNode(handers){}
private:
    //请求消息路由
    RouteResult RequestMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest);
    //应答消息路由
    RouteResult AnswerMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest);
    //请求消息处理
    DiamRetCode RequestMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour, DiamPeer* dest);
    //应答消息处理
    DiamRetCode AnswerMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour, DiamPeer* dest);
};

#endif //__DIAM_ROUTENODE_REJECT_H__
