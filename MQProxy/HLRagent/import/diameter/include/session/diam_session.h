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
    //������Ϣ����ӿ�
    DiamRetCode recvMsg(DiamMsg& msg);
    //������Ϣ,����ҵ���޹ص���Ϣ������Ϣ�ύ��·��ģ��
    DiamRetCode sendMsg(DiamUINT32 linkId, DiamMsg& msg);
    //ע��ص�����
    DiamRetCode registerMsgFunc(diamMsgFunc func);

private:
    //����ҵ��������������Ϣ
    DiamRetCode processReqMsg(DiamUINT32 serverId, SharedPtr<DiamMsg>& msg);
    //����ҵ��������Ӧ����Ϣ
    DiamRetCode processAnsMsg(DiamUINT32 serverId, SharedPtr<DiamMsg>& msg);
private:
    //�ص�����
    diamMsgFunc msg_callback_;
    //��ǰsession��Ӧ��Ӧ��
    DiamUINT32 appid_;
};

#endif
