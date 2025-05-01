/**
 * @file    _test_curl.cpp
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2020/02/24 21:41 created.
 * @copyright (C)Somma, Inc. All rights reserved.
 **/
#include "stdafx.h"
#include "_MyLib/src/curl_client.h"
#include "_MyLib/src/curl_client_support.h"
#include "_MyLib/src/StopWatch.h"

#include "json/json.h"

 /// @brief	
bool test_curl_https_down_with_auth()
{
	_mem_dump_console
	_mem_check_begin
	{
		std::string url;
		std::string id;
		std::string pw;

		std::cout << "URL: ";
		std::cin >> url;
		std::cout << "ID : ";
		std::cin >> id;
		std::cout << "PW : ";
		std::cin >> pw;

		
		//
		//	다운로드 임시파일 생성
		//
		std::wstring file_path = L"c:\\dbg\\download.dat";
		handle_ptr temp_file(open_file_to_write(file_path.c_str()),
							 [](HANDLE h) 
		{
			if (INVALID_HANDLE_VALUE != h)
			{
				CloseHandle(h);
			}
		});
		if (temp_file.get() == INVALID_HANDLE_VALUE)
		{
			log_err
				"Can not create file. path=%ws",
				file_path.c_str()
				log_end;
			return false;
		}

		//
		//	creates a download context
		//
		http_download_ctx ctx(url.c_str(), temp_file.get());
		

		//
		//	Create curl client
		//
		auto _cc = std::make_unique<curl_client>();
		if (!_cc)
		{
			log_err "not enought memory" log_end;
			return false;
		}

		if (true != _cc->initialize())
		{
			log_err "curl client initialize() failed." log_end;
			return false;
		}

		//
		//	enable auth
		//
		_cc->enable_auth(id.c_str(), pw.c_str());

		StopWatch sw; sw.Start();

		//
		//	download
		//
		HTTP_CODE http_response_code = 404;
		_ASSERTE(true == _cc->http_download_file(&ctx,
												 http_response_code));
		if (200 == http_response_code)
		{
			_ASSERTE(is_file_existsW(file_path));
		}

		sw.Stop();
		
		std::string md5;
		std::string sha2;
		_ASSERTE(ctx.get_md5(md5));
		_ASSERTE(ctx.get_sha2(sha2));

		log_info
			"download url=%s, local=%ws, md5=%s, sha2=%s, elapsed=%f sec",
			url.c_str(),
			file_path.c_str(),
			md5.c_str(),
			sha2.c_str(),
			sw.GetDurationSecond()
			log_end;

		_ASSERTE(_cc);
	}
	_mem_check_end;
	return true;
}


/// @brief	
bool test_curl_https()
{
	pcurl_client _curl_client = new curl_client();
	if (nullptr == _curl_client)
	{
		log_err "not enought memory" log_end;
		return false;
	}

	std::stringstream http_header;
	http_header << "authorization: Bearer "
		<< "FOO";

	if (true != _curl_client->initialize())
	{
		log_err "curl client initialize() failed." log_end;
		return false;
	}

	_curl_client->append_header("authorization", http_header.str().c_str());

	const char* url = "https://www.google.com/";

	HTTP_CODE http_response_code = 404;
	std::string res;
	_ASSERTE(true == _curl_client->http_get(url, http_response_code, res));

	_ASSERTE(_curl_client != nullptr);
	delete _curl_client; _curl_client = nullptr;
	return true;
}

/// @brief	
bool test_curl_http()
{
	pcurl_client _curl_client = new curl_client();
	if (nullptr == _curl_client)
	{
		log_err "not enought memory" log_end;
		return false;
	}

	if (true != _curl_client->initialize())
	{
		log_err "curl client initialize() failed." log_end;
		return false;
	}

	const char* url = "http://192.168.10.200:5601/app/kibana#/monster?_g=()";
	HTTP_CODE http_response_code = 404;
	CMemoryStream stream;
	_ASSERTE(true == _curl_client->http_get(url, http_response_code, stream));

	auto dumps = dump_memory(0LL, 
							 (unsigned char*)(stream.GetMemory()), 
							 (UINT32)stream.GetSize());
	std::for_each(dumps.begin(), 
				  dumps.end(), 
				  [](std::string& dump) 
	{
		printf("%s\n", dump.c_str()); 
	});

	_ASSERTE(_curl_client != nullptr);
	delete _curl_client; _curl_client = nullptr;
	return true;
}

/// @brief	
bool test_curl_http_upload()
{
	pcurl_client client = new curl_client();

	const wchar_t* path = L"C:\\Windows\\System32\\notepad.exe";
	const char*  url = "http://localhost:33330/upload";
	HTTP_CODE response_code;
	std::string response;
	client->initialize();

	std::map<std::string, std::string> forms;
	forms["group_guid"] = "invalid_group_guid";
	forms["host_guid"] = "invalid_host_guid";
	forms["full_path"] = "invalid_full_path";

	bool ret = false;
	do
	{
		if (true != client->http_file_upload(url, path, forms, response_code, response))
		{
			log_err "http_file_upload() failed. path=%ws, url=%s, status_code=%u",
				path,
				url,
				response_code
				log_end;
			break;
		}

		ret = true;
	} while (false);

	if (true == ret)
	{
		log_info "file upload success. path=%ws, url=%s, status_code=%u",
			path,
			url,
			response_code
			log_end;
	}

	if (nullptr != client)
	{
		delete client; client = nullptr;
	}
	return false;
}

bool test_curl_http_post_with_response_header()
{
_mem_check_begin
{
	pcurl_client _curl_client = new curl_client();
	if (nullptr == _curl_client)
	{
		log_err "not enought memory" log_end;
		return false;
	}

	if (true != _curl_client->initialize())
	{
		log_err "curl client initialize() failed." log_end;
		return false;
	}

	//
	//	자체 서버 켜놓고 테스트 진행 필요
	//	Python -m http.server
	//
	const char* url = nullptr;

	_ASSERTE(nullptr != url);
	HTTP_CODE http_response_code = 404;
	CMemoryStream stream;
	std::map<std::string, std::string> http_response_header;
	Json::Value root;

	root["host_name"] = "Test";
	root["host_ip"] = "127.0.0.1";

	Json::StreamWriterBuilder builder;
	std::stringstream request_body;
	try
	{
		std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
		writer->write(root, &request_body);
	}
	catch (const std::exception& ex)
	{
		log_err "Exception, Can not create request body with json. ex=%s", ex.what() log_end;
		return false;
	}

	_ASSERTE(true == _curl_client->http_post(url,
												request_body.str().c_str(),
												http_response_code,
												http_response_header,
												stream));

	_ASSERTE(0 < http_response_header.size());
	_ASSERTE(_curl_client != nullptr);
	delete _curl_client; _curl_client = nullptr;
}
_mem_check_end;
	return true;
}

/// @brief	`python -m http.server` 명령으로 간단하게 웹서버 띄워놓고 테스트
bool test_curl_http_patch()
{
_mem_check_begin
{
	pcurl_client _client = new curl_client();
	if (nullptr == _client)
	{
		log_err "not enought memory" log_end;
		return false;
	}

	if (true != _client->initialize())
	{
		log_err "curl client initialize() failed." log_end;
		return false;
	}

	auto url = "localhost:8000";
	_client->append_header("accept", "application/json");
	_client->append_header("content-type", "application/json");
	
	Json::Value json;
	json["data"]["revision"] = 0;	// 서버에서 관리하는 값이므로 무의미함
	json["data"]["mode"] = 0;
	json["data"]["allow_procs"].append("");
	json["data"]["allow_ips"].append("");
	json["data"]["allow_domains"].append("");
	json["data"]["block_procs"].append("");
	json["data"]["block_ips"].append("");
	json["data"]["block_domains"].append("");

	HTTP_CODE http_response_code = 404;
	CMemoryStream stream;

	Json::StreamWriterBuilder builder;
	std::stringstream request_body;
	try
	{
		std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
		writer->write(json, &request_body);
	}
	catch (const std::exception& ex)
	{
		log_err "Exception, Can not create request body with json. ex=%s", ex.what() log_end;
		return false;
	}

	_ASSERTE(true == _client->http_patch(
		url,
		request_body.str().c_str(),
		http_response_code,		
		stream));

	log_info "http requested. ret code=%u", http_response_code log_end;

	_ASSERTE(_client != nullptr);
	delete _client; _client = nullptr;
}
_mem_check_end;

	return true;
}