#ifndef __DIAM_ROUTE_NODE_LOCAL__
#define __DIAM_ROUTE_NODE_LOCAL__

#include <route/diam_route_node.h>

//·�ɱ��ش���ڵ�
class DiamRouteNodeLocal : public DiamRouteNode
{
public:
    DiamRouteNodeLocal(DiamMsgRouteHandler* (&handlers)[HANDER_MAX]);
    ~DiamRouteNodeLocal();

private:
    //������Ϣ·��
    virtual RouteResult RequestMsgRoute(SharedPtr<DiamMsg> &msg, DiamPeer*& dest);
    //������Ϣ����
    virtual DiamRetCode RequestMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour,DiamPeer* dest);
    //Ӧ����Ϣ����
    virtual DiamRetCode AnswerMsgProcess(SharedPtr<DiamMsg> &msg, DiamPeer* sour,DiamPeer* dest);

};


#endif //__DIAM_ROUTE_NODE_LOCAL__
