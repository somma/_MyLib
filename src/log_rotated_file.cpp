/**
 * @file    log_rotated_file
 * @brief   file logging module that roate old logs.
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2025-09-18 created
 * @copyright (C)Somma, Inc. All rights reserved.
**/

#include "stdafx.h"
#include "_MyLib/src/log_rotated_file.h"
#include "_MyLib/src/log.h"
#include <iomanip>
#include <chrono>
#include <filesystem>


/// @brief 생성자
LogRotatedFile::LogRotatedFile(): 
    _stop_requested(false), 
    _initialized(false), 
    _max_file_size(100 * 1024 * 1024), 
    _max_backup_files(5), 
    _current_file_size(0)
{
}

/// @brief 소멸자
LogRotatedFile::~LogRotatedFile()
{
    finalize();
}

/// @brief 로거 초기화
bool LogRotatedFile::initialize(
    _In_ const std::wstring& log_file_path, 
    _In_ size_t max_file_size,
    _In_ int max_backup_files
)
{
    if (_initialized.load())
    {
        return true; // 이미 초기화됨
    }

    try
    {
        _log_file_path = log_file_path;
        _max_file_size = max_file_size;
        _max_backup_files = max_backup_files;
        _current_file_size = 0;

        // 로그 파일 디렉토리 생성        
        std::filesystem::path log_path(_log_file_path);
        std::filesystem::path log_dir = log_path.parent_path();
        
        if (!log_dir.empty() && !std::filesystem::exists(log_dir))
        {
            std::filesystem::create_directories(log_dir);
        }

        // 로그 파일 열기 (append 모드)
        _log_file.open(_log_file_path, std::ios::out | std::ios::app);
        if (!_log_file.is_open())
        {
            return false;
        }

        // 현재 파일 크기 확인
        if (std::filesystem::exists(_log_file_path))
        {
            _current_file_size = std::filesystem::file_size(_log_file_path);
        }

        // 로그 처리 쓰레드 시작
        _stop_requested = false;
        _log_thread = std::thread(&LogRotatedFile::log_thread_proc, this);

        _initialized = true;

        // 초기화 완료 로그
        dbg_print(DPFLTR_INFO_LEVEL, "LogRotatedFile initialized successfully.\n");
        return true;
    }
    catch (const std::exception& e)
    {
        // 오류 발생 시 정리
        if (_log_file.is_open())
        {
            _log_file.close();
        }
        
        // 예외 정보를 디버그 출력 (파일 로그는 불가능)
        std::string error_msg = "LogRotatedFile initialization failed: ";
        error_msg += e.what();
        error_msg += "\n";
        dbg_print(DPFLTR_ERROR_LEVEL, error_msg.c_str());        
        return false;
    }
}

/// @brief 로거 종료
void LogRotatedFile::finalize()
{
    if (!_initialized.load())
    {
        return;
    }

    try
    {
        // 종료 로그 추가
        dbg_print(DPFLTR_INFO_LEVEL, "Finalizing LogRotatedFile...\n");

        // 쓰레드 종료 요청
        _stop_requested = true;

        // 쓰레드 종료 대기
        if (_log_thread.joinable())
        {
            _log_thread.join();
        }

        // 남은 로그 모두 처리
        std::string log_entry;
        while (_logs.try_pop(log_entry))
        {
            if (_log_file.is_open())
            {
                std::lock_guard<std::mutex> lock(_file_mutex);
                _log_file << log_entry << std::endl;
                _log_file.flush();
            }
        }

        // 파일 닫기
        if (_log_file.is_open())
        {
            _log_file.close();
        }

        _initialized = false;
    }
    catch (const std::exception& e)
    {
        // 종료 중 예외 발생 시 디버그 출력
        std::string error_msg = "LogRotatedFile finalization error: ";
        error_msg += e.what();
        error_msg += "\n";
        dbg_print(DPFLTR_ERROR_LEVEL, error_msg.c_str());
    }
}

/// @brief 로그 추가
void LogRotatedFile::add_log(_In_ const char* data, _In_ size_t size)
{
    if (!_initialized.load())
    {
        return;
    }

    if (data == nullptr || size == 0)
    {
        return;
    }

    try
    {
        std::string log_entry;
        log_entry.reserve(size);
        log_entry.assign(data, size);
        
        _logs.push(std::move(log_entry));
    }
    catch (const std::exception&)
    {
        // 로그 추가 실패 시 무시 (무한 루프 방지)
    }
}

/// @brief 로그 처리 쓰레드 함수
void LogRotatedFile::log_thread_proc()
{
    try
    {
        while (!_stop_requested.load())
        {
            std::string log_entry;
            
            // 로그 큐에서 항목 가져오기 (100ms 간격으로 체크)
            if (_logs.try_pop(log_entry))
            {
                std::lock_guard<std::mutex> lock(_file_mutex);
                
                if (_log_file.is_open())
                {
                    // 파일에 로그 기록
                    _log_file << log_entry << std::endl;
                    _log_file.flush();
                    
                    // 파일 크기 업데이트
                    _current_file_size += log_entry.length() + 1; // +1 for newline
                    
                    // 파일 크기 체크 및 로테이션
                    if (_current_file_size >= _max_file_size)
                    {
                        rotate_log_file();
                    }
                }
            }
            else
            {
                // 로그가 없으면 잠시 대기
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    catch (const std::exception& e)
    {
        // 쓰레드 예외 발생 시 디버그 출력
        std::string error_msg = "LogRotatedFile thread error: ";
        error_msg += e.what();
        error_msg += "\n";
        dbg_print(DPFLTR_ERROR_LEVEL, error_msg.c_str());
    }
}

/// @brief 파일 로테이션 처리
void LogRotatedFile::rotate_log_file()
{
    try
    {
        // 현재 파일 닫기
        if (_log_file.is_open())
        {
            _log_file.close();
        }

        std::filesystem::path log_path(_log_file_path);
        std::wstring base_name = log_path.stem().wstring();
        std::wstring extension = log_path.extension().wstring();
        std::wstring directory = log_path.parent_path().wstring();

        // 기존 백업 파일들을 순서대로 이동
        for (int i = _max_backup_files - 1; i >= 1; --i)
        {
            std::wstring old_backup = directory + L"\\" + base_name + L"." + std::to_wstring(i) + extension;
            std::wstring new_backup = directory + L"\\" + base_name + L"." + std::to_wstring(i + 1) + extension;
            
            if (std::filesystem::exists(old_backup))
            {
                if (i == _max_backup_files - 1)
                {
                    // 가장 오래된 파일 삭제
                    std::filesystem::remove(old_backup);
                }
                else
                {
                    // 파일 이름 변경
                    std::filesystem::rename(old_backup, new_backup);
                }
            }
        }

        // 현재 로그 파일을 .1로 이동
        std::wstring first_backup = directory + L"\\" + base_name + L".1" + extension;
        if (std::filesystem::exists(_log_file_path))
        {
            std::filesystem::rename(_log_file_path, first_backup);
        }

        // 새 로그 파일 생성
        _log_file.open(_log_file_path, std::ios::out | std::ios::trunc);
        _current_file_size = 0;

        // 로테이션 완료 로그
        if (_log_file.is_open())
        {
            auto timestr = time_now_to_str(true, false);

            std::string rotation_msg = "Log file rotated. New log started.";
            _log_file << "[" << timestr << "] " << rotation_msg << std::endl;
            _log_file.flush();
            _current_file_size += rotation_msg.length() + timestr.length() + 4; // +4 for brackets and space
        }
    }
    catch (const std::exception& e)
    {
        // 로테이션 실패 시 디버그 출력
        std::string error_msg = "Log rotation failed: ";
        error_msg += e.what();
        error_msg += "\n";
        dbg_print(DPFLTR_ERROR_LEVEL, error_msg.c_str());
        
        // 새 파일로 다시 시도
        try
        {
            if (!_log_file.is_open())
            {
                _log_file.open(_log_file_path, std::ios::out | std::ios::app);
            }
        }
        catch (const std::exception&)
        {
            // 완전 실패
            dbg_print(DPFLTR_ERROR_LEVEL, "Failed to reopen log file after rotation failure.\n");
        }
    }
}

