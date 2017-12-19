#ifndef __BASIC_TIME_H__
#define __BASIC_TIME_H__
#include <util/diam_ace.h>
#include<string>

#ifdef WIN32
#   include <sys/timeb.h>
#   include <time.h>
#else
#   include <sys/time.h>
#endif

enum
{
    Year = 0,
    Month,
    Day,
    Hour,
    Minutes,
    Seconds
};

#define MAX_TIME_STR_LEN  20

class CBasicTime
{
private:
    CBasicTime(void);
    virtual ~CBasicTime(void);
public:
    static bool strToTime(char *pChar,struct tm *pTM);
    static bool strToTime(char* pStrTime, int nStrTimeLen, char* pTimeFmt, int nFmtLen, struct tm* retTime);
    static bool add(struct tm *tmIn,int lAdd,int nType);

    static int convert(struct tm *tmSource,char *szDestination);
    static bool convert(const time_t *ttSource,std::string& strDestination);
    static bool convert(const std::string& strSource, time_t *ttDestination);
    static bool convert(const std::string& strSource,struct tm *m_tm);
    static bool convert(const time_t *ttSource,struct tm *m_tm);
    static bool convert(struct tm* m_tm,std::string& strDest);
    static bool convert(struct tm* m_tm,time_t* ttDestination);
    static bool addTime(const time_t *ttSource,time_t *ttDest,int interval,int nType);
    static bool addTime(const std::string& source,std::string& dest,int interval,int nType);

    static std::string getCurrentTime();
    static std::string getCurrentTime(const std::string timeformat);
    static bool getDiff(int& nret,const std::string& strStart,const std::string& strEnd);
    static int getWeekDay(const time_t& ttIn);
    static int getWeekDay(const std::string& strIn);
    static int getHour();
    static int getMinute;
    static int getSecond();

    static bool timeverify(int year,int month,int day,int hour,int min,int sec);
    static bool IsLeapYear(int year);
    static int getMonDays(int year, int month);
};
#endif //__BASIC_TIME_H__
