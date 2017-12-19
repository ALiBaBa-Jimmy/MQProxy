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
#ifndef __X_WRITER_H
#define __X_WRITER_H

#include "xostype.h"
#include "xmltree.h"

#ifdef __cplusplus
extern "C" {
#endif

xmlDocPtr xmlCreateWriter(XCONST XCHAR* rootName, XCONST XCHAR* rootContent);
XVOID xmlReleaseDoc(xmlDocPtr doc);
xmlNodePtr xmlGetRootNode(xmlDocPtr doc);
xmlNodePtr xmlAddChildNode(xmlNodePtr parentNode,xmlElementType type, XCONST XCHAR* name, XCONST XCHAR* content);
XVOID xmlDelChildNode(xmlNodePtr node);
XVOID xmlSetNodeContent(xmlNodePtr node, XCONST XCHAR* val);
XVOID xmlSetNodeProperty(xmlNodePtr node, XCONST XCHAR* key, XCONST XCHAR* val);
XVOID xmlDumpToMemory(xmlDocPtr doc, XCHAR** out, XS32 * len, XCONST XCHAR* encode);
XS32 xmlDumpToFile(xmlDocPtr doc, XCONST XCHAR* fileName, XCONST XCHAR* encode);
XS32 xmlDumpToFd(xmlDocPtr doc, FILE * fd);

#ifdef __cplusplus
}
#endif

#endif
