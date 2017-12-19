/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  filename:       diam_datatype.h
**  description:
**  author:         panjian
**  data:           2014.07.16
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   panjian         2014.07.16        create
**************************************************************/
#ifndef __DIAM_DATATYPE_H__
#define __DIAM_DATATYPE_H__

#include <string>
#include <list>

typedef enum
{
    DIAM_AVP_TYPE_UNKNOWN,
    DIAM_AVP_TYPE_OCTETSTRING,
    DIAM_AVP_TYPE_ADDRESS,
    DIAM_AVP_TYPE_INT32,
    DIAM_AVP_TYPE_INT64,
    DIAM_AVP_TYPE_UINT32,
    DIAM_AVP_TYPE_UINT64,
    DIAM_AVP_TYPE_UTF8STRING,
    DIAM_AVP_TYPE_ENUM,
    DIAM_AVP_TYPE_DIAMIDENTITY,
    DIAM_AVP_TYPE_DIAMURI,
    DIAM_AVP_TYPE_GROUPED,
    DIAM_AVP_TYPE_TIME,
    DIAM_AVP_TYPE_IPFILTER_RULE,
    DIAM_AVP_TYPE_QOSFILTER_RULE
} AvpDataType;

typedef unsigned int DiamCommandCode;
typedef unsigned int DiamVendorId;
typedef unsigned int DiamAVPCode;
typedef unsigned int DiamAppId;
typedef unsigned int DiamAvpFlag;

typedef bool               DiamBool;
typedef char               DiamChar;
typedef char               DiamINT8;
typedef short              DiamINT16;
typedef int                DiamINT32;
typedef long long          DiamINT64;
typedef unsigned char      DiamUINT8;
typedef unsigned short     DiamUINT16;
typedef unsigned int       DiamUINT32;
typedef unsigned long long DiamUINT64;
typedef unsigned int       DiamEnum;
typedef unsigned int       DiamTime;
typedef std::string        DiamUTF8String;
typedef std::string        DiamIdentity;
typedef std::string        DiamIPAddress;
typedef class DiamBody     DiamGroup;

#define COMMON_LENGTH  4096

typedef struct OctetString
{
    DiamUINT32 length;
    DiamChar   value[COMMON_LENGTH];
} DiamOctetString;

typedef struct
{
    DiamUINT16      type;
    DiamOctetString address;
} DiamAddress;

typedef struct
{
    DiamUTF8String fqdn;
    DiamUINT16     port;
    DiamUINT8      transport:2;
    DiamUINT8      protocol:2;
    DiamUINT8      scheme:2;
} DiamURI;

enum {
    DIAM_IPFILTER_RULE_SRCDST_EXACT,
    DIAM_IPFILTER_RULE_SRCDST_MASK,
    DIAM_IPFILTER_RULE_SRCDST_KEYWORD_ANY,
    DIAM_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED
};

class DiamUInt8Range
{
public:
    DiamUInt8Range(DiamUINT8 first, DiamUINT8 last) : first(first), last(last)
    {}
    DiamUInt8Range()
    {}
    DiamUINT8 first;
    DiamUINT8 last;
};

class DiamUInt16Range
{
public:
    DiamUInt16Range(DiamUINT16 first, DiamUINT16 last) :
        first(first), last(last)
    {}
    DiamUInt16Range(DiamUINT16 single) : first(single), last(single)
    {}
    DiamUInt16Range()
    {}
    DiamUINT16 first;
    DiamUINT16 last;
};

class DiamIPFilterRuleSrcDst
{
public:
    DiamIPFilterRuleSrcDst(DiamUINT8 repr=DIAM_IPFILTER_RULE_SRCDST_EXACT,
                           DiamUTF8String ipno=std::string(),
                           DiamUINT8 bits=0,
                           bool mod=true)
        : modifier(mod), representation(repr), ipno(ipno), bits(bits)
    {}
    bool modifier;
    DiamUINT8 representation;
    DiamUTF8String ipno;
    DiamUINT8 bits;
    std::list<DiamUInt16Range> portRangeList;
};

/*! IPFilterRule type. */
class DiamIPFilterRule
{
public:
    DiamIPFilterRule() : frag(false), established(false), setup(false)
    {}
    DiamUINT8 action;
    DiamUINT8 dir;
    DiamUINT8 proto;
    DiamIPFilterRuleSrcDst src, dst;
    bool frag;
    std::list<int> ipOptionList;
    std::list<int> tcpOptionList;
    bool established;
    bool setup;
    std::list<int> tcpFlagList;
    std::list<DiamUInt8Range> icmpTypeRangeList;
} ;

typedef enum
{
    DIAM_ADDRESS_RESERVED = 0,
    DIAM_ADDRESS_IP,   // IP (IP version 4)
    DIAM_ADDRESS_IP6,  // IP6 (IP version 6)
    DIAM_ADDRESS_NSAP, // NSAP
    DIAM_ADDRESS_HDLC, // (8-bit multidrop)
    DIAM_ADDRESS_BBN,  // 1822
    DIAM_ADDRESS_802,  // 802 (includes all 802 media plus Ethernet "canonical format")
    DIAM_ADDRESS_E163, // E.163
    DIAM_ADDRESS_E164, // E.164 (SMDS, Frame Relay, ATM)
    DIAM_ADDRESS_F69,  // F.69 (Telex)
    DIAM_ADDRESS_X121, // (X.25, Frame Relay)
    DIAM_ADDRESS_IPX,  // IPX
    DIAM_ADDRESS_ATALK, // Appletalk
    DIAM_ADDRESS_DECIV, // Decnet IV
    DIAM_ADDRESS_BVINE, // Banyan Vines
    DIAM_ADDRESS_E164N, // E.164 with NSAP format subaddress
    DIAM_ADDRESS_DNS,   // DNS (Domain Name System)
    DIAM_ADDRESS_DN,    // Distinguished Name
    DIAM_ADDRESS_ASN,   // AS Number
    DIAM_ADDRESS_XTPV4, // XTP over IP version 4
    DIAM_ADDRESS_XTPV6, // XTP over IP version 6
    DIAM_ADDRESS_XTP,   // native mode XTP
    DIAM_ADDRESS_FCP,   // Fibre Channel World-Wide Port Name
    DIAM_ADDRESS_FCN,   // Fibre Channel World-Wide Node Name
    DIAM_ADDRESS_GWID,  // GWID
    DIAM_ADDRESS_RESERVED2 = 65535
} EDiamAddrType;

typedef  short              INT16;
typedef  int                INT32;
typedef  long long          INT64;
typedef  unsigned char      UINT8;
typedef  unsigned short     UINT16;
typedef  unsigned int       UINT32;
typedef  unsigned long long UINT64;
typedef  unsigned char      BYTE;
typedef  bool               Boolean;

#endif //__DIAM_DATATYPE_H__

