#pragma once
#include "mysql.h"
#include <iostream>
#include <vector>
#include "DbFactoryMysql.h"

using namespace std;

typedef struct
{
	XS8 caller_number[21];
	XS8 called_number[21];
	XS8 msg_content_len;
	XS8 msg_content[141];
	int msg_ref;
	int msg_id;
	int msg_validity;
	XS8 msg_priority;
	XS8 msg_origin;
	int msg_come_time;
	XS8 msg_report_flag;
	XS8 msg_protocol_id;
	XS8 msg_data_coding;
	XS8 msg_message_type;
	XS8 msg_headind;
	XS8 msg_7bit_bytes;

}NOSEND_INFO_MSG;


int main(void)
{
	XS32 nRet = XERROR ;
	XS32 connid;
	int dmCode;
	char blobstatic[12] = {0x12, 0x00, 0x34, 0x00, 0x56, 0x00, 0x78, 0x00, 0x90, 0x00, 0x13, 0x67};
	CDbStatement *pDbStatement;
	CDbResult *pDbResult;
	NOSEND_INFO_MSG  new_msg ={0};


	CDbService* pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
	if (pDbService == NULL)
	{
		cout<<"CDbService* is NULL��"<<endl;
		return 0;
	}


	//��ʼ�����ӳ�	
	nRet = pDbService->DbInitConnection("root","xinwei","smcdb","127.16.8.72",XTRUE);
	if (nRet != XSUCC)
	{
		cout<<"DbInitConnection is XERROR��"<<endl;
		return 0;
	}


	//��ʼ�����ӣ����һ��connid���Ӻ�
	connid = pDbService->DbCreateConn();
	cout<<"connid��"<<connid<<endl;
	cout<<"�ɹ��������ݿ⣡"<<endl<<"��ѡ��Ҫ���еĲ���(1��ʾ�������ݣ�2����һ�����ݣ�3ɾ��һ�����ݣ�4����һ�����ݣ�6�˳�)"<<endl;


	cin>>dmCode;
	switch (dmCode)
	{
	case 1:
		cout<<"��ѯ���ݣ�"<<endl;


		pDbStatement = pDbService->DbCreateStatement(connid,"select * from smc_no_send_info_00");
		if (pDbStatement == NULL)
		{
			cout<<"create statement is NULL,error"<<endl;
			return XERROR;
		}

		//ִ�в�ѯ
		nRet = pDbStatement->ExecuteQuery(&pDbResult);
		if(nRet != XSUCC)
		{
			cout<<"create resultset is NULL,error,nRet is "<<nRet<<endl;
			return nRet;
		}

		while(pDbResult->IsHaveNextRow())
		{

			pDbResult->GetDbChar(1, new_msg.caller_number);
			pDbResult->GetDbChar(2, new_msg.called_number );
			new_msg.msg_content_len = pDbResult->GetDbTiny(3);
			pDbResult->GetDbBlob(4, new_msg.msg_content);
			new_msg.msg_ref = pDbResult->GetDbInt(5);
			new_msg.msg_id = pDbResult->GetDbInt(6);
			new_msg.msg_validity = pDbResult->GetDbInt(7);
			new_msg.msg_priority = pDbResult->GetDbTiny(8);
			new_msg.msg_origin = pDbResult->GetDbTiny(9);
			new_msg.msg_come_time = pDbResult->GetDbInt(10);

			new_msg.msg_report_flag = pDbResult->GetDbTiny(11);
			new_msg.msg_protocol_id = pDbResult->GetDbTiny(12);
			new_msg.msg_data_coding = pDbResult->GetDbTiny(13);
			new_msg.msg_message_type = pDbResult->GetDbTiny(14);
			new_msg.msg_headind = pDbResult->GetDbTiny(15);
			new_msg.msg_7bit_bytes = pDbResult->GetDbTiny(16);

		}

		//��ֹ��statement���
		pDbService->DbDestroyStatement(connid,pDbStatement);
		pDbStatement = NULL;
		pDbResult = NULL;
		break;
	case 2:		
		cout<<"�������ݣ�"<<endl;

		//����һ�����ݵ�smnosendtable����
		pDbStatement = pDbService->DbCreateStatement(connid,"insert into smc_no_send_info_00 (caller_number,called_number,msg_content_len,\
		msg_content,msg_ref,msg_id,msg_validity,msg_priority,msg_origin,msg_come_time,msg_report_flag,msg_protocol_id,msg_data_coding,msg_message_type,\
		msg_headind,msg_7bit_bytes) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
		if (pDbStatement == NULL)
		{
			cout<<"create statement is NULL,error"<<endl;
			return XERROR;
		}

		pDbStatement->SetDbChar(1,"15656456");
		pDbStatement->SetDbChar(2,"4648913");
		pDbStatement->SetDbTiny(3,3);
		pDbStatement->SetDbBlob(21,12,blobstatic);
		pDbStatement->SetDbInt(5,434523);
		pDbStatement->SetDbInt(6,4378);
		pDbStatement->SetDbInt(7,4593);
		pDbStatement->SetDbTiny(8,6);
		pDbStatement->SetDbTiny(9,7);
		pDbStatement->SetDbInt(10,434);
		pDbStatement->SetDbTiny(11,1);
		pDbStatement->SetDbTiny(12,2);
		pDbStatement->SetDbTiny(13,3);
		pDbStatement->SetDbTiny(14,4);
		pDbStatement->SetDbTiny(15,5);
		pDbStatement->SetDbTiny(16,6);



		nRet = pDbStatement->ExecuteUpdate();
		if (nRet != XSUCC)
		{
			pDbService->DbRollback(connid);
			return nRet;
		}

		//��ֹ��statement���
		pDbService->DbDestroyStatement(connid,pDbStatement);
		pDbStatement = NULL;
		pDbResult = NULL;

		//commit�ò���
		pDbService->DbCommit(connid);
		break;
	case 3:
		cout<<"ɾ�����ݣ�"<<endl;


		pDbStatement = pDbService->DbCreateStatement(connid,"delete from smc_no_send_info_00 where msg_id = ?");	
		if (pDbStatement == NULL)
		{
			cout<<"create statement is NULL,error"<<endl;
			return XERROR;
		}

		pDbStatement->SetDbInt(1,4378);

		nRet = pDbStatement->ExecuteUpdate();
		if (nRet != XSUCC)
		{
			pDbService->DbRollback(connid);
			return nRet;
		}
		//��ֹ��statement���
		pDbService->DbDestroyStatement(connid,pDbStatement);
		pDbStatement = NULL;
		pDbResult = NULL;

		pDbService->DbCommit(connid);
		break;
	case 4:
		cout<<"�������ݣ�"<<endl;


		pDbStatement = pDbService->DbCreateStatement(connid,"update smc_no_send_info_00 set msg_ref=? where msg_id=?");
		if (pDbStatement == NULL)
		{
			cout<<"create statement is NULL,error"<<endl;
			return XERROR;
		}

		pDbStatement->SetDbInt(1,516);
		pDbStatement->SetDbInt(2,4378);

		nRet = pDbStatement->ExecuteUpdate();
		if (nRet != XSUCC)
		{
			pDbService->DbRollback(connid);
			return nRet;
		}
		//��ֹ��statement���
		pDbService->DbDestroyStatement(connid,pDbStatement);
		pDbStatement = NULL;
		pDbResult = NULL;

		pDbService->DbCommit(connid);
		break;
	default:
		break;
	}

	
	//�ͷ����ӳ��еĸ�����
	pDbService->DbDestroyConn(connid);
	//�Ͽ����ӳ�
	pDbService->DbDestroyConnection();

	cout<<"success"<<endl;
	//һ��Ҫִ��DestroyDbService,��CDbFactory::getInstance()->GetDbService()���ʹ��
	CDbFactoryMysql::GetInstance()->DestroyDbService(pDbService);
	pDbService = NULL;

	return 0;
}
