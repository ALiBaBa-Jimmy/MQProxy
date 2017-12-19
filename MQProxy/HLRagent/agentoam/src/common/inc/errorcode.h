#ifndef _XW_ERRORCODE_H
#define _XW_ERRORCODE_H

//物理数据库和内存数据库的错误码定义头文件
#ifndef XSUCC
	#define XSUCC 				(0)
	#define XERROR 				(-1)
	#define XNOFOUNDDATA		(2) //更新和删除操作时，若数据不存在返回该错误码(自定义)
	#define XKEYREPEAT           (1) //插入数据时，若插入数据的主键值冲突则返回该错误码(getErrorCode()返回的错误码)
#endif


// 内存数据库错误 1100-1199
// TCP 错误       1200-1299
// OSS 编解码错误 1300-1399
// DB 编解码错误  1400-1599



//内存数据库错误码
typedef enum
{	
	MEM_DB_SUCCESS=0,	
	MEM_DB_ERR_BEGIN = 1100,
	MEM_DB_ERR_NO_DATA_EFFECT, //=3,
	MEM_DB_ERR_OPEN,// = 4 //DB打开错误(为避免和ORACLE数据库的错误码冲突，而更改其值)
	MEM_DB_ERR_END
}E_MEM_DB_ERR_CODE;

//定义内存数据库的访问模式
enum accessType 
{	
	ReadOnly=0,
	AllAccess,
	ConcurrentRead,
	ConcurrentUpdate
};
#endif

