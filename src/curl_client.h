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

typedef class curl_client
{
public:
	curl_client();
	~curl_client();

public:
	bool initialize();
	bool http_get(_In_		 const char* url,
				  _Out_		 std::string& response);
	bool http_post(_In_		 const char* url,
				   _In_		 const std::string& body_data,
				   _Out_	 std::string& response);
private:
	bool set_common_opt(_In_ const char* url,
						_Out_ CMemoryStream& stream);
	bool perform();
	void finalize();

private:
	CURL* _curl;

} *pcurl_client;