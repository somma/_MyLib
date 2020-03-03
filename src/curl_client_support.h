/**
 * @file    curl_client_support.h
 * @brief	This module implement support modules for curl_client class.
 *
 * @author  somma (somma@somma.kr)
 * @date    2020/03/01 20:30 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#pragma once
#include "_MyLib/src/BaseWindowsHeader.h"
#include "_MyLib/src/CStream.h"
#include "_MyLib/src/md5.h"
#include "_MyLib/src/sha2.h"


/// @brief	Context that holds file-downlaod related stuff
typedef class http_download_ctx
{
public:
	http_download_ctx(const char* const url);
	~http_download_ctx();

	bool initialize(
		_In_ const wchar_t* const file_path);

	bool update(
		_In_ const void* const buffer, 
		_In_ const size_t cb_buffer);

	bool get_md5(_Out_ std::string& value);
	bool get_sha2(_Out_ std::string& value);

	bool finalize();

public:
	std::string _url;
	std::wstring _file_path;
	HANDLE _file_handle;

}*phttp_download_ctx;

size_t
curl_wcb_to_stream(
	_In_ void* ptr,
	_In_ size_t size,
	_In_ size_t nmemb,
	_In_ void* userdata
);

size_t
curl_wcb_to_ctx(
	_In_ void* rcvd_buffer,
	_In_ size_t rcvd_block_size,
	_In_ size_t rcvd_block_count,
	_In_ void* userdata);


