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

#define _create_file_cache \
                "CREATE TABLE file_hash ( "\
                "`id`	INTEGER PRIMARY KEY AUTOINCREMENT, "\
                "`path`	TEXT NOT NULL, "\
                "`create_time`	INTEGER NOT NULL, "\
                "`write_time`	INTEGER NOT NULL, "\
                "`size`	INTEGER NOT NULL, "\
                "`md5`	TEXT NOT NULL, "\
                "`sha2`	TEXT NOT NULL "\
                ") "

#define _insert_file_cache \
                L"INSERT INTO file_hash "\
                L"( "\
                L" path,  "\
                L" create_time, "\
                L" write_time, "\
                L" size, "\
                L" md5, "\
                L" sha2  "\
                L")  "\
                L"VALUES "\
                L"( "\
                L" \"%ws\", "\
                L" %llu, "\
                L" %llu, "\
                L" %llu, "\
                L" \"%S\", "\
                L" \"%S\" "\
                L") "

#define _select_file_cache \
                L"SELECT md5, sha2 FROM file_hash "\
                L"WHERE "\
                L" path = \"%ws\" AND "\
                L" create_time = %llu AND "\
                L" write_time = %llu AND "\
                L" size = %llu; "

/// @brief  constructor
FileInfoCache::FileInfoCache() : _initialized(false), _size(0), _hit_count(0)
{
}

/// @brief  
FileInfoCache::~FileInfoCache()
{
    finalize();
}

/// @brief  
bool FileInfoCache::initialize(const wchar_t* db_file_path, bool delete_if_exist)
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

    _db.open(db_file_path);
    try
    {
		if (true != _db.tableExists("file_hash"))
		{
			_db.execDML(_create_file_cache);
		}
    }
    catch (CppSQLite3Exception& e)
    {
        log_err
            "_db.execDML() failed. dml = %s, ecode = %d, emsg = %s", 
            _create_file_cache,
            e.errorCode(),
            e.errorMessage()
            log_end;

        _db.close();
        return false;
    }

    _initialized = true;
    return true;
}

/// @brief  
void FileInfoCache::finalize()
{
    if (true != _initialized) return;

    try
    {
        _db.close();
        _initialized = false;
    }
    catch (CppSQLite3Exception& e)
    {
        log_err
            "_db.close() failed. ecode = %d, emsg = %s",
            e.errorCode(),
            e.errorMessage()
            log_end
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
    const wchar_t * path, 
    uint64_t create_time, 
    uint64_t write_time, 
    uint64_t size, 
    const char * md5_str, 
    const char * sha2_str
    )
{
    _ASSERTE(NULL != path);
    _ASSERTE(NULL != md5_str);
    _ASSERTE(NULL != sha2_str);

    wchar_t buf[2048];

    bool free_pbuf = false;
    wchar_t* pbuf = buf;
    uint32_t buf_len = sizeof(buf);

    // build query string
    for (;;) 
    {
        HRESULT hr = StringCbPrintfW(
                        pbuf, 
                        buf_len, 
                        _insert_file_cache, 
                        path, 
                        create_time, 
                        write_time, 
                        size,
                        md5_str, 
                        sha2_str);        
        if (TRUE != SUCCEEDED(hr))
        {
            if (hr == STRSAFE_E_INSUFFICIENT_BUFFER)
            {
                buf_len += 1024;

                if (true != free_pbuf)
                {
                    free_pbuf = true;
                }
                else
                {
                    free(pbuf); pbuf = NULL;
                }

                pbuf = (wchar_t*) malloc(buf_len);
                if (NULL == pbuf)
                {
                    log_err "malloc(%u) for query buffer failed.", buf_len log_end;
                    break;
                }                
                continue;       // retry with new and big buffer
            }
            else
            {
                log_err "StringCbPrintfW( insert_file_hash query ) failed. hr = %u", hr log_end;
                break;
            }
        }
        else
        {
            break;  // ok!
        }
    }
    
    // run query
    bool ret = false;
    try
    {
        if (0 < _db.execDML(WcsToMbsEx(pbuf).c_str()))
        {
            InterlockedIncrement(&_size);
            ret = true;
        }
    }
    catch (CppSQLite3Exception& e)
    {
        log_err
            "_db.execDML() failed. dml = %ws, ecode = %d, emsg = %s",
            pbuf,
            e.errorCode(),
            e.errorMessage()
            log_end
    }

    if (free_pbuf) free_and_nil(pbuf);
    return ret;
}

/// @brief  
bool 
FileInfoCache::get_flie_info(
    const wchar_t * path, 
    uint64_t create_time, 
    uint64_t write_time, 
    uint64_t size, 
    std::string & md5_str, 
    std::string & sha2_str
    )
{
    _ASSERTE(NULL != path);

    wchar_t buf[2048];

    bool free_pbuf = false;
    wchar_t* pbuf = buf;
    uint32_t buf_len = sizeof(buf);

    // build query string
    for (;;)
    {
        HRESULT hr = StringCbPrintfW(pbuf, 
									 buf_len, 
									 _select_file_cache, 
									 path, 
									 create_time, 
									 write_time, 
									 size);
        if (TRUE != SUCCEEDED(hr))
        {
            if (hr == STRSAFE_E_INSUFFICIENT_BUFFER)
            {
                buf_len += 1024;

                if (true != free_pbuf)
                {
                    free_pbuf = true;
                }
                else
                {
                    free(pbuf); pbuf = NULL;
                }

                pbuf = (wchar_t*)malloc(buf_len);
                if (NULL == pbuf)
                {
                    log_err "malloc(%u) for query buffer failed.", buf_len log_end;
                    break;
                }

                continue;       // retry with new and big buffer
            }
            else
            {
                log_err "StringCbPrintfW( insert_file_hash query ) failed. hr = %u", hr log_end;
                break;
            }
        }
        else
        {
            break;  // ok!
        }
    }

    // run query
    bool ret = false;
    try
    {
        CppSQLite3Query rs = _db.execQuery(WcsToMbsEx(pbuf).c_str());

        if (true == rs.eof()) return false;

        md5_str = rs.getStringField(0, "");
        sha2_str = rs.getStringField(1, "");
        //log_dbg "cache hit. path = %ws", path log_end;
        InterlockedIncrement(&_hit_count);

        ret = true;
    }
    catch (CppSQLite3Exception& e)
    {
        log_err
            "_db.execQuery() failed. dml = %ws, ecode = %d, emsg = %s",
            pbuf,
            e.errorCode(),
            e.errorMessage()
            log_end
    }

    if (free_pbuf) free_and_nil(pbuf);
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
	//	Hash 바이너리 버퍼를 hex 문자열로 변환
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
	//	cache 파일 경로는 `현재경로\ficache.db` 로 지정
	//
	std::wstringstream cache_path;
	cache_path << get_current_module_dirEx() << L"\\ficache.db";
	if (true != fi->initialize(cache_path.str().c_str(), false))
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
