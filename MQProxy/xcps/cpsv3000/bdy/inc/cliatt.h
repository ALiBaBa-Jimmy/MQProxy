/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: clicmds.h
**
**  description:  XOSϵͳģ��������ͷ�ļ�
**
**  author: zhanglei
**
**  date:   2006.3.7
**
***************************************************************
**                           history                     
**  
***************************************************************
**   author          date              modification            
**   zhanglei         2006.3.7              create  
**************************************************************/

#ifndef _CLIATT_H_
#define _CLIATT_H_

#ifdef  __cplusplus
extern  "C"
{
#endif


/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/ 
#include "cliconfig.h"

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
                �ṹ��ö������
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                API ����
-------------------------------------------------------------------------*/

XPUBLIC XVOID ATT_CliMsgSwitch( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv );
XPUBLIC XVOID ATT_CliAppFIDInfor( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv );
XPUBLIC XVOID ATT_CliAttInfor( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv );

#ifdef  __cplusplus
}
#endif

#endif /* _CLIATT_H_ */
