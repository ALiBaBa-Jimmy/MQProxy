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

/************************************ģ��ŷ�Χ�궨��************************/



/************************************����ֵ�궨��************************/
//�ɹ�
#define SUCCESS                     0
//ʧ��
#define FAILURE                     -1
//��������
#define ERR_ARGUMENT                -2
//����ʧ��
#define ERR_MALLOC                  -3
//������
#define ERR_NOT_EXIST               -4
//���
#define ERR_OVERFLOW                -5
//��������
#define ERR_CREATE                  -6
//��ͻ����
#define ERR_CONFLICT                -7
//�޿ռ����
#define ERR_NO_SPACE                -8
//�Ѵ��ڴ���
#define ERR_EXIST                   -9
//�ѱ�ʹ�ô���
#define ERR_USING                   -10
//ϵͳ����
#define ERR_SYSTEM                  -11
//���ɴ����
#define ERR_UNREACHABLE             -12
//IP�ݽ�
#define ERR_EXHAUSTION              -13

/* �汾���� */
#define ERR_WRONG_VERSION           -14

/* ���մ��� */
#define ERR_RECV_ERROR              -15

#define ERR_WRONG_MSG_TYPE          -16

#define GLOBAL_TRUE                 1
#define GLOBAL_FALSE                0

#endif

