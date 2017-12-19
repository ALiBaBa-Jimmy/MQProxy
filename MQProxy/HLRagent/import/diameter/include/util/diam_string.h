#ifndef __UTIL_STRING_UTIL_H__
#define __UTIL_STRING_UTIL_H__

#include <string>
#include <list>
#include <api/diam_datatype.h>

class StringUtil
{
public:
    static char* trim(char* pData,int& nDataLen);
    static char* trimLeft(char* pData,int& nDataLen);
    static char* trimRight(char* pData,int& nDataLen);
    static Boolean  isDigit(char* pData,int nDataLen);
    static Boolean  isFloat(char* pData,int nDataLen);

    ///////////////////////////////////////////////////////////////
    //	函 数 名 : string toUpper
    //	函数功能 : 将传入参数的字符串中小写的字母转换成大写的
    //	备    注 :
    //	返 回 值 : string,返回转换后没有小写字母的字符串
    //	参数说明 : const  std::string &  value：需要转换的字符串
    ///////////////////////////////////////////////////////////////
    static std::string toUpper(const  std::string &  value);
    ///////////////////////////////////////////////////////////////
    //	函 数 名 : string toLower
    //	函数功能 : 将传入参数的字符串中大写的字母转换成小写的
    //	备    注 :
    //	返 回 值 : string,返回转换后没有大写字母的字符串
    //	参数说明 : const  std::string &  value：需要转换的字符串
    ///////////////////////////////////////////////////////////////
    static std::string toLower(const  std::string &  value);
    ///////////////////////////////////////////////////////////////
    //	函 数 名 : equalsIgnoreCase
    //	函数功能 : 比较两个字符串是否相等，比较时忽略大小写
    //	备    注 :
    //	返 回 值 : Boolean,如果相等，则返回true,否则返回false
    //	参数说明 : const  std::string &  value,const  std::string &  another
    //
    ///////////////////////////////////////////////////////////////
    static Boolean equalsIgnoreCase(const  std::string &  value,const  std::string &  another);
    ///////////////////////////////////////////////////////////////
    //	函 数 名 : string trim
    //	函数功能 : 去除去符串头尾的空格和其它不可见符号(在ASCII中小于等于空格的符号)
    //	备    注 :
    //	返 回 值 : string，处理后的字符串
    //	参数说明 : const  std::string &  value：需要处理的参数
    ///////////////////////////////////////////////////////////////
    static std::string trim(const  std::string &  value);

    ///////////////////////////////////////////////////////////////
    //	函 数 名 : string trimLeft
    //	函数功能 : 去除去符串左边的空格和其它不可见符号(在ASCII中小于等于空格的符号)
    //	备    注 :
    //	返 回 值 : string，处理后的字符串
    //	参数说明 : const  std::string &  value：需要处理的参数
    ///////////////////////////////////////////////////////////////
    static std::string trimLeft(const std::string & value);

    ///////////////////////////////////////////////////////////////
    //	函 数 名 : string trimRight
    //	函数功能 : 去除去符串右边的空格和其它不可见符号(在ASCII中小于等于空格的符号)
    //	备    注 :
    //	返 回 值 : string，处理后的字符串
    //	参数说明 : const  std::string &  value：需要处理的参数
    ///////////////////////////////////////////////////////////////
    static std::string trimRight(const std::string & value);

    ///////////////////////////////////////////////////////////////
    //	函 数 名 : skipWhiteSpace
    //	函数功能 : 从字符串指定位置index开始找第一个非空字符的位置
    //	备    注 :
    //	返 回 值 : Int如果找到，则返回找到处索引，如果没有找到，则返回-1
    //	参数说明 : const  std::string &  value: 需要找的字符串
    //unsigned int index　开始找的位置
    ///////////////////////////////////////////////////////////////
    static INT32 skipWhiteSpace(const  std::string &  value, unsigned int index);

    ///////////////////////////////////////////////////////////////
    //	函 数 名 : findNextWhiteSpace
    //	函数功能 : 从字符串指定位置index开始找第一个空字符的位置
    //	备    注 :
    //	返 回 值 : Int如果找到，则返回找到处索引，如果没有找到，则返回-1
    //	参数说明 : const  std::string &  value: 需要找的字符串
    //unsigned int index　开始找的位置
    ///////////////////////////////////////////////////////////////
    static INT32 findNextWhiteSpace(const  std::string &  value, unsigned int index);

    ///////////////////////////////////////////////////////////////
    //	函 数 名 : startsWith
    //	函数功能 : 判断字符串value前缀是否为prefix
    //	备    注 :
    //	返 回 值 : Boolean，如果是则返回true,否则返回false
    //	参数说明 : const  std::string &  value: 需要判断的字符串
    //const  std::string &  prefix　前缀
    ///////////////////////////////////////////////////////////////
    static Boolean startsWith(const  std::string &  value,const  std::string &  prefix);

    static Boolean endsWith(const  std::string &  value,const  std::string &  postfix);
    ///////////////////////////////////////////////////////////////
    //	函 数 名 : replace
    //	函数功能 : 将字符串value中的原old字符串，用replaceValue字符串替换
    //	备    注 :
    //	返 回 值 : string,返回替换后的字符串
    //	参数说明 : const  std::string &  value: 需要处理的字符串
    ///////////////////////////////////////////////////////////////
    static std::string replace(const  std::string &  value,const  std::string &  old,const  std::string &  replaceValue);

    static void splitString(std::string OrigString, std::string Seperator,std::list<std::string>& OutputList);

    static INT64 getIntValue(char * pstr);

    static UINT32 BCDToStr(char* pSrc, UINT32 srcLen, char* pDest, UINT32 destLen);

};

#endif //__UTIL_STRING_UTIL_H__

