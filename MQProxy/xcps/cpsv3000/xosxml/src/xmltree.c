/*
 * tree.c : implementation of access function for an XML tree.
 *
 * References:
 *   XHTML 1.0 W3C REC: http://www.w3.org/TR/2002/REC-xhtml1-20020801/
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 *
 */

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "../inc/xmltree.h"
#include "../inc/xmlparser.h"
#include "../inc/xmlentities.h"
#include "../inc/xmlerror.h"

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

int __xmlRegisterCallbacks = 0;

/************************************************************************
 *                                    *
 *        Forward declarations                    *
 *                                    *
 ************************************************************************/
/*
static int xmlParseWcToMb(XCONST xmlChar* encoding,
        XCONST xmlChar* in,
        int *inlen,
        xmlChar* out,
        int *outlen);
*/

static xmlBufferAllocationScheme xmlBufferAllocScheme = XML_BUFFER_ALLOC_EXACT;
/************************************************************************
 *                                    *
 *        A few static variables and macros            *
 *                                    *
 ************************************************************************/
/* #undef xmlStringText */
static XCONST xmlChar xmlStringText[] = { 't', 'e', 'x', 't', 0 };
/* #undef xmlStringTextNoenc */
static XCONST xmlChar xmlStringTextNoenc[] =
              { 't', 'e', 'x', 't', 'n', 'o', 'e', 'n', 'c', 0 };
/* #undef xmlStringComment */
XCONST xmlChar xmlStringComment[] = { 'c', 'o', 'm', 'm', 'e', 'n', 't', 0 };

static int xmlCheckDTD = 1;

#define UPDATE_LAST_CHILD_AND_PARENT(n) if ((n) != NULL) {        \
    xmlNodePtr ulccur = (n)->children;                    \
    if (ulccur == NULL) {                        \
        (n)->last = NULL;                        \
    } else {                                \
        while (ulccur->next != NULL) {                    \
        ulccur->parent = (n);                    \
        ulccur = ulccur->next;                    \
    }                                \
    ulccur->parent = (n);                        \
    (n)->last = ulccur;                        \
}}

#define IS_STR_XML(str) ((str != NULL) && (str[0] == 'x') && \
  (str[1] == 'm') && (str[2] == 'l') && (str[3] == 0))

#define IS_BLANK(c) (((c) == 0x20) || ((c) == 0x09) || ((c) == 0xa) ||    \
((c) == 0x0D))

/**
 * xmlSplitQName3:
 * @name:  the full QName
 * @len: an int *
 *
 * parse an XML qualified name string,i
 *
 * returns NULL if it is not a Qualified Name, otherwise, update len
 *         with the lenght in byte of the prefix and return a pointer
 *         to the start of the name without the prefix
 */

XCONST xmlChar *
xmlSplitQName3(XCONST xmlChar *name, int *len) {
    int l = 0;

    if (name == NULL) return(NULL);
    if (len == NULL) return(NULL);

    /* nasty but valid */
    if (name[0] == ':')
        return(NULL);

    /*
     * we are not trying to validate but just to cut, and yes it will
     * work even if this is as set of UTF-8 encoded chars
     */
    while ((name[l] != 0) && (name[l] != ':'))
        l++;

    if (name[l] == 0)
        return(NULL);

    *len = l;

    return(&name[l+1]);
}

/************************************************************************
 *                                    *
 *        Check Name, NCName and QName strings            *
 *                                    *
 ************************************************************************/

#define CUR_SCHAR(s, l) xmlStringCurrentChar(NULL, s, &l)


/************************************************************************
 *                                    *
 *        Allocation and deallocation of basic structures        *
 *                                    *
 ************************************************************************/

/**
 * xmlNewNs:
 * @node:  the element carrying the namespace
 * @href:  the URI associated
 * @prefix:  the prefix for the namespace
 *
 * Creation of a new Namespace. This function will refuse to create
 * a namespace with a similar prefix than an existing one present on this
 * node.
 * We use href==NULL in the case of an element creation where the namespace
 * was not defined.
 * Returns a new namespace pointer or NULL
 */
xmlNsPtr
xmlNewNs(xmlNodePtr node, XCONST xmlChar *href, XCONST xmlChar *prefix) {
    xmlNsPtr cur;

    if ((node != NULL) && (node->type != XML_ELEMENT_NODE))
        return(NULL);

    if ((prefix != NULL) && (xmlStrEqual(prefix, BAD_CAST "xml"))) {
        /* xml namespace is predefined, no need to add it */
        if (xmlStrEqual(href, XML_XML_NAMESPACE))
            return(NULL);

        /*
         * Problem, this is an attempt to bind xml prefix to a wrong
         * namespace, which breaks
         * Namespace constraint: Reserved Prefixes and Namespace Names
         * from XML namespace. But documents authors may not care in
         * their context so let's proceed.
         */
    }

    /*
     * Allocate a new Namespace and fill the fields.
     */
    cur = (xmlNsPtr) xmlMalloc(sizeof(xmlNs));
    if (cur == NULL) {
        return(NULL);
    }
    XOS_MemSet(cur, 0, sizeof(xmlNs));
    cur->type = XML_LOCAL_NAMESPACE;

    if (href != NULL)
        cur->href = xmlStrdup(href);
    if (prefix != NULL)
        cur->prefix = xmlStrdup(prefix);

    
    return(cur);
}


/**
 * xmlFreeNs:
 * @cur:  the namespace pointer
 *
 * Free up the structures associated to a namespace
 */
void xmlFreeNs(xmlNsPtr cur) {
    if (cur == NULL) {
        return;
    }
    if (cur->href != NULL) xmlFree((char *) cur->href);
    if (cur->prefix != NULL) xmlFree((char *) cur->prefix);
    xmlFree(cur);
}


/**
 * DICT_COPY:
 * @str:  a string
 *
 * Copy a string using a "dict" dictionnary in the current scope,
 * if availabe.
 */
#define DICT_COPY(str, cpy) \
    if (str) { \
    if (dict) { \
        if (xmlDictOwns(dict, (XCONST xmlChar *)(str))) \
        cpy = (xmlChar *) (str); \
        else \
        cpy = (xmlChar *) xmlDictLookup((dict), (XCONST xmlChar *)(str), -1); \
    } else \
        cpy = xmlStrdup((XCONST xmlChar *)(str)); }

/**
 * DICT_CONST_COPY:
 * @str:  a string
 *
 * Copy a string using a "dict" dictionnary in the current scope,
 * if availabe.
 */
#define DICT_CONST_COPY(str, cpy) \
    if (str) { \
    if (dict) { \
        if (xmlDictOwns(dict, (XCONST xmlChar *)(str))) \
        cpy = (XCONST xmlChar *) (str); \
        else \
        cpy = xmlDictLookup((dict), (XCONST xmlChar *)(str), -1); \
    } else \
        cpy = (XCONST xmlChar *) xmlStrdup((XCONST xmlChar *)(str)); }


/**
* xmlNewDoc:
* @version:  xmlChar string giving the version of XML "1.0"
*
* Returns a new document
*/
xmlDocPtr xmlNewDoc(XCONST xmlChar *version) 
{
    xmlDocPtr cur;
    
    if (version == XNULL) 
    {
        return(XNULL);
    }
    
    /*
    * Allocate a new document and fill the fields.
    */
    cur = (xmlDocPtr) xmlMalloc(sizeof(xmlDoc));
    if (cur == XNULL) 
    {
        return(XNULL);
    }
    
    cur->type = XML_DOCUMENT_NODE;
    cur->version = xmlStrdup(version); 
    cur->name = XNULL;
    cur->root = XNULL; 
    cur->children = XNULL;
    cur->encoding = XNULL;
    cur->standalone = -1;
    cur->compression = -1; /* not initialized */

    cur->charset = XML_CHAR_ENCODING_UTF8;
    
#ifndef XML_WITHOUT_CORBA
    cur->_private = XNULL;
#endif
    return(cur);
}

/**
* xmlFreeDoc:
* @cur:  pointer to the document
* @:  
*
* Free up all the structures used by a document, tree included.
*/
XVOID xmlFreeDoc(xmlDocPtr cur) 
{
    if (cur == XNULL) 
    {
        return;
    }
    if (cur->version != XNULL) 
    {
        xmlFree((XS8 *) cur->version);
    }
    if (cur->name != XNULL)
    {
        xmlFree((XS8 *) cur->name);
    }
    if (cur->encoding != XNULL) 
    {
        xmlFree((XS8 *) cur->encoding);
    }
    if (cur->root != XNULL) 
    {
        xmlFreeNodeList(cur->root);
    }
    XOS_MemSet(cur, -1, sizeof(xmlDoc));
    xmlFree(cur);
}

/**
* xmlStringGetNodeList:
* @doc:  the document
* @value:  the value of the attribute
*
* Parse the value string and build the node list associated. Should
* produce a flat tree with only TEXTs and ENTITY_REFs.
* Returns a pointer to the first child
*/
xmlNodePtr xmlStringGetNodeList(xmlDocPtr doc, XCONST xmlChar *value) 
{
    xmlNodePtr ret  = XNULL, last = XNULL;
    xmlNodePtr node = XNULL;
    XCONST xmlChar *cur = value;
    XCONST xmlChar *q   = XNULL;
    
    if (value == XNULL) 
        return(XNULL);
    
    q = cur;
    while (*cur != 0) 
    {
        cur++;
    }
    
    if (cur != q) 
    {
    /*
    * Handle the last piece of text.
    */
        if ((last != XNULL) && (last->type == XML_TEXT_NODE)) 
        {
            xmlNodeAddContentLen(last, q, (XS32)(cur - q));
        } 
        else 
        {
            node = xmlNewDocTextLen(doc, q, (XS32)(cur - q));
            if (node == XNULL) 
                return(ret);
            
            if (last == XNULL)
            {
                last = ret = node;
            }
            else 
            {
                last->next = node;
                node->prev = last;
                last = node;
            }
        }
    }
    return(ret);
}

/**
 * xmlNodeListGetString:
 * @doc:  the document
 * @list:  a Node list
 * @inLine:  should we replace entity contents or show their external form
 *
 * Build the string equivalent to the text contained in the Node list
 * made of TEXTs and ENTITY_REFs
 *
 * Returns a pointer to the string copy, the caller must free it with xmlFree().
 */
xmlChar *xmlNodeListGetString(xmlDocPtr doc, xmlNodePtr list, XS32 inLine)
{
    xmlNodePtr node = list;
    xmlChar *ret = XNULL;
    
    if ( XNULL == doc )
    {
    }
    
    if (list == XNULL)
    {
        return(XNULL);
    }
    
    while (node != XNULL) 
    {
        if ((node->type == XML_TEXT_NODE) 
            || (node->type == XML_CDATA_SECTION_NODE)) 
        {
            if (inLine) 
            {
#ifndef XML_USE_BUFFER_CONTENT
                ret = xmlStrcat(ret, node->content);
#else
                ret = xmlStrcat(ret, xmlBufferContent(node->content));
#endif
            } 
            
        }
        
        node = node->next;
    }
    return(ret);
}

static xmlAttrPtr
xmlNewPropInternal(xmlNodePtr node, xmlNsPtr ns,
                   XCONST xmlChar * name, XCONST xmlChar * value,
                   int eatname)
{
    xmlAttrPtr cur;
    xmlDocPtr doc = NULL;

    if ((node != NULL) && (node->type != XML_ELEMENT_NODE)) {
        return (NULL);
    }

    /*
     * Allocate a new property and fill the fields.
     */
    cur = (xmlAttrPtr) xmlMalloc(sizeof(xmlAttr));
    if (cur == NULL) {
        return (NULL);
    }
    
    memset(cur, 0, sizeof(xmlAttr));
    cur->type = XML_ATTRIBUTE_NODE;

    if (node != NULL) {
        doc = node->doc;
        cur->doc = doc;
    }

    if (eatname == 0)
        cur->name = xmlStrdup(name);
    else
        cur->name = name;

    if (value != NULL) {
        xmlNodePtr tmp;
        cur->val = xmlNewDocText(doc, value);
        tmp = cur->val;
        while (tmp != NULL) {
            tmp->parent = (xmlNodePtr) cur;
            tmp = tmp->next;
        }
    }

    /*
     * Add it at the end to preserve parsing order ...
     */
    if (node != NULL) {
        if (node->properties == NULL) {
            node->properties = cur;
        } else {
            xmlAttrPtr prev = node->properties;

            while (prev->next != NULL)
                prev = prev->next;
            prev->next = cur;
        }
    }
    return (cur);
}

/**
 * xmlNewProp:
 * @node:  the holding node
 * @name:  the name of the attribute
 * @value:  the value of the attribute
 *
 * Create a new property carried by a node.
 * Returns a pointer to the attribute
 */
xmlAttrPtr
xmlNewProp(xmlNodePtr node, XCONST xmlChar *name, XCONST xmlChar *value) {

    if (name == NULL) {
        return(NULL);
    }

    return xmlNewPropInternal(node, NULL, name, value, 0);
}

/**
* xmlNewNsProp:
* @node:  the holding node
* @ns:  the namespace
* @name:  the name of the attribute
* @value:  the value of the attribute
*
* Create a new property tagged with a namespace and carried by a node.
* Returns a pointer to the attribute
*/
xmlAttrPtr xmlNewNsProp(xmlNodePtr node, 
                        XCONST xmlChar *name,
                        XCONST xmlChar *value) 
{
    xmlAttrPtr cur = XNULL; 
    
    if (name == XNULL) 
    {
        return(XNULL);
    }    
    /*
    * Allocate a new property and fill the fields.
    */
    cur = (xmlAttrPtr) xmlMalloc(sizeof(xmlAttr));
    if (cur == XNULL) 
    {
        return(XNULL);
    }    
    cur->type = XML_ATTRIBUTE_NODE;
    cur->node = node; 
    cur->name = xmlStrdup(name);
    if (value != XNULL)
    {
        cur->val = xmlStringGetNodeList(node->doc, value);
    }
    else 
    {
        cur->val = XNULL;
    }
    
#ifndef XML_WITHOUT_CORBA
    cur->_private = XNULL;
    cur->vepv = XNULL;
#endif
    
    /*
    * Add it at the end to preserve parsing order ...
    */
    cur->next = XNULL;
    if (node != XNULL) 
    {
        if (node->properties == XNULL) 
        {
            node->properties = cur;
        } 
        else 
        {
            xmlAttrPtr prev = node->properties;
            
            while (prev->next != XNULL)
            {
                prev = prev->next;
            }
            prev->next = cur;
        }
    }
    return(cur);
}

/**
 * xmlFreePropList:
 * @cur:  the first property in the list
 *
 * Free a property and all its siblings, all the children are freed too.
 */
void xmlFreePropList(xmlAttrPtr cur)
{
    xmlAttrPtr next;
    if (cur == NULL) return;
    while (cur != NULL) {
        next = cur->next;
        xmlFreeProp(cur);
        cur = next;
    }
}

/**
 * xmlFreeProp:
 * @cur:  an attribute
 *
 * Free one attribute, all the content is freed too
 */
void xmlFreeProp(xmlAttrPtr cur) {
    if (cur == XNULL) 
    {
        return;
    }
    
    if (cur->name != XNULL) 
        xmlFree((XS8 *) cur->name);
    
    if (cur->val != XNULL) 
        xmlFreeNodeList(cur->val);
    
    XOS_MemSet(cur, -1, sizeof(xmlAttr));
    xmlFree(cur);
}

/**
 * xmlRemoveProp:
 * @cur:  an attribute
 *
 * Unlink and free one attribute, all the content is freed too
 * Note this doesn't work for namespace definition attributes
 *
 * Returns 0 if success and -1 in case of error.
 */
int
xmlRemoveProp(xmlAttrPtr cur) {
    xmlAttrPtr tmp;
    if (cur == NULL) {
        return(-1);
    }

    tmp = (xmlAttrPtr)cur->val;

    while (tmp != NULL) {
        if (tmp->next == cur) {
            tmp->next = cur->next;
            xmlFreeProp(cur);
            return(0);
        }
        tmp = tmp->next;
    }

    return(-1);
}

/**
 * xmlNewNode:
 * @ns:  namespace if any
 * @name:  the node name
 * @type:  the node type
 * Creation of a new node element. @ns is optional (NULL).
 *
 * Returns a pointer to the new node object. Uses xmlStrdup() to make
 * copy of @name.
 */
xmlNodePtr xmlNewNode( XCONST xmlChar *name,xmlElementType type)
{
    xmlNodePtr cur;
    
    if (name == XNULL) 
    {
        return(XNULL);
    }
#ifdef LIBXML_DOCB_ENABLED
    if (type < XML_ELEMENT_NODE || type > XML_DOCB_DOCUMENT_NODE)
    {
        return XNULL;
    }
#else
    if (type < XML_ELEMENT_NODE || type > XML_XINCLUDE_END)
    {
        return XNULL;
    }
#endif
    /*
    * Allocate a new node and fill the fields.
    */
    cur = (xmlNodePtr) xmlMalloc(sizeof(xmlNode));
    if (cur == XNULL) 
    {
        return(XNULL);
    }
    
    cur->type = type;
    cur->doc = XNULL;
    cur->parent = XNULL; 
    cur->next = XNULL;
    cur->prev = XNULL;
    cur->children = NULL;
    cur->last = XNULL;
    cur->properties = XNULL;
    cur->ns = XNULL;
    cur->name = xmlStrdup(name);
    
    cur->content = XNULL;
    
#ifndef XML_WITHOUT_CORBA
    cur->_private = XNULL;
#endif
    return(cur);
}

/**
 * xmlNewDocNode:
 * @doc:  the document
 * @ns:  namespace if any
 * @name:  the node name
 * @content:  the XML text content if any
 *
 * Creation of a new node element within a document. @ns and @content
 * are optional (NULL).
 * NOTE: @content is supposed to be a piece of XML CDATA, so it allow entities
 *       references, but XML special chars need to be escaped first by using
 *       xmlEncodeEntitiesReentrant(). Use xmlNewDocRawNode() if you don't
 *       need entities support.
 *
 * Returns a pointer to the new node object.
 */
xmlNodePtr xmlNewDocNode(xmlDocPtr doc, 
                         XCONST xmlChar *name, 
                         XCONST xmlChar *content)
{
    xmlNodePtr cur;
    
    cur = xmlNewNode( name,XML_ELEMENT_NODE);
    if (cur != XNULL) 
    {
        cur->doc = doc;
        if (content != XNULL) 
        {
            cur->children = xmlStringGetNodeList(doc, content);
            UPDATE_LAST_CHILD_AND_PARENT(cur)
        }
    }
    return(cur);
}

/**
 * xmlNewText:
 * @content:  the text content
 *
 * Creation of a new text node.
 * Returns a pointer to the new node object.
 */
xmlNodePtr xmlNewText(XCONST xmlChar *content)
{
    xmlNodePtr cur = XNULL;
    
    /*
    * Allocate a new node and fill the fields.
    */
    cur = (xmlNodePtr)xmlMalloc(sizeof(xmlNode));
    if (cur == XNULL) 
    {
        return(XNULL);
    }
    
    cur->type   = XML_TEXT_NODE;
    cur->doc    = XNULL;
    cur->parent = XNULL; 
    cur->next   = XNULL; 
    cur->prev   = XNULL; 
    cur->children = XNULL; 
    cur->last   = XNULL; 
    cur->properties = XNULL; 
    cur->type   = XML_TEXT_NODE;
    cur->ns = XNULL;
    cur->name   = xmlStrdup(xmlStringText);
    
    if (content != XNULL) 
    {
#ifndef XML_USE_BUFFER_CONTENT
        cur->content = xmlStrdup(content);
#else
        cur->content = xmlBufferCreateSize(0);
        xmlBufferSetAllocationScheme(cur->content,
            xmlGetBufferAllocationScheme());
        xmlBufferAdd(cur->content, content, -1);
#endif
    } 
    else 
        cur->content = XNULL;
#ifndef XML_WITHOUT_CORBA
    cur->_private = XNULL;
#endif    
    return(cur);
}

/**
 * xmlNewDocText:
 * @doc: the document
 * @content:  the text content
 *
 * Creation of a new text node within a document.
 * Returns a pointer to the new node object.
 */
xmlNodePtr xmlNewDocText    (xmlDocPtr doc, XCONST xmlChar *content)
{
    xmlNodePtr cur;

    cur = xmlNewText(content);
    if (cur != NULL) cur->doc = doc;
    return(cur);
}

/**
 * xmlNewTextLen:
 * @content:  the text content
 * @len:  the text len.
 *
 * Creation of a new text node with an extra parameter for the content's length
 * Returns a pointer to the new node object.
 */
xmlNodePtr xmlNewTextLen(XCONST xmlChar *content, XS32 len) 
{
    xmlNodePtr cur = XNULL;
    
    /*
    * Allocate a new node and fill the fields.
    */
    cur = (xmlNodePtr) xmlMalloc(sizeof(xmlNode));
    if (cur == XNULL) 
    {
        return(XNULL);
    }
    
    cur->type   = XML_TEXT_NODE;
    cur->doc    = XNULL; 
    cur->parent = XNULL; 
    cur->prev   = XNULL; 
    cur->next   = XNULL; 
    cur->children = XNULL; 
    cur->last   = XNULL; 
    cur->properties = XNULL; 
    cur->name   = xmlStrdup(xmlStringText);
    
    if (content != XNULL) 
    {
#ifndef XML_USE_BUFFER_CONTENT
        cur->content = xmlStrndup(content, len);
#else
        cur->content = xmlBufferCreateSize(len);
        xmlBufferSetAllocationScheme(cur->content,xmlGetBufferAllocationScheme());
        xmlBufferAdd(cur->content, content, len);
#endif
    } 
    else 
        cur->content = XNULL;
#ifndef XML_WITHOUT_CORBA
    cur->_private = XNULL;
#endif    
    return(cur);
}

/**
 * xmlNewDocTextLen:
 * @doc: the document
 * @content:  the text content
 * @len:  the text len.
 *
 * Creation of a new text node with an extra content length parameter. The
 * text node pertain to a given document.
 * Returns a pointer to the new node object.
 */
xmlNodePtr xmlNewDocTextLen(xmlDocPtr doc, XCONST xmlChar *content, XS32 len)
{
    xmlNodePtr cur = XNULL;
    
    cur = xmlNewTextLen(content, len);
    if (cur != XNULL) 
        cur->doc = doc;
    return(cur);
}

/**
 * xmlNewComment:
 * @content:  the comment content
 *
 * Creation of a new node containing a comment.
 * Returns a pointer to the new node object.
 */
xmlNodePtr xmlNewComment(XCONST xmlChar *content)
{
    xmlNodePtr cur = XNULL;
    
    /*
    * Allocate a new node and fill the fields.
    */
    cur = (xmlNodePtr) xmlMalloc(sizeof(xmlNode));
    if (cur == XNULL) 
    {
        return(XNULL);
    }
    
    cur->type = XML_COMMENT_NODE;
    cur->doc = XNULL; 
    cur->parent = XNULL; 
    cur->prev = XNULL; 
    cur->next = XNULL; 
    cur->children = XNULL; 
    cur->last = XNULL; 
    cur->properties = XNULL; 
    cur->ns = XNULL;
    cur->type = XML_COMMENT_NODE;
    cur->name = xmlStrdup(xmlStringText);
    
    if (content != XNULL) 
    {
#ifndef XML_USE_BUFFER_CONTENT
        cur->content = xmlStrdup(content);
#else
        cur->content = xmlBufferCreateSize(0);
        xmlBufferSetAllocationScheme(cur->content,
            xmlGetBufferAllocationScheme());
        xmlBufferAdd(cur->content, content, -1);
#endif
    } 
    else 
        cur->content = XNULL;
#ifndef XML_WITHOUT_CORBA
    cur->_private = XNULL;
#endif    
    return(cur);
}

/**
 * xmlNewDocComment:
 * @doc:  the document
 * @content:  the comment content
 *
 * Creation of a new node containing a comment within a document.
 * Returns a pointer to the new node object.
 */
xmlNodePtr xmlNewDocComment(xmlDocPtr doc, XCONST xmlChar *content)
{
    xmlNodePtr cur = XNULL;
    
    cur = xmlNewComment(content);
    if (cur != XNULL) 
        cur->doc = doc;
    return(cur);
}

/**
 * xmlSetTreeDoc:
 * @tree:  the top element
 * @doc:  the document
 *
 * update all nodes under the tree to point to the right document
 */
void
xmlSetTreeDoc(xmlNodePtr tree, xmlDocPtr doc) {
    xmlAttrPtr prop;

    if (tree == NULL)
        return;
    if (tree->doc != doc) {
        if(tree->type == XML_ELEMENT_NODE) {
            prop = tree->properties;
            while (prop != NULL) {
                prop->doc = doc;
                xmlSetListDoc(prop->val, doc);
                prop = prop->next;
            }
        }
        if (tree->children != NULL)
            xmlSetListDoc(tree->children, doc);
        tree->doc = doc;
    }
}

/**
 * xmlSetListDoc:
 * @list:  the first element
 * @doc:  the document
 *
 * update all nodes in the list to point to the right document
 */
void
xmlSetListDoc(xmlNodePtr list, xmlDocPtr doc) {
    xmlNodePtr cur;

    if (list == NULL)
        return;
    cur = list;
    while (cur != NULL) {
        if (cur->doc != doc)
            xmlSetTreeDoc(cur, doc);
        cur = cur->next;
    }
}

/**
 * xmlAddSibling:
 * @cur:  the child node
 * @elem:  the new node
 *
 * Add a new element @elem to the list of siblings of @cur
 * merging adjacent TEXT nodes (@elem may be freed)
 * If the new element was already inserted in a document it is
 * first unlinked from its existing context.
 *
 * Returns the new element or NULL in case of error.
 */
xmlNodePtr xmlAddSibling(xmlNodePtr cur, xmlNodePtr elem)
{
    xmlNodePtr parent = XNULL;
    
    if (cur == XNULL) 
    {
        return(XNULL);
    }
    
    if (elem == XNULL) 
    {
        return(XNULL);
    }
    
    /*
    * Constant time is we can rely on the ->parent->last to find
    * the last sibling.
    */
    if ((cur->parent != XNULL) && 
        (cur->parent->children != XNULL) &&
        (cur->parent->last != XNULL) &&
        (cur->parent->last->next == XNULL)) 
    {
        cur = cur->parent->last;
    } 
    else 
    {
        while (cur->next != XNULL) cur = cur->next;
    }
    
    xmlUnlinkNode(elem);
    if (elem->doc == XNULL)
        elem->doc = cur->doc; /* the parent may not be linked to a doc ! */
    
    parent = cur->parent;
    elem->prev = cur;
    elem->next = XNULL;
    elem->parent = parent;
    cur->next = elem;
    
    if (parent != XNULL)
        parent->last = elem;
    
    return(elem);
}

/**
 * xmlAddChild:
 * @parent:  the parent node
 * @cur:  the child node
 *
 * Add a new node to @parent, at the end of the child (or property) list
 * merging adjacent TEXT nodes (in which case @cur is freed)
 * If the new node is ATTRIBUTE, it is added into properties instead of children.
 * If there is an attribute with equal name, it is first destroyed.
 *
 * Returns the child or NULL in case of error.
 */
xmlNodePtr xmlAddChild(xmlNodePtr parent, xmlNodePtr cur)
{
    xmlNodePtr prev;
    
    if (parent == XNULL) 
    {
        return(XNULL);
    }
    
    if (cur == XNULL) 
    {
        return(XNULL);
    }
    
   
    /*
    * add the new element at the end of the childs list.
    */
    cur->parent = parent;
    cur->doc = parent->doc; /* the parent may not be linked to a doc ! */
    
                            /*
                            * Handle the case where parent->content != XNULL, in that case it will
                            * create a intermediate TEXT node.
    */
    if (parent->content != XNULL) 
    {
        xmlNodePtr text;
        
#ifndef XML_USE_BUFFER_CONTENT
        text = xmlNewDocText(parent->doc, parent->content);
#else
        text = xmlNewDocText(parent->doc, xmlBufferContent(parent->content));
#endif
        if (text != XNULL) 
        {
            text->next = parent->children;
            if (text->next != XNULL)
                text->next->prev = text;
            parent->children = text;
            UPDATE_LAST_CHILD_AND_PARENT(parent)
                
#ifndef XML_USE_BUFFER_CONTENT
            xmlFree(parent->content);
#else
            xmlBufferFree(parent->content);
#endif
            parent->content = XNULL;
        }
    }
    if (parent->children== XNULL) 
    {
        parent->children= cur;
        parent->last = cur;
    } 
    else 
    {
        prev = parent->last;
        prev->next = cur;
        cur->prev = prev;
        parent->last = cur;
    }
    
    return(cur);
}

/**
 * xmlGetLastChild:
 * @parent:  the parent node
 *
 * Search the last child of a node.
 * Returns the last child or NULL if none.
 */
xmlNodePtr xmlGetLastChild(xmlNodePtr parent)
{
    if (parent == XNULL) 
    {
        return(XNULL);
    }
    return(parent->last);
}

/**
 * xmlFreeNodeList:
 * @cur:  the first node in the list
 *
 * Free a node and all its siblings, this is a recursive behaviour, all
 * the children are freed too.
 */
XVOID xmlFreeNodeList(xmlNodePtr cur)
{
    xmlNodePtr next;
    if (cur == XNULL) 
    {
        return;
    }
    while (cur != XNULL) 
    {
        next = cur->next;
        xmlFreeNode(cur);
        cur = next;
    }
}

/**
 * xmlFreeNode:
 * @cur:  the node
 *
 * Free a node, this is a recursive behaviour, all the children are freed too.
 * This doesn't unlink the child from the list, use xmlUnlinkNode() first.
 */
XVOID xmlFreeNode(xmlNodePtr cur)
{
    if (cur == XNULL) 
    {
        return;
    }
    cur->doc = XNULL;
    cur->parent = XNULL;
    cur->next = XNULL;
    cur->prev = XNULL;
    
    if (cur->children != XNULL) 
        xmlFreeNodeList(cur->children);
    if (cur->properties != XNULL) 
        xmlFreePropList(cur->properties);
    
    if (cur->type != XML_ENTITY_REF_NODE)
#ifndef XML_USE_BUFFER_CONTENT
        if (cur->content != XNULL) 
            xmlFree(cur->content);
#else
        if (cur->content != XNULL) 
            xmlBufferFree(cur->content);
#endif
        
        if (cur->name != XNULL) 
            xmlFree((XS8 *) cur->name);
        
        XOS_MemSet(cur, -1, sizeof(xmlNode));
        xmlFree(cur);
}

/**
 * xmlUnlinkNode:
 * @cur:  the node
 *
 * Unlink a node from it's current context, the node is not freed
 */
XVOID xmlUnlinkNode(xmlNodePtr cur)
{
    if (cur == XNULL) 
    {
        return;
    }
    
    if ((cur->doc != XNULL) && (cur->doc->root == cur))
        cur->doc->root = XNULL;
    
    if ((cur->parent != XNULL) && (cur->parent->children == cur))
        cur->parent->children = cur->next;
    
    if ((cur->parent != XNULL) && (cur->parent->last == cur))
        cur->parent->last = cur->prev;
    
    if (cur->next != XNULL)
        cur->next->prev = cur->prev;
    
    if (cur->prev != XNULL)
        cur->prev->next = cur->next;
    
    cur->next = cur->prev = XNULL;
    cur->parent = XNULL;
    cur->ns = XNULL;
}

/**
 * xmlReplaceNode:
 * @old:  the old node
 * @cur:  the node
 *
 * Unlink the old node from its current context, prune the new one
 * at the same place. If @cur was already inserted in a document it is
 * first unlinked from its existing context.
 *
 * Returns the @old node
 */
xmlNodePtr xmlReplaceNode(xmlNodePtr old, xmlNodePtr cur) {
    if (old == cur) return(NULL);
    if ((old == NULL) || (old->parent == NULL)) {
        return(NULL);
    }
    if (cur == NULL) {
        xmlUnlinkNode(old);
        return(old);
    }
    if (cur == old) {
        return(old);
    }
    if ((old->type==XML_ATTRIBUTE_NODE) && (cur->type!=XML_ATTRIBUTE_NODE)) {
        return(old);
    }
    if ((cur->type==XML_ATTRIBUTE_NODE) && (old->type!=XML_ATTRIBUTE_NODE)) {
        return(old);
    }
    xmlUnlinkNode(cur);
    xmlSetTreeDoc(cur, old->doc);
    cur->parent = old->parent;
    cur->next = old->next;
    
    if (cur->next != NULL)
        cur->next->prev = cur;
    cur->prev = old->prev;
    if (cur->prev != NULL)
        cur->prev->next = cur;
    if (cur->parent != NULL) {
        if (cur->type == XML_ATTRIBUTE_NODE) {
            if (cur->parent->properties == (xmlAttrPtr)old)
                cur->parent->properties = ((xmlAttrPtr) cur);
        } else {
            if (cur->parent->children == old)
                cur->parent->children = cur;
            if (cur->parent->last == old)
                cur->parent->last = cur;
        }
    }
    old->next = old->prev = NULL;
    old->parent = NULL;
    return(old);
}

/**
 * xmlDocGetRootElement:
 * @doc:  the document
 *
 * Get the root element of the document (doc->children is a list
 * containing possibly comments, PIs, etc ...).
 *
 * Returns the #xmlNodePtr for the root or NULL
 */
xmlNodePtr xmlDocGetRootElement(xmlDocPtr doc)
{
    xmlNodePtr ret;
    
    if (doc == XNULL)
    {
        return(XNULL);
    }

    if(doc->root) {
        ret = doc->root;
    } else {
        ret = doc->children;
    }
    
    while (ret != XNULL) 
    {
        if (ret->type == XML_ELEMENT_NODE)
        {
            return(ret);
        }
        ret = ret->next;
    }
    return(ret);
}

/**
 * xmlDocSetRootElement:
 * @doc:  the document
 * @root:  the new document root element, if root is NULL no action is taken,
 *         to remove a node from a document use xmlUnlinkNode(root) instead.
 *
 * Set the root element of the document (doc->children is a list
 * containing possibly comments, PIs, etc ...).
 *
 * Returns the old root element if any was found, NULL if root was NULL
 */
xmlNodePtr
xmlDocSetRootElement(xmlDocPtr doc, xmlNodePtr root) {
    xmlNodePtr old = NULL;

    if (doc == NULL) return(NULL);
    if (root == NULL)
        return(NULL);
    
    xmlUnlinkNode(root);
    xmlSetTreeDoc(root, doc);
    root->parent = (xmlNodePtr) doc;
    doc->root = root;
    old = doc->children;
    while (old != NULL) {
        if (old->type == XML_ELEMENT_NODE)
            break;
        old = old->next;
    }
    if (old == NULL) {
        if (doc->children == NULL) {
            doc->children = root;
            doc->last = root;
        } else {
            xmlAddSibling(doc->children, root);
        }
    } else {
        xmlReplaceNode(old, root);
    }
    return(old);
}

/**
 * xmlNodeSetContent:
 * @cur:  the node being modified
 * @content:  the new value of the content
 *
 * Replace the content of a node.
 * NOTE: @content is supposed to be a piece of XML CDATA, so it allows entity
 *       references, but XML special chars need to be escaped first by using
 *       xmlEncodeEntitiesReentrant() resp. xmlEncodeSpecialChars().
 */
void
xmlNodeSetContent(xmlNodePtr cur, XCONST xmlChar *content) {
    if (cur == NULL) {
        return;
    }
    switch (cur->type) {
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ELEMENT_NODE:
        case XML_ATTRIBUTE_NODE:
            if (cur->children != NULL) xmlFreeNodeList(cur->children);
            cur->children = xmlStringGetNodeList(cur->doc, content);
            UPDATE_LAST_CHILD_AND_PARENT(cur)
            break;
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
            if ((cur->content != NULL) && (cur->content != (xmlChar *) &(cur->properties))) {
                xmlFree(cur->content);
            }
            if (cur->children != NULL) xmlFreeNodeList(cur->children);
            cur->last = cur->children = NULL;
            if (content != NULL) {
                cur->content = xmlStrdup(content);
            } else
                cur->content = NULL;
            cur->properties = NULL;
            break;
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
        case XML_DOCUMENT_TYPE_NODE:
        case XML_XINCLUDE_START:
        case XML_XINCLUDE_END:
              break;
        case XML_NOTATION_NODE:
            break;
        case XML_DTD_NODE:
            break;
        case XML_NAMESPACE_DECL:
            break;
        case XML_ELEMENT_DECL:
        /* TODO !!! */
            break;
        case XML_ATTRIBUTE_DECL:
        /* TODO !!! */
            break;
        case XML_ENTITY_DECL:
        /* TODO !!! */
            break;
    }
}

/**
 * xmlNodeAddContentLen:
 * @cur:  the node being modified
 * @content:  extra content
 * @len:  the size of @content
 *
 * Append the extra substring to the node content.
 * NOTE: In contrast to xmlNodeSetContentLen(), @content is supposed to be
 *       raw text, so unescaped XML special chars are allowed, entity
 *       references are not supported.
 */
XVOID xmlNodeAddContentLen(xmlNodePtr cur, XCONST xmlChar *content, XS32 len)
{
    if (cur == XNULL) 
    {
        return;
    }
    if (len <= 0) return;
    switch (cur->type) {
    case XML_DOCUMENT_FRAG_NODE:
    case XML_ELEMENT_NODE:
        {
        xmlNodePtr last = XNULL, new;
        
        if (cur->children != XNULL) {
            last = cur->last;
        } else {
            if (cur->content != XNULL) {
#ifndef XML_USE_BUFFER_CONTENT
                cur->children = xmlStringGetNodeList(cur->doc, cur->content);
#else
                cur->children = xmlStringGetNodeList(cur->doc,
                    xmlBufferContent(cur->content));
#endif
                UPDATE_LAST_CHILD_AND_PARENT(cur)
#ifndef XML_USE_BUFFER_CONTENT
                    xmlFree(cur->content);
#else
                xmlBufferFree(cur->content);
#endif
                cur->content = XNULL;
                last = cur->last;
            }
        }
        new = xmlNewTextLen(content, len);
        if (new != XNULL) {
            xmlAddChild(cur, new);
            if ((last != XNULL) && (last->next == new)) {
                xmlTextMerge(last, new);
            }
        }
        break;
                           }
    case XML_ATTRIBUTE_NODE:
        break;
    case XML_TEXT_NODE:
    case XML_CDATA_SECTION_NODE:
    case XML_ENTITY_REF_NODE:
    case XML_ENTITY_NODE:
    case XML_PI_NODE:
    case XML_COMMENT_NODE:
    case XML_NOTATION_NODE:
        if (content != XNULL) {
#ifndef XML_USE_BUFFER_CONTENT
            cur->content = xmlStrncat(cur->content, content, len);
#else
            xmlBufferAdd(cur->content, content, len);
#endif
        }
        break;
    case XML_DOCUMENT_NODE:
    case XML_HTML_DOCUMENT_NODE:
    case XML_DOCUMENT_TYPE_NODE:
        break;
    default:
        break;
    }
}

/**
 * xmlNodeAddContent:
 * @cur:  the node being modified
 * @content:  extra content
 *
 * Append the extra substring to the node content.
 * NOTE: In contrast to xmlNodeSetContent(), @content is supposed to be
 *       raw text, so unescaped XML special chars are allowed, entity
 *       references are not supported.
 */
XVOID xmlNodeAddContent(xmlNodePtr cur, XCONST xmlChar *content)
{
    XS32 len;
    
    if (cur == XNULL) 
    {
        return;
    }
    if (content == XNULL) return;
    len = xmlStrlen(content);
    xmlNodeAddContentLen(cur, content, len);
}

/**
 * xmlTextMerge:
 * @first:  the first text node
 * @second:  the second text node being merged
 *
 * Merge two text nodes into one
 * Returns the first text node augmented
 */
xmlNodePtr xmlTextMerge(xmlNodePtr first, xmlNodePtr second)
{
    if (first == XNULL) return(second);
    if (second == XNULL) return(first);
    if (first->type != XML_TEXT_NODE) return(first);
    if (second->type != XML_TEXT_NODE) return(first);
#ifndef XML_USE_BUFFER_CONTENT
    xmlNodeAddContent(first, second->content);
#else
    xmlNodeAddContent(first, xmlBufferContent(second->content));
#endif
    xmlUnlinkNode(second);
    xmlFreeNode(second);
    return(first);
}

static xmlAttrPtr
xmlGetPropNodeInternal(xmlNodePtr node, XCONST xmlChar *name,
               XCONST xmlChar *nsName, int useDTD)
{
    xmlAttrPtr prop;

    if ((node == NULL) || (node->type != XML_ELEMENT_NODE) || (name == NULL))
    return(NULL);

    if (node->properties != NULL)
    {
        prop = node->properties;
        if (nsName == NULL)
        {
            do
            {
                if (xmlStrEqual(prop->name, name))
                {
                    return(prop);
                }
                prop = prop->next;
            } while (prop != NULL);
        }
        else
        {
            return(NULL);
        }
    }
    else
    {
        return(NULL);
    }
    return(NULL);
}

/**
 * xmlHasNsProp:
 * @node:  the node
 * @name:  the attribute name
 * @nameSpace:  the URI of the namespace
 *
 * Search for an attribute associated to a node
 * This attribute has to be anchored in the namespace specified.
 * This does the entity substitution.
 * This function looks in DTD attribute declaration for #FIXED or
 * default declaration values unless DTD use has been turned off.
 * Note that a namespace of NULL indicates to use the default namespace.
 *
 * Returns the attribute or the attribute declaration or NULL
 *     if neither was found.
 */
xmlAttrPtr
xmlHasNsProp(xmlNodePtr node, XCONST xmlChar *name, XCONST xmlChar *nameSpace) {

    return(xmlGetPropNodeInternal(node, name, nameSpace, xmlCheckDTD));
}

/**
 * xmlGetProp:
 * @node:  the node
 * @name:  the attribute name
 *
 * Search and get the value of an attribute associated to a node
 * This does the entity substitution.
 * This function looks in DTD attribute declaration for #FIXED or
 * default declaration values unless DTD use has been turned off.
 * NOTE: this function acts independently of namespaces associated
 *       to the attribute. Use xmlGetNsProp() or xmlGetNoNsProp()
 *       for namespace aware processing.
 *
 * Returns the attribute value or NULL if not found.
 *     It's up to the caller to free the memory with xmlFree().
 */
xmlChar * xmlGetProp(xmlNodePtr node, XCONST xmlChar *name)
{
    xmlAttrPtr prop;
    
    if ((node == XNULL) || (name == XNULL)) 
    {
        return(XNULL);
    }
    /*
    * Check on the properties attached to the node
    */
    prop = node->properties;
    while (prop != XNULL) 
    {
        if (!xmlStrcmp(prop->name, name))  
        {
            xmlChar *ret;
            
            ret = xmlNodeListGetString(node->doc, prop->val, 1);
            if (ret == XNULL)
            {
                return(xmlStrdup((xmlChar *)""));
            }
            
            return(ret);
        }
        prop = prop->next;
    }
    
    return(XNULL);
}

/**
 * xmlSetProp:
 * @node:  the node
 * @name:  the attribute name (a QName)
 * @value:  the attribute value
 *
 * Set (or reset) an attribute carried by a node.
 * If @name has a prefix, then the corresponding
 * namespace-binding will be used, if in scope; it is an
 * error it there's no such ns-binding for the prefix in
 * scope.
 * Returns the attribute pointer.
 *
 */
xmlAttrPtr
xmlSetProp(xmlNodePtr node, XCONST xmlChar *name, XCONST xmlChar *value) {
    int len;
    XCONST xmlChar *nqname;

    if ((node == NULL) || (name == NULL) || (node->type != XML_ELEMENT_NODE))
        return(NULL);

    /*
     * handle QNames
     */
    nqname = xmlSplitQName3(name, &len);
    if (nqname != NULL) {
        xmlChar *prefix = xmlStrndup(name, len);
        if (prefix != NULL)
            xmlFree(prefix);
    }
    return(xmlSetNsProp(node, NULL, name, value));
}

/**
 * xmlSetNsProp:
 * @node:  the node
 * @ns:  the namespace definition
 * @name:  the attribute name
 * @value:  the attribute value
 *
 * Set (or reset) an attribute carried by a node.
 * The ns structure must be in scope, this is not checked
 *
 * Returns the attribute pointer.
 */
xmlAttrPtr xmlSetNsProp(xmlNodePtr node, xmlNsPtr ns, XCONST xmlChar *name,
         XCONST xmlChar *value)
{
    xmlAttrPtr prop;

    if (ns && (ns->href == NULL))
        return(NULL);
    prop = xmlGetPropNodeInternal(node, name, (ns != NULL) ? ns->href : NULL, 0);
    if (prop != NULL) {
        /*
        * Modify the attribute's value.
        */
        if (prop->val != NULL)
            xmlFreeNodeList(prop->val);
        prop->val = NULL;
        if (value != NULL) {
            xmlNodePtr tmp;
            prop->val = xmlNewDocText(node->doc, value);
            tmp = prop->val;
            while (tmp != NULL) {
                tmp->parent = (xmlNodePtr) prop;
                tmp = tmp->next;
            }
        }
        return(prop);
    }
    /*
    * No equal attr found; create a new one.
    */
    return(xmlNewPropInternal(node, ns, name, value, 0));
}


/**
 * xmlNodeIsText:
 * @node:  the node
 *
 * Is this node a Text node ?
 * Returns 1 yes, 0 no
 */
int
xmlNodeIsText(xmlNodePtr node) {
    if (node == XNULL) 
        return(0);
    
    if (node->type == XML_TEXT_NODE) 
        return(1);
    return(0);
}

/**
 * xmlIsBlankNode:
 * @node:  the node
 *
 * Checks whether this node is an empty or whitespace only
 * (and possibly ignorable) text-node.
 *
 * Returns 1 yes, 0 no
 */
XS32 xmlIsBlankNode(xmlNodePtr node)
{
    xmlChar *cur;
    if (node == XNULL) return(0);
    
    if (node->type != XML_TEXT_NODE) return(0);
    if (node->content == XNULL) return(0);
    cur = node->content;
    while (*cur != 0) 
    {
        if (!IS_BLANK(*cur)) return(0);
        cur++;
    }
    
    return(1);
}

/**
 * xmlTextConcat:
 * @node:  the node
 * @content:  the content
 * @len:  @content length
 *
 * Concat the given string at the end of the existing node content
 *
 * Returns -1 in case of error, 0 otherwise
 */

XVOID xmlTextConcat(xmlNodePtr node, XCONST xmlChar *content, XS32 len)
{
    if (node == XNULL) return;
    
    if ((node->type != XML_TEXT_NODE) &&
        (node->type != XML_CDATA_SECTION_NODE)) 
    {
        return;
    }
#ifndef XML_USE_BUFFER_CONTENT
    node->content = xmlStrncat(node->content, content, len);
#else
    xmlBufferAdd(node->content, content, len);
#endif
}

/************************************************************************
 *                                    *
 *            Output : to a FILE or in memory            *
 *                                    *
 ************************************************************************/

/**
 * xmlBufferCreate:
 *
 * routine to create an XML buffer.
 * returns the new structure.
 */
xmlBufferPtr
xmlBufferCreate(void) {
    xmlBufferPtr ret;

    ret = (xmlBufferPtr) xmlMalloc(sizeof(xmlBuffer));
    if (ret == NULL) {
        return(NULL);
    }
    ret->use = 0;
    ret->size = BASE_BUFFER_SIZE;
    ret->alloc = xmlBufferAllocScheme;
    ret->content = (xmlChar *) xmlMalloc(ret->size * sizeof(xmlChar));
    if (ret->content == NULL) {
        xmlFree(ret);
        return(NULL);
    }
    ret->content[0] = 0;
    return(ret);
}

/**
 * xmlBufferCreateSize:
 * @size: initial size of buffer
 *
 * routine to create an XML buffer.
 * returns the new structure.
 */
xmlBufferPtr xmlBufferCreateSize(size_t size) {
    xmlBufferPtr ret;

    ret = (xmlBufferPtr) xmlMalloc(sizeof(xmlBuffer));
    if (ret == NULL) {
        return(NULL);
    }
    ret->use = 0;
    ret->alloc = xmlBufferAllocScheme;
    ret->size = (XU32)(size ? size+2 : 0);         /* +1 for ending null */
    if (ret->size){
        ret->content = (xmlChar *) xmlMalloc(ret->size * sizeof(xmlChar));
        if (ret->content == NULL) {
            xmlFree(ret);
            return(NULL);
        }
        ret->content[0] = 0;
    } else{
        ret->content = NULL;
    }
    ret->content = NULL;
    return(ret);
}


/**
 * xmlBufferSetAllocationScheme:
 * @buf:  the buffer to tune
 * @scheme:  allocation scheme to use
 *
 * Sets the allocation scheme for this buffer
 */
void
xmlBufferSetAllocationScheme(xmlBufferPtr buf, xmlBufferAllocationScheme scheme)
{
    if (buf == NULL) {
        return;
    }
    buf->alloc = scheme;
}


/**
* xmlBufferFree:
* @buf:  the buffer to free
*
* Frees an XML buffer.
*/
XVOID xmlBufferFree(xmlBufferPtr buf) 
{
    if (buf == XNULL) 
    {
        return;
    }
    if (buf->content != XNULL) 
    {
#ifndef XML_USE_BUFFER_CONTENT
        XOS_MemSet(buf->content, -1, BASE_BUFFER_SIZE);
#else
        XOS_MemSet(buf->content, -1, buf->size);
#endif
        xmlFree(buf->content);
    }
    XOS_MemSet(buf, -1, sizeof(xmlBuffer));
    xmlFree(buf);
}

/**
 * xmlBufferEmpty:
 * @buf:  the buffer
 *
 * empty a buffer.
 */
void
xmlBufferEmpty(xmlBufferPtr buf) {
    if (buf == NULL) return;
    if (buf->content == NULL) return;
    buf->use = 0;
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) {
        buf->content = BAD_CAST "";
    } else if ((buf->alloc == XML_BUFFER_ALLOC_IO) &&
               (buf->content != NULL)) {
        size_t start_buf = buf->content - buf->content;

    buf->size += (XU32)start_buf;
        buf->content = buf->content;
        buf->content[0] = 0;
    } else {
        buf->content[0] = 0;
    }
}

/**
 * xmlBufferShrink:
 * @buf:  the buffer to dump
 * @len:  the number of xmlChar to remove
 *
 * Remove the beginning of an XML buffer.
 *
 * Returns the number of #xmlChar removed, or -1 in case of failure.
 */
int xmlBufferShrink(xmlBufferPtr buf, unsigned int len) {
    if (len == 0) 
        return(0);
    
    if (len > (XS32)buf->use) 
        return(-1);
    
    buf->use -= len;
    if(buf->use > 0 && len > 0) {
        XOS_MemMove(buf->content, &buf->content[len], buf->use * sizeof(xmlChar));
    }

    if(buf->use >= 0 && buf->use < buf->size) {
        buf->content[buf->use] = 0;
    }
    return(len);
}

/**
 * xmlBufferGrow:
 * @buf:  the buffer
 * @len:  the minimum free size to allocate
 *
 * Grow the available space of an XML buffer.
 *
 * Returns the new available space or -1 in case of error
 */
int
xmlBufferGrow(xmlBufferPtr buf, unsigned int len) {
    int size;
    xmlChar *newbuf;

    if (buf == NULL) return(-1);

    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return(0);
    if (len + buf->use < buf->size) return(0);

    /*
     * Windows has a BIG problem on realloc timing, so we try to double
     * the buffer size (if that's enough) (bug 146697)
     * Apparently BSD too, and it's probably best for linux too
     * On an embedded system this may be something to change
     */
#if 1
    if (buf->size > len)
        size = buf->size * 2;
    else
        size = buf->use + len + 100;
#else
    size = buf->use + len + 100;
#endif

    if ((buf->alloc == XML_BUFFER_ALLOC_IO) && (buf->content != NULL)) {
        size_t start_buf = buf->content - buf->content;

    newbuf = (xmlChar *) xmlRealloc(buf->content, start_buf + size);
    if (newbuf == NULL) {
        return(-1);
    }
    buf->content = newbuf;
    buf->content = newbuf + start_buf;
    } else {
    newbuf = (xmlChar *) xmlRealloc(buf->content, size);
    if (newbuf == NULL) {
        return(-1);
    }
    buf->content = newbuf;
    }
    buf->size = size;
    return(buf->size - buf->use);
}

/**
 * xmlBufferDump:
 * @file:  the file output
 * @buf:  the buffer to dump
 *
 * Dumps an XML buffer to  a FILE *.
 * Returns the number of #xmlChar written
 */
int
xmlBufferDump(FILE *file, xmlBufferPtr buf) {
    int ret;

    if (buf == NULL) {
        return(0);
    }
    if (buf->content == NULL) {
        return(0);
    }
    if (file == NULL)
        file = stdout;
    ret = (XS32)fwrite(buf->content, sizeof(xmlChar), buf->use, file);
    return(ret);
}

/**
 * xmlBufferContent:
 * @buf:  the buffer
 *
 * Function to extract the content of a buffer
 *
 * Returns the internal content
 */

XCONST xmlChar *xmlBufferContent(XCONST xmlBufferPtr buf)
{
    if(!buf)
        return NULL;

    return buf->content;
}

/**
 * xmlBufferLength:
 * @buf:  the buffer
 *
 * Function to get the length of a buffer
 *
 * Returns the length of data in the internal content
 */

int xmlBufferLength(XCONST xmlBufferPtr buf)
{
    if(!buf)
        return 0;

    return buf->use;
}

/**
* xmlBufferResize:
* @buf:  the buffer to resize
* @len:  the desired size
*
* Resize a buffer to accomodate minimum size of <len>.
*
* Returns  0 in case of problems, 1 otherwise
*/
int xmlBufferResize(xmlBufferPtr buf, unsigned int size)
{
    unsigned int newSize = (buf->size ? buf->size*2 : size);/*take care of empty case*/
    xmlChar* rebuf = XNULL;
    
    /* Don't resize if we don't have to */
    if(size < (XS32)buf->size)
        return 1;
    
    /* figure out new size */
    switch(buf->alloc)
    {
    case XML_BUFFER_ALLOC_DOUBLEIT:
        while(size > newSize) newSize *= 2;
        break;
    case XML_BUFFER_ALLOC_EXACT:
        newSize = size+10;
        break;
    default:
        newSize = size+10;
        break;
    }
    
    if (buf->content == XNULL)
        rebuf = (xmlChar *) xmlMalloc(newSize * sizeof(xmlChar));
    else
        rebuf = (xmlChar *) xmlRealloc(buf->content, 
        newSize * sizeof(xmlChar));
    if (rebuf == XNULL) 
    {
        /* xmlBufferAdd : out of memory! */
        return 0;
    }
    buf->content = rebuf;
    buf->size = newSize;

    return 1;
}

/**
 * xmlBufferAdd:
 * @buf:  the buffer to dump
 * @str:  the #xmlChar string
 * @len:  the number of #xmlChar to add
 *
 * Add a string range to an XML buffer. if len == -1, the length of
 * str is recomputed.
 *
 * Returns 0 successful, a positive error code number otherwise
 *         and -1 in case of internal or API error.
 */
XVOID xmlBufferAdd(xmlBufferPtr buf, XCONST xmlChar *str, XS32 len)
{
    XS32 l, needSize;
    
    if (str == XNULL) 
    {
        return;
    }
    if (len < -1) 
    {
        return;
    }
    if (len == 0) 
        return;
    
    /* CJN What's this for??? */
    if (len < 0)
    {
        l = xmlStrlen(str);
    }
    else 
    {
        for (l = 0;l < len;l++)
        {
            if (str[l] == 0)
            {
                break;
            }
        }
    }
    if (l < len)
    {  
        len = l; 
        /*xmlBufferAdd bad length*/
    }
    
    /* CJN 11.18.99 okay, now I'm using the length */
    if(len == -1) 
        len = l;
    
    if (len <= 0) 
        return;
    
    needSize = buf->use + len + 2;
    if( needSize > (XS32)buf->size )
    {
        if(!xmlBufferResize(buf, needSize))
        {
            /* xmlBufferAdd : out of memory! */
            return;
        }
    }
    
    XOS_MemMove(&buf->content[buf->use], str, len*sizeof(xmlChar));
    buf->use += len;
    buf->content[buf->use] = 0;
}

/**
 * xmlBufferAddHead:
 * @buf:  the buffer
 * @str:  the #xmlChar string
 * @len:  the number of #xmlChar to add
 *
 * Add a string range to the beginning of an XML buffer.
 * if len == -1, the length of @str is recomputed.
 *
 * Returns 0 successful, a positive error code number otherwise
 *         and -1 in case of internal or API error.
 */
int
xmlBufferAddHead(xmlBufferPtr buf, XCONST xmlChar *str, int len) {
    unsigned int needSize;

    if (buf == NULL)
        return(-1);
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return -1;
    if (str == NULL) {
        return -1;
    }
    if (len < -1) {
        return -1;
    }
    if (len == 0) return 0;

    if (len < 0)
        len = xmlStrlen(str);

    if (len <= 0) return -1;

    if ((buf->alloc == XML_BUFFER_ALLOC_IO) && (buf->content != NULL)) {
        size_t start_buf = buf->content - buf->content;

    if (start_buf > (unsigned int) len) {
        /*
         * We can add it in the space previously shrinked
         */
        buf->content -= len;
            memmove(&buf->content[0], str, len);
        buf->use += len;
        buf->size += len;
        return(0);
    }
    }
    needSize = buf->use + len + 2;
    if (needSize > buf->size){
        if (!xmlBufferResize(buf, needSize)){
            return XML_ERR_NO_MEMORY;
        }
    }

    memmove(&buf->content[len], &buf->content[0], buf->use);
    memmove(&buf->content[0], str, len);
    buf->use += len;
    buf->content[buf->use] = 0;
    return 0;
}

/**
 * xmlBufferCat:
 * @buf:  the buffer to add to
 * @str:  the #xmlChar string
 *
 * Append a zero terminated string to an XML buffer.
 *
 * Returns 0 successful, a positive error code number otherwise
 *         and -1 in case of internal or API error.
 */
int
xmlBufferCat(xmlBufferPtr buf, XCONST xmlChar *str) {
    if (buf == NULL)
        return(-1);
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return -1;
    if (str == NULL) return -1;
    xmlBufferAdd(buf, str, -1);
    return 0;
}

/**
 * xmlBufferCCat:
 * @buf:  the buffer to dump
 * @str:  the C char string
 *
 * Append a zero terminated C string to an XML buffer.
 *
 * Returns 0 successful, a positive error code number otherwise
 *         and -1 in case of internal or API error.
 */
int
xmlBufferCCat(xmlBufferPtr buf, XCONST char *str) {
    XCONST char *cur;

    if (buf == NULL)
        return(-1);
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return -1;
    if (str == NULL) {
        return -1;
    }
    for (cur = str;*cur != 0;cur++) {
        if (buf->use  + 10 >= buf->size) {
            if (!xmlBufferResize(buf, buf->use+10)){
                return XML_ERR_NO_MEMORY;
            }
        }
        buf->content[buf->use++] = *cur;
    }
    buf->content[buf->use] = 0;
    return 0;
}

/**
 * xmlBufferWriteCHAR:
 * @buf:  the XML buffer
 * @string:  the string to add
 *
 * routine which manages and grows an output buffer. This one adds
 * xmlChars at the end of the buffer.
 */
void
xmlBufferWriteCHAR(xmlBufferPtr buf, XCONST xmlChar *string) {
    if (buf == NULL)
        return;
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return;
    xmlBufferCat(buf, string);
}

/**
 * xmlBufferWriteChar:
 * @buf:  the XML buffer output
 * @string:  the string to add
 *
 * routine which manage and grows an output buffer. This one add
 * C chars at the end of the array.
 */
void
xmlBufferWriteChar(xmlBufferPtr buf, XCONST char *string) {
    if (buf == NULL)
        return;
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return;
    xmlBufferCCat(buf, string);
}


/**
 * xmlBufferWriteQuotedString:
 * @buf:  the XML buffer output
 * @string:  the string to add
 *
 * routine which manage and grows an output buffer. This one writes
 * a quoted or double quoted #xmlChar string, checking first if it holds
 * quote or double-quotes internally
 */
void
xmlBufferWriteQuotedString(xmlBufferPtr buf, XCONST xmlChar *string) {
    XCONST xmlChar *cur, *base;
    if (buf == NULL)
        return;
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return;
    if (xmlStrchr(string, '\"')) {
        if (xmlStrchr(string, '\'')) {
            xmlBufferCCat(buf, "\"");
            base = cur = string;
            while(*cur != 0){
                if(*cur == '"'){
                    if (base != cur)
                        xmlBufferAdd(buf, base, (XS32)(cur - base));
                    xmlBufferAdd(buf, BAD_CAST "&quot;", 6);
                    cur++;
                    base = cur;
                }
                else {
                    cur++;
                }
            }
            if (base != cur)
                xmlBufferAdd(buf, base, (XS32)(cur - base));
            xmlBufferCCat(buf, "\"");
        }else{
            xmlBufferCCat(buf, "\'");
            xmlBufferCat(buf, string);
            xmlBufferCCat(buf, "\'");
        }
    } else {
        xmlBufferCCat(buf, "\"");
        xmlBufferCat(buf, string);
        xmlBufferCCat(buf, "\"");
    }
}

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

