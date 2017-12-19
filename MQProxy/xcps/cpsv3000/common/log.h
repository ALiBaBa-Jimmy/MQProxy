#ifndef __LOG_H__
#define __LOG_H__

#ifdef WIN32
    
#else
#include <unistd.h>
#endif

#include <time.h>
#include <sys/stat.h>

#include "pub_types.h"


#ifdef  __cplusplus
extern  "C"{
#endif

enum log_level_type
{
    LOG_DEBUG = 1U,
    LOG_INFO,
    LOG_WARNING,
    LOG_ALERT,
    LOG_ERROR,
    LOG_UNKNOW_LEVEL
}; 

s32 log_init(const s8 *path, const s8 *name, u32 file_size,u32 total_file_size, s32 log_timeout);

//设置是否终端也打印,1为打印,0为关闭打印
void log_term_set(s32 flag);

void log_level_set(enum log_level_type level);

enum log_level_type log_level();

s32 log_file_fresh();

void log_write(enum log_level_type level,const s8* file,s32 file_line,const s8 *format,...);

#define logger(level, format, ...) log_write(level,__FUNCTION__,__LINE__,format,##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif