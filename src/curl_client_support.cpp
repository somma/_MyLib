/**
 * @file    curl_client_support.cpp
 * @brief	This module implement support modules for curl_client class.
 *
 * @author  somma (somma@somma.kr)
 * @date    2020/03/01 20:30 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"
#include "_MyLib/src/curl_client_support.h"
#include "_MyLib/src/Win32Utils.h"
#include "_MyLib/src/log.h"


/// @brief	Constructor
http_download_ctx::http_download_ctx(const char* const url)
	:
	_url(nullptr == url ? _null_stringa : url),
	_file_handle(INVALID_HANDLE_VALUE)
{
	_ASSERTE(nullptr != url);
}

/// @brief	
http_download_ctx::~http_download_ctx()
{
	finalize();
}

/// @brief	
bool http_download_ctx::initialize(_In_ const wchar_t* const file_path)
{
	_ASSERTE(nullptr != file_path);
	_ASSERTE(INVALID_HANDLE_VALUE == _file_handle);

	if (nullptr == file_path) { return false; }
	if (INVALID_HANDLE_VALUE != _file_handle) return false;

	//
	//	로컬 파일 생성
	//
	_file_path = file_path;
	_file_handle = open_file_to_write(file_path);
	if (INVALID_HANDLE_VALUE == _file_handle)
	{
		log_err
			"CreateFile() failed. path=%ws, gle=%u",
			file_path,
			GetLastError()
			log_end;
		return false;
	}

	return true;
}

/// @brief	
bool 
http_download_ctx::update(
	_In_ const void* const buffer, 
	_In_ const size_t cb_buffer
)
{
	if (INVALID_HANDLE_VALUE == _file_handle) return false;

	//
	//	Write to file
	//
	DWORD bytes_written = 0;
	if (!WriteFile((HANDLE)_file_handle,
					buffer,
					cb_buffer,
					&bytes_written,
					nullptr))
	{
		log_err "WriteFile() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	//
	//	NOTE
	//
	//	원래의 구현의도는 현재 루틴에서 전달받은 버퍼를 통해 해시값을 
	//	같이 계산하는 것이었으나, 다운로드 시간이 굉장히 길어지는 이슈가
	//	발생해서, 여기서는 파일로 저장만하고, 해시값을 계산하는 것은 
	//	별도의 메소드에서 구현한다. 
	//	

	return true;
}

/// @brief	다운로드한 파일의 MD5 해시 값을 구한다.
bool http_download_ctx::get_md5(_Out_ std::string& value)
{
	if (INVALID_HANDLE_VALUE == _file_handle) return false;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(_file_handle,
												   0,
												   nullptr,
												   FILE_BEGIN))
	{
		log_err
			"SetFilePointer() failed. path=%ws, gle = %u",
			_file_path.c_str(),
			GetLastError()
			log_end;
		return false;
	}
	
	return ::get_md5(_file_handle, value);
}

/// @brief	다운로드한 파일의 SHA2 해시 값을 구한다.
bool http_download_ctx::get_sha2(_Out_ std::string& value)
{
	if (INVALID_HANDLE_VALUE == _file_handle) return false;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(_file_handle,
												   0,
												   nullptr,
												   FILE_BEGIN))
	{
		log_err
			"SetFilePointer() failed. path=%ws, gle = %u",
			_file_path.c_str(),
			GetLastError()
			log_end;
		return false;
	}

	return ::get_sha2(_file_handle, value);
}


/// @brief	
bool
http_download_ctx::finalize()
{
	if (INVALID_HANDLE_VALUE != _file_handle)
	{
		CloseHandle(_file_handle);
		_file_handle = INVALID_HANDLE_VALUE;
	}

	return true;
}

///	@brief	libcUrl Write Callback To Stream 
///			데이터 수신시 호출되는 콜백 함수로 fread() 함수 처럼 구조화된 
///			IO 를 위해 size, nmemb 를 사용함
///	@param	ptr         수신한 데이터
///	@param	size        nmemb(메모리 블럭) 의 사이즈 / 엘리먼트 사이즈
///	@param	nmemb       메모리 블럭의 갯수 / 엘리먼크 갯수
///	@param	userdata    CURLOPT_WRITEDATA 옵션 설정 시 설정한 데이터 포인터
///	@return	처리된 byte 수, size * nmemb 값과 다른 값이 리턴되면 curl 이 
///			세션을 중지한다.
size_t
curl_wcb_to_stream(
	_In_ void* ptr,
	_In_ size_t size,
	_In_ size_t nmemb,
	_In_ void* userdata
)
{
	_ASSERTE(NULL != ptr);
	_ASSERTE(NULL != userdata);
	if (NULL == ptr || NULL == userdata) return 0;

	UINT32 DataSizeToWrite = (UINT32)(size * nmemb);
	CMemoryStream* stream = (CMemoryStream*)userdata;

	if (-1 == stream->WriteToStream((char*)ptr, DataSizeToWrite))
	{
		log_err "Can not write to memory stream" log_end;
		return 0;
	}
	return (size * nmemb);
}

///	@brief	libcUrl Write Callback To file 
size_t
curl_wcb_to_ctx(
	_In_ void* rcvd_buffer,
	_In_ size_t rcvd_block_size,
	_In_ size_t rcvd_block_count,
	_In_ void* userdata
)
{
	_ASSERTE(nullptr != rcvd_buffer);
	_ASSERTE(rcvd_block_size > 0);
	_ASSERTE(rcvd_block_count > 0);
	_ASSERTE(userdata != nullptr);

	if (nullptr == rcvd_buffer ||
		0 == rcvd_block_size ||
		0 == rcvd_block_count ||
		userdata == nullptr)
	{
		return 0;
	}

	phttp_download_ctx ctx = (phttp_download_ctx)userdata;

	if (!ctx->update(rcvd_buffer, rcvd_block_size * rcvd_block_count))
	{
		log_err "ctx->update() failed." log_end;
		return 0;
	}

	return (rcvd_block_size * rcvd_block_count);
}