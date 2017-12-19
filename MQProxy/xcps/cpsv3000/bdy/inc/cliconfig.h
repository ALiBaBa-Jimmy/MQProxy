/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: cliconfig.h
**
**  description:  ������ģ�����ù�����
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
* ��ģ�鸺���û����õĶ�������
* 
*/


/* 
*   ͷ�ļ����� 
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "xostype.h"


/*-------------------------------------------------------------
                  �궨��
--------------------------------------------------------------*/

/**/
#ifdef XOS_EW_START
#define XOS_PATH_ENV    "XOS_PATH_ENV"
#endif

/* �ֽ�����*/
#undef  __LITTLE_ENDIAN_SYSTEM__
#define __BIG_ENDIAN_SYSTEM__

/*XOS �汾�汾��*/
#define XOS_OS_VERSION  "CPS_V3000_R002_B16.RELEASE.5SP1LT38"
#define XOS_BUILD_TIME  "(Build "__DATE__ " " __TIME__")"


#define XOS_PATH_ENV    "XOS_PATH_ENV"

/* ϵͳʱ������ */
#define K_HWTIC_PER_SEC 1

#define K_MAKEUP_BUF_SIZE   1000

/* telnet�ȴ���ʱ */
#define K_SOCK_TO   30

/* ƽ̨ѡ�� */
#undef  __SINGLE_THREADED_SERVER_ENABLED__
#define __MULTI_THREADED_SERVER_ENABLED__

/*HTTP ������г��� */
#define kHTTPD_QUEUE_SIZE           10

/*���������ر�*/
#undef __BSC_TASK_CLI__

/* 
* Telnet ���
*/

/* Telnet Port */
#define kCLI_FIXED_PORT 23

/* CLI Task Priority */
#define kCLI_SERVER_PRIO    100

/* Session Timeout Length */
#define kCLI_TIMEOUT    1800


/* ֧��shell */
#define __CLI_CONSOLE_ENABLED__ 

/* ֧�������ʾ������ */
#define     CLI_MAX_CMD_CLASS 200

/* ��� Telnet  client ������ */
#define kCLI_MAX_CLI_TASK           16

#define kCLI_HISTORY_BUFFER_SIZE    30

#define CLI_REGVER_MAX 50

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _CLICONFIG_H_ */

