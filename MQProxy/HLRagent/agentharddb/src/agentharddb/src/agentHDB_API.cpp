#include "agentHDB_API.h"
#include "fid_def.h"
#include "DbFactory.h"
#include "SPRTraceStruct.h"
#include "DbErrorCode.h"

SubLoadCallBack   g_LoadSubInfo = NULL;
SubLoadUtBindCallBack   g_LoadUtBindInfoCallback = NULL;
extern callbackTraceFun g_callbackTraceFunOfHssHDb;

extern CDbService *g_PDbService;

/***********************************
功能描述：把ASCII字符转为十六进制
示例：
     输入 0x61
	 输出 0x0a
************************************/
XU8 DB_ChrToHex(XU8 chr)
{
	XU8 temp1[17]="0123456789abcdef";
	XU8 temp2[7]="ABCDEF";
	XU8 i;

	for (i=0; i< 16;i++)
	{
		if (chr == temp1[i])
			return i;
	}
	for (i=0; i< 6;i++)
	{
		if (chr == temp2[i])
			return 10+i;
	}

	return -1;
}

/************************************************************************
   功能描述：把 十六进制 转为 字符串
示例：
    输入：0x61，0x62，0x63，0x64
	输出：ab，cd
                                                                  
/************************************************************************/

XU64 DB_HexToStr(XU8 *pHex,XU8 * pStr,XU64 ulLen)
{
	XU64 i;

	for (i=0; i< ulLen;i++)
	{
 		pStr[i] = DB_ChrToHex(pHex[2*i]) * 16 + DB_ChrToHex(pHex[2*i+1]);
	}

	return XSUCC;
}
/************************************************************************
   功能描述：把 字符串 转为 十六进制
示例：
    输入：ab，cd
	输出：0x61，0x62，0x63，0x64
                                                                  
/************************************************************************/
XU64 DB_StrToHex(XU8 * pStr,XU8 *pHex,XU64 ulLen)
{
	XU64 i;

	for (i=0; i< ulLen;i++)
	{
		pHex[2*i]   = ( pStr[i] / 16 > 9) ?(55 + pStr[i] / 16 ):(48 + pStr[i] / 16 );
		pHex[2*i+1] = ( pStr[i] % 16 > 9) ?(55 + pStr[i] % 16 ):(48 + pStr[i] % 16 );
	}

	return XSUCC;
}



XS32 agentHDB_Update(SDbReqSql *req)
{
    XS32     nRet = XSUCC;

    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
	XU32 fid = FID_AGENTDB;


    sprintf(sql,"%s ",  req->conditionSql);
    g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
    
	XU32 connid = g_PDbService->DbCreateConn();

    pDbStatement = g_PDbService->DbCreateStatement(connid,sql);
    if ( NULL == pDbStatement )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"configure statement is NULL,error!");
        nRet = ERROR_CODE_DB_STMENT_ERROR;
    }
    else
    {

		nRet = pDbStatement->ExecuteUpdate();
		if (nRet != XSUCC)
		{
			g_PDbService->DbRollback(connid);
			nRet = ERROR_CODE_DB_EXECUTE_QUERY;
		}
        else 
        {
    		g_PDbService->DbCommit(connid);
        }
		
    } 
	//终止该statement语句
	g_PDbService->DbDestroyStatement(connid,pDbStatement);
	pDbStatement = NULL;
	//释放连接池中的该连接
	g_PDbService->DbDestroyConn(connid);

    return nRet; 
}






XS32 agentHDB_QryLocalNetWorkingbyUid(SDbReqSql *req, XU8 *pNetworkId)
{
    XS32     nRet = XSUCC;
    XS32     connid = 0;
    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
	XU32    fid = FID_AGENTDB;
    if( NULL == req )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"parameters is null!!");
        nRet = ERROR_CODE_DB_NULL_PTR;
    }
    else
    {
        sprintf(sql,"%s ", req->conditionSql);
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
		XU32 connid = g_PDbService->DbCreateConn();

        pDbStatement = g_PDbService->DbCreateStatement(connid,sql);
        if ( NULL == pDbStatement )
        {
            g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"Exec sql failed: %s", sql);
            nRet = ERROR_CODE_DB_STMENT_ERROR;
        }
        else
        {
            //执行查询
            nRet = pDbStatement->ExecuteQuery(&pDbResult);
            if(nRet != XSUCC)
            {
                g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"create result set is NULL,error,nRet is ");
                nRet = ERROR_CODE_DB_EXECUTE_QUERY;
            }
            else
            {
                while(pDbResult->IsHaveNextRow())
                {
                    pDbResult->GetDbChar(1, (XS8 *)pNetworkId);
                    Count++;
                                        
                    
                }
                if( 0 == Count )
                {
                    nRet = ERROR_CODE_DB_NO_DATA_EFFECT;
                }
            }

            //终止该statement语句
            g_PDbService->DbDestroyStatement(connid,pDbStatement);
            pDbStatement = NULL;
            pDbResult = NULL;
			g_PDbService->DbDestroyConn(connid);
        } 
    }

    return nRet; 
}



XS32 agentHDB_QryuidbyAlias(SDbReqSql *req, XU8 *uid)
{
    XS32     nRet = XSUCC;
    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU8 uidStr[9]={0};
    XU16         Count = 0;
	XU32    fid = FID_AGENTDB;
    if( NULL == req )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"parameters is null!!");
        nRet = ERROR_CODE_DB_NULL_PTR;
    }
    else
    {
        sprintf(sql,"%s ", req->conditionSql);
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
		XU32 connid = g_PDbService->DbCreateConn();

        pDbStatement = g_PDbService->DbCreateStatement(connid,sql);
        if ( NULL == pDbStatement )
        {
            g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"Exec sql failed: %s", sql);
            nRet = ERROR_CODE_DB_STMENT_ERROR;
        }
        else
        {
            //执行查询
            nRet = pDbStatement->ExecuteQuery(&pDbResult);
            if(nRet != XSUCC)
            {
                g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"create result set is NULL,error,nRet is ");
                nRet = ERROR_CODE_DB_EXECUTE_QUERY;
            }
            else
            {
                while(pDbResult->IsHaveNextRow())
                {
                    pDbResult->GetDbChar(1, (XS8 *)uidStr);
                    DB_HexToStr(uidStr, uid, LENGTH_OF_TEL);
                    Count++;
                                        
                    
                }
                if( 0 == Count )
                {
                    nRet = ERROR_CODE_DB_NO_DATA_EFFECT;
                }
            }

            //终止该statement语句
            g_PDbService->DbDestroyStatement(connid,pDbStatement);
            pDbStatement = NULL;
            pDbResult = NULL;
			g_PDbService->DbDestroyConn(connid);
        } 
    }

    return nRet; 
}


XS32 agentHDB_QryuidbyTel(SDbReqSql *req, XU8 *uid)
{
    XS32     nRet = XSUCC;
    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU8 uidStr[9]={0};
    XU16         Count = 0;
	XU32    fid = FID_AGENTDB;
    if( NULL == req )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"parameters is null!!");
        nRet = ERROR_CODE_DB_NULL_PTR;
    }
    else
    {
        sprintf(sql,"%s ", req->conditionSql);
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
		XU32 connid = g_PDbService->DbCreateConn();

        pDbStatement = g_PDbService->DbCreateStatement(connid,sql);
        if ( NULL == pDbStatement )
        {
            g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"Exec sql failed: %s", sql);
            nRet = ERROR_CODE_DB_STMENT_ERROR;
        }
        else
        {
            //执行查询
            nRet = pDbStatement->ExecuteQuery(&pDbResult);
            if(nRet != XSUCC)
            {
                g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"create result set is NULL,error,nRet is ");
                nRet = ERROR_CODE_DB_EXECUTE_QUERY;
            }
            else
            {
                while(pDbResult->IsHaveNextRow())
                {
                    pDbResult->GetDbChar(1, (XS8 *)uidStr);
                    DB_HexToStr(uidStr, uid, LENGTH_OF_TEL);
                    Count++;
                                        
                    
                }
                if( 0 == Count )
                {
                    nRet = ERROR_CODE_DB_NO_DATA_EFFECT;
                }
            }

            //终止该statement语句
            g_PDbService->DbDestroyStatement(connid,pDbStatement);
            pDbStatement = NULL;
            pDbResult = NULL;
			g_PDbService->DbDestroyConn(connid);
        } 
    }

    return nRet; 
}



XS32 agentHDB_QryCamTalkInfo(SDbReqSql *req, TXdbHdbDyn *pResult)
{
    XS32     nRet = XSUCC;
    XS32     connid = 0;
    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
    XU8 Cssidstr[9] = {0};
    XU8 TmpStr[64] = {0};
	XU32    fid = FID_AGENTDB;
    if( NULL == req )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"parameters is null!!");
        nRet = ERROR_CODE_DB_NULL_PTR;
    }
    else
    {
        sprintf(sql,"%s ", req->conditionSql);
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
		XU32 connid = g_PDbService->DbCreateConn();

        pDbStatement = g_PDbService->DbCreateStatement(connid,sql);
        if ( NULL == pDbStatement )
        {
            g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"configure statement is NULL,error!");
            nRet = ERROR_CODE_DB_STMENT_ERROR;
        }
        else
        {
            //执行查询
            nRet = pDbStatement->ExecuteQuery(&pDbResult);
            if(nRet != XSUCC)
            {
                g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"create result set is NULL,error,nRet is ");
                nRet = ERROR_CODE_DB_EXECUTE_QUERY;
            }
            else
            {
                while(pDbResult->IsHaveNextRow())
                {
                    pDbResult->GetDbChar(1, (XS8 *)TmpStr);
                    DB_HexToStr(TmpStr, pResult->camtalkDynInfo.camtalkTelno, LENGTH_OF_TEL);
                    pResult->camtalkDynInfo.iCssPublicIp = pDbResult->GetDbUInt(2);
                    pResult->camtalkDynInfo.iCssPrivateIp = pDbResult->GetDbUInt(3);
                    pDbResult->GetDbChar(4, (XS8 *)TmpStr);
                    DB_HexToStr(TmpStr, pResult->camtalkDynInfo.cssId, LENGTH_OF_CSSID);

                    pResult->camtalkDynInfo.iXWIpV4 = pDbResult->GetDbUInt(5);
                    pResult->camtalkDynInfo.sXWIpPort = pDbResult->GetDbUInt(6);
                    pResult->camtalkDynInfo.cCamtalkStatus = pDbResult->GetDbUInt(7);
                    pDbResult->GetDbChar(8, (XS8 *)pResult->camtalkDynInfo.privRegDate);
                    pDbResult->GetDbChar(9, (XS8 *)pResult->camtalkDynInfo.curRegDate);
                    Count++;
                                        
                    
                }
                if( 0 == Count )
                {
                    nRet = ERROR_CODE_DB_NO_DATA_EFFECT;
                }
            }

            //终止该statement语句
            g_PDbService->DbDestroyStatement(connid,pDbStatement);
            pDbStatement = NULL;
            pDbResult = NULL;
			g_PDbService->DbDestroyConn(connid);
        } 
    }

    return nRet; 
}






XS32 agentHDB_QryNationCodebyTelNo(SDbReqSql *req, XU8 *pNation)
{
    XS32     nRet = XSUCC;
    XS32     connid = 0;
    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XU8 uidstr[9] = {0};
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
	XU32    fid = FID_AGENTDB;
    if( NULL == req )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"parameters is null!!");
        nRet = ERROR_CODE_DB_NULL_PTR;
    }
    else
    {
        sprintf(sql,"%s ", req->conditionSql);
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
		XU32 connid = g_PDbService->DbCreateConn();

        pDbStatement = g_PDbService->DbCreateStatement(connid,sql);
        if ( NULL == pDbStatement )
        {
            g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"configure statement is NULL,error!");
            nRet = ERROR_CODE_DB_STMENT_ERROR;
        }
        else
        {
            //执行查询
            nRet = pDbStatement->ExecuteQuery(&pDbResult);
            if(nRet != XSUCC)
            {
                g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"create result set is NULL,error,nRet is ");
                nRet = ERROR_CODE_DB_EXECUTE_QUERY;
            }
            else
            {
                while(pDbResult->IsHaveNextRow())
                {
                    pDbResult->GetDbChar(1, (XS8 *)pNation);
                    //DB_HexToStr(uidstr, pNation, 4);
                    Count++;
                                        
                    
                }
                if( 0 == Count )
                {
                    nRet = ERROR_CODE_DB_NO_DATA_EFFECT;
                }
            }

            //终止该statement语句
            g_PDbService->DbDestroyStatement(connid,pDbStatement);
            pDbStatement = NULL;
            pDbResult = NULL;
			g_PDbService->DbDestroyConn(connid);
        } 
    }

    return nRet; 
}

XS32 agentHDB_SubOper(SDbReqSql *req)
{
    XS32     nRet = XSUCC;

    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
	XU32 fid = FID_AGENTDB;


    sprintf(sql,"%s ", req->conditionSql);
    g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", req->conditionSql);
    
	XU32 connid = g_PDbService->DbCreateConn();

    pDbStatement = g_PDbService->DbCreateStatement(connid,sql);
    if ( NULL == pDbStatement )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"configure statement is NULL,error!");
        nRet = ERROR_CODE_DB_STMENT_ERROR;
    }
    else
    {
		nRet = pDbStatement->ExecuteUpdate();
		if (nRet != XSUCC)
		{
			g_PDbService->DbRollback(connid);
			nRet = ERROR_CODE_DB_EXECUTE_QUERY;
		}
        else 
        {
    		g_PDbService->DbCommit(connid);
        }
		
    } 
	//终止该statement语句
	g_PDbService->DbDestroyStatement(connid,pDbStatement);
	pDbStatement = NULL;
	//释放连接池中的该连接
	g_PDbService->DbDestroyConn(connid);

    return nRet; 
}




XS32 agentHDB_SubQryOper(SDbReqSql *req)
{
    XS32     nRet = XSUCC;
    XU32 type;
    XU8 topic[32] = {0};
    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
	XU32 fid = FID_AGENTDB;


    sprintf(sql,"%s ", req->conditionSql);
    g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", req->conditionSql);
    
	XU32 connid = g_PDbService->DbCreateConn();

    pDbStatement = g_PDbService->DbCreateStatement(connid,sql);
    if ( NULL == pDbStatement )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"configure statement is NULL,error!");
        nRet = ERROR_CODE_DB_STMENT_ERROR;
    }
    else
    {
            //执行查询
            nRet = pDbStatement->ExecuteQuery(&pDbResult);
            if(nRet != XSUCC)
            {
                g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"create result set is NULL,error,nRet is ");
                nRet = ERROR_CODE_DB_EXECUTE_QUERY;
            }
            else
            {
                while(pDbResult->IsHaveNextRow())
                {
                    type = pDbResult->GetDbUInt(1);
                    pDbResult->GetDbChar(2, (XS8 *)topic);
                    if(NULL != g_LoadSubInfo)
                    {
                        g_LoadSubInfo(type, (XCONST XCHAR*)topic, XTRUE);
                    }
                    Count++;
                }
                if( 0 == Count )
                {
                    nRet = ERROR_CODE_DB_NO_DATA_EFFECT;
                }
            }

		
    } 
	//终止该statement语句
	g_PDbService->DbDestroyStatement(connid,pDbStatement);
	pDbStatement = NULL;
	//释放连接池中的该连接
	g_PDbService->DbDestroyConn(connid);

    return nRet; 
}




XS32 agentHDB_QryDynInfobyUid(SDbReqSql *req, TXdbHdbDyn *pResult)
{
    XS32     nRet = XSUCC;
    XS32     connid = 0;
    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
    XU8 vssidstr[9] = {0};
    XU8 tmpstr[32] = {0};
    XU8 HsStatusstr[3] = {0};
	XU32    fid = FID_AGENTDB;
    if( NULL == req )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"parameters is null!!");
        nRet = ERROR_CODE_DB_NULL_PTR;
    }
    else
    {
        sprintf(sql,"%s ", req->conditionSql);
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
		XU32 connid = g_PDbService->DbCreateConn();

        pDbStatement = g_PDbService->DbCreateStatement(connid,sql);
        if ( NULL == pDbStatement )
        {
            g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"configure statement is NULL,error!");
            nRet = ERROR_CODE_DB_STMENT_ERROR;
        }
        else
        {
            //执行查询
            nRet = pDbStatement->ExecuteQuery(&pDbResult);
            if(nRet != XSUCC)
            {
                g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"create result set is NULL,error,nRet is ");
                nRet = ERROR_CODE_DB_EXECUTE_QUERY;
            }
            else
            {
                while(pDbResult->IsHaveNextRow())
                {
                    pDbResult->GetDbChar(1, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, pResult->pid, LENGTH_OF_PID);

                    pDbResult->GetDbChar(2, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, pResult->bsId, LENGTH_OF_BSID);
                    pDbResult->GetDbChar(3, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, pResult->bscId, LENGTH_OF_BSCID);
                    pDbResult->GetDbChar(4, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, pResult->laiId, LENGTH_OF_LAIID);
                    pDbResult->GetDbChar(5, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, pResult->vssId, LENGTH_OF_VSSID);

                    pDbResult->GetDbChar(6, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, &pResult->hsStatus, 1);
                    pResult->xwipv4 = pDbResult->GetDbUInt(7);
                    pResult->xwipport = pDbResult->GetDbUInt(8);

                    pDbResult->GetDbChar(9, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, pResult->voiceAnchor.bsID, LENGTH_OF_BSID);
                    pDbResult->GetDbChar(10, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, pResult->voiceAnchor.bscID, LENGTH_OF_BSCID);
                    pDbResult->GetDbChar(11, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, pResult->voiceAnchor.laiID, LENGTH_OF_LAIID);
                    pDbResult->GetDbChar(12, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, pResult->voiceAnchor.vssID, LENGTH_OF_VSSID);

                    pDbResult->GetDbChar(13, (XS8 *)tmpstr);
                    DB_HexToStr(tmpstr, &pResult->pidPort, 1);

                    pResult->sagIpv4 = pDbResult->GetDbUInt(14);
                    pDbResult->GetDbChar(15, (XS8 *)pResult->cNetworkID);
                    pDbResult->GetDbChar(16, (XS8 *)pResult->authNet);
                    pResult->sagIp_voice = pDbResult->GetDbUInt(17);
                    pResult->sagIp_data = pDbResult->GetDbUInt(18);
                    
                   Count++;
                                        
                    
                }
                if( 0 == Count )
                {
                    nRet = ERROR_CODE_DB_NO_DATA_EFFECT;
                }
            }

            //终止该statement语句
            g_PDbService->DbDestroyStatement(connid,pDbStatement);
            pDbStatement = NULL;
            pDbResult = NULL;
			g_PDbService->DbDestroyConn(connid);
        } 
    }

    return nRet; 
}

XS32 agentHDB_SubLoadCallback(SubLoadCallBack func)
{

    g_LoadSubInfo = func;

	return XSUCC;
}



XS32 agentHDB_SubLoadUtBindCallback(SubLoadUtBindCallBack func)
{

    g_LoadUtBindInfoCallback = func;

	return XSUCC;
}




XS32 agentHDB_LoadUtInfo(SDbReqSql *req)
{
    XS32     nRet = XSUCC;
    XU8 pidStr[9]={0};
    XU8 uidStr[9]={0};
    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
	XU32 fid = FID_AGENTDB;
    SLeaseHoldCtlTable UtInfo={0};


    sprintf(sql,"%s ", req->conditionSql);
    g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", req->conditionSql);
    
	XU32 connid = g_PDbService->DbCreateConn();

    pDbStatement = g_PDbService->DbCreateStatement(connid,sql);
    if ( NULL == pDbStatement )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"configure statement is NULL,error!");
        nRet = ERROR_CODE_DB_STMENT_ERROR;
    }
    else
    {
            //执行查询
            nRet = pDbStatement->ExecuteQuery(&pDbResult);
            if(nRet != XSUCC)
            {
                g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"create result set is NULL,error,nRet is ");
                nRet = ERROR_CODE_DB_EXECUTE_QUERY;
            }
            else
            {
                while(pDbResult->IsHaveNextRow())
                {
                     
                    pDbResult->GetDbChar(1, (XS8 *)pidStr);
                    DB_HexToStr(pidStr, UtInfo.pid, LENGTH_OF_PID);
                    
                    UtInfo.port = pDbResult->GetDbUInt(2);

                    pDbResult->GetDbChar(3, (XS8 *)uidStr);
                    DB_HexToStr(uidStr, UtInfo.creditUid, LENGTH_OF_UID);
                    
                    pDbResult->GetDbChar(4, (XS8 *)uidStr);
                    DB_HexToStr(uidStr, UtInfo.bindUid, LENGTH_OF_UID);

                    UtInfo.terminalStatus = pDbResult->GetDbUInt(5);

                    UtInfo.creditStatus = pDbResult->GetDbUInt(6);
                    pDbResult->GetDbChar(7, (XS8 *)UtInfo.NetId);
                    if(NULL != g_LoadUtBindInfoCallback)
                    {
                        g_LoadUtBindInfoCallback(&UtInfo);
                    }
                    Count++;
                }
                if( 0 == Count )
                {
                    nRet = ERROR_CODE_DB_NO_DATA_EFFECT;
                }
            }

		
    } 
	//终止该statement语句
	g_PDbService->DbDestroyStatement(connid,pDbStatement);
	pDbStatement = NULL;
	//释放连接池中的该连接
	g_PDbService->DbDestroyConn(connid);

    return nRet; 
}

