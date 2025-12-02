/**
 * @file    log_rotated_file
 * @brief   file logging module that rotates old logs.
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
#include <fstream>

/// @brief  Rotating file logger class
class LogRotatedFile
{
public:
    LogRotatedFile();
    ~LogRotatedFile();

    /// @brief  Initialize logger
    /// @param  log_file_path   Log file path
    /// @param  max_file_size   Maximum file size in bytes (default: 100MB)
    /// @param  max_backup_files Maximum backup file count (default: 5)
    /// @return Initialization success
    bool initialize(
        _In_ const std::wstring& log_file_path, 
        _In_ size_t max_file_size = 100 * 1024 * 1024,
        _In_ int max_backup_files = 5);
    
    /// @brief  Finalize logger
    void finalize();

    /// @brief  Add log entry
    /// @param  data    Log data pointer
    /// @param  size    Log data size
    void add_log(_In_ const char* data, _In_ size_t size);

    /// @brief  Check initialization status
    /// @return Initialization status
    bool is_initialized() const { return _initialized; }

private:
    /// @brief  Log processing thread function
    void log_thread_proc();
    
    /// @brief  File rotation handler
    void rotate_log_file();
    
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

