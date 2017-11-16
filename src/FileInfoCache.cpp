/**
 * @file    FileInfoCache.h
 * @brief   
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2017/11/14 11:54 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"
#include "Singleton.h"
#include "FileInfoCache.h"
#include "md5.h"
#include "sha2.h"
#include "Win32Utils.h"

#define _create_file_cache \
                "CREATE TABLE file_hash ( "\
                "`id`	INTEGER PRIMARY KEY AUTOINCREMENT, "\
                "`path`	TEXT NOT NULL, "\
                "`create_time`	INTEGER NOT NULL, "\
                "`write_time`	INTEGER NOT NULL, "\
                "`size`	INTEGER NOT NULL, "\
                "`md5`	TEXT NOT NULL, "\
                "`sha2`	TEXT NOT NULL, "\
				"`hit_count` TIMESTAMP DEFAULT 1"\
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
// file cache�� ������ ����(�⺻��: 5000��, ����ڰ� size�� ���� �� ��� �޶� �� �� �ִ�)
// �̻��� ��� "hit_count"�� ��� ������ ���ڵ�� ���� �Ѵ�.
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
		// �����ͺ��̽� ������ ����, precompiled statement ���� �����Ѵ�.
		//

		_db.open(db_file_path);
		
		//
		// "file_hash" ���̺� ���� ������ üũ �� �� ������ ���� �����.
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
		//	statement ��ü���� �����Ѵ�. 
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

	// ĳ�� ������ �ʱ�ȭ
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
		// statement��ü���� �����Ѵ�.
		//
		delete _select_cache_stmt; _select_cache_stmt = nullptr;
		delete _insert_cache_stmt; _insert_cache_stmt = nullptr;
		delete _update_cache_stmt; _update_cache_stmt = nullptr;
		delete _delete_cache_stmt; _delete_cache_stmt = nullptr;
		//
		// db ������ ���� �Ѵ�.
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

	if (true != is_file_existsW(file_path))
	{
		//log_err "No file exists. file=%ws",
		//	file_path
		//	log_end;
		return false;
	}

	//
	//	ĳ�� ��ȸ�� ���� �⺻ ������ ���Ѵ�.
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
	//	���� ����� 0 �̸� �׳� ����
	//
	if (size == 0) return true;

	//
	//	1st phase, ĳ�ÿ��� ã�ƺ���. 
	//
	std::string md5;
	std::string sha2;


	// ĳ�� ������ ���� �Ѵٸ� ĳ�� ������ ��ȯ�Ѵ�.
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
	//	2nd phase, ���������� ���ϰ�, ĳ�ÿ� ����Ѵ�.
	// 
	if (true != file_util_get_hash(file_path, md5, sha2))
	{
		log_err "file_util_get_hash() failed. file=%ws",
			file_path
		log_end;
		return false;
	}

	//
	// ĳ�ÿ� ��� �ϱ��� ĳ�� ����� ������ ����(�⺻��: 5000��, 
	// ����ڰ� cache_size�� ���� �� ��� �޶� �� �� �ִ�) �� �ʰ��ϴ� ��� 
	// table���� "hit count"�� ����� ���� �� ��� ������ �����͵���
	// �����Ѵ�.
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
		// ������ ���ڵ� ���� �ɽ� ������� ū ��� 0���� �ʱ�ȭ ��Ų��.
		//

		if (_size < delete_record_count)
		{
			_ASSERTE(!"oops, deleted record is larger than cache size");
			InterlockedExchange64(&_size, 0);
		}

		//
		// ������ ���� ���� ���� ĳ�� ������� ������ ���ڵ� �� ��ŭ 
		// ���ش�.
		// =====================����===========================
		// ���� �ڵ忡�� �⺻ ĳ�� ������(5000��) �̻��� ���� ������
		// ĳ���� �Ǿ� ���ڵ尡 ���� �Ǵ� �ڵ带 �׽�Ʈ �غ��� ����
		// ������ ���� �α��ڵ带 ���� �ξ����� ���� ���� �� �� �ִ�.
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
	//	���������� �����Ѵ�.
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
	// ���� �ؽ� ���̺� �빮��, �ҹ��� ���� ���� ��� Ȯ���� ���� �ҹ��ڷ� �Է� �Ѵ�.
	//
	std::wstring pathl(path);
	to_lower_string<std::wstring>(pathl);

	try
    {
		//
		// _insert_cache_stmt�� ���ε� �Ǿ� �ִ� ������ ����
		//
		_insert_cache_stmt->reset();


		//
		// ���� ����(path, create_time, write_time, md5, sha1)ĳ�ø� �����Ѵ�.
		//
		_insert_cache_stmt->bind(1, WcsToMbsUTF8Ex(pathl.c_str()).c_str());
		_insert_cache_stmt->bind(2, static_cast<long long>(create_time));
		_insert_cache_stmt->bind(3, static_cast<long long>(write_time));
		_insert_cache_stmt->bind(4, static_cast<long long>(size));
		_insert_cache_stmt->bind(5, md5_str);
		_insert_cache_stmt->bind(6, sha2_str);
		
		//
		// ������ ������ ���̺� ���� ���� ������ �Է��� �Ǿ��ٸ� ��ȯ����
		// 1�� �̱� ������ ������ ���� ó�� �Ѵ�.
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
	// ���� �ؽ� ���̺� �빮��, �ҹ��� ���� ���� ��� Ȯ���� ���� �ҹ��ڷ� �Է� �Ǿ��ִ�.
	// �׷���, ĳ�� ������ �������� ��θ� �ҹ��ڷ� ��ȯ �� ���������� ��� �Ѵ�.
	//
	std::wstring pathl(path);
	to_lower_string<std::wstring>(pathl);

	// run query
    try
    {
		//
		// _select_cache_stmt�� ���ε� �Ǿ� �ִ� ������ ����
		//
		_select_cache_stmt->reset();
		
#pragma todo("���� ����(���) ���� ���� �̽�")
//	- ĳ�� ���� �� path �����͸� ����� "string" ���·�
//    �ְ� ������ ����(���� �̽��� �߻� �Ͽ�����) path��
//    "hash"�� ���� �Ͽ� ĳ�� �� ����Ʈ�Ͽ� ����Ѵ�.

		//
		// ���� �ؽ�(md5, sha2)ĳ�ø� �о� �´�.
		//
		_select_cache_stmt->bind(1, WcsToMbsUTF8Ex(pathl.c_str()).c_str());
		_select_cache_stmt->bind(2, static_cast<long long>(create_time));
		_select_cache_stmt->bind(3, static_cast<long long>(write_time));
		_select_cache_stmt->bind(4, static_cast<long long>(size));
		CppSQLite3Query rs = _select_cache_stmt->execQuery();

		//
		// �˻� ����� ���ٸ� �Լ��� ����������.
		//
		if (true == rs.eof()) return ret;

		md5_str = rs.getStringField(0, "");
		sha2_str = rs.getStringField(1, "");
		
		int32_t hit_count = rs.getIntField(2);

		//
		// ���� �ؽ�(md5, sha2)ĳ�� �а� �� �� hit count�� ���� ��Ų ��
		// ���̺��� ���� �����Ѵ�.
		//
		_update_cache_stmt->reset();

		_update_cache_stmt->bind(1, ++hit_count);
		_update_cache_stmt->bind(2, WcsToMbsUTF8Ex(pathl.c_str()).c_str());
		_update_cache_stmt->bind(3, static_cast<long long>(create_time));
		_update_cache_stmt->bind(4, static_cast<long long>(write_time));
		_update_cache_stmt->bind(5, static_cast<long long>(size));
		
		//
		// ã���� �ϴ� ������ ������ ���̺� ���� ���� �ϴ� ��� ���� �����
		// 1�� �̱� ������ ������ ���� ó�� �Ѵ�.
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
	handle_ptr file_handle(
		CreateFileW(file_path,
					GENERIC_READ,
					FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL),
		[](HANDLE h) 
		{
			if (INVALID_HANDLE_VALUE != h)
			{
				CloseHandle(h);
			}
		});

    if (INVALID_HANDLE_VALUE == file_handle.get())
    {
        log_err
            "CreateFileW() failed. path=%ws, gle = %u",
            file_path, 
			GetLastError()
            log_end;
        return false; 
    }
    
    if (INVALID_SET_FILE_POINTER == SetFilePointer(file_handle.get(), 
												   0, 
												   NULL, 
												   FILE_BEGIN))
    {
        log_err
            "SetFilePointer() failed. path=%ws, gle = %u", 
			file_path, 
			GetLastError()
            log_end;
        return false;
    }

    uint8_t sha2_buf[32];
    MD5_CTX ctx_md5;
    sha256_ctx ctx_sha2;
    MD5Init(&ctx_md5, 0);
    sha256_begin(&ctx_sha2);

    const uint32_t read_buffer_size = 4096;
    uint8_t read_buffer[read_buffer_size];
    DWORD read = read_buffer_size;

    while (read_buffer_size == read)
    {
		if (FALSE == ::ReadFile(file_handle.get(),
								read_buffer,
								read_buffer_size,
								&read,
								NULL))
        {
            log_err
                "ReadFile() failed. path=%ws, gle = 0x%08x",
                file_path,
                GetLastError()
                log_end;
            return false;
        }

        if (0 != read)
        {
            MD5Update(&ctx_md5, read_buffer, read);
            sha256_hash(read_buffer, read, &ctx_sha2);
        }
    }

    MD5Final(&ctx_md5);
    sha256_end(sha2_buf, &ctx_sha2);

	//
	//	Hash ���̳ʸ� ���۸� hex ���ڿ��� ��ȯ
	//
	if (true != bin_to_hexa_fast(sizeof(ctx_md5.digest),
								 ctx_md5.digest,
								 false,
								 md5))
	{
		log_err "bin_to_hexa_fast() failed. " log_end;
		return false;
	}

	if (true != bin_to_hexa_fast(sizeof(sha2_buf), 
								 sha2_buf, 
								 false, 
								 sha2))
	{
		log_err "bin_to_hexa_fast() failed. " log_end;
		return false;
	}
    return true;
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
	//	cache ���� ��δ� `������\ficache.db` �� ����
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
