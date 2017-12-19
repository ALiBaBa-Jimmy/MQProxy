/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: parser.h 
**
**  description: : Interfaces, constants and types related to the XML parser.
**
**  author: zhanglei
**
**  date:   2006.3.7
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   zhanglei         2006.3.7              create  
**************************************************************/

#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "xostype.h"
#include "xmltree.h"
#include "xmlencoding.h"
#include "xmlIO.h"


/*
* Compatibility naming layer with libxml1
*/
#ifndef xmlChildrenNode
#define xmlChildrenNode children
#endif



//typedef struct _XOSxmlParserInputBuffer xmlParserInputBuffer;
//typedef xmlParserInputBuffer *xmlParserInputBufferPtr;

//typedef struct _XOSxmlDoc xmlDoc;
//typedef xmlDoc *xmlDocPtr;

//typedef struct _XOSxmlNode xmlNode;
//typedef xmlNode *xmlNodePtr;

//typedef int ( *xmlInputReadCallback) (void * context, char * buffer, int len);
//typedef int ( *xmlInputCloseCallback) (void * context);

/**
* an xmlParserInput is an input flow for the XML processor.
* Each entity parsed is associated an xmlParserInput (except the
* few predefined ones). This is the case both for internal entities
* - in which case the flow is already completely in memory - or
* external entities - in which case we use the buf structure for
* progressive reading and I18N conversions to the internal UTF-8 format.
*/
typedef XVOID (* xmlParserInputDeallocate)(xmlChar *);
typedef struct _XOSxmlParserInput xmlParserInput;
typedef xmlParserInput *xmlParserInputPtr;
struct _XOSxmlParserInput 
{
    /* Input buffer */
    xmlParserInputBufferPtr buf;      /* UTF-8 encoded buffer */
    
    XCONST XS8 *filename;             /* The file analyzed, if any */
    XCONST XS8 *directory;            /* the directory/base of teh file */
    XCONST xmlChar *base;              /* Base of the array to parse */
    XCONST xmlChar *cur;               /* Current XS8 being parsed */
    XS32 length;                       /* length if known */
    XS32 line;                         /* Current line */
    XS32 col;                          /* Current column */
    XS32 consumed;                     /* How many xmlChars already consumed */
    xmlParserInputDeallocate free;    /* function to deallocate the base */
    /* added after 2.3.5 integration */
    XCONST xmlChar *end;               /* end of the arry to parse */
    XCONST xmlChar *encoding;          /* the encoding string for entity */
    XCONST xmlChar *version;           /* the version string for entity */
    XS32 standalone;                   /* Was that entity marked standalone */
};


struct _XOSxmlParserInputBuffer 
{
    /* Inputs */
    FILE          *file;    /* Input on file handler */
    int           fd;       /* Input on a file descriptor */
    
    xmlBufferPtr buffer;    /* Local buffer encoded in  UTF-8 */
    /* Added when merging 2.3.5 code */
    xmlBufferPtr raw;       /* if encoder != NULL buffer for raw input */
    xmlInputReadCallback readcallback;
    xmlInputCloseCallback closecallback;
};
/**
* the parser can be asked to collect Node informations, i.e. at what
* place in the file they were detected. 
* NOTE: This is off by default and not very well tested.
*/
typedef struct _XOSxmlParserNodeInfo xmlParserNodeInfo;
typedef xmlParserNodeInfo *xmlParserNodeInfoPtr;

struct _XOSxmlParserNodeInfo 
{
    XCONST struct _XOSxmlNode* node;
    /* Position & line # that text that created the node begins & ends on */
    XU32 begin_pos;
    XU32 begin_line;
    XU32 end_pos;
    XU32 end_line;
};

typedef struct _XOSxmlParserNodeInfoSeq xmlParserNodeInfoSeq;
typedef xmlParserNodeInfoSeq *xmlParserNodeInfoSeqPtr;
struct _XOSxmlParserNodeInfoSeq 
{
    XU32 maximum;
    XU32 length;
    xmlParserNodeInfo* buffer;
};

/**
* The parser is now working also as a state based parser
* The recursive one use the stagte info for entities processing
*/
typedef enum 
{
    XML_PARSER_EOF = -1,    /* nothing is to be parsed */
    XML_PARSER_START = 0,    /* nothing has been parsed */
    XML_PARSER_MISC,        /* Misc* before XS32 subset */
    XML_PARSER_PI,        /* Whithin a processing instruction */
    XML_PARSER_DTD,        /* within some DTD content */
    XML_PARSER_PROLOG,        /* Misc* after internal subset */
    XML_PARSER_COMMENT,        /* within a comment */
    XML_PARSER_START_TAG,    /* within a start tag */
    XML_PARSER_CONTENT,        /* within the content */
    XML_PARSER_CDATA_SECTION,    /* within a CDATA section */
    XML_PARSER_END_TAG,        /* within a closing tag */
    XML_PARSER_ENTITY_DECL,    /* within an entity declaration */
    XML_PARSER_ENTITY_VALUE,    /* within an entity value in a decl */
    XML_PARSER_ATTRIBUTE_VALUE,    /* within an attribute value */
    XML_PARSER_EPILOG,         /* the Misc* after the last end tag */
    /* added after 2.3.5 integration */
    XML_PARSER_SYSTEM_LITERAL,    /* within a SYSTEM value */
    XML_PARSER_IGNORE        /* within an IGNORED section */
} xmlParserInputState;

/**
* The parser context.
* NOTE This doesn't completely defines the parser state, the (current ?)
*      design of the parser uses recursive function calls since this allow
*      and easy mapping from the production rules of the specification
*      to the actual code. The drawback is that the actual function call
*      also reflect the parser state. However most of the parsing routines
*      takes as the only argument the parser context pointer, so migrating
*      to a state based parser for progressive parsing shouldn't be too hard.
*/
typedef struct _XOSxmlParserCtxt xmlParserCtxt;
typedef xmlParserCtxt *xmlParserCtxtPtr;
struct _XOSxmlParserCtxt 
{
    XVOID            *userData;        /* the document being built */
    xmlDocPtr           myDoc;        /* the document being built */
    XS32            wellFormed;        /* is the document well formed */
    XS32       replaceEntities;        /* shall we replace entities ? */
    XCONST xmlChar       *version;        /* the XML version string */
    XCONST xmlChar      *encoding;        /* encoding, if any */
    XS32            standalone;        /* standalone document */
    
    /* Input stream stack */
    xmlParserInputPtr  input;         /* Current input stream */
    XS32                inputNr;       /* Number of current input streams */
    XS32                inputMax;      /* Max number of input streams */
    xmlParserInputPtr *inputTab;      /* stack of inputs */
    
    /* Node analysis stack only used for DOM building */
    xmlNodePtr         node;          /* Current parsed Node */
    XS32                nodeNr;        /* Depth of the parsing stack */
    XS32                nodeMax;       /* Max depth of the parsing stack */
    xmlNodePtr        *nodeTab;       /* array of nodes */
    
    /*XS32 record_info;                   Whether node info should be kept */
    xmlParserNodeInfoSeq node_seq;    /* info about each node parsed */
    
    XS32 errNo;                        /* error code */
    
    XS32             hasPErefs;        /* the internal subset has PE refs */
    XS32              external;        /* are we parsing an external entity */
    
    XS32                 valid;        /* is the document valid */
    
    xmlParserInputState instate;      /* current type of input */
    XS32                 token;        /* next XS8 look-ahead */    
    
    XS8           *directory;        /* the data directory */
    
    /* Node name stack only used for HTML parsing */
    xmlChar           *name;          /* Current parsed Node */
    XS32                nameNr;        /* Depth of the parsing stack */
    XS32                nameMax;       /* Max depth of the parsing stack */
    xmlChar *         *nameTab;       /* array of nodes */
    
    XS32               nbChars;       /* number of xmlChar processed */
    XS32            checkIndex;       /* used by progressive parsing lookup */
    XS32             keepBlanks;       /* ugly but ... */
    
    /* Added after integration of 2.3.5 parser */
    XS32             disableSAX;       /* SAX callbacks are disabled */
    XS32               inSubset;       /* Parsing is in XS32 1/ext 2 subset */
    
    XS32                depth;         /* to prevent entity substitution loops */
    xmlParserInputPtr  entity;        /* used to check entities boundaries */
                                      XS32                charset;       /* encoding of the in-memory content
                                      actually an xmlCharEncoding */
    XS32                nodelen;       /* Those two fields are there to */
    XS32                nodemem;       /* Speed up large node parsing */
    XS32                pedantic;      /* signal pedantic warnings or loose
                                          behaviour */
    XVOID              *_private;      /* For user data, libxml won't touch it */
                                                                         
};


xmlDocPtr  xmlParseFile(XCONST XS8 *filename);
xmlDocPtr xmlParseMemory(const char *buffer, int size);

xmlNodePtr xmlDocGetRootElement(xmlDocPtr doc);
XS32 xmlIsBlankNode(xmlNodePtr node);
xmlChar *xmlNodeListGetString(xmlDocPtr doc, xmlNodePtr list, XS32 inLine);
xmlChar * xmlGetProp(xmlNodePtr node, XCONST xmlChar *name);

void xmlCleanupParser();

#ifdef __cplusplus
}
#endif

#endif /* __XML_PARSER_H__ */
