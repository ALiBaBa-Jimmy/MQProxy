/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��: diam_interface.h
��    ��: diameterЭ��ӿ��ļ�
ʱ    ��: 2012��8��27��
**************************************************************************/
#ifndef __DIAM_INTERFACE_H__
#define __DIAM_INTERFACE_H__

#include <api/diam_message.h>

#ifdef  __cplusplus
extern  "C" {
#endif

/**************************************************************************
��������: ��Ϣ�ص���������
��    ��: DiamMsg&
�� �� ֵ: void
**************************************************************************/
typedef void (*diamMsgFunc)(DiamMsg&);

/**************************************************************************
��������: ��־�ص���������
��    ��: logLevel:��־����
�� �� ֵ: void
**************************************************************************/
typedef void (*diamLogFunc)(ELogLevel logLevel, const DiamChar *logInfo);

/**************************************************************************
��������: ��·�¼��ص���������
��    ��: linkId:��·Id
�� �� ֵ: void
**************************************************************************/
typedef void (*diamLinkEventFunc)(DiamUINT32 linkId, DiamLinkEvent ev);

/**************************************************************************
��������: ��Ϣ���ٵ�ԭʼ��Ϣ
��    ��: linkId: ��·ID, buffer��ԭʼ��Ϣ flag��0:�յ�����Ϣ 1:���͵���Ϣ
�� �� ֵ: void
**************************************************************************/
typedef void (*diamMsgTrace)(DiamUINT32 linkId, void* buffer, int flag);

/**************************************************************************
�� �� ��: diam_init_protocol
��������: ��ʼ��Э��ջ
��    ��: EDiamNEType: ��Ԫ����
          diamLogFunc: ��־�ص�����
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_init_protocol(EDiamNEType ne, diamLogFunc func = NULL);

/**************************************************************************
��������: ������־����
��    ��: ELogLevel: ���ö�Ӧ����־����
�� �� ֵ: void
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_set_loglevel(ELogLevel level);

/**************************************************************************
�� �� ��: diam_register_msgfunc
��������: ע����Ϣ����ص�����
��    ��: appid :Ӧ��ID
		  func  :�ص�����
		  һ��Ӧ��ע��һ���ص�����,���������Ӧ����Ϣ�����лص�����֮��ַ�
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_register_msgfunc(DiamAppId appid, diamMsgFunc func);

/**************************************************************************
�� �� ��: diam_register_eventfunc
��������: ע����·�¼��ص�����,��ʹ�ø÷���ǰ����Ҫ����Ӷ��ڵ���·��Э��ջ��
��    ��: linkIndex :��·ID
          func      :�ص�����
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_register_eventfunc(DiamUINT32 linkIndex, diamLinkEventFunc func);
/**************************************************************************
�� �� ��: diam_start_listen
��������: �������ؼ����˿�
��    ��: tcpPort : ����������TCP�˿�,Ϊ0������
          sctpPort: ����������SCTP�˿�,Ϊ0������
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_start_server(DiamUINT32  serverId,
                                              const char* masterIp,
                                              const char* slaveIp = NULL,
                                              DiamUINT32 port = 3868,
                                              PROTOCOL_TYPE type = PROTOCOL_TCP);

/**************************************************************************
�� �� ��: diam_modify_server
��������: �������ؼ����˿�
��    ��: tcpPort : ����������TCP�˿�,Ϊ0������
sctpPort: ����������SCTP�˿�,Ϊ0������
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_modify_server(DiamUINT32  serverId,
                                               const char* masterIp,
                                               const char* slaveIp = NULL,
                                               DiamUINT32 port = 3868,
                                               PROTOCOL_TYPE type = PROTOCOL_TCP);

/**************************************************************************
�� �� ��: diam_reset_listen
��������: �������ؼ����˿�
��    ��: tcpPort : ����������TCP�˿�,Ϊ0������
          sctpPort: ����������SCTP�˿�,Ϊ0������
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_stop_server(DiamUINT32 serverId);

/**************************************************************************
�� �� ��: diam_reset_listen
��������: �������ؼ����˿�
��    ��: tcpPort : ����������TCP�˿�,Ϊ0������
sctpPort: ����������SCTP�˿�,Ϊ0������
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_del_server(DiamUINT32 serverId);

/**************************************************************************
�� �� ��: diam_add_peerinfo
��������: ���Եȶ���Ϣ��ӵ�Э��ջ��,���ǲ�����Եȶ˽�������,��Ҫ���ڷ����
          ���ͻ��˵���Ϣ��ӵ�Э��ջ��
��    ��: identity :�Եȶ����ӱ�ʶ
          port     :�Եȶ˶˿�
          useSctp  :�Ƿ�ʹ��SCTP
          useTls   :�Ƿ�ʹ��TLS
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_add_client(DiamUINT32 linkIndex, 
                                              const char* masterIp, 
                                              const char* slaveIp = NULL, 
                                              DiamUINT32 port = 3868,
                                              DiamUINT32 serverId = 0,
                                              PROTOCOL_TYPE type=PROTOCOL_TCP);

/**************************************************************************
�� �� ��: diam_add_peerinfo
��������: ���Եȶ���Ϣ��Э��ջ��ɾ��
��    ��: serverId :�Եȶ����ӱ�ʶ linkIndex:
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_del_client(DiamUINT32 linkIndex);

/**************************************************************************
�� �� ��: diam_create_connect
��������: ����������˵�����
��    ��: linkIndex:��·��
          localip  :����IP
          localport:���ض˿�
          identity :�Զ�IP
          port     :�Զ˶˿�
		  useSctp  :�Ƿ�ʹ��Sctp
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
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
�� �� ��: diam_delete_connect
��������: �ͷŸ��Եȶ˵�����,��Ҫ�������ܷ����ɾ��
��    ��: linkIndex:��·��
�� �� ֵ: �ɹ�:DIAM_RET_SUCCESS,����Ϊʧ��
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_release_connect(DiamUINT32 linkIndex);

/**************************************************************************
�� �� ��: diam_get_linkstatus
��������: ��ȡ��·״̬
��    ��: linkIndex:��·��
�� �� ֵ: DIAM_RET_SUCCESS,��·״̬OK,������·��OK
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_get_linkstatus(DiamUINT32 linkIndex);

/**************************************************************************
�� �� ��: diam_get_peerInfo
��������: 
��    ��: 
�� �� ֵ: DIAM_RET_SUCCESS,��·״̬OK,������·��OK
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_get_allpeerinfo(ConnectInfo& conn);

/**************************************************************************
�� �� ��: diam_send_msg
��������: ������Ϣ
��    ��: CMessage& msg ���͵���Ϣ,��Ϣ����ҪЯ��Ŀ��peer��Ϣ
�� �� ֵ: DIAM_RET_SUCCESS:���ͳɹ�
**************************************************************************/
DIAMETER_EXPORT DiamRetCode diam_send_msg(DiamMsg& msg, DiamUINT32 linkIndex = 0);

#ifdef __cplusplus
}
#endif

#endif

