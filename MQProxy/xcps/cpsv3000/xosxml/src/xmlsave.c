/*
 * xmlsave.c: Implemetation of the document serializer
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#include "../inc/xmlstring.h"
#include "../inc/xmltree.h"
#include "../inc/xmlsave.h"
#include "../inc/xmlIO.h"
#include "../inc/xmlentities.h"

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/************************************************************************
 *                                    *
 *            XHTML detection                    *
 *                                    *
 ************************************************************************/
#define XHTML_STRICT_PUBLIC_ID BAD_CAST \
   "-//W3C//DTD XHTML 1.0 Strict//EN"
#define XHTML_STRICT_SYSTEM_ID BAD_CAST \
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
#define XHTML_FRAME_PUBLIC_ID BAD_CAST \
   "-//W3C//DTD XHTML 1.0 Frameset//EN"
#define XHTML_FRAME_SYSTEM_ID BAD_CAST \
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd"
#define XHTML_TRANS_PUBLIC_ID BAD_CAST \
   "-//W3C//DTD XHTML 1.0 Transitional//EN"
#define XHTML_TRANS_SYSTEM_ID BAD_CAST \
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"

#define XHTML_NS_NAME BAD_CAST "http://www.w3.org/1999/xhtml"

const char *xmlTreeIndentString = "  ";
//static const char *xmlTreeIndentStringThrDef = "  ";

int xmlIndentTreeOutput = 1;
//static int xmlIndentTreeOutputThrDef = 1;

#define IS_CHAR(c)                            \
    ( ( ((c) >= 0x20) && ((c) < 0x7F) ) ||                \
((c) == 0x09) || ((c) == 0x0a)   || ((c) == 0x0d) )


#define IS_BYTE_CHAR(c)     xmlIsChar_ch(c)

#define xmlIsChar_ch(c)        (((0x9 <= (c)) && ((c) <= 0xa)) || \
                 ((c) == 0xd) || \
                  (0x20 <= (c)))

static const xmlChar xmlStringTextNoenc[] =
              { 't', 'e', 'x', 't', 'n', 'o', 'e', 'n', 'c', 0 };

/************************************************************************
 *                                    *
 *             Output error handlers                *
 *                                    *
 ************************************************************************/



/************************************************************************
 *                                    *
 *            Special escaping routines            *
 *                                    *
 ************************************************************************/
static xmlChar *
xmlSerializeHexCharRef(xmlChar *out, int val) {
    xmlChar *ptr;

    *out++ = '&';
    *out++ = '#';
    *out++ = 'x';
    if (val < 0x10) ptr = out;
    else if (val < 0x100) ptr = out + 1;
    else if (val < 0x1000) ptr = out + 2;
    else if (val < 0x10000) ptr = out + 3;
    else if (val < 0x100000) ptr = out + 4;
    else ptr = out + 5;
    out = ptr + 1;
    while (val > 0) {
    switch (val & 0xF) {
        case 0: *ptr-- = '0'; break;
        case 1: *ptr-- = '1'; break;
        case 2: *ptr-- = '2'; break;
        case 3: *ptr-- = '3'; break;
        case 4: *ptr-- = '4'; break;
        case 5: *ptr-- = '5'; break;
        case 6: *ptr-- = '6'; break;
        case 7: *ptr-- = '7'; break;
        case 8: *ptr-- = '8'; break;
        case 9: *ptr-- = '9'; break;
        case 0xA: *ptr-- = 'A'; break;
        case 0xB: *ptr-- = 'B'; break;
        case 0xC: *ptr-- = 'C'; break;
        case 0xD: *ptr-- = 'D'; break;
        case 0xE: *ptr-- = 'E'; break;
        case 0xF: *ptr-- = 'F'; break;
        default: *ptr-- = '0'; break;
    }
    val >>= 4;
    }
    *out++ = ';';
    *out = 0;
    return(out);
}

/**
 * xmlEscapeEntities:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of unescaped UTF-8 bytes
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and escape them. Used when there is no
 * encoding specified.
 *
 * Returns 0 if success, or -1 otherwise
 * The value of @inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 * The value of @outlen after return is the number of octets consumed.
 */
static int
xmlEscapeEntities(xmlChar* out, int *outlen,
                 const xmlChar* in, int *inlen) {
    xmlChar* outstart = out;
    const xmlChar* base = in;
    xmlChar* outend = out + *outlen;
    const xmlChar* inend;
    int val;

    inend = in + (*inlen);
    
    while ((in < inend) && (out < outend)) {
        if (*in == '<') {
        if (outend - out < 4) break;
        *out++ = '&';
        *out++ = 'l';
        *out++ = 't';
        *out++ = ';';
        in++;
        continue;
    } else if (*in == '>') {
        if (outend - out < 4) break;
        *out++ = '&';
        *out++ = 'g';
        *out++ = 't';
        *out++ = ';';
        in++;
        continue;
    } else if (*in == '&') {
        if (outend - out < 5) break;
        *out++ = '&';
        *out++ = 'a';
        *out++ = 'm';
        *out++ = 'p';
        *out++ = ';';
        in++;
        continue;
    } else if (((*in >= 0x20) && ((unsigned char)(*in) < 0x80)) ||
               (*in == '\n') || (*in == '\t')) {
        /*
         * default case, just copy !
         */
        *out++ = *in++;
        continue;
    } else if ((unsigned char)(*in) >= 0x80) {
        /*
         * We assume we have UTF-8 input.
         */
        if (outend - out < 10) break;

        if ((unsigned char)(*in) < 0xC0) {
        in++;
        goto error;
        } else if ((unsigned char)(*in) < 0xE0) {
        if (inend - in < 2) break;
        val = (in[0]) & 0x1F;
        val <<= 6;
        val |= (in[1]) & 0x3F;
        in += 2;
        } else if ((unsigned char)(*in) < 0xF0) {
        if (inend - in < 3) break;
        val = (in[0]) & 0x0F;
        val <<= 6;
        val |= (in[1]) & 0x3F;
        val <<= 6;
        val |= (in[2]) & 0x3F;
        in += 3;
        } else if ((unsigned char)(*in) < 0xF8) {
        if (inend - in < 4) break;
        val = (in[0]) & 0x07;
        val <<= 6;
        val |= (in[1]) & 0x3F;
        val <<= 6;
        val |= (in[2]) & 0x3F;
        val <<= 6;
        val |= (in[3]) & 0x3F;
        in += 4;
        } else {
        in++;
        goto error;
        }
        if (!IS_CHAR(val)) {
        in++;
        goto error;
        }

        /*
         * We could do multiple things here. Just save as a char ref
         */
        out = xmlSerializeHexCharRef(out, val);
    } else if (IS_BYTE_CHAR(*in)) {
        if (outend - out < 6) break;
        out = xmlSerializeHexCharRef(out, *in++);
    } else {
        in++;
        goto error;
    }
    }
    *outlen = (XS32)(out - outstart);
    *inlen = (XS32)(in - base);
    return(0);
error:
    *outlen = (XS32)(out - outstart);
    *inlen = (XS32)(in - base);
    return(-1);
}

/************************************************************************
 *                                    *
 *            Allocation and deallocation            *
 *                                    *
 ************************************************************************/
/**
 * xmlSaveCtxtInit:
 * @ctxt: the saving context
 *
 * Initialize a saving context
 */
static void
xmlSaveCtxtInit(xmlSaveCtxtPtr ctxt)
{
    int i;
    int len;

    if (ctxt == NULL) return;
    if ((ctxt->encoding == NULL) && (ctxt->escape == NULL))
        ctxt->escape = xmlEscapeEntities;
    len = xmlStrlen((xmlChar *)xmlTreeIndentString);
    if ((xmlTreeIndentString == NULL) || (len == 0)) {
        XOS_MemSet(&ctxt->indent[0], 0, MAX_INDENT + 1);
    } else {
    ctxt->indent_size = len;
    ctxt->indent_nr = MAX_INDENT / ctxt->indent_size;
    for (i = 0;i < ctxt->indent_nr;i++)
        XOS_MemCpy(&ctxt->indent[i * ctxt->indent_size], xmlTreeIndentString,
           ctxt->indent_size);
        ctxt->indent[ctxt->indent_nr * ctxt->indent_size] = 0;
    }

}

/************************************************************************
 *                                    *
 *           Dumping XML tree content to a simple buffer        *
 *                                    *
 ************************************************************************/
/**
 * xmlAttrSerializeContent:
 * @buf:  the XML buffer output
 * @doc:  the document
 * @attr:  the attribute pointer
 *
 * Serialize the attribute in the buffer
 */
static void
xmlAttrSerializeContent(xmlOutputBufferPtr buf, xmlAttrPtr attr)
{
    xmlNodePtr children;

    children = attr->val;
    while (children != NULL) {
        switch (children->type) {
            case XML_TEXT_NODE:
                xmlAttrSerializeTxtContent(buf->buffer, attr->doc,
                                   attr, children->content);
                break;
            case XML_ENTITY_REF_NODE:
                xmlBufferAdd(buf->buffer, BAD_CAST "&", 1);
                xmlBufferAdd(buf->buffer, children->name,
                             xmlStrlen(children->name));
                xmlBufferAdd(buf->buffer, BAD_CAST ";", 1);
                break;
            default:
                /* should not happen unless we have a badly built tree */
                break;
        }
        children = children->next;
    }
}

/************************************************************************
 *                                    *
 *           Dumping XML tree content to an I/O output buffer    *
 *                                    *
 ************************************************************************/

static int xmlSaveSwitchEncoding(xmlSaveCtxtPtr ctxt, const char *encoding) {
    xmlOutputBufferPtr buf = ctxt->buf;

    if ((encoding != NULL) && (buf->encoder == NULL) && (buf->conv == NULL)) {
        buf->encoder = xmlFindCharEncodingHandler((const char *)encoding);
        if (buf->encoder == NULL) {
               return(-1);
        }
        buf->conv = xmlBufferCreate();
        if (buf->conv == NULL) {
            xmlCharEncCloseFunc(buf->encoder);
            return(-1);
        }
    /*
     * initialize the state, e.g. if outputting a BOM
     */
        xmlCharEncOutFunc(buf->encoder, buf->conv, NULL);
    }
    return(0);
}

static int xmlSaveClearEncoding(xmlSaveCtxtPtr ctxt) {
    xmlOutputBufferPtr buf = ctxt->buf;
    xmlOutputBufferFlush(buf);
    xmlCharEncCloseFunc(buf->encoder);
    xmlBufferFree(buf->conv);
    buf->encoder = NULL;
    buf->conv = NULL;
    return(0);
}

static void xmlNodeListDumpOutput(xmlSaveCtxtPtr ctxt, xmlNodePtr cur);
static void xmlNodeDumpOutputInternal(xmlSaveCtxtPtr ctxt, xmlNodePtr cur);
static int xmlDocContentDumpOutput(xmlSaveCtxtPtr ctxt, xmlDocPtr cur);

/**
 * xmlNodeListDumpOutput:
 * @cur:  the first node
 *
 * Dump an XML node list, recursive behaviour, children are printed too.
 */
static void
xmlNodeListDumpOutput(xmlSaveCtxtPtr ctxt, xmlNodePtr cur) {
    xmlOutputBufferPtr buf;

    if (cur == NULL) return;
    buf = ctxt->buf;
    while (cur != NULL) {
        if ((ctxt->format == 1) && (xmlIndentTreeOutput) &&
            ((cur->type == XML_ELEMENT_NODE) ||
             (cur->type == XML_COMMENT_NODE) ||
             (cur->type == XML_PI_NODE))) {
                 
               xmlOutputBufferWrite(buf, ctxt->indent_size *
                                 (ctxt->level > ctxt->indent_nr ? 
                      ctxt->indent_nr : ctxt->level),
                     ctxt->indent);
        }
        
        xmlNodeDumpOutputInternal(ctxt, cur);
        
        if (ctxt->format == 1) {
            xmlOutputBufferWrite(buf, 1, "\n");
        }
        cur = cur->next;
    }
}

/**
 * xmlOutputBufferWriteWSNonSig:
 * @ctxt:  The save context
 * @extra: Number of extra indents to apply to ctxt->level
 *
 * Write out formatting for non-significant whitespace output.
 */
static void
xmlOutputBufferWriteWSNonSig(xmlSaveCtxtPtr ctxt, int extra)
{
    int i;
    if ((ctxt == NULL) || (ctxt->buf == NULL))
        return;
    xmlOutputBufferWrite(ctxt->buf, 1, "\n");
    for (i = 0; i < (ctxt->level + extra); i += ctxt->indent_nr) {
        xmlOutputBufferWrite(ctxt->buf, ctxt->indent_size *
                ((ctxt->level + extra - i) > ctxt->indent_nr ?
                 ctxt->indent_nr : (ctxt->level + extra - i)),
                ctxt->indent);
    }
}

/**
 * xmlNsDumpOutput:
 * @buf:  the XML buffer output
 * @cur:  a namespace
 * @ctxt: the output save context. Optional.
 *
 * Dump a local Namespace definition.
 * Should be called in the context of attributes dumps.
 * If @ctxt is supplied, @buf should be its buffer.
 */
static void
xmlNsDumpOutput(xmlOutputBufferPtr buf, xmlNsPtr cur, xmlSaveCtxtPtr ctxt) {
    if ((cur == NULL) || (buf == NULL)) return;
    if ((cur->type == XML_LOCAL_NAMESPACE) && (cur->href != NULL)) {
        if (xmlStrEqual(cur->prefix, BAD_CAST "xml"))
            return;

        if (ctxt != NULL && ctxt->format == 2)
            xmlOutputBufferWriteWSNonSig(ctxt, 2);
        else
            xmlOutputBufferWrite(buf, 1, " ");

        /* Within the context of an element attributes */
        if (cur->prefix != NULL) {
            xmlOutputBufferWrite(buf, 6, "xmlns:");
            xmlOutputBufferWriteString(buf, (const char *)cur->prefix);
        } else {
            xmlOutputBufferWrite(buf, 5, "xmlns");
        }
        
        xmlOutputBufferWrite(buf, 1, "=");
        xmlBufferWriteQuotedString(buf->buffer, cur->href);
    }
}

/**
 * xmlNsDumpOutputCtxt
 * @ctxt: the save context
 * @cur:  a namespace
 *
 * Dump a local Namespace definition to a save context.
 * Should be called in the context of attribute dumps.
 */
static void
xmlNsDumpOutputCtxt(xmlSaveCtxtPtr ctxt, xmlNsPtr cur) {
    xmlNsDumpOutput(ctxt->buf, cur, ctxt);
}

/**
 * xmlAttrDumpOutput:
 * @buf:  the XML buffer output
 * @cur:  the attribute pointer
 *
 * Dump an XML attribute
 */
static void
xmlAttrDumpOutput(xmlSaveCtxtPtr ctxt, xmlAttrPtr cur) {
    xmlOutputBufferPtr buf;

    if (cur == NULL) return;
    buf = ctxt->buf;
    if (buf == NULL) return;
    if (ctxt->format == 2)
        xmlOutputBufferWriteWSNonSig(ctxt, 2);
    else
        xmlOutputBufferWrite(buf, 1, " ");
    
    xmlOutputBufferWriteString(buf, (const char *)cur->name);
    xmlOutputBufferWrite(buf, 2, "=\"");
    xmlAttrSerializeContent(buf, cur);
    xmlOutputBufferWrite(buf, 1, "\"");
}

/**
 * xmlAttrListDumpOutput:
 * @buf:  the XML buffer output
 * @doc:  the document
 * @cur:  the first attribute pointer
 * @encoding:  an optional encoding string
 *
 * Dump a list of XML attributes
 */
static void
xmlAttrListDumpOutput(xmlSaveCtxtPtr ctxt, xmlAttrPtr cur) {
    if (cur == NULL) return;
    while (cur != NULL) {
        xmlAttrDumpOutput(ctxt, cur);
        cur = cur->next;
    }
}


/**
 * xmlNodeDumpOutputInternal:
 * @cur:  the current node
 *
 * Dump an XML node, recursive behaviour, children are printed too.
 */
static void
xmlNodeDumpOutputInternal(xmlSaveCtxtPtr ctxt, xmlNodePtr cur) {
    int format;
    xmlNodePtr tmp;
    xmlChar *start, *end;
    xmlOutputBufferPtr buf;

    if (cur == NULL) return;
    buf = ctxt->buf;
    if (cur->type == XML_XINCLUDE_START)
        return;
    if (cur->type == XML_XINCLUDE_END)
        return;
    if ((cur->type == XML_DOCUMENT_NODE) ||
        (cur->type == XML_HTML_DOCUMENT_NODE)) {
        xmlDocContentDumpOutput(ctxt, (xmlDocPtr) cur);
        return;
    }
    if (cur->type == XML_DTD_NODE) {
        return;
    }
    if (cur->type == XML_DOCUMENT_FRAG_NODE) {
        xmlNodeListDumpOutput(ctxt, cur->children);
        return;
    }
    if (cur->type == XML_ELEMENT_DECL) {
        return;
    }
    if (cur->type == XML_ATTRIBUTE_DECL) {
        return;
    }
    if (cur->type == XML_ENTITY_DECL) {
        xmlDumpEntityDecl(buf->buffer, (xmlEntityPtr) cur);
        return;
    }
    if (cur->type == XML_TEXT_NODE) {
        if (cur->content != NULL) {
            if (cur->name != xmlStringTextNoenc) {
                    xmlOutputBufferWriteEscape(buf, cur->content, ctxt->escape);
            } else {
            /*
             * Disable escaping, needed for XSLT
             */
                xmlOutputBufferWriteString(buf, (const char *) cur->content);
            }
        }
        return;
    }
    if (cur->type == XML_PI_NODE) {
        if (cur->content != NULL) {
            xmlOutputBufferWrite(buf, 2, "<?");
            xmlOutputBufferWriteString(buf, (const char *)cur->name);
            if (cur->content != NULL) {
                if (ctxt->format == 2)
                    xmlOutputBufferWriteWSNonSig(ctxt, 0);
                else
                    xmlOutputBufferWrite(buf, 1, " ");
                xmlOutputBufferWriteString(buf, (const char *)cur->content);
            }
            xmlOutputBufferWrite(buf, 2, "?>");
        } else {
            xmlOutputBufferWrite(buf, 2, "<?");
            xmlOutputBufferWriteString(buf, (const char *)cur->name);
            if (ctxt->format == 2)
                xmlOutputBufferWriteWSNonSig(ctxt, 0);
            xmlOutputBufferWrite(buf, 2, "?>");
        }
        return;
    }
    if (cur->type == XML_COMMENT_NODE) {
        if (cur->content != NULL) {
            xmlOutputBufferWrite(buf, 4, "<!--");
            xmlOutputBufferWriteString(buf, (const char *)cur->content);
            xmlOutputBufferWrite(buf, 3, "-->");
        }
        return;
    }
    if (cur->type == XML_ENTITY_REF_NODE) {
        xmlOutputBufferWrite(buf, 1, "&");
        xmlOutputBufferWriteString(buf, (const char *)cur->name);
        xmlOutputBufferWrite(buf, 1, ";");
        return;
    }
    if (cur->type == XML_CDATA_SECTION_NODE) {
        if (cur->content == NULL || *cur->content == '\0') {
            xmlOutputBufferWrite(buf, 12, "<![CDATA[]]>");
        } else {
            start = end = cur->content;
            while (*end != '\0') {
                if ((*end == ']') && (*(end + 1) == ']') &&
                    (*(end + 2) == '>')) {
                    end = end + 2;
                    xmlOutputBufferWrite(buf, 9, "<![CDATA[");
                    xmlOutputBufferWrite(buf, (XS32)(end - start), (const char *)start);
                    xmlOutputBufferWrite(buf, 3, "]]>");
                    start = end;
                }
                end++;
            }
            if (start != end) {
                xmlOutputBufferWrite(buf, 9, "<![CDATA[");
                xmlOutputBufferWriteString(buf, (const char *)start);
                xmlOutputBufferWrite(buf, 3, "]]>");
            }
        }
        return;
    }
    if (cur->type == XML_ATTRIBUTE_NODE) {
        xmlAttrDumpOutput(ctxt, (xmlAttrPtr) cur);
        return;
    }
    if (cur->type == XML_NAMESPACE_DECL) {
        xmlNsDumpOutputCtxt(ctxt, (xmlNsPtr) cur);
        return;
    }

    format = ctxt->format;
    if (format == 1) {
        tmp = cur->children;
        while (tmp != NULL) {
            if ((tmp->type == XML_TEXT_NODE) ||
            (tmp->type == XML_CDATA_SECTION_NODE) ||
            (tmp->type == XML_ENTITY_REF_NODE)) {
            ctxt->format = 0;
            break;
            }
            tmp = tmp->next;
        }
    }
    xmlOutputBufferWrite(buf, 1, "<");
    if ((cur->ns != NULL) && (cur->ns->prefix != NULL)) {
        xmlOutputBufferWriteString(buf, (const char *)cur->ns->prefix);
        xmlOutputBufferWrite(buf, 1, ":");
    }

    xmlOutputBufferWriteString(buf, (const char *)cur->name);
    
    if (cur->properties != NULL)
        xmlAttrListDumpOutput(ctxt, cur->properties);

    if (((cur->type == XML_ELEMENT_NODE) || (cur->content == NULL)) &&
    (cur->children == NULL) && ((ctxt->options & XML_SAVE_NO_EMPTY) == 0)) {
        if (ctxt->format == 2)
            xmlOutputBufferWriteWSNonSig(ctxt, 0);
        xmlOutputBufferWrite(buf, 2, "/>");
        ctxt->format = format;
        return;
    }
    if (ctxt->format == 2)
        xmlOutputBufferWriteWSNonSig(ctxt, 1);
    xmlOutputBufferWrite(buf, 1, ">");
    if ((cur->type != XML_ELEMENT_NODE) && (cur->content != NULL)) {
        xmlOutputBufferWriteEscape(buf, cur->content, ctxt->escape);
    }
    if (cur->children != NULL) {
        if (ctxt->format == 1) xmlOutputBufferWrite(buf, 1, "\n");
        if (ctxt->level >= 0) ctxt->level++;
        xmlNodeListDumpOutput(ctxt, cur->children);
        if (ctxt->level > 0) ctxt->level--;
        if ((xmlIndentTreeOutput) && (ctxt->format == 1))
            xmlOutputBufferWrite(buf, ctxt->indent_size *
                                 (ctxt->level > ctxt->indent_nr ? 
                      ctxt->indent_nr : ctxt->level),
                     ctxt->indent);
    }
    xmlOutputBufferWrite(buf, 2, "</");
    if ((cur->ns != NULL) && (cur->ns->prefix != NULL)) {
        xmlOutputBufferWriteString(buf, (const char *)cur->ns->prefix);
        xmlOutputBufferWrite(buf, 1, ":");
    }

    xmlOutputBufferWriteString(buf, (const char *)cur->name);
    if (ctxt->format == 2)
        xmlOutputBufferWriteWSNonSig(ctxt, 0);
    xmlOutputBufferWrite(buf, 1, ">");
    ctxt->format = format;
}

/**
 * xmlDocContentDumpOutput:
 * @cur:  the document
 *
 * Dump an XML document.
 */
static int
xmlDocContentDumpOutput(xmlSaveCtxtPtr ctxt, xmlDocPtr cur) {
    const xmlChar *oldenc = cur->encoding;
    const xmlChar *oldctxtenc = ctxt->encoding;
    const xmlChar *encoding = ctxt->encoding;
    xmlCharEncodingOutputFunc oldescape = ctxt->escape;
    xmlCharEncodingOutputFunc oldescapeAttr = ctxt->escapeAttr;
    xmlOutputBufferPtr buf = ctxt->buf;
    xmlCharEncoding enc;
    int switched_encoding = 0;

    if ((cur->type != XML_HTML_DOCUMENT_NODE) &&
        (cur->type != XML_DOCUMENT_NODE))
     return(-1);

    if (ctxt->encoding != NULL) {
        cur->encoding = BAD_CAST ctxt->encoding;
    } else if (cur->encoding != NULL) {
    encoding = cur->encoding;
    } else if (cur->charset != XML_CHAR_ENCODING_UTF8) {
    encoding = (const xmlChar *)
             xmlGetCharEncodingName((xmlCharEncoding) cur->charset);
    }

    if (((cur->type == XML_HTML_DOCUMENT_NODE) &&
         ((ctxt->options & XML_SAVE_AS_XML) == 0) &&
         ((ctxt->options & XML_SAVE_XHTML) == 0)) ||
        (ctxt->options & XML_SAVE_AS_HTML)) {
        
        return(-1);
        
    } else if ((cur->type == XML_DOCUMENT_NODE) ||
               (ctxt->options & XML_SAVE_AS_XML) ||
               (ctxt->options & XML_SAVE_XHTML)) {
        enc = xmlParseCharEncoding((const char*) encoding);
        if ((encoding != NULL) && (oldctxtenc == NULL) &&
            (buf->encoder == NULL) && (buf->conv == NULL) &&
            ((ctxt->options & XML_SAVE_NO_DECL) == 0)) {
            
            if ((enc != XML_CHAR_ENCODING_UTF8) &&
            (enc != XML_CHAR_ENCODING_NONE) &&
            (enc != XML_CHAR_ENCODING_ASCII)) {
                /*
                 * we need to switch to this encoding but just for this
                 * document since we output the XMLDecl the conversion
                 * must be done to not generate not well formed documents.
                 */
                if (xmlSaveSwitchEncoding(ctxt, (const char*) encoding) < 0) {
                    cur->encoding = oldenc;
                    return(-1);
                }
                switched_encoding = 1;
            }
            if (ctxt->escape == xmlEscapeEntities)
                ctxt->escape = NULL;
            if (ctxt->escapeAttr == xmlEscapeEntities)
                ctxt->escapeAttr = NULL;
        }


        /*
         * Save the XML declaration
         */
        if ((ctxt->options & XML_SAVE_NO_DECL) == 0) {
            xmlOutputBufferWrite(buf, 14, "<?xml version=");
            if (cur->version != NULL) 
                xmlBufferWriteQuotedString(buf->buffer, cur->version);
            else
                xmlOutputBufferWrite(buf, 5, "\"1.0\"");
            if (encoding != NULL) {
            xmlOutputBufferWrite(buf, 10, " encoding=");
            xmlBufferWriteQuotedString(buf->buffer, (xmlChar *) encoding);
            }
            switch (cur->standalone) {
            case 0:
                xmlOutputBufferWrite(buf, 16, " standalone=\"no\"");
                break;
            case 1:
                xmlOutputBufferWrite(buf, 17, " standalone=\"yes\"");
                break;
            }
            xmlOutputBufferWrite(buf, 3, "?>\n");
        }

        if (cur->children != NULL) {
            xmlNodePtr child = cur->children;

            while (child != NULL) {
                ctxt->level = 0;
                xmlNodeDumpOutputInternal(ctxt, child);
                xmlOutputBufferWrite(buf, 1, "\n");
                child = child->next;
            }
        }
    }

    /*
     * Restore the state of the saving context at the end of the document
     */
    if ((switched_encoding) && (oldctxtenc == NULL)) {
        xmlSaveClearEncoding(ctxt);
        ctxt->escape = oldescape;
        ctxt->escapeAttr = oldescapeAttr;
    }
    cur->encoding = oldenc;
    return(0);
}

/**
 * xmlAttrSerializeTxtContent:
 * @buf:  the XML buffer output
 * @doc:  the document
 * @attr: the attribute node
 * @string: the text content
 *
 * Serialize text attribute values to an xml simple buffer
 */
void
xmlAttrSerializeTxtContent(xmlBufferPtr buf, xmlDocPtr doc,
                           xmlAttrPtr attr, const xmlChar * string)
{
    xmlChar *base, *cur;

    if (string == NULL)
        return;
    base = cur = (xmlChar *) string;
    while (*cur != 0) {
        if (*cur == '\n') {
            if (base != cur)
                xmlBufferAdd(buf, base, (XS32)(cur - base));
            xmlBufferAdd(buf, BAD_CAST "&#10;", 5);
            cur++;
            base = cur;
        } else if (*cur == '\r') {
            if (base != cur)
                xmlBufferAdd(buf, base, (XS32)(cur - base));
            xmlBufferAdd(buf, BAD_CAST "&#13;", 5);
            cur++;
            base = cur;
        } else if (*cur == '\t') {
            if (base != cur)
                xmlBufferAdd(buf, base, (XS32)(cur - base));
            xmlBufferAdd(buf, BAD_CAST "&#9;", 4);
            cur++;
            base = cur;
        } else if (*cur == '"') {
            if (base != cur)
                xmlBufferAdd(buf, base, (XS32)(cur - base));
            xmlBufferAdd(buf, BAD_CAST "&quot;", 6);
            cur++;
            base = cur;
        } else if (*cur == '<') {
            if (base != cur)
                xmlBufferAdd(buf, base, (XS32)(cur - base));
            xmlBufferAdd(buf, BAD_CAST "&lt;", 4);
            cur++;
            base = cur;
        } else if (*cur == '>') {
            if (base != cur)
                xmlBufferAdd(buf, base, (XS32)(cur - base));
            xmlBufferAdd(buf, BAD_CAST "&gt;", 4);
            cur++;
            base = cur;
        } else if (*cur == '&') {
            if (base != cur)
                xmlBufferAdd(buf, base, (XS32)(cur - base));
            xmlBufferAdd(buf, BAD_CAST "&amp;", 5);
            cur++;
            base = cur;
        } else if (((unsigned char)(*cur) >= 0x80) && ((doc == NULL) ||
                                      (doc->encoding == NULL))) {
            /*
             * We assume we have UTF-8 content.
             */
            xmlChar tmp[10];
            int val = 0, l = 1;

            if (base != cur)
                xmlBufferAdd(buf, base, (XS32)(cur - base));
            if ((unsigned char)(*cur) < 0xC0) {
                if (doc != NULL)
                    doc->encoding = xmlStrdup(BAD_CAST "ISO-8859-1");
                xmlSerializeHexCharRef(tmp, *cur);
                xmlBufferAdd(buf, (xmlChar *) tmp, -1);
                cur++;
                base = cur;
                continue;
            } else if ((unsigned char)(*cur) < 0xE0) {
                val = (cur[0]) & 0x1F;
                val <<= 6;
                val |= (cur[1]) & 0x3F;
                l = 2;
            } else if ((unsigned char)(*cur) < 0xF0) {
                val = (cur[0]) & 0x0F;
                val <<= 6;
                val |= (cur[1]) & 0x3F;
                val <<= 6;
                val |= (cur[2]) & 0x3F;
                l = 3;
            } else if ((unsigned char)(*cur) < 0xF8) {
                val = (cur[0]) & 0x07;
                val <<= 6;
                val |= (cur[1]) & 0x3F;
                val <<= 6;
                val |= (cur[2]) & 0x3F;
                val <<= 6;
                val |= (cur[3]) & 0x3F;
                l = 4;
            }
            if ((l == 1) || (!IS_CHAR(val))) {
                if (doc != NULL)
                    doc->encoding = xmlStrdup(BAD_CAST "ISO-8859-1");
        
                xmlSerializeHexCharRef(tmp, *cur);
                xmlBufferAdd(buf, (xmlChar *) tmp, -1);
                cur++;
                base = cur;
                continue;
            }
            /*
             * We could do multiple things here. Just save
             * as a char ref
             */
            xmlSerializeHexCharRef(tmp, val);
            xmlBufferAdd(buf, (xmlChar *) tmp, -1);
            cur += l;
            base = cur;
        } else {
            cur++;
        }
    }
    if (base != cur)
        xmlBufferAdd(buf, base, (XS32)(cur - base));
}

/************************************************************************
 *                                    *
 *        Saving functions front-ends                *
 *                                    *
 ************************************************************************/

/**
 * xmlDocDumpFormatMemoryEnc:
 * @out_doc:  Document to generate XML text from
 * @doc_txt_ptr:  Memory pointer for allocated XML text
 * @doc_txt_len:  Length of the generated XML text
 * @txt_encoding:  Character encoding to use when generating XML text
 * @format:  should formatting spaces been added
 *
 * Dump the current DOM tree into memory using the character encoding specified
 * by the caller.  Note it is up to the caller of this function to free the
 * allocated memory with xmlFree().
 * Note that @format = 1 provide node indenting only if xmlIndentTreeOutput = 1
 * or xmlKeepBlanksDefault(0) was called
 */

void
xmlDocDumpFormatMemoryEnc(xmlDocPtr out_doc, xmlChar **doc_txt_ptr,
        int * doc_txt_len, const char * txt_encoding,
        int format) {
    xmlSaveCtxt ctxt;
    int                         dummy = 0;
    xmlOutputBufferPtr          out_buff = NULL;
    xmlCharEncodingHandlerPtr   conv_hdlr = NULL;

    if (doc_txt_len == NULL) {
        doc_txt_len = &dummy;   /*  Continue, caller just won't get length */
    }

    if (doc_txt_ptr == NULL) {
        *doc_txt_len = 0;
        return;
    }

    *doc_txt_ptr = NULL;
    *doc_txt_len = 0;

    if (out_doc == NULL) {
        /*  No document, no output  */
        return;
    }

    /*
     *  Validate the encoding value, if provided.
     *  This logic is copied from xmlSaveFileEnc.
     */

    if (txt_encoding == NULL)
        txt_encoding = (const char *) out_doc->encoding;
    if (txt_encoding != NULL) {
        /*
        conv_hdlr = xmlFindCharEncodingHandler(txt_encoding);
        if ( conv_hdlr == NULL ) {
            return;
        }*/
    }

    if ((out_buff = xmlAllocOutputBuffer(conv_hdlr)) == NULL ) {
        return;
    }

    memset(&ctxt, 0, sizeof(ctxt));
    ctxt.doc = out_doc;
    ctxt.buf = out_buff;
    ctxt.level = 0;
    ctxt.format = format ? 1 : 0;
    ctxt.encoding = (const xmlChar *) txt_encoding;
    
    xmlSaveCtxtInit(&ctxt);
    
    ctxt.options |= XML_SAVE_AS_XML;
    xmlDocContentDumpOutput(&ctxt, out_doc);
    xmlOutputBufferFlush(out_buff);
    
    if (out_buff->conv != NULL) {
        *doc_txt_len = out_buff->conv->use;
        *doc_txt_ptr = xmlStrndup(out_buff->conv->content, *doc_txt_len);
    } else {
        *doc_txt_len = out_buff->buffer->use;
        *doc_txt_ptr = xmlStrndup(out_buff->buffer->content, *doc_txt_len);
    }
    (void)xmlOutputBufferClose(out_buff);

    if ((*doc_txt_ptr == NULL) && (*doc_txt_len > 0)) {
        *doc_txt_len = 0;
    }

    return;
}

/**
 * xmlDocDumpMemoryEnc:
 * @out_doc:  Document to generate XML text from
 * @doc_txt_ptr:  Memory pointer for allocated XML text
 * @doc_txt_len:  Length of the generated XML text
 * @txt_encoding:  Character encoding to use when generating XML text
 *
 * Dump the current DOM tree into memory using the character encoding specified
 * by the caller.  Note it is up to the caller of this function to free the
 * allocated memory with xmlFree().
 */

void
xmlDocDumpMemoryEnc(xmlDocPtr out_doc, xmlChar **doc_txt_ptr,
                int * doc_txt_len, const char * txt_encoding) {
    xmlDocDumpFormatMemoryEnc(out_doc, doc_txt_ptr, doc_txt_len,
                          txt_encoding, 0);
}

/**
 * xmlDocFormatDump:
 * @f:  the FILE*
 * @cur:  the document
 * @format: should formatting spaces been added
 *
 * Dump an XML document to an open FILE.
 *
 * returns: the number of bytes written or -1 in case of failure.
 * Note that @format = 1 provide node indenting only if xmlIndentTreeOutput = 1
 * or xmlKeepBlanksDefault(0) was called
 */
int
xmlDocFormatDump(FILE *f, xmlDocPtr cur, int format) {
    xmlSaveCtxt ctxt;
    xmlOutputBufferPtr buf;
    const char * encoding;
    xmlCharEncodingHandlerPtr handler = NULL;
    int ret;

    if (cur == NULL) {
        return(-1);
    }
    encoding = (const char *) cur->encoding;

    if (encoding != NULL) {
        /*
        handler = xmlFindCharEncodingHandler(encoding);
        if (handler == NULL) {
            xmlFree((char *) cur->encoding);
            cur->encoding = NULL;
            encoding = NULL;
        }*/
    }
    buf = xmlOutputBufferCreateFile(f, handler);
    if (buf == NULL) return(-1);
    memset(&ctxt, 0, sizeof(ctxt));
    ctxt.doc = cur;
    ctxt.buf = buf;
    ctxt.level = 0;
    ctxt.format = format ? 1 : 0;
    ctxt.encoding = (const xmlChar *) encoding;
    xmlSaveCtxtInit(&ctxt);
    ctxt.options |= XML_SAVE_AS_XML;
    xmlDocContentDumpOutput(&ctxt, cur);

    ret = xmlOutputBufferClose(buf);
    return(ret);
}

/**
 * xmlSaveFormatFileEnc:
 * @filename:  the filename or URL to output
 * @cur:  the document being saved
 * @encoding:  the name of the encoding to use or NULL.
 * @format:  should formatting spaces be added.
 *
 * Dump an XML document to a file or an URL.
 *
 * Returns the number of bytes written or -1 in case of error.
 * Note that @format = 1 provide node indenting only if xmlIndentTreeOutput = 1
 * or xmlKeepBlanksDefault(0) was called
 */
int
xmlSaveFormatFileEnc( const char * filename, xmlDocPtr cur,
            const char * encoding, int format ) {
    xmlSaveCtxt ctxt;
    xmlOutputBufferPtr buf;
    xmlCharEncodingHandlerPtr handler = NULL;
    int ret;

    if (cur == NULL)
        return(-1);

    if (encoding == NULL)
        encoding = (const char *) cur->encoding;

    if (encoding != NULL) {

        //handler = xmlFindCharEncodingHandler(encoding);
        //if (handler == NULL)
        //    return(-1);
    }

    /* 
     * save the content to a temp buffer.
     */
    buf = xmlOutputBufferCreateFilename(filename, handler, cur->compression);
    if (buf == NULL) return(-1);
    memset(&ctxt, 0, sizeof(ctxt));
    ctxt.doc = cur;
    ctxt.buf = buf;
    ctxt.level = 0;
    ctxt.format = format ? 1 : 0;
    ctxt.encoding = (const xmlChar *) encoding;
    xmlSaveCtxtInit(&ctxt);
    ctxt.options |= XML_SAVE_AS_XML;

    xmlDocContentDumpOutput(&ctxt, cur);
    ret = xmlOutputBufferClose(buf);
    return(ret);
}
#ifdef __cplusplus
}
#endif /* _ _cplusplus */

