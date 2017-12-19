/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xmlencoding.h 
**
**  description: : Interfaces, constants and types related to the XML parser.
**
**  author: liguoqiang01093
**
**
**************************************************************/

#ifndef __XML_CHAR_ENCODING_H__
#define __XML_CHAR_ENCODING_H__

#include "xmlstring.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * xmlCharEncoding:
 *
 * Predefined values for some standard encodings.
 * Libxml does not do beforehand translation on UTF8 and ISOLatinX.
 * It also supports ASCII, ISO-8859-1, and UTF16 (LE and BE) by default.
 *
 * Anything else would have to be translated to UTF8 before being
 * given to the parser itself. The BOM for UTF16 and the encoding
 * declaration are looked at and a converter is looked for at that
 * point. If not found the parser stops here as asked by the XML REC. A
 * converter can be registered by the user using xmlRegisterCharEncodingHandler
 * but the current form doesn't allow stateful transcoding (a serious
 * problem agreed !). If iconv has been found it will be used
 * automatically and allow stateful transcoding, the simplest is then
 * to be sure to enable iconv and to provide iconv libs for the encoding
 * support needed.
 *
 * Note that the generic "UTF-16" is not a predefined value.  Instead, only
 * the specific UTF-16LE and UTF-16BE are present.
 */
typedef enum {
    XML_CHAR_ENCODING_ERROR=   -1, /* No char encoding detected */
    XML_CHAR_ENCODING_NONE=	0, /* No char encoding detected */
    XML_CHAR_ENCODING_UTF8=	1, /* UTF-8 */
    XML_CHAR_ENCODING_UTF16LE=	2, /* UTF-16 little endian */
    XML_CHAR_ENCODING_UTF16BE=	3, /* UTF-16 big endian */
    XML_CHAR_ENCODING_UCS4LE=	4, /* UCS-4 little endian */
    XML_CHAR_ENCODING_UCS4BE=	5, /* UCS-4 big endian */
    XML_CHAR_ENCODING_EBCDIC=	6, /* EBCDIC uh! */
    XML_CHAR_ENCODING_UCS4_2143=7, /* UCS-4 unusual ordering */
    XML_CHAR_ENCODING_UCS4_3412=8, /* UCS-4 unusual ordering */
    XML_CHAR_ENCODING_UCS2=	9, /* UCS-2 */
    XML_CHAR_ENCODING_8859_1=	10,/* ISO-8859-1 ISO Latin 1 */
    XML_CHAR_ENCODING_8859_2=	11,/* ISO-8859-2 ISO Latin 2 */
    XML_CHAR_ENCODING_8859_3=	12,/* ISO-8859-3 */
    XML_CHAR_ENCODING_8859_4=	13,/* ISO-8859-4 */
    XML_CHAR_ENCODING_8859_5=	14,/* ISO-8859-5 */
    XML_CHAR_ENCODING_8859_6=	15,/* ISO-8859-6 */
    XML_CHAR_ENCODING_8859_7=	16,/* ISO-8859-7 */
    XML_CHAR_ENCODING_8859_8=	17,/* ISO-8859-8 */
    XML_CHAR_ENCODING_8859_9=	18,/* ISO-8859-9 */
    XML_CHAR_ENCODING_2022_JP=  19,/* ISO-2022-JP */
    XML_CHAR_ENCODING_SHIFT_JIS=20,/* Shift_JIS */
    XML_CHAR_ENCODING_EUC_JP=   21,/* EUC-JP */
    XML_CHAR_ENCODING_ASCII=    22, /* pure ASCII */
    XML_CHAR_ENCODING_GB2312 =  23 /*GB2312*/
} xmlCharEncoding;



/**
 * xmlCharEncodingInputFunc:
 * @out:  a pointer to an array of bytes to store the UTF-8 result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of chars in the original encoding
 * @inlen:  the length of @in
 *
 * Take a block of chars in the original encoding and try to convert
 * it to an UTF-8 block of chars out.
 *
 * Returns the number of bytes written, -1 if lack of space, or -2
 *     if the transcoding failed.
 * The value of @inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictiable.
 * The value of @outlen after return is the number of octets consumed.
 */
typedef int (* xmlCharEncodingInputFunc)(xmlChar *out, int *outlen,
                                         const xmlChar *in, int *inlen);


/**
 * xmlCharEncodingOutputFunc:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of UTF-8 chars
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and try to convert it to another
 * encoding.
 * Note: a first call designed to produce heading info is called with
 * in = NULL. If stateful this should also initialize the encoder state.
 *
 * Returns the number of bytes written, -1 if lack of space, or -2
 *     if the transcoding failed.
 * The value of @inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictiable.
 * The value of @outlen after return is the number of octets produced.
 */
typedef int (* xmlCharEncodingOutputFunc)(xmlChar *out, int *outlen,
                                          const xmlChar *in, int *inlen);


struct _xmlCharEncodingHandler {
    char                       *name;
    xmlCharEncodingInputFunc   input;
    xmlCharEncodingOutputFunc  output;
};
typedef struct _xmlCharEncodingHandler xmlCharEncodingHandler;
typedef xmlCharEncodingHandler *xmlCharEncodingHandlerPtr;

/**
 * xmlBufferAllocationScheme:
 *
 * A buffer allocation scheme can be defined to either match exactly the
 * need or double it's allocated size each time it is found too small.
 */

typedef enum {
    XML_BUFFER_ALLOC_DOUBLEIT,	/* double each time one need to grow */
    XML_BUFFER_ALLOC_EXACT,	/* grow only to the minimal size */
     XML_BUFFER_ALLOC_IMMUTABLE, /* immutable buffer */
    XML_BUFFER_ALLOC_IO		/* special allocation scheme used for I/O */
} xmlBufferAllocationScheme;

/**
 * xmlBuffer:
 *
 * A buffer structure.
 */
struct _XOSxmlBuffer 
{
    xmlChar *content;        /* The buffer content UTF8 */
    XU32 use;        /* The buffer size used */
    XU32 size;        /* The buffer size */
    xmlBufferAllocationScheme alloc; /* The realloc method */
};
typedef struct _XOSxmlBuffer xmlBuffer;
typedef xmlBuffer *xmlBufferPtr;

#ifdef __cplusplus
}
#endif

#include "xmltree.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Interfaces for encoding handlers.
 */
void xmlInitCharEncodingHandlers	(void);
void xmlCleanupCharEncodingHandlers	(void);
void xmlRegisterCharEncodingHandler	(xmlCharEncodingHandlerPtr handler);
xmlCharEncodingHandlerPtr xmlFindCharEncodingHandler	(const char *name);
xmlCharEncodingHandlerPtr xmlNewCharEncodingHandler	(const char *name, 
                          		 xmlCharEncodingInputFunc input,
                          		 xmlCharEncodingOutputFunc output);

/*
 * Interfaces for encoding names and aliases.
 */
int xmlAddEncodingAlias		(const char *name, const char *alias);
int xmlDelEncodingAlias		(const char *alias);
const char * xmlGetEncodingAlias		(const char *alias);
void xmlCleanupEncodingAliases	(void);
xmlCharEncoding xmlParseCharEncoding		(const char *name);
const char * xmlGetCharEncodingName		(xmlCharEncoding enc);

/*
 * Interfaces directly used by the parsers.
 */
int xmlCharEncOutFunc (xmlCharEncodingHandler *handler, xmlBufferPtr out, xmlBufferPtr in);

int xmlCharEncCloseFunc		(xmlCharEncodingHandler *handler);

#ifdef __cplusplus
}
#endif

#endif /* __XML_CHAR_ENCODING_H__ */
