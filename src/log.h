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
#include "_MyLib/src/Queue.h"

/// @brief log level
#define log_level_debug         3
#define log_level_info          2
#define log_level_warn          1
#define log_level_critical      0
#define log_level_error         log_level_critical

/// @brief log to 
#define log_to_none		0x00000000
#define log_to_file		0x00000001
#define log_to_ods		0x00000002
#define log_to_con		0x00000004
#define log_to_all		(log_to_file | log_to_ods | log_to_con)

/// @brief	log_id
#define log_id_base		0xffffffff

/// @brief  log_mask
#define log_mask_all    0xffffffff		// 모든 로그를 활성화
#define log_mask_sys    0x00000001      // for log_info, log_err, xxx

/// @brief	 Maximum log count on single log file.
#define	_max_log_count_def 60000

/// @brief	로테이팅 된 로그 파일의 최대 갯수
///			이 갯수 보다 많은 로그 파일이 존재하는 경우 로그 로테이팅 시 
///			가장 오래된 로그파일을 삭제한다. 
#define _max_log_files_def 20


//
// C interface
//

void 
dbg_print(
	_In_ uint32_t log_level, 
	_In_ const char* msg
);

bool 
initialize_log(
	_In_ uint32_t log_id,
	_In_ uint32_t log_mask,
	_In_ uint32_t log_level,
	_In_ uint32_t log_to,
	_In_opt_z_ const wchar_t* log_file_path,
	_In_ uint32_t max_log_count = _max_log_count_def,
	_In_ uint32_t max_log_files = _max_log_files_def
	);

void 
finalize_log(
	_In_ uint32_t log_id
	);

bool
set_log_format(
	_In_ uint32_t log_id,
	_In_ bool show_level,
	_In_ bool show_current_time,
	_In_ bool show_process_name, 
	_In_ bool show_pid_tid,
	_In_ bool show_function_name
	);

bool
get_log_format(
	_In_ uint32_t log_id,
	_Out_ bool& show_level,
	_Out_ bool& show_current_time,
	_Out_ bool& show_process_name,
	_Out_ bool& show_pid_tid,
	_Out_ bool& show_function_name
	);

bool
set_log_env(
	_In_ uint32_t log_id,
	_In_ uint32_t log_mask,
	_In_ uint32_t log_level
);

bool
get_log_env(
	_In_ uint32_t log_id,
	_Out_ uint32_t& log_mask,
	_Out_ uint32_t& log_level
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