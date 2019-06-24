/**
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
	_curl(nullptr),
	_header_items(nullptr)
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

bool
curl_client::initialize(
	_In_ const char* http_header,
	_In_ long connection_timeout, 
	_In_ long read_timeout,
	_In_ long ssl_verifypeer
	)
{
	//	Get a curl object
	_curl = curl_easy_init();
	if (nullptr == _curl)
	{
		log_err "curl_easy_init() failed." log_end
			return false;
	}

	set_common_opt(http_header, connection_timeout, read_timeout, ssl_verifypeer);

	return true;
}

/// @brief
void curl_client::finalize()
{
	if (nullptr != _header_items)
	{
		curl_slist_free_all(_header_items);
	}
	curl_easy_cleanup(_curl);
}

/// @brief	
bool
curl_client::http_get(
	_In_ const char* url,
	_Out_ CMemoryStream& stream
)
{
	_ASSERTE(nullptr != url);
	if (nullptr == url) return false;

	_ASSERTE(NULL != _curl);
	if (nullptr == url || nullptr == _curl)
	{
		log_err "curl_client object is not initialized." log_end;
		return false;
	}

	// 우선 스트림을 초기화 한다. 
	stream.ClearStream();

	//	Set url
	CURLcode curl_code = curl_easy_setopt(_curl,
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

	// perform
	if (true != perform())
	{
		log_err "perform() failed." log_end;
		return false;
	}

	return true;
}

/// @brief	http get 요청을 수행한다.
/// @param	url			전송할 서버 주소
/// @param	response	response data를 저장하기 위한 자료구조
/// @return	성공시 true, 실패시 false
bool
curl_client::http_get(
	_In_ const char* url,
	_Out_ std::string& response
	)
{
	_ASSERTE(nullptr != url);
	if (nullptr == url) return false;

	CMemoryStream stream;
	if (true != http_get(url, stream))	
	{
		log_err "http_get() failed. url=%s", url log_end;
		return false;
	}

	// response data 에 null terminate 를 추가하고
	// Stream 을 string  으로 변환 후 리턴한다.
	char nt = '\0';
	stream.WriteToStream(&nt, sizeof(char));
	response = (char*)stream.GetMemory();
	stream.ClearStream();
	
	return true;
}


///	@brief	http post 요청을 수행한다.
/// @param	url				전송할 서버 주소
///	@param	post_data		전송할 데이터
///	@param	response		response data 를 저장하기위한 자료구조
///	@return	성공시 true, 실패시 false
bool
curl_client::http_post(
	_In_  const char* url,
	_In_  const std::string& post_data,
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
	_In_ const char* http_header,
	_In_ long connection_timeout,
	_In_ long read_timeout,
	_In_ long ssl_verifypeer
	)
{
	CURLcode curl_code = CURLE_OK;

	bool ret = false;
	do
	{
		//
		// Custom HTTP Header가 있는 경우 Header 설정을 추가한다.
		//
		if (strcmp(http_header, _null_http_header_string) != 0)
		{
			_header_items = curl_slist_append(_header_items, http_header);
			_ASSERTE(nullptr != _header_items);

			curl_code = curl_easy_setopt(_curl,
										 CURLOPT_HTTPHEADER,
										 _header_items);
			if (CURLE_OK != curl_code)
			{
				log_err "curl_easy_setopt() failed. curl_code = %d, %s",
					curl_code,
					curl_easy_strerror(curl_code)
					log_end;
				return false;
			}
		}

		//
		// SSL Verifyer 유무 설정
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_SSL_VERIFYPEER,
									 ssl_verifypeer);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
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
			break;
		}

		// Set timeout
		// 목적지 서버상태가 Overload 상태인지, 또는 크리티컬한 에러가 있는 상태인지에 따라 적용
		// e.g. 설정한 시간(10초)내에 서버에서 응답이 없을 경우 강제 해제
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_CONNECTTIMEOUT,
									 connection_timeout);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		// Set connect timeout
		// 굉장히 큰파일, 또는 느린 연결 속도(네트워크속도에 좌우됨) 등에 따라 적용
		// e.g. 설정한 시간(90초) 내에 다운로드가 완료되지 않을 경우 강제 해제
		curl_code = curl_easy_setopt(_curl,
									CURLOPT_TIMEOUT,
									read_timeout);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
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
			break;
		}
#endif
		ret = true;
	} while (false);

	return ret;
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
	if (CURLE_OK != curl_easy_getinfo(_curl,
									  CURLINFO_RESPONSE_CODE,
									  &http_code))
	{
		log_err "curl_easy_getinfo() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;

		return false;
	}

	if (http_code != 200)
	{
		log_err "http request failed. response code = %u",
			http_code
			log_end;

		return false;
	}

	return true;
}