/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: port.h
**
**  description: user fid defination ,  config by user  
**
**  author: wangzongyou
**
**  date:   2006.7.13
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**************************************************************/
#ifndef _XOS_PORT_H_
#define _XOS_PORT_H_
#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/*-------------------------------------------------------------
                  包含头文件
--------------------------------------------------------------*/ 
#include "xoscfg.h"


/* #ifdef XOS_NEED_MAIN */ /* del by wulei for mss */
/*-------------------------------------------------------------
                  宏定义
--------------------------------------------------------------*/
#ifndef XOS_EBSC
#define FID_SA    220
#define FID_OAM   221
#endif
#define FID_BM     99
#ifndef FID_XOSMAX
#define FID_XOSMAX 100
#endif

#define FID_USERMIN FID_XOSMAX

#define FID_BASE_PROTOCOL 101
#define FID_MAX_PROTOCOL 200

#define FID_BASE_CNMS 201
#define FID_MAX_CNMS 400

#define FID_BASE_XPMS 401
#define FID_MAX_XPMS 500

#define FID_TS          501

#define FID_BASE_EBSC 1001
#ifdef  XOS_EBSC
#define FID_SYS_FIRST 1024
#define FID_OAM       1025
#define FID_CC        1026
#define FID_SMS       1027
#define FID_MM        1028
#define FID_SAB       1029
#define FID_SAB1      1030
#define FID_SA        1031
#define FID_SA2       1032
#define FID_SA3       1033
#define FID_SBADAPT   1034
#define FID_MEDIA     1035
#define FID_CLK       1036
#define FID_TLP       1037
#define FID_PPPOE     1038
#define FID_GWF       1039
#define FID_BCSMS     1040
#define FID_LAPD      1041
#define FID_DRIVER    1042
#define FID_SCTP      1043
#define FID_COMM      1044
#define FID_SYS_MAX   1045
#endif
#define FID_MAX_EBSC 1200

#define FID_BASE_HLR 1201
#define FID_MAX_HLR 1400

#define FID_BASE_SAG 1401
#define FID_MAX_SAG 1600

#define FID_BASE_SS 1601
#define FID_MAX_SS  1699

#define FID_BASE_SAAP 1700
#define FID_MAX_SAAP 1800

#define FID_MAX    4096



/*-------------------------------------------------------------
                  结构和枚举声明
--------------------------------------------------------------*/
#if 0
/* 业务应用的模块配置表*/
typedef enum
{
    FID_USERMIN = FID_XOSMAX,
    /* wulei add for ss 2007-1-5 */
    FID_BASE_V3000 = FID_MAX_V3000,
    FID_TRAP,
    FID_SA,
    FID_OAM,



    /* SAG产品 */

    /* SS产品 */

    FID_MAX
}e_USERFID;
#endif

/*-------------------------------------------------------------
                  全局变量
--------------------------------------------------------------*/


/*-------------------------------------------------------------------------
              接口函数
-------------------------------------------------------------------------*/

/* #endif */  /* XOS_NEED_MAIN */ /* del by wulei for mss */

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _XOS_CONFIG_H_ */

