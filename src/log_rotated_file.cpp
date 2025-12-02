/**
 * @file    log_rotated_file
 * @brief   file logging module that rotates old logs.
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2025-09-18 created
 * @copyright (C)Somma, Inc. All rights reserved.
**/

#include "stdafx.h"
#include "_MyLib/src/log_rotated_file.h"
#include "_MyLib/src/log_internal.h"
#include "_MyLib/src/Win32Utils.h"
#include <filesystem>

// Use log_internal namespace for cleaner code
using log_internal::dbg_log;

/// @brief  Constructor
LogRotatedFile::LogRotatedFile(): 
    _stop_requested(false), 
    _initialized(false), 
    _max_file_size(100 * 1024 * 1024), 
    _max_backup_files(5), 
    _current_file_size(0)
{
}

/// @brief  Destructor
LogRotatedFile::~LogRotatedFile()
{
    finalize();
}

/// @brief  Initialize logger
bool LogRotatedFile::initialize(
    _In_ const std::wstring& log_file_path, 
    _In_ size_t max_file_size,
    _In_ int max_backup_files
)
{
    if (_initialized.load())
    {
        return true; // Already initialized
    }

    try
    {
        _log_file_path = log_file_path;
        _max_file_size = max_file_size;
        _max_backup_files = max_backup_files;
        _current_file_size = 0;

        // Create log file directory        
        std::filesystem::path log_path(_log_file_path);
        std::filesystem::path log_dir = log_path.parent_path();
        
        if (!log_dir.empty() && !std::filesystem::exists(log_dir))
        {
            std::filesystem::create_directories(log_dir);
        }

        // Open log file in append mode
        _log_file.open(_log_file_path, std::ios::out | std::ios::app);
        if (!_log_file.is_open())
        {
            // Get detailed error information
            DWORD last_error = GetLastError();
            int err_no = errno;
            char errno_buf[256] = { 0 };
            strerror_s(errno_buf, sizeof(errno_buf), err_no);

            dbg_log(LOG_INTERNAL_DPFLTR_ERROR_LEVEL, __FUNCTION__,
                "Failed to open log file. path=%s, gle=%u, errno=%d (%s)",
                std::filesystem::path(_log_file_path).string().c_str(),
                last_error, err_no, errno_buf);
            return false;
        }

        // Check current file size
        if (std::filesystem::exists(_log_file_path))
        {
            _current_file_size = std::filesystem::file_size(_log_file_path);
        }

        // Start log processing thread
        _stop_requested = false;
        _log_thread = std::thread(&LogRotatedFile::log_thread_proc, this);

        _initialized = true;

        // Initialization complete log
        dbg_log(LOG_INTERNAL_DPFLTR_INFO_LEVEL, __FUNCTION__, "LogRotatedFile initialized successfully.");
        return true;
    }
    catch (const std::exception& e)
    {
        // Clean up on error
        if (_log_file.is_open())
        {
            _log_file.close();
        }
        
        // Output exception info to debug (file log unavailable)
        dbg_log(LOG_INTERNAL_DPFLTR_ERROR_LEVEL, __FUNCTION__,
            "LogRotatedFile initialization failed: %s", e.what());        
        return false;
    }
}

/// @brief  Finalize logger
void LogRotatedFile::finalize()
{
    if (!_initialized.load())
    {
        return;
    }

    try
    {
        // Finalization start log
        dbg_log(LOG_INTERNAL_DPFLTR_INFO_LEVEL, __FUNCTION__, "Finalizing LogRotatedFile...");

        // Request thread termination
        _stop_requested = true;

        // Wait for thread to finish
        if (_log_thread.joinable())
        {
            _log_thread.join();
        }

        // Process all remaining logs
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

        // Close file
        if (_log_file.is_open())
        {
            _log_file.close();
        }

        _initialized = false;
    }
    catch (const std::exception& e)
    {
        // Output exception during finalization
        dbg_log(LOG_INTERNAL_DPFLTR_ERROR_LEVEL, __FUNCTION__,
            "LogRotatedFile finalization error: %s", e.what());
    }
}

/// @brief  Add log entry
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
        // Ignore add_log failure to prevent infinite loop
    }
}

/// @brief  Log processing thread function
void LogRotatedFile::log_thread_proc()
{
    try
    {
        while (!_stop_requested.load())
        {
            std::string log_entry;
            
            // Get item from log queue (checks every 100ms)
            if (_logs.try_pop(log_entry))
            {
                std::lock_guard<std::mutex> lock(_file_mutex);
                
                if (_log_file.is_open())
                {
                    // Write log to file
                    _log_file << log_entry << std::endl;
                    _log_file.flush();
                    
                    // Update file size
                    _current_file_size += log_entry.length() + 1; // +1 for newline
                    
                    // Check file size and rotate if needed
                    if (_current_file_size >= _max_file_size)
                    {
                        rotate_log_file();
                    }
                }
            }
            else
            {
                // Sleep briefly when queue is empty
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    catch (const std::exception& e)
    {
        // Output thread exception
        dbg_log(LOG_INTERNAL_DPFLTR_ERROR_LEVEL, __FUNCTION__,
            "LogRotatedFile thread error: %s", e.what());
    }
}

/// @brief  Rotate log file
void LogRotatedFile::rotate_log_file()
{
    try
    {
        // Close current file
        if (_log_file.is_open())
        {
            _log_file.close();
        }

        std::filesystem::path log_path(_log_file_path);
        std::wstring base_name = log_path.stem().wstring();
        std::wstring extension = log_path.extension().wstring();
        std::wstring directory = log_path.parent_path().wstring();

        // Shift existing backup files in order
        for (int i = _max_backup_files - 1; i >= 1; --i)
        {
            std::wstring old_backup = directory + L"\\" + base_name + L"." + std::to_wstring(i) + extension;
            std::wstring new_backup = directory + L"\\" + base_name + L"." + std::to_wstring(i + 1) + extension;
            
            if (std::filesystem::exists(old_backup))
            {
                if (i == _max_backup_files - 1)
                {
                    // Delete the oldest file
                    std::filesystem::remove(old_backup);
                }
                else
                {
                    // Rename file
                    std::filesystem::rename(old_backup, new_backup);
                }
            }
        }

        // Move current log file to .1
        std::wstring first_backup = directory + L"\\" + base_name + L".1" + extension;
        if (std::filesystem::exists(_log_file_path))
        {
            std::filesystem::rename(_log_file_path, first_backup);
        }

        // Create new log file
        _log_file.open(_log_file_path, std::ios::out | std::ios::trunc);
        _current_file_size = 0;

        // Log rotation complete message
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
        // Output rotation failure
        dbg_log(LOG_INTERNAL_DPFLTR_ERROR_LEVEL, __FUNCTION__,
            "Log rotation failed: %s", e.what());
        
        // Try to reopen the file
        try
        {
            if (!_log_file.is_open())
            {
                _log_file.open(_log_file_path, std::ios::out | std::ios::app);
            }
        }
        catch (const std::exception&)
        {
            // Complete failure
            dbg_log(LOG_INTERNAL_DPFLTR_ERROR_LEVEL, __FUNCTION__,
                "Failed to reopen log file after rotation failure.");
        }
    }
}

