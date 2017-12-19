/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     oam_install_main.h
* Author:       xhm
* Date：        2014-09-02
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
#define OAM_SEND_MAX_NUM        32      //OAM发送最大次数

#define MAX_TB_NUM              200     //最大表ID数
#define MAX_SLOT_ID             14      //最大槽位号
#define HA_IP_NUM               2       //主备IP数
#define NE_PROCESS_MAX_NUM      16      //网元进程最大数
#define DESC_INFO_LEN           128     //描述信息长度


#define OAM_SESSION_MAX_NUM     128     //会话最大数
#define OAM_SESSION_HEAD_UNUSED   0     //会话头未使用
#define OAM_SESSION_HEAD_USED     1     //会话头已使用

#define AGENT_IP_TABLE_ID       500     //agentip 表ID(1条记录为agentIP,另外一条为前插板IP)
#define TID_MODULE_MANAGE       505     //进程表ID

#define OAM_MSGTYPE_RESPONSE    0x200   //OAM响应消息ID 
#define OAM_MSGTYPE_SYNC        0x201   //OAM向agent获取配置数据消息ID
#define OAM_MSGTYPE_NOTIFY      0x203   //OAM(BM进程)状态上报消息

//定义表相关字段
#define OAM_TABLE_MAX_NUM       200     //表的最大个数
#define FILE_PATH_MAX_LEN       128     //文件路径的最大长度
#define VALUE_MAX_LEN           8       //临时字段长度
#define BUFF_MAX_LEN            4096    //临时缓冲区长度
#define TID_PROCESS_MANAGE      505     //进程表ID

//业务模块注册消息ID定义
#define OAM_REG_TALBEID         1       //注册表ID
#define APP_REGISTER_MSG        7       //OAM识别应用模块SYNC的消息
#define OAM_OFFSET_LEN          2       //偏移位数

//Agent消息ID定义
#define AGENT_CFG_MSG           0x100   //Agent配置消息
#define AGENT_OPER_MSG          0x101   //Agent操作(单板、网元、进程等)消息
#define AGENT_SYNC_MSG          0x102   //Agent同步消息
#define AGENT_FMT_MSG           0x103   //Agent格式化消息
#define AGENT_DEBUG_REG_MSG     0x104   //Agent软调/维护请求消息
#define AGENT_DEBUG_RSP_MSG     0x105   //Agent软调/维护响应消息

//与TS通信消息ID
#define OAM_MSG_IP_REQ          0x88    //TS获取前插板IP消息
#define OAM_MSG_IP_RSP          0x89    //OAM回TS IP消息响应
#define OAM_MSG_NAV_INFO        0x90    //OAM更新导航树信息消息

//定义性能相关定义
#define OAM_PM_MAX_NUM          200     //网元性能测量指标的最大个数
#define OAM_PM_NOTIFY_MSG       0x300   //PM性能上报消息ID
#define PM_IDENTITY_LEN         32      //话统唯一标识长度


//定义告警相关定义
#define OAM_FM_NOTIFY_MSG       0x500   //告警上报消息ID
#define FM_ITEM_MAX_NUM         8       //告警定位信息最大数
#define FM_INFO_VAL_LEN         64      //告警定位信息
#define FM_ALARM_MAX_LEN        4096    //当前告警最大记录
#define FM_TYPE_NUM             2       //告警类型最大数
#define FM_MSG_SAVE_NUM         200     //告警消息缓存最大数

//软调相关
#define OAM_PARA_MAX_NUM        32      //软调参数最大数
#define OAM_PARA_NAME_LEN       8       //软调参数名称最大长度
#define DEBUG_RSP_MAX_LEN       4096    //软调响应数据最大长度

//特定返回码
#define OAM_ERRORCODE_UNEXPECTED_DATA  0x100  //进程不需要返回数据错误码
/*设置t_XOSCOMMHEAD消息头信息*/
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


/*释放指针内存*/
#define PTR_MEM_FREE(fid, ptr)      \
do                                  \
{                                   \
    if(XNULL != ptr)                \
    {                               \
        XOS_MemFree(fid, ptr);      \
        ptr = XNULL;                \
    }                               \
}while(0)


/* 检查函数的指针参数是否为空 */
#define PTR_NULL_CHECK(ptr,rtVal)     \
do                                    \
{                                     \
    if(NULL == ptr)                   \
    {                                 \
        XOS_PRINT(MD(FID_OAM,PL_ERR), "prt is null!\r\n");\
        return rtVal;                 \
    }                                 \
}while(0)


//告警类别枚举定义
typedef enum fm_class_e{
    FM_CLASS_ALARM = 1,   //故障告警
    FM_CLASS_EVENT,       //事件告警
}FM_CLASS_E;

//网元告警类型枚举定义
typedef enum fm_type_e{
    FM_TYPE_SOFT = 1,     //软件告警
    FM_TYPE_HARD,         //硬件告警
    FM_TYPE_SERVICE,      //业务告警
    FM_TYPE_LINK,         //链路告警
}FM_TYPE_E;

//告警级别枚举定义
typedef enum fm_level_e{
    FM_LEVEL_CRITICAL = 1,//紧急告警
    FM_LEVEL_MAJOR,       //重要告警
    FM_LEVELE_MINOR,      //次要告警
    FM_LEVEL_HINT,        //提示告警
}FM_LEVEL_E;


//OAM告警类型枚举定义
typedef enum alarm_type_e{
    ALARM_TYPE_CUR = 0,   //当前告警
    ALARM_TYPE_HISTORY,   //历史告警
}ALARM_TYPE_E;

    
//告警标志枚举定义
typedef enum fm_flag_e{
    FM_FLAG_RECOVER = 0,  //告警恢复
    FM_FLAG_NOTIFY,       //告警上报
}FM_FLAG_E;
    
typedef struct oam_tid_mid_t
{
    XU32 uiTableId;   //表ID
    XU32 uiModuleId;  //模块ID
}OAM_TID_MID_T;

//TS导航树节点信息
typedef struct ts_nav_info{
    XU16 usNeType;                 //网元类型
    XU16 usNeId;                   //网元ID
    XU16 usDescLen;                //网元描述长度
    XS8 ucDesc[FILE_PATH_MAX_LEN + 1]; //网元描述
    XU16 usLogPid;                 //逻辑进程ID
    XU16 usModuleType;             //进程类型
    XU16 usSlotId;                //槽位号
}TS_NAV_INFO;


//导航树消息
typedef struct ts_cfg_req_t{
    XU32 uiPidNum;                 //进程ID数量
    XU32 uiLen;                    //指针总长度      
    TS_NAV_INFO *pNavData;         //导航树节点
}TS_CFG_REQ_T;


//导航树消息
typedef struct ts_ip_info_t{
    XU32 uiIpAddr;                 //前插板IP
    XU32 uiMslotId;                //主槽位号   
    XU32 uiSslotId;                //备槽位号
}TS_IP_INFO_T;

/*消息头定义*/
typedef struct agent_oam_cfg_head_t
{
    XU32 uiMsgId;     //消息ID,表示网元间还是网元内部
    XU32 uiMsgType;   //消息类型
    XU16 usNeId;      //网元ID
    XU16 usPid;       //进程ID
    XU32 uiTransId;   //传输ID
    XU32 uiSessionId; //会话ID
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


//网元上报给OAM数据结构
typedef struct pm_data_t
{
    XU32 uiPmId;        //测量指标ID
    XU32 uiValue;       //测量指标值
}PM_DATA_T;


//网元上报给OAM数据结构
typedef struct pm_report_t
{
    XU64 uiCurTime;     //当前上报时间("yyyymmddHHMMSS"格式)
    XU32 uiClassId;     //类ID 对应文档的measureobj id
    XU32 uiInstanceId;  //实例ID,如S1链路ID=1的话统数据
    XU32 uiCycle;       //循环周期,对应文档的mincycle
    XS8  ucIdentity[PM_IDENTITY_LEN];  //唯一标识,对应文档的identity
    XU32 uiNum;         //指标数
    PM_DATA_T stPmData[OAM_PM_MAX_NUM]; //性能测量数据
}PM_REPORT_T;


//OAM发送给agent数据结构
typedef struct oam_pm_report_t
{
    XU16 usNeId;        //网元ID
    XU16 usPid;         //进程ID
    PM_REPORT_T stPmReport; //性能测量上报结构
}OAM_PM_REPORT_T;

//告警相关结构定义
//告警位置信息结构定义
typedef struct oam_loc_info_t
{
    XU16 usRackId;
    XU16 usFrameId;
    XU16 usSlotId;
    XU16 usNeId;
    XU16 usProcId;
}OAM_LOC_INFO_T;

//告警定位信息结构定义
typedef struct fm_data_t
{
    XU16 usFieldId;              //field ID
    XU16 usValLen;               //数据长度
    XU8 ucData[FM_INFO_VAL_LEN]; //告警定位信息数据
}FM_DATA_T;

//告警参数信息结构定义
typedef struct fm_para_t
{
    XU32 uiAlarmId;      //告警ID
    XU64 ulAlarmSeq;     //告警流水号
    XU8 ucDataNum;       //定位信息数
    FM_DATA_T stFmData[FM_ITEM_MAX_NUM];  //告警定位信息
}FM_PARA_T;

//告警信息结构定义
typedef struct fm_alarm_t
{
    XU32 uiAlarmClass;         //告警类别 0:故障告警 1:事件告警
    XU8 ucAlarmFlag;           //告警标志 0:告警恢复 1:告警上报
    XU64 ulAlarmTime;          //告警产生时间
    OAM_LOC_INFO_T stFmLocInfo;//告警位置信息
    FM_PARA_T stFmPara;        //告警参数信息
}FM_ALARM_T;


//告警信息结构定义
typedef struct fm_agt_alarm_t
{
    XU32 uiAlarmSeq;          //告警流水号
    FM_ALARM_T stFmAlarm;     //网元告警信息
}FM_AGT_ALARM_T;


//告警查询信息结构定义
typedef struct fm_alarm_lst_t
{
    XU32 uiAlarmNum;           //告警数
    FM_AGT_ALARM_T stAgtFmAlarm[FM_ALARM_MAX_LEN];//告警信息
}FM_ALARM_LST_T;


//告警信息缓存结构定义
typedef struct fm_alarm_save_t
{
    XU16 usHead;          //头位置
    XU16 usTail;          //尾位置
    FM_ALARM_T stFmAlarm[FM_MSG_SAVE_NUM];  //网元告警信息
}FM_ALARM_SAVE_T;


//告警信息缓存结构定义
typedef struct oam_hainfo_t
{
    XU16 usSlotId;                 //槽位号
    t_IPADDR stIpAddr[HA_IP_NUM];  //IP信息
}OAM_HAINFO_T;

//进程信息管理结构定义
typedef struct module_manage_t{
    XU16 usNeId;                   //网元ID
    XU16 usProcId;                 //进程ID
    PROC_TYPE_E eProcType;         //进程类型
    XU16 usFrameId;                //机框号
    XU16 usMSlotId;                //主槽位号
    XU16 usSSlotId;                //备槽位号
    XS8 ucDesc[DESC_INFO_LEN];     //进程描述
    XU8 ucStatus;                  //进程状态
}MODULE_MANAGE_T;


typedef struct ne_mdu_t{
    XU32 uiProcNum;                 //进程数
    MODULE_MANAGE_T stMduInfo[NE_PROCESS_MAX_NUM];   //进程信息
}NE_MDU_T;



//参数属性结构定义
typedef struct para_property_t{
    XU8 ucName[OAM_PARA_NAME_LEN];//字段名
    XU8 ucVal[DESC_INFO_LEN];    //字段值
}PARA_PROPERTY_T;

//维护命令请求结构
typedef struct oam_softdebug_req_t{
    XU32 uiFieldNum; //软调参数个数
    PARA_PROPERTY_T stParaProperty[OAM_PARA_MAX_NUM];    //软调参数结构信息
}OAM_SOFTDEBUG_REQ_T;

//维护命令响应结构
typedef struct oam_softdebug_rsp_t{
    XU32 uiMsgLen; //消息长度
    XU8 ucRspVal[DEBUG_RSP_MAX_LEN];    //软调响应数据值
}OAM_SOFTDEBUG_RSP_T;

//OAM维护命令结构
typedef struct oam_agt_sftdbg_rsp_t{
    OAM_LOC_INFO_T stLocInfo;           //位置信息
    OAM_SOFTDEBUG_RSP_T stRspInfo;      //软调响应信息
}OAM_AGT_SFTDBG_RSP_T;

//OAM维护命令结构
typedef struct oam_agt_sftdbg_req_t{
    XU32 uiModuleId; //软调参数个数
    OAM_SOFTDEBUG_REQ_T stSftDbgReq;      //软调请求信息
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
