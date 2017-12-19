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
    //	�� �� �� : string toUpper
    //	�������� : ������������ַ�����Сд����ĸת���ɴ�д��
    //	��    ע :
    //	�� �� ֵ : string,����ת����û��Сд��ĸ���ַ���
    //	����˵�� : const  std::string &  value����Ҫת�����ַ���
    ///////////////////////////////////////////////////////////////
    static std::string toUpper(const  std::string &  value);
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : string toLower
    //	�������� : ������������ַ����д�д����ĸת����Сд��
    //	��    ע :
    //	�� �� ֵ : string,����ת����û�д�д��ĸ���ַ���
    //	����˵�� : const  std::string &  value����Ҫת�����ַ���
    ///////////////////////////////////////////////////////////////
    static std::string toLower(const  std::string &  value);
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : equalsIgnoreCase
    //	�������� : �Ƚ������ַ����Ƿ���ȣ��Ƚ�ʱ���Դ�Сд
    //	��    ע :
    //	�� �� ֵ : Boolean,�����ȣ��򷵻�true,���򷵻�false
    //	����˵�� : const  std::string &  value,const  std::string &  another
    //
    ///////////////////////////////////////////////////////////////
    static Boolean equalsIgnoreCase(const  std::string &  value,const  std::string &  another);
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : string trim
    //	�������� : ȥ��ȥ����ͷβ�Ŀո���������ɼ�����(��ASCII��С�ڵ��ڿո�ķ���)
    //	��    ע :
    //	�� �� ֵ : string���������ַ���
    //	����˵�� : const  std::string &  value����Ҫ����Ĳ���
    ///////////////////////////////////////////////////////////////
    static std::string trim(const  std::string &  value);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : string trimLeft
    //	�������� : ȥ��ȥ������ߵĿո���������ɼ�����(��ASCII��С�ڵ��ڿո�ķ���)
    //	��    ע :
    //	�� �� ֵ : string���������ַ���
    //	����˵�� : const  std::string &  value����Ҫ����Ĳ���
    ///////////////////////////////////////////////////////////////
    static std::string trimLeft(const std::string & value);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : string trimRight
    //	�������� : ȥ��ȥ�����ұߵĿո���������ɼ�����(��ASCII��С�ڵ��ڿո�ķ���)
    //	��    ע :
    //	�� �� ֵ : string���������ַ���
    //	����˵�� : const  std::string &  value����Ҫ����Ĳ���
    ///////////////////////////////////////////////////////////////
    static std::string trimRight(const std::string & value);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : skipWhiteSpace
    //	�������� : ���ַ���ָ��λ��index��ʼ�ҵ�һ���ǿ��ַ���λ��
    //	��    ע :
    //	�� �� ֵ : Int����ҵ����򷵻��ҵ������������û���ҵ����򷵻�-1
    //	����˵�� : const  std::string &  value: ��Ҫ�ҵ��ַ���
    //unsigned int index����ʼ�ҵ�λ��
    ///////////////////////////////////////////////////////////////
    static INT32 skipWhiteSpace(const  std::string &  value, unsigned int index);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : findNextWhiteSpace
    //	�������� : ���ַ���ָ��λ��index��ʼ�ҵ�һ�����ַ���λ��
    //	��    ע :
    //	�� �� ֵ : Int����ҵ����򷵻��ҵ������������û���ҵ����򷵻�-1
    //	����˵�� : const  std::string &  value: ��Ҫ�ҵ��ַ���
    //unsigned int index����ʼ�ҵ�λ��
    ///////////////////////////////////////////////////////////////
    static INT32 findNextWhiteSpace(const  std::string &  value, unsigned int index);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : startsWith
    //	�������� : �ж��ַ���valueǰ׺�Ƿ�Ϊprefix
    //	��    ע :
    //	�� �� ֵ : Boolean��������򷵻�true,���򷵻�false
    //	����˵�� : const  std::string &  value: ��Ҫ�жϵ��ַ���
    //const  std::string &  prefix��ǰ׺
    ///////////////////////////////////////////////////////////////
    static Boolean startsWith(const  std::string &  value,const  std::string &  prefix);

    static Boolean endsWith(const  std::string &  value,const  std::string &  postfix);
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : replace
    //	�������� : ���ַ���value�е�ԭold�ַ�������replaceValue�ַ����滻
    //	��    ע :
    //	�� �� ֵ : string,�����滻����ַ���
    //	����˵�� : const  std::string &  value: ��Ҫ������ַ���
    ///////////////////////////////////////////////////////////////
    static std::string replace(const  std::string &  value,const  std::string &  old,const  std::string &  replaceValue);

    static void splitString(std::string OrigString, std::string Seperator,std::list<std::string>& OutputList);

    static INT64 getIntValue(char * pstr);

    static UINT32 BCDToStr(char* pSrc, UINT32 srcLen, char* pDest, UINT32 destLen);

};

#endif //__UTIL_STRING_UTIL_H__

