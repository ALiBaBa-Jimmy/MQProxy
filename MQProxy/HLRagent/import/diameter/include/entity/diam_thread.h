/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��6��4��
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

    std::string  name_;          //�߳�����
    DiamBool     running_;       //�߳�����flag
    ACE_Mutex    mutex_;         //�߳���
};

//�����߳�
class DiamWorkTask : public DiamTask
{
public:
    DiamWorkTask(DiamRecvQueue* queue, DiamPeer* peer);
    ~DiamWorkTask();

private:
    // ACE�߳����
    int svc();
    //���ն���
    DiamRecvQueue* queue_;
    //��ǰ�̹߳�����PEER;
    DiamPeer*      peer_;
};

//DiameterIO�����߳�
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
    // ACE�߳����
    virtual int svc();
    // ��������
    virtual int ProcData() = 0;
    // �������
    virtual int ProcError() = 0;
    //�ɼ����̴߳����Ĵ���Э�����
    virtual DiamSocket* GetDiamSocket();

protected:
    //��Ϣ�������
    DiamHandler       handler_;
    //peer info
    DiamPeer*         peer_;
    //
    DiamTransChannel* trans_;
    //����IO����
    DiamSocket*       socket_;

    DiamChar          buffer[MAX_PACKET_LENGTH];
};

//DiameterIO�����߳�
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

//DiameterIO�����߳�
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

//Diameter����˼��������߳�
class DiamTransListen;
//�����߳�
class DiamTaskServer : public DiamTask
{
public:
    DiamTaskServer(DiamTransListen* peer);
    ~DiamTaskServer();

private:
    // ACE�߳����
    int svc();

    int StartListen();
    int ProcConnect();

private:
    DiamTransListen*  diam_server_;
    DiamSocketServer  socket_;

};

#endif  //__DIAM_THREAD_H__

