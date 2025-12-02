/**
 * @file    Logging module
 * @brief   initialize_log() 함수를 명시적으로 호출하면, log level, log target
 *			(file, debugger, console, etc) 지정/변경 가능
 *
 *			log format 지정/변경 가능
 *			multi thread 환경에서 serialization 이 됨
 *			log_err, log_err 같은 매크로만 사용하면 debugger, console 로 메세지 출력 가능
 *
 *			커널모드 log 모듈과 user mode log 모듈이 모두 DPFLTR_IHVDRIVER_ID 를 
 *			사용하기 때문에 WinDbg 에서 User mode 메세지만 필터링 하고 싶은 경우
 *			`.ofilter` 를 써야 함
 *
 *			kd>ed nt!Kd_IHVDRIVER_Mask 0xff
 *			kd>.ofilter /! "*MonsterK*"
 *
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2015/01/12 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include "_MyLib/src/log.h"
#include "_MyLib/src/log_internal.h"
#include "_MyLib/src/Win32Utils.h"
#include "_MyLib/src/log_rotated_file.h"

#include <map>
#include <shared_mutex>

//=============================================================================
// Anonymous namespace for internal helper functions
//=============================================================================
namespace
{
    constexpr const size_t _max_log_buf = 2048;

    /// @brief  Convert log_level to DPFLTR level
    uint32_t to_dpfltr_level(_In_ uint32_t log_level)
    {
        switch (log_level)
        {
        case log_level_debug: return LOG_INTERNAL_DPFLTR_INFO_LEVEL;
        case log_level_info:  return LOG_INTERNAL_DPFLTR_TRACE_LEVEL;
        case log_level_warn:  return LOG_INTERNAL_DPFLTR_WARNING_LEVEL;
        case log_level_error: return LOG_INTERNAL_DPFLTR_ERROR_LEVEL;
        default:              return LOG_INTERNAL_DPFLTR_ERROR_LEVEL;
        }
    }
} // anonymous namespace


//=============================================================================
// LogContext class - manages logger object per log_id
//=============================================================================

/// @brief  Logger context class for managing logger object per log_id
class LogContext
{
public:
    LogContext(LogParam& param) : _param(param), _file_logger(nullptr)
    {
    }

    LogContext() : _file_logger(nullptr)
    {
    }

    ~LogContext()
    {
        finalize();
    }

    bool initialize()
    {
        if (nullptr != _file_logger) { return true; }
        _file_logger = new LogRotatedFile();
        if (!_file_logger->initialize(_param._log_file_path,
                                      _param._log_file_size,
                                      _param._log_file_backup_count))
        {
            delete _file_logger; _file_logger = nullptr;
            return false;
        }

        return true;
    }

    void finalize()
    {
        if (nullptr != _file_logger)
        {
            _file_logger->finalize();
            delete _file_logger; _file_logger = nullptr;
        }
    }

    LogParam _param;
    LogRotatedFile* _file_logger;
};


//=============================================================================
// Global variables
//=============================================================================

static std::shared_mutex _logger_lock;
static std::map<uint32_t, LogContext*> _loggers;    // Logger objects by log_id
static LogContext _default_logger;                   // Default logger for log_xxx calls without initialize_log()


//=============================================================================
// Public API implementations
//=============================================================================

/// @brief  Output message to debugger (public API)
void dbg_print(_In_ uint32_t log_level, _In_ const char* msg)
{
    log_internal::dbg_output(to_dpfltr_level(log_level), msg);
}


/**
 * @brief   Initialize log module
 * @warning DO NOT use log_err/log_dbg/log_warn macros inside this function.
 *          They will cause deadlock because we hold _logger_lock.
 *          Use log_internal::dbg_log() directly instead.
**/
bool initialize_log(_In_ LogParam& param)
{
    // Check the input 
    if (FlagOn(param._log_to, log_to_file) && param._log_file_path.empty())
    {
        ClearFlag(param._log_to, log_to_file);
    }

    // Lock and register logger object
    std::lock_guard<std::shared_mutex> lock(_logger_lock);

    auto ret = _loggers.insert(std::make_pair(param._log_id, nullptr));
    if (ret.second)
    {
        LogContext* ctx = new LogContext(param);
        if (!ctx->initialize())
        {
            // Use dbg_log directly to avoid deadlock (we already hold _logger_lock)
            log_internal::dbg_log(LOG_INTERNAL_DPFLTR_ERROR_LEVEL, __FUNCTION__,
                "Failed to initialize logger. log_id=%u", param._log_id);
            delete ctx;
            _loggers.erase(ret.first);
            return false;
        }

        ctx->_file_logger->add_log(LOG_BEGIN, CB_LOG_BEGIN);
        ret.first->second = ctx;

        // Use dbg_log directly to avoid deadlock (we already hold _logger_lock)
        log_internal::dbg_log(LOG_INTERNAL_DPFLTR_INFO_LEVEL, __FUNCTION__,
            "Logger initialized successfully. log_id=%u", param._log_id);
    }
    else
    {
        // Logger with same log_id already registered
        // Use dbg_log directly to avoid deadlock (we already hold _logger_lock)
        log_internal::dbg_log(LOG_INTERNAL_DPFLTR_WARNING_LEVEL, __FUNCTION__,
            "Logger already registered. log_id=%u", param._log_id);
    }

    return true;
}

/// @brief  Finalize log module
/// @warning DO NOT use log_err/log_dbg/log_warn macros inside this function.
///          Use log_internal::dbg_log() directly to avoid deadlock.
void finalize_log(_In_ uint32_t log_id)
{
    std::lock_guard<std::shared_mutex> lock(_logger_lock);
    const auto entry = _loggers.find(log_id);
    if (entry == _loggers.cend())
    {
        // Use dbg_log directly to avoid deadlock (we already hold _logger_lock)
        log_internal::dbg_log(LOG_INTERNAL_DPFLTR_WARNING_LEVEL, __FUNCTION__,
            "Logger not found. log_id=%u", log_id);
        return;
    }

    _ASSERTE(nullptr != entry->second);
    if (nullptr != entry->second)
    {
        entry->second->_file_logger->add_log(LOG_END, CB_LOG_END);
        entry->second->finalize();
        delete entry->second;

        // Use dbg_log directly to avoid deadlock (we already hold _logger_lock)
        log_internal::dbg_log(LOG_INTERNAL_DPFLTR_INFO_LEVEL, __FUNCTION__,
            "Logger finalized. log_id=%u", log_id);
    }
    _loggers.erase(entry);
}

/// @brief  Get log parameters for specified log_id
void get_log_param(_In_ uint32_t log_id, _Out_ LogParam& param)
{
    std::shared_lock<std::shared_mutex> lock(_logger_lock);
    const auto entry = _loggers.find(log_id);
    if (entry == _loggers.cend())
    {
        param = _default_logger._param;
    }
    else
    {
        param = entry->second->_param;
    }
}


/// @brief  Set log parameters
/// @warning DO NOT use log_err/log_dbg/log_warn macros inside this function.
///          Use log_internal::dbg_log() directly to avoid deadlock.
void set_log_param(_In_ LogParam& param)
{
    std::lock_guard<std::shared_mutex> lock(_logger_lock);
    const auto entry = _loggers.find(param._log_id);
    if (entry == _loggers.cend())
    {
        _default_logger._param = param;
    }
    else
    {
        entry->second->_param = param;
    }
}

//=============================================================================
// log_write_fmt implementation
//=============================================================================

#ifndef _NO_LOG_

/// @brief  Write formatted log message
void log_write_fmt(
    _In_ uint32_t log_id,
    _In_ uint32_t log_mask,
    _In_ uint32_t log_level,
    _In_z_ const char* function,
    _In_z_ const char* fmt,
    _In_ ...
)
{
    _ASSERTE(nullptr != function);
    _ASSERTE(nullptr != fmt);
    if (nullptr == function || nullptr == fmt) return;

    char log_buffer[_max_log_buf];
    size_t remain = sizeof(log_buffer);
    char* pos = log_buffer;
    console_font_color color = fc_none;

    std::shared_lock<std::shared_mutex> lock(_logger_lock);

    // Find log context
    LogContext* ctx = nullptr;
    const auto entry = _loggers.find(log_id);
    if (entry == _loggers.cend())
    {
        ctx = &_default_logger;
    }
    else
    {
        ctx = entry->second;
    }
    LogParam* param = &ctx->_param;

    // Filter by mask
    if (!FlagOn(param->_log_mask, log_mask))
    {
        return;
    }

    // Filter by level
    if (param->_log_level < log_level)
    {
        return;
    }

    // Build log message
    {
        // Time
        if (param->_show_time)
        {
            StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s ", current_time_to_iso8601().c_str());
        }

        // Log level
        if (param->_show_level)
        {
            switch (log_level)
            {
            case log_level_debug:
                StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s", "[DEBG] ");
                break;
            case log_level_info:
                StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s", "[INFO] ");
                color = fc_green;
                break;
            case log_level_warn:
                StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s", "[WARN] ");
                color = fc_green;
                break;
            case log_level_error:
                StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s", "[EROR] ");
                color = fc_red;
                break;
            default:
                _ASSERTE(!"never reach here!");
                return;
            }
        }

        // Process name
        if (param->_show_process)
        {
            StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%ws", get_current_module_fileEx().c_str());
        }

        // PID:TID
        if (param->_show_pid_tid)
        {
            StringCbPrintfExA(pos, remain, &pos, &remain, 0, "(%5u:%5u) : ", GetCurrentProcessId(), GetCurrentThreadId());
        }

        // Function name
        if (param->_show_function)
        {
            StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s : ", function);
        }

        // Format message
        va_list args;
        va_start(args, fmt);
        HRESULT hRes = StringCbVPrintfExA(pos, remain, &pos, &remain, 0, fmt, args);
        if (S_OK != hRes)
        {
            // May occur when invalid characters are included
            StringCbPrintfExA(pos, remain, &pos, &remain, 0, "invalid function call parameters");
        }
        va_end(args);
    }

    // Write log to targets
    if (FlagOn(param->_log_to, log_to_file))
    {
        _ASSERTE(nullptr != ctx->_file_logger);
        if (nullptr != ctx->_file_logger)
        {
            ctx->_file_logger->add_log(log_buffer, (_max_log_buf - remain));
        }
    }

    if (FlagOn(param->_log_to, log_to_ods))
    {
        dbg_print(log_level, log_buffer);
    }

    if (FlagOn(param->_log_to, log_to_con))
    {
        // Line feed
        StringCbPrintfExA(pos, remain, &pos, &remain, 0, "\n");
        write_to_console(color, log_buffer);
    }
}

#endif // _NO_LOG_


