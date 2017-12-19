#include "mysql.h"
#include <iostream>
#include <vector>

#include "OssTagInfo.h"
#include "DbFactory.h"


using namespace std;



int main(void)
{
	string sqlString;
   int length,type;
	string name;
	char Name[20] = {0};
	XS64 nRet = XERROR ;
	int rowaffected = 0;
	XS64 connid;
	string tag;

	int nDbType = DB_TYPE_MYSQL;//;DB_TYPE_ORACLE
	CDbService* pDbService = CDbFactory::getInstance(nDbType)->GetDbService();

	
	if (pDbService == NULL)
	{
		cout<<"CDbService* is NULL！"<<endl;
		return 0;
	}
	
	//初始化环境
	SDllEnv szDllEnv;
	szDllEnv.mod = THREADED_MUTEXED;
	nRet = pDbService->DbInit((XU8*)&szDllEnv);
	if (nRet == XERROR)
	{
		cout<<"DbInit is XERROR！"<<endl;
		return 0;
	}
	//初始化连接池	
	if(DB_TYPE_ORACLE == nDbType)
	{
		nRet = pDbService->DbInitConnection("hlr3000wzj50","hlr3000","ORA10G_172.16.8.119",NULL,XTRUE);
	}
	else if (DB_TYPE_MYSQL == nDbType)
	{
		nRet = pDbService->DbInitConnection("root","xinwei","test","172.16.8.117",XTRUE);
	}
	else
	{
		return 0;
	}
	if (nRet != XSUCC)
	{
		cout<<"DbInitConnection is XERROR！"<<endl;
		return 0;
	}
	cout<<"成功连接数据库！"<<endl<<"请选择要进行的操作(1显示所有数据；2插入一条数据；3删除一条数据；4更新一条数据；6退出)"<<endl;

	int dmCode;
	cin>>dmCode;

	CDbStatement *pDbStatement;
	CDbResult *pDbResult;

	switch (dmCode)
	{
	case 1:
		{	
	//初始化连接，获得一个connid连接号
	//connid = pDbService->DbCreateConn("hlr3000wzj50","hlr3000");
	connid = pDbService->DbCreateConn();
	
	//查询oss_tag_info表中所有的信息，并输出到vector中//where tag = :v1
	pDbStatement = pDbService->DbCreateStatement(connid,"select tag,length,type,name from oss_tag_info ");// where tag =?
	//pDbStatement = pDbService->DbCreateStatement(connid,"select tag ,length from oss_tag_info ");// where tag =?
	//pDbStatement = pDbService->DbCreateStatement(connid,"select type,tag,length,name from oss_tag_info ");// where tag =?
	//pDbStatement = pDbService->DbCreateStatement(connid,"select operid,opertype,packtype from oss_oper_msg_def");// where tag =?
	if (pDbStatement == NULL)
	{
		cout<<"create statement is NULL,error"<<endl;
		return XERROR;
	}

	
	//pDbStatement->SetDbString(1,"2001");
	//pDbStatement->SetBindParmCount(1);
	//执行查询
	nRet = pDbStatement->ExecuteQuery(&pDbResult);
	if(nRet != XSUCC)
	{
		cout<<"create resultset is NULL,error,nRet is "<<nRet<<endl;
		return nRet;
	}
  
	while(pDbResult->IsHaveNextRow())
	{
		tag = pDbResult->GetDbString(1);
		length = pDbResult->GetDbInt(2);
		
		type = pDbResult->GetDbInt(3);
		
		//name = pDbResult->GetDbString(4);
		XS8 namec[100] ={0};
		pDbResult->GetDbChar(4, (XS8*)namec);
		
		cout<<"tag = "<<tag <<endl;
		cout<<"Length = "<<length <<endl;
		cout<<"Type = "<<type  <<endl;
		cout<<"Name1 = "<<namec<<endl;
	}
	//终止该statement语句
	pDbService->DbDestroyStatement(connid,pDbStatement);
	pDbStatement = NULL;
	pDbResult = NULL;
	//释放连接池中的该连接
	pDbService->DbDestroyConn(connid);	
		}
		break;
	case 2:
		{			
			//初始化连接，获得一个connid连接号
			connid = pDbService->DbCreateConn();
			cout<<"connid："<<connid<<endl;
			cout<<"插入数据："<<endl;

			//查询oss_tag_info表中所有的信息，并输出到vector中
			if(DB_TYPE_ORACLE == nDbType)
			{
				pDbStatement = pDbService->DbCreateStatement(connid,"insert into oss_tag_info(tag,length,type,name) values(:v1,:v2,:v3,:v4)");
		
			}
			else if (DB_TYPE_MYSQL == nDbType)
			{
				pDbStatement = pDbService->DbCreateStatement(connid,"insert into oss_tag_info(tag,length,type,name) values(?,?,?,?)");
			}
			//pDbStatement = pDbService->DbCreateStatement(connid,"insert into oss_tag_info(tag,length,type,name) values(?,?, 2, '123')");
			if (pDbStatement == NULL)
			{
				cout<<"create statement is NULL,error"<<endl;
				return XERROR;
			}
			pDbStatement->SetDbChar(1,"24");
			
			
			pDbStatement->SetDbInt(2,34);
			
			pDbStatement->SetDbInt(3,34);
			pDbStatement->SetDbChar(4,"lixiatestlixiatestlixiatestlixiatestlixiatest");

			
			nRet = pDbStatement->ExecuteUpdate();
			if (nRet != XSUCC)
			{
				pDbService->DbRollback(connid);
				return nRet;
			}

			//终止该statement语句
			pDbService->DbDestroyStatement(connid,pDbStatement);
			pDbStatement = NULL;
			pDbResult = NULL;

			//commit该操作
			pDbService->DbCommit(connid);
			
			//释放连接池中的该连接
			pDbService->DbDestroyConn(connid);
		}
		break;
	case 3:
		//初始化连接，获得一个connid连接号
		connid = pDbService->DbCreateConn("hlr3000wzj50","hlr3000");
		cout<<"connid："<<connid<<endl;
		cout<<"删除数据："<<endl;

	
		//查询oss_tag_info表中所有的信息，并输出到vector中
		if(DB_TYPE_ORACLE == nDbType)
		{
				pDbStatement = pDbService->DbCreateStatement(connid,"delete from oss_tag_info where tag = :v1 and length = :v2");
		
		}
		else if (DB_TYPE_MYSQL == nDbType)
		{
				pDbStatement = pDbService->DbCreateStatement(connid,"delete from oss_tag_info where tag = ? and length = ?");
		}	
		if (pDbStatement == NULL)
		{
			cout<<"create statement is NULL,error"<<endl;
			return XERROR;
		}
		pDbStatement->SetDbString(1,"22");
	
		pDbStatement->SetDbInt(2,34);

		nRet = pDbStatement->ExecuteUpdate();
		if (nRet != XSUCC)
		{
			pDbService->DbRollback(connid);
			return nRet;
		}
		//终止该statement语句
		pDbService->DbDestroyStatement(connid,pDbStatement);
		pDbStatement = NULL;
		pDbResult = NULL;

		pDbService->DbCommit(connid);
		
		//释放连接池中的该连接
		pDbService->DbDestroyConn(connid);
	    break;
	case 4:
		//初始化连接，获得一个connid连接号
		connid = pDbService->DbCreateConn("hlr3000wzj50","hlr3000");

		cout<<"connid："<<connid<<endl;
		cout<<"更新数据："<<endl;
		
		//传入sql语句到CDbProcess中

		//查询oss_tag_info表中所有的信息，并输出到vector中
		if(DB_TYPE_ORACLE == nDbType)
		{
			pDbStatement = pDbService->DbCreateStatement(connid,"update oss_tag_info set length=:v1,type=:v2,name=:v3 where tag = :v4");
	
		
		}
		else if (DB_TYPE_MYSQL == nDbType)
		{
				pDbStatement = pDbService->DbCreateStatement(connid,"update oss_tag_info set length=?,type=?,name=? where tag = ?");
	
		}	
		if (pDbStatement == NULL)
		{
			cout<<"create statement is NULL,error"<<endl;
			return XERROR;
		}
		//pDbStatement->SetDbInt(1,43);
		//pDbStatement->SetDbInt(2,43);
		pDbStatement->SetDbString(3,"lixiatest43");
		pDbStatement->SetDbString(4,"2014");

		nRet = pDbStatement->ExecuteUpdate();
		if (nRet != XSUCC)
		{
			pDbService->DbRollback(connid);
			return nRet;
		}
		//终止该statement语句
		pDbService->DbDestroyStatement(connid,pDbStatement);
		pDbStatement = NULL;
		pDbResult = NULL;

		pDbService->DbCommit(connid);
		
		//释放连接池中的该连接
		pDbService->DbDestroyConn(connid);
		break;
		
	default:
		break;
	}

	
	//断开连接池
	pDbService->DbDestroyConnection("hlr3000wzj50");
	//数据库环境参数销毁	
	pDbService->DbDestroy();
	
	cout<<"success"<<endl;
	//一定要执行DestroyDbService,与CDbFactory::getInstance()->GetDbService()配对使用
	CDbFactory::getInstance()->DestroyDbService(pDbService);
	pDbService = NULL;
	
	return 0;
}
