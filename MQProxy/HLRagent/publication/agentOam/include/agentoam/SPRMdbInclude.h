/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMdbInclude.h
* Author:       luhaiyan
* Date��        09-29-2014
* OverView:     �ڴ����ݿ�����ṩ�Ľӿ�
*
* History:      ������ʷ�޸�����ǰ��
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
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