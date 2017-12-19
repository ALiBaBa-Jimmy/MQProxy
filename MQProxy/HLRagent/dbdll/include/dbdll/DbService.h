#pragma once

#include "DbStatement.h"

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
	XS32 mod;
}SDllEnv;

class CDbService
{
public:

	/*���ݿ⻷��������ʼ��*/
    virtual XS32 DbInit(XU8* env)=0; 
	/*���ݿ⻷����������*/
	virtual XS32 DbDestroy(XVOID)=0;
	/*�õ���ʵ��EnvMode*/
	virtual XS32 GetDbEnvMode(XS32 dbMode)=0;
	//parmeter : �����ַ����ĸ�ʽ��Ҫ���壻
	// mysql ���ݿ� XS8 *connstring��ʾip,XS8*parmeter=���ݿ���
	virtual XS32 DbInitConnection(XS8 *user, XS8 *pwd, XS8 *connstring,XS8*parmeter=NULL,XBOOL isDefault=XTRUE)=0;
    
	virtual XS32 DbDestroyConnection()=0;
	
	/*�����ӳ��У�������õ����ݿ�����*/
	virtual XU32 DbCreateConn(XVOID)=0;

	virtual XU32 DbCreateConn(XS8 *user,XS8 *pwd)=0;
    
	virtual XS32 DBCheckConnection(XU32 dbConnId)=0;

	/*�����ӳ��У��ͷ����ݿ�����*/
	virtual XS32 DbDestroyConn(XU32 dbConnId)=0;

	
	/*�����ύ*/
	virtual XS32 DbCommit(XU32 dbConnId)=0;
	/*����ع�*/
	virtual XS32 DbRollback(XU32 dbConnId)=0;

	
	/*��ѯ���ݿ�������*/
	virtual CDbStatement* DbCreateStatement(XU32 dbConnId,XS8* sql)=0;
    
    virtual XS32 DbDestroyStatement(XU32 dbConnId,CDbStatement* pStatment)=0;

};
