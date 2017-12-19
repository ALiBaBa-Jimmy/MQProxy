// Copyright 2001-2002 Huawei Technologies Co.,Ltd. All rights reserved.

#ifndef __UTIL_CALENDAR_H__
#define __UTIL_CALENDAR_H__

#include <string>
#include <util/diam_datetime.h>
#include <util/diam_time.h>
#include <util/diam_exception.h>

class DateTimeException : public Exception
{
public:
    DateTimeException(const std::string & file, const INT32 & line,const std::string &message);
private:
    std::string _message;
};

///////////////////////////////////////////////////////////////
//	��    �� : Calendar
//	��    �� : ����ʱ���࣬�����ꡢ�¡��ա�ʱ���֡�����Ϣ
//	��    �� :
///////////////////////////////////////////////////////////////
class Calendar
{
public:
    //*****************************************
    //�·ݱ�ʶ���ӣ���ʼ
    //*****************************************
    enum Month
    {
        Month_January = 1,
        Month_February,
        Month_March,
        Month_April,
        Month_May,
        Month_Jule,
        Month_July,
        Month_August,
        Month_September,
        Month_October,
        Month_November,
        Month_December
    };
    //*****************************************
    //���ڱ�ʶ���ӣ���ʼ����ʾ������
    //*****************************************
    enum Week
    {
        Week_Sunday = 0,
        Week_Monday,
        Week_Tuesday,
        Week_Wednesday,
        Week_Thursday,
        Week_Friday,
        Week_Saturday
    };

    enum
    {
        Begin_Year = 1900, //struct tm.tm_year ��1900�꿪ʼ
        Min_Year = 1971,
        Max_Year = 2037,
        One_Day_Sec = 86400
    };

public:
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : Calenda
    //	�������� : ���캯��
    //	�������� : Ĭ��Ϊ��ǰ����ʱ��
    //	��    ע :
    //	�� �� ֵ :
    //	����˵�� : ��
    ///////////////////////////////////////////////////////////////
    Calendar();

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : Calenda
    //	�������� : ���캯������DateTime������ʼ������ʱ��
    //	��    ע :
    //	�� �� ֵ :
    //	����˵�� : DateTime dateTime���ڳ�ʼ������ʱ��Ĳ���
    ///////////////////////////////////////////////////////////////
    Calendar(const DateTime &dateTime);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : Calenda
    //	�������� : ���캯������������ʱ��������ʼ������ʱ��
    //	��    ע :
    //	�� �� ֵ :
    //	����˵�� : Int year��	�꣬��1970~1937
    //				 Int month���£���1~12
    //				 Int day��	�գ���1~31
    //				 Int hr:	ʱ����0~23
    //				 Int mi,    �֣���0~59
    //				 Int sec    �룬��0~59
    ///////////////////////////////////////////////////////////////
    Calendar(INT32 year, INT32 month, INT32 day, INT32 hour, INT32 minute, INT32 second);


    ///////////////////////////////////////////////////////////////
    //	�� �� �� : Calenda
    //	�������� : ���캯�������ַ�����ָ���ĸ�ʽ���ͣ�����ת��Ϊ����ʱ��
    //	��    ע :
    //	�� �� ֵ :
    //	����˵�� : const string & dateTime��������ʼ�����ַ���
    //				 const string & fmt�������ַ����ĸ�ʽ��Ĭ��Ϊ"yyyy-mm-dd hh:mi:ss"
    ///////////////////////////////////////////////////////////////
    Calendar(const std::string & dateTime, const std::string & fmt ="yyyy-mm-dd hh:mi:ss");
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : format
    //	�������� : ��ָ���ĸ�ʽ��������ʱ��ת��Ϊ�ַ���
    //	��    ע :
    //	�� �� ֵ : string
    //	����˵�� : const string & fmt����Ҫת���ɵĸ�ʽ��ָ��Ϊ "yyyy-mm-dd hh:mi:ss"
    ///////////////////////////////////////////////////////////////
    std::string format(const std::string & fmt = "yyyy-mm-dd hh:mi:ss") const;

public:
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : getYear
    //	�������� : �õ�����ʱ���е���
    //	�� �� ֵ : Intֵ����Ϊ1970~2037
    //	����˵�� : ��
    ///////////////////////////////////////////////////////////////
    INT32 getYear() const;


    ///////////////////////////////////////////////////////////////
    //	�� �� �� : getMonth
    //	�������� : �õ�����ʱ���е���
    //	��    ע :
    //	�� �� ֵ : Int��ֵ��1~12��
    //	����˵�� :
    ///////////////////////////////////////////////////////////////
    INT32 getMonth() const;

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : getDayOfMonth
    //	�������� : �õ�����ʱ���е��գ���һ�����е�λ��
    //	��    ע :
    //	�� �� ֵ : Int��ֵ��1~31��
    //	����˵�� :
    ///////////////////////////////////////////////////////////////
    INT32 getDayOfMonth() const;

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : getDayOfWeek
    //	�������� : �õ����ڼ�
    //	��    ע :
    //	�� �� ֵ : Week���μ���ö�����͵�˵��
    //	����˵�� :
    ///////////////////////////////////////////////////////////////
    Week getDayOfWeek() const;

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : getHour
    //	�������� : �õ�����ʱ���е�Сʱ
    //	��    ע :
    //	�� �� ֵ : Int��ֵ��0~23��
    //	����˵�� :
    ///////////////////////////////////////////////////////////////
    INT32 getHour() const;

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : getMinute
    //	�������� : �õ�����ʱ���еķ�
    //	��    ע :
    //	�� �� ֵ : Int��ֵ��0~59��
    //	����˵�� :
    ///////////////////////////////////////////////////////////////
    INT32 getMinute() const;

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : getSecond
    //	�������� : �õ�����ʱ���е���
    //	��    ע :
    //	�� �� ֵ : Int��ֵ��0~59��
    //	����˵�� :
    ///////////////////////////////////////////////////////////////
    INT32 getSecond() const;

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : set
    //	�������� : ��������ʱ������������ʱ��
    //	��    ע :
    //	�� �� ֵ :
    //	����˵�� : Int year��	�꣬��1970~1937
    //				 Int month���£���1~12
    //				 Int day��	�գ���1~31
    //				 Int hour:	ʱ����0~23
    //				 Int minute,   �֣���0~59
    //				 Int second    �룬��0~59
    ///////////////////////////////////////////////////////////////
    void set(INT32 year, INT32 month, INT32 day, INT32 hour, INT32 minute, INT32 second);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : set
    //	�������� : ����������������ʱ��
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int year��	�꣬��1970~1937
    //				 Int month���£���1~12
    //				 Int day��	�գ���1~31
    ///////////////////////////////////////////////////////////////
    void set(INT32 year, INT32 month, INT32 day);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : set
    //	�������� : ��Time��������ʱ�䣬Timeֻ������ʱ����
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Time  time_���ṹ�μ�Time.h
    ///////////////////////////////////////////////////////////////
    void set(Time  time_);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : setDateTime
    //	�������� : ��DateTime��������ʱ��
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : const DateTime &dateTime���μ�DateTime�ṹ
    ///////////////////////////////////////////////////////////////
    void setDateTime(const DateTime &dateTime);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : setYear
    //	�������� : ������
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int year��ֵ��1970~1937֮��
    ///////////////////////////////////////////////////////////////
    void setYear(INT32 year);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : setMonth
    //	�������� : ������
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int month��ֵ��1~12֮��
    ///////////////////////////////////////////////////////////////
    void setMonth(INT32 month);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : setDayOfMonth
    //	�������� : ������
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int day��ֵ��1~31֮��
    ///////////////////////////////////////////////////////////////
    void setDayOfMonth(INT32 day);
    void setDate(INT32 day,INT32 month,INT32 year);
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : setHour
    //	�������� : ����ʱ
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int hour��ֵ��0~59֮��
    ///////////////////////////////////////////////////////////////
    void setHour(INT32 hour);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : setMinute
    //	�������� : ���÷�
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int minute��ֵ��0~59֮��
    ///////////////////////////////////////////////////////////////
    void setMinute(INT32 minute);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : setSecond
    //	�������� : ������
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int second ֵ��0~59֮��
    ///////////////////////////////////////////////////////////////
    void setSecond(INT32 second);
public:
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : getActualMaximumDayOfMonth
    //	�������� : �õ������µ��������
    //	��    ע :
    //	�� �� ֵ : Int����0~31֮��
    //	����˵�� :
    ///////////////////////////////////////////////////////////////
    INT32 getActualMaximumDayOfMonth() const;
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : getActualMaximumDayOfMonth
    //	�������� : �õ�ָ�����������µ��������
    //	��    ע :
    //	�� �� ֵ : Int
    //	����˵�� : const Int year,������
    //				 Int month����Ҫ�������
    ///////////////////////////////////////////////////////////////
    INT32 getActualMaximumDayOfMonth(const INT32 year, const INT32 month) const;
public:
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : addSeconds
    //	�������� : ������
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int sec
    ///////////////////////////////////////////////////////////////
    void addSeconds(INT32 sec);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : addDays
    //	�������� : ������
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int day
    ///////////////////////////////////////////////////////////////
    void addDays(INT32 day);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : addMonths
    //	�������� : ������
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int mon
    ///////////////////////////////////////////////////////////////
    void addMonths(INT32 mon);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : addYears
    //	�������� : ������
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : Int year
    ///////////////////////////////////////////////////////////////
    void addYears(INT32 year);

    std::string toString();
public:
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : isLeapYear
    //	�������� : �ж��Ƿ�Ϊ����
    //	��    ע :
    //	�� �� ֵ : boolean,�����귵��true,���򷵻�false
    //	����˵�� : Int yearҪ�жϵ����
    ///////////////////////////////////////////////////////////////
    static Boolean isLeapYear(INT32 year);

    ///////////////////////////////////////////////////////////////
    //	�� �� �� : getDateTime
    //	�������� : ��DateTime��ʽ��������ʱ��
    //	��    ע :
    //	�� �� ֵ : DateTime���μ�DateTime.h
    //	����˵�� :
    ///////////////////////////////////////////////////////////////
    DateTime getDateTime() ;
protected:
    ///////////////////////////////////////////////////////////////
    //	�� �� �� : setDateTime
    //	�������� : ��struct tmֵת��������ʱ��
    //	��    ע :
    //	�� �� ֵ : void
    //	����˵�� : struct tm & dateTime
    ///////////////////////////////////////////////////////////////
    void setDateTime(struct tm & dateTime);
private:
    struct tm _dateTime;
};

#endif //__UTIL_CALENDAR_H__