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
http_download_ctx::http_download_ctx(
	_In_ const char* url,
	_In_ const HANDLE file_handle
)
	:
	_cancel(false),
	_url(url),
	_file_handle(file_handle)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(INVALID_HANDLE_VALUE != file_handle);
}

/// @brief	
http_download_ctx::~http_download_ctx()
{
	_url = _null_stringa;
	_file_handle = INVALID_HANDLE_VALUE;
}

/// @brief	
bool 
http_download_ctx::update(
	_In_ const void* const buffer, 
	_In_ const size_t cb_buffer
)
{
	if (_cancel)
	{
		log_info
			"Download request canceled. url=%s, file=0x%p",
			_url.c_str(),
			_file_handle			
			log_end;
		return false;
	}

	if (INVALID_HANDLE_VALUE == _file_handle) return false;

	//
	//	Write to file
	//
	DWORD bytes_written = 0;
	if (!WriteFile((HANDLE)_file_handle,
					buffer,
					(DWORD)cb_buffer,
					&bytes_written,
					nullptr))
	{
		log_err "WriteFile() failed. gle=%u, file=0x%p",
			GetLastError(), 
			_file_handle
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

	FlushFileBuffers(_file_handle);
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
			"SetFilePointer() failed. file=0x%p, gle=%u",
			_file_handle,
			GetLastError()
			log_end;
		return false;
	}
	
	return get_file_hash_by_filehandle(_file_handle,
									   &value,
									   nullptr);
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
			"SetFilePointer() failed. file=0x%p, gle=%u",
			_file_handle,
			GetLastError()
			log_end;
		return false;
	}

	return get_file_hash_by_filehandle(_file_handle,
									   nullptr,
									   &value);
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