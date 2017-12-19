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
#ifndef __DIAM_TRANS_H__
#define __DIAM_TRANS_H__

#include <peer/diam_peer_define.h>
#include <entity/diam_thread.h>

class DiamPeer;
class DiamTransChannel
{
public:
    DiamTransChannel(DiamPeer* peer, 
                     DiamUINT32 serverId,
                     DiamUINT32  idx, 
                     const char* remote_master_ip, 
                     const char* remote_slave_ip, 
                     DiamUINT32  remoteport, 
                     const char* local_master_ip, 
                     const char* local_slave_ip, 
                     DiamUINT32  localport, 
                     PROTOCOL_TYPE prtltype, 
                     EDiamServMode mode);
    ~DiamTransChannel();

public:
    DiamUINT32 server_id() {
        return server_id_;
    }
    DiamUINT32 index() {
        return index_;
    }
    const char* remote_master_ip() {
        return remote_master_ip_.c_str();
    }
    const char* remote_slave_ip() {
        return remote_slave_ip_.c_str();
    }
    DiamUINT32 remote_port() {
        return remote_port_;
    }
    const char* local_master_ip() {
        return local_master_ip_.c_str();
    }
    const char* local_slave_ip() {
        return local_slave_ip_.c_str();
    }
    DiamUINT32 local_port() {
        return local_port_;
    }
    EDiamDisconnectCause disconnect_cause() {
        return cause_;
    }
    DiamUINT32 expiration() {
        return expiration_;
    }
    DiamBool is_static() {
        return static_;
    }
    PROTOCOL_TYPE prtl_type() {
        return prtl_type_;
    }
    EDiamServMode  serv_mode() {
        return mode_;
    }
    DiamBool status() {
        return  socket_ ?  socket_->status() : false;
    }
    DiamUINT32 recv_cnt() {
        return recv_cnt_;
    }
    DiamUINT32 send_cnt() {
        return send_cnt_;
    }

    void set_server_id(DiamUINT32 serverId) {
        server_id_ = serverId;
    }
    void set_index(DiamUINT32 index) {
        index_ = index;
    }
    void set_remote_master_ip(const char* remoteip) {
        remote_master_ip_ = remoteip;
    }
    void set_remote_slave_ip(const char* remoteip) {
        remote_slave_ip_ = remoteip;
    }
    void set_remote_port(DiamUINT32 port) {
        remote_port_ = port;
    }
    void set_local_master_ip(const char* localip) {
        local_master_ip_ = localip;
    }
    void set_local_slave_ip(const char* localip) {
        local_slave_ip_ = localip;
    }
    void set_local_port(DiamUINT32 port) {
        local_port_ = port;
    }
    void set_disconnect_cause(EDiamDisconnectCause cause) {
        cause_ = cause;
    }
    void set_expiration(DiamUINT32 expiration) {
        expiration_ = expiration;
    }
    void set_static(DiamBool is_static) {
        static_ = is_static;
    }
    void set_prtl_type(PROTOCOL_TYPE type) {
        prtl_type_ = type;
    }
    void set_mode(EDiamServMode mode) {
        mode_ = mode;
    }
    void set_recv_inc(){
        recv_cnt_++;
    }
    void set_send_inc(){
        send_cnt_++;
    }

    void reset_recv_msg(){
        recv_cnt_ = 0;
    }
    void reset_send_msg(){
        send_cnt_ = 0;
    }

public:
    DiamRetCode  Start(DiamSocket* socket = NULL);
    DiamRetCode  Stop();
    DiamRetCode  ReStart();

private:
    DiamUINT32             server_id_;
    DiamUINT32             index_;            //索引
    DiamUTF8String         remote_master_ip_; //对等端标识
    DiamUTF8String         remote_slave_ip_;  //对等端标识
    DiamUINT32             remote_port_;      //对等端端口
    DiamUTF8String         local_master_ip_;  //本地Ip
    DiamUTF8String         local_slave_ip_;   //本地Ip
    DiamUINT32             local_port_;       //本地端口
    EDiamDisconnectCause   cause_;            //失连原因
    DiamUINT32             expiration_;       //生命周期
    DiamBool               static_;           //是否静态配置
    PROTOCOL_TYPE          prtl_type_;        //协议类型
    DiamUINT32             send_cnt_;
    DiamUINT32             recv_cnt_;

    DiamSocket*            socket_;           //socket对象
    EDiamServMode          mode_;             //工作模式

    DiamIORecvTask*        recv_thread_;      //接收线程
    DiamIOSendTask*        send_thread_;      //发送线程

private:
    DiamPeer*              diam_peer_;
    ACE_RW_Mutex           event_mutex_;
};
typedef std::map<DiamUINT32, DiamTransChannel*>  DiamTransPeerMap;

class DiamServer;
class DiamTransListen
{
public:
    DiamTransListen(DiamServer* peer, DiamUINT32 idx, const char* local_master_ip, const char* local_slave_ip, DiamUINT32  localport, PROTOCOL_TYPE prtltype);
    ~DiamTransListen();

public:
    DiamUINT32 index() {
        return index_;
    }
    const char* local_master_ip() {
        return local_master_ip_.c_str();
    }
    const char* local_slave_ip() {
        return local_slave_ip_.c_str();
    }
    DiamUINT32 local_port() {
        return local_port_;
    }
    PROTOCOL_TYPE prtl_type() {
        return prtl_type_;
    }

    void set_index(DiamUINT32 index) {
        index_ = index;
    }
    void set_local_master_ip(const char* localip) {
        local_master_ip_ = localip;
    }
    void set_local_slave_ip(const char* localip) {
        local_slave_ip_ = localip;
    }
    void set_local_port(DiamUINT32 port) {
        local_port_ = port;
    }
    void set_prtl_type(PROTOCOL_TYPE type) {
        prtl_type_ = type;
    }

public:
    DiamRetCode  Start();
    DiamRetCode  Stop();
    DiamRetCode  ReStart();

private:
    DiamUINT32             index_;            //索引
    DiamUTF8String         local_master_ip_;         //本地Ip
    DiamUTF8String         local_slave_ip_;         //本地Ip
    DiamUINT32             local_port_;       //本地端口
    PROTOCOL_TYPE          prtl_type_;        //协议类型

    //传输线程
    DiamTaskServer*        server_thread_;

private:
    DiamServer*            diam_peer_;
    ACE_RW_Mutex           event_mutex_;
};

typedef std::map<DiamUINT32, DiamTransListen*>  DiamTransListenMap;

#endif //__DIAM_TRANS_H__
