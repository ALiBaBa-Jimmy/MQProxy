/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: diam_interface.h
功    能: diameter协议接口文件
时    间: 2012年8月27日
**************************************************************************/
#ifndef __DIAM_INTERFACE_H__
#define __DIAM_INTERFACE_H__

#include <api/diam_message.h>

#ifdef  __cplusplus
extern  "C" {
#endif

/**************************************************************************
函数功能: 消息回调函数定义
参    数: DiamMsg&
返 回 值: void
**************************************************************************/
typedef void (*diamMsgFunc)(DiamMsg&);

/**************************************************************************
函数功能: 日志回调函数定义
参    数: logLevel:日志级别
返 回 值: void
**************************************************************************/
typedef void (*diamLogFunc)(ELogLevel logLevel, const DiamChar *logInfo);

/**************************************************************************
函数功能: 链路事件回调函数定义
参    数: linkId:链路Id
返 回 值: void
**************************************************************************/
typedef void (*diamLinkEventFunc)(DiamUINT32 linkId, DiamLinkEvent ev);

/**************************************************************************
函数功能: 消息跟踪的原始消息
参    数: linkId: 链路ID, buffer：原始消息 flag：0:收到的消息 1:发送的消息
返 回 值: void
**************************************************************************/
typedef void (*diamMsgTrace)(DiamUINT32 linkId, void* buffer, int flag);

/**************************************************************************
函 数 名: diam_init_protocol
函数功能: 初始化协议栈
参    数: EDiamNEType: 网元类型
          diamLogFunc: 日志回调函数
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_init_protocol(EDiamNEType ne, diamLogFunc func = NULL);

/**************************************************************************
函数功能: 设置日志级别
参    数: ELogLevel: 设置对应的日志级别
返 回 值: void
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_set_loglevel(ELogLevel level);

/**************************************************************************
函 数 名: diam_register_msgfunc
函数功能: 注册消息处理回调函数
参    数: appid :应用ID
		  func  :回调函数
		  一个应用注册一个回调函数,给命令码对应的消息处理有回调函数之间分发
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_register_msgfunc(DiamAppId appid, diamMsgFunc func);

/**************************************************************************
函 数 名: diam_register_eventfunc
函数功能: 注册链路事件回调函数,在使用该方法前，需要先添加对于的链路到协议栈中
参    数: linkIndex :链路ID
          func      :回调函数
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_register_eventfunc(DiamUINT32 linkIndex, diamLinkEventFunc func);
/**************************************************************************
函 数 名: diam_start_listen
函数功能: 建立本地监听端口
参    数: tcpPort : 本机监听的TCP端口,为0不监听
          sctpPort: 本机监听的SCTP端口,为0不监听
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_start_server(DiamUINT32  serverId,
                                              const char* masterIp,
                                              const char* slaveIp = NULL,
                                              DiamUINT32 port = 3868,
                                              PROTOCOL_TYPE type = PROTOCOL_TCP);

/**************************************************************************
函 数 名: diam_modify_server
函数功能: 建立本地监听端口
参    数: tcpPort : 本机监听的TCP端口,为0不监听
sctpPort: 本机监听的SCTP端口,为0不监听
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_modify_server(DiamUINT32  serverId,
                                               const char* masterIp,
                                               const char* slaveIp = NULL,
                                               DiamUINT32 port = 3868,
                                               PROTOCOL_TYPE type = PROTOCOL_TCP);

/**************************************************************************
函 数 名: diam_reset_listen
函数功能: 建立本地监听端口
参    数: tcpPort : 本机监听的TCP端口,为0不监听
          sctpPort: 本机监听的SCTP端口,为0不监听
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_stop_server(DiamUINT32 serverId);

/**************************************************************************
函 数 名: diam_reset_listen
函数功能: 建立本地监听端口
参    数: tcpPort : 本机监听的TCP端口,为0不监听
sctpPort: 本机监听的SCTP端口,为0不监听
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_del_server(DiamUINT32 serverId);

/**************************************************************************
函 数 名: diam_add_peerinfo
函数功能: 将对等端信息添加到协议栈中,但是不会跟对等端建立连接,主要用于服务端
          将客户端的信息添加到协议栈中
参    数: identity :对等端连接标识
          port     :对等端端口
          useSctp  :是否使用SCTP
          useTls   :是否使用TLS
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_add_client(DiamUINT32 linkIndex, 
                                              const char* masterIp, 
                                              const char* slaveIp = NULL, 
                                              DiamUINT32 port = 3868,
                                              DiamUINT32 serverId = 0,
                                              PROTOCOL_TYPE type=PROTOCOL_TCP);

/**************************************************************************
函 数 名: diam_add_peerinfo
函数功能: 将对等端信息从协议栈中删除
参    数: serverId :对等端连接标识 linkIndex:
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_del_client(DiamUINT32 linkIndex);

/**************************************************************************
函 数 名: diam_create_connect
函数功能: 建立跟服务端的连接
参    数: linkIndex:链路号
          localip  :本地IP
          localport:本地端口
          identity :对端IP
          port     :对端端口
		  useSctp  :是否使用Sctp
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_create_connect(DiamUINT32  linkIndex,
                                                const char* local_masterIp,
                                                const char* local_slaveIp,
                                                DiamUINT32  localPort,
                                                const char* remote_masterIp,
                                                const char* remote_slaveIp,
                                                DiamUINT32  remoteport,
                                                PROTOCOL_TYPE type=PROTOCOL_TCP);

/**************************************************************************
函 数 名: diam_delete_connect
函数功能: 释放跟对等端的连接,主要用于网管发起的删除
参    数: linkIndex:链路号
返 回 值: 成功:DIAM_RET_SUCCESS,其他为失败
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_release_connect(DiamUINT32 linkIndex);

/**************************************************************************
函 数 名: diam_get_linkstatus
函数功能: 获取链路状态
参    数: linkIndex:链路号
返 回 值: DIAM_RET_SUCCESS,链路状态OK,其他链路不OK
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_get_linkstatus(DiamUINT32 linkIndex);

/**************************************************************************
函 数 名: diam_get_peerInfo
函数功能: 
参    数: 
返 回 值: DIAM_RET_SUCCESS,链路状态OK,其他链路不OK
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_get_allpeerinfo(ConnectInfo& conn);

/**************************************************************************
函 数 名: diam_send_msg
函数功能: 发送消息
参    数: CMessage& msg 发送的消息,消息中需要携带目的peer信息
返 回 值: DIAM_RET_SUCCESS:发送成功
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_send_msg(DiamMsg& msg, DiamUINT32 linkIndex = 0);

#ifdef __cplusplus
}
#endif

#endif

