
#pragma once
//#include "errorcode.h"
//
//#include "OCSException.h"
//
//
////define 类型
//typedef unsigned char       XU8; 
//typedef char                XS8;
//typedef unsigned short 	    XU16;
//typedef short 			    XS16;
//typedef unsigned int 		XU32;
//typedef int 				XS32;
//typedef unsigned long long	XU64;
//typedef long long			XS64;
//typedef void                XVOID;
//typedef bool                XBOOL;
typedef float 				XF32;
typedef double 				XD64;
//
///*define time type*/
//typedef unsigned int XW_LONG_TIME_T;
//
//#ifndef XBOOLVALUE
//typedef enum
//{
//    XFALSE=0,
//    XTRUE  
//}XBOOLVALUE;
//#endif
//
//#ifndef XCONST
//	#define XCONST  const
//	#define XSTATIC static
//	#define XEXTERN extern
//#endif 
//#ifndef XPUBLIC
//	#define XPUBLIC
//#endif
//
//
//
//#ifndef XNULL
//	#define XNULL				(0)
//#endif
//
//
//#ifndef NULL
//	#define NULL				(0)
//#endif
//
///* 字符的比较*/
//#define XOS_StrCmp(s1, s2)	strcmp((char *)s1,(char *)s2)   /*#不提倡使用*/
//#define XOS_StrNcmp(s1,s2,n)		strncmp((char *)s1,(char *)s2,(size_t)(n))
//
///* 字符的拷贝*/
//#define XOS_StrCpy(dest,src)    strcpy((char *)dest , (char *)src)   /*#不提倡使用*/
//#define XOS_StrNcpy(dest, src,n )    strncpy((char *)dest,(char *)src,(size_t)(n) )
//
///*字符串连接*/
//#define XOS_StrCat(dest, src)   strcat((char *)dest, (XCONST XCHAR *)src)   /*#不提倡使用*/
//#define XOS_StrNCat(dest,src,n) strncat((XCHAR *)dest, (XCONST XCHAR *)src,(size_t)(n))
//
///* 取字符长度*/
//#define XOS_StrLen(s)	strlen((char *)s)
//
//
///* 字符的查找*/
//#define XOS_StrChr(s, c)	strchr((char *)s,(int)(c))
//#define XOS_StrStr(haystack,needle)	strstr((char *)haystack,(char *)needle)
//
///* 内存的拷贝，赋值*/
//#define XOS_MemCpy(dest,src,n)	memcpy(dest,src,n)
//#define XOS_MemMove(dest,src,n)  memmove(dest,src,n)
//#define XOS_MemSet(dest,src,n)	memset(dest,src,n)
//#define XOS_MemCmp(dest,src,n)  memcmp(dest,src,n)
//
///* 字符大小写转换*/
//#define XOS_ToLower( c )    tolower( c )
//#define XOS_ToUpper( c )    toupper( c )
//
///* 字节序转换*/
///* 2 字节网络序转主机序*/
//#define XOS_NtoHs(netshort)	ntohs((unsigned short int)(netshort))    
//
///* 2 字节主机序转网络序*/
//#define XOS_HtoNs(hostshort)	htons((unsigned short int)(hostshort))    
//
///* 4 字节网络序转主机*/
//#define XOS_NtoHl(netlong)	ntohl((unsigned long int)(netlong))    
//
///* 4 字节主机序转网络序*/
//#define XOS_HtoNl(hostlong)	htonl((unsigned long int)(hostlong))    
//
///* 取最大、最小值*/
//#define XOS_MAX(x,y) ((x)>=(y)?(x):(y))
//#define XOS_MIN(x,y) ((x)>=(y)?(y):(x))
//
//
////定义sql语句的最大长度
//#define MAX_SQL_LENGTH 2048
// 
//const XU32 MAX_TCP_BUFFER_SIZE = 4096;
//
//#define MAX_LOG_LENGTH 1024

