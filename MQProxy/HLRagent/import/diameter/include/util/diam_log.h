#ifndef __DIAM_LOG_H__
#define __DIAM_LOG_H__

#include <string>
#include <util/diam_ace.h>
#include <util/diam_datetime.h>
#include <api/diam_interface.h>
#include <api/diam_define.h>

#if defined (_MSC_VER)  && (_MSC_VER <= 1200)
#		define  __FUNCTION__ ""
#endif

#define MAX_MESSAGE 100*1024

//日志级别
typedef ELogLevel LogPriority;

const std::string g_LogPriorityList[] =
{
    "trace",
    "debug",
    "info",
    "waring",
    "error",
    "off"
};

extern ACE_Thread_Mutex g_mutex;
extern LogPriority g_nInitLevel;
extern diamLogFunc g_log_func;

LogPriority toLevel(const std::string& strlevel);
std::string toLevel(LogPriority level);
void setLevel(const std::string& level);
void setLevel(LogPriority level);
bool isOutput(LogPriority level);
void logPrint(UINT32 pid, UINT32 tid, INT32 line, INT32 priority, std::string& message);
void setLogFunc(diamLogFunc func);

extern std::string GetFormatString(const char * formatString, ...);
extern std::string GetFormatString(const std::string & message);
extern std::string GetFormatInt(const long & errorID);

#define LOG_TRACE_MSG(MESSAGE) do {\
    if(isOutput(LOG_TRACE))\
{\
    g_mutex.acquire();\
    std::string message = GetFormatString MESSAGE;\
    if(g_log_func) \
        (*g_log_func)(LOG_TRACE, message.c_str());\
    else\
        logPrint(ACE_OS::getpid(), ACE_Thread::self(), __LINE__, LOG_TRACE, message);\
    g_mutex.release();\
}\
}while (0)

#define LOG_DEBUG_MSG(MESSAGE) do {\
	if(isOutput(LOG_DEBUG))\
{\
    g_mutex.acquire();\
    std::string message = GetFormatString MESSAGE;\
    if(g_log_func) \
        (*g_log_func)(LOG_DEBUG, message.c_str());\
    else\
        logPrint(ACE_OS::getpid(), ACE_Thread::self(), __LINE__, LOG_DEBUG, message);\
    g_mutex.release();\
}\
}while (0)

#define LOG_INFO_MSG(MESSAGE) do {\
    if(isOutput(LOG_INFO))\
{\
    g_mutex.acquire();\
    std::string message = GetFormatString MESSAGE;\
    if(g_log_func) \
        (*g_log_func)(LOG_INFO, message.c_str());\
    else\
        logPrint(ACE_OS::getpid(), ACE_Thread::self(), __LINE__, LOG_INFO, message);\
    g_mutex.release();\
}\
}while (0)

#define LOG_WARNING_MSG(MESSAGE) do {\
    if(isOutput(LOG_WARNING))\
{\
    g_mutex.acquire();\
    std::string message = GetFormatString MESSAGE;\
    if(g_log_func) \
        (*g_log_func)(LOG_WARNING, message.c_str());\
    else\
        logPrint(ACE_OS::getpid(), ACE_Thread::self(), __LINE__, LOG_WARNING, message);\
    g_mutex.release();\
}\
}while (0)

#define LOG_ERROR_MSG(MESSAGE) do {\
    if(isOutput(LOG_ERROR))\
{\
    g_mutex.acquire();\
    std::string message = GetFormatString MESSAGE;\
    if(g_log_func) \
     (*g_log_func)(LOG_WARNING, message.c_str());\
    else\
        logPrint(ACE_OS::getpid(), ACE_Thread::self(), __LINE__, LOG_ERROR, message);\
    g_mutex.release();\
}\
}while (0)

#endif //__UTIL_LOG_UTILS_H__

