/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosfilesys.h
**
**  description:  对win2000/turbolinux/solaris/vxworks下的文件和目录操作进行封装的头文件
**
**  author: lixiangni
**
**  date:   2006.8.22
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   lxn                2006.8.22          create
**************************************************************/
#ifndef _XOS_FILESYS_H_
#define _XOS_FILESYS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------*
                 定义包含的头文件
*--------------------------------------------------------------*/
#include "xosos.h"

/*-------------------------------------------------------------*
                 全局宏定义
*--------------------------------------------------------------*/

/*
20061020 modify
与以前的文件操作兼容，将xosos.h中定义的文件操作宏迁移至这里
*/

/* 关于文件操作的宏*/
#define XOS_Fgets(string,n,stream)  fgets( (char *)string,(int)(n),(FILE *)stream )

#define XOS_Fopen( filename , mode )  fopen( (const char *)filename,(const char *)mode )
#define XOS_Fclose( fileptr )  XOS_CloseFile(&fileptr);
#define XOS_Fread( buffer,size,count,stream )   fread( (void *)buffer,(size_t )(size),(size_t )(count),(FILE *)stream )
#define XOS_Fwrite( buffer,size,count,stream )  fwrite( (const void *)buffer,(size_t )(size),(size_t)(count),(FILE *)stream )
#define XOS_FFlush(fdptr) fflush(fdptr)

/*

*/
//#ifndef __STDC__
//#define __STDC__
//#endif
#ifndef MAX_DIRANDFILEPATH_LEN
#define MAX_DIRANDFILEPATH_LEN 4096  /*目录和文件路径的最大长度*/
#endif

#ifndef  MAX_FILECAP
#define  MAX_FILECAP 1024*1024*10 /*定义文件最大容量为10M*/
#endif

#define XF_SEEKCUR    1
#define XF_SEEKEND    2
#define XF_SEEKSET    0
/*-------------------------------------------------------------*
                  内部宏定义(最后可以移植到内部使用的.h文件中)
*--------------------------------------------------------------*/
#define MAX_FILECPYMEM 1024*1024*10 /*读文件时最大的缓冲内存*/
#define MAX_ORDERLEN 255 /*命令行以及其参数的总共长度*/
#define XOS_GetFileLen  XOS_FileLen /*保持取消xosos.h中xos_GetFileLen后的兼容性*/

/*--------------------------------------------------------------*
                 全局结构定义
*---------------------------------------------------------------*/
    
    typedef enum           /*文件打开模式定义*/
    {
            XF_RTMODE = 0,
            XF_WTMODE,
            XF_ATMODE,
            XF_RBMODE,
            XF_WBMODE,
            XF_ABMODE,
            XF_RPLUSTMODE,
            XF_WPLUSTMODE,
            XF_APLUSTMODE,
            XF_RPLUSBMODE,
            XF_WPLUSBMODE,
            XF_APLUSBMODE
    }e_OPENFILEMODE; 

    /*删除目录模式结构*/
    typedef enum
    {
            EXPDEL = 0,
            NORMALDEL
    }e_DELDIRMODE;
    /*错误码定义*/
    typedef enum
    {           
            XERRPATH = -2, /*无效文件/目录路径*/
            XERRPARA = -3,/*无效参数*/
            XINVALS = -4,/*无效的文件流*/
            XERRMODE = -5, /*无效文件打开模式，包括读写文件模式不对称*/
            XFILEFULL = -6,/*文件满*/
            XFEOF = -7, /*文件结尾*/
            XDIREMPTY = -8, /*目录非空*/
            XMALLOCERROR = -9,/*错误码，内存申请失败*/
            XFPATHEXSS  =-10,/*路径过长*/
            XF_PLATEXPERROR = -11,/*平台定义系统异常错误（主要指没有制定相应的底层工作系统）*/
            XF_NOFIXNAME = -12, /*输入的文件没有扩展名，只做判断用*/
            XF_EXIST = -13,/*文件或者目录已经存在*/
            XF_LOGKFAILED = -14/*加锁失败*/
    }e_ERRORCODENUM;
    
    /*文件属性结构定义*/
     struct _FILEATT
    { 
         XS16 f_gid;  /**/
         time_t f_latime;  /*文件最晚访问时间*/
         time_t f_lctime;  /*文件最晚创建时间*/
         XU32 f_dev;  /*文件所在磁盘序号*/
         XU16 f_ino;  /**/
         XU16 f_mode;  /*文件打开模式掩码*/ 
         time_t f_mtime;  /*文件最晚修改时间*/
         XS16 f_nlink;  /*非NTFS文件系统中为1*/
         XU32 f_rdev;  /*同f_dev*/
         XUFILESIZE f_size;  /*bytes的文件大小*/
         XS16 f_uid;  /**/
         
    };
    
typedef struct _FILEATT t_FILEATT;    


/*---------------------------------全局变量定义-----------*/

//XSTATIC XS32 XOS_FILE_ERRORNO;
/*--------------------------------------------------------------*
                 内部结构定义(最后可以移植到内部使用的.h文件中)
*---------------------------------------------------------------*/


/*---------------------------------------------------------------*
              文件管理模块软件接口
*----------------------------------------------------------------*/
/************************************************************************
函数名:    FS_Init
功能：对文件管理模块进行初始化
输入：
输出：
返回：           
说明：这里主要是对文件管理模块中的全局变量进行初始化
                包括:系统默认工作目录等
************************************************************************/
XEXTERN XVOID FS_Init(XVOID);

/*---------------------------------------------------------------*
               文件和目录操作对外接口函数
*----------------------------------------------------------------*/


/************************************************************************
函数名:    XOS_OpenFile
功能：建立文件
输入：filename -- 要打开的文件名，不能有重名的文件，否则返回错误，统一的路径格式为/path/filename+扩展名
      如果没有扩展名，则统一文件扩展名为.txt      
输出：
返回：    成功返回指向该文件的文件指针
          失败返回NULL           
说明：对各个系统下的打开文件函数的封装，同时根据操作返回错误参数.
      打开文件模式为：二进制，可读写
************************************************************************/
XEXTERN FILE* XOS_OpenFile(XCONST XCHAR * filename, e_OPENFILEMODE mode);

/************************************************************************
函数名:    XOS_WriteFile
功能：将数据写入文件
输入：ptrdata--要写得数据指针，非空。
      datasize--要写入的数据中每个单独的数据结构的大小
      datanum--对上述数据结构总共要写多少次
     fileptr -- 要读取的文件指针，必须不为空且与存在的文件对应，否则返回错误。应该为打开文件所得到的文件指针      
输出：
返回：    成功返回XSUCC
          失败返回XERROR           
说明：datasize*datanum 应该等于ptrdata的字节数
************************************************************************/
XEXTERN XS32 XOS_WriteFile(XCONST XVOID *ptrdata, XU32 datasize, XU32 datanum, FILE *fileptr);

/************************************************************************
函数名:    XOS_GetFileAtt
功能：获得文件的各属性
输入：filename -- 文件名，必须存在，否则返回错误，统一的路径格式为/path/filename+扩展名
      如果没有扩展名，则统一文件扩展名为.txt      
      fileatt--t_FILEATT结构，用来记录文件的所有属性值
输出：
返回：    成功返回XSUCC
          失败返回XERROR,以及错误码           
说明：
************************************************************************/
XEXTERN XS32 XOS_GetFileAtt(XCONST XCHAR * filename, t_FILEATT * fileatt);

/************************************************************************
函数名:    XOS_FileLen
功能：获得文件长度,字节
输入：filename -- 文件名，必须存在，否则返回错误，统一的路径格式为/path/filename+扩展名
      如果没有扩展名，则统一文件扩展名为.txt      
     length--文件长度
输出：
返回：    成功返回XSUCC
          失败返回XERROR,以及错误码           
说明：
************************************************************************/
XEXTERN XS32 XOS_FileLen(XCONST XCHAR * filename, XUFILESIZE *filelength);

/************************************************************************
函数名:    XOS_SetFilePtr
功能：设置文件指针位置
输入：fileptr -- 文件指针
      offset--偏移量
      currentpos-开始位置，值包括：
                            XF_SEEKCUR-从当前文件指针位置开始
                            XF_SEEKSET-从文件头开始
                            XF_SEEKEND---从文件末尾开始
      
输出：
返回：    成功返回文件字节数大小
          失败返回XERROR           
说明：
************************************************************************/
XEXTERN XS32 XOS_SetFilePtr(const FILE *fileptr, XS32 offset, XU32 currentpos);

/************************************************************************
函数名:    XOS_GetFilePtr
功能：设置文件指针位置
输入：fileptr -- 文件指针      
输出：
返回：    成功返回文件指针位置
          失败返回XERROR以及错误码           
说明：
************************************************************************/
XEXTERN XS32 XOS_GetFilePtr(FILE *fileptr);

/************************************************************************
函数名:    XOS_ReadFile
功能：读取文件内容到指定的内存中
输入：desdata--将文件内容读取后放入的内存块地址
      datasize--desdata所指数据结构的大小
      length-- desdata中所指数据结构的总长度 
    fileptr -- 要读取的文件指针，必须不为空且与存在的文件对应，否则返回错误。应该为打开文件所得到的文件指针      
输出：
返回：    成功返回总共读取的字符个数
          失败返回XERROR           
说明：datasize*length必须等于desdata的字节数，同时必须等于fileptr所指文件的总字节数；
      该函数根据目标内存desdata的大小来读取文件中的所有数据
************************************************************************/
XEXTERN XS32 XOS_ReadFile(XVOID *desdataptr, XU32 datasize,XU32 datanum, FILE *fileptr);

#if 0
/************************************************************************
函数名:    XOS_ReadVoxFile
功能：读取文件所有内容到指定的内存中
输入：filename--将文件名
           result--存储读出的数据
      resultlen-- 取出数据的实际长度
    filemaxlen --      
输出：
返回：    成功返回总共读取的字符个数
          失败返回XERROR           
说明：
************************************************************************/
XEXTERN XS32 XOS_ReadVoxFile(XU8 *filename ,XU8 *result ,XU32 *resultlen ,XU32 filemaxlen);
#endif

/************************************************************************
函数名:    XOS_CloseFile
功能：关闭文件
输入：fileptr -- 要关闭的文件指针的指针，必须不为空且与存在的文件对应，否则返回错误。应该为打开文件所得到的文件指针      
输出：
返回：    成功返回XSUCC
          失败返回XERROR           
说明：
************************************************************************/
XEXTERN XS32 XOS_CloseFile(FILE **fileptr);

/************************************************************************
函数名:    XOS_DeleteFile
功能：删除文件
输入：filename -- 要删除的文件名，必须存在且处于关闭状态，否则返回错误，统一的路径格式为/path/filename+扩展名
      如果没有扩展名，则统一文件扩展名为.txt      
输出：
返回：    成功返回XSUCC
          失败返回XERROR           
说明：
************************************************************************/
XEXTERN XS32 XOS_DeleteFile(XCONST XCHAR * filename);

/************************************************************************
函数名:    XOS_MoveFile
功能：将源文件移动到目标文件所指的路径下
输入：srcfilename -- 要移动的源文件，必须存在。该文件在进行操作之前必须处于关闭状态
      desfilename--要目的文件，该文件已经存在时，返回报错。如果该文件不存在，则将生成新文件并将源文件移动过来     
输出：
返回：    成功返回XSUCC
          失败返回XERROR           
说明：文件名统一格式为:/path/filename+扩展名。如果没有扩展名，则以.txt文件对待
************************************************************************/
XEXTERN XS32 XOS_MoveFile(XCONST XCHAR * srcfilename, XCONST XCHAR * desfilename);

/************************************************************************
函数名:    XOS_CopyFile
功能：将源文件复制一份到目标文件，源文件保持不变
输入：srcfilename -- 要复制的源文件，必须存在。该文件在进行操作之前必须处于关闭状态
       desfilename--要复制的目的文件，该文件已经存在时，必须处于关闭状态，且内容将被覆盖；
                    如果该文件不存在，则将生成新的文件
输出：
返回：    成功返回XSUCC
          失败返回XERROR           
说明：将srcfilename中的所有内容写入到desfilename中，操作之前两个文件必须处于关闭状态；
       文件路径格式为：/path/filename+扩展名，否则都以.txt文件对待
************************************************************************/
XEXTERN XS32 XOS_CopyFile(XCONST XCHAR * srcfilename, XCONST XCHAR * desfilename);

/************************************************************************
函数名:    XOS_CreatDir
功能：建立文件
输入：dirname--要创建的目录路径名。不能和已经存在的目录路径名重复，否则出错。
       统一格式为：/path/dirname      
输出：
返回：    成功返回XSUCC
          失败返回XERROR           
说明：
************************************************************************/
XEXTERN XS32 XOS_CreatDir(XCONST XCHAR * dirname);

/************************************************************************
函数名:    XOS_ChangCurrentDir
功能：建立文件
输入：dirpath -- 要进入的工作目录，必须存在，否则返回错误，统一的路径格式为/path/dirname      
输出：
返回：    成功返回XSUCC
          失败返回XERROR           
说明：
************************************************************************/
XEXTERN XS32 XOS_ChangeCurrentDir(XCONST XCHAR * dirpath);

/************************************************************************
函数名:    XOS_DeleteDir
功能：删除目录
输入：dirname -- 要删除的目录路径名，必须存在，否则返回错误，统一的路径格式为/path/dirname

输出：
返回：    成功返回XSUCC
          失败返回XERROR           
说明：
************************************************************************/
XEXTERN XS32 XOS_DeleteDir(XCONST XCHAR * dirname,e_DELDIRMODE delmode);

/************************************************************************
函数名:    XOS_IsExistDir
功能：查看dirname目录是否已经建立， Dirname格式为：/exm/dirname。
输入：dirname-要查看的目录名，统一的路径格式为/path/dirname

输出：
返回：    成功返回XSUCC
          失败返回XERROR           
说明：
************************************************************************/
XEXTERN XS32 XOS_IsExistDir(XCONST XCHAR * dirname);

/************************************************************************
函数名:    XOS_GetCurrentWorkDir
功能：查看当前用户所在的工作目录路径

输出：
返回：    成功返回工作路径
          失败返回NULL           
说明：
************************************************************************/
XEXTERN XVOID XOS_GetCurrentWorkDir (XCHAR *currentdir,XU32 maxlen);

/************************************************************************
函数名:    XOS_GetCurrDiskNO
功能：获取当前工作硬盘的序号。
输入：

输出：
返回：    成功返回硬盘序号
          失败返回XERROR以及错误码           
说明：
************************************************************************/
XEXTERN XS8 XOS_GetCurrDiskNO(XVOID);

/************************************************************************
函数名:    XOS_CurrDiskFreeCap
功能：获取当前工作硬盘的剩余可用空间容量。
输入： driveno-当前工作硬盘序号
       Cap -要返回的剩余空间


输出：
返回：    成功返回XSUCC
          失败返回XERROR           
说明： 
************************************************************************/
XEXTERN XS32 XOS_CurrDiskFreeCap(XCONST XU8 driveno,XU32 *cap);

/**********************************************************
  功能：判断输入文件路径串是否正确,如果路径组成合法则在不同的系统下转化为不同的格式
  输入串错误返回XERROR，否则转化串，并返回XSUCC
***********************************************************/
XS32 FS_formatpath(XCONST XCHAR *srcpathstr,XCHAR *formatpathstr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*xosfilesys.h*/


