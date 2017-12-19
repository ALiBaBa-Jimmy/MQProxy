/*
 * ** **************************************************************************
 * ** md5.h -- Header file for implementation of MD5 Message Digest Algorithm **
 * ** Updated: 2/13/90 by Ronald L. Rivest                                    **
 * ** (C) 1990 RSA Data Security, Inc.                                        **
 * ** **************************************************************************
 */

#ifndef _XOSMD5_H_
#define _XOSMD5_H_

#include "xostype.h"
#include "xoscfg.h"

#ifdef __cplusplus
extern          "C" {
#endif
#pragma pack(1)

    /*
     * MDstruct is the data structure for a message digest computation.
     */
    typedef struct {
        unsigned int    buffer[4];      /* Holds 4-word result of MD computation */
        unsigned char   count[8];       /* Number of bits processed so far */
        unsigned int    done;   /* Nonzero means MD computation finished */
    } MDstruct, *MDptr;

    /*MD5命令码定义*/
    typedef enum
    {
        BeginMd5Req, 
        BeginMd5Ack,
        UpdateReq,
        UpdateAck,
        GetReq,
        GetAck,
        CheckSumReq,
        CheckSumAck,
        CheckFileReq,
        CheckFileAck,
    }e_Md5Cmd;

    typedef enum
    {
        Md5Ok = 0,
        Md5NotDone = 1,
        Md5CountErr = 2,
    }e_Md5Result;
    
    typedef struct t_MD5HEADER
    {
        XS32 length; //消息体长度
        MDstruct md5;
        XS8 result;
        XS8 *message; //业务消息
    }t_MD5HEADER;
    

    typedef struct t_MD5UPDATER
    {
        XS32 updateLen;
        XS8 *buffer;
    }t_MD5UPDATER;

    typedef struct t_MD5GET
    {
        XS32 bufLen;
        XS8* buffer;
    }t_MD5GET;

    typedef struct t_MD5CHECKSUMREQ
    {
        XS32 srcLen;
        XS8 *srcMsg;
    }t_MD5CHECKSUMREQ;
    
    typedef struct t_MD5CHECKSUMACK
    {
        XS32 srcLen;
        XS32 macLen;
        XS8 *srcMsg;
        XS8 *macMsg;
    }t_MD5CHECKSUMACK;

    typedef struct t_MD5CHECKFILEREQ
    {
        XS8 fileName[255];
    }t_MD5CHECKFILEREQ;
    
    typedef struct t_MD5CHECKFILEACK
    {
        XS8 fileName[255];
        XS32 msgLen;
        XS8 *msg;
    }t_MD5CHECKFILEACK;
    

#pragma pack()


e_Md5Result XOS_MDupdate(MDptr, unsigned char *, unsigned int);
void XOS_MDget(MDstruct * MD, unsigned char * buf, size_t buflen);
int  XOS_MDchecksum(unsigned char * data, size_t len,unsigned char * mac, size_t maclen);
int XOS_MDcheckfile(const char * pfilename,unsigned char * mac,size_t maclen);

    /*
     * ** End of md5.h
     * ****************************(cut)****************************************
     */
#ifdef __cplusplus
}
#endif
#endif                          /* MD5_H */
