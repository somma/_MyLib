/**
 * @file    FileInfoCache.h
 * @brief   
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2017/11/14 11:54 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#pragma once

#include "CppSQLite\CppSQLite3.h"


/// @brief	FileInformationn class
typedef class FileInformation
{
public:
	FileInformation()
		:
		size(0xffffffffffffffff),
		create_time(0),
		write_time(0),
		md5("none"),
		sha2("none")
	{
	}

	uint64_t    size;
	uint64_t    create_time;
	uint64_t    write_time;
	std::string md5;            // hex string
	std::string sha2;           // hex string	

} *PFileInformation;



#pragma todo("꼭 수정해야 할 내용 있음")
//	- 캐시의 사이즈가 무한정 커질 수 있음
//		hitcount 필드를 추가하고
//		주기적으로? 캐시 사이즈가 특정 갯수에 도달하면?
//		hitcount 가 작은 순서로 삭제 후 VACUUM 을 수행 (요건 그냥 내 생각)
//
//	- file path 를 utf8 로 저장해야 함
//	- file path 가 현재 대소문자가 구분되는것 같은데, 대소문자 구분 없어야 함
//

typedef class FileInfoCache
{
public:
	FileInfoCache();
	virtual ~FileInfoCache();

	bool initialize(_In_ const wchar_t* db_file_path, _In_ bool delete_if_exist = false);
	void finalize();

	bool get_file_information(_In_ const wchar_t* file_path, _Out_ FileInformation& file_information);

	uint32_t size() {return _size; }
	uint32_t hit_count() {return _hit_count;}

private:
	bool insert_file_info(_In_ const wchar_t* path,
						  _In_ uint64_t create_time,
						  _In_ uint64_t write_time,
						  _In_ uint64_t size,
						  _In_ const char* md5_str,
						  _In_ const char* sha2_str);

	bool get_flie_info(_In_ const wchar_t* path,
					   _In_ uint64_t create_time,
					   _In_ uint64_t write_time,
					   _In_ uint64_t size,
					   _Out_ std::string& md5_str,
					   _Out_ std::string& sha2_str);

	bool file_util_get_hash(_In_ const wchar_t* file_path,
							_Out_ std::string& md5,
							_Out_ std::string& sha2);

private:
	bool            _initialized;
	CppSQLite3DB	_db;
	uint32_t        _size;
	uint32_t        _hit_count;

} *PFileInfoCache;



//
//	Singleton 객체를 이용한 C api 
// 
bool fi_initialize();
void fi_finalize();

bool fi_get_file_information(_In_ const wchar_t* file_path, _Out_ FileInformation& file_information);





