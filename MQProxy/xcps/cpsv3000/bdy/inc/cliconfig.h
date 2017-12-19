/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: cliconfig.h
**
**  description:  命令行模块配置管理部分
**
**  author: zhanglei
**
**  date:   2006.3.7
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   zhanglei         2006.3.7              create  
**************************************************************/

#ifndef _CLICONFIG_H_
#define _CLICONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */


/*
* 本模块负责用户配置的定制内容
* 
*/


/* 
*   头文件引用 
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "xostype.h"


/*-------------------------------------------------------------
                  宏定义
--------------------------------------------------------------*/

/**/
#ifdef XOS_EW_START
#define XOS_PATH_ENV    "XOS_PATH_ENV"
#endif

/* 字节序定义*/
#undef  __LITTLE_ENDIAN_SYSTEM__
#define __BIG_ENDIAN_SYSTEM__

/*XOS 版本版本号*/
#define XOS_OS_VERSION  "CPS_V3000_R002_B16.RELEASE.5SP1LT38"
#define XOS_BUILD_TIME  "(Build "__DATE__ " " __TIME__")"


#define XOS_PATH_ENV    "XOS_PATH_ENV"

/* 系统时钟设置 */
#define K_HWTIC_PER_SEC 1

#define K_MAKEUP_BUF_SIZE   1000

/* telnet等待超时 */
#define K_SOCK_TO   30

/* 平台选择 */
#undef  __SINGLE_THREADED_SERVER_ENABLED__
#define __MULTI_THREADED_SERVER_ENABLED__

/*HTTP 请求队列长度 */
#define kHTTPD_QUEUE_SIZE           10

/*任务驱动关闭*/
#undef __BSC_TASK_CLI__

/* 
* Telnet 相关
*/

/* Telnet Port */
#define kCLI_FIXED_PORT 23

/* CLI Task Priority */
#define kCLI_SERVER_PRIO    100

/* Session Timeout Length */
#define kCLI_TIMEOUT    1800


/* 支持shell */
#define __CLI_CONSOLE_ENABLED__ 

/* 支持最大提示符数量 */
#define     CLI_MAX_CMD_CLASS 200

/* 最大 Telnet  client 连接数 */
#define kCLI_MAX_CLI_TASK           16

#define kCLI_HISTORY_BUFFER_SIZE    30

#define CLI_REGVER_MAX 50

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _CLICONFIG_H_ */

