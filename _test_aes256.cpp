/**
 * @file    _test_aes256.cpp
 * @brief  aes256으로 암호화 유닛테스트 구현 파일 
 * 
 * 
 * @author  JaeHyun, Park (jaehyun@somma.kr)
 * @date    2020/04/08 14:02 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/

#include "stdafx.h"
#include "_MyLib/src/AirCrypto.h"
#include "_MyLib/src/log.h"
#include "_MyLib/src/Win32Utils.h"

/// @brief  test_aes256_file()에서 설정한 경로에 샘플 파일을 생성한다.
bool 
create_test_sample(
	_In_ const std::wstring& target_file_path
)
{
	_ASSERTE(nullptr != target_file_path.c_str());
	if (nullptr == target_file_path.c_str())
	{
		return false;
	}

	std::wstring directory;
	if (!extract_last_tokenW(const_cast<std::wstring&>(target_file_path),
		L"\\",
		directory,
		true,
		false))
	{
		log_err "extract_last_tokenW(path=%ws) failed.", target_file_path.c_str() log_end;
		return false;
	}

	//테스트할 디렉토리 확인
	if (!is_dir(directory.c_str()))
	{
		log_info "create directory(%ws)!!", directory.c_str() log_end;
		if (!CreateDirectoryW(directory.c_str(), NULL))
		{
			log_err "CreateDirectoryW failed. gle=%u", GetLastError() log_end;
			return false;
		}
	}
	
	//파일이 존재하면 생성할 필요가 없음
	if (is_file_existsW(target_file_path.c_str()))
	{
		log_info "%ws file exists!!", target_file_path.c_str() log_end;
		return true;
	}

	//테스트 파일 생성
	HANDLE create_file = CreateFileW(target_file_path.c_str(),
									 GENERIC_WRITE,
									 NULL,
									 NULL,
									 CREATE_ALWAYS,
									 FILE_ATTRIBUTE_NORMAL,
									 NULL);
	if (INVALID_HANDLE_VALUE == create_file)
	{
		log_err "CreateFileW(%ws) failded, gle = %u", target_file_path.c_str(), GetLastError() log_end;
		return false;
	}

	char buf[] = "test_code";
	DWORD write_length;

	if (!WriteFile(create_file, 
				   buf, 
				   (uint32_t)strlen(buf),
				   &write_length, 
				   NULL))
	{
		log_err "WriteFile(%ws) failed, gle = %u", target_file_path.c_str(), GetLastError() log_end;
		CloseHandle(create_file);
		return false;
	}

	CloseHandle(create_file);
	return true;
}

/**
* @brief	target_file_path, encrypt_file_path, decrypt_file_path 는 절대 경로로 설정한다.
			C:\\_test_aes256 에 폴더와 파일을 생성하며, 결과도 C:\\_test_aes256에 생성이 된다.
*/
/// @brief  target_file_path, encrypt_file_path, decrypt_file_path 는 절대 경로로 설정한다.
///			C:\\_test_aes256 에 폴더와 파일을 생성하며, 결과도 C : \\_test_aes256에 생성이 된다.
/// @return file을 암, 복호화 성공하면 true 리턴
bool 
test_aes256_file()
{
	std::wstring target_file_path = L"C:\\_test_aes256\\ase256_test_before.conf";
	std::wstring encrypt_file_path = L"C:\\_test_aes256\\ase256_test.crypto";
	std::wstring decrypt_file_path = L"C:\\_test_aes256\\ase256_test_after.conf";
	unsigned char origin_key[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890*!=&?&/";

	//테스트 샘플 생성
	if (!create_test_sample(target_file_path))
	{
		log_err "create_test_sample failed." log_end;
		DeleteFileW(target_file_path.c_str());
		RemoveDirectoryW(L"C:\\_test_aes256");
		return false;
	}

	//이미 암호화 파일이 존재하면 삭제
	if (is_file_existsW(encrypt_file_path.c_str()))
	{
		log_err "encrypt file exits, encrypt file delete!!" log_end;
		::DeleteFileW(encrypt_file_path.c_str());
	}

	//aes256 파일 암호화
	if (!aes256_file_encrypt(origin_key,
							 target_file_path,
							 encrypt_file_path))
	{
		log_err "aes256_encrypt() err" log_end;
		return false;
	}

	//aes256 복호화 파일 존재하면 삭제
	if (is_file_existsW(decrypt_file_path.c_str()))
	{
		log_err "decrypt file exits, decrypt file delete!!" log_end;
		::DeleteFileW(decrypt_file_path.c_str());
	}

	//aes256 파일 복호화
	if (!aes256_file_decrypt(origin_key,
							 encrypt_file_path,
							 decrypt_file_path))
	{
		log_err "aes256_encrypt() err" log_end;
		return false;
	}

	log_info "[result] aes256 test folder : C:\\_test_aes256" log_end;

	return true;
}

/// @brief	wstring aes256으로 암호화
/// @return 암, 복호화 결과 같으면 true 리턴
///			암, 복호화 실패 또는 결과가 다르면 false 리턴
bool 
test_aes256_wstring()
{
	unsigned char origin_key[] = "See me If you can";

	std::wstring plain_data = L"Hello Somma World!";
	std::wstring encrypted_data;
	std::wstring decrypted_data;

	// 문자열 AES256 암호화
	if (!aes256_encrypt(origin_key,
						plain_data,
						encrypted_data))
	{
		log_err "aes256_encrypt Failed()" log_end;
		return false;
	}

	// 문자열 AES256 복호화
	if (!aes256_decrypt(origin_key,
						encrypted_data,
						decrypted_data))
	{
		log_err "aes256_decrypt Failed()" log_end;
		return false;
	}

	// 평문과 복호화 결과가 같은지 확인
	if (0 != plain_data.compare(decrypted_data.c_str()))
	{
		log_err "aes256_decrypt Failed()" log_end;
		return false;
	}

	log_info "aes256 decrypt Success" log_end;
	return true;
}