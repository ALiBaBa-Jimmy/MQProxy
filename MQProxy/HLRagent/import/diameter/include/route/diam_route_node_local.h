#ifndef __DIAM_ROUTE_NODE_LOCAL__
#define __DIAM_ROUTE_NODE_LOCAL__

#include <route/diam_route_node.h>

//路由本地处理节点
class DiamRouteNodeLocal : public DiamRouteNode
{
public:
    DiamRouteNodeLocal(DiamMsgRouteHandler* (&handlers)[HANDER_MAX]);
    ~DiamRouteNodeLocal();

private:
    //请求消息路由
    virtual RouteResult RequestMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest);
    //请求消息处理
    virtual DiamRetCode RequestMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour,DiamPeer* dest);
    //应答消息处理
    virtual DiamRetCode AnswerMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour,DiamPeer* dest);

};


#endif //__DIAM_ROUTE_NODE_LOCAL__
