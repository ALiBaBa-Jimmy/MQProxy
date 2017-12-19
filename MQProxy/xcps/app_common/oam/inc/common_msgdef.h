#ifndef _COMMON_MSG_DEF_
#define _COMMON_MSG_DEF_

#ifdef __cplusplus
          extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
               ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xostype.h"
#include "xosnetinf.h"
/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
#define MAX_WORKSPACE_NUM        2     /* ���������� */
#define PROCESS_NAME_MAX_LEN     128   /* ��������󳤶� */
#define PROCESS_MAX_NUM          8    /* һ�������������� */

#define MD5_STRING_LEN           32    /* MD5�ַ������� 32�ֽ� */
#define MAX_STRING_LEN           128  /* ��Ϣ���ַ�������󳤶� */
#define MAX_DISK_NUM             4    /* Ӳ�������� */
#define MAX_DISK_PART_NUM        12   /* �������������� */
#define MAX_DISK_NAME_LEN        32   /* ����������󳤶� */

#define MAX_CPU_CORE_NUM         64   /* cpu������ */
#define MAX_VER_LEN              64   /* �汾��Ϣ�ַ������� */

#define XOS_MAC_SIZE             6    /* mac���ֽڸ��� */


/*-------------------------------------------------------------------------
                ���ݽṹ����
-------------------------------------------------------------------------*/

/* ----------------������Ϣ�Ŷ��� --------------------------*/
typedef enum
{
    MSG_GET_AGENTIP        = 0x10,   //��ȡAgent IP
    
    MSG_REQ_PROC_OPER      = 0x20,  //��������������Ϣ
    MSG_REQ_SYNC_PROCTBL   = 0x22,  //����ͬ�����̱���Ϣ   ---- ��ʱ����

    MSG_CMD_PROC_OPRE      = 0x30,  //����������Ϣ

    MSG_CMD_BOARD_RESET    = 0x40,  //����������Ϣ
    MSG_CMD_POWERDONW      = 0x42,  //�����µ���Ϣ
    MSG_CMD_BOARD_DEL      = 0x44,  //ɾ������
    
    MSG_REP_BOARDINFO      = 0x50,  //�ϱ�������Ϣ��Ϣ
    MSG_REP_PROCINFO       = 0x52,  //�ϱ���Ԫ������Ϣ��Ϣ
    MSG_REP_NETIFINFO      = 0x54,  //�ϱ�������Ϣ��Ϣ
    MSG_REP_BOARDSTAT      = 0x56,   //�ϱ�����״̬  ----- ��ʱ����
    MSG_REP_PROCSTAT       = 0x58,   //�ϱ�����״̬
    MSG_REP_NETIFSTAT      = 0x5a,   //�ϱ�����link״̬
    MSG_REP_DISKINFO       = 0x5c,   //�ϱ�������Ϣ
    MSG_REP_BOARD_HARDINFO = 0x5e,   //�����ϱ�����Ӳ����Ϣ

    MSG_QUE_BOARDINFO      = 0x60,   //��ѯ������Ϣ��Ϣ
    MSG_QUE_PROCINFO       = 0x62,   //��ѯ��Ԫ������Ϣ��Ϣ
    MSG_QUE_NETIFINFO      = 0x64,   //��ѯ������Ϣ��Ϣ
    MSG_QUE_BOARDSTAT      = 0x66,   //��ѯ����״̬  ----- ��ʱ����
    MSG_QUE_PROCSTAT       = 0x68,   //��ѯ����״̬
    MSG_QUE_NETIFSTAT      = 0x6a,   //��ѯ����link״̬
    MSG_QUE_DISKINFO       = 0x6c,   //��ѯ������Ϣ
    MSG_QUE_BOARD_HARDINFO = 0x6e    //��ѯ����Ӳ����Ϣ
}E_OAM_BM_MsgID;



/* ��������ö�ټ� xosenc.h --  e_NetPort */


/* ����״̬ */
enum
{
    E_PROC_EXIST = 0,     /* ������λ */
    E_PROC_NOT_EXIST,     /* ���̲��� */
    E_PROC_UNKNOWN        /* ״̬δ֪��(��ȡ״̬ʧ��) */
};

/* ���ڹ���ģʽ */
enum
{
    E_DUPLEX_FULL = 0,   /* ȫ˫�� */
    E_DUPLEX_HALF,       /* ��˫�� */
    E_DUPLEX_UNKNOWN
};
/* �������� */
enum
{
    E_SPEED_10M = 0, 
    E_SPEED_100M,
    E_SPEED_1000M,
    E_SPEED_10000M, /* ���� */
    //E_SPEED_10G_OPT, /* ��� */
    //E_SPEED_10G_SFP,  /* ��� */
    E_SPEED_UNKNOWN
};

/* ��������״̬ */
enum
{
    E_LINKUP = 0,   /* ��������״̬up*/
    E_LINKDOWN,     /* ��������״̬down */
};


/* ��������״̬ */
enum
{
    E_NORMAL = 0,  /* ����״̬���� */
    E_ABNORMAL,    /* ����״̬�쳣 */
};

/* ����������־ */
enum
{
    E_PROC_STOP = 0,   /* ����ֹͣ */
    E_PROC_START,      /* �������� */
    E_PROC_RESTART,    /* �������� */
    E_PROC_DEL,        /* ����ɾ�� */
    E_PORC_UNKNOWN     /* ״̬δ֪ */
};

/* �������ƶ��� */
enum
{
    E_MFG_ADLINK = 0,    /* adlink */
    E_MFG_EMERSON,       /* emerson */
    E_MFG_UNNOWN
};

enum
{
    E_DISK_OK = 0,  /* Ӳ���������� */
    E_DISK_NOT_OK,  /* Ӳ�̲����� */
};
//OAM���ò�������ö�ٶ���
typedef enum oam_cfg_e{
    OAM_CFG_ADD = 1, //����
    OAM_CFG_DEL,     //ɾ��
    OAM_CFG_MOD,     //�޸�
    OAM_CFG_GET,     //��ѯ����
    OAM_CFG_SYNC,    //ͬ��
    OAM_CFG_FMT      //��ʽ��
}OAM_CFG_E;

//��Ԫ����ö�ٶ���
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


//��������ö�ٶ���
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

/* ����ip������Ϣ */
typedef struct
{
    XU32 uiIpaddr;
    XU32 uiNetmask; 
    XU32 uiGateway; 
    XU8  ucMacAddr[XOS_MAC_SIZE];
}t_IpAttr;

/* ����ͳ����Ϣ */
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

/* ����������Ϣ */
typedef struct {
    XU8 ucPortType;   //�������Ƶ�ö��ֵ e_NetPort
    XU8 ucNetInfState;
    XU8 ucLinkState;
    XU8 ucSpeedMode;
    XU8 ucWorkingMode;
}t_PortAttr;

/* ��������������Ϣ */
typedef struct {
    XU8  ucPortsNum;  
    t_PortAttr  stPortAttr[MAX_NETPORT_NUM];
}t_PortAttrList;


/* ����ͳ����Ϣ */
typedef struct {
    t_PortAttr stAttr;
    t_IpAttr   stIpAddr;
    t_Statstic stStatis;
}t_PortStatis;

/* ��������ͳ����Ϣ */
typedef struct {
    XU16 usSlotId;
    XU8  ucPortsNum;
    t_PortStatis  stPortStatis[MAX_NETPORT_NUM];
}t_PortStatisList;




/* ����������Ϣ */
typedef struct{
    XU8 ucUsedRatio;                  /* ����ʹ���� */
    XU8 ucPartName[MAX_VER_LEN];      /* �������� �� /var��/home */
    XU8 ucDevName[MAX_DISK_NAME_LEN]; /* �����豸���ƣ���sda1 */
}t_DiskPart;

/* ����״̬ */
typedef struct {
    XU8 ucDiskState;    //����״̬
    XU8 ucDiskName[MAX_DISK_NAME_LEN];
}t_DiskInfo;

/* ����ͳ����Ϣ */
typedef struct {

    XU16 usSlotId;
    XU8 ucDiskNum;    /* ���̸��� */
    t_DiskInfo stDiskInfo[MAX_DISK_NUM];       /* ������Ϣ */
    XU8 ucPartNum;    /* �������� */
    t_DiskPart stPartInfo[MAX_DISK_PART_NUM];  /* ������Ϣ */
}t_DiskStatisInfo;





/* ����ͳ����Ϣ */
typedef struct {
    XU16 usNetId;
    XU16 usProcId;
    XU8  ucProccessState;
    XU8  ucProcCpuUsedRatio;
    XU8  ucProcMemUsedRatio;
    XU32 uiProcRunTime; //����
}t_ProcessStatis;

/* ���н���ͳ����Ϣ */
typedef struct {
    XU16 usSlotId;
    XU8  ucProcNum;
    t_ProcessStatis stProcStatis[PROCESS_MAX_NUM];
}t_ProcessStatisList;



/* �ϱ�����ͳ����Ϣ */
typedef struct {
    XU16 usSlotId;
    XU8  ucCpuUsedRatio;    
    XU8  ucCpuCores;
    XU8  ucCoreUsedRatio[MAX_CPU_CORE_NUM]; /* ���к˵�cpuʹ���� */
    XU8  ucMemUsedRatio; 
    XU64 u64SysStartTime;  //��λ ����
    XU64 u64SysRunTime;    //��λ ����
}t_BoardStatisInfo;





/* �ϱ�����Ӳ����Ϣ */
typedef struct
{
    XU16 usSlotId;
    XU8  ucBoardType;
    XU8  ucCpuCoreNum;
    XS32 siMemSize;  /* �ڴ��ܴ�С��GB */
    XS32 siDiskSize; /* �����ܴ�С��GB */
    XS8  ucCpuType[MAX_STRING_LEN];
    XS8  ucSysVersion[MAX_VER_LEN];
    t_PortAttrList stPortAttrList;
}t_BoardInfo;





/* ����״̬��Ϣ */
typedef struct {
    XU16 usSlotId;
    XU16 usNetId;
    XU16 usProcId;
    XU8 ucProccessState;
}t_ProcessState;

/* ���н���״̬ 
typedef struct {
    XS32 iSlot;
    XS32 ProcNum;
    t_ProcessState tProcStat[PROCESS_MAX_NUM];
}t_ProcessStateList;
*/





/* ����״̬��Ϣ */
typedef struct {
    XU16 usSlotId;
    XU8  ucPortType;
    XU8  ucNetInfState;
}t_NetPortState;

/* ��������״̬ 
typedef struct {
    XS32  iSlot;
    XS32  iPortsNum;  
    t_NetPortState  NetPortStat[MAX_NETPORT_NUM];
}t_NetPortStatList;
*/





/* �ظ���������������Ϣ */
typedef struct {
    XU16 usNeID;            /* ��ԪID */
    XU16 usProcessId;       /* ��Ԫ�߼�����ID */
    XS32 siRetVal;          /* ����ֵ */
}t_ResProcOperMsg;

//������Ϣ�ṹ
typedef struct {
    XU16  uwNeID;                              /* ��ԪID */
    XU16  uwWorkspaceId;                       /* ������ID */
    XU16  uwProcLogicId;                       /* ��Ԫ�߼�����ID */
    XU16  uwStartFlag;                         /* ��������/ֹͣ��־: 3ɾ������2-����1-���� 0-ֹͣ */
    XU16  uwLowSlot;                           /* TS�Ͳ�λ��*/
    XU16  uwHighSlot;                          /* TS�߲�λ��*/    
    XU32  uwNeType;                            /* ��Ԫ���� */
    XCHAR  strPkgName[PROCESS_NAME_MAX_LEN];   /* ��������������ļ������ͬ�� */
    XCHAR  strMD5[MD5_STRING_LEN+1];           /* ���̰汾����md5 */
    //XU8  strVersion[128];                    /* �汾��Ϣ */
}t_ProcessInfo;

//��BM�·�������Ԫ���̣�����/ֹͣ����Ϣ
typedef struct {
    XS32 uiProcessNum;                         /* ������ */
    t_ProcessInfo  tProcess[PROCESS_MAX_NUM];  /* ���̽ṹ */
    //t_BoardInfo stBoardInfo;                 /* Ŀ�����Ϣ */ 
} t_NeOperMsg;


//OAM������������ṹ
typedef struct agt_oam_cfg_req_t{
    XU32 uiIndex;        //����
    XU32 uiSessionId;    //�ỰID
    XU16 usNeId;         //��ԪID
    XU16 usPid;          //����ID
    XU16 usModuleId;     //ģ��ID
    XU32 uiOperType;     //��������
    XU32 uiTableId;      //��ID
    XU32 uiRecNum;       //���¼��
    XU32 uiMsgLen;       //��Ϣ����
    XS8 *pData;          //��������
}AGT_OAM_CFG_REQ_T;

//OAM����������Ӧ�ṹ
typedef struct agt_oam_cfg_rsp_t{
    XU32 uiIndex;        //����
    XU32 uiSessionId;    //�ỰID
    XU16 usNeId;         //��ԪID
    XU16 usPid;          //����ID
    XU16 usModuleId;     //ģ��ID
    XU32 uiOperType;     //��������
    XU32 uiTableId;      //��ID
    XU32 uiRecNum;       //���¼��
    XU32 uiRetCode;      //������
    XU32 uiMsgLen;       //��Ϣ����
    XS8 *pRetData;       //��������
}AGT_OAM_CFG_RSP_T;

#pragma pack()

#ifdef __cplusplus
    }
#endif /* __cplusplus */

#endif /* _COMMON_MSG_DEF_ */ 
