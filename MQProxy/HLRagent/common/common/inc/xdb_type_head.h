#ifndef _XDB_TYPE_HEAD_H
#define _XDB_TYPE_HEAD_H


#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/****** 旧平台使用,现在去掉*************
** #define _UCHAR              unsigned char
** #define _USHORT             unsigned short
** #define _ULONG              unsigned long
** #define _UINT               unsigned int
**************************************************/


#define NUM_OF_VECTORS          3

// add by dufei

// MAP 接口类的所有字段宏定义
#define LENGTH_OF_RAND				 16
#define LENGTH_OF_SRES				 4
#define LENGTH_OF_KS				 12 
#define LENGTH_OF_UID				 4
#define LENGTH_OF_DN                 16
#define LENGTH_OF_VSSID              4
#define LENGTH_OF_SCADDR             4
#define LENGTH_OF_EBSCNUM            4
#define LENGTH_OF_BTSNUM             2
#define LENGTH_OF_UTTYPE             2
#define LENGTH_OF_HLRID              4
#define LENGTH_OF_CNTRX_ID           2
#define LENGTH_OF_CGROUP_NUM         4
#define LENGTH_OF_CGROUP_PREFIX      3
#define LENGTH_OF_CSUB_SHORTNUM      3
#define LENGTH_OF_CALLPASSWD         8
#define LENGTH_OF_HOTLINENUM         16
#define LENGTH_OF_SHRINKNUM          1
#define LENGTH_OF_REALNUM            16
#define LENGTH_OF_CID                3
#define LIID_LEN   25


// BOSS 接口类所有字段长度定义
#define LENGTH_OF_CSSID          4 //定义css网元的设备ID
#define LENGTH_OF_UID            4     
#define LENGTH_OF_PID            4
#define LENGTH_OF_SID            16
#define LENGTH_OF_TEL            16   
#define LENGTH_OF_ODB            2
#define LENGTH_OF_VOICE          2
#define LENGTH_OF_SMS            2
#define LENGTH_OF_BEARER         2
#define LENGTH_OF_NARROWBAND     2
#define LENGTH_OF_BROADBAND      2
#define LENGTH_OF_ROAM_ID        4  /*漫游ID长度 漫游模板＋虚拟hlr*/
#define LENGTH_OF_ROAM_TPL_ID    1  /*漫游模板ID长度*/
#define LENGTH_OF_ROAM_TPL_NAME  32 /*漫游模板名长度*/
#define LENGTH_OF_ROAM_NAME      32
#define LENGTH_OF_ROAM_AREALIST  256
#define LENGTH_OF_SUB_TPL_ID     2  /*用户模板ID长度*/
#define LENGTH_OF_SUB_TPL_NAME   32 /*用户模板名长度*/
#define LENGTH_OF_VHLR           2  /*虚拟HLR长度*/
#define LENGTH_OF_HSTYPE         1
#define LENGTH_OF_DATE           14
#define LENGTH_OF_SS             6
#define LENGTH_OF_SUb_CAT        1 /*用户主叫类别*/
#define LENGTH_OF_AUTHTYPE       1
#define LENGTH_OF_TWINS          1
#define LENGTH_OF_PREPAY         1
#define LENGTH_OF_ONECALLTWO     1
#define LENGTH_OF_SUBSTATE       1
#define LENGTH_OF_SHRINKDN       1 
#define LENGTH_OF_REALDN         16
#define LENGTH_OF_CATEGORY       1
#define LENGTH_OF_ESMCID         4
#define LENGTH_OF_USERBM		 50
	
	/*长度定义：动态信息*/
#define LENGTH_OF_HSSTATUS       1
#define LENGTH_OF_HSVERSION       1
#define LENGTH_OF_BSID           2
#define LENGTH_OF_BSCID          4
#define LENGTH_OF_LAIID          5
#define LENGTH_OF_VSSID          4
#define LENGTH_OF_SMCLIST        80
	
	
	/*长度定义：补充业务信息*/
#define LENGTH_OF_SS_CODE        1
#define LENGTH_OF_SS_STATE       1
#define LENGTH_OF_SS_PWD         8
#define LENGTH_OF_SS_PWD_OUTPRIV 4
#define LENGTH_OF_SS_PWD_FFC     6
#define LENGTH_OF_SS_PWD_CALL    8
	
	/*by wuhuan mod*/
	//前转号码长度
#define LENGTH_OF_SS_DN          16
#define LENGTH_OF_SS_OPTION      2


#define LENGTH_OF_SS_CTRX_EXT    4
#define LENGTH_OF_SS_CTRX_PRIV   2
#define LENGTH_OF_GID            4
#define LENGTH_OF_GNAME          20
#define LENGTH_OF_EXT            3
#define LENGTH_OF_PRIV           1

#define PID_LEN						4					//PID长度

/*by wuhuan mod*/
#define LENGTH_OF_SS_HOTLINE     16
#define LENGTH_OF_PWD_CALL       8
#define LENGTH_OF_PWD_ODB        4


#define  LENGTH_OF_CTX_NAME             32        // ctx 业务类
#define  LENGTH_OF_CTX_GROUP_NAME       32
#define  LENGTH_OF_CTX_OPERATOR         8
#define  LENGTH_OF_CTX_OPERATOR_PWD     8
#define  LENGTH_OF_CTX_PRN              3
#define  LENGTH_OF_CTX_OPERATOR_POPEDOM 1
#define  LENGTH_OF_CTX_CALL_OPT_PREF    1
#define  LENGTH_OF_CTX_CTX_PREF         1
#define  LENGTH_OF_CTX_POPEDOM          1
#define  LENGTH_OF_CTX_GTN              9
#define  LENGTH_OF_CTX_INFO             32
#define  LENGTH_OF_CTX_WAC_INFO         32
#define  LENGTH_OF_CTX_GROUP_INFO       32

#define  LENGTH_OF_CLAN_DEVID		4
#define  LENGTH_OF_CLAN_GRPID		2

#define YES                     1
#define NO                      0
#define MAX_CF_NUM              4
#define MAX_NOREPLY_TIME        30
#define FWD_NUM_LEN             16 
#define FWD_OPT_LEN              2
#define MAX_SS_NUM              100
#define MAX_SEND_BUF            800

#define MAX_NUM_OF_FWD          1
#define SERV_CODE_LEN           1    /* 业务码长茺 */

//用户业务信息相关长度定义
/*若此处宏定义做修改，注意需同时修改
  XS32 XdbQrySubHlrOperation(XU32 dbTaskId, XU8 *uid, XU16 operCode, SSubOneHlrOperationList * pSubOneHlrOperationList)
  及XS32 XdbCfgSubHlrOperation(XU32 dbTaskId, SSubOneHlrOperation *pSubOneHlrOperation)
  中相关定义*/
#define MAX_OPER_PROPERTY_LENGTH                       32 //业务码属性，最大长度为32
#define MAX_OPER_CODE_COUNT_OF_ONE_STRUCT              32 //一个SSubHlrOperation支持的最大业务码个数，设置为operation的最大有效位数
#define MAX_OPERATION_COUNT                            1 ///最大SSubHlrOperation个数，MAX_OPERATION_COUNT目前等于1，存储业务码为1-32的业务信息，每业务码增加32个MAX_OPERATION_COUNT宏定义要加1
#define MAX_OPER_CODE_COUNT MAX_OPERATION_COUNT*MAX_OPER_CODE_COUNT_OF_ONE_STRUCT


// dufei
#define LENGTH_OF_T 2
#define	LENGTH_OF_TWINFLAG			1	 // 孖机处理方式
#define LENGTH_OF_GATEGORY			1
#define LENGTH_OF_PREPAYIND			1
#define	LENGTH_OF_STATUS			1
#define	LENGTH_OF_ONECALLTWO        1
#define LENGTH_OF_INTERVAL			2
#define LENGTH_OF_DHCP_RENEW_STA	1
#define LENGTH_OF_VLAN_TAG			2
#define LENGTH_OF_FIXED_IP		    4
#define LENGTH_OF_MOBILITY			1
#define LENGTH_OF_MAX_IP_NUM		1
#define LENGTH_OF_SD_INDEX			2
#define LENGTH_OF_LOGSTATUS			1
#define LENGTH_OF_FIXED_IP_NUM      4
#define LENGTH_OF_MACADDR			6
#define LENGTH_OF_ANCHORBTSID		4
#define BLANK_LONG          0xFFFFFFFF

#define LENGTH_OF_GROUP_TELNO		16
#define LENGTH_OF_EXTERNAL_TELNO       16
#define LENGTH_OF_FIX_AREA_PROPERTY		1
#define LENGTH_OF_EXTERNAL_TELNO       16

#define LENGTH_OF_QUERY_COUNT 4
#define LENGTH_OF_PAGESIZE 2
#define LENGTH_OF_PAGEINDEX 2
#define LENGTH_OF_BTSNUM 2

#define	LENGTH_OF_BTSID4			4

#define LENGTH_OF_GROUP_DESCRIPTION  32 
typedef enum
{
	//销户
	e_UssClose,
	//开户
	e_UssOpen,
	//烧录未开户
	e_UssNotOpen
}E_USERSTATE_STATIC;



//针对到SJ接口中可能出现的错误
typedef enum
{
	e_ErrorBegin=0,							//开始
		e_NotError=0,							//成功(无错误)
		
		e_DbResZeroError,						//删除，修改，查询影响为0
		e_UnknownError,							//异常,未知错误
		e_OpTypeError,							//操作类型错误
		e_OpObjError,							//操作对象错误
		e_PackLenError,							//包长错误
		e_ParamError,							//参数错误
		e_ModUserStateError,					//修改用户状态失败
		
		e_NotLogin,								//未登陆
		e_RepeatLogin,							//重复登陆
		e_ViHlrNotExist,						//该虚拟HLR不存在
		e_OpUserNotExist,						//该操作员已不存在
		e_OpUserPasswdError,					//登陆密码错误
		e_LogoutError,							//注销失败
		
		e_DbUnknownError,						//对库操作失败
		e_FileNameError,						//文件名出错
		e_UidExist,								//该UID已存在
		e_UidNotExist,							//该UID不存在
		e_DnExist,								//该电话已存在
		e_DnNotExist,							//该电话不存在
		
		e_UserExist,							//该用户已存在
		e_UserNotExist,							//该用户不存在
		
		e_OpUserOrViHlrNotMatch,				//操作员与虚拟HLR编号不匹配
		
		e_TemplateNotExist,						//模板ID不存在		
		e_TemplateExist,						//新增模板时,模板ID已存在
		e_CtxNotExist,							//群ID不存在
		e_CtxExist,								//群ID已存在
		e_CtxUserExist,							//群组用户已存在
		e_ShortNoExist,							//该缩位号码已存在
		
		e_NotVoIce,								//未签语音业务
		e_ResourceIsUse,						//资源在使用 当删除的时候
		
		e_ToScuSynError,						//到SCU数据同步失败
		e_UpdateStateError,						//修改用户状态失败
		
		e_RepeatOpenUser,						//重复开户
		e_RepeatDelUser,						//不能给未开户用户销户
		e_FileOpenError,						//文件打开失败
		e_FileReadError,						//文件读取失败
		e_FileInvalidFormat,					//无效的文件格式
		e_FileExceedRecLmttn,					//文件记录数超出上限
		e_FileInternalError,					//内部错误
		e_FileContinuous,						//未完
		
		e_MuNewUidError,						//换号操作，新UID状态不对
		e_MuUidError,							//换号操作，旧UID状态不对
		e_PackTypeError,						//公有包头类型出错
		e_SsNotP,								//该项操作需要先进行签约
		e_SsNotR,								//该项操作需要先进行注册
		e_SsNotA,								//该项操作需要先进行激活
		e_DelUserAttError,                      //删除用户业务相关数据出错
		e_NotOpenuser,							//用户未开户
		e_BatOpenUserError,						//批量开户失败
		
		e_GroupNotExist,						//组ID不存在
		e_GroupExist,							//组ID已存在
		e_CtxUserShortNoLenError,				//群组用户短号长度出错
		e_CtxGroupOverrun,						//群中的组个数超出
		
		e_GtnNotExist,							//GTN ID不存在
		e_GtnExist,								//GTN ID已存在
		
		e_GtnNumNotExist,						//总机号码不存在
		e_GtnNumExist,							//总机号码已存在
		
		e_ErrorEND								//结束
}E_SJ_ERROR_TYPE;


// sj3 操作对象
typedef enum
{
	e_OPObjBegin = 0x00,							//开始
	e_MEMObjID = 0x02,

	e_Version = 0x10 ,						            //版本对象



	e_VoSer = 0x50 ,						    //语音业务对象(对应语音业务设置)
	e_DaSer = 0x51 ,						    //数据业务对象(对应数据业务设置)

	e_UserTemplet = 0x50,						//用户模板
	e_RoamTemplet,								//漫游模板
	e_Rest,										//Rest
	e_Cl,										//CancelLocation
	e_TempletOpen,								//模板开户
	e_User,										//用户
	e_Batch=0x56,								//批量用户
	e_Mz = 0x56,								// cpez 与mz的关系上面的批量用户没有用到
	e_BatchCl,									//批量CancelLocation
	e_StopUser,									//停机
	e_VPNManager = 0x59,							//虚拟专网组织管理
	//e_Group,									//群组
	e_GroupUser,								//群组用户
	e_ShortNo=0x5b,									//缩位号码
	e_CallPasswd,								//呼叫密码
	e_LockPasswd,								//闭锁密码
	e_HotlineNum,								//热线电话
	
	e_SsState=e_HotlineNum+2,					//补充业务状态
	
	
	e_FileBatch,								//文件批量操作
	e_SsPasswd,									//补充业务密码
	e_DestCode,									//目的码
	e_Ocb,										//呼出限制
	e_Wac,										//广域群
	e_Centrex,									//群
	e_Gtn,										//总机
	e_Opt,										//话务员
	e_GroupEx,									//组
	e_CentrexUser,								//群用户
	e_Octr,										//一呼双响

	e_Stbc = 0x6c,						        		//烧录信息



    e_ChangeUid = 0x6d ,                 // 	


    e_Sj3UserGropTep  ,                 // 	组模板 T 

	e_filebatchStbc        = 0x70,      // 
	e_UserOpenTemplet      = 0x71,						//用户模板
	e_FileUserOpenTemplet  = 0x72,						//模板	

    // HLR License 信息
    e_Hlr_LicInfo = 0x80,
    e_DbsSer = 0x81 ,						    //数据组播业务对象add by lhy 2010.03.30

	e_Cid_Tpl = 0x83,					// CID模板信息
	e_Roam_Tpl = 0x84,					// 漫游模板信息
	e_Argmnt_Tpl = 0x85,				// 宽带数据协约模板

	e_OPObjBtsInfo = 0x87,					// 基站数据操作
	e_SynHlr = 0x8a,					// 同步hlr

	e_Addr_Communication = 0x8b,			// 通讯录 透传消息
	e_Ip = 0x8c,						//配置人员IP请求		0x8c
	e_Acl = 0x8d,						// _配置ACL请求
	e_ChangePwd = 0x8e,				//用户密码重置请求//add by lx 2013.01.25
	//e_AnchorMod = 0x91,

	// 集群操作对象
	e_Clan_Oper = 0xa0  ,    // 集群操作员业务对象
	e_Clan_Login   ,          // 登录对象    
	e_Clan_Org     ,          // 组织业务对象      
	e_Clan_User    ,          // 调度用户业务对象     
	e_Clan_Group   ,          // 组业务对象       
	e_Clan_OperGrp    ,       // 操作员的组        
	e_Clan_OperUser   ,       // 操作员的调度用户 
	e_Clan_GroupUser  ,       // 组调度用户 
	e_Clan_WatchTask  ,       // 监视任务 
	e_Clan_Shakehand  ,       // 握手
	e_Clan_Vsersion = 0xaa,			// 协议版本管理对象
	e_Clan_FixArea =0xab,			// 固定区域 
	e_Clan_GroupOstdUser =0xac,		// 组外网用户 
	e_Clan_BSList =0xad,			// 基站列表 
	e_Clan_GroupUserList =0xae,		// 组用户列表查询
	e_Clan_OSSTransmit =0xaf,		// OSS透传消息


    e_OPObjEnd,							//结束


}E_SJ_OPOBJ_ID;



// sj3 操作类型
typedef enum
{
	e_OPTypeBegin = -1 ,					

	e_OPTypeAdd ,							//增	
	e_OPTypeDel ,							//删	
	e_OPTypeMod ,							//修	
	e_OPTypeQue ,							//查	
	e_OPTypeListQue ,						//集中查询	
	e_OPTypeOther,							//11.1.5
	e_OPTypeModAnchorBts,					//修改锚基站
	e_OPTypeQryEMS,							//查询EMS的ip和port
	//e_OPType6,	               				// 目前只有boss使用
	//e_OPTypeConfigUserState=8,			//设置用户停机状态//add 2013.03.26
	//e_OPTypeCreditControl,				//信控请求//add 2013.03.26
	//e_OPTypeEncryptParamSet = 0x0A,         //加密参数设置请求
	//e_OPTypeEncryptParamQue = 0x0B,         //加密参数查询
 	//e_OPTypeConfigLeaseBind=12,         //设置出租未激活状态请求/取消租赁绑定关系add 2013.07.10
	//e_OPTypeQryLeaseBind=13,           //查询终端租赁绑定关系
	//e_OPTypeLeaseActivate=14,           //设置终端出租激活状态请求 2013.07.26
	//e_OPTypeCancelCreditStatus = 15,/*取消信控状态*/
	//e_OPTypeQueryCreditStatus = 16,	/*查询信控状态*/
	//e_OPTypeQueryUtInfo= 17,		/*查询终端硬件类型*/
	//e_OPTypeDelUser=0x12,	//剔除用户信息
	//e_OPTypeLoadUserInfo=0x14,//装载用户信息到内存请求
	//e_OPTypeMdyTelno=0x15,//修改电话号码请求
	e_OPTypeSynOptBts =0x20, 				// 优选基站同步
	e_OPTypeSynArgmntTpl = 0x21,			//宽带协约模板更新

	e_OPTypeEnd 		
}E_SJ_OPTYPE_ID;


typedef enum
{
	e_OPTypeUserBegin = -1 ,					

	//e_OPTypeAdd ,							//增	
	//e_OPTypeDel ,							//删	
	//e_OPTypeMod ,							//修	
	//e_OPTypeQue ,							//查	
	//e_OPTypeListQue ,						//集中查询	
	//e_OPTypeOther,							//11.1.5
	e_OPType6 =6,	               				// 目前只有boss使用
	e_OPTypeConfigUserState=8,			//设置用户停机状态//add 2013.03.26
	e_OPTypeCreditControl,				//信控请求//add 2013.03.26
	e_OPTypeEncryptParamSet = 0x0A,         //加密参数设置请求
	e_OPTypeEncryptParamQue = 0x0B,         //加密参数查询
	e_OPTypeConfigLeaseBind=12,         //设置出租未激活状态请求/取消租赁绑定关系add 2013.07.10
	e_OPTypeQryLeaseBind=13,           //查询终端租赁绑定关系
	e_OPTypeLeaseActivate=14,           //设置终端出租激活状态请求 2013.07.26
	e_OPTypeCancelCreditStatus = 15,/*取消信控状态*/
	e_OPTypeQueryCreditStatus = 16,	/*查询信控状态*/
	e_OPTypeQueryUtInfo= 17,		/*查询终端硬件类型*/
	e_OPTypeDelUser=0x12,	//剔除用户信息
	e_OPTypeBatchAddDelDCSGroupUser = 0x13, //批量增加删除组的调度用户//add 2013.09.23
	e_OPTypeLoadUserInfo=0x14,//装载用户信息到内存请求
	e_OPTypeMdyTelno=0x15,//修改电话号码请求
	e_OPTypeHlrOperSupportSet=0x16, //设置用户业务支持   add 2013.12.21
	e_OPTypeHlrOperSupportQry=0x17, //查询用户业务支持   add 2013.12.21
	e_OPTypeDelUserByPid=0x18,//根据PID踢除用户
	e_OPTypeQryUserInfoByUid=0x19,//根据UID查询所有信息请求
	e_OPTypeQryUserInfoByTelno=0x1A,//根据电话号码查询所有信息请求
	e_OPTypeLoadTelnoInfoToMem=0x1B,//绑定或解绑时发装载用户信息到内存请求
	e_OPTypeUnBindTelToUid=0x1C,	//解绑字号码请求
	e_OPTypeUserEnd 		
}E_SJ_USER_OPTYPE_ID;


// sj3 所有参数类型列表
typedef enum
{
	e_SjTagBegin = 0,

	e_SysVersion = 101 ,   // 版本

	e_AccountNum  = 1001		,	
	e_AcPeriod					, 
	e_ACSTATE					,
	e_Admin_Status				,
	e_ANCHOR_BTS_ID				,
	e_AUTHTYPE					,
	e_B_UT_TYPE					,
	e_BatType 					,							
	e_BEARTYPE					,
	e_BURNTIME					,							
	e_CALLOPTPREF				,							
	e_CALLPWD					,						
	e_CancelLocVal				,		
	e_CATEGORY					,			
	e_COLLECTION_INTERVAL		,	
	e_CREATEDATE				,			
	e_CRSGPPDOM					,			
	e_CTXACTFLAG				,			
	e_CTXID						,				
	e_CTXLIST					,			//	1020
	e_CTXNOTYPE					,			
	e_CTXPDOM					,				
	e_CTXPREF					,				
	e_CTXUSERLIST				,			
	e_DATATYPE					,			
	e_DESCRIPTION				,			
	e_DESTCODE					,			
	e_DCODELIST					,			
	e_DHCP_RENEW_STA			,		
	e_DoubleRingNum				,		
	e_FIELDNUM					,			
	e_FileName					,			
	e_FIXED_IP					,			
	e_FIXED_IP_NUM				,		
	e_Flag						,	//1035			
	e_FWDFLAG					,				
	e_FWDOPT					,				
	e_FWDTELNO					,			
	e_GROUPID					,				
	e_GROUPLIST					,		//	1040
	e_GROUPNAME					,			
	e_GROUPPFX					,			
	e_GTNID						,				
	e_GTNLIST					,				
	e_GTNNUM					,				
	e_HOTLINENUM				,			
	e_IDType					,				
	e_IndexType					,			
	e_KDCType					,				
	e_KVALUE					,				
	e_LASTREGDATE	            ,
	e_LOCKPWD	                , 
	e_MAC						,
	e_MAX_IP_NUM				,
	e_MAXCTXNUM					,
	e_MAXGROUPNUM				,
	e_MAXOPTNUM					,
	e_MAXUSERNUM				,
	e_MOBILITY					,
	e_ModifyType				,		// 1060
	e_ODB						,
	e_OPTID						,
	e_OPTLIST					,
	e_OPTPDOM					,
	e_OPTPLC					,
	e_OPTPWD					,
	e_OPTTYPE					,
	e_OUERRORLIST				,
	e_OUTCTXCLIP				,
	e_PERF_LOG_STATUS			,
	e_PID						,
	e_PIDSEG					,
	e_PREPAYIND					,
	e_PRN						,
	e_PRNLEN					,
	e_REALNO					,
	e_REGDATE					,
	e_RID						,
	e_SCRPDOM					,
	e_SD_INDEX					,		//	1080
	e_SERVINGBS					,//1081
	e_SERVINGBSC				,//1082	
	e_SERVINGLA	=1083			,//1083
	e_LAID		=1083			,
	e_SERVINGSS	=1084		,//1084
	e_SHORTNO	=1085			,
	e_SID						,
	e_SMTYPE					,
	e_SNOLIST					,
	e_SUMCOUNT					,
	e_SUPPLEMENTNO				,
	e_SUPPLEMENTTYPE			,	
	e_TEID						,
	e_TELNO						,
	e_TERMINALSTATE				,
	e_TERMINALTYPE				,
	e_TRANSNO					,
	e_TWINFLAG					,
	e_UID						,
	e_UIDSEG					,
	e_UserNo					,					// 1100
	e_VLAN_TAG					,
	e_VOICETYPE					,
	e_WACID						,
	e_WACPDOM					,					 // 1104

	e_TemplateID                ,
		
	 e_OperatorName =1106, 	//0x0452
	 e_Password=1107 ,			//0x0453
     e_IpGroup ,

	e_TERMINALDNTYPE,
	

	e_VoSerOrDataType ,   // 语音与数据标志位1110
	e_CallerNum ,		  // 号码变换1111

        e_ChargeFlag,               // HLR是否支持计费开关
        e_LicInfo,                      // HLR License信息
	  e_SS_INFO_EXT,           //补充业务列表2   1114
	  e_SQLString= 1115,
	  e_UserTag=1116, //用户属性
	  e_OutPriv = 1200 ,		// 呼出限制	CallOutLimit	1200
	 e_Cfb ,		    	// 遇忙前转	FwdBusy	
	 e_Cfu ,		    	// 无条件前转	Fwd	
	 e_Cfnrc ,		    	// 不可及前转	FwdNoTouch	
	 e_Cfnry ,		    	// 无响应前转	FwdNoResponse	
	 e_Cw ,			    	// 呼叫等待	WaitCalled	
	 e_NodDisturd ,			// 免打打扰	KeepDistrouble	
	 e_ShrinkDN ,			// 缩位拔号	ShortNumber	
	 e_HotLineCS ,			// 超时热线	TimeOutHotline	
	 e_HotLineJS ,			// 即时热线	InstanceHotline	
	 e_Ctrx ,				// 群组用户	GroupUser	

	 e_OneForTwo ,			// 一呼双响 1211
	 e_OneCallTwo ,		// 一号双机 1212

	 e_OneCallTell ,		// 一号双机从机号码 1213



     // 终端管理功能
     e_UserAuth   ,      // 开关   1214
  	 e_UserBm 		    , //  别名
  	 e_UserGroup 	    , // 组名
   	 e_UtAcperiod	    , // 周期时间 	    

   	 e_UserGroupTep	    , // 组模板名 	

	e_FileBatchstbcOrder 	  = 1221 ,	// 进度查询命令序号
	e_FileBatchstbcItemsCount = 1222 ,	// 批量烧录信息文件所包含需烧录的信息总条数
	e_FileBatchstbcCurrent = 1223 ,		// 当前的烧录进度	
	e_FileBatchstbcEndFlag = 1224,		// 烧录是否已完成，0表示未完成，1表示完成。

	e_LoginType = 1235,                 //LOGINTYPE  0x01:Wi128；0x02:MEM综合管理客户端。0x03：通讯录服务器//add by cyl 2012.09.22
	e_BTSIDList = 1250,					//MEM可登录或者不允许登录的基站列表 复合tag
	e_PermitInd = 1251,					//0:表示不允许接入的CID,1：表示允许接入的CID
	e_OperateInd = 1252,
	 e_UserDbsRight=1301	    , // 数据组播权限	

	e_QueryCount = 1302,
	e_BTSID4 = 1308,
	e_Tag_Bts = 1311,
	e_BTSNumOss=1312,
	e_PageIndex= 1313,
	e_PageSize= 1314,
	e_RAID= 1315,
	e_BtsName= 1316,

	e_HardwareType = 1317,
	e_SoftwareType=1318,
	e_ActiveSWVersion=1319,
	e_StandbySWVersion=1320,
	e_HardwareVersion=1321,
	e_UtNum=1325,

	e_UserMgmtFlag = 1329,				// 用户管理状态

	e_UL_Max_BW = 1330,
	e_UL_Min_BW	= 1331,
	e_DL_Max_BW	= 1332,
	e_DL_Min_BW	= 1333,
	e_If_Maintein = 1334,
	e_Min_UL_Maintein_BW = 1335,
	e_Min_DL_Maintein_BW = 1336,
	e_ServiceDescriptor	= 1337,
	e_ActiveFlag	= 1338,
	e_OdbSwitch	= 1339,
	e_DOWNLOADPATH =1340,
	e_PACKED_FILE_NAME =1341,
	e_PACKED_FILE_DATA =1344,
	e_TIME_STMP =1342,
	e_IP_ADDR =1343,
	e_PARAMFLAG =1440,

	e_mzInfo = 1441,

	e_CidList = 1253,//modify by lx 2012.05.15
	//网络模板选择模块处理
	e_NetSelectTmpId=1442,    //modify by lx 2012.05.15
	e_NetSelectTplName=1443,  
	e_NetSelectTmpList=1444,  
	e_ForbidCallFlag = 1445, //禁话标识

	e_ManagerStatus = 1446,//管理停机状态//add  2013.03.26
	e_VoiceStatus = 1447,//语音停机状态
	e_DataStatus = 1448,//数据停机状态
	e_BWStatus = 1449,//数据带宽业务状态

	e_DENInd = 1450, //加密功能启用标识
	e_KeyLen = 1451, //密钥长度
	e_KeyKillInd = 1452, //密钥遥毁标识
	e_KeyUpdatePeriod = 1453, //密钥更新周期
	
	e_BindUid = 1454,//绑定UID //add 2013.07.10
	e_LeaseHoldFlag,//租赁绑定关系设置or取消标识//0取消1确认
	e_LeaseHoldCtlList,//复合TAG 租赁绑定关系列表
	e_Tag_Manager_Name = 1457,
	e_Tag_Manager_Pwd	= 1458,
	e_Tag_Manager_Pwd_Old = 1459,
	e_Tag_Manager_Pwd_New = 1460,
	e_Tag_VPN_Manager = 1461,
	e_Tag_Ture_Result = 1462,
	e_Tag_OrgName = 1463,
	e_Tag_OrgID = 1464,
	e_Tag_OrgPrio = 1465,
	e_Tag_VPNOrg = 1466,
	e_Tag_BeginTelno = 1467,
	e_Tag_EndTelno =	1468,
	e_Tag_TelID = 1469,
	e_Tag_Orgtel = 1470,
	e_Tag_UserTelno =	1471,
	e_Tag_DCSUSER_UID = 1472,
	e_Tag_ShortNoLen = 1473,
	e_Tag_Begin_Gid =	1474,
	e_Tag_End_Gid = 1475,
	e_Tag_GrpID_ID = 1476,
	e_Tag_OrgGrp = 1477,
	e_Tag_GrpOrgPrio =	1478,
	e_CreditStatusTag = 1479,/*信控状态*/
	e_LastRegisterTimeTag = 1480,	/*最后登录时间*/
	e_UtInfoTag = 1481,/*UT信息*/
	e_PreferLanguageInd = 1482, /*提示音语言类型指示 add by wj 2013.08.30*/
	e_GroupNum = 1483, //组数
	e_GroupId = 1484, //组识别码
	e_GroupUserId = 1485, //组用户
	e_DeleteGroupUser = 1486, //删除组用户
	e_AddGroupUser = 1487, //增加组用户
	e_PttPriority = 1488,  //话权优先级
	
	e_OperationCode = 1489, /*业务码*/
	e_OperationFlag = 1490, /*是否开通业务信息*/
	e_OperationProperty = 1491, /*业务信息*/
	e_HlrOperation = 1492, /*复合TAG 用户业务*/

	e_NationalCode = 1493,/*国家码*/
	e_TelnoType = 1494,/*电话号码类型*/
	e_NewTelnoList = 1495,/*电话号码列表*/
	e_UnBindTelnoList = 1496,/*解绑电话号码列表*/

	// SAC 新增 TYPE
	 e_CidTmp = 2000,		// cid 模板号
	 e_PidPort ,			// pid 端号号
	 e_AgenSsIp,			// 代理软交换IP       AgenSsIp =               2002;
     e_AgenSsIpPort,		// 代理软交换端口     AgenSsIpPort =           2003;
	 e_RegSsIp,				// 注册软交换IP       RegSsIp =                2004;
	 e_RegSsIpPort,				// 注册软交换端口     RegSsIpPort =            2005;
	 e_WirelessRegCircleValue,		// 无线注册周期(值)   WirelessRegCircleValue = 2006;
	 e_WirelessRegCircleType,			// 无线注册周期(单位) WirelessRegCircleType =  2007;
	 e_SipRegCircleValue,				// SIP注册周期(值)    SipRegCircleValue =      2008;
	 e_SipRegCircleType,				// SIP注册周期(单位)  SipRegCircleType =       2009;
	 e_SipPassword,					    // SIP密码            SipPassword =            2010;	 

    // mem
     e_memDnsIp =  2011 , // DNS IP
  	 e_memIp 		    , // IP
  	 e_memSubMask 	    , // 子网掩码
   	 e_memGateway	    , // 网关 	 

	 e_StbcUserType = 2015 ,
	 e_memDhcpType ,    // mem dhcp type

	e_Cid,				// CID
	e_RoamTplName,		// 漫游模板名称
	e_SdName,			// 宽带数据协约模板名称
	e_WifiSpot=	2020,

	e_CentrexId,
	e_CentrexGrpNum =2022,

    // 集群用户
    e_ClanDispather            = 3000  ,   //   集群操作员              
    e_ClanDispather_UserName   = 3001  ,   //   操作员名称              
    e_ClanDispather_UserPwd    = 3002  ,   //   操作员密码              
    e_ClanDispather_UserRole   = 3003  ,   //   操作角色                
    e_ClanDispather_OrganizeID = 3004  ,   //   组织ID                  
    e_ClanDispather_UID        = 3005  ,   //   操作员UID               
    e_ClanDispather_UserTelNo  = 3006  ,   //   操作员电话号码          
    e_ClanDispather_SID        = 3007  ,   //   用户SID(密文)  	
    e_ClanDispather_UserPwd_Old= 3008  ,   //   操作员旧密码  
    e_ClanDispather_GroupID 		  ,    //   组ID    	
    e_ClanDispather_UserPriority      ,    //  user优先级     
	e_ClanDispather_LinkCallTellNo   ,
	e_ClanDispather_EnableFlag      ,
	e_ClanDispather_NightServiceTellNO   ,
	e_UserRecordAttri		= 3014,		// 签约录音业务
	e_groupRecordAttri		= 3015,		// 组签约录音业务

	
        
    e_ClanOrganize             = 3100  ,   //   组织                    
    e_ClanOrganize_OrganizeID  = 3101  ,   //   组织ID                  
    e_ClanOrganize_OrganizeName= 3102  ,   //   组织名称  

    e_ClanDcsUser              = 3200  ,   //   调度用户                
    e_ClanDcsUser_UID          = 3201  ,   //   调度用户UID             
    e_ClanDcsUser_UserTelNo    = 3202  ,   //   调度用户电话号码        
    e_ClanDcsUser_OrganizeID   = 3203  ,   //   调度用户UID             
    e_ClanDcsUser_UserDesc     = 3204  ,   //   调度用户名称  
	e_ClanDcsUser_UserPriority 		   ,
	e_ClanDcsUser_Attach_GID      	   ,
	e_ClanDcsUser_AuthCallTelNo		   ,	
	e_ClanDcsUser_IsDispather		   ,
	e_ClanDcsUser_UserProInGrp 		   , 
	e_ClanDcsUser_Ptt_Priority		   ,
	e_ClanDcsUser_DbsSwitch ,			   //  用户数据组播开关
	e_ClanDcsUser_Department=	3212,//用户单位
	e_ClanDcsUser_duty	=3213,  // 用户职务
	e_ClanDcsUser_ForbidStatus=3214,	 //用户禁话状态
	e_ClanDcsUser_ShortCodes=3215,//调度用户短号码//add 2013.08.14
	e_ClanDcsUser_CommRange = 3216,//调度用户通信权限

    e_ClanDcsGroup             = 3300  ,   //   组                      
    e_ClanDcsGroup_GroupID     = 3301  ,   //   组ID                    
    e_ClanDcsGroup_OrganizeID  = 3302  ,   //   组织ID                  
    e_ClanDcsGroup_GroupName   = 3303  ,   //   组名称                  
    e_ClanDcsGroup_VSSIDList   = 3304  ,   //   组呼区域范围            
    e_ClanDcsGroup_VoiceOpen   = 3305  ,   //   语音开关                
    e_ClanDcsGroup_MessageOpen = 3306  ,   //   短信开关                
    e_ClanDcsGroup_DataSerOpen = 3307  ,   //   宽带数据业务开关        
    e_ClanDcsGroup_Supplement  = 3308  ,   //   集群补充业务            
    e_ClanDcsGroup_GCallTimer  = 3309  ,   //   组呼定时器              
    e_ClanDcsGroup_Priority    = 3310  ,   //   组优先级                
    e_ClanDcsGroup_FreeTimer   = 3311  ,   //   组呼空闲定时器          
    e_ClanDcsGroup_LETimer     = 3312  ,   //   迟后进入周期字段        
    e_ClanDcsGroup_ParGrpID     	   ,   //   上级组ID
    e_ClanDcsGroup_ISTEMP     	   ,	   //   是否临时组
	e_ClanDcsGroup_GDescription,           //   组描述

	e_TaskID               = 3400  ,   //   任务ID                  
    e_Task_QueryType       = 3401  ,   //   查询类型                
    e_Task_Flag            = 3402  ,   //   标志(开始或结束)        
    e_Task_Status          = 3403  ,   //   用户状态                
    e_Task_Status_UID      = 3404  ,   //   用户UID                 
    e_Task_Status_GID      = 3405  ,   //   组ID                    
    e_Task_Status_value    = 3406  ,   //   状态值                  
    e_Task_Status_PeerTelNo= 3407  ,   //   对方通话号码 
	e_Task_Status_DigitNo  = 3408  ,   //   需要查询用户的电话号码//add by lx 2012.09.14
	e_TranceID             = 3500  ,   //   每个消息体前的TraceID   
	e_CcbIndex			   = 3501  ,   //   pull的ccbindex         
                                                                        
                                                                        
  
	e_ClanDevid                = 3502  ,    //  DEVID
	e_Clan_DbsRight = 3505,		// 数据组播权限
	e_Clan_Grp_DbsSwitch = 3506, // 组数据组播开关
	e_Clan_SDdh_Version = 3507, // Sddh接口版本
	e_FixAreaNumber=	3508,
	e_FixAreaProperty=	3509,
	e_GroupTelno			   = 3510,		//GroupTelno
	e_External_Telno		   = 3511,		//External_Telno
	e_FixAreaName,		//3512
	e_FixArea,//	3513	
	e_BTSNum=3514,
	e_BTSID=3515,
	e_SXCID	=3516,	
	e_DcsGroupPerTimer = 3517,
	e_BtsInfo=3518,
	
	
	
	
	/*计费*/	
	ChargePepayind 			    = 4000 ,    // 付费类型
	ChargeFlag               		   ,    //  
	ChargeType               	 	   ,    //  
	ChargeNum                          ,    // 

	e_ADDR_IP = 5001,
	e_ADDR_MASK,
	e_ADDR_GATEWAY,
	e_MAJOR_DNS,
	e_MINOR_DNS,


	// acl
	e_ACL_INFO=5006,	
	e_WHERECONTION	=5007,
	
	//用户密码修改
	e_NewPwd=5008,//新的密码
	e_SjTagEnd
}E_SJ_TAG;



//补充业务码
typedef enum
{
	e_SsBegin=0x00,				
	e_SsClip=0x11,				//主叫号码显示
	e_SsClir=0x12,				//主叫号码限制显示
	e_SsMct=0x15,				//恶意呼叫跟踪
	e_SsCfu=0x21,				//无条件前转
	e_SsCfb=0x29,				//遇忙前转
	e_SsCfnry=0x2a,				//无应答前转
	e_SsCfnrc=0x2b,				//不可及前转
	e_SsPEct=0x31,				//呼叫转移（拍叉前转）
	e_SsCw=0x41,				//呼叫等待
	e_SsCh=0x42,				//呼叫保持
	e_SsMpty=0x51,				//三方通话
	e_SsCliro=0xe1,				//主叫号码显示限制逾越
	e_SsHlto=0xe2,				//超时热线（二期）
	e_SsShrink=0xe3,			//缩位拨号（二期）
	e_SsOcb=0xe4,				//呼出限制
	e_SsDds=0xe5,				//免打扰业务（二期）
	e_SsMee=0xea,				//会议电话
	e_SsSar=0xeb,				//指定目的码限制（二期）
	e_SsSaa=0xec,				// 指定目的码接续（二期）
	e_SsCentrex=0xed,			//Centrex群组
	e_SsPasswdCall=0xee,		//密码呼叫（二期）
	e_SsIhl=0xef,				//即时热线（二期）
	e_SsAc=0xf1,				//闹铃业务（二期）
	e_SsFfc=0xf2,				//任我行（二期）
	e_SsSc=0xf3,				//灵通呼（短消息提醒）（二期）
	e_SsAoc=0xf4,				//话费立显（二期）
    e_SsCr=0xf5,                //彩铃
    e_SsOctr=0xf6,              //一呼双响
	e_SsEnd								
}E_SJ_SS;


#ifdef __cplusplus
}
#endif /* _ _cplusplus */
#endif

