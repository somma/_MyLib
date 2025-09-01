/**
 * @file    log_rotated_file
 * @brief   file logging module that roate old logs.
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2025-09-18 created
 * @copyright (C)Somma, Inc. All rights reserved.
**/

#pragma once
#include <concurrent_queue.h>
#include <mutex>
#include <thread>
#include <atomic>
#include <sstream>
#include <fstream>

class LogRotatedFile
{
public:
    LogRotatedFile();
    virtual ~LogRotatedFile();

    /// @brief 로거 초기화
    /// @param log_file_path 로그 파일 경로
    /// @param max_file_size 최대 파일 크기 (바이트, 기본값: 100MB)
    /// @param max_backup_files 최대 백업 파일 수 (기본값: 5)
    /// @return 초기화 성공 여부
    bool initialize(
        _In_ const std::wstring& log_file_path, 
        _In_ size_t max_file_size = 100 * 1024 * 1024,
        _In_ int max_backup_files = 5);
    
    /// @brief 로거 종료
    void finalize();

    /// @brief 로그 추가
    /// @param data 로그 데이터 포인터
    /// @param size 로그 데이터 크기
    void add_log(_In_ const char* data, _In_ size_t size);

    /// @brief 초기화 상태 확인
    /// @return 초기화 여부
    bool is_initialized() const { return _initialized; }

private:
    /// @brief 로그 처리 쓰레드 함수
    void log_thread_proc();
    
    /// @brief 파일 로테이션 처리
    void rotate_log_file();

    /// @brief  time string 문자열 생성
    std::wstring get_current_time_string();
    
private:
    Concurrency::concurrent_queue<std::string> _logs;
    std::thread _log_thread;
    std::atomic<bool> _stop_requested;
    std::atomic<bool> _initialized;
    
    std::wstring _log_file_path;
    std::ofstream _log_file;
    size_t _max_file_size;
    int _max_backup_files;
    size_t _current_file_size;
    
    mutable std::mutex _file_mutex;
};

