#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef enum  
{
	ERR_CODE_BEGIN = 0,
	ERR_CODE_PVIINFO_NOT_EXSIT=3001, //IMSI不存在-------------业务错误
	ERR_CODE_PVI_APN_INFO_NOT_EXIST,//IMSI没有APN------------业务错误
	ERR_CODE_PVI_EPS_INFO_NOT_EXIST,//IMSI没有EPS签约数据---没有QOS_CLASS_IDENTIFIER字段-----业务错误
	ERR_CODE_DB_EXCEPTION,			//数据库异常(ERROR_CODE_DB_BEGIN~ERROR_CODE_DB_END)
	ERR_CODE_PUI_DATA_NOT_EXIST,	//PUI数据不存在
	ERR_CODE_PVI_DATA_NOT_EXIST,	//PVI数据不存在
	ERR_CODE_PUI_SERVICE_INDICATION_DATA_NOT_EXIST,	//sh接口中透明表中,pui没有保存service-indication所标示的数据
	ERR_CODE_PUI_PVI_TYPE_NOT_EXIST,	//PUI不支持这种终端类型
	ERR_CODE_AMF_NOT_EXIST,			//不存在AMF
	ERR_CODE_OP_NOT_EXIST,			//不存在OP
	ERR_CODE_IFC_NOT_EXIST,			//不存在IFC
	ERR_CODE_AUTHINFO_NOT_EXIST,	//不存在鉴权信息
	ERR_CODE_END,

}E_ERROR_CODE;

typedef enum
{
    DB_MODULE_PATAMETER_ISNULL, /* 参数为空 */
    DB_MODULE_CREATE_STATEMEN_ERROR, /* 创建数据库语句出错 */
    DB_MODULE_EXECUTE_ERROR, /* 数据库执行出错 */
    DB_MODULE_QUERY_NO_DATA,
}E_DB_MODULE_ERROR_CODE;





#pragma pack()

#ifdef __cplusplus
}
#endif 

