/*
 * entities.c : implementation for the XML entities handling
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */


#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include "../inc/xmlentities.h"
#include "../inc/xmlparser.h"

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/**
 * xmlDumpEntityContent:
 * @buf:  An XML buffer.
 * @content:  The entity content.
 *
 * This will dump the quoted string value, taking care of the special
 * treatment required by %
 */
static void
xmlDumpEntityContent(xmlBufferPtr buf, const xmlChar *content) {
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return;
    if (xmlStrchr(content, '%')) {
        const xmlChar * base, *cur;

    xmlBufferCCat(buf, "\"");
    base = cur = content;
    while (*cur != 0) {
        if (*cur == '"') {
        if (base != cur)
            xmlBufferAdd(buf, base, (XS32)(cur - base));
        xmlBufferAdd(buf, BAD_CAST "&quot;", 6);
        cur++;
        base = cur;
        } else if (*cur == '%') {
        if (base != cur)
            xmlBufferAdd(buf, base, (XS32)(cur - base));
        xmlBufferAdd(buf, BAD_CAST "&#x25;", 6);
        cur++;
        base = cur;
        } else {
        cur++;
        }
    }
    if (base != cur)
        xmlBufferAdd(buf, base, (XS32)(cur - base));
    xmlBufferCCat(buf, "\"");
    } else {
        xmlBufferWriteQuotedString(buf, content);
    }
}

/**
 * xmlDumpEntityDecl:
 * @buf:  An XML buffer.
 * @ent:  An entity table
 *
 * This will dump the content of the entity table as an XML DTD definition
 */
void
xmlDumpEntityDecl(xmlBufferPtr buf, xmlEntityPtr ent) 
{
    if ((buf == NULL) || (ent == NULL)) return;

    switch (ent->etype) {
    case XML_INTERNAL_GENERAL_ENTITY:
        xmlBufferWriteChar(buf, "<!ENTITY ");
        xmlBufferWriteCHAR(buf, ent->name);
        xmlBufferWriteChar(buf, " ");
        if (ent->orig != NULL)
            xmlBufferWriteQuotedString(buf, ent->orig);
        else
            xmlDumpEntityContent(buf, ent->content);
        xmlBufferWriteChar(buf, ">\n");
        break;
    case XML_EXTERNAL_GENERAL_PARSED_ENTITY:
        xmlBufferWriteChar(buf, "<!ENTITY ");
        xmlBufferWriteCHAR(buf, ent->name);
        if (ent->ExternalID != NULL) {
             xmlBufferWriteChar(buf, " PUBLIC ");
             xmlBufferWriteQuotedString(buf, ent->ExternalID);
             xmlBufferWriteChar(buf, " ");
             xmlBufferWriteQuotedString(buf, ent->SystemID);
        } else {
             xmlBufferWriteChar(buf, " SYSTEM ");
             xmlBufferWriteQuotedString(buf, ent->SystemID);
        }
        xmlBufferWriteChar(buf, ">\n");
        break;
    case XML_EXTERNAL_GENERAL_UNPARSED_ENTITY:
        xmlBufferWriteChar(buf, "<!ENTITY ");
        xmlBufferWriteCHAR(buf, ent->name);
        if (ent->ExternalID != NULL) {
             xmlBufferWriteChar(buf, " PUBLIC ");
             xmlBufferWriteQuotedString(buf, ent->ExternalID);
             xmlBufferWriteChar(buf, " ");
             xmlBufferWriteQuotedString(buf, ent->SystemID);
        } else {
             xmlBufferWriteChar(buf, " SYSTEM ");
             xmlBufferWriteQuotedString(buf, ent->SystemID);
        }
        if (ent->content != NULL) { /* Should be true ! */
            xmlBufferWriteChar(buf, " NDATA ");
        if (ent->orig != NULL)
            xmlBufferWriteCHAR(buf, ent->orig);
        else
            xmlBufferWriteCHAR(buf, ent->content);
        }
        xmlBufferWriteChar(buf, ">\n");
        break;
    case XML_INTERNAL_PARAMETER_ENTITY:
        xmlBufferWriteChar(buf, "<!ENTITY % ");
        xmlBufferWriteCHAR(buf, ent->name);
        xmlBufferWriteChar(buf, " ");
        if (ent->orig == NULL)
            xmlDumpEntityContent(buf, ent->content);
        else
            xmlBufferWriteQuotedString(buf, ent->orig);
        xmlBufferWriteChar(buf, ">\n");
        break;
    case XML_EXTERNAL_PARAMETER_ENTITY:
        xmlBufferWriteChar(buf, "<!ENTITY % ");
        xmlBufferWriteCHAR(buf, ent->name);
        if (ent->ExternalID != NULL) {
             xmlBufferWriteChar(buf, " PUBLIC ");
             xmlBufferWriteQuotedString(buf, ent->ExternalID);
             xmlBufferWriteChar(buf, " ");
             xmlBufferWriteQuotedString(buf, ent->SystemID);
        } else {
             xmlBufferWriteChar(buf, " SYSTEM ");
             xmlBufferWriteQuotedString(buf, ent->SystemID);
        }
        xmlBufferWriteChar(buf, ">\n");
        break;
    default:
        break;
    }
}

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

