#ifndef __SPR_ALARM_STRUCT_H
#define __SPR_ALARM_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef enum
{
	eEHSS_ALARM_ID_IP_CHANGE	= 10000, //IP ��ַ����
	eEHSS_ALARM_ID_IP_DELETE	= 10001, //IP ��ַɾ��
	eEHSS_ALARM_ID_S6A_LINK		= 10002, //eMME��eHSS��·����
	eEHSS_ALARM_ID_CX_LINK		= 10003, //TCF��eHSS��·����
	eEHSS_ALARM_ID_PH_LINK		= 10004, //SMC��eHSS��·����
	eEHSS_ALARM_ID_SH_LINK		= 10005, //TAS��eHSS��·����
	eEHSS_ALARM_ID_DB_LINK		= 10100, //���ݿ�����ʧ�ܸ澯
	eEHSS_ALARM_ID_DB_CHANGE	= 10101, //���ݿ������޸ĸ澯
	eEHSS_ALARM_ID_DB_NOT_CFG	= 10102, //���ݿ�û������
	eEHSS_ALARM_ID_USER			= 12000, //�û�������
	eEHSS_ALARM_ID_HA_STATUS	= 12001, //�����л�

}EEHssAlarmID;

//eHSS�澯���࣬ÿ��澯��keyֵ��һ��
typedef enum
{
	E_EHSS_ALARM_TYPE_DIAMTER_LINK	= 0, // diameter��·��ظ澯 
	E_EHSS_ALARM_TYPE_DB			= 1, // ���ݿ���ظ澯
	E_EHSS_ALARM_TYPE_IP			= 2,  // IP��ظ澯 
	E_EHSS_ALARM_TYPE_USER			= 3,  // �û���ظ澯 
}EEHSSAlarmType;

typedef enum
{
	EHSS_ALARM_FIELDID_RACK_NO			= 1,  //���ܺ� 
	EHSS_ALARM_FIELDID_SUBRACK_NO		= 2,  //����� 
	EHSS_ALARM_FIELDID_SLOT_NO			= 3,  //��λ�� 
	EHSS_ALARM_FIELDID_NE_ID			= 4,  //��ԪID 
	EHSS_ALARM_FIELDID_MOULD_ID			= 5,  //����ID 
	EHSS_ALARM_FIELDID_LINK_ID			= 6,  //��·ID 
	EHSS_ALARM_FIELDID_LINK_GROUP		= 7,  //��·��ID
	EHSS_ALARM_FIELDID_TARGET_EMME_IP	= 8,  //Ŀ��eMME IP
	EHSS_ALARM_FIELDID_TARGET_EMME_PORT = 9,  //Ŀ��eMME Port
	EHSS_ALARM_FIELDID_TARGET_TCF_IP	= 10,  //Ŀ��TCF IP
	EHSS_ALARM_FIELDID_TARGET_TCF_PORT	= 11,  //Ŀ��TCF Port
	EHSS_ALARM_FIELDID_TARGET_SMC_IP	= 12,  //Ŀ��SMC IP
	EHSS_ALARM_FIELDID_TARGET_SMC_PORT	= 13,  //Ŀ��SMC Port
	EHSS_ALARM_FIELDID_TARGET_TAS_IP	= 14,  //Ŀ��TAS IP
	EHSS_ALARM_FIELDID_TARGET_TAS_PORT	= 15,  //Ŀ��TAS Port
	EHSS_ALARM_FIELDID_IP_ETHE_MENT		= 16,  //IP�������� (IpEtheMent;//��ַ����-ö�٣�21��BACK1;22��BACK2;41��FAB1;42��FAB2)
	EHSS_ALARM_FIELDID_IP_TYPE			= 17,  //IP ���ͣ�0:����IP
	EHSS_ALARM_FIELDID_IP_OLD			= 18,  //ԭ����IP 
	EHSS_ALARM_FIELDID_MASK_OLD			= 19,  //ԭ��������
	EHSS_ALARM_FIELDID_IP_NEW			= 20,  //�µ�IP 
	EHSS_ALARM_FIELDID_MASK_NEW			= 21,  //�µ�����
	EHSS_ALARM_FIELDID_PVI				= 22,  //pvi
	EHSS_ALARM_FIELDID_PVI_TYPE			= 23,  //pvitype
	EHSS_ALARM_FIELDID_PUI				= 24,  //pui 
	EHSS_ALARM_FIELDID_PUI_TYPE			= 25,  //puitype
	EHSS_ALARM_FIELDID_HA_STATUS_OLD	= 26,  //ԭ����״̬
	EHSS_ALARM_FIELDID_HA_STATUS_NEW	= 27,  //�µ�״̬
}EEHssAlarmFieldId;

#define  MAX_ALARM_LENGTH 1000

//�澯key��Ӧ����seq��ص���Ϣ
typedef struct SAlarmSeqValue 
{
	
	XU64 nAlarmSqn;		//�澯sqn	
	XU64 nUpdateTime;	//�澯sqn���µ�ʱ��
	XU32 nAlarmId;
}SAlarmSeqValue;


//OAM_AlarmSendע�ắ��,��ҪΪ�˽��windows�¶�̬����Ϣ���Ͳ��˵�����
typedef XS32 (*callbackOAM_AlarmSendFun) (XVOID* pMsg, XU32 uiAlarmClass, XU32 uiAlarmFlag );

#pragma pack()

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
