/****************************************************************************
*��Ȩ     : Xinwei Telecom Technology Inc
*�ļ���   : oam_db_cli.cpp
*�ļ����� : ����������Դ�ļ�
*����     : xiaohuiming
*�������� : 2014-07-15
*�޸ļ�¼ :
****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "oam_cli.h" 
#include "oam_main.h"
#include "oam_file.h"

/*����ģ���ϵ�ṹ*/
XEXTERN OAM_TID_MID_T g_TidMidArray[MAX_TB_NUM];
XEXTERN XU32 g_TidMidCnt;
XEXTERN OAM_TID_MID_T g_AsyncTidMidArray[MAX_TB_NUM];
XEXTERN XU32 g_AsyncTidMidCnt;


//#pragma warning(disable:4996)
/***************************************************************************
*������:OAM_CliRegister
*����  :������ע��ӿ�
*����  :
*���  :
*����  :
*         XVOID
*˵��  :
***************************************************************************/
XVOID OAM_CliRegister()
{
    XS32 siCmdIndex = 0;

    siCmdIndex = XOS_RegistCmdPrompt(SYSTEM_MODE, "oam", "oam command line", 
                                                                    "no para" );
    
    (void)XOS_RegistCommand(siCmdIndex, 
                            (CmdHandlerFunc)OAM_xmlFileRead, 
                            "readxmlfile", 
                            "readxmlfile", 
                            "no para");
    (void)XOS_RegistCommand(siCmdIndex, 
                            (CmdHandlerFunc)OAM_xmlFileWrite, 
                            "writexmlfile", 
                            "writexmlfile", 
                            "no para");
    (void)XOS_RegistCommand(siCmdIndex, 
                            (CmdHandlerFunc)OAM_CfgMsgTest, 
                            "oamcfgmsgtest", 
                            "para1: FID; para2: operType[1:add;2:del;3:mod]; para3: tableId; para4: data", 
                            "no para");
    (void)XOS_RegistCommand(siCmdIndex, 
                            (CmdHandlerFunc)OAM_ShowInfo, 
                            "showboardinfo", 
                            "showboardinfo", 
                            "no para");
    (void)XOS_RegistCommand(siCmdIndex, 
                            (CmdHandlerFunc)OAM_ShowMTInfo, 
                            "showtableinfo", 
                            "showtableinfo", 
                            "no para");
    (void)XOS_RegistCommand(siCmdIndex, 
                            (CmdHandlerFunc)OAM_FmMsgTest, 
                            "oamfmmsgtest", 
                            "para1: 0: recover; 1: notify", 
                            "no para");
    return;
}



/***************************************************************************
*������:ShowMySQLScriptInfo
*����  :��ʾsql�ű�ִ�к���
*����  :
*           CLI_ENV *pCliEnv,
*           XS32 siArgc,
*           XCHAR **ppArgv
*���  :
*����  :
*         XVOID
*˵��  :
***************************************************************************/
XVOID OAM_xmlFileRead(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv)
{
    XCHAR ucDir[256] = "D:\\test\\test.xml";

    XU32 start_ticks1 = 0;
    XU32 start_ticks  = 0;
    XU32 end_ticks    = 0;
    XU32 end_ticks1   = 0; 

    start_ticks = XOS_GetSysTicks();
    if(XERROR == OAM_XmlTblInfoGet(ucDir,200,1000, OAM_CFG_SYNC))
    {
        return;
    }
    end_ticks = XOS_GetSysTicks();
    printf("OAM_XmlFileRead time:%d\r\n", (end_ticks - start_ticks ));
    start_ticks1 = XOS_GetSysTicks();
    //OAM_PrintXmlData();
    //OAM_CfgDataHashShow(pCliEnv);
    end_ticks1 = XOS_GetSysTicks();
    printf("OAM_CfgDataHashShow time:%d\r\n", (end_ticks1 - start_ticks1 ));
}
extern XS8 g_xmlFilePath[FILE_PATH_MAX_LEN];
    
XVOID OAM_xmlFileWrite(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv)
{
    AGT_OAM_CFG_REQ_T stCfgData;
    XOS_MemSet(&stCfgData, 0, sizeof(AGT_OAM_CFG_REQ_T));
    stCfgData.uiIndex = 2;
    stCfgData.uiRecNum =1;
    stCfgData.uiTableId=300;
    stCfgData.usNeId = 52;
    stCfgData.pData = (XS8*)XOS_MemMalloc(1, 32);

    XOS_MemCpy(stCfgData.pData, "1234567890123456789", 19);
    XOS_MemSet(g_xmlFilePath, 0, FILE_PATH_MAX_LEN);
    XOS_StrCpy(g_xmlFilePath, "D:\\test\\test.xml");
    OAM_XmlCfgDataWrite(&stCfgData, 3);
}


/***************************************************************************
*������:OAM_CfgMsgTest
*����  :OAM����������Ϣ������
*����  :
*           CLI_ENV *pCliEnv,
*           XS32 siArgc,
*           XCHAR **ppArgv
*���  :
*����  :
*         XVOID
*˵��  :
***************************************************************************/
XVOID OAM_CfgMsgTest(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv)
{
    XU32 uiFid;
    XU32 uiOperType;
    XU32 uiTableId;

    typedef struct test_cfg_req_t{
        XU32 uiIndex;        //����
        XU16 usNeId;         //��ԪID
        XU16 usModuleId;     //ģ��ID
        XU32 uiOperType;     //��������
        XU32 uiTableId;      //��ID
        XU32 uiRecNum;       //���¼��
        XU32 uiMsgLen;       //��Ϣ����
        XS8  ucData[1024];   //��������
    }TEST_CFG_REQ_T;

    TEST_CFG_REQ_T* pAgtOamCfgReq = XNULL;

    XU32 uiMsgLen = 0;

    /*����������Ҫ4������*/
    if(4+1 > siArgc)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "invalid command line parameter num\r\n");
        return;
    }

    while(ppArgv[4][uiMsgLen] != '\0')
    {
        uiMsgLen++;
    }

    if(uiMsgLen > 0)
    {
        XOS_StrToNum(ppArgv[1], &uiFid);
        XOS_StrToNum(ppArgv[2], &uiOperType);
        XOS_StrToNum(ppArgv[3], &uiTableId);

        pAgtOamCfgReq = (TEST_CFG_REQ_T*)XOS_MemMalloc(FID_OAM, sizeof(TEST_CFG_REQ_T));
        if(XNULL == pAgtOamCfgReq)
        {
            XOS_PRINT(MD(FID_OAM, PL_ERR), "XOS_MemMalloc failed\r\n");
            return;
        }
        XOS_MemSet(pAgtOamCfgReq, 0, sizeof(TEST_CFG_REQ_T));
        XOS_MemCpy(pAgtOamCfgReq->ucData, ppArgv[4], uiMsgLen);

        pAgtOamCfgReq->uiIndex    = 0;
        pAgtOamCfgReq->usNeId     = 0;
        pAgtOamCfgReq->usModuleId = 0;
        pAgtOamCfgReq->uiTableId  = uiTableId;
        pAgtOamCfgReq->uiRecNum   = 1;
        pAgtOamCfgReq->uiMsgLen   = uiMsgLen;

        switch(uiOperType)
        {
        case 1:
            pAgtOamCfgReq->uiOperType = OAM_CFG_ADD;
            break;
        case 2:
            pAgtOamCfgReq->uiOperType = OAM_CFG_DEL;
            break;
        case 3:
            pAgtOamCfgReq->uiOperType = OAM_CFG_MOD;
            break;
        default:
            XOS_PRINT(MD(FID_OAM, PL_ERR), "invalid OperType\r\n");
            return;
        }

        if(XSUCC != OAM_CfgMsgSend(uiFid, uiOperType, pAgtOamCfgReq, uiMsgLen))
        {
            XOS_PRINT(MD(FID_OAM, PL_WARN), "OAM_CfgMsgSend failed\r\n");
            PTR_MEM_FREE(FID_OAM, pAgtOamCfgReq);
        }
    }
}


/***************************************************************************
*������:ShowMySQLScriptInfo
*����  :��ʾsql�ű�ִ�к���
*����  :
*           CLI_ENV *pCliEnv,
*           XS32 siArgc,
*           XCHAR **ppArgv
*���  :
*����  :
*         XVOID
*˵��  :
***************************************************************************/
XVOID OAM_ShowInfo(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv)
{
    XU32 uiNeID = 0;
    XU32 uiPID = 0;
    XU32 uiFrameId = 0;
    XU32 uiSlotId = 0;
    
    #ifdef XOS_LINUX
        if(XERROR == (uiNeID = XOS_GetNeId()))
        {
            XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetNeId failed.\r\n", uiNeID);
            return;
        }

        if(XERROR == (uiPID = XOS_GetLogicPid()))
        {
            XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetLogicPid failed.\r\n",uiPID);
            return;
        }

        if(XERROR == (uiFrameId = XOS_GetShelfNum()))
        {
            XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetShelfID[%d] failed.\r\n",uiFrameId);
            return;
        }

        if(XERROR == (uiSlotId  = XOS_GetSlotNum()))
        {
            XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetSlotNum[%d] failed.\r\n",uiSlotId);
            return;
        }
    
    #endif
        
    XOS_CliExtPrintf(pCliEnv, "/************BOARD INFO************/\r\n");
    XOS_CliExtPrintf(pCliEnv, "NE    ID:%d\r\n", uiNeID);
    XOS_CliExtPrintf(pCliEnv, "PROC  ID:%d\r\n", uiPID);
    XOS_CliExtPrintf(pCliEnv, "FRAME ID:%d\r\n", uiFrameId);
    XOS_CliExtPrintf(pCliEnv, "SLOT  ID:%d\r\n", uiSlotId);
    XOS_CliExtPrintf(pCliEnv, "/************OARD INFO************/\r\n");

}


/***************************************************************************
*������:OAM_ShowMTInfo
*����  :��ʾ����ģ���Ӧ��ϵ
*����  :
*           CLI_ENV *pCliEnv,
*           XS32 siArgc,
*           XCHAR **ppArgv
*���  :
*����  :
*         XVOID
*˵��  :
***************************************************************************/
XVOID OAM_ShowMTInfo(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv)
{
    XU32 uiLoop = 0;

    XOS_CliExtPrintf(pCliEnv, "/************TABLE RRELATE MODULE ************/\r\n");
    XOS_CliExtPrintf(pCliEnv, "********Index     TableId      ModuleID*******\r\n");
    for (uiLoop = 0; uiLoop < g_TidMidCnt; uiLoop++)
    {
        XOS_CliExtPrintf(pCliEnv, "         %-4d      %-4d        %-4d    \r\n", uiLoop,
                             g_TidMidArray[uiLoop].uiTableId,
                             g_TidMidArray[uiLoop].uiModuleId);
    }
    
    XOS_CliExtPrintf(pCliEnv, "/************TABLE RRELATE MODULE ************/\r\n");

    
    XOS_CliExtPrintf(pCliEnv, "\r\n/*********ASYNC TABLE RRELATE MODULE *********/\r\n");
    XOS_CliExtPrintf(pCliEnv, "********Index     TableId      ModuleID*******\r\n");
    for (uiLoop = 0; uiLoop < g_AsyncTidMidCnt; uiLoop++)
    {
        XOS_CliExtPrintf(pCliEnv, "         %-4d      %-4d        %-4d    \r\n", uiLoop,
                             g_AsyncTidMidArray[uiLoop].uiTableId,
                             g_AsyncTidMidArray[uiLoop].uiModuleId);
    }
    
    XOS_CliExtPrintf(pCliEnv, "/*********ASYNC TABLE RRELATE MODULE *********/\r\n");

}

/***************************************************************************
*������:OAM_FmMsgTest
*����  :fm�澯�ӿڲ���
*����  :
*           CLI_ENV *pCliEnv,
*           XS32 siArgc,
*           XCHAR **ppArgv
*���  :
*����  :
*         XVOID
*˵��  :
***************************************************************************/
XVOID OAM_FmMsgTest(CLI_ENV *pCliEnv, XS32 siArgc, XS8 **ppArgv)
{
    XU32 uiAlarmFlag = 0;
    FM_PARA_T stFmPara;
    XOS_MemSet(&stFmPara, 0, sizeof(FM_PARA_T));

    /*����������Ҫ1������*/
    if(1 > siArgc)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "invalid command line parameter num\r\n");
        return;
    }

    XOS_StrToNum(ppArgv[1], &uiAlarmFlag);

    stFmPara.uiAlarmId = 1000;
    stFmPara.ulAlarmSeq = 1;
    stFmPara.ucDataNum  = 1;
    stFmPara.stFmData[0].usFieldId = 1;
    stFmPara.stFmData[0].usValLen  = 10;
    XOS_MemCpy(stFmPara.stFmData[0].ucData, "123456789", 9);
    
    OAM_AlarmSend(&stFmPara, FM_CLASS_ALARM, uiAlarmFlag);
}


#ifdef __cplusplus
}
#endif


