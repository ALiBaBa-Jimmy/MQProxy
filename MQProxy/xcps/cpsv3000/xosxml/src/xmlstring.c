/*
 * string.c : an XML string utilities module
 *
 * This module provides various utility functions for manipulating
 * the xmlChar* type. All functions named xmlStr* have been moved here
 * from the parser.c file (their original home). 
 *
 * See Copyright for the status of this software.
 *
 * UTF8 string routines from:
 * William Brack <wbrack@mmm.com.hk>
 *
 * daniel@veillard.com
 */

#include <stdlib.h>
#include <string.h>
#include "xmlstring.h"

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

#define IS_CHAR(c)                            \
    ( ( ((c) >= 0x20) && ((c) < 0x7F) ) ||                \
((c) == 0x09) || ((c) == 0x0a)   || ((c) == 0x0d) )

static const unsigned char casemap[256] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
    0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
    0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
    0x78,0x79,0x7A,0x7B,0x5C,0x5D,0x5E,0x5F,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
    0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
    0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
    0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
    0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
    0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
    0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
    0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
    0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
    0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
    0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
    0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,
    0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
    0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
    0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

#ifdef XOS_VXWORKS
xmlChar *strdup( XCONST xmlChar *s1 )
{
    xmlChar *p  = XNULL;
    XS32 s32Len = 0;
    
    if ( XNULL == s1 )
    {
        return XNULL;
    }
    
    s32Len = strlen( s1 );
    if ( 0 > s32Len  )
    {
        return XNULL;
    }
    
    p = (xmlChar*)xmlMalloc( s32Len + 1 );
    memset( p, 0x00, s32Len+1 );
    strcpy( p, s1 );
    
    return p;
}
#endif


/**
* xmlStrndup:
* @cur:  the input xmlChar *
* @len:  the len of @cur
*
* a strndup for array of xmlChar's
*
* Returns a new xmlChar * or XNULL
*/
xmlChar *xmlStrndup(XCONST xmlChar *cur, XS32 len) 
{
    xmlChar *ret;
    
    if ((cur == XNULL) || (len < 0)) return(XNULL);
    ret = xmlMalloc((len + 1) * sizeof(xmlChar));
    if (ret == XNULL) 
    {
        return(XNULL);
    }
    XOS_MemCpy(ret, cur, len * sizeof(xmlChar));
    ret[len] = 0;
    return(ret);
}

/**
* xmlCharStrndup:
* @cur:  the input XS8 *
* @len:  the len of @cur
*
* a strndup for XS8's to xmlChar's
*
* Returns a new xmlChar * or XNULL
*/

xmlChar *xmlCharStrndup(XCONST XS8 *cur, XS32 len) 
{
    XS32 i;
    xmlChar *ret;
    
    if ((cur == XNULL) || (len < 0)) return(XNULL);
    ret = xmlMalloc((len + 1) * sizeof(xmlChar));
    if (ret == XNULL) 
    {
        return(XNULL);
    }
    for (i = 0;i < len;i++)
        ret[i] = (xmlChar) cur[i];
    ret[len] = 0;
    return(ret);
}

/**
* xmlCharStrdup:
* @cur:  the input XS8 *
* @len:  the len of @cur
*
* a strdup for XS8's to xmlChar's
*
* Returns a new xmlChar * or XNULL
*/
xmlChar *xmlCharStrdup(XCONST XS8 *cur) 
{
    XCONST XS8 *p = cur;
    
    if (cur == XNULL) return(XNULL);
    while (*p != '\0') p++;
    return(xmlCharStrndup(cur, (XS32)(p - cur)));
}

/**
* xmlStrcmp:
* @str1:  the first xmlChar *
* @str2:  the second xmlChar *
*
* a XOS_StrCmp for xmlChar's
*
* Returns the integer result of the comparison
*/
XS32 xmlStrcmp(XCONST xmlChar *str1, XCONST xmlChar *str2) 
{
    register XS32 tmp;
    
    if ((str1 == XNULL) && (str2 == XNULL)) return(0);
    if (str1 == XNULL) return(-1);
    if (str2 == XNULL) return(1);
    do 
    {
        tmp = *str1++ - *str2++;
        if (tmp != 0) return(tmp);
    } while ((*str1 != 0) && (*str2 != 0));
    return (*str1 - *str2);
}

/* Added from 2.3.5 */
/**
* xmlStrEqual:
* @str1:  the first xmlChar *
* @str2:  the second xmlChar *
*
* Check if both string are equal of have same content
* Should be a bit more readable and faster than xmlStrEqual()
*
* Returns 1 if they are equal, 0 if they are different
*/
XS32 xmlStrEqual(XCONST xmlChar *str1, XCONST xmlChar *str2) 
{
    if (str1 == str2) 
        return(1);
    if (str1 == XNULL) 
        return(0);
    if (str2 == XNULL) 
        return(0);
    do 
    {
        if (*str1++ != *str2) 
            return(0);
    } while (*str2++);
    return(1);
}

/**
* xmlStrncmp:
* @str1:  the first xmlChar *
* @str2:  the second xmlChar *
* @len:  the max comparison length
*
* a strncmp for xmlChar's
*
* Returns the integer result of the comparison
*/
XS32 xmlStrncmp(XCONST xmlChar *str1, XCONST xmlChar *str2, XS32 len) 
{
    register XS32 tmp;
    
    if (len <= 0) 
        return(0);
    if ((str1 == XNULL) && (str2 == XNULL)) 
        return(0);
    if (str1 == XNULL) 
        return(-1);
    if (str2 == XNULL) 
        return(1);
    do 
    {
        tmp = *str1++ - *str2++;
        if (tmp != 0) 
            return(tmp);
        len--;
        if (len <= 0) 
            return(0);
    } while ((*str1 != 0) && (*str2 != 0));
    
    return (*str1 - *str2);
}

/**
* xmlStrchr:
* @str:  the xmlChar * array
* @val:  the xmlChar to search
*
* a strchr for xmlChar's
*
* Returns the xmlChar * for the first occurence or XNULL.
*/
XCONST xmlChar *xmlStrchr(XCONST xmlChar *str, xmlChar val) 
{
    if (str == XNULL) return(XNULL);
    while (*str != 0) 
    {
        if (*str == val) return((xmlChar *) str);
        str++;
    }
    return(XNULL);
}
/**
* xmlStrstr:
* @str:  the xmlChar * array (haystack)
* @val:  the xmlChar to search (needle)
*
* a strstr for xmlChar's
*
* Returns the xmlChar * for the first occurence or XNULL.
*/
XCONST xmlChar *xmlStrstr(XCONST xmlChar *str, xmlChar *val) 
{
    XS32 n;
    
    if (str == XNULL) return(XNULL);
    if (val == XNULL) return(XNULL);
    n = xmlStrlen(val);
    
    if (n == 0) return(str);
    while (*str != 0) 
    {
        if (*str == *val) 
        {
            if (!xmlStrncmp(str, val, n)) 
                return((XCONST xmlChar *) str);
        }
        str++;
    }
    return(XNULL);
}

/**
* xmlStrsub:
* @str:  the xmlChar * array (haystack)
* @start:  the index of the first XS8 (zero based)
* @len:  the length of the substring
*
* Extract a substring of a given string
*
* Returns the xmlChar * for the first occurence or XNULL.
*/
xmlChar *xmlStrsub(XCONST xmlChar *str, XS32 start, XS32 len) 
{
    XS32 i;
    
    if (str == XNULL) 
        return(XNULL);
    if (start < 0) 
        return(XNULL);
    if (len < 0) 
        return(XNULL);
    
    for (i = 0;i < start;i++) 
    {
        if (*str == 0) 
            return(XNULL);
        str++;
    }
    if (*str == 0) return(XNULL);
    return(xmlStrndup(str, len));
}

/**
* xmlStrlen:
* @str:  the xmlChar * array
*
* length of a xmlChar's string
*
* Returns the number of xmlChar contained in the ARRAY.
*/

XS32 xmlStrlen(XCONST xmlChar *str) 
{
    XS32 len = 0;
    
    if (str == XNULL) return(0);
    while (*str != 0) 
    {
        str++;
        len++;
    }
    return(len);
}

/**
* xmlStrncat:
* @cur:  the original xmlChar * array
* @add:  the xmlChar * array added
* @len:  the length of @add
*
* a strncat for array of xmlChar's
*
* Returns a new xmlChar * containing the concatenated string.
*/
xmlChar *xmlStrncat(xmlChar *cur, XCONST xmlChar *add, XS32 len) 
{
    XS32 size;
    xmlChar *ret;
    
    if ((add == XNULL) || (len == 0))
        return(cur);
    
    if (cur == XNULL)
        return(xmlStrndup(add, len));
    
    size = xmlStrlen(cur);
    ret = xmlRealloc(cur, (size + len + 1) * sizeof(xmlChar));
    if (ret == XNULL) 
    {
        return(cur);
    }
    XOS_MemCpy(&ret[size], add, len * sizeof(xmlChar));
    ret[size + len] = 0;
    return(ret);
}

/**
* xmlStrcat:
* @cur:  the original xmlChar * array
* @add:  the xmlChar * array added
*
* a strcat for array of xmlChar's
*
* Returns a new xmlChar * containing the concatenated string.
*/
xmlChar *xmlStrcat(xmlChar *cur, XCONST xmlChar *add) 
{
    XCONST xmlChar *p = add;
    
    if (add == XNULL) 
    {
        return(cur);
    }
    if (cur == XNULL) 
    {
        return(xmlStrdup(add));
    }
    
    while (IS_CHAR(*p)) 
    {
        p++;
    }
    
    return(xmlStrncat(cur, add, (XS32)(p - add)));
}

/**
* xmlStrdup:
* @cur:  the input xmlChar *
*
* a strdup for array of xmlChar's
*
* Returns a new xmlChar * or XNULL
*/
xmlChar *xmlStrdup(XCONST xmlChar *cur) 
{
    XCONST xmlChar *p = cur;
    
    if (cur == XNULL) return(XNULL);
    while (*p) p++;
    return(xmlStrndup(cur, (XS32)(p - cur)));
}

/**
 * xmlStrncasecmp:
 * @str1:  the first xmlChar *
 * @str2:  the second xmlChar *
 * @len:  the max comparison length
 *
 * a strncasecmp for xmlChar's
 *
 * Returns the integer result of the comparison
 */

int
xmlStrncasecmp(const xmlChar *str1, const xmlChar *str2, int len) {
    register int tmp;

    if (len <= 0) return(0);
    if (str1 == str2) return(0);
    if (str1 == NULL) return(-1);
    if (str2 == NULL) return(1);
    do {
        tmp = casemap[(int)(*str1++)] - casemap[(int)(*str2)];
        if (tmp != 0 || --len == 0) return(tmp);
    } while (*str2++ != 0);
    return 0;
}

int xmlGetUTF8Char(const xmlChar *utf, int *len) {
    unsigned int c;

    if (utf == NULL)
        goto error;
    if (len == NULL)
        goto error;
    if (*len < 1)
        goto error;

    c = utf[0];
    if (c & 0x80) {
        if (*len < 2)
            goto error;
        if ((utf[1] & 0xc0) != 0x80)
            goto error;
        if ((c & 0xe0) == 0xe0) {
            if (*len < 3)
                goto error;
            if ((utf[2] & 0xc0) != 0x80)
                goto error;
            if ((c & 0xf0) == 0xf0) {
                if (*len < 4)
                    goto error;
                if ((c & 0xf8) != 0xf0 || (utf[3] & 0xc0) != 0x80)
                    goto error;
                *len = 4;
                /* 4-byte code */
                c = (utf[0] & 0x7) << 18;
                c |= (utf[1] & 0x3f) << 12;
                c |= (utf[2] & 0x3f) << 6;
                c |= utf[3] & 0x3f;
            } else {
              /* 3-byte code */
                *len = 3;
                c = (utf[0] & 0xf) << 12;
                c |= (utf[1] & 0x3f) << 6;
                c |= utf[2] & 0x3f;
            }
        } else {
          /* 2-byte code */
            *len = 2;
            c = (utf[0] & 0x1f) << 6;
            c |= utf[1] & 0x3f;
        }
    } else {
        /* 1-byte code */
        *len = 1;
    }
    return(c);

error:
    if (len != NULL)
    *len = 0;
    return(-1);
}

/**
 * xmlCheckUTF8:
 * @utf: Pointer to putative UTF-8 encoded string.
 *
 * Checks @utf for being valid UTF-8. @utf is assumed to be
 * null-terminated. This function is not super-strict, as it will
 * allow longer UTF-8 sequences than necessary. Note that Java is
 * capable of producing these sequences if provoked. Also note, this
 * routine checks for the 4-byte maximum size, but does not check for
 * 0x10ffff maximum value.
 *
 * Return value: true if @utf is valid.
 **/
int xmlCheckUTF8(const xmlChar *utf)
{
    int ix;
    xmlChar c;

    if (utf == NULL)
        return(0);
    /*
     * utf is a string of 1, 2, 3 or 4 bytes.  The valid strings
     * are as follows (in "bit format"):
     *    0xxxxxxx                                      valid 1-byte
     *    110xxxxx 10xxxxxx                             valid 2-byte
     *    1110xxxx 10xxxxxx 10xxxxxx                    valid 3-byte
     *    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx           valid 4-byte
     */
    for (ix = 0; (c = utf[ix]);) {      /* string is 0-terminated */
        if ((c & 0x80) == 0x00) {    /* 1-byte code, starts with 10 */
            ix++;
    } else if ((c & 0xe0) == 0xc0) {/* 2-byte code, starts with 110 */
        if ((utf[ix+1] & 0xc0 ) != 0x80)
            return 0;
        ix += 2;
    } else if ((c & 0xf0) == 0xe0) {/* 3-byte code, starts with 1110 */
        if (((utf[ix+1] & 0xc0) != 0x80) ||
            ((utf[ix+2] & 0xc0) != 0x80))
            return 0;
        ix += 3;
    } else if ((c & 0xf8) == 0xf0) {/* 4-byte code, starts with 11110 */
        if (((utf[ix+1] & 0xc0) != 0x80) ||
            ((utf[ix+2] & 0xc0) != 0x80) ||
        ((utf[ix+3] & 0xc0) != 0x80))
            return 0;
        ix += 4;
    } else                /* unknown encoding */
        return 0;
      }
      return(1);
}

/**
 * xmlCopyCharMultiByte:
 * @out:  pointer to an array of xmlChar
 * @val:  the char value
 *
 * append the char value in the array 
 *
 * Returns the number of xmlChar written
 */
int xmlCopyCharMultiByte(xmlChar *out, int val) {
    if (out == NULL) return(0);
    /*
     * We are supposed to handle UTF8, check it's valid
     * From rfc2044: encoding of the Unicode values on UTF-8:
     *
     * UCS-4 range (hex.)           UTF-8 octet sequence (binary)
     * 0000 0000-0000 007F   0xxxxxxx
     * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
     * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx 
     */
    if  (val >= 0x80) {
        xmlChar *savedout = out;
        int bits;
        if (val <   0x800) { *out++= (val >>  6) | 0xC0;  bits=  0; }
        else if (val < 0x10000) { *out++= (val >> 12) | 0xE0;  bits=  6;}
        else if (val < 0x110000)  { *out++= (val >> 18) | 0xF0;  bits=  12; }
        else {
            return(0);
        }
        for ( ; bits >= 0; bits-= 6)
            *out++= ((val >> bits) & 0x3F) | 0x80 ;
        return (XS32)(out - savedout);
    }
    *out = (xmlChar) val;
    return 1;
}

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

