#ifndef __UTIL_DATETIME_H__
#define __UTIL_DATETIME_H__

#include <sstream>
#include <api/diam_datatype.h>

#ifndef _WIN32
#   include <sys/time.h>
#endif

class DateTime
{
public:
    DateTime();
    static  DateTime now();
    static  DateTime seconds(INT64);
    static  DateTime milliSeconds(INT64);
    static  DateTime microSeconds(INT64);

    std::string toString() const;
#ifndef _WIN32
    operator timeval() const;
#endif
    INT64 toSeconds() const;
    INT64 toMilliSeconds() const;
    INT64 toMicroSeconds() const;

    DateTime operator +(const DateTime& rhs) const;
    DateTime& operator +=(const DateTime& rhs);
    DateTime operator-(const DateTime& rhs) const;
    bool operator<(const DateTime& rhs) const;
    bool operator>(const DateTime& rhs) const;
    bool operator == (const DateTime& rhs) const;

public:
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar & useconds;
    }

private:
    DateTime(INT64 usec) : useconds(usec)
    {
    }
    INT64 useconds;

private:
    enum
    {
        Begin_Year = 1900, //struct tm.tm_year 从1900年开始
        Max_Number = 512
    };
};

std::ostream&  operator<<(std::ostream&, const DateTime&);

#endif //__UTIL_DATETIME_H__

