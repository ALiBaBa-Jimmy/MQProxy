#ifndef _SPR_MDB_OAM_STRUCT_H__
#define _SPR_MDB_OAM_STRUCT_H__
//#ifdef __cplusplus
//extern "C" {
//#endif
//
#include <vector>
#include <string>

#pragma pack(1)

#define MAX_LEN_IP_ADDR 16
#define MAX_LEN_DESC 128
#define MAX_LEN_TEL_PRE 32
#define MAX_LEN_IMSI_PRE 16
#define MAX_LEN_INDEX_VALUE 32
#define LENGTH_OF_DB_USER_NAME 32
#define LENGTH_OF_DB_PWD 32
#define LENGTH_OF_DB_NAME 32

#define MAX_RECORD_NUM_IP_CFG 32
#define MAX_RECORD_NUM_OSS_CFG 32
#define MAX_RECORD_NUM_TEL_PRE_CFG 1024
#define MAX_RECORD_NUM_IMSI_PRE_CFG 65535
#define MA_RECORD_NUM_SOFT_PARA 32 
#define EHSS_OAM_SOFT_PARA_LEN  64
typedef enum 
{
	E_DB_LINK_STATUS_SUCCESS=0,
	E_DB_LINK_STATUS_ERROR=1
}E_DB_LINK_STATUS;

typedef enum 
{
	E_IPTYPE_OSSACC_IP = 0,
	E_IPTYPE_DIAMETER_IP = 1
}E_EHSSOAM_IPTYPE;

//IP配置表
typedef struct
{
	XU32 iIndex;
	XU32 iIpType;//IP类型，0:浮动IP
	XU8  cIpAddr[MAX_LEN_IP_ADDR];
	XU8  cIpNetMask[MAX_LEN_IP_ADDR];
	XU32 iIpEtheMent;//地址类型-枚举：21：BACK1;22：BACK2;41：FAB1;42：FAB2
	XU8  cDesc[MAX_LEN_DESC];
}SEhssOamIpCfg;

//可接入受理台配置表
typedef struct
{
	XU32 iIndex;
	XU8  cPeerIpAddr[MAX_LEN_IP_ADDR];
	XU16 sPeerIpPort;
	XU8  cDesc[MAX_LEN_DESC];
}SEhssOamAccOssCfg;

//电话号码前缀配置表
typedef struct
{
	XU8  cTelPre[MAX_LEN_TEL_PRE];
	XU8  cEhssIp[MAX_LEN_IP_ADDR];
}SEhssOamTelPreCfg;

//IMSI号段配置
typedef struct
{
	XU8 cImsiPre[MAX_LEN_IMSI_PRE];
	XU8 cEhssIp[MAX_LEN_IP_ADDR];
	XU8  cDesc[MAX_LEN_DESC];
}SEhssOamImsiPreCfg;

//软件参数配置
typedef struct
{
	XU32 iIndexId; //参数ID
	XU8 cIndexValue[MAX_LEN_INDEX_VALUE];//参数值
}SEhssOamSoftParaCfg;
//软件参数配置
typedef struct
{
    XU8 szParaName[EHSS_OAM_SOFT_PARA_LEN];
	XU32 iIndexId; //参数ID
	//XU8 szParaName[64];
	XU32 iValue;//参数值
}tSEhssOamSoftParaCfg;
//DB配置
typedef struct
{
	XU8 cIpAddr[MAX_LEN_IP_ADDR];
	XU8 cUserName[LENGTH_OF_DB_USER_NAME];
	XU8 cPwd[LENGTH_OF_DB_PWD];
	XU8 cDbName[LENGTH_OF_DB_NAME];
}SEhssOamDbCfg;

//查询DB配置表结果
typedef struct
{
	 XU32 recIndex;
	 XU8 cDbLinkStatus;
}SEhssOamDbQryResult;

//OssPort配置
typedef struct
{
	 XU16 ehssOssPort;          //OSS端口	
}SEhssOssPortCfg;

//电话号码前缀列表
typedef struct
{
	std::vector<std::string> vectorTelPreList;	
}STelPreList;


//imsi前缀列表
typedef struct
{
	std::vector<std::string> vectorImsiPreList;	
}SImsiPreList;




#pragma pack()


//#ifdef __cplusplus
//}
//#endif

#endif
