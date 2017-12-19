#ifndef _COMMON_MSG_DEF_
#define _COMMON_MSG_DEF_

#ifdef __cplusplus
          extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
               包含头文件
-------------------------------------------------------------------------*/
#include "xostype.h"
#include "xosnetinf.h"
/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
#define MAX_WORKSPACE_NUM        2     /* 工作区个数 */
#define PROCESS_NAME_MAX_LEN     128   /* 进程名最大长度 */
#define PROCESS_MAX_NUM          8    /* 一个单板最大进程数 */

#define MD5_STRING_LEN           32    /* MD5字符串长度 32字节 */
#define MAX_STRING_LEN           128  /* 消息中字符串的最大长度 */
#define MAX_DISK_NUM             4    /* 硬盘最大个数 */
#define MAX_DISK_PART_NUM        12   /* 磁盘最大分区个数 */
#define MAX_DISK_NAME_LEN        32   /* 磁盘名称最大长度 */

#define MAX_CPU_CORE_NUM         64   /* cpu最大核数 */
#define MAX_VER_LEN              64   /* 版本信息字符串长度 */

#define XOS_MAC_SIZE             6    /* mac的字节个数 */


/*-------------------------------------------------------------------------
                数据结构定义
-------------------------------------------------------------------------*/

/* ----------------网管消息号定义 --------------------------*/
typedef enum
{
    MSG_GET_AGENTIP        = 0x10,   //获取Agent IP
    
    MSG_REQ_PROC_OPER      = 0x20,  //请求启动进程消息
    MSG_REQ_SYNC_PROCTBL   = 0x22,  //请求同步进程表消息   ---- 暂时不用

    MSG_CMD_PROC_OPRE      = 0x30,  //操作进程消息

    MSG_CMD_BOARD_RESET    = 0x40,  //重启单板消息
    MSG_CMD_POWERDONW      = 0x42,  //单板下电消息
    MSG_CMD_BOARD_DEL      = 0x44,  //删除单板
    
    MSG_REP_BOARDINFO      = 0x50,  //上报单板信息消息
    MSG_REP_PROCINFO       = 0x52,  //上报网元进程信息消息
    MSG_REP_NETIFINFO      = 0x54,  //上报网口信息消息
    MSG_REP_BOARDSTAT      = 0x56,   //上报单板状态  ----- 暂时不用
    MSG_REP_PROCSTAT       = 0x58,   //上报进程状态
    MSG_REP_NETIFSTAT      = 0x5a,   //上报网口link状态
    MSG_REP_DISKINFO       = 0x5c,   //上报磁盘信息
    MSG_REP_BOARD_HARDINFO = 0x5e,   //单次上报单板硬件信息

    MSG_QUE_BOARDINFO      = 0x60,   //查询单板信息消息
    MSG_QUE_PROCINFO       = 0x62,   //查询网元进程信息消息
    MSG_QUE_NETIFINFO      = 0x64,   //查询网口信息消息
    MSG_QUE_BOARDSTAT      = 0x66,   //查询单板状态  ----- 暂时不用
    MSG_QUE_PROCSTAT       = 0x68,   //查询进程状态
    MSG_QUE_NETIFSTAT      = 0x6a,   //查询网口link状态
    MSG_QUE_DISKINFO       = 0x6c,   //查询磁盘信息
    MSG_QUE_BOARD_HARDINFO = 0x6e    //查询单板硬件信息
}E_OAM_BM_MsgID;



/* 网口名称枚举见 xosenc.h --  e_NetPort */


/* 进程状态 */
enum
{
    E_PROC_EXIST = 0,     /* 进程在位 */
    E_PROC_NOT_EXIST,     /* 进程不在 */
    E_PROC_UNKNOWN        /* 状态未知，(获取状态失败) */
};

/* 网口工作模式 */
enum
{
    E_DUPLEX_FULL = 0,   /* 全双工 */
    E_DUPLEX_HALF,       /* 半双工 */
    E_DUPLEX_UNKNOWN
};
/* 网口速率 */
enum
{
    E_SPEED_10M = 0, 
    E_SPEED_100M,
    E_SPEED_1000M,
    E_SPEED_10000M, /* 万兆 */
    //E_SPEED_10G_OPT, /* 光口 */
    //E_SPEED_10G_SFP,  /* 电口 */
    E_SPEED_UNKNOWN
};

/* 网口连接状态 */
enum
{
    E_LINKUP = 0,   /* 网口连接状态up*/
    E_LINKDOWN,     /* 网口连接状态down */
};


/* 网口物理状态 */
enum
{
    E_NORMAL = 0,  /* 网口状态正常 */
    E_ABNORMAL,    /* 网口状态异常 */
};

/* 进程启动标志 */
enum
{
    E_PROC_STOP = 0,   /* 进程停止 */
    E_PROC_START,      /* 进程启动 */
    E_PROC_RESTART,    /* 进程重启 */
    E_PROC_DEL,        /* 进程删除 */
    E_PORC_UNKNOWN     /* 状态未知 */
};

/* 厂商名称定义 */
enum
{
    E_MFG_ADLINK = 0,    /* adlink */
    E_MFG_EMERSON,       /* emerson */
    E_MFG_UNNOWN
};

enum
{
    E_DISK_OK = 0,  /* 硬盘是正常的 */
    E_DISK_NOT_OK,  /* 硬盘不正常 */
};
//OAM配置操作类型枚举定义
typedef enum oam_cfg_e{
    OAM_CFG_ADD = 1, //增加
    OAM_CFG_DEL,     //删除
    OAM_CFG_MOD,     //修改
    OAM_CFG_GET,     //查询操作
    OAM_CFG_SYNC,    //同步
    OAM_CFG_FMT      //格式化
}OAM_CFG_E;

//网元类型枚举定义
typedef enum ne_type_e{
    NE_TYPE_MME = 1,
    NE_TYPE_TCF,
    NE_TYPE_TMG,
    NE_TYPE_SPGW,
    NE_TYPE_HSS,
    NE_TYPE_SMC,
    NE_TYPE_TAS,
    NE_TYPE_AGT,
    NE_TYPE_TCN, 
    NE_TYPE_OMS,
    NE_TYPE_DCS,
    NE_TYPE_ENB
}NE_TYPE_E;


//进程类型枚举定义
typedef enum proc_type_e{
    PROC_TYPE_MME = 1,
    PROC_TYPE_TCF,
    PROC_TYPE_TMG,
    PROC_TYPE_SPGW,
    PROC_TYPE_HSS,
    PROC_TYPE_SMC,
    PROC_TYPE_TAS,
    PROC_TYPE_AGT,
    PROC_TYPE_BM,
    PROC_TYPE_TS
}PROC_TYPE_E;
#pragma pack(1)

/* 网口ip配置信息 */
typedef struct
{
    XU32 uiIpaddr;
    XU32 uiNetmask; 
    XU32 uiGateway; 
    XU8  ucMacAddr[XOS_MAC_SIZE];
}t_IpAttr;

/* 网口统计信息 */
typedef struct
{
    XU32 uiRxBitRate;
    XU32 uiTxBitRate;
    XU32 uiRxPkgRate;
    XU32 uiTxPkgRate;
    XU64 u64RxPkg;
    XU64 u64TxPkg;
    XU64 u64RxByte;
    XU64 u64TxByte;
}t_Statstic;

/* 网口属性信息 */
typedef struct {
    XU8 ucPortType;   //网口名称的枚举值 e_NetPort
    XU8 ucNetInfState;
    XU8 ucLinkState;
    XU8 ucSpeedMode;
    XU8 ucWorkingMode;
}t_PortAttr;

/* 所有网口属性信息 */
typedef struct {
    XU8  ucPortsNum;  
    t_PortAttr  stPortAttr[MAX_NETPORT_NUM];
}t_PortAttrList;


/* 网口统计信息 */
typedef struct {
    t_PortAttr stAttr;
    t_IpAttr   stIpAddr;
    t_Statstic stStatis;
}t_PortStatis;

/* 所有网口统计信息 */
typedef struct {
    XU16 usSlotId;
    XU8  ucPortsNum;
    t_PortStatis  stPortStatis[MAX_NETPORT_NUM];
}t_PortStatisList;




/* 单个分区信息 */
typedef struct{
    XU8 ucUsedRatio;                  /* 分区使用率 */
    XU8 ucPartName[MAX_VER_LEN];      /* 分区名称 如 /var、/home */
    XU8 ucDevName[MAX_DISK_NAME_LEN]; /* 磁盘设备名称，如sda1 */
}t_DiskPart;

/* 磁盘状态 */
typedef struct {
    XU8 ucDiskState;    //磁盘状态
    XU8 ucDiskName[MAX_DISK_NAME_LEN];
}t_DiskInfo;

/* 磁盘统计信息 */
typedef struct {

    XU16 usSlotId;
    XU8 ucDiskNum;    /* 磁盘个数 */
    t_DiskInfo stDiskInfo[MAX_DISK_NUM];       /* 磁盘信息 */
    XU8 ucPartNum;    /* 分区个数 */
    t_DiskPart stPartInfo[MAX_DISK_PART_NUM];  /* 分区信息 */
}t_DiskStatisInfo;





/* 进程统计信息 */
typedef struct {
    XU16 usNetId;
    XU16 usProcId;
    XU8  ucProccessState;
    XU8  ucProcCpuUsedRatio;
    XU8  ucProcMemUsedRatio;
    XU32 uiProcRunTime; //秒数
}t_ProcessStatis;

/* 所有进程统计信息 */
typedef struct {
    XU16 usSlotId;
    XU8  ucProcNum;
    t_ProcessStatis stProcStatis[PROCESS_MAX_NUM];
}t_ProcessStatisList;



/* 上报单板统计信息 */
typedef struct {
    XU16 usSlotId;
    XU8  ucCpuUsedRatio;    
    XU8  ucCpuCores;
    XU8  ucCoreUsedRatio[MAX_CPU_CORE_NUM]; /* 所有核的cpu使用率 */
    XU8  ucMemUsedRatio; 
    XU64 u64SysStartTime;  //单位 秒数
    XU64 u64SysRunTime;    //单位 秒数
}t_BoardStatisInfo;





/* 上报单板硬件信息 */
typedef struct
{
    XU16 usSlotId;
    XU8  ucBoardType;
    XU8  ucCpuCoreNum;
    XS32 siMemSize;  /* 内存总大小，GB */
    XS32 siDiskSize; /* 磁盘总大小，GB */
    XS8  ucCpuType[MAX_STRING_LEN];
    XS8  ucSysVersion[MAX_VER_LEN];
    t_PortAttrList stPortAttrList;
}t_BoardInfo;





/* 进程状态消息 */
typedef struct {
    XU16 usSlotId;
    XU16 usNetId;
    XU16 usProcId;
    XU8 ucProccessState;
}t_ProcessState;

/* 所有进程状态 
typedef struct {
    XS32 iSlot;
    XS32 ProcNum;
    t_ProcessState tProcStat[PROCESS_MAX_NUM];
}t_ProcessStateList;
*/





/* 网口状态消息 */
typedef struct {
    XU16 usSlotId;
    XU8  ucPortType;
    XU8  ucNetInfState;
}t_NetPortState;

/* 所有网口状态 
typedef struct {
    XS32  iSlot;
    XS32  iPortsNum;  
    t_NetPortState  NetPortStat[MAX_NETPORT_NUM];
}t_NetPortStatList;
*/





/* 回复网管启动进程消息 */
typedef struct {
    XU16 usNeID;            /* 网元ID */
    XU16 usProcessId;       /* 网元逻辑进程ID */
    XS32 siRetVal;          /* 返回值 */
}t_ResProcOperMsg;

//进程信息结构
typedef struct {
    XU16  uwNeID;                              /* 网元ID */
    XU16  uwWorkspaceId;                       /* 工作区ID */
    XU16  uwProcLogicId;                       /* 网元逻辑进程ID */
    XU16  uwStartFlag;                         /* 进程启动/停止标志: 3删除进程2-重启1-启动 0-停止 */
    XU16  uwLowSlot;                           /* TS低槽位号*/
    XU16  uwHighSlot;                          /* TS高槽位号*/    
    XU32  uwNeType;                            /* 网元类型 */
    XCHAR  strPkgName[PROCESS_NAME_MAX_LEN];   /* 程序包名，进程文件名与此同名 */
    XCHAR  strMD5[MD5_STRING_LEN+1];           /* 进程版本包的md5 */
    //XU8  strVersion[128];                    /* 版本信息 */
}t_ProcessInfo;

//给BM下发操作网元进程（启动/停止）消息
typedef struct {
    XS32 uiProcessNum;                         /* 进程数 */
    t_ProcessInfo  tProcess[PROCESS_MAX_NUM];  /* 进程结构 */
    //t_BoardInfo stBoardInfo;                 /* 目标板信息 */ 
} t_NeOperMsg;


//OAM配置数据请求结构
typedef struct agt_oam_cfg_req_t{
    XU32 uiIndex;        //索引
    XU32 uiSessionId;    //会话ID
    XU16 usNeId;         //网元ID
    XU16 usPid;          //进程ID
    XU16 usModuleId;     //模块ID
    XU32 uiOperType;     //操作类型
    XU32 uiTableId;      //表ID
    XU32 uiRecNum;       //表记录数
    XU32 uiMsgLen;       //消息长度
    XS8 *pData;          //配置数据
}AGT_OAM_CFG_REQ_T;

//OAM配置数据响应结构
typedef struct agt_oam_cfg_rsp_t{
    XU32 uiIndex;        //索引
    XU32 uiSessionId;    //会话ID
    XU16 usNeId;         //网元ID
    XU16 usPid;          //进程ID
    XU16 usModuleId;     //模块ID
    XU32 uiOperType;     //操作类型
    XU32 uiTableId;      //表ID
    XU32 uiRecNum;       //表记录数
    XU32 uiRetCode;      //返回码
    XU32 uiMsgLen;       //消息长度
    XS8 *pRetData;       //返回数据
}AGT_OAM_CFG_RSP_T;

#pragma pack()

#ifdef __cplusplus
    }
#endif /* __cplusplus */

#endif /* _COMMON_MSG_DEF_ */ 
