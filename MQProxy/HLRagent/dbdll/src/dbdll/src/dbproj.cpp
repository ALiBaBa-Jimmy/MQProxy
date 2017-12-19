// dbproj.cpp : 定义 DLL 应用程序的入口点。
//


#include "os.h"
#include "dbproj.h"
#include "publictype.h"
#include "xostype.h"
#include <stdio.h>
/*XBOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			//XS32 procRes = 0;
			//XS8 username[32] = {0};
			//XS8 passwd[32] = {0};
			//XS8 connstring[32] = {0};
			//读取oss.xml文件的信息
			//开始读取oracle连接字符串 
			
			
			
			
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:



	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}*/
/*
// 这是导出变量的一个示例
DBPROJ_API XS32 ndbproj=0;

// 这是导出函数的一个示例。
DBPROJ_API XS32 fndbproj(XVOID)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 dbproj.h
Cdbproj::Cdbproj()
{ 
	return; 
}*/
/************************************************************************
函数名  : ReadCfgFromXML()
功能    : 获取 SAG 配置信息
输入    : filename   XOS 配置文件名
输出    : szAppName 应用名 例如 SAG
          szModuleName 模块名 例如: CC
		  szKey  模块键值名称;
		  pszResult 结果返回字符串指针
		  nResultLen  结果返回字符串缓冲区长度

举例:
		_CHAR result[1024] = {0};
		if ( SUCC == ReadCfgFromXML( "sag.xml", "SAG", "CC", "Key1", 1024, result ) )
		{
		}
		else
		{
		}

返回    : XBOOL
说明：

************************************************************************/
/*XS32 ReadCfgFromXML( const XS8* szFileName,
 					const  XS8* szAppName,
					const  XS8* szModuleName,
					const  XS8* szKey,
					XU32  nResultLen,
					 XS8* pszResult )
{
	xmlDocPtr  doc	    = NULL;
    xmlNodePtr cur      = NULL;
	xmlChar* pTempStr   = NULL;

	if ( NULL == szFileName
		|| NULL == szAppName
		|| NULL == szModuleName
		|| NULL == szKey
		|| NULL == pszResult
		|| 0    >= nResultLen )
	{
		return XERROR;
	}

	memset( pszResult, 0x00, nResultLen );

	doc = xmlParseFile( szFileName );
    if (doc == NULL) 
	{
		return( XERROR );
	} 
    cur = xmlDocGetRootElement(doc);
    if (cur == NULL) 
	{
		xmlFreeDoc(doc);
		return( XERROR );
    }
	if ( strcmp( cur->name, "MODULES") ) 
	{
		xmlFreeDoc(doc);
		return( XERROR );
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
		cur = cur -> next;
    }
    if ( cur == NULL )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}

    while ( cur != NULL ) 
	{
        if ( !strcmp(cur->name, szAppName ) )
		{
			break;
		}

		cur = cur->next;
    }
	if ( NULL == cur )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}
	cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
		cur = cur -> next;
    }
    if ( cur == NULL )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}

	while ( cur != NULL ) 
	{
        if ( !strcmp(cur->name, szModuleName ) )
		{
			break;
		}

		cur = cur->next;
    }
	if ( NULL == cur )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}
	cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
		cur = cur -> next;
    }
    if ( cur == NULL )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}

	while ( cur != NULL ) 
	{
        if ( !strcmp(cur->name, szKey ) )
		{
			break;
		}

		cur = cur->next;
    }
	if ( NULL == cur )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}

	pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
	if ( NULL == pTempStr )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}

	strncpy( pszResult, pTempStr, (nResultLen - 1) );

	xmlFreeDoc(doc);
	return XSUCC;
}
*/
