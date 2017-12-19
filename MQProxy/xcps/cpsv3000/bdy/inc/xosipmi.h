/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosipmi.h
**
**  description: ipmi defination
**
**  author: spj
**
**  date:   2014.9.13
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            

**************************************************************/
#ifndef _XOS_IPMI_H_
#define _XOS_IPMI_H_

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

#include "xostype.h"
#include "xosencap.h"
#include "xostrace.h"
#include "xoscfg.h"


#define IPMI_SUCC   0
#define IPMI_ERR    -1

/* 槽位电源单板状态 */
#define IPMI_POWER_ON 0   /*  */
#define IPMI_POWER_OFF 1  /*  */
#define IPMI_NO_BOARD  2  /*  */

XS32 XOS_GetPowerState(XS32 iSlotNum);

XS32 XOS_GetSlotNum(XVOID);

XS32 XOS_GetShelfNum(XVOID);

XS32 XOS_SetShelfNum(XS32 iNum);

XS32 XOS_GetRtmName(XS8 *buf, XU32 len);

XS32 XOS_GetBoardName(XS8 *buf, XU32 len);

XS32 XOS_GetMfgName(XS8 *buf, XU32 len);

#ifdef __cplusplus
    }
#endif /* _ _cplusplus */

#endif //_XOS_IPMI_H_
