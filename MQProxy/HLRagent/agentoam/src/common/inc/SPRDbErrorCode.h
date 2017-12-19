#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef enum  
{
	ERR_CODE_BEGIN = 0,
	ERR_CODE_PVIINFO_NOT_EXSIT=3001, //IMSI������-------------ҵ�����
	ERR_CODE_PVI_APN_INFO_NOT_EXIST,//IMSIû��APN------------ҵ�����
	ERR_CODE_PVI_EPS_INFO_NOT_EXIST,//IMSIû��EPSǩԼ����---û��QOS_CLASS_IDENTIFIER�ֶ�-----ҵ�����
	ERR_CODE_DB_EXCEPTION,			//���ݿ��쳣(ERROR_CODE_DB_BEGIN~ERROR_CODE_DB_END)
	ERR_CODE_PUI_DATA_NOT_EXIST,	//PUI���ݲ�����
	ERR_CODE_PVI_DATA_NOT_EXIST,	//PVI���ݲ�����
	ERR_CODE_PUI_SERVICE_INDICATION_DATA_NOT_EXIST,	//sh�ӿ���͸������,puiû�б���service-indication����ʾ������
	ERR_CODE_PUI_PVI_TYPE_NOT_EXIST,	//PUI��֧�������ն�����
	ERR_CODE_AMF_NOT_EXIST,			//������AMF
	ERR_CODE_OP_NOT_EXIST,			//������OP
	ERR_CODE_IFC_NOT_EXIST,			//������IFC
	ERR_CODE_AUTHINFO_NOT_EXIST,	//�����ڼ�Ȩ��Ϣ
	ERR_CODE_END,

}E_ERROR_CODE;

typedef enum
{
    DB_MODULE_PATAMETER_ISNULL, /* ����Ϊ�� */
    DB_MODULE_CREATE_STATEMEN_ERROR, /* �������ݿ������� */
    DB_MODULE_EXECUTE_ERROR, /* ���ݿ�ִ�г��� */
    DB_MODULE_QUERY_NO_DATA,
}E_DB_MODULE_ERROR_CODE;





#pragma pack()

#ifdef __cplusplus
}
#endif 

