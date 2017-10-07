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

#include "stdafx.h"
#include "log.h"

/**
 * @brief	
**/
static boost::mutex     _logger_lock;
static slogger*		    _logger = nullptr;
static bool			    _show_process_name = true;
static bool			    _show_pid_tid = true;
static bool			    _show_function_name = true;

static uint32_t         _log_mask = log_mask_all; 
static uint32_t			_log_level = log_level_error;
static uint32_t			_log_to = log_to_none;

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
	_In_ uint32_t log_mask,
	_In_ uint32_t log_level, 
	_In_ uint32_t log_to,
	_In_opt_z_ const wchar_t* log_file_path	
	)
{
	_log_mask = log_mask;
	_log_level = log_level;
	_log_to = log_to;

	if (nullptr == log_file_path && FlagOn(log_to, log_to_file))
	{
		OutputDebugStringA("[ERR ] initialize_log(), Invalid parameter mix \n");
		return false;
	}

	{
		boost::lock_guard< boost::mutex > lock(_logger_lock);
		if (NULL != _logger) return true;

		slogger* local_slogger = new slogger(log_level, log_to);
		if (NULL == local_slogger)
		{
			OutputDebugStringA("[ERR ] initialize_log(), insufficient resource for slogger.\n");
			return false;
		}

		if (true != local_slogger->slog_start(log_file_path))
		{
			OutputDebugStringA("[ERR ] initialize_log(), _logger->slog_start() failed.\n");
			delete local_slogger;
			return false;
		}

		// 
		// exchange instance
		// 

		_logger = local_slogger;
		local_slogger = NULL;
	}
	

	//
	//	파일 로그가 활성화된 경우 로그파일에 로그 헤더를 기록한다.
	// 

	if (nullptr != log_file_path && FlagOn(log_to, log_to_file))
	{
		std::string now; GetTimeStringA(now);		
		log_write_fmt(log_mask_sys, 
					  log_level,
					  __FUNCTION__, 
					  "\n==\n== %s, log start.\n==", 
					  now.c_str());
	}

	return true;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void 
finalize_log(
	)
{
	boost::lock_guard< boost::mutex > lock(_logger_lock);
	if (NULL == _logger) return;
	_logger->slog_stop();
	delete _logger;  _logger = NULL;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void
set_log_format(
	_In_ bool show_process_name, 
	_In_ bool show_pid_tid,
	_In_ bool show_function_name
	)
{
    boost::lock_guard< boost::mutex > lock(_logger_lock);

	_show_process_name = show_process_name;
	_show_pid_tid = show_pid_tid;
	_show_function_name = show_function_name;
}


/// @brief	log 설정 갱신
void
set_log_env(
    _In_ uint32_t mask, 
	_In_ uint32_t log_level, 
	_In_ uint32_t log_to
    )
{
	boost::lock_guard< boost::mutex > lock(_logger_lock);
	
	bool update_logger = false;
	if (_log_mask != mask) 
	{
		_log_mask = mask;
	}

	if (_log_level != log_level)
	{
		_log_level = log_level;
		update_logger = true;
	}

	if (_log_to != log_to)
	{
		_log_to = log_to;
		update_logger = true;
	}

	if (nullptr != _logger && true == update_logger)
	{
		_logger->set_log_env(log_level, log_to);
	}
}

/// @brief	_log_mask 
uint32_t get_log_mask()
{
	return _log_mask;
}

void set_log_mask(_In_ uint32_t mask)
{
	boost::lock_guard< boost::mutex > lock(_logger_lock);	
	if (_log_mask != mask) 
	{
		_log_mask = mask;
	}
}

/// @brief	_log_level
uint32_t get_log_level()
{
	return _log_level;
}

void set_log_level(_In_ uint32_t log_level)
{
	boost::lock_guard< boost::mutex > lock(_logger_lock);
	
	if (_log_level != log_level)
	{
		_log_level = log_level;

		if (nullptr != _logger)
		{
			_logger->set_log_env(log_level, _logger->log_to());
		}
	}
}

/// @briefg	_log_to 
uint32_t get_log_to()
{
	return _log_to;
}

void set_log_to(_In_ uint32_t log_to)
{
	boost::lock_guard< boost::mutex > lock(_logger_lock);

	if (_log_to != log_to)
	{
		_log_to = log_to;

		if (nullptr != _logger)
		{
			_logger->set_log_env(_logger->log_level(), log_to);
		}
	}
}


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
    _In_ uint32_t log_mask,
    _In_ uint32_t log_level, 	
	_In_z_ const char* function,
    _In_z_ const char* fmt, 
    _In_ ...
    )
{
    // check log mask & level
    if (log_mask != (_log_mask & log_mask)) return;
	if (_log_level > log_level) return;

    if (NULL == fmt) return;

	char log_buffer[2048];
    size_t remain = sizeof(log_buffer);
    char* pos = log_buffer;
    va_list args;

    // log level
    switch (log_level)
    {
    case log_level_debug: StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s", "[DEBG] "); break;
    case log_level_info:  StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s", "[INFO] "); break;
    case log_level_warn:  StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s", "[WARN] "); break;
    case log_level_error: StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s", "[EROR] "); break;
    default:
        _ASSERTE(!"never reach here!");
        return;
    }

    //> show process name
    if (true == _show_process_name)
    {
        StringCbPrintfExA(
            pos, 
            remain, 
            &pos, 
            &remain, 
            0, 
            "%ws",
            get_current_module_fileEx().c_str()
            );
    }

    //> show pid, tid
    if (true == _show_pid_tid)
    {
        StringCbPrintfExA(
            pos,
            remain,
            &pos,
            &remain,
            0,
            "(%+5u:%+5u) : ",
            GetCurrentProcessId(),
            GetCurrentThreadId()
            );
    }

    //> show function name
    if (true == _show_function_name)
    {
        StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%s : ", function);
    }

    va_start(args,fmt);
    HRESULT hRes = StringCbVPrintfExA(
                        pos, 
                        remain, 
                        &pos,
                        &remain,
                        0, 
                        fmt, 
                        args
                        );

    if (S_OK != hRes)
    {
		// invalid character 가 끼어있는 경우 발생 할 수 있음
        StringCbPrintfExA(
            pos, 
            remain, 
            &pos, 
            &remain,
            0, 
            "invalid function call parameters"
            );
    }    
    va_end(args);

    // line feed
    StringCbPrintfExA(pos, remain, &pos, &remain, 0, "\n");

    // Let's write logs.
    {
        boost::lock_guard< boost::mutex > lock(_logger_lock);

        if (NULL != _logger)
        {
            _logger->slog_write(log_level, _log_to, log_buffer);
        }
        else
        {			
            if (FlagOn(_log_to, log_to_con))
            {
                switch (log_level)
                {
                case log_level_error: // same as log_level_critical
                    write_to_console(fc_red, log_buffer);
                    break;
                case log_level_info:
                case log_level_warn:
                    write_to_console(fc_green, log_buffer);
                    break;
                default:
                    write_to_console(fc_none, log_buffer);
                }
            }

			if (FlagOn(_log_to, log_to_ods))
            {
                OutputDebugStringA(log_buffer);
            }
        }
    }
}
#endif// _NO_LOG_




/// @brief  Writes log without decoration
void
log_write_fmt_without_deco(
    _In_ uint32_t log_mask,
    _In_ uint32_t log_level,
    _In_z_ const char* fmt,
    _In_ ...
    )
{
	// check log mask & level
	if (log_mask != (_log_mask & log_mask)) return;
	if (_log_level > log_level) return;

    if (NULL == fmt) return;

    char log_buffer[2048];
    size_t remain = sizeof(log_buffer);
    char* pos = log_buffer;
    va_list args;

    va_start(args, fmt);
	HRESULT hRes = StringCbVPrintfExA(pos,
									  remain,
									  &pos,
									  &remain,
									  0,
									  fmt,
									  args
	);

    if (S_OK != hRes)
    {
        // invalid character 가 끼어있는 경우 발생 할 수 있음
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "invalid function call parameters"
		);
    }
    va_end(args);

    // line feed
    StringCbPrintfExA(pos, remain, &pos, &remain, 0, "\n");

    // Let's write logs.
    {
        boost::lock_guard< boost::mutex > lock(_logger_lock);

        if (NULL != _logger)
        {
            _logger->slog_write(log_level, _log_to, log_buffer);
        }
        else
        {
            if (FlagOn(_log_to, log_to_con))
            {
                switch (log_level)
                {
                case log_level_error: // same as log_level_critical
                    write_to_console(fc_red, log_buffer);
                    break;
                case log_level_info:
                case log_level_warn:
                    write_to_console(fc_green, log_buffer);
                    break;
                default:
                    write_to_console(fc_none, log_buffer);
                }
            }

			if (FlagOn(_log_to, log_to_ods))
            {
                OutputDebugStringA(log_buffer);
            }
        }
    }

}

/*****************************************************************************/
/*					slogger class implementation							 */
/*****************************************************************************/

/**
 * @brief	constructor
 */
slogger::slogger(_In_ uint32_t log_level, _In_ uint32_t log_to): 
	_stop_logger(true), 
	_log_level(log_level),
	_log_to(log_to),
	_logger_thread(NULL), 
	_log_file_handle(INVALID_HANDLE_VALUE)
{
}

/**
 * @brief destructor 
 */
slogger::~slogger()
{
    slog_stop();
}

/**
 * @brief	start logger
 */
bool 
slogger::slog_start(
	_In_opt_z_ const wchar_t *log_file_path
	)
{
	_ASSERTE(NULL == _logger_thread);
    if (NULL != _logger_thread) { return false; }
    
	// if log file specified, create log file.
	if (NULL != log_file_path)
	{
		_log_file_handle = CreateFileW(log_file_path,
									   GENERIC_WRITE,
									   FILE_SHARE_WRITE | FILE_SHARE_READ,
									   NULL,
									   OPEN_ALWAYS,
									   FILE_ATTRIBUTE_NORMAL,
									   NULL);
		if (INVALID_HANDLE_VALUE == _log_file_handle)
		{
			return false;
		}
	}

    _stop_logger = false;
    _logger_thread = new boost::thread( boost::bind(&slogger::slog_thread, this) );
    return true;
}


/**
 * @brief	stop logger
*/
void slogger::slog_stop()
{
    if (true == _stop_logger) return;
 
    _stop_logger = true;
	
	if (NULL != _logger_thread)
	{
		_logger_thread->join();
		//std::cout << boost::format("tid=0x%08x, %s logger thread joined. \n") % GetCurrentThreadId() % __FUNCTION__;
		delete _logger_thread; _logger_thread = NULL;
	}

    if (INVALID_HANDLE_VALUE != _log_file_handle)
	{
		CloseHandle(_log_file_handle); 
		_log_file_handle = INVALID_HANDLE_VALUE;
	}

	_ASSERTE(true == _log_queue.empty());
	_ASSERTE(INVALID_HANDLE_VALUE == _log_file_handle);
	_ASSERTE(nullptr == _logger_thread);
}

/**
 * @brief	log 큐에 로그를 push 한다.
*/
void 
slogger::slog_write(
	_In_ uint32_t level, 
	_In_ uint32_t log_to, 
	_In_z_ const char* log_message
	)
{
	_ASSERTE(NULL != log_message);
	if (NULL == log_message) return;

	// check log level
	if (log_level() > level) return;

	// enqueue log
	plog_entry le = new log_entry(level, log_to, log_message);

	boost::lock_guard< boost::mutex > lock(_lock);
    _log_queue.push(le);
}
 
/**
 * @brief	logger worker thread, pops log entry from log queue and writes log to output.
*/
void slogger::slog_thread()
{
    //std::cout << boost::format("tid=0x%08x, %s logger thread started \n") % GetCurrentThreadId() % __FUNCTION__;

    while (true != _stop_logger)
    {
        if (true == _log_queue.empty()) 
        {
            Sleep(10);
            continue;
        }

		plog_entry log = NULL;
		{
			boost::lock_guard< boost::mutex > lock(_lock);
			log = _log_queue.pop();
		}
		     
		if (log->_log_to & log_to_con)
		{
            switch (log->_log_level)
            {
            case log_level_error: // same as log_level_critical
                write_to_console(fc_red, log->_msg.c_str());
                break;
            case log_level_info:
            case log_level_warn: 
                write_to_console(fc_green, log->_msg.c_str());
                break;
            default:
                write_to_console(fc_none, log->_msg.c_str());
            }
		}

		if (log->_log_to & log_to_file)
		{
			if (INVALID_HANDLE_VALUE != _log_file_handle)
			{
				write_to_filea(_log_file_handle, "%s", log->_msg.c_str());
			}
		}

		if (log->_log_to & log_to_ods)
		{			
			OutputDebugStringA(log->_msg.c_str());
		}

		delete log;
    }

    // flush all logs to target media only file.
    //write_to_console(wtc_green, "[INFO] finalizing logs...");
	{
		boost::lock_guard< boost::mutex > lock(_lock);

		while (true != _log_queue.empty())
		{
			plog_entry log = _log_queue.pop();
			if (log->_log_to & log_to_file)
			{
				if (INVALID_HANDLE_VALUE != _log_file_handle)
				{
					write_to_filea(_log_file_handle, "%s", log->_msg.c_str());
				}
			}

			if (log->_log_to & log_to_con)
			{
				switch (log->_log_level)
				{
				case log_level_error: // same as log_level_critical
					write_to_console(fc_red, log->_msg.c_str());
					break;
				case log_level_info:
				case log_level_warn:
					write_to_console(fc_green, log->_msg.c_str());
					break;
				default:
					write_to_console(fc_none, log->_msg.c_str());
				}
			}

			if (log->_log_to & log_to_ods)
			{
				OutputDebugStringA(log->_msg.c_str());
			}

			delete log;
		}
		//write_to_console(wtc_green, "done.\n");
		//std::cout << boost::format("tid=0x%08x, %s logger thread terminated \n") % GetCurrentThreadId() % __FUNCTION__;

		_ASSERTE(true == _log_queue.empty());
	}
}