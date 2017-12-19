// 下列 ifdef 块是创建使从 DLL 导出更简单的
//宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 DBPROJ_EXPORTS
// 符号编译的。在使用此 DLL 的
//任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将 
// DBPROJ_API 函数视为是从此 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#if (defined(WIN32))
#ifdef DBPROJ_EXPORTS
#define DBPROJ_API __declspec(dllexport)
#else
#define DBPROJ_API __declspec(dllimport)
	#endif
#endif


#ifdef LINUX	
	#define DBPROJ_API 
#endif

// 此类是从 dbproj.dll 导出的
/*class DBPROJ_API Cdbproj {
public:
	Cdbproj(XVOID);
	// TODO: 在此添加您的方法。
};

extern DBPROJ_API XS32 ndbproj;

DBPROJ_API XS32 fndbproj(XVOID);*/
