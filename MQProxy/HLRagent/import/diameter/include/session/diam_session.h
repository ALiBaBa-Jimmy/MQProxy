#ifndef __DIAM_SESSION_H__
#define __DIAM_SESSION_H__

#include <api/diam_interface.h>
#include <util/diam_sharedptr.h>

class DiamSession
{
public:
    DiamSession(DiamUINT32 appId);
    virtual ~DiamSession();

public:
    //本例消息处理接口
    DiamRetCode recvMsg(DiamMsg& msg);
    //发送消息,完善业务无关的消息并将消息提交给路由模块
    DiamRetCode sendMsg(DiamUINT32 linkId, DiamMsg& msg);
    //注册回调函数
    DiamRetCode registerMsgFunc(diamMsgFunc func);

private:
    //处理业务层过来的请求消息
    DiamRetCode processReqMsg(DiamUINT32 serverId, SharedPtr<DiamMsg>& msg);
    //处理业务层过来的应答消息
    DiamRetCode processAnsMsg(DiamUINT32 serverId, SharedPtr<DiamMsg>& msg);
private:
    //回调函数
    diamMsgFunc msg_callback_;
    //当前session对应的应用
    DiamUINT32 appid_;
};

#endif
