/******************************************************************************
 *
 *	Copyright (C) 2007 AhnLab, Inc. All rights reserved.
 *
 *	This program is strictly confidential and not be used in outside the office...and 
 *	some other copyright agreement etc.
 *
 *	File:
 *		D:\SVN\PROJECT\MEDICINE\ASDINFRA\2.0\SEEDER\TRUNK\SRC\COMMON\CSqlite.h
 *
 *	Author:
 *
 *	DESCRIPTION :
 *		
 *	HISTORY :
 * 	- 2015:1:19 created by somma
/*****************************************************************************/
#pragma once

#include <list>
#include "asd_sal.h"
#include "SQLite.h"


/**
 * @brief	base class for local sqlite database
**/
class CLocalDb
{
public:
	CLocalDb();
	virtual ~CLocalDb();

	virtual bool initialize(_In_ std::wstring& local_db_path);
	void finalize();

	bool Exec(_In_ const wchar_t* query_string);
	bool ExecQuery(_In_ const wchar_t* query_string, _Out_ CSQLiteQuery& query_result);
	
protected:
	bool	_initialized;
	CSQLite _db;
};
