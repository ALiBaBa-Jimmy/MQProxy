#ifndef __DIAM_ROUTE_HANDER_H__
#define __DIAM_ROUTE_HANDER_H__

#include <api/diam_message.h>
#include <api/diam_resultcode.h>
#include <api/diam_messageoper.h>
#include <peer/diam_peer.h>
#include <util/diam_sharedptr.h>

class DiamMsgRouteHandler
{
public:
    virtual DiamRetCode Request(SharedPtr<DiamMsg> &msg, DiamPeer*source, DiamPeer *dest) = 0;
    virtual DiamRetCode Answer(SharedPtr<DiamMsg> &msg, DiamPeer *source, DiamPeer *dest) = 0;

protected:
    virtual ~DiamMsgRouteHandler() { }
};

class LocalMsgHandler : public DiamMsgRouteHandler
{
public:
    LocalMsgHandler() {}
    DiamRetCode Request(SharedPtr<DiamMsg> &msg, DiamPeer *source, DiamPeer *dest);
    DiamRetCode Answer(SharedPtr<DiamMsg> &msg, DiamPeer *source, DiamPeer *dest);
};

class ProxyMsgHandler : public DiamMsgRouteHandler
{
public:
    ProxyMsgHandler() {}
    DiamRetCode Request(SharedPtr<DiamMsg> &msg, DiamPeer *source, DiamPeer *dest);
    DiamRetCode Answer(SharedPtr<DiamMsg> &msg, DiamPeer *source,  DiamPeer *dest);
};

class ErrorMsgHandler : public DiamMsgRouteHandler
{
public:
    ErrorMsgHandler() {}
    DiamRetCode Request(SharedPtr<DiamMsg> &msg, DiamPeer *source, DiamPeer *dest);
    DiamRetCode Answer(SharedPtr<DiamMsg> &msg, DiamPeer *source,  DiamPeer *dest);
private:
    DiamRetCode LocalErrorHandling(SharedPtr<DiamMsg> &msg, DiamPeer *source, DiamPeer *dest);
};

class RedirectMsgHandler
{
public:
    RedirectMsgHandler() {}
public:
    DiamRetCode Request(SharedPtr<DiamMsg> &msg, DiamPeer *source);
    DiamRetCode Answer(SharedPtr<DiamMsg> &msg);
    bool IsRedirected(SharedPtr<DiamMsg> &msg);
};


#endif //__DIAM_ROUTE_HANDER_H__

