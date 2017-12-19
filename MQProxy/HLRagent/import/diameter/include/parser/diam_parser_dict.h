#ifndef __DIAM_PARSER_DICT_H__
#define __DIAM_PARSER_DICT_H__

static const char* dataTypeStr[16] =
{
    "Unknown",
    "OctetString",
    "Address",
    "Integer32",
    "Integer64",
    "Unsigned32",
    "Unsigned64",
    "UTF8String",
    "Enumerated",
    "DiameterIdentity",
    "DiameterURI",
    "Grouped",
    "Time",
    "IPFilterRule",
    "QoSFilterRule"
};

/*!
* diameter version declaration
*/
#define DIAM_VERSION_MAJOR          0x01
#define DIAM_VERSION_MINOR          0x00
#define DIAM_VERSION_MICRO          0x05

#define DIAM_NO_VENDOR_ID           0
#define DIAM_BASE_APPLICATION_ID    0

#define DIAM_PROTOCOL_VERSION       0x1
#define DIAM_FLG_SET                0x1
#define DIAM_FLG_CLR                0x0

#define HEADER_SIZE                20
#define QUAL_INFINITY              65535 /* 2^16 -1 */

typedef enum
{
    DIAM_PARSE_ERROR_TYPE_NORMAL  = 0,
    DIAM_PARSE_ERROR_TYPE_BUG     = 1
} DIAM_PARSE_ERROR_TYPE;

enum ParseOption
{
    PARSE_LOOSE = 0,
    PARSE_STRICT = 1
};

typedef enum
{
    MISSING_CONTAINER = 1,
    TOO_MUCH_AVP_ENTRIES,
    TOO_LESS_AVP_ENTRIES,
    PROHIBITED_CONTAINER,
    INVALID_CONTAINER_PARAM,
    INVALID_CONTAINER_CONTENTS,
    UNSUPPORTED_FUNCTIONALITY,
    INVALID_PARSER_USAGE,
    MISSING_AVP_DICTIONARY_ENTRY,
    MISSING_AVP_VALUE_PARSER
} DIAM_PARSE_ERROR;

typedef enum
{
    DIAM_AVP_FLAG_NONE =                 0,
    DIAM_AVP_FLAG_MANDATORY =            0x1,
    DIAM_AVP_FLAG_RESERVED =             0x2,
    DIAM_AVP_FLAG_VENDOR_SPECIFIC =      0x4,
    DIAM_AVP_FLAG_END_TO_END_ENCRYPT =   0x10,
    DIAM_AVP_FLAG_UNKNOWN =              0x10000,
    DIAM_AVP_FLAG_ENCRYPT =              0x40000
} DIAM_AVPFlagEnum;

enum
{
    DIAM_TRANSPORT_PROTO_TCP = 0,
    DIAM_TRANSPORT_PROTO_SCTP,
    DIAM_TRANSPORT_PROTO_UDP
};

enum
{
    DIAM_PROTO_DIAMETER = 0,
    DIAM_PROTO_RADIUS,
    DIAM_PROTO_TACACSPLUS
};

enum
{
    DIAM_SCHEME_AAA = 0,
    DIAM_SCHEME_AAAS
};

enum
{
    DIAM_IPFILTER_RULE_IP_OPTION_SSRR=1,
    DIAM_IPFILTER_RULE_IP_OPTION_LSRR,
    DIAM_IPFILTER_RULE_IP_OPTION_RR,
    DIAM_IPFILTER_RULE_IP_OPTION_TS
};

enum
{
    DIAM_IPFILTER_RULE_TCP_OPTION_MSS=1,
    DIAM_IPFILTER_RULE_TCP_OPTION_WINDOW,
    DIAM_IPFILTER_RULE_TCP_OPTION_SACK,
    DIAM_IPFILTER_RULE_TCP_OPTION_TS,
    DIAM_IPFILTER_RULE_TCP_OPTION_CC
};

enum
{
    DIAM_IPFILTER_RULE_TCP_FLAG_FIN=1,
    DIAM_IPFILTER_RULE_TCP_FLAG_SYN,
    DIAM_IPFILTER_RULE_TCP_FLAG_RST,
    DIAM_IPFILTER_RULE_TCP_FLAG_PSH,
    DIAM_IPFILTER_RULE_TCP_FLAG_ACK,
    DIAM_IPFILTER_RULE_TCP_FLAG_URG
};

enum
{
    DIAM_IPFILTER_RULE_ACTION_PERMIT,
    DIAM_IPFILTER_RULE_ACTION_DENY
};

enum
{
    DIAM_IPFILTER_RULE_DIRECTION_IN,
    DIAM_IPFILTER_RULE_DIRECTION_OUT
};

#endif //__DIAM_PARSER_DICT_H__
