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
//	���� ����
// 
//	cURL ���̺귯���� ����ϱ� ���� curl_global_init() �Լ��� ȣ���ϰ�,
//	���� �� curl_global_cleanup() �Լ��� ȣ���� �־�� �Ѵ�. 
//	curl_global_init() �Լ��� ��� curl_easy_init() ���� �ڵ����� ȣ�� ��
//
//	������ curl_global_init() ȣ�� �� curl_global_cleanup() �� ȣ���ص�
//	_CrtMemDumpAllObjectsSince() ���� �Լ����� memory leak �� �����ȴ�. 
//	���ͳݿ� ���õ� �������� ���� ������ OpenSSL ���� �޸� ���̶�� �� ������
//	������ �ذ�å�� ����. 
// 
//	cURL ���̺귯�� ��� �� �޸𸮰� �����ϴ� ������ ���� ������ ũ�� ���������� 
//	�ʴ´�. ������ _Crtxxx() ���� �Լ����� �������� ���� �ϱ� ���ؼ��� 
//	_CrtMemCheckpoint() ȣ�� ������ curl_global_init() �� ȣ���ϰ�, 
//	_CrtMemDumpAllObjectsSince() ���Ŀ� curl_global_cleanup() �� ȣ���ؾ� �Ѵ�. 
// 
//	���� curl_global_init(), curl_global_cleanup() �Լ��� thread safe ���� 
//	�ʱ� ������ curl_client �� ����/�Ҹ�� ȣ���ϴ� ���� �����ϰ�, main �Լ����
//	ȣ���ϴ� ���� �����ϴ�. 
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

static const char* _null_http_header_string = "";

typedef class curl_client
{
public:
	curl_client();
	~curl_client();

public:
	bool initialize(_In_ const char* http_header = _null_http_header_string,
					_In_ long connection_timeout = 10,
					_In_ long read_timeout = 90,
					_In_ long ssl_verifypeer = 1);
	bool http_get(_In_ const char* url, _Out_ long& http_response_code, _Out_ CMemoryStream& stream);
	bool http_get(_In_ const char* url, _Out_ long& http_response_code, _Out_ std::string& response);

	bool http_post(_In_ const char* url, _In_ const std::string& post_data, _Out_ long& http_response_code, _Out_ CMemoryStream& stream);
	bool http_post(_In_	const char* url, _In_ const std::string& body_data, _Out_ long& http_response_code, _Out_ std::string& response);
		
private:
	bool set_common_opt(_In_ const char* http_header = _null_http_header_string,
						_In_ long connection_timeout = 10,
						_In_ long read_timeout = 90,
						_In_ long ssl_verifypeer = 1);
	bool perform(_Out_ long& http_response_code);
	void finalize();

private:
	CURL*		_curl;
	curl_slist* _header_items;

} *pcurl_client;