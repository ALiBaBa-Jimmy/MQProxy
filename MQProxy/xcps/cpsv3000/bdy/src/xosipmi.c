/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosipmi.c
**
**  description:  ipmi
**
**  author: spj
**
**  date:   2014.9.14
**
***************************************************************
**                          history
**
***************************************************************
**   author              date              modification

**************************************************************/
#ifdef  __cplusplus
extern   "C"  {
#endif  /*  __cplusplus */

#include "xosipmi.h"

/*-------------------------------------------------------------------------
                                模块内部宏定义
-------------------------------------------------------------------------*/
#define MAX_SLOT_NUM  14
#define MIN_SLOT_NUM  1

#define IPMI_MAX_BUF_SIZE  128

#define IPMB_BASE_ADDR (0x40)
/*-------------------------------------------------------------------------
                               模块内部数据结构
-------------------------------------------------------------------------*/
static XBOOL g_IpmiIsOk = XFALSE;  /* 检测ipmi是否可用， true可用，false不可用 */

/* 凌华板的IPMB addr */
static XU32 g_IpmiAddr[MAX_SLOT_NUM] = {0x9a,0x96,0x92,0x8e,0x8a,0x86,0x82,0x84,0x88,0x8c,0x90,0x94,0x98,0x9c};
/* 凌华板的logic 到 物理slot 的映射 */
static XU32 g_SlotMap[MAX_SLOT_NUM] = {7,8,6,9,5,10,4,11,3,12,2,13,1,14};

/*-------------------------------------------------------------------------
                              模块内部全局变量
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
                             模块内部函数
-------------------------------------------------------------------------*/

/************************************************************************
函数名  : XOS_IpmiIsOk
功能    : 判断ipmitool是否可用
输入    : 
输出    :
返回    : 可用返回ture， 不可用返回false
说明：  
************************************************************************/
XBOOL XOS_IpmiIsOk(XVOID)
{
    return g_IpmiIsOk;
}
/************************************************************************
函数名  : XOS_IpmiIsOk
功能    : 设置ipmi的状态值，是否可用
输入    : val: ture表示可用， false表示不可用
输出    :
返回    : 
说明：  
************************************************************************/
XVOID XOS_SetIpmiFlag(XBOOL val)
{
    g_IpmiIsOk = val;
}
#ifdef XOS_LINUX
/************************************************************************
函数名  : XOS_CheckIpmiEnv
功能    : 检测单板ipmi的支持环境，ipmitool是否可用
输入    : 
输出    :
返回    : 可用返回0， 不可用返回-1
说明: 此函数只能在linux下使用
************************************************************************/
XS32 XOS_CheckIpmiEnv(XVOID)
{
    XS8 cmd[100] = {0};
    XS32 ret = 0;

    if (access("/etc/init.d/ipmi", F_OK))
    {
        printf("file %s isn't exist!\n","/etc/init.d/ipmi");
        return XERROR;
    }

    /* 判断ipmi服务状态 */
    XOS_Sprintf(cmd, sizeof(cmd), "/etc/init.d/ipmi status >/dev/null"); 
    ret = XOS_ExeCmdRetVal(cmd);
    if (ret)
    {
        printf("XOS_CheckIpmiEnv get ipmi status fail! cmd:%s, ret:%d\n",cmd,ret);
        return XERROR;
    }

	/* 命令是否可用 */
    XOS_Sprintf(cmd, sizeof(cmd), "ipmitool mc info >/dev/null 2>&1"); 
    ret = XOS_ExeCmdRetVal(cmd);
    if (ret)
    {
        printf("XOS_CheckIpmiEnv exe cmd test ret fail! cmd:%s, ret:%d\n",cmd,ret);
        return XERROR;
    }

    return XSUCC;
}
#endif
/************************************************************************
 函数名: XOS_GetPowerState
 功能: 获取当前单板是否上电
 输入: iSlotNum 要获取上电状态单板的槽位号
 输出:
 返回: 0: 单板已上电
       1: 单板在位，未上电
       2: 单板不在位， 或ipmitool命令无法使用
      -2: 命令执行失败
      -1: 参数非法
 说明: 目前硬件只针对凌华板，且只能在linux下具有ipmi的环境下正常使用，否则返回-1
************************************************************************/
XS32 XOS_GetPowerState(XS32 iSlotNum)
{
#ifdef XOS_LINUX
    XS8 cmd[IPMI_MAX_BUF_SIZE] = {0};
    XS8 buf[IPMI_MAX_BUF_SIZE] = {0};
    XS32 ret = 0;

    if (!XOS_IpmiIsOk())
    {
        return IPMI_ERR;
    }
    
    if (iSlotNum < MIN_SLOT_NUM || iSlotNum > MAX_SLOT_NUM)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetPowerState param err,SlotNum:%d!\n",iSlotNum);
        return IPMI_ERR;
    }

    XOS_Sprintf(cmd,sizeof(cmd),"ipmitool -t 0x%x raw 0x2c 0x12 0x00 0x00 0x00 2>/dev/null |awk '{print $2}'",g_IpmiAddr[iSlotNum-1]);
    ret = XOS_ExeCmdByPopen_Ex(cmd, buf, sizeof(buf),300);
    if (ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetPowerState cmd:%s fail,ret:%d!\n",cmd,ret);
        return IPMI_ERR;
    }

    if (0 == XOS_StrNcmp(buf, "01", 2))       /* 单板已上电 */
    {
        return IPMI_POWER_ON; 
    }
    else if (0 == XOS_StrNcmp(buf, "00", 2)) /* 单板在位，没上电 */
    {
        return IPMI_POWER_OFF;
    }
    else                                 /* 单板不在位， 或ipmitool命令无法使用*/
    {
        return IPMI_NO_BOARD;
    }
#else
    return IPMI_ERR;
#endif
}

/************************************************************************
 函数名: XOS_GetSlotNum
 功能: 获取当前单板槽位号
 输入:
 输出:
 返回: 成功返回1-14, 否则返回XERROR
 说明: 目前硬件只针对凌华板，且只能在linux下具有ipmi的环境下正常使用，否则返回-1
************************************************************************/
XS32 XOS_GetSlotNum(XVOID)
{
#ifdef XOS_LINUX
    XS32 ret = 0;
    XS8  buf[IPMI_MAX_BUF_SIZE] = {0};
    XS32 GetAddr = 0; 
    XS32 LogicSlot = 0; 

    if (!XOS_IpmiIsOk())
    {
        return IPMI_ERR;
    }

    ret = XOS_ExeCmdByPopen_Ex("ipmicmd -k \"0f 00 2c 01 00\" smi 0 2>/dev/null|awk '{print $7}'", buf, sizeof(buf),100);
    if (ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetSlotNum ipmicmd get slot num fail,ret:%d!\n",ret);
        return IPMI_ERR;
    }

    if (0 == XOS_StrLen(buf))
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "get slot num fail, maybe because ipmi env is not viable!\n");
        return IPMI_ERR;
    }

    GetAddr = (XS32)strtoul(buf, NULL, 16);
    LogicSlot = GetAddr-IPMB_BASE_ADDR;
    
    if (LogicSlot < MIN_SLOT_NUM || LogicSlot > MAX_SLOT_NUM)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "get logic slot num fail,logic:%d!\n",LogicSlot);
        return IPMI_ERR;
    }
    
    return g_SlotMap[LogicSlot-1];
#else
    return IPMI_ERR;
#endif
}
/************************************************************************
 函数名: XOS_GetShelfNum
 功能: 获取当前单板机框号
 输入:
 输出:
 返回: 成功返回值0-0xff, 否则返回XERROR
 说明: 目前硬件只针对凌华板，且只能在linux下具有ipmi的环境下正常使用，否则返回-1
************************************************************************/
XS32 XOS_GetShelfNum(XVOID)
{
#ifdef XOS_LINUX
    XS32 ret = 0;
    XS8 buf[IPMI_MAX_BUF_SIZE] = {0};

    if (!XOS_IpmiIsOk())
    {
        return IPMI_ERR;
    }
    ret = XOS_ExeCmdByPopen_Ex("ipmicmd -k \"00 20 00 2c 02 00\" smi 0 2>/dev/null |awk '{print $8}'", buf, sizeof(buf),200);
    if (ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetShelfNum get shelf num fail,ret:%d!\n",ret);
        return IPMI_ERR;
    }

    if (0 == XOS_StrLen(buf))
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "get shelf num fail, maybe because ipmi env is not viable!\n");
        return IPMI_ERR;
    }

    return strtol(buf, NULL, 16);
#else
    return IPMI_ERR;
#endif 
}
/************************************************************************
 函数名: XOS_SetShelfNum
 功能: 设置当前单板机框号
 输入: iNum : 要设置机框号
 输出:
 返回: 成功返回XSUCC, 否则返回XERROR
 说明: 目前硬件只针对凌华板，且只能在linux下具有ipmi的环境下正常使用，否则返回-1
************************************************************************/
XS32 XOS_SetShelfNum(XS32 iNum)
{
#ifdef XOS_LINUX
    XS32 ret = 0;
    XU32 retval = 0;
    XS8 buf[IPMI_MAX_BUF_SIZE] = {0};
    XS8 cmd[IPMI_MAX_BUF_SIZE] = {0};

    if (!XOS_IpmiIsOk())
    {
        return IPMI_ERR;
    }

    if (iNum < 0 || iNum > 0xff)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_SetShelfNum param error!Only 0-0xff is allowed\n");
        return IPMI_ERR;
    }

    XOS_Sprintf(cmd, sizeof(cmd),"ipmitool -t 0x20 raw 0x2c 3 0 1 %d 2>/dev/null", iNum);
    ret = XOS_ExeCmdByPopen_Ex(cmd, buf, sizeof(buf),500);
    if (ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_SetShelfNum exe cmd %s fail,ret:%d!\n",cmd,ret);
        return IPMI_ERR;
    }

    if (0 == XOS_StrLen(buf))
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "set slot num fail, maybe because ipmi env is not viable!\n");
        return IPMI_ERR;
    }

    retval = strtoul(buf, NULL, 16);  //正常时，ipmitool打印返回值为0
    if (0 != retval)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_SetShelfNum set shelf num fail,ret:%d!\n",ret);
        return IPMI_ERR;
    }
    
    return IPMI_SUCC;
#else
    return IPMI_ERR;
#endif
}
/************************************************************************
 函数名: XOS_GetRtmName
 功能: 获取后卡板型号
 输入: len : 传入的buf长度 
 输出: buf : 返回的获取到的名称
 返回: 成功返回0，失败返回-1
 说明: 目前硬件只针对凌华板，且只能在linux下具有ipmi的环境下正常使用，否则返回-1
************************************************************************/
XS32 XOS_GetRtmName(XS8 *buf, XU32 len)
{
#ifdef XOS_LINUX
    XS32 ret = 0;
    XS32 i;
    XS8 buftemp[IPMI_MAX_BUF_SIZE] = {0};

    if (!XOS_IpmiIsOk())
    {
        return IPMI_ERR;
    }

    if (NULL == buf)
    {
        return IPMI_ERR;
    }

    for (i = 0; i < 3; i++)
    {
        XOS_MemSet(buftemp, 0, sizeof(buftemp));
        ret = XOS_ExeCmdByPopen_Ex("ipmitool fru |grep \"Board Product\"|sort|uniq|sed -n '2p'|awk '{print $4}' 2>/dev/null", buftemp, sizeof(buftemp),1000);
        if (ret)
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetBoardName get board name fail,ret:%d!\n",ret);
            return IPMI_ERR;
        }

        if (XOS_StrLen(buftemp) > 0)
        {
            buftemp[XOS_StrLen(buftemp)-1] = '\0';  // 去掉最后的 \n
            break;
        }
        else
        {
            XOS_Sleep(10);
            continue;
        }
    }

    if (0 == XOS_StrLen(buftemp))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetRtmName don't get rtm name!\n");
        return -1;
    }

    XOS_StrNcpy(buf,buftemp,XOS_MIN(XOS_StrLen(buftemp),len-1));

    return IPMI_SUCC;
#else
    return -1;
#endif
}
/************************************************************************
 函数名: XOS_GetBoardName
 功能: 获取主板型号
 输入: len : 传入的buf长度 
 输出: buf : 返回的获取到的名称
 返回: 成功返回0，失败返回-1
 说明: 目前硬件只针对凌华板，且只能在linux下具有ipmi的环境下正常使用，否则返回-1
************************************************************************/
XS32 XOS_GetBoardName(XS8 *buf, XU32 len)
{
#ifdef XOS_LINUX
    XS32 ret = 0;
    XS32 i;
    XS8 buftemp[IPMI_MAX_BUF_SIZE] = {0};
    
    if (!XOS_IpmiIsOk())
    {
        return -1;
    }

    if (NULL == buf)
    {
        return -1;
    }

    for (i = 0; i < 3; i++)
    {
        XOS_MemSet(buftemp, 0, sizeof(buftemp));
        ret = XOS_ExeCmdByPopen_Ex("ipmitool fru |grep \"Board Product\"|sort|uniq|sed -n '1p'|awk '{print $4}' 2>/dev/null", buftemp, sizeof(buftemp),1000);
        if (ret)
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetBoardName get board name fail,ret:%d!\n",ret);
            return -1;
        }

        if (XOS_StrLen(buftemp) > 0)
        {
            buftemp[XOS_StrLen(buftemp)-1] = '\0';  // 去掉最后的 \n
            break;
        }
        else
        {
            XOS_Sleep(10);
            continue;
        }
    }

    if (0 == XOS_StrLen(buftemp))
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "XOS_GetBoardName don't get board name!\n");
        return -1;
    }

    XOS_StrNcpy(buf,buftemp,XOS_MIN(XOS_StrLen(buftemp),len-1));
    
    return 0;
#else
    return -1;
#endif
}
/************************************************************************
 函数名: XOS_GetMfgName
 功能: 获取厂商名称
 输入: len : 传入的buf长度 
 输出: buf : 返回的获取到的名称
 返回: 成功返回0，失败返回-1
 说明: 目前硬件只针对凌华板，且只能在linux下具有ipmi的环境下正常使用，否则返回-1
************************************************************************/
XS32 XOS_GetMfgName(XS8 *buf, XU32 len)
{
#ifdef XOS_LINUX
    XS32 ret = 0;
    XS32 i;
    XS8 buftemp[IPMI_MAX_BUF_SIZE] = {0};
    
    if (!XOS_IpmiIsOk())
    {
        return -1;
    }

    if (NULL == buf)
    {
        return -1;
    }

    for (i = 0; i < 3; i++)
    {
        XOS_MemSet(buftemp, 0, sizeof(buftemp));
        ret = XOS_ExeCmdByPopen_Ex("ipmitool fru |awk '/Manufacturer/{print $4}'|uniq", buftemp, sizeof(buftemp),1000);
        if (ret)
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetMfgName get mfg name fail,ret:%d!\n",ret);
            return -1;
        }

        if (XOS_StrLen(buftemp) > 0)
        {
            buftemp[XOS_StrLen(buftemp)-1] = '\0';  // 去掉最后的 \n
            break;
        }
        else
        {
            XOS_Sleep(10);
            continue;
        }
    }

    if (0 == XOS_StrLen(buftemp))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetMfgName don't get mfg name!\n");
        return -1;
    }

    XOS_StrNcpy(buf,buftemp,XOS_MIN(XOS_StrLen(buftemp),len-1));
    
    return 0;
#else
    return -1;
#endif

}

#ifdef  __cplusplus
    }
#endif  /*  __cplusplus */

