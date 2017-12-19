/*----------------------------------------------------------------------
    tcpMain.h - ȫ�ֺ������Ͷ���

    ��Ȩ���� 2004 -2006 ������˾������SAG��Ŀ��.

    author: �ź�

    �޸���ʷ��¼
    --------------------
    ��Ӱ�Ȩ˵��.
----------------------------------------------------------------------*/

#ifndef _TCPMAIN_H_
#define _TCPMAIN_H_

#include "xosshell.h"
#include "saap_def.h"

#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

    //------------------------�궨��------------------------------------

#define SAAPANDTCP_HEAD_LEN  10   //����ͷ���� �μ����ݽṹSAAP_AND_TCP_PACKAGE_HEAD�Ķ���

#define MAX_TCPE_HAND_TIMES  10
#define TCP_REG_TIMER_NUM    MAX_TCPEN_LINK_NUM    //ע������ʱ����Ŀ
#define TCP_TIMER_NUM        3                     //��������ʱ����Ŀ


//Э��涨,�����޸�
#define TCP_CURR_FLAG              0x7E        //���ı߽�
#define TCP_HEAD_FLAG              0xA5        //������������ʼ
#define TCP_END_FLAG               0x0D        //��������������

#define TCP_ERROR_NUM              0xFFFF

#define TCP_ROUTER_MAX_NUM         32    // ��������·�ɴ���
#define    SAAP_USER_TYPE          0x1100       // TCP/IP �е�SAAP�û�����
#define    TCP_USERTYPE_HANDSHAKE  0x1110       // TCP �����û�����
#define    MAX_TCPE_USER_NUM 10
#define    TCP_USERTYPE_CBUA       0x1112       // TCPE ֱ����������
#define    TCP_USERTYPE_CBUA_2     0x1113 //CBUA2
#define    TCP_USERTYPE_VMR        0x1115 // VMR
//Э��涨,�����޸�

/* tcp link status */
#define   TCP_LINK_OK            0          // ����
#define   TCP_LINK_FAIL          1          // ��ͨ

#define   TCP_hand_reg           0x00       // ��������
#define   TCP_hand_ack           0x01       // ����Ӧ��

#define   TCP_HAND_ERR           0xEEEE      // ��������ȥ֪ͨ

// ·���뱾�ص��ú�
#define   TCP_INVIKE_LOCAL       0x01       // ���ص���
#define   TCP_INVIKE_ROUTE       0x02       // ·�ɵ���

typedef struct
{
    XU32 userType;
    XU32 userFid;
}TCPE_USER_FID_T;

// TCP/IP ��װ���ϲ�Ľӿ����ݽṹ
typedef struct  TCP_AND_PRO_LAYER_Interface
{
    XU16        DstDeviceID;        /*Ŀ���������*/
    XU16        GuestDeviceID;      /*Դ���������*/
    XU16        UserType;           /*�û�����*/
    XU16        RouterNum ;         /*·�ɴ���*/
    XU16        MsgLen;             /*�û�����*/
}TCP_AND_PRO_LAYER_INTERFACE;

// TCP/IP ��װ�����ϲ�Ľӿڽṹ
typedef struct TCP_AND_PRO_LAYER_MessagPBuf
{
    COMMON_HEADER_SAAP                   com;
    TCP_AND_PRO_LAYER_INTERFACE      tcpHead;
    XU8                          ucBuffer[1];
} TCP_AND_PRO_LAYER_MSG_BUF;

// TCP/IP ��װ����һ��Ľӿ����ݽṹ
typedef struct  TCP_AND_NEXT_LAYER_Interface
{
    XU8     ucHeadFlag7E;
    XU8     ucHeadFlagA5;

    XU16        MsgLen;             /*�û�����*/
    XU16        DstDeviceID;        /*Ŀ���������*/
    XU16        GuestDeviceID;      /*Դ���������*/
    XU16        UserType;           /*�û�����*/
    XU16        RouterNum ;         /*���ڼ�¼�����˶��ٸ�·�ɵ�*/
}TCP_AND_NEX_LAYER_INTERFACE;

// TCP/IP ��װ�����²�����ݰ��ӿڽṹ,������������Ϣ
typedef struct TCP_AND_NEX_LAYER_MessagPBuf
{
    COMMON_HEADER_SAAP                   com;
    TCP_AND_NEX_LAYER_INTERFACE      tcpHead;
    XU8                          ucBuffer[1];
} TCP_AND_NEX_LAYER_MSG_BUF;

/* TCP/IP ��װ��������Ϣ */
typedef struct TCP_hand_msg
{
    COMMON_HEADER_SAAP                   comHead;
    TCP_AND_NEX_LAYER_INTERFACE      tcpHead;
    XU8                          msgType;       //��Ϣ����
    XU8                          Tailbuff[2];
} TCP_hand_msg;

// TCP/IP ��װ�����²������ӿڽṹ,�����������Ϣ
typedef struct TCP_AND_NEX_LAYER_COM_Buf
{
    COMMON_HEADER_SAAP                   com;
    XU8                          ucBuffer[1];
} TCP_AND_NEX_LAYER_COM_BUF;

#ifdef SCALE_CPU_VX
/*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

XS8   TCP_InitProc(XVOID *Para1, XVOID *Para2 );
XS8   TCP_MsgProc(XVOID *msg, XVOID *Para);
XS8   TCP_TimeoutProc (t_BACKPARA  *pstPara);

//ƽ̨���а�����
XS32  Tcp_ntlmsg_proc( COMMON_HEADER_SAAP *pstMsg );

//ҵ������ƽ̨�İ�����,������ҵ��ͨ��tcpe���͵��������ݰ�
//flag ����������·������Ϣ�뱾�������
void TCP_Svc2Tcp_DataReqProc( COMMON_HEADER_SAAP *pstMsg , XU8 flag );

//������ض�λ����
//ͨ��ƽ̨NTL���е�ҵ��İ�����,����NTL->TCPE->APP�������ݰ�
void Tcp_ntlmsg_packetdecodeproc( COMMON_HEADER_SAAP *pstMsg );

//��������ָʾ
void TCP_Tcp2Svc_DataIndProc( COMMON_HEADER_SAAP *pstMsg, XU8 *MsgBuffer );

// TCP ���ֺ���,
void Tcp_lint_scan(XU16 ccbNo) ;
void TCP_hand_msg_proc( COMMON_HEADER_SAAP *pstMsg );

XS32 tcp_check_lintIndex(XU16 index); // �˺����������������·�ŵ���Ч��

/*20090722 add below*/
XS32 tcpe_setfid_byusertype(XS32 userType,XS32 userFid);
XS32 tcpe_getfid_byusertype(XS32 UserType);
/*20090722 add above*/
XU16 tcpe_getDefaultLink();

#ifdef  __cplusplus
}
#endif

#endif


