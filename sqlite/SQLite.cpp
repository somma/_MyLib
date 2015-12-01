// SQLite.cpp: implementation of the CSQLite class.
//
//////////////////////////////////////////////////////////////////////

#include "SQLite.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSQLite::CSQLite()
{
	m_pDb = NULL;
	
	sqlite3_initialize();
}

CSQLite::~CSQLite()
{
	sqlite3_shutdown();
}

int CSQLite::Open(LPCTSTR lpszFileName)
{
	int nRtVal;

#ifdef UNICODE
	char szFileNameA[MAX_PATH];
	WideCharToMultiByte(	CP_ACP,
							0,
							lpszFileName,
							-1,
							szFileNameA,
							MAX_PATH,
							NULL,
							NULL	);
	nRtVal = sqlite3_open(szFileNameA, &m_pDb);
#else
	nRtVal = sqlite3_open(lpszFileName, &m_pDb);
#endif

	sqlite3_busy_timeout(m_pDb, 60000);
	sqlite3_enable_shared_cache(1);

	sqlite3_exec(m_pDb, "PRAGMA journal_mode=OFF;", NULL, NULL, NULL);
	sqlite3_exec(m_pDb, "PRAGMA synchronous=OFF;", NULL, NULL, NULL);

	return nRtVal;
}

int CSQLite::Close()
{
	return sqlite3_close(m_pDb);
}

int CSQLite::Exec(LPCTSTR lpszQuery, int (*callback)(void*,int,char**,char**), void *pContext, LPTSTR pszErrMsg, DWORD dwLength)
{
	char *pErrMsgA;
	char *pszQueryA;
	int nRtVal;
	const char* pszTail = NULL;

#ifdef UNICODE
	char szQueryA[SQLITE_MAX_BUFF_SIZE];
	WideCharToMultiByte(	CP_ACP,
							0,
							lpszQuery,
							-1,
							szQueryA,
							SQLITE_MAX_BUFF_SIZE,
							NULL,
							NULL	);
	pszQueryA = szQueryA;
#else
	pszQueryA = lpszQuery;
#endif

	nRtVal = sqlite3_exec(m_pDb, pszQueryA, callback, pContext, &pErrMsgA);
	if( SQLITE_OK != nRtVal )
	{
#ifdef _UNICODE
		MultiByteToWideChar(	CP_ACP,
								0,
								(LPCSTR)pErrMsgA,
								-1,
								pszErrMsg,
								dwLength	);
#else
		StringCchCopy(pszErrMsg, dwLength, pErrMsgA);
#endif
		sqlite3_free(pErrMsgA);
	}

	return nRtVal;
}

int CSQLite::Exec(LPCSTR lpszQuery, int (*callback)(void*,int,char**,char**), void *pContext, LPTSTR pszErrMsg, DWORD dwLength)
{
	char *pErrMsgA;
	int nRtVal;
	const char* pszTail = NULL;
	
	nRtVal = sqlite3_exec(m_pDb, lpszQuery, callback, pContext, &pErrMsgA);
	if( SQLITE_OK != nRtVal )
	{
#ifdef _UNICODE
		MultiByteToWideChar(	CP_ACP,
								0,
								(LPCSTR)pErrMsgA,
								-1,
								pszErrMsg,
								dwLength	);
#else
		StringCchCopy(pszErrMsg, dwLength, pErrMsgA);
#endif
		sqlite3_free(pErrMsgA);
	}
	
	return nRtVal;
}

int CSQLite::Exec(LPCTSTR lpszQuery)
{
	int nRtVal = -ERROR_FUNCTION_FAILED;
	
	if( NULL == m_pDb )
	{
		return nRtVal;
	}
	
	const char *pszTail = NULL;
	
#ifdef UNICODE
	char szQueryA[SQLITE_MAX_BUFF_SIZE];
	WideCharToMultiByte(	CP_ACP,
		0,
		lpszQuery,
		-1,
		szQueryA,
		SQLITE_MAX_BUFF_SIZE,
		NULL,
		NULL	);
	nRtVal = sqlite3_exec(m_pDb, szQueryA, NULL, NULL, NULL);
	//nRtVal = sqlite3_prepare(m_pDb, szQueryA, -1, &pStmt, &pszTail);
#else
	nRtVal = sqlite3_exec(m_pDb, lpszQuery, NULL, NULL, NULL);
	//nRtVal = sqlite3_prepare(m_pDb, lpszQuery, -1, &pStmt, &pszTail);
#endif
	
	if( SQLITE_OK == nRtVal )
	{
		//sqlite3_finalize(pStmt);
	}
	
	return nRtVal;
}

int CSQLite::ExecQuery(LPCTSTR lpszQuery, CSQLiteQuery& rQuery)
{
	int nRtVal = -ERROR_FUNCTION_FAILED;

	if( NULL == m_pDb )
	{
		return nRtVal;
	}
	
	const char *pszTail = NULL;
	sqlite3_stmt* pStmt;

#ifdef UNICODE
	char szQueryA[SQLITE_MAX_BUFF_SIZE];
	WideCharToMultiByte(	CP_ACP,
							0,
							lpszQuery,
							-1,
							szQueryA,
							SQLITE_MAX_BUFF_SIZE,
							NULL,
							NULL	);
	nRtVal = sqlite3_prepare(m_pDb, szQueryA, -1, &pStmt, &pszTail);
#else
	nRtVal = sqlite3_prepare(m_pDb, lpszQuery, -1, &pStmt, &pszTail);
#endif

	if( SQLITE_OK == nRtVal )
	{
		nRtVal = sqlite3_step(pStmt);
		if( SQLITE_DONE == nRtVal )
		{
			rQuery.Init(m_pDb, pStmt, TRUE);

			nRtVal = ERROR_SUCCESS;
		}
		else if ( SQLITE_ROW == nRtVal )
		{
			rQuery.Init(m_pDb, pStmt, FALSE);

			nRtVal = ERROR_SUCCESS;
		}
		else
		{
			sqlite3_finalize(pStmt);
		}
	}

	return nRtVal;
}

CSQLiteStmt::CSQLiteStmt()
{
	m_pDB = NULL;
	m_pStmt = NULL;
}

CSQLiteStmt::CSQLiteStmt(const CSQLiteStmt& rStmt)
{
	m_pDB = rStmt.m_pDB;
	m_pStmt = rStmt.m_pStmt;
	// Only one object can own VM
	const_cast<CSQLiteStmt&>(rStmt).m_pStmt = 0;
}

CSQLiteStmt::CSQLiteStmt(sqlite3* pDB, sqlite3_stmt* pStmt)
{
	m_pDB = pDB;
	m_pStmt = pStmt;
}

CSQLiteStmt::~CSQLiteStmt()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}

CSQLiteStmt& CSQLiteStmt::operator=(const CSQLiteStmt& rStmt)
{
	m_pDB = rStmt.m_pDB;
	m_pStmt = rStmt.m_pStmt;

	const_cast<CSQLiteStmt&>(rStmt).m_pStmt = 0;
	return *this;
}

int CSQLiteStmt::execDML()
{
	if( NULL == m_pDB || NULL == m_pStmt )
	{
		return -1;
	}

	int nRet = sqlite3_step(m_pStmt);

	if (nRet == SQLITE_DONE)
	{
		int nRowsChanged = sqlite3_changes(m_pDB);

		nRet = sqlite3_reset(m_pStmt);
		if (nRet != SQLITE_OK)
		{
		}

		return nRowsChanged;
	}
	else
	{
		nRet = sqlite3_reset(m_pStmt);
	}

	return -1;
}

int CSQLiteStmt::execQuery(CSQLiteQuery &rQuery)
{
	int nRtVal;

	if( NULL == m_pDB || NULL == m_pStmt )
	{
		return -ERROR_FUNCTION_FAILED;
	}

	nRtVal = sqlite3_step(m_pStmt);
	if (nRtVal == SQLITE_DONE)
	{
		rQuery.Init(m_pDB, m_pStmt, TRUE, FALSE);

		nRtVal = ERROR_SUCCESS;
	}
	else if (nRtVal == SQLITE_ROW)
	{
		rQuery.Init(m_pDB, m_pStmt, FALSE, FALSE);
		
		nRtVal = ERROR_SUCCESS;
	}
	else
	{
		nRtVal = sqlite3_reset(m_pStmt);
	}

	return nRtVal;
}

int CSQLiteStmt::bind(int nParam, LPCTSTR szValue)
{
	char szValueA[SQLITE_MAX_BUFF_SIZE];
	char *pszValueA;

	if( NULL == m_pStmt )
	{
		return -ERROR_FUNCTION_FAILED;
	}

#ifdef _UNICODE
	WideCharToMultiByte(	CP_ACP,
							0,
							szValue,
							-1,
							szValueA,
							SQLITE_MAX_BUFF_SIZE,
							NULL,
							NULL	);
	pszValueA = szValueA;
#else
	pszValueA = szValue;
#endif

	return sqlite3_bind_text(m_pStmt, nParam, pszValueA, -1, SQLITE_TRANSIENT);
}

int CSQLiteStmt::bind(int nParam, const int nValue)
{
	if( NULL == m_pStmt )
	{
		return -ERROR_FUNCTION_FAILED;
	}

	return sqlite3_bind_int(m_pStmt, nParam, nValue);
}

int CSQLiteStmt::bind(int nParam, const double dValue)
{
	if( NULL == m_pStmt )
	{
		return -ERROR_FUNCTION_FAILED;
	}

	return sqlite3_bind_double(m_pStmt, nParam, dValue);
}

int CSQLiteStmt::bind(int nParam, const unsigned char* blobValue, int nLen)
{
	if( NULL == m_pStmt )
	{
		return -ERROR_FUNCTION_FAILED;
	}

	return sqlite3_bind_blob(m_pStmt, nParam,
								(const void*)blobValue, nLen, SQLITE_TRANSIENT);
}
	
int CSQLiteStmt::bindNull(int nParam)
{
	if( NULL == m_pStmt )
	{
		return -ERROR_FUNCTION_FAILED;
	}

	return sqlite3_bind_null(m_pStmt, nParam);
}

int CSQLiteStmt::reset()
{
	int nRtVal = ERROR_SUCCESS;

	if( NULL != m_pStmt )
	{
		nRtVal = sqlite3_reset(m_pStmt);
	}

	return nRtVal;
}

int CSQLiteStmt::finalize()
{
	int nRtVal = ERROR_SUCCESS;
	
	if( NULL != m_pStmt )
	{
		nRtVal = sqlite3_finalize(m_pStmt);
	}
	
	return nRtVal;
}

CSQLiteQuery::CSQLiteQuery()
{
	m_pStmt = 0;
	m_bEof = TRUE;
	m_nCols = 0;
	m_bOwnStmt = FALSE;
}


CSQLiteQuery::CSQLiteQuery(const CSQLiteQuery& rQuery)
{
	m_pStmt = rQuery.m_pStmt;
	// Only one object can own the VM
	const_cast<CSQLiteQuery&>(rQuery).m_pStmt = 0;
	m_bEof = rQuery.m_bEof;
	m_nCols = rQuery.m_nCols;
	m_bOwnStmt = rQuery.m_bOwnStmt;
}


CSQLiteQuery::CSQLiteQuery(sqlite3* pDB,
							sqlite3_stmt* pStmt,
							BOOL bEof,
							BOOL bOwnStmt/*=TRUE*/)
{
	m_pDB = pDB;
	m_pStmt = pStmt;
	m_bEof = bEof;
	m_nCols = sqlite3_column_count(m_pStmt);
	m_bOwnStmt = bOwnStmt;
}


CSQLiteQuery::~CSQLiteQuery()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}


CSQLiteQuery& CSQLiteQuery::operator=(const CSQLiteQuery& rQuery)
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
	m_pStmt = rQuery.m_pStmt;
	// Only one object can own the VM
	const_cast<CSQLiteQuery&>(rQuery).m_pStmt = 0;
	m_bEof = rQuery.m_bEof;
	m_nCols = rQuery.m_nCols;
	m_bOwnStmt = rQuery.m_bOwnStmt;
	return *this;
}

void CSQLiteQuery::Init(sqlite3 *pDB, sqlite3_stmt *pStmt, BOOL bEof, BOOL bOwnStmt)
{
	finalize();

	m_pDB = pDB;
	m_pStmt = pStmt;
	
	m_bEof = bEof;
	m_nCols = sqlite3_column_count(pStmt);
	m_bOwnStmt = bOwnStmt;
}

int CSQLiteQuery::ColumnCount()
{
	if( NULL == m_pStmt )
	{
		return -ERROR_FUNCTION_FAILED;
	}

	return m_nCols;
}

int CSQLiteQuery::ColumnValue(int iCol, int& nValue, BOOL& bIsNull)
{
	if( SQLITE_NULL == ColumnType(iCol) )
	{
		bIsNull = TRUE;
	}
	else
	{
		nValue = sqlite3_column_int(m_pStmt, iCol);
		bIsNull = FALSE;
	}
	
	return ERROR_SUCCESS;
}

int CSQLiteQuery::ColumnValue(int iCol, USHORT& uValue, BOOL& bIsNull)
{
	int nValue;
	int nRtVal;

	nRtVal = ColumnValue(iCol, nValue, bIsNull);
	if( SQLITE_OK == nRtVal )
	{
		uValue = (USHORT)nValue;
	}
	
	return ERROR_SUCCESS;
}

int CSQLiteQuery::ColumnValue(int iCol, INT64& nValue, BOOL& bIsNull)
{
	if( SQLITE_NULL == ColumnType(iCol) )
	{
		bIsNull = TRUE;
	}
	else
	{
		nValue = sqlite3_column_int64(m_pStmt, iCol);
		bIsNull = FALSE;
	}
	
	return ERROR_SUCCESS;
}

int CSQLiteQuery::ColumnValue(int iCol, std::wstring& strValue, BOOL& bIsNull)
{
	int nRtVal = ERROR_SUCCESS;

	wchar_t str_tmp[SQLITE_MAX_BUFF_SIZE] = {0,};
	nRtVal = ColumnValue(iCol, str_tmp, RTL_NUMBER_OF(str_tmp), bIsNull);
	if( ERROR_SUCCESS == nRtVal )
	{
		strValue = str_tmp;
	}
	
	return nRtVal;
}

int CSQLiteQuery::ColumnValue(int iCol, LPTSTR pszValue, DWORD dwLength, BOOL& bIsNull)
{
	int nRtVal = ERROR_SUCCESS;
	const unsigned char *lpszValueA;

	if( SQLITE_NULL == ColumnType(iCol) )
	{
		bIsNull = TRUE;
	}
	else
	{
		lpszValueA = sqlite3_column_text(m_pStmt, iCol);
		if( NULL == lpszValueA )
		{
			nRtVal = -ERROR_FUNCTION_FAILED;
		}
		else
		{
#ifdef _UNICODE
			MultiByteToWideChar(	CP_ACP,
									0,
									(LPCSTR)lpszValueA,
									-1,
									pszValue,
									dwLength	);
#else
			StringCchCopy(pszValue, dwLength, lpszValueA);
#endif
			bIsNull = FALSE;
		}
	}

	return nRtVal;
}

int CSQLiteQuery::ColumnIndex(LPCTSTR lpszColName, int& nIndex)
{
	char szColNameA[SQLITE_MAX_BUFF_SIZE];
	char *pszColNameA;

	if( NULL == m_pStmt || NULL == lpszColName)
	{
		return -ERROR_FUNCTION_FAILED;
	}

#ifdef _UNICODE
	WideCharToMultiByte(	CP_ACP,
							0,
							lpszColName,
							-1,
							szColNameA,
							SQLITE_MAX_BUFF_SIZE,
							NULL,
							NULL	);
	pszColNameA = szColNameA;
#else
	pszColNameA = lpszColName;
#endif

	for (int nCol = 0; nCol < m_nCols; nCol++)
	{
		LPCSTR lpszTemp = sqlite3_column_name(m_pStmt, nCol);

		if(0 == strcmp(pszColNameA, lpszTemp))
		{
			nIndex = nCol;
			return ERROR_SUCCESS;
		}
	}

	return -ERROR_NOT_FOUND;
}


int CSQLiteQuery::ColumnName(int nCol, LPTSTR pszValue, DWORD dwLength)
{
	int nRtVal = -ERROR_FUNCTION_FAILED;
	const char *pszValueA;

	if( NULL == m_pStmt )
	{
		return nRtVal;
	}

	pszValueA = sqlite3_column_name(m_pStmt, nCol);
	if( NULL != pszValueA )
	{
#ifdef _UNICODE
		MultiByteToWideChar(	CP_ACP,
								0,
								pszValueA,
								-1,
								pszValue,
								dwLength	);
#else
		StringCchCopy(pszValue, dwLength, pszValueA);
#endif
		nRtVal = ERROR_SUCCESS;
	}

	return nRtVal;
}


void CSQLiteQuery::ColumnDeclType(int nCol, LPTSTR pszType, DWORD dwLength)
{
	LPCSTR pszTypeA = sqlite3_column_decltype(m_pStmt, nCol);
	if( NULL != pszTypeA )
	{
#ifdef _UNICODE
		MultiByteToWideChar(	CP_ACP,
								0,
								pszTypeA,
								-1,
								pszType,
								dwLength	);
#else
		StringCchCopy(pszType, dwLength, pszTypeA);
#endif
	}
}


int CSQLiteQuery::ColumnType(int nCol)
{
	if( NULL == m_pStmt || nCol < 0 || nCol > m_nCols-1 )
	{
		return -ERROR_FUNCTION_FAILED;
	}

	return sqlite3_column_type(m_pStmt, nCol);
}

BOOL CSQLiteQuery::IsEOF()
{
	return m_bEof;
}

int CSQLiteQuery::Step()
{
	int nRtVal;

	if( NULL == m_pStmt )
	{
		return -ERROR_FUNCTION_FAILED;
	}

	nRtVal = sqlite3_step(m_pStmt);

	if( SQLITE_DONE == nRtVal )
	{
		// no rows
		m_bEof = TRUE;
	}
	else if( SQLITE_ROW == nRtVal )
	{
		// more rows, nothing to do
	}
	else
	{
		sqlite3_finalize(m_pStmt);

		m_pStmt = 0;
	}

	return nRtVal;
}


int CSQLiteQuery::finalize()
{
	int nRtVal = ERROR_SUCCESS;

	if( NULL != m_pStmt && TRUE == m_bOwnStmt )
	{
		nRtVal = sqlite3_finalize(m_pStmt);

		m_pStmt = 0;
	}

	return nRtVal;
}
