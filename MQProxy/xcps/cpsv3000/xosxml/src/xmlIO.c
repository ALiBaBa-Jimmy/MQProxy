/*
 * xmlIO.c : implementation of the I/O interfaces used by the parser
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 *
 * 14 Nov 2000 ht - for VMS, truncated name of long functions to under 32 char
 */


#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#if defined(_WIN32_WCE)
#include <winnls.h> /* for CP_UTF8 */
#endif

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/* Figure a portable way to know if a file is a directory. */
#ifndef HAVE_STAT
#  ifdef HAVE__STAT
     /* MS C library seems to define stat and _stat. The definition
        is identical. Still, mapping them to each other causes a warning. */
#    ifndef _MSC_VER
#      define stat(x,y) _stat(x,y)
#    endif
#    define HAVE_STAT
#  endif
#else
#  ifdef HAVE__STAT
#    if defined(_WIN32) || defined (__DJGPP__) && !defined (__CYGWIN__)
#      define stat _stat
#    endif
#  endif
#endif
#ifdef HAVE_STAT
#  ifndef S_ISDIR
#    ifdef _S_ISDIR
#      define S_ISDIR(x) _S_ISDIR(x)
#    else
#      ifdef S_IFDIR
#        ifndef S_IFMT
#          ifdef _S_IFMT
#            define S_IFMT _S_IFMT
#          endif
#        endif
#        ifdef S_IFMT
#          define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#        endif
#      endif
#    endif
#  endif
#endif

#include "../inc/xmlparser.h"
#include "../inc/xmlIO.h"
#include "../inc/xmlstring.h"

/* #define VERBOSE_FAILURE */
/* #define DEBUG_EXTERNAL_ENTITIES */
/* #define DEBUG_INPUT */

#ifdef DEBUG_INPUT
#define MINLEN 40
#else
#define MINLEN 4000
#endif

/*
 * Input I/O callback sets
 */
typedef struct _xmlInputCallback {
    xmlInputMatchCallback matchcallback;
    xmlInputOpenCallback opencallback;
    xmlInputReadCallback readcallback;
    xmlInputCloseCallback closecallback;
} xmlInputCallback;

static int    xmlFileRead (void * context, char * buffer, int len);
static int xmlFileClose (void * context);

/**
 * xmlFileOpenW:
 * @filename:  the URI for matching
 *
 * output to from FILE *,
 * if @filename is "-" then the standard output is used
 *
 * Returns an I/O context or NULL in case of error
 */
static void * xmlFileOpenW (const char *filename) {
    const char *path = NULL;
    FILE *fd;

    if (!strcmp(filename, "-")) {
        fd = stdout;
        return((void *) fd);
    }

    if (!xmlStrncasecmp(BAD_CAST filename, BAD_CAST "file://localhost/", 17))
#if defined (_WIN32) || defined (__DJGPP__) && !defined(__CYGWIN__)
        path = &filename[17];
#else
        path = &filename[16];
#endif
    else if (!xmlStrncasecmp(BAD_CAST filename, BAD_CAST "file:///", 8)) {
#if defined (_WIN32) || defined (__DJGPP__) && !defined(__CYGWIN__)
        path = &filename[8];
#else
        path = &filename[7];
#endif
    } else
        path = filename;

    if (path == NULL)
        return(NULL);

     fd = fopen(path, "wb");
    return((void *) fd);
}

/**
 * xmlFileRead:
 * @context:  the I/O context
 * @buffer:  where to drop data
 * @len:  number of bytes to write
 *
 * Read @len bytes to @buffer from the I/O channel.
 *
 * Returns the number of bytes written or < 0 in case of failure
 */
int
xmlFileRead (void * context, char * buffer, int len) {
    int ret;
    if ((context == NULL) || (buffer == NULL))
        return(-1);
    ret = (XS32)fread(&buffer[0], 1,  len, (FILE *) context);
    return(ret);
}

/**
 * xmlFileWrite:
 * @context:  the I/O context
 * @buffer:  where to drop data
 * @len:  number of bytes to write
 *
 * Write @len bytes from @buffer to the I/O channel.
 *
 * Returns the number of bytes written
 */
static int
xmlFileWrite (void * context, const char * buffer, int len) {
    int items;

    if ((context == NULL) || (buffer == NULL))
        return(-1);
    items = (XS32)fwrite(&buffer[0], len, 1, (FILE *) context);
    if ((items == 0) && (ferror((FILE *) context))) {
        return(-1);
    }
    return(items * len);
}

/**
 * xmlFileClose:
 * @context:  the I/O context
 *
 * Close an I/O channel
 *
 * Returns 0 or -1 in case of error
 */
int
xmlFileClose (void * context) {
    FILE *fil;
    int ret;

    if (context == NULL)
        return(-1);
    fil = (FILE *) context;
    if ((fil == stdout) || (fil == stderr)) {
        ret = fflush(fil);
        return(0);
    }
    if (fil == stdin)
        return(0);
    ret = ( fclose((FILE *) context) == EOF ) ? -1 : 0;
    return(ret);
}

/**
 * xmlFileFlush:
 * @context:  the I/O context
 *
 * Flush an I/O channel
 */
static int
xmlFileFlush (void * context) {
    int ret;

    if (context == NULL)
        return(-1);
    ret = ( fflush((FILE *) context) == EOF ) ? -1 : 0;
    return(ret);
}


/**
* xmlAllocParserInputBuffer:
* @enc:  the charset encoding if known
*
* Create a buffered parser input for progressive parsing
*
* Returns the new parser input or XNULL
*/
xmlParserInputBufferPtr xmlAllocParserInputBuffer() 
{
    xmlParserInputBufferPtr ret;
    
    ret = (xmlParserInputBufferPtr) xmlMalloc(sizeof(xmlParserInputBuffer));
    if (ret == XNULL) 
    {
        return(XNULL);
    }
    
    XOS_MemSet(ret, 0, (size_t)sizeof(xmlParserInputBuffer));
    
    ret->buffer = xmlBufferCreate();
    if (ret->buffer == XNULL) 
    {
        xmlFree(ret);
        return(XNULL);
    }
    ret->buffer->alloc = XML_BUFFER_ALLOC_DOUBLEIT;
    ret->fd = -1;
    /* 2.3.5 */
    ret->raw = XNULL;
    
    return(ret);
}

/**
 * xmlAllocOutputBuffer:
 * @encoder:  the encoding converter or NULL
 *
 * Create a buffered parser output
 *
 * Returns the new parser output or NULL
 */
xmlOutputBufferPtr xmlAllocOutputBuffer(xmlCharEncodingHandlerPtr encoder) {
    xmlOutputBufferPtr ret;

    ret = (xmlOutputBufferPtr) xmlMalloc(sizeof(xmlOutputBuffer));
    if (ret == NULL) {
        return(NULL);
    }
    memset(ret, 0, (size_t) sizeof(xmlOutputBuffer));
    ret->buffer = xmlBufferCreate();
    if (ret->buffer == NULL) {
        xmlFree(ret);
        return(NULL);
    }

    /* try to avoid a performance problem with Windows realloc() */
    if (ret->buffer->alloc == XML_BUFFER_ALLOC_EXACT)
        ret->buffer->alloc = XML_BUFFER_ALLOC_DOUBLEIT;

    ret->encoder = encoder;
    if (encoder != NULL) {
        ret->conv = xmlBufferCreateSize(4000);
        if (ret->conv == NULL) {
            xmlFree(ret);
            return(NULL);
        }

        /*
         * This call is designed to initiate the encoder state
         */
        xmlCharEncOutFunc(encoder, ret->conv, NULL);
    } else {
        ret->conv = NULL;
    }
    ret->writecallback = NULL;
    ret->closecallback = NULL;
    ret->context = NULL;
    ret->written = 0;

    return(ret);
}

/**
 * xmlAllocOutputBufferInternal:
 * @encoder:  the encoding converter or NULL
 *
 * Create a buffered parser output
 *
 * Returns the new parser output or NULL
 */
xmlOutputBufferPtr
xmlAllocOutputBufferInternal(xmlCharEncodingHandlerPtr encoder) {
    xmlOutputBufferPtr ret;

    ret = (xmlOutputBufferPtr) xmlMalloc(sizeof(xmlOutputBuffer));
    if (ret == NULL) {
        return(NULL);
    }
    memset(ret, 0, (size_t) sizeof(xmlOutputBuffer));
    ret->buffer = xmlBufferCreate();
    if (ret->buffer == NULL) {
        xmlFree(ret);
        return(NULL);
    }


    /*
     * For conversion buffers we use the special IO handling
     * We don't do that from the exported API to avoid confusing
     * user's code.
     */
    ret->buffer->alloc = XML_BUFFER_ALLOC_IO;
    ret->buffer->content = ret->buffer->content;

    ret->encoder = encoder;
    if (encoder != NULL) {
        ret->conv = xmlBufferCreateSize(4000);
    if (ret->conv == NULL) {
        xmlFree(ret);
        return(NULL);
    }

    /*
     * This call is designed to initiate the encoder state
     */
    xmlCharEncOutFunc(encoder, ret->conv, NULL);
    } else
        ret->conv = NULL;
    ret->writecallback = 0;
    ret->closecallback = NULL;
    ret->context = NULL;
    ret->written = 0;

    return(ret);
}


/**
* xmlFreeParserInputBuffer:
* @in:  a buffered parser input
*
* Free up the memory used by a buffered parser input
*/
XVOID xmlFreeParserInputBuffer(xmlParserInputBufferPtr in) 
{
    if (in->buffer != XNULL) 
    {
        xmlBufferFree(in->buffer);
        in->buffer = XNULL;
    }
    
    if (in->fd >= 0)
    {
        close(in->fd);
    }
    /* 2.3.5 */
    if (in->raw) 
    {
        xmlBufferFree(in->raw);
        in->raw = XNULL;
    }
    XOS_MemSet(in, 0xbe, (size_t) sizeof(xmlParserInputBuffer));
    xmlFree(in);
}

/**
 * xmlOutputBufferClose:
 * @out:  a buffered output
 *
 * flushes and close the output I/O channel
 * and free up all the associated resources
 *
 * Returns the number of byte written or -1 in case of error.
 */
int
xmlOutputBufferClose(xmlOutputBufferPtr out)
{
    int written;
    int err_rc = 0;

    if (out == NULL)
        return (-1);
    if (out->writecallback != NULL) {
        //xmlOutputBufferFlush(out);
        out->written = out->writecallback(out->context, (const char*)out->buffer->content, out->buffer->use);
    }
    if (out->closecallback != NULL) {
        err_rc = out->closecallback(out->context);
    }
    
    written = out->written;
    if (out->conv) {
        xmlBufferFree(out->conv);
        out->conv = NULL;
    }
    if (out->encoder != NULL) {
        xmlCharEncCloseFunc(out->encoder);
    }
    if (out->buffer != NULL) {
        xmlBufferFree(out->buffer);
        out->buffer = NULL;
    }

    if (out->error)
        err_rc = -1;
    xmlFree(out);
    return ((err_rc == 0) ? written : err_rc);
}


/**
* xmlParserInputBufferCreateFilename:
* @filename:  a C string containing the filename
* @enc:  the charset encoding if known
*
* Create a buffered parser input for the progressive parsing of a file
* If filename is "-' then we use stdin as the input.
* Automatic support for ZLIB/Compress compressed document is provided
* by default if found at compile-time.
*
* Returns the new parser input or NULL
*/
xmlParserInputBufferPtr xmlParserInputBufferCreateFilename( const char *filename ) 
{
    xmlParserInputBufferPtr ret;
    int  input   = -1;
    
    if (filename == NULL) 
    {
        return(NULL);
    }    
    
#ifdef WIN32
    input = _open(filename, O_RDONLY | _O_BINARY);
#else
    input = open( filename, O_RDONLY, 0644 );
#endif
    if (input < 0) 
    {
        return(NULL);
    }
    
    
    ret = xmlAllocParserInputBuffer( );
    if (ret != NULL) 
    {
        ret->fd    = input;
    }
    xmlParserInputBufferRead( ret, 4 );
    
    return(ret);
}

/**
 * xmlParserInputBufferCreateFile:
 * @file:  a FILE*
 * @enc:  the charset encoding if known
 *
 * Create a buffered parser input for the progressive parsing of a FILE *
 * buffered C I/O
 *
 * Returns the new parser input or NULL
 */
xmlParserInputBufferPtr
xmlParserInputBufferCreateFile(FILE *file, xmlCharEncoding enc) {
    xmlParserInputBufferPtr ret;

    if (file == NULL) return(NULL);

    ret = xmlAllocParserInputBuffer(enc);
    if (ret != NULL) {
        ret->file = file;
        ret->readcallback = xmlFileRead;
        ret->closecallback = xmlFileFlush;
    }

    return(ret);
}


/**
 * xmlParserInputBufferGrow:
 * @in:  a buffered parser input
 * @len:  indicative value of the amount of chars to read
 *
 * Grow up the content of the input buffer, the old data are preserved
 * This routine handle the I18N transcoding to internal UTF-8
 * This routine is used when operating the parser in normal (pull) mode
 *
 * TODO: one should be able to remove one extra copy by copying directly
 *       onto in->buffer or in->raw
 *
 * Returns the number of chars read and stored in the buffer, or -1
 *         in case of error.
 */
XS32 xmlParserInputBufferGrow(xmlParserInputBufferPtr in, XS32 len)
{
    XS8 *buffer = XNULL;
    XS32 res = 0;
    XS32 nbchars = 0;
    XS32 buffree;
    
    if ((len <= MINLEN) && (len != 4)) 
        len = MINLEN;
    buffree = in->buffer->size - in->buffer->use;
    if (buffree <= 0) 
    {
        return(0);
    }
    if (len > buffree) 
        len = buffree;
    
    buffer = xmlMalloc((len + 1) * sizeof(XS8));
    if (buffer == XNULL) 
    {
        /* xmlParserInputBufferGrow : out of memory ! */
        return(-1);
    }
    
    if (in->file != XNULL) 
    {
        res = (XS32)fread(&buffer[0], 1, len, in->file);
    } 
    else if (in->fd >= 0) 
    {
        res = read(in->fd, &buffer[0], len);
    } 
    else 
    {
        /* xmlParserInputBufferGrow : no input ! */
        xmlFree(buffer);
        return(-1);
    }
    
    if (res == 0) 
    {
        xmlFree(buffer);
        return(0);
    }
    
    if (res < 0) 
    {
        /* read error */
        xmlFree(buffer);
        return(-1);
    }
    
    nbchars = res;
    buffer[nbchars] = 0;
    xmlBufferAdd(in->buffer, (xmlChar *) buffer, nbchars);
    
    xmlFree(buffer);
    return(nbchars);
}

/**
 * xmlParserInputBufferRead:
 * @in:  a buffered parser input
 * @len:  indicative value of the amount of chars to read
 *
 * Refresh the content of the input buffer, the old data are considered
 * consumed
 * This routine handle the I18N transcoding to internal UTF-8
 *
 * Returns the number of chars read and stored in the buffer, or -1
 *         in case of error.
 */
XS32 xmlParserInputBufferRead(xmlParserInputBufferPtr in, XS32 len)
{
    /* xmlBufferEmpty(in->buffer); */
    if ( (in->file   != NULL ) 
        || (in->fd     >= 0    ))
    {
        return(xmlParserInputBufferGrow(in, len));
    }
    else
        return(0);
}


/*
* xmlParserGetDirectory:
* @filename:  the path to a file
*
* lookup the directory for that file
*
* Returns a new allocated string containing the directory, or XNULL.
*/
XS8 * xmlParserGetDirectory(XCONST XS8 *filename) 
{
    XS8 *ret = XNULL;
    XS8 dir[1024] = {0};
    XS8 *cur = XNULL;
    XS8 sep = '/';
    
    if (filename == XNULL) return(XNULL);
    
#ifdef XOS_WIN32
    sep = '\\';
#endif
    
    XOS_StrNcpy(dir, filename, 1023);
    dir[1023] = 0;
    cur = &dir[XOS_StrLen(dir)];
    while (cur > dir) 
    {
        if (*cur == sep) break;
        cur --;
    }
    if (*cur == sep) 
    {
        if (cur == dir) dir[1] = 0;
        else *cur = 0;
        ret = xmlMemStrdup(dir);
    } 
    else 
    {
        if ( getcwd( dir, 1024) != XNULL) 
        {
            dir[1023] = 0;
            ret = xmlMemStrdup(dir);
        }
    }
    return(ret);
}

int xmlOutputBufferWrite(xmlOutputBufferPtr out, int len, const char *buf) {
    int nbchars = 0; /* number of chars to output to I/O */
    int ret;         /* return from function call */
    int written = 0; /* number of char written to I/O so far */
    int chunk;       /* number of byte curreent processed from buf */

    if ((out == NULL) || (out->error)) return(-1);
    if (len < 0) return(0);
    if (out->error) return(-1);

    do {
        chunk = len;
        if (chunk > 4 * MINLEN)
            chunk = 4 * MINLEN;

        /*
         * first handle encoding stuff.
         */
        if (out->encoder != NULL) {
            /*
             * Store the data in the incoming raw buffer
             */
            if (out->conv == NULL) {
                out->conv = xmlBufferCreate();
            }
            xmlBufferAdd(out->buffer, (const xmlChar *) buf, chunk);

            if ((out->buffer->use < MINLEN) && (chunk == len))
                goto done;

            /*
             * convert as much as possible to the parser reading buffer.
             */
            ret = xmlCharEncOutFunc(out->encoder, out->conv, out->buffer);
            if ((ret < 0) && (ret != -3)) {
                return(-1);
            }
            nbchars = out->conv->use;
        } else {
            xmlBufferAdd(out->buffer, (const xmlChar *) buf, chunk);
            nbchars = out->buffer->use;
        }
        buf += chunk;
        len -= chunk;

        if ((nbchars < MINLEN) && (len <= 0))
            goto done;

        if (out->writecallback) {
            /*
             * second write the stuff to the I/O channel
             */
            if (out->encoder != NULL) {
                ret = out->writecallback(out->context,
                         (const char *)out->conv->content, nbchars);
                if (ret >= 0)
                    xmlBufferShrink(out->conv, ret);
            } else {
                ret = out->writecallback(out->context,
                     (const char *)out->buffer->content, nbchars);
                if (ret >= 0)
                    xmlBufferShrink(out->buffer, ret);
            }
            if (ret < 0) {
                return(ret);
            }
            out->written += ret;
        }
        written += nbchars;
    } while (len > 0);

done:
    return(written);
}

/**
 * xmlOutputBufferWriteString:
 * @out:  a buffered parser output
 * @str:  a zero terminated C string
 *
 * Write the content of the string in the output I/O buffer
 * This routine handle the I18N transcoding from internal UTF-8
 * The buffer is lossless, i.e. will store in case of partial
 * or delayed writes.
 *
 * Returns the number of chars immediately written, or -1
 *         in case of error.
 */
int
xmlOutputBufferWriteString(xmlOutputBufferPtr out, const char *str) {
    int len;

    if ((out == NULL) || (out->error)) return(-1);
    if (str == NULL)
        return(-1);
    len = (XS32)strlen(str);

    if (len > 0)
        return(xmlOutputBufferWrite(out, len, str));
    return(len);
}

/**
 * xmlEscapeContent:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of unescaped UTF-8 bytes
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and escape them.
 * Returns 0 if success, or -1 otherwise
 * The value of @inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 * The value of @outlen after return is the number of octets consumed.
 */
static int
xmlEscapeContent(xmlChar* out, int *outlen,
                 const xmlChar* in, int *inlen) {
    xmlChar* outstart = out;
    xmlChar* base = (xmlChar*)in;
    xmlChar* outend = out + *outlen;
    const xmlChar* inend;

    inend = in + (*inlen);

    while ((in < inend) && (out < outend)) {
       if (*in == '<') {
        if (outend - out < 4) break;
        *out++ = '&';
        *out++ = 'l';
        *out++ = 't';
        *out++ = ';';
    } else if (*in == '>') {
        if (outend - out < 4) break;
        *out++ = '&';
        *out++ = 'g';
        *out++ = 't';
        *out++ = ';';
    } else if (*in == '&') {
        if (outend - out < 5) break;
        *out++ = '&';
        *out++ = 'a';
        *out++ = 'm';
        *out++ = 'p';
        *out++ = ';';
    } else if (*in == '\r') {
        if (outend - out < 5) break;
        *out++ = '&';
        *out++ = '#';
        *out++ = '1';
        *out++ = '3';
        *out++ = ';';
    } else {
        *out++ = (xmlChar) *in;
    }
    ++in;
    }
    *outlen = (XS32)(out - outstart);
    *inlen = (XS32)(in - base);
    return(0);
}

/**
 * xmlOutputBufferWriteEscape:
 * @out:  a buffered parser output
 * @str:  a zero terminated UTF-8 string
 * @escaping:  an optional escaping function (or NULL)
 *
 * Write the content of the string in the output I/O buffer
 * This routine escapes the caracters and then handle the I18N
 * transcoding from internal UTF-8
 * The buffer is lossless, i.e. will store in case of partial
 * or delayed writes.
 *
 * Returns the number of chars immediately written, or -1
 *         in case of error.
 */
int
xmlOutputBufferWriteEscape(xmlOutputBufferPtr out, const xmlChar *str,
                           xmlCharEncodingOutputFunc escaping) {
    int nbchars = 0; /* number of chars to output to I/O */
    int ret;         /* return from function call */
    int written = 0; /* number of char written to I/O so far */
    int oldwritten=0;/* loop guard */
    int chunk;       /* number of byte currently processed from str */
    int len;         /* number of bytes in str */
    int cons;        /* byte from str consumed */

    if ((out == NULL) || (out->error) || (str == NULL) ||
        (out->buffer == NULL) ||
        (out->buffer->alloc == XML_BUFFER_ALLOC_IMMUTABLE)) return(-1);
    
    len = (XS32)strlen((const char *)str);
    if (len < 0) return(0);
    if (out->error) return(-1);
    if (escaping == NULL) escaping = xmlEscapeContent;

    do {
        oldwritten = written;

        /*
         * how many bytes to consume and how many bytes to store.
         */
        cons = len;
        chunk = (out->buffer->size - out->buffer->use) - 1;

            /*
         * make sure we have enough room to save first, if this is
         * not the case force a flush, but make sure we stay in the loop
         */
        if (chunk < 40) {
            if (xmlBufferGrow(out->buffer, out->buffer->size + 100) < 0)
                return(-1);
            oldwritten = -1;
            continue;
        }

        /*
         * first handle encoding stuff.
         */
        if (out->encoder != NULL) {
            /*
             * Store the data in the incoming raw buffer
             */
            if (out->conv == NULL) {
                out->conv = xmlBufferCreate();
            }
            ret = escaping(out->buffer->content + out->buffer->use ,
                           &chunk, str, &cons);
            if ((ret < 0) || (chunk == 0)) /* chunk==0 => nothing done */
                return(-1);
            out->buffer->use += chunk;
            out->buffer->content[out->buffer->use] = 0;

            if ((out->buffer->use < MINLEN) && (cons == len))
                goto done;

            /*
             * convert as much as possible to the output buffer.
             */
            ret = xmlCharEncOutFunc(out->encoder, out->conv, out->buffer);
            if ((ret < 0) && (ret != -3)) {
                return(-1);
            }
            nbchars = out->conv->use;
        } else {
            ret = escaping(out->buffer->content + out->buffer->use ,
                           &chunk, str, &cons);
            if ((ret < 0) || (chunk == 0)) /* chunk==0 => nothing done */
                return(-1);
            out->buffer->use += chunk;
            out->buffer->content[out->buffer->use] = 0;
            nbchars = out->buffer->use;
        }
        str += cons;
        len -= cons;

        if ((nbchars < MINLEN) && (len <= 0))
            goto done;

        if (out->writecallback) {
            /*
             * second write the stuff to the I/O channel
             */
            if (out->encoder != NULL) {
            ret = out->writecallback(out->context,
                     (const char *)out->conv->content, nbchars);
            if (ret >= 0)
                xmlBufferShrink(out->conv, ret);
            } else {
            ret = out->writecallback(out->context,
                     (const char *)out->buffer->content, nbchars);
            if (ret >= 0)
                xmlBufferShrink(out->buffer, ret);
            }
            if (ret < 0) {
                return(ret);
            }
            out->written += ret;
        } else if (out->buffer->size - out->buffer->use < MINLEN) {
            xmlBufferResize(out->buffer, out->buffer->size + MINLEN);
        }
        written += nbchars;
    } while ((len > 0) && (oldwritten != written));

done:
    return(written);
}

/**
 * xmlOutputBufferFlush:
 * @out:  a buffered output
 *
 * flushes the output I/O channel
 *
 * Returns the number of byte written or -1 in case of error.
 */
int
xmlOutputBufferFlush(xmlOutputBufferPtr out) {
    int nbchars = 0, ret = 0;

    if ((out == NULL) || (out->error)) return(-1);
    /*
     * first handle encoding stuff.
     */
    if ((out->conv != NULL) && (out->encoder != NULL)) {
        /*
         * convert as much as possible to the parser reading buffer.
         */
        nbchars = xmlCharEncOutFunc(out->encoder, out->conv, out->buffer);
        if (nbchars < 0) {
            return(-1);
        }
    }

    /*
     * second flush the stuff to the I/O channel
     */
    if ((out->conv != NULL) && (out->encoder != NULL) && (out->writecallback != NULL)) {
        ret = out->writecallback(out->context, (const char *)out->conv->content, out->conv->use);
        if (ret >= 0) {
            xmlBufferShrink(out->conv, ret);
        }
    } else if (out->writecallback != NULL) {
        ret = out->writecallback(out->context,
                   (const char *)out->buffer->content, out->buffer->use);
        if (ret >= 0)
            xmlBufferShrink(out->buffer, ret);
    }
    if (ret < 0) {
        return(ret);
    }
    out->written += ret;

    return(ret);
}

/**
 * xmlOutputBufferCreateFile:
 * @file:  a FILE*
 * @encoder:  the encoding converter or NULL
 *
 * Create a buffered output for the progressive saving to a FILE *
 * buffered C I/O
 *
 * Returns the new parser output or NULL
 */
xmlOutputBufferPtr
xmlOutputBufferCreateFile(FILE *file, xmlCharEncodingHandlerPtr encoder) {
    xmlOutputBufferPtr ret;

    if (file == NULL) return(NULL);

    ret = xmlAllocOutputBufferInternal(encoder);
    if (ret != NULL) {
        ret->context = file;
        ret->writecallback = xmlFileWrite;
        ret->closecallback = xmlFileClose;
    }

    return(ret);
}


xmlOutputBufferPtr xmlOutputBufferCreateFilename(const char *filename,
                              xmlCharEncodingHandlerPtr encoder,
                              int compression)
{
    xmlOutputBufferPtr ret = NULL;
    void * fd = xmlFileOpenW(filename);
    if(fd) {
        ret = xmlAllocOutputBufferInternal(encoder);
        if(ret) {
            ret->context = fd;
            ret->writecallback = xmlFileWrite;
            ret->closecallback = xmlFileClose;
        }
    }
    return ret;
}

xmlParserInputBufferPtr xmlParserInputBufferCreateMem(const char *mem, int size, xmlCharEncoding enc)
{
    xmlParserInputBufferPtr ret;

    if (size <= 0) return(NULL);
    if (mem == NULL) return(NULL);

    ret = xmlAllocParserInputBuffer(enc);
    if (ret != NULL) {
        xmlBufferAdd(ret->buffer, (const xmlChar *) mem, size);
    }

    return(ret);
}

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

