/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosftpclient.h
**
**  description:
**
**  author: zengjiandong
**
**  date:   2006.10.19
**
***************************************************************
**                          history
**
***************************************************************
**   author                      date           modification
**   zengjiandong         2006.10.19            create
**************************************************************/
#ifdef XOS_FTP_CLIENT
#ifndef _XOS_FTP_H_
#define _XOS_FTP_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xosshell.h"
#include "xosntl.h"

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
#define MAX_USERNAME_LEN (32+1)     /*����û�������*/
#define MAX_PASSWORD_LEN (16+1)     /*������볤��*/
#define MAX_DIRECTORY_LEN (256+1)   /*���·������*/
/*  ftp�ͻ��˾��  */
XOS_DECLARE_HANDLE(XOS_HFTPCLT);

/*-------------------------------------------------------------------------
                �ṹ��ö������
-------------------------------------------------------------------------*/

/*ftp�ĵ�ǰ״̬*/
typedef enum
{
    eFTPSTATEINIT = 0,  /*ftp�ĳ�ʼ״̬��δ���ӵ�¼*/
    eFTPCTRLINITED,     /*�������ӳ�ʼ��*/
    eFTPSTATELOGIN,     /*�û���¼�ɹ����״̬*/
    eFTPSTATEDISCON     /*��¼�ɹ���FTP������ʶϿ�*/
}e_FTPSTATE;

/*ftp��ǰ���ڴ�����¼�*/
typedef enum
{
    eFTPNONE = 0,      /*��ǰû�д����κ��¼�*/
    eFTPGETTOFILE,     /*�����ļ��������ļ�*/
    eFTPGETTOMEM,      /*�����ļ����ڴ�*/
    eFTPPUTFROMFILE,   /*�ϴ����ش����ļ�*/
    eFTPPUTFROMMEM,    /*�ϴ������ڴ��ļ�*/
    eFTPCREATFILE,     /*����һ�����ļ�*/
    eFTPLIST,          /*��ȡָ��Ŀ¼�µ��ļ��б�*/
    eFTPNAME,          /*�ڿ��������ϴ����û���*/
    eFTPPASS,          /*�ڿ��������ϴ�������*/
    eFTPDEFDIR,        /*ȡĬ��·��*/
    eFTPCWD,           /*�ı䵱ǰ����Ŀ¼*/
    eFTPSIZE,          /*ȡ�ļ���С*/
    eFTPMKD,           /*�½�Ŀ¼*/
    eFTPRMD,           /*ɾ��Ŀ¼*/
    eFTPRNCHECK,       /*������׼��*/
    eFTPRNEXCUTE,      /*����������*/
    eFTPRMF,           /*ɾ���ļ�*/
    eFTPPASV,          /*���ͱ���ģʽ����*/
    eFTPTYPE,          /*�����ļ�����*/
    eFTPPORT,          /*���ͱ���ip��port*/
    eFTPQUIT           /*�˳���¼*/
}e_FTPCUREVENT;



/*����ʱ�յ�����Ϣ�������*/
typedef enum
{
    ePREPARE_SUC = 0,     /*ֻ�յ�����׼����ȷ*/
    ePRE_AND_TRANS,       /*����׼���ʹ���ɹ�*/
    ePRE_AND_TRANSFAIL    /*����׼��������ʧ��*/
}e_TRANSREPLYINFO;

/*FTP ������Ϣ�ṹ��*/
typedef struct
{
    t_IPADDR          dataLinkAddr;      /*����IP�Ͷ˿�*/
    t_IPADDR          dstAddr;           /*ftp������IP�Ͷ˿�*/
    t_IPADDR          locAddr;           /*����ip�Ͷ˿�*/
    t_XOSSEMID        semaphore;         /*�ź���*/
    e_FTPSTATE        curState;          /*��ǰ״̬*/
    e_FTPCUREVENT     curEvent;          /*��ǰ������¼�*/
    e_FTPCUREVENT     curTransfer;       /*��ǰ��������*/
    XSFILESIZE        fileSize;          /*�õ����ļ��Ĵ�С*/
    XS32              ret;               /*��ǰ�¼��Ĵ�����*/
    XS32              retOfPut;          /*�ϴ��Ƿ��쳣*/
    e_TRANSREPLYINFO  replyInfoType;     /*���ڴ���������(�ļ����б�),���ܳ��ֵ�����Ӧ����һ����Ϣ������*/
    t_XINETFD         dataSockId;        /*����������Ϣ����*/
    XU32              count;             /*����*/
    XUFILESIZE        bufferSize;        /*�ڴ��С*/
    XVOID*            pBuffer;           /*�ڴ�ָ��*/
    FILE              *pFile;            /*�ļ�ָ��*/
    HLINKHANDLE       linkHandle;        /*��·���*/
    PTIMER            timerId;           /*��ʱ�����*/
    XS8   passWord[MAX_PASSWORD_LEN];    /*ftp�û�����*/
    XS8   userName[MAX_USERNAME_LEN];    /*ftp�û���*/
    XS8   defDir[MAX_DIRECTORY_LEN];     /*���������ص�Ĭ��·��*/
    XS8   curDir[MAX_DIRECTORY_LEN];     /*��ǰ����Ŀ¼*/
    XS8   curFolder[MAX_DIRECTORY_LEN];  /*��ǰ�ļ���(ɾ��Ŀ¼��)*/
    XS8   RemFile[MAX_DIRECTORY_LEN];
    XS8   newFileName[MAX_DIRECTORY_LEN];
} t_XOSFTPCLIENT;

/*�������ӽṹ��
typedef struct
{
    HLINKHANDLE hDataLink;
    t_XOSFTPCLIENT *pCtrlLink;
}t_FTPDATALINK;*/
/*-------------------------------------------------------------------------
                API ����
-------------------------------------------------------------------------*/

/************************************************************************
������: XOS_FtpLogin
���ܣ���¼ftp������
���룺pServAddr         -ftp������ip
      pUserName         -�û���¼�ʺ�
      pPasswd           -�û���¼����
�����hFtpClt           -ftp�û����
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpLogin(t_IPADDR *pServAddr, XS8 *pUserName, XS8 *pPassWd, XOS_HFTPCLT *hFtpClt);

/************************************************************************
������: XOS_FtpGetFileSize
���ܣ����ftp��������һ���ļ��Ĵ�С
���룺hFtpClt            -ftp�û����
      pRemFile           -�������ļ�
�����N/A
���أ��ɹ��򷵻��ļ���С
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpGetFileSize( XOS_HFTPCLT hFtpClt, XS8 *pRemFile );

/************************************************************************
������: XOS_FtpGet
���ܣ���ftp����һ���ļ������ش���
���룺hFtpClt              -ftp�û����
      pRemFile             -�������ļ�
      pLocFile             -�����ļ�
      time                 -�����ļ�������ʱ��,��λ(��),0��ʾû����
�����N/A
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpGetToFile( XOS_HFTPCLT hFtpClt, XS8 *pRemFile, XS8 *pLocFile, XU32 time);

/************************************************************************
������: XOS_FtpGetToMem
���ܣ���ftp����һ���ļ��������ڴ�
���룺hFtpClt              -ftp�û����
      pRemFile             -�������ļ�
      pBuff                -�����ļ�·��
      buffSize             -�����ļ���
      time                 -�����ļ�������ʱ��,��λ(��),0��ʾû����
�����N/A
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpGetToMem( XOS_HFTPCLT hFtpClt, XS8 *pRemFile, XS8 *pBuff, XU32 buffSize, XU32 time);

/************************************************************************
������: XOS_FtpPut
���ܣ������ش��̵��ļ��ϴ���ftp������
���룺hFtpClt              -ftp�û����
      pLocFile             -�����ļ�
      pRemFile             -�������ļ�
      time                 -�ϴ��ļ�������ʱ��,��λ(��),0��ʾû����
�����N/A
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpPutFromFile( XOS_HFTPCLT hFtpClt, XS8 *pLocFile, XS8 *pRemFile, XU32 time);

/************************************************************************
������: XOS_FtpPutFromMem
���ܣ��������ڴ���ļ��ϴ���ftp������
���룺hFtpClt              -ftp�û����
      pBuff                -�����ڴ��ļ�ָ��
      buffSize             -�����ڴ��ļ���С
      pRemFile             -�������ļ���
      time                 -�ϴ��ļ�������ʱ��,��λ(��),0��ʾû����
�����N/A
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpPutFromMem( XOS_HFTPCLT hFtpClt, XS8 *pBuff, XU32 buffSize, XS8 *pRemFile, XU32 time);

/************************************************************************
������: XOS_FtpCurrentWorkDir
���ܣ�ȡ�õ�ǰ����Ŀ¼
���룺hFtpClt           -ftp�û����
�����pWorkDir          -��ǰ����Ŀ¼
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpCurrentWorkDir( XOS_HFTPCLT hFtpClt, XVOID **pWorkDir );

/************************************************************************
������: XOS_FtpChangeWorkDir
���ܣ��ı䵱ǰ����Ŀ¼
���룺hFtpClt      -ftp�û����
      pWorkDir     -�µĵ�ǰ����Ŀ¼
�����N/A
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpChangeWorkDir( XOS_HFTPCLT hFtpClt, XS8 *pWorkDir );

/************************************************************************
������: XOS_FtpMakeDir
���ܣ��ڵ�ǰ����Ŀ¼�´���һ���µ��ļ���
���룺hFtpClt     -ftp�û����
      pNewFolder  -Ҫ�������ļ�����
�����N/A
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpMakeDir( XOS_HFTPCLT hFtpClt, XS8 *pNewFolder );

/************************************************************************
������: XOS_FtpRemoveDir
���ܣ��ڵ�ǰ����Ŀ¼��ɾ��һ���ļ���
���룺hFtpClt         -ftp�û����
      pFolderName     -Ҫɾ�����ļ�����
�����N/A
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpRemoveDir( XOS_HFTPCLT hFtpClt, XS8 *pFolderName );

/************************************************************************
������: XOS_FtpRenameDir
���ܣ��ı䵱ǰ����Ŀ¼��һ���ļ��е�����
���룺hFtpClt          -ftp�û����
      pOldFolder       -�ɵ��ļ�����
      pNewFolder       -�µ��ļ�����
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpRenameDir( XOS_HFTPCLT hFtpClt, XS8* pOldFolder, XS8 *pNewFolder );

/************************************************************************
������: XOS_FtpCreatFile
���ܣ��ڵ�ǰ����Ŀ¼�´���һ���µ��ļ�
���룺hFtpClt    -ftp�û����
      pFileName  -Ҫ�������ļ�������
�����N/A
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpCreatFile( XOS_HFTPCLT hFtpClt, XS8 *pFileName );

/************************************************************************
������: XOS_FtpDeleteFile
���ܣ��ڵ�ǰ����Ŀ¼��ɾ��һ�����ڵ��ļ�
���룺hFtpClt    -ftp�û����
      pFileName  -��ǰ����Ŀ¼�µ�һ���ļ�������
�����N/A
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpDeleteFile( XOS_HFTPCLT hFtpClt, XS8 *pFileName );

/************************************************************************
������: XOS_FtpRenameFile
���ܣ�����ǰ����Ŀ¼�µ�һ���ļ�����
���룺hFtpClt                -ftp�û����
      pOldFileName           -��ǰ����Ŀ¼�µ�һ���ļ�������
      pNewFileName           -�µ��ļ���
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpRenameFile( XOS_HFTPCLT hFtpClt, XS8 *pOldFileName, XS8 *pNewFileName );

/************************************************************************
������: XOS_FtpClose
���ܣ���ftp�������Ͽ���������,�˳���¼
���룺hFtpClt       -ftp�û����
�����N/A
���أ��ɹ��򷵻�XSUCC
      ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpClose( XOS_HFTPCLT *hFtpClt );

XS8 XOS_FtpInit( XVOID *t, XVOID *v );
XS8 FTP_TimerProc( t_BACKPARA* pBackPara);
XS8 FTP_MsgProc( XVOID* pMsgP, XVOID* sb);

/*ftp����ṹ*/
typedef struct
{
    XBOOL initialized;  /*�ڴ��ʼ���ı�־*/

    t_XOSMUTEXID contrlTblLock; /* ����ɾ��ftp��¼�û�ʱ����*/
    XOS_HARRAY   ftpConnectTbl;/*�û���Ϣ��¼��*/
}t_FTPMNT;

/*�ϴ��������������*/
typedef struct
{
    t_IPADDR ftpServAddr;    /*�������˵ĵ�ַ*/
    t_XINETFD *pSockId;      /*����sock*/
    e_FTPCUREVENT  curEvent; /*��ǰ�¼�*/
    XSFILESIZE      buffLen;       /*�ϴ����ݵĳ��ȣ� �ļ���С�����ڴ���С
                                            �ļ��б������������ʵ���б��С*/

    XUFILESIZE ulDownLen;          /*��ǰ���ص��ļ����ڴ��С*/

    FILE *pFile;             /*�ļ��ϴ� ������ʱ�õ�,Ҫ�ϴ����ļ�����Ҫ���ص��ļ�ָ��*/

    XCHAR * pBuffer;         /*�ڴ���ϴ�������ʱ������õ�,�ϴ����������ڴ���׵�ַ
                                              �ļ��б�ʱ��Ϊ�������*/
    t_XOSSEMID  *pSem;       /*�ź����������ز���ʱ������·��ͬ��*/
}t_FTPDATALINKENTRY;

/*----------------------bruce add -------------------------------------*/
/*��Ϣ�ṹ*/

typedef enum
{
    FTP_QUE_NULL = 0,
    FTP_QUE_FULL,
    FTP_QUE_FAI,
    FTP_QUE_SUC,
}e_FTP_ADD;

/*ftp�ϴ��ڴ����ݵ�������*/
typedef struct
{
    /*��Ϣ��ˮ��*/
    XU64 ulSerial; 
    t_IPADDR destAddr; /*��������ַ*/

    XU32 ulUserLen;    /*�û�������*/
    XU8 *ucpUser;/*�û���*/

    XU32 ulPassLen;/*���볤��*/
    XU8 *ucpPass;/*����*/
    
    XU32 ulRelDirLen; /*���Ŀ¼����*/
    XU8 *ucpReldir; /*���Ŀ¼(����ڵ�ǰ�Ĺ���Ŀ¼),��Ҫ�û��ͷ�*/

    XU32 ulFileNameLen;/*�������ļ�������*/
    XU8 *ucpFileName;/*�������ļ���*/
    
    XU32 ulBufferLen;    /*���ݳ���*/
    XU8 *ucpBuffer;/*���ϴ����ݻ���ָ��,��Ҫ�û��ͷ�*/

}t_PUTFROMMEM_REQ;

/*ftp�ϴ��ļ���������*/
typedef struct
{
    XU64 ulSerial; 
    t_IPADDR destAddr;            /*��������ַ*/

    XU32 ulUserLen;                  /*�û�������*/
    XU8 *ucpUser;                 /*�û���*/

    XU32 ulPassLen;               /*���볤��*/
    XU8 *ucpPass;                 /*����*/
    
    XU32 ulRelDirLen;             /*���Ŀ¼����*/
    XU8 *ucpReldir;               /*���Ŀ¼(����ڵ�ǰ�Ĺ���Ŀ¼),��Ҫ�û��ͷ�*/

    XU32 ulFileNameLen;           /*�������ļ�������*/
    XU8 *ucpFileName;             /*�������ļ���*/
    
    XU32 ulUploadFilePathLen;              /*��Ҫ�ϴ����ļ���·������*/
    XU8 *ucpUploadFilePath;       /*��Ҫ�ϴ����ļ���·��*/

}t_PUTFROMFILE_REQ;

/*ftp�ӷ����������ļ����ļ�*/
typedef struct
{
    XU64 ulSerial; 
    t_IPADDR destAddr;            /*��������ַ*/

    XU32 ulUserLen;                  /*�û�������*/
    XU8 *ucpUser;                 /*�û���*/

    XU32 ulPassLen;               /*���볤��*/
    XU8 *ucpPass;                 /*����*/
    
    XU32 ulRelDirLen;             /*���Ŀ¼����*/
    XU8 *ucpReldir;               /*���Ŀ¼(����ڵ�ǰ�Ĺ���Ŀ¼),��Ҫ�û��ͷ�*/

    XU32 ulFileNameLen;           /*�������ļ�������*/
    XU8 *ucpFileName;             /*�ļ���*/
    
    XU32 ulSaveFilePathLen;          /*��Ҫ������ļ���·������*/
    XU8 *ucpSaveFilePath;         /*�ļ���·��*/

    XU32 ulTime;                  /*����ʱ��*/
    XU16 ulMaxReDown;             /*������ش���*/

    XU32 ulMd5Len;                /*md5����*/
    XU8 *ucpMd5;                  /*md5��*/

}t_GETTOFILE_REQ;


/*��Ӧ��Ϣ*/
typedef struct
{
    /*��Ϣ��ˮ��*/
    XU64 ulSerial; 
    XU32 ulResult; /*����Ľ��*/
}t_PUTFROMMEM_RSP;

/*��Ӧ��Ϣ*/
typedef struct
{
    /*��Ϣ��ˮ��*/
    XU64 ulSerial; 
    XU32 ulResult; /*����Ľ��*/
    XU32 ulFileSize; /*�ļ���С*/
}t_GETTOFILE_RSP;


/*��Ӧ��Ϣ״̬*/
typedef enum
{
    eFTP_SUCC = 0,
    eFTP_PARA_INVALID = 1, /*�������Ϸ�,��������ĸ�ʽ*/
    eFTP_LOGIN_FAILED,/*��¼ʧ�ܣ��������û��������벻ƥ��*/
    eFTP_TRAN_FAILED,/*����ʧ��,������·����Ȩ�޲���*/
    eFTP_LOC_FILE_FAILED, /*�����ļ���ʧ��*/
    eFTP_LOC_FILE_EXIST, /*�����ļ�����*/
    eFTP_REMOTE_FILE_FAILED, /*Զ���ļ�������*/
    eFTP_DEL_FILE_FAILED,/*ɾ�������ļ�ʧ��*/
    eFTP_DWONLOADING,    /*�ļ�����������*/
    eFTP_GENERATE_MD5_ERROR, /*����md5����*/
    eFTP_MD5_ERROR,      /*md5У�����*/
}e_FTPRSP_STATUS;




/*�ͻ���Ϣ����*/
typedef enum
{
    eFTP_NULL,
    eFTP_GETTOFILE,     /*�����ļ��������ļ�*/
    eFTP_GETTOFILEAck,
    eFTP_GETTOMEM,      /*�����ļ����ڴ�*/
    eFTP_GETTOMEMAck,
    eFTP_PUTFROMFILE,   /*�ϴ����ش����ļ�*/
    eFTP_PUTFROMFILEAck,
    eFTP_PUTFROMMEM,    /*�ϴ������ڴ��ļ�*/
    eFTP_PUTFROMMEMAck,
    eFTP_LIST,          /*��ȡָ��Ŀ¼�µ��ļ��б�*/
    eFTP_LISTAck
}e_CLIEVENT;


/*�ط���Ϣ*/
typedef struct
{
    XU32 nSndNum;
    t_XOSCOMMHEAD reMsg;    
}t_ReFtpMsg;

XS8 FTP_ClientMsgPro(t_XOSCOMMHEAD* pMsg);
XS8 FTP_NtlMsgPro(t_XOSCOMMHEAD* pMsg);
XS32 FTP_SendMsgToCli(XU32 ulDestFid, XU32 ulRst,XU64 ulSerial);
XS32 FTP_FreePUTFROMMEM_REQ(t_XOSCOMMHEAD *pReq);
XS32 FTP_FreePUTFROMFILE_REQ(t_XOSCOMMHEAD *pMsg);

int XOS_CheckFtpCtrl(char *pszBuf, int nLen ,int *pnCode);
XS8 XOS_FtpPutMemTest(void);
XS8 XOS_FtpPutFileTest(void);

XS32  PutFileToServer_Test(XCHAR *logfilename);

XS32 XOS_FtpInitMsgQueue();
XS32 XOS_FtpInitMsgDealTaskPool(void);
XS32 XOS_FtpClearMsgQueue(t_XOSCOMMHEAD *pMsg);
XS32 XOS_FtpAddMsgQueue(t_XOSCOMMHEAD *pMsg);
XS8 FTP_PutMemMsgProTskEntry(t_XOSCOMMHEAD* pMsg);
XS8 FTP_PutFileMsgProTskEntry(t_XOSCOMMHEAD* pMsg);
void XOS_FtpClearMsgMem(t_XOSCOMMHEAD* paras);

XS32 XOS_FtpInitReMsgQueue();
XS32 XOS_FtpAddReMsgQueue(t_ReFtpMsg *pMsg);
XS32 XOS_FtpClearReMsgQueue(t_ReFtpMsg *pMsg);
int XOS_FtpPutReMsg(t_ReFtpMsg *pMsg);
XS8 FTP_PutFileMsgDo(t_XOSCOMMHEAD* pMsg);
XS8 FTP_PutFileReMsgProTskEntry(t_ReFtpMsg* pMsg);
t_ReFtpMsg XOS_FtpGetReMsg();
XS8 FTP_PutMemMsgDo(t_XOSCOMMHEAD* pMsg);
XS8 FTP_PutMemReMsgProTskEntry(t_ReFtpMsg* pMsg);

void XOS_FtpFreeMsgMem(int msgId, void *pBuffer);
XCHAR* XOS_FtpMallocMsgMem(int msgId, XCHAR *pBuffer);
XS32 FTP_CliInit(XVOID);
XVOID FtpCliShowMsgSize(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void XOS_FtpClearXosMsgMem(t_XOSCOMMHEAD* paras);
XS32 FTP_FreeXosPUTFROMMEM_REQ(t_XOSCOMMHEAD *pMsg);
XS32 FTP_FreeXosPUTFROMFILE_REQ(t_XOSCOMMHEAD *pMsg);

/************************************************************************
 ������:FTP_FreeGETTOFILE_REQ
 ����:  �ͷŴӷ����������ļ����ļ���������Ϣ
 ����:  pReq ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS32 FTP_FreeGETTOFILE_REQ(t_XOSCOMMHEAD *pMsg);

/************************************************************************
 ������:FTP_FreeXosPUTFROMFILE_REQ
 ����:  �ͷŴӷ����������ļ����ļ�������Ϣ
 ����:  pReq ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS32 FTP_FreeXosGETTOFILE_REQ(t_XOSCOMMHEAD *pMsg);

/************************************************************************
 ������:FTP_GetFileMsgProTskEntry
 ����: �����ļ��������ļ���������Ϣ
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 FTP_GetFileMsgProTskEntry(t_XOSCOMMHEAD* pMsg);

/************************************************************************
 ������:FTP_GetFileMsgDo
 ����: �����ļ��������ļ���������Ϣ
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 FTP_GetFileMsgDo(t_XOSCOMMHEAD* pMsg, XU32 *pFileSize);


/************************************************************************
 ������:FTP_GetFileReMsgProTskEntry
 ����: �����ļ��������ļ���������Ϣ
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 FTP_GetFileReMsgProTskEntry(t_ReFtpMsg* pMsg);

XS8  XOS_FtpGetFileTest(void);

/************************************************************************
 ������:FTP_SendMsgToCli
 ����:  ������Ӧ��Ϣ���ͻ���
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS32 FTP_Send_GettoFile_Ack_MsgToCli(XU32 ulDestFid, XU32 ulFsmId, XU32 ulRst, XU64 ulSerial, XU32 ulFileSize);

/************************************************************************
 ������:FTP_CoverLowerChar
 ����: ���ַ����еĴ�д��ĸת��ΪСд��ĸ
 ����:  pszIn ��Ҫת�����ַ���
        nLen  �ַ�������
 ���:
 ����: 
 ˵��:
************************************************************************/
void FTP_CoverLowerChar(char *pszIn, int nLen);

/************************************************************************
 ������:FTP_Snprintf_Md5String
 ����: ��md5ת��Ϊ�ַ���
 ����:  pszMd5 ��Ҫת�����ַ���
        nMd5Len  �ַ�������
        pszOut   ת������ַ���
        nOutLen  ת������ַ�������
 ���:
 ����: 
 ˵��:
************************************************************************/
void FTP_Snprintf_Md5String(const char *pszMd5, int nMd5Len, char *pszOut, int nOutLen);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xosftp.h*/
#endif /*XOS_FTP_CLIENT*/


