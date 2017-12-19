/*
 * Summary: interface for the I/O interfaces used by the parser
 * Description: interface for the I/O interfaces used by the parser
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Daniel Veillard
 */

#ifndef __XML_IO_H__
#define __XML_IO_H__

#include <stdio.h>
#include "xmltree.h"
#include "xmlencoding.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*xmlOutputWriteCallback) (void * context, const char * buffer, int len);
typedef int (*xmlOutputCloseCallback) (void * context);
typedef int (*xmlInputMatchCallback) (char const *filename);
typedef void * ( *xmlInputOpenCallback) (char const *filename);
typedef int ( *xmlInputReadCallback) (void * context, char * buffer, int len);
typedef int ( *xmlInputCloseCallback) (void * context);

struct _xmlOutputBuffer {
    void*                   context;
    xmlOutputWriteCallback  writecallback;
    xmlOutputCloseCallback  closecallback;
    
    xmlCharEncodingHandlerPtr encoder; /* I18N conversions to UTF-8 */
    
    xmlBufferPtr buffer;    /* Local buffer encoded in UTF-8 or ISOLatin */
    xmlBufferPtr conv;      /* if encoder != NULL buffer for output */
    int written;            /* total number of byte written */
    int error;
};

typedef struct _xmlOutputBuffer xmlOutputBuffer;
typedef xmlOutputBuffer *xmlOutputBufferPtr;
typedef struct _XOSxmlParserInputBuffer xmlParserInputBuffer;
typedef xmlParserInputBuffer *xmlParserInputBufferPtr;

XVOID xmlFreeParserInputBuffer(xmlParserInputBufferPtr in);
XS32 xmlParserInputBufferGrow(xmlParserInputBufferPtr in, XS32 len);
XS32 xmlParserInputBufferRead(xmlParserInputBufferPtr in, XS32 len);
xmlParserInputBufferPtr xmlAllocParserInputBuffer();
xmlParserInputBufferPtr xmlParserInputBufferCreateFilename( const char *filename ) ;
XS8 * xmlParserGetDirectory(XCONST XS8 *filename) ;

int xmlOutputBufferWrite(xmlOutputBufferPtr out, int len, const char *buf);
int xmlOutputBufferWriteString(xmlOutputBufferPtr out, const char *str);

int xmlOutputBufferWriteEscape(xmlOutputBufferPtr out, const xmlChar *str,
                           xmlCharEncodingOutputFunc escaping);
int xmlOutputBufferFlush(xmlOutputBufferPtr out);
xmlOutputBufferPtr xmlOutputBufferCreateFile(FILE *file, xmlCharEncodingHandlerPtr encoder);
xmlOutputBufferPtr xmlOutputBufferCreateFilename(const char *URI,
                              xmlCharEncodingHandlerPtr encoder,
                              int compression );

xmlOutputBufferPtr xmlAllocOutputBuffer(xmlCharEncodingHandlerPtr encoder);
int xmlOutputBufferClose(xmlOutputBufferPtr out);

xmlParserInputBufferPtr xmlParserInputBufferCreateMem(const char *mem, int size, xmlCharEncoding enc);

/**
 * Default 'file://' protocol callbacks 
 */

#ifdef __cplusplus
}
#endif

#endif /* __XML_IO_H__ */
