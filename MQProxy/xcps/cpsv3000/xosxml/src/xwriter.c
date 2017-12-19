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
函数名  : xmlCreateWriter
功能    : 创建一个xmlDoc变量，用来写xml语句
输入    : 
输出    :
返回    : xmlDocPtr
说明：
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
函数名  : xmlReleaseDoc
功能    : doc 
输入    : 
输出    :
返回    : XVOID
说明：
************************************************************************/
XVOID xmlReleaseDoc(xmlDocPtr doc)
{
    xmlFreeDoc(doc);
}
/************************************************************************
函数名  : xmlGetRootNode
功能    : 创建一个xmlDoc变量，用来写xml root节点
输入    : 
输出    :
返回    : xmlNodePtr
说明：
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
函数名  : xmlAddChildNode
功能    : 在父节点下增加一个子节点
输入    :  paramNode 父节点
            type :node type
            name 子节点名称
            content 子节点内容
输出    :
返回    : xmlNodePtr
说明：
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
函数名  : xmlDelChildNode
功能    : 在父节点下删除一个子节点
输入    :  paramNode 要删除的子节点
输出    :
返回    : void
说明：
************************************************************************/
XVOID xmlDelChildNode(xmlNodePtr node)
{
    xmlUnlinkNode(node);
    xmlFreeNode(node);
}

/************************************************************************
函数名  : xmlSetNodeContent
功能    : 设置一个节点的内容
输入    :  node 节点
            val 内容
输出    :
返回    : void
说明：
************************************************************************/
XVOID xmlSetNodeContent(xmlNodePtr node, XCONST XCHAR* val)
{
    if(node && val) {
        xmlNodeSetContent(node, val);
    }
}
/************************************************************************
函数名  : xmlSetNodeProperty
功能    : 设置一个节点的属性
输入    :  node 节点
            key 属性名称
            val 属性值
输出    :
返回    : void
说明：
************************************************************************/
XVOID xmlSetNodeProperty(xmlNodePtr node, XCONST XCHAR* key,XCONST XCHAR* val)
{
    if(node && key && val) {
        xmlSetProp(node, (const xmlChar*)key, (XCONST xmlChar*)val);
    }
}
/************************************************************************
函数名  : xmlDumpToMemory
功能    : 输出到内存
输入    :  doc 
            out 输出
            len 输出长度
            encode编码要求
输出    :
返回    : void
说明：
************************************************************************/
XVOID xmlDumpToMemory(xmlDocPtr doc, XCHAR** out, XS32 * len, XCONST XCHAR* encode)
{
    if(doc) {
        xmlDocDumpMemoryEnc(doc, (xmlChar**)out, len, encode);
    }
}
/************************************************************************
函数名  : xmlDumpToFile
功能    : 输出到文件
输入    :  doc 
            out 输出
            len 输出长度
            encode编码要求
输出    :
返回    : XS32
说明：
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
函数名  : xmlDumpToFd
功能    : 输出到文件
输入    :  fd 文件描述符
输出    :
返回    : XS32
说明：
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

