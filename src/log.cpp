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
#include "_MyLib/src/log_impl.h"

#include <map>
#include <shared_mutex>

static std::shared_mutex	_logger_lock;
static std::map<uint32_t, slogger*> _loggers;




// refac - slogger 클래스 내부로 이동
//static bool				_show_level = true;
//static bool				_show_current_time = true;
//static bool			    _show_process_name = true;
//static bool			    _show_pid_tid = true;
//static bool			    _show_function_name = true;
//
//static uint32_t         _log_mask = log_mask_all;
//#ifdef _DEBUG
//static uint32_t			_log_level = log_level_debug;
//#else
//static uint32_t			_log_level = log_level_info;
//#endif
//static uint32_t			_log_to = log_to_ods;


/// @brief	ntdll::DbgPrintEx 
///			(ref) dpfilter.h
#define DPFLTR_ERROR_LEVEL 0
#define DPFLTR_WARNING_LEVEL 1
#define DPFLTR_TRACE_LEVEL 2
#define DPFLTR_INFO_LEVEL 3
#define DPFLTR_MASK 0x80000000

#define DPFLTR_IHVDRIVER_ID 77

typedef 
ULONG (__cdecl *fnDbgPrintEx) (
	_In_ ULONG ComponentId,
	_In_ ULONG Level,
	_In_z_ _Printf_format_string_ PCSTR Format,
	...
);
static fnDbgPrintEx _dbg_print = nullptr;

void dbg_print(_In_ uint32_t log_level, _In_ const char* msg)
{
	//
	//	Initialize DbgPrintEx/OutputDebugString wrapper
	//
	if (nullptr == _dbg_print)
	{
		HMODULE nt = GetModuleHandleW(L"ntdll.dll");
		_ASSERTE(nt);
		if (nt)
		{
			_dbg_print = (fnDbgPrintEx)GetProcAddress(nt, "DbgPrintEx");
		}
	}

	//
	//	log level 변환
	//
	uint32_t ll = DPFLTR_ERROR_LEVEL;
	switch (log_level)
	{
	case log_level_debug: 
		ll = DPFLTR_INFO_LEVEL; 
		break;
	case log_level_info:
		ll = DPFLTR_TRACE_LEVEL;
		break;
	case log_level_warn:
		ll = DPFLTR_WARNING_LEVEL;
		break;
	case log_level_error:
		ll = DPFLTR_ERROR_LEVEL;
		break;	
	}

	if (_dbg_print)
	{
		_dbg_print(DPFLTR_IHVDRIVER_ID,
				   ll,
				   "%s",
				   msg);
	}
	else
	{
		OutputDebugStringA(msg);
	}
}


/**
 * @brief	log 모듈을 초기화한다.
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool
initialize_log(
	_In_ uint32_t log_id,
	_In_ uint32_t log_mask,
	_In_ uint32_t log_level,
	_In_ uint32_t log_to,
	_In_opt_z_ const wchar_t* log_file_path,
	_In_ uint32_t max_log_count,
	_In_ uint32_t max_log_files
)
{
	//	Check the input 
	if (nullptr == log_file_path && FlagOn(log_to, log_to_file))
	{
		dbg_print(log_level_error, "[ERR ] initialize_log(), Invalid parameter mix\n");
		return false;
	}
	
	//	Lock
	std::lock_guard<std::shared_mutex> lock(_logger_lock);

	//	Register logger object
	auto ret = _loggers.insert(std::make_pair(log_id, nullptr));
	if (ret.second)
	{
		// 새로운 logger 객체 생성 후 등록
		slogger* logger = new slogger(log_id,
									  log_mask,
									  log_level,
									  log_to,
									  log_file_path,
									  max_log_count,
									  max_log_files);
		if (nullptr == logger)
		{
			dbg_print(log_level_error, "[ERR ] Can not initialize logger(insufficient resource)\n");
			return false;
		}

		if (true != logger->slog_start())
		{

			dbg_print(log_level_error, "[ERR ] Can not start logger\n");
			delete logger;
			return false;
		}
		
		ret.first->second = logger;	//<!
	}
	else
	{
		// 이미 동일한 log_id 의 객체가 등록되어있음
		dbg_print(log_level_warn, "[WARN] logger already registered.");		
	}

	return true;
}

/// @brief	
void
finalize_log(
	_In_ uint32_t log_id
)
{
	std::lock_guard<std::shared_mutex> lock(_logger_lock);
	const auto entry = _loggers.find(log_id);
	if (entry == _loggers.cend())
	{
		return;
	}

	_ASSERTE(nullptr != entry->second);
	if (nullptr != entry->second)
	{
		entry->second->slog_stop();
		delete entry->second;
	}
	_loggers.erase(entry);
}

/// @brief	
bool
set_log_format(
	_In_ uint32_t log_id,
	_In_ bool show_level,
	_In_ bool show_current_time,
	_In_ bool show_process_name,
	_In_ bool show_pid_tid,
	_In_ bool show_function_name
)
{
	std::shared_lock<std::shared_mutex> lock(_logger_lock);
	
	const auto entry = _loggers.find(log_id);
	if (entry == _loggers.cend())
	{
		return false;
	}

	_ASSERTE(nullptr != entry->second);
	if (nullptr != entry->second)
	{
		entry->second->set_log_format(show_level,
									  show_current_time,
									  show_process_name,
									  show_pid_tid,
									  show_function_name);
		return true;
	}
	else
	{
		return false;
	}
}

/// @brief	
bool
get_log_format(
	_In_ uint32_t log_id,
	_Out_ bool& show_level,
	_Out_ bool& show_current_time,
	_Out_ bool& show_process_name,
	_Out_ bool& show_pid_tid,
	_Out_ bool& show_function_name
)
{
	std::shared_lock<std::shared_mutex> lock(_logger_lock);

	const auto entry = _loggers.find(log_id);
	if (entry == _loggers.cend())
	{
		return false;
	}

	_ASSERTE(nullptr != entry->second);
	if (nullptr != entry->second)
	{
		entry->second->get_log_format(
			show_level,
			show_current_time,
			show_process_name,
			show_pid_tid,
			show_function_name
		);

		return true;
	}
	else
	{
		return false;
	}
}

// refac - del
///// @brief	log 설정 갱신
//void
//set_log_env(
//	_In_ uint32_t mask,
//	_In_ uint32_t log_level,
//	_In_ uint32_t log_to
//)
//{
//	std::lock_guard<std::shared_mutex> lock(_logger_lock);
//
//	bool update_logger = false;
//	if (_log_mask != mask)
//	{
//		_log_mask = mask;
//	}
//
//	if (_log_level != log_level)
//	{
//		_log_level = log_level;
//		update_logger = true;
//	}
//
//	if (_log_to != log_to)
//	{
//		_log_to = log_to;
//		update_logger = true;
//	}
//
//	if (nullptr != _logger && true == update_logger)
//	{
//		_logger->set_log_env(log_level, log_to);
//	}
//}
//
///// @brief	_log_mask 
//uint32_t get_log_mask(_In_ uint32_t log_id)
//{
//	std::shared_lock<std::shared_mutex> lock(_logger_lock);
//	const auto& entry = _loggers.find(log_id);
//	if (entry != _loggers.cend() && nullptr == entry->second)
//	{
//		return entry->second->get_log_mask();
//	}
//	else
//	{
//		return log_mask_all;
//	}
//}

/// @brief	
bool 
set_log_env(
	_In_ uint32_t log_id,
	_In_ uint32_t log_mask,
	_In_ uint32_t log_level
)
{
	std::shared_lock<std::shared_mutex> lock(_logger_lock);
	const auto& entry = _loggers.find(log_id);
	if (entry != _loggers.cend() && nullptr == entry->second)
	{
		entry->second->set_log_env(log_mask, log_level);
		return true;
	}
	else
	{
		return false;
	}
}

/// @brief	
bool
get_log_env(
	_In_ uint32_t log_id,
	_Out_ uint32_t& log_mask,
	_Out_ uint32_t& log_level
)
{
	std::shared_lock<std::shared_mutex> lock(_logger_lock);
	const auto& entry = _loggers.find(log_id);
	if (entry != _loggers.cend() && nullptr == entry->second)
	{
		entry->second->get_log_env(log_mask, log_level);
		return true;
	}
	else
	{
		return false;
	}
}


/*

void set_log_mask(_In_ uint32_t log_id, _In_ uint32_t mask)
{
	std::shared_lock<std::shared_mutex> lock(_logger_lock);
	const auto& entry = _loggers.find(log_id);
	if (entry != _loggers.cend() && nullptr == entry->second)
	{
		entry->second->set_log_mask(mask);
	}
}

/// @brief	_log_level
uint32_t get_log_level(_In_ uint32_t log_id)
{
	std::shared_lock<std::shared_mutex> lock(_logger_lock);
	const auto& entry = _loggers.find(log_id);
	if (entry != _loggers.cend() && nullptr == entry->second)
	{
		return entry->second->get_log_level();
	}
	else
	{
		return log_level_error;
	}
}

void set_log_level(_In_ uint32_t log_id, _In_ uint32_t log_level)
{
	std::shared_lock<std::shared_mutex> lock(_logger_lock);
	const auto& entry = _loggers.find(log_id);
	if (entry != _loggers.cend() && nullptr == entry->second)
	{
		entry->second->set_log_level(log_level);
	}
}

/// @briefg	_log_to 
uint32_t get_log_to(_In_ uint32_t log_id)
{
	std::shared_lock<std::shared_mutex> lock(_logger_lock);
	const auto& entry = _loggers.find(log_id);
	if (entry != _loggers.cend() && nullptr == entry->second)
	{
		return entry->second->get_log_to();
	}
	else
	{
		return log_to_none;
	}
}

void set_log_to(_In_ uint32_t log_id, _In_ uint32_t log_to)
{
	std::shared_lock<std::shared_mutex> lock(_logger_lock);
	const auto& entry = _loggers.find(log_id);
	if (entry != _loggers.cend() && nullptr == entry->second)
	{
		entry->second->set_log_to(log_to);
	}
}*/

// refac
//
//const char* log_level_to_str(_In_ uint32_t log_level)
//{
//	switch (log_level)
//	{
//	case log_level_debug: return "debug";
//	case log_level_info: return "info";
//	case log_level_warn: return "warn";
//	case log_level_error: return "error";
//	}
//	return "unknown";
//}
//
//const char* log_to_to_str(_In_ uint32_t log_to)
//{
//	switch (log_to)
//	{
//	case log_to_none: return "none";
//	case log_to_file: return"file";
//	case log_to_ods: return "ods";
//	case log_to_con: return "con";
//	case (log_to_file | log_to_ods): return "file|ods";
//	case (log_to_file | log_to_con): return "file|con";
//	case (log_to_ods | log_to_con): return "ods|con";
//	case log_to_all: return "file|ods|con";
//	}
//	return "unknown";
//}


/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
#ifndef _NO_LOG_

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
	_ASSERTE(nullptr != function);
	_ASSERTE(nullptr != fmt);
	if (nullptr == function || nullptr == fmt) return; 
	
	std::shared_lock<std::shared_mutex> lock(_logger_lock);
	auto entry = _loggers.find(log_id);
	if (entry != _loggers.cend() && nullptr != entry->second)
	{
		va_list args;
		va_start(args, fmt);
		entry->second->slog_write(log_mask, log_level, function, fmt, args);
		va_end(args);
	}
	else
	{
		//
		//	logger 를 초기화 하지 않았다면 console | ods 에 모두 출력한다.
		//
		char log_buffer[2048];
		size_t remain = sizeof(log_buffer);
		char* pos = log_buffer;

		//> time
		StringCbPrintfExA(
			pos,
			remain,
			&pos,
			&remain,
			0,
			"%s ",
			time_now_to_str(true, false).c_str());

		//> log level
		console_font_color color = fc_none;
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

		//> show process name
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "%ws",
						  get_current_module_fileEx().c_str());

		//> show pid, tid		
		StringCbPrintfExA(pos,
							remain,
							&pos,
							&remain,
							0,
							"(%+5u:%+5u) : ",
							GetCurrentProcessId(),
							GetCurrentThreadId());
	
		//> show function name		
		StringCbPrintfExA(pos,
							remain,
							&pos,
							&remain,
							0,
							"%s : ",
							function);
		
		va_list args;
		va_start(args, fmt);
		HRESULT hRes = StringCbVPrintfExA(pos,
											remain,
											&pos,
											&remain,
											0,
											fmt,
											args);

		if (S_OK != hRes)
		{
			// invalid character 가 끼어있는 경우 발생 할 수 있음
			StringCbPrintfExA(pos,
							  remain,
							  &pos,
							  &remain,
							  0,
							  "invalid function call parameters");
		}
		va_end(args);

		// line feed
		StringCbPrintfExA(pos, remain, &pos, &remain, 0, "\n");

		write_to_console(color, log_buffer);
		dbg_print(log_level, log_buffer);
	}	
}
#endif// _NO_LOG_


// refac - del
///// @brief  Writes log without decoration
//void
//log_write_fmt_without_deco(
//	_In_ uint32_t log_id,
//	_In_ uint32_t log_mask,
//	_In_ uint32_t log_level,
//	_In_ bool linefeed,
//	_In_z_ const char* fmt,
//	_In_ ...
//)
//{
//	// check log mask & level
//	if (log_mask != (_log_mask & log_mask)) return;
//	if (log_level > _log_level) return;
//
//	if (NULL == fmt) return;
//
//	char log_buffer[2048];
//	size_t remain = sizeof(log_buffer);
//	char* pos = log_buffer;
//	va_list args;
//
//	va_start(args, fmt);
//	HRESULT hRes = StringCbVPrintfExA(pos,
//									  remain,
//									  &pos,
//									  &remain,
//									  0,
//									  fmt,
//									  args
//	);
//
//	if (S_OK != hRes)
//	{
//		// invalid character 가 끼어있는 경우 발생 할 수 있음
//		StringCbPrintfExA(pos,
//						  remain,
//						  &pos,
//						  &remain,
//						  0,
//						  "invalid function call parameters"
//		);
//	}
//	va_end(args);
//
//	// line feed
//	if (linefeed)
//	{
//		StringCbPrintfExA(pos, remain, &pos, &remain, 0, "\n");
//	}	
//
//	// Let's write logs.
//	{
//		std::shared_lock<std::shared_mutex> lock(_logger_lock);
//		if (NULL != _logger)
//		{
//			_logger->slog_write(log_level, log_buffer);
//		}
//		else
//		{
//			if (FlagOn(_log_to, log_to_con))
//			{
//				switch (log_level)
//				{
//				case log_level_error: // same as log_level_critical
//					write_to_console(fc_red, log_buffer);
//					break;
//				case log_level_info:
//				case log_level_warn:
//					write_to_console(fc_green, log_buffer);
//					break;
//				default:
//					write_to_console(fc_none, log_buffer);
//				}
//			}
//
//			if (FlagOn(_log_to, log_to_ods))
//			{
//				dbg_print(log_level, log_buffer);
//			}
//		}
//	}
//
//}
