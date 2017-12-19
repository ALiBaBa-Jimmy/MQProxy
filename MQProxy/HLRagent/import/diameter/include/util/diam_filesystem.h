#ifndef __UTIL_FILE_SYSTEM_H__
#define __UTIL_FILE_SYSTEM_H__

#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <util/diam_exception.h>
#include <util/diam_datetime.h>

class FileSystemException : public Exception
{
public:
    FileSystemException(const string & file, const  INT32 & line,
                        const string &message,const INT32 & code);
};

#define MAX_LENGTH_PATH 512


class  FileSystem
{
private:
    FileSystem();
public:
    static bool isRoot(const std::string& szPath);
    static std::string simplify(const std::string& szPath);
    static std::string pathCat(const std::string& szPath,const std::string& szFileName);
    static std::string nativePath(const std::string& szPath);
    static std::string getBasename(const std::string& szPath);
    static std::string getDirname(const std::string& szPath);
    static std::string getCurrentPath();
    static void remove(const std::string& szPath);
    static void removeRecursive(const std::string& szPath);
    static void createDirectory(const std::string& szPath);
    static void createDirectoryRecursive(const std::string& szPath);
    static long size(const std::string& path);
    static bool IsExists(const std::string& path);

public:
    static const std::string _separator;
    //»ù×¼Ä¿Â¼
    static std::string m_strBasePath;
};

#endif //__UTIL_FILE_SYSTEM_H__


