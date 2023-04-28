/**
 * @file    curl_client.cpp
 * @brief
 *
 * @author  Jungil, Yang (chaserhunter@somma.kr)
 * @date    2017/09/07 16:30 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/

#include "stdafx.h"
#include "_MyLib/src/log.h"
#include "_MyLib/src/md5.h"
#include "_MyLib/src/sha2.h"
#include "_MyLib/src/curl_client.h"

#define _def_conn_timeout	10			// 10초
#define _def_read_timeout	60 * 5		// 5분

static const char* _null_http_header_string = "";
static size_t On_Callback_response_header(_In_ void* pData,
										  _In_ size_t tSize,
										  _In_ size_t tCount,
										  _In_ void* pmUser);


/// @brief
curl_client::curl_client() 
	: 
	_curl(nullptr), 
	_url(_null_stringa),
	_conn_timeout(_def_conn_timeout),
	_read_timeout(_def_read_timeout),
	_follow_location(true),
	_ssl_verify_peer(true),
	_ssl_verify_host(true),
	_user_agent(_null_stringa),
	_verbose(false)
{
}

/// @brief
curl_client::~curl_client()
{
	finalize();
}

/// @brief	
bool 
curl_client::initialize()
{
	_curl = curl_easy_init();
	if (nullptr == _curl)
	{
		log_err "curl_easy_init() failed." log_end;
		return false;
	}

	return true;
}

/// @brief
void 
curl_client::finalize()
{
	_header_fields.clear();
	curl_easy_cleanup(_curl);

	// curl_global_cleanup() 호출은 thread safe 하지 않기때문에
	// 라이브러리 내에서 호출하면 알아채기 어려운 버그를 만들어낼 수 
	// 있다. 이 함수는 application 레벨에서 종료 직전에 호출하는걸로.
	// 아니면 어차피 application 종료되면 알아서 문제 해결됨
	//
	//curl_global_cleanup();
}


/// @brief	
bool 
curl_client::enable_auth(
	_In_ const char* id,
	_In_ const char* password
)
{
	_ASSERTE(nullptr != id);
	_ASSERTE(nullptr != password);
	if (nullptr == id || nullptr == password) return false;

	//
	//	Set auth mode
	//
	auto curl_code = curl_easy_setopt(_curl,
									  CURLOPT_HTTPAUTH,
									  (long)CURLAUTH_BASIC);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}
	
	//
	// set user name and password for the authentication
	//
	std::string id_n_pw = std::string(id) + ":" + std::string(password);
	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_USERPWD,
								 id_n_pw.c_str());
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	return true;
}

///	@brief
bool
curl_client::enable_bearer_auth(
	_In_ const char* bearer_token
)
{
	_ASSERTE(nullptr != bearer_token);
	if (nullptr == bearer_token) return false;

	//
	//	Set bearer auth mode
	//
	auto curl_code = curl_easy_setopt(_curl,
									  CURLOPT_HTTPAUTH,
									  (long)CURLAUTH_BEARER);

	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code =%d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	//
	// set bearer token for the authentication
	//
	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_XOAUTH2_BEARER,
								 bearer_token);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	return true;
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

	// 이미 `key` 값이 존재한다면 새로운 값으로 대체
	_header_fields[key] = value;
}

/// @brief	HTTP(S) GET 요청을 하고, 요청 성공시 true 를 리턴한다.
///
///			성공은 HTTP(S) 요청의 전송의 성공 유무를 의미하는 것이지
///			HTTP(S) request 응답의 성공(e.g. HTTP 200 OK)을 의미하지 않는다.
///			HTTP(S) 응답에 성공유무는 http_response_code 변수를 통해서
///			확인해야 한다. 
bool
curl_client::http_get(
	_In_ const char* url,
	_Out_ HTTP_CODE& http_response_code,
	_Out_ CMemoryStream& stream
)
{
	_ASSERTE(nullptr != _curl);
	_ASSERTE(nullptr != url);
	if (nullptr == url || nullptr == _curl) return false;

	//
	//	Prepare (set common options)
	//
	if (!prepare_perform(url, false))
	{
		log_err "prepare_perform() failed. " log_end;
		return false;
	}

	//
	//	응답데이터 처리
	//
	stream.ClearStream(); 
	auto curl_code = curl_easy_setopt(_curl,
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

	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_WRITEFUNCTION,
								 curl_wcb_to_stream);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	//
	//	Perform http(s) I/O
	//
	if (true != perform(http_response_code))
	{
		log_err "perform() failed." log_end;
		return false;
	}

	return true;
}

/// @brief	HTTP(S) GET 요청을 하고, 요청 성공시 true 를 리턴한다.
///
///			성공은 HTTP(S) 요청의 전송의 성공 유무를 의미하는 것이지
///			HTTP(S) request 응답의 성공(e.g. HTTP 200 OK)을 의미하지 않는다.
///			HTTP(S) 응답에 성공유무는 http_response_code 변수를 통해서
///			확인해야 한다. 
bool
curl_client::http_get(
	_In_ const char* url,
	_Out_ HTTP_CODE& http_response_code,
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


/// @brief	HTTP(S) 파일 다운로드
bool
curl_client::http_download_file(
	_In_ http_download_ctx* ctx,
	_Out_ HTTP_CODE& http_response_code
	)
{
	_ASSERTE(nullptr != _curl);
	_ASSERTE(nullptr != ctx);
	if (nullptr == ctx || nullptr == _curl) return false;

	//
	//	Prepare (set common options)
	//	
	if (!prepare_perform(ctx->url(), false))
	{
		log_err "prepare_perform() failed. " log_end;
		return false;
	}

	//
	//	Set mothod specific options
	//
	
	//
	//	파일핸들을 다운로드 콜백 데이터로 설정하고, 
	//	콜백함수를 지정한다.
	//
	auto curl_code = curl_easy_setopt(_curl,
									  CURLOPT_WRITEDATA,
									  ctx);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_WRITEFUNCTION,
								 curl_wcb_to_ctx);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}	

	//
	//	Perform http(s) I/O
	//
	if (true != perform(http_response_code))
	{
		log_err "perform() failed." log_end;
		return false;
	}

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
	_Out_  HTTP_CODE& http_response_code,
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

/// @brief	
bool
curl_client::http_file_upload(
	_In_z_ const char* url,
	_In_z_ const wchar_t* file_path,
	_In_   Forms& forms,
	_Out_  HTTP_CODE& http_response_code,
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
	
	//
	//	Prepare (set common options)
	//
	if (!prepare_perform(url, true))
	{
		log_err "prepare_perform() failed. " log_end;
		return false;
	}

	//
	//	Set mothod specific options
	//
	stream.ClearStream();
	auto curl_code = curl_easy_setopt(_curl,
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

	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_WRITEFUNCTION,
								 curl_wcb_to_stream);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}
	
	//
	//	Perform http(s) I/O
	//
	std::string file_path_utf8 = WcsToMbsUTF8Ex(file_path);
	if (true != perform(file_path_utf8.c_str(), 
						forms, 
						http_response_code))
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
	_Out_  HTTP_CODE& http_response_code,
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

/// @brief	
bool
curl_client::http_post(
	_In_z_ const char* url,
	_In_z_ const char* data,
	_Out_ HTTP_CODE& http_response_code,
	_Out_ CMemoryStream& stream
	)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(nullptr != data);
	if (nullptr == url || nullptr == data) return false;

	//
	//	Prepare (set common options)
	//
	if (!prepare_perform(url, true))
	{
		log_err "prepare_perform() failed. " log_end;
		return false;
	}

	//
	//	Set mothod specific options
	//
	bool ret = false;
	do
	{
		//
		//	Received data stuff
		//
		stream.ClearStream();
		auto curl_code = curl_easy_setopt(_curl,
										  CURLOPT_WRITEDATA,
										  &stream);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_WRITEFUNCTION,
									 curl_wcb_to_stream);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		//
		//	HTTP POST stuff
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_POST,
									 1);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_POSTFIELDSIZE,
									 strlen(data));
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
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
			break;
		}

		ret = true;		//<!
	} while (false);
	
	if (!ret) return false;

	//
	//	Perform http(s) I/O
	//
	if (true != perform(http_response_code))
	{
		log_err "perform() failed." log_end;
		return false;
	}

	return true;
}

bool 
curl_client::http_post(
	_In_ const char* url, 
	_In_ const char* data, 
	_Out_ HTTP_CODE& http_response_code,
	_Out_ std::map<std::string, std::string>& http_response_header,
	_Out_ CMemoryStream& stream
)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(nullptr != data);
	if (nullptr == url || nullptr == data) return false;

	//
	//	Prepare (set common options)
	//
	if (!prepare_perform(url, true))
	{
		log_err "prepare_perform() failed. " log_end;
		return false;
	}

	//
	//	Set mothod specific options
	//
	bool ret = false;
	do
	{
		//
		//	Received data stuff
		//
		stream.ClearStream();
		auto curl_code = curl_easy_setopt(_curl,
										  CURLOPT_WRITEDATA,
										  &stream);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_WRITEFUNCTION,
									 curl_wcb_to_stream);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		//
		//	HTTP POST stuff
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_POST,
									 1);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_POSTFIELDSIZE,
									 strlen(data));
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
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
			break;
		}

		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_HEADERFUNCTION,
									 On_Callback_response_header);
		if (CURLE_OK != curl_code)
		{
			log_err "set http header Function Failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_HEADERDATA,
									 &http_response_header);
		if (CURLE_OK != curl_code)
		{
			log_err "set http response header failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		ret = true;		//<!
	} while (false);

	if (!ret) return false;

	//
	//	Perform http(s) I/O
	//
	if (true != perform(http_response_code))
	{
		log_err "perform() failed." log_end;
		return false;
	}

	return true;
}

/// @brief	
bool curl_client::http_patch(
	_In_z_ const char* url,
	_In_z_ const char* data,
	_Out_  HTTP_CODE& http_response_code,
	_Out_  CMemoryStream& stream
)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(nullptr != data);
	if (nullptr == url || nullptr == data) return false;

	//
	//	Prepare (set common options)
	//
	if (!prepare_perform(url, true))
	{
		log_err "prepare_perform() failed. " log_end;
		return false;
	}

	//
	//	Set mothod specific options
	//
	bool ret = false;
	do
	{
		//
		//	Received data stuff
		//
		stream.ClearStream();
		auto curl_code = curl_easy_setopt(_curl,
										  CURLOPT_WRITEDATA,
										  &stream);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_WRITEFUNCTION,
									 curl_wcb_to_stream);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		//
		//	HTTP PATCH stuff (https://curl.se/libcurl/c/CURLOPT_CUSTOMREQUEST.html)
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_CUSTOMREQUEST,
									 "PATCH");
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		// 보내는 데이터는 CURLOPT_POSTFIELDSIZE 로 보내야 함
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_POSTFIELDSIZE,
									 strlen(data));
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
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
			break;
		}

		ret = true;		//<!
	} while (false);

	if (!ret) return false;

	//
	//	Perform http(s) I/O
	//
	if (true != perform(http_response_code))
	{
		log_err "perform() failed." log_end;
		return false;
	}

	return true;
}

/// @brief	
bool 
curl_client::http_patch(
	_In_z_ const char* url,
	_In_z_ const char* data,
	_Out_  HTTP_CODE& http_response_code
)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(nullptr != data);
	if (nullptr == url || nullptr == data) return false;

	//
	//	Prepare (set common options)
	//
	if (!prepare_perform(url, true))
	{
		log_err "prepare_perform() failed. " log_end;
		return false;
	}

	//
	//	Set mothod specific options
	//
	bool ret = false;
	do
	{
		//
		//	HTTP PATCH stuff (https://curl.se/libcurl/c/CURLOPT_CUSTOMREQUEST.html)
		//
		auto curl_code = curl_easy_setopt(_curl,
										  CURLOPT_CUSTOMREQUEST,
										  "PATCH");
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		ret = true;		//<!

	} while (false);
	if (!ret) return false;

	//
	//	Perform http(s) I/O
	//
	if (true != perform(http_response_code))
	{
		log_err "perform() failed." log_end;
		return false;
	}

	return true;
}

///	@brief	HTTP(S) 요청의 공통 옵션들을 설정하고, 성공시 true 를 리턴
bool 
curl_client::prepare_perform(
	_In_ const char* const url, 
	_In_ const bool has_payload
)
{
	_ASSERTE(nullptr != url);
	if (nullptr == url) return false;
	
	CURLcode curl_code = CURLE_OK;
	bool ret = false;
	do
	{
		//
		//	Set URL
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_URL,
									 url);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}
		
		//
		//	전송하는 payload 가 없는 경우와 있는 경우에 따라서 CURLOPT_NOBODY 를 
		//	켜거나 꺼줘야 함
		//
		//	REF: https://github.com/libcpr/cpr/blob/master/cpr/session.cpp
		//	void Session::PrepareGet() {
		//		// In case there is a body or payload for this request, we create 
		//		// a custom GET-Request since a GET-Request with body is based on 
		//		// the HTTP RFC **not** a leagal request.
		//		if (hasBodyOrPayload_) {
		//			curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);
		//			curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, "GET");
		//		}
		//		else {
		//			curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);
		//			curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, nullptr);
		//			curl_easy_setopt(curl_->handle, CURLOPT_HTTPGET, 1L);
		//		}
		//		prepareCommon();
		//	}
		//
		if (has_payload)
		{
			curl_code = curl_easy_setopt(_curl,
										 CURLOPT_NOBODY,
										 0L);
			if (CURLE_OK != curl_code)
			{
				log_err "curl_easy_setopt() failed. curl_code = %d, %s",
					curl_code,
					curl_easy_strerror(curl_code)
					log_end;
				break;
			}
		}


		//
		// Set timeout
		//	목적지 서버상태가 Overload 상태인지, 또는 크리티컬한 에러가 있는 
		//	상태인지에 따라 적용
		//	e.g. 설정한 시간(10초)내에 서버에서 응답이 없을 경우 강제 해제
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_CONNECTTIMEOUT,
									 _conn_timeout);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		//
		//	Set connect timeout
		//	굉장히 큰파일, 또는 느린 연결 속도(네트워크속도에 좌우됨) 등에 
		//	따라 적용
		//	e.g. 설정한 시간(90초) 내에 다운로드가 완료되지 않을 경우 
		//	강제 해제
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_TIMEOUT,
									 _read_timeout);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		//
		//	HTTP redirection 
		//
		//	example.com is redirected, so we tell libcurl to follow 
		//	redirection
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_FOLLOWLOCATION,
									 true == _follow_location? 1 : 0);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		//
		//	SSL, Verify peer
		//		
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_SSL_VERIFYPEER,
									 true == _ssl_verify_peer? 1 : 0);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}
		
		//
		//	SSL, Verify host name 
		//		0: does not verify host name
		//		1,2 : verify host name (default)
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_SSL_VERIFYHOST,
									 true == _ssl_verify_host ? 2 : 0);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		//
		//	Set user-agent
		//
		if (!_user_agent.empty())
		{
			curl_code = curl_easy_setopt(_curl,
										 CURLOPT_USERAGENT,
										 _user_agent.c_str());
			if (CURLE_OK != curl_code)
			{
				log_err "curl_easy_setopt() failed. curl_code = %d, %s",
					curl_code,
					curl_easy_strerror(curl_code)
					log_end;
				break;
			}
		}
		

		//
		//	get verbose debug output 
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_VERBOSE,
									 true == _verbose ? 1 : 0);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		ret = true;		//<!		
	} while (false);
		
	return ret;
}

/// @brief	HTTP(S) Request 를 전송하고, 응답코드를 확인한다.
bool curl_client::perform(
	_Out_ HTTP_CODE& http_response_code
)
{
	http_response_code = 0;
	CURLcode curl_code = CURLE_OK;
	curl_slist* header_list = nullptr;
	bool ret = false;
	do
	{
		//
		//	Set HTTP Headers 
		//	
		if (true != _header_fields.empty())
		{
			std::string header_item;
			for (const auto& hf : _header_fields)
			{
				header_item = hf.first;
				header_item += ":";
				header_item += hf.second;
				header_list = curl_slist_append(header_list, 
												header_item.c_str());
			}

			curl_code = curl_easy_setopt(_curl, 
										 CURLOPT_HTTPHEADER, 
										 header_list);
			if (CURLE_OK != curl_code)
			{
				log_err "set http header failed. curl_code = %d, %s",
					curl_code,
					curl_easy_strerror(curl_code)
					log_end;
				break;
			}
		}

		//
		//	Execute HTTP(S) I/O
		//
		curl_code = curl_easy_perform(_curl);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_perform() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		//
		//	Get the last response code.
		//
		long http_code = 0;
		if (CURLE_OK != curl_easy_getinfo(_curl,
										  CURLINFO_RESPONSE_CODE,
										  &http_code))
		{
			log_err "curl_easy_getinfo() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}
		http_response_code = http_code;

		ret = true;		//<!		
	} while (false);

	//
	//	Free header_list 
	//
	if (nullptr != header_list)
	{
		curl_slist_free_all(header_list);
	}

	//
	//	설정 완료된 HTTP Header 필드들은 정리
	//
	_header_fields.clear();

	//
	//	reset curl handle
	//
	curl_easy_reset(_curl);	
	return ret;
}

/// @brief	
bool 
curl_client::perform(
	_In_ const char* file_path,
	_In_ Forms& forms,
	_Out_ HTTP_CODE& http_response_code
)
{
	CURLcode curl_code = curl_easy_setopt(_curl, 
										  CURLOPT_NOPROGRESS, 
										  1);
	if (CURLE_OK != curl_code)
	{
		log_err "set noprogress option failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}

	curl_code = curl_easy_setopt(_curl, 
								 CURLOPT_TCP_KEEPALIVE, 
								 1L);
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

	for (const auto& fm : forms)
	{
		part = curl_mime_addpart(http_multipart_form);
		curl_mime_name(part, fm.first.c_str());
		curl_mime_data(part, fm.second.c_str(), CURL_ZERO_TERMINATED);
	}
	curl_code = curl_easy_setopt(_curl, 
								 CURLOPT_MIMEPOST, 
								 http_multipart_form);
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
		curl_mime_free(http_multipart_form); 
		http_multipart_form = nullptr;
	}

	//
	//	설정 완료된 HTTP Header 필드들은 정리
	//
	_header_fields.clear();

	//
	// reset curl handle
	//
	curl_easy_reset(_curl);

	return true;
}

///	@brief	
size_t
On_Callback_response_header(
	_In_ void* pData,
	_In_ size_t tSize,
	_In_ size_t tCount,
	_In_ void* pmUser
)
{
	size_t length = tSize * tCount, index = 0;
	while (index < length)
	{
		unsigned char *temp = (unsigned char *)pData + index;
		if ((temp[0] == '\r') || (temp[0] == '\n'))
			break;
		index++;
	}

	std::string str((unsigned char*)pData, (unsigned char*)pData + index);
	std::map<std::string, std::string>* pmHeader = (std::map<std::string, std::string>*)pmUser;
	size_t pos = str.find(": ");
	if (pos != std::string::npos)
		pmHeader->insert(std::pair<std::string, std::string>(str.substr(0, pos), str.substr(pos + 2)));

	return (tCount);
}


