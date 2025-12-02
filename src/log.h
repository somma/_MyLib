/**
 * @file    Logging module
 * @brief   initialize_log() 함수를 명시적으로 호출하면, log level, log target 
 *			(file, debugger, console, etc) 지정/변경 가능
 *
 *			log format 지정/변경 가능
 *
 *			multi thread 환경에서 serialization 이 됨
 *
 *			log_err, log_err 같은 매크로만 사용하면 debugger, console 로 메세지 출력 가능
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2015/01/12 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#ifndef _log_h_
#define _log_h_

#include <mutex>
#include "_MyLib/src/Win32Utils.h"

/// @brief  log_id
constexpr const uint32_t log_id_base = 0xffffffff;

/// @brief  log_mask
constexpr const uint32_t log_mask_all = 0xffffffff;		// 모든 로그를 활성화
constexpr const uint32_t log_mask_sys = 0x00000001;      // for log_info, log_err, xxx
														 
/// @brief log level
constexpr const uint32_t log_level_debug = 3;
constexpr const uint32_t log_level_info = 2;
constexpr const uint32_t log_level_warn = 1;
constexpr const uint32_t log_level_error = 0;

/// @brief log to 
constexpr const uint32_t log_to_none = 0x00000000;
constexpr const uint32_t log_to_file = 0x00000001;
constexpr const uint32_t log_to_ods = 0x00000002;
constexpr const uint32_t log_to_con = 0x00000004;
constexpr const uint32_t log_to_all = (log_to_file | log_to_ods | log_to_con);

constexpr const size_t _max_log_file_size = 100 * 1024 * 1024;		// 100MB
constexpr const int _max_log_file_backup = 5;

constexpr const char* LOG_BEGIN = "LOG INITIALIZED >>>";
#define CB_LOG_BEGIN strlen(LOG_BEGIN)
constexpr const char* LOG_END = "<<< LOG FINALIZED";
#define CB_LOG_END strlen(LOG_END)

/// @brief  Log parameters class
class LogParam
{
public:
    LogParam(
        uint32_t log_id, uint32_t log_mask, uint32_t log_level, uint32_t log_to,
        bool show_level, bool show_time, bool show_process, bool show_pid_tid, bool show_function,
        std::wstring log_file_path,
        size_t log_file_size = _max_log_file_size,
        int log_file_backup = _max_log_file_backup
    ) noexcept
        : _log_id(log_id)
        , _log_mask(log_mask)
        , _log_level(log_level)
        , _log_to(log_to)
        , _log_file_path(log_file_path)
        , _log_file_size(log_file_size)
        , _log_file_backup_count(log_file_backup)
        , _show_level(show_level)
        , _show_time(show_time)
        , _show_process(show_process)
        , _show_pid_tid(show_pid_tid)
        , _show_function(show_function)
    {
    }

    LogParam() noexcept
        : _log_id(log_id_base)
        , _log_mask(log_mask_all)
        , _log_level(log_level_info)
        , _log_to(log_to_ods)
        , _log_file_path(_null_stringw)
        , _log_file_size(0)
        , _log_file_backup_count(0)
        , _show_level(true)
        , _show_time(true)
        , _show_process(false)
        , _show_pid_tid(true)
        , _show_function(false)
    {
    }

    uint32_t _log_id;
    uint32_t _log_mask;
    uint32_t _log_level;
    uint32_t _log_to;

    std::wstring _log_file_path;
    size_t _log_file_size;
    int _log_file_backup_count;

    bool _show_level;
    bool _show_time;
    bool _show_process;
    bool _show_pid_tid;
    bool _show_function;
};


//
// C interface
//

/// @brief	ntdll::DbgPrintEx 
///			(ref) dpfilter.h
#define DPFLTR_ERROR_LEVEL 0
#define DPFLTR_WARNING_LEVEL 1
#define DPFLTR_TRACE_LEVEL 2
#define DPFLTR_INFO_LEVEL 3
#define DPFLTR_MASK 0x80000000
#define DPFLTR_IHVDRIVER_ID 77

void 
dbg_print(
	_In_ uint32_t log_level, 
	_In_ const char* msg
);

bool 
initialize_log(
	_In_ LogParam& param
);

void 
finalize_log(
	_In_ uint32_t log_id
	);

void
get_log_param(
	_In_ uint32_t log_id,
	_Out_ LogParam& param
);

void
set_log_param(
	_In_ LogParam& param
);

#ifdef _NO_LOG_

inline
void
log_write_fmt(
	_In_ uint32_t log_id,
	_In_ uint32_t log_mask,
	_In_ uint32_t log_level,	
	_In_z_ const char* function,
	_In_z_ const char* fmt,
	_In_ ...
)
{
}

#else

void
log_write_fmt(
	_In_ uint32_t log_id,
	_In_ uint32_t log_mask,
	_In_ uint32_t log_level,	
	_In_z_ const char* function,
	_In_z_ const char* fmt,
	_In_ ...
);

#endif // _NO_LOG_


//
// define macro for convenience
//
#define log_err		log_write_fmt( log_id_base, log_mask_sys, log_level_error, __FUNCTION__, 
#define log_warn	log_write_fmt( log_id_base, log_mask_sys, log_level_warn, __FUNCTION__,  
#define log_info	log_write_fmt( log_id_base, log_mask_sys, log_level_info, __FUNCTION__, 
#define log_dbg		log_write_fmt( log_id_base, log_mask_sys, log_level_debug, __FUNCTION__, 
#define log_end		);


#endif//_log_h_