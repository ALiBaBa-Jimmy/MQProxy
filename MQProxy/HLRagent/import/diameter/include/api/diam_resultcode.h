/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��8��28��
**************************************************************************/
#ifndef __DIAM_RESULTCODE_H__
#define __DIAM_RESULTCODE_H__

//rfc3588����
#define  DIAMETER_MULTI_ROUND_AUTH                       1001
#define  DIAMETER_SUCCESS                                2001
#define  DIAMETER_LIMITED_SUCCESS                        2002
#define  DIAMETER_COMMAND_UNSUPPORTED                    3001
#define  DIAMETER_UNABLE_TO_DELIVER                      3002
#define  DIAMETER_REALM_NOT_SERVED                       3003
#define  DIAMETER_TOO_BUSY                               3004
#define  DIAMETER_LOOP_DETECTED                          3005
#define  DIAMETER_REDIRECT_INDICATION                    3006
#define  DIAMETER_APPLICATION_UNSUPPORTED                3007
#define  DIAMETER_INVALID_HDR_BITS                       3008
#define  DIAMETER_INVALID_AVP_BITS                       3009
#define  DIAMETER_UNKNOWN_PEER                           3010
#define  DIAMETER_AUTHENTICATION_REJECTED                4001
#define  DIAMETER_OUT_OF_SPACE                           4002
#define  DIAMETER_ELECTION_LOST                          4003
#define  DIAMETER_AVP_UNSUPPORTED                        5001
#define  DIAMETER_UNKNOWN_SESSION_ID                     5002
#define  DIAMETER_AUTHORIZATION_REJECTED                 5003
#define  DIAMETER_INVALID_AVP_VALUE                      5004
#define  DIAMETER_MISSING_AVP                            5005
#define  DIAMETER_RESOURCES_EXCEEDED                     5006
#define  DIAMETER_CONTRADICTING_AVPS                     5007
#define  DIAMETER_AVP_NOT_ALLOWED                        5008
#define  DIAMETER_AVP_OCCURS_TOO_MANY_TIMES              5009
#define  DIAMETER_NO_COMMON_APPLICATION                  5010
#define  DIAMETER_UNSUPPORTED_VERSION                    5011
#define  DIAMETER_UNABLE_TO_COMPLY                       5012
#define  DIAMETER_INVALID_BIT_IN_HEADER                  5013
#define  DIAMETER_INVALID_AVP_LENGTH                     5014
#define  DIAMETER_INVALID_MESSAGE_LENGTH                 5015
#define  DIAMETER_INVALID_AVP_BIT_COMBO                  5016
#define  DIAMETER_NO_COMMON_SECURITY                     5017


/*BWT��Ŀ����Ĵ�����*/
#define  BWT_DIAMETER_ERROR_USER_UNKNOWN                 5001 //�����޴��û����û�������
#define  BWT_DIAMETER_ERROR_AUTH_SCHEME_NOT_SUPPORTED    5006 //��֧�ּ�Ȩ��������ָʾ�ļ�Ȩ����
#define  BWT_DIAMETER_ERROR_PARAMETER_ERROR	             5103 //����ȱʧ�������쳣
#define  BWT_DIAMETER_ERROR_AUTH_UNRIGNT                 5104 //��Ȩ��ͨ��
#define  BWT_DIAMETER_ERROR_OPERATION_NOT_ALLOWED        5105 //�������û����д˲���
#define  BWT_DIAMETER_ERROR_USER_DATA_CANNOT_BE_READ     5106 //������������������
#define  BWT_DIAMETER_ERROR_GROUP_UNKNOWN                5107 //�����޴��飬���û�������
#define  BWT_DIAMETER_ERROR_RAT_NOT_ALLOWED              5421 //��������������ն˽���
#define  BWT_DIAMETER_ERROR_ROAMING_NOT_ALLOWED          5004 //�������û�����
#define  BWT_DIAMETER_ERROR_USER_DATA_CANNOT_BE_MODIFIED 5110 //�������޸���������û�����
#define  BWT_DIAMETER_ERROR_GROUP_ISEXIST                5111 //���Ѿ�����
#define  BWT_DIAMETER_ERROR_NO_RESOURCE                  5112 //��Դ�Ѿ��ľ�
#define  BWT_DIAMETER_ERROR_UNKNOWN_EPS_SUBSCRIPTION	 5420 //RDS��û���û�UID��EPSǩԼ����
#define  BWT_DIAMETER_ERROR_EQUIPMENT_UNKNOWN            5422 //��HSS���豸��ʶ����ʶ��
#define  BWT_DIAMETER_ERROR_UNKOWN_SERVING_NODE          5423 //RDSָʾ֪ͨ������Դ��㲻���ն˵�ע����
#define  BWT_DIAMETER_AUTHENTICATION_DATA_UNAVAILABLE    4181 //RDS���������벻������ʱ�Դ�������RAN���·�������
#define  BWT_DIAMETER_ERROR_IDENTITIES_DONT_MATCH        5002 //�û���˽�б�ʶͬ���б�ʶ��ƥ��
#define  DIAMETER_ERROR_IDENTITY_NOT_REGISTERED          5003 //�û�û��ҵ��ע�ᣬ�ͷ�����ز���
#define  DIAMETER_ERROR_IDENTITY_ALREADY_REGISTERED      5005 //�û������ҵ��ע��
#define  DIAMETER_ERROR_TOO_MUCH_DATA                    5008 //���͵����ն˵����ݳ������������
#define  DIAMETER_ERROR_NOT_SUPPORTED_USER_DATA          5009 //SCC֪ͨRDS��֧�ֽ��յ������ݸ�ʽ
#define  BWT_DIAMETER_ERROR_SUBSCRIBE_UNKNOWN            5424 //���Ĳ�����
#define  BWT_DIAMETER_ERROR_SKR_STATUS_NOT_MATCH         5425 //ң�Ρ�ң�С�����״̬��ƥ��

typedef enum
{
    RCODE_NOT_PRESENT,
    RCODE_INFORMATIONAL,
    RCODE_SUCCESS,
    RCODE_PROTOCOL_ERROR,
    RCODE_TRANSIENT_FAILURE,
    RCODE_PERMANENT_FAILURE
} RCODE;

#endif //__DIAM_RESULTCODE_H__
