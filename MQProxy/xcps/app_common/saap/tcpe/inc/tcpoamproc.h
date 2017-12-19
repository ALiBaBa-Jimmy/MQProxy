/*----------------------------------------------------------------------
    tcpOamProc.h - �ֲ��ꡢ���͡�����

    ��Ȩ���� 2004 -2006 ������˾������SAG��Ŀ��.

    author: �ź�

    �޸���ʷ��¼:wangcaiyu
    �޸���ʷ��¼:zengguanqun 2008-10-08
    --------------------
    ��Ӱ�Ȩ˵��.
----------------------------------------------------------------------*/

#ifndef _TCPOAMPROC_H_
#define _TCPOAMPROC_H_

#include "oam_public_data_def.h"
#include "oam_tab_def.h"
#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

//------------------------�궨��------------------------------------
#define SAG_PERFORM_STAT_INCLUDE 1
#define  TCP_LINT_MAX       MAX_TCPEN_LINK_NUM

/* fsm status */
#define   LINT_CCB_NULL                          0   // ��
#define   LINT_CCB_USER                          1   // ��ʹ��

//��ʱ����,����ʼƫ��800,����Ĭ��ֵ
#define   TCP_HAND_TIMER                         801   // ��·����
#define   TCP_TIMER_INIT                         901
#define   TCP_TIMER_START                        902
//#define   TCP_LINK_INIT                          802   //��ʼ��
//#define   TCP_LINK_START                         803   // ����

#define   TCP_TWO_SECOND                         2000   // 2 ��

//------------------------�ṹ�͹����嶨��----------------------------
XEXTERN XU8 g_Tcp_LinkHand;      // 1 ����, 0 ������
XEXTERN XU8 g_Tcp_Trace;         // 1 ����, 0 ������

XEXTERN t_TCPESTAT  gTcpeStaData;

#define TCPE_LINK_DESC_LEN  32

//����agent����ʱʹ�ô�ѡ����
#if 0
typedef enum
{
    TCPE_LOCOL_DP_TABLE = 1,
    TCPE_TCP_CFG_TABLE = 2, //tcpe��·����
    TCPE_CFG_TABLE_BUTT
}TCPE_TABLE;
#endif

#if 1
typedef enum
{
    TCPE_LOCOL_DP_TABLE = xwTcpeLocalTable,
    TCPE_TCP_CFG_TABLE  = xwTcpeLinkTable, //tcpe��·����
    TCPE_CFG_TABLE_BUTT
}TCPE_TABLE;
#endif

// TCP/IP��װ����·���ݽṹ
typedef struct _tagTcpLintCcb
{
    int i;

    XU16                  usIndex ;       // ����ֵ
    XU8                   ucFlag;         // 0 Ϊ��

    XU8                   ucModel ;       // ����ģʽ
    XU8                   ucHandState;        // ״̬ �����Ƿ�����

    MSG_OWNER_SAAP        stUser;         // ҵ��ʹ����(����IP���ͨ��)
    MSG_OWNER_SAAP        stProvider;     // ҵ���ṩ��(����IP���ͨ��)
    SYS_IP_ADDR           stPeerAddr;     // �Զ˵�IP��ַ
    SYS_IP_ADDR           stMyAddr;       // �Լ���IP��ַ

    XU16                  DstDeviceID ;   // Ŀ����������

    //�������������ݽṹ
    PTIMER                htTm;           // ��ʱ�����
    PTIMER                linkTm;           // ��ʱ�����
    XU16                  hankCount;      // ���ּ�������ÿ����һ������ + 1���յ�һ��Ӧ�� -1

    XU16                  bufLen ;                        // ��¼������ĳ���
    XU8                   ucBuffer[ MAX_IP_DATA_LEN ];    // ������Ϣ������

    //add by wangcaiyu 2006-11-29
    XU8                  ucType;                         // Ŀ������������ 0 SSTP 1 HLR 2 MCBTS
    XU16                 usNatLinkIdx;                   // NATģ��������·������

    XU8                  ucArrLinkDesc[TCPE_LINK_DESC_LEN]; //32 +1
    XU32                 ulLinkStatus;                      //TCP����״̬
    XU32                 ulAppHandle;
    XU32                 ulLinkHandle;
    XS32                 ulDefultLinkFlag;
    //end by wangcaiy*/

}TCP_LINT_CCB;

// TCP/IP��װ����·���ݽṹ
typedef struct _tagTcpLintCcbIndex
{

    XU16                    GlobalDstID;           // ������������
    XU16                    ProIndex;              // Ĭ����һ���ڵ�����
    XU16                    cout;                  // ��ǰ����·��
    TCP_LINT_CCB            lintCcb[TCP_LINT_MAX];

}TCP_LINT_CCB_INDEX;

/*���������*/
typedef struct _tagTcpLocaoDp
{
    XS32 LocolDp;//8888
}T_TCPE_LOCOLDP_TBL;

//TCPE��·����
typedef struct _tagTcpeTcpTbl
{
    XS32     ulInkIndex;        //��·����
    XS32     DstDpId;           //Ŀ�������
    XS32     connectType;       // 2-�ͻ���,3-������
    XS32     localIp;           //����ip
    XS32     localPort;
    XS32     remoteIp;
    XS32     remotePort;
    XS32     defaultLinkFlag;
    XU8      szLinkDesc[TCPE_LINK_DESC_LEN*2];
    XS32     linkStatus;
}T_TCPE_LINK_TBL;

//tcp��·״̬
enum TCPE_TCP_LINK_STATE
{

    TCPE_TCP_LINK_STATE_NULL        =0,        //δ����
    TCPE_TCP_LINK_STATE_INIT        =1,        //��ʼ��
    TCPE_TCP_LINK_STATE_START       =2,        //����
    TCPE_TCP_LINK_STATE_BUILD       =3,        //����

    TCPE_TCP_LINK_STATE_STOP        =4,        //ֹͣ

    TCPE_TCP_LINK_STATE_INIT_FAIL   =5,    //��ʼ��ʧ��
    TCPE_TCP_LINK_STATE_START_FAIL  =6,    //����ʧ��
    TCPE_TCP_LINK_STATE_LISTEN      =7,        //�������� listen
    TCPE_TCP_LINK_STATE_CONNING     =8,       //�ͻ�����������
    TCPE_TCP_LINK_STATE_BUTT
};

enum TCPE_TCP_ALARM_STATE
{
    TCPE_TCP_ALARM_STATE_CON       =0,         //���ӽ���
    TCPE_TCP_ALARM_STATE_DISC      =1,         //����δ����

    TCPE_TCP_ALARM_STATE_BUTT
};

#ifdef SCALE_CPU_VX
/*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

// TCP lint ��·��ʼ��
XS32 tcpe_link_inittable( void );

// �˺�����ɱ��������������� , ע TCPECAP ��ֻ��������㣬�豸IDΪGT�룬��SAAP�㴦��
XU32 tcp_SetLocalDstID(XU16 LocId );

// �˺��������һ������������
XU32 tcp_SetParentDstIDIndex(XU16 index );

XS32 TCP_command_init();

void TcpTblRegToOAM(XU32 uiModId);
XU8  TcpeOamCallBack(XU32  uiTableId, XU16 usMsgId, XU32 uiSequence,XU8 ucPackEnd, tb_record *ptRow);

//tcpe localDp table oam msg process
XS32 Tcpe_oamLocalDpTableOpr(XU16 usMsgId, T_TCPE_LOCOLDP_TBL* pTcpTbl, char* mask);
XS32 Tcpe_oamSyncLocalDpTable(T_TCPE_LOCOLDP_TBL * pLocalDpTbl);
XS32 Tcpe_oamUpdateLocalDpTable(T_TCPE_LOCOLDP_TBL * pLocalDpTbl, char* mask);

//tcpe table oam msg process
XS32 Tcpe_oamLinkTableOpr(XU16 usMsgId, T_TCPE_LINK_TBL* pTcpTbl, char* mask);
XS32 Tcpe_oamSyncLinkTable(T_TCPE_LINK_TBL* pTcpTbl);
XS32 Tcpe_oamInsertLinkTable(T_TCPE_LINK_TBL* pTcpTbl);
XS32 Tcpe_oamUpdateLinkTable(T_TCPE_LINK_TBL* pTcpTbl, char* mask);
XS32 Tcpe_oamDeleteLinkTable(T_TCPE_LINK_TBL * pTcpTbl);
XS32 Tcpe_oamGetLinkTableStatus(T_TCPE_LINK_TBL * pTcpTbl);

//link maintenance
XS32 tcpe_link_delete(XU32 ulLinkIndex);
XS32 tcpe_link_clearstatus(XU32 ulLinkIndex);
XS32 tcpe_link_setstatus(XU32 ulLinkIndex,XS32 linkStatus);
XS32 tcpe_link_clearbuff(XU32 ulLinkIndex);
XS32 tcpe_link_clearstat(int link_index);

//link alert ntl msg trigger
XS32 tcpe_link_init(XU32 ulLinkIndex);
XS32 tcpe_link_initack( t_LINKINITACK* pLinkInitAck);
XS32 tcpe_link_startack(t_STARTACK* pLinkStartAck);
XS32 tcpe_link_errorsend(t_SENDERROR* pIpSendErrMsg);
XS32 tcpe_link_start(XU32 ulLinkIndex );
XS32 tcpe_link_release(XU32 ulLinkIndex);

XS32 tcpe_link_alarm(XU8 ucAlarmState, XS32 ulLinkIndex);
XVOID Tcp_cmdShowUserFid(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
XVOID Tcp_cmdSetLocalTestFlag(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
extern XU16 TCPE_GetLocalDpId();
#ifdef  __cplusplus
}
#endif

#endif


