/****************************************************************************
*版权     : Xinwei Telecom Technology Inc
*文件名   : oam_main.cpp
*文件描述 : OAM 模块文件操作
*作者     : xiaohuiming
*创建日期 : 2014-09-01
*修改记录 :
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


XS8 g_xmlFilePath[FILE_PATH_MAX_LEN] = {0}; /*XML文件路径*/
XEXTERN XU32 g_AgtSrvIpAddr;
XEXTERN XU32 g_TsFbIpAddr;
XEXTERN XS8 g_ProcessType[VALUE_MAX_LEN];
XEXTERN NE_MDU_T g_NeProcInfo;
XEXTERN XU32 g_SlotId[HA_IP_NUM];

/***************************************************************************
*函数名: OAM_XmlCfgDataWrite
*功能  : 增量数据记录写入文件
*输入  :
*           XS8* pCfgData   记录数据(字符串流)
*           XU32 uiDataLen  数据长度
*输出  :
*           XS8* pRetData   记录数据(二进制流)
*           XU32 *pRetValLen 返回数据的长度
*返回  :
*说明  :
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
        
    /*指针参数检查*/
    if (XNULL == pCfgData || XNULL == pRetData)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "[OAM_XmlCfgDataFmt]null ptr.\r\n");
        return;
    }

    pTmpCfgData = (XU8*)pCfgData;

    /*获取记录字段数(1字节)*/
    ucTmpNum = *pTmpCfgData - '0';
    pTmpCfgData++;
    XOS_MemSet(ucTmpStr, 0, sizeof(ucTmpStr));
    XOS_MemCpy(ucTmpStr, pTmpCfgData, ucTmpNum);
    XOS_StrToNum(ucTmpStr, &uiValue);
    ucFieldNum = (XU8)uiValue;
    pTmpCfgData += ucTmpNum;

    /*按照T(类型, W(字段位宽), L(字符串值长度), V(实际字符串值)解析每个记录值)*/
    for (ucLoop = 0; ucLoop < ucFieldNum; ucLoop++)
    {
        if(pTmpCfgData >= (pCfgData + uiDataLen))
        {
            XOS_PRINT(MD(FID_OAM, PL_WARN), "Field[%d] is null\r\n", ucLoop); 
            return;
        }
        /*获取字段类型(1字节)*/
        ucType = *pTmpCfgData - '0';
        pTmpCfgData++;

        /*获取字段位宽(1字节)*/
        ucTmpNum = *pTmpCfgData - '0';
        pTmpCfgData ++;
        XOS_MemSet(ucTmpStr, 0, sizeof(ucTmpStr));
        XOS_MemCpy(ucTmpStr, pTmpCfgData, ucTmpNum);
        XOS_StrToNum(ucTmpStr, &uiValue);
        usWidth = (XU16)uiValue;
        pTmpCfgData += ucTmpNum;

        /*返回记录的实际长度*/
        *pRetValLen += usWidth;
        
        /*字符串数据的长度*/
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
*函数名:OAM_XmlRecNodeGet
*功能  :表记录数据的获取
*输入  :
*           xmlDocPtr doc    xml文件    
*输出  :
            xmlNodePtr       节点      
*返回  :
*           0        成功
*           其他值   失败
*说明  :
***************************************************************************/
xmlNodePtr OAM_XmlRootParse(xmlDocPtr doc)
{
    xmlNodePtr cur = NULL;
    
    /*指针入参检查*/
    PTR_NULL_CHECK(doc, XNULL);
    
    cur = xmlDocGetRootElement(doc);
    if(cur == XNULL)
    {
        xmlFreeDoc(doc);
        return XNULL;
    }

    /*根*/
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
        /*如果没有子树,退出*/
        xmlFreeDoc(doc);
        return XNULL;
    }

    return cur;
}


/***************************************************************************
*函数名:OAM_XmlRecNodeGet
*功能  :表记录数据的获取
*输入  :
*           xmlDocPtr doc             xml文件    
*           xmlNodePtr subCur         记录节点
*           AGT_OAM_CFG_REQ_T *pCfgData  表数据存储结构
*           XU32 uiTableId            表ID
*           XU32 uiModuleId           关注表的模块ID
*输出  :
*返回  :
*           0        成功
*           其他值   失败
*说明  :
***************************************************************************/
XS32 OAM_XmlTblNodeParse(xmlDocPtr doc, xmlNodePtr cur, AGT_OAM_CFG_REQ_T *pCfgData, 
                                                                XU32 uiTableId, XU32 uiModuleId)
{
    XU32 uiLoop = 0;
    XU32 uiRecLen = 0;
    xmlNodePtr subCur = NULL;
    xmlChar* pTempStr = XNULL;
    
    /*指针入参检查*/
    PTR_NULL_CHECK(doc, XNULL);
    PTR_NULL_CHECK(cur, XNULL);
    PTR_NULL_CHECK(pCfgData, XNULL);
    PTR_NULL_CHECK(pCfgData->pData, XNULL);

    /*获取进程类型*/
    pTempStr = xmlGetProp(cur, (xmlChar*)"type");
    if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
    {
        XOS_StrNcpy(g_ProcessType, pTempStr,XOS_MIN(XOS_StrLen(pTempStr),sizeof(g_ProcessType)-1));
        XOS_StrToLow((XU8*)g_ProcessType);
        xmlFree(pTempStr);
        pTempStr = XNULL;
    }

    /*查找子节点*/
    subCur = cur->xmlChildrenNode;
    while(subCur && xmlIsBlankNode (subCur))
    {
        subCur = subCur ->next;
    }
    
    if(subCur == XNULL)
    {
        /*如果没有子树,退出*/
        xmlFreeDoc(doc);
        return XERROR;
    }

    while(subCur != XNULL)
    {
        /*获取表ID*/
        pTempStr = xmlGetProp(subCur, (xmlChar*)"id");
        if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
        {
            XOS_StrToNum(pTempStr, &pCfgData->uiTableId); 
            xmlFree(pTempStr);
            pTempStr = XNULL;
        }

        /*表记录数*/
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
                /*发送配置消息到业务模块*/
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
*函数名:OAM_XmlRecNodeGet
*功能  :表记录数据的获取
*输入  :
*           xmlDocPtr doc             xml文件
*           xmlNodePtr subCur         记录节点
*           AGT_OAM_CFG_REQ_T *pCfgData   表数据存储结构
*           XU32 uiTableId           表ID
*输出  :
*返回  :
*           0        成功
*           其他值   失败
*说明  :
***************************************************************************/
XS32 OAM_XmlRecNodeGet(xmlDocPtr doc, xmlNodePtr subCur, AGT_OAM_CFG_REQ_T *pCfgData, 
                                                                                    XU32 uiTableId)
{
    XU32 uiIpNum = 0;
    xmlNodePtr childCur = NULL;
    xmlChar* pTempStr   = XNULL;
    XS8 *pData = XNULL;
    XU32 uiRetLen = 0;

    /*查找子节点*/
    childCur = subCur->xmlChildrenNode;
    while(childCur && xmlIsBlankNode (childCur))
    {
        childCur = childCur ->next;
    }
    
    if(childCur == XNULL)
    {
        /*如果没有子树,即只有表名没有表记录,为了格式化删除网元可能存在的数据,返回OK*/
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
            /*取索引*/
            XOS_StrToNum(pTempStr, &pCfgData->uiIndex); 
            xmlFree( pTempStr );
            pTempStr   = XNULL;

            /*取二进制流*/
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
                            XOS_StrtoIp(pTempStr,&g_AgtSrvIpAddr);/*agent IP获取*/
                        }
                        else
                        {
                            XOS_StrtoIp(pTempStr,&g_TsFbIpAddr);  /*前插板IP获取*/
                        }
                    }
                    
                    uiIpNum++;
                }
                else
                {
                    /*格式化字符串数据流成二进制数据流*/
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
*函数名:OAM_XmlFilePathGet
*功能  :xml文件名所在路径获取
*输入  :
*
*输出  :
*返回  :
*           0        成功
*           其他值   失败
*说明  :
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

    /*获取网元类型*/
    if (XNULL != XOS_GetNeTypeStr())
    {
        XOS_StrCpy(ucNeType, XOS_GetNeTypeStr());
    }
    else
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetNeTypeStr[%s] failed.\r\n", XOS_GetNeTypeStr());
        return XSUCC;
    }
    

    /*获取进程类型*/
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
*函数名:OAM_XmlFileRead
*功能  :获取MML命令
*输入  :
*            XS8 * pucFileName  指定路径文件
*            XU32 uiTableId     表ID
*            XU32 uiModuleId    模块ID
*            XU32 uiType        类型(同步和格式化)
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_XmlTblInfoGet(XS8* pFileName, XU32 uiTableId, XU32 uiModuleId, XU32 uiType)
{
    xmlDocPtr  doc      = NULL;
    xmlNodePtr cur      = NULL;
    XS8 ucXmlData[BUFF_MAX_LEN] = {0};
    AGT_OAM_CFG_REQ_T stCfgData = {0}; 
    
    /*指针入参检查*/
    PTR_NULL_CHECK(pFileName, XERROR);

    doc = xmlParseFile(pFileName);
    if(doc == XNULL)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"xmlParseFile fail!!!\r\n");
        return XERROR;
    }

    /*根节点处理*/
    cur = OAM_XmlRootParse(doc);

    stCfgData.uiOperType = uiType;    
    stCfgData.pData = ucXmlData;

    /*表名子树*/
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
*函数名: AGT_DbDataFmtToFile
*功能  : 记录写入文件
*输入  :
*           xmlNodePtr rootNode      要写入的根节点
*           FMT_DATA_T* stFmtData    节点数据
*输出  :
*返回  :
*说明  :
***************************************************************************/
XVOID OAM_XmlTblDataAdd(xmlNodePtr rootNode, AGT_OAM_CFG_REQ_T* pCfgData, XBOOL bHeadFlag)
{
    XS8 ucTmpVal[VALUE_MAX_LEN] = {0};
    xmlNodePtr childNode      = NULL;
    xmlNodePtr surChildNode   = NULL;
    AGT_OAM_CFG_REQ_T* pTempCfgData = XNULL;

    /*指针入参检查*/
    if (XNULL == pCfgData)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"pCfgData is null!\r\n");
        return;
    }
    
    pTempCfgData = pCfgData;

    if (bHeadFlag)
    {
        /*表ID*/
        childNode = xmlAddChildNode(rootNode, XML_ELEMENT_NODE, "table", NULL);
        XOS_Sprintf(ucTmpVal, sizeof(ucTmpVal), "%d", pTempCfgData->uiTableId);
        xmlSetNodeProperty(childNode, "id", ucTmpVal);
        XOS_MemSet(ucTmpVal, 0, VALUE_MAX_LEN);
        
        /*表记录数*/
        XOS_Sprintf(ucTmpVal, sizeof(ucTmpVal), "%d", pTempCfgData->uiRecNum);
        xmlSetNodeProperty(childNode, "num", ucTmpVal);
        XOS_MemSet(ucTmpVal, 0, VALUE_MAX_LEN);
    }
    
    /*记录值*/
    surChildNode = xmlAddChildNode(childNode, XML_ELEMENT_NODE, "items", NULL);
    XOS_Sprintf(ucTmpVal, sizeof(ucTmpVal), "%d", pTempCfgData->uiIndex);
    xmlSetNodeProperty(surChildNode, "index", ucTmpVal);
    xmlSetNodeContent(surChildNode, pTempCfgData->pData);

    return;
}


/***************************************************************************
*函数名: OAM_XmlNewDocCreate
*功能  : 创建新的doc文件
*输入  :
*           xmlDocPtr doc
*           xmlNodePtr rootNode
*输出  :
*返回  :
*说明  :
***************************************************************************/
XS32 OAM_XmlNewDocCreate(xmlDocPtr doc, xmlNodePtr rootNode)
{
    xmlDocPtr newDoc       = NULL;
    xmlNodePtr newRootNode = NULL;
    
    /*创建新文档*/
    newDoc = xmlCreateWriter("", NULL);
    if(NULL != newDoc) 
    {
        newRootNode = xmlGetRootNode(newDoc);
        if(NULL != newRootNode)
        {   
            /*替换要节点*/
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
*函数名: OAM_XmlTblNodeWrite
*功能  : xml文件的表节点记录操作(增删改等)
*输入  :
*           xmlNodePtr curNode       操作的节点
*           AGT_OAM_CFG_REQ_T* pCfgData  更新的数据
*           XU32 uiType              操作类型
*           XU32 uiRecNum            表记录数
*输出  :
*返回  :
*说明  :
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

    /*指针入参检查*/
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
                /*找到了对应删除的节点,调用删除接口删除*/
                xmlDelChildNode(tableNode);
                bHaveRecord = XFALSE;
            }
        }
#endif
        /*修改记录值*/
        if (bHaveRecord)
        {
            XOS_Sprintf(ucTmpVal, sizeof(ucTmpVal), "%d", uiRecNum);
            xmlSetNodeProperty(tableNode, "num", ucTmpVal);
        }
    }

    if (bHaveRecord)
    {
        /*指向items节点*/
        while(subCur != XNULL )
        {
            /*操作具体记录值*/
            itemsChildNode = subCur;
                    
            /*获取index值*/
            pTempStr = xmlGetProp(subCur, (xmlChar*)"index");
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                XOS_StrToNum(pTempStr, &uiIndex);
                xmlFree(pTempStr);
                pTempStr = XNULL;
            }
                
            /*找到了对应修改或删除的节点*/
            if (pCfgData->uiIndex == uiIndex)
            {
                if (OAM_CFG_DEL == uiType)
                {
                    /*找到了对应删除的节点,调用删除接口删除*/
                    xmlDelChildNode(itemsChildNode);
                    break;
                }
                else //if (OAM_CFG_MOD == uiType)
                {
                    /*修改记录数据内容*/
                    xmlSetNodeContent(itemsChildNode, pCfgData->pData);
                    break;
                }
            }
            else
            {
                /*指向下一条记录*/
                subCur = itemsChildNode->next;
            }
        }

        /*增加表节点记录*/
        if (OAM_CFG_ADD == uiType)
        {
            OAM_XmlTblDataAdd(tableNode, pCfgData, XFALSE);
        }
    }
}

/***************************************************************************
*函数名: OAM_XmlCfgDataWrite
*功能  : 增量数据记录写入文件
*输入  :
*           AGT_OAM_CFG_REQ_T* pCfgData   表数据
*           XS32 ucType               类型 1:add,2:del,3:mod
*输出  :
*返回  :
*           0           成功
*           其他值      失败
*说明  :
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

    /*指针入参检查*/
    PTR_NULL_CHECK(pCfgData, XERROR);
    
    pTempCfgData = pCfgData;
    
    doc = xmlParseFile(g_xmlFilePath);
    if(XNULL == doc)
    {
        xmlReleaseDoc(doc);
        return XERROR;
    }
    
    /*获取根节点*/
    cur = xmlGetRootNode(doc);
    if(cur == XNULL)
    {
        xmlReleaseDoc(doc);
        return XERROR;
    }

    /*根节点configdata*/
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
    else /*查找网元ID节点*/
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
            
            /*表ID获取*/
            pTempStr = xmlGetProp(cur, (xmlChar*)"id");
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                XOS_StrToNum(pTempStr, &uiTableId); 
                xmlFree(pTempStr);
                pTempStr = XNULL;
            }

            /*表ID记录数获取*/
            pTempStr = xmlGetProp(cur, (xmlChar*)"num");
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                XOS_StrToNum(pTempStr, &uiRecNum); 
                xmlFree(pTempStr);
                pTempStr = XNULL;
            }

            /*找到对应表ID即退出循环*/
            if (pCfgData->uiTableId == uiTableId)
            {
                break; 
            }

            cur = cur->next;
        }
    }   

    /*如果没有表节点记录，则写入子节点*/
    if(XNULL == cur)
    {
        OAM_XmlTblDataAdd(ProcNode, pTempCfgData, XTRUE);
    }
    else
    {
        /*表记录节点操作*/
        OAM_XmlTblNodeWrite(cur, pTempCfgData, uiType, uiRecNum );
    }

    if (XSUCC != OAM_XmlNewDocCreate(doc, rootNode))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_XmlNewDocCreate failed!!!\r\n");
        return XERROR;
    }

    
    return XSUCC;
}
