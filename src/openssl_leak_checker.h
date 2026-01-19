/**
 * @file    openssl_leak_checker
 * @brief   OpenSSL 은 초기화시에 내부적으로 정적 객체(?)등을 왕창 할당하는것 같음 (코드레벨에서 본적없으나)
 *          Memory Leak Check 할때 이것때문에 가짜 Leak 이 자꾸 출력되어 유틸리티를 만듬
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    09/14/2025 12:27 Created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once
#include <crtdbg.h>
#include "_MyLib/src/log.h"
#include "_MyLib/src/AirCrypto.h"

/// @brief OpenSSL 정적 메모리를 무시하는 메모리 누수 검사기
class OpenSSLMemoryLeakChecker
{
private:
    _CrtMemState m_baseline_state;
    bool m_baseline_set = false;

public:
    /// @brief OpenSSL 정적 메모리 할당 완료 후 기준점 설정
    void set_baseline_after_openssl_init()
    {
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

        // OpenSSL 첫 호출로 정적 메모리 할당 완료
        const char* dummy_key = "baseline_key_123456";
        const char* dummy_data = "baseline_data";
        unsigned char* dummy_output = nullptr;
        uint32_t dummy_size = 0;

        // 새로운 함수로도 호출 (정적 메모리 완전 초기화)
        dummy_output = nullptr;
        if (aes256_crypt_buffer_v2((const unsigned char*)dummy_key,
                                   (uint32_t)strlen(dummy_key),
                                   (const unsigned char*)dummy_data,
                                   (uint32_t)strlen(dummy_data),
                                   dummy_output,
                                   dummy_size,
                                   true))
        {
            free_and_nil(dummy_output);
        }

        // 기준점 메모리 상태 저장
        _CrtMemCheckpoint(&m_baseline_state);
        m_baseline_set = true;
    }

    /// @brief 현재 메모리 상태와 기준점 비교
    bool check_for_leaks(const char* test_name)
    {
        if (!m_baseline_set)
        {
            log_err "no memory baseline set." log_end;
            return false;
        }

        _CrtMemState current_state, diff_state;
        _CrtMemCheckpoint(&current_state);

        if (_CrtMemDifference(&diff_state, &m_baseline_state, &current_state))
        {
            log_err "test=%s, real leak found!", test_name log_end;
            _CrtMemDumpStatistics(&diff_state);
            return false;
        }
        else
        {
            log_info "test=%s, no leak found!", test_name log_end;
            return true;
        }
    }
};