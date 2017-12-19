/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMdbInclude.h
* Author:       luhaiyan
* Date：        09-29-2014
* OverView:     内存数据库对外提供的接口
*
* History:      最新历史修改在最前面
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*******************************************************************************/
#ifndef __SPR_MDB_INCLUDE_H__
#define __SPR_MDB_INCLUDE_H__




#ifdef __cplusplus
extern "C" {
#endif


typedef XS8 (*callBackDiamUaInit)(XVOID);

typedef XS32 (*callBackDbInitData)(XU32 fid);

typedef XS32 (*callBackXOSCliExtPrintf)( CLI_ENV* pCliEnv, const XCHAR* pFmt, ... );

#ifdef __cplusplus
}
#endif 

#endif 