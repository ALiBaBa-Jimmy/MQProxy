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
                  包含头文件
-------------------------------------------------------------------------*/
#include "xosshell.h"
#include "xosntl.h"

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
#define MAX_USERNAME_LEN (32+1)     /*最大用户名长度*/
#define MAX_PASSWORD_LEN (16+1)     /*最大密码长度*/
#define MAX_DIRECTORY_LEN (256+1)   /*最大路径长度*/
/*  ftp客户端句柄  */
XOS_DECLARE_HANDLE(XOS_HFTPCLT);

/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/

/*ftp的当前状态*/
typedef enum
{
    eFTPSTATEINIT = 0,  /*ftp的初始状态，未连接登录*/
    eFTPCTRLINITED,     /*控制连接初始化*/
    eFTPSTATELOGIN,     /*用户登录成功后的状态*/
    eFTPSTATEDISCON     /*登录成功后FTP连接因故断开*/
}e_FTPSTATE;

/*ftp当前正在处理的事件*/
typedef enum
{
    eFTPNONE = 0,      /*当前没有处理任何事件*/
    eFTPGETTOFILE,     /*下载文件到磁盘文件*/
    eFTPGETTOMEM,      /*下载文件到内存*/
    eFTPPUTFROMFILE,   /*上传本地磁盘文件*/
    eFTPPUTFROMMEM,    /*上传本地内存文件*/
    eFTPCREATFILE,     /*创建一个新文件*/
    eFTPLIST,          /*获取指定目录下的文件列表*/
    eFTPNAME,          /*在控制连接上传输用户名*/
    eFTPPASS,          /*在控制连接上传输密码*/
    eFTPDEFDIR,        /*取默认路径*/
    eFTPCWD,           /*改变当前工作目录*/
    eFTPSIZE,          /*取文件大小*/
    eFTPMKD,           /*新建目录*/
    eFTPRMD,           /*删除目录*/
    eFTPRNCHECK,       /*重命名准备*/
    eFTPRNEXCUTE,      /*进行重命名*/
    eFTPRMF,           /*删除文件*/
    eFTPPASV,          /*发送被动模式请求*/
    eFTPTYPE,          /*传输文件类型*/
    eFTPPORT,          /*发送本地ip和port*/
    eFTPQUIT           /*退出登录*/
}e_FTPCUREVENT;



/*下载时收到的消息情况分类*/
typedef enum
{
    ePREPARE_SUC = 0,     /*只收到下载准备正确*/
    ePRE_AND_TRANS,       /*下载准备和传输成功*/
    ePRE_AND_TRANSFAIL    /*下载准备但传输失败*/
}e_TRANSREPLYINFO;

/*FTP 对象信息结构体*/
typedef struct
{
    t_IPADDR          dataLinkAddr;      /*本地IP和端口*/
    t_IPADDR          dstAddr;           /*ftp服务器IP和端口*/
    t_IPADDR          locAddr;           /*本地ip和端口*/
    t_XOSSEMID        semaphore;         /*信号量*/
    e_FTPSTATE        curState;          /*当前状态*/
    e_FTPCUREVENT     curEvent;          /*当前处理的事件*/
    e_FTPCUREVENT     curTransfer;       /*当前传输任务*/
    XSFILESIZE        fileSize;          /*得到的文件的大小*/
    XS32              ret;               /*当前事件的处理结果*/
    XS32              retOfPut;          /*上传是否异常*/
    e_TRANSREPLYINFO  replyInfoType;     /*用于处理在下载(文件或列表),可能出现的两个应答在一个消息里的情况*/
    t_XINETFD         dataSockId;        /*数据连接信息索引*/
    XU32              count;             /*计数*/
    XUFILESIZE        bufferSize;        /*内存大小*/
    XVOID*            pBuffer;           /*内存指针*/
    FILE              *pFile;            /*文件指针*/
    HLINKHANDLE       linkHandle;        /*链路句柄*/
    PTIMER            timerId;           /*定时器句柄*/
    XS8   passWord[MAX_PASSWORD_LEN];    /*ftp用户密码*/
    XS8   userName[MAX_USERNAME_LEN];    /*ftp用户名*/
    XS8   defDir[MAX_DIRECTORY_LEN];     /*服务器返回的默认路径*/
    XS8   curDir[MAX_DIRECTORY_LEN];     /*当前工作目录*/
    XS8   curFolder[MAX_DIRECTORY_LEN];  /*当前文件夹(删除目录用)*/
    XS8   RemFile[MAX_DIRECTORY_LEN];
    XS8   newFileName[MAX_DIRECTORY_LEN];
} t_XOSFTPCLIENT;

/*数据连接结构体
typedef struct
{
    HLINKHANDLE hDataLink;
    t_XOSFTPCLIENT *pCtrlLink;
}t_FTPDATALINK;*/
/*-------------------------------------------------------------------------
                API 声明
-------------------------------------------------------------------------*/

/************************************************************************
函数名: XOS_FtpLogin
功能：登录ftp服务器
输入：pServAddr         -ftp服务器ip
      pUserName         -用户登录帐号
      pPasswd           -用户登录密码
输出：hFtpClt           -ftp用户句柄
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpLogin(t_IPADDR *pServAddr, XS8 *pUserName, XS8 *pPassWd, XOS_HFTPCLT *hFtpClt);

/************************************************************************
函数名: XOS_FtpGetFileSize
功能：获得ftp服务器上一个文件的大小
输入：hFtpClt            -ftp用户句柄
      pRemFile           -服务器文件
输出：N/A
返回：成功则返回文件大小
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpGetFileSize( XOS_HFTPCLT hFtpClt, XS8 *pRemFile );

/************************************************************************
函数名: XOS_FtpGet
功能：从ftp下载一个文件到本地磁盘
输入：hFtpClt              -ftp用户句柄
      pRemFile             -服务器文件
      pLocFile             -本地文件
      time                 -下载文件允许的最长时间,单位(秒),0表示没限制
输出：N/A
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpGetToFile( XOS_HFTPCLT hFtpClt, XS8 *pRemFile, XS8 *pLocFile, XU32 time);

/************************************************************************
函数名: XOS_FtpGetToMem
功能：从ftp下载一个文件到本地内存
输入：hFtpClt              -ftp用户句柄
      pRemFile             -服务器文件
      pBuff                -本地文件路径
      buffSize             -本地文件名
      time                 -下载文件允许的最长时间,单位(秒),0表示没限制
输出：N/A
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpGetToMem( XOS_HFTPCLT hFtpClt, XS8 *pRemFile, XS8 *pBuff, XU32 buffSize, XU32 time);

/************************************************************************
函数名: XOS_FtpPut
功能：将本地磁盘的文件上传到ftp服务器
输入：hFtpClt              -ftp用户句柄
      pLocFile             -本地文件
      pRemFile             -服务器文件
      time                 -上传文件允许的最长时间,单位(秒),0表示没限制
输出：N/A
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpPutFromFile( XOS_HFTPCLT hFtpClt, XS8 *pLocFile, XS8 *pRemFile, XU32 time);

/************************************************************************
函数名: XOS_FtpPutFromMem
功能：将本地内存的文件上传到ftp服务器
输入：hFtpClt              -ftp用户句柄
      pBuff                -本地内存文件指针
      buffSize             -本地内存文件大小
      pRemFile             -服务器文件名
      time                 -上传文件允许的最长时间,单位(秒),0表示没限制
输出：N/A
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpPutFromMem( XOS_HFTPCLT hFtpClt, XS8 *pBuff, XU32 buffSize, XS8 *pRemFile, XU32 time);

/************************************************************************
函数名: XOS_FtpCurrentWorkDir
功能：取得当前工作目录
输入：hFtpClt           -ftp用户句柄
输出：pWorkDir          -当前工作目录
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpCurrentWorkDir( XOS_HFTPCLT hFtpClt, XVOID **pWorkDir );

/************************************************************************
函数名: XOS_FtpChangeWorkDir
功能：改变当前工作目录
输入：hFtpClt      -ftp用户句柄
      pWorkDir     -新的当前工作目录
输出：N/A
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpChangeWorkDir( XOS_HFTPCLT hFtpClt, XS8 *pWorkDir );

/************************************************************************
函数名: XOS_FtpMakeDir
功能：在当前工作目录下创建一个新的文件夹
输入：hFtpClt     -ftp用户句柄
      pNewFolder  -要创建的文件夹名
输出：N/A
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpMakeDir( XOS_HFTPCLT hFtpClt, XS8 *pNewFolder );

/************************************************************************
函数名: XOS_FtpRemoveDir
功能：在当前工作目录下删除一个文件夹
输入：hFtpClt         -ftp用户句柄
      pFolderName     -要删除的文件夹名
输出：N/A
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpRemoveDir( XOS_HFTPCLT hFtpClt, XS8 *pFolderName );

/************************************************************************
函数名: XOS_FtpRenameDir
功能：改变当前工作目录下一个文件夹的名字
输入：hFtpClt          -ftp用户句柄
      pOldFolder       -旧的文件夹名
      pNewFolder       -新的文件夹名
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpRenameDir( XOS_HFTPCLT hFtpClt, XS8* pOldFolder, XS8 *pNewFolder );

/************************************************************************
函数名: XOS_FtpCreatFile
功能：在当前工作目录下创建一个新的文件
输入：hFtpClt    -ftp用户句柄
      pFileName  -要创建的文件的名字
输出：N/A
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpCreatFile( XOS_HFTPCLT hFtpClt, XS8 *pFileName );

/************************************************************************
函数名: XOS_FtpDeleteFile
功能：在当前工作目录下删除一个存在的文件
输入：hFtpClt    -ftp用户句柄
      pFileName  -当前工作目录下的一个文件的名字
输出：N/A
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpDeleteFile( XOS_HFTPCLT hFtpClt, XS8 *pFileName );

/************************************************************************
函数名: XOS_FtpRenameFile
功能：将当前工作目录下的一个文件改名
输入：hFtpClt                -ftp用户句柄
      pOldFileName           -当前工作目录下的一个文件的名字
      pNewFileName           -新的文件名
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpRenameFile( XOS_HFTPCLT hFtpClt, XS8 *pOldFileName, XS8 *pNewFileName );

/************************************************************************
函数名: XOS_FtpClose
功能：与ftp服务器断开控制连接,退出登录
输入：hFtpClt       -ftp用户句柄
输出：N/A
返回：成功则返回XSUCC
      失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpClose( XOS_HFTPCLT *hFtpClt );

XS8 XOS_FtpInit( XVOID *t, XVOID *v );
XS8 FTP_TimerProc( t_BACKPARA* pBackPara);
XS8 FTP_MsgProc( XVOID* pMsgP, XVOID* sb);

/*ftp管理结构*/
typedef struct
{
    XBOOL initialized;  /*内存初始化的标志*/

    t_XOSMUTEXID contrlTblLock; /* 增加删除ftp登录用户时上琐*/
    XOS_HARRAY   ftpConnectTbl;/*用户信息记录表*/
}t_FTPMNT;

/*上传下载任务传入参数*/
typedef struct
{
    t_IPADDR ftpServAddr;    /*服务器端的地址*/
    t_XINETFD *pSockId;      /*本端sock*/
    e_FTPCUREVENT  curEvent; /*当前事件*/
    XSFILESIZE      buffLen;       /*上传内容的长度， 文件大小或者内存块大小
                                            文件列表是输出参数，实际列表大小*/

    XUFILESIZE ulDownLen;          /*当前下载的文件或内存大小*/

    FILE *pFile;             /*文件上传 和下载时用到,要上传的文件或者要下载的文件指针*/

    XCHAR * pBuffer;         /*内存块上传和下载时候可以用到,上传或者下载内存的首地址
                                              文件列表时，为输出参数*/
    t_XOSSEMID  *pSem;       /*信号量用于下载操作时两条链路的同步*/
}t_FTPDATALINKENTRY;

/*----------------------bruce add -------------------------------------*/
/*消息结构*/

typedef enum
{
    FTP_QUE_NULL = 0,
    FTP_QUE_FULL,
    FTP_QUE_FAI,
    FTP_QUE_SUC,
}e_FTP_ADD;

/*ftp上传内存数据到服务器*/
typedef struct
{
    /*消息流水号*/
    XU64 ulSerial; 
    t_IPADDR destAddr; /*服务器地址*/

    XU32 ulUserLen;    /*用户名长度*/
    XU8 *ucpUser;/*用户名*/

    XU32 ulPassLen;/*密码长度*/
    XU8 *ucpPass;/*密码*/
    
    XU32 ulRelDirLen; /*相对目录长度*/
    XU8 *ucpReldir; /*相对目录(相对于当前的工作目录),需要用户释放*/

    XU32 ulFileNameLen;/*保存至文件名长度*/
    XU8 *ucpFileName;/*保存至文件名*/
    
    XU32 ulBufferLen;    /*数据长度*/
    XU8 *ucpBuffer;/*需上传数据缓冲指针,需要用户释放*/

}t_PUTFROMMEM_REQ;

/*ftp上传文件到服务器*/
typedef struct
{
    XU64 ulSerial; 
    t_IPADDR destAddr;            /*服务器地址*/

    XU32 ulUserLen;                  /*用户名长度*/
    XU8 *ucpUser;                 /*用户名*/

    XU32 ulPassLen;               /*密码长度*/
    XU8 *ucpPass;                 /*密码*/
    
    XU32 ulRelDirLen;             /*相对目录长度*/
    XU8 *ucpReldir;               /*相对目录(相对于当前的工作目录),需要用户释放*/

    XU32 ulFileNameLen;           /*保存至文件名长度*/
    XU8 *ucpFileName;             /*保存至文件名*/
    
    XU32 ulUploadFilePathLen;              /*需要上传的文件名路径长度*/
    XU8 *ucpUploadFilePath;       /*需要上传的文件名路径*/

}t_PUTFROMFILE_REQ;

/*ftp从服务器下载文件到文件*/
typedef struct
{
    XU64 ulSerial; 
    t_IPADDR destAddr;            /*服务器地址*/

    XU32 ulUserLen;                  /*用户名长度*/
    XU8 *ucpUser;                 /*用户名*/

    XU32 ulPassLen;               /*密码长度*/
    XU8 *ucpPass;                 /*密码*/
    
    XU32 ulRelDirLen;             /*相对目录长度*/
    XU8 *ucpReldir;               /*相对目录(相对于当前的工作目录),需要用户释放*/

    XU32 ulFileNameLen;           /*服务器文件名长度*/
    XU8 *ucpFileName;             /*文件名*/
    
    XU32 ulSaveFilePathLen;          /*需要保存的文件名路径长度*/
    XU8 *ucpSaveFilePath;         /*文件名路径*/

    XU32 ulTime;                  /*下载时间*/
    XU16 ulMaxReDown;             /*最大下载次数*/

    XU32 ulMd5Len;                /*md5长度*/
    XU8 *ucpMd5;                  /*md5串*/

}t_GETTOFILE_REQ;


/*响应消息*/
typedef struct
{
    /*消息流水号*/
    XU64 ulSerial; 
    XU32 ulResult; /*请求的结果*/
}t_PUTFROMMEM_RSP;

/*响应消息*/
typedef struct
{
    /*消息流水号*/
    XU64 ulSerial; 
    XU32 ulResult; /*请求的结果*/
    XU32 ulFileSize; /*文件大小*/
}t_GETTOFILE_RSP;


/*响应消息状态*/
typedef enum
{
    eFTP_SUCC = 0,
    eFTP_PARA_INVALID = 1, /*参数不合法,请检测参数的格式*/
    eFTP_LOGIN_FAILED,/*登录失败，可能是用户名与密码不匹配*/
    eFTP_TRAN_FAILED,/*传输失败,可能是路径或权限不够*/
    eFTP_LOC_FILE_FAILED, /*本地文件打开失败*/
    eFTP_LOC_FILE_EXIST, /*本地文件存在*/
    eFTP_REMOTE_FILE_FAILED, /*远端文件不存在*/
    eFTP_DEL_FILE_FAILED,/*删除本地文件失败*/
    eFTP_DWONLOADING,    /*文件重新下载中*/
    eFTP_GENERATE_MD5_ERROR, /*产生md5错误*/
    eFTP_MD5_ERROR,      /*md5校验错误*/
}e_FTPRSP_STATUS;




/*客户消息类型*/
typedef enum
{
    eFTP_NULL,
    eFTP_GETTOFILE,     /*下载文件到磁盘文件*/
    eFTP_GETTOFILEAck,
    eFTP_GETTOMEM,      /*下载文件到内存*/
    eFTP_GETTOMEMAck,
    eFTP_PUTFROMFILE,   /*上传本地磁盘文件*/
    eFTP_PUTFROMFILEAck,
    eFTP_PUTFROMMEM,    /*上传本地内存文件*/
    eFTP_PUTFROMMEMAck,
    eFTP_LIST,          /*获取指定目录下的文件列表*/
    eFTP_LISTAck
}e_CLIEVENT;


/*重发消息*/
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
 函数名:FTP_FreeGETTOFILE_REQ
 功能:  释放从服务器下载文件到文件的请求消息
 输入:  pReq 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_FreeGETTOFILE_REQ(t_XOSCOMMHEAD *pMsg);

/************************************************************************
 函数名:FTP_FreeXosPUTFROMFILE_REQ
 功能:  释放从服务器下载文件到文件请求消息
 输入:  pReq 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_FreeXosGETTOFILE_REQ(t_XOSCOMMHEAD *pMsg);

/************************************************************************
 函数名:FTP_GetFileMsgProTskEntry
 功能: 下载文件到本地文件的请求消息
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_GetFileMsgProTskEntry(t_XOSCOMMHEAD* pMsg);

/************************************************************************
 函数名:FTP_GetFileMsgDo
 功能: 下载文件到本地文件的请求消息
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_GetFileMsgDo(t_XOSCOMMHEAD* pMsg, XU32 *pFileSize);


/************************************************************************
 函数名:FTP_GetFileReMsgProTskEntry
 功能: 下载文件到本地文件的请求消息
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_GetFileReMsgProTskEntry(t_ReFtpMsg* pMsg);

XS8  XOS_FtpGetFileTest(void);

/************************************************************************
 函数名:FTP_SendMsgToCli
 功能:  发送响应消息给客户端
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_Send_GettoFile_Ack_MsgToCli(XU32 ulDestFid, XU32 ulFsmId, XU32 ulRst, XU64 ulSerial, XU32 ulFileSize);

/************************************************************************
 函数名:FTP_CoverLowerChar
 功能: 将字符串中的大写字母转换为小写字母
 输入:  pszIn 需要转换的字符串
        nLen  字符串长度
 输出:
 返回: 
 说明:
************************************************************************/
void FTP_CoverLowerChar(char *pszIn, int nLen);

/************************************************************************
 函数名:FTP_Snprintf_Md5String
 功能: 将md5转换为字符串
 输入:  pszMd5 需要转换的字符串
        nMd5Len  字符串长度
        pszOut   转换后的字符串
        nOutLen  转换后的字符串长度
 输出:
 返回: 
 说明:
************************************************************************/
void FTP_Snprintf_Md5String(const char *pszMd5, int nMd5Len, char *pszOut, int nOutLen);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xosftp.h*/
#endif /*XOS_FTP_CLIENT*/


