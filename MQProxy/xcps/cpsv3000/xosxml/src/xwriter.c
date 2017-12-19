/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xwriter.h 
**
**  description: : Interfaces, constants and types related to the XML writer.
**
**  author: liguoqiang01093
**
**  date:   2012.9.13
**
***************************************************************
**                          history                     
**  
***************************************************************
**************************************************************/

#include "xwriter.h"
#include "xmlsave.h"

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/************************************************************************
������  : xmlCreateWriter
����    : ����һ��xmlDoc����������дxml���
����    : 
���    :
����    : xmlDocPtr
˵����
************************************************************************/
xmlDocPtr xmlCreateWriter(XCONST XCHAR* rootName, XCONST XCHAR* rootContent)
{
    xmlDocPtr xmldoc = NULL;
    xmlNodePtr xmlnode = NULL;
    
    xmldoc = xmlNewDoc(BAD_CAST "1.0");
    if(xmldoc) {
        xmlnode = xmlNewDocNode(xmldoc,
                (const xmlChar*)rootName,
                (rootContent ? (const xmlChar*)rootContent : NULL));
        xmlDocSetRootElement(xmldoc, xmlnode);
    }
    return xmldoc;
}
/************************************************************************
������  : xmlReleaseDoc
����    : doc 
����    : 
���    :
����    : XVOID
˵����
************************************************************************/
XVOID xmlReleaseDoc(xmlDocPtr doc)
{
    xmlFreeDoc(doc);
}
/************************************************************************
������  : xmlGetRootNode
����    : ����һ��xmlDoc����������дxml root�ڵ�
����    : 
���    :
����    : xmlNodePtr
˵����
************************************************************************/
xmlNodePtr xmlGetRootNode(xmlDocPtr doc)
{
    xmlNodePtr node = NULL;
    
    if(doc) {
        node = xmlDocGetRootElement(doc);
    }
    return node;
}
/************************************************************************
������  : xmlAddChildNode
����    : �ڸ��ڵ�������һ���ӽڵ�
����    :  paramNode ���ڵ�
            type :node type
            name �ӽڵ�����
            content �ӽڵ�����
���    :
����    : xmlNodePtr
˵����
************************************************************************/
xmlNodePtr xmlAddChildNode(xmlNodePtr parentNode, xmlElementType type,XCONST XCHAR* name, XCONST XCHAR* content)
{
    xmlNodePtr node = NULL;
    if(NULL != (node = xmlNewNode((const xmlChar*)name,type))) {
        if(content) {
            xmlSetNodeContent(node, (XCHAR*)content);
        }
        xmlAddChild(parentNode, node);
    }
    return node;
}
/************************************************************************
������  : xmlDelChildNode
����    : �ڸ��ڵ���ɾ��һ���ӽڵ�
����    :  paramNode Ҫɾ�����ӽڵ�
���    :
����    : void
˵����
************************************************************************/
XVOID xmlDelChildNode(xmlNodePtr node)
{
    xmlUnlinkNode(node);
    xmlFreeNode(node);
}

/************************************************************************
������  : xmlSetNodeContent
����    : ����һ���ڵ������
����    :  node �ڵ�
            val ����
���    :
����    : void
˵����
************************************************************************/
XVOID xmlSetNodeContent(xmlNodePtr node, XCONST XCHAR* val)
{
    if(node && val) {
        xmlNodeSetContent(node, val);
    }
}
/************************************************************************
������  : xmlSetNodeProperty
����    : ����һ���ڵ������
����    :  node �ڵ�
            key ��������
            val ����ֵ
���    :
����    : void
˵����
************************************************************************/
XVOID xmlSetNodeProperty(xmlNodePtr node, XCONST XCHAR* key,XCONST XCHAR* val)
{
    if(node && key && val) {
        xmlSetProp(node, (const xmlChar*)key, (XCONST xmlChar*)val);
    }
}
/************************************************************************
������  : xmlDumpToMemory
����    : ������ڴ�
����    :  doc 
            out ���
            len �������
            encode����Ҫ��
���    :
����    : void
˵����
************************************************************************/
XVOID xmlDumpToMemory(xmlDocPtr doc, XCHAR** out, XS32 * len, XCONST XCHAR* encode)
{
    if(doc) {
        xmlDocDumpMemoryEnc(doc, (xmlChar**)out, len, encode);
    }
}
/************************************************************************
������  : xmlDumpToFile
����    : ������ļ�
����    :  doc 
            out ���
            len �������
            encode����Ҫ��
���    :
����    : XS32
˵����
************************************************************************/
XS32 xmlDumpToFile(xmlDocPtr doc, XCONST XCHAR* fileName, XCONST XCHAR* encode)
{
    XS32 ret = -1;
    if(doc) {
        ret = xmlSaveFormatFileEnc(fileName, doc, encode, 1);
    }
    return ret;
}
/************************************************************************
������  : xmlDumpToFd
����    : ������ļ�
����    :  fd �ļ�������
���    :
����    : XS32
˵����
************************************************************************/
XS32 xmlDumpToFd(xmlDocPtr doc, FILE * fd)
{
    XS32 ret = -1;
    if(doc) {
        ret = xmlDocFormatDump(fd, doc, 0);
    }
    return ret;
}

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

