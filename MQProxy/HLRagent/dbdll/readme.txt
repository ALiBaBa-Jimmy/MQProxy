1、根据合并oss版本时遇到的问题，修改dbproj获得最终修改版本  2011.12.26
2、修改插入数据时，当插入已有数据时，程序崩溃的问题         2011.12.29
3、针对插入数据时，当串长度超过16字节时，程序崩溃的问题，添加setDbChar类型 
    dbproj_v3     2011.12.29
4、问题：查询数据时，当表中串长度超过16字节时，getDBstring无法读出串，程序崩溃的问题
   解决：添加XS8* GetDbChar 
   版本：dbproj_v4  2011.12.30
5、将xostype.h中的xu64和xs64改为long long
  
6、修改XS8* GetDbChar(nCol) 为 void GetDbChar(nCol,XS8*)
   版本：dbproj_v5  2011.12.30
7、在xostype.h中增加#define XNOFOUNDDATA		(-2)
   将xostype.h修改命名为publictype.h
   版本:dbproj_v6  2011.12.31
8、将createstatement中添加对sql为NULL时候的判断
   修改GetDbDouble()返回值修改为double

   修改SetDbDouble()传入参数值修改为double
 
   添加	void SetDbLong(XU32 nCol, long value);
	void SetDbULong(XU32 nCol, unsigned long value);
	long GetDbLong(XU32 nCol);
	unsigned long GetDbULong(XU32 nCol);
	XU32 GetDbUInt(XU32 nCol);
	void SetDbUInt(XU32 nCol, XU32 value);
   版本：dbproj_v7 2012.1.5
9、添加错误码处理函数GetErrorCode();
   修改publictype.h中定义的错误码 
   在publictype.h中删除occi的库引入
   修改连接号 从1开始递增（原来为从0开始递增）
    版本：dbproj_v8 2012.1.11
10、修改ExecuteQurey()的返回值为错误码
    版本：dbproj_v9 2012.03.15
11、在serviceOracle中 添加对map 和 connid的锁机制
    2012.03.28