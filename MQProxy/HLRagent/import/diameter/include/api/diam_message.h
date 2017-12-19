/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��5��8��
**************************************************************************/
#ifndef __DIAM_MESSAGE_H__
#define __DIAM_MESSAGE_H__

#include <parser/diam_errstatus.h>
#include <parser/diam_avpcontainer.h>
#include <util/diam_config.h>

//ͷ��bitλ
typedef struct
{
    DiamUINT8 r:1; //����/Ӧ���־
    DiamUINT8 p:1; //�Ƿ�ɴ����־
    DiamUINT8 e:1; //������Ϣ��־
    DiamUINT8 t:1; //�ش���Ϣ��־
    DiamUINT8 u:1;
    DiamUINT8 rsvd:3;
} DiamHeadFlag;

typedef struct
{
    DiamUINT8 v;
    DiamUINT8 m;
    DiamUINT8 p;
} AvpHeadFlag;

class DIAMETER_EXPORT DiamHeader
{
public:
    DiamHeader(DiamUINT32 ver,
               DiamUINT32 length,
               DiamHeadFlag flags,
               DiamCommandCode code,
               DiamAppId appId,
               DiamUINT32 hh,
               DiamUINT32 ee);
    DiamHeader();
    CDictObject *getDictHandle();
    const char* getCommandName();

public:
    DiamUINT32    version;
    DiamHeadFlag  flags;
    DiamUINT32    length:24;
    DiamCommandCode code:24;
    DiamAppId appId;
    DiamUINT32    hh;
    DiamUINT32    ee;

public:
    CDictObject* dictHandle;
};

class DIAMETER_EXPORT DiamAvpHeader
{
public:
    DiamAvpHeader();
    DiamUINT32   code;
    AvpHeadFlag  flag;
    DiamUINT32   length:24;
    DiamUINT32   vendor;
    char*        value_p;
    DiamAvpParseType& ParseType();

private:
    DiamAvpParseType parseType;
};

class DIAMETER_EXPORT DiamMsg
{
public:
    DiamMsg();
    ~DiamMsg();

    DiamAppId ApplicationId() {
        return hdr.appId;
    }
    DiamUINT32        HH() {
        return hdr.hh;
    }
    DiamUINT32        EE() {
        return hdr.ee;
    }
    DiamCommandCode   CommandCode() {
        return hdr.code;
    }

public:
    DiamBool request() {
        return hdr.flags.r;
    }
    DiamBool answer() {
        return !hdr.flags.r;
    }
    DiamBool error() {
        return hdr.flags.e;
    }
    DiamBool capabilities() {
        return (hdr.code == DIAM_MSGCODE_CAPABILITIES_EXCHG);
    }
    DiamBool watchdog() {
        return (hdr.code == DIAM_MSGCODE_WATCHDOG);
    }

public:
    DiamHeader  hdr;
    DiamBody    bdy;
};

#endif /*__DIAM_MESSAGE_H__*/

