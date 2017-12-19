/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年8月17日
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
    //请求消息路由
    virtual RouteResult RequestMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest) = 0;
    //应答消息路由
    virtual RouteResult AnswerMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest);
    //请求消息处理
    virtual DiamRetCode RequestMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour,DiamPeer* dest) = 0;
    //应答消息处理
    virtual DiamRetCode AnswerMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour,DiamPeer* dest) = 0;
    //环路检测
    int LoopDetection(SharedPtr<DiamMsg> &msg);
    //错误消息处理
    DiamRetCode ErrorMsgHandling(SharedPtr<DiamMsg> msg, DiamPeer *sour, DiamPeer *dest);

public:
    //将请求消息加入到响应等待队列中，没有将消息体放入队列中
    int Add(int localh2h, SharedPtr<DiamMsg> &msg, DiamPeer *sour, DiamPeer *dest);
    //查询队列消息
    DiamRouterPendingReqPtr LookupQueuedMessage(int h2hId);
    //存储消息体
    int StoreRequestMessage(int h2hId, SharedPtr<DiamMsg> &msg);
    //删除消息体
    int DeleteQueuedMessage(int h2hId);
    //清理所有缓存的消息
    void ClearQueuedMessages();
    //未决队列
    DiamProtectedMap<int, DiamRouterPendingReqPtr> req_map_;

protected:
    //对应的消息处理句柄
    DiamMsgRouteHandler* (&route_handler_)[HANDER_MAX];
    //重定向处理句柄
    RedirectMsgHandler    redirAgent_;

private:
    DiamRouteNode* next_;
};

#endif //__DIAM_ROUTE_NODE_H__

