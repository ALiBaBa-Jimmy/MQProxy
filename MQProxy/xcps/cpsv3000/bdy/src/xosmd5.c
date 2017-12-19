/*
 * ** **************************************************************************
 * ** md5.c -- Implementation of MD5 Message Digest Algorithm                 **
 * ** Updated: 2/16/90 by Ronald L. Rivest                                    **
 * ** (C) 1990 RSA Data Security, Inc.                                        **
 * ** **************************************************************************
 */

/*
 * Implementation notes:
 * ** This implementation assumes that ints are 32-bit quantities.
 * ** If the machine stores the least-significant byte of an int in the
 * ** least-addressed byte (eg., VAX and 8086), then LOWBYTEFIRST should be
 * ** set to TRUE.  Otherwise (eg., SUNS), LOWBYTEFIRST should be set to
 * ** FALSE.  Note that on machines with LOWBYTEFIRST FALSE the routine
 * ** XOS_MDupdate modifies has a side-effect on its input array (the order of bytes
 * ** in each word are reversed).  If this is undesired a call to XOS_MDreverse(X) can
 * ** reverse the bytes of X back into order after each call to XOS_MDupdate.
 */

/*
 * code uses WORDS_BIGENDIAN defined by configure now  -- WH 9/27/95 
 */

/*
 * Compile-time includes 
 */
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include "xosmd5.h"
#include "xosmodule.h"
#include "xosmem.h"

#ifdef XOS_VXWORKS
#include "xosencap.h"
#endif

/*
 * Compile-time declarations of MD5 ``magic constants''.
 */
#pragma pack(1)
#define I0  0x67452301          /* Initial values for MD buffer */
#define I1  0xefcdab89
#define I2  0x98badcfe
#define I3  0x10325476
#define fs1  7                  /* round 1 shift amounts */
#define fs2 12
#define fs3 17
#define fs4 22
#define gs1  5                  /* round 2 shift amounts */
#define gs2  9
#define gs3 14
#define gs4 20
#define hs1  4                  /* round 3 shift amounts */
#define hs2 11
#define hs3 16
#define hs4 23
#define is1  6                  /* round 4 shift amounts */
#define is2 10
#define is3 15
#define is4 21


/*
 * Compile-time macro declarations for MD5.
 * ** Note: The ``rot'' operator uses the variable ``tmp''.
 * ** It assumes tmp is declared as unsigned int, so that the >>
 * ** operator will shift in zeros rather than extending the sign bit.
 */
#define    f(X,Y,Z)             ((X&Y) | ((~X)&Z))
#define    g(X,Y,Z)             ((X&Z) | (Y&(~Z)))
#define h(X,Y,Z)             (X^Y^Z)
#define i_(X,Y,Z)            (Y ^ ((X) | (~Z)))
#define rot(X,S)             (tmp=X,(tmp<<S) | (tmp>>(32-S)))
#define ff(A,B,C,D,i,s,lp)   A = rot((A + f(B,C,D) + X[i] + lp),s) + B
#define gg(A,B,C,D,i,s,lp)   A = rot((A + g(B,C,D) + X[i] + lp),s) + B
#define hh(A,B,C,D,i,s,lp)   A = rot((A + h(B,C,D) + X[i] + lp),s) + B
#define ii(A,B,C,D,i,s,lp)   A = rot((A + i_(B,C,D) + X[i] + lp),s) + B

#define STDC_HEADERS

#ifdef STDC_HEADERS
#define Uns(num) num##U
#else
#define Uns(num) num
#endif                          /* STDC_HEADERS */

static int g_words_bigendian=0;

/*
 * XOS_MDbegin(MD)
 * ** Input: MD -- an MDptr
 * ** Initialize the MDstruct prepatory to doing a message digest computation.
 */
static void XOS_MDbegin(MDptr);
static void XOS_MDWordsEndian();
static void XOS_MDblock(MDptr, unsigned int *);
static void XOS_MDreverse(unsigned int *);
static XS32 Md5_msgToUser(t_XOSUSERID *pLinkUser, e_Md5Cmd msgType, XVOID *pContent,  XS32 len );
/*
 * XOS_MDupdate(MD,X,count)
 * ** Input: MD -- an MDptr
 * **        X -- a pointer to an array of unsigned characters.
 * **        count -- the number of bits of X to use (an unsigned int).
 * ** Updates MD using the first ``count'' bits of X.
 * ** The array pointed to by X is not modified.
 * ** If count is not a multiple of 8, XOS_MDupdate uses high bits of last byte.
 * ** This is the basic input routine for a user.
 * ** The routine terminates the MD computation when count < 512, so
 * ** every MD computation should end with one call to XOS_MDupdate with a
 * ** count less than 512.  Zero is OK for a count.
 */


/*
 declare FidList information
*/
static XS8 Md5_InitProc(XVOID *Para1, XVOID *Para2 );
static XS8 Md5_MsgProc(XVOID *Para1, XVOID *Para2);

t_XOSFIDLIST g_Md5FidInfo = {
    {"FID_MD5",  XNULL, FID_MD5},
    {Md5_InitProc , XNULL, XNULL},
    {Md5_MsgProc,  XNULL},
    eXOSMode,
    XNULL};


/*
Entry function as Md5 module
*/
XS32  XOS_FIDMD5(HANDLE hDir,XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST NTLLoginList;
    XS32 ret = XSUCC;

    XOS_MemSet( &NTLLoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    NTLLoginList.stack     = &g_Md5FidInfo;
    XOS_StrNcpy(NTLLoginList.taskname , "Tsk_MD5", MAX_TID_NAME_LEN);

    NTLLoginList.TID        = FID_MD5;  
    NTLLoginList.prio      = TSK_PRIO_NORMAL;
    NTLLoginList.quenum = MAX_MSGS_IN_QUE;
    NTLLoginList.stacksize = 102400;
    ret = XOS_MMStartFid(&NTLLoginList, XNULLP, XNULLP);

    return ret;
}

XS8 Md5_InitProc(XVOID *Para1, XVOID *Para2 )
{
    XOS_MDWordsEndian();
    return XSUCC;
}
/*
 * 处理MD5消息
 *
*/
XS8 Md5_MsgProc(XVOID *msg, XVOID *Para)
{
    t_XOSCOMMHEAD * pxosMsg = NULL;
    t_MD5HEADER * pmd5Header = XNULL;
    t_MD5HEADER * pmd5HeaderAck = XNULL;
    t_MD5UPDATER* pUpdater = XNULL;
    t_MD5GET* pGet = XNULL;
    t_MD5CHECKSUMREQ* pmd5CheckSumReq = XNULL;
    t_MD5CHECKSUMACK* pmd5CheckSumAck = XNULL;
    t_MD5CHECKFILEREQ* pmd5CheckFileReq = XNULL;
    t_MD5CHECKFILEACK* pmd5CheckFileAck = XNULL;
    XS32 len = 0;
    
    if(XNULL == msg)
    {
        goto Err;
    }
    pxosMsg=(t_XOSCOMMHEAD*)msg;
    if(FID_MD5 != pxosMsg->datadest.FID)
    {
        XOS_Trace(MD(FID_MD5,PL_ERR),"FID is invalid, invalid FID is %d.", pxosMsg->datadest.FID);
        goto Err;
    }
    pmd5Header = (t_MD5HEADER*)(pxosMsg->message);
    if(pmd5Header == XNULL) {
        XOS_Trace(MD(FID_MD5,PL_ERR),"Md5 Header is invalid, header address is null");
        goto Err;
    }
    
    switch(pxosMsg->msgID)
    {
    case BeginMd5Req:
        pmd5HeaderAck = (t_MD5HEADER*)XOS_MemMalloc(FID_MD5, sizeof(t_MD5HEADER));
        XOS_MemSet(pmd5HeaderAck, 0, sizeof(t_MD5HEADER));
        pmd5HeaderAck->result = Md5Ok;
        XOS_MDbegin(&(pmd5HeaderAck->md5));
        Md5_msgToUser(&(pxosMsg->datasrc),
            BeginMd5Ack,
            (XVOID*)pmd5HeaderAck,
            sizeof(t_MD5HEADER));
        XOS_MemFree(FID_MD5, pmd5HeaderAck);
        break;
    case UpdateReq:
        pUpdater = (t_MD5UPDATER*)pmd5Header->message;
        if(pUpdater == XNULL) {
            XOS_Trace(MD(FID_MD5,PL_ERR),"Md5 Updater request is invalid, the pointer is null");
            goto Err;
        }
        pmd5HeaderAck = (t_MD5HEADER*)XOS_MemMalloc(FID_MD5, sizeof(t_MD5HEADER));
        XOS_MemSet(pmd5HeaderAck, 0, sizeof(t_MD5HEADER));
        pmd5HeaderAck->result = XOS_MDupdate(&(pmd5HeaderAck->md5), (XU8*)(pUpdater->buffer), pUpdater->updateLen);
        Md5_msgToUser(&(pxosMsg->datasrc),
            UpdateAck,
            (XVOID*)pmd5HeaderAck,
            sizeof(t_MD5HEADER));
        break;
    case GetReq:
        len = sizeof(t_MD5HEADER) + sizeof(t_MD5GET) + 16;
        pmd5HeaderAck = (t_MD5HEADER*)XOS_MemMalloc(FID_MD5, len);
        XOS_MemSet(pmd5HeaderAck, 0, len);
        
        pGet = (t_MD5GET*)((XS8*)pmd5HeaderAck + sizeof(t_MD5HEADER));
        pGet->buffer = (XS8*)pGet + sizeof(t_MD5GET);
        pGet->bufLen = 16;

        pmd5HeaderAck->result = Md5Ok;
        pmd5HeaderAck->length = sizeof(t_MD5GET) + 16;
        pmd5HeaderAck->message = (XS8*)pGet;
        XOS_MemCpy(&(pmd5HeaderAck->md5), &(pmd5Header->md5), sizeof(MDstruct));
        
        XOS_MDget(&(pmd5Header->md5), (XU8*)(pGet->buffer), pGet->bufLen);
        
        Md5_msgToUser(&(pxosMsg->datasrc),
            GetAck,
            (XVOID*)pmd5HeaderAck,
            len);
        break;
    case CheckSumReq:
        pmd5CheckSumReq = (t_MD5CHECKSUMREQ*)pmd5Header->message;
        if(pmd5CheckSumReq == XNULL) {
            XOS_Trace(MD(FID_MD5,PL_ERR),"Md5 CheckSum request is invalid, the pointer is null");
            goto Err;
        }
        len = sizeof(t_MD5HEADER) + sizeof(t_MD5CHECKSUMACK) + pmd5CheckSumReq->srcLen + 16;
        pmd5HeaderAck = (t_MD5HEADER*)XOS_MemMalloc(FID_MD5, len);
        pmd5CheckSumAck = (t_MD5CHECKSUMACK*)((XS8*)pmd5HeaderAck + sizeof(t_MD5HEADER));
        
        XOS_MemSet(pmd5HeaderAck, 0, len);

        pmd5HeaderAck->length = sizeof(t_MD5CHECKSUMACK) + pmd5CheckSumReq->srcLen + 16;
        pmd5HeaderAck->message = (XS8*)pmd5CheckSumAck;
        
        pmd5CheckSumAck->srcLen = pmd5CheckSumReq->srcLen;
        pmd5CheckSumAck->srcMsg = (XS8*)pmd5CheckSumAck + sizeof(t_MD5CHECKSUMACK);
        pmd5CheckSumAck->macMsg = (XS8*)pmd5CheckSumAck + sizeof(t_MD5CHECKSUMACK) + pmd5CheckSumAck->srcLen;
        pmd5CheckSumAck->macLen = 16;
        
        XOS_MemCpy(pmd5CheckSumAck->srcMsg, pmd5CheckSumReq->srcMsg, pmd5CheckSumReq->srcLen);
        
        pmd5HeaderAck->result = XOS_MDchecksum((XU8*)(pmd5CheckSumReq->srcMsg),
            pmd5CheckSumReq->srcLen,
            (XU8*)(pmd5CheckSumAck->macMsg),
            pmd5CheckSumAck->macLen);

        Md5_msgToUser(&(pxosMsg->datasrc),
            CheckSumAck,
            (XVOID*)pmd5HeaderAck,
            len);
        break;
    case CheckFileReq:
        pmd5CheckFileReq = (t_MD5CHECKFILEREQ*)pmd5Header->message;
        if(pmd5CheckFileReq == XNULL) {
            XOS_Trace(MD(FID_MD5,PL_ERR),"Md5 CheckFile request is invalid, the pointer is null");
            goto Err;
        }
        len = sizeof(t_MD5HEADER) + sizeof(t_MD5CHECKFILEACK) + 16;
        pmd5HeaderAck = (t_MD5HEADER*)XOS_MemMalloc(FID_MD5, len);
        XOS_MemSet(pmd5HeaderAck, 0, len);

        pmd5CheckFileAck = (t_MD5CHECKFILEACK*)((XS8*)pmd5HeaderAck + sizeof(t_MD5HEADER));
        XOS_MemCpy(pmd5CheckFileAck->fileName, pmd5CheckFileReq->fileName, sizeof(pmd5CheckFileReq->fileName));
        pmd5CheckFileAck->msg = (XS8*)((XS8*)pmd5CheckFileAck + sizeof(t_MD5CHECKFILEACK));
        pmd5CheckFileAck->msgLen = 16;
        pmd5HeaderAck->length = sizeof(t_MD5CHECKFILEACK) + 16;
        pmd5HeaderAck->message = (XS8*)pmd5CheckFileAck;
        pmd5HeaderAck->result = XOS_MDcheckfile(pmd5CheckFileReq->fileName,
            (XU8*)(pmd5CheckFileAck->msg),
            pmd5CheckFileAck->msgLen);

        Md5_msgToUser(&(pxosMsg->datasrc),
            CheckFileAck,
            (XVOID*)pmd5HeaderAck,
            len);
        break;
    default:
        XOS_Trace(MD(FID_MD5,PL_MIN),"invalid msgId %d.", pxosMsg->msgID);
           break;
    }
    return XSUCC;
Err:
    return XERROR;
}


XS32 Md5_msgToUser(t_XOSUSERID *pLinkUser, e_Md5Cmd msgType, XVOID *pContent,  XS32 len )
{
    XS32 ret = 0;
    t_XOSCOMMHEAD *pMsg = NULL;

    if((pContent == XNULL) || (pLinkUser == XNULL) ||(len <= 0) )
    {
        XOS_Trace(MD(FID_MD5,PL_ERR),"Md5_msg ToUser()->input param error!");
        return XERROR;
    }
    /*分配消息内存*/
    pMsg = XOS_MsgMemMalloc(FID_MD5, (XU32)len);
    if(pMsg == XNULLP)
    {
        XOS_Trace(MD(FID_MD5,PL_ERR),"NTL_msg ToUser()->malloc msg failed!");
        return XERROR;
    }

    /*填写消息数据*/
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datasrc.FID = FID_MD5;
    pMsg->length = (XU32)len;
    pMsg->msgID = msgType;
    pMsg->prio = eNormalMsgPrio;
    if(pLinkUser == XNULL )/*8888*/
    {
        XOS_MsgMemFree(FID_MD5, pMsg);
        return XERROR;
    }
    
    XOS_MemCpy(&(pMsg->datadest),pLinkUser,sizeof(t_XOSUSERID));
    pMsg->datadest.PID = pMsg->datasrc.PID; // 20110322 add,
    XOS_MemCpy(pMsg->message, pContent,(XU32)len);

    /*发送数据*/
    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_MD5, PL_ERR),"Md5_msg ToUser()->send msg type[%d] to FID[%d] failed!",msgType,pLinkUser->FID);
        /*clean up */
        XOS_MsgMemFree(FID_MD5, pMsg);
        return XERROR; 
    }

    return XSUCC;
}

/*
 * XOS_MDbegin(MDp)
 * ** Initialize message digest buffer MDp. 
 * ** This is a user-callable routine.
 */
void XOS_MDbegin(MDptr MDp)
{
    int             i;
    MDp->buffer[0] = I0;
    MDp->buffer[1] = I1;
    MDp->buffer[2] = I2;
    MDp->buffer[3] = I3;
    for (i = 0; i < 8; i++)
        MDp->count[i] = 0;
    MDp->done = 0;
}

/*
 * XOS_MDreverse(X)
 * ** Reverse the byte-ordering of every int in X.
 * ** Assumes X is an array of 16 ints.
 * ** The macro revx reverses the byte-ordering of the next word of X.
 */
#define revx { t = (*X << 16) | (*X >> 16); \
           *X++ = ((t & 0xFF00FF00) >> 8) | ((t & 0x00FF00FF) << 8); }

void XOS_MDreverse(unsigned int *X)
{
    register unsigned int t;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
    revx;
}

/*
 * XOS_MDblock(MDp,X)
 * ** Update message digest buffer MDp->buffer using 16-word data block X.
 * ** Assumes all 16 words of X are full of data.
 * ** Does not update MDp->count.
 * ** This routine is not user-callable. 
 */
static void XOS_MDblock(MDptr MDp, unsigned int *X)
{
    register unsigned int tmp, A, B, C, D;      /* hpux sysv sun */

    if(1==g_words_bigendian)
    {
        XOS_MDreverse(X);
    }

    A = MDp->buffer[0];
    B = MDp->buffer[1];
    C = MDp->buffer[2];
    D = MDp->buffer[3];

    /*
     * Update the message digest buffer 
     */
    ff(A, B, C, D, 0, fs1, Uns(3614090360));    /* Round 1 */
    ff(D, A, B, C, 1, fs2, Uns(3905402710));
    ff(C, D, A, B, 2, fs3, Uns(606105819));
    ff(B, C, D, A, 3, fs4, Uns(3250441966));
    ff(A, B, C, D, 4, fs1, Uns(4118548399));
    ff(D, A, B, C, 5, fs2, Uns(1200080426));
    ff(C, D, A, B, 6, fs3, Uns(2821735955));
    ff(B, C, D, A, 7, fs4, Uns(4249261313));
    ff(A, B, C, D, 8, fs1, Uns(1770035416));
    ff(D, A, B, C, 9, fs2, Uns(2336552879));
    ff(C, D, A, B, 10, fs3, Uns(4294925233));
    ff(B, C, D, A, 11, fs4, Uns(2304563134));
    ff(A, B, C, D, 12, fs1, Uns(1804603682));
    ff(D, A, B, C, 13, fs2, Uns(4254626195));
    ff(C, D, A, B, 14, fs3, Uns(2792965006));
    ff(B, C, D, A, 15, fs4, Uns(1236535329));
    gg(A, B, C, D, 1, gs1, Uns(4129170786));    /* Round 2 */
    gg(D, A, B, C, 6, gs2, Uns(3225465664));
    gg(C, D, A, B, 11, gs3, Uns(643717713));
    gg(B, C, D, A, 0, gs4, Uns(3921069994));
    gg(A, B, C, D, 5, gs1, Uns(3593408605));
    gg(D, A, B, C, 10, gs2, Uns(38016083));
    gg(C, D, A, B, 15, gs3, Uns(3634488961));
    gg(B, C, D, A, 4, gs4, Uns(3889429448));
    gg(A, B, C, D, 9, gs1, Uns(568446438));
    gg(D, A, B, C, 14, gs2, Uns(3275163606));
    gg(C, D, A, B, 3, gs3, Uns(4107603335));
    gg(B, C, D, A, 8, gs4, Uns(1163531501));
    gg(A, B, C, D, 13, gs1, Uns(2850285829));
    gg(D, A, B, C, 2, gs2, Uns(4243563512));
    gg(C, D, A, B, 7, gs3, Uns(1735328473));
    gg(B, C, D, A, 12, gs4, Uns(2368359562));
    hh(A, B, C, D, 5, hs1, Uns(4294588738));    /* Round 3 */
    hh(D, A, B, C, 8, hs2, Uns(2272392833));
    hh(C, D, A, B, 11, hs3, Uns(1839030562));
    hh(B, C, D, A, 14, hs4, Uns(4259657740));
    hh(A, B, C, D, 1, hs1, Uns(2763975236));
    hh(D, A, B, C, 4, hs2, Uns(1272893353));
    hh(C, D, A, B, 7, hs3, Uns(4139469664));
    hh(B, C, D, A, 10, hs4, Uns(3200236656));
    hh(A, B, C, D, 13, hs1, Uns(681279174));
    hh(D, A, B, C, 0, hs2, Uns(3936430074));
    hh(C, D, A, B, 3, hs3, Uns(3572445317));
    hh(B, C, D, A, 6, hs4, Uns(76029189));
    hh(A, B, C, D, 9, hs1, Uns(3654602809));
    hh(D, A, B, C, 12, hs2, Uns(3873151461));
    hh(C, D, A, B, 15, hs3, Uns(530742520));
    hh(B, C, D, A, 2, hs4, Uns(3299628645));
    ii(A, B, C, D, 0, is1, Uns(4096336452));    /* Round 4 */
    ii(D, A, B, C, 7, is2, Uns(1126891415));
    ii(C, D, A, B, 14, is3, Uns(2878612391));
    ii(B, C, D, A, 5, is4, Uns(4237533241));
    ii(A, B, C, D, 12, is1, Uns(1700485571));
    ii(D, A, B, C, 3, is2, Uns(2399980690));
    ii(C, D, A, B, 10, is3, Uns(4293915773));
    ii(B, C, D, A, 1, is4, Uns(2240044497));
    ii(A, B, C, D, 8, is1, Uns(1873313359));
    ii(D, A, B, C, 15, is2, Uns(4264355552));
    ii(C, D, A, B, 6, is3, Uns(2734768916));
    ii(B, C, D, A, 13, is4, Uns(1309151649));
    ii(A, B, C, D, 4, is1, Uns(4149444226));
    ii(D, A, B, C, 11, is2, Uns(3174756917));
    ii(C, D, A, B, 2, is3, Uns(718787259));
    ii(B, C, D, A, 9, is4, Uns(3951481745));

    MDp->buffer[0] += A;
    MDp->buffer[1] += B;
    MDp->buffer[2] += C;
    MDp->buffer[3] += D;
    if(1==g_words_bigendian)
    {
        XOS_MDreverse(X);
    }
}

/*
 * XOS_MDupdate(MDp,X,count)
 * ** Input: MDp -- an MDptr
 * **        X -- a pointer to an array of unsigned characters.
 * **        count -- the number of bits of X to use.
 * **                 (if not a multiple of 8, uses high bits of last byte.)
 * ** Update MDp using the number of bits of X given by count.
 * ** This is the basic input routine for an MD5 user.
 * ** The routine completes the MD computation when count < 512, so
 * ** every MD computation should end with one call to XOS_MDupdate with a
 * ** count less than 512.  A call with count 0 will be ignored if the
 * ** MD has already been terminated (done != 0), so an extra call with count
 * ** 0 can be given as a ``courtesy close'' to force termination if desired.
 * ** Returns : 0 if processing succeeds or was already done;
 * **          -1 if processing was already done
 * **          -2 if count was too large
 */
e_Md5Result XOS_MDupdate(MDptr MDp, unsigned char *X, unsigned int count)
{
    unsigned int    i, tmp, bit, byte, mask;
    unsigned char   XX[64];
    unsigned char  *p;
    /*
     * return with no error if this is a courtesy close with count
     * ** zero and MDp->done is true.
     */
    if (count == 0 && MDp->done)
        return Md5Ok;
    /*
     * check to see if MD is already done and report error 
     */
    if (MDp->done) {
        return Md5NotDone;
    }
    /*
     * if (MDp->done) { fprintf(stderr,"\nError: XOS_MDupdate MD already done."); return; }
     */
    /*
     * Add count to MDp->count 
     */
    tmp = count;
    p = MDp->count;
    while (tmp) {
        tmp += *p;
        *p++ = tmp;
        tmp = tmp >> 8;
    }
    /*
     * Process data 
     */
    if (count == 512) {         /* Full block of data to handle */
        XOS_MDblock(MDp, (unsigned int *) X);
    } else if (count > 512)     /* Check for count too large */
        return Md5CountErr;
    /*
     * { fprintf(stderr,"\nError: XOS_MDupdate called with illegal count value %d.",count);
     * return;
     * }
     */
    else {                      /* partial block -- must be last block so finish up */
        /*
         * Find out how many bytes and residual bits there are 
         */
        int             copycount;
        byte = count >> 3;
        bit = count & 7;
        copycount = byte;
        if (bit)
            copycount++;
        /*
         * Copy X into XX since we need to modify it 
         */
        memset(XX, 0, sizeof(XX));
        memcpy(XX, X, copycount);

        /*
         * Add padding '1' bit and low-order zeros in last byte 
         */
        mask = ((unsigned long) 1) << (7 - bit);
        XX[byte] = (XX[byte] | mask) & ~(mask - 1);
        /*
         * If room for bit count, finish up with this block 
         */
        if (byte <= 55) {
            for (i = 0; i < 8; i++)
                XX[56 + i] = MDp->count[i];
            XOS_MDblock(MDp, (unsigned int *) XX);
        } else {                /* need to do two blocks to finish up */
            XOS_MDblock(MDp, (unsigned int *) XX);
            for (i = 0; i < 56; i++)
                XX[i] = 0;
            for (i = 0; i < 8; i++)
                XX[56 + i] = MDp->count[i];
            XOS_MDblock(MDp, (unsigned int *) XX);
        }
        /*
         * Set flag saying we're done with MD computation 
         */
        MDp->done = 1;
    }
    return 0;
}

void XOS_MDget(MDstruct * MD, unsigned char * buf, size_t buflen)
{
    int             i, j;

    /*
     * copy the checksum to the outgoing data (all of it that is requested). 
     */
    for (i = 0; i < 4 && i * 4 < (int) buflen; i++)
        for (j = 0; j < 4 && i * 4 + j < (int) buflen; j++)
            buf[i * 4 + j] = (MD->buffer[i] >> j * 8) & 0xff;
}

/*
 * XOS_MDchecksum(data, len, MD5): do a checksum on an arbirtrary amount of data 
 */
int XOS_MDchecksum(unsigned char * data, size_t len, unsigned char * mac, size_t maclen)
{
    MDstruct        md;
    MDstruct       *MD = &md;
    int             rc = 0;
    
    XOS_MDWordsEndian();
    XOS_MDbegin(MD);
    while (len >= 64) {
        rc = XOS_MDupdate(MD, data, 64 * 8);
        if (rc)
            goto check_end;
        data += 64;
        len -= 64;
    }
    rc = XOS_MDupdate(MD, data, (XU32)(len * 8));
    if (rc)
        goto check_end;

    /*
     * copy the checksum to the outgoing data (all of it that is requested). 
     */
    XOS_MDget(MD, mac, maclen);

  check_end:
    memset(&md, 0, sizeof(md));
    return rc;
}

int XOS_MDcheckfile(const char * pfilename,unsigned char * mac,size_t maclen)
{
    MDstruct st_md;
    MDstruct *ptr_md = &st_md;
    int  rc = -3;
    char buff[1024]={0};
    unsigned char * ptr_data; 
    int i_readsize=0;
    FILE * ptr_fp = NULL;

#if 0    
#ifdef XOS_VXWORKS
    int taskid;//任务等级
    int taskPro;//任务优先级
    int taskProTmp = 120;
    taskid=taskIdSelf();
    taskPriorityGet(taskid,&taskPro);
    taskPrioritySet(taskid,taskProTmp);
#endif
#endif

    ptr_fp = fopen(pfilename,"rb+");
    if( NULL == ptr_fp)
    {
        goto check_end;
    }
    XOS_MDWordsEndian();
    XOS_MDbegin(ptr_md);

    //md5文件校验时，不能大块得读文件，会造成读取出来的实际字节数不是64的整数倍，造成校验不正确
    while(0<(i_readsize = (XS32)fread(buff, 1, sizeof(buff), ptr_fp)))
    {
        ptr_data=((unsigned char *)buff);
        while (i_readsize >= 64) 
        {
            rc = XOS_MDupdate(ptr_md, ptr_data, 64 * 8);
            if (rc)
            {
                goto check_end;
            }
            ptr_data += 64;
            i_readsize -= 64;
        }
        if(i_readsize>0)
        {
            rc = XOS_MDupdate(ptr_md, ptr_data, i_readsize * 8);
            if (rc)
            {
                goto check_end;
            }
        }
    }
    /*
     * copy the checksum to the outgoing data (all of it that is requested). 
     */
    XOS_MDget(ptr_md, mac, maclen);

  check_end:
    memset(&st_md, 0, sizeof(st_md));
    if( NULL != ptr_fp)
    {
        fclose(ptr_fp);
        ptr_fp = NULL;
    }
#if 0
#ifdef XOS_VXWORKS
    taskPrioritySet(taskid,taskPro);
#endif
#endif
    return rc;
}

void XOS_MDWordsEndian()
{
    union
    {
        short a;
        char c[sizeof(short)];
    }un;

    un.a=0x0102;
    if (sizeof(short)==2)
    {
        if ((un.c[0]==0x02) && (un.c[1]==0x01))
        {
            //LITTLE_ENDIAN
            g_words_bigendian = 0;
        }
        if ((un.c[0]==0x01) && (un.c[1]==0x02))
        {
            //BIG_ENDIAN
            g_words_bigendian = 1;
        }
    }
}

#pragma pack()
/*
 * ** End of md5.c
 * ****************************(cut)****************************************
 */
