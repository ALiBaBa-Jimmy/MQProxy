#include "MysqlHDB_API.h"
#include "DbFactoryMysql.h"
#include "fid_def.h"
#include "DbFactory.h"
#include "SPRTraceStruct.h"
#include "DbErrorCode.h"



extern callbackTraceFun g_callbackTraceFunOfHssHDb;







XS32 MysqlHDB_QryLocalNetWorkingbyUid(XS32 connid, SDbReqSql *req, XU8 *pNetworkId)
{
    XS32     nRet = XSUCC;
    
    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
	XU32    fid = FID_AGENTDB;
    CDbService   *pDbService = NULL;
    if( NULL == req )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"parameters is null!!");
        nRet = ERROR_CODE_DB_NULL_PTR;
    }
    else
    {
        pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
        sprintf(sql,"%s ", req->conditionSql);
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
		

        pDbStatement = pDbService->DbCreateStatement(connid,sql);
        if ( NULL == pDbStatement )
        {
            g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"Exec sql failed: %s", sql);
            nRet = ERROR_CODE_DB_STMENT_ERROR;
        }
        else
        {
            //Ö´ÐÐ²éÑ¯
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

            //ÖÕÖ¹¸ÃstatementÓï¾ä
            pDbService->DbDestroyStatement(connid,pDbStatement);
            pDbStatement = NULL;
            pDbResult = NULL;
			
        } 
    }

    return nRet; 
}



XS32 MysqlHDB_QryuidbyTel(XS32 connid, SDbReqSql *req, XU8 *uid)
{
    XS32     nRet = XSUCC;
    CDbService   *pDbService = NULL;
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
        pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
        sprintf(sql,"%s ", req->conditionSql);
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
		

        pDbStatement = pDbService->DbCreateStatement(connid,sql);
        if ( NULL == pDbStatement )
        {
            g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"Exec sql failed: %s", sql);
            nRet = ERROR_CODE_DB_STMENT_ERROR;
        }
        else
        {
            //Ö´ÐÐ²éÑ¯
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
                    DB_HexToStr(uidStr, uid, LENGTH_OF_UID);
                    Count++;
                                        
                    
                }
                if( 0 == Count )
                {
                    nRet = ERROR_CODE_DB_NO_DATA_EFFECT;
                }
            }

            //ÖÕÖ¹¸ÃstatementÓï¾ä
            pDbService->DbDestroyStatement(connid,pDbStatement);
            pDbStatement = NULL;
            pDbResult = NULL;
			
        } 
    }

    return nRet; 
}




XS32 MysqlHDB_QryuidbyAlias(XS32 connid, SDbReqSql *req, XU8 *uid)
{
    XS32     nRet = XSUCC;
    CDbService   *pDbService = NULL;
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
        pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
        sprintf(sql,"%s ", req->conditionSql);
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
		

        pDbStatement = pDbService->DbCreateStatement(connid,sql);
        if ( NULL == pDbStatement )
        {
            g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"Exec sql failed: %s", sql);
            nRet = ERROR_CODE_DB_STMENT_ERROR;
        }
        else
        {
            //Ö´ÐÐ²éÑ¯
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

            //ÖÕÖ¹¸ÃstatementÓï¾ä
            pDbService->DbDestroyStatement(connid,pDbStatement);
            pDbStatement = NULL;
            pDbResult = NULL;
			
        } 
    }

    return nRet; 
}




XS32 MysqlHDB_Update(XS32 connid, SDbReqSql *req)
{
    XS32     nRet = XSUCC;

    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    CDbService   *pDbService = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
	XU32 fid = FID_AGENTDB;


    sprintf(sql,"%s ",  req->conditionSql);
    g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);
    
	
    pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
    pDbStatement = pDbService->DbCreateStatement(connid,sql);
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
			pDbService->DbRollback(connid);
			nRet = ERROR_CODE_DB_EXECUTE_QUERY;
		}
        else 
        {
    		pDbService->DbCommit(connid);
        }
		
    } 
	//ÖÕÖ¹¸ÃstatementÓï¾ä
	pDbService->DbDestroyStatement(connid,pDbStatement);
	pDbStatement = NULL;
    pDbResult = NULL;
	
    return nRet; 
}


XS32 MysqlHDB_LoadUtInfo(XS32 connid, SDbReqSql *req)
{
    XS32     nRet = XSUCC;
    XU8 pidStr[9]={0};
    XU8 uidStr[9]={0};
    CDbService   *pDbService = NULL;
    CDbStatement *pDbStatement = NULL;
    CDbResult    *pDbResult = NULL;
    XS8          sql[DB_MAX_SQL_LENGTH*2] = {0};
    XU16         Count = 0;
	XU32 fid = FID_AGENTDB;
    SLeaseHoldCtlTable UtInfo={0};



    pDbService = CDbFactoryMysql::GetInstance()->GetDbService();
    sprintf(sql,"%s ", req->conditionSql);
    g_callbackTraceFunOfHssHDb(XNULL,MD(fid, PL_DBG),"sql = %s", sql);


    pDbStatement = pDbService->DbCreateStatement(connid,sql);
    if ( NULL == pDbStatement )
    {
        g_callbackTraceFunOfHssHDb(XNULL,MD(fid,PL_ERR),"configure statement is NULL,error!");
        nRet = ERROR_CODE_DB_STMENT_ERROR;
    }
    else
    {
            //Ö´ÐÐ²éÑ¯
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
	//ÖÕÖ¹¸ÃstatementÓï¾ä
	pDbService->DbDestroyStatement(connid,pDbStatement);
	pDbStatement = NULL;
    pDbResult = NULL;

    return nRet; 
}

