/******************************************************************************
 *
 *	Copyright (C) 2007 AhnLab, Inc. All rights reserved.
 *
 *	This program is strictly confidential and not be used in outside the office...and 
 *	some other copyright agreement etc.
 *
 *	File:
 *		D:\svn\Project\Medicine\ASDInfra\2.0\seeder\Trunk\Src\Common\CLocalDb.cpp
 *
 *	Author:
 *
 *	DESCRIPTION :
 *		
 *	HISTORY :
 * 	- 2015:1:19 created by somma
/*****************************************************************************/
#include "StdAfx.h"
#include "LocalDb.h"

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
CLocalDb::CLocalDb()
: _initialized(false)
{

}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
CLocalDb::~CLocalDb()
{
	finalize();
}

/**
 * @brief	존재하는 db 파일을 오픈한다. 
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool CLocalDb::initialize(_In_ std::wstring& local_db_path)
{
	if (true == _initialized) return true;

	if (true != is_file_existsW(local_db_path.c_str()))
	{
		log_info 
			L"[CLocalDb::initialize] db does not exist. file = %s", 
			local_db_path.c_str() 
		log_end
		return false;
	}

	int iret = _db.Open(local_db_path.c_str());
	if (SQLITE_OK != iret)
	{
		log_err 
			L"[CLocalDb::initialize] CSQLite::Open( %s )", 
			local_db_path.c_str() 
		log_end
		return false;
	}

	//> need to do something else? override this method plz.

	_initialized = true;
	return true;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void CLocalDb::finalize()
{
	if (true == _initialized)
	{
		_db.Close();
		_initialized = false;
	}
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool 
CLocalDb::Exec(_In_ const wchar_t* query_string)
{
	_ASSERTE(NULL != query_string);
	if (NULL == query_string) return false;

	int ret = _db.Exec(query_string);
	if (SQLITE_OK != ret)
	{
		log_err 
			L"[CLocalDb::Exec] exec local query = %s, ret = %u", 
			query_string,
			ret 
		log_end
		return false;
	}

	return true;
}


/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool 
CLocalDb::ExecQuery(
	_In_ const wchar_t* query_string, 
	_Out_ CSQLiteQuery& query_result
	)
{
	_ASSERTE(NULL != query_string);
	if (NULL == query_string) return false;

	int ret = _db.ExecQuery(query_string, query_result);
	if (SQLITE_OK != ret)
	{
		log_err 
			L"[CLocalDb::ExecQuery] exec local query = %s, ret = %u", 
			query_string, 
			ret 
		log_end
		return false;
	}

	return true;
}
