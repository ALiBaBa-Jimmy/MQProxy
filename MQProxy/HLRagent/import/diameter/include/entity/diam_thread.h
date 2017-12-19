/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年6月4日
**************************************************************************/
#ifndef __DIAM_THREAD_H__
#define __DIAM_THREAD_H__

#include <api/diam_datatype.h>
#include <util/diam_ace.h>
#include <util/diam_queue.h>
#include <transport/diam_socket.h>
#include <transport/diam_handler.h>

#define  MAX_PACKET_LENGTH    8192

class DiamTask : public ACE_Task<ACE_MT_SYNCH>
{
public:
    DiamTask();
    ~DiamTask();

    virtual int Start();
    virtual int Stop();

    std::string& name();

protected:
    bool Active();
    void InActive();

    std::string  name_;          //线程名称
    DiamBool     running_;       //线程运行flag
    ACE_Mutex    mutex_;         //线程锁
};

//工作线程
class DiamWorkTask : public DiamTask
{
public:
    DiamWorkTask(DiamRecvQueue* queue, DiamPeer* peer);
    ~DiamWorkTask();

private:
    // ACE线程入口
    int svc();
    //接收队列
    DiamRecvQueue* queue_;
    //当前线程归属的PEER;
    DiamPeer*      peer_;
};

//DiameterIO处理线程
class DiamPeer;
class DiamTransChannel;
class DiamIOTask : public DiamTask
{
public:
    DiamIOTask(DiamTransChannel* trans, DiamPeer* peer);
    virtual ~DiamIOTask();

public:
    void SetDiamSocket(DiamSocket* socket);

protected:
    // ACE线程入口
    virtual int svc();
    // 处理数据
    virtual int ProcData() = 0;
    // 处理错误
    virtual int ProcError() = 0;
    //由监听线程创建的传输协议对象
    virtual DiamSocket* GetDiamSocket();

protected:
    //消息处理对象
    DiamHandler       handler_;
    //peer info
    DiamPeer*         peer_;
    //
    DiamTransChannel* trans_;
    //传输IO对象
    DiamSocket*       socket_;

    DiamChar          buffer[MAX_PACKET_LENGTH];
};

//DiameterIO接收线程
class DiamIORecvTask : public DiamIOTask
{
public:
    DiamIORecvTask(DiamTransChannel* trans, DiamPeer* peer) : DiamIOTask(trans, peer) {
        name_ = "recv thread";
    }

private:
    int ProcData();
    int ProcError();
};

//DiameterIO发送线程
class DiamIOSendTask : public DiamIOTask
{
public:
    DiamIOSendTask(DiamTransChannel* trans, DiamPeer* peer) : DiamIOTask(trans, peer) {
        name_ = "send thread";
    }

private:
    int ProcData();
    int ProcError();
};

//Diameter服务端监听处理线程
class DiamTransListen;
//服务线程
class DiamTaskServer : public DiamTask
{
public:
    DiamTaskServer(DiamTransListen* peer);
    ~DiamTaskServer();

private:
    // ACE线程入口
    int svc();

    int StartListen();
    int ProcConnect();

private:
    DiamTransListen*  diam_server_;
    DiamSocketServer  socket_;

};

#endif  //__DIAM_THREAD_H__

