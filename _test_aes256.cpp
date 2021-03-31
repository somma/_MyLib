#include "stdafx.h"
#include "_MyLib/src/AirCrypto.h"
#include "_MyLib/src/log.h"
#include "_MyLib/src/Win32Utils.h"

/**
* @brief	test_aes256()에서 설정한 경로에 샘플 파일을 생성한다.
*/
bool create_test_sample(_In_ const std::wstring& target_file_path)
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

bool test_aes256()
{
	bool ret = false;
	std::wstring target_file_path = L"C:\\_test_aes256\\ase256_test_before.conf";
	std::wstring encrypt_file_path = L"C:\\_test_aes256\\ase256_test.crypto";
	std::wstring decrypt_file_path = L"C:\\_test_aes256\\ase256_test_after.conf";
	unsigned char origin_key[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890*!=&?&/";

	//	테스트 샘플이 있으면 제거한다.
	if (true != WUDeleteDirectoryW(L"C:\\_test_aes256"))
	{
		//
		//	Nothing to do....
		//
	}

	do
	{
		//	테스트 샘플 생성
		if (true != create_test_sample(target_file_path))
		{
			log_err "create_test_sample failed." log_end;
			break;
		}

		//	aes256 암호화
		if (true != aes256_encrypt(origin_key,
								   target_file_path,
								   encrypt_file_path))
		{
			log_err "aes256_encrypt() err" log_end;
			break;
		}

		//	aes256 복호화
		if (true != aes256_decrypt(origin_key,
								   encrypt_file_path,
								   decrypt_file_path))
		{
			log_err "aes256_decrypt() err" log_end;
			break;
		}

		//
		//	Sample, Decryption file의 sha2를 비교한다.
		//	비교해서 같으면 올바르게 복호화했다고 본다.
		//
		std::string sha2_original_file;
		if (true != get_file_hash_by_filepath(decrypt_file_path.c_str(),
											  nullptr,
											  &sha2_original_file))
		{
			log_err "get_file_hash_by_filepath() failed." log_end;
			break;
		}

		std::string sha2_decryption_file;
		if (true != get_file_hash_by_filepath(decrypt_file_path.c_str(),
											  nullptr,
											  &sha2_decryption_file))
		{
			log_err "get_file_hash_by_filepath() failed." log_end;
			break;
		}

		if (true != sha2_original_file._Equal(sha2_decryption_file.c_str()))
		{
			log_err "Decryption failed." log_end;
			break;
		}

		ret = true;
	} while (false);
	
	//	테스트 샘플이 있으면 정리.
	if (true != WUDeleteDirectoryW(L"C:\\_test_aes256"))
	{
		//
		//	Nothing to do....
		//
	}

	log_info "[result] aes256 test folder : C:\\_test_aes256" log_end;

	return ret;
}
