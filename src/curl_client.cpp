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
	_connection_timeout(0),
	_read_timeout(0),
	_ssl_verifypeer(0)
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
	CMemoryStream* stream = (CMemoryStream*)userdata;

	if (-1 == stream->WriteToStream(ptr, DataSizeToWrite))
	{
		log_err "Can not write to memory stream" log_end;
		return 0;
	}
	return (size * nmemb);
}

bool
curl_client::initialize(
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

	_connection_timeout = connection_timeout;
	
	_read_timeout = read_timeout;
	
	_ssl_verifypeer = ssl_verifypeer;
	
	return true;
}

/// @brief
void curl_client::finalize()
{
	curl_easy_cleanup(_curl);

	// curl_global_cleanup() 호출은 thread safe 하지 않기때문에
	// 라이브러리 내에서 호출하면 알아채기 어려운 버그를 만들어낼 수 
	// 있다. 이 함수는 application 레벨에서 종료 직전에 호출하는걸로.
	// 아니면 어차피 application 종료되면 알아서 문제 해결됨
	//
	//curl_global_cleanup();
}

/// @brief
void
curl_client::append_header(
	_In_z_ const char* key, 
	_In_z_ const char* value
	)
{
	_ASSERTE(nullptr != _curl);
	_ASSERTE(nullptr != key);
	_ASSERTE(nullptr != value);
	if (nullptr == _curl || nullptr == key || nullptr == value) return;

	_header_fields[key] = value;
}

/// @brief	
bool
curl_client::http_get(
	_In_ const char* url,
	_Out_ long& http_response_code,
	_Out_ CMemoryStream& stream
)
{
	_ASSERTE(nullptr != _curl);
	_ASSERTE(nullptr != url);
	if (nullptr == url || nullptr == _curl) return false;

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
	if (true != perform(http_response_code))
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
	_Out_ long& http_response_code,
	_Out_ std::string& response
	)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(nullptr != _curl);
	if (nullptr == url || nullptr == _curl) return false;

	CMemoryStream stream;
	if (true != http_get(url, http_response_code, stream))	
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
	_In_z_ const char* url,
	_In_z_ const char* data,
	_Out_  long& http_response_code,
	_Out_  std::string& response
	)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(nullptr != data);
	_ASSERTE(NULL != _curl);
	if (nullptr == url || nullptr == data || nullptr == _curl) return false;

	CMemoryStream stream;
	if (true != http_post(url, data, http_response_code, stream))
	{
		log_err "http_post() failed. url=%s", url log_end;
		return false;
	}

	//	response data 에 null terminate 를 추가한다.
	char nt = '\0';
	stream.WriteToStream(&nt, sizeof(char));

	response = (char*)stream.GetMemory();
	stream.ClearStream();
	return true;
}

bool
curl_client::http_file_upload(
	_In_z_ const char* url,
	_In_z_ const wchar_t* file_path,
	_In_   Forms& forms,
	_Out_  long& http_response_code,
	_Out_  CMemoryStream& stream
	)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(nullptr != file_path);
	_ASSERTE(NULL != _curl);
	if (nullptr == url || nullptr == file_path || nullptr == _curl) return false;

	if (true != is_file_existsW(file_path))
	{
		log_err "The file to be transferred does not exist. path=%ws",
			file_path
			log_end;
		return false;
	}

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

	//	perform
	std::string file_path_utf8 = WcsToMbsUTF8Ex(file_path);
	if (true != perform(file_path_utf8.c_str(), forms, http_response_code))
	{
		log_err "perform() failed." log_end;
		return false;
	}

	return true;
}

bool 
curl_client::http_file_upload(
	_In_z_ const char* url,
	_In_z_ const wchar_t* file_path,
	_In_   Forms& forms,
	_Out_  long& http_response_code,
	_Out_  std::string& response)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(nullptr != file_path);
	_ASSERTE(NULL != _curl);
	if (nullptr == url || nullptr == file_path || nullptr == _curl) return false;

	if (true != is_file_existsW(file_path))
	{
		log_err "The file to be transferred does not exist. path=%ws",
			file_path
			log_end;
		return false;
	}

	CMemoryStream stream;
	if (true != http_file_upload(url, file_path, forms, http_response_code, stream))
	{
		log_err "http_post() failed. url=%s", url log_end;
		return false;
	}

	//	response data 에 null terminate 를 추가한다.
	char nt = '\0';
	stream.WriteToStream(&nt, sizeof(char));

	response = (char*)stream.GetMemory();
	stream.ClearStream();
	return true;
}

///
///
///
///
bool
curl_client::http_post(
	_In_z_ const char* url,
	_In_z_ const char* data,
	_Out_ long& http_response_code,
	_Out_ CMemoryStream& stream
	)
{
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
								 strlen(data));
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
								 data);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	//	perform
	if (true != perform(http_response_code))
	{
		log_err "perform() failed." log_end;
		return false;
	}

	return true;
}

/// @brief	전송을 하기위한 기본 options을 설정한다.
bool
curl_client::set_common_opt(
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
curl_client::perform(
	_Out_ long& http_response_code
	)
{
	CURLcode curl_code = CURLE_OK;

	// Set HTTP Headers
	curl_slist* header_list = nullptr;
	if (true != _header_fields.empty())
	{
		std::string header_item;
		for (auto hf : _header_fields)
		{
			header_item = hf.first;
			header_item += ":";
			header_item += hf.second;
			header_list = curl_slist_append(header_list, header_item.c_str());
		}

		curl_code = curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, header_list);
		if (CURLE_OK != curl_code)
		{
			log_err "set http header failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			return false;
		}

		// 설정 완료된 HTTP Header 필드들은 정리한다.
		_header_fields.clear();
	}
	
	set_common_opt(_connection_timeout, _read_timeout, _ssl_verifypeer);

	//	Execute
	curl_code = curl_easy_perform(_curl);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_perform() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		//
		// 요청 실패한 경우 HTTP 응답값을 0으로 반환한다.
		//
		http_response_code = 0;
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
		//
		// HTTP 응답값을 가져오지 못한 경우 0으로 반환한다.
		//
		http_response_code = 0;
		return false;
	}
	http_response_code = http_code;

	// HTTP Headers 정리
	if (nullptr != header_list)
	{
		curl_slist_free_all(header_list);
		header_list = nullptr;
	}

	// reset curl handle
	curl_easy_reset(_curl);
	
	return true;
}			

bool 
curl_client::perform(
	_In_ const char* file_path,
	_In_ Forms& forms,
	_Out_ long& http_response_code
	)
{
	set_common_opt(_connection_timeout, _read_timeout, _ssl_verifypeer);

	CURLcode curl_code = curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 1);
	if (CURLE_OK != curl_code)
	{
		log_err "set noprogress option failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	curl_code = curl_easy_setopt(_curl, CURLOPT_TCP_KEEPALIVE, 1L);
	if (CURLE_OK != curl_code)
	{
		log_err "set tcp keepalive option failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	curl_mime* http_multipart_form = nullptr;
	curl_mimepart* part = nullptr;

	http_multipart_form = curl_mime_init(_curl);
	_ASSERTE(nullptr != http_multipart_form);

	// set file name
	part = curl_mime_addpart(http_multipart_form);
	
	curl_mime_name(part, "file");
	curl_mime_filename(part, file_name_from_file_patha(file_path).c_str());
	curl_mime_filedata(part, file_path);
	curl_mime_type(part, "application/octect-stream");

	for (auto fm : forms)
	{
		part = curl_mime_addpart(http_multipart_form);
		curl_mime_name(part, fm.first.c_str());
		curl_mime_data(part, fm.second.c_str(), CURL_ZERO_TERMINATED);
	}
	curl_code = curl_easy_setopt(_curl, CURLOPT_MIMEPOST, http_multipart_form);
	if (CURLE_OK != curl_code)
	{
		log_err "set tcp keepalive option failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	//	Execute
	curl_code = curl_easy_perform(_curl);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_perform() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		//
		// 요청 실패한 경우 HTTP 응답값을 0으로 반환한다.
		//
		http_response_code = 0;
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
		//
		// HTTP 응답값을 가져오지 못한 경우 0으로 반환한다.
		//
		http_response_code = 0;
		return false;
	}
	http_response_code = http_code;


	if (nullptr != http_multipart_form)
	{
		curl_mime_free(http_multipart_form); http_multipart_form = nullptr;
	}

	// reset curl handle
	curl_easy_reset(_curl);

	return true;
}