/*----------------------------------------------------------------------
    saapMain.h - ȫ�ֺ������Ͷ���

    ��Ȩ���� 2004 -2006 ������˾������SAG��Ŀ��.

    author: �ź�

    �޸���ʷ��¼
    --------------------
    ��Ӱ�Ȩ˵��.
----------------------------------------------------------------------*/

#ifndef _SAAPMAIN_H_
#define _SAAPMAIN_H_

#include "saappublichead.h"

#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

//------------------------�궨��------------------------------------

    #define SAAP_TIMER_NUM          3
    #define SAAP_TIMER_TEST         0 //���Զ�ʱ��

    #define SAAP_G_DN_GT            0x12
    #define SAAP_G_EID_GT           0x82
    #define SAAP_G_UID_GT           0x92

    #define SAAP_REG_TIMER_NUM      512      // ע������ʱ����Ŀ

    // hand ������
    #define SAAP_USER_TYPE          0x1100   // TCP/IP �е�SAAP�û�����
    #define SAAP_ROUTER_MAX_NUM     32       // ������󾭹��ҽڵ���

    /* fsm status */
    #define   SAAP_LINT_RUNING      0        // ����
    #define   SAAP_LINT_FAIL        1        // ��ͨ

    #define SAAP_GUEST_TAG          0x20     // Ŀ�ı�ǩ
    #define SAAP_INVITE_TAG         0x21     // Դ��ǩ

    // OAM ���ú�
    #define   SAAP_OAM_CF_SET_GT_INFO         0xC005        // ����HLR GT����
    #define   SAAP_OAM_CF_DEL_GT_IFNO         0x0F          // ɾ��HLR GT����

    #define   SAAP_OAM_CF_SET_EID_ROUTE       0xC00C        // �����豸��·��ѡ��
    #define   SAAP_OAM_CF_DEL_EID_ROUTE       0xC00D        // ɾ���豸��·��ѡ��
    #define   SAAP_OAM_CF_SET_HLRNO_ROUTE     0xC00E        // ����SHLR_NO·��ѡ��
    #define   SAAP_OAM_CF_DEL_HLRNO_ROUTE     0xC00F        // ɾ��SHLR_NO·��ѡ��
    #define   SAAP_OAM_CF_SET_TEL_ROUTE       0xC010        // ���ú������·��ѡ��
    #define   SAAP_OAM_CF_DEL_TEL_ROUTE       0xC011        // ɾ���������·��ѡ��
    #define   SAAP_OAM_CF_ADD_TCP_LINK        0xC012        //����TCP��·
    #define   SAAP_OAM_CF_DEL_TCP_LINK        0xC013        //ɾ��TCP��·

    #define   SAAP_OAM_CF_HLRNO_FLAG          0x00     // hlr no ��Ӧ����UID
    #define   SAAP_OAM_CF_DN_FLAG             0x01     // dn     ��Ӧ�绰����

    // ·���뱾�ص��ú�
    #define   SAAP_INVIKE_LOCAL       0x01      // ���ص���
    #define   SAAP_INVIKE_ROUTE       0x02      // ·�ɵ���

typedef enum
{
    SAAP_ERR_NOT_FIND_UID_ROUTE = 1,
    SAAP_ERR_MAX
}SAAP_ERR_E;

    
//�ӿ�Э�鴦�������������ݽṹ
    typedef struct  APP_OR_SAAP_COMMON_Header
    {
        COMMON_HEADER_SAAP          com;            // ��Ϣͷ
        APP_AND_SAAP_PACKAGE_HEAD   appSaapHead;    //
        XU8                         ucBuffer[1];
    }APP_OR_SAAP_COMMON_HEADER;

    typedef struct  SAAP_AND_TCP_PACKAGE_Head
    {
        XU16        DstDeviceID;        /*Ŀ���������*/
        XU16        InviteDeviceID;     /*Դ���������*/
        XU16        UserType;           /*�û�����*/
        XU16        RouterNum ;         /*·�ɴ���*/
        XU16        MsgLen;             /*�û�����*/
    }SAAP_AND_TCP_PACKAGE_HEAD; //9999 ���ĵ��в��,������ǰ�涨��

    // SAAP �� TCP/IP ��װ��Ľӿڰ�
    typedef struct SAAP_AND_TCP_MessagPBuf
    {
        COMMON_HEADER_SAAP           com;
        SAAP_AND_TCP_PACKAGE_HEAD    tcpHead;
        XU8                          ucBuffer[1];
    } SAAP_AND_TCP_MSG_BUF;

    // SAAP �� TCP/IP ��װ������ְ��ṹ
    typedef struct SAAP_HAND_TCP_MessagPBuf
    {
        COMMON_HEADER_SAAP           com;
        SAAP_AND_TCP_PACKAGE_HEAD    tcpHead  ;
        XU8                          UserType ;     // �û�����
        XU8                          ucBuffer[1];   // �����û�����������
    } SAAP_HAND_TCP_MSG_BUF;

    typedef struct SAAP_Dn_Buf
    {
        XU8                          CellNum ;      // ���Ÿ���
        XU16                         Cell ;         // ����
        XU8                          TellNum ;      // �绰����
        XU32                         Tell ;         // ����
    } SAAP_DN_MSG_BUF;

/* shlrHEADER�ṹ*/
typedef struct
{
    t_XOSCOMMHEAD             MsgHead;
    APP_AND_SAAP_PACKAGE_HEAD SaapHead;
    XU32                      ulGtValue;
    XU8                       DN[8];
}SAAP_MSG_TEST_MSG;

//saap ��Ϣͳ�ƽӿ�
typedef struct
{
    XU32 ulLinkFailNum;       /* �յ�TCPE ����ʧ��֪ͨ�Ĵ���*/
    XU32 ulRecvMsgCnt[10];

    XU32  ulSendMsgCnt[10];    /*  SAAP�㷢����SMAP��Ϣ*/
    XU32  ulRouteMsgCnt;      /*  SAAP��·�ɵ���Ϣ */
}t_SAAPSTAT;

#define MAX_UID_SEG_NUM 128
typedef struct
{
    XU8 ucFlag;
    XU32 ulUidStart;
    XU32 ulUidEnd;
    XU16 dstDpID;
}SAAP_UID_SEG_T;

typedef struct
{
    XU32 ulUidSegRouteFlag; /*0-none, 1-cfg form File, 2-cfg from nms*/
    XU32 ulUidSegNum;
    SAAP_UID_SEG_T stUidSeg[MAX_UID_SEG_NUM];
}SAAP_UID_SEG_CFG;

#ifdef SCALE_CPU_VX
/*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

    //------------------------��������------------------------------------
    XS8 SAAP_InitProc(XVOID *Para1, XVOID *Para2 );
    XS8 SAAP_MsgProc(XVOID *msg, XVOID *Para);
    XS8 SAAP_TimeoutProc(t_BACKPARA  *pstPara);

    void  SAAP_TimeInit(void);           //��ʱ����ʼ��
    void  SAAP_TimeStart(void);          //����ʱ��

    XS32  saap_local_Init(void);         //saap ��ʼ��

    XS32  svc_to_saap_data_proc( COMMON_HEADER_SAAP *pMsg , XU8 flag ) ;     // ����ҵ����SAAP����Ϣ, flag ��ʾΪ·�ɻ��Ǳ���

    XS32  ip_to_saap_data_proc( COMMON_HEADER_SAAP *pMsg ) ;     // ����IP ����SAAP����Ϣ

    // �˺������ҵ����SAAP�ṹ���׼SAAP���ṹ��ת������
    XU32  saap_localID_to_globalID( XU8 *pSaapHead , APP_AND_SAAP_PACKAGE_HEAD *pAappSaapHead  );

    XU32  saap_globalID_to_localID( APP_AND_SAAP_PACKAGE_HEAD *pAappSaapHead  , XU8 *pSaapHead  );

    // �˺������ҵ����SAAP�ṹ���׼SAAP���ṹ��ת������
    void  saap_GT_to_BCD( XU8 SaapHeadBCD[] , XU32 GtID  );
    XU32  saap_BCD_to_GT(XU8 flag , XU8 SaapHeadBCD[]  );

    // �˺�������GT������������
    XU16  saap_DstID_from_Gt(  struct Gst gstVale ,XU32 ProtocolTag ) ;

    // �˺������ڱȶ�GT�Ŷ��Ƿ�Ϊ�����Ŷ�
    XU16 saap_check_no_proc(XU32 proTag, XU8 flag , XU16 hlrno, XU32 dnno) ;
    XS32  saap_regCmd();
    XVOID saap_showsaapinfo(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_setlocalep(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_addeidroute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_adduidroute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_addtellnoroute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_delEidRoute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_delUidRoute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_delTellNoRoute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);

    XVOID SaapTest(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_setlocaluidprefix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_setlocaltellnoprefix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_deleid(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_deluidprifix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_deltellnoprefix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_statclear(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
    XVOID saap_showstat(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
    XVOID saap_setstatswitch(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
    XVOID saap_StartTestTimer(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_StopTestTimer(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_SetTestData(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XS32 SaapPeriodTest(XU32 numPerSecond);
    XVOID saap_showlocalinfo(CLI_ENV* pCliEnv);
    XVOID saap_showeidroute(CLI_ENV* pCliEnv,XU32 ulFlag);
    XVOID saap_showhlrroute(CLI_ENV* pCliEnv,XU32 ulFlag);
    XVOID saap_showsmcroute(CLI_ENV* pCliEnv,XU32 ulFlag);
    XS32 SaapSendTestMsg(XU32 ProTag, XU32 routeType,XU32 gtValue,XU8 tellNo[8]);
    void SaapTblRegToOAM(XU32 uiModId);
    XVOID saap_CliSetFilter(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_CliClsFilter(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_CliShowFilter(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    //
    XU16  saap_getDstID_from_UidSeg(XU32 ulUid);
    XS32 saap_SetUidSegRouteFlag(XU32 ulFlag);
    XS32 saap_addUidSegRoute(XU32 ulUidStart,XU32 ulUidEnd,XU16 dstDp);
    XS32 saap_delUidSegRoute(XS32 routeIdx);
    XVOID saap_showUidSeg(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_showrbtroute(CLI_ENV* pCliEnv,XU32 ulFlag);
#ifdef  __cplusplus
}
#endif

#endif


