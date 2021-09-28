/**
 * @file    curl_client.h
 * @brief   
 *
 * @author  Jungil, Yang (chaserhunter@somma.kr)
 * @date    2017/09/07 16:30 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/

#pragma once
#include <Windows.h>
#include "CStream.h"
#include "curl/curl.h"
#include "_MyLib/src/curl_client_support.h"


//
//	주의 사항
// 
//	cURL 라이브러리를 사용하기 전에 curl_global_init() 함수를 호출하고,
//	종료 시 curl_global_cleanup() 함수를 호출해 주어야 한다. 
//	curl_global_init() 함수의 경우 curl_easy_init() 에서 자동으로 호출 됨
//
//	문제는 curl_global_init() 호출 후 curl_global_cleanup() 을 호출해도
//	_CrtMemDumpAllObjectsSince() 등의 함수에서 memory leak 이 감지된다. 
//	인터넷에 관련된 질문들은 많이 있으나 OpenSSL 관련 메모리 릭이라는 것 정도뿐
//	명쾌한 해결책이 없다. 
// 
//	cURL 라이브러리 사용 중 메모리가 증가하는 현상은 없기 때문에 크게 문제되지는 
//	않는다. 하지만 _Crtxxx() 관련 함수들의 리포팅을 방지 하기 위해서는 
//	_CrtMemCheckpoint() 호출 이전에 curl_global_init() 을 호출하고, 
//	_CrtMemDumpAllObjectsSince() 이후에 curl_global_cleanup() 을 호출해야 한다. 
// 
//	또한 curl_global_init(), curl_global_cleanup() 함수는 thread safe 하지 
//	않기 때문에 curl_client 의 생성/소멸시 호출하는 것은 위험하고, main 함수등에서
//	호출하는 것이 안전하다. 
// 
//
//		int main()
//		{
//			curl_global_init(CURL_GLOBAL_ALL);
//		
//			_CrtMemState memoryState = { 0 };
//			_CrtMemCheckpoint(&memoryState);
//			_CrtSetBreakAlloc(860);
//
//			//....................
//
//			_CrtMemDumpAllObjectsSince(&memoryState);
//			curl_global_cleanup();
//			rturn 0;
//		}
//

#define HTTP_OK	200

typedef class curl_client
{
public:
	curl_client();
	~curl_client();

public:
	bool initialize();

	bool http_get(
		_In_z_ const char* url,
		_Out_ long& http_response_code,
		_Out_ CMemoryStream& stream);

	bool http_get(
		_In_z_ const char* url,
		_Out_ long& http_response_code,
		_Out_ std::string& response);

	bool http_download_file(
		_In_ http_download_ctx* ctx,
		_Out_ long& http_response_code);

	bool http_post(
		_In_z_ const char* url,
		_In_z_ const char* data,
		_Out_  long& http_response_code,
		_Out_  CMemoryStream& stream);

	bool http_post(
		_In_z_ const char* url,
		_In_z_ const char* data,
		_Out_  long& http_response_code,
		_Out_  std::map<std::string, std::string>& http_response_header,
		_Out_  CMemoryStream& stream);

	bool http_post(
		_In_z_ const char* url,
		_In_z_ const char* data,
		_Out_  long& http_response_code,
		_Out_  std::string& response);

	//
	// http_file_upload 함수를 사용하면, 파일 이름은 서버로 전송된다.
	// 만약, 추가적으로 전송할 데이터가 있다면 forms를 사용한다.
	//
#pragma todo("Forms 객체를 파라미터로 받지 않게. add_form() 형태로 리팩토링")
	typedef std::map<std::string, std::string> Forms;
	bool http_file_upload(
		_In_z_ const char* url,
		_In_z_ const wchar_t* file_path,
		_In_   Forms& forms,
		_Out_  long& http_response_code,
		_Out_  CMemoryStream& stream);
	bool http_file_upload(
		_In_z_ const char* url,
		_In_z_ const wchar_t* file_path,
		_In_   Forms& forms,
		_Out_  long& http_response_code,
		_Out_  std::string& response);

private:
	//
	//	CURL object
	//
	CURL* _curl;

private:
	//
	//	Options that applied on every I/O perform
	//
	std::string _url;
	long _conn_timeout;
	long _read_timeout;
	bool _follow_location;
	bool _ssl_verify_peer;
	bool _ssl_verify_host;
	std::string _user_agent;
	bool _verbose;

	typedef std::map<std::string, std::string> Fields;
	Fields _header_fields;
public:
	bool enable_auth(_In_ const char* id, _In_ const char* password);
	bool enable_bearer_auth(_In_ const char* bearer_token);
	void append_header(_In_z_ const char* key, _In_z_ const char* value);

	void set_url(const char* const url) { _url = url; }
	void set_connection_timeout(const long value) { _conn_timeout = value; }
	void set_read_timeout(const long value) { _read_timeout = value; }
	void set_follow_location(const bool value) { _follow_location = value; }
	void set_ssl_verify_peer(const bool value) { _ssl_verify_peer = value; }
	void set_ssl_verify_host(const bool value) { _ssl_verify_host = value; }
	void set_user_agent(const char* value) { _user_agent = value; }
	void set_verbose(const bool value) { _verbose = value; }


private:
	bool prepare_perform(_In_ const char* const url);

	bool perform(_Out_ long& http_response_code);

	// multipart/form type을 request body data에 설정한 후 전송하는 함수
	bool perform(
		_In_ const char* file_path, 
		_In_ Forms& forms, 
		_Out_ long& http_response_code);

	void finalize();



} *pcurl_client;
