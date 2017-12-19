#include <stdio.h>
#include "xmlparser.h"

#include "xosxml.h"
#include "xosmmgt.h"

static int readUnicode(int argc, char ** argv);
static int readUnixXml();
static int readFromBuffer();
static XS16 XML_readStatCfg(XCONST XS8 *filename);
static int readPmCfg();


int main(int argc, char** argv)
{
    t_MEMCFG memcfg;
    //readUnicode(argc, argv);
    //readUnixXml();
    readFromBuffer();

    //XML_readMemCfg1(&memcfg, "./xos.xml");
    //XML_readStatCfg("./sxc.xml");
    
    //XosMM_XmlReadCfg();
    //readPmCfg();
    //XosMM_XMLreadMemCfgNew( &tbl_t);
    //xmlCleanupParser();
    return 0;
}

int readFromBuffer()
{
    xmlDocPtr doc;
    xmlNodePtr rootnode;
    xmlNodePtr node = NULL;
    xmlChar* val = 0;
    const char *content = "<test tid=\"中文文中\" operator=\"&lt;=\" operator2=\"&&amp;\" \
        operator3=\"&#x27;\" operator4=\"&lt;&lt;\" operator5=\"&#60;\" \
        operator6=\"&#38;\" operator7=\"&#x3e;&#x22;&#x3E;&#x27;&#34;&apos;&#60;&#39;&#x3C;&quot;&#x26;&#62;&gt;&#38;&#x3c;&amp;&#x3C;\" value=\"6\" />";

    if(NULL == (doc = xmlParseMemory(content, strlen(content)))) {
        printf("cann't parse xml,exit!\n");
        return 0;
    }
    rootnode = xmlDocGetRootElement(doc);
    node = rootnode;
    if(xmlStrcmp(node->name, (const xmlChar*)"test") == 0) {
        node = node->children;
        while(node && xmlStrcmp(node->name, (const xmlChar*)"tid") != 0) {
            node = node->next;
        }
        while(node && xmlStrcmp(node->name, (const xmlChar*)"operator") == 0) {
            val = xmlGetProp(node, "uri");
            if(val) {
                printf("val=%s\n",val);
                xmlFree(val);
            }
            node = node->next;
        }
    }
    xmlFreeDoc(doc);
}

static int readPmCfg()
{
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar* pTempStr   = XNULL;
    XU32 g_OmIP;

    /*读取文件*/
    doc = xmlParseFile("./pm_cfg.xml");
    if (doc == XNULL)
    {
        return( XFALSE );
    }
    
    /*找到根节点*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        return( XFALSE );
    }
    if ( strcmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        return( XFALSE );
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        return( XFALSE );
    }
    
    /*找到 PMC 主节点*/
    while ( cur != XNULL )
    {
        if ( !strcmp(cur->name, "PMC" ) )
        {
            break;
        }
        cur = cur->next;
    }
        
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
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
        return( XFALSE );
    }

    /*遍历  子节点*/
    while ( cur != XNULL )
    {
        /*OMIP  节点*/
        if (!strcmp(cur->name, "OMIP" ))
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr  )
            {
                printf("omIP:%s\n", pTempStr);
    
                xmlFree( pTempStr );
                pTempStr = XNULL;
            }
            else
            {
                return XFALSE;
            }
            
        }

        cur = cur->next;
    }
    
    xmlFreeDoc(doc);

    return  XTRUE; 
}

static int readUnixXml()
{
    xmlDocPtr doc;
    xmlNodePtr rootnode;
    xmlNodePtr node = NULL;
    xmlChar* val = 0;

    if(NULL == (doc = xmlParseFile("./user_profile.xml"))) {
        printf("cann't find sxc.xml file,exit!\n");
        return 0;
    }
    rootnode = xmlDocGetRootElement(doc);
    node = rootnode;
    if(xmlStrcmp(node->name, (const xmlChar*)"user_profile") == 0) {
        node = node->children;
        while(node && xmlStrcmp(node->name, (const xmlChar*)"database") != 0) {
            node = node->next;
        }
        if(node) {
            if(xmlStrcmp(node->name, (const xmlChar*)"database") == 0) {
                node = node->children;
                if(node->type == XML_TEXT_NODE) {node = node->next;}
                if(xmlStrcmp(node->name, (const xmlChar*)"host") == 0) {
                    
                    val = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(val) {
                        printf("host name content:%s\n",val);
                        xmlFree(val);
                    }
                }
            }
        }
    }
    xmlFreeDoc(doc);
    return 0;
}

static int readUnicode(int argc, char ** argv)
{
    xmlDocPtr doc;
    xmlNodePtr rootnode;
    xmlNodePtr node = NULL;

    if(NULL == (doc = xmlParseFile("./sxc.xml"))) {
        printf("cann't find sxc.xml file,exit!\n");
        return 0;
    }
    rootnode = xmlDocGetRootElement(doc);
    node = rootnode;
    if(xmlStrcmp(node->name, (const xmlChar*)"MODULES") == 0) {
        node = node->children;
        while(node && xmlStrcmp(node->name, (const xmlChar*)"STAT_TABLES") != 0) {
            node = node->next;
        }
        if(xmlStrcmp(node->name, (const xmlChar*)"STAT_TABLES") == 0) {
            node = node->children;
            if(node->type == XML_TEXT_NODE) {node = node->next;}
            if(xmlStrcmp(node->name, (const xmlChar*)"CHANPIN") == 0) {
                node = node->children;
                if(node->type == XML_TEXT_NODE) {node = node->next;}
                if(xmlStrcmp(node->name, (const xmlChar*)"MODULE_ID") == 0) {
                    node = node->children;
                    if(node->type == XML_TEXT_NODE) {node = node->next;}
                    if(xmlStrcmp(node->name, (const xmlChar*)"TAB_ID") == 0) {
                        const xmlChar* val = xmlGetProp(node, "name");
                        if(val) {
                            printf("name TAB_ID  property:%s\n", (XCHAR*)val);
                            xmlFree(val);
                        }
                        node = node->children;
                        if(node->type == XML_TEXT_NODE) {node = node->next;}
                        if(xmlStrcmp(node->name, (const xmlChar*)"TABLE_SIZE") == 0) {
                            val = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                            if(val) {
                                printf("name TABLE_SIZE content:%s\n",val);
                                xmlFree(val);
                            }
                        }

                    }
                }
            }
        }
    }
    xmlFreeDoc(doc);
    return 0;
}

XS16 XML_readStatCfg(XCONST XS8 *filename)
{
    static XU32 numberId = 0;   //当前模块的索引号
    XU32 tmpTabNum = 0;
    xmlDocPtr  doc        = NULL;
    xmlNodePtr  tmpcur  = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar*   pTempStr = XNULL;
    
    doc = xmlParseFile( filename );
    if (doc == XNULL)
    {
        return( XFALSE );
    }
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        return XFALSE ;
    }
    //如果没找到STAT_TABLES
    if ( 0 != strcmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
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
        return XFALSE ;
    }

    while(cur && (0 != strcmp( cur->name, "STAT_TABLES")))
    {
        cur = cur ->next;
    }
    if(cur == XNULL)
    {
        xmlFreeDoc(doc);
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
        return XFALSE ;
    }
        
    while ( cur != XNULL )
    {
        //如果没找到STAT_TABLES
        pTempStr = xmlGetProp(cur, "name");
        if ( (0 == strcmp(cur->name, "CHANPIN" )) && (0 == strcmp(pTempStr, "SXC")))
        {
            cur = cur->xmlChildrenNode;
            xmlFree(pTempStr);
            break;
        }
            
        cur = cur->next;
    }
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
        return XFALSE ;
    }
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        return XFALSE ;
    }
        
//   while ( cur && xmlIsBlankNode ( cur ) )
//    {    
        while ( cur != XNULL )
        {
            /*模块ID*/
            if ( 0 == strcmp(cur->name, "MODULE_ID" ) )
            {
                pTempStr = xmlGetProp(cur, "num");
                if ( XNULL != pTempStr && strlen(pTempStr) > 0 )
                {
                    printf("MODULE_ID prop:module_id-num=%d\n", (XU32) atol(pTempStr));
                    xmlFree( pTempStr );
                    pTempStr   = XNULL;
                }
            }
            cur = cur->xmlChildrenNode;
            while ( cur && xmlIsBlankNode ( cur ) )
            {
                cur = cur ->next;
            }
            if ( cur == XNULL )
            {
                xmlFreeDoc(doc);
                return XFALSE ;
            }
            /*表ID*/
            while(cur != XNULL)
            {
                if ( 0 == strcmp(cur->name, "TAB_ID" ) )
                {
                    pTempStr = xmlGetProp(cur, "num");
                    if ( XNULL != pTempStr && strlen(pTempStr) > 0 )
                    {
                        printf("TAB_ID prop:tab_id-num=%d\n", (XU32) atol(pTempStr));
                        xmlFree( pTempStr );
                        pTempStr   = XNULL;
                    }
                }
                cur = cur->xmlChildrenNode;
                while ( cur && xmlIsBlankNode ( cur ) )
                {
                    cur = cur ->next;
                }
                if ( cur == XNULL )
                {
                    xmlFreeDoc(doc);
                    return XFALSE ;
                }
                /*表大小*/
                if ( 0 == strcmp(cur->name, "TABLE_SIZE" ) )
                {
                    pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
                    if ( XNULL != pTempStr && strlen(pTempStr) > 0 )
                    {
                        printf("TABLE_SIZE value=%d\n", (XU32) atol(pTempStr));
                        xmlFree( pTempStr );
                        pTempStr   = XNULL;
                        tmpTabNum++;  //模块内的第几张表
                    }
                }
                cur = cur->parent;
                while ( cur && xmlIsBlankNode ( cur ) )
                {
                    cur = cur ->next;
                }
                if ( XNULL == cur )
                {
                    xmlFreeDoc(doc);
                    return XFALSE ;
                }
                tmpcur = cur;
                cur = cur->next;
                while ( cur && xmlIsBlankNode ( cur ) )
                {
                    cur = cur ->next;
                }
            }
            cur = tmpcur->parent;
            while ( cur && xmlIsBlankNode ( cur ) )
            {
                cur = cur ->next;
            }
            if ( XNULL == cur )
            {
                xmlFreeDoc(doc);
                return XFALSE ;
            }
            cur = cur->next;
             while ( cur && xmlIsBlankNode ( cur ) )
            {
                cur = cur ->next;
            }
            tmpTabNum = 0;
            numberId++;
        }

    xmlFreeDoc(doc);
    return XTRUE;
}