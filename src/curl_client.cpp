﻿/**
* @file    curl_client.cpp
* @brief
*
* @author  Jungil, Yang (chaserhunter@somma.kr)
* @date    2017/09/07 16:30 created.
* @copyright (C)Somma, Inc. All rights reserved.
**/

#include "stdafx.h"
#include "curl_client.h"
#include "log.h"

/// @brief
curl_client::curl_client() :
	_curl(nullptr)
{
}

curl_client::~curl_client()
{
	finalize();
}

///	@brief	libcUrl 에서 데이터 수신시 호출되는 콜백 함수
///	@param	ptr         수신한 데이터
///	@param	size        nmemb(메모리 블럭) 의 사이즈 / 엘리먼트 사이즈
///	@param	nmemb       메모리 블럭의 갯수 / 엘리먼크 갯수
///	@param	userdata    CURLOPT_WRITEDATA 옵션 설정 시 설정한 데이터 포인터
///	@remarks	fread (void * DstBuf, size_t ElementSize, size_t Count, FILE * FileStream) 함수 처럼
///			구조화된 IO 를 위해 size, nmemb 를 사용함
///	@return	처리된 byte 수. ( size * nmemb ) 값과 다른 값이 리턴되면 curl 이 세션을 중지함
size_t
http_get_write_callback(
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
	CStream* stream = (CStream*)userdata;

	if (-1 == stream->WriteToStream(ptr, DataSizeToWrite))
	{
		log_err "Can not write to memory stream" log_end;
		return 0;
	}
	return (size * nmemb);
}

/// @brief
bool curl_client::initialize()
{
	//	Get a curl object
	_curl = curl_easy_init();
	if (nullptr == _curl)
	{
		log_err "curl_easy_init() failed." log_end
		return false;
	}

	return true;
}

/// @brief
void curl_client::finalize()
{
	curl_easy_cleanup(_curl);
}

///	@brief	http post 요청을 수행한다.
/// @param	url				전송할 서버 주소
///	@param	post_data		전송할 데이터
///	@param	response		response data 를 저장하기위한 자료구조
///	@return	성공시 true, 실패시 false
bool
curl_client::http_post(
	_In_ const char* url,
	_In_ const std::string& post_data,
	_Out_ std::string& response
)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(true != post_data.empty());
	_ASSERTE(NULL != _curl);
	if (nullptr == url || true == post_data.empty() || nullptr == _curl)
	{
		return false;
	}

	CMemoryStream stream;
	CURLcode curl_code = CURLE_OK;

	//	Set Common Opt
	if (true != set_common_opt(url, stream))
	{
		log_err "set_common_opt() failed." log_end;
		return false;
	}

	//	Set HTTP method to POST
	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_POST,
								 1);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	//	sending data
	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_POSTFIELDSIZE,
								 post_data.length());
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_POSTFIELDS,
								 post_data.c_str());
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	//	perform
	if (true != perform())
	{
		log_err "perform() failed." log_end;
		return false;
	}

	//	response data 에 null terminate 를 추가한다.
	char nt = '\0';
	stream.WriteToStream(&nt, sizeof(char));

	response = (char*)stream.GetMemory();
	stream.ClearStream();
	return true;
}



/// @brief	전송을 하기위한 기본 options을 설정한다.
bool
curl_client::set_common_opt(
	_In_ const char* url,
	_Out_ CMemoryStream &stream
	)
{
	_ASSERTE(NULL != url);
	if (NULL == url) return false;

	CURLcode curl_code = CURLE_OK;

	//	Set url
	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_URL,
								 url);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	//	Set callback
	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_WRITEFUNCTION,
								 http_get_write_callback);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	//	Set receiving data
	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_WRITEDATA,
								 &stream);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

#ifdef _DEBUG 
	//	get verbose debug output please
	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_VERBOSE,
								 1L);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}
#endif

	return true;
}

/// @brief	전송을 실행하고, 문제가 없는지 확인한다.
bool
curl_client::perform()
{
	CURLcode curl_code = CURLE_OK;

	//	Execute
	curl_code = curl_easy_perform(_curl);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_perform() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;

		return false;
	}

	//	Get the last response code.
	long http_code = 0;
	curl_easy_getinfo(_curl,
					  CURLINFO_RESPONSE_CODE,
					  &http_code);
	if (http_code != 200)
	{
		log_err "http request failed. response code = %u",
			http_code
			log_end;

		return false;
	}

	return true;
}