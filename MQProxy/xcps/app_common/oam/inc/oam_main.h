/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     oam_install_main.h
* Author:       xhm
* Date��        2014-09-02
* OverView:     process install message
*
* History:      create
* Revisor:      xhm
* Date:         2014-09-02
* Description:  create the file
*******************************************************************************/
#ifndef __OAM_MAIN_H__
#define __OAM_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xosmodule.h"
#include "xoshash.h"
#include "xosarray.h"
#include "xosshell.h"
#include "xmltree.h"
#include "xwriter.h"
#include "xmlparser.h"
#include "oam_cli.h"
#include "xosipmi.h"
#include "common_msgdef.h"

#pragma pack(1)
#define OAM_SEND_MAX_NUM        32      //OAM����������

#define MAX_TB_NUM              200     //����ID��
#define MAX_SLOT_ID             14      //����λ��
#define HA_IP_NUM               2       //����IP��
#define NE_PROCESS_MAX_NUM      16      //��Ԫ���������
#define DESC_INFO_LEN           128     //������Ϣ����


#define OAM_SESSION_MAX_NUM     128     //�Ự�����
#define OAM_SESSION_HEAD_UNUSED   0     //�Ựͷδʹ��
#define OAM_SESSION_HEAD_USED     1     //�Ựͷ��ʹ��

#define AGENT_IP_TABLE_ID       500     //agentip ��ID(1����¼ΪagentIP,����һ��Ϊǰ���IP)
#define TID_MODULE_MANAGE       505     //���̱�ID

#define OAM_MSGTYPE_RESPONSE    0x200   //OAM��Ӧ��ϢID 
#define OAM_MSGTYPE_SYNC        0x201   //OAM��agent��ȡ����������ϢID
#define OAM_MSGTYPE_NOTIFY      0x203   //OAM(BM����)״̬�ϱ���Ϣ

//���������ֶ�
#define OAM_TABLE_MAX_NUM       200     //���������
#define FILE_PATH_MAX_LEN       128     //�ļ�·������󳤶�
#define VALUE_MAX_LEN           8       //��ʱ�ֶγ���
#define BUFF_MAX_LEN            4096    //��ʱ����������
#define TID_PROCESS_MANAGE      505     //���̱�ID

//ҵ��ģ��ע����ϢID����
#define OAM_REG_TALBEID         1       //ע���ID
#define APP_REGISTER_MSG        7       //OAMʶ��Ӧ��ģ��SYNC����Ϣ
#define OAM_OFFSET_LEN          2       //ƫ��λ��

//Agent��ϢID����
#define AGENT_CFG_MSG           0x100   //Agent������Ϣ
#define AGENT_OPER_MSG          0x101   //Agent����(���塢��Ԫ�����̵�)��Ϣ
#define AGENT_SYNC_MSG          0x102   //Agentͬ����Ϣ
#define AGENT_FMT_MSG           0x103   //Agent��ʽ����Ϣ
#define AGENT_DEBUG_REG_MSG     0x104   //Agent���/ά��������Ϣ
#define AGENT_DEBUG_RSP_MSG     0x105   //Agent���/ά����Ӧ��Ϣ

//��TSͨ����ϢID
#define OAM_MSG_IP_REQ          0x88    //TS��ȡǰ���IP��Ϣ
#define OAM_MSG_IP_RSP          0x89    //OAM��TS IP��Ϣ��Ӧ
#define OAM_MSG_NAV_INFO        0x90    //OAM���µ�������Ϣ��Ϣ

//����������ض���
#define OAM_PM_MAX_NUM          200     //��Ԫ���ܲ���ָ���������
#define OAM_PM_NOTIFY_MSG       0x300   //PM�����ϱ���ϢID
#define PM_IDENTITY_LEN         32      //��ͳΨһ��ʶ����


//����澯��ض���
#define OAM_FM_NOTIFY_MSG       0x500   //�澯�ϱ���ϢID
#define FM_ITEM_MAX_NUM         8       //�澯��λ��Ϣ�����
#define FM_INFO_VAL_LEN         64      //�澯��λ��Ϣ
#define FM_ALARM_MAX_LEN        4096    //��ǰ�澯����¼
#define FM_TYPE_NUM             2       //�澯���������
#define FM_MSG_SAVE_NUM         200     //�澯��Ϣ���������

//������
#define OAM_PARA_MAX_NUM        32      //������������
#define OAM_PARA_NAME_LEN       8       //�������������󳤶�
#define DEBUG_RSP_MAX_LEN       4096    //�����Ӧ������󳤶�

//�ض�������
#define OAM_ERRORCODE_UNEXPECTED_DATA  0x100  //���̲���Ҫ�������ݴ�����
/*����t_XOSCOMMHEAD��Ϣͷ��Ϣ*/
#define SET_XOS_MSG_HEAD(pMsg, msgId, srcFid, dstFid) \
((t_XOSCOMMHEAD*) pMsg)->datasrc.PID = XOS_GetLocalPID(); \
((t_XOSCOMMHEAD*) pMsg)->datasrc.FID = srcFid; \
((t_XOSCOMMHEAD*) pMsg)->datasrc.FsmId = 0; \
((t_XOSCOMMHEAD*) pMsg)->datadest.PID = XOS_GetLocalPID(); \
((t_XOSCOMMHEAD*) pMsg)->datadest.FID = dstFid; \
((t_XOSCOMMHEAD*) pMsg)->datadest.FsmId = 0; \
((t_XOSCOMMHEAD*) pMsg)->msgID = msgId; \
((t_XOSCOMMHEAD*) pMsg)->subID = 0; \
((t_XOSCOMMHEAD*) pMsg)->prio = eNormalMsgPrio


/*�ͷ�ָ���ڴ�*/
#define PTR_MEM_FREE(fid, ptr)      \
do                                  \
{                                   \
    if(XNULL != ptr)                \
    {                               \
        XOS_MemFree(fid, ptr);      \
        ptr = XNULL;                \
    }                               \
}while(0)


/* ��麯����ָ������Ƿ�Ϊ�� */
#define PTR_NULL_CHECK(ptr,rtVal)     \
do                                    \
{                                     \
    if(NULL == ptr)                   \
    {                                 \
        XOS_PRINT(MD(FID_OAM,PL_ERR), "prt is null!\r\n");\
        return rtVal;                 \
    }                                 \
}while(0)


//�澯���ö�ٶ���
typedef enum fm_class_e{
    FM_CLASS_ALARM = 1,   //���ϸ澯
    FM_CLASS_EVENT,       //�¼��澯
}FM_CLASS_E;

//��Ԫ�澯����ö�ٶ���
typedef enum fm_type_e{
    FM_TYPE_SOFT = 1,     //����澯
    FM_TYPE_HARD,         //Ӳ���澯
    FM_TYPE_SERVICE,      //ҵ��澯
    FM_TYPE_LINK,         //��·�澯
}FM_TYPE_E;

//�澯����ö�ٶ���
typedef enum fm_level_e{
    FM_LEVEL_CRITICAL = 1,//�����澯
    FM_LEVEL_MAJOR,       //��Ҫ�澯
    FM_LEVELE_MINOR,      //��Ҫ�澯
    FM_LEVEL_HINT,        //��ʾ�澯
}FM_LEVEL_E;


//OAM�澯����ö�ٶ���
typedef enum alarm_type_e{
    ALARM_TYPE_CUR = 0,   //��ǰ�澯
    ALARM_TYPE_HISTORY,   //��ʷ�澯
}ALARM_TYPE_E;

    
//�澯��־ö�ٶ���
typedef enum fm_flag_e{
    FM_FLAG_RECOVER = 0,  //�澯�ָ�
    FM_FLAG_NOTIFY,       //�澯�ϱ�
}FM_FLAG_E;
    
typedef struct oam_tid_mid_t
{
    XU32 uiTableId;   //��ID
    XU32 uiModuleId;  //ģ��ID
}OAM_TID_MID_T;

//TS�������ڵ���Ϣ
typedef struct ts_nav_info{
    XU16 usNeType;                 //��Ԫ����
    XU16 usNeId;                   //��ԪID
    XU16 usDescLen;                //��Ԫ��������
    XS8 ucDesc[FILE_PATH_MAX_LEN + 1]; //��Ԫ����
    XU16 usLogPid;                 //�߼�����ID
    XU16 usModuleType;             //��������
    XU16 usSlotId;                //��λ��
}TS_NAV_INFO;


//��������Ϣ
typedef struct ts_cfg_req_t{
    XU32 uiPidNum;                 //����ID����
    XU32 uiLen;                    //ָ���ܳ���      
    TS_NAV_INFO *pNavData;         //�������ڵ�
}TS_CFG_REQ_T;


//��������Ϣ
typedef struct ts_ip_info_t{
    XU32 uiIpAddr;                 //ǰ���IP
    XU32 uiMslotId;                //����λ��   
    XU32 uiSslotId;                //����λ��
}TS_IP_INFO_T;

/*��Ϣͷ����*/
typedef struct agent_oam_cfg_head_t
{
    XU32 uiMsgId;     //��ϢID,��ʾ��Ԫ�仹����Ԫ�ڲ�
    XU32 uiMsgType;   //��Ϣ����
    XU16 usNeId;      //��ԪID
    XU16 usPid;       //����ID
    XU32 uiTransId;   //����ID
    XU32 uiSessionId; //�ỰID
}AGT_OAM_CFG_HEAD_T;

typedef struct agt_cfg_req_t
{
    AGT_OAM_CFG_HEAD_T  stAgtCfgHead;
    AGT_OAM_CFG_REQ_T   stAgtOamData;
}AGT_CFG_REQ_T;

typedef struct agt_cfg_rsp_t
{
    AGT_OAM_CFG_HEAD_T   stAgtCfgHead;
    AGT_OAM_CFG_RSP_T    stAgtOamData;
}AGT_CFG_RSP_T;

typedef struct oam_session_head_t
{
    XU8                 ucUsed;
    AGT_OAM_CFG_HEAD_T  stAgtCfgHead;    
}OAM_SESSION_HEAD_T;


//��Ԫ�ϱ���OAM���ݽṹ
typedef struct pm_data_t
{
    XU32 uiPmId;        //����ָ��ID
    XU32 uiValue;       //����ָ��ֵ
}PM_DATA_T;


//��Ԫ�ϱ���OAM���ݽṹ
typedef struct pm_report_t
{
    XU64 uiCurTime;     //��ǰ�ϱ�ʱ��("yyyymmddHHMMSS"��ʽ)
    XU32 uiClassId;     //��ID ��Ӧ�ĵ���measureobj id
    XU32 uiInstanceId;  //ʵ��ID,��S1��·ID=1�Ļ�ͳ����
    XU32 uiCycle;       //ѭ������,��Ӧ�ĵ���mincycle
    XS8  ucIdentity[PM_IDENTITY_LEN];  //Ψһ��ʶ,��Ӧ�ĵ���identity
    XU32 uiNum;         //ָ����
    PM_DATA_T stPmData[OAM_PM_MAX_NUM]; //���ܲ�������
}PM_REPORT_T;


//OAM���͸�agent���ݽṹ
typedef struct oam_pm_report_t
{
    XU16 usNeId;        //��ԪID
    XU16 usPid;         //����ID
    PM_REPORT_T stPmReport; //���ܲ����ϱ��ṹ
}OAM_PM_REPORT_T;

//�澯��ؽṹ����
//�澯λ����Ϣ�ṹ����
typedef struct oam_loc_info_t
{
    XU16 usRackId;
    XU16 usFrameId;
    XU16 usSlotId;
    XU16 usNeId;
    XU16 usProcId;
}OAM_LOC_INFO_T;

//�澯��λ��Ϣ�ṹ����
typedef struct fm_data_t
{
    XU16 usFieldId;              //field ID
    XU16 usValLen;               //���ݳ���
    XU8 ucData[FM_INFO_VAL_LEN]; //�澯��λ��Ϣ����
}FM_DATA_T;

//�澯������Ϣ�ṹ����
typedef struct fm_para_t
{
    XU32 uiAlarmId;      //�澯ID
    XU64 ulAlarmSeq;     //�澯��ˮ��
    XU8 ucDataNum;       //��λ��Ϣ��
    FM_DATA_T stFmData[FM_ITEM_MAX_NUM];  //�澯��λ��Ϣ
}FM_PARA_T;

//�澯��Ϣ�ṹ����
typedef struct fm_alarm_t
{
    XU32 uiAlarmClass;         //�澯��� 0:���ϸ澯 1:�¼��澯
    XU8 ucAlarmFlag;           //�澯��־ 0:�澯�ָ� 1:�澯�ϱ�
    XU64 ulAlarmTime;          //�澯����ʱ��
    OAM_LOC_INFO_T stFmLocInfo;//�澯λ����Ϣ
    FM_PARA_T stFmPara;        //�澯������Ϣ
}FM_ALARM_T;


//�澯��Ϣ�ṹ����
typedef struct fm_agt_alarm_t
{
    XU32 uiAlarmSeq;          //�澯��ˮ��
    FM_ALARM_T stFmAlarm;     //��Ԫ�澯��Ϣ
}FM_AGT_ALARM_T;


//�澯��ѯ��Ϣ�ṹ����
typedef struct fm_alarm_lst_t
{
    XU32 uiAlarmNum;           //�澯��
    FM_AGT_ALARM_T stAgtFmAlarm[FM_ALARM_MAX_LEN];//�澯��Ϣ
}FM_ALARM_LST_T;


//�澯��Ϣ����ṹ����
typedef struct fm_alarm_save_t
{
    XU16 usHead;          //ͷλ��
    XU16 usTail;          //βλ��
    FM_ALARM_T stFmAlarm[FM_MSG_SAVE_NUM];  //��Ԫ�澯��Ϣ
}FM_ALARM_SAVE_T;


//�澯��Ϣ����ṹ����
typedef struct oam_hainfo_t
{
    XU16 usSlotId;                 //��λ��
    t_IPADDR stIpAddr[HA_IP_NUM];  //IP��Ϣ
}OAM_HAINFO_T;

//������Ϣ����ṹ����
typedef struct module_manage_t{
    XU16 usNeId;                   //��ԪID
    XU16 usProcId;                 //����ID
    PROC_TYPE_E eProcType;         //��������
    XU16 usFrameId;                //�����
    XU16 usMSlotId;                //����λ��
    XU16 usSSlotId;                //����λ��
    XS8 ucDesc[DESC_INFO_LEN];     //��������
    XU8 ucStatus;                  //����״̬
}MODULE_MANAGE_T;


typedef struct ne_mdu_t{
    XU32 uiProcNum;                 //������
    MODULE_MANAGE_T stMduInfo[NE_PROCESS_MAX_NUM];   //������Ϣ
}NE_MDU_T;



//�������Խṹ����
typedef struct para_property_t{
    XU8 ucName[OAM_PARA_NAME_LEN];//�ֶ���
    XU8 ucVal[DESC_INFO_LEN];    //�ֶ�ֵ
}PARA_PROPERTY_T;

//ά����������ṹ
typedef struct oam_softdebug_req_t{
    XU32 uiFieldNum; //�����������
    PARA_PROPERTY_T stParaProperty[OAM_PARA_MAX_NUM];    //��������ṹ��Ϣ
}OAM_SOFTDEBUG_REQ_T;

//ά��������Ӧ�ṹ
typedef struct oam_softdebug_rsp_t{
    XU32 uiMsgLen; //��Ϣ����
    XU8 ucRspVal[DEBUG_RSP_MAX_LEN];    //�����Ӧ����ֵ
}OAM_SOFTDEBUG_RSP_T;

//OAMά������ṹ
typedef struct oam_agt_sftdbg_rsp_t{
    OAM_LOC_INFO_T stLocInfo;           //λ����Ϣ
    OAM_SOFTDEBUG_RSP_T stRspInfo;      //�����Ӧ��Ϣ
}OAM_AGT_SFTDBG_RSP_T;

//OAMά������ṹ
typedef struct oam_agt_sftdbg_req_t{
    XU32 uiModuleId; //�����������
    OAM_SOFTDEBUG_REQ_T stSftDbgReq;      //���������Ϣ
}OAM_AGT_SFTDBG_REQ_T;
XS32 OAMEntry(HANDLE hdir, XS32 argc, XS8** argv);
XS8  OAM_Init(XVOID* Para1, XVOID* Para2);
XS8  OAM_Notice(XVOID* pMsg, XVOID* Para);
XS8  OAM_MsgProc(XVOID* pMsg, XVOID* Para );
XS8  OAM_TimeOut(t_BACKPARA* para);
XS8  OAM_BmMsgProc(t_XOSCOMMHEAD *pMsg );
XS8 OAM_NtlMsgProc(t_XOSCOMMHEAD *pMsg );
XS32 OAM_AgtRspSend(XVOID*pMsg);
XS32 OAM_AgtCfgMsgProc(XVOID*pMsg);
XS32 OAM_AgtSftDbgMsgProc(XVOID*pMsg);
XS32 OAM_PmReportMsgProc(XVOID*pMsg);
XS32 OAM_AgtDataMsgProc(XVOID*pMsg, XU32 uiMsgId);
XS32 OAM_AgtRspMsgProc(XVOID*pMsg);
XS32 OAM_AppRspMsgProc(XVOID*pMsg);
XS32 OAM_SelfMsgProc(XVOID*pMsg);
XS8  OAM_AppRegMsgProc(XVOID* pRecvMsg, XVOID *pMsg,XU32 uiModuleId);
XS32 OAM_CfgMsgSend(XU32 uiModuleId, XU32 uiMsgId, XVOID*pBuffer, XU32 uiDataLen);
XS32 OAM_TransMsgSend(XVOID* pMsg, XU32 uiMsgId, XU32 uiDstFid);
XS32 OAM_AppRegister(XU32 uiModuleId, const XU32 *pTableId, XU32 uiTableNum);
XS32 OAM_OperMsgSend(XU32 uiModuleId, XU32 uiMsgId, XU32 uiMsgLen, XVOID*pBuffer);
XS32 OAM_AgtReqSend(XVOID* pMsg, XU32 uiMsgType, XU32 uiModuleId, XU32 uiTableId);
XS32 OAM_AgtSyncReqSend(XU32 uiMsgType, XU32 uiModuleId, XU32 uiTableId);
XVOID OAM_AgtOamDataNtoH(AGT_OAM_CFG_REQ_T* pAgtOamData);
XS32 OAM_HeadIdxGetBySid(XU32 uiSessionId, XU32* puiIndex);
XS32 OAM_HeadIndexGet(XU32* puiIndex);
XS32 OAM_TsNavMsgProc(XVOID*pMsg);
XS32 OAM_AgtFmtMsgProc();
XS32 OAM_TableIdFind(XU32 uiTableId, XU32* pModList,XU32* pCount);
XS32 OAM_TableIdAdd(XU32 uiTableId, XU32 uiModuleId);
XS32 OAM_MsgIdSend(XU32 uiModuleId, XU32 uiMsgId);
XS32 OAM_AsyncTableIdAdd(XU32 uiTableId, XU32 uiModuleId);
XS32 OAM_AlarmSend(XVOID* pMsg, XU32 uiAlarmClass, XU32 uiAlarmFlag);
XS32 OAM_AlarmLocInfoGet(XVOID*pMsg);
XVOID OAM_GetComPatcCurTime(XS8* pDst, XU32 uiDstLen);
XS32 OAM_AgtMsgSend(XVOID* pMsg, XU32 uiMsgLen);
XS32 OAM_AlarmRepeatSend();
XVOID OAM_AlarmMsgSave(XVOID* pMsg);
XVOID OAM_NeProcInfoProc(XVOID* pMsg, XU32 uiType, XU32 uiMsgLen);
XVOID OAM_HaInfoGet(OAM_HAINFO_T *pHaInfo);
XS32 OAM_SoftDebugRspSend(OAM_SOFTDEBUG_RSP_T* pMsg);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif
