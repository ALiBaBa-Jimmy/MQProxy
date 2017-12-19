
#ifndef _MYSQL_CONNECTION_POOL_H_
#define _MYSQL_CONNECTION_POOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os.h"
#include "mysql.h"

#pragma pack(1)


#define MYSQL_CONNECT_POOL_MAX_COUNT  10		/* ���ӳص���������� */

#define pMysql(dbTaskId)       &mysql_connectionpool.st[dbTaskId].mySql
	
typedef MYSQL 		MYSQL_Connection, *pMysqlConn;
typedef MYSQL_RES	*pMysqlRes;
typedef MYSQL_FIELD  *pMysqlField;
typedef MYSQL_ROW   MysqlRow, *pMysqlRow;

typedef struct
{
	XU8 	connect_state;   
	struct 
	{
		XU32	QryCount;
		XU8     use_state;
	}Link[MYSQL_CONNECT_POOL_MAX_COUNT];
}_MYSQL_LinkState;

typedef struct
{	
	int  in_use;			 /* 1:����������ʹ��, 0:���ӿ��� */
	MYSQL_Connection mySql;
}_MYSQL_Connection;

typedef struct   
{
	int in_use_Count;		/* ���ӳ�������ʹ�õ������� */
	_MYSQL_Connection st[MYSQL_CONNECT_POOL_MAX_COUNT];
}_MYSQL_ConnectionPool;



//t_XOSMUTEXID  t_mutex;
ACE_RW_Thread_Mutex m_ConnidConnLock;//��д��������m_mapConnidConn

	
#pragma pack()

/*************************************************************************/
XS32 MYSQL_ConnectPoolInit(XU8 *u,XU8 *p,XU8 *h,XU8 *dbname, XU32 dbport);

XS32 MYSQL_ConnectPoolGet(XU32 *dbTaskId);

XS32 MYSQL_ConnectPoolRealse(XU32 dbTaskId);

XS32 MYSQL_ConnectPoolDestroy();

XS8* TraceMysqlLastError(dbTaskId);

/*************************************************************************/

XBOOL MYSQL_Rollback(XU32 dbTaskId);

XBOOL MYSQL_Commit(XU32 dbTaskId);

XS32 MYSQL_Select(XU32 dbTaskId, XU8 *sqlstr, pMysqlRes *pmysqlres);

XS32 MYSQL_Execute(XU32 dbTaskId, XU8 *sqlstr, XU32 *rowaffected);

/**************************************************************************/

XS32 MYSQL_CheckLinkStatus(void *linkstruct);


// ���ݿⱸ�ݽӿ�

// ����ṹת��ΪSQL���浽�ļ�
int Tablestruct_2_sql(MYSQL *pConn, char *pTableName, char *pfiletime);

// ����ṹ������ת��ΪSQL���浽�ļ�
int Tabledata_2_sql(MYSQL *pConn, char *pTableName, char *pfiletime);


int MYSQL_Backup();

int MYSQL_Restore(char *pfilename);

int Execute_Restore(MYSQL *pConn, char *pSql);


int BatchLoadUserFromFile();

#ifdef __cplusplus
}
#endif

#endif
