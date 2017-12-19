/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��6��4��
**************************************************************************/
#ifndef __DIAM_APPLICATION_H__
#define __DIAM_APPLICATION_H__

#include <api/diam_interface.h>
#include <api/diam_message.h>
#include <util/diam_sharedptr.h>
#include <peer/diam_peer.h>
#include <map>

class CDiamApplication
{
public:
    CDiamApplication();
    // ��������
    virtual ~CDiamApplication();

public:
    //��ʼ�������ļ�,��־,��������
    DiamRetCode init(EDiamNEType ne);
    //ע��ص�����
    DiamRetCode registerMsgFunc(DiamAppId appid, diamMsgFunc func);
    //
    DiamRetCode registerEventFunc(DiamUINT32 linkIndex, diamLinkEventFunc func);
    //�������ؼ���
    DiamRetCode startListen(DiamUINT32 serverId, const char* master, const char* slave, DiamUINT32 port, PROTOCOL_TYPE type);
    //
    DiamRetCode modifyListen(DiamUINT32 serverId, const char* master, const char* slave, DiamUINT32 port, PROTOCOL_TYPE type);
    //ֹͣ���ؼ���
    DiamRetCode stopListen(DiamUINT32 serverId);
    //
    DiamRetCode deleteListen(DiamUINT32 serverId);
    //�������м����˿�
    DiamRetCode startAllListen();
    //ֹͣ���м����˿�
    DiamRetCode stopAllListen();
    //��ӶԵȶ���Ϣ��Э��ջ��
    DiamRetCode addPeerInfo(DiamUINT32 linkIndex, 
        const char* masterIp, 
        const char* slaveIp, 
        DiamUINT32 port,
        DiamUINT32 serverId,
        PROTOCOL_TYPE type=PROTOCOL_TCP);
    //��Э��ջ��ɾ���Եȶ���Ϣ
    DiamRetCode delPeerInfo(DiamUINT32 linkIndex);
    //�����Եȶ˲�����������
    DiamRetCode createPeerConnect(DiamUINT32  linkIndex,
                                  const char* local_masterIp,
                                  const char* local_slaveIp,
                                  DiamUINT32  localPort,
                                  const char* remote_masterIp,
                                  const char* remote_slaveIp,
                                  DiamUINT32  remoteport,
                                  PROTOCOL_TYPE type=PROTOCOL_TCP);
    //�ͷ����Ӳ�ɾ���Եȶ�
    DiamRetCode releasePeerConnect(DiamUINT32 linkIndex);
    //
    DiamRetCode getPeerStatus(DiamUINT32 linkIndex);
    //
    DiamRetCode getAllPeerInfo(ConnectInfo& conn);
    //������Ϣ
    DiamRetCode sendDiamMsg(DiamUINT32 serverId, DiamMsg& msg);

    DiamRetCode recvDiamMsg(DiamMsg& msg);

};

#endif   // __DIAM_APPLICATION_H__ 
