
#include <string.h>
#include "../inc/xmlstring.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "../inc/xmlencoding.h"

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

static xmlCharEncodingHandlerPtr xmlUTF16LEHandler = NULL;
static xmlCharEncodingHandlerPtr xmlUTF16BEHandler = NULL;

typedef struct _xmlCharEncodingAlias xmlCharEncodingAlias;
typedef xmlCharEncodingAlias *xmlCharEncodingAliasPtr;
struct _xmlCharEncodingAlias {
    const char *name;
    const char *alias;
};

static xmlCharEncodingAliasPtr xmlCharEncodingAliases = NULL;
static int xmlCharEncodingAliasesNb = 0;
static int xmlCharEncodingAliasesMax = 0;

static int xmlLittleEndian = 1;

/************************************************************************
 *                                    *
 *        Conversions To/From UTF8 encoding            *
 *                                    *
 ************************************************************************/

/**
 * asciiToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of ASCII chars
 * @inlen:  the length of @in
 *
 * Take a block of ASCII chars in and try to convert it to an UTF-8
 * block of chars out.
 * Returns 0 if success, or -1 otherwise
 * The value of @inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 * The value of @outlen after return is the number of octets consumed.
 */
static int
asciiToUTF8(xmlChar* out, int *outlen,
              const xmlChar* in, int *inlen) {
    xmlChar* outstart = out;
    const xmlChar* base = in;
    const xmlChar* processed = in;
    xmlChar* outend = out + *outlen;
    const xmlChar* inend;
    unsigned int c;

    inend = in + (*inlen);
    while ((in < inend) && (out - outstart + 5 < *outlen)) {
    c= *in++;

        if (out >= outend)
        break;
        if (c < 0x80) {
        *out++ = c;
    } else {
        *outlen = (XS32)(out - outstart);
        *inlen = (XS32)(processed - base);
        return(-1);
    }

    processed = (const xmlChar*) in;
    }
    *outlen = (XS32)(out - outstart);
    *inlen = (XS32)(processed - base);
    return(*outlen);
}
/*
*
*
*
* add by liguoqiang01093
*/
static int
gb2312ToUTF8(xmlChar* out, int *outlen,
              const xmlChar* in, int *inlen)
{
    xmlChar* outstart = out;
    const xmlChar* base = in;
    const xmlChar* processed = in;
    xmlChar* outend = out + *outlen;
    const xmlChar* inend;
    unsigned int c;

    inend = in + (*inlen);
    while ((in < inend) && (out - outstart < *outlen)) {
        c= *in++;
        if (out >= outend)
            break;
        /*
        if (c < 0x80) {
            ascii_mbtowc((unsigned short*)out, (const unsigned char*)&c, 1);
            out += 2;
        } else {
            s[i++] = c;
            if(i == 2) {
                unsigned char buf[2];
                buf[0] = s[0]-0x80; buf[1] = s[1]-0x80;
                gb2312_mbtowc((unsigned short*)out, buf, 2);
                i = 0;
                out+=2;
            }
        }*/
        *out++ = c;
        processed = (const xmlChar*)in;
    }
    *outlen = (XS32)(out - outstart);
    *inlen = (XS32)(processed - base);
    return(*outlen);
}

/*
*
*
*
* add by liguoqiang01093
*/
static int
UTF8Togb2312(xmlChar* out, int *outlen,
              const xmlChar* in, int *inlen)
{
    xmlChar* outstart = out;
    const xmlChar* base = in;
    const xmlChar* processed = in;
    const xmlChar* inend;

    inend = in + (*inlen);
    while ((in < inend) && (out - outstart < *outlen)) {
        /*
        if( ascii_wctomb((unsigned char*)out, *((unsigned short*)in), 2) == -1) {
            ret = gb2312_wctomb(s, *((unsigned short*)in), 2);
            if (ret != -1) {
                out[0] = s[0]+0x80;
                out[1] = s[1]+0x80;
                out += 2;
            }
        } else {
            out++;
        }
        in += 2;*/
        *out++ = *in++;
        processed = (const xmlChar*)in;
    }
    *outlen = (XS32)(out - outstart);
    *inlen = (XS32)(processed - base);
    return(*outlen);
}

/**
 * UTF8ToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @inb:  a pointer to an array of UTF-8 chars
 * @inlenb:  the length of @in in UTF-8 chars
 *
 * No op copy operation for UTF8 handling.
 *
 * Returns the number of bytes written, or -1 if lack of space.
 *     The value of *inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 */
static int
UTF8ToUTF8(xmlChar* out, int *outlen,
           const xmlChar* inb, int *inlenb)
{
    int len;

    if ((out == NULL) || (inb == NULL) || (outlen == NULL) || (inlenb == NULL))
    return(-1);
    if (*outlen > *inlenb) {
    len = *inlenb;
    } else {
    len = *outlen;
    }
    if (len < 0)
    return(-1);

    memcpy(out, inb, len);

    *outlen = len;
    *inlenb = len;
    return(*outlen);
}


/**
 * UTF8Toisolat1:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of UTF-8 chars
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and try to convert it to an ISO Latin 1
 * block of chars out.
 *
 * Returns the number of bytes written if success, -2 if the transcoding fails,
           or -1 otherwise
 * The value of @inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 * The value of @outlen after return is the number of octets consumed.
 */
int
UTF8Toisolat1(xmlChar* out, int *outlen,
              const xmlChar* in, int *inlen) {
    const xmlChar* processed = in;
    const xmlChar* outend;
    const xmlChar* outstart = out;
    const xmlChar* instart = in;
    const xmlChar* inend;
    unsigned int c, d;
    int trailing;

    if ((out == NULL) || (outlen == NULL) || (inlen == NULL)) return(-1);
    if (in == NULL) {
        /*
     * initialization nothing to do
     */
    *outlen = 0;
    *inlen = 0;
    return(0);
    }
    inend = in + (*inlen);
    outend = out + (*outlen);
    while (in < inend) {
    d = *in++;
    if      (d < 0x80)  { c= d; trailing= 0; }
    else if (d < 0xC0) {
        /* trailing byte in leading position */
        *outlen = (XS32)(out - outstart);
        *inlen = (XS32)(processed - instart);
        return(-2);
        } else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
        else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
        else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
    else {
        /* no chance for this in IsoLat1 */
        *outlen = (XS32)(out - outstart);
        *inlen = (XS32)(processed - instart);
        return(-2);
    }

    if (inend - in < trailing) {
        break;
    }

    for ( ; trailing; trailing--) {
        if (in >= inend)
        break;
        if (((d= *in++) & 0xC0) != 0x80) {
        *outlen = (XS32)(out - outstart);
        *inlen = (XS32)(processed - instart);
        return(-2);
        }
        c <<= 6;
        c |= d & 0x3F;
    }

    /* assertion: c is a single UTF-4 value */
    if (c <= 0xFF) {
        if (out >= outend)
        break;
        *out++ = c;
    } else {
        /* no chance for this in IsoLat1 */
        *outlen = (XS32)(out - outstart);
        *inlen = (XS32)(processed - instart);
        return(-2);
    }
    processed = in;
    }
    *outlen = (XS32)(out - outstart);
    *inlen = (XS32)(processed - instart);
    return(*outlen);
}

/**
 * UTF16LEToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @inb:  a pointer to an array of UTF-16LE passwd as a byte array
 * @inlenb:  the length of @in in UTF-16LE chars
 *
 * Take a block of UTF-16LE ushorts in and try to convert it to an UTF-8
 * block of chars out. This function assumes the endian property
 * is the same between the native type of this machine and the
 * inputed one.
 *
 * Returns the number of bytes written, or -1 if lack of space, or -2
 *     if the transcoding fails (if *in is not a valid utf16 string)
 *     The value of *inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 */
static int
UTF16LEToUTF8(xmlChar* out, int *outlen,
            const xmlChar* inb, int *inlenb)
{
    xmlChar* outstart = out;
    const xmlChar* processed = inb;
    xmlChar* outend = out + *outlen;
    unsigned short* in = (unsigned short*) inb;
    unsigned short* inend;
    unsigned int c, d, inlen;
    xmlChar *tmp;
    int bits;

    if ((*inlenb % 2) == 1)
        (*inlenb)--;
    inlen = *inlenb / 2;
    inend = in + inlen;
    while ((in < inend) && (out - outstart + 5 < *outlen)) {
        if (xmlLittleEndian) {
        c= *in++;
    } else {
        tmp = (xmlChar *) in;
        c = *tmp++;
        c = c | (((unsigned int)*tmp) << 8);
        in++;
    }
        if ((c & 0xFC00) == 0xD800) {    /* surrogates */
        if (in >= inend) {           /* (in > inend) shouldn't happens */
        break;
        }
        if (xmlLittleEndian) {
        d = *in++;
        } else {
        tmp = (xmlChar *) in;
        d = *tmp++;
        d = d | (((unsigned int)*tmp) << 8);
        in++;
        }
            if ((d & 0xFC00) == 0xDC00) {
                c &= 0x03FF;
                c <<= 10;
                c |= d & 0x03FF;
                c += 0x10000;
            }
            else {
        *outlen = (XS32)(out - outstart);
        *inlenb = (XS32)(processed - inb);
            return(-2);
        }
        }

    /* assertion: c is a single UTF-4 value */
        if (out >= outend)
        break;
        if      (c <    0x80) {  *out++=  c;                bits= -6; }
        else if (c <   0x800) {  *out++= ((c >>  6) & 0x1F) | 0xC0;  bits=  0; }
        else if (c < 0x10000) {  *out++= ((c >> 12) & 0x0F) | 0xE0;  bits=  6; }
        else                  {  *out++= ((c >> 18) & 0x07) | 0xF0;  bits= 12; }

        for ( ; bits >= 0; bits-= 6) {
            if (out >= outend)
            break;
            *out++= ((c >> bits) & 0x3F) | 0x80;
        }
    processed = (const xmlChar*) in;
    }
    *outlen = (XS32)(out - outstart);
    *inlenb = (XS32)(processed - inb);
    return(*outlen);
}

/**
 * UTF16BEToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @inb:  a pointer to an array of UTF-16 passed as a byte array
 * @inlenb:  the length of @in in UTF-16 chars
 *
 * Take a block of UTF-16 ushorts in and try to convert it to an UTF-8
 * block of chars out. This function assumes the endian property
 * is the same between the native type of this machine and the
 * inputed one.
 *
 * Returns the number of bytes written, or -1 if lack of space, or -2
 *     if the transcoding fails (if *in is not a valid utf16 string)
 * The value of *inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 */
static int
UTF16BEToUTF8(xmlChar* out, int *outlen,
            const xmlChar* inb, int *inlenb)
{
    xmlChar* outstart = out;
    const xmlChar* processed = inb;
    xmlChar* outend = out + *outlen;
    unsigned short* in = (unsigned short*) inb;
    unsigned short* inend;
    unsigned int c, d, inlen;
    xmlChar *tmp;
    int bits;

    if ((*inlenb % 2) == 1)
        (*inlenb)--;
    inlen = *inlenb / 2;
    inend= in + inlen;
    while (in < inend) {
    if (xmlLittleEndian) {
        tmp = (xmlChar *) in;
        c = *tmp++;
        c = c << 8;
        c = c | (unsigned int) *tmp;
        in++;
    } else {
        c= *in++;
    }
        if ((c & 0xFC00) == 0xD800) {    /* surrogates */
        if (in >= inend) {           /* (in > inend) shouldn't happens */
        *outlen = (XS32)(out - outstart);
        *inlenb = (XS32)(processed - inb);
            return(-2);
        }
        if (xmlLittleEndian) {
        tmp = (xmlChar *) in;
        d = *tmp++;
        d = d << 8;
        d = d | (unsigned int) *tmp;
        in++;
        } else {
        d= *in++;
        }
            if ((d & 0xFC00) == 0xDC00) {
                c &= 0x03FF;
                c <<= 10;
                c |= d & 0x03FF;
                c += 0x10000;
            }
            else {
        *outlen = (XS32)(out - outstart);
        *inlenb = (XS32)(processed - inb);
            return(-2);
        }
        }

    /* assertion: c is a single UTF-4 value */
        if (out >= outend)
        break;
        if      (c <    0x80) {  *out++=  c;                bits= -6; }
        else if (c <   0x800) {  *out++= ((c >>  6) & 0x1F) | 0xC0;  bits=  0; }
        else if (c < 0x10000) {  *out++= ((c >> 12) & 0x0F) | 0xE0;  bits=  6; }
        else                  {  *out++= ((c >> 18) & 0x07) | 0xF0;  bits= 12; }

        for ( ; bits >= 0; bits-= 6) {
            if (out >= outend)
            break;
            *out++= ((c >> bits) & 0x3F) | 0x80;
        }
    processed = (const xmlChar*) in;
    }
    *outlen = (XS32)(out - outstart);
    *inlenb = (XS32)(processed - inb);
    return(*outlen);
}

/************************************************************************
 *                                    *
 *        Generic encoding handling routines            *
 *                                    *
 ************************************************************************/

/**
 * xmlCleanupEncodingAliases:
 *
 * Unregisters all aliases
 */
void
xmlCleanupEncodingAliases(void) {
    int i;

    if (xmlCharEncodingAliases == NULL)
    return;

    for (i = 0;i < xmlCharEncodingAliasesNb;i++) {
    if (xmlCharEncodingAliases[i].name != NULL)
        xmlFree((char *) xmlCharEncodingAliases[i].name);
    if (xmlCharEncodingAliases[i].alias != NULL)
        xmlFree((char *) xmlCharEncodingAliases[i].alias);
    }
    xmlCharEncodingAliasesNb = 0;
    xmlCharEncodingAliasesMax = 0;
    xmlFree(xmlCharEncodingAliases);
    xmlCharEncodingAliases = NULL;
}

/**
 * xmlGetEncodingAlias:
 * @alias:  the alias name as parsed, in UTF-8 format (ASCII actually)
 *
 * Lookup an encoding name for the given alias.
 *
 * Returns NULL if not found, otherwise the original name
 */
const char *
xmlGetEncodingAlias(const char *alias) {
    int i;
    char upper[100];

    if (alias == NULL)
    return(NULL);

    if (xmlCharEncodingAliases == NULL)
    return(NULL);

    for (i = 0;i < 99;i++) {
        upper[i] = toupper(alias[i]);
    if (upper[i] == 0) break;
    }
    upper[i] = 0;

    /*
     * Walk down the list looking for a definition of the alias
     */
    for (i = 0;i < xmlCharEncodingAliasesNb;i++) {
    if (!strcmp(xmlCharEncodingAliases[i].alias, upper)) {
        return(xmlCharEncodingAliases[i].name);
    }
    }
    return(NULL);
}

/**
 * xmlAddEncodingAlias:
 * @name:  the encoding name as parsed, in UTF-8 format (ASCII actually)
 * @alias:  the alias name as parsed, in UTF-8 format (ASCII actually)
 *
 * Registers an alias @alias for an encoding named @name. Existing alias
 * will be overwritten.
 *
 * Returns 0 in case of success, -1 in case of error
 */
int
xmlAddEncodingAlias(const char *name, const char *alias) {
    int i;
    char upper[100];

    if ((name == NULL) || (alias == NULL))
    return(-1);

    for (i = 0;i < 99;i++) {
        upper[i] = toupper(alias[i]);
    if (upper[i] == 0) break;
    }
    upper[i] = 0;

    if (xmlCharEncodingAliases == NULL) {
    xmlCharEncodingAliasesNb = 0;
    xmlCharEncodingAliasesMax = 20;
    xmlCharEncodingAliases = (xmlCharEncodingAliasPtr)
          xmlMalloc(xmlCharEncodingAliasesMax * sizeof(xmlCharEncodingAlias));
    if (xmlCharEncodingAliases == NULL)
        return(-1);
    } else if (xmlCharEncodingAliasesNb >= xmlCharEncodingAliasesMax) {
    xmlCharEncodingAliasesMax *= 2;
    xmlCharEncodingAliases = (xmlCharEncodingAliasPtr)
          xmlRealloc(xmlCharEncodingAliases,
                 xmlCharEncodingAliasesMax * sizeof(xmlCharEncodingAlias));
    }
    /*
     * Walk down the list looking for a definition of the alias
     */
    for (i = 0;i < xmlCharEncodingAliasesNb;i++) {
    if (!strcmp(xmlCharEncodingAliases[i].alias, upper)) {
        /*
         * Replace the definition.
         */
        xmlFree((char *) xmlCharEncodingAliases[i].name);
        xmlCharEncodingAliases[i].name = xmlMemStrdup(name);
        return(0);
    }
    }
    /*
     * Add the definition
     */
    xmlCharEncodingAliases[xmlCharEncodingAliasesNb].name = xmlMemStrdup(name);
    xmlCharEncodingAliases[xmlCharEncodingAliasesNb].alias = xmlMemStrdup(upper);
    xmlCharEncodingAliasesNb++;
    return(0);
}

/**
 * xmlDelEncodingAlias:
 * @alias:  the alias name as parsed, in UTF-8 format (ASCII actually)
 *
 * Unregisters an encoding alias @alias
 *
 * Returns 0 in case of success, -1 in case of error
 */
int
xmlDelEncodingAlias(const char *alias) {
    int i;

    if (alias == NULL)
    return(-1);

    if (xmlCharEncodingAliases == NULL)
    return(-1);
    /*
     * Walk down the list looking for a definition of the alias
     */
    for (i = 0;i < xmlCharEncodingAliasesNb;i++) {
    if (!strcmp(xmlCharEncodingAliases[i].alias, alias)) {
        xmlFree((char *) xmlCharEncodingAliases[i].name);
        xmlFree((char *) xmlCharEncodingAliases[i].alias);
        xmlCharEncodingAliasesNb--;
        memmove(&xmlCharEncodingAliases[i], &xmlCharEncodingAliases[i + 1],
            sizeof(xmlCharEncodingAlias) * (xmlCharEncodingAliasesNb - i));
        return(0);
    }
    }
    return(-1);
}

/**
 * xmlParseCharEncoding:
 * @name:  the encoding name as parsed, in UTF-8 format (ASCII actually)
 *
 * Compare the string to the encoding schemes already known. Note
 * that the comparison is case insensitive accordingly to the section
 * [XML] 4.3.3 Character Encoding in Entities.
 *
 * Returns one of the XML_CHAR_ENCODING_... values or XML_CHAR_ENCODING_NONE
 * if not recognized.
 */
xmlCharEncoding
xmlParseCharEncoding(const char* name)
{
    const char *alias;
    char upper[500];
    int i;

    if (name == NULL)
    return(XML_CHAR_ENCODING_NONE);

    /*
     * Do the alias resolution
     */
    alias = xmlGetEncodingAlias(name);
    if (alias != NULL)
    name = alias;

    for (i = 0;i < 499;i++) {
        upper[i] = toupper(name[i]);
    if (upper[i] == 0) break;
    }
    upper[i] = 0;

    if (!strcmp(upper, "")) return(XML_CHAR_ENCODING_NONE);
    if (!strcmp(upper, "UTF-8")) return(XML_CHAR_ENCODING_UTF8);
    if (!strcmp(upper, "UTF8")) return(XML_CHAR_ENCODING_UTF8);

    /*
     * NOTE: if we were able to parse this, the endianness of UTF16 is
     *       already found and in use
     */
    if (!strcmp(upper, "UTF-16")) return(XML_CHAR_ENCODING_UTF16LE);
    if (!strcmp(upper, "UTF16")) return(XML_CHAR_ENCODING_UTF16LE);

    if (!strcmp(upper, "ISO-10646-UCS-2")) return(XML_CHAR_ENCODING_UCS2);
    if (!strcmp(upper, "UCS-2")) return(XML_CHAR_ENCODING_UCS2);
    if (!strcmp(upper, "UCS2")) return(XML_CHAR_ENCODING_UCS2);

    /*
     * NOTE: if we were able to parse this, the endianness of UCS4 is
     *       already found and in use
     */
    if (!strcmp(upper, "ISO-10646-UCS-4")) return(XML_CHAR_ENCODING_UCS4LE);
    if (!strcmp(upper, "UCS-4")) return(XML_CHAR_ENCODING_UCS4LE);
    if (!strcmp(upper, "UCS4")) return(XML_CHAR_ENCODING_UCS4LE);


    if (!strcmp(upper,  "ISO-8859-1")) return(XML_CHAR_ENCODING_8859_1);
    if (!strcmp(upper,  "ISO-LATIN-1")) return(XML_CHAR_ENCODING_8859_1);
    if (!strcmp(upper,  "ISO LATIN 1")) return(XML_CHAR_ENCODING_8859_1);

    if (!strcmp(upper,  "ISO-8859-2")) return(XML_CHAR_ENCODING_8859_2);
    if (!strcmp(upper,  "ISO-LATIN-2")) return(XML_CHAR_ENCODING_8859_2);
    if (!strcmp(upper,  "ISO LATIN 2")) return(XML_CHAR_ENCODING_8859_2);

    if (!strcmp(upper,  "ISO-8859-3")) return(XML_CHAR_ENCODING_8859_3);
    if (!strcmp(upper,  "ISO-8859-4")) return(XML_CHAR_ENCODING_8859_4);
    if (!strcmp(upper,  "ISO-8859-5")) return(XML_CHAR_ENCODING_8859_5);
    if (!strcmp(upper,  "ISO-8859-6")) return(XML_CHAR_ENCODING_8859_6);
    if (!strcmp(upper,  "ISO-8859-7")) return(XML_CHAR_ENCODING_8859_7);
    if (!strcmp(upper,  "ISO-8859-8")) return(XML_CHAR_ENCODING_8859_8);
    if (!strcmp(upper,  "ISO-8859-9")) return(XML_CHAR_ENCODING_8859_9);

    if (!strcmp(upper, "ISO-2022-JP")) return(XML_CHAR_ENCODING_2022_JP);
    if (!strcmp(upper, "SHIFT_JIS")) return(XML_CHAR_ENCODING_SHIFT_JIS);
    if (!strcmp(upper, "EUC-JP")) return(XML_CHAR_ENCODING_EUC_JP);
    if (!strcmp(upper, "GB2312")) return(XML_CHAR_ENCODING_GB2312);

    return(XML_CHAR_ENCODING_ERROR);
}

/**
 * xmlGetCharEncodingName:
 * @enc:  the encoding
 *
 * The "canonical" name for XML encoding.
 * C.f. http://www.w3.org/TR/REC-xml#charencoding
 * Section 4.3.3  Character Encoding in Entities
 *
 * Returns the canonical name for the given encoding
 */

const char*
xmlGetCharEncodingName(xmlCharEncoding enc) {
    switch (enc) {
        case XML_CHAR_ENCODING_ERROR:
        return(NULL);
        case XML_CHAR_ENCODING_NONE:
        return(NULL);
        case XML_CHAR_ENCODING_UTF8:
        return("UTF-8");
        case XML_CHAR_ENCODING_UTF16LE:
        return("UTF-16");
        case XML_CHAR_ENCODING_UTF16BE:
        return("UTF-16");
        case XML_CHAR_ENCODING_EBCDIC:
            return("EBCDIC");
        case XML_CHAR_ENCODING_UCS4LE:
            return("ISO-10646-UCS-4");
        case XML_CHAR_ENCODING_UCS4BE:
            return("ISO-10646-UCS-4");
        case XML_CHAR_ENCODING_UCS4_2143:
            return("ISO-10646-UCS-4");
        case XML_CHAR_ENCODING_UCS4_3412:
            return("ISO-10646-UCS-4");
        case XML_CHAR_ENCODING_UCS2:
            return("ISO-10646-UCS-2");
        case XML_CHAR_ENCODING_8859_1:
        return("ISO-8859-1");
        case XML_CHAR_ENCODING_8859_2:
        return("ISO-8859-2");
        case XML_CHAR_ENCODING_8859_3:
        return("ISO-8859-3");
        case XML_CHAR_ENCODING_8859_4:
        return("ISO-8859-4");
        case XML_CHAR_ENCODING_8859_5:
        return("ISO-8859-5");
        case XML_CHAR_ENCODING_8859_6:
        return("ISO-8859-6");
        case XML_CHAR_ENCODING_8859_7:
        return("ISO-8859-7");
        case XML_CHAR_ENCODING_8859_8:
        return("ISO-8859-8");
        case XML_CHAR_ENCODING_8859_9:
        return("ISO-8859-9");
        case XML_CHAR_ENCODING_2022_JP:
            return("ISO-2022-JP");
        case XML_CHAR_ENCODING_SHIFT_JIS:
            return("Shift-JIS");
        case XML_CHAR_ENCODING_EUC_JP:
            return("EUC-JP");
    case XML_CHAR_ENCODING_ASCII:
        return(NULL);
        case XML_CHAR_ENCODING_GB2312:
            return ("GB2312");
    }
    return(NULL);
}

/************************************************************************
 *                                    *
 *            Char encoding handlers                *
 *                                    *
 ************************************************************************/


/* the size should be growable, but it's not a big deal ... */
#define MAX_ENCODING_HANDLERS 50
static xmlCharEncodingHandlerPtr *handlers = NULL;
static int nbCharEncodingHandler = 0;

/*
 * The default is UTF-8 for XML, that's also the default used for the
 * parser internals, so the default encoding handler is NULL
 */

static xmlCharEncodingHandlerPtr xmlDefaultCharEncodingHandler = NULL;

/**
 * xmlNewCharEncodingHandler:
 * @name:  the encoding name, in UTF-8 format (ASCII actually)
 * @input:  the xmlCharEncodingInputFunc to read that encoding
 * @output:  the xmlCharEncodingOutputFunc to write that encoding
 *
 * Create and registers an xmlCharEncodingHandler.
 *
 * Returns the xmlCharEncodingHandlerPtr created (or NULL in case of error).
 */
xmlCharEncodingHandlerPtr
xmlNewCharEncodingHandler(const char *name,
                          xmlCharEncodingInputFunc input,
                          xmlCharEncodingOutputFunc output) {
    xmlCharEncodingHandlerPtr handler;
    const char *alias;
    char upper[500];
    int i;
    char *up = NULL;

    /*
     * Do the alias resolution
     */
    alias = xmlGetEncodingAlias(name);
    if (alias != NULL)
        name = alias;

    /*
     * Keep only the uppercase version of the encoding.
     */
    if (name == NULL) {
        return(NULL);
    }
    for (i = 0;i < 499;i++) {
        upper[i] = toupper(name[i]);
        if (upper[i] == 0) break;
    }
    upper[i] = 0;
    up = xmlMemStrdup(upper);
    if (up == NULL) {
        return(NULL);
    }

    /*
     * allocate and fill-up an handler block.
     */
    handler = (xmlCharEncodingHandlerPtr)
              xmlMalloc(sizeof(xmlCharEncodingHandler));
    if (handler == NULL) {
        xmlFree(up);
        return(NULL);
    }
    memset(handler, 0, sizeof(xmlCharEncodingHandler));
    handler->input = input;
    handler->output = output;
    handler->name = up;

    /*
     * registers and returns the handler.
     */
    xmlRegisterCharEncodingHandler(handler);
    return(handler);
}

/**
 * xmlInitCharEncodingHandlers:
 *
 * Initialize the char encoding support, it registers the default
 * encoding supported.
 * NOTE: while public, this function usually doesn't need to be called
 *       in normal processing.
 */
void
xmlInitCharEncodingHandlers(void) {
    unsigned short int tst = 0x1234;
    xmlChar *ptr = (xmlChar *) &tst;

    if (handlers != NULL) return;

    handlers = (xmlCharEncodingHandlerPtr *)
        xmlMalloc(MAX_ENCODING_HANDLERS * sizeof(xmlCharEncodingHandlerPtr));

    if (*ptr == 0x12) xmlLittleEndian = 0;
    else if (*ptr == 0x34) xmlLittleEndian = 1;
    else {
    }

    if (handlers == NULL) {
        return;
    }
    xmlNewCharEncodingHandler("UTF-8", UTF8ToUTF8, UTF8ToUTF8);
    xmlNewCharEncodingHandler("GB2312", gb2312ToUTF8, UTF8Togb2312);

    xmlUTF16LEHandler =
          xmlNewCharEncodingHandler("UTF-16LE", UTF16LEToUTF8, NULL);
    xmlUTF16BEHandler =
          xmlNewCharEncodingHandler("UTF-16BE", UTF16BEToUTF8, NULL);
    xmlNewCharEncodingHandler("UTF-16", UTF16LEToUTF8, NULL);
    xmlNewCharEncodingHandler("ASCII", asciiToUTF8, NULL);
    xmlNewCharEncodingHandler("US-ASCII", asciiToUTF8, NULL);

}

/**
 * xmlCleanupCharEncodingHandlers:
 *
 * Cleanup the memory allocated for the char encoding support, it
 * unregisters all the encoding handlers and the aliases.
 */
void
xmlCleanupCharEncodingHandlers(void) {
    xmlCleanupEncodingAliases();

    if (handlers == NULL) return;

    for (;nbCharEncodingHandler > 0;) {
        nbCharEncodingHandler--;
    if (handlers[nbCharEncodingHandler] != NULL) {
        if (handlers[nbCharEncodingHandler]->name != NULL)
        xmlFree(handlers[nbCharEncodingHandler]->name);
        xmlFree(handlers[nbCharEncodingHandler]);
    }
    }
    xmlFree(handlers);
    handlers = NULL;
    nbCharEncodingHandler = 0;
    xmlDefaultCharEncodingHandler = NULL;
}

/**
 * xmlRegisterCharEncodingHandler:
 * @handler:  the xmlCharEncodingHandlerPtr handler block
 *
 * Register the char encoding handler, surprising, isn't it ?
 */
void
xmlRegisterCharEncodingHandler(xmlCharEncodingHandlerPtr handler) {
    if (handlers == NULL) xmlInitCharEncodingHandlers();
    if ((handler == NULL) || (handlers == NULL)) {
        return;
    }

    if (nbCharEncodingHandler >= MAX_ENCODING_HANDLERS) {
        return;
    }
    handlers[nbCharEncodingHandler++] = handler;
}

/**
 * xmlFindCharEncodingHandler:
 * @name:  a string describing the char encoding.
 *
 * Search in the registered set the handler able to read/write that encoding.
 *
 * Returns the handler or NULL if not found
 */
xmlCharEncodingHandlerPtr
xmlFindCharEncodingHandler(const char *name) {
    const char *nalias;
    const char *norig;
    xmlCharEncoding alias;
    char upper[100];
    int i;

    if (handlers == NULL) xmlInitCharEncodingHandlers();
    if (name == NULL) return(xmlDefaultCharEncodingHandler);
    if (name[0] == 0) return(xmlDefaultCharEncodingHandler);

    /*
     * Do the alias resolution
     */
    norig = name;
    nalias = xmlGetEncodingAlias(name);
    if (nalias != NULL)
        name = nalias;

    /*
     * Check first for directly registered encoding names
     */
    for (i = 0;i < 99;i++) {
        upper[i] = toupper(name[i]);
        if (upper[i] == 0) break;
    }
    upper[i] = 0;

    if (handlers != NULL) {
        for (i = 0;i < nbCharEncodingHandler; i++) {
            if (!strcmp(upper, handlers[i]->name)) {
                return(handlers[i]);
            }
        }
    }

    /*
     * Fallback using the canonical names
     */
    alias = xmlParseCharEncoding(norig);
    if (alias != XML_CHAR_ENCODING_ERROR) {
        const char* canon;
        canon = xmlGetCharEncodingName(alias);
        if ((canon != NULL) && (strcmp(name, canon))) {
            return(xmlFindCharEncodingHandler(canon));
        }
    }

    /* If "none of the above", give up */
    return(NULL);
}

/**
 * xmlCharEncOutFunc:
 * @handler:    char enconding transformation data structure
 * @out:  an xmlBuffer for the output.
 * @in:  an xmlBuffer for the input
 *
 * Generic front-end for the encoding handler output function
 * a first call with @in == NULL has to be made firs to initiate the
 * output in case of non-stateless encoding needing to initiate their
 * state or the output (like the BOM in UTF16).
 * In case of UTF8 sequence conversion errors for the given encoder,
 * the content will be automatically remapped to a CharRef sequence.
 *
 * Returns the number of byte written if success, or
 *     -1 general error
 *     -2 if the transcoding fails (for *in is not valid utf8 string or
 *        the result of transformation can't fit into the encoding we want), or
 */
int
xmlCharEncOutFunc(xmlCharEncodingHandler *handler, xmlBufferPtr out,  xmlBufferPtr in)
{
    int ret = -2;
    int written;
    int writtentot = 0;
    int toconv;
    int output = 0;

    if (handler == NULL) return(-1);
    if (out == NULL) return(-1);

    if(in == NULL) {
        out->use = 0;
        return 0;
    }

    if(out->size < in->use) {
        return -1;
    }
    if(out->content && in->content) {
        XOS_MemMove(out->content, in->content, in->use);
        out->use = in->use;
    }
    
    return out->use;
retry:

    written = out->size - out->use;

    if (written > 0)
    written--; /* Gennady: count '/0' */

    /*
     * First specific handling of in = NULL, i.e. the initialization call
     */
    if (in == NULL) {
        toconv = 0;
        if (handler->output != NULL) {
            ret = handler->output(&out->content[out->use], &written,
                      NULL, &toconv);
            if (ret >= 0) { /* Gennady: check return value */
            out->use += written;
            out->content[out->use] = 0;
            }
        }

            return(0);
        }

        /*
         * Conversion itself.
         */
        toconv = in->use;
        if (toconv == 0)
        return(0);
        if (toconv * 4 >= written) {
            xmlBufferGrow(out, toconv * 4);
        written = out->size - out->use - 1;
        }
        if (handler->output != NULL) {
        ret = handler->output(&out->content[out->use], &written,
                              in->content, &toconv);
        if (written > 0) {
            xmlBufferShrink(in, toconv);
            out->use += written;
            writtentot += written;
        }
        out->content[out->use] = 0;
        }
        else {
            return(-1);
        }

        if (ret >= 0) output += ret;

        /*
         * Attempt to handle error cases
         */
        switch (ret) {
            case 0:

            break;
            case -1:

            break;
            case -3:
            break;
            case -2: {
            int len = in->use;
            const xmlChar *utf = (const xmlChar *) in->content;
            int cur;

            cur = xmlGetUTF8Char(utf, &len);
            if (cur > 0) {
                //xmlChar charref[20];

                /*
                 * Removes the UTF8 sequence, and replace it by a charref
                 * and continue the transcoding phase, hoping the error
                 * did not mangle the encoder state.
                 */
                //snprintf((char *) &charref[0], sizeof(charref), "&#%d;", cur);
                //xmlBufferShrink(in, len);
                //xmlBufferAddHead(in, charref, -1);

                goto retry;
            } else {
                /*char buf[50];

                snprintf(&buf[0], 49, "0x%02X 0x%02X 0x%02X 0x%02X",
                     in->content[0], in->content[1],
                     in->content[2], in->content[3]);
                buf[49] = 0;*/
                if (in->alloc != XML_BUFFER_ALLOC_IMMUTABLE)
                    in->content[0] = ' ';
            }
            break;
        }
    }
    return(ret);
}

/**
 * xmlCharEncCloseFunc:
 * @handler:    char enconding transformation data structure
 *
 * Generic front-end for encoding handler close function
 *
 * Returns 0 if success, or -1 in case of error
 */
int
xmlCharEncCloseFunc(xmlCharEncodingHandler *handler) {
    int ret = 0;
    int tofree = 0;
    if (handler == NULL) return(-1);
    if (handler->name == NULL) return(-1);

    if (tofree) {
        /* free up only dynamic handlers iconv/uconv */
        if (handler->name != NULL)
            xmlFree(handler->name);
        handler->name = NULL;
        xmlFree(handler);
    }

    return(ret);
}
#ifdef __cplusplus
}
#endif /* _ _cplusplus */

