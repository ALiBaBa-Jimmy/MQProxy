#ifndef __SPR_ALARM_STRUCT_H
#define __SPR_ALARM_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef enum
{
	eEHSS_ALARM_ID_IP_CHANGE	= 10000, //IP 地址更改
	eEHSS_ALARM_ID_IP_DELETE	= 10001, //IP 地址删除
	eEHSS_ALARM_ID_S6A_LINK		= 10002, //eMME到eHSS链路故障
	eEHSS_ALARM_ID_CX_LINK		= 10003, //TCF到eHSS链路故障
	eEHSS_ALARM_ID_PH_LINK		= 10004, //SMC到eHSS链路故障
	eEHSS_ALARM_ID_SH_LINK		= 10005, //TAS到eHSS链路故障
	eEHSS_ALARM_ID_DB_LINK		= 10100, //数据库连接失败告警
	eEHSS_ALARM_ID_DB_CHANGE	= 10101, //数据库连接修改告警
	eEHSS_ALARM_ID_DB_NOT_CFG	= 10102, //数据库没有配置
	eEHSS_ALARM_ID_USER			= 12000, //用户不存在
	eEHSS_ALARM_ID_HA_STATUS	= 12001, //主备切换

}EEHssAlarmID;

//eHSS告警分类，每类告警的key值不一样
typedef enum
{
	E_EHSS_ALARM_TYPE_DIAMTER_LINK	= 0, // diameter链路相关告警 
	E_EHSS_ALARM_TYPE_DB			= 1, // 数据库相关告警
	E_EHSS_ALARM_TYPE_IP			= 2,  // IP相关告警 
	E_EHSS_ALARM_TYPE_USER			= 3,  // 用户相关告警 
}EEHSSAlarmType;

typedef enum
{
	EHSS_ALARM_FIELDID_RACK_NO			= 1,  //机架号 
	EHSS_ALARM_FIELDID_SUBRACK_NO		= 2,  //机框号 
	EHSS_ALARM_FIELDID_SLOT_NO			= 3,  //槽位号 
	EHSS_ALARM_FIELDID_NE_ID			= 4,  //网元ID 
	EHSS_ALARM_FIELDID_MOULD_ID			= 5,  //进程ID 
	EHSS_ALARM_FIELDID_LINK_ID			= 6,  //链路ID 
	EHSS_ALARM_FIELDID_LINK_GROUP		= 7,  //链路组ID
	EHSS_ALARM_FIELDID_TARGET_EMME_IP	= 8,  //目标eMME IP
	EHSS_ALARM_FIELDID_TARGET_EMME_PORT = 9,  //目标eMME Port
	EHSS_ALARM_FIELDID_TARGET_TCF_IP	= 10,  //目标TCF IP
	EHSS_ALARM_FIELDID_TARGET_TCF_PORT	= 11,  //目标TCF Port
	EHSS_ALARM_FIELDID_TARGET_SMC_IP	= 12,  //目标SMC IP
	EHSS_ALARM_FIELDID_TARGET_SMC_PORT	= 13,  //目标SMC Port
	EHSS_ALARM_FIELDID_TARGET_TAS_IP	= 14,  //目标TAS IP
	EHSS_ALARM_FIELDID_TARGET_TAS_PORT	= 15,  //目标TAS Port
	EHSS_ALARM_FIELDID_IP_ETHE_MENT		= 16,  //IP网口类型 (IpEtheMent;//地址类型-枚举：21：BACK1;22：BACK2;41：FAB1;42：FAB2)
	EHSS_ALARM_FIELDID_IP_TYPE			= 17,  //IP 类型，0:浮动IP
	EHSS_ALARM_FIELDID_IP_OLD			= 18,  //原来的IP 
	EHSS_ALARM_FIELDID_MASK_OLD			= 19,  //原来的掩码
	EHSS_ALARM_FIELDID_IP_NEW			= 20,  //新的IP 
	EHSS_ALARM_FIELDID_MASK_NEW			= 21,  //新的掩码
	EHSS_ALARM_FIELDID_PVI				= 22,  //pvi
	EHSS_ALARM_FIELDID_PVI_TYPE			= 23,  //pvitype
	EHSS_ALARM_FIELDID_PUI				= 24,  //pui 
	EHSS_ALARM_FIELDID_PUI_TYPE			= 25,  //puitype
	EHSS_ALARM_FIELDID_HA_STATUS_OLD	= 26,  //原来的状态
	EHSS_ALARM_FIELDID_HA_STATUS_NEW	= 27,  //新的状态
}EEHssAlarmFieldId;

#define  MAX_ALARM_LENGTH 1000

//告警key对应的与seq相关的信息
typedef struct SAlarmSeqValue 
{
	
	XU64 nAlarmSqn;		//告警sqn	
	XU64 nUpdateTime;	//告警sqn更新的时间
	XU32 nAlarmId;
}SAlarmSeqValue;


//OAM_AlarmSend注册函数,主要为了解决windows下动态库消息发送不了的问题
typedef XS32 (*callbackOAM_AlarmSendFun) (XVOID* pMsg, XU32 uiAlarmClass, XU32 uiAlarmFlag );

#pragma pack()

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
