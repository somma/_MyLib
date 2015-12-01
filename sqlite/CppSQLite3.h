////////////////////////////////////////////////////////////////////////////////
// CppSQLite3 - A C++ wrapper around the SQLite3 embedded database library.
//
// Copyright (c) 2004 Rob Groves. All Rights Reserved. rob.groves@btinternet.com
// 
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without a written
// agreement, is hereby granted, provided that the above copyright notice, 
// this paragraph and the following two paragraphs appear in all copies, 
// modifications, and distributions.
//
// IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
// PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
// EVEN IF THE AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
// ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". THE AUTHOR HAS NO OBLIGATION
// TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
// V3.0		03/08/2004	-Initial Version for sqlite3
//
// V3.1		16/09/2004	-Implemented getXXXXField using sqlite3 functions
//						-Added CppSQLiteDB3::tableExists()
////////////////////////////////////////////////////////////////////////////////
#ifndef _CppSQLite3_H_
#define _CppSQLite3_H_

#include "sqlite3.h"
#include <windows.h>
#include <cstdio>
#include <cstring>

#define CPPSQLITE_ERROR 1000

class CppSQLite3Exception
{
public:

    CppSQLite3Exception(const int nErrCode,
                    char* szErrMess,
                    bool bDeleteMsg=true);

    CppSQLite3Exception(const CppSQLite3Exception&  e);

    virtual ~CppSQLite3Exception();

    const int errorCode() { return mnErrCode; }

    LPCTSTR errorMessage() { return mpszErrMess; }

    static LPCTSTR errorCodeAsString(int nErrCode);

private:

    int mnErrCode;
    TCHAR* mpszErrMess;
};


class CppSQLite3Buffer
{
public:

    CppSQLite3Buffer();

    ~CppSQLite3Buffer();

    LPCTSTR format(LPCTSTR szFormat, ...);

    operator const char*() { return mpBuf; }

    void clear();

private:

    char* mpBuf;
};


class CppSQLite3Binary
{
public:

    CppSQLite3Binary();

    ~CppSQLite3Binary();

    void setBinary(const unsigned char* pBuf, int nLen);
    void setEncoded(const unsigned char* pBuf);

    const unsigned char* getEncoded();
    const unsigned char* getBinary();

    int getBinaryLength();

    unsigned char* allocBuffer(int nLen);

    void clear();

private:

    unsigned char* mpBuf;
    int mnBinaryLen;
    int mnBufferLen;
    int mnEncodedLen;
    bool mbEncoded;
};


class CppSQLite3Query
{
public:

    CppSQLite3Query();

    CppSQLite3Query(const CppSQLite3Query& rQuery);

    CppSQLite3Query(sqlite3* pDB,
				sqlite3_stmt* pVM,
                bool bEof,
                bool bOwnVM=true);

    CppSQLite3Query& operator=(const CppSQLite3Query& rQuery);

    virtual ~CppSQLite3Query();

    int numFields();

    int fieldIndex(LPCTSTR szField);
    LPCTSTR fieldName(int nCol);

    LPCTSTR fieldDeclType(int nCol);
    int fieldDataType(int nCol);

    LPCTSTR fieldValue(int nField);
    LPCTSTR fieldValue(LPCTSTR szField);

    int getIntField(int nField, int nNullValue=0);
    int getIntField(LPCTSTR szField, int nNullValue=0);

    double getFloatField(int nField, double fNullValue=0.0);
    double getFloatField(LPCTSTR szField, double fNullValue=0.0);

    LPCTSTR getStringField(int nField, LPCTSTR szNullValue=TEXT(""));
    LPCTSTR getStringField(LPCTSTR szField, LPCTSTR szNullValue=TEXT(""));

    const unsigned char* getBlobField(int nField, int& nLen);
    const unsigned char* getBlobField(LPCTSTR szField, int& nLen);

    bool fieldIsNull(int nField);
    bool fieldIsNull(LPCTSTR szField);

    bool eof();

    void nextRow();

    void finalize();

private:

    void checkVM();

	sqlite3* mpDB;
    sqlite3_stmt* mpVM;
    bool mbEof;
    int mnCols;
    bool mbOwnVM;
};


class CppSQLite3Table
{
public:

    CppSQLite3Table();

    CppSQLite3Table(const CppSQLite3Table& rTable);

    CppSQLite3Table(LPTSTR* paszResults, int nRows, int nCols);

    virtual ~CppSQLite3Table();

    CppSQLite3Table& operator=(const CppSQLite3Table& rTable);

    int numFields();

    int numRows();

    LPCTSTR fieldName(int nCol);

    LPCTSTR fieldValue(int nField);
    LPCTSTR fieldValue(LPCTSTR szField);

    int getIntField(int nField, int nNullValue=0);
    int getIntField(LPCTSTR szField, int nNullValue=0);

    double getFloatField(int nField, double fNullValue=0.0);
    double getFloatField(LPCTSTR szField, double fNullValue=0.0);

    LPCTSTR getStringField(int nField, LPCTSTR szNullValue=TEXT(""));
    LPCTSTR getStringField(LPCTSTR szField, LPCTSTR szNullValue=TEXT(""));

    bool fieldIsNull(int nField);
    bool fieldIsNull(LPCTSTR szField);

    void setRow(int nRow);

    void finalize();

private:

    void checkResults();

    int mnCols;
    int mnRows;
    int mnCurrentRow;
    LPTSTR* mpaszResults;
};


class CppSQLite3Statement
{
public:

    CppSQLite3Statement();

    CppSQLite3Statement(const CppSQLite3Statement& rStatement);

    CppSQLite3Statement(sqlite3* pDB, sqlite3_stmt* pVM);

    virtual ~CppSQLite3Statement();

    CppSQLite3Statement& operator=(const CppSQLite3Statement& rStatement);

    int execDML();

    CppSQLite3Query execQuery();

    void bind(int nParam, LPCTSTR szValue);
    void bind(int nParam, const int nValue);
    void bind(int nParam, const double dwValue);
    void bind(int nParam, const unsigned char* blobValue, int nLen);
    void bindNull(int nParam);

    void reset();

    void finalize();

private:

    void checkDB();
    void checkVM();

    sqlite3* mpDB;
    sqlite3_stmt* mpVM;
};


class CppSQLite3DB
{
public:

    CppSQLite3DB();

    virtual ~CppSQLite3DB();

    void open(LPCTSTR szFile);

    void close();

	bool tableExists(LPCTSTR szTable);

    int execDML(LPCTSTR szSQL);

    CppSQLite3Query execQuery(LPCTSTR* szSQL);

    int execScalar(LPCTSTR szSQL);

    CppSQLite3Table getTable(LPCTSTR szSQL);

    CppSQLite3Statement compileStatement(LPCTSTR szSQL);

    sqlite_int64 lastRowId();

    void interrupt() { sqlite3_interrupt(mpDB); }

    void setBusyTimeout(int nMillisecs);

    static LPCTSTR SQLiteVersion() { return TEXT(SQLITE_VERSION); }

private:

    CppSQLite3DB(const CppSQLite3DB& db);
    CppSQLite3DB& operator=(const CppSQLite3DB& db);

    sqlite3_stmt* compile(LPCTSTR szSQL);

    void checkDB();

    sqlite3* mpDB;
    int mnBusyTimeoutMs;
};

#endif
