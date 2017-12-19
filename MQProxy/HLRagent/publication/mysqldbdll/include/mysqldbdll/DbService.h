#pragma once

#include "DbStatement.h"
#include "MysqlDbDll.h"
#define DEFAULT_FID_INTERFACE  0

typedef struct 
{
	XS8* user;
	XS8* pwd;
}SUserInfo;

 enum EnvMode
  {
   /* DEFAULT = OCI_DEFAULT,
    OBJECT = OCI_OBJECT,
    SHARED = OCI_SHARED,
    NO_USERCALLBACKS = OCI_NO_UCB,
    THREADED_MUTEXED = OCI_THREADED,
    THREADED_UNMUTEXED = OCI_THREADED | OCI_NO_MUTEX,
    EVENTS = OCI_EVENTS,
    USE_LDAP = OCI_USE_LDAP*/
    DEFAULT = 0,
    OBJECT = 1,
    SHARED = 2,
    NO_USERCALLBACKS = 3,
    THREADED_MUTEXED = 4,
    THREADED_UNMUTEXED = 5,
    EVENTS = 6,
    USE_LDAP = 7
  };
typedef struct
{
	int mod;
}SDllEnv;

class MYSQLDBDLL_API CDbService
{
public:

	/*���ݿ⻷��������ʼ��*/
    virtual XS32 DbInit(XU8* env, XU32 fid=DEFAULT_FID_INTERFACE)=0; 
	/*���ݿ⻷����������*/
	virtual XS32 DbDestroy(XU32 fid=DEFAULT_FID_INTERFACE)=0;
	/*�õ���ʵ��EnvMode*/
	virtual XS32 GetDbEnvMode(int dbMode)=0;
	//parmeter : �����ַ����ĸ�ʽ��Ҫ���壻
	// mysql ���ݿ� XS8 *connstring��ʾip,XS8*parmeter=���ݿ���
	virtual XS32 DbInitConnection(XS8 *user, XS8 *pwd, XS8 *connstring,XS8*parmeter=NULL,bool isDefault=true, XU32 fid=DEFAULT_FID_INTERFACE)=0;
    
	virtual XS32 DbDestroyConnection(XU32 fid=DEFAULT_FID_INTERFACE)=0;
	
	/*�����ӳ��У�������õ����ݿ�����*/
	virtual XS32 DbCreateConn(XU32 fid=DEFAULT_FID_INTERFACE)=0;

	virtual XS32 DbCreateConn(XS8 *user,XS8 *pwd, XU32 fid=DEFAULT_FID_INTERFACE)=0;
    
	virtual XS32 DBCheckConnection(XU32 dbConnId, XU32 fid=DEFAULT_FID_INTERFACE)=0;

	//check���е�����,���û�����ӳɹ�����ping����������
	virtual XS32 DBCheckConnection()=0;

	/*�����ӳ��У��ͷ����ݿ�����*/
	virtual XS32 DbDestroyConn(XU32 dbConnId, XU32 fid=DEFAULT_FID_INTERFACE)=0;

	
	/*�����ύ*/
	virtual XS32 DbCommit(XU32 dbConnId, XU32 fid=DEFAULT_FID_INTERFACE)=0;
	/*����ع�*/
	virtual XS32 DbRollback(XU32 dbConnId, XU32 fid=DEFAULT_FID_INTERFACE)=0;

	
	/*��ѯ���ݿ�������*/
	virtual CDbStatement* DbCreateStatement(XU32 dbConnId,XS8* sql, XU32 fid=DEFAULT_FID_INTERFACE)=0;
    
    virtual XS32 DbDestroyStatement(XU32 dbConnId,CDbStatement* pStatment, XU32 fid=DEFAULT_FID_INTERFACE)=0;

	/*�õ���Ҫ���ص�������Ϣ type������ʲô���أ�retStr Ҫ���صĴ���strLen�����ݵ�retStr��󳤶�*/
	virtual XS32 GetString(XU32 type, XS8* retStr, XU32 strLen)=0;

};
