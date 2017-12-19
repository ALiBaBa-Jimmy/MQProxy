/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  parser.c 
**
**  description: an XML 1.0 non-verifying parser
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
**   zhanglei         2006.4.7              create  
**************************************************************/

/**/

#include "../inc/xmlparser.h"
#include "../inc/xmlerror.h"

#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef XOS_WIN32
#include <io.h>
#include <direct.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */


#define LIBXML_VERSION "1.0"

/*
* Constants.
*/
#define XML_DEFAULT_VERSION          "1.0"
#define XML_PARSER_BIG_BUFFER_SIZE 1000
#define XML_PARSER_BUFFER_SIZE     100
#define INPUT_CHUNK                   250

/* we need to keep enough input to show errors in context */
#define LINE_LEN          80
#define MINLEN            4000


/*
* Macro used to grow the current buffer.
*/
#if 0 
#define growBufferReentrant() {                        \
    buffer_size *= 2;                            \
    buffer = (xmlChar *)                        \
            xmlRealloc(buffer, buffer_size * sizeof(xmlChar));    \
    if (buffer == XNULL) {                        \
    return(XNULL);                            \
    }                                    \
}
#endif
/*
* Macro used to grow the current buffer.
*/
#define growBuffer(buffer) {                        \
    buffer##_size *= 2;                            \
    buffer = (xmlChar *) xmlRealloc(buffer, buffer##_size * sizeof(xmlChar));    \
    if (buffer == XNULL) {                        \
    perror("realloc failed");                    \
    return(XNULL);                            \
    }                                    \
}
#define PUSH_AND_POP1(scope, type, name)                \
    scope XS32 name##Push(xmlParserCtxtPtr ctxt, type value) {        \
    if (ctxt->name##Nr >= ctxt->name##Max) {                \
    ctxt->name##Max *= 2;                        \
    ctxt->name##Tab = (XVOID *) xmlRealloc(ctxt->name##Tab,        \
    ctxt->name##Max * sizeof(ctxt->name##Tab[0]));    \
    if (ctxt->name##Tab == XNULL) {                    \
    return(0);                            \
    }                                \
    }                                    \
    ctxt->name##Tab[ctxt->name##Nr] = value;                \
    ctxt->name = value;                            \
    return(ctxt->name##Nr++);                        \
}                                    \
scope type name##Pop(xmlParserCtxtPtr ctxt) {                \
    type ret;                                \
    if (ctxt->name##Nr <= 0) return(0);                    \
    ctxt->name##Nr--;                            \
    if (ctxt->name##Nr > 0)                        \
    ctxt->name = ctxt->name##Tab[ctxt->name##Nr - 1];        \
    else                                \
    ctxt->name = XNULL;                        \
    ret = ctxt->name##Tab[ctxt->name##Nr];                \
    ctxt->name##Tab[ctxt->name##Nr] = 0;                \
    return(ret);                            \
}                                    \
    

PUSH_AND_POP1(extern, xmlParserInputPtr, input)
PUSH_AND_POP1(extern, xmlNodePtr, node)
PUSH_AND_POP1(extern, xmlChar*, name)


#define PUSH_AND_POP(scope, type, name)                    \
scope XS32 name##OldPush(xmlParserCtxtPtr ctxt, type value) {        \
    if (ctxt->name##Nr >= ctxt->name##Max) {                \
    ctxt->name##Max *= 2;                        \
    ctxt->name##Tab = (XVOID *) xmlRealloc(ctxt->name##Tab,        \
    ctxt->name##Max * sizeof(ctxt->name##Tab[0]));    \
    if (ctxt->name##Tab == XNULL) {                    \
    return(0);                            \
    }                                \
    }                                    \
    ctxt->name##Tab[ctxt->name##Nr] = value;                \
    ctxt->name = value;                            \
    return(ctxt->name##Nr++);                        \
}                                    \
scope type name##OldPop(xmlParserCtxtPtr ctxt) {            \
    type ret;                                \
    if (ctxt->name##Nr <= 0) return(0);                    \
    ctxt->name##Nr--;                            \
    if (ctxt->name##Nr > 0)                        \
    ctxt->name = ctxt->name##Tab[ctxt->name##Nr - 1];        \
    else                                \
    ctxt->name = XNULL;                        \
    ret = ctxt->name##Tab[ctxt->name##Nr];                \
    ctxt->name##Tab[ctxt->name##Nr] = 0;                \
    return(ret);                            \
}                                    \
    
PUSH_AND_POP(XSTATIC, xmlParserInputPtr, input)
PUSH_AND_POP(XSTATIC, xmlChar*, name)

#define RAW (ctxt->token ? -1 : (*ctxt->input->cur))
#define SKIP(val) ctxt->nbChars += (val),ctxt->input->cur += (val)
#define NXT(val) ctxt->input->cur[(val)]

#define SHRINK if (ctxt->input->cur - ctxt->input->base > INPUT_CHUNK) {\
    xmlParserInputShrink(ctxt->input);                    \
    if ((*ctxt->input->cur == 0) &&                    \
    (xmlParserInputGrow(ctxt->input, INPUT_CHUNK) <= 0))        \
    xmlPopInput(ctxt);                        \
}

#define GROW if (ctxt->input->end - ctxt->input->cur < INPUT_CHUNK) {    \
    xmlParserInputGrow(ctxt->input, INPUT_CHUNK);            \
    if ((*ctxt->input->cur == 0) &&                    \
    (xmlParserInputGrow(ctxt->input, INPUT_CHUNK) <= 0))        \
    xmlPopInput(ctxt);                        \
}


/*
* alpha    = lowalpha | upalpha
*/
#define IS_ALPHA(x) (IS_LOWALPHA(x) || IS_UPALPHA(x))

#define IS_LOWALPHA(x) (((x) >= 'a') && ((x) <= 'z'))
#define IS_UPALPHA(x) (((x) >= 'A') && ((x) <= 'Z'))
#define IS_ALPHANUM(x) (IS_ALPHA(x) || IS_DIGIT(x))
#define IS_HEX(x) ((IS_DIGIT(x)) || (((x) >= 'a') && ((x) <= 'f')) || \
        (((x) >= 'A') && ((x) <= 'F')))

#define IS_MARK(x) (((x) == '-') || ((x) == '_') || ((x) == '.') ||    \
    ((x) == '!') || ((x) == '~') || ((x) == '*') || ((x) == '\'') ||    \
    ((x) == '(') || ((x) == ')'))

#define IS_RESERVED(x) (((x) == ';') || ((x) == '/') || ((x) == '?') ||    \
    ((x) == ':') || ((x) == '@') || ((x) == '&') || ((x) == '=') ||    \
    ((x) == '+') || ((x) == '$') || ((x) == ','))

#define IS_UNRESERVED(x) (IS_ALPHANUM(x) || IS_MARK(x))

#define IS_ESCAPED(p) ((*(p) == '%') && (IS_HEX((p)[1])) &&        \
(IS_HEX((p)[2])))

#define IS_URIC_NO_SLASH(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||\
    ((*(p) == ';')) || ((*(p) == '?')) || ((*(p) == ':')) ||\
    ((*(p) == '@')) || ((*(p) == '&')) || ((*(p) == '=')) ||\
    ((*(p) == '+')) || ((*(p) == '$')) || ((*(p) == ',')))

/*
* pchar = unreserved | escaped | ":" | "@" | "&" | "=" | "+" | "$" | ","
*/
#define IS_PCHAR(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||    \
    ((*(p) == ':')) || ((*(p) == '@')) || ((*(p) == '&')) ||\
    ((*(p) == '=')) || ((*(p) == '+')) || ((*(p) == '$')) ||\
    ((*(p) == ',')))

/*
* rel_segment   = 1*( unreserved | escaped |
*                 ";" | "@" | "&" | "=" | "+" | "$" | "," )
*/

#define IS_SEGMENT(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||    \
    ((*(p) == ';')) || ((*(p) == '@')) || ((*(p) == '&')) ||    \
    ((*(p) == '=')) || ((*(p) == '+')) || ((*(p) == '$')) ||    \
    ((*(p) == ',')))

/*
* scheme = alpha *( alpha | digit | "+" | "-" | "." )
*/
#define IS_SCHEME(x) ((IS_ALPHA(x)) || (IS_DIGIT(x)) ||            \
((x) == '+') || ((x) == '-') || ((x) == '.'))

/*
* reg_name = 1*( unreserved | escaped | "$" | "," |
*                ";" | ":" | "@" | "&" | "=" | "+" )
*/
#define IS_REG_NAME(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||    \
    ((*(p) == '$')) || ((*(p) == ',')) || ((*(p) == ';')) ||        \
    ((*(p) == ':')) || ((*(p) == '@')) || ((*(p) == '&')) ||        \
((*(p) == '=')) || ((*(p) == '+')))

/*
* userinfo = *( unreserved | escaped | ";" | ":" | "&" | "=" |
*                      "+" | "$" | "," )
*/
#define IS_USERINFO(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||    \
    ((*(p) == ';')) || ((*(p) == ':')) || ((*(p) == '&')) ||        \
    ((*(p) == '=')) || ((*(p) == '+')) || ((*(p) == '$')) ||        \
    ((*(p) == ',')))

/*
* uric = reserved | unreserved | escaped
*/
#define IS_URIC(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||        \
(IS_RESERVED(*(p))))

/*
* Skip to next pointer XS8, handle escaped sequences
*/
#define NEXT(p) ((*p == '%')? p += 3 : p++)

#define XML_MAX_NAMELEN 1000

/**
* A few macros needed to help building the parser.
*/


typedef XU8 CHARVAL;

#define NEXTCHARVAL(p) (XU32) *(p);
#define SKIPCHARVAL(p) (p)++;

/************************************************************************
*                                    *
* 8bits / ISO-Latin version of the macros.                *
*                                    *
************************************************************************/
/*
* [2] XS8 ::= #x9 | #xA | #xD | [#x20-#x7F] 
* any  character, excluding the surrogate blocks
*/
#define IS_CHAR(c)                            \
    ((((c) >= 0x20) && ((c) < 0x7F)) ||                \
(((c) >= 0x01) && ((c) <= 0x1F))||    \
(((c) & 0x80) == 0x80))

#define IS_GB2312(c) (((unsigned char)c) >= 0x80 && ((unsigned char)c) <= 0xff)
/*
* [85] BaseChar ::= ... XS32 list see REC ...
*/
#define IS_BASECHAR(c)                            \
    ( (((c) >= 0x0041) && ((c) <= 0x005A)) ||    \
    (((c) >= 0x0061) && ((c) <= 0x007A))     \
)

/*
* [88] Digit ::= ... XS32 list see REC ...
*/
#define IS_DIGIT(c) (((c) >= 0x30) && ((c) <= 0x39))

/*
* [84] Letter ::= BaseChar | Ideographic 
*/
#define IS_LETTER(c) IS_BASECHAR(c)


/*
* [87] CombiningChar ::= ... int list see REC ...
*/
/*not support unicode ,so do not process 0,which means finish symbol,2014.2.26.Liukai*/
//#define IS_COMBINING(c)  ( (c) == 0)
#define IS_COMBINING(c)  (0)
/*
* [89] Extender ::= #x00B7 | #x02D0 | #x02D1 | #x0387 | #x0640 |
*                   #x0E46 | #x0EC6 | #x3005 | [#x3031-#x3035] |
*                   [#x309D-#x309E] | [#x30FC-#x30FE]
*/
#define IS_EXTENDER(c) ((c) == 0xb7)


/*
* Blank chars.
*
* [3] S ::= (#x20 | #x9 | #xD | #xA)+
*/
#define IS_BLANK(c) (((c) == 0x20) || ((c) == 0x09) || ((c) == 0xa) ||    \
((c) == 0x0D))

/*
* [13] PubidChar ::= #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%]
*/
#define IS_PUBIDCHAR(c)                            \
    (((c) == 0x20) || ((c) == 0x0D) || ((c) == 0x0A) ||            \
    (((c) >= 'a') && ((c) <= 'z')) ||                    \
    (((c) >= 'A') && ((c) <= 'Z')) ||                    \
    (((c) >= '0') && ((c) <= '9')) ||                    \
    ((c) == '-') || ((c) == '\'') || ((c) == '(') || ((c) == ')') ||    \
    ((c) == '+') || ((c) == ',') || ((c) == '.') || ((c) == '/') ||    \
    ((c) == ':') || ((c) == '=') || ((c) == '?') || ((c) == ';') ||    \
    ((c) == '!') || ((c) == '*') || ((c) == '#') || ((c) == '@') ||    \
((c) == '$') || ((c) == '_') || ((c) == '%'))


#define MOVETO_ENDTAG(p)                        \
while (IS_CHAR(*p) && (*(p) != '>')) (p)++

/*-------------------------------------------------------------------------
模块内部全局变量
-------------------------------------------------------------------------*/
XS32 xmlSubstituteEntitiesDefaultValue = 0;
XS32 xmlKeepBlanksDefaultValue = 0;
XSTATIC XS32 xmlUseNewParserDefault = 0;
//XSTATIC xmlChar xmlStringText[] = { 't', 'e', 'x', 't', 0 };
xmlBufferAllocationScheme xmlBufferAllocScheme = XML_BUFFER_ALLOC_EXACT;

typedef struct xmlEntityValue
{
    const char *name;
    const char *value;
}xmlEntityValue;

typedef struct xosxmlEntityValue
{
    const int index;
    const char *name;
    const char *value;
}xosxmlEntityValue;

#define XOSXMLMAXHASH (50) /*此值不能修改*/
#define ESCAPE_LEN_4 (4)
#define ESCAPE_LEN_5 (5)
#define ESCAPE_LEN_6 (6)
/*转义字符哈希表*/
const xosxmlEntityValue xosxmlEntityValues[XOSXMLMAXHASH] = 
{ {0,"",""},{0,"",""},{2,"&#x3e;",">"},{0,"",""},{0,"",""},
  {0,"",""},{0,"",""},{0,"",""},{8,"&#x22;","\""},{0,"",""},
  {10,"&#x3E;",">"},{0,"",""},{0,"",""},{13,"&#x27;","'"},{0,"",""},
  {15,"&#34;","\""},{0,"",""},{0,"",""},{18,"&apos;","'"},{0,"",""},
  {20,"&#39;","'"},{0,"",""},{0,"",""},{0,"",""},{24,"&#60;","<"},
  {0,"",""},{0,"",""},{0,"",""},{28,"&quot;","\""},{0,"",""},
  {0,"",""},{0,"",""},{32,"&#x26;","&"},{0,"",""},{0,"",""},
  {0,"",""},{36,"&#62;",">"},{0,"",""},{38,"&gt;",">"},{39,"&#38;","&"},
  {40,"&#x3c;","<"},{0,"",""},{0,"",""},{0,"",""},{0,"",""},
  {45,"&lt;","<"},{0,"",""},{47,"&amp;","&"},{48,"&#x3C;","<"},{0,"",""},
};

const xmlEntityValue xmlEntityValues[] = 
{
    { "&lt;", "<" },
    { "&gt;", ">" },
    { "&apos;", "'" },
    { "&quot;", "\"" },
    { "&amp;", "&" },
        
    { "&#60;", "<"},
    { "&#62;", ">"},
    { "&#39;", "'"},
    { "&#34;", "\""},
    { "&#38;", "&"},

    { "&#x3c;", "<"},
    { "&#x3e;", ">"},
    { "&#x27;", "'"},
    { "&#x22;", "\""},
    { "&#x26;", "&"},

    { "&#x3C;", "<"},
    { "&#x3E;", ">"},    
};



/*-------------------------------------------------------------------------
模块内部函数
-------------------------------------------------------------------------*/
/**
* The public function calls related to validity checking v-v
*/
XS32    xmlIsMixedElement(xmlDocPtr doc, XCONST xmlChar *name);
XSTATIC XVOID xmlOldFreeInputStream(xmlParserInputPtr input);
XSTATIC xmlChar * xmlOldParseName(xmlParserCtxtPtr ctxt);
XSTATIC xmlChar * xmlOldParseVersionInfo(xmlParserCtxtPtr ctxt);
XSTATIC xmlChar * xmlOldParseEncodingDecl(xmlParserCtxtPtr ctxt);
XSTATIC XVOID xmlOldParseElement(xmlParserCtxtPtr ctxt);
xmlChar * xmlCharStrdup(XCONST XS8 *cur);

xmlParserInputBufferPtr xmlAllocParserInputBuffer();
XS32     xmlParserInputBufferRead(xmlParserInputBufferPtr in,XS32 len);
XS32     xmlParserInputBufferGrow(xmlParserInputBufferPtr in,XS32 len);
XVOID    xmlFreeParserInputBuffer(xmlParserInputBufferPtr in);
XS8 *    xmlParserGetDirectory(XCONST XS8 *filename);
XS32     xmlParserInputGrow(xmlParserInputPtr in,XS32 len);
/**
* xmlChar handling
*/
xmlChar *xmlStrdup(XCONST xmlChar *cur);
xmlChar *xmlStrndup(XCONST xmlChar *cur,XS32 len);
xmlChar *xmlStrsub(XCONST xmlChar *str,XS32 start,XS32 len);
XCONST  xmlChar *xmlStrchr(XCONST xmlChar *str,xmlChar val);
XCONST  xmlChar *xmlStrstr(XCONST xmlChar *str,xmlChar *val);
XS32    xmlStrcmp(XCONST xmlChar *str1,XCONST xmlChar *str2);
XS32    xmlStrncmp(XCONST xmlChar *str1,XCONST xmlChar *str2,XS32 len);
XS32    xmlStrlen(XCONST xmlChar *str);
xmlChar *xmlStrcat(xmlChar *cur,XCONST xmlChar *add);
xmlChar *xmlStrncat(xmlChar *cur,XCONST xmlChar *add,XS32 len);
XS32    xmlStrEqual(XCONST xmlChar *str1, XCONST xmlChar *str2);

/**
* Basic parsing Interfaces
*/
xmlDocPtr    xmlParseFile(XCONST XS8 *filename);
XS32         xmlParseDocument(xmlParserCtxtPtr ctxt);
xmlDocPtr    xmlSAXParseFile(XCONST XS8 *filename,XS32 recovery);


/**
* Parser contexts handling.
*/
XVOID        xmlInitParserCtxt(xmlParserCtxtPtr ctxt);
XVOID        xmlFreeParserCtxt(xmlParserCtxtPtr ctxt);

/**
* Parser context
*/
xmlParserCtxtPtr    xmlCreateFileParserCtxt(XCONST XS8 *filename);

/**
* Input Streams
*/
    xmlChar    xmlPopInput(xmlParserCtxtPtr ctxt);

/*
* Generated by MACROS on top of parser.c c.f. PUSH_AND_POP
*/
XS32 nodePush(xmlParserCtxtPtr ctxt,xmlNodePtr value);
xmlNodePtr    nodePop(xmlParserCtxtPtr ctxt);
XS32 inputPush(xmlParserCtxtPtr ctxt, xmlParserInputPtr value);
xmlParserInputPtr    inputPop(xmlParserCtxtPtr ctxt);

XVOID        startDocument(XVOID *ctx);
XVOID        attribute(XVOID *ctx,
                       XCONST xmlChar *fullname,
                       XCONST xmlChar *value);
XVOID        startElement(XVOID *ctx,
                          XCONST xmlChar *fullname,
                          XCONST xmlChar **atts);
XVOID        endElement(XVOID *ctx,XCONST xmlChar *name);
XVOID        characters(XVOID *ctx,
                        XCONST xmlChar *ch,
                        XS32 len);
XVOID        comment(XVOID *ctx,XCONST xmlChar *value);

XVOID xmlParserInputShrink(xmlParserInputPtr in);
/* a few of the old parser 1.8.11 entry points needed */
XS32 xmlOldParseChunk(xmlParserCtxtPtr ctxt, 
                      XCONST XS8 *chunk, 
                      XS32 size,
                      XS32 terminate);
XS32 xmlOldParseDocument(xmlParserCtxtPtr ctxt);
XVOID xmlNextChar(xmlParserCtxtPtr ctxt);
xmlChar xmlPopInput(xmlParserCtxtPtr ctxt);
XS32 xmlParserInputGrow(xmlParserInputPtr in, XS32 len);
XS32 xmlSkipBlankChars(xmlParserCtxtPtr ctxt);

xmlParserInputBufferPtr 
xmlParserInputBufferCreateFilename(XCONST XS8 *filename); 
xmlParserCtxtPtr xmlNewParserCtxt();
XSTATIC XVOID xmlOldParseContent(xmlParserCtxtPtr ctxt);
XSTATIC XVOID xmlOldParseRootComment(xmlParserCtxtPtr ctxt);
XSTATIC XVOID xmlOldParseElement(xmlParserCtxtPtr ctxt);
XSTATIC xmlChar xmlOldPopInput(xmlParserCtxtPtr ctxt);
XSTATIC XS32 areBlanksOld(xmlParserCtxtPtr ctxt, XCONST xmlChar *str, XS32 len);
XSTATIC XVOID xmlOldParseCharData(xmlParserCtxtPtr ctxt, XS32 cdata);
XSTATIC xmlChar *xmlOldParseStartTag(xmlParserCtxtPtr ctxt);
XSTATIC XVOID xmlOldParseXMLDecl(xmlParserCtxtPtr ctxt);
XSTATIC XVOID xmlOldParseCharData(xmlParserCtxtPtr ctxt, XS32 cdata);
XVOID xmlFreeInputStream(xmlParserInputPtr input);
xmlParserInputPtr xmlNewInputStream(xmlParserCtxtPtr ctxt);
XSTATIC xmlDocPtr xmlSAXParseMemory(const char *buffer, int size);
XSTATIC xmlParserCtxtPtr xmlCreateMemoryParserCtxt(const char *buffer, int size);

/*
* List of XML prefixed PI allowed by W3C specs
*/
XCONST XS8 *xmlW3CPIs[] = 
{
    "xml-stylesheet",
    XNULL
};


/**
* xmlNewStringInputStream:
* @ctxt:  an XML parser context
* @buffer:  an memory buffer
*
* Create a new input stream based on a memory buffer.
* Returns the new input stream
*/
xmlParserInputPtr xmlNewStringInputStream(xmlParserCtxtPtr ctxt, XCONST xmlChar *buffer) 
{
    xmlParserInputPtr input = XNULL;
    
    if (buffer == XNULL) 
    {
        ctxt->errNo = XML_ERR_INTERNAL_ERROR;
        return(XNULL);
    }
    
    input = xmlNewInputStream(ctxt);
    if (input == XNULL) 
    {
        return(XNULL);
    }
    input->base = buffer;
    input->cur = buffer;
    input->length = xmlStrlen(buffer);
    input->end = &buffer[input->length];
    return(input);
}

/************************************************************************
*                                                       *
*         Progressive parsing interfaces                *
*                                                       *
************************************************************************/

/**
* xmlOldParserInputGrow:
* @in:  an XML parser input
* @len:  an indicative size for the lookahead
*
* This function increase the input for the parser. It tries to
* preserve pointers to the input buffer, and keep already read data
*
* Returns the number of xmlChars read, or -1 in case of error, 0 indicate the
* end of this entity
*/
XSTATIC XS32 xmlOldParserInputGrow(xmlParserInputPtr in, XS32 len) 
{
    XS32 ret;
    XS32 index;

    if (in->buf == XNULL) return(-1);
    if (in->base == XNULL) return(-1);
    if (in->cur == XNULL) return(-1);
    if (in->buf->buffer == XNULL) return(-1);

    index = (XS32)(in->cur - in->base);
    if ( in->buf->buffer->use > (XU32)(index + INPUT_CHUNK) ) 
    {
        return(0);
    }
    if ( (in->buf->file != XNULL)
        || (in->buf->fd >= 0))
    {
        ret = xmlParserInputBufferGrow(in->buf, len);
    }
    else
    {
        return(0);
    }    
    /*
    * NOTE : in->base may be a "dandling" i.e. freed pointer in this
    *        block, but we use it really as an integer to do some
    *        pointer arithmetic. Insure will raise it as a bug but in
    *        that specific case, that's not !
    */
    if (in->base != in->buf->buffer->content) 
    {
    /*
    * the buffer has been realloced
    */
        index = (XS32)(in->cur - in->base);
        in->base = in->buf->buffer->content;
        in->cur = &in->buf->buffer->content[index];
    }
    
    return(ret);
}

/**
* xmlOldPushInput:
* @ctxt:  an XML parser context
* @input:  an XML parser input fragment (entity, XML fragment ...).
*
* xmlOldPushInput: switch to a new input stream which is stacked on top
*               of the previous one(s).
*/
XVOID xmlOldPushInput(xmlParserCtxtPtr ctxt, xmlParserInputPtr input) 
{
    if (input == XNULL) 
    {
        return;
    }
    inputOldPush(ctxt, input);
}

/**
* xmlOldFreeInputStream:
* @input:  an xmlParserInputPtr
*
* Free up an input stream.
*/
XSTATIC XVOID xmlOldFreeInputStream(xmlParserInputPtr input) 
{
    if (input == XNULL)
        return;
    
    if (input->filename != XNULL) 
        xmlFree((XS8 *) input->filename);
    
    if (input->directory != XNULL) 
        xmlFree((XS8 *) input->directory);
    
    if ((input->free != XNULL) && (input->base != XNULL))
        input->free((xmlChar *) input->base);
    
    if (input->buf != XNULL) 
        xmlFreeParserInputBuffer(input->buf);
    
    XOS_MemSet(input, -1, sizeof(xmlParserInput));
    xmlFree(input);
}

/************************************************************************
 函数名:xmlBKDRHash
 功能: 字符串哈希函数
 输入:  
 输出:
 返回:   成功返回替换字符对象，失败返回NULL
 说明:
************************************************************************/
int  xmlBKDRHash( const void* param, int hashSize)
{
    int hashKey = 0;
    int ulSeed = 13131; //31、131、1313、13131
    char* pChar = (char*)param;
    int nCount = 0;
    if(param == XNULLP)
    {
        return 0;/*此情况不会出现*/
    }

    while(*pChar && (nCount++ < 6))
    {
        hashKey = hashKey * ulSeed + (*pChar++);
    }

    hashKey = hashKey & 0X7FFFFFFF;

    return hashKey % ((XU32)hashSize);
}
/************************************************************************
 函数名:xmlBKDRHashFind
 功能: 根据字符串查找转义字符
 输入:  
 输出:
 返回:   成功返回替换字符对象，失败返回NULL
 说明:
************************************************************************/
xosxmlEntityValue*  xmlBKDRHashFind( const char* param, int len, int hashSize)
{
    int hashKey = 0;
    int ulSeed = 13131; //31、131、1313、13131
    char *pChar = (char*)param;
    int nCount = 0;

    //if(param == XNULLP)
    //{
    //    return 0;/*此情况不会出现*/
    //}

    while(*pChar && (nCount++ < len))
    {
        hashKey = hashKey * ulSeed + (*pChar++);
    }

    hashKey = hashKey & 0X7FFFFFFF;

    hashKey = hashKey % ((int)hashSize);

    /*解决冲突*/
    if(0 == strncmp((char*)xosxmlEntityValues[hashKey].name, (char*)param, pChar-param))
    {
        return (xosxmlEntityValue*)&xosxmlEntityValues[hashKey];
    }

    return NULL;
}

/************************************************************************
 函数名:xmlEscapeDeal
 功能:  解析转义字符
 输入:  
 输出:
 返回:  成功返回替换字符对象 , 失败返回NULL
 说明:
************************************************************************/
xosxmlEntityValue* xmlEscapeDeal( const void* param)
{
    xosxmlEntityValue *pcRst = NULL;
    int nLen = (int)strlen((char*)param);

    if(nLen < ESCAPE_LEN_4)
    {
        return NULL;
    }

    if(nLen >= ESCAPE_LEN_4)
    {
        pcRst = xmlBKDRHashFind((const char *)param, ESCAPE_LEN_4, XOSXMLMAXHASH);
        if(pcRst != 0)
        {
           return pcRst;
        }       
    }   
   
    if(nLen >= ESCAPE_LEN_5)
    {
        pcRst = xmlBKDRHashFind((const char *)param, ESCAPE_LEN_5, XOSXMLMAXHASH);
        if(pcRst != 0)
        {
            return pcRst;
        }   
    } 

    if(nLen >= ESCAPE_LEN_6)
    {
        pcRst = xmlBKDRHashFind((const char *)param, ESCAPE_LEN_6, XOSXMLMAXHASH);
        if(pcRst != 0 )
        {
           return pcRst;
        }

    }    
    
    return pcRst;
}




/**
* xmlOldParseAttValue:
* @ctxt:  an XML parser context
*
* parse a value for an attribute
* Note: the parser won't do substitution of entities here, this
* will be handled later in xmlStringGetNodeList
*
* [10] AttValue ::= '"' ([^<&"] | Reference)* '"' |
*                   "'" ([^<&'] | Reference)* "'"
*
* 3.3.3 Attribute-Value Normalization:
* Before the value of an attribute is passed to the application or
* checked for validity, the XML processor must normalize it as follows: 
* - a character reference is processed by appending the referenced
*   character to the attribute value
* - an entity reference is processed by recursively processing the
*   replacement text of the entity 
* - a whitespace character (#x20, #xD, #xA, #x9) is processed by
*   appending #x20 to the normalized value, except that only a single
*   #x20 is appended for a "#xD#xA" sequence that is part of an external
*   parsed entity or the literal entity value of an internal parsed entity 
* - other characters are processed by appending them to the normalized value 
* If the declared value is not CDATA, then the XML processor must further
* process the normalized attribute value by discarding any leading and
* trailing space (#x20) characters, and by replacing sequences of space
* (#x20) characters by a single space (#x20) character.  
* All attributes for which no declaration has been read should be treated
* by a non-validating parser as if declared CDATA.
*
* Returns the AttValue parsed or XNULL. The value has to be freed by the caller.
*/

XSTATIC xmlChar *xmlOldParseAttValue(xmlParserCtxtPtr ctxt) 
{
    xmlChar limit    = 0;
    xmlChar *buffer  = XNULL;
    XS32 buffer_size = 0;
    xmlChar *out     = XNULL;
    
    XS32 entity_len = 0;
    XS32 j = 0;
    XS32 flag = 0;
    xmlChar cur      = XNULL;
    xosxmlEntityValue *pescape = NULL;
    
    SHRINK;
    if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '"') 
    {
        ctxt->instate = XML_PARSER_ATTRIBUTE_VALUE;
        limit = '"';
        xmlNextChar(ctxt);
    } 
    else if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '\'') 
    {
        limit = '\'';
        ctxt->instate = XML_PARSER_ATTRIBUTE_VALUE;
        xmlNextChar(ctxt);
    } 
    else 
    {
        ctxt->errNo = XML_ERR_ATTRIBUTE_NOT_STARTED;
        /*AttValue: \" or ' expected*/
        ctxt->wellFormed = 0;
        return(XNULL);
    }
    
    /*
    * allocate a translation buffer.
    */
    buffer_size = XML_PARSER_BUFFER_SIZE;
    buffer = (xmlChar *)xmlMalloc(buffer_size * sizeof(xmlChar));
    if (buffer == XNULL) 
    {
        return(XNULL);
    }
    out = buffer;
    
    /*
    * Ok loop until we reach one of the ending XS8 or a size limit.
    */
    cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
    while ((ctxt->token != 0) 
        || ((cur != limit) 
        && (cur != '<'))) 
    {       
        if (cur == 0) break;
        if (cur == '&') 
        {
            flag = 0;            
            pescape = NULL;
            
            if(NULL != (pescape=xmlEscapeDeal(ctxt->input->cur)))
            {
                entity_len = (int)strlen(pescape->name);
                
                for(j = 0; j < entity_len; j++)
                {
                    xmlNextChar(ctxt);
                }
                
                *out++ = *(pescape->value);
                    
                if (out - buffer > buffer_size - 10) 
                {
                    XS32 tmp_idx = (XS32)(out - buffer);
                    
                    growBuffer(buffer);
                    out = &buffer[tmp_idx];
                }
                
                flag = 1;
            }
            
            if(!flag)   /*not assemble symbols for "&lt;",etc*/
            {
                *out++ = cur;
                if (out - buffer > buffer_size - 10) 
                {
                    XS32 tmp_idx = (XS32)(out - buffer);
                    
                    growBuffer(buffer);
                    out = &buffer[tmp_idx];
                }
                xmlNextChar(ctxt);
            }
        }
        else 
        {
            /*  invalid for UTF-8 , use COPY(out); !!!!!! */
            if ((ctxt->token == 0) 
                && ((cur == 0x20) 
                || (cur == 0xD) 
                || (cur == 0xA) 
                || (cur == 0x9))) 
            {
                *out++ = 0x20;
                if (out - buffer > buffer_size - 10) 
                {
                    XS32 tmp_idx = (XS32)(out - buffer);
                    
                    growBuffer(buffer);
                    out = &buffer[tmp_idx];
                }
            } 
            else 
            {
                *out++ = cur;
                if (out - buffer > buffer_size - 10) 
                {
                    XS32 tmp_idx = (XS32)(out - buffer);
                    
                    growBuffer(buffer);
                    out = &buffer[tmp_idx];
                }
            }
            xmlNextChar(ctxt);
        }
        cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
    }
    *out++ = 0;
    if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '<') 
    {
        /*Unescaped '<' not allowed in attributes values*/
        ctxt->errNo = XML_ERR_LT_IN_ATTRIBUTE;
        ctxt->wellFormed = 0;
    } 
    else if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != limit) 
    {
        /*AttValue: ' expected*/
        ctxt->errNo = XML_ERR_ATTRIBUTE_NOT_FINISHED;
        ctxt->wellFormed = 0;
    } 
    else
        xmlNextChar(ctxt);
    return(buffer);
}

/**
* xmlOldParseComment:
* @ctxt:  an XML parser context
*
* Skip an XML (SGML) comment <!-- .... -->
*  The spec says that "For compatibility, the string "--" (double-hyphen)
*  must not occur within comments. "
*
* [15] Comment ::= '<!--' ((XS8 - '-') | ('-' (XS8 - '-')))* '-->'
*/
XSTATIC XVOID xmlOldParseComment(xmlParserCtxtPtr ctxt) {
    xmlChar *buf = XNULL;
    XS32 len = 0;
    XS32 size = XML_PARSER_BUFFER_SIZE;
    xmlChar q;
    xmlChar r;
    xmlChar cur;
    xmlParserInputState state;
    
    /*
    * Check that there is a comment right here.
    */
    if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '<') || (NXT(1) != '!') ||
        (NXT(2) != '-') || (NXT(3) != '-')) return;
    
    state = ctxt->instate;
    ctxt->instate = XML_PARSER_COMMENT;
    SHRINK;
    SKIP(4);
    buf = (xmlChar *) xmlMalloc(size * sizeof(xmlChar));
    if (buf == XNULL) 
    {
        ctxt->instate = state;
        return;
    }
    q = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
    xmlNextChar(ctxt);
    r = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
    xmlNextChar(ctxt);
    cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));

    while (((cur != '>') ||(r != '-') || (q != '-')))
    {
        if ((r == '-') && (q == '-')) 
        {
            /*Comment must not contain '--' (double-hyphen)`*/
            ctxt->errNo = XML_ERR_HYPHEN_IN_COMMENT;
            ctxt->wellFormed = 0;
        }
        if (len + 1 >= size) {
            size *= 2;
            buf = xmlRealloc(buf, size * sizeof(xmlChar));
            if (buf == XNULL) 
            {
                ctxt->instate = state;
                return;
            }
        }
        buf[len++] = q;
        q = r;
        r = cur;
        xmlNextChar(ctxt);
        cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
        if (cur == 0) 
        {
            SHRINK;
            if (ctxt->input->end - ctxt->input->cur < INPUT_CHUNK) 
            {
                xmlParserInputGrow(ctxt->input, INPUT_CHUNK);
                if ((*ctxt->input->cur == 0) &&
                    (xmlParserInputGrow(ctxt->input, INPUT_CHUNK) <= 0))
                    xmlPopInput(ctxt);
            }
            cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
        }
    }
    
    buf[len] = 0;
    if (!IS_CHAR(cur)) 
    {
        /*Comment not terminated \n<!--%.50s*/
        ctxt->errNo = XML_ERR_COMMENT_NOT_FINISHED;
        ctxt->wellFormed = 0;
    } 
    else 
    {
        xmlNextChar(ctxt);
        comment(ctxt->userData, buf);
        xmlFree(buf);
    }
    ctxt->instate = state;
}

/**
* xmlOldParsePITarget:
* @ctxt:  an XML parser context
* 
* parse the name of a PI
*
* [17] PITarget ::= Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'))
*
* Returns the PITarget name or XNULL
*/

XSTATIC xmlChar *xmlOldParsePITarget(xmlParserCtxtPtr ctxt) 
{
    xmlChar *name;
    
    name = xmlOldParseName(ctxt);
    if ((name != XNULL) &&
        ((name[0] == 'x') || (name[0] == 'X')) &&
        ((name[1] == 'm') || (name[1] == 'M')) &&
        ((name[2] == 'l') || (name[2] == 'L')))
    {
        XS32 i;
        for (i = 0;;i++) 
        {
            if (xmlW3CPIs[i] == XNULL) break;
            if (!xmlStrcmp(name, (XCONST xmlChar *)xmlW3CPIs[i]))
                return(name);
        }
        
    }
    return(name);
}

/**
* xmlOldParsePI:
* @ctxt:  an XML parser context
* 
* parse an XML Processing Instruction.
*
* [16] PI ::= '<?' PITarget (S (XS8* - (XS8* '?>' XS8*)))? '?>'
*
* The processing is transfered to SAX once parsed.
*/

XSTATIC XVOID xmlOldParsePI(xmlParserCtxtPtr ctxt) 
{
    xmlChar *buf = XNULL;
    XS32 len = 0;
    XS32 size = XML_PARSER_BUFFER_SIZE;
    xmlChar cur;
    xmlChar *target;
    xmlParserInputState state;
    
    if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '<') && (NXT(1) == '?')) {
        state = ctxt->instate;
        ctxt->instate = XML_PARSER_PI;
        /*
        * this is a Processing Instruction.
        */
        SKIP(2);
        SHRINK;
        
        /*
        * Parse the target name and check for special support like
        * namespace.
        */
        target = xmlOldParsePITarget(ctxt);
        if (target != XNULL) {
            buf = (xmlChar *) xmlMalloc(size * sizeof(xmlChar));
            if (buf == XNULL) 
            {
                ctxt->instate = state;
                return;
            }
            cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
            if (!IS_BLANK(cur)) 
            {
                /*xmlParsePI: PI %s space expected\*/
                
                ctxt->errNo = XML_ERR_SPACE_REQUIRED;
                ctxt->wellFormed = 0;
                
            }
            xmlSkipBlankChars(ctxt);
            cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
            while (IS_CHAR(cur) &&
                ((cur != '?') || (NXT(1) != '>'))) {
                if (len + 1 >= size) {
                    size *= 2;
                    buf = xmlRealloc(buf, size * sizeof(xmlChar));
                    if (buf == XNULL) 
                    {
                        ctxt->instate = state;
                        return;
                    }
                }
                buf[len++] = cur;
                xmlNextChar(ctxt);
                cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
                if (cur == 0) 
                {
                    SHRINK;
                    if (ctxt->input->end - ctxt->input->cur < INPUT_CHUNK) 
                    {
                        xmlParserInputGrow(ctxt->input, INPUT_CHUNK);
                        if ((*ctxt->input->cur == 0) &&
                            (xmlParserInputGrow(ctxt->input, INPUT_CHUNK) <= 0))
                            xmlPopInput(ctxt);
                    }
                    
                    cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
                }
            }
            buf[len] = 0;
            if (!IS_CHAR(cur)) 
            {
                /*xmlParsePI: PI %s never end ...*/
                ctxt->errNo = XML_ERR_PI_NOT_FINISHED;
                ctxt->wellFormed = 0;
            } 
            else 
            {
                SKIP(2);
            }
            xmlFree(buf);
            xmlFree(target);
        } 
        else 
        {
            /*xmlParsePI : no target name*/
            
            ctxt->errNo = XML_ERR_PI_NOT_STARTED;
            ctxt->wellFormed = 0;
        }
        ctxt->instate = state;
    }
}



/**
* xmlOldParseEndTag:
* @ctxt:  an XML parser context
*
* parse an end of tag
*
* [42] ETag ::= '</' Name S? '>'
*
* With namespace
*
* [NS 9] ETag ::= '</' QName S? '>'
*/

XSTATIC XVOID xmlOldParseEndTag(xmlParserCtxtPtr ctxt) 
{
    xmlChar *name = XNULL;
    xmlChar *oldname = XNULL;
    
    GROW;
    if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '<') 
        || (NXT(1) != '/')) 
    {
        /*xmlParseEndTag: '</' not found*/
        ctxt->wellFormed = 0;
        ctxt->errNo = XML_ERR_LTSLASH_REQUIRED;
        return;
    }
    SKIP(2);
    
    name = xmlOldParseName(ctxt);
    
    /*
    * We should definitely be at the ending "S? '>'" part
    */
    GROW;
    xmlSkipBlankChars(ctxt);
    if ((!IS_CHAR((ctxt->token ? ctxt->token : (*ctxt->input->cur)))) 
        || ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '>')) 
    {
        /*End tag : expected '>'*/
        ctxt->errNo = XML_ERR_GT_REQUIRED;
        ctxt->wellFormed = 0;
    } 
    else
        xmlNextChar(ctxt);
    
        /*
        * [ WFC: Element Type Match ]
        * The Name in an element's end-tag must match the element type in the
        * start-tag. 
        *
    */
    if ((name == XNULL) 
        || (ctxt->name == XNULL) 
        || (xmlStrcmp(name, ctxt->name))) 
    {
    /*Opening and ending tag mismatch: %s and ,
    Ending tag eror for:,
    Ending tag error: internal error ???\n");
        */    
        ctxt->errNo = XML_ERR_TAG_NAME_MISMATCH;
        ctxt->wellFormed = 0;
    }
    
    /*
    * SAX: End of Tag
    */
    endElement(ctxt->userData, name);
    
    if (name != XNULL)
        xmlFree(name);
    oldname = nameOldPop(ctxt);
    if (oldname != XNULL) 
    {
        xmlFree(oldname);
    }
    return;
}


/**
* xmlOldParseVersionNum:
* @ctxt:  an XML parser context
*
* parse the XML version value.
*
* [26] VersionNum ::= ([a-zA-Z0-9_.:] | '-')+
*
* Returns the string giving the XML version number, or XNULL
*/
XSTATIC xmlChar *xmlOldParseVersionNum(xmlParserCtxtPtr ctxt) 
{
    xmlChar *buf = XNULL;
    XS32 len = 0;
    XS32 size = 10;
    xmlChar cur;
    
    buf = (xmlChar *) xmlMalloc(size * sizeof(xmlChar));
    if (buf == XNULL) 
    {
        return(XNULL);
    }
    cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
    while (IS_CHAR(cur) &&
        (((cur >= 'a') && (cur <= 'z')) ||
        ((cur >= 'A') && (cur <= 'Z')) ||
        ((cur >= '0') && (cur <= '9')) ||
        (cur == '_') || (cur == '.') ||
        (cur == ':') || (cur == '-'))) 
    {
        if (len + 1 >= size) 
        {
            size *= 2;
            buf = xmlRealloc(buf, size * sizeof(xmlChar));
            if (buf == XNULL) 
            {
                return(XNULL);
            }
        }
        buf[len++] = cur;
        xmlNextChar(ctxt);
        cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
    }
    buf[len] = 0;
    return(buf);
}

/**
* xmlOldParseVersionInfo:
* @ctxt:  an XML parser context
* 
* parse the XML version.
*
* [24] VersionInfo ::= S 'version' Eq (' VersionNum ' | " VersionNum ")
* 
* [25] Eq ::= S? '=' S?
*
* Returns the version string, e.g. "1.0"
*/

XSTATIC xmlChar *xmlOldParseVersionInfo(xmlParserCtxtPtr ctxt) {
    xmlChar *version = XNULL;
    
    if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == 'v') && (NXT(1) == 'e') &&
        (NXT(2) == 'r') && (NXT(3) == 's') &&
        (NXT(4) == 'i') && (NXT(5) == 'o') &&
        (NXT(6) == 'n')) 
    {
        SKIP(7);
        xmlSkipBlankChars(ctxt);
        if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '=') 
        {
            /*xmlParseVersionInfo : expected '='*/
            
            ctxt->wellFormed = 0;
            ctxt->errNo = XML_ERR_EQUAL_REQUIRED;
            return(XNULL);
        }
        xmlNextChar(ctxt);
        xmlSkipBlankChars(ctxt);
        if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '"') 
        {
            xmlNextChar(ctxt);
            version = xmlOldParseVersionNum(ctxt);
            if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '"') 
            {
                /*String not closed\n%.50s*/
                ctxt->wellFormed = 0;
                ctxt->errNo = XML_ERR_STRING_NOT_CLOSED;
            } 
            else
                xmlNextChar(ctxt);
        } 
        else if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '\'')
        {
            xmlNextChar(ctxt);
            version = xmlOldParseVersionNum(ctxt);
            if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '\'') 
            {
                /*String not closed\n%.50s*/
                ctxt->errNo = XML_ERR_STRING_NOT_CLOSED;
                ctxt->wellFormed = 0;
            } 
            else
                xmlNextChar(ctxt);
        } 
        else 
        {
            /*xmlParseVersionInfo : expected ' or \"*/
            ctxt->wellFormed = 0;
            ctxt->errNo = XML_ERR_STRING_NOT_STARTED;
        }
    }
    return(version);
}

/**
* xmlOldParseEncName:
* @ctxt:  an XML parser context
*
* parse the XML encoding name
*
* [81] EncName ::= [A-Za-z] ([A-Za-z0-9._] | '-')*
*
* Returns the encoding name value or XNULL
*/
XSTATIC xmlChar *xmlOldParseEncName(xmlParserCtxtPtr ctxt) {
    xmlChar *buf = XNULL;
    XS32 len = 0;
    XS32 size = 10;
    xmlChar cur;
    
    cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
    if (((cur >= 'a') && (cur <= 'z')) ||
        ((cur >= 'A') && (cur <= 'Z'))) {
        buf = (xmlChar *) xmlMalloc(size * sizeof(xmlChar));
        if (buf == XNULL) {
            return(XNULL);
        }
        
        buf[len++] = cur;
        xmlNextChar(ctxt);
        cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
        while (IS_CHAR(cur) &&
            (((cur >= 'a') && (cur <= 'z')) ||
            ((cur >= 'A') && (cur <= 'Z')) ||
            ((cur >= '0') && (cur <= '9')) ||
            (cur == '.') || (cur == '_') ||
            (cur == '-'))) {
            if (len + 1 >= size) 
            {
                size *= 2;
                buf = xmlRealloc(buf, size * sizeof(xmlChar));
                if (buf == XNULL) 
                {
                    return(XNULL);
                }
            }
            buf[len++] = cur;
            xmlNextChar(ctxt);
            cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
            if (cur == 0) {
                SHRINK;
                GROW;
                cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
            }
        }
        buf[len] = 0;
    } 
    else 
    {
        /*Invalid XML encoding name*/
        ctxt->wellFormed = 0;
        ctxt->errNo = XML_ERR_ENCODING_NAME;
    }
    return(buf);
}

/**
* xmlOldParseEncodingDecl:
* @ctxt:  an XML parser context
* 
* parse the XML encoding declaration
*
* [80] EncodingDecl ::= S 'encoding' Eq ('"' EncName '"' |  "'" EncName "'")
*
* TODO: this should setup the conversion filters.
*
* Returns the encoding value or XNULL
*/

XSTATIC xmlChar *xmlOldParseEncodingDecl(xmlParserCtxtPtr ctxt) 
{
    xmlChar *encoding = XNULL;
    
    xmlSkipBlankChars(ctxt);
    if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == 'e') 
        && (NXT(1) == 'n') 
        && (NXT(2) == 'c') 
        && (NXT(3) == 'o') 
        && (NXT(4) == 'd') 
        && (NXT(5) == 'i') 
        && (NXT(6) == 'n') 
        && (NXT(7) == 'g')) 
    {
        SKIP(8);
        xmlSkipBlankChars(ctxt);
        if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '=') 
        {
            ctxt->wellFormed = 0;
            ctxt->errNo = XML_ERR_EQUAL_REQUIRED;
            return(XNULL);
        }
        xmlNextChar(ctxt);
        xmlSkipBlankChars(ctxt);
        if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '"') 
        {
            xmlNextChar(ctxt);
            encoding = xmlOldParseEncName(ctxt);
            if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '"') 
            {
                /*String not closed\n%.50s*/
                ctxt->wellFormed = 0;
                ctxt->errNo = XML_ERR_STRING_NOT_CLOSED;
            } 
            else
                xmlNextChar(ctxt);
        } else if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '\''){
            xmlNextChar(ctxt);
            encoding = xmlOldParseEncName(ctxt);
            if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '\'') 
            {
                ctxt->wellFormed = 0;
                ctxt->errNo = XML_ERR_STRING_NOT_CLOSED;
            } 
            else
                xmlNextChar(ctxt);
        } 
        else if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '"')
        {
            ctxt->wellFormed = 0;
            ctxt->errNo = XML_ERR_STRING_NOT_STARTED;
        }
    }
    return(encoding);
}

/**
* xmlOldParseSDDecl:
* @ctxt:  an XML parser context
*
* parse the XML standalone declaration
*
* [32] SDDecl ::= S 'standalone' Eq
*                 (("'" ('yes' | 'no') "'") | ('"' ('yes' | 'no')'"')) 
*
* [ VC: Standalone Document Declaration ]
* TODO The standalone document declaration must have the value "no"
* if any external markup declarations contain declarations of:
*  - attributes with default values, if elements to which these
*    attributes apply appear in the document without specifications
*    of values for these attributes, or
*  - entities (other than amp, lt, gt, apos, quot), if references
*    to those entities appear in the document, or
*  - attributes with values subject to normalization, where the
*    attribute appears in the document with a value which will change
*    as a result of normalization, or
*  - element types with element content, if white space occurs directly
*    within any instance of those types.
*
* Returns 1 if standalone, 0 otherwise
*/

XSTATIC XS32 xmlOldParseSDDecl(xmlParserCtxtPtr ctxt) 
{
    XS32 standalone = -1;
    
    xmlSkipBlankChars(ctxt);
    if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == 's') && (NXT(1) == 't') &&
        (NXT(2) == 'a') && (NXT(3) == 'n') &&
        (NXT(4) == 'd') && (NXT(5) == 'a') &&
        (NXT(6) == 'l') && (NXT(7) == 'o') &&
        (NXT(8) == 'n') && (NXT(9) == 'e')) 
    {
        SKIP(10);
        xmlSkipBlankChars(ctxt);
        if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '=') 
        {
            /*XML standalone declaration : expected '='*/
            ctxt->errNo = XML_ERR_EQUAL_REQUIRED;
            ctxt->wellFormed = 0;
            return(standalone);
        }
        xmlNextChar(ctxt);
        xmlSkipBlankChars(ctxt);
        if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '\''){
            xmlNextChar(ctxt);
            if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == 'n') && (NXT(1) == 'o')) {
                standalone = 0;
                SKIP(2);
            } else if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == 'y') && (NXT(1) == 'e') &&
                   (NXT(2) == 's')) 
            {
                standalone = 1;
                SKIP(3);
            } 
            else 
            {
                /*standalone accepts only 'yes' or 'no'*/
                ctxt->errNo = XML_ERR_STANDALONE_VALUE;
                ctxt->wellFormed = 0;
            }
            
            if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '\'') 
            {
                ctxt->errNo = XML_ERR_STRING_NOT_CLOSED;
                ctxt->wellFormed = 0;
            } 
            else
                xmlNextChar(ctxt);
        } 
        else if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '"')
        {
            xmlNextChar(ctxt);
            if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == 'n') 
                && (NXT(1) == 'o')) 
            {
                standalone = 0;
                SKIP(2);
            } 
            else if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == 'y') 
                && (NXT(1) == 'e') 
                && (NXT(2) == 's')) 
            {
                standalone = 1;
                SKIP(3);
            } 
            else 
            {
                /*standalone accepts only 'yes' or 'no'*/
                ctxt->errNo = XML_ERR_STANDALONE_VALUE;
                ctxt->wellFormed = 0;
            }
            if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '"') 
            {
                ctxt->wellFormed = 0;
                ctxt->errNo = XML_ERR_STRING_NOT_CLOSED;
            } 
            else
                xmlNextChar(ctxt);
        } 
        else 
        {
            /*Standalone value not found*/
            ctxt->wellFormed = 0;
            ctxt->errNo = XML_ERR_STRING_NOT_STARTED;
        }
    }
    return(standalone);
}

/**
* startDocument:
* @ctx: the user data (XML parser context)
*
* called when the document start being processed.
*/
XVOID startDocument(XVOID *ctx)
{
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;
    xmlDocPtr doc;
    
    doc = ctxt->myDoc = xmlNewDoc(ctxt->version);
    if (doc != XNULL) 
    {
        if (ctxt->encoding != XNULL)
            doc->encoding = xmlStrdup(ctxt->encoding);
        else
            doc->encoding = XNULL;
        doc->standalone = ctxt->standalone;
    }
}

/**
* comment:
* @ctx: the user data (XML parser context)
* @value:  the comment content
*
* A comment has been parsed.
*/
XVOID comment(XVOID *ctx, XCONST xmlChar *value)
{
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;
    xmlNodePtr ret;
    xmlNodePtr parent = ctxt->node;
    
    ret = xmlNewDocComment(ctxt->myDoc, value);
    if (ret == XNULL) 
        return;
    
    if (ctxt->myDoc->root == XNULL) 
    {
        ctxt->myDoc->root = ret;
    } 
    else if (parent == XNULL) 
    {
        parent = ctxt->myDoc->root;
    }
    if (parent != XNULL) 
    {
        if (parent->type == XML_ELEMENT_NODE) 
        {            
            xmlAddChild(parent, ret);
        } 
        else 
        {
            xmlAddSibling(parent, ret);
        }
    }
}

/**
* xmlNewInputStream:
* @ctxt:  an XML parser context
*
* Create a new input stream structure
* Returns the new input stream or XNULL
*/
xmlParserInputPtr xmlNewInputStream(xmlParserCtxtPtr ctxt) 
{
    xmlParserInputPtr input;
    
    input = (xmlParserInputPtr) xmlMalloc(sizeof(xmlParserInput));
    if (input == XNULL) 
    {
        ctxt->errNo = XML_ERR_NO_MEMORY;
        
        /*alloc: couldn't allocate a new input stream*/
        
        ctxt->errNo = XML_ERR_NO_MEMORY;
        return(XNULL);
    }
    input->filename = XNULL;
    input->directory = XNULL;
    input->base = XNULL;
    input->cur = XNULL;
    input->buf = XNULL;
    input->line = 1;
    input->col = 1;
    input->buf = XNULL;
    input->free = XNULL;
    input->consumed = 0;
    input->length = 0;
    /* 2.3.5 */
    input->encoding = XNULL;
    input->version = XNULL;
    return(input);
}
/**
* xmlFreeInputStream:
* @input:  an xmlParserInputPtr
*
* Free up an input stream.
*/
XVOID xmlFreeInputStream(xmlParserInputPtr input) 
{
    if (input == XNULL) 
        return;
    
    if (input->filename != XNULL) 
        xmlFree((XS8 *) input->filename);
    
    if (input->directory != XNULL) 
        xmlFree((XS8 *) input->directory);
    
    if ((input->free != XNULL) && (input->base != XNULL))
        input->free((xmlChar *) input->base);
    
    if (input->buf != XNULL) 
        xmlFreeParserInputBuffer(input->buf);
    
    /* 2.3.5 */
    if (input->encoding != XNULL) 
        xmlFree((XS8 *) input->encoding);
    
    if (input->version != XNULL) 
        xmlFree((XS8 *) input->version);
    
    XOS_MemSet(input, -1, sizeof(xmlParserInput));
    
    xmlFree(input);
}

/**
* xmlOldParseAttribute:
* @ctxt:  an XML parser context
* @value:  a xmlChar ** used to store the value of the attribute
*
* parse an attribute
*
* [41] Attribute ::= Name Eq AttValue
*
* [ WFC: No External Entity References ]
* Attribute values cannot contain direct or indirect entity references
* to external entities.
*
* [ WFC: No < in Attribute Values ]
* The replacement text of any entity referred to directly or indirectly in
* an attribute value (other than "&lt;") must not contain a <. 
* 
* [ VC: Attribute Value Type ]
* The attribute must have been declared; the value must be of the type
* declared for it.
*
* [25] Eq ::= S? '=' S?
*
* With namespace:
*
* [NS 11] Attribute ::= QName Eq AttValue
*
* Also the case QName == xmlns:??? is handled independently as a namespace
* definition.
*
* Returns the attribute name, and the value in *value.
*/
XSTATIC xmlChar *xmlOldParseAttribute(xmlParserCtxtPtr ctxt, xmlChar **value) 
{
    xmlChar *name, *val;
    
    *value = XNULL;
    name = xmlOldParseName(ctxt);
    if (name == XNULL) 
    {
        /*error parsing attribute name*/
        ctxt->wellFormed = 0;
        ctxt->errNo = XML_ERR_NAME_REQUIRED;
        return(XNULL);
    }
    
    /*
    * read the value
    */
    xmlSkipBlankChars(ctxt);
    if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '=') 
    {
        xmlNextChar(ctxt);
        xmlSkipBlankChars(ctxt);
        val = xmlOldParseAttValue(ctxt);
        ctxt->instate = XML_PARSER_CONTENT;
    } 
    else 
    {
        /*Specification mandate value for attribute %s*/
        ctxt->errNo = XML_ERR_ATTRIBUTE_WITHOUT_VALUE;
        ctxt->wellFormed = 0;
        return(XNULL);
    }
    
    *value = val;
    return(name);
}

/**
* xmlOldParseDocument :
* @ctxt:  an XML parser context
* 
* parse an XML document (and build a tree if using the standard SAX
* interface).
*
* [1] document ::= prolog element Misc*
*
* [22] prolog ::= XMLDecl? Misc* (doctypedecl Misc*)?
*
* Returns 0, -1 in case of error. the parser context is augmented
*                as a result of the parsing.
*/
XS32 xmlOldParseDocument(xmlParserCtxtPtr ctxt) 
{    
    if ( (ctxt->input->end - ctxt->input->cur) < INPUT_CHUNK) 
    {
        xmlParserInputGrow(ctxt->input, INPUT_CHUNK);
        if ((*ctxt->input->cur == 0) 
            && (xmlParserInputGrow(ctxt->input, INPUT_CHUNK) <= 0))
        {
            xmlPopInput(ctxt);
        }
    }    
    ctxt->pedantic = 0; /* we run the old 1.8.11 parser */    
    
                        /*
                        * Wipe out everything which is before the first '<'
    */
    if (IS_BLANK((ctxt->token ? ctxt->token : (*ctxt->input->cur)))) 
    {
        /*Extra spaces at the beginning of the document are not allowed*/
        
        ctxt->errNo = XML_ERR_DOCUMENT_START;
        ctxt->wellFormed = 0;
        xmlSkipBlankChars(ctxt);
    }
    
    if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == 0) 
    {
        /*Document is empty*/        
        ctxt->errNo = XML_ERR_DOCUMENT_EMPTY;
        ctxt->wellFormed = 0;
    }
    
    /*
    * Check for the XMLDecl in the Prolog.
    */
    GROW;
    
    if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '<') 
        && (NXT(1) == '?') 
        && (NXT(2) == 'x') 
        && (NXT(3) == 'm') 
        && (NXT(4) == 'l') 
        && (IS_BLANK(NXT(5)))) 
    {
        xmlOldParseXMLDecl(ctxt);
        xmlSkipBlankChars(ctxt);
    } 
    else if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '<') 
        && (NXT(1) == '?') 
        && (NXT(2) == 'X') 
        && (NXT(3) == 'M') 
        && (NXT(4) == 'L') 
        && (IS_BLANK(NXT(5)))) 
    {
    /*
    * The first drafts were using <?XML and the final W3C REC
    * now use <?xml ...
        */
        xmlOldParseXMLDecl(ctxt);
        xmlSkipBlankChars(ctxt);
    } 
    else 
    {
        ctxt->version = xmlCharStrdup(XML_DEFAULT_VERSION);
    }
    
    startDocument(ctxt->userData);
    
    /*
    * The Misc part of the Prolog
    */
    GROW;
    
    /*
    * Time to start parsing the tree itself
    */
    GROW;
    if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '<') 
    {
        /*Start tag expect, '<' not found*/
        
        ctxt->errNo = XML_ERR_DOCUMENT_EMPTY;
        ctxt->wellFormed = 0;
        ctxt->instate = XML_PARSER_EOF;
    } 
    else 
    {
        ctxt->instate = XML_PARSER_CONTENT;
        xmlOldParseElement(ctxt);
        ctxt->instate = XML_PARSER_EPILOG;
        
        if (ctxt->token ? ctxt->token : ((*ctxt->input->cur) != 0 && !(IS_BLANK(*ctxt->input->cur) )))
        {
            /*Extra content at the end of the document*/
            
            ctxt->wellFormed = 0;
            ctxt->errNo = XML_ERR_DOCUMENT_END;
        }
        ctxt->instate = XML_PARSER_EOF;
    }
    
    if (! ctxt->wellFormed) 
        return(-1);
    
    return(0);
    
}
/**
* xmlOldParseCharData:
* @ctxt:  an XML parser context
* @cdata:  XS32 indicating whether we are within a CDATA section
*
* parse a CharData section.
* if we are within a CDATA section ']]>' marks an end of section.
*
* [14] CharData ::= [^<&]* - ([^<&]* ']]>' [^<&]*)
*/
XSTATIC XVOID xmlOldParseCharData(xmlParserCtxtPtr ctxt, XS32 cdata) 
{
    xmlChar buf[XML_PARSER_BIG_BUFFER_SIZE];
    XS32 nbchar = 0;
    xmlChar cur;
    
    SHRINK;
    cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
    while (((cur != '<') || (ctxt->token == '<')) &&
        ((cur != '&') || (ctxt->token == '&'))&& 
        IS_CHAR(cur)) 
    {
        if ((cur == ']') && (NXT(1) == ']') &&
            (NXT(2) == '>')) 
        {
            if (cdata) 
                break;
            else 
            {
                /*Sequence ']]>' not allowed in content*/
                ctxt->errNo = XML_ERR_MISPLACED_CDATA_END;
            }
        }
        buf[nbchar++] = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
        if (nbchar == XML_PARSER_BIG_BUFFER_SIZE) 
        {
            if (areBlanksOld(ctxt, buf, nbchar)) 
            {
            } 
            else 
            {
                characters(ctxt->userData, buf, nbchar);
            }
            
            nbchar = 0;
        }
        
        xmlNextChar(ctxt);
        
        cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
    }
    
    if (nbchar != 0) 
    {
    /*
    * Ok the segment is to be consumed as chars.
        */
        if (areBlanksOld(ctxt, buf, nbchar)) 
        {
        } 
        else 
        {
            characters(ctxt->userData, buf, nbchar);
        }
    }
}

/**
* xmlOldParseXMLDecl:
* @ctxt:  an XML parser context
* 
* parse an XML declaration header
*
* [23] XMLDecl ::= '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
*/
XSTATIC XVOID xmlOldParseXMLDecl(xmlParserCtxtPtr ctxt) 
{
    xmlChar *version;
    /*
    * We know that '<?xml' is here.
    */
    SKIP(5);
    
    if (!IS_BLANK((ctxt->token ? ctxt->token : (*ctxt->input->cur)))) 
    {
        /*Blank needed after '<?xml'*/
        ctxt->errNo = XML_ERR_SPACE_REQUIRED;
        ctxt->wellFormed = 0;
    }
    xmlSkipBlankChars(ctxt);
    
    /*
    * We should have the VersionInfo here.
    */
    version = xmlOldParseVersionInfo(ctxt);
    if (version == XNULL)
        version = xmlCharStrdup(XML_DEFAULT_VERSION);
    
    ctxt->version = xmlStrdup(version);
    xmlFree(version);
    
    /*
    * We may have the encoding declaration
    */
    if (!IS_BLANK((ctxt->token ? ctxt->token : (*ctxt->input->cur)))) 
    {
        if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '?') && (NXT(1) == '>')) 
        {
            SKIP(2);
            return;
        }
        
        /*Blank needed here*/        
        ctxt->errNo = XML_ERR_SPACE_REQUIRED;
        ctxt->wellFormed = 0;
        
    }
    ctxt->encoding = xmlOldParseEncodingDecl(ctxt);
    
    /*
    * We may have the standalone status.
    */
    if ((ctxt->encoding != XNULL) && (!IS_BLANK((ctxt->token ? ctxt->token : (*ctxt->input->cur))))) 
    {
        if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '?') && (NXT(1) == '>')) 
        {
            SKIP(2);
            return;
        }
        /*Blank needed here*/
        
        ctxt->wellFormed = 0;
        ctxt->errNo = XML_ERR_SPACE_REQUIRED;
    }
    xmlSkipBlankChars(ctxt);
    ctxt->standalone = xmlOldParseSDDecl(ctxt);
    
    xmlSkipBlankChars(ctxt);
    if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '?') && (NXT(1) == '>')) 
    {
        SKIP(2);
    } 
    else if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '>') 
    {
        /* Deprecated old WD ... */
        /*XML declaration must end-up with '?>'*/
        
        ctxt->wellFormed = 0;
        ctxt->errNo = XML_ERR_XMLDECL_NOT_FINISHED;
        xmlNextChar(ctxt);
    } 
    else 
    {
        /*parsing XML declaration: '?>' expected*/
        ctxt->wellFormed = 0;
        ctxt->errNo = XML_ERR_XMLDECL_NOT_FINISHED;
        MOVETO_ENDTAG(ctxt->input->cur);
        xmlNextChar(ctxt);
    }
}


/**
* xmlNextChar:
* @ctxt:  the XML parser context
*
* Skip to the next XS8 input XS8.
*/
XVOID xmlNextChar(xmlParserCtxtPtr ctxt) 
{
    if (ctxt->instate == XML_PARSER_EOF) 
    {
        ctxt->token = -1;
        return;
    }
    
    /*
    *   2.11 End-of-Line Handling
    *   the literal two-character sequence "#xD#xA" or a standalone
    *   literal #xD, an XML processor must pass to the application
    *   the single character #xA. 
    */
    if (ctxt->token != 0) 
    {
        ctxt->token = 0;
    }
    else if (ctxt->charset == XML_CHAR_ENCODING_UTF8) 
    {
        if ((*ctxt->input->cur == 0) &&
            (xmlParserInputGrow(ctxt->input, INPUT_CHUNK) <= 0) &&
            (ctxt->instate != XML_PARSER_COMMENT)) 
        {
        /*
        * If we are at the end of the current entity and
        * the context allows it, we pop consumed entities
        * automatically.
        * the auto closing should be blocked in other cases
            */
            xmlPopInput(ctxt);
        } 
        else 
        {
            if (*(ctxt->input->cur) == '\n') 
            {
                ctxt->input->line++; ctxt->input->col = 1;
            } 
            else 
                ctxt->input->col++;
            
            if (ctxt->charset == XML_CHAR_ENCODING_UTF8) 
            {
            /*
            * We are supposed to handle UTF8, check it's valid
            * From rfc2044: encoding of the Unicode values on UTF-8:
            *
            * UCS-4 range (hex.)           UTF-8 octet sequence (binary)
            * 0000 0000-0000 007F   0xxxxxxx
            * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
            * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx 
            *
            * Check for the 0x110000 limit too
                */
                const xmlChar *cur = ctxt->input->cur;
                XU8 c;
                
                c = *cur;
                /*utf-8的中文字符是变长字节，下面的处理只存储第一个字节，余下的跳过了，
                不符合next char的功能，不论单字节还是变长的多字节，都要挨个处理*/
                if(0)   //if (c & 0x80) 
                {
                    if (cur[1] == 0)
                        xmlParserInputGrow(ctxt->input, INPUT_CHUNK);
                    if ((cur[1] & 0xc0) != 0x80)
                        goto encoding_error;
                    if ((c & 0xe0) == 0xe0) 
                    {
                        XS32 val;
                        
                        if (cur[2] == 0)
                            xmlParserInputGrow(ctxt->input, INPUT_CHUNK);
                        if ((cur[2] & 0xc0) != 0x80)
                            goto encoding_error;
                        if ((c & 0xf0) == 0xf0) 
                        {
                            if (cur[3] == 0)
                                xmlParserInputGrow(ctxt->input, INPUT_CHUNK);
                            if (((c & 0xf8) != 0xf0) ||
                                ((cur[3] & 0xc0) != 0x80))
                                goto encoding_error;
                            /* 4-byte code */
                            ctxt->input->cur += 4;
                            val = (cur[0] & 0x7) << 18;
                            val |= (cur[1] & 0x3f) << 12;
                            val |= (cur[2] & 0x3f) << 6;
                            val |= cur[3] & 0x3f;
                        } 
                        else 
                        {
                            /* 3-byte code */
                            ctxt->input->cur += 3;
                            val = (cur[0] & 0xf) << 12;
                            val |= (cur[1] & 0x3f) << 6;
                            val |= cur[2] & 0x3f;
                        }
                        
                        if (((val > 0xd7ff) && (val < 0xe000)) ||
                            ((val > 0xfffd) && (val < 0x10000)) ||
                            (val >= 0x110000)) 
                        {
                            /*XS8 0x%X out of allowed range*/
                            goto encoding_error;
                        }    
                    } 
                    else
                        /* 2-byte code */
                        ctxt->input->cur += 2;
                } 
                else
                    /* 1-byte code */
                    ctxt->input->cur++;
            } 
            else 
            {
            /*
            * Assume it's a fixed lenght encoding (1) with
            * a compatibke encoding for the ASCII set, since
            * XML constructs only use < 128 chars
                */
                ctxt->input->cur++;
            }
            ctxt->nbChars++;
            if (*ctxt->input->cur == 0)
                xmlParserInputGrow(ctxt->input, INPUT_CHUNK);
        }
    } 
    else 
    {
        ctxt->input->cur++;
        ctxt->nbChars++;
        if (*ctxt->input->cur == 0)
            xmlParserInputGrow(ctxt->input, INPUT_CHUNK);
    }
    
    if ((*ctxt->input->cur == 0) &&
        (xmlParserInputGrow(ctxt->input, INPUT_CHUNK) <= 0))
        xmlPopInput(ctxt);
    
    return;
    
encoding_error:
/*
* If we detect an UTF8 error that probably mean that the
* input encoding didn't get properly advertized in the
* declaration header. Report the error and switch the encoding
* to ISO-Latin-1 (if you don't like this policy, just declare the
* encoding !)
    */
    
    /*Input is not proper UTF-8, indicate encoding !*/
    
    ctxt->charset = XML_CHAR_ENCODING_8859_1; 
    ctxt->input->cur++;
    return;
}



/************************************************************************
*                                    *
*        Content access functions                *
*                                    *
************************************************************************/ 

/**
* attribute:
* @ctx: the user data (XML parser context)
* @name:  The attribute name
* @value:  The attribute value
*
* Handle an attribute that has been read by the parser.
* The default handling is to convert the attribute into an
* DOM subtree and past it in a new xmlAttr element added to
* the element.
*/
XVOID attribute(XVOID *ctx, XCONST xmlChar *fullname, XCONST xmlChar *value)
{
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;
    xmlAttrPtr ret = XNULL;
    
    /* !!!!!! <a toto:arg="" xmlns:toto="http://toto.com"> */
    ret = xmlNewNsProp(ctxt->node, fullname, XNULL);
    
    if (ret != XNULL) 
    {
        if ((ctxt->replaceEntities == 0) /* && (!ctxt->html) */)
            ret->val = xmlStringGetNodeList(ctxt->myDoc, value);
        else
            ret->val = xmlNewDocText(ctxt->myDoc, value);
    }
    
    if ( ctxt->wellFormed && ctxt->myDoc )
    {
        ctxt->valid &= 0;
    } 
}
/**
* startElement:
* @ctx: the user data (XML parser context)
* @name:  The element name
* @atts:  An array of name/value attributes pairs, XNULL terminated
*
* called when an opening tag has been processed.
*/
XVOID startElement(XVOID *ctx, XCONST xmlChar *fullname, XCONST xmlChar **atts)
{
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;
    xmlNodePtr ret        = XNULL;
    xmlNodePtr parent     = ctxt->node;
    XCONST xmlChar *att   = XNULL;
    XCONST xmlChar *value = XNULL;
    XS32 i                = 0;
    
    /*
    * Note : the namespace resolution is deferred until the end of the
    *        attributes parsing, since local namespace can be defined as
    *        an attribute at this level.
    */
    ret = xmlNewDocNode(ctxt->myDoc, fullname, XNULL);
    if (ret == XNULL) 
        return;
    
    if (ctxt->myDoc->root == XNULL) 
    {        
        ctxt->myDoc->root = ret;
    } 
    else if (parent == XNULL) 
    {
        parent = ctxt->myDoc->root;
    }
    
    /*
    * We are parsing a new node.
    */
    nodePush(ctxt, ret);
    
    /*
    * Link the child element
    */
    if (parent != XNULL) 
    {
        if (parent->type == XML_ELEMENT_NODE) 
        {            
            xmlAddChild(parent, ret);
        } 
        else 
        {
            xmlAddSibling(parent, ret);
        }
    }
    
    /*
    * process all the attributes whose name start with "xml"
    */
    if (atts != XNULL) 
    {
        i = 0;
        att = atts[i++];
        value = atts[i++];
        while ((att != XNULL) && (value != XNULL)) 
        {
            if ((att[0] == 'x') && (att[1] == 'm') && (att[2] == 'l'))
                attribute(ctxt, att, value);
            
            att = atts[i++];
            value = atts[i++];
        }
    }
    
    /*
    * process all the other attributes
    */
    if (atts != XNULL) 
    {
        i = 0;
        att = atts[i++];
        value = atts[i++];
        while ((att != XNULL) && (value != XNULL)) 
        {
            if ((att[0] != 'x') || (att[1] != 'm') || (att[2] != 'l'))
                attribute(ctxt, att, value);
            
                /*
                * Next ones
            */
            att = atts[i++];
            value = atts[i++];
        }
    }
}
/**
* endElement:
* @ctx: the user data (XML parser context)
* @name:  The element name
*
* called when the end of an element has been detected.
*/
XVOID endElement(XVOID *ctx, XCONST xmlChar *name)
{
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;
    
    if ( XNULL == name )
    {
    }   
    
    /*
    * end of parsing of this node.
    */
    nodePop(ctxt);
}


/**
* characters:
* @ctx: the user data (XML parser context)
* @ch:  a xmlChar string
* @len: the number of xmlChar
*
* receiving some chars from the parser.
* Question: how much at a time ???
*/
XVOID characters(XVOID *ctx, XCONST xmlChar *ch, XS32 len)
{
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr)ctx;
    xmlNodePtr lastChild = XNULL;
    
    /*
    * Handle the data if any. If there is no child
    * add it as content, otherwise if the last child is text,
    * concatenate it, else create a new node of type text.
    */
    
    if (ctxt->node == XNULL) 
    {
        return;
    }
    lastChild = xmlGetLastChild(ctxt->node);
    
    if (lastChild == XNULL)
    {
        xmlNodeAddContentLen(ctxt->node, ch, len);
    }
    else 
    {
        if (xmlNodeIsText(lastChild))
        {
            xmlTextConcat(lastChild, ch, len);
        }
        else 
        {
            lastChild = xmlNewTextLen(ch, len);
            xmlAddChild(ctxt->node, lastChild);
        }
    }
}

/************************************************************************
*                                    *
*        Commodity functions to handle parser contexts        *
*                                    *
************************************************************************/
/**
* xmlInitParserCtxt:
* @ctxt:  an XML parser context
*
* Initialize a parser context
*/
XVOID xmlInitParserCtxt(xmlParserCtxtPtr ctxt)
{
    /* Allocate the Input stack */
    ctxt->inputTab = (xmlParserInputPtr *) xmlMalloc(5 * sizeof(xmlParserInputPtr));
    ctxt->inputNr = 0;
    ctxt->inputMax = 5;
    ctxt->input = XNULL;
    ctxt->version = XNULL;
    ctxt->encoding = XNULL;
    ctxt->standalone = -1;
    
    ctxt->hasPErefs = 0;
    ctxt->external = 0;
    ctxt->instate = XML_PARSER_START;
    ctxt->token = 0;
    ctxt->directory = XNULL;
    
    /* Allocate the Node stack */
    ctxt->nodeTab = (xmlNodePtr *) xmlMalloc(10 * sizeof(xmlNodePtr));
    ctxt->nodeNr = 0;
    ctxt->nodeMax = 10;
    ctxt->node = XNULL;
    
    /* Allocate the Name stack */
    ctxt->nameTab = (xmlChar **) xmlMalloc(10 * sizeof(xmlChar *));
    ctxt->nameNr = 0;
    ctxt->nameMax = 10;
    ctxt->name = XNULL;
    
    ctxt->userData = ctxt;
    ctxt->myDoc = XNULL;
    ctxt->wellFormed = 1;
    ctxt->valid = 1;
    ctxt->keepBlanks = xmlKeepBlanksDefaultValue;
    
    ctxt->replaceEntities = xmlSubstituteEntitiesDefaultValue;
    ctxt->nbChars = 0;
    ctxt->checkIndex = 0;
    ctxt->errNo = XML_ERR_OK;
    
    /* 2.3.5 */
    ctxt->charset = XML_CHAR_ENCODING_UTF8;
    ctxt->depth = 0;
    
}
/**
* xmlFreeParserCtxt:
* @ctxt:  an XML parser context
*
* Free all the memory used by a parser context. However the parsed
* document in ctxt->myDoc is not freed.
*/
XVOID xmlFreeParserCtxt(xmlParserCtxtPtr ctxt)
{
    xmlParserInputPtr input = XNULL;
    xmlChar *oldname = XNULL;
    
    if (ctxt == XNULL) 
        return;
    
    while ((input = inputPop(ctxt)) != XNULL) 
    {
        xmlFreeInputStream(input);
    }
    
    while ((oldname = namePop(ctxt)) != XNULL) 
    {
        xmlFree(oldname);
    }
    
    if (ctxt->nameTab != XNULL) 
        xmlFree(ctxt->nameTab);
    
    if (ctxt->nodeTab != XNULL) 
        xmlFree(ctxt->nodeTab);
    
    if (ctxt->inputTab != XNULL) 
        xmlFree(ctxt->inputTab);
    
    if (ctxt->version != XNULL) 
        xmlFree((XS8 *) ctxt->version);
    
    if (ctxt->encoding != XNULL) 
        xmlFree((XS8 *) ctxt->encoding);
    
    if (ctxt->directory != XNULL) 
        xmlFree((XS8 *) ctxt->directory);
    
    
    xmlFree(ctxt);
}




/**
* xmlIsMixedElement
* @doc:  the document
* @name:  the element name
*
* Search in the DtDs whether an element accept Mixed content (or ANY)
* basically if it is supposed to accept text childs
*
* returns 0 if no, 1 if yes, and -1 if no element description is available
*/
XS32 xmlIsMixedElement(xmlDocPtr doc, XCONST xmlChar *name) 
{
    if (doc == XNULL
        || XNULL == name ) 
        return(-1);    
    
    return(0);
}
/**
* xmlOldParseStartTag:
* @ctxt:  an XML parser context
* 
* parse a start of tag either for rule element or
* EmptyElement. In both case we don't parse the tag closing chars.
*
* [40] STag ::= '<' Name (S Attribute)* S? '>'
*
* [ WFC: Unique Att Spec ]
* No attribute name may appear more than once in the same start-tag or
* empty-element tag. 
*
* [44] EmptyElemTag ::= '<' Name (S Attribute)* S? '/>'
*
* [ WFC: Unique Att Spec ]
* No attribute name may appear more than once in the same start-tag or
* empty-element tag. 
*
* With namespace:
*
* [NS 8] STag ::= '<' QName (S Attribute)* S? '>'
*
* [NS 10] EmptyElement ::= '<' QName (S Attribute)* S? '/>'
*
* Returne the element name parsed
*/
XSTATIC xmlChar *xmlOldParseStartTag(xmlParserCtxtPtr ctxt) 
{
    xmlChar *name;
    xmlChar *attname;
    xmlChar *attvalue;
    xmlChar **atts = XNULL;
    XS32    nbatts = 0;
    XS32    maxatts = 0;
    XS32    i = 0;
    
    if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '<') 
        return(XNULL);

    /*root节点之前可能有多行注释，循环处理完所有的注释行*/
    while(1)
    {
        if ((RAW == '<') 
            && (NXT(1) == '!') 
            && (NXT(2) == '-') 
            && (NXT(3) == '-')) 
        {
            xmlOldParseRootComment(ctxt);
        }
        else
            break;
    }
    
    xmlNextChar(ctxt);    
    name = xmlOldParseName(ctxt);
    if (name == XNULL) 
    {
        /*xmlParseStartTag: invalid element name*/
        
        ctxt->errNo = XML_ERR_NAME_REQUIRED;
        ctxt->wellFormed = 0;
        return(XNULL);
    }
    
    /*
    * Now parse the attributes, it ends up with the ending
    *
    * (S Attribute)* S?
    */
    xmlSkipBlankChars(ctxt);
    GROW;
    
    while ((IS_CHAR((ctxt->token ? ctxt->token : (*ctxt->input->cur)))) &&
        ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '>') && 
        (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '/') || (NXT(1) != '>'))) 
    {
        XCONST xmlChar *q = ctxt->input->cur;
        XS32 cons = ctxt->input->consumed;
        
        attname = xmlOldParseAttribute(ctxt, &attvalue);
        if ((attname != XNULL) && (attvalue != XNULL)) 
        {
        /*
        * [ WFC: Unique Att Spec ]
        * No attribute name may appear more than once in the same
        * start-tag or empty-element tag. 
            */
            for (i = 0; i < nbatts;i += 2) 
            {
                if (!xmlStrcmp(atts[i], attname)) 
                {
                    /*Attribute %s redefined*/
                    ctxt->wellFormed = 0;
                    ctxt->errNo = XML_ERR_ATTRIBUTE_REDEFINED;
                    xmlFree(attname);
                    xmlFree(attvalue);
                    goto failed;
                }
            }
            
            /*
            * Add the pair to atts
            */
            if (atts == XNULL) 
            {
                maxatts = 10;
                atts = (xmlChar **) xmlMalloc(maxatts * sizeof(xmlChar *));
                if (atts == XNULL) 
                {
                    return(XNULL);
                }
            } 
            else if (nbatts + 4 > maxatts) 
            {
                maxatts *= 2;
                atts = (xmlChar **) xmlRealloc(atts,
                    maxatts * sizeof(xmlChar *));
                if (atts == XNULL) 
                {
                    return(XNULL);
                }
            }
            atts[nbatts++] = attname;
            atts[nbatts++] = attvalue;
            atts[nbatts] = XNULL;
            atts[nbatts + 1] = XNULL;
        }
        
failed:     
        xmlSkipBlankChars(ctxt);
        if ((cons == ctxt->input->consumed) && (q == ctxt->input->cur)) 
        {
            /*xmlParseStartTag: problem parsing attributes*/
            ctxt->errNo = XML_ERR_INTERNAL_ERROR;
            ctxt->wellFormed = 0;
            break;
        }
        GROW;
    }
    
    /*
    * SAX: Start of Element !
    */
    startElement( ctxt->userData, name,(XCONST xmlChar **) atts );
    
    if (atts != XNULL) 
    {
        for (i = 0;i < nbatts;i++) 
            xmlFree((xmlChar *) atts[i]);
        xmlFree(atts);
    }
    return(name);
}

/**
* xmlOldParseName:
* @ctxt:  an XML parser context
*
* parse an XML name.
*
* [4] NameChar ::= Letter | Digit | '.' | '-' | '_' | ':' |
*                  CombiningChar | Extender
*
* [5] Name ::= (Letter | '_' | ':') (NameChar)*
*
* [6] Names ::= Name (S Name)*
*
* Returns the Name parsed or XNULL
*/
XSTATIC xmlChar *xmlOldParseName(xmlParserCtxtPtr ctxt) 
{
    xmlChar buf[XML_MAX_NAMELEN];
    XS32 len = 0;
    xmlChar cur;
    
    GROW;
    cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
    if (!IS_LETTER(cur) && (cur != '_') &&
        (cur != ':')) 
    {
        return(XNULL);
    }
    
    while ((IS_LETTER(cur)) 
        || (IS_DIGIT(cur)) 
        || (cur == '.') 
        || (cur == '-') 
        || (cur == '_') 
        || (cur == ':') 
        || (IS_COMBINING(cur)) 
#ifdef UNICODE
        || (IS_EXTENDER(cur))
#endif
        ) 
    {
        buf[len++] = cur;
        
        xmlNextChar(ctxt);
        
        cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
        if (len >= XML_MAX_NAMELEN) 
        {
            while ((IS_LETTER(cur)) 
                || (IS_DIGIT(cur)) 
                || (cur == '.') 
                || (cur == '-') 
                || (cur == '_') 
                || (cur == ':') 
                || (IS_COMBINING(cur)) 
#ifdef UNICODE
                || (IS_EXTENDER(cur))
#endif
                ) 
            {
                xmlNextChar(ctxt);
                cur = (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur));
            }
            break;
        }
    }
    return(xmlStrndup(buf, len));
}


/**
* areBlanksOld:
* @ctxt:  an XML parser context
* @str:  a xmlChar *
* @len:  the size of @str
*
* Is this a sequence of blank chars that one can ignore ?
*
* Returns 1 if ignorable 0 otherwise.
*/
XSTATIC XS32 areBlanksOld(xmlParserCtxtPtr ctxt, XCONST xmlChar *str, XS32 len) 
{
    XS32 i, ret;
    xmlNodePtr lastChild;
    
    /*
    * Check that the string is made of blanks
    */
    for (i = 0;i < len;i++)
        if (!(IS_BLANK(str[i]))) 
            return(0);
        
            /*
            * Look if the element is mixed content in the Dtd if available
        */
        if (ctxt->myDoc != XNULL) 
        {
            /*node为空说明处理的是注释行，直接返回，否则下面的ctxt->node->name报段错误*/
            if(ctxt->node == NULL)
                return(1);
            ret = xmlIsMixedElement(ctxt->myDoc, ctxt->node->name);
            if (ret == 0) 
                return(1);
            if (ret == 1) 
                return(0);
        }
        
        /*
        * Do we allow an heuristic on white space
        */
        if (ctxt->keepBlanks)
            return(0);
        
        if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '<') 
            return(0);
        if (ctxt->node == XNULL) 
            return(0);
        
        if ((ctxt->node->children == XNULL) 
            && ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '<') 
            && (NXT(1) == '/')) 
            return(0);
        
        lastChild = xmlGetLastChild(ctxt->node);
        if (lastChild == XNULL) 
        {
            if (ctxt->node->content != XNULL) 
                return(0);
        } 
        else if (xmlNodeIsText(lastChild))
            return(0);
        else if ((ctxt->node->children != XNULL) &&
            (xmlNodeIsText(ctxt->node->children)))
            return(0);
        
        return(1);
}
/**
* xmlOldPopInput:
* @ctxt:  an XML parser context
*
* xmlOldPopInput: the current input pointed by ctxt->input came to an end
*          pop it and return the next XS8.
*
* Returns the current xmlChar in the parser context
*/
XSTATIC xmlChar xmlOldPopInput(xmlParserCtxtPtr ctxt) 
{
    if (ctxt->inputNr == 1) 
        return(0); /* End of main Input */
    
    xmlOldFreeInputStream(inputOldPop(ctxt));
    if ((*ctxt->input->cur == 0) 
        && (xmlOldParserInputGrow(ctxt->input, INPUT_CHUNK) <= 0))
        return(xmlOldPopInput(ctxt));
    
    return((xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur)));
}
/**
* xmlParseDocument:
* @ctxt:  an XML parser context
* 
* parse an XML document (and build a tree if using the standard SAX
* interface).
*
* [1] document ::= prolog element Misc*
*
* [22] prolog ::= XMLDecl? Misc* (doctypedecl Misc*)?
*
* Returns 0, -1 in case of error. the parser context is augmented
*                as a result of the parsing.
*/
XS32 xmlParseDocument(xmlParserCtxtPtr ctxt) 
{
    if (!xmlUseNewParserDefault)
        return(xmlOldParseDocument(ctxt));    
    
    return(0);
}


/**
* xmlOldParseContent:
* @ctxt:  an XML parser context
*
* Parse a content:
*
* [43] content ::= (element | CharData | Reference | CDSect | PI | Comment)*
*/
XSTATIC XVOID xmlOldParseContent(xmlParserCtxtPtr ctxt) 
{
    GROW;
    while (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '<') 
        || (NXT(1) != '/')) 
    {
        XCONST xmlChar *test = ctxt->input->cur;
        XS32 cons = ctxt->input->consumed;
        XS32 tok = ctxt->token;
        
        /*
        * Handle  possible processed charrefs.
        */
        if (ctxt->token != 0) 
        {
            xmlOldParseCharData(ctxt, 0);
        }
        /*
        * First case : a Processing Instruction.
        */
        else if ((RAW == '<') && (NXT(1) == '?')) 
        {
            xmlOldParsePI(ctxt);
        }        
        /*
        * Third case :  a comment
        */
        else if ((RAW == '<') 
            && (NXT(1) == '!') 
            && (NXT(2) == '-') 
            && (NXT(3) == '-')) 
        {
            xmlOldParseComment(ctxt);
            ctxt->instate = XML_PARSER_CONTENT;
        }        
        /*
        * Fourth case :  a sub-element.
        */
        else if (RAW == '<') 
        {
            xmlOldParseElement(ctxt);
        }        
        /*
        * Last case, text. Note that References are handled directly.
        */
        else 
        {
            xmlOldParseCharData(ctxt, 0);
        }
        
        GROW;
        /*
        * Pop-up of finished entities.
        */
        while ((RAW == 0) && (ctxt->inputNr > 1))
            xmlOldPopInput(ctxt);
        SHRINK;
        
        if ((cons == ctxt->input->consumed) 
            && (test == ctxt->input->cur) 
            && (tok == ctxt->token)) 
        {
            /*detected an error in element content*/
            
            ctxt->errNo = XML_ERR_INTERNAL_ERROR;
            ctxt->wellFormed = 0;
            break;
        }
    }
}

/************************************************************************
函数名:xmlOldParseRootComment
功能:  处理root节点之前的注释行
输入:
ctxt －xml结构块
输出:
返回: 
说明:只在分析root节点的xmlOldParseStartTag中调用
************************************************************************/
XSTATIC XVOID xmlOldParseRootComment(xmlParserCtxtPtr ctxt) 
{
    GROW;
    while (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) != '<') 
        || (NXT(1) != '/')) 
    {
        /*
        *  case :  a comment
        */
        if ((RAW == '<') 
            && (NXT(1) == '!') 
            && (NXT(2) == '-') 
            && (NXT(3) == '-')) 
        {
            xmlOldParseComment(ctxt);
            ctxt->instate = XML_PARSER_CONTENT;
        }
        else if (RAW == '<') 
        {
            break;
        }
        else 
        {
            xmlOldParseCharData(ctxt, 0);
            break;
        }
        
//        GROW;
    }
}
/**
* xmlOldParseElement:
* @ctxt:  an XML parser context
*
* parse an XML element, this is highly recursive
*
* [39] element ::= EmptyElemTag | STag content ETag
*
* [ WFC: Element Type Match ]
* The Name in an element's end-tag must match the element type in the
* start-tag. 
*
* [ VC: Element Valid ]
* An element is valid if there is a declaration matching elementdecl
* where the Name matches the element type and one of the following holds:
*  - The declaration matches EMPTY and the element has no content.
*  - The declaration matches children and the sequence of child elements
*    belongs to the language generated by the regular expression in the
*    content model, with optional white space (characters matching the
*    nonterminal S) between each pair of child elements. 
*  - The declaration matches Mixed and the content consists of character
*    data and child elements whose types match names in the content model. 
*  - The declaration matches ANY, and the types of any child elements have
*    been declared.
*/

XSTATIC XVOID xmlOldParseElement(xmlParserCtxtPtr ctxt) 
{
    xmlChar *name;
    xmlChar *oldname;
    
    name = xmlOldParseStartTag(ctxt);
    if (name == XNULL) 
    {
        return;
    }
    nameOldPush(ctxt, name);
    
    /*
    * Check for an Empty Element.
    */
    if (((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '/') 
        && (NXT(1) == '>')) 
    {
        SKIP(2);
        
        endElement(ctxt->userData, name);
        
        oldname = nameOldPop(ctxt);
        if (oldname != XNULL) 
        {
            xmlFree(oldname);
        }
        return;
    }
    
    if ((ctxt->token ? ctxt->token : (*ctxt->input->cur)) == '>') 
    {
        xmlNextChar( ctxt );
    } 
    else 
    {
        /*Couldn't find end of Start Tag\n%.30s*/
        ctxt->wellFormed = 0;
        ctxt->errNo = XML_ERR_GT_REQUIRED;
        
        /*
        * end of parsing of this node.
        */
        /*nodeOldPop(ctxt);*/
        oldname = nameOldPop(ctxt);
        if (oldname != XNULL) 
        {
            xmlFree(oldname);
        }        
        
        return;
    }
    
    /*
    * Parse the content of the element:
    */
    xmlOldParseContent(ctxt);
    if (!IS_CHAR((ctxt->token ? ctxt->token : (*ctxt->input->cur)))) 
    {
        /*Premature end of data in tag %.30s*/
        ctxt->wellFormed = 0;
        ctxt->errNo = XML_ERR_TAG_NOT_FINISHED;
        
        /*
        * end of parsing of this node.
        */
        /*nodeOldPop(ctxt);*/
        oldname = nameOldPop(ctxt);
        if (oldname != XNULL) 
        {
            xmlFree(oldname);
        }
        return;
    }
    
    /*
    * parse the end of tag: '</' should be here.
    */
    xmlOldParseEndTag(ctxt);
    
}
/**
* xmlParserInputShrink:
* @in:  an XML parser input
*
* This function removes used input for the parser.
*/
XVOID xmlParserInputShrink(xmlParserInputPtr in) 
{
    XS32 used;
    XS32 ret;
    XS32 index;
    
    
    if (in->buf == XNULL) 
        return;
    if (in->base == XNULL) 
        return;
    if (in->cur == XNULL) 
        return;
    if (in->buf->buffer == XNULL) 
        return;
    
    used = (XS32)(in->cur - in->buf->buffer->content);
    if (used > INPUT_CHUNK) 
    {
        ret = xmlBufferShrink(in->buf->buffer, used - LINE_LEN);
        if (ret > 0) 
        {
            in->cur -= ret;
            in->consumed += ret;
        }
        in->end = &in->buf->buffer->content[in->buf->buffer->use];
    }
    
    if (in->buf->buffer->use > INPUT_CHUNK) 
    {
        return;
    }
    xmlParserInputBufferRead(in->buf, 2 * INPUT_CHUNK);
    if (in->base != in->buf->buffer->content) 
    {
    /*
    * the buffer has been realloced
        */
        index = (XS32)(in->cur - in->base);
        in->base = in->buf->buffer->content;
        in->cur = &in->buf->buffer->content[index];
    }
    in->end = &in->buf->buffer->content[in->buf->buffer->use];
    
}
/**
* xmlNewParserCtxt:
*
* Allocate and initialize a new parser context.
*
* Returns the xmlParserCtxtPtr or XNULL
*/
xmlParserCtxtPtr xmlNewParserCtxt()
{
    xmlParserCtxtPtr ctxt;
    
    ctxt = (xmlParserCtxtPtr) xmlMalloc(sizeof(xmlParserCtxt));
    if (ctxt == XNULL) 
    {
    /* xmlNewParserCtxt : cannot allocate context 
        */
        return(XNULL);
    }
    XOS_MemSet(ctxt, 0, sizeof(xmlParserCtxt));
    xmlInitParserCtxt(ctxt);
    return(ctxt);
}

/**
* xmlSkipBlankChars:
* @ctxt:  the XML parser context
*
* skip all blanks character found at that point in the input streams.
* It pops up finished entities in the process if allowable at that point.
*
* Returns the number of space chars skipped
*/

XS32 xmlSkipBlankChars(xmlParserCtxtPtr ctxt) 
{
    XS32 cur = 0, res = 0;
    
    /*
    * It's Okay to use (ctxt->token ? ctxt->token : (*ctxt->input->cur))/NEXT here since all the blanks are on
    * the ASCII range.
    */
    do 
    {
        cur = (ctxt->token ? ctxt->token : (*ctxt->input->cur));
        while (IS_BLANK(cur)) 
        { 
            /* CHECKED tstblanks.xml */
            xmlNextChar(ctxt);
            cur = (ctxt->token ? ctxt->token : (*ctxt->input->cur));
            res++;
        }
        while ((cur == 0) 
            && (ctxt->inputNr > 1) 
            && (ctxt->instate != XML_PARSER_COMMENT)) 
        {
            xmlPopInput(ctxt);
            cur = (ctxt->token ? ctxt->token : (*ctxt->input->cur));
        }
        
        /* DEPR if (*ctxt->input->cur == '&') xmlParserHandleReference(ctxt); */
    } while (IS_BLANK(cur)); /* CHECKED tstblanks.xml */
    
    return(res);
}

/**
* xmlParserInputGrow:
* @in:  an XML parser input
* @len:  an indicative size for the lookahead
*
* This function increase the input for the parser. It tries to
* preserve pointers to the input buffer, and keep already read data
*
* Returns the number of xmlChars read, or -1 in case of error, 0 indicate the
* end of this entity
*/
XS32 xmlParserInputGrow(xmlParserInputPtr in, XS32 len) 
{
    XS32 ret   = 0;
    XS32 index = 0;
    
    if (   in->buf == XNULL
        || in->base == XNULL
        || in->cur == XNULL
        || in->buf->buffer == XNULL ) 
    {
        return(-1);
    }
    
    index = (XS32)(in->cur - in->base);
    if ( in->buf->buffer->use > (XU32)(index + INPUT_CHUNK) ) 
    {
        return(0);
    }
    if ( (in->buf->file != XNULL ) 
        || (in->buf->fd >= 0) )
    {
        ret = xmlParserInputBufferGrow(in->buf, len);
    }
    else
    {
        return(0);
    }
    
    /*
    * NOTE : in->base may be a "dandling" i.e. freed pointer in this
    *        block, but we use it really as an integer to do some
    *        pointer arithmetic. Insure will raise it as a bug but in
    *        that specific case, that's not !
    */
    if (in->base != in->buf->buffer->content) 
    {
    /*
    * the buffer has been realloced
        */
        index = (XS32)(in->cur - in->base);
        in->base = in->buf->buffer->content;
        in->cur = &in->buf->buffer->content[index];
    }
    in->end = &in->buf->buffer->content[in->buf->buffer->use];
    
    return(ret);
}
/**
* xmlPopInput:
* @ctxt:  an XML parser context
*
* xmlPopInput: the current input pointed by ctxt->input came to an end
*          pop it and return the next XS8.
*
* Returns the current xmlChar in the parser context
*/
xmlChar xmlPopInput(xmlParserCtxtPtr ctxt) 
{
    if (ctxt->inputNr == 1) 
        return(0); /* End of main Input */
    
    
    xmlFreeInputStream(inputPop(ctxt));
    
    if ((*ctxt->input->cur == 0) &&
        (xmlParserInputGrow(ctxt->input, INPUT_CHUNK) <= 0))
        return(xmlPopInput(ctxt));
    
    return( (xmlChar)(ctxt->token ? ctxt->token : (*ctxt->input->cur)));
}



/**
* xmlCreateFileParserCtxt :
* @filename:  the filename
*
* Create a parser context for a file content. 
* Automatic support for ZLIB/Compress compressed document is provided
* by default if found at compile-time.
*
* Returns the new parser context or XNULL
*/
xmlParserCtxtPtr xmlCreateFileParserCtxt(XCONST XS8 *filename)
{
    xmlParserCtxtPtr ctxt;
    xmlParserInputPtr inputStream;
    xmlParserInputBufferPtr buf;
    XS8 *directory = XNULL;
    
    buf = xmlParserInputBufferCreateFilename(filename);
    if (buf == XNULL) 
    {
        return(XNULL);
    }
    
    ctxt = xmlNewParserCtxt();
    if (ctxt == XNULL) 
    {
        return(XNULL);
    }
    
    inputStream = xmlNewInputStream(ctxt);
    if (inputStream == XNULL) 
    {
        xmlFreeParserCtxt(ctxt);
        return(XNULL);
    }
    
    inputStream->filename = xmlMemStrdup(filename);
    inputStream->buf = buf;
    inputStream->base = inputStream->buf->buffer->content;
    inputStream->cur = inputStream->buf->buffer->content;
    inputStream->end =
        &inputStream->buf->buffer->content[inputStream->buf->buffer->use];
    
    inputPush(ctxt, inputStream);
    if ((ctxt->directory == XNULL) && (directory == XNULL))
        directory = xmlParserGetDirectory(filename);
    if ((ctxt->directory == XNULL) && (directory != XNULL))
        ctxt->directory = directory;
    
    return(ctxt);
}

/**
* xmlSAXParseFile :
* @sax:  the SAX handler block
* @filename:  the filename
* @recovery:  work in recovery mode, i.e. tries to read no Well Formed
*             documents
*
* parse an XML file and build a tree. Automatic support for ZLIB/Compress
* compressed document is provided by default if found at compile-time.
* It use the given SAX function block to handle the parsing callback.
* If sax is XNULL, fallback to the default DOM tree building routines.
*
* Returns the resulting document tree
*/

xmlDocPtr xmlSAXParseFile(XCONST XS8 *filename, XS32 recovery) 
{
    xmlDocPtr ret;
    xmlParserCtxtPtr ctxt;
    XS8 *directory = XNULL;
    
    ctxt = xmlCreateFileParserCtxt(filename);
    if (ctxt == XNULL) 
        return(XNULL);
    
    if ((ctxt->directory == XNULL) && (directory == XNULL))
    {
        directory = xmlParserGetDirectory(filename);
    }
    if ((ctxt->directory == XNULL) && (directory != XNULL))
    {
        ctxt->directory = (XS8 *)xmlStrdup((xmlChar *)directory); /* !!!!!!! */
    }
    
    xmlParseDocument(ctxt);
    
    if ((ctxt->wellFormed) || recovery)
    {
        ret = ctxt->myDoc;
    }
    else 
    {
        ret = XNULL;
        xmlFreeDoc(ctxt->myDoc);
        ctxt->myDoc = XNULL;
    }
    
    xmlFreeParserCtxt(ctxt);
    
    return(ret);
}


/**
* xmlParseFile :
* @filename:  the filename
*
* parse an XML file and build a tree. Automatic support for ZLIB/Compress
* compressed document is provided by default if found at compile-time.
*
* Returns the resulting document tree
*/
xmlDocPtr xmlParseFile(XCONST XS8 *filename) 
{
    return( xmlSAXParseFile( filename, 0 ) );
}
/**
* xmlParseMemory :
*
* 
* 
*
* Returns the resulting document tree
*/
xmlDocPtr xmlParseMemory(const char *buffer, int size)
{
    return xmlSAXParseMemory(buffer, size);
}

XSTATIC xmlDocPtr xmlSAXParseMemory(const char *buffer, int size)
{
    xmlDocPtr ret;
    xmlParserCtxtPtr ctxt;

    ctxt = xmlCreateMemoryParserCtxt(buffer, size);
    if (ctxt == NULL) return(NULL);
    
    xmlParseDocument(ctxt);

    if (ctxt->wellFormed) ret = ctxt->myDoc;
    else {
       ret = NULL;
       xmlFreeDoc(ctxt->myDoc);
       ctxt->myDoc = NULL;
    }
    xmlFreeParserCtxt(ctxt);

    return(ret);
}

XSTATIC xmlParserCtxtPtr xmlCreateMemoryParserCtxt(const char *buffer, int size)
{
    xmlParserCtxtPtr ctxt;
    xmlParserInputPtr input;
    xmlParserInputBufferPtr buf;

    if (buffer == NULL)
        return(NULL);
    if (size <= 0)
        return(NULL);

    ctxt = xmlNewParserCtxt();
    if (ctxt == NULL)
        return(NULL);

    /* TODO: xmlParserInputBufferCreateStatic, requires some serious changes */
    buf = xmlParserInputBufferCreateMem(buffer, size, XML_CHAR_ENCODING_NONE);
    if (buf == NULL) {
        xmlFreeParserCtxt(ctxt);
        return(NULL);
    }

    input = xmlNewInputStream(ctxt);
    if (input == NULL) {
        xmlFreeParserInputBuffer(buf);
        xmlFreeParserCtxt(ctxt);
        return(NULL);
    }

    input->filename = NULL;
    input->buf = buf;
    input->base = input->buf->buffer->content;
    input->cur = input->buf->buffer->content;
    input->end = &input->buf->buffer->content[input->buf->buffer->use];

    inputPush(ctxt, input);
    return(ctxt);
}

void xmlCleanupParser()
{
}
#ifdef __cplusplus
}
#endif /* _ _cplusplus */

