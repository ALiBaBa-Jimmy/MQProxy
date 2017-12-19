/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年8月28日
**************************************************************************/
#ifndef __DIAM_RESULTCODE_H__
#define __DIAM_RESULTCODE_H__

//rfc3588定义
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


/*BWT项目定义的错误码*/
#define  BWT_DIAMETER_ERROR_USER_UNKNOWN                 5001 //网络无此用户，用户不存在
#define  BWT_DIAMETER_ERROR_AUTH_SCHEME_NOT_SUPPORTED    5006 //不支持鉴权请求中所指示的鉴权方案
#define  BWT_DIAMETER_ERROR_PARAMETER_ERROR	             5103 //参数缺失，参数异常
#define  BWT_DIAMETER_ERROR_AUTH_UNRIGNT                 5104 //鉴权不通过
#define  BWT_DIAMETER_ERROR_OPERATION_NOT_ALLOWED        5105 //不允许用户进行此操作
#define  BWT_DIAMETER_ERROR_USER_DATA_CANNOT_BE_READ     5106 //不允许读所请求的数据
#define  BWT_DIAMETER_ERROR_GROUP_UNKNOWN                5107 //网络无此组，组用户不存在
#define  BWT_DIAMETER_ERROR_RAT_NOT_ALLOWED              5421 //不允许此种无线终端接入
#define  BWT_DIAMETER_ERROR_ROAMING_NOT_ALLOWED          5004 //不允许用户漫游
#define  BWT_DIAMETER_ERROR_USER_DATA_CANNOT_BE_MODIFIED 5110 //不允许修改所请求的用户数据
#define  BWT_DIAMETER_ERROR_GROUP_ISEXIST                5111 //组已经存在
#define  BWT_DIAMETER_ERROR_NO_RESOURCE                  5112 //资源已经耗尽
#define  BWT_DIAMETER_ERROR_UNKNOWN_EPS_SUBSCRIPTION	 5420 //RDS中没用用户UID的EPS签约数据
#define  BWT_DIAMETER_ERROR_EQUIPMENT_UNKNOWN            5422 //在HSS中设备标识不能识别
#define  BWT_DIAMETER_ERROR_UNKOWN_SERVING_NODE          5423 //RDS指示通知请求中源结点不是终端的注册结点
#define  BWT_DIAMETER_AUTHENTICATION_DATA_UNAVAILABLE    4181 //RDS出现了意想不到的临时性错误，请求RAN重新发起请求
#define  BWT_DIAMETER_ERROR_IDENTITIES_DONT_MATCH        5002 //用户的私有标识同公有标识不匹配
#define  DIAMETER_ERROR_IDENTITY_NOT_REGISTERED          5003 //用户没有业务注册，就发起相关操作
#define  DIAMETER_ERROR_IDENTITY_ALREADY_REGISTERED      5005 //用户已完成业务注册
#define  DIAMETER_ERROR_TOO_MUCH_DATA                    5008 //推送到接收端的数据超出其接收能力
#define  DIAMETER_ERROR_NOT_SUPPORTED_USER_DATA          5009 //SCC通知RDS不支持接收到的数据格式
#define  BWT_DIAMETER_ERROR_SUBSCRIBE_UNKNOWN            5424 //订阅不存在
#define  BWT_DIAMETER_ERROR_SKR_STATUS_NOT_MATCH         5425 //遥晕、遥毙、复活状态不匹配

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
