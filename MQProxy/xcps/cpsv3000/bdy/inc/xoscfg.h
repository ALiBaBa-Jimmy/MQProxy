/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosmodule.h
**
**  description: module managment defination 
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
#ifndef _XOS_CONFIG_H_
#define _XOS_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/*-------------------------------------------------------------
                  包含头文件
--------------------------------------------------------------*/


/*-------------------------------------------------------------
                  宏定义
--------------------------------------------------------------*/
#define FID_XOSMIN    0
#define FID_ROOT      1
#define FID_CLI       2
#define FID_TIME      3
#define FID_NTL       4
#define FID_FTP       5
#define FID_IPC       6
#define FID_TELNETD   7
#define FID_IPCMGNT   8
#define FID_TRACE     9
#define FID_FILE      10
#define FID_LOG       11
#define FID_NUMTREE   12
#define FID_HB        13 /*FM 发送心跳任务*/
#define FID_HA_AGENT  (14)
#define FID_NTPC      30
#define FID_MON       31
#define FID_UDT       32
#define FID_MD5       33
#define FID_TA        34

/* HA 消息 Added by liujun, 2014/12/25 */
#define FID_HA        35

/*for agent and oam definition begin*/
#define FID_COMM                     200
#define FID_OAM_AGENT                201
#define FID_SNMP_SET                 202
#define FID_SNMP_GET                 203
#define FID_SNMP_PMGET               204
#define FID_OAM_SAM                  205
#define FID_SNMP_SETLNK              206
#define FID_SNMP_GETLNK              207
#define FID_SNMP_PMGETLNK            208
#define FID_DATA_PERSISTENCY         209

//#ifdef USE_PM
#define FID_AGENT_PM_MANAGER         210
#define FID_AGENT_PM_SCHEDULE        211
#define FID_AGENT_PM_SAMPLE          212
#define FID_AGENT_PM_REPORTER        213
//#endif /*USE_PM*/

#ifdef USE_APP
#define FID_APP  214
#endif /*USE_APP*/

//#ifdef  USE_FM
#define FID_RSET 215
#define FID_FM   216
#define FID_TRAP 217

#define FID_NKUA 1639

/*for agent and oam definition end*/

#define FID_XOSMAX  100

/*-------------------------------------------------------------
                  结构和枚举声明
--------------------------------------------------------------*/

#if 0
/* xos 平台内部模块列表*/
typedef enum
{
    FID_XOSMIN = 0,
    FID_ROOT,
    FID_CLI,
    FID_TIME,
    FID_NTL,
#ifdef XOS_FTP_CLIENT
    FID_FTP,
#endif
    FID_IPC,
    FID_TELNETD,
#ifdef XOS_IPC_MGNT
    FID_IPCMGNT,
#endif
    FID_TRACE,
    FID_FILE,
    FID_LOG,
    FID_NUMTREE,
    FID_XOSMAX
    
}e_XOSFID;
#endif


/*-------------------------------------------------------------------------
              接口函数
-------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _XOS_CONFIG_H_ */

