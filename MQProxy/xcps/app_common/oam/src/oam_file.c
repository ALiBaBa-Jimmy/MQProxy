/****************************************************************************
*��Ȩ     : Xinwei Telecom Technology Inc
*�ļ���   : oam_main.cpp
*�ļ����� : OAM ģ���ļ�����
*����     : xiaohuiming
*�������� : 2014-09-01
*�޸ļ�¼ :
****************************************************************************/
#include <stdlib.h>
#ifdef WIN32
//#include "windows.h"
#pragma warning(disable:4996)
#else
#include "sys/types.h"
#include "dirent.h"
#endif
#include "oam_file.h"


XS8 g_xmlFilePath[FILE_PATH_MAX_LEN] = {0}; /*XML�ļ�·��*/
XEXTERN XU32 g_AgtSrvIpAddr;
XEXTERN XU32 g_TsFbIpAddr;
XEXTERN XS8 g_ProcessType[VALUE_MAX_LEN];
XEXTERN NE_MDU_T g_NeProcInfo;
XEXTERN XU32 g_SlotId[HA_IP_NUM];

/***************************************************************************
*������: OAM_XmlCfgDataWrite
*����  : �������ݼ�¼д���ļ�
*����  :
*           XS8* pCfgData   ��¼����(�ַ�����)
*           XU32 uiDataLen  ���ݳ���
*���  :
*           XS8* pRetData   ��¼����(��������)
*           XU32 *pRetValLen �������ݵĳ���
*����  :
*˵��  :
***************************************************************************/
XVOID OAM_XmlCfgDataFmt(XS8* pCfgData, XU32 uiDataLen, XS8* pRetData, XU32 *pRetValLen)
{
    XU8 ucLoop     = 0;
    XU32 uiValue   = 0;
    XU64 ulValue   = 0;
    XU16 usWidth   = 0;
    XU8 ucValLen   = 0;
    XU8 ucFieldNum = 0;
    XU8 *pTmpCfgData = XNULL;
    XU8 ucType = FLD_TYPE_BUTT;
    XU8 ucTmpNum   = 0;
    XS8 ucTmpStr[FMT_VAL_MAX_LEN] = { 0 };
        
    /*ָ��������*/
    if (XNULL == pCfgData || XNULL == pRetData)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "[OAM_XmlCfgDataFmt]null ptr.\r\n");
        return;
    }

    pTmpCfgData = (XU8*)pCfgData;

    /*��ȡ��¼�ֶ���(1�ֽ�)*/
    ucTmpNum = *pTmpCfgData - '0';
    pTmpCfgData++;
    XOS_MemSet(ucTmpStr, 0, sizeof(ucTmpStr));
    XOS_MemCpy(ucTmpStr, pTmpCfgData, ucTmpNum);
    XOS_StrToNum(ucTmpStr, &uiValue);
    ucFieldNum = (XU8)uiValue;
    pTmpCfgData += ucTmpNum;

    /*����T(����, W(�ֶ�λ��), L(�ַ���ֵ����), V(ʵ���ַ���ֵ)����ÿ����¼ֵ)*/
    for (ucLoop = 0; ucLoop < ucFieldNum; ucLoop++)
    {
        if(pTmpCfgData >= (pCfgData + uiDataLen))
        {
            XOS_PRINT(MD(FID_OAM, PL_WARN), "Field[%d] is null\r\n", ucLoop); 
            return;
        }
        /*��ȡ�ֶ�����(1�ֽ�)*/
        ucType = *pTmpCfgData - '0';
        pTmpCfgData++;

        /*��ȡ�ֶ�λ��(1�ֽ�)*/
        ucTmpNum = *pTmpCfgData - '0';
        pTmpCfgData ++;
        XOS_MemSet(ucTmpStr, 0, sizeof(ucTmpStr));
        XOS_MemCpy(ucTmpStr, pTmpCfgData, ucTmpNum);
        XOS_StrToNum(ucTmpStr, &uiValue);
        usWidth = (XU16)uiValue;
        pTmpCfgData += ucTmpNum;

        /*���ؼ�¼��ʵ�ʳ���*/
        *pRetValLen += usWidth;
        
        /*�ַ������ݵĳ���*/
        ucTmpNum = *pTmpCfgData - '0';
        pTmpCfgData++;
        XOS_MemSet(ucTmpStr, 0, sizeof(ucTmpStr));
        XOS_MemCpy(ucTmpStr, pTmpCfgData, ucTmpNum);
        XOS_StrToNum(ucTmpStr, &uiValue);
        ucValLen = (XU8)uiValue;
        pTmpCfgData += ucTmpNum;
        
        switch(ucType)
        {
            case FLD_TYPE_INT:
            {
                XOS_MemSet(ucTmpStr, 0, sizeof(ucTmpStr));
                XOS_MemCpy(ucTmpStr, pTmpCfgData, ucValLen);
                XOS_StrToNum((XS8*)ucTmpStr, &uiValue);
                XOS_MemCpy(pRetData, (char*)&uiValue, sizeof(XU32));
                pRetData += sizeof(XU32);
                
            }
            break;
            
            case FLD_TYPE_SHORT:
            {
                XOS_MemSet(ucTmpStr, 0, sizeof(ucTmpStr));
                XOS_MemCpy(ucTmpStr, pTmpCfgData, ucValLen);
                XOS_StrToNum((XS8*)ucTmpStr, &uiValue);
                XOS_MemCpy(pRetData, (char*)&uiValue, sizeof(XU16));
                pRetData += sizeof(XU16);
            }
            break;
            
            case FLD_TYPE_CHAR:
            {
                XOS_MemSet(ucTmpStr, 0, sizeof(ucTmpStr));
                XOS_MemCpy(ucTmpStr, pTmpCfgData, ucValLen);
                XOS_StrToNum((XS8*)ucTmpStr, &uiValue);
                XOS_MemCpy(pRetData, (char*)&uiValue, sizeof(XU8));
                pRetData += sizeof(XU8);
            }
            break;
            
            case FLD_TYPE_LONG:
            {
                XOS_MemSet(ucTmpStr, 0, sizeof(ucTmpStr));
                XOS_MemCpy(ucTmpStr, pTmpCfgData, ucValLen);
                XOS_StrToLongNum((XS8*)pTmpCfgData, &ulValue);
                XOS_MemCpy(pRetData, (char*)&ulValue, sizeof(XU64));
                pRetData += sizeof(XU64);
            }
            break;
            
            case FLD_TYPE_STRING:
            {
                XOS_MemCpy(pRetData, pTmpCfgData, ucValLen);
                if (ucValLen < usWidth)
                {
                    *(pRetData + ucValLen) = '\0';
                }
                
                pRetData += usWidth;
            }
            break;
            
            default:
                break;
        }

        pTmpCfgData += ucValLen;
    }

    return;
}

/***************************************************************************
*������:OAM_XmlRecNodeGet
*����  :���¼���ݵĻ�ȡ
*����  :
*           xmlDocPtr doc    xml�ļ�    
*���  :
            xmlNodePtr       �ڵ�      
*����  :
*           0        �ɹ�
*           ����ֵ   ʧ��
*˵��  :
***************************************************************************/
xmlNodePtr OAM_XmlRootParse(xmlDocPtr doc)
{
    xmlNodePtr cur = NULL;
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(doc, XNULL);
    
    cur = xmlDocGetRootElement(doc);
    if(cur == XNULL)
    {
        xmlFreeDoc(doc);
        return XNULL;
    }

    /*��*/
    if(XOS_StrCmp(cur->name, "configdata"))
    {
        xmlFreeDoc(doc);
        return XNULL;
    }
    
    cur = cur->xmlChildrenNode;
    while(cur && xmlIsBlankNode (cur))
    {
        cur = cur ->next;
    }
    
    if(cur == XNULL)
    {
        /*���û������,�˳�*/
        xmlFreeDoc(doc);
        return XNULL;
    }

    return cur;
}


/***************************************************************************
*������:OAM_XmlRecNodeGet
*����  :���¼���ݵĻ�ȡ
*����  :
*           xmlDocPtr doc             xml�ļ�    
*           xmlNodePtr subCur         ��¼�ڵ�
*           AGT_OAM_CFG_REQ_T *pCfgData  �����ݴ洢�ṹ
*           XU32 uiTableId            ��ID
*           XU32 uiModuleId           ��ע���ģ��ID
*���  :
*����  :
*           0        �ɹ�
*           ����ֵ   ʧ��
*˵��  :
***************************************************************************/
XS32 OAM_XmlTblNodeParse(xmlDocPtr doc, xmlNodePtr cur, AGT_OAM_CFG_REQ_T *pCfgData, 
                                                                XU32 uiTableId, XU32 uiModuleId)
{
    XU32 uiLoop = 0;
    XU32 uiRecLen = 0;
    xmlNodePtr subCur = NULL;
    xmlChar* pTempStr = XNULL;
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(doc, XNULL);
    PTR_NULL_CHECK(cur, XNULL);
    PTR_NULL_CHECK(pCfgData, XNULL);
    PTR_NULL_CHECK(pCfgData->pData, XNULL);

    /*��ȡ��������*/
    pTempStr = xmlGetProp(cur, (xmlChar*)"type");
    if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
    {
        XOS_StrNcpy(g_ProcessType, pTempStr,XOS_MIN(XOS_StrLen(pTempStr),sizeof(g_ProcessType)-1));
        XOS_StrToLow((XU8*)g_ProcessType);
        xmlFree(pTempStr);
        pTempStr = XNULL;
    }

    /*�����ӽڵ�*/
    subCur = cur->xmlChildrenNode;
    while(subCur && xmlIsBlankNode (subCur))
    {
        subCur = subCur ->next;
    }
    
    if(subCur == XNULL)
    {
        /*���û������,�˳�*/
        xmlFreeDoc(doc);
        return XERROR;
    }

    while(subCur != XNULL)
    {
        /*��ȡ��ID*/
        pTempStr = xmlGetProp(subCur, (xmlChar*)"id");
        if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
        {
            XOS_StrToNum(pTempStr, &pCfgData->uiTableId); 
            xmlFree(pTempStr);
            pTempStr = XNULL;
        }

        /*���¼��*/
        pTempStr = xmlGetProp(subCur, (xmlChar*)"num");
        if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
        {
            XOS_StrToNum(pTempStr, &pCfgData->uiRecNum); 
            xmlFree(pTempStr);
            pTempStr = XNULL;
        }
            
        if (pCfgData->uiTableId == uiTableId)
        {
            if (XSUCC != OAM_XmlRecNodeGet(doc, subCur, pCfgData, uiTableId))
            {
                XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_XmlRecNodeGet failed.\r\n");
                return XERROR;
            }

            if (TID_MODULE_MANAGE == uiTableId)
            {
                XOS_MemSet(&g_NeProcInfo, 0, sizeof(g_NeProcInfo));
                uiRecLen = pCfgData->uiMsgLen/pCfgData->uiRecNum;
                for (uiLoop = 0; uiLoop < pCfgData->uiRecNum; uiLoop++)
                {
                    XOS_MemCpy(&g_NeProcInfo.stMduInfo[uiLoop], pCfgData->pData + (uiLoop*uiRecLen), 
                                                                                          uiRecLen);
                    g_NeProcInfo.uiProcNum++;
                }
            }

            else
            {
                /*����������Ϣ��ҵ��ģ��*/
                if (FID_OAM != uiModuleId)
                {
                    if ((pCfgData->uiOperType != OAM_CFG_FMT && pCfgData->uiRecNum > 0)
                        || (pCfgData->uiOperType == OAM_CFG_FMT))
                    {
                        if (XSUCC != OAM_CfgMsgSend(uiModuleId, AGENT_CFG_MSG, 
                                                            (XVOID*)pCfgData, pCfgData->uiMsgLen))
                        {
                            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_CfgMsgSend[%d]failed\r\n",uiModuleId);
                            return XERROR;
                        }
                    }
                }
            }

            break;
        }
        
        subCur = subCur->next;
    }
    
    return XSUCC;
}


/***************************************************************************
*������:OAM_XmlRecNodeGet
*����  :���¼���ݵĻ�ȡ
*����  :
*           xmlDocPtr doc             xml�ļ�
*           xmlNodePtr subCur         ��¼�ڵ�
*           AGT_OAM_CFG_REQ_T *pCfgData   �����ݴ洢�ṹ
*           XU32 uiTableId           ��ID
*���  :
*����  :
*           0        �ɹ�
*           ����ֵ   ʧ��
*˵��  :
***************************************************************************/
XS32 OAM_XmlRecNodeGet(xmlDocPtr doc, xmlNodePtr subCur, AGT_OAM_CFG_REQ_T *pCfgData, 
                                                                                    XU32 uiTableId)
{
    XU32 uiIpNum = 0;
    xmlNodePtr childCur = NULL;
    xmlChar* pTempStr   = XNULL;
    XS8 *pData = XNULL;
    XU32 uiRetLen = 0;

    /*�����ӽڵ�*/
    childCur = subCur->xmlChildrenNode;
    while(childCur && xmlIsBlankNode (childCur))
    {
        childCur = childCur ->next;
    }
    
    if(childCur == XNULL)
    {
        /*���û������,��ֻ�б���û�б��¼,Ϊ�˸�ʽ��ɾ����Ԫ���ܴ��ڵ�����,����OK*/
        //xmlFreeDoc(doc);
        return XSUCC;
    }

    pData = pCfgData->pData;
    
    /*items*/
    while(childCur != XNULL)
    {
        pTempStr = xmlGetProp(childCur, (xmlChar*)"index");
        if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
        {
            /*ȡ����*/
            XOS_StrToNum(pTempStr, &pCfgData->uiIndex); 
            xmlFree( pTempStr );
            pTempStr   = XNULL;

            /*ȡ��������*/
            pTempStr = xmlNodeListGetString( doc, childCur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                if (AGENT_IP_TABLE_ID == uiTableId)
                {
                    if (OAM_OFFSET_LEN <= uiIpNum)
                    {
                        XOS_StrToNum(pTempStr, &g_SlotId[uiIpNum - OAM_OFFSET_LEN]);
                        if (OAM_OFFSET_LEN + 1 == uiIpNum)
                        {
                            return XSUCC;
                        }
                    }
                    else
                    {
                        if (0 == uiIpNum)
                        {
                            XOS_StrtoIp(pTempStr,&g_AgtSrvIpAddr);/*agent IP��ȡ*/
                        }
                        else
                        {
                            XOS_StrtoIp(pTempStr,&g_TsFbIpAddr);  /*ǰ���IP��ȡ*/
                        }
                    }
                    
                    uiIpNum++;
                }
                else
                {
                    /*��ʽ���ַ����������ɶ�����������*/
                    uiRetLen = 0;
                    OAM_XmlCfgDataFmt(pTempStr, XOS_StrLen(pTempStr), pData, &uiRetLen);

                    pData += uiRetLen;
                    pCfgData->uiMsgLen += uiRetLen;
                    //pCfgData->uiRecNum++;
                    xmlFree( pTempStr );
                    pTempStr = XNULL;
                }
            }
        }

        childCur = childCur->next;
    }

    return XSUCC;
}


/***************************************************************************
*������:OAM_XmlFilePathGet
*����  :xml�ļ�������·����ȡ
*����  :
*
*���  :
*����  :
*           0        �ɹ�
*           ����ֵ   ʧ��
*˵��  :
***************************************************************************/
XS32 OAM_XmlFilePathGet()
{
    XU32 uiPid  = 0;
    XU32 uiNeId = 0;
    XU32 uiWorkSpaceId = 0;
    XS8 ucNeType[VALUE_MAX_LEN]={0};
    XS8 ucProcType[VALUE_MAX_LEN]={0};

    //#ifdef XOS_LINUX
	#if 0
    if(XERROR == (uiNeId = XOS_GetNeId()))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetNeId[%d] failed.\r\n",uiNeId);
        return XSUCC;
    }

    if(XERROR == (uiPid = XOS_GetLogicPid()))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetLogicPid[%d] failed.\r\n",uiPid);
        return XSUCC;
    }

    if(XERROR == (uiWorkSpaceId = XOS_GetWorkspaceId()))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetLogicPid[%d] failed.\r\n",uiWorkSpaceId);
        return XSUCC;
    }

    /*��ȡ��Ԫ����*/
    if (XNULL != XOS_GetNeTypeStr())
    {
        XOS_StrCpy(ucNeType, XOS_GetNeTypeStr());
    }
    else
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetNeTypeStr[%s] failed.\r\n", XOS_GetNeTypeStr());
        return XSUCC;
    }
    

    /*��ȡ��������*/
    if (XNULL != XOS_GetProcTypeStr())
    {
        XOS_StrCpy(ucProcType, XOS_GetProcTypeStr());
    }
    else
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetProcTypeStr[%s] failed.\r\n", XOS_GetProcTypeStr());
        return XSUCC;
    }
    #endif

    XOS_Sprintf(g_xmlFilePath, sizeof(g_xmlFilePath), 
        "/usr/xinwei/tcn2000/%s_%d/workspace%d/%s/procid_%d/%s.xml", 
                             ucNeType, uiNeId, uiWorkSpaceId, ucProcType, uiPid, ucProcType);

    return XSUCC;
}


/****************************************************************************
*������:OAM_XmlFileRead
*����  :��ȡMML����
*����  :
*            XS8 * pucFileName  ָ��·���ļ�
*            XU32 uiTableId     ��ID
*            XU32 uiModuleId    ģ��ID
*            XU32 uiType        ����(ͬ���͸�ʽ��)
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_XmlTblInfoGet(XS8* pFileName, XU32 uiTableId, XU32 uiModuleId, XU32 uiType)
{
    xmlDocPtr  doc      = NULL;
    xmlNodePtr cur      = NULL;
    XS8 ucXmlData[BUFF_MAX_LEN] = {0};
    AGT_OAM_CFG_REQ_T stCfgData = {0}; 
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pFileName, XERROR);

    doc = xmlParseFile(pFileName);
    if(doc == XNULL)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"xmlParseFile fail!!!\r\n");
        return XERROR;
    }

    /*���ڵ㴦��*/
    cur = OAM_XmlRootParse(doc);

    stCfgData.uiOperType = uiType;    
    stCfgData.pData = ucXmlData;

    /*��������*/
    while(cur != XNULL)
    {
        if (XSUCC != OAM_XmlTblNodeParse(doc, cur, &stCfgData, uiTableId, uiModuleId))
        {
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_XmlTblNodeParse fail!!!\r\n");
            return XERROR;
        }
            
        cur = cur->next;
    }
    
    xmlFreeDoc(doc);
            
    return XSUCC;
    
}


/***************************************************************************
*������: AGT_DbDataFmtToFile
*����  : ��¼д���ļ�
*����  :
*           xmlNodePtr rootNode      Ҫд��ĸ��ڵ�
*           FMT_DATA_T* stFmtData    �ڵ�����
*���  :
*����  :
*˵��  :
***************************************************************************/
XVOID OAM_XmlTblDataAdd(xmlNodePtr rootNode, AGT_OAM_CFG_REQ_T* pCfgData, XBOOL bHeadFlag)
{
    XS8 ucTmpVal[VALUE_MAX_LEN] = {0};
    xmlNodePtr childNode      = NULL;
    xmlNodePtr surChildNode   = NULL;
    AGT_OAM_CFG_REQ_T* pTempCfgData = XNULL;

    /*ָ����μ��*/
    if (XNULL == pCfgData)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"pCfgData is null!\r\n");
        return;
    }
    
    pTempCfgData = pCfgData;

    if (bHeadFlag)
    {
        /*��ID*/
        childNode = xmlAddChildNode(rootNode, XML_ELEMENT_NODE, "table", NULL);
        XOS_Sprintf(ucTmpVal, sizeof(ucTmpVal), "%d", pTempCfgData->uiTableId);
        xmlSetNodeProperty(childNode, "id", ucTmpVal);
        XOS_MemSet(ucTmpVal, 0, VALUE_MAX_LEN);
        
        /*���¼��*/
        XOS_Sprintf(ucTmpVal, sizeof(ucTmpVal), "%d", pTempCfgData->uiRecNum);
        xmlSetNodeProperty(childNode, "num", ucTmpVal);
        XOS_MemSet(ucTmpVal, 0, VALUE_MAX_LEN);
    }
    
    /*��¼ֵ*/
    surChildNode = xmlAddChildNode(childNode, XML_ELEMENT_NODE, "items", NULL);
    XOS_Sprintf(ucTmpVal, sizeof(ucTmpVal), "%d", pTempCfgData->uiIndex);
    xmlSetNodeProperty(surChildNode, "index", ucTmpVal);
    xmlSetNodeContent(surChildNode, pTempCfgData->pData);

    return;
}


/***************************************************************************
*������: OAM_XmlNewDocCreate
*����  : �����µ�doc�ļ�
*����  :
*           xmlDocPtr doc
*           xmlNodePtr rootNode
*���  :
*����  :
*˵��  :
***************************************************************************/
XS32 OAM_XmlNewDocCreate(xmlDocPtr doc, xmlNodePtr rootNode)
{
    xmlDocPtr newDoc       = NULL;
    xmlNodePtr newRootNode = NULL;
    
    /*�������ĵ�*/
    newDoc = xmlCreateWriter("", NULL);
    if(NULL != newDoc) 
    {
        newRootNode = xmlGetRootNode(newDoc);
        if(NULL != newRootNode)
        {   
            /*�滻Ҫ�ڵ�*/
            xmlReplaceNode(newRootNode, rootNode);
            xmlDumpToFile(newDoc, g_xmlFilePath, "UTF-8");
        }
        
        xmlReleaseDoc(newDoc);
        xmlReleaseDoc(doc);
    }
    else
    {
        xmlReleaseDoc(doc);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"xmlCreateWriter newDoc failed!!!\r\n");
        return XERROR;
    }        

    return XSUCC;
}


/***************************************************************************
*������: OAM_XmlTblNodeWrite
*����  : xml�ļ��ı�ڵ��¼����(��ɾ�ĵ�)
*����  :
*           xmlNodePtr curNode       �����Ľڵ�
*           AGT_OAM_CFG_REQ_T* pCfgData  ���µ�����
*           XU32 uiType              ��������
*           XU32 uiRecNum            ���¼��
*���  :
*����  :
*˵��  :
***************************************************************************/
XVOID OAM_XmlTblNodeWrite(xmlNodePtr curNode, AGT_OAM_CFG_REQ_T* pCfgData, XU32 uiType, 
                                                                                    XU32 uiRecNum )
{
    XU32 uiIndex                = 0;
    XBOOL bHaveRecord           = XTRUE;
    xmlChar* pTempStr           = XNULL;
    xmlNodePtr subCur           = NULL;
    xmlNodePtr tableNode        = NULL;
    xmlNodePtr itemsChildNode   = NULL;
    XS8 ucTmpVal[VALUE_MAX_LEN] = {0}; 

    /*ָ����μ��*/
    if (XNULL == pCfgData)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"pCfgData is null!\r\n");
        return;
    }
    
    tableNode = curNode;
    subCur = curNode->xmlChildrenNode;
        
    if ((OAM_CFG_ADD == uiType) || (OAM_CFG_DEL == uiType))
    {
        if (OAM_CFG_ADD == uiType)
        {
            uiRecNum = uiRecNum + 1;
        }
        else
        {
            uiRecNum = uiRecNum - 1;
        }
#if 0
        if (0 == uiRecNum)
        {
            if (OAM_CFG_DEL == uiType)
            {
                /*�ҵ��˶�Ӧɾ���Ľڵ�,����ɾ���ӿ�ɾ��*/
                xmlDelChildNode(tableNode);
                bHaveRecord = XFALSE;
            }
        }
#endif
        /*�޸ļ�¼ֵ*/
        if (bHaveRecord)
        {
            XOS_Sprintf(ucTmpVal, sizeof(ucTmpVal), "%d", uiRecNum);
            xmlSetNodeProperty(tableNode, "num", ucTmpVal);
        }
    }

    if (bHaveRecord)
    {
        /*ָ��items�ڵ�*/
        while(subCur != XNULL )
        {
            /*���������¼ֵ*/
            itemsChildNode = subCur;
                    
            /*��ȡindexֵ*/
            pTempStr = xmlGetProp(subCur, (xmlChar*)"index");
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                XOS_StrToNum(pTempStr, &uiIndex);
                xmlFree(pTempStr);
                pTempStr = XNULL;
            }
                
            /*�ҵ��˶�Ӧ�޸Ļ�ɾ���Ľڵ�*/
            if (pCfgData->uiIndex == uiIndex)
            {
                if (OAM_CFG_DEL == uiType)
                {
                    /*�ҵ��˶�Ӧɾ���Ľڵ�,����ɾ���ӿ�ɾ��*/
                    xmlDelChildNode(itemsChildNode);
                    break;
                }
                else //if (OAM_CFG_MOD == uiType)
                {
                    /*�޸ļ�¼��������*/
                    xmlSetNodeContent(itemsChildNode, pCfgData->pData);
                    break;
                }
            }
            else
            {
                /*ָ����һ����¼*/
                subCur = itemsChildNode->next;
            }
        }

        /*���ӱ�ڵ��¼*/
        if (OAM_CFG_ADD == uiType)
        {
            OAM_XmlTblDataAdd(tableNode, pCfgData, XFALSE);
        }
    }
}

/***************************************************************************
*������: OAM_XmlCfgDataWrite
*����  : �������ݼ�¼д���ļ�
*����  :
*           AGT_OAM_CFG_REQ_T* pCfgData   ������
*           XS32 ucType               ���� 1:add,2:del,3:mod
*���  :
*����  :
*           0           �ɹ�
*           ����ֵ      ʧ��
*˵��  :
***************************************************************************/
XS32 OAM_XmlCfgDataWrite(AGT_OAM_CFG_REQ_T* pCfgData, XU32 uiType)
{
    XU32 uiTableId              = 0;
    XU32 uiRecNum               = 0;
    xmlNodePtr ProcNode         = NULL;
    xmlDocPtr doc               = NULL;
    xmlNodePtr rootNode         = NULL;
    xmlNodePtr cur              = NULL;
    xmlChar* pTempStr           = XNULL;
    AGT_OAM_CFG_REQ_T *pTempCfgData = XNULL;

    /*ָ����μ��*/
    PTR_NULL_CHECK(pCfgData, XERROR);
    
    pTempCfgData = pCfgData;
    
    doc = xmlParseFile(g_xmlFilePath);
    if(XNULL == doc)
    {
        xmlReleaseDoc(doc);
        return XERROR;
    }
    
    /*��ȡ���ڵ�*/
    cur = xmlGetRootNode(doc);
    if(cur == XNULL)
    {
        xmlReleaseDoc(doc);
        return XERROR;
    }

    /*���ڵ�configdata*/
    if(XOS_StrCmp(cur->name, "configdata"))
    {
        xmlReleaseDoc(doc);
        return XERROR;
    }

    rootNode = cur;
    
    cur = cur->xmlChildrenNode;
    while(cur && xmlIsBlankNode (cur))
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        xmlReleaseDoc(doc);
        return XERROR;
    }
    else /*������ԪID�ڵ�*/
    {

        
        ProcNode = cur;

        cur = cur->xmlChildrenNode;
        while(cur && xmlIsBlankNode (cur))
        {
            cur = cur ->next;
        }
        if ( cur == XNULL )
        {
            xmlReleaseDoc(doc);
            return XERROR;
        }

        while(cur != XNULL)
        {
            uiRecNum = 0;
            
            /*��ID��ȡ*/
            pTempStr = xmlGetProp(cur, (xmlChar*)"id");
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                XOS_StrToNum(pTempStr, &uiTableId); 
                xmlFree(pTempStr);
                pTempStr = XNULL;
            }

            /*��ID��¼����ȡ*/
            pTempStr = xmlGetProp(cur, (xmlChar*)"num");
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                XOS_StrToNum(pTempStr, &uiRecNum); 
                xmlFree(pTempStr);
                pTempStr = XNULL;
            }

            /*�ҵ���Ӧ��ID���˳�ѭ��*/
            if (pCfgData->uiTableId == uiTableId)
            {
                break; 
            }

            cur = cur->next;
        }
    }   

    /*���û�б�ڵ��¼����д���ӽڵ�*/
    if(XNULL == cur)
    {
        OAM_XmlTblDataAdd(ProcNode, pTempCfgData, XTRUE);
    }
    else
    {
        /*���¼�ڵ����*/
        OAM_XmlTblNodeWrite(cur, pTempCfgData, uiType, uiRecNum );
    }

    if (XSUCC != OAM_XmlNewDocCreate(doc, rootNode))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_XmlNewDocCreate failed!!!\r\n");
        return XERROR;
    }

    
    return XSUCC;
}
