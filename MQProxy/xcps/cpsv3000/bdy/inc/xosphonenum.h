/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosphonenum.h
**
**  description: 号码分析树的公共头文件
**
**  author: wentao
**
**  date:   2006.11.13
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**************************************************************/

#ifndef _XOSPHONENUM_H_
#define _XOSPHONENUM_H_

#ifdef  __cplusplus
extern  "C"
{
#endif

#include "xostype.h"
#include "xosencap.h"
//#include "xosos.h"




#define MAXPHONELEN 40  /*号码的最大长度*/
#define    NUM_EXIST            2    /*  */
#define NUM_NON                3

#define MEET_NUM         5    /*接入码，重新收号*/

#define INITANA           0xffffffff



/*网管需要配置的号码字冠表结构*/
typedef struct
{
    XU8  pNum[MAXPHONELEN+1]; /* 电话号码 */
    XU32  pLen;   /* 号码的长度  */
    XU32  len;   /* 号码全长度(包括字冠) */
    XU8  pWord;
}t_CHARNET;

/************************外部函数声明***********************/
/************************************************************
  函数功能：添加号码（树不存在就先创建树）
  参数    ：info    -    电话号码字符串(以'\0'结束的字符串，长度为0~ MAXPHONELEN)
                    电话号码，举例："13800138000"
            len    -    电话号码的长度，如"13800138000"的长度为 11
                    len 取值范围：0 ~ MAXPHONELEN
            property    -    号码信息（必须为初始化了的结构指针，不应为XNULL）

  返回值  ：成功返回    XSUCC
            失败返回：    1. 一般失败XERROR(-1)(例如：参数有误,内存分配失败)
                        2.号码已经存在NUM_EXIST(2)

************************************************************/
XEXTERN XS32 XOS_PhoneAddInfo(XU8 *info, XU32 len, t_CHARNET *property);

/************************************************************
  函数功能：删除电话号码
  参数    ：info    -    电话号码字符串(以'\0'结束的字符串，长度为0~ MAXPHONELEN)
                        电话号码，举例："13800138000"
            len    -    电话号码的长度，如"13800138000"的长度为 11
                        len 取值范围：0 ~ MAXPHONELEN

  返回值  ：成功返回    XSUCC
            失败返回：    1. 一般失败XERROR(-1)
                        3.号码不存在NUM_NON(3)

************************************************************/
XEXTERN XS32 XOS_PhoneDelInfo(XU8 *info, XU32 len);

/************************************************************
  函数功能：删除所有拨号计划
  参数    ：N/A
  返回值  ：成功返回    XSUCC
            失败返回：    XERROR
************************************************************/
XEXTERN XS32 XOS_DelAllPNum( XVOID );

/************************************************************
  函数功能：搜索号码
  参数    ：info    -    电话号码字符串(以'\0'结束的字符串，长度为0~ MAXPHONELEN)
                        电话号码，举例："13800138000"
            len    -    电话号码的长度，如" 13800138000"的长度为 11
                        len 取值范围：0 ~ MAXPHONELEN
            property    -    返回查找到的号码信息

  返回值  ：成功返回    XSUCC
            失败返回 ：    XERROR
************************************************************/
XEXTERN XS32 XOS_PhoneSearchInfo(XU8 *info, XU32 len, t_CHARNET *property);

/************************************************************
  函数功能：分析号码串(整串号码)
  参数    ：info    -    电话号码字符串(以'\0'结束的字符串，长度为0~ MAXPHONELEN)
                        电话号码，举例："13800138000"
            len    -    电话号码的长度，如" 13800138000"的长度为 11
                        len 取值范围：0 ~ MAXPHONELEN
            property    -    返回分析出的号码信息
            
  返回值  ：成功：XSUCC （返回号码属性（查找成功时有效））
            失败：XERROR
************************************************************/
XEXTERN XS32 XOS_AnaStrInfo(XU8 *info, XU32 len, t_CHARNET *property);

/************************************************************
  函数功能：按位分析号码，将查到的号码信息填入用户传入的pWord_p并返回相应的index信息；
  参数    ：bcd  -   为号码的1 位（bcd码，f1-f9 ,fa,fb,fc依次对应1－9，'0'，'*','#'）
            index    -     为分析树索引（由平台按照需要定义）
            pWord_p  -   返回的结构体property中的pWord（结构体定义见后面的系统数据结构）
            pLen    -    为返回属性的结构长度(号码全长，包括字冠)

  返回值  ：查找结果（0成功 -1 失败 2 继续搜号）
            号码属性及属性(property结构体中的pWord)的全长度（查找成功时有效）
            分析树索引（继续查找时有效）
            
  注意：关于分析树索引index，
        当分析的是第一位号码时应用需要将它的值初始化为INITANA（即0xffffffff，
        平台会依据此值来判断是否是新来的需要分析的号码）再将其地址传入做参数；
        若是查找到相应的号码就将其信息填入索引表并将索引返回，
        之后用户需要继续分析时就可以将上次返回的索引做参数了。
************************************************************/
XEXTERN XS32 XOS_AnaSchInfo(XU32 bcd, XU8 *pWord_p, XU32 *pLen, XU32 *index);

/************************************************************
  函数功能：Index的空间释放
  参数    ：indexnum    -    为准备释放的index号，范围：0 ~ MAX_INDEX_NUM
  
  返回值  ：成功返回   XSUCC
            失败返回   XERROR
***********************************************************/
XEXTERN XS32 XOS_IndexFree(XU32 indexnum);



#ifdef  __cplusplus
}
#endif

#endif/*_XOSPHONENUM_H_*/


