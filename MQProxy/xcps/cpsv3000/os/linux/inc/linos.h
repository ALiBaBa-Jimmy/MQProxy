/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: os.h
**
**  description:  ��������ϵͳ�²�ͬ�ĺ궨�� 
**
**  author: wentao
**
**  date:   2006.3.29
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   wentao         2006.3.29              create  
**************************************************************/
#ifndef _OS_H_
#define _OS_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/* According to POSIX 1003.1-2001 */
#include <sys/select.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "errno.h"

#include <linux/reboot.h>
#include <sys/reboot.h>
#include <sys/syscall.h>


/*pthread_mutex_t glmutex = PTHREAD_MUTEX_INITIALIZER;//Ϊ���ӱ�pc-lint�ĸ澯*/

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
//typedef pthread_rwlock_t t_XOSRWLOCKID;  /* ��д�� */
typedef pthread_mutex_t  t_XOSMUTEXID;
typedef pthread_t        t_XOSTASKID;
typedef long             t_XOSMSGQID;
/*typedef long             t_XOSSEMID;*/
typedef sem_t             t_XOSSEMID;
typedef long             t_XOSSEMINI;

#define OS_GetChar()     getchar()
#define gettid()         syscall(__NR_gettid)

typedef void* (*os_taskfunc)(void *);       /*������ָ������*/


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*os.h*/

