/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年6月4日
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
    // 析构函数
    virtual ~CDiamApplication();

public:
    //初始化配置文件,日志,垃圾回收
    DiamRetCode init(EDiamNEType ne);
    //注册回调函数
    DiamRetCode registerMsgFunc(DiamAppId appid, diamMsgFunc func);
    //
    DiamRetCode registerEventFunc(DiamUINT32 linkIndex, diamLinkEventFunc func);
    //建立本地监听
    DiamRetCode startListen(DiamUINT32 serverId, const char* master, const char* slave, DiamUINT32 port, PROTOCOL_TYPE type);
    //
    DiamRetCode modifyListen(DiamUINT32 serverId, const char* master, const char* slave, DiamUINT32 port, PROTOCOL_TYPE type);
    //停止本地监听
    DiamRetCode stopListen(DiamUINT32 serverId);
    //
    DiamRetCode deleteListen(DiamUINT32 serverId);
    //启动所有监听端口
    DiamRetCode startAllListen();
    //停止所有监听端口
    DiamRetCode stopAllListen();
    //添加对等端信息到协议栈中
    DiamRetCode addPeerInfo(DiamUINT32 linkIndex, 
        const char* masterIp, 
        const char* slaveIp, 
        DiamUINT32 port,
        DiamUINT32 serverId,
        PROTOCOL_TYPE type=PROTOCOL_TCP);
    //从协议栈中删除对等端信息
    DiamRetCode delPeerInfo(DiamUINT32 linkIndex);
    //创建对等端并建立的连接
    DiamRetCode createPeerConnect(DiamUINT32  linkIndex,
                                  const char* local_masterIp,
                                  const char* local_slaveIp,
                                  DiamUINT32  localPort,
                                  const char* remote_masterIp,
                                  const char* remote_slaveIp,
                                  DiamUINT32  remoteport,
                                  PROTOCOL_TYPE type=PROTOCOL_TCP);
    //释放连接并删除对等端
    DiamRetCode releasePeerConnect(DiamUINT32 linkIndex);
    //
    DiamRetCode getPeerStatus(DiamUINT32 linkIndex);
    //
    DiamRetCode getAllPeerInfo(ConnectInfo& conn);
    //发送消息
    DiamRetCode sendDiamMsg(DiamUINT32 serverId, DiamMsg& msg);

    DiamRetCode recvDiamMsg(DiamMsg& msg);

};

#endif   // __DIAM_APPLICATION_H__ 
