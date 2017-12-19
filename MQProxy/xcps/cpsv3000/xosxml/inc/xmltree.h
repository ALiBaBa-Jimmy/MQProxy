/*
 * Summary: interfaces for tree manipulation
 * Description: this module describes the structures found in an tree resulting
 *              from an XML or HTML parsing, as well as the API provided for
 *              various processing on that tree
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Daniel Veillard
 */

#ifndef __XML_TREE_H__
#define __XML_TREE_H__

#include <stdio.h>
#include "xostype.h"
#include "xmlIO.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * BASE_BUFFER_SIZE:
 *
 * default buffer size 4000.
 */
#define BASE_BUFFER_SIZE 4096

/**
 * LIBXML_NAMESPACE_DICT:
 *
 * Defines experimental behaviour:
 * 1) xmlNs gets an additional field @context (a xmlDoc)
 * 2) when creating a tree, xmlNs->href is stored in the dict of xmlDoc.
 */
/* #define LIBXML_NAMESPACE_DICT */

/**
 * XML_XML_NAMESPACE:
 *
 * This is the namespace for the special xml: prefix predefined in the
 * XML Namespace specification.
 */
#define XML_XML_NAMESPACE \
    (XCONST xmlChar *) "http://www.w3.org/XML/1998/namespace"

/**
 * XML_XML_ID:
 *
 * This is the name for the special xml:id attribute
 */
#define XML_XML_ID (XCONST xmlChar *) "xml:id"

/*
 * The different element types carried by an XML tree.
 *
 * NOTE: This is synchronized with DOM Level1 values
 *       See http://www.w3.org/TR/REC-DOM-Level-1/
 *
 * Actually this had diverged a bit, and now XML_DOCUMENT_TYPE_NODE should
 * be deprecated to use an XML_DTD_NODE.
 */
typedef enum {
    XML_ELEMENT_NODE=        1,
    XML_ATTRIBUTE_NODE=        2,
    XML_TEXT_NODE=        3,
    XML_CDATA_SECTION_NODE=    4,
    XML_ENTITY_REF_NODE=    5,
    XML_ENTITY_NODE=        6,
    XML_PI_NODE=        7,
    XML_COMMENT_NODE=        8,
    XML_DOCUMENT_NODE=        9,
    XML_DOCUMENT_TYPE_NODE=    10,
    XML_DOCUMENT_FRAG_NODE=    11,
    XML_NOTATION_NODE=        12,
    XML_HTML_DOCUMENT_NODE=    13,
    XML_DTD_NODE=        14,
    XML_ELEMENT_DECL=        15,
    XML_ATTRIBUTE_DECL=        16,
    XML_ENTITY_DECL=        17,
    XML_NAMESPACE_DECL=        18,
    XML_XINCLUDE_START=        19,
    XML_XINCLUDE_END=        20
#ifdef LIBXML_DOCB_ENABLED
   ,XML_DOCB_DOCUMENT_NODE=    21
#endif
} xmlElementType;


/**
 * xmlNotation:
 *
 * A DTD Notation definition.
 */
struct _xmlNotation {
    XCONST xmlChar               *name;            /* Notation name */
    XCONST xmlChar               *PublicID;    /* Public identifier, if any */
    XCONST xmlChar               *SystemID;    /* System identifier, if any */
};
typedef struct _xmlNotation xmlNotation;
typedef xmlNotation *xmlNotationPtr;


/**
 * xmlAttributeType:
 *
 * A DTD Attribute type definition.
 */

typedef enum {
    XML_ATTRIBUTE_CDATA = 1,
    XML_ATTRIBUTE_ID,
    XML_ATTRIBUTE_IDREF    ,
    XML_ATTRIBUTE_IDREFS,
    XML_ATTRIBUTE_ENTITY,
    XML_ATTRIBUTE_ENTITIES,
    XML_ATTRIBUTE_NMTOKEN,
    XML_ATTRIBUTE_NMTOKENS,
    XML_ATTRIBUTE_ENUMERATION,
    XML_ATTRIBUTE_NOTATION
} xmlAttributeType;

/**
 * xmlAttributeDefault:
 *
 * A DTD Attribute default definition.
 */

typedef enum {
    XML_ATTRIBUTE_NONE = 1,
    XML_ATTRIBUTE_REQUIRED,
    XML_ATTRIBUTE_IMPLIED,
    XML_ATTRIBUTE_FIXED
} xmlAttributeDefault;

/**
 * xmlEnumeration:
 *
 * List structure used when there is an enumeration in DTDs.
 */

typedef struct _XOSxmlEnumeration xmlEnumeration;
typedef xmlEnumeration *xmlEnumerationPtr;
struct _XOSxmlEnumeration 
{
    struct _XOSxmlEnumeration    *next;    /* next one */
    XCONST xmlChar            *name;    /* Enumeration name */
};

/**
 * xmlAttribute:
 *
 */
typedef struct _XOSxmlAttribute xmlAttribute;
typedef xmlAttribute *xmlAttributePtr;
struct _XOSxmlAttribute 
{
    XCONST xmlChar         *elem;    /* Element holding the attribute */
    XCONST xmlChar         *name;    /* Attribute name */
    struct _XOSxmlAttribute   *next;       /* list of attributes of an element */
    xmlAttributeType       type;    /* The type */
    xmlAttributeDefault    def;        /* the default */
    XCONST xmlChar         *defaultValue;/* or the default value */
    xmlEnumerationPtr      tree;        /* or the enumeration tree if any */
    XCONST xmlChar         *prefix;      /* the namespace prefix if any */
};


/**
 * xmlElementContentType:
 *
 * Possible definitions of element content types.
 */
typedef enum {
    XML_ELEMENT_CONTENT_PCDATA = 1,
    XML_ELEMENT_CONTENT_ELEMENT,
    XML_ELEMENT_CONTENT_SEQ,
    XML_ELEMENT_CONTENT_OR
} xmlElementContentType;

/**
 * xmlElementContentOccur:
 *
 * Possible definitions of element content occurrences.
 */
typedef enum {
    XML_ELEMENT_CONTENT_ONCE = 1,
    XML_ELEMENT_CONTENT_OPT,
    XML_ELEMENT_CONTENT_MULT,
    XML_ELEMENT_CONTENT_PLUS
} xmlElementContentOccur;

/**
 * xmlElementContent:
 *
 * An XML Element content as stored after parsing an element definition
 * in a DTD.
 */

typedef struct _xmlElementContent xmlElementContent;
typedef xmlElementContent *xmlElementContentPtr;
struct _xmlElementContent {
    xmlElementContentType     type;    /* PCDATA, ELEMENT, SEQ or OR */
    xmlElementContentOccur    ocur;    /* ONCE, OPT, MULT or PLUS */
    XCONST xmlChar             *name;    /* Element name */
    struct _xmlElementContent *c1;    /* first child */
    struct _xmlElementContent *c2;    /* second child */
    struct _xmlElementContent *parent;    /* parent */
    XCONST xmlChar             *prefix;    /* Namespace prefix */
};

/**
 * xmlElementTypeVal:
 *
 * The different possibilities for an element content type.
 */

typedef enum {
    XML_ELEMENT_TYPE_UNDEFINED = 0,
    XML_ELEMENT_TYPE_EMPTY = 1,
    XML_ELEMENT_TYPE_ANY,
    XML_ELEMENT_TYPE_MIXED,
    XML_ELEMENT_TYPE_ELEMENT
} xmlElementTypeVal;



#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
extern "C" {
#endif

/**
 * XML_LOCAL_NAMESPACE:
 *
 * A namespace declaration node.
 */
#define XML_LOCAL_NAMESPACE XML_NAMESPACE_DECL
typedef xmlElementType xmlNsType;

typedef struct _XOSxmlDoc xmlDoc;
typedef xmlDoc *xmlDocPtr;

/**
 * xmlNs:
 *
 * An XML namespace.
 * Note that prefix == NULL is valid, it defines the default namespace
 * within the subtree (until overridden).
 *
 * xmlNsType is unified with xmlElementType.
 */

typedef struct _xmlNs xmlNs;
typedef xmlNs *xmlNsPtr;
struct _xmlNs {
    struct _xmlNs  *next;    /* next Ns link for this node  */
    xmlNsType      type;    /* global or local */
    XCONST xmlChar *href;    /* URL for the namespace */
    XCONST xmlChar *prefix;    /* prefix for the namespace */
    void           *_private;   /* application data */
    struct _XOSxmlDoc* context;        /* normally an xmlDoc */
};

/**
 * xmlNode:
 *
 * A node in an XML tree.
 */
typedef struct _XOSxmlNode xmlNode;
typedef xmlNode *xmlNodePtr;
struct _XOSxmlNode {
    void           *_private;    /* application data */
    xmlElementType   type;    /* type number, must be second ! */
    XCONST xmlChar   *name;      /* the name of the node, or the entity */
    struct _XOSxmlNode* children;    /* parent->childs link */
    struct _XOSxmlNode* last;    /* last child link */
    struct _XOSxmlNode* parent;    /* child->parent link */
    struct _XOSxmlNode* next;    /* next sibling link  */
    struct _XOSxmlNode* prev;    /* previous sibling link  */
    struct _XOSxmlDoc*  doc;    /* the containing document */

    /* End of common part */
    xmlNs           *ns;        /* pointer to the associated namespace */
    xmlChar         *content;   /* the content */
    struct _XOSxmlAttr *properties;/* properties list */
    unsigned short   line;    /* line number */
};

/**
 * xmlElement:
 *
 * An XML Element declaration from a DTD.
 */

typedef struct _xmlElement xmlElement;
typedef xmlElement *xmlElementPtr;
struct _xmlElement {
    void           *_private;            /* application data */
    xmlElementType          type;       /* XML_ELEMENT_DECL, must be second ! */
    XCONST xmlChar          *name;    /* Element name */
    struct _XOSxmlNode*    children;    /* NULL */
    struct _XOSxmlNode*        last;    /* NULL */
    struct _xmlDtd       *parent;    /* -> DTD */
    struct _XOSxmlNode*        next;    /* next sibling link  */
    struct _XOSxmlNode*        prev;    /* previous sibling link  */
    struct _XOSxmlDoc*          doc;       /* the containing document */

    xmlElementTypeVal      etype;    /* The type */
    xmlElementContentPtr content;    /* the allowed element content */
    xmlAttributePtr   attributes;    /* List of the declared attributes */
    XCONST xmlChar        *prefix;    /* the namespace prefix if any */
    void          *contModel;
};



/**
 * xmlDtd:
 *
 * An XML DTD, as defined by <!DOCTYPE ... There is actually one for
 * the internal subset and for the external subset.
 */
typedef struct _xmlDtd xmlDtd;
typedef xmlDtd *xmlDtdPtr;
struct _xmlDtd {
    void           *_private;    /* application data */
    xmlElementType  type;       /* XML_DTD_NODE, must be second ! */
    XCONST xmlChar *name;    /* Name of the DTD */
    struct _XOSxmlNode* children;    /* the value of the property link */
    struct _XOSxmlNode* last;    /* last child link */
    struct _XOSxmlNode* parent;    /* child->parent link */
    struct _XOSxmlNode* next;    /* next sibling link  */
    struct _XOSxmlNode* prev;    /* previous sibling link  */
    struct _XOSxmlDoc*  doc;    /* the containing document */

    /* End of common part */
    void          *notations;   /* Hash table for notations if any */
    void          *elements;    /* Hash table for elements if any */
    void          *attributes;  /* Hash table for attributes if any */
    void          *entities;    /* Hash table for entities if any */
    XCONST xmlChar *ExternalID;    /* External identifier for PUBLIC DTD */
    XCONST xmlChar *SystemID;    /* URI for a SYSTEM or PUBLIC DTD */
    void          *pentities;   /* Hash table for param entities if any */
};


/*
* A attribute of an XML node.
*/
typedef struct _XOSxmlAttr xmlAttr;
typedef xmlAttr *xmlAttrPtr;
struct _XOSxmlAttr 
{
#ifndef XML_WITHOUT_CORBA
    XVOID           *_private;    /* for Corba, must be first ! */
    XVOID           *vepv;        /* for Corba, must be next ! */
#endif
    xmlAttributeType etype;
    xmlElementType  type;       /* XML_ATTRIBUTE_NODE, must be third ! */
    struct _XOSxmlNode *node;    /* attr->node link */
    struct _XOSxmlAttr *next;    /* attribute list link */
    XCONST xmlChar   *name;     /* the name of the property */
    struct _XOSxmlNode *val;    /* the value of the property */
    struct _XOSxmlDoc * doc;
    
};

/**
 * xmlID:
 *
 * An XML ID instance.
 */

typedef struct _xmlID xmlID;
typedef xmlID *xmlIDPtr;
struct _xmlID {
    struct _xmlID    *next;    /* next ID */
    XCONST xmlChar    *value;    /* The ID name */
    xmlAttrPtr        attr;    /* The attribute holding it */
    XCONST xmlChar    *name;    /* The attribute if attr is not available */
    int               lineno;    /* The line number if attr is not available */
    struct _XOSxmlDoc*   doc;    /* The document holding the ID */
};

/**
 * xmlRef:
 *
 * An XML IDREF instance.
 */

typedef struct _xmlRef xmlRef;
typedef xmlRef *xmlRefPtr;
struct _xmlRef {
    struct _xmlRef    *next;    /* next Ref */
    XCONST xmlChar     *value;    /* The Ref name */
    xmlAttrPtr        attr;    /* The attribute holding it */
    XCONST xmlChar    *name;    /* The attribute if attr is not available */
    int               lineno;    /* The line number if attr is not available */
};


/**
 * XML_GET_CONTENT:
 *
 * Macro to extract the content pointer of a node.
 */
#define XML_GET_CONTENT(n)                    \
    ((n)->type == XML_ELEMENT_NODE ? NULL : (n)->content)

/**
 * xmlDocProperty
 *
 * Set of properties of the document as found by the parser
 * Some of them are linked to similary named xmlParserOption
 */
typedef enum {
    XML_DOC_WELLFORMED        = 1<<0, /* document is XML well formed */
    XML_DOC_NSVALID        = 1<<1, /* document is Namespace valid */
    XML_DOC_OLD10        = 1<<2, /* parsed with old XML-1.0 parser */
    XML_DOC_DTDVALID        = 1<<3, /* DTD validation was successful */
    XML_DOC_XINCLUDE        = 1<<4, /* XInclude substitution was done */
    XML_DOC_USERBUILT        = 1<<5, /* Document was built using the API
                                           and not by parsing an instance */
    XML_DOC_INTERNAL        = 1<<6, /* built for internal processing */
    XML_DOC_HTML        = 1<<7  /* parsed or built HTML document */
} xmlDocProperties;

/**
 * xmlDoc:
 *
 * An XML document.
 */
struct _XOSxmlDoc {
    void           *_private;    /* application data */
    xmlElementType  type;       /* XML_DOCUMENT_NODE, must be second ! */
    xmlChar           *name;    /* name/filename/URI of the document */
    struct _XOSxmlNode* children;    /* the document tree */
    struct _XOSxmlNode* last;    /* last child link */
    struct _XOSxmlNode* parent;    /* child->parent link */
    struct _XOSxmlNode* next;    /* next sibling link  */
    struct _XOSxmlNode* prev;    /* previous sibling link  */
    struct _XOSxmlDoc*  doc;    /* autoreference to itself */

    /* End of common part */
    int             compression;/* level of zlib compression */
    int             standalone; /* standalone document (no external refs) 
                     1 if standalone="yes"
                     0 if standalone="no"
                    -1 if there is no XML declaration
                    -2 if there is an XML declaration, but no
                    standalone attribute was specified */
    XCONST xmlChar  *version;    /* the XML version string */
    XCONST xmlChar  *encoding;   /* external initial encoding, if any */
    int             charset;    /* encoding of the in-memory content
                   actually an xmlCharEncoding */
    struct _XOSxmlNode *root;    /* the document tree */
};


typedef struct _xmlDOMWrapCtxt xmlDOMWrapCtxt;
typedef xmlDOMWrapCtxt *xmlDOMWrapCtxtPtr;

/**
 * xmlDOMWrapAcquireNsFunction:
 * @ctxt:  a DOM wrapper context
 * @node:  the context node (element or attribute) 
 * @nsName:  the requested namespace name
 * @nsPrefix:  the requested namespace prefix 
 *
 * A function called to acquire namespaces (xmlNs) from the wrapper.
 *
 * Returns an xmlNsPtr or NULL in case of an error.
 */
typedef xmlNsPtr (*xmlDOMWrapAcquireNsFunction) (xmlDOMWrapCtxtPtr ctxt,
                         xmlNodePtr node,
                         XCONST xmlChar *nsName,
                         XCONST xmlChar *nsPrefix);

/**
 * xmlDOMWrapCtxt:
 *
 * Context for DOM wrapper-operations.
 */
struct _xmlDOMWrapCtxt {
    void * _private;
    /*
    * The type of this context, just in case we need specialized
    * contexts in the future.
    */
    int type;
    /*
    * Internal namespace map used for various operations.
    */
    void * namespaceMap;
    /*
    * Use this one to acquire an xmlNsPtr intended for node->ns.
    * (Note that this is not intended for elem->nsDef).
    */
    xmlDOMWrapAcquireNsFunction getNsForNodeFunc;
};

/**
 * xmlChildrenNode:
 *
 * Macro for compatibility naming layer with libxml1. Maps
 * to "children."
 */
#ifndef xmlChildrenNode
#define xmlChildrenNode children
#endif

/**
 * xmlRootNode:
 *
 * Macro for compatibility naming layer with libxml1. Maps 
 * to "children".
 */
#ifndef xmlRootNode
#define xmlRootNode children
#endif

XCONST xmlChar * xmlSplitQName3        (XCONST xmlChar *name,
                     int *len);

/*
 * Handling Buffers.
 */

xmlBufferPtr xmlBufferCreate    (void);
xmlBufferPtr xmlBufferCreateSize    (size_t size);

int xmlBufferResize        (xmlBufferPtr buf, unsigned int size);
void xmlBufferFree        (xmlBufferPtr buf);

int xmlBufferDump        (FILE *file,xmlBufferPtr buf);

XVOID xmlBufferAdd(xmlBufferPtr buf, XCONST xmlChar *str, XS32 len);
int xmlBufferAddHead    (xmlBufferPtr buf, XCONST xmlChar *str, int len);
int xmlBufferCat        (xmlBufferPtr buf,     XCONST xmlChar *str);
int xmlBufferCCat        (xmlBufferPtr buf, XCONST xmlChar *str);

int    xmlBufferShrink        (xmlBufferPtr buf, unsigned int len);

int xmlBufferGrow        (xmlBufferPtr buf,
                     unsigned int len);
void xmlBufferEmpty        (xmlBufferPtr buf);

XCONST xmlChar* xmlBufferContent    (XCONST xmlBufferPtr buf);

void xmlBufferSetAllocationScheme(xmlBufferPtr buf, xmlBufferAllocationScheme scheme);
int xmlBufferLength        (XCONST xmlBufferPtr buf);

/*
 * Creating/freeing new structures.
 */
xmlDtdPtr xmlCreateIntSubset    (xmlDocPtr doc,
                     XCONST xmlChar *name,
                     XCONST xmlChar *ExternalID,
                     XCONST xmlChar *SystemID);
xmlNsPtr xmlNewNs        (xmlNodePtr node,
                     XCONST xmlChar *href,
                     XCONST xmlChar *prefix);
void xmlFreeNs        (xmlNsPtr cur);

//need
xmlDocPtr xmlNewDoc        (XCONST xmlChar *version);
//need
XVOID xmlFreeDoc(xmlDocPtr cur);

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_HTML_ENABLED) || \
    defined(LIBXML_SCHEMAS_ENABLED)
    
xmlAttrPtr xmlNewProp        (xmlNodePtr node,
                     XCONST xmlChar *name,
                     XCONST xmlChar *value);
#endif

xmlAttrPtr xmlNewNsProp(xmlNodePtr node, 
                        XCONST xmlChar *name,
                        XCONST xmlChar *value);

void xmlFreePropList        (xmlAttrPtr cur);
void xmlFreeProp        (xmlAttrPtr cur);


/*
 * Creating new nodes.
 */
 //need
xmlNodePtr xmlNewDocNode(xmlDocPtr doc, 
                         XCONST xmlChar *name, 
                         XCONST xmlChar *content);

//need
xmlNodePtr xmlNewNode( XCONST xmlChar *name,xmlElementType type);

//need
xmlNodePtr xmlNewDocText    (xmlDocPtr doc, XCONST xmlChar *content);

xmlNodePtr xmlNewText        (XCONST xmlChar *content);

xmlNodePtr xmlNewDocTextLen(xmlDocPtr doc, XCONST xmlChar *content, XS32 len);
xmlNodePtr xmlNewTextLen(XCONST xmlChar *content, XS32 len);

xmlNodePtr xmlNewDocComment(xmlDocPtr doc, XCONST xmlChar *content);
xmlNodePtr xmlNewComment(XCONST xmlChar *content);

/*
 * Navigating.
 */
long xmlGetLineNo        (xmlNodePtr node);


//need
xmlNodePtr xmlDocGetRootElement(xmlDocPtr doc);
xmlNodePtr xmlGetLastChild(xmlNodePtr parent);

XS32 xmlNodeIsText(xmlNodePtr node);
XS32 xmlIsBlankNode(xmlNodePtr node);

/*
 * Changing the structure.
 */
 //need
xmlNodePtr xmlDocSetRootElement    (xmlDocPtr doc, xmlNodePtr root);

//need
xmlNodePtr xmlAddChild(xmlNodePtr parent, xmlNodePtr cur);

//need
xmlNodePtr xmlReplaceNode        (xmlNodePtr old,
                     xmlNodePtr cur);

//need
xmlNodePtr xmlAddSibling(xmlNodePtr cur, xmlNodePtr elem);

//need
XVOID xmlUnlinkNode(xmlNodePtr cur);

xmlNodePtr xmlTextMerge(xmlNodePtr first, xmlNodePtr second);

XVOID xmlTextConcat(xmlNodePtr node, XCONST xmlChar *content, XS32 len);
//need
XVOID xmlFreeNodeList(xmlNodePtr cur);
XVOID xmlFreeNode(xmlNodePtr cur);

//need
void xmlSetTreeDoc        (xmlNodePtr tree, xmlDocPtr doc);
//need
void xmlSetListDoc        (xmlNodePtr list, xmlDocPtr doc);

/*
 * Changing the content.
 */
//need 
xmlAttrPtr xmlSetProp    (xmlNodePtr node, XCONST xmlChar *name, XCONST xmlChar *value);
//need
xmlAttrPtr xmlSetNsProp        (xmlNodePtr node,
                     xmlNsPtr ns,
                     XCONST xmlChar *name,
                     XCONST xmlChar *value);

xmlChar * xmlGetProp(xmlNodePtr node, XCONST xmlChar *name);

xmlAttrPtr xmlHasNsProp        (xmlNodePtr node,
                     XCONST xmlChar *name,
                     XCONST xmlChar *nameSpace);
//need
xmlNodePtr xmlStringGetNodeList    (xmlDocPtr doc,     XCONST xmlChar *value);

xmlChar *xmlNodeListGetString(xmlDocPtr doc, xmlNodePtr list, XS32 inLine);

//need
void xmlNodeSetContent    (xmlNodePtr cur, XCONST xmlChar *content);


XVOID xmlNodeAddContent(xmlNodePtr cur, XCONST xmlChar *content);

XVOID xmlNodeAddContentLen(xmlNodePtr cur, XCONST xmlChar *content, XS32 len);

/*
 * Removing content.
 */
int xmlRemoveProp        (xmlAttrPtr cur);

/*
 * Saving.
 */
int xmlDocFormatDump    (FILE *f,
                     xmlDocPtr cur,
                     int format);

int xmlSaveFormatFileEnc    (XCONST xmlChar *filename,
                     xmlDocPtr cur,
                     XCONST xmlChar *encoding,
                     int format);
void xmlBufferWriteQuotedString(xmlBufferPtr buf, XCONST xmlChar *string);


void xmlBufferWriteCHAR(xmlBufferPtr buf, XCONST xmlChar *string);
void xmlBufferWriteChar(xmlBufferPtr buf, XCONST xmlChar *string);
void xmlBufferWriteQuotedString(xmlBufferPtr buf, XCONST xmlChar *string);

#ifdef __cplusplus
}
#endif


#endif /* __XML_TREE_H__ */

