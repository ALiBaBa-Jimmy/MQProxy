/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月7日
**************************************************************************/
#ifndef __DIAM_TYPEDEF_H__
#define __DIAM_TYPEDEF_H__

#include <string>
#include <list>
#include <api/diam_datatype.h>

static inline DiamUINT32 DIAM_SWAP_32(DiamUINT32 b)
{
    return ((((b) & 0xff000000) >> 24) | (((b) & 0x00ff0000) >>  8) | \
            (((b) & 0x0000ff00) <<  8) | (((b) & 0x000000ff) << 24));
}

static inline DiamUINT64 DIAM_SWAP_64(DiamUINT64 b)
{
    union {
        DiamUINT64 ll;
        DiamUINT32 l[2];
    } w, r;
    w.ll = b;
    r.l[0] = DIAM_SWAP_32( w.l[1] );
    r.l[1] = DIAM_SWAP_32( w.l[0] );
    return r.ll;
}

#if __BYTE_ORDER == __BIG_ENDIAN
#define DIAM_NTOH_32(x) (x)
#define DIAM_NTOH_64(x) (x)
#define DIAM_HTON_32(x) (x)
#define DIAM_HTON_64(x) (x)
#else
#define DIAM_NTOH_32(x) DIAM_SWAP_32(x)
#define DIAM_NTOH_64(x) DIAM_SWAP_64(x)
#define DIAM_HTON_32(x) DIAM_SWAP_32(x)
#define DIAM_HTON_64(x) DIAM_SWAP_64(x)
#endif

#endif /*__DIAM_TYPEDEF_H__*/

