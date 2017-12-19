/*-------------------------------------------------------------------
    smu_sj3_type.h -  ���� SJ3 �ӿ��е�������������

    ��Ȩ���� 2005 -2008 ������˾������HLR��Ŀ��. 

    �޸���ʷ��¼
    --------------------
    15.00.01,  02-28-2008,     zhanghai     ����
---------------------------------------------------------------------*/

#ifndef _SMU_SJ3_TYPE_H_
#define _SMU_SJ3_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "xostype.h"                 // wei
#include "xdb_type_head.h"

#pragma pack(1)
#pragma warning(disable:4786)

//����HLR��EMS͸����OSSģ�����Ϣ���ȱ�־
#define HLR_EMS_TRANSFER_OSS_DATALENGTH  (0xFFFFFFFE)

#define MAX_SDDH_MSG_BUFFER_LEN  2000

#define MAX_OSS_MSG_BUFFER_LEN   1460-SJ3_OPER_RSP_HEAD_LEN  //TCP��data<=1460(����4���ֽ�ͷβ)

// SJ3 ������ͷ����
#define SJ_REQ		0x00						//ҵ�������
#define SJ_RSP		0x01						//ҵ��Ӧ���
#define PU_REQ  	0x04						//�������������½/ע����
#define PU_RSP	    0x05						//����Ӧ�������½/ע����	
#define CW_REQ		0x08						//���������
#define CW_RSP		0x09						//����Ӧ���

// Ӫ�ʽӿ�ҵ����Ϣ�ķ�����Ϣͷ����(���б�ͷ+ҵ����Ϣ˽�б�ͷ+TCPE��װ����6)
#define SJ3_OPER_RSP_HEAD_LEN   (sizeof(t_Sj3_Public_Head)+sizeof(t_Sj3_Oper_Head)+6)
#define SJ3_OPER_RSP_TAIL_LEN   (2)


#define SIZE_OF_MAC_ADDR                                6
#define SIZE_OF_IP_ADDR                                 4
#define SIZE_OF_BTS_ID                                  4
#define SIZE_OF_BTS_RAID_ID                             4
#define SIZE_OF_VOICE_MASK                              4
#define LENGTH_OF_NATIONCODE                            3
#define MAX_APC_COUNT                                   20
#define LENGTH_OF_SHORT_CODES       4
#define LENGTH_OF_KEYUPDATEPERIOD   2
#define MAX_SUB_TELNO_NUM 5 //�涨����Ӻ������Ϊ4��
#define COUNT_OF_SHRINKDN      16     /*�Ǽ�ʹ�õ���λ���Ÿ���*/
#define COUNT_OF_SA            10      /*ָ��Ŀ�������*/
#define LENGTH_OF_NETWORKING   10



typedef XU8 TBWState;
typedef XU8 TBWUTType;
typedef XS8 TRoamFlag;
typedef XS8 TDHCPReletFlag;
typedef XS16 TPeriodRegTime;
typedef XU16 TVLANTag;
typedef XU8  TVoiceMask[SIZE_OF_VOICE_MASK];
typedef XU8 TAPCCount;
typedef XU16 TBWAgrmntID;
typedef XU8 TIPAddr[SIZE_OF_IP_ADDR];
typedef XU8 TMACAddr[SIZE_OF_MAC_ADDR];
typedef XU8 TPCFlag;
typedef XU16 TPCInterval;
typedef XU8 TBTSID[SIZE_OF_BTS_ID];
typedef XU8 TBTSRAID[SIZE_OF_BTS_RAID_ID];

// sj3 ������Ϣͷ
typedef struct  
{
    XU8    TMP1;
    XU8    TMP2;
    XU16 len;

}tTCPE_HEAD;

// sj3 ������Ϣͷ
typedef struct  
{
    tTCPE_HEAD tcpe;
	XU8		ucPackType;						//������(0x00��ҵ������� 0x01��ҵ��Ӧ��� 0x04����������� 0x05������Ӧ��� 0x08����������� 0x09������Ӧ���)
	XU8	    ossId ;
	XU16	usDlgId;						//�ỰID	
}t_Sj3_Public_Head;


// sj3 ҵ�񹫹���Ϣͷ
typedef struct 
{
	XU8						ucOperateObjId;			// ��������ID
	XU8						ucOperateTypeId;	    // ��������ID 0x00��������0x01��ɾ����0x02���޸ģ�0x03����ѯ��0x04���б��ѯ��0x05��������
	XU8						ucIsSucc;				// 0x00���ɹ� 0x01��ʧ��
	XU8						ucIsEnd;				// �Ƿ��к����� 0x00���к�����   0x01�����һ��
	XU8						ucPackId;				// �����
	XU8						uctopic[17];			// ����Ա����
	XU16					usLen;					// ������(��������ͷ)     
}t_Sj3_Oper_Head;



#define LENGTH_OF_NETWORK_ID  10
#define LENGTH_OF_HLR_ID      12
//��¼camtalk��½����Ϣ
typedef struct
{
	XU8 userid[LENGTH_OF_UID];
	XU8 camtalkTelno[LENGTH_OF_TEL];
	XU32 iCssPublicIp;
	XU32 iCssPrivateIp;
	XU8  cssId[LENGTH_OF_CSSID];
	XU32 iXWIpV4;//��¼camtalk��¼�ն˵�IP��PORT
	XU16 sXWIpPort;
	XU8  cCamtalkStatus;//camtalk���ػ�״̬��1��������0���ػ�
	XU8  privRegDate[LENGTH_OF_DATE+1];
	XU8  curRegDate[LENGTH_OF_DATE+1];
    XU8  NetId[LENGTH_OF_NETWORK_ID];
    XU8 updateFlg;
}SHlrCamtalkDynInfo;


typedef struct
{
	XU8 uid[LENGTH_OF_UID];
	XU8 cNetworkID[LENGTH_OF_NETWORK_ID];
    XU8 tel[LENGTH_OF_TEL];
    XU8 VssId[LENGTH_OF_VSSID];
    XU32 sagIp;
    XU8 RegType;
    SHlrCamtalkDynInfo CamtalkInfo;
   	XU8 bsId[LENGTH_OF_BSID];    /*��վID       */
	XU8 bscId[LENGTH_OF_BSCID];       /*��վ������ID */
	XU8 laiId[LENGTH_OF_LAIID];   /*λ����ID     */
    XU8 hsstatus;
	XU32 xwipv4;
	XU16 xwipport;
	XU8  pid[LENGTH_OF_PID]           ; 
	XU8 pidPort;

}SUpdateUserInfoReq;

typedef enum 
{
    e_regTypeVoice = 0,
    e_regTypeBW,
    e_regTypeCalmTalk
}E_HLR_AGENT_REGTYPE;



typedef struct
{
    XU8  uid[LENGTH_OF_UID]; 
    XU8  hsStatus;
    XU8  cNetworkID[LENGTH_OF_NETWORK_ID];
}tRep2Agent;


typedef enum
{
    e_FLAG_TAG = 1,  
	e_INDEX_TAG = 2,//
	e_UID_TAG = 3,//UID��TAG
	e_STATIC_TAG = 4,//��̬���ݵ�TAG
	e_DYN_TAG = 5,//��̬���ݵ�TAG
	e_SS_TAG = 6,//�������ݵ�TAG
	e_NETWORKING_TAG,
	e_TELNO_TAG,
	e_AGENT_VSSID,
	e_AGENT_SAGIP,
	e_AGENT_REGTYPE,
	e_AGENT_CAM_TALK,
	e_AGENT_RegInfo,
	e_AGENT_Report,
	e_AGENT_CTRX = 15,
	e_AGENT_AUTHINFO = 16,
	e_Agent_Pid = 17,
	e_Agent_PidBindInfo = 18,
	e_Agent_USERBM ,  /*����*/
	e_Agent_ModifyInfo,
	e_Agent_UtLease,
	e_Agent_BtsId,
	e_Agent_ReadforSM,
    e_BUTT_TAG
}EAgentTag;

typedef enum
{
    MQTT_MSG_ID_PUSH,       //������Ϣ
    MQTT_MSG_ID_SUBSCRI,  //����
    MQTT_MSG_ID_unSUBSCRI,  //ȡ������
    MQTT_MSG_ID_BUTT        //�Ƿ���Ϣ
}E_InnerMSGIDtoMQTT;
#define  MQTT_PUSHTOPIC_QRY_ALL_INFO    "QryAllInfoMsg"  //��ѯ������Ϣ
#define  MQTT_USER_REGISTER_SUCC_INFO   "UserRegSuccMsg"  //ע��ɹ���Ϣ
#define  MQTT_USER_BUSINESS_CHANGE      "BusinessChangeMsg"//ҵ������Ϣ
#define  MQTT_CALLING_REQ_QryNetWork      "QryNetIdMsg" /*�������� */


#define  MQTT_DELIVERY_2_HLR      "hlr" /*�����hlr�ַ���Ϣ*/

typedef struct
{
	XU32  linkID;                       /*�Զ�����·ID*/
	XU8   peerType;                          /*�Զ�����*/
	XU32  msgLenth;							 /*�ϲ�Ҫ������Ϣ�ĳ���*/
    XU32  msgID;
    XU8   uID[4];
    XCHAR   topic[32];
	XCHAR* pData;							 /*Ҫ���͵�����ͷָ��*/ 
}t_AGENTUA;

typedef struct 
{
    
	t_Sj3_Public_Head		Ph;						// ���ð�ͷ
	t_Sj3_Oper_Head			OpHead ;                // ����ҵ��ͷ
	XU8						usBuf[2000];            // �����ϢΪ 2000
}t_Sj3_Oper_Rsp;
#define   LENGTH_HEAD   (sizeof(t_Sj3_Public_Head)+sizeof(t_Sj3_Oper_Head))
#define   LENGTH_TAIL   (2)
typedef enum 
{

    e_OperObj = 0x99,
    BUTT_OperObj

}E_OperObj;
typedef enum 
{
    e_Qry_OperType = 0X01,
    e_UpdateLocation_OperType = 2,//ע������ 
    e_QryNetWorkId_OperType = 3,/*��DNS��ѯuid��������*/
    e_Delete_OperType = 4,  //ɾ���ϰݷõ� ����Ϣ��UDC�ô���agent���� �������ϵİݷõ�HLR
    e_UpdateHome_OperType = 5,//���¹���hlr��networkid  ����Ϣ��UDC�ô���agent���� ���������û��Ĺ������� HLR
    e_BusinessChange_OperType = 6,/*ҵ������Ϣ*/

    e_Subscribe_OperType = 7,
    e_unSubscribe_OperType = 8,
    e_Smc_Report_SM_Deliviry_Status = 9,/*�ò����û�����smc report��Ϣ����status�ֶ�*/
    e_Smc_Alert_Service_Centre = 10,/*�ò�������alert��Ϣת�������ط��� smc*/
    e_Sag_AuthInfoUpdate = 11,/*��½����*/
    e_Sag_AuthInfoDelete = 12,/*ɾ���ϵ�½���ڴ�*/



    e_Sag_PullQryUidbyTel = 13,/*��UDC��ѯ�û�uid*/

    e_auth_SynUtBindInfo = 14,/*�ò�������ͬ�����ް󶨹�ϵ��*/
    e_Auth_ModifyPidInfo = 15,
    
    e_Auth_UtLeaseSyn=16,
	e_Notify_Insert_Req=17,//֪ͨHLR�·�insert����
	e_Smc_AlertRsp_Status,
    e_ReadyforSMAlertMsg,/*readyforsm ��ϢҪ��ת��������alert*/
    e_QryuidVisitNetId = 20,
    e_HandShake,
    
    BUTT_OperType

}E_OperType;


/************************************************************************
                         ��ϣ��̬��Ϣ
/************************************************************************/



typedef struct
{
    TRoamFlag roamFlag;
    TDHCPReletFlag dhcpReletFlag;
    TPeriodRegTime periodRegTime;
    TVLANTag vlanTag;
    XS8 mem_ipFlag;		/* IP�Ƿ�dhcp��� */
    TAPCCount maxAPCCount;
    TVoiceMask voiceMask;
    XU8 subClass;
    XS8 mtFlag;			/* ����ά�ֱ�� */
    XS16 maxULRate;
    XS16 minULRate;
    XS16 maxDLRate;
    XS16 minDLRate;    
    TAPCCount curBWAddrCount;
    XS8 soft_flag;		/* softphone���: softphone: 7, ����: 0 */
}TBWBasicInfo;

typedef struct
{
	XU8 nationCode[LENGTH_OF_NATIONCODE];  //������
	XU8 telno[LENGTH_OF_TEL];        //����������ĵ绰����
	XU8 totalTelno[LENGTH_OF_TEL];   //��������ĵ绰����
	XU16 telnoType;                        //��������0�������� 1���Ӻ���
}SSubTelnoInfo;

typedef struct
{
	XU8 mem_dns[SIZE_OF_IP_ADDR];
	XU8 mem_ip[SIZE_OF_IP_ADDR];
	XU8 mem_submask[SIZE_OF_IP_ADDR];
	XU8 mem_getway[SIZE_OF_IP_ADDR];
	XU8 mem_rsv[4];	
	//˵�� mem_rsv[4]�ֱ�Ϊ:Rsv[1] CIDFlag[1]     DENFlag[1]     Rsv[1]     
	/*   CIDFlag  ��bit������
		bit0��ʾϵͳ���ܣ�0����֧�ֺڰ��������ƣ�1��֧�ֺڰ��������ƣ���
		bit1��ʾ�ڰ��������±�ʶ��0�������£�1�����£���
		bit2���ֶ�θ��²�����0��1���淢�ͣ���bit2�ն˲�������
				��bit2��������Ҫ��Ϊ����SAG�����ֶ�θ��²�������ΪSAG�Ƚ�BWInfo��ͬ�Ż��·�BWInfo���նˡ�
		Bit3���л��Ż�����ָʾ��1Ϊ֧���л��Ż���0����֧���л��Ż�
		Bit6Bit5Bit4����̬���ִ������ޣ�000Ϊ10%��001Ϊ20%��010Ϊ30%��011Ϊ40%��100Ϊ50%��101Ϊ60%��110Ϊ70%��111Ϊ80%
		����bitԤ������
	*/
	/*	DENFlag     ��bit������
		Bit0Ϊ���ܹ������ñ�ʶ(0�������ü��ܣ�1�����ü���)��
		bit3bit2bit1��ʾ����ǿ��(000:64bit;001:128bit;010:256bit)��
		bit4�Ƿ���Կң�ٱ�ʶ(1��ң�٣�0����ң��)
	*/
	//   add userPriorFlag 2013.11.19
	/*   ���һ��Rsv[1] �������û����ȼ���Ӧ��Ϣ*/
	/*   Bit0~2���û����ȼ���ȡֵ��Χ0~7
		Bit3~4��
		0�������ȼ�+����ҵ�񲻿�ѹ��
		1�������ȼ�+����ҵ���ѹ��
		2�������ȼ�
	*/
	//	����bitԤ�� 	
}TBWMem;

typedef struct  /* ά�ִ��� */
{
	XU16 ul_maintain; /*������С*/
	XU16 dl_maintain; /*������С*/
}TBWMainTain;

typedef struct
{
    TMACAddr macAddr;
    TIPAddr fixIPAddr;
    TBTSID anchorBTSID;
    TBTSRAID btsRAID;
}TBWAddr;

typedef struct
{
    TBWBasicInfo basis;
    TBWAddr addr[MAX_APC_COUNT];
	TBWMainTain maintain;	/*ά�ִ���*/
	TBWMem		meminfo;	/* Memģ�� */
}TBWInfo;
typedef struct
{
    TBWState adminStatus;
    TPCFlag pcFlag;
    TPCInterval pcInterval;

    TBWInfo bwInfo;
}TXdbScuBWData;

typedef struct
{
	XU32 camTalkFlag   :1; //camTalkҵ�񿪹�
	XU32 reserved      :31; //Ԥ��
}TXdbHdbSubOperation;

typedef struct
{
	TXdbHdbSubOperation szSubOperation; //��λ����ҵ�񣬵�iλΪ��i+1ҵ����Ŀ��ػ��ߵ�32*n+��i+1���Ŀ���
	XU8 operProperty[MAX_OPER_CODE_COUNT_OF_ONE_STRUCT][MAX_OPER_PROPERTY_LENGTH]; 
	/*˵����MAX_OPER_CODE_COUNT_OF_ONE_STRUCTһ��SSubHlrOperation֧�ֵ����ҵ���������
	  ����Ϊoperation�������Чλ����ĿǰΪ��ʡ�ڴ�����Ϊ1. MAX_OPER_PROPERTY_LENGTHҵ�������ԣ���󳤶�Ϊ32 */
}SSubHlrOperation;

//�Ӻ����б�
typedef struct
{
	XU8 subTelnoNum;//�ӵ绰�������
	SSubTelnoInfo subTelnoList[MAX_SUB_TELNO_NUM+1];//�ӵ绰�����б�
}SSubTelnoInfoList;

/************************************************************************
                         ��ϣ��̬��Ϣ
/************************************************************************/
typedef struct 
{
	XU8     pid       [LENGTH_OF_PID]           ;     /*PID*/
	XU8     sid       [LENGTH_OF_SID]           ;     /*SID*/
	XU8     tel       [LENGTH_OF_TEL]           ;     /*�绰���룬֧��32λ��*/
	XU8     callerNum [LENGTH_OF_TEL]           ;     /*������ʾ���룬֧��32λ��*/
    XU8     odb		  [LENGTH_OF_ODB]           ;     /*��Ӫ�̱�������λ��ʾ*/
    XU8     hsType    [LENGTH_OF_HSTYPE]        ;     /*�ն�����*/
	XU8     subState                            ;     /*�û�״̬*/
    XU8     authType                            ;     /*��Ȩ����*/
    XU8     twins                               ;     /*�������ʽ*/
	XU8     prepay                              ;     /*Ԥ����*/
	XU8     onecalltwo                          ;     /*һ��˫��*/
	XU8     voice     [LENGTH_OF_VOICE     ]    ;     /*����ҵ��*/
	XU8     sms       [LENGTH_OF_SMS       ]    ;     /*����ҵ��*/
	XU8     bearer    [LENGTH_OF_BEARER    ]    ;	   /*����ҵ��*/
	XU8     narrowBand[LENGTH_OF_NARROWBAND]    ;     /*խ��ҵ��*/
	XU8     broadBand [LENGTH_OF_BROADBAND ]    ;     /*���ҵ��*/
	XU8     roamTplID [LENGTH_OF_ROAM_TPL_ID]   ;     /*����ģ��ID*/
	XU8     ssInfo    [LENGTH_OF_SS        ]    ;     /*��Ӫ���ṩ�Ĳ���ҵ���б�*/
	XU8     opendate  [LENGTH_OF_DATE      ]    ;     /*��������*/
	XU8     vhlr      [LENGTH_OF_VHLR      ]    ;
    XU8     category  [LENGTH_OF_SUb_CAT   ]    ;     /*�����û����*/             
    XU16    serverflag;							/* add by dufei 07.04*/
	XU8    userAuthType;						/*�ն˼�Ȩ*/
	XU8    userUtacperiod[2];
	XU8	    userBm[50];
	XU8    chargeNum       [LENGTH_OF_TEL];
	XU16   chargeType;	
	XU8    chargeFlag;
	XU8    userPriority;
	XU16  dbsRight;							 /*�����鲥Ȩ�� add by lhy 2010.03.30*/
	XU16  unSuccFlag; //ҵ��ɹ���ʶ add by lhy 20111.05.07
	XU8		userRecordAttri;
    TXdbScuBWData     bwData;
	XU8 pidPort;
	XU8 userTag;
	XU8 netSelectTmpId;//����ѡ��ģ�� ID add by lx 2012.05.15
	XU8 changeFlag;//����ѡ��ģ���Ƿ� �仯��ʾ add by lx 2012.05.15
	XU8  StunKillReviveStatus;//ң��ң�и���״̬  add by cyl 2012.09.10
	XU8  stunKillReviveChangeFlag;//ҡ��ҡ�и���仯��ʾadd by lx 2012.11.19
	XU32 timeStamp;
	XU8 DENFlag; //���ܱ�ʶ   add by wj 2013.05.14 
				 /*Bit 7:Ԥ�� Bit 6��Ԥ�� Bit 5��Ԥ�� 
				   Bit 4����Կң�ٱ�ʶ(1��ң�٣�0����ң��) 
				   Bit3 Bit2 Bit1����ʾ����ǿ��(000:64bit;001:128bit;010:256bit) 
				   Bit0 �����ܹ������ñ�ʶ(0�������ü��ܣ�1�����ü���)*/
	XU8 DENFinv[LENGTH_OF_KEYUPDATEPERIOD]; //�������� add by wj 2013.05.14 
	XU8 UserShortCodes[LENGTH_OF_SHORT_CODES];//�û��̺ź���add 2013.08.14
	XU16 UserCommRange;//�û�ͨ��Ȩ��
	XU16 DcsUserOrganizeID	; //��֯ID
	XU8 preferLanguageInd;//��ʾ����������ָʾadd by wj 2013.08.30
	XU16 usSrvAgrmntId; // �ն�ҵ��������Ϣ(���ЭԼģ��ID)add 2013.11.19
	SSubHlrOperation szSubHLROperation[MAX_OPERATION_COUNT]; //���SSubHlrOperation������MAX_OPERATION_COUNTĿǰ����1���洢ҵ����Ϊ1-32��ҵ����Ϣ��ÿҵ��������32��, MAX_OPERATION_COUNT�궨��Ҫ��1
	SSubTelnoInfoList subTelnoInfoList;     /*�Ӻ����б�*/
    XU8 liId[LIID_LEN];
	XU8 liMode;
	XU8 cCid[LENGTH_OF_CID]; //����CID��Ӫ��ģ���
	XU8 LocalNetwork[LENGTH_OF_NETWORK_ID];
}TXdbHashStat;



/************************************************************************
                         ��ϣ��̬��Ϣ
/************************************************************************/
#define SIZE_OF_UT_ACTIVE_SWV          4
#define SIZE_OF_UT_STANDBY_SWV                                      4
#define SIZE_OF_UT_HWV         16
#define SIZE_OF_UT_SW         2
#define SIZE_OF_UT_HW         2
#define LENGTH_OF_PID 4
#define LENGTH_OF_UID 4
#define LENGTH_OF_BTSID 4

typedef XU8 TUTActiveSWV[SIZE_OF_UT_ACTIVE_SWV];  //4
typedef XU8 TUTStandbySWV[SIZE_OF_UT_STANDBY_SWV]; //4
typedef XU8 TUTHWV[SIZE_OF_UT_HWV]; //16
typedef XU8 TUTSWType[SIZE_OF_UT_SW];//2   
typedef XU8 TUTHWType[SIZE_OF_UT_HW];//2

typedef struct
{
    TUTHWType hwType;
    TUTSWType swType;
    TUTActiveSWV actvSWV; 				//4
    TUTStandbySWV standbySWV;
    TUTHWV hwVersion;
}TUTInfo;


/* ��̬���ݱ�ṹ */

typedef struct 
{
	XU8    uid[LENGTH_OF_UID];
	XU8     bsID     [LENGTH_OF_BSID] ;
    XU8     bscID    [LENGTH_OF_BSCID];
    XU8     laiID    [LENGTH_OF_LAIID];
	XU8     vssID    [LENGTH_OF_VSSID]; //����êSXC
	XU8     regDate [LENGTH_OF_DATE];    /*��ǰע��ʱ��*/
} TXdbScuVoiceAnchor;//add 09.21

#ifndef MillTime64
	#define MillTime64 unsigned long long
#endif

//�ն�״̬�ĸ����ֶεĸ���ʱ��//2013.05.17
typedef struct  
{
	MillTime64  AnchorTime;		//bsID,bscID,laiID,vssID����ʱ��
	MillTime64  PidTime;			//pid,pidPort����ʱ��
	MillTime64  voiceAnchorTime;	//voiceAnchor����ʱ��
	MillTime64  PurgedTime;       //HsStatus��purge����ʱ��    
	MillTime64  RoamRestrictTime; //HsStatus��RoamRestrict����ʱ��
	MillTime64  MNRFTime;         //HsStatus��MNRF����ʱ��
	MillTime64  MCEFTime;         //HsStatus��MCEF����ʱ��
	MillTime64  twinTime;         //HsStatus��twin����ʱ��
}SHashDynUptTime;

/************************************************************************
                         ��ϣ��̬��Ϣ
/************************************************************************/
typedef struct 
{
	XU8     hsStatus    ;
	XU8     hsVersion    ;
	XU8     bsID     [LENGTH_OF_BSID] ;
    XU8     bscID    [LENGTH_OF_BSCID];
    XU8     laiID    [LENGTH_OF_LAIID];
	XU8     vssID    [LENGTH_OF_VSSID];
	XU32	xwipv4;
	XU16	xwipport;
	XU8     pid[LENGTH_OF_PID];
	//XU8     smc      [LENGTH_OF_SMCLIST];  /*����SMC�б�*/
	/* add dufei */
	XU8   currRegDate [LENGTH_OF_DATE];    /*��ǰע��ʱ��*/
	XU8	  prevRegDate[LENGTH_OF_DATE]; /*�ϴ�ע��ʱ��*/
	XU8	    hsType;
	TUTInfo utInfo;	// 05.29
	TXdbScuVoiceAnchor voiceAnchor; //��������ê��Ϣ add 09.21
	XU8 pidPort; //2011.12.15 mz add
	XU32 sagIpv4;//2013.03.18 add sagip
	SHashDynUptTime hashDynUptTime;//hash��̬���ݵĸ���ʱ�� 2013.05.17
	SHlrCamtalkDynInfo hashCamtalkDyn;//��¼��UID��Ӧ��camtalk��½��Ϣ
	XU8  cNetworkID[LENGTH_OF_NETWORK_ID];//���������ID�ļ�¼
	XU8  authNet[LENGTH_OF_NETWORK_ID];//��½������
	XU32 sagIp_voice;
    XU32 sagIp_data;
    XU8 updateFlg;
}TXdbHashDyn;

/************************************************************************
                         ��ϣ������Ϣ
/************************************************************************/



/*���洢��Ҫ����״̬�����Ĳ���ҵ��*/

/*����������ǰת*/
typedef struct
{
	XU8    state;
	XU8    dn    [LENGTH_OF_SS_DN    ];
	XU8    option[LENGTH_OF_SS_OPTION];
}TXdbHashSsCfu;
/*��æ����ǰת*/
typedef TXdbHashSsCfu TXdbHashSsCfb;

/*��Ӧ�����ǰת*/
typedef TXdbHashSsCfu TXdbHashSsCfnry;

/*���ɼ�����ǰת*/
typedef TXdbHashSsCfu TXdbHashSsCfnrc;

/*��������ǰת*/
typedef TXdbHashSsCfu TXdbHashSsCf;

/*���к�����ʾ*/
typedef TXdbHashSsCfu TXdbHashSsColp;
/*�Ȼ�*/
typedef struct 
{
	XU8    state;
	XU8    hotDN[LENGTH_OF_SS_HOTLINE];
}TXdbHashSsHotLine; 

/*��ʱ�Ȼ�*/
typedef struct
{
	XU8    state;
	XU8    hotDN[LENGTH_OF_SS_HOTLINE];

}TXdbHashSsHotLineCS;
/*��ʱ�Ȼ�*/
 typedef struct
 {
     XU8    state;
     XU8    hotDN[LENGTH_OF_SS_HOTLINE];
 
 }TXdbHashSsHotLineJS;

 /*�������*/
typedef struct
{
	XU8    state;
	XU8    callPwd[LENGTH_OF_PWD_CALL]; 
}TXdbHashSsCallPswd;

/*��������*/
typedef struct
{
	XU8     state;
	XU8     odbPswd[LENGTH_OF_PWD_ODB];  

}TXdbHashSsOdbPswd;

/*��λ����*/
typedef struct 
{
	XU8    shrinkDn;
	XU8    realDn  [LENGTH_OF_REALDN  ];
}TXdbHashShrinkDn;

typedef struct
{
	XU8           state;
	XU8           count;
	TXdbHashShrinkDn  shrinkDn[COUNT_OF_SHRINKDN]; 

}TXdbHashSsShrinkDN;

typedef struct
{
	XU8           state;
	XU8           kValue;
     XU8           pwd[LENGTH_OF_SS_PWD_OUTPRIV];

}TXdbHashSsOutPriv;

typedef struct
{
   XU8 dn[LENGTH_OF_TEL];
   
}TSaSingle;

typedef struct
{
    XU8    state;
    XU8    count;
    TSaSingle sa[COUNT_OF_SA];
    
}TXdbHashSsAimLimt;

typedef TXdbHashSsAimLimt TXdbHashSsAimConn;

/*Ⱥ���û�*/
typedef struct
{
	XU8     state;
    XU8     msisdn[LENGTH_OF_TEL];
    XU8     prn[LENGTH_OF_CTX_PRN];
    XU32    ctxID;
    XU32    groupID;
    XU32    popedom_of_crsgp;  /*ͬ�����Ȩ��*/
    XU32    popedom_of_scr;    /*ָ������Ȩ��*/
	XU8     priv[LENGTH_OF_PRIV];   /*����Ȩ��*/

}TXdbHashSsCtrx;


/*һ��˫��*/
typedef struct
{
    
    XU8 state;  /*�û�״̬����ʾǩԼ*/
    XU8 dn[LENGTH_OF_TEL];
    
}TXdbHashSsOneForTwo;


/*�����*/
typedef struct
{
    XU8 state;
    
}TXdbHashSsNotDisturb;

typedef TXdbHashSsNotDisturb TXdbHashSsCw;

/*������*/
typedef struct
{
    XU8 account[LENGTH_OF_TEL];
    XU8 pwd[LENGTH_OF_SS_PWD_FFC];
        
}TXdbHashFFC;

typedef struct
{
    /*������ǰת*/
	TXdbHashSsCfu      tCfu;
	
    /*��æǰת*/
    TXdbHashSsCfb      tCfb;
    
	/*��Ӧ��ǰת*/
    TXdbHashSsCfnry    tCfnry;
	
	/*���ɼ�ǰת*/
    TXdbHashSsCfnrc    tCfnrc;

    /*�Ȼ�*/
    TXdbHashSsHotLine  tHotline; /*scu���ĺ�ɾ��*/

    /*��ʱ�Ȼ�*/
    TXdbHashSsHotLineCS tHotlineCS;

    /*��ʱ�Ȼ�*/
    TXdbHashSsHotLineJS tHotlineJS;

    /*�������*/
    TXdbHashSsCallPswd tCallPswd;

    /*��λ����*/
    TXdbHashSsShrinkDN tShrinkDN;//[COUNT_OF_SHRINKDN];

    /*��������*/
    TXdbHashSsOutPriv tOutPriv;

    /*ָ��Ŀ��������*/
    TXdbHashSsAimLimt tAimLimit;

    /*ָ��Ŀ��������*/
    TXdbHashSsAimConn tAimConn;

    /*�����*/
    TXdbHashSsNotDisturb tNotDisturb;

    /*���еȴ�*/
    TXdbHashSsCw tCw;

    /*����������*/
    TXdbHashFFC  tFFC;
    
	/*Ⱥ���û�*/
    TXdbHashSsCtrx     tCtrx;


    TXdbHashSsOneForTwo tOneForTwo; /*һ��˫��*/    
    

}TXdbHashSS;



/******************Dyn Information*************************/

typedef struct
{
	XU8   uid       [LENGTH_OF_UID]           ;     /*UID*/
	XU8   hsType;				//�ն�����
	XU8   hsStatus;                        /*�ն�״̬     */
	XU8   hsVersion;                        /*�ն˰汾     */
	XU8   bsId        [LENGTH_OF_BSID];    /*��վID       */
	XU8   bscId   [LENGTH_OF_BSCID];                        /*��վ������ID */
	XU8   laiId       [LENGTH_OF_LAIID];   /*λ����ID     */
	XU8   vssId       [LENGTH_OF_VSSID];   /*����ID     */
	XU8   prevRegDate [LENGTH_OF_DATE];    /*�ϴ�ע��ʱ�� */
	XU8   currRegDate [LENGTH_OF_DATE];    /*��ǰע��ʱ��*/
	XU32	xwipv4;
	XU16	xwipport;
	XU8   pid       [LENGTH_OF_PID]           ; 
	TUTInfo	utInfo;	
	TXdbScuVoiceAnchor voiceAnchor; //��������ê��Ϣ add 09.21
	XU8 pidPort; 	//2011.12.15mz����
	XU32 sagIpv4;/*��¼SAGIP add 2013.03.18*/
	SHashDynUptTime hashDynUptTime;//��̬������Ϣ����ʱ��2013.05.17
	SHlrCamtalkDynInfo camtalkDynInfo;//camtalk��½��Ϣ2015.04.15
	XU8  cNetworkID[LENGTH_OF_NETWORK_ID];//���������ID�ļ�¼
	XU8  authNet[LENGTH_OF_NETWORK_ID];//���������ID�ļ�¼
	XU32 sagIp_voice;
    XU32 sagIp_data;
    XU8 updateFlg;
}TXdbHdbDyn;

/*һ�������µ�����*/
typedef struct
{
    TXdbHdbDyn DynInfo;
    XU8  HomeNetID[LENGTH_OF_NETWORK_ID];
    XU8  OldNetID[LENGTH_OF_NETWORK_ID];/*�ϰݷõ������*/
    XU8 regType;
}tUpdateDyn;


typedef enum
{
   e_TYPE_HLR,
   e_TYPE_UDC,
   
   e_TYPE_INVALID

}e_Agent_Type;



typedef struct
{
	XU8 uid[LENGTH_OF_UID];
    XU8 authNet[LENGTH_OF_NETWORK_ID];
    XU8 OldauthNet[LENGTH_OF_NETWORK_ID];
	
    XU8 AuthType;
	XU8 pid[LENGTH_OF_PID]; 
	XU8 pidPort;
    XU8 hsStatus;
    XU8 homeNet[LENGTH_OF_NETWORK_ID];
    TXdbHdbDyn tOldDynInfo;
}tAuthInfo;

extern XU8 g_HLRagent_Type;
#pragma pack()

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xos.h*/
