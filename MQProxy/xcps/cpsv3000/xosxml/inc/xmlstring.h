/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xmlstring.h 
**
**  description: : Interfaces, constants and types related to the XML parser.
**
**  author: liguoqiang01093
**
**
**************************************************************/

#ifndef __XML_STRING_H__
#define __XML_STRING_H__

//#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "xostype.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * xmlChar:
 *
 * This is a basic byte in an UTF-8 encoded string.
 * It's unsigned allowing to pinpoint case where char * are assigned
 * to xmlChar * (possibly making serialization back impossible).
 */
/* 
#ifdef UNICODE
typedef XU16 xmlChar;
#else
typedef XS8 xmlChar;
#endif */
//typedef unsigned char xmlChar;
typedef char xmlChar;



#define xmlFree(x)       free((x))
#define xmlMalloc(x)     malloc(x)

/**
 * BAD_CAST:
 *
 * Macro to cast a string to an xmlChar * when one know its safe.
 */
#define BAD_CAST (xmlChar *)

/*
 * xmlChar handling
 */
#ifdef XOS_VXWORKS
xmlChar *strdup( const xmlChar *s1 );
#define xmlMemStrdup(x)  strdup((x))
#else
#define xmlMemStrdup(x)  strdup((x))
#endif

#define xmlRealloc(p, x) realloc((p), (x))
/* 字符的拷贝*/
#define XOS_StrCpy(dest,src)    strcpy((char *)dest , (char *)src)   /*#不提倡使用*/
#define XOS_StrNcpy(dest, src,n )    strncpy((char *)dest,(char *)src,(size_t)(n) )

/*字符串连接*/
#define XOS_StrCat(dest, src)   strcat((char *)dest, (XCONST XCHAR *)src)   /*#不提倡使用*/
#define XOS_StrNCat(dest,src,n) strncat((XCHAR *)dest, (XCONST XCHAR *)src,(size_t)(n))

/* 取字符长度*/
#define XOS_StrLen(s)    strlen((char *)s)

/* 内存的拷贝，赋值*/
#define XOS_MemCpy(dest,src,n)    memcpy(dest,src,n)
#define XOS_MemMove(dest,src,n)  memmove(dest,src,n)
#define XOS_MemSet(dest,src,n)    memset(dest,src,n)
#define XOS_MemCmp(dest,src,n)  memcmp(dest,src,n)


xmlChar *xmlStrndup(XCONST xmlChar *cur, XS32 len);
xmlChar *xmlCharStrndup(XCONST XS8 *cur, XS32 len);
xmlChar *xmlCharStrdup(XCONST XS8 *cur);
XS32 xmlStrcmp(XCONST xmlChar *str1, XCONST xmlChar *str2);
XS32 xmlStrEqual(XCONST xmlChar *str1, XCONST xmlChar *str2);
XS32 xmlStrncmp(XCONST xmlChar *str1, XCONST xmlChar *str2, XS32 len);
XCONST xmlChar *xmlStrchr(XCONST xmlChar *str, xmlChar val);
XCONST xmlChar *xmlStrstr(XCONST xmlChar *str, xmlChar *val);
xmlChar *xmlStrsub(XCONST xmlChar *str, XS32 start, XS32 len);
XS32 xmlStrlen(XCONST xmlChar *str);
xmlChar *xmlStrncat(xmlChar *cur, XCONST xmlChar *add, XS32 len);
xmlChar *xmlStrcat(xmlChar *cur, XCONST xmlChar *add);
xmlChar *xmlStrdup(XCONST xmlChar *cur) ;
int xmlStrncasecmp  (const xmlChar *str1, const xmlChar *str2, int len);
int xmlCopyCharMultiByte(xmlChar *out, int val);

int xmlGetUTF8Char(const xmlChar *utf, int *len);
int xmlCheckUTF8(const xmlChar *utf);
	
#ifdef __cplusplus
}
#endif
#endif /* __XML_STRING_H__ */
