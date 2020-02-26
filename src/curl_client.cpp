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
curl_wcb_to_file(
	_In_ void* rcvd_buffer,
	_In_ size_t rcvd_block_size,
	_In_ size_t rcvd_block_count,
	_In_ void* out_file_handle)
{
	_ASSERTE(nullptr != rcvd_buffer);
	_ASSERTE(rcvd_block_size > 0);
	_ASSERTE(rcvd_block_count > 0);
	_ASSERTE(out_file_handle != nullptr);
	_ASSERTE(out_file_handle != INVALID_HANDLE_VALUE);

	if (nullptr == rcvd_buffer ||
		0 == rcvd_block_size ||
		0 == rcvd_block_count ||
		out_file_handle == nullptr ||
		out_file_handle == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD bytes_written = 0;
	if (!WriteFile((HANDLE)out_file_handle,
				   rcvd_buffer,
				   (DWORD)(rcvd_block_size * rcvd_block_count),
				   &bytes_written,
				   nullptr))
	{
		log_err "WriteFile() failed. gle=%u",
			GetLastError()
			log_end;
		return 0;
	}

	return (rcvd_block_size * rcvd_block_count);
}





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

/// @brief	HTTP(S) GET 요청을 하고, 요청 성공시 true 를 리턴한다.
///
///			성공은 HTTP(S) 요청의 전송의 성공 유무를 의미하는 것이지
///			HTTP(S) request 응답의 성공(e.g. HTTP 200 OK)을 의미하지 않는다.
///			HTTP(S) 응답에 성공유무는 http_response_code 변수를 통해서
///			확인해야 한다. 
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

	//
	//	Prepare (set common options)
	//
	if (!prepare_perform(url, 10, 90, true, true, true, false))
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


/// @brief	HTTP(S) 인증이 걸린 페이지에서 파일을 다운로드한다.
bool 
curl_client::http_down_with_auth(
	_In_ const char* const url,
	_In_ const char* const id,
	_In_ const char* const pw,	
	_In_ const wchar_t* const target_path,
	_Out_ long& http_response_code
	)
{
	_ASSERTE(nullptr != _curl);
	_ASSERTE(nullptr != url);
	if (nullptr == url || nullptr == _curl) return false;

	//
	//	Prepare (set common options)
	//
	if (!prepare_perform(url, 10, 90, true, true, true, false))
	{
		log_err "prepare_perform() failed. " log_end;
		return false;
	}

	//
	//	Set mothod specific options
	//
	//	파일의 사이즈를 알 수 없기때문에 HTTP(S) 응답데이터를 메모리에
	//	적재하면 안되고, 임시파일에 저장한다.
	//	

	//
	//	임시파일 경로 생성 (랜덤하게 생성한 파일명이 다섯번이나 존재한다면 포기)
	//
	handle_ptr tmp_file(
		open_file_to_write(target_path), 
		[](HANDLE h) 
	{
		if (INVALID_HANDLE_VALUE != h)
		{
			CloseHandle(h);
		}
	});

	if (tmp_file.get() == INVALID_HANDLE_VALUE)
	{
		log_err
			"Can not create local temp file for download."
			log_end;
		return false;
	}

	//
	//	파일핸들을 다운로드 콜백 데이터로 설정하고, 
	//	콜백함수를 지정한다.
	//
	auto curl_code = curl_easy_setopt(_curl,
									  CURLOPT_WRITEDATA,
									  tmp_file.get());
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
								 curl_wcb_to_file);
	if (CURLE_OK != curl_code)
	{
		log_err "curl_easy_setopt() failed. curl_code = %d, %s",
			curl_code,
			curl_easy_strerror(curl_code)
			log_end;
		return false;
	}
	
	//
	//	Set auth mode
	//
	curl_code = curl_easy_setopt(_curl, 
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
	std::stringstream id_n_pw;
	id_n_pw << id << ":" << pw;
	curl_code = curl_easy_setopt(_curl,
								 CURLOPT_USERPWD,								 
								 id_n_pw.str().c_str());
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

/// @brief	
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
	
	//
	//	Prepare (set common options)
	//
	if (!prepare_perform(url, 10, 90, true, true, true, false))
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

/// @brief	
bool
curl_client::http_post(
	_In_z_ const char* url,
	_In_z_ const char* data,
	_Out_ long& http_response_code,
	_Out_ CMemoryStream& stream
	)
{
	_ASSERTE(nullptr != url);
	_ASSERTE(nullptr != data);
	if (nullptr == url || nullptr == data) return false;

	//
	//	Prepare (set common options)
	//
	if (!prepare_perform(url, 10, 90, true, true, true, false))
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

///	@brief	HTTP(S) 요청의 공통 옵션들을 설정하고, 성공시 true 를 리턴
bool
curl_client::prepare_perform(
	_In_ const char* url,
	_In_ long connection_timeout,
	_In_ long read_timeout,
	_In_ bool follow_location,
	_In_ bool ssl_verify_peer,
	_In_ bool ssl_verify_host_name,
	_In_ bool verbose
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
		// Set timeout
		//	목적지 서버상태가 Overload 상태인지, 또는 크리티컬한 에러가 있는 
		//	상태인지에 따라 적용
		//	e.g. 설정한 시간(10초)내에 서버에서 응답이 없을 경우 강제 해제
		//
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

		//
		//	Set connect timeout
		//	굉장히 큰파일, 또는 느린 연결 속도(네트워크속도에 좌우됨) 등에 
		//	따라 적용
		//	e.g. 설정한 시간(90초) 내에 다운로드가 완료되지 않을 경우 
		//	강제 해제
		//
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

		//
		//	HTTP redirection 
		//
		//	example.com is redirected, so we tell libcurl to follow 
		//	redirection
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_FOLLOWLOCATION,
									 true == follow_location ? 1 : 0);
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
									 true == ssl_verify_peer ? 1 : 0);
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
									 true == ssl_verify_host_name ? 2 : 0);
		if (CURLE_OK != curl_code)
		{
			log_err "curl_easy_setopt() failed. curl_code = %d, %s",
				curl_code,
				curl_easy_strerror(curl_code)
				log_end;
			break;
		}

		//
		//	get verbose debug output 
		//
		curl_code = curl_easy_setopt(_curl,
									 CURLOPT_VERBOSE,
									 true == verbose ? 1 : 0);
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
bool curl_client::perform(_Out_ long& http_response_code)
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

			//
			//	설정 완료된 HTTP Header 필드들은 정리
			//
			_header_fields.clear();
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
	_Out_ long& http_response_code
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

	// reset curl handle
	curl_easy_reset(_curl);

	return true;
}

// refac - remove me
HANDLE curl_client::get_tempfile()
{
	HANDLE h_file = INVALID_HANDLE_VALUE;
	std::wstring ws;
	if (true != get_temp_dirW(ws))
	{
		log_err "get_temp_dirW() for mmq failed." log_end;
		return INVALID_HANDLE_VALUE;
	}

	for (int i = 5; i > 0; --i)
	{
		std::wstringstream wss;
		wss << ws << generate_random_stringw(16);

		if (is_file_existsW(wss.str().c_str()))
		{
			continue;
		}
		else
		{
			h_file = open_file_to_write(wss.str().c_str());
			if (INVALID_HANDLE_VALUE == h_file)
			{
				log_err
					"open_file_to_write() failed. file=%ws",
					wss.str().c_str()
					log_end;
				continue;
			}
			else
				return h_file;		//<!
		}
	}

	return INVALID_HANDLE_VALUE;
}
