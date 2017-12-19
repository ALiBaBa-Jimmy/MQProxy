/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosapp.h
**
**  description: ����ҵ��ʹ��ƽֻ̨������ĸ�ͷ�ļ�,

         ע��:     ���ļ����� ��ƽ̨�ڵ��κ��ļ�����.                        
**
**  author: wangzongyou
**
**  date:   2006.7.24
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**************************************************************/
#ifndef _XOS_SHELL_H_
#define _XOS_SHELL_H_
#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/*-------------------------------------------------------------
                  ����ͷ�ļ�
--------------------------------------------------------------*/ 
#include "xostype.h"
#include "xosencap.h"
#include "xosos.h"
#include "xosmem.h"
#include "xostrace.h"
#include "xostimer.h"
#include "xostl.h"
#include "xosmodule.h"
#include "xosarray.h"
#include "xoshash.h"
#include "xoslist.h"
#include "clishell.h"
#include "xosroot.h"
#include "xosmmgt.h"
#include "xosport.h"
#include "xosfilesys.h"
#include "xosha.h"
#ifdef XOSENC
#include "xosenc.h"
#endif

/*-------------------------------------------------------------
                  �궨��
--------------------------------------------------------------*/

/*-------------------------------------------------------------
                  �ṹ��ö������
--------------------------------------------------------------*/

/*-------------------------------------------------------------------------
              �ӿں���
-------------------------------------------------------------------------*/
XEXTERN XS8 XOS_InfoReg(t_XOSLOGINLIST *);

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _XOS_CONFIG_H_ */

