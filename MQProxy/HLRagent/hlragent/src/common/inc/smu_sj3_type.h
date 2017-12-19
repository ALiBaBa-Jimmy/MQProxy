/*-------------------------------------------------------------------
    smu_sj3_type.h -  定义 SJ3 接口中的所有数据类型

    版权所有 2005 -2008 信威公司深研所HLR项目组. 

    修改历史记录
    --------------------
    15.00.01,  02-28-2008,     zhanghai     创建
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

//定义HLR的EMS透传到OSS模块的消息长度标志
#define HLR_EMS_TRANSFER_OSS_DATALENGTH  (0xFFFFFFFE)

#define MAX_SDDH_MSG_BUFFER_LEN  2000

#define MAX_OSS_MSG_BUFFER_LEN   1460-SJ3_OPER_RSP_HEAD_LEN  //TCP包data<=1460(包括4个字节头尾)

// SJ3 公共包头类型
#define SJ_REQ		0x00						//业务请求包
#define SJ_RSP		0x01						//业务应答包
#define PU_REQ  	0x04						//公共请求包（登陆/注销）
#define PU_RSP	    0x05						//公共应答包（登陆/注销）	
#define CW_REQ		0x08						//联络请求包
#define CW_RSP		0x09						//联络应答包

// 营帐接口业务消息的返回消息头长度(公有报头+业务消息私有报头+TCPE封装长度6)
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
#define MAX_SUB_TELNO_NUM 5 //规定最大子号码个数为4个
#define COUNT_OF_SHRINKDN      16     /*登记使用的缩位拨号个数*/
#define COUNT_OF_SA            10      /*指定目的码个数*/
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

// sj3 公共消息头
typedef struct  
{
    XU8    TMP1;
    XU8    TMP2;
    XU16 len;

}tTCPE_HEAD;

// sj3 公共消息头
typedef struct  
{
    tTCPE_HEAD tcpe;
	XU8		ucPackType;						//包类型(0x00：业务请求包 0x01：业务应答包 0x04：公共请求包 0x05：公共应答包 0x08：联络请求包 0x09：联络应答包)
	XU8	    ossId ;
	XU16	usDlgId;						//会话ID	
}t_Sj3_Public_Head;


// sj3 业务公共消息头
typedef struct 
{
	XU8						ucOperateObjId;			// 操作对象ID
	XU8						ucOperateTypeId;	    // 操作类型ID 0x00：新增；0x01：删除；0x02：修改；0x03：查询；0x04：列表查询；0x05：其它；
	XU8						ucIsSucc;				// 0x00：成功 0x01：失败
	XU8						ucIsEnd;				// 是否有后续包 0x00：有后续包   0x01：最后一包
	XU8						ucPackId;				// 包序号
	XU8						uctopic[17];			// 操作员名字
	XU16					usLen;					// 包长度(不包括包头)     
}t_Sj3_Oper_Head;



#define LENGTH_OF_NETWORK_ID  10
#define LENGTH_OF_HLR_ID      12
//记录camtalk登陆的信息
typedef struct
{
	XU8 userid[LENGTH_OF_UID];
	XU8 camtalkTelno[LENGTH_OF_TEL];
	XU32 iCssPublicIp;
	XU32 iCssPrivateIp;
	XU8  cssId[LENGTH_OF_CSSID];
	XU32 iXWIpV4;//记录camtalk登录终端的IP和PORT
	XU16 sXWIpPort;
	XU8  cCamtalkStatus;//camtalk开关机状态：1：开机；0：关机
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
   	XU8 bsId[LENGTH_OF_BSID];    /*基站ID       */
	XU8 bscId[LENGTH_OF_BSCID];       /*基站控制器ID */
	XU8 laiId[LENGTH_OF_LAIID];   /*位置区ID     */
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
	e_UID_TAG = 3,//UID的TAG
	e_STATIC_TAG = 4,//静态数据的TAG
	e_DYN_TAG = 5,//动态数据的TAG
	e_SS_TAG = 6,//补充数据的TAG
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
	e_Agent_USERBM ,  /*别名*/
	e_Agent_ModifyInfo,
	e_Agent_UtLease,
	e_Agent_BtsId,
	e_Agent_ReadforSM,
    e_BUTT_TAG
}EAgentTag;

typedef enum
{
    MQTT_MSG_ID_PUSH,       //推送消息
    MQTT_MSG_ID_SUBSCRI,  //订阅
    MQTT_MSG_ID_unSUBSCRI,  //取消订阅
    MQTT_MSG_ID_BUTT        //非法消息
}E_InnerMSGIDtoMQTT;
#define  MQTT_PUSHTOPIC_QRY_ALL_INFO    "QryAllInfoMsg"  //查询所有信息
#define  MQTT_USER_REGISTER_SUCC_INFO   "UserRegSuccMsg"  //注册成功消息
#define  MQTT_USER_BUSINESS_CHANGE      "BusinessChangeMsg"//业务变更消息
#define  MQTT_CALLING_REQ_QryNetWork      "QryNetIdMsg" /*呼叫请求 */


#define  MQTT_DELIVERY_2_HLR      "hlr" /*向各个hlr分发消息*/

typedef struct
{
	XU32  linkID;                       /*对端主链路ID*/
	XU8   peerType;                          /*对端类型*/
	XU32  msgLenth;							 /*上层要发送消息的长度*/
    XU32  msgID;
    XU8   uID[4];
    XCHAR   topic[32];
	XCHAR* pData;							 /*要发送的数据头指针*/ 
}t_AGENTUA;

typedef struct 
{
    
	t_Sj3_Public_Head		Ph;						// 公用包头
	t_Sj3_Oper_Head			OpHead ;                // 公共业务头
	XU8						usBuf[2000];            // 最大消息为 2000
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
    e_UpdateLocation_OperType = 2,//注册流程 
    e_QryNetWorkId_OperType = 3,/*向DNS查询uid归属网络*/
    e_Delete_OperType = 4,  //删除老拜访地 该消息由UDC得代理agent发起 推送至老的拜访地HLR
    e_UpdateHome_OperType = 5,//更新归属hlr的networkid  该消息由UDC得代理agent发起 推送至该用户的归属网络 HLR
    e_BusinessChange_OperType = 6,/*业务变更消息*/

    e_Subscribe_OperType = 7,
    e_unSubscribe_OperType = 8,
    e_Smc_Report_SM_Deliviry_Status = 9,/*该操作用户更新smc report消息更新status字段*/
    e_Smc_Alert_Service_Centre = 10,/*该操作用于alert消息转至归属地发送 smc*/
    e_Sag_AuthInfoUpdate = 11,/*登陆更新*/
    e_Sag_AuthInfoDelete = 12,/*删除老登陆地内存*/



    e_Sag_PullQryUidbyTel = 13,/*上UDC查询用户uid*/

    e_auth_SynUtBindInfo = 14,/*该操作用于同步租赁绑定关系表*/
    e_Auth_ModifyPidInfo = 15,
    
    e_Auth_UtLeaseSyn=16,
	e_Notify_Insert_Req=17,//通知HLR下发insert请求
	e_Smc_AlertRsp_Status,
    e_ReadyforSMAlertMsg,/*readyforsm 消息要求转到归属的alert*/
    e_QryuidVisitNetId = 20,
    e_HandShake,
    
    BUTT_OperType

}E_OperType;


/************************************************************************
                         哈希静态信息
/************************************************************************/



typedef struct
{
    TRoamFlag roamFlag;
    TDHCPReletFlag dhcpReletFlag;
    TPeriodRegTime periodRegTime;
    TVLANTag vlanTag;
    XS8 mem_ipFlag;		/* IP是否dhcp标记 */
    TAPCCount maxAPCCount;
    TVoiceMask voiceMask;
    XU8 subClass;
    XS8 mtFlag;			/* 带宽维持标记 */
    XS16 maxULRate;
    XS16 minULRate;
    XS16 maxDLRate;
    XS16 minDLRate;    
    TAPCCount curBWAddrCount;
    XS8 soft_flag;		/* softphone标记: softphone: 7, 其他: 0 */
}TBWBasicInfo;

typedef struct
{
	XU8 nationCode[LENGTH_OF_NATIONCODE];  //国家码
	XU8 telno[LENGTH_OF_TEL];        //不带国家码的电话号码
	XU8 totalTelno[LENGTH_OF_TEL];   //带国家码的电话号码
	XU16 telnoType;                        //号码类型0：主号码 1：子号码
}SSubTelnoInfo;

typedef struct
{
	XU8 mem_dns[SIZE_OF_IP_ADDR];
	XU8 mem_ip[SIZE_OF_IP_ADDR];
	XU8 mem_submask[SIZE_OF_IP_ADDR];
	XU8 mem_getway[SIZE_OF_IP_ADDR];
	XU8 mem_rsv[4];	
	//说明 mem_rsv[4]分别为:Rsv[1] CIDFlag[1]     DENFlag[1]     Rsv[1]     
	/*   CIDFlag  按bit解析，
		bit0表示系统功能（0：不支持黑白名单机制；1：支持黑白名单机制）；
		bit1表示黑白名单更新标识（0：不更新；1：更新）。
		bit2区分多次更新操作，0和1交替发送，此bit2终端不解析。
				（bit2的作用主要是为了让SAG来区分多次更新操作，因为SAG比较BWInfo不同才会下方BWInfo给终端。
		Bit3：切换优化功能指示，1为支持切换优化；0：不支持切换优化
		Bit6Bit5Bit4：动态保持带宽门限，000为10%，001为20%，010为30%，011为40%，100为50%，101为60%，110为70%，111为80%
		其他bit预留。）
	*/
	/*	DENFlag     按bit解析，
		Bit0为加密功能启用标识(0：不启用加密；1：启用加密)；
		bit3bit2bit1表示加密强度(000:64bit;001:128bit;010:256bit)；
		bit4是否密钥遥毁标识(1：遥毁；0：不遥毁)
	*/
	//   add userPriorFlag 2013.11.19
	/*   最后一个Rsv[1] 用来存用户优先级相应信息*/
	/*   Bit0~2：用户优先级，取值范围0~7
		Bit3~4：
		0：高优先级+数据业务不可压缩
		1：高优先级+数据业务可压缩
		2：低优先级
	*/
	//	其他bit预留 	
}TBWMem;

typedef struct  /* 维持带宽 */
{
	XU16 ul_maintain; /*上行最小*/
	XU16 dl_maintain; /*下行最小*/
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
	TBWMainTain maintain;	/*维持带宽*/
	TBWMem		meminfo;	/* Mem模块 */
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
	XU32 camTalkFlag   :1; //camTalk业务开关
	XU32 reserved      :31; //预留
}TXdbHdbSubOperation;

typedef struct
{
	TXdbHdbSubOperation szSubOperation; //按位设置业务，第i位为第i+1业务码的开关或者第32*n+（i+1）的开关
	XU8 operProperty[MAX_OPER_CODE_COUNT_OF_ONE_STRUCT][MAX_OPER_PROPERTY_LENGTH]; 
	/*说明：MAX_OPER_CODE_COUNT_OF_ONE_STRUCT一个SSubHlrOperation支持的最大业务码个数，
	  设置为operation的最大有效位数，目前为了省内存设置为1. MAX_OPER_PROPERTY_LENGTH业务码属性，最大长度为32 */
}SSubHlrOperation;

//子号码列表
typedef struct
{
	XU8 subTelnoNum;//子电话号码个数
	SSubTelnoInfo subTelnoList[MAX_SUB_TELNO_NUM+1];//子电话号码列表
}SSubTelnoInfoList;

/************************************************************************
                         哈希静态信息
/************************************************************************/
typedef struct 
{
	XU8     pid       [LENGTH_OF_PID]           ;     /*PID*/
	XU8     sid       [LENGTH_OF_SID]           ;     /*SID*/
	XU8     tel       [LENGTH_OF_TEL]           ;     /*电话号码，支持32位长*/
	XU8     callerNum [LENGTH_OF_TEL]           ;     /*主叫显示号码，支持32位长*/
    XU8     odb		  [LENGTH_OF_ODB]           ;     /*运营商闭锁，按位表示*/
    XU8     hsType    [LENGTH_OF_HSTYPE]        ;     /*终端类型*/
	XU8     subState                            ;     /*用户状态*/
    XU8     authType                            ;     /*鉴权类型*/
    XU8     twins                               ;     /*马机处理方式*/
	XU8     prepay                              ;     /*预付费*/
	XU8     onecalltwo                          ;     /*一号双机*/
	XU8     voice     [LENGTH_OF_VOICE     ]    ;     /*语音业务*/
	XU8     sms       [LENGTH_OF_SMS       ]    ;     /*短信业务*/
	XU8     bearer    [LENGTH_OF_BEARER    ]    ;	   /*承载业务*/
	XU8     narrowBand[LENGTH_OF_NARROWBAND]    ;     /*窄带业务*/
	XU8     broadBand [LENGTH_OF_BROADBAND ]    ;     /*宽带业务*/
	XU8     roamTplID [LENGTH_OF_ROAM_TPL_ID]   ;     /*漫游模板ID*/
	XU8     ssInfo    [LENGTH_OF_SS        ]    ;     /*运营商提供的补充业务列表*/
	XU8     opendate  [LENGTH_OF_DATE      ]    ;     /*开户日期*/
	XU8     vhlr      [LENGTH_OF_VHLR      ]    ;
    XU8     category  [LENGTH_OF_SUb_CAT   ]    ;     /*主叫用户类别*/             
    XU16    serverflag;							/* add by dufei 07.04*/
	XU8    userAuthType;						/*终端鉴权*/
	XU8    userUtacperiod[2];
	XU8	    userBm[50];
	XU8    chargeNum       [LENGTH_OF_TEL];
	XU16   chargeType;	
	XU8    chargeFlag;
	XU8    userPriority;
	XU16  dbsRight;							 /*数据组播权限 add by lhy 2010.03.30*/
	XU16  unSuccFlag; //业务成功标识 add by lhy 20111.05.07
	XU8		userRecordAttri;
    TXdbScuBWData     bwData;
	XU8 pidPort;
	XU8 userTag;
	XU8 netSelectTmpId;//网络选择模板 ID add by lx 2012.05.15
	XU8 changeFlag;//网络选择模板是否 变化标示 add by lx 2012.05.15
	XU8  StunKillReviveStatus;//遥晕遥毙复活状态  add by cyl 2012.09.10
	XU8  stunKillReviveChangeFlag;//摇晕摇毙复活变化标示add by lx 2012.11.19
	XU32 timeStamp;
	XU8 DENFlag; //加密标识   add by wj 2013.05.14 
				 /*Bit 7:预留 Bit 6：预留 Bit 5：预留 
				   Bit 4：密钥遥毁标识(1：遥毁；0：不遥毁) 
				   Bit3 Bit2 Bit1：表示加密强度(000:64bit;001:128bit;010:256bit) 
				   Bit0 ：加密功能启用标识(0：不启用加密；1：启用加密)*/
	XU8 DENFinv[LENGTH_OF_KEYUPDATEPERIOD]; //加密周期 add by wj 2013.05.14 
	XU8 UserShortCodes[LENGTH_OF_SHORT_CODES];//用户短号号码add 2013.08.14
	XU16 UserCommRange;//用户通信权限
	XU16 DcsUserOrganizeID	; //组织ID
	XU8 preferLanguageInd;//提示音语言类型指示add by wj 2013.08.30
	XU16 usSrvAgrmntId; // 终端业务描述信息(宽带协约模板ID)add 2013.11.19
	SSubHlrOperation szSubHLROperation[MAX_OPERATION_COUNT]; //最大SSubHlrOperation个数，MAX_OPERATION_COUNT目前等于1，存储业务码为1-32的业务信息，每业务码增加32个, MAX_OPERATION_COUNT宏定义要加1
	SSubTelnoInfoList subTelnoInfoList;     /*子号码列表*/
    XU8 liId[LIID_LEN];
	XU8 liMode;
	XU8 cCid[LENGTH_OF_CID]; //增加CID运营商模板号
	XU8 LocalNetwork[LENGTH_OF_NETWORK_ID];
}TXdbHashStat;



/************************************************************************
                         哈希动态信息
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


/* 动态数据表结构 */

typedef struct 
{
	XU8    uid[LENGTH_OF_UID];
	XU8     bsID     [LENGTH_OF_BSID] ;
    XU8     bscID    [LENGTH_OF_BSCID];
    XU8     laiID    [LENGTH_OF_LAIID];
	XU8     vssID    [LENGTH_OF_VSSID]; //语音锚SXC
	XU8     regDate [LENGTH_OF_DATE];    /*当前注册时间*/
} TXdbScuVoiceAnchor;//add 09.21

#ifndef MillTime64
	#define MillTime64 unsigned long long
#endif

//终端状态的各个字段的更新时间//2013.05.17
typedef struct  
{
	MillTime64  AnchorTime;		//bsID,bscID,laiID,vssID更新时间
	MillTime64  PidTime;			//pid,pidPort更新时间
	MillTime64  voiceAnchorTime;	//voiceAnchor更新时间
	MillTime64  PurgedTime;       //HsStatus：purge更新时间    
	MillTime64  RoamRestrictTime; //HsStatus：RoamRestrict更新时间
	MillTime64  MNRFTime;         //HsStatus：MNRF更新时间
	MillTime64  MCEFTime;         //HsStatus：MCEF更新时间
	MillTime64  twinTime;         //HsStatus：twin更新时间
}SHashDynUptTime;

/************************************************************************
                         哈希动态信息
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
	//XU8     smc      [LENGTH_OF_SMCLIST];  /*增加SMC列表*/
	/* add dufei */
	XU8   currRegDate [LENGTH_OF_DATE];    /*当前注册时间*/
	XU8	  prevRegDate[LENGTH_OF_DATE]; /*上次注册时间*/
	XU8	    hsType;
	TUTInfo utInfo;	// 05.29
	TXdbScuVoiceAnchor voiceAnchor; //新增语音锚信息 add 09.21
	XU8 pidPort; //2011.12.15 mz add
	XU32 sagIpv4;//2013.03.18 add sagip
	SHashDynUptTime hashDynUptTime;//hash动态数据的更新时间 2013.05.17
	SHlrCamtalkDynInfo hashCamtalkDyn;//记录该UID对应的camtalk登陆信息
	XU8  cNetworkID[LENGTH_OF_NETWORK_ID];//增加网络号ID的记录
	XU8  authNet[LENGTH_OF_NETWORK_ID];//登陆地网络
	XU32 sagIp_voice;
    XU32 sagIp_data;
    XU8 updateFlg;
}TXdbHashDyn;

/************************************************************************
                         哈希补充信息
/************************************************************************/



/*仅存储需要额外状态或号码的补充业务*/

/*无条件呼叫前转*/
typedef struct
{
	XU8    state;
	XU8    dn    [LENGTH_OF_SS_DN    ];
	XU8    option[LENGTH_OF_SS_OPTION];
}TXdbHashSsCfu;
/*遇忙呼叫前转*/
typedef TXdbHashSsCfu TXdbHashSsCfb;

/*无应答呼叫前转*/
typedef TXdbHashSsCfu TXdbHashSsCfnry;

/*不可及呼叫前转*/
typedef TXdbHashSsCfu TXdbHashSsCfnrc;

/*隐含呼叫前转*/
typedef TXdbHashSsCfu TXdbHashSsCf;

/*被叫号码显示*/
typedef TXdbHashSsCfu TXdbHashSsColp;
/*热话*/
typedef struct 
{
	XU8    state;
	XU8    hotDN[LENGTH_OF_SS_HOTLINE];
}TXdbHashSsHotLine; 

/*即时热话*/
typedef struct
{
	XU8    state;
	XU8    hotDN[LENGTH_OF_SS_HOTLINE];

}TXdbHashSsHotLineCS;
/*超时热话*/
 typedef struct
 {
     XU8    state;
     XU8    hotDN[LENGTH_OF_SS_HOTLINE];
 
 }TXdbHashSsHotLineJS;

 /*密码呼叫*/
typedef struct
{
	XU8    state;
	XU8    callPwd[LENGTH_OF_PWD_CALL]; 
}TXdbHashSsCallPswd;

/*闭锁密码*/
typedef struct
{
	XU8     state;
	XU8     odbPswd[LENGTH_OF_PWD_ODB];  

}TXdbHashSsOdbPswd;

/*缩位拨号*/
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

/*群组用户*/
typedef struct
{
	XU8     state;
    XU8     msisdn[LENGTH_OF_TEL];
    XU8     prn[LENGTH_OF_CTX_PRN];
    XU32    ctxID;
    XU32    groupID;
    XU32    popedom_of_crsgp;  /*同组代答权限*/
    XU32    popedom_of_scr;    /*指定代答权限*/
	XU8     priv[LENGTH_OF_PRIV];   /*出呼权限*/

}TXdbHashSsCtrx;


/*一呼双响*/
typedef struct
{
    
    XU8 state;  /*用户状态，表示签约*/
    XU8 dn[LENGTH_OF_TEL];
    
}TXdbHashSsOneForTwo;


/*免打扰*/
typedef struct
{
    XU8 state;
    
}TXdbHashSsNotDisturb;

typedef TXdbHashSsNotDisturb TXdbHashSsCw;

/*任我行*/
typedef struct
{
    XU8 account[LENGTH_OF_TEL];
    XU8 pwd[LENGTH_OF_SS_PWD_FFC];
        
}TXdbHashFFC;

typedef struct
{
    /*无条件前转*/
	TXdbHashSsCfu      tCfu;
	
    /*遇忙前转*/
    TXdbHashSsCfb      tCfb;
    
	/*无应答前转*/
    TXdbHashSsCfnry    tCfnry;
	
	/*不可及前转*/
    TXdbHashSsCfnrc    tCfnrc;

    /*热话*/
    TXdbHashSsHotLine  tHotline; /*scu更改后，删除*/

    /*超时热话*/
    TXdbHashSsHotLineCS tHotlineCS;

    /*即时热话*/
    TXdbHashSsHotLineJS tHotlineJS;

    /*密码呼叫*/
    TXdbHashSsCallPswd tCallPswd;

    /*缩位拨号*/
    TXdbHashSsShrinkDN tShrinkDN;//[COUNT_OF_SHRINKDN];

    /*呼出限制*/
    TXdbHashSsOutPriv tOutPriv;

    /*指定目的码限制*/
    TXdbHashSsAimLimt tAimLimit;

    /*指定目的码续接*/
    TXdbHashSsAimConn tAimConn;

    /*免打扰*/
    TXdbHashSsNotDisturb tNotDisturb;

    /*呼叫等待*/
    TXdbHashSsCw tCw;

    /*任我行密码*/
    TXdbHashFFC  tFFC;
    
	/*群组用户*/
    TXdbHashSsCtrx     tCtrx;


    TXdbHashSsOneForTwo tOneForTwo; /*一呼双响*/    
    

}TXdbHashSS;



/******************Dyn Information*************************/

typedef struct
{
	XU8   uid       [LENGTH_OF_UID]           ;     /*UID*/
	XU8   hsType;				//终端类型
	XU8   hsStatus;                        /*终端状态     */
	XU8   hsVersion;                        /*终端版本     */
	XU8   bsId        [LENGTH_OF_BSID];    /*基站ID       */
	XU8   bscId   [LENGTH_OF_BSCID];                        /*基站控制器ID */
	XU8   laiId       [LENGTH_OF_LAIID];   /*位置区ID     */
	XU8   vssId       [LENGTH_OF_VSSID];   /*软交换ID     */
	XU8   prevRegDate [LENGTH_OF_DATE];    /*上次注册时间 */
	XU8   currRegDate [LENGTH_OF_DATE];    /*当前注册时间*/
	XU32	xwipv4;
	XU16	xwipport;
	XU8   pid       [LENGTH_OF_PID]           ; 
	TUTInfo	utInfo;	
	TXdbScuVoiceAnchor voiceAnchor; //新增语音锚信息 add 09.21
	XU8 pidPort; 	//2011.12.15mz增加
	XU32 sagIpv4;/*记录SAGIP add 2013.03.18*/
	SHashDynUptTime hashDynUptTime;//动态数据信息更新时间2013.05.17
	SHlrCamtalkDynInfo camtalkDynInfo;//camtalk登陆信息2015.04.15
	XU8  cNetworkID[LENGTH_OF_NETWORK_ID];//增加网络号ID的记录
	XU8  authNet[LENGTH_OF_NETWORK_ID];//增加网络号ID的记录
	XU32 sagIp_voice;
    XU32 sagIp_data;
    XU8 updateFlg;
}TXdbHdbDyn;

/*一张网更新的数据*/
typedef struct
{
    TXdbHdbDyn DynInfo;
    XU8  HomeNetID[LENGTH_OF_NETWORK_ID];
    XU8  OldNetID[LENGTH_OF_NETWORK_ID];/*老拜访地网络号*/
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
