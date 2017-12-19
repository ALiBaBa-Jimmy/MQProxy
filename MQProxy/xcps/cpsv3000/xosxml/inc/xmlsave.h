/*
 * Summary: the XML document serializer
 * Description: API to save document or subtree of document
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Daniel Veillard
 */

#ifndef __XML_XMLSAVE_H__
#define __XML_XMLSAVE_H__

#include "xmltree.h"
#include "xmlencoding.h"
#include "xmlIO.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_INDENT 60

//typedef struct _XOSxmlEntity xmlEntity;
//typedef xmlEntity *xmlEntityPtr;

/**
 * xmlSaveOption:
 *
 * This is the set of XML save options that can be passed down
 * to the xmlSaveToFd() and similar calls.
 */
typedef enum {
    XML_SAVE_FORMAT     = 1<<0,    /* format save output */
    XML_SAVE_NO_DECL    = 1<<1,    /* drop the xml declaration */
    XML_SAVE_NO_EMPTY    = 1<<2, /* no empty tags */
    XML_SAVE_NO_XHTML    = 1<<3, /* disable XHTML1 specific rules */
    XML_SAVE_XHTML    = 1<<4, /* force XHTML1 specific rules */
    XML_SAVE_AS_XML     = 1<<5, /* force XML serialization on HTML doc */
    XML_SAVE_AS_HTML    = 1<<6, /* force HTML serialization on XML doc */
    XML_SAVE_WSNONSIG   = 1<<7  /* format with non-significant whitespace */
} xmlSaveOption;


typedef struct _xmlSaveCtxt xmlSaveCtxt;
typedef xmlSaveCtxt *xmlSaveCtxtPtr;
struct _xmlSaveCtxt {
    void *_private;
    int type;
    int fd;
    const xmlChar *filename;
    const xmlChar *encoding;
    xmlCharEncodingHandlerPtr handler;
    xmlOutputBufferPtr buf;
    xmlDocPtr doc;
    int options;
    int level;
    int format;
    char indent[MAX_INDENT + 1];    /* array for indenting output */
    int indent_nr;
    int indent_size;
    xmlCharEncodingOutputFunc escape;    /* used for element content */
    xmlCharEncodingOutputFunc escapeAttr;/* used for attribute content */
};

int xmlSaveFormatFileEnc( const char * filename, xmlDocPtr cur,
            const char * encoding, int format );


void xmlDocDumpMemoryEnc(xmlDocPtr out_doc, xmlChar **doc_txt_ptr,
                int * doc_txt_len, const char * txt_encoding);
void xmlDocDumpFormatMemoryEnc(xmlDocPtr out_doc, xmlChar **doc_txt_ptr,
        int * doc_txt_len, const char * txt_encoding, int format);

int xmlDocFormatDump(FILE *f, xmlDocPtr cur, int format);

void xmlAttrSerializeTxtContent(xmlBufferPtr buf, xmlDocPtr doc,
                           xmlAttrPtr attr, const xmlChar * string);

#ifdef __cplusplus
}
#endif
#endif /* __XML_XMLSAVE_H__ */


