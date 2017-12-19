#ifndef __DIAM_APP_CONFIG_H__
#define __DIAM_APP_CONFIG_H__

#include <api/diam_datatype.h>
#include <api/diam_define.h>
#include <map>

struct DiamDataVendorSpecificAppId
{
    DiamUINT32 vendorId;                  // vendor application id
    DiamUINT32 authAppId;                 // auth application id
    DiamUINT32 acctAppId;                 // acct application id
public:
    DiamDataVendorSpecificAppId():vendorId(0),
        authAppId(0),
        acctAppId(0) {}
};

typedef std::list<DiamUINT32>                      DiamApplicationIdLst;
typedef std::list<DiamUINT32>                      DiamSupportedVendorIdLst;
typedef std::list<DiamAddress*>                    DiamHostIpList;
typedef std::list<DiamDataVendorSpecificAppId>     DiamVendorSpecificIdLst;

struct DiamRunTime
{
    DiamUINT32 originStateId; // runtime origin state
public:
    DiamRunTime():originStateId(0) {}
} ;

struct DiamDataGeneral
{
    DiamUTF8String product;                    // readable string product name
    DiamUINT32     version;                    // current version
    DiamUINT32     vendor;                     // local vendor id
    DiamApplicationIdLst supportedVendorIdLst; // supported vendor application id
    DiamApplicationIdLst authAppIdLst;         // auth application id
    DiamApplicationIdLst acctAppIdLst;         // acct application id
    DiamVendorSpecificIdLst vendorSpecificId;  // vendor specific app id
public:
    DiamDataGeneral()
    {
        product ="Diameter" ;
        version = 1;
        vendor = 0 ;
        supportedVendorIdLst.push_back(8);
        authAppIdLst.push_back(16777251);
        authAppIdLst.push_back(16777252);
    }
} ;

struct DiamDataTransportMngt
{
    std::map<std::string, DiamUINT32> ip_map;  // ip list
    DiamUINT32  use_ipv6;                      // IPV6
    DiamUINT32  watchdog_timeout;              // Watchdog timeout
    DiamUINT32  reconnect_interval;            // ReConnect interval
    DiamUINT32  reconnect_max;                 // ReConnect max
    DiamUINT32  retx_interval;                 // Req ReTx interval
    DiamUINT32  retx_max_count;                // Req ReTx max count
    DiamUINT32  rx_buffer_size;                // Receive Buffer Size
    DiamIdentity realm;                        // Realm
    DiamIdentity hostname;                     // Hostname

public:
    DiamDataTransportMngt():
        use_ipv6(0),
        watchdog_timeout(4),
        reconnect_interval(5),
        reconnect_max(4),
        retx_interval(10),
        retx_max_count(4),
        rx_buffer_size(2048),
        hostname(""),
        realm("")
        {}

    void InitHostName(EDiamNEType ne)
    {
        char name[128]= "";
#ifndef WIN32
        gethostname(name, 128);
#else
        strncpy(name, "windows", strlen("windows"));
#endif // WIN32

        switch (ne)
        {
        case DIAM_HSS:
            realm = "ehss.diameter.com";
            break;
        case DIAM_MME:
            realm = "emme.diameter.com";
            break;
        case DIAM_TCF:
            realm = "tcf.diameter.com";
            break;
        case DIAM_SMC:
            realm = "smc.diameter.com";
            break;
        case DIAM_TAS:
            realm = "tas.diameter.com";
            break;
        case DIAM_SPGW:
            realm = "spgw.diameter.com";
            break;
        case DIAM_PCRF:
            realm = "pcrf.diameter.com";
            break;
        default:
            break;
        }
        hostname = name;
        hostname += ".";
        hostname += realm;
    }
} ;

typedef struct
{
    DiamRunTime runtime;                // runtime configuration
    DiamDataGeneral general;            // general configuration
    DiamDataTransportMngt transport;    // transport configuration
} DiamDataRoot;

class DiamCfgLib
{
public:
    //单例构造函数
    static DiamCfgLib* Instance();

public:
    DiamDataRoot* getRootNode();

private:
    static DiamCfgLib* instance;
    DiamCfgLib();
    virtual ~DiamCfgLib();

private:
    DiamDataRoot root;
};

#define DIAM_CFG_ROOT()            (DiamCfgLib::Instance()->getRootNode())
#define DIAM_CFG_RUNTIME()         (&(DiamCfgLib::Instance()->getRootNode()->runtime))
#define DIAM_CFG_GENERAL()         (&(DiamCfgLib::Instance()->getRootNode()->general))
#define DIAM_CFG_TRANSPORT()       (&(DiamCfgLib::Instance()->getRootNode()->transport))

#endif //__DIAM_APP_CONFIG_H__

