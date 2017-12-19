#ifndef __DIAM_SERVER_H__
#define __DIAM_SERVER_H__

#include <transport/diam_trans.h>
#include <peer/diam_peer_define.h>

class DiamServer
{
public:
    DiamServer(DiamUINT32 serverId);
    ~DiamServer();

public:
    DiamRetCode AddListen(DiamUINT32 idx, const char* local_master_ip, const char* local_slave_ip, DiamUINT32  localport, PROTOCOL_TYPE prtltype);
    DiamRetCode ModListen(DiamUINT32 idx, const char* local_master_ip, const char* local_slave_ip, DiamUINT32  localport, PROTOCOL_TYPE prtltype);

    const char* GetMasterIp();
    const char* GetSlaveIp();
public:

    DiamRetCode Start();
    DiamRetCode Stop();
    DiamRetCode ReStart();

private:
    DiamUINT32           server_id_;          //状态
    DiamPeerCapabilities server_capabilities_;//能力信息
    DiamTransListen*     server_listen_;      //监听信息（双监听）
    ACE_RW_Mutex         event_mutex_;
};

#endif //__DIAM_SERVER_H__
