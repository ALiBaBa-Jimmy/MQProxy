/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: parser.h
**
**  description: : Interfaces, constants and types related to the XML parser.
**
**  author: zhanglei
**
**  date:   2006.3.7
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   zhanglei         2006.3.7              create
**************************************************************/

#ifndef __XOSXML_H__
#define __XOSXML_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "xostype.h"
#include "xoscfg.h"
    
#define MAX_MEMBLOCK_NUM 50
#ifndef XOS_MAX_PATH
#define XOS_MAX_PATH 255
#endif
#ifndef USERNAME_PWD_LEN
#define USERNAME_PWD_LEN 50
#endif
    
    /*读热补丁配置文件的信息*/
#ifndef XOS_MAX_PATH_NUM_STR
#define  XOS_MAX_PATH_NUM_STR 10     /*热补丁的个数字符串长度*/
#endif
    
#ifndef XOS_PATH_LOAD_STR
#define  XOS_PATH_LOAD_STR 2        /*热补丁是否自动加载标志的字符串长度*/
#endif


typedef struct
{
    XCHAR  maxPatchNum[XOS_MAX_PATH_NUM_STR];  /* 热补丁最大个数,字符串格式*/
    XCHAR  patchPath[XOS_MAX_PATH];            /* 热补丁路径,字符串格式*/
    XCHAR  patchLoadFlag[XOS_PATH_LOAD_STR];   /* 热补丁是否自动加载标志,字符串格式,0/1*/
}t_PATCHCFG;

typedef struct
{
    XU32 blockSize;     /*内存块的大小*/
    XU32 blockNums;  /*内存的块的个数*/
}t_MEMBLOCK;

/*定义内存出错管理结构*/
typedef struct
{
    XS8  _enableBoot;
    XS32 _logStackNum;
}t_MEMEXCFG;

/*读配置文件的信息*/
typedef struct
{
    XU32  memTypes; /* 配置文件中配置的内存的条目数*/
    t_MEMBLOCK *pMemBlock; /*读出来数据信息的首地址*/
}t_MEMCFG;



/*att 需读配置文件获取的数据*/
typedef struct
{
    XU16  maxAtts;  /*支持的最大的att数量*/
    XU16  port;     /*att 在xos这端的绑定的端口*/
    XU32  ip;       /*att 在xos这端绑定的ip , 单ip时填零就可以*/
} t_ATTGENCFG;

/* Telnet D 需读配置文件获取的数据 */
typedef struct
{
    XU16  maxTelClients;  /*支持的最大的客户端数量*/
    XU16  port;           /*监听端口*/
    XU32  ip;             /*监听ip , 单ip时填零就可以*/
} t_TelnetDCfg;


/*日志模块配置参数*/
typedef struct t_LogCfg
{
    XU8   _nEnableRemote;       /*是否支持远程记录日志*/
    XU32  _nRemoteAddr;         /*远程服务器IP地址*/
    XU16  _nRemotePort;         /*远程服务器端口号*/
    XU16  _nLocalPort;          /*本地端口号*/
    XU32  _nMaxFlow;            /*模块日志流量*/
    XU32  _nLogSize;            /*本地日志文件大小*/
}t_LogCfg;

/* HB configuration */
typedef struct
{
    XU32  m_u16HB;           /*HB 时间间隔*/
    XU16  m_u32HBPortLocal;  /*HB 发向 SWD 端口*/
    XU32  m_u32HBIpLocal;    /*SWD IP */
    
} t_FaultMHBCfg;

/*-------------------------------------------------------------------------
API 声明
-------------------------------------------------------------------------*/
XBOOL XML_ReadAttGenCfg( t_ATTGENCFG* pAttGenCfg , XCHAR* filename );
XBOOL XML_GetTelDCfgFromXosXml( t_TelnetDCfg* pTelDCfg, XCHAR* szFileName );
XS16 XML_readMemCfg( t_MEMCFG * pMemCfg, XCONST XS8 *filename );
XS16 XML_readMemExCfg( t_MEMEXCFG * pMemExCfg, XCONST XS8 *filename );
XS16 XML_readMemCfgRelease( t_MEMCFG * pMemCfg );
XBOOL XML_ReadLogCfg(t_LogCfg *logcfgs, XCHAR * filename);
XBOOL XML_GetHBCfgFromXosXml( t_FaultMHBCfg* pFMCfg, XCONST XS8 *filename );
XBOOL XOS_GetLogServFromXml(XU32 *IP,XU32 *Port,XCHAR * filename);
XBOOL XML_readPatchCfg( t_PATCHCFG * pPatchCfg, XCONST XS8 *filename );

#ifdef __cplusplus
}
#endif

#endif /* __XOSXML_H__ */


