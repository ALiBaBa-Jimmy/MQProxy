/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     oam_file.h
* Author:       xhm
* Date：        2014-09-02
* OverView:     process install message
*
* History:      create
* Revisor:      xhm
* Date:         2014-09-02
* Description:  create the file
*******************************************************************************/
#ifndef __OAM_FILE_H__
#define __OAM_FILE_H__

#include "xosmodule.h"
#include "xoshash.h"
#include "xosarray.h"
#include "xosshell.h"
#include "xmltree.h"
#include "xwriter.h"
#include "xmlparser.h"
#include "oam_cli.h"
#include "oam_main.h"

#pragma pack(1)

#define FMT_VAL_MAX_LEN         32  //格式化字段值的最大长度
typedef enum fld_type_e{
    FLD_TYPE_INT = 1,     //4字节整型
    FLD_TYPE_SHORT = 2,   //短整型short
    FLD_TYPE_CHAR = 3,    //一个字节
    FLD_TYPE_LONG = 4,    //8个字节
    FLD_TYPE_STRING = 5,  //字符串
    FLD_TYPE_BUTT
}FLD_TYPE_E;


extern XVOID OAM_XmlCfgDataFmt(XS8* pCfgData, XU32 uiDataLen, XS8* pRetData, XU32 *pRetValLen);
extern xmlNodePtr OAM_XmlRootParse(xmlDocPtr doc);
extern XS32 OAM_XmlTblNodeParse(xmlDocPtr doc, xmlNodePtr cur, 
                                    AGT_OAM_CFG_REQ_T *pCfgData, XU32 uiTableId, XU32 uiModuleId);
extern XS32 OAM_XmlRecNodeGet(xmlDocPtr doc, xmlNodePtr subCur, 
                                                    AGT_OAM_CFG_REQ_T *pCfgData, XU32 uiTableId);
extern XS32 OAM_XmlFilePathGet();
extern XS32 OAM_XmlCfgDataWrite(AGT_OAM_CFG_REQ_T* pCfgData, XU32 uiType);
extern XS32 OAM_XmlTblInfoGet(XS8* pFileName, XU32 uiTableId, XU32 uiModuleId, XU32 uiType);
extern XS32 OAM_XmlNewDocCreate(xmlDocPtr doc, xmlNodePtr rootNode);
extern XVOID OAM_XmlTblNodeWrite(xmlNodePtr curNode, AGT_OAM_CFG_REQ_T* pCfgData, 
                                                                    XU32 uiType, XU32 uiRecNum );
extern XVOID OAM_XmlTblDataAdd(xmlNodePtr rootNode, AGT_OAM_CFG_REQ_T* pCfgData, 
                                                                        XBOOL bHeadFlag);
#pragma pack()

#endif

