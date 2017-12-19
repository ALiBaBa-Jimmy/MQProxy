/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosapp.h
**
**  description: 所有业务使用平台只需包括的该头文件,

         注意:     该文件不能 被平台内的任何文件包含.                        
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
                  包含头文件
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
                  宏定义
--------------------------------------------------------------*/

/*-------------------------------------------------------------
                  结构和枚举声明
--------------------------------------------------------------*/

/*-------------------------------------------------------------------------
              接口函数
-------------------------------------------------------------------------*/
XEXTERN XS8 XOS_InfoReg(t_XOSLOGINLIST *);

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _XOS_CONFIG_H_ */

