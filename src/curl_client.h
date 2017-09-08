/**
 * @file    curl_client.h
 * @brief   
 *
 * @author  Jungil, Yang (chaserhunter@somma.kr)
 * @date    2017/09/07 16:30 created.
 * @copyright (C)SOMMA, INC. All rights reserved.
**/

#pragma once
#include <Windows.h>
#include "CStream.h"

#define CURL_STATICLIB
#include "curl/curl.h"
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wldap32.lib")

typedef class curl_client
{

public:
	curl_client():
		_curl(nullptr)
	{
	}

	virtual ~curl_client() { finalize(); }

public:
	bool initialize();
	void finalize();

	bool http_post(_In_ const char* url, 
				   _In_ const std::string post_data, 
				   _Out_ std::string& response);

private:
	static size_t http_get_write_callback(_In_ void* ptr, 
										  _In_ size_t size, 
										  _In_ size_t nmemb, 
										  _In_ void* userdata);
	bool set_common_opt(_In_ const char* url, 
						_Out_ CMemoryStream &stream);
	bool perform();

private:
	CURL* _curl;

} *pcurl_client;