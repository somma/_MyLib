// SQLite.h: interface for the CSQLite class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SQLITE_H__3A40912B_8B52_47CD_BB1E_E8699AB2FF88__INCLUDED_)
#define AFX_SQLITE_H__3A40912B_8B52_47CD_BB1E_E8699AB2FF88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <string>
#include "sqlite3.h"

#define SQLITE_MAX_BUFF_SIZE		4096

class CSQLiteQuery
{
public:
	
    CSQLiteQuery();
	
    CSQLiteQuery(const CSQLiteQuery& rQuery);
	
    CSQLiteQuery(sqlite3* pDB,
		sqlite3_stmt* pStmt,
		BOOL bEof,
		BOOL bOwnStmt=TRUE);
	
    CSQLiteQuery& operator=(const CSQLiteQuery& rQuery);
	
    virtual ~CSQLiteQuery();
	
	void Init(sqlite3 *pDB, sqlite3_stmt *pStmt, BOOL bEof, BOOL bOwnStmt=TRUE);
	
    int ColumnCount();
	
	int ColumnIndex(LPCTSTR lpszColName, int& nIndex);
    int ColumnName(int nCol, LPTSTR pszValue, DWORD dwLength);
	
    int ColumnType(int nCol);
	void ColumnDeclType(int nCol, LPTSTR pszType, DWORD dwLength);
	
	int ColumnValue(int iCol, std::wstring& strValue, BOOL& bIsNull);
	int ColumnValue(int iCol, LPTSTR pszValue, DWORD dwLength, BOOL& bIsNull);
	int ColumnValue(int iCol, int& nValue, BOOL& bIsNull);
	int ColumnValue(int iCol, USHORT& uValue, BOOL& bIsNull);
	int ColumnValue(int iCol, INT64& nValue, BOOL& bIsNull);
	
    BOOL IsEOF();
    int Step();
    int finalize();
	
private:
	
	sqlite3* m_pDB;
    sqlite3_stmt* m_pStmt;
    BOOL m_bEof;
    int m_nCols;
    BOOL m_bOwnStmt;
};

class CSQLiteStmt
{
public:
	
    CSQLiteStmt();
	
    CSQLiteStmt(const CSQLiteStmt& rStmt);
	
    CSQLiteStmt(sqlite3* pDB, sqlite3_stmt* pStmt);
	
    virtual ~CSQLiteStmt();
	
    CSQLiteStmt& operator=(const CSQLiteStmt& rStmt);
	
    int execDML();
	
    int execQuery(CSQLiteQuery& rQuery);
	
    int bind(int nParam, LPCTSTR szValue);
    int bind(int nParam, const int nValue);
    int bind(int nParam, const double dwValue);
    int bind(int nParam, const unsigned char* blobValue, int nLen);
    int bindNull(int nParam);
	
    int reset();
	
    int finalize();
	
private:
    sqlite3* m_pDB;
    sqlite3_stmt* m_pStmt;
};

class CSQLite
{
public:
	CSQLite();
	virtual ~CSQLite();

	int Open(LPCTSTR lpszFileName);
	int Exec(LPCTSTR lpszQuery, int (*callback)(void*,int,char**,char**), void *pContext, LPTSTR pszErrMsg, DWORD dwLength);
	int Exec(LPCSTR lpszQuery, int (*callback)(void*,int,char**,char**), void *pContext, LPTSTR pszErrMsg, DWORD dwLength);
	int Exec(LPCTSTR lpszQuery);
	int ExecQuery(LPCTSTR lpszQuery, CSQLiteQuery& rQuery);
	int Close();

protected:
	sqlite3		*m_pDb;
};

#endif // !defined(AFX_SQLITE_H__3A40912B_8B52_47CD_BB1E_E8699AB2FF88__INCLUDED_)
