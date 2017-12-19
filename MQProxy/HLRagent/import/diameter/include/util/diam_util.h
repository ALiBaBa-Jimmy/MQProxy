
/***************************************************************
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  filename:       diam_utillock.h
**  description:
**  author:         panjian
**  data:           2014.07.16
***************************************************************
**                          history
***************************************************************
**   author          date              modification
**   panjian         2014.07.16        create
**************************************************************/
#ifndef __DIAM_UTIL_H__
#define __DIAM_UTIL_H__

#include <api/diam_datatype.h>
#include <stdio.h>

#define INET_ADDR_LEN 18

#define Inet_ntoa(naddr,straddr) { \
    register char *__p = (char *)&naddr; \
    sprintf(straddr, "%d.%d.%d.%d", \
    __p[0]&0xff, __p[1]&0xff, __p[2]&0xff, __p[3]&0xff); \
}

DiamUINT64 GetIdentify(DiamUINT32 ulIPAddr, DiamUINT32 port);

DiamBool IptoStr( DiamUINT32 inetAddress , char *pString );

DiamUINT32 StrToIp(const char *pString);

#endif //


