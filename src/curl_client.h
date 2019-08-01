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
	bool initialize(_In_ long connection_timeout = 10,
					_In_ long read_timeout = 90,
					_In_ long ssl_verifypeer = 1);
	
	void set_connection_timeout(_In_ long connection_timeout) { _connection_timeout = connection_timeout; }

	void set_read_timeout(_In_ long read_timeout) { _read_timeout = read_timeout; }
	
	void set_ssl_verifypeer(_In_ long ssl_verifypeer) { _ssl_verifypeer = ssl_verifypeer; }

	void append_header(_In_z_ const char* key, _In_z_ const char* value);

	bool http_get(_In_z_ const char* url, _Out_ long& http_response_code, _Out_ CMemoryStream& stream);
	bool http_get(_In_z_ const char* url, _Out_ long& http_response_code, _Out_ std::string& response);

	bool http_post(_In_z_ const char* url,
				   _In_z_ const char* data,
				   _Out_  long& http_response_code, 
				   _Out_  CMemoryStream& stream);
	bool http_post(_In_z_ const char* url,
				   _In_z_ const char* data,
				   _Out_  long& http_response_code, 
				   _Out_  std::string& response);

	//
	// http_file_upload �Լ��� ����ϸ�, ���� �̸��� ������ ���۵ȴ�.
	// ����, �߰������� ������ �����Ͱ� �ִٸ� forms�� ����Ѵ�.
	//
	typedef std::map<std::string, std::string> Forms;
	bool http_file_upload(_In_z_ const char* url,
						  _In_z_ const wchar_t* file_path,
						  _In_   Forms& forms,
						  _Out_  long& http_response_code,
						  _Out_  CMemoryStream& stream);
	bool http_file_upload(_In_z_ const char* url,
						  _In_z_ const wchar_t* file_path,
						  _In_   Forms& forms,
						  _Out_  long& http_response_code,
						  _Out_  std::string& response);
		
private:
	bool set_common_opt(_In_ long connection_timeout = 10,
						_In_ long read_timeout = 90,
						_In_ long ssl_verifypeer = 1);
	bool perform(_Out_ long& http_response_code);
	// multipart/form type�� request body data�� ������ �� �����ϴ� �Լ�
	bool perform(_In_ const char* file_path, _In_   Forms& forms, _Out_ long& http_response_code);

	void finalize();

private:
	CURL* _curl;
	long  _connection_timeout;
	long  _read_timeout;
	long  _ssl_verifypeer;
private:
	typedef std::map<std::string, std::string> Fields;
	Fields _header_fields;
} *pcurl_client;
