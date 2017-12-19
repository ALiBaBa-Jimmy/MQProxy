 /* Summary: interface for the XML entities handling
 * Description: this module provides some of the entity API needed
 *              for the parser and applications.
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Daniel Veillard
 */

#ifndef __XML_ENTITIES_H__
#define __XML_ENTITIES_H__

#include "xmltree.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The different valid entity types.
 */
typedef enum {
    XML_INTERNAL_GENERAL_ENTITY = 1,
    XML_EXTERNAL_GENERAL_PARSED_ENTITY = 2,
    XML_EXTERNAL_GENERAL_UNPARSED_ENTITY = 3,
    XML_INTERNAL_PARAMETER_ENTITY = 4,
    XML_EXTERNAL_PARAMETER_ENTITY = 5,
    XML_INTERNAL_PREDEFINED_ENTITY = 6
} xmlEntityType;

/*
 * An unit of storage for an entity, contains the string, the value
 * and the linkind data needed for the linking in the hash table.
 */

typedef struct _XOSxmlEntity xmlEntity;
typedef xmlEntity *xmlEntityPtr;
struct _XOSxmlEntity {
    void           *_private;            /* application data */
    xmlElementType          type;       /* XML_ENTITY_DECL, must be second ! */
    const xmlChar          *name;    /* Entity name */
    struct _XOSxmlNode    *children;    /* First child link */
    struct _XOSxmlNode        *last;    /* Last child link */
    struct _xmlDtd       *parent;    /* -> DTD */
    struct _XOSxmlNode        *next;    /* next sibling link  */
    struct _XOSxmlNode        *prev;    /* previous sibling link  */
    struct _XOSxmlDoc          *doc;       /* the containing document */

    xmlChar                *orig;    /* content without ref substitution */
    xmlChar             *content;    /* content or ndata if unparsed */
    int                   length;    /* the content length */
    xmlEntityType          etype;    /* The entity type */
    const xmlChar    *ExternalID;    /* External identifier for PUBLIC */
    const xmlChar      *SystemID;    /* URI for a SYSTEM or PUBLIC Entity */

    struct __XOSxmlEntity     *nexte;    /* unused */
    const xmlChar           *URI;    /* the full URI as computed */
    int                    owner;    /* does the entity own the childrens */
    int             checked;    /* was the entity content checked */
                    /* this is also used to count entites
                     * references done from that entity */
};

void xmlDumpEntityDecl    (xmlBufferPtr buf, xmlEntityPtr ent);

    
#ifdef __cplusplus
}
#endif

# endif /* __XML_ENTITIES_H__ */
