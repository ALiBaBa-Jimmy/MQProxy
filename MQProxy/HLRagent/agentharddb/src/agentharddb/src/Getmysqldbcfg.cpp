#include "Getmysqldbcfg.h"
#include "xosxml.h"
#include "xmlparser.h"
#include "fid_def.h"
#include "xwriter.h"
/************************************************************************
函数名:  MYSQL_GetDbInitsCfg
功能：   从XML文件得到数据库初始化模块的配置信息
输入：   pFile， xml文件名
输出：   pOut，输出字符串
返回：   成功 XSUCC
         失败 XERROR
说明：
************************************************************************/
XS32 ORACLE_GetDbInitsCfg(XU32 fid, SDbConfigInfo* pOut, XS8* pFile)
{
#if 0
	//XS32 siRet;
	xmlDocPtr  doc   = XNULL;
	xmlNodePtr cur      = NULL;
	xmlChar* pTempStr   = NULL;
	xmlNodePtr tmpcur = NULL;
	XU32 index = 0;

	if (XNULLP == pOut || XNULLP == pFile)
	{
		XOS_Trace(MD(fid, PL_EXP), "input point is null");
		return XERROR;
	}

	doc =  xmlParseFile( pFile );
	if (doc == XNULL) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		return XERROR;
	} 
	cur = xmlDocGetRootElement( doc );
	if (cur == XNULL) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return XERROR;
	}
	if ( XOS_StrCmp( cur->name, "MODULES") ) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return( XERROR );
	}
	cur = cur->xmlChildrenNode;
	while ( cur && xmlIsBlankNode ( cur ) )
	{
		cur = cur->next;
	}
	if ( cur == XNULL )
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return( XERROR );
	}
	tmpcur = cur;
	do
	{
		while ( cur != XNULL ) 
		{
			if ( !XOS_StrNcmp(cur->name, "DB_CFG" ,6) )
			{
				break;
			}
			cur = cur->next;
		}
		if ( XNULL == cur )
		{
			XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
			xmlFreeDoc(doc);
			return( XERROR ); 
		}
		cur = cur->xmlChildrenNode;
		while ( cur && xmlIsBlankNode ( cur ) )
		{
			cur = cur->next;
		}
		if ( cur == XNULL )
		{
			xmlFreeDoc(doc);
			return( XERROR );
		}

		while(XNULLP != cur)
		{
			if ( !XOS_StrCmp(cur->name, "USER") )
			{
				pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
				if ( XNULL != pTempStr )
				{
					XOS_StrCpy(pOut->user, pTempStr);
					xmlFree( pTempStr );
					pTempStr = XNULL;
				}
			}

			else if ( !XOS_StrCmp(cur->name, "PASSWORD") )
			{
				pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
				if ( XNULL != pTempStr )
				{
					XOS_StrCpy(pOut->pwd, pTempStr);
					xmlFree( pTempStr );
					pTempStr = XNULL;
				}
			}

			else if ( !XOS_StrCmp(cur->name, "CONNSTRING") )
			{
				pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
				if ( XNULL != pTempStr )
				{
					XOS_StrCpy(pOut->connstring, pTempStr);
					xmlFree( pTempStr );
					pTempStr = XNULL;
				}
			}
			else if(!XOS_StrCmp(cur->name, "USER_alias") )
			{
				pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
				if ( XNULL != pTempStr )
				{
					XOS_StrCpy(pOut->user_ali, pTempStr);
					xmlFree( pTempStr );
					pTempStr = XNULL;
				}
			}
			else if(!XOS_StrCmp(cur->name, "PASSWORD_alias") )
			{
				pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
				if ( XNULL != pTempStr )
				{
					XOS_StrCpy(pOut->pwd_ali, pTempStr);
					xmlFree( pTempStr );
					pTempStr = XNULL;
				}
			}
			else if(!XOS_StrCmp(cur->name, "CONNSTRING_alias") )
			{
				pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
				if ( XNULL != pTempStr )
				{
					XOS_StrCpy(pOut->connstring_ali, pTempStr);
					xmlFree( pTempStr );
					pTempStr = XNULL;
				}
			}
			cur = cur->next;
		}
		cur=tmpcur->next;
		tmpcur = cur;
		index++;
	}while(cur!=NULL);


	xmlFreeDoc(doc);
#endif
	return XSUCC;
}


/************************************************************************
函数名:  MYSQL_GetDbInitsCfg
功能：   从XML文件得到数据库初始化模块的配置信息
输入：   pFile， xml文件名
输出：   pOut，输出字符串
返回：   成功 XSUCC
         失败 XERROR
说明：
************************************************************************/
XS32 MYSQL_GetDbInitsCfg(XU32 fid, SDbConfigInfo* pOut, XS8* pFile)
{
	//XS32 siRet;
	xmlDocPtr  doc   = XNULL;
	xmlNodePtr cur      = NULL;
	xmlChar* pTempStr   = NULL;

	if (XNULLP == pOut || XNULLP == pFile)
	{
		XOS_Trace(MD(fid, PL_EXP), "input point is null");
		return XERROR;
	}

	doc =  xmlParseFile( pFile );
	if (doc == XNULL) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		return XERROR;
	} 
	cur = xmlDocGetRootElement( doc );
	if (cur == XNULL) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return XERROR;
	}
	if ( XOS_StrCmp( cur->name, "MODULES") ) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return( XERROR );
	}
	cur = cur->xmlChildrenNode;
	while ( cur && xmlIsBlankNode ( cur ) )
	{
		cur = cur->next;
	}
	if ( cur == XNULL )
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return( XERROR );
	}

	while ( cur != XNULL ) 
	{
		if ( !XOS_StrCmp(cur->name, "DB_CFG" ) )
		{
			break;
		}
		cur = cur->next;
	}
	if ( XNULL == cur )
	{
		XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
		xmlFreeDoc(doc);
		return( XERROR ); 
	}
	cur = cur->xmlChildrenNode;
	while ( cur && xmlIsBlankNode ( cur ) )
	{
		cur = cur->next;
	}
	if ( cur == XNULL )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}

	while(XNULLP != cur)
	{
		if ( !XOS_StrCmp(cur->name, "DATABASE") )
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->dbName, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}

		else if ( !XOS_StrCmp(cur->name, "USER_NAME") )
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->user, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}

		else if ( !XOS_StrCmp(cur->name, "PASSWORD") )
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->pwd, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}
		else if (!XOS_StrCmp(cur->name, "SERVERNAME") )
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->ip, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}
		else
		{
		}
		cur = cur->next;
	}
	xmlFreeDoc(doc);

	return XSUCC;
}

/************************************************************************
函数名:  MYSQL_RecordDbCfgToXML
功能：   将数据库的配置信息从内存中写入xml文件中
输入：   pDbCfgInfo:数据库配置信息
		 pFile：xml文件
返回：   成功 XSUCC
         失败 XERROR
说明：
************************************************************************/
XS32 MYSQL_RecordDbCfgToXML(XU32 fid, SDbConfigInfo* pDbCfgInfo, XS8* pFile)
{
	//xmlDocPtr  doc   = XNULL;
	//xmlNodePtr cur      = NULL;
	//xmlChar* pTempStr   = NULL;

	//xmlDocPtr newDoc             = NULL;
 //   xmlNodePtr newRootNode       = NULL;
	//xmlNodePtr rootNode          = NULL;

	//if (XNULLP == pDbCfgInfo || XNULLP == pFile)
	//{
	//	XOS_Trace(MD(fid, PL_EXP), "input point is null");
	//	return XERROR;
	//}

	//doc =  xmlParseFile( pFile );
	//if (doc == XNULL) 
	//{
	//	XOS_Trace(MD(fid, PL_EXP), "parse dbcfg file failed");
	//	return XERROR;
	//} 
	////获得根结点
	//cur = xmlDocGetRootElement( doc );
	//if (cur == XNULL) 
	//{
	//	XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
	//	xmlFreeDoc(doc);
	//	return XERROR;
	//}
	////判断根节点是否正确
	//if ( XOS_StrCmp( cur->name, "MODULES") ) 
	//{
	//	XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
	//	xmlFreeDoc(doc);
	//	return( XERROR );
	//}

	//rootNode = cur;

	////获得下一个节点,若为空节点，则移到下一个节点。否则退出循环
	//while ( cur && xmlIsBlankNode ( cur ) )
	//{
	//	cur = cur->next;
	//}
	//if ( cur == XNULL )
	//{
	//	XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
	//	xmlFreeDoc(doc);
	//	return( XERROR );
	//}

	//cur = cur->xmlChildrenNode;
	//while ( cur && xmlIsBlankNode ( cur ) )
	//{
	//	cur = cur->next;
	//}
	//if ( cur == XNULL )
	//{
	//	xmlFreeDoc(doc);
	//	return( XERROR );
	//}

	//while ( cur != XNULL ) 
	//{
	//	if ( XOS_StrCmp(cur->name, "DB_CFG")== 0 )
	//	{
	//		break;
	//	}
	//	cur = cur->next;
	//}
	//if ( XNULL == cur )
	//{
	//	XOS_Trace(MD(fid, PL_EXP), "parse dbscfg file failed");
	//	xmlFreeDoc(doc);
	//	return( XERROR ); 
	//}

	//cur = cur->xmlChildrenNode;
	//while ( cur && xmlIsBlankNode ( cur ) )
	//{
	//	cur = cur->next;
	//}
	//if ( cur == XNULL )
	//{
	//	xmlFreeDoc(doc);
	//	return( XERROR );
	//}

	//while(XNULLP != cur)
	//{
	//	if ( !XOS_StrCmp(cur->name, "DATABASE") )
	//	{
	//		xmlNodeSetContent(cur,(const xmlChar*)pDbCfgInfo->dbName);	
	//	}
	//	else if ( !XOS_StrCmp(cur->name, "USER_NAME") )
	//	{
	//		xmlNodeSetContent(cur,(const xmlChar*)pDbCfgInfo->user);
	//
	//	}
	//	else if ( !XOS_StrCmp(cur->name, "PASSWORD") )
	//	{
	//		xmlNodeSetContent(cur,(const xmlChar*)pDbCfgInfo->pwd);
	//		
	//	}
	//	else if (!XOS_StrCmp(cur->name, "SERVERNAME") )
	//	{
	//		xmlNodeSetContent(cur,(const xmlChar*)pDbCfgInfo->ip);
	//	}
	//	else
	//	{
	//	}
	//	cur = cur->next;
	//}

	///*创建新文档*/
 //   newDoc = xmlCreateWriter("", NULL);
 //   if(NULL != newDoc) 
 //   {
 //       newRootNode = xmlGetRootNode(newDoc);
 //       if(NULL != newRootNode)
 //       {   /*替换要节点*/
 //           xmlReplaceNode(newRootNode, rootNode);
 //           xmlDumpToFile(newDoc, pFile, NULL);
 //       }
 //       xmlReleaseDoc(newDoc);
 //   }
 //   else
 //   {
 //       xmlFreeDoc(doc);
 //       xmlCleanupParser();
 //       return XERROR;
 //   }    

 //   xmlFreeDoc(doc);
 //   xmlCleanupParser();

	return XSUCC;	
}
