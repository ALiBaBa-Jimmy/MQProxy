#ifndef _SAAP_DEF_H
#define _SAAP_DEF_H
#include "xosshell.h"
#ifdef  __cplusplus
extern  "C"
{
#endif

#define MAX_IP_DATA_LEN  (6*1460) //20101021 cxf mod it 4->6 ,because zyf say dataLen may >6k

#ifdef SCALE_CPU_VX
#define CANCLE_PACK #pragma pack(0)
#else
#define CANCLE_PACK #pragma pack()
#endif

enum
{
    SYS_IP_TYPE_NOTUSE = 0,
    SYS_IP_TYPE_TCP,
    SYS_IP_TYPE_UDP,
    SYS_IP_TYPE_RAW,
    SYS_IP_TYPE_MAX
};

#define SAAP_MAX_TELNO_LENGTH       8           /*���ĵ绰�����ֽ���8888*/
#define MAX_DIGITAL_LEN             64          /*�����볤��8888*/
#define DIGITAL_BUFF_LEN            32          /*���뻺��������*/
#define BCD_NUM_MASK                0x0f
#define BCD_NUM_MIN_VAL             0x00
#define BCD_NUM_MAX_VAL             0x0c

#define BCD_NUM_ZERO                0x0a
#define BCD_NUM_STAR                0x0b
#define BCD_NUM_WELL                0x0c

#define SVC_NATAP_IP_CFG        0x01                 /* ҵ������NATAP��·����Ϣ */
#define SVC_NATAP_IP_DEL        0x02                 /* ҵ��ɾ��NATAP��·����Ϣ */
#define NATAP_SVC_CFG_ACK       0x03                 /* NATAP��ҵ�����·������Ӧ��Ϣ������NATAP��·�� */
#define NATAP_SVC_CFG_IND       0x04                 /* NATAP��ҵ�����·����ָʾ��Ϣ������NATAP��·״̬ */

#define NATAP_SVC_ERR_IND       0x05                 /* NATAP��ҵ��Ĵ���ָʾ��Ϣ */
#define SVC_NATAP_ERR_ACK       0x06                 /* ҵ���NATAP�Ĵ�����Ӧ�����������ָʾһ�µ�ԭ�� */

#define SVC_NATAP_SVC_REQ       0x07                 /* ҵ���NATAP��ҵ��������Ϣ����ʾҪ�������ݸ�NATAP */
#define NATAP_SVC_SVC_IND       0x08                 /* NATAP��ҵ���ҵ��ָʾ��Ϣ����ʾҪ�������ݸ�ҵ�� */
#define NATAP_SVC_ALARM         0x09                 /* NATģ�鷢�͸��ϲ�ĸ澯��ϢID */

/* -----------------------ͳ�����-------------------------------------------*/
#define STAT_TO_SVC_CFG            0xcc10            /* ͳ��������Ϣ��    */
#define STAT_TO_SVC_START_REQ      0xcc11            /* ͳ��������Ϣ��    */
#define STAT_TO_SVC_START_RSP      0xcc12            /* ͳ��������Ӧ��Ϣ��*/
#define STAT_TO_SVC_CLEAR_DATA     0xcc13            /* ͳ�������Ϣ��    */


/*TCPEN��ض���*/
#define MAX_TCPEN_LINK_NUM          64       //���tcp��װ��·��
#define DEFAULT_TCPEN_STAT_LINKID   61       //Ĭ��ͳ��̨������·��
#define DEFAULT_TCPEN_MEDIA_LINKID  62       //Ĭ��ý��������·��
#define DEFAULT_TCPEN_RNMS_LINKID   63       //Ĭ������������·��
#define STAT_DPID                  ((BLANK_USHORT)-1)
#define RNMS_DPID                  (BLANK_USHORT)   //û��ʹ�ô˺�,
#define MED_DPID                   ((STAT_DPID) - 1)//û��ʹ�ô˺�,

//#define FID_MM 1
//#define FID_CC 3

#define FID_TCPE           1701
#define FID_SAAP           1702

//�ַ���ת��Ϊ���ֺ����궨��
#define XOS_ATOF(str)                     (atof((const char *)str))   //convert a string to a double (ANSI)
#define XOS_ATOI(str)                     (atoi((const char *)str))   //
#define XOS_ATOL(str)                     (atol((const char *)str))   //10����

//long->�ַ���
#define XOS_LTOA(lValue,pStr,nRadix) ltoa((lValue),(pStr),(nRadix))
#define XOS_ULTOA(ulValue,pStr,nRadix) _ultoa((ulValue),(pStr),(nRadix)) //9999

//9999 below
#define SYS_RETURN return
//9999 above

//��Ϣ���������Խṹ
typedef struct
{
    XU32   ucModId;           //ģ���
    XU32   ucFId;             //���ܿ��
    XU32   usFsmId;           //�ڲ����Ӻ�
}MSG_OWNER_SAAP;

//������Ϣͷ�ṹ
typedef struct tagCommonHeader_saap
{
    MSG_OWNER_SAAP   stSender;       //������
    MSG_OWNER_SAAP   stReceiver;     //������
    XU16     usMsgId;                //��Ϣ����
    XU16     subID;                  //��Ϣ������

    XU32     prio;                   //��Ϣ���ȼ�,���������ƽ̨��Ϣͷһ�������ת��Ч��
    XU32     usMsgLen;               //��Ϣ����
    void     *message;               //��Ϣָ�룻
}COMMON_HEADER_SAAP;                 //����ϢӦ��XOSƽ̨��Ϣͷ��ʽ����

//ͨ�ü���Ϣ�ṹ
typedef struct tagCommonMessage_saap
{
    COMMON_HEADER_SAAP stHeader;
    XU8 ucBuffer[1];
}COMMON_MESSAGE_SAAP;

//-------------------------------ö�ٶ���------------------------------------
typedef struct tagSysIPAddr
{
    XU32      ip;     //IPV4��ַ
    XU16      port;   //�˿ں�
}SYS_IP_ADDR;


#define SWITCH_ON               1
#define SWITCH_OFF              0

#define FLAG_YES                1
#define FLAG_NO                 0
#define SAAP_MAX_SPA_CLI_NUM    16

/*wzy add */
#define IN           /*��������ı�־*/
#define OUT          /*��������ı�־*/
#define SAAP_MAX_MSG_LEN        (8*1024) //0x7fff    //�����Ϣ��󳤶�

#ifdef XOS_LINUX
#define MD( ulFid, ulLevel) (XCHAR*)(__FILE__), (XU32)(__LINE__), (XCHAR*)(__FUNCTION__), (XU32 )(ulFid), (e_PRINTLEVEL)(ulLevel)
#else
#define MD( ulFid, ulLevel) (XCHAR*)(__FILE__), (XU32)(__LINE__), NULL, (XU32 )(ulFid), (e_PRINTLEVEL)(ulLevel)
#endif

#define SAAP_SWAP_HIGH4_BIT(VAL) ( ( (VAL & 0xF) << 4) | (VAL >> 4) )

//IPģ�����ϲ�֮�����ϢID
enum
{
    //IP������Ϣ
    MSG_IP_CONFIG = 1,          //����          OAM->IP
    MSG_IP_ADDRCHANGE,          //IP��ַ�ı�    OAM->IP
    //MSG_IP_CONNECTIND,        //����ָʾ      IP->USER
    MSG_IP_CLOSEREQ,            //�ر�����      USER->IP
    //MSG_IP_CLOSEIND,          //�ر�ָʾ      IP->USER
    MSG_IP_DATAREQ,             //��������      USER->IP
    //MSG_IP_DATAIND,           //����ָʾ      IP->USER
    MSG_IP_CFGACK,              // ����ȷ��     IP->USER
    MSG_IP_REFRESH,             // ˢ����·    USER->IP
#ifdef SYS_IP_LINK_STATIC
    MSG_IP_STATIC_RESET,        //ͳ�Ƹ�λ
#endif

    //LAPD������Ϣ
    MSG_LAPD_CONFIG,            //����          OAM->LAPD
    MSG_LAPD_DLESTABLISHREQ,    //��������      OAM->LAPD
    MSG_LAPD_DLRELEASEREQ,      //�ͷ�����      OAM->LAPD
    MSG_LAPD_DLDATAREQ,         //��������      USER->LAPD
    MSG_LAPD_DLDATAIND,         //����ָʾ      LAPD->USER
    MSG_LAPD_STATUSIND,         //״ָ̬ʾ      LAPD->OAM

    MSG_EXIT,                   //�˳�
    MSG_MAX
};


/* TCPE ͳ�ƽṹ */

#define  TCP_LINT_STAT_MAX       MAX_TCPEN_LINK_NUM  //9999
typedef enum
{
    eSendMsg=10,
    eSendErrMsg,
    eRecvMsg,
    eRecvErrMsg,
    eSendFail,
    eLinkTry,
    eLinkHand,
    eLinkHandAck,
    eLinkLose,
    eLinkShutStop,
    eOtherStat
}e_TCPE_STAT;

typedef struct
{
    //XU16                usLinkIdx;             // ��·ID�����ΪBLANK_USHORT����Ҫͳ��
    XU32                  ulSendMsgCnt;          // ��·���͵���Ϣ������
    XU32                  ulSendErrMsgCnt;       // ��·���͵Ĵ�����Ϣ����������
    XU32                  ulRecvMsgCnt;          // ��·�յ�����Ϣ������
    XU32                  ulRecvErrMsgCnt;       // ��·�յ�������Ϣ����������
    XU32                  ulSendFailCnt;         // ����ʧ�ܴ���[�յ�NTL��send err
    XU32                  ulLinkTryCnt;          // ��·�ؽ�������
    XU32                  ulLinkHand;            // ��·������Ϣ
    XU32                  ulLinkHandAck;         // ��·����Ӧ����Ϣ
    XU32                  ulLinkLoseCnt;         // ��·ʧ������
    XU32                  ulLinkShutStop;        // ��·�رմ���ͳ��
    XU32                  ulOther;               // ����ͳ�ƵĴ������
    XU32                  ulSendFailReason[eOtherErrorReason+1];
}t_TCPE_LINKSTAT;

typedef struct
{
    t_TCPE_LINKSTAT msgStat[TCP_LINT_STAT_MAX];
    t_TCPE_LINKSTAT msgIn;
}t_TCPESTAT;//�ṹ��ΪȫF��ֵ����ӡ, ��ʼ��ΪȫF

typedef struct
{
    XU32 busyflag;       //�Ƿ���Ч�ͻ���
    XU32 IpAddr;
    XU32 port;
    XU8 ucCodePlan;      //��żƻ�
    XU32 gtValue;        //GT
    XU32 ulSend2MntFlag; //���͵�MNT��ʶ,ƥ�������������1,�������0
}SAAP_SPA_CLI_STRU_T;


typedef struct
{
    XU32 ulAllSendFlag; // ȫ�������ʶ
    XU32 ulTraceFlag;
    XU32 cliNum;
    XU32 linkHandle;
    XU32 ulTraceId;
    SAAP_SPA_CLI_STRU_T cliData[SAAP_MAX_SPA_CLI_NUM];
}SAAP_SPA_CLI_DATA_T;

extern XS32 SYS_XOSMsg2CpsMsg(t_XOSCOMMHEAD *pxosMsg,COMMON_HEADER_SAAP *pCpsMsg);
extern XS32 SAAP_SockGetV4Addr(XCHAR *cIP, XU32 *addr);
extern int  SYS_MSGSEND( COMMON_HEADER_SAAP *psCpsMsg);
extern int  SAAP_StartTimer(PTIMER *ptHandle,XU32 fid,XU32 len,XU32 tmName,XU32 para,XU32 mode );
extern XS32 SYS_Str2Bcd(XCHAR * bcdstr);
extern char* SAAP_IptoStr( XU32 inetAddress , char *pString );
extern XS32 SAAP_StrTelToDn(XCHAR *pStrTel, XU8 *pDn );
extern int  TCPE_SendMsg2Ip( COMMON_HEADER_SAAP *psCpsMsg);
extern XU32 SAAP_MsgNeedToMnt(COMMON_HEADER_SAAP *psCpsMsg);
extern XU32 SAAP_SendMsgToMnt(COMMON_HEADER_SAAP *psCpsMsg);
XU32 SAAP_FilterSaap2Tcpe(COMMON_HEADER_SAAP *pstMsg);
XU32 SAAP_FilterSaap2Srv(COMMON_HEADER_SAAP *pstMsg);
#ifdef  __cplusplus
}
#endif
#endif


