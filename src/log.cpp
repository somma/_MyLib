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
#include "_MyLib/src/Win32Utils.h"
#include "_MyLib/src/log_rotated_file.h"

#include <map>
#include <shared_mutex>
#include <atomic>

/// @brief	log_id 에 따른 로거 객체 관리를 위한 클래스
class LogContext
{
public:
	LogContext(LogParam& param) :_param(param), _file_logger(nullptr)
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


//
// 전역 변수/객체
//
static std::shared_mutex _logger_lock;
static std::map<uint32_t, LogContext*> _loggers;		// log_id 기반 로거 객체
static LogContext _default_logger;						// initialize_log() 호출 없이 log_xxx 만 호출했을때 처리를 위한 객체


constexpr const size_t _max_log_buf = 2048;

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
bool initialize_log(_In_ LogParam& param)
{
	//	Check the input 
	if (FlagOn(param._log_to, log_to_file) && param._log_file_path.empty())
	{
		ClearFlag(param._log_to, log_to_file);		
	}

	// Lock and Register logger object
	std::lock_guard<std::shared_mutex> lock(_logger_lock);

	auto ret = _loggers.insert(std::make_pair(param._log_id, nullptr));
	if (ret.second)
	{
		LogContext* ctx = new LogContext(param);
		if (!ctx->initialize())
		{
			dbg_print(log_level_error, "[ERR ] Failed to initialize logger.\n");
			delete ctx;
			return false;
		}

		ctx->_file_logger->add_log(LOG_BEGIN, CB_LOG_BEGIN); 
		ret.first->second = ctx;	//<!		
		
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
		entry->second->_file_logger->add_log(LOG_END, CB_LOG_END);
		entry->second->finalize();
		delete entry->second;
	}
	_loggers.erase(entry);
}

/// @brief	
void
get_log_param(
	_In_ uint32_t log_id,
	_Out_ LogParam& param
)
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


/// @brief	
void
set_log_param(
	_In_ LogParam& param
)
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

///// @brief	
//bool
//set_log_format(
//	_In_ uint32_t log_id,
//	_In_ bool show_level,
//	_In_ bool show_current_time,
//	_In_ bool show_process_name,
//	_In_ bool show_pid_tid,
//	_In_ bool show_function_name
//)
//{
//	_show_level = show_level;
//	_show_current_time = show_current_time;
//	_show_process_name = show_process_name;
//	_show_pid_tid = show_pid_tid;
//	_show_function_name = show_function_name;
//	return true;
//}

///// @brief	
//bool
//get_log_format(
//	_In_ uint32_t log_id,
//	_Out_ bool& show_level,
//	_Out_ bool& show_current_time,
//	_Out_ bool& show_process_name,
//	_Out_ bool& show_pid_tid,
//	_Out_ bool& show_function_name
//)
//{
//	std::shared_lock<std::shared_mutex> lock(_logger_lock);
//
//	const auto entry = _loggers.find(log_id);
//	if (entry == _loggers.cend())
//	{
//		return false;
//	}
//
//	_ASSERTE(nullptr != entry->second);
//	if (nullptr != entry->second)
//	{
//		entry->second->get_log_format(
//			show_level,
//			show_current_time,
//			show_process_name,
//			show_pid_tid,
//			show_function_name
//		);
//
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}

///// @brief	
//bool 
//set_log_env(
//	_In_ uint32_t log_id,
//	_In_ uint32_t log_mask,
//	_In_ uint32_t log_level
//)
//{
//	std::shared_lock<std::shared_mutex> lock(_logger_lock);
//	const auto& entry = _loggers.find(log_id);
//	if (entry != _loggers.cend() && nullptr == entry->second)
//	{
//		entry->second->set_log_env(log_mask, log_level);
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}

///// @brief	
//bool
//get_log_env(
//	_In_ uint32_t log_id,
//	_Out_ uint32_t& log_mask,
//	_Out_ uint32_t& log_level
//)
//{
//	std::shared_lock<std::shared_mutex> lock(_logger_lock);
//	const auto& entry = _loggers.find(log_id);
//	if (entry != _loggers.cend() && nullptr == entry->second)
//	{
//		entry->second->get_log_env(log_mask, log_level);
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}

#ifndef _NO_LOG_

/// @brief	
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

	char log_buffer[_max_log_buf];
	size_t remain = sizeof(log_buffer);
	char* pos = log_buffer;
	console_font_color color = fc_none;

	std::shared_lock<std::shared_mutex> lock(_logger_lock);

	//
	// find log context 
	//

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

	//
	// filter 
	//
	if (!FlagOn(param->_log_mask, log_mask))
	{
		return;
	}

	if (param->_log_level < log_level)
	{
		return;
	}

	//
	// build log message 
	//
	do
	{	
		//> time
		if (param->_show_time)
		{
			StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s ", current_time_to_iso8601().c_str());
		}

		//> log level		
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

		//> show process name
		if (param->_show_process)
		{
			StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%ws", get_current_module_fileEx().c_str());
		}

		//> show pid, tid		
		if (param->_show_pid_tid)
		{
			StringCbPrintfExA(pos, remain, &pos, &remain, 0, "(%+5u:%+5u) : ", GetCurrentProcessId(), GetCurrentThreadId());
		}

		//> show function name		
		if (param->_show_function)
		{
			StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s : ", function);
		}

		va_list args;
		va_start(args, fmt);
		HRESULT hRes = StringCbVPrintfExA(pos, remain, &pos, &remain, 0, fmt, args);
		if (S_OK != hRes)
		{
			// invalid character 가 끼어있는 경우 발생 할 수 있음
			StringCbPrintfExA(pos, remain, &pos, &remain, 0, "invalid function call parameters");
		}
		va_end(args);

	} while (false);
	
	//
	// write log 
	//	
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
		// line feed
		StringCbPrintfExA(pos, remain, &pos, &remain, 0, "\n");
		write_to_console(color, log_buffer);
	}
}
#endif// _NO_LOG_


