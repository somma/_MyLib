/**
 * @file    _test_memory_leak.cpp
 * @brief   aes256_crypt_buffer 메모리 누수 테스트
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2024-12-19 created
 * @copyright (C)Somma, Inc. All rights reserved.
**/

#include "stdafx.h"
#include "_MyLib/src/AirCrypto.h"
#include "_MyLib/src/openssl_leak_checker.h"

/// @brief aes256_crypt_buffer_v2() 함수 테스트 (보안 강화 함수)
bool test_aes256_crypt_buffer_v2()
{
    // OpenSSL 정적 메모리 기준점 설정
    OpenSSLMemoryLeakChecker checker;
    checker.set_baseline_after_openssl_init();

    bool all_tests_passed = true;

    log_info "=== aes256_crypt_buffer_v2() 함수 테스트 시작 ===" log_end;

    // 테스트 1: 기본 암호화/복호화 테스트
    {
        const char* test_key = "v2_test_key_123456789012";
        const char* test_data = "V2 function test data with enhanced security features.";

        unsigned char* encrypted = nullptr;
        uint32_t encrypted_size = 0;

        log_info "V2 암호화 테스트 시작" log_end;

        if (aes256_crypt_buffer_v2((const unsigned char*)test_key,
                                   (uint32_t)strlen(test_key),
                                   (const unsigned char*)test_data,
                                   (uint32_t)strlen(test_data),
                                   encrypted,
                                   encrypted_size,
                                   true))
        {
            log_info "V2 암호화 성공: encrypted_size=%u", encrypted_size log_end;

            unsigned char* decrypted = nullptr;
            uint32_t decrypted_size = 0;

            if (aes256_crypt_buffer_v2((const unsigned char*)test_key,
                                       (uint32_t)strlen(test_key),
                                       encrypted,
                                       encrypted_size,
                                       decrypted,
                                       decrypted_size,
                                       false))
            {
                log_info "V2 복호화 성공: decrypted_size=%u", decrypted_size log_end;

                // 데이터 검증
                std::string original_data(test_data);
                std::string decrypted_data((char*)decrypted, decrypted_size);

                if (original_data == decrypted_data)
                {
                    log_info "V2 데이터 검증 성공" log_end;
                }
                else
                {
                    log_err "V2 데이터 검증 실패: original='%s', decrypted='%s'",
                        original_data.c_str(), decrypted_data.c_str() log_end;
                    all_tests_passed = false;
                }

                free_and_nil(decrypted);
            }
            else
            {
                log_err "V2 복호화 실패!" log_end;
                all_tests_passed = false;
            }

            free_and_nil(encrypted);
        }
        else
        {
            log_err "V2 암호화 실패!" log_end;
            all_tests_passed = false;
        }

        if (!checker.check_for_leaks("V2 기본 암호화/복호화"))
        {
            all_tests_passed = false;
        }
    }

    log_info "=== aes256_crypt_buffer_v2() 함수 테스트 완료 ===" log_end;
    return all_tests_passed;
}


/// @brief 통합 테스트 함수
bool test_aes256_crypt_buffer()
{
    OpenSSLMemoryLeakChecker checker;
    checker.set_baseline_after_openssl_init();
    
    // V2 함수 테스트
    if (!test_aes256_crypt_buffer_v2())
    {
        log_err "test_aes256_crypt_buffer_v2() failed." log_end;        
        return false;
    }

    // 추가 반복 테스트 (V1과 V2 모두)
    {
        log_info "=== 반복 메모리 누수 테스트 시작 ===" log_end;

        const char* test_key = "repeat_test_key_123456789012";
        const char* test_data = "Repeated test data for memory leak detection.";

        // V2 함수 반복 테스트        
        for (int i = 0; i < 50; ++i)
        {
            unsigned char* output = nullptr;
            uint32_t output_size = 0;

            if (aes256_crypt_buffer_v2((const unsigned char*)test_key,
                                       (uint32_t)strlen(test_key),
                                       (const unsigned char*)test_data,
                                       (uint32_t)strlen(test_data),
                                       output,
                                       output_size,
                                       true))
            {
                if (i % 10 == 0)
                {
                    log_info "V2 반복 테스트: %d/25, output_size=%u", i + 1, output_size log_end;
                }
                free_and_nil(output);
            }
            else
            {
                log_err "V2 반복 테스트 %d번째 실패", i + 1 log_end;
                return false;
            }
        }

        if (!checker.check_for_leaks("반복 메모리 누수 테스트"))
        {
            return false;
        }
    }

    return true;
}

/**
* @brief	test_aes256_encrypt_decrypt_file()에서 설정한 경로에 샘플 파일을 생성한다.
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

bool test_aes256_encrypt_decrypt_file()
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

    // OpenSSL 정적 메모리 기준점 설정
    OpenSSLMemoryLeakChecker checker;
    checker.set_baseline_after_openssl_init();

    do
    {
        //	테스트 샘플 생성
        if (true != create_test_sample(target_file_path))
        {
            log_err "create_test_sample failed." log_end;
            break;
        }

        //	aes256 암호화
        if (true != aes256_encrypt_file(origin_key,
                                        target_file_path,
                                        encrypt_file_path))
        {
            log_err "aes256_encrypt_file() err" log_end;
            break;
        }

        //	aes256 복호화
        if (true != aes256_decrypt_file(origin_key,
                                        encrypt_file_path,
                                        decrypt_file_path))
        {
            log_err "aes256_decrypt_file() err" log_end;
            break;
        }

		//
		//	Original file과 Decrypted file의 sha2를 비교한다.
		//	비교해서 같으면 올바르게 복호화했다고 본다.
		//
		std::string sha2_original_file;
		if (true != get_file_hash_by_filepath(target_file_path.c_str(),
											  nullptr,
											  &sha2_original_file))
		{
			log_err "get_file_hash_by_filepath() failed. file=%ws", target_file_path.c_str() log_end;
			break;
		}

		std::string sha2_decryption_file;
		if (true != get_file_hash_by_filepath(decrypt_file_path.c_str(),
											  nullptr,
											  &sha2_decryption_file))
		{
			log_err "get_file_hash_by_filepath() failed. file=%ws", decrypt_file_path.c_str() log_end;
			break;
		}

		if (true != sha2_original_file._Equal(sha2_decryption_file.c_str()))
		{
			log_err "Decryption failed. original hash=%s, decrypted hash=%s", 
				sha2_original_file.c_str(), sha2_decryption_file.c_str() log_end;
			break;
		}        ret = true;
    } while (false);

    //	테스트 샘플이 있으면 정리.
    if (true != WUDeleteDirectoryW(L"C:\\_test_aes256"))
    {
        //
        //	Nothing to do....
        //
    }

    log_info "[result] aes256 test folder : C:\\_test_aes256" log_end;

    checker.check_for_leaks("aes256_encrypt_file");
    return ret;
}
