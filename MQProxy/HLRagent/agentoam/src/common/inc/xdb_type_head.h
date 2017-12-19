#ifndef _XDB_TYPE_HEAD_H
#define _XDB_TYPE_HEAD_H


#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/****** ��ƽ̨ʹ��,����ȥ��*************
** #define _UCHAR              unsigned char
** #define _USHORT             unsigned short
** #define _ULONG              unsigned long
** #define _UINT               unsigned int
**************************************************/


#define NUM_OF_VECTORS          3

// add by dufei

// MAP �ӿ���������ֶκ궨��
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


// BOSS �ӿ��������ֶγ��ȶ���
#define LENGTH_OF_CSSID          4 //����css��Ԫ���豸ID
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
#define LENGTH_OF_ROAM_ID        4  /*����ID���� ����ģ�士����hlr*/
#define LENGTH_OF_ROAM_TPL_ID    1  /*����ģ��ID����*/
#define LENGTH_OF_ROAM_TPL_NAME  32 /*����ģ��������*/
#define LENGTH_OF_ROAM_NAME      32
#define LENGTH_OF_ROAM_AREALIST  256
#define LENGTH_OF_SUB_TPL_ID     2  /*�û�ģ��ID����*/
#define LENGTH_OF_SUB_TPL_NAME   32 /*�û�ģ��������*/
#define LENGTH_OF_VHLR           2  /*����HLR����*/
#define LENGTH_OF_HSTYPE         1
#define LENGTH_OF_DATE           14
#define LENGTH_OF_SS             6
#define LENGTH_OF_SUb_CAT        1 /*�û��������*/
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
	
	/*���ȶ��壺��̬��Ϣ*/
#define LENGTH_OF_HSSTATUS       1
#define LENGTH_OF_HSVERSION       1
#define LENGTH_OF_BSID           2
#define LENGTH_OF_BSCID          4
#define LENGTH_OF_LAIID          5
#define LENGTH_OF_VSSID          4
#define LENGTH_OF_SMCLIST        80
	
	
	/*���ȶ��壺����ҵ����Ϣ*/
#define LENGTH_OF_SS_CODE        1
#define LENGTH_OF_SS_STATE       1
#define LENGTH_OF_SS_PWD         8
#define LENGTH_OF_SS_PWD_OUTPRIV 4
#define LENGTH_OF_SS_PWD_FFC     6
#define LENGTH_OF_SS_PWD_CALL    8
	
	/*by wuhuan mod*/
	//ǰת���볤��
#define LENGTH_OF_SS_DN          16
#define LENGTH_OF_SS_OPTION      2


#define LENGTH_OF_SS_CTRX_EXT    4
#define LENGTH_OF_SS_CTRX_PRIV   2
#define LENGTH_OF_GID            4
#define LENGTH_OF_GNAME          20
#define LENGTH_OF_EXT            3
#define LENGTH_OF_PRIV           1

#define PID_LEN						4					//PID����

/*by wuhuan mod*/
#define LENGTH_OF_SS_HOTLINE     16
#define LENGTH_OF_PWD_CALL       8
#define LENGTH_OF_PWD_ODB        4


#define  LENGTH_OF_CTX_NAME             32        // ctx ҵ����
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
#define SERV_CODE_LEN           1    /* ҵ���볤�� */

//�û�ҵ����Ϣ��س��ȶ���
/*���˴��궨�����޸ģ�ע����ͬʱ�޸�
  XS32 XdbQrySubHlrOperation(XU32 dbTaskId, XU8 *uid, XU16 operCode, SSubOneHlrOperationList * pSubOneHlrOperationList)
  ��XS32 XdbCfgSubHlrOperation(XU32 dbTaskId, SSubOneHlrOperation *pSubOneHlrOperation)
  ����ض���*/
#define MAX_OPER_PROPERTY_LENGTH                       32 //ҵ�������ԣ���󳤶�Ϊ32
#define MAX_OPER_CODE_COUNT_OF_ONE_STRUCT              32 //һ��SSubHlrOperation֧�ֵ����ҵ�������������Ϊoperation�������Чλ��
#define MAX_OPERATION_COUNT                            1 ///���SSubHlrOperation������MAX_OPERATION_COUNTĿǰ����1���洢ҵ����Ϊ1-32��ҵ����Ϣ��ÿҵ��������32��MAX_OPERATION_COUNT�궨��Ҫ��1
#define MAX_OPER_CODE_COUNT MAX_OPERATION_COUNT*MAX_OPER_CODE_COUNT_OF_ONE_STRUCT


// dufei
#define LENGTH_OF_T 2
#define	LENGTH_OF_TWINFLAG			1	 // �I������ʽ
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
	//����
	e_UssClose,
	//����
	e_UssOpen,
	//��¼δ����
	e_UssNotOpen
}E_USERSTATE_STATIC;



//��Ե�SJ�ӿ��п��ܳ��ֵĴ���
typedef enum
{
	e_ErrorBegin=0,							//��ʼ
		e_NotError=0,							//�ɹ�(�޴���)
		
		e_DbResZeroError,						//ɾ�����޸ģ���ѯӰ��Ϊ0
		e_UnknownError,							//�쳣,δ֪����
		e_OpTypeError,							//�������ʹ���
		e_OpObjError,							//�����������
		e_PackLenError,							//��������
		e_ParamError,							//��������
		e_ModUserStateError,					//�޸��û�״̬ʧ��
		
		e_NotLogin,								//δ��½
		e_RepeatLogin,							//�ظ���½
		e_ViHlrNotExist,						//������HLR������
		e_OpUserNotExist,						//�ò���Ա�Ѳ�����
		e_OpUserPasswdError,					//��½�������
		e_LogoutError,							//ע��ʧ��
		
		e_DbUnknownError,						//�Կ����ʧ��
		e_FileNameError,						//�ļ�������
		e_UidExist,								//��UID�Ѵ���
		e_UidNotExist,							//��UID������
		e_DnExist,								//�õ绰�Ѵ���
		e_DnNotExist,							//�õ绰������
		
		e_UserExist,							//���û��Ѵ���
		e_UserNotExist,							//���û�������
		
		e_OpUserOrViHlrNotMatch,				//����Ա������HLR��Ų�ƥ��
		
		e_TemplateNotExist,						//ģ��ID������		
		e_TemplateExist,						//����ģ��ʱ,ģ��ID�Ѵ���
		e_CtxNotExist,							//ȺID������
		e_CtxExist,								//ȺID�Ѵ���
		e_CtxUserExist,							//Ⱥ���û��Ѵ���
		e_ShortNoExist,							//����λ�����Ѵ���
		
		e_NotVoIce,								//δǩ����ҵ��
		e_ResourceIsUse,						//��Դ��ʹ�� ��ɾ����ʱ��
		
		e_ToScuSynError,						//��SCU����ͬ��ʧ��
		e_UpdateStateError,						//�޸��û�״̬ʧ��
		
		e_RepeatOpenUser,						//�ظ�����
		e_RepeatDelUser,						//���ܸ�δ�����û�����
		e_FileOpenError,						//�ļ���ʧ��
		e_FileReadError,						//�ļ���ȡʧ��
		e_FileInvalidFormat,					//��Ч���ļ���ʽ
		e_FileExceedRecLmttn,					//�ļ���¼����������
		e_FileInternalError,					//�ڲ�����
		e_FileContinuous,						//δ��
		
		e_MuNewUidError,						//���Ų�������UID״̬����
		e_MuUidError,							//���Ų�������UID״̬����
		e_PackTypeError,						//���а�ͷ���ͳ���
		e_SsNotP,								//���������Ҫ�Ƚ���ǩԼ
		e_SsNotR,								//���������Ҫ�Ƚ���ע��
		e_SsNotA,								//���������Ҫ�Ƚ��м���
		e_DelUserAttError,                      //ɾ���û�ҵ��������ݳ���
		e_NotOpenuser,							//�û�δ����
		e_BatOpenUserError,						//��������ʧ��
		
		e_GroupNotExist,						//��ID������
		e_GroupExist,							//��ID�Ѵ���
		e_CtxUserShortNoLenError,				//Ⱥ���û��̺ų��ȳ���
		e_CtxGroupOverrun,						//Ⱥ�е����������
		
		e_GtnNotExist,							//GTN ID������
		e_GtnExist,								//GTN ID�Ѵ���
		
		e_GtnNumNotExist,						//�ܻ����벻����
		e_GtnNumExist,							//�ܻ������Ѵ���
		
		e_ErrorEND								//����
}E_SJ_ERROR_TYPE;


// sj3 ��������
typedef enum
{
	e_OPObjBegin = 0x00,							//��ʼ
	e_MEMObjID = 0x02,

	e_Version = 0x10 ,						            //�汾����



	e_VoSer = 0x50 ,						    //����ҵ�����(��Ӧ����ҵ������)
	e_DaSer = 0x51 ,						    //����ҵ�����(��Ӧ����ҵ������)

	e_UserTemplet = 0x50,						//�û�ģ��
	e_RoamTemplet,								//����ģ��
	e_Rest,										//Rest
	e_Cl,										//CancelLocation
	e_TempletOpen,								//ģ�忪��
	e_User,										//�û�
	e_Batch=0x56,								//�����û�
	e_Mz = 0x56,								// cpez ��mz�Ĺ�ϵ����������û�û���õ�
	e_BatchCl,									//����CancelLocation
	e_StopUser,									//ͣ��
	e_VPNManager = 0x59,							//����ר����֯����
	//e_Group,									//Ⱥ��
	e_GroupUser,								//Ⱥ���û�
	e_ShortNo=0x5b,									//��λ����
	e_CallPasswd,								//��������
	e_LockPasswd,								//��������
	e_HotlineNum,								//���ߵ绰
	
	e_SsState=e_HotlineNum+2,					//����ҵ��״̬
	
	
	e_FileBatch,								//�ļ���������
	e_SsPasswd,									//����ҵ������
	e_DestCode,									//Ŀ����
	e_Ocb,										//��������
	e_Wac,										//����Ⱥ
	e_Centrex,									//Ⱥ
	e_Gtn,										//�ܻ�
	e_Opt,										//����Ա
	e_GroupEx,									//��
	e_CentrexUser,								//Ⱥ�û�
	e_Octr,										//һ��˫��

	e_Stbc = 0x6c,						        		//��¼��Ϣ



    e_ChangeUid = 0x6d ,                 // 	


    e_Sj3UserGropTep  ,                 // 	��ģ�� T 

	e_filebatchStbc        = 0x70,      // 
	e_UserOpenTemplet      = 0x71,						//�û�ģ��
	e_FileUserOpenTemplet  = 0x72,						//ģ��	

    // HLR License ��Ϣ
    e_Hlr_LicInfo = 0x80,
    e_DbsSer = 0x81 ,						    //�����鲥ҵ�����add by lhy 2010.03.30

	e_Cid_Tpl = 0x83,					// CIDģ����Ϣ
	e_Roam_Tpl = 0x84,					// ����ģ����Ϣ
	e_Argmnt_Tpl = 0x85,				// �������ЭԼģ��

	e_OPObjBtsInfo = 0x87,					// ��վ���ݲ���
	e_SynHlr = 0x8a,					// ͬ��hlr

	e_Addr_Communication = 0x8b,			// ͨѶ¼ ͸����Ϣ
	e_Ip = 0x8c,						//������ԱIP����		0x8c
	e_Acl = 0x8d,						// _����ACL����
	e_ChangePwd = 0x8e,				//�û�������������//add by lx 2013.01.25
	//e_AnchorMod = 0x91,

	// ��Ⱥ��������
	e_Clan_Oper = 0xa0  ,    // ��Ⱥ����Աҵ�����
	e_Clan_Login   ,          // ��¼����    
	e_Clan_Org     ,          // ��֯ҵ�����      
	e_Clan_User    ,          // �����û�ҵ�����     
	e_Clan_Group   ,          // ��ҵ�����       
	e_Clan_OperGrp    ,       // ����Ա����        
	e_Clan_OperUser   ,       // ����Ա�ĵ����û� 
	e_Clan_GroupUser  ,       // ������û� 
	e_Clan_WatchTask  ,       // �������� 
	e_Clan_Shakehand  ,       // ����
	e_Clan_Vsersion = 0xaa,			// Э��汾�������
	e_Clan_FixArea =0xab,			// �̶����� 
	e_Clan_GroupOstdUser =0xac,		// �������û� 
	e_Clan_BSList =0xad,			// ��վ�б� 
	e_Clan_GroupUserList =0xae,		// ���û��б��ѯ
	e_Clan_OSSTransmit =0xaf,		// OSS͸����Ϣ


    e_OPObjEnd,							//����


}E_SJ_OPOBJ_ID;



// sj3 ��������
typedef enum
{
	e_OPTypeBegin = -1 ,					

	e_OPTypeAdd ,							//��	
	e_OPTypeDel ,							//ɾ	
	e_OPTypeMod ,							//��	
	e_OPTypeQue ,							//��	
	e_OPTypeListQue ,						//���в�ѯ	
	e_OPTypeOther,							//11.1.5
	e_OPTypeModAnchorBts,					//�޸�ê��վ
	e_OPTypeQryEMS,							//��ѯEMS��ip��port
	//e_OPType6,	               				// Ŀǰֻ��bossʹ��
	//e_OPTypeConfigUserState=8,			//�����û�ͣ��״̬//add 2013.03.26
	//e_OPTypeCreditControl,				//�ſ�����//add 2013.03.26
	//e_OPTypeEncryptParamSet = 0x0A,         //���ܲ�����������
	//e_OPTypeEncryptParamQue = 0x0B,         //���ܲ�����ѯ
 	//e_OPTypeConfigLeaseBind=12,         //���ó���δ����״̬����/ȡ�����ް󶨹�ϵadd 2013.07.10
	//e_OPTypeQryLeaseBind=13,           //��ѯ�ն����ް󶨹�ϵ
	//e_OPTypeLeaseActivate=14,           //�����ն˳��⼤��״̬���� 2013.07.26
	//e_OPTypeCancelCreditStatus = 15,/*ȡ���ſ�״̬*/
	//e_OPTypeQueryCreditStatus = 16,	/*��ѯ�ſ�״̬*/
	//e_OPTypeQueryUtInfo= 17,		/*��ѯ�ն�Ӳ������*/
	//e_OPTypeDelUser=0x12,	//�޳��û���Ϣ
	//e_OPTypeLoadUserInfo=0x14,//װ���û���Ϣ���ڴ�����
	//e_OPTypeMdyTelno=0x15,//�޸ĵ绰��������
	e_OPTypeSynOptBts =0x20, 				// ��ѡ��վͬ��
	e_OPTypeSynArgmntTpl = 0x21,			//���ЭԼģ�����

	e_OPTypeEnd 		
}E_SJ_OPTYPE_ID;


typedef enum
{
	e_OPTypeUserBegin = -1 ,					

	//e_OPTypeAdd ,							//��	
	//e_OPTypeDel ,							//ɾ	
	//e_OPTypeMod ,							//��	
	//e_OPTypeQue ,							//��	
	//e_OPTypeListQue ,						//���в�ѯ	
	//e_OPTypeOther,							//11.1.5
	e_OPType6 =6,	               				// Ŀǰֻ��bossʹ��
	e_OPTypeConfigUserState=8,			//�����û�ͣ��״̬//add 2013.03.26
	e_OPTypeCreditControl,				//�ſ�����//add 2013.03.26
	e_OPTypeEncryptParamSet = 0x0A,         //���ܲ�����������
	e_OPTypeEncryptParamQue = 0x0B,         //���ܲ�����ѯ
	e_OPTypeConfigLeaseBind=12,         //���ó���δ����״̬����/ȡ�����ް󶨹�ϵadd 2013.07.10
	e_OPTypeQryLeaseBind=13,           //��ѯ�ն����ް󶨹�ϵ
	e_OPTypeLeaseActivate=14,           //�����ն˳��⼤��״̬���� 2013.07.26
	e_OPTypeCancelCreditStatus = 15,/*ȡ���ſ�״̬*/
	e_OPTypeQueryCreditStatus = 16,	/*��ѯ�ſ�״̬*/
	e_OPTypeQueryUtInfo= 17,		/*��ѯ�ն�Ӳ������*/
	e_OPTypeDelUser=0x12,	//�޳��û���Ϣ
	e_OPTypeBatchAddDelDCSGroupUser = 0x13, //��������ɾ����ĵ����û�//add 2013.09.23
	e_OPTypeLoadUserInfo=0x14,//װ���û���Ϣ���ڴ�����
	e_OPTypeMdyTelno=0x15,//�޸ĵ绰��������
	e_OPTypeHlrOperSupportSet=0x16, //�����û�ҵ��֧��   add 2013.12.21
	e_OPTypeHlrOperSupportQry=0x17, //��ѯ�û�ҵ��֧��   add 2013.12.21
	e_OPTypeDelUserByPid=0x18,//����PID�߳��û�
	e_OPTypeQryUserInfoByUid=0x19,//����UID��ѯ������Ϣ����
	e_OPTypeQryUserInfoByTelno=0x1A,//���ݵ绰�����ѯ������Ϣ����
	e_OPTypeLoadTelnoInfoToMem=0x1B,//�󶨻���ʱ��װ���û���Ϣ���ڴ�����
	e_OPTypeUnBindTelToUid=0x1C,	//����ֺ�������
	e_OPTypeUserEnd 		
}E_SJ_USER_OPTYPE_ID;


// sj3 ���в��������б�
typedef enum
{
	e_SjTagBegin = 0,

	e_SysVersion = 101 ,   // �汾

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
	

	e_VoSerOrDataType ,   // ���������ݱ�־λ1110
	e_CallerNum ,		  // ����任1111

        e_ChargeFlag,               // HLR�Ƿ�֧�ּƷѿ���
        e_LicInfo,                      // HLR License��Ϣ
	  e_SS_INFO_EXT,           //����ҵ���б�2   1114
	  e_SQLString= 1115,
	  e_UserTag=1116, //�û�����
	  e_OutPriv = 1200 ,		// ��������	CallOutLimit	1200
	 e_Cfb ,		    	// ��æǰת	FwdBusy	
	 e_Cfu ,		    	// ������ǰת	Fwd	
	 e_Cfnrc ,		    	// ���ɼ�ǰת	FwdNoTouch	
	 e_Cfnry ,		    	// ����Ӧǰת	FwdNoResponse	
	 e_Cw ,			    	// ���еȴ�	WaitCalled	
	 e_NodDisturd ,			// ������	KeepDistrouble	
	 e_ShrinkDN ,			// ��λ�κ�	ShortNumber	
	 e_HotLineCS ,			// ��ʱ����	TimeOutHotline	
	 e_HotLineJS ,			// ��ʱ����	InstanceHotline	
	 e_Ctrx ,				// Ⱥ���û�	GroupUser	

	 e_OneForTwo ,			// һ��˫�� 1211
	 e_OneCallTwo ,		// һ��˫�� 1212

	 e_OneCallTell ,		// һ��˫���ӻ����� 1213



     // �ն˹�����
     e_UserAuth   ,      // ����   1214
  	 e_UserBm 		    , //  ����
  	 e_UserGroup 	    , // ����
   	 e_UtAcperiod	    , // ����ʱ�� 	    

   	 e_UserGroupTep	    , // ��ģ���� 	

	e_FileBatchstbcOrder 	  = 1221 ,	// ���Ȳ�ѯ�������
	e_FileBatchstbcItemsCount = 1222 ,	// ������¼��Ϣ�ļ�����������¼����Ϣ������
	e_FileBatchstbcCurrent = 1223 ,		// ��ǰ����¼����	
	e_FileBatchstbcEndFlag = 1224,		// ��¼�Ƿ�����ɣ�0��ʾδ��ɣ�1��ʾ��ɡ�

	e_LoginType = 1235,                 //LOGINTYPE  0x01:Wi128��0x02:MEM�ۺϹ���ͻ��ˡ�0x03��ͨѶ¼������//add by cyl 2012.09.22
	e_BTSIDList = 1250,					//MEM�ɵ�¼���߲������¼�Ļ�վ�б� ����tag
	e_PermitInd = 1251,					//0:��ʾ����������CID,1����ʾ��������CID
	e_OperateInd = 1252,
	 e_UserDbsRight=1301	    , // �����鲥Ȩ��	

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

	e_UserMgmtFlag = 1329,				// �û�����״̬

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
	//����ģ��ѡ��ģ�鴦��
	e_NetSelectTmpId=1442,    //modify by lx 2012.05.15
	e_NetSelectTplName=1443,  
	e_NetSelectTmpList=1444,  
	e_ForbidCallFlag = 1445, //������ʶ

	e_ManagerStatus = 1446,//����ͣ��״̬//add  2013.03.26
	e_VoiceStatus = 1447,//����ͣ��״̬
	e_DataStatus = 1448,//����ͣ��״̬
	e_BWStatus = 1449,//���ݴ���ҵ��״̬

	e_DENInd = 1450, //���ܹ������ñ�ʶ
	e_KeyLen = 1451, //��Կ����
	e_KeyKillInd = 1452, //��Կң�ٱ�ʶ
	e_KeyUpdatePeriod = 1453, //��Կ��������
	
	e_BindUid = 1454,//��UID //add 2013.07.10
	e_LeaseHoldFlag,//���ް󶨹�ϵ����orȡ����ʶ//0ȡ��1ȷ��
	e_LeaseHoldCtlList,//����TAG ���ް󶨹�ϵ�б�
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
	e_CreditStatusTag = 1479,/*�ſ�״̬*/
	e_LastRegisterTimeTag = 1480,	/*����¼ʱ��*/
	e_UtInfoTag = 1481,/*UT��Ϣ*/
	e_PreferLanguageInd = 1482, /*��ʾ����������ָʾ add by wj 2013.08.30*/
	e_GroupNum = 1483, //����
	e_GroupId = 1484, //��ʶ����
	e_GroupUserId = 1485, //���û�
	e_DeleteGroupUser = 1486, //ɾ�����û�
	e_AddGroupUser = 1487, //�������û�
	e_PttPriority = 1488,  //��Ȩ���ȼ�
	
	e_OperationCode = 1489, /*ҵ����*/
	e_OperationFlag = 1490, /*�Ƿ�ͨҵ����Ϣ*/
	e_OperationProperty = 1491, /*ҵ����Ϣ*/
	e_HlrOperation = 1492, /*����TAG �û�ҵ��*/

	e_NationalCode = 1493,/*������*/
	e_TelnoType = 1494,/*�绰��������*/
	e_NewTelnoList = 1495,/*�绰�����б�*/
	e_UnBindTelnoList = 1496,/*���绰�����б�*/

	// SAC ���� TYPE
	 e_CidTmp = 2000,		// cid ģ���
	 e_PidPort ,			// pid �˺ź�
	 e_AgenSsIp,			// ��������IP       AgenSsIp =               2002;
     e_AgenSsIpPort,		// ���������˿�     AgenSsIpPort =           2003;
	 e_RegSsIp,				// ע������IP       RegSsIp =                2004;
	 e_RegSsIpPort,				// ע�������˿�     RegSsIpPort =            2005;
	 e_WirelessRegCircleValue,		// ����ע������(ֵ)   WirelessRegCircleValue = 2006;
	 e_WirelessRegCircleType,			// ����ע������(��λ) WirelessRegCircleType =  2007;
	 e_SipRegCircleValue,				// SIPע������(ֵ)    SipRegCircleValue =      2008;
	 e_SipRegCircleType,				// SIPע������(��λ)  SipRegCircleType =       2009;
	 e_SipPassword,					    // SIP����            SipPassword =            2010;	 

    // mem
     e_memDnsIp =  2011 , // DNS IP
  	 e_memIp 		    , // IP
  	 e_memSubMask 	    , // ��������
   	 e_memGateway	    , // ���� 	 

	 e_StbcUserType = 2015 ,
	 e_memDhcpType ,    // mem dhcp type

	e_Cid,				// CID
	e_RoamTplName,		// ����ģ������
	e_SdName,			// �������ЭԼģ������
	e_WifiSpot=	2020,

	e_CentrexId,
	e_CentrexGrpNum =2022,

    // ��Ⱥ�û�
    e_ClanDispather            = 3000  ,   //   ��Ⱥ����Ա              
    e_ClanDispather_UserName   = 3001  ,   //   ����Ա����              
    e_ClanDispather_UserPwd    = 3002  ,   //   ����Ա����              
    e_ClanDispather_UserRole   = 3003  ,   //   ������ɫ                
    e_ClanDispather_OrganizeID = 3004  ,   //   ��֯ID                  
    e_ClanDispather_UID        = 3005  ,   //   ����ԱUID               
    e_ClanDispather_UserTelNo  = 3006  ,   //   ����Ա�绰����          
    e_ClanDispather_SID        = 3007  ,   //   �û�SID(����)  	
    e_ClanDispather_UserPwd_Old= 3008  ,   //   ����Ա������  
    e_ClanDispather_GroupID 		  ,    //   ��ID    	
    e_ClanDispather_UserPriority      ,    //  user���ȼ�     
	e_ClanDispather_LinkCallTellNo   ,
	e_ClanDispather_EnableFlag      ,
	e_ClanDispather_NightServiceTellNO   ,
	e_UserRecordAttri		= 3014,		// ǩԼ¼��ҵ��
	e_groupRecordAttri		= 3015,		// ��ǩԼ¼��ҵ��

	
        
    e_ClanOrganize             = 3100  ,   //   ��֯                    
    e_ClanOrganize_OrganizeID  = 3101  ,   //   ��֯ID                  
    e_ClanOrganize_OrganizeName= 3102  ,   //   ��֯����  

    e_ClanDcsUser              = 3200  ,   //   �����û�                
    e_ClanDcsUser_UID          = 3201  ,   //   �����û�UID             
    e_ClanDcsUser_UserTelNo    = 3202  ,   //   �����û��绰����        
    e_ClanDcsUser_OrganizeID   = 3203  ,   //   �����û�UID             
    e_ClanDcsUser_UserDesc     = 3204  ,   //   �����û�����  
	e_ClanDcsUser_UserPriority 		   ,
	e_ClanDcsUser_Attach_GID      	   ,
	e_ClanDcsUser_AuthCallTelNo		   ,	
	e_ClanDcsUser_IsDispather		   ,
	e_ClanDcsUser_UserProInGrp 		   , 
	e_ClanDcsUser_Ptt_Priority		   ,
	e_ClanDcsUser_DbsSwitch ,			   //  �û������鲥����
	e_ClanDcsUser_Department=	3212,//�û���λ
	e_ClanDcsUser_duty	=3213,  // �û�ְ��
	e_ClanDcsUser_ForbidStatus=3214,	 //�û�����״̬
	e_ClanDcsUser_ShortCodes=3215,//�����û��̺���//add 2013.08.14
	e_ClanDcsUser_CommRange = 3216,//�����û�ͨ��Ȩ��

    e_ClanDcsGroup             = 3300  ,   //   ��                      
    e_ClanDcsGroup_GroupID     = 3301  ,   //   ��ID                    
    e_ClanDcsGroup_OrganizeID  = 3302  ,   //   ��֯ID                  
    e_ClanDcsGroup_GroupName   = 3303  ,   //   ������                  
    e_ClanDcsGroup_VSSIDList   = 3304  ,   //   �������Χ            
    e_ClanDcsGroup_VoiceOpen   = 3305  ,   //   ��������                
    e_ClanDcsGroup_MessageOpen = 3306  ,   //   ���ſ���                
    e_ClanDcsGroup_DataSerOpen = 3307  ,   //   �������ҵ�񿪹�        
    e_ClanDcsGroup_Supplement  = 3308  ,   //   ��Ⱥ����ҵ��            
    e_ClanDcsGroup_GCallTimer  = 3309  ,   //   �����ʱ��              
    e_ClanDcsGroup_Priority    = 3310  ,   //   �����ȼ�                
    e_ClanDcsGroup_FreeTimer   = 3311  ,   //   ������ж�ʱ��          
    e_ClanDcsGroup_LETimer     = 3312  ,   //   �ٺ���������ֶ�        
    e_ClanDcsGroup_ParGrpID     	   ,   //   �ϼ���ID
    e_ClanDcsGroup_ISTEMP     	   ,	   //   �Ƿ���ʱ��
	e_ClanDcsGroup_GDescription,           //   ������

	e_TaskID               = 3400  ,   //   ����ID                  
    e_Task_QueryType       = 3401  ,   //   ��ѯ����                
    e_Task_Flag            = 3402  ,   //   ��־(��ʼ�����)        
    e_Task_Status          = 3403  ,   //   �û�״̬                
    e_Task_Status_UID      = 3404  ,   //   �û�UID                 
    e_Task_Status_GID      = 3405  ,   //   ��ID                    
    e_Task_Status_value    = 3406  ,   //   ״ֵ̬                  
    e_Task_Status_PeerTelNo= 3407  ,   //   �Է�ͨ������ 
	e_Task_Status_DigitNo  = 3408  ,   //   ��Ҫ��ѯ�û��ĵ绰����//add by lx 2012.09.14
	e_TranceID             = 3500  ,   //   ÿ����Ϣ��ǰ��TraceID   
	e_CcbIndex			   = 3501  ,   //   pull��ccbindex         
                                                                        
                                                                        
  
	e_ClanDevid                = 3502  ,    //  DEVID
	e_Clan_DbsRight = 3505,		// �����鲥Ȩ��
	e_Clan_Grp_DbsSwitch = 3506, // �������鲥����
	e_Clan_SDdh_Version = 3507, // Sddh�ӿڰ汾
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
	
	
	
	
	/*�Ʒ�*/	
	ChargePepayind 			    = 4000 ,    // ��������
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
	
	//�û������޸�
	e_NewPwd=5008,//�µ�����
	e_SjTagEnd
}E_SJ_TAG;



//����ҵ����
typedef enum
{
	e_SsBegin=0x00,				
	e_SsClip=0x11,				//���к�����ʾ
	e_SsClir=0x12,				//���к���������ʾ
	e_SsMct=0x15,				//������и���
	e_SsCfu=0x21,				//������ǰת
	e_SsCfb=0x29,				//��æǰת
	e_SsCfnry=0x2a,				//��Ӧ��ǰת
	e_SsCfnrc=0x2b,				//���ɼ�ǰת
	e_SsPEct=0x31,				//����ת�ƣ��Ĳ�ǰת��
	e_SsCw=0x41,				//���еȴ�
	e_SsCh=0x42,				//���б���
	e_SsMpty=0x51,				//����ͨ��
	e_SsCliro=0xe1,				//���к�����ʾ������Խ
	e_SsHlto=0xe2,				//��ʱ���ߣ����ڣ�
	e_SsShrink=0xe3,			//��λ���ţ����ڣ�
	e_SsOcb=0xe4,				//��������
	e_SsDds=0xe5,				//�����ҵ�񣨶��ڣ�
	e_SsMee=0xea,				//����绰
	e_SsSar=0xeb,				//ָ��Ŀ�������ƣ����ڣ�
	e_SsSaa=0xec,				// ָ��Ŀ������������ڣ�
	e_SsCentrex=0xed,			//CentrexȺ��
	e_SsPasswdCall=0xee,		//������У����ڣ�
	e_SsIhl=0xef,				//��ʱ���ߣ����ڣ�
	e_SsAc=0xf1,				//����ҵ�񣨶��ڣ�
	e_SsFfc=0xf2,				//�����У����ڣ�
	e_SsSc=0xf3,				//��ͨ��������Ϣ���ѣ������ڣ�
	e_SsAoc=0xf4,				//�������ԣ����ڣ�
    e_SsCr=0xf5,                //����
    e_SsOctr=0xf6,              //һ��˫��
	e_SsEnd								
}E_SJ_SS;


#ifdef __cplusplus
}
#endif /* _ _cplusplus */
#endif

