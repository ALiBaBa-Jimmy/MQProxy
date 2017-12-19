#ifndef __PUB_TYPES_H__
#define __PUB_TYPES_H__
#include <stdlib.h>

#ifndef __no_used__
#define __no_used__  __attribute__((unused))
#endif

#ifndef u8
#define u8 unsigned char
#endif

#ifndef s8
#define s8 char
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef s16
#define s16 short
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef s32
#define s32 int
#endif

#ifdef WIN32

#ifndef u64
#define u64 unsigned _int64
#endif

#ifndef s64
#define s64 _int64
#endif

#else

#ifndef u64
#define u64 unsigned long long
#endif

#ifndef s64
#define s64 long long
#endif

#ifndef likely
#define likely(x)   __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#endif

#ifdef PLAT_64
#define PLAT_INT u64
#else
#define PLAT_INT u32
#endif

/************************************模块号范围宏定义************************/



/************************************错误值宏定义************************/
//成功
#define SUCCESS                     0
//失败
#define FAILURE                     -1
//参数错误
#define ERR_ARGUMENT                -2
//分配失败
#define ERR_MALLOC                  -3
//不存在
#define ERR_NOT_EXIST               -4
//溢出
#define ERR_OVERFLOW                -5
//创建错误
#define ERR_CREATE                  -6
//冲突错误
#define ERR_CONFLICT                -7
//无空间错误
#define ERR_NO_SPACE                -8
//已存在错误
#define ERR_EXIST                   -9
//已被使用错误
#define ERR_USING                   -10
//系统错误
#define ERR_SYSTEM                  -11
//不可达错误
#define ERR_UNREACHABLE             -12
//IP枯竭
#define ERR_EXHAUSTION              -13

/* 版本错误 */
#define ERR_WRONG_VERSION           -14

/* 接收错误 */
#define ERR_RECV_ERROR              -15

#define ERR_WRONG_MSG_TYPE          -16

#define GLOBAL_TRUE                 1
#define GLOBAL_FALSE                0

#endif

