/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  filename:       diam_peermgt.h
**  description:
**  author:         panjian
**  data:           2014.07.16
**
***************************************************************
**                          history
***************************************************************
**   author          date              modification
**   panjian         2014.07.16        create
**************************************************************/
#ifndef __DIAM_PEERMGT_H__
#define __DIAM_PEERMGT_H__

#include <util/diam_ace.h>
#include <api/diam_define.h>
#include <peer/diam_peer.h>
#include <map>

class DiamPeerMgt
{
public:
    DiamPeerMgt();
    ~DiamPeerMgt();

public:
    //添加一个对的端
    DiamRetCode AddPeer(DiamUINT32 serverId, 
                        DiamUINT32 idx, 
                        const char* remote_master_ip,
                        const char* remote_slave_ip,
                        DiamUINT32 remoteport,
                        const char* local_master_ip ="",
                        const char* local_slave_ip ="",
                        DiamUINT32 localport = 0, 
                        PROTOCOL_TYPE type=PROTOCOL_TCP,
                        EDiamServMode mode = DIAM_MODE_CLIENT);
    //修改一个对的端
    DiamRetCode ModPeer(DiamUINT32 serverId, 
                        DiamUINT32 idx, 
                        const char* remote_master_ip,
                        const char* remote_slave_ip,
                        DiamUINT32 remoteport,
                        const char* local_master_ip ="",
                        const char* local_slave_ip ="",
                        DiamUINT32 localport = 0, 
                        PROTOCOL_TYPE type=PROTOCOL_TCP,
                        EDiamServMode mode = DIAM_MODE_CLIENT);
    //删除一个对的端
    DiamRetCode DelPeer(DiamUINT32 idx);

    DiamPeer* QryPeerByAddr(DiamUINT32 serverId, DiamUINT32 remote_master_ip, DiamUINT32 remote_slave_ip, DiamUINT32 remoteport);
    DiamPeer* QryPeerByAddr(DiamUINT32 serverId, const char* master_ip, const char* slave_ip, DiamUINT32 remoteport);
    DiamPeer* QryPeerByAddr(const char* peerAddr);
    DiamPeer* QryPeerByLinkId(DiamUINT32 linkId);
    DiamPeer* QryPeerByPeerName(std::string& peername);

    DiamRetCode Start(DiamUINT32 serverId);
    DiamRetCode Stop(DiamUINT32 serverId);
    DiamRetCode ReStart(DiamUINT32 serverId);

    DiamRetCode GetPeerConnStatus(DiamUINT32 idx);
    DiamRetCode GetAllPeerInfo(ConnectInfo& conn);

    void GetPeerInfo(std::string& output);

private:
    std::map<DiamUINT32, DiamPeer*> m_PeerMap; //<connectId, DiamPeer*>
};

typedef ACE_Singleton<DiamPeerMgt, ACE_Recursive_Thread_Mutex> DIAM_PEER_S;
#define DIAM_PEER_MGT DIAM_PEER_S::instance()

#endif //__DIAM_PEERMGT_H__

