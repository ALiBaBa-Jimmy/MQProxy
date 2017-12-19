#pragma once
#include "errorcode.h"

typedef enum
{
	ERROR_CODE_DB_SUCCESS=0,
	ERROR_CODE_DB_BEGIN = 1400,
	ERROR_CODE_DB_NULL_PTR,						// ��ָ�����
	ERROR_CODE_DB_CONN_FAIL,					// ���ݿ����Ӵ���
	ERROR_CODE_DB_CONN_POOL_IS_FULL,	// ���ӳ���
	/*ERROR_CODE_DB_DECODE_ERROR,				//3 �������
	ERROR_CODE_DB_TRANSFER_PROCESS_NULLPTR,	//4 �����û�г�ʼ��������
	ERROR_CODE_DB_TIME_OUT,	//��ʱ	*/
	ERROR_CODE_DB_END,
	ERROR_CODE_DB_NO_DATA_EFFECT,        //��ѯ���Ϊ��
	ERROR_CODE_DB_EXECUTE_QUERY,			 //���ݿ�ִ�г���
    ERROR_CODE_DB_STMENT_ERROR
}DB_ERROR_CODE_DEF;
