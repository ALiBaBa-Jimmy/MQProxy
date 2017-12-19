#ifndef _XW_ERRORCODE_H
#define _XW_ERRORCODE_H

//�������ݿ���ڴ����ݿ�Ĵ����붨��ͷ�ļ�
#ifndef XSUCC
	#define XSUCC 				(0)
	#define XERROR 				(-1)
	#define XNOFOUNDDATA		(2) //���º�ɾ������ʱ�������ݲ����ڷ��ظô�����(�Զ���)
	#define XKEYREPEAT           (1) //��������ʱ�����������ݵ�����ֵ��ͻ�򷵻ظô�����(getErrorCode()���صĴ�����)
#endif


// �ڴ����ݿ���� 1100-1199
// TCP ����       1200-1299
// OSS �������� 1300-1399
// DB ��������  1400-1599



//�ڴ����ݿ������
typedef enum
{	
	MEM_DB_SUCCESS=0,	
	MEM_DB_ERR_BEGIN = 1100,
	MEM_DB_ERR_NO_DATA_EFFECT, //=3,
	MEM_DB_ERR_OPEN,// = 4 //DB�򿪴���(Ϊ�����ORACLE���ݿ�Ĵ������ͻ����������ֵ)
	MEM_DB_ERR_END
}E_MEM_DB_ERR_CODE;

//�����ڴ����ݿ�ķ���ģʽ
enum accessType 
{	
	ReadOnly=0,
	AllAccess,
	ConcurrentRead,
	ConcurrentUpdate
};
#endif

