/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  parser.c
**
**  description: an XML 1.0 non-verifying parser
**
**  author: zhanglei
**
**  date:   2006.3.7
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   zhanglei         2006.4.7              create
**************************************************************/

/**/
#include "xosxml.h"
#include "xmlparser.h"
#include "xosencap.h"
#include "clishell.h"

/************************************************************************
������  : XML_ReadAttGenCfg
����    : get XOS ATT Server configure informations
����    : filename   XOS �����ļ���
���    : �ú������ڰ��ͨѸ������,��ͨ������
����    : XBOOL
˵����
************************************************************************/
XBOOL XML_ReadAttGenCfg( t_ATTGENCFG* pAttGenCfg , XCHAR* filename )
{
    /* none XOS Vxworks */
    //#ifndef XOS_VXWORKS
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar* pTempStr   = XNULL;
    //#endif
    
    if ( XNULL == pAttGenCfg )
    {
        return( XFALSE );
    }
    
    XOS_MemSet( pAttGenCfg, 0x00, sizeof(t_ATTGENCFG) );
    pAttGenCfg->ip       = 0;
    pAttGenCfg->maxAtts  = 16;
    pAttGenCfg->port     = 5000 + XOS_GetLocalPID();
    
    /* none XOS Vxworks include WIN2K & TURBO LINUX */
    //#ifndef XOS_VXWORKS
    doc = xmlParseFile(filename);
    if (doc == XNULL)
    {
        return( XFALSE );
    }
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "ATT_SERVER" ) )
        {
            break;
        }
        
        cur = cur->next;
    }
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "IP") )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr )
            {
                if( XSUCC != XOS_StrtoIp( (XCHAR*)pTempStr, &pAttGenCfg->ip ) )
                {
                    xmlFree( pTempStr );
                    goto OUT_LABLE;
                }
                xmlFree( pTempStr );
                pTempStr = XNULL;
            }
            
            pTempStr = xmlGetProp( cur, (xmlChar*)"port" );
            if ( XNULL != pTempStr )
            {
                pAttGenCfg->port = (XU16)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr = XNULL;
            }
        }
        
        if ( !XOS_StrCmp(cur->name, "MAX_CON_NUM") )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr )
            {
                pAttGenCfg->maxAtts = (XU16)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr = XNULL;
            }
        }
        
        cur = cur->next;
    }
    
OUT_LABLE:
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    //#endif /*  none XOS Vxworks end WIN2K & TURBO LINUX  */
    
    /*  XOS Vxworks begin  */
    //#ifdef XOS_VXWORKS
    //    pAttGenCfg->ip       = 0;
    //    pAttGenCfg->maxAtts  = 16;
    //    pAttGenCfg->port     = 5000 + XOS_GetLocalPID();
    //#endif /* XOS Vxworks end */
    return XTRUE;
}

XS16 XML_readMemCfg1( t_MEMCFG * pMemCfg, XCONST XS8 *filename );
/************************************************************************
������  : XML_readMemCfg
����    : get XOS Memory configure informations
����    : filename   XOS �����ļ���
���    :
����    :
˵��    ��
************************************************************************/
XS16 XML_readMemCfg( t_MEMCFG * pMemCfg, XCONST XS8 *filename )
{
    //#if defined(XOS_WIN32) || defined(XOS_LINUX) || defined (XOS_SOLARIS) || defined(XOS_VTA)
#if !defined(XOS_VXWORKS) || defined(XOS_VTA)
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    XU32       i        = 0;
#endif
    
    /*t_MEMCFG tt;*/
    
    if ( XNULL == pMemCfg )
    {
        return XFALSE;
    }
    XOS_MemSet( pMemCfg, 0x00, sizeof(t_MEMCFG) );
    
    pMemCfg->pMemBlock =  (t_MEMBLOCK*)xmlMalloc( MAX_MEMBLOCK_NUM*sizeof( t_MEMBLOCK ) );
    if ( XNULL == pMemCfg->pMemBlock )
    {
        return( XFALSE );
    }
    XOS_MemSet( pMemCfg->pMemBlock, 0x00, MAX_MEMBLOCK_NUM*sizeof( t_MEMBLOCK ) );
    
    /*XML_readMemCfg1( &tt,  "ftp://zl:z123456@168.0.200.100/export/home/zl/xos.xml" );*/
    
    /*  WIN2K & TURBO LINUX  */
#if !defined(XOS_VXWORKS) || defined(XOS_VTA)
    
    /* ftp://zl:z123456@168.0.200.100:21/export/home/zl/xos.xml */
    doc = xmlParseFile( filename );
    if (doc == XNULL)
    {
        XML_readMemCfgRelease( (t_MEMCFG*)pMemCfg );
        return( XFALSE );
    }
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        goto ERR_OUT_LABLE;
    }
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        goto ERR_OUT_LABLE;
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        goto ERR_OUT_LABLE;
    }
    
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "MEMORY_POOL" ) )
        {
            break;
        }
        
        cur = cur->next;
    }
    if ( XNULL == cur )
    {
        goto ERR_OUT_LABLE;
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        goto ERR_OUT_LABLE;
    }
    
    i = 0;
    while ( cur != XNULL && i < MAX_MEMBLOCK_NUM )
    {
        if ( !XOS_StrCmp(cur->name, "MEMORY_SIZE" ) )
        {
            pTempStr = xmlGetProp(cur, (xmlChar*)"num");
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pMemCfg->pMemBlock[i].blockNums  = (XU32)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr   = XNULL;
                
                pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
                if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
                {
                    pMemCfg->pMemBlock[i].blockSize = (XU32)atol( (char*)pTempStr );
                    xmlFree( pTempStr );
                    pTempStr   = XNULL;
                    
                    i++;
                }
            }
        }
        
        cur = cur->next;
    }
    pMemCfg->memTypes = i;
    
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XSUCC;
    
ERR_OUT_LABLE:
    xmlFreeDoc(doc);
    XML_readMemCfgRelease( (t_MEMCFG*)pMemCfg );
    xmlCleanupParser();
    return( XERROR);
    
#endif  /*  WIN2K & TURBO LINUX  end*/
    
    /*  XOS Vxworks && NVTA begin  */
#if defined(XOS_VXWORKS) && !defined (XOS_VTA)
    
    pMemCfg->pMemBlock[ 0 ].blockNums = 3000;
    pMemCfg->pMemBlock[ 0 ].blockSize = 32;
    
    pMemCfg->pMemBlock[ 1 ].blockNums = 5000;
    pMemCfg->pMemBlock[ 1 ].blockSize = 64;
    
    pMemCfg->pMemBlock[ 2 ].blockNums = 3000;
    pMemCfg->pMemBlock[ 2 ].blockSize = 128;
    
    pMemCfg->pMemBlock[ 3 ].blockNums = 1000;
    pMemCfg->pMemBlock[ 3 ].blockSize = 256;
    
    pMemCfg->pMemBlock[ 4 ].blockNums = 500;
    pMemCfg->pMemBlock[ 4 ].blockSize = 512;
    
    pMemCfg->pMemBlock[ 5 ].blockNums = 500;
    pMemCfg->pMemBlock[ 5 ].blockSize = 1024;
    
    pMemCfg->pMemBlock[ 6 ].blockNums = 500;
    pMemCfg->pMemBlock[ 6 ].blockSize = 2048;
    
    pMemCfg->pMemBlock[ 7 ].blockNums = 100;
    pMemCfg->pMemBlock[ 7 ].blockSize = 4096;
    
    pMemCfg->pMemBlock[ 8 ].blockNums = 100;
    pMemCfg->pMemBlock[ 8 ].blockSize = 8192;
    
    pMemCfg->pMemBlock[ 9 ].blockNums = 30;
    pMemCfg->pMemBlock[ 9 ].blockSize = 16384;
    
    pMemCfg->pMemBlock[ 10 ].blockNums = 8;
    pMemCfg->pMemBlock[ 10 ].blockSize = 32768;
    
    pMemCfg->pMemBlock[ 11 ].blockNums = 10;
    pMemCfg->pMemBlock[ 11 ].blockSize = 65536;
    
    pMemCfg->pMemBlock[ 12 ].blockNums = 10;
    pMemCfg->pMemBlock[ 12 ].blockSize = 131072;
    
    pMemCfg->pMemBlock[ 13 ].blockNums = 5;
    pMemCfg->pMemBlock[ 13 ].blockSize = 262144;
    
    pMemCfg->pMemBlock[ 14 ].blockNums = 2;
    pMemCfg->pMemBlock[ 14 ].blockSize = 524288;
    
    pMemCfg->pMemBlock[ 15 ].blockNums = 3;
    pMemCfg->pMemBlock[ 15 ].blockSize = 1048576;
    
    pMemCfg->pMemBlock[ 16 ].blockNums = 1;
    pMemCfg->pMemBlock[ 16 ].blockSize = 2097152;
    
    pMemCfg->memTypes = 17;
    
#ifdef BSI_BRD
    
    pMemCfg->pMemBlock[ 10 ].blockNums = 15;
    pMemCfg->pMemBlock[ 10 ].blockSize = 32768;
    
    pMemCfg->pMemBlock[ 11 ].blockNums = 10;
    pMemCfg->pMemBlock[ 11 ].blockSize = 65536;
    
    pMemCfg->pMemBlock[ 12 ].blockNums = 10;
    pMemCfg->pMemBlock[ 12 ].blockSize = 131072;
    
    pMemCfg->pMemBlock[ 13 ].blockNums = 2;
    pMemCfg->pMemBlock[ 13 ].blockSize = 262144;
    
    pMemCfg->pMemBlock[ 14 ].blockNums = 1;
    pMemCfg->pMemBlock[ 14 ].blockSize = 524288;
    
    pMemCfg->pMemBlock[ 15 ].blockNums = 2;
    pMemCfg->pMemBlock[ 15 ].blockSize = 1048576;
    
    pMemCfg->pMemBlock[ 16 ].blockNums = 1;
    pMemCfg->pMemBlock[ 16 ].blockSize = 2097152;
    
    pMemCfg->memTypes = 17;
#endif
    
    return XSUCC;
    
#endif /* XOS_VXWORKS*/
}

/************************************************************************
������  : XML_readMemExCfg
����    : ��ȡ�ڴ�������Ϣ�����ڴ��쳣�����������Ϣ
����    : filename   XOS �����ļ���
���    :
����    : XBOOL
˵����
************************************************************************/
XS16 XML_readMemExCfg( t_MEMEXCFG * pMemExCfg, XCONST XS8 *filename )
{
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    
    XOS_MemSet( pMemExCfg, 0x00, sizeof(t_MEMEXCFG) );
    
    doc = xmlParseFile( filename );
    if (doc == XNULL) {
        return XERROR;
    }
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL) {
        goto ERR;
    }
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        goto ERR;
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        goto ERR;
    }
    
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "MEM_CFG" ) )
        {
            break;
        }
        
        cur = cur->next;
    }
    if ( XNULL == cur )
    {
        goto ERR;
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        goto ERR;
    }
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "MEM_BAD" ) )
        {
            break;
        }
        
        cur = cur->next;
    }
    if ( XNULL == cur )
    {
        goto ERR;
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        goto ERR;
    }
    while ( cur != XNULL)
    {
        if ( !XOS_StrCmp(cur->name, "ENABLE_BOOT" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pMemExCfg->_enableBoot = (XS8)atoi( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        if ( !XOS_StrCmp(cur->name, "LOG_STACK_NUM" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pMemExCfg->_logStackNum = atoi( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        cur = cur->next;
    }
ERR:    
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XSUCC;
}

/************************************************************************
������  : XML_readMemCfgRelease
����    : �ͷ��ڴ�������Ϣ
����    : filename   XOS �����ļ���
���    :
����    : XBOOL
˵����
************************************************************************/
XS16 XML_readMemCfgRelease( t_MEMCFG * pMemCfg )
{
    if ( XNULL == pMemCfg )
    {
        return XTRUE;
    }
    
    xmlFree( pMemCfg->pMemBlock );
    
    return XTRUE;
}

/************************************************************************
������  : XML_GetTelDCfgFromXosXml
����    : get XOS Telnet Server configure informations
����    : filename   XOS �����ļ���
���    :
����    : XBOOL
˵����
************************************************************************/
XBOOL XML_GetTelDCfgFromXosXml( t_TelnetDCfg* pTelDCfg, XCHAR* szFileName )
{
    /* XOS VTA and  WIN2K & TURBO LINUX */
#if !defined(XOS_VXWORKS) || defined(XOS_VTA)
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar* pTempStr   = XNULL;
#endif
    
    if ( XNULL == szFileName || XNULL == pTelDCfg )
    {
        return XFALSE;
    }
    XOS_MemSet( pTelDCfg, 0x00, sizeof(t_TelnetDCfg) );
    
    /*  XOS VTA and  WIN2K & TURBO LINUX */
#if !defined(XOS_VXWORKS) || defined(XOS_VTA)
    doc = xmlParseFile( szFileName );
    if (doc == XNULL)
    {
        return( XFALSE );
    }
    cur = xmlDocGetRootElement( doc );
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur -> next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "TELNET_SERVER" ) )
        {
            break;
        }
        
        cur = cur->next;
    }
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur -> next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "IP") )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr )
            {
                if( XSUCC != XOS_StrtoIp( (XCHAR*)pTempStr, &pTelDCfg->ip ) )
                {
                    xmlFree( pTempStr );
                    goto OUT_LABLE;
                }
                xmlFree( pTempStr );
                pTempStr = XNULL;
            }
            
            pTempStr = xmlGetProp( cur, (xmlChar*)"port" );
            if ( XNULL != pTempStr )
            {
                pTelDCfg->port = (XU16)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr = XNULL;
            }
        }
        
        if ( !XOS_StrCmp(cur->name, "MAX_CON_NUM") )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr )
            {
                pTelDCfg->maxTelClients = (XU16)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr = XNULL;
            }
        }
        
        cur = cur->next;
    }
    
    return XTRUE;
    
OUT_LABLE:
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XFALSE;
    
#endif /*  XOS VTA and  WIN2K & TURBO LINUX  */
    
    /*  XOS Vxworks none XOS_VTA  */
#if defined(XOS_VXWORKS) && !defined (XOS_VTA)
    
    pTelDCfg->ip             = 0;
    pTelDCfg->maxTelClients  = 10;
    pTelDCfg->port           = 1024;
    
#endif /* XOS Vxworks end */
    
    return XTRUE;
}

XS16 XML_readMemCfg1( t_MEMCFG * pMemCfg, XCONST XS8 *filename )
{
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    XU32       i        = 0;
    
    if ( XNULL == pMemCfg )
    {
        return XFALSE;
    }
    XOS_MemSet( pMemCfg, 0x00, sizeof(t_MEMCFG) );
    
    pMemCfg->pMemBlock =  (t_MEMBLOCK*)xmlMalloc( MAX_MEMBLOCK_NUM*sizeof( t_MEMBLOCK ) );
    if ( XNULL == pMemCfg->pMemBlock )
    {
        return( XFALSE );
    }
    XOS_MemSet( pMemCfg->pMemBlock, 0x00, MAX_MEMBLOCK_NUM*sizeof( t_MEMBLOCK ) );
    
    /* ftp://zl:z123456@168.0.200.100:21/export/home/zl/xos.xml */
    doc = xmlParseFile( filename );
    if (doc == XNULL)
    {
        XML_readMemCfgRelease( (t_MEMCFG*)pMemCfg );
        return( XFALSE );
    }
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        goto ERR_OUT_LABLE;
    }
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        goto ERR_OUT_LABLE;
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur -> next;
    }
    if ( cur == XNULL )
    {
        goto ERR_OUT_LABLE;
    }
    
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "MEMORY_POOL" ) )
        {
            break;
        }
        
        cur = cur->next;
    }
    if ( XNULL == cur )
    {
        goto ERR_OUT_LABLE;
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        goto ERR_OUT_LABLE;
    }
    
    i = 0;
    while ( cur != XNULL && i < MAX_MEMBLOCK_NUM )
    {
        if ( !XOS_StrCmp(cur->name, "MEMORY_SIZE" ) )
        {
            pTempStr = xmlGetProp(cur, (xmlChar*)"num");
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pMemCfg->pMemBlock[i].blockNums  = (XU32)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr   = XNULL;
                
                pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
                if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
                {
                    pMemCfg->pMemBlock[i].blockSize = (XU32)atol( (char*)pTempStr );
                    xmlFree( pTempStr );
                    pTempStr   = XNULL;
                    
                    i++;
                }
            }
        }
        
        cur = cur->next;
    }
    pMemCfg->memTypes = i;
    
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XSUCC;
    
ERR_OUT_LABLE:
    xmlFreeDoc(doc);
    XML_readMemCfgRelease( (t_MEMCFG*)pMemCfg );
    xmlCleanupParser();
    return( XERROR);
    
}

/************************************************************************
������  : XOS_ReadLogCfg
����    : get XOS LOG configure informations
����    : filename   XOS �����ļ���
���    :
����    : XBOOL
˵����    XFALSE ��ȡ�����ļ�ʧ�� XTRUEΪ�ɹ�
************************************************************************/
XBOOL XML_ReadLogCfg(t_LogCfg *logcfgs, XCHAR * filename)
{
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    
    /*�������*/
    if(!logcfgs)
    {
        return XFALSE;
    }
    if(!filename)
    {
        return XFALSE;
    }

    XOS_MemSet( logcfgs, 0x00, sizeof(t_LogCfg));
#if (defined(XOS_VXWORKS))
    logcfgs->_nEnableRemote = 1;                  /*Ĭ�ϴ�Զ����־����*/
    logcfgs->_nRemoteAddr   = 0x7F000001;         /*Ĭ��Զ�̷����ַ*/
#else
    logcfgs->_nEnableRemote = 0;                  /*Ĭ�Ϲر�Զ����־����*/
    logcfgs->_nRemoteAddr   = 0x7F000001;         /*Ĭ��Զ�̷����ַ*/
#endif
    logcfgs->_nRemotePort   = 19999;              /*Ĭ��Զ�̷���˿�*/
    logcfgs->_nLocalPort    = 19998;               /*Ĭ�ϱ��ض˿�*/
    logcfgs->_nMaxFlow      = 0;                  /*Ĭ����������*/
    logcfgs->_nLogSize      = 1;                  /*Ĭ���ļ���С1M*/

    doc = xmlParseFile(filename);
    if (doc == XNULL)
    {
        return (XFALSE);
    }
    
    /*�Ҹ��ڵ�*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    /*���ڵ�*/
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    /*LOGCFG���ڵ�*/
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "LOGCFG" ) )
        {
            break;
        }
        cur = cur->next;
    }
    
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XTRUE ;
    }
    
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XTRUE ;
    }
    /*����LOGCFG�ӽڵ�*/
    while ( cur != XNULL )
    {
        /*�Ƿ�����Զ����־����������*/
        if ( !XOS_StrCmp(cur->name, "ENABLEREMOTE" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                logcfgs->_nEnableRemote = (XU8)atoi(pTempStr);
                if(logcfgs->_nEnableRemote != 0)
                {
                    logcfgs->_nEnableRemote = 1;
                }
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }            
        }
        /*LOG������IP��ַ*/
        if ( !XOS_StrCmp(cur->name, "LOG_SERVER" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                XOS_StrtoIp((XCHAR*)pTempStr, &logcfgs->_nRemoteAddr);
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }            
        }
        /*LOG�������˿�*/
        if ( !XOS_StrCmp(cur->name, "LOG_TCP_PORT" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                logcfgs->_nRemotePort = (XU16) atoi((char*)pTempStr);
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }            
        }
        /*���ض˿ں�*/
        if ( !XOS_StrCmp(cur->name, "LOCAL_TCP_PORT") )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                logcfgs->_nLocalPort = (XU16)atoi((char*)pTempStr);
                xmlFree( pTempStr );
                pTempStr = XNULL;
            }            
        }
        /*��������*/
        if ( !XOS_StrCmp(cur->name, "LOG_FLOW" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                logcfgs->_nMaxFlow = atoi((char*)pTempStr);
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }            
        }
#if 0
        /*������־�ļ���С*/
        if ( !XOS_StrCmp(cur->name, "LOCAL_LOG_SIZE" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                logcfgs->_nLogSize = atoi((char*)pTempStr);
                xmlFree( pTempStr );
                pTempStr   = XNULL;
                if(logcfgs->_nLogSize)
            }
        }
#endif
        cur = cur->next;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XTRUE;
    
}

/************************************************************************
������  : XOS_GetLogServFromXml
����    : ��ȡlog�ϴ�server��������Ϣ
����    : filename   XOS �����ļ���
���    :
����    : XBOOL
˵����
************************************************************************/

XBOOL XOS_GetLogServFromXml(XU32 *IP,XU32 *Port,XCHAR * filename)
{
    /*�������*/
#if defined(XOS_VXWORKS) && !defined (XOS_VTA)
    *IP = 0XA900C602;
    *Port = 90000;
    
#endif  /*_XOS_VXWORKS_*/
    
#if !defined(XOS_VXWORKS) || defined(XOS_VTA)
    
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    
    if ( XNULL == IP || XNULL == Port )
    {
        return XFALSE;
    }
    
    doc = xmlParseFile( filename);
    if (doc == XNULL)
    {
        return( XFALSE );
    }
    
    /*�Ҹ��ڵ�*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    /*���ڵ�*/
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    /*LOGCFG���ڵ�*/
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "LOGSERV" ) )
        {
            break;
        }
        cur = cur->next;
    }
    
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    
    while ( cur != XNULL )
    {
        /*�ϴ�������IP��ַ�ڵ�*/
        if ( !XOS_StrCmp(cur->name, "IP") )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                if( XSUCC != XOS_StrtoIp( (XCHAR*)pTempStr, IP) )
                {
                    xmlFree( pTempStr );
                    pTempStr = XNULL ;
                    /*
                    xmlFreeDoc(doc);
                    return XFALSE ;
                    */
                }
                xmlFree( pTempStr );
                pTempStr = XNULL;
            }
            
            pTempStr = xmlGetProp( cur, (xmlChar*)"port" );
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                *Port = (XU32)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr = XNULL;
            }
        }
        
        cur = cur->next;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XTRUE;
    
#endif /*UN_XOS_VXWORKS*/
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XTRUE;
}

/************************************************************************
������  : XML_GetHBCfgFromXosXml
����    : XPMS HB configure informations
����    : filename   XOS �����ļ���
���    :
����    : XBOOL
˵����
************************************************************************/
XBOOL XML_GetHBCfgFromXosXml( t_FaultMHBCfg* pFMCfg, XCONST XS8 *filename )
{
#ifndef XOS_VXWORKS
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    
    if ( XNULL == pFMCfg )
    {
        return XFALSE;
    }
    XOS_MemSet( pFMCfg, 0x00, sizeof(t_FaultMHBCfg) );
    
    doc = xmlParseFile( filename );
    if (doc == XNULL)
    {
        return( XFALSE );
    }
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur -> next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "XPMS" ) )
        {
            break;
        }
        
        cur = cur->next;
    }
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "HBLOCALIP" ) )
        {
            pTempStr = xmlGetProp(cur, (xmlChar*)"hb");
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pFMCfg->m_u16HB = (XU16)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr   = XNULL;
                
                if ( pFMCfg->m_u16HB < 1 || pFMCfg->m_u16HB > 24*60*60 )
                {
                    cli_log_write( eCLIError,
                        "fault manage cfg HBLOCALIP.hb:\"%d\" out of range [1,24*60*60]." ,
                        pFMCfg->m_u16HB );
                    
                    xmlFreeDoc(doc);
                    xmlCleanupParser();
                    return XFALSE;
                }
            }
            
            pTempStr = xmlGetProp(cur, (xmlChar*)"port");
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pFMCfg->m_u32HBPortLocal = (XU32)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr   = XNULL;
                
                pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
                if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
                {
                    pFMCfg->m_u32HBIpLocal = inet_addr( (const char *)pTempStr );
                    cli_log_write( eCLINormal, "fault manage cfg HBLOCALIP.ip:\"%s\".", pTempStr );
                    xmlFree( pTempStr );
                    pTempStr   = XNULL;
                }
            }
        }
        
        cur = cur->next;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
#endif
    
    if ( 0 == pFMCfg->m_u16HB
        || 0 == pFMCfg->m_u32HBIpLocal
        || 0 == pFMCfg->m_u32HBPortLocal )
    {
        cli_log_write( eCLIError, "fault manage cfg invalidate." );
        return XFALSE;
    }
    
    return XTRUE;
}

/************************************************************************
������  : XML_readPatchCfg
����    : get XOS Memory configure informations
����    : filename   XOS �����ļ���
���    :
����    :
˵��    ��
************************************************************************/
XBOOL XML_readPatchCfg( t_PATCHCFG * pPatchCfg, XCONST XS8 *filename )
{
    xmlDocPtr  doc      = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    
    if( XNULL == pPatchCfg)
    {
        return XFALSE;
    }
    
    XOS_MemSet( pPatchCfg, 0x00, sizeof(t_PATCHCFG));
    
    doc = xmlParseFile( filename);
    if (doc == XNULL)
    {
        return( XFALSE );
    }
    
    /*�Ҹ��ڵ�*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    
    /*���ڵ�*/
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur->next;
    }
    
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    
    /*PATCHCFG���ڵ�*/
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "PATCHCFG" ) )
        {
            break;
        }
        cur = cur->next;
    }
    
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XFALSE ;
    }
    
    /*����PATCHCFG�ӽڵ�*/
    while ( cur != XNULL )
    {
        /*��󲹶�����*/
        if ( !XOS_StrCmp(cur->name, "PATCHNUM" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr
                && XOS_StrLen(pTempStr) > 0
                && XOS_StrLen(pTempStr) < XOS_MAX_PATH_NUM_STR)
            {
                XOS_StrNcpy(pPatchCfg->maxPatchNum, pTempStr, XOS_MAX_PATH_NUM_STR);
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        
        /*��������·��*/
        if ( !XOS_StrCmp(cur->name, "PATCHPATH" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr
                && XOS_StrLen(pTempStr) > 0
                && XOS_StrLen(pTempStr) < XOS_MAX_PATH)
            {
                XOS_StrNcpy(pPatchCfg->patchPath, pTempStr, XOS_MAX_PATH);
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        
        /*ϵͳ����ʱ�Ƿ��Զ����ز���*/
        if ( !XOS_StrCmp(cur->name, "PATCHLOAD" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr
                && XOS_StrLen(pTempStr) > 0
                && XOS_StrLen(pTempStr) < XOS_PATH_LOAD_STR)
            {
                XOS_StrNcpy(pPatchCfg->patchLoadFlag, pTempStr, XOS_PATH_LOAD_STR);
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        
        cur = cur->next;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XTRUE;
}


