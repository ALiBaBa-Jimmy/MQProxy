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
    
    /*���Ȳ��������ļ�����Ϣ*/
#ifndef XOS_MAX_PATH_NUM_STR
#define  XOS_MAX_PATH_NUM_STR 10     /*�Ȳ����ĸ����ַ�������*/
#endif
    
#ifndef XOS_PATH_LOAD_STR
#define  XOS_PATH_LOAD_STR 2        /*�Ȳ����Ƿ��Զ����ر�־���ַ�������*/
#endif


typedef struct
{
    XCHAR  maxPatchNum[XOS_MAX_PATH_NUM_STR];  /* �Ȳ���������,�ַ�����ʽ*/
    XCHAR  patchPath[XOS_MAX_PATH];            /* �Ȳ���·��,�ַ�����ʽ*/
    XCHAR  patchLoadFlag[XOS_PATH_LOAD_STR];   /* �Ȳ����Ƿ��Զ����ر�־,�ַ�����ʽ,0/1*/
}t_PATCHCFG;

typedef struct
{
    XU32 blockSize;     /*�ڴ��Ĵ�С*/
    XU32 blockNums;  /*�ڴ�Ŀ�ĸ���*/
}t_MEMBLOCK;

/*�����ڴ�������ṹ*/
typedef struct
{
    XS8  _enableBoot;
    XS32 _logStackNum;
}t_MEMEXCFG;

/*�������ļ�����Ϣ*/
typedef struct
{
    XU32  memTypes; /* �����ļ������õ��ڴ����Ŀ��*/
    t_MEMBLOCK *pMemBlock; /*������������Ϣ���׵�ַ*/
}t_MEMCFG;



/*att ��������ļ���ȡ������*/
typedef struct
{
    XU16  maxAtts;  /*֧�ֵ�����att����*/
    XU16  port;     /*att ��xos��˵İ󶨵Ķ˿�*/
    XU32  ip;       /*att ��xos��˰󶨵�ip , ��ipʱ����Ϳ���*/
} t_ATTGENCFG;

/* Telnet D ��������ļ���ȡ������ */
typedef struct
{
    XU16  maxTelClients;  /*֧�ֵ����Ŀͻ�������*/
    XU16  port;           /*�����˿�*/
    XU32  ip;             /*����ip , ��ipʱ����Ϳ���*/
} t_TelnetDCfg;


/*��־ģ�����ò���*/
typedef struct t_LogCfg
{
    XU8   _nEnableRemote;       /*�Ƿ�֧��Զ�̼�¼��־*/
    XU32  _nRemoteAddr;         /*Զ�̷�����IP��ַ*/
    XU16  _nRemotePort;         /*Զ�̷������˿ں�*/
    XU16  _nLocalPort;          /*���ض˿ں�*/
    XU32  _nMaxFlow;            /*ģ����־����*/
    XU32  _nLogSize;            /*������־�ļ���С*/
}t_LogCfg;

/* HB configuration */
typedef struct
{
    XU32  m_u16HB;           /*HB ʱ����*/
    XU16  m_u32HBPortLocal;  /*HB ���� SWD �˿�*/
    XU32  m_u32HBIpLocal;    /*SWD IP */
    
} t_FaultMHBCfg;

/*-------------------------------------------------------------------------
API ����
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


