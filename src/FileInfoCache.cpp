/**
 * @file    FileInfoCache.h
 * @brief   
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2017/11/14 11:54 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"
#include "_MyLib/src/log.h"
#include "_MyLib/src/Win32Utils.h"
#include "_MyLib/src/Singleton.h"
#include "_MyLib/src/FileInfoCache.h"
#include "_MyLib/src/md5.h"
#include "_MyLib/src/sha2.h"

#define _create_file_cache \
                "CREATE TABLE file_hash ( "\
                "`id`	INTEGER PRIMARY KEY AUTOINCREMENT, "\
                "`path`	TEXT NOT NULL, "\
                "`create_time`	INTEGER NOT NULL, "\
                "`write_time`	INTEGER NOT NULL, "\
                "`size`	INTEGER NOT NULL, "\
                "`md5`	TEXT NOT NULL, "\
                "`sha2`	TEXT NOT NULL, "\
				"`hit_count` INTEGER DEFAULT 1"\
                ") "

#define _select_file_cache \
                "SELECT md5, sha2, hit_count FROM file_hash "\
                "WHERE "\
                " path = ?1 AND "\
                " create_time = ?2 AND "\
                " write_time = ?3 AND "\
                " size = ?4"

#define _insert_file_cache \
                "INSERT INTO file_hash "\
                "( "\
                " path,  "\
                " create_time, "\
                " write_time, "\
                " size, "\
                " md5, "\
                " sha2  "\
                ")  "\
                "VALUES "\
                "( "\
                " ?1, "\
                " ?2, "\
                " ?3, "\
                " ?4, "\
                " ?5, "\
                " ?6 "\
                ") "

#define _update_file_cache \
				"UPDATE file_hash "\
				"SET "\
				"	hit_count = ?1	"\
				"WHERE "\
				" path = ?2 AND "\
                " create_time = ?3 AND "\
                " write_time = ?4 AND "\
                " size = ?5"
//
// file cache가 일정한 개수(기본값: 5000개, 사용자가 size를 변경 한 경우 달라 질 수 있다)
// 이상인 경우 "hit_count"의 평균 이하인 레코드는 삭제 한다.
// (eg. avg(hit_count) => 1.5, round(avg(hit_count)) => 2.0)
//
#define _delete_file_cache \
				"DELETE FROM file_hash "\
				"WHERE "\
				" hit_count <= "\
				" (SELECT round(avg(hit_count)) from file_hash)"

/// @brief  constructor
FileInfoCache::FileInfoCache() : 
	_initialized(false), 
	_size(0), 
	_hit_count(0),
	_cache_size(0),
	_select_cache_stmt(nullptr),
	_insert_cache_stmt(nullptr),
	_update_cache_stmt(nullptr),
	_delete_cache_stmt(nullptr)
{
}

/// @brief  
FileInfoCache::~FileInfoCache()
{
    finalize();
}

/// @brief  
bool 
FileInfoCache::initialize(
	_In_ const wchar_t* db_file_path,
	_In_ int64_t cache_size,
	_In_ bool delete_if_exist
	)
{
	_ASSERTE(NULL != db_file_path);
    if (NULL == db_file_path) return false;

	if (true == _initialized) return true;

	if (true == is_file_existsW(db_file_path) && true == delete_if_exist)
    {
        if (!DeleteFileW(db_file_path))
        {
            log_err "DeleteFileW( %ws ) failed. gle = %u", 
				db_file_path, 
				GetLastError()
                log_end;
            return false;
        }
    }
	
	try 
	{
		//
		// 데이터베이스 파일을 열고, precompiled statement 들을 생성한다.
		//

		_db.open(db_file_path);
		
		//
		// "file_hash" 테이블 존재 유무를 체크 한 후 없으면 새로 만든다.
		//
		if (true != _db.tableExists("file_hash"))
		{
			_db.execDML(_create_file_cache);
		}

		if (nullptr != _select_cache_stmt) { delete _select_cache_stmt; }
		if (nullptr != _insert_cache_stmt) { delete _insert_cache_stmt; }
		if (nullptr != _update_cache_stmt) { delete _update_cache_stmt; }
		if (nullptr != _delete_cache_stmt) { delete _delete_cache_stmt; }
		//
		//	statement 객체들을 생성한다. 
		// 
		_select_cache_stmt = _db.compileStatement(_select_file_cache);
		_insert_cache_stmt = _db.compileStatement(_insert_file_cache);
		_update_cache_stmt = _db.compileStatement(_update_file_cache);
		_delete_cache_stmt = _db.compileStatement(_delete_file_cache);
	}
	catch (CppSQLite3Exception& e)
	{
		log_err
			"sqlite exception. FileInfoCache::initialize, code = %d, msg = %s",
			e.errorCode(),
			e.errorMessage()
		log_end;
		return false;
	}

	// 캐시 사이즈 초기화
	_cache_size = cache_size;

    _initialized = true;
    return true;
}

/// @brief  
void FileInfoCache::finalize()
{
    if (true != _initialized) return;

    try
    {
		//
		// statement객체들을 제거한다.
		//
		delete _select_cache_stmt; _select_cache_stmt = nullptr;
		delete _insert_cache_stmt; _insert_cache_stmt = nullptr;
		delete _update_cache_stmt; _update_cache_stmt = nullptr;
		delete _delete_cache_stmt; _delete_cache_stmt = nullptr;
		//
		// db 연결을 종료 한다.
		//
        _db.close();
        _initialized = false;
    }
    catch (CppSQLite3Exception& e)
    {
		log_err
			"_db.close() failed. ecode = %d, emsg = %s",
			e.errorCode(),
			e.errorMessage()
		log_end;
    }
}

/// @brief 
bool 
FileInfoCache::get_file_information(
	_In_ const wchar_t* file_path,
	_Out_ FileInformation& file_information
	)
{
	_ASSERTE(nullptr != file_path);
	if (nullptr == file_path) return false;

	//
	//	캐시 조회를 위한 기본 정보를 구한다.
	//
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (!GetFileAttributesExW(file_path,
							  GetFileExInfoStandard,
							  &fad))
	{
		log_err "GetFileAttributesExW() failed. file=%ws, gle=%u",
			file_path,
			GetLastError()
			log_end;
		return false;
	}

	if (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		log_warn "Not file. path=%ws",
			file_path
		log_end;
		return false;
	}

	uint64_t create_time = file_time_to_int(&fad.ftCreationTime);
	uint64_t write_time = file_time_to_int(&fad.ftLastWriteTime);	
	uint64_t size = ((uint64_t)fad.nFileSizeHigh << 32) | fad.nFileSizeLow;
	
	//
	//	파일 사이즈가 0 이면 그냥 리턴
	//
	if (size == 0) return true;

	//
	//	1st phase, 캐시에서 찾아본다. 
	//
	std::string md5;
	std::string sha2;


	// 캐시 정보가 존재 한다면 캐시 정보를 반환한다.
	if (get_flie_info(file_path,
					  create_time,
					  write_time,
					  size,
					  md5,
					  sha2))
	{
		file_information.size = size;
		file_information.create_time = create_time;
		file_information.write_time = write_time;
		file_information.md5 = md5;
		file_information.sha2 = sha2;

		return true;
	}
	

	//
	//	2nd phase, 파일정보를 구하고, 캐시에 등록한다.
	// 
	if (true != file_util_get_hash(file_path, md5, sha2))
	{
		log_err "file_util_get_hash() failed. file=%ws",
			file_path
		log_end;
		return false;
	}

	//
	// 캐시에 등록 하기전 캐시 사이즈가 일정한 개수(기본값: 5000개, 
	// 사용자가 cache_size를 변경 한 경우 달라 질 수 있다) 를 초과하는 경우 
	// table내의 "hit count"의 평균을 구한 후 평균 이하인 데이터들을
	// 삭제한다.
	//
	if (_cache_size < _size)
	{
		int32_t delete_record_count = 0;
		
		try 
		{
			delete_record_count = _delete_cache_stmt->execDML();
		}
		catch (CppSQLite3Exception& e)
		{
			log_err
				"sqlite exception. get_file_information, code = %d, msg = %s",
				e.errorCode(),
				e.errorMessage()
			log_end;
		}

		//
		// 삭제된 레코드 수가 케시 사이즈보다 큰 경우 0으로 초기화 시킨다.
		//

		if (_size < delete_record_count)
		{
			_ASSERTE(!"oops, deleted record is larger than cache size");
			InterlockedExchange64(&_size, 0);
		}

		//
		// 데이터 삭제 이후 현재 캐시 사이즈에서 삭제된 레코드 수 만큼 
		// 빼준다.
		// =====================참고===========================
		// 현재 코드에서 기본 캐시 사이즈(5000개) 이상의 파일 정보가
		// 캐싱이 되어 레코드가 삭제 되는 코드를 테스트 해보지 못해
		// 다음과 같은 로깅코드를 남겨 두었으며 추후 삭제 할 수 있다.
		// ===================================================
		//
		
		InterlockedAdd64(&_size,-delete_record_count);

		log_dbg
			"delete file info cache record(count:%lu)",
			delete_record_count
		log_end;
	}

	

	if (true != insert_file_info(file_path,
								 create_time,
								 write_time,
								 size,
								 md5.c_str(),
								 sha2.c_str()))
	{
		log_err "insert_file_info() failed. file=%ws",
			file_path
		log_end;
		return false;
	}

	//
	//	파일정보를 리턴한다.
	// 
	file_information.size = size;
	file_information.create_time = create_time;
	file_information.write_time = write_time;
	file_information.md5 = md5;
	file_information.sha2 = sha2;

	log_dbg "File hash registered. file=%ws",
		file_path
		log_end;
	return true;
}

/// @brief  
bool 
FileInfoCache::insert_file_info(
	_In_ const wchar_t* path, 
	_In_ uint64_t create_time,
	_In_ uint64_t write_time,
	_In_ uint64_t size,
	_In_ const char* md5_str,
	_In_ const char* sha2_str
    )
{
    _ASSERTE(NULL != path);
    _ASSERTE(NULL != md5_str);
    _ASSERTE(NULL != sha2_str);
	if (NULL == path) return false;
	if (NULL == md5_str) return false;
	if (NULL == sha2_str) return false;

    // run query
    bool ret = false;
    
	//
	// 파일 해시 테이블 대문자, 소문자 구별 없이 경로 확인을 위해 소문자로 입력 한다.
	//
	std::wstring pathl(path);
	to_lower_string<std::wstring>(pathl);

	try
    {
		//
		// _insert_cache_stmt에 바인딩 되어 있는 데이터 제거
		//
		_insert_cache_stmt->reset();


		//
		// 파일 정보(path, create_time, write_time, md5, sha1)캐시를 저장한다.
		//
		_insert_cache_stmt->bind(1, WcsToMbsUTF8Ex(pathl.c_str()).c_str());
		_insert_cache_stmt->bind(2, static_cast<long long>(create_time));
		_insert_cache_stmt->bind(3, static_cast<long long>(write_time));
		_insert_cache_stmt->bind(4, static_cast<long long>(size));
		_insert_cache_stmt->bind(5, md5_str);
		_insert_cache_stmt->bind(6, sha2_str);
		
		//
		// 파일의 정보가 테이블 내에 정상 적으로 입력이 되었다면 반환값이
		// 1개 이기 때문에 다음과 같이 처리 한다.
		//
		if (0 < _insert_cache_stmt->execDML())
		{
			InterlockedIncrement64(&_size);
			ret = true;
		}
    }
    catch (CppSQLite3Exception& e)
    {
		log_err
			"sqlite exception. FileInfoCache::insert_file_info, ecode = %d, emsg = %s",
			e.errorCode(),
			e.errorMessage()
		log_end;
    }

    return ret;
}


/// @brief  
bool
FileInfoCache::get_flie_info(
    _In_ const wchar_t * path, 
	_In_ uint64_t create_time,
	_In_ uint64_t write_time,
	_In_ uint64_t size,
    _Out_ std::string & md5_str, 
	_Out_ std::string & sha2_str
    )
{
    _ASSERTE(NULL != path);
	if (NULL == path) return false;

    bool ret = false;

	//
	// 파일 해시 테이블 대문자, 소문자 구별 없이 경로 확인을 위해 소문자로 입력 되어있다.
	// 그래서, 캐시 쿼리문 조건으로 경로를 소문자로 변환 후 쿼리문에서 사용 한다.
	//
	std::wstring pathl(path);
	to_lower_string<std::wstring>(pathl);

	// run query
    try
    {
		//
		// _select_cache_stmt에 바인딩 되어 있는 데이터 제거
		//
		_select_cache_stmt->reset();
		
		//
		// 파일 해시(md5, sha2)캐시를 읽어 온다.
		//
		_select_cache_stmt->bind(1, WcsToMbsUTF8Ex(pathl.c_str()).c_str());
		_select_cache_stmt->bind(2, static_cast<long long>(create_time));
		_select_cache_stmt->bind(3, static_cast<long long>(write_time));
		_select_cache_stmt->bind(4, static_cast<long long>(size));
		CppSQLite3Query rs = _select_cache_stmt->execQuery();

		//
		// 검색 결과가 없다면 함수를 빠져나간다.
		//
		if (true == rs.eof()) return ret;

		md5_str = rs.getStringField(0, "");
		sha2_str = rs.getStringField(1, "");
		
		int32_t hit_count = rs.getIntField(2);

		//
		// 파일 해시(md5, sha2)캐시 읽고 난 후 hit count를 증가 시킨 후
		// 테이블내의 값을 변경한다.
		//
		_update_cache_stmt->reset();

		_update_cache_stmt->bind(1, ++hit_count);
		_update_cache_stmt->bind(2, WcsToMbsUTF8Ex(pathl.c_str()).c_str());
		_update_cache_stmt->bind(3, static_cast<long long>(create_time));
		_update_cache_stmt->bind(4, static_cast<long long>(write_time));
		_update_cache_stmt->bind(5, static_cast<long long>(size));
		
		//
		// 찾고자 하는 파일의 정보가 테이블 내에 존재 하는 경우 쿼리 결과가
		// 1개 이기 때문에 다음과 같이 처리 한다.
		//
		if(0 < _update_cache_stmt->execDML())
		{
			InterlockedIncrement64(&_hit_count);
			ret = true;
		}
    }
    catch (CppSQLite3Exception& e)
    {
		log_err
			"sqlite exception. FileInfoCache::get_file_info, ecode = %d, emsg = %s",
			e.errorCode(),
			e.errorMessage()
		log_end;
    }

	return ret;
}

/// @brief 
bool 
FileInfoCache::file_util_get_hash(
	_In_ const wchar_t* file_path, 
	_Out_ std::string& md5, 
	_Out_ std::string& sha2)
{
	return get_file_hash_by_filepath(file_path, &md5, &sha2);	
}



// ============================================================================
//
//	C API
//
// ============================================================================

/// @brief 
bool fi_initialize()
{
	FileInfoCache* fi = Singleton<FileInfoCache>::GetInstancePointer();

	//
	//	cache 파일 경로는 `현재경로\ficache.db` 로 지정
	//
	std::wstringstream cache_path;
	cache_path << get_current_module_dirEx() << L"\\ficache.db";
	if (true != fi->initialize(cache_path.str().c_str(), 5000, false))
	{
		return false;
	}
	return true;
}

/// @brief 
void fi_finalize()
{
	Singleton<FileInfoCache>::ReleaseInstance();
}

/// @brief 
bool 
fi_get_file_information(
	_In_ const wchar_t* file_path,
	_Out_ FileInformation& file_information
	)
{
	std::unique_ptr<FileInfoCache, void(*)(_In_ FileInfoCache*)> fi(
		Singleton<FileInfoCache>::GetInstancePointer(),
		[](_In_ FileInfoCache*)
	{
		Singleton<FileInfoCache>::ReleaseInstance();
	});

	return fi.get()->get_file_information(file_path, file_information);
}
