/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  filename:       diam_peerdata.h
**  description:
**  author:         panjian
**  data:           2014.07.16
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   panjian         2014.07.16        create
**************************************************************/
#ifndef __DIAM_PEER_H__
#define __DIAM_PEER_H__

#include <api/diam_datatype.h>
#include <api/diam_interface.h>
#include <util/diam_queue.h>
#include <entity/diam_thread.h>
#include <peer/diam_peer_define.h>
#include <peer/diam_peer_state.h>
#include <transport/diam_trans.h>

class DiamPeer : public DiamPeerState
{
public:
    DiamPeer(DiamUINT32 serverId);
    virtual ~DiamPeer();

public:
    DiamUINT32     serverId() {
        return server_id_;
    }
    DiamRetCode    AddLink(DiamUINT32 idx, 
                           DiamUINT32 server_id, 
                           const char* remote_master_ip,
                           const char* remote_slave_ip, 
                           DiamUINT32  remoteport, 
                           const char* local_master_ip, 
                           const char* local_slave_ip, 
                           DiamUINT32  localport, 
                           PROTOCOL_TYPE prtltype, 
                           EDiamServMode mode = DIAM_MODE_CLIENT);

    DiamRetCode    ModLink(DiamUINT32 idx, 
                           DiamUINT32 server_id, 
                           const char* remote_master_ip,
                           const char* remote_slave_ip, 
                           DiamUINT32  remoteport, 
                           const char* local_master_ip, 
                           const char* local_slave_ip, 
                           DiamUINT32  localport, 
                           PROTOCOL_TYPE prtltype, 
                           EDiamServMode mode = DIAM_MODE_CLIENT);

    DiamRetCode   MatchAddr(const char* remote_master_ip,
                            const char* remote_slave_ip, 
                            DiamUINT32  remoteport);

    DiamRetCode   MatchAddr(const char* peerAddr);

public:
    void         RecvMsg(SharedPtr<DiamMsg>& msg);
    DiamRetCode  SendMsg(SharedPtr<DiamMsg>& msg);
    void         StatusNofify(DiamUINT32 linkid, DiamLinkEvent ev);
    void         SetEventFunc(diamLinkEventFunc func);

    DiamRetCode  Start(DiamSocket* socket = NULL);
    DiamRetCode  Stop();
    DiamRetCode  ReStart();

public:
    void MsgIdTxMessage(DiamMsg &msg);
    bool MsgIdRxMessage(DiamMsg &msg);
    //校验Peer
    //bool ValidatePeer(DiamUINT32 &rcode, std::string& msg);

    void SendCER();
    void SendCEA(DiamUINT32 rcode = DIAMETER_SUCCESS, std::string msg = "Capabilities negotiation completed successfully");
    void ProcessCER(DiamMsg &msg);
    void ProcessCEA(DiamMsg &msg);
    void AssembleCE(DiamMsg& msg, bool request = true);
    void DisassembleCE(DiamMsg& msg);

    /*
    void SendDWR();
    void SendDWA(DiamUINT32 rcode, std::string &msg, DiamUINT32 hh, DiamUINT32 ee);
    void AssembleDW(DiamMsg &msg,  bool request = true);
    void DisassembleDW(DiamMsg &msg);

    void SendDPR(bool initiator);
    void SendDPA(bool initiator, DiamUINT32 rcode, DiamUINT32 hh, DiamUINT32 ee);
    void AssembleDP(DiamMsg &msg, bool request = true);
    void DisassembleDP(DiamMsg &msg);
    */
    void DumpPeerCapabilities();

public:
    DiamRecvQueue& recv_queue() {
        return recv_queue_;
    }
    DiamSendQueue& send_queue() {
        return send_queue_;
    }
    DiamPeerCapabilities& peer_capabilities() {
        return peer_data_;
    }
    const char* getPeerRemoteAddr();
    DiamTransChannel* getPeerChannel();

private:
    DiamUINT32           server_id_;
    DiamUINT32           link_id_;
    DiamPeerCapabilities peer_data_;  //对等端能力信息

    friend class DiamPeerMgt;
    DiamTransChannel*    channel_;    //连接信息（多链路）

private:
    //工作队列
    DiamRecvQueue        recv_queue_; //请求消息缓存队列
    DiamSendQueue        send_queue_; //发送消息缓存队列
    DiamWorkTask*        work_thread_;//工作线程

    ACE_RW_Mutex         event_mutex_;
};

#endif //__DIAM_PEERDATA_H__

