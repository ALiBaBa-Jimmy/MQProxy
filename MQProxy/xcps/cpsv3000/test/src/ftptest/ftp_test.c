
#include "ftp_test.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 FtpTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 FtpTestInit( XVOID*, XVOID*);
    XS8 FtpTestClose(XVOID *Para1, XVOID *Para2);
    XS8 FtpTestMsgPro( XVOID* pMsgP, XVOID* sb);
    
    XS32 PutFileFromMem(XS8* ipaddr, XS32 port, XS8* username, XS8* passwd, XS8* message, XS8* remotefile);
    XS32 PutFile(XS8* ipaddr, XS32 port, XS8* username, XS8* passwd, XS8* localfile, XS8* remotefile);
    XS32 GetFile(XS8* ipaddr, XS32 port, XS8* username, XS8* passwd, XS8* localfile, XS8* remotefile);
    
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherFtpTestFid ={
    { "FID_FTP_TEST",  XNULL, FID_FTP_TEST,},
    { FtpTestInit, NULL, FtpTestClose,},
    { FtpTestMsgPro, XNULL,}, eXOSMode, NULL
};


// ----------------------------- MyTest -----------------------------


XS32 FtpTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherFtpTestFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_FTP_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_FTP_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList,XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "FtpTest", "Ftp Test", "");
    XOS_RegistCommand(promptID, FtpTestCmd, "PutFromMem", "Put memory buffer as file to ftp server", "ipaddr port username passwd message remotefile");
    XOS_RegistCommand(promptID, FtpTestCmd, "PutFile", "Put localfile to ftp server", "ipaddr port username passwd localfile remotefile");
    XOS_RegistCommand(promptID, FtpTestCmd, "GetFile", "get file from remote server", "ipaddr port usename passwd remotefile localfile");
    
    return XSUCC;
}

XS8 FtpTestInit( XVOID* argv1, XVOID* argv2)
{
    MMInfo("FtpTestInit begin");

    MMInfo("FtpTestInit end");
    return XSUCC;
}


XS8 FtpTestClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "FtpTestClose ...");
    return 0;
}

XS8 FtpTestMsgPro( XVOID* pMsgP, XVOID* sb)
{
    t_XOSCOMMHEAD* pCpsHead = XNULL;
    t_PUTFROMMEM_RSP* pPutFromMemRsp = XNULL;
    t_GETTOFILE_RSP* pGetFileRsp = XNULL;

    pCpsHead = (t_XOSCOMMHEAD*)pMsgP;

    switch(pCpsHead->msgID) {
    case eFTP_PUTFROMMEMAck:
        pPutFromMemRsp = (t_PUTFROMMEM_RSP*)pCpsHead->message;
        if(pPutFromMemRsp->ulResult == eFTP_SUCC) {
            XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "eFTP_PUTFROMMEMAck result ok");
        } else {
            XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "eFTP_PUTFROMMEMAck invalid, result:%d", pPutFromMemRsp->ulResult);
        }
        break;
    case eFTP_PUTFROMFILEAck:
        pPutFromMemRsp = (t_PUTFROMMEM_RSP*)pCpsHead->message;
        if(pPutFromMemRsp->ulResult == eFTP_SUCC) {
            XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "eFTP_PUTFROMFILEAck result ok");
        } else {
            XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "eFTP_PUTFROMFILEAck invalid, result:%d", pPutFromMemRsp->ulResult);
        }
        break;
    case eFTP_GETTOFILEAck:
        pGetFileRsp = (t_GETTOFILE_RSP*)pCpsHead->message;
        if(pGetFileRsp) {
            if(pGetFileRsp->ulResult == eFTP_SUCC) {
                XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "eFTP_GETTOFILEAck result ok, filesize=%d", pGetFileRsp->ulFileSize);
            } else {
                XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "eFTP_GETTOFILEAck invalid, result:%d", pGetFileRsp->ulResult);
            }
        }
        break;
    default:
        break;
    }
    return 0;
}

void FtpTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    if(siArgc < 2) {
        return;
    }
    if(XOS_StrCmp(ppArgv[0], "PutFromMem") == 0) {
        if(siArgc < 7) {
            XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "FtpTestCmd, missing parameters");
        } else {
            if(PutFileFromMem(ppArgv[1], atoi(ppArgv[2]), ppArgv[3], ppArgv[4], ppArgv[5], ppArgv[6]) != XSUCC) {
                XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "Put file from memory fail");
            }
        }
    } else if(XOS_StrCmp(ppArgv[0], "PutFile") == 0) {
        if(siArgc < 7) {
            XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "FtpTestCmd, missing parameters");
        } else {
            if(PutFile(ppArgv[1], atoi(ppArgv[2]), ppArgv[3], ppArgv[4], ppArgv[5], ppArgv[6]) != XSUCC) {
                XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "Put file to remote server fail!!");
            }
        }
    } else if(XOS_StrCmp(ppArgv[0], "GetFile") == 0) {
        if(siArgc < 7) {
            XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "FtpTestCmd, missing parameters");
        } else {
            if(GetFile(ppArgv[1], atoi(ppArgv[2]), ppArgv[3], ppArgv[4], ppArgv[5], ppArgv[6]) != XSUCC) {
                XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "get file from remote server fail!!");
            }
        }
    }
}

XS32 PutFileFromMem(XS8* ipaddr, XS32 port, XS8* username, XS8* passwd, XS8* message, XS8* remotefile)
{
    t_XOSCOMMHEAD* pCpsHead = XNULL;
    t_PUTFROMMEM_REQ*   pPutMemData = XNULL;

    char ucRelDir[255] = ".";

    pCpsHead = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_FTP_TEST, sizeof(t_PUTFROMMEM_REQ));
    if( XNULL == pCpsHead )
    {
        return XERROR;
    }
    pPutMemData = (t_PUTFROMMEM_REQ*)((XS8*)pCpsHead + sizeof(t_XOSCOMMHEAD));
    XOS_MemSet(pPutMemData, 0, sizeof(t_PUTFROMMEM_REQ));
    
    pCpsHead->datasrc.PID   = XOS_GetLocalPID();
    pCpsHead->datasrc.FID = FID_FTP_TEST;
    pCpsHead->datasrc.FsmId = BLANK_ULONG;
    pCpsHead->datadest.PID  = XOS_GetLocalPID();
    pCpsHead->datadest.FID  = FID_FTP;
    pCpsHead->datadest.FsmId = BLANK_ULONG;
    pCpsHead->msgID         = eFTP_PUTFROMMEM;
    pCpsHead->subID         = BLANK_USHORT;
    pCpsHead->prio          = eNormalMsgPrio;
    pCpsHead->length        = sizeof(t_PUTFROMMEM_REQ) + sizeof(t_XOSCOMMHEAD);
    pCpsHead->message       = (XS8*)pPutMemData;
    

    /*服务器地址*/
    XOS_StrtoIp(ipaddr, &pPutMemData->destAddr.ip);
    pPutMemData->destAddr.port = port;

    /*用户名*/
    pPutMemData->ucpUser = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutMemData->ucpUser)
    {
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutMemData->ucpUser, 0, 128);
    pPutMemData->ulUserLen =strlen(username);
    XOS_StrNcpy(pPutMemData->ucpUser, username, 128);

    /*密码*/
    pPutMemData->ucpPass = XOS_MemMalloc(FID_FTP_TEST, 128);
    if(XNULL == pPutMemData->ucpPass)
    {
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpUser);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutMemData->ucpPass, 0, 128);
    pPutMemData->ulPassLen = strlen(passwd);
    XOS_StrNcpy(pPutMemData->ucpPass, passwd, 128);

    /*相对路径*/
    pPutMemData->ucpReldir = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutMemData->ucpReldir)
    {
        XOS_MemFree(FID_FTP_TEST, pPutMemData->ucpUser);
        XOS_MemFree(FID_FTP_TEST, pPutMemData->ucpPass);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutMemData->ucpReldir, 0, 128);        
    pPutMemData->ulRelDirLen = strlen(ucRelDir);
    XOS_StrNcpy(pPutMemData->ucpReldir, ucRelDir, 128);

    /*需要保存的文件名*/
    pPutMemData->ucpFileName = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutMemData->ucpFileName)
    {
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpReldir);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutMemData->ucpFileName, 0, 128);
    pPutMemData->ulFileNameLen = strlen(remotefile);
    XOS_StrNcpy(pPutMemData->ucpFileName, remotefile, 128);

    /*内容*/
    pPutMemData->ucpBuffer = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutMemData->ucpBuffer)
    {
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpReldir);
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpFileName);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutMemData->ucpBuffer, 0, 128);
    pPutMemData->ulBufferLen = strlen(message);
    XOS_StrNcpy(pPutMemData->ucpBuffer, message, strlen(message));


    if(XERROR == XOS_MsgSend(pCpsHead))
    {
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpReldir);
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpFileName);
        XOS_MemFree(FID_FTP_TEST,pPutMemData->ucpBuffer);
        
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
    }
    return XSUCC;
}

XS32 PutFile(XS8* ipaddr, XS32 port, XS8* username, XS8* passwd, XS8* localfile, XS8* remotefile)
{
    t_XOSCOMMHEAD* pCpsHead = XNULL;
    t_PUTFROMFILE_REQ*   pPutFileData = XNULL;
    char ucRelDir[255] = ".";

    pCpsHead = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_FTP_TEST, sizeof(t_PUTFROMFILE_REQ));
    pPutFileData = (t_PUTFROMFILE_REQ*)((XS8*)pCpsHead + sizeof(t_XOSCOMMHEAD));

    XOS_MemSet(pPutFileData, 0, sizeof(t_PUTFROMFILE_REQ));
    
    pCpsHead->datasrc.PID   = XOS_GetLocalPID();
    pCpsHead->datasrc.FID = FID_FTP_TEST;
    pCpsHead->datasrc.FsmId = BLANK_ULONG;
    pCpsHead->datadest.PID  = XOS_GetLocalPID();
    pCpsHead->datadest.FID  = FID_FTP;
    pCpsHead->datadest.FsmId = BLANK_ULONG;
    pCpsHead->msgID         = eFTP_PUTFROMFILE;
    pCpsHead->subID         = BLANK_USHORT;
    pCpsHead->prio          = eNormalMsgPrio;
    pCpsHead->length        = sizeof(t_PUTFROMFILE_REQ);
    pCpsHead->message       = (XS8*)pPutFileData;

    /*服务器地址*/
    XOS_StrtoIp(ipaddr, &pPutFileData->destAddr.ip);
    pPutFileData->destAddr.port = port;

    /*用户名*/
    pPutFileData->ucpUser = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutFileData->ucpUser)
    {
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutFileData->ucpUser, 0, 128);
    pPutFileData->ulUserLen = XOS_MIN(strlen(username), 128-1);
    XOS_StrNcpy(pPutFileData->ucpUser, username, pPutFileData->ulUserLen);

    /*密码*/
    pPutFileData->ucpPass = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutFileData->ucpPass)
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutFileData->ucpPass, 0, 128);
    pPutFileData->ulPassLen = XOS_MIN(strlen(passwd), 128-1);
    XOS_StrNcpy(pPutFileData->ucpPass, passwd, pPutFileData->ulPassLen);

    /*相对路径*/
    pPutFileData->ucpReldir = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutFileData->ucpReldir)
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpPass);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    
    memset(pPutFileData->ucpReldir, 0, 128);
    pPutFileData->ulRelDirLen = XOS_MIN(strlen(ucRelDir), 128-1);
    XOS_StrNcpy(pPutFileData->ucpReldir, ucRelDir, pPutFileData->ulRelDirLen);

    /*需要服务器保存的文件名*/
    pPutFileData->ucpFileName = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutFileData->ucpFileName)
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpReldir);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutFileData->ucpFileName, 0, 128);
    pPutFileData->ulFileNameLen = XOS_MIN(strlen(remotefile), 128-1);
    XOS_StrNcpy(pPutFileData->ucpFileName, remotefile, pPutFileData->ulFileNameLen);

    /*需要上传的文件*/
    pPutFileData->ucpUploadFilePath = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutFileData->ucpUploadFilePath)
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpReldir);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpFileName);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutFileData->ucpUploadFilePath, 0, 128);
    pPutFileData->ulUploadFilePathLen = XOS_MIN(strlen(localfile),128-1);
    XOS_StrNcpy(pPutFileData->ucpUploadFilePath, localfile, pPutFileData->ulUploadFilePathLen);


    if(XERROR == XOS_MsgSend(pCpsHead))
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpReldir);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpFileName);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUploadFilePath);
        
        
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
    }

    return XSUCC;
}

XS32 GetFile(XS8* ipaddr, XS32 port, XS8* username, XS8* passwd, XS8* localfile, XS8* remotefile)
{
    t_XOSCOMMHEAD* pCpsHead = XNULL;
    t_GETTOFILE_REQ*   pPutFileData = XNULL;

    char ucRelDir[255] = ".";

    pCpsHead = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_FTP_TEST, sizeof(t_GETTOFILE_REQ));
    pPutFileData = (t_GETTOFILE_REQ*)((XS8*)pCpsHead + sizeof(t_XOSCOMMHEAD));
    
    XOS_MemSet(pPutFileData, 0, sizeof(t_GETTOFILE_REQ));
    
    pCpsHead->datasrc.PID   = XOS_GetLocalPID();
    pCpsHead->datasrc.FID = FID_FTP_TEST;
    pCpsHead->datasrc.FsmId = BLANK_ULONG;
    pCpsHead->datadest.PID  = XOS_GetLocalPID();
    pCpsHead->datadest.FID  = FID_FTP;
    pCpsHead->datadest.FsmId = BLANK_ULONG;
    pCpsHead->msgID         = eFTP_GETTOFILE;
    pCpsHead->subID         = BLANK_USHORT;
    pCpsHead->prio          = eNormalMsgPrio;
    pCpsHead->length        = sizeof(t_GETTOFILE_REQ);
    pCpsHead->message       = (XS8*)pPutFileData;

    /*服务器地址*/
    XOS_StrtoIp(ipaddr, &pPutFileData->destAddr.ip);
    pPutFileData->destAddr.port = port;

    /*用户名*/
    pPutFileData->ucpUser = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutFileData->ucpUser)
    {
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutFileData->ucpUser, 0, 128);
    pPutFileData->ulUserLen = XOS_MIN(strlen(username),128-1);
    XOS_StrNcpy(pPutFileData->ucpUser, username, pPutFileData->ulUserLen);

    /*密码*/
    pPutFileData->ucpPass = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutFileData->ucpPass)
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutFileData->ucpPass, 0, 128);
    pPutFileData->ulPassLen = XOS_MIN(strlen(passwd), 128-1);
    XOS_StrNcpy(pPutFileData->ucpPass, passwd, pPutFileData->ulPassLen);

    /*相对路径*/
    pPutFileData->ucpReldir = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutFileData->ucpReldir)
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutFileData->ucpReldir, 0, 128);        
    pPutFileData->ulRelDirLen = XOS_MIN(strlen(ucRelDir), 128-1);
    XOS_StrNcpy(pPutFileData->ucpReldir, ucRelDir, pPutFileData->ulRelDirLen);

    /*需要下载的文件名*/
    pPutFileData->ucpFileName = XOS_MemMalloc(FID_FTP_TEST,128);
    if(XNULL == pPutFileData->ucpFileName)
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpReldir);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutFileData->ucpFileName, 0, 128);
    pPutFileData->ulFileNameLen = XOS_MIN(strlen(remotefile), 128-1);
    XOS_StrNcpy(pPutFileData->ucpFileName, remotefile, pPutFileData->ulFileNameLen);

    /*本地保存的文件的绝对路径*/
    pPutFileData->ucpSaveFilePath = XOS_MemMalloc(FID_FTP,128);
    if(XNULL == pPutFileData->ucpSaveFilePath)
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpReldir);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpFileName);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }
    memset(pPutFileData->ucpSaveFilePath, 0, 128);
    pPutFileData->ulSaveFilePathLen = XOS_MIN(strlen(localfile),128-1);
    XOS_StrNcpy(pPutFileData->ucpSaveFilePath, localfile, pPutFileData->ulSaveFilePathLen);

    /*下载时间200秒*/
    pPutFileData->ulTime = 3000;

    /*重新下载的次数*/
    pPutFileData->ulMaxReDown = 2;

    pPutFileData->ucpMd5= XOS_MemMalloc(FID_FTP_TEST, 128);
    if(XNULL == pPutFileData->ucpSaveFilePath)
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpReldir);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpFileName);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpSaveFilePath);
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
        return XERROR;
    }

    memset(pPutFileData->ucpMd5, 0, 128);

    if(XERROR == XOS_MsgSend(pCpsHead))
    {
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpUser);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpPass);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpReldir);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpFileName);
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpSaveFilePath);    
        XOS_MemFree(FID_FTP_TEST,pPutFileData->ucpMd5);    
        XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
    }

    return XSUCC;
}
