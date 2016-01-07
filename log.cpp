/**----------------------------------------------------------------------------
 * log.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2015:1:12 16:54 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "log.h"

/**
 * @brief	
**/
static boost::mutex     _logger_lock;
static slogger*		    _logger = NULL;
static bool			    _show_process_name = true;
static bool			    _show_pid_tid = true;
static bool			    _show_function_name = true;
static uint32_t         _log_mask = log_mask_all;

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
	_In_ uint32_t log_level, 
	_In_opt_z_ const wchar_t* log_file_path
	)
{
    boost::lock_guard< boost::mutex > lock(_logger_lock);
    if (NULL != _logger) return true;

	slogger* local_slogger = new slogger();
	if (NULL == local_slogger) 
	{
		OutputDebugStringA("[ERR ] initialize_log(), insufficient resource for slogger.\n");
		return false;
	}

	if (true != local_slogger->slog_start(log_level, log_file_path))
	{
		OutputDebugStringA("[ERR ] initialize_log(), _logger->slog_start() failed.\n");
		return false;
	}

	// exchange instance
    _logger = local_slogger;
	local_slogger = NULL;

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


/**
 * @brief	log mask 를 설정한다. ( mask = 0xffffffff : 모든 로그를 활성화 )
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void
set_log_mask(
    _In_ uint32_t mask
    )
{
    _log_mask = mask;
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
log_write_fmt(
    _In_ uint32_t log_mask,
    _In_ uint32_t log_level, 
	_In_ uint32_t log_to,
	_In_z_ const char* function,
    _In_z_ const char* fmt, 
    _In_ ...
    )
{
    // check log mask
    if (log_mask != (_log_mask & log_mask)) return;

    // check base log level
    {
        boost::lock_guard< boost::mutex > lock(_logger_lock);
        if (NULL != _logger && _logger->slog_get_base_log_level() > log_level) return;
    }
    
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
            _logger->slog_write(log_level, log_to, log_buffer);
        }
        else
        {
            if (log_to & log_to_con)
            {
                switch (log_level)
                {
                case log_level_error: // same as log_level_critical
                    write_to_console(wtc_red, log_buffer);
                    break;
                case log_level_info:
                case log_level_warn:
                    write_to_console(wtc_green, log_buffer);
                    break;
                default:
                    write_to_console(wtc_none, log_buffer);
                }
            }

            if (log_to & log_to_ods)
            {
                OutputDebugStringA(log_buffer);
            }
        }
    }
}

/// @brief  Writes log without decoration
void
log_write_fmt_without_deco(
    _In_ uint32_t log_mask,
    _In_ uint32_t log_level,
    _In_ uint32_t log_to,
    _In_z_ const char* fmt,
    _In_ ...
    )
{
    // check log mask
    if (log_mask == (_log_mask & log_mask)) return;

    // check base log level
    {
        boost::lock_guard< boost::mutex > lock(_logger_lock);
        if (NULL != _logger && _logger->slog_get_base_log_level() > log_level) return;
    }

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

    va_start(args, fmt);
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
            _logger->slog_write(log_level, log_to, log_buffer);
        }
        else
        {
            if (log_to & log_to_con)
            {
                switch (log_level)
                {
                case log_level_error: // same as log_level_critical
                    write_to_console(wtc_red, log_buffer);
                    break;
                case log_level_info:
                case log_level_warn:
                    write_to_console(wtc_green, log_buffer);
                    break;
                default:
                    write_to_console(wtc_none, log_buffer);
                }
            }

            if (log_to & log_to_ods)
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
slogger::slogger() 
    : _stop_logger(true), 
      _lock(NULL), 
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
	_In_ uint32_t base_log_level, 	
	_In_opt_z_ const wchar_t *log_file_path
	)
{
	_ASSERTE(NULL == _lock);
    _ASSERTE(NULL == _logger_thread);
    if (NULL != _lock || NULL != _logger_thread) { return false; }
    
	// if log file specified, create log file.
	if (NULL != log_file_path)
	{
		_log_file_handle = open_file_to_write(log_file_path);
		if (INVALID_HANDLE_VALUE == _log_file_handle) return false;
	}

    slog_set_base_log_level(base_log_level);

    _lock = new AKCriticalSection();
    if (TRUE != _lock->Init()) return false;

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

	if(NULL != _lock)
	{
		_lock->Terminate();
		delete (_lock); _lock=NULL;
	}
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
	if (slog_get_base_log_level() > level) return;

	// enqueue log
	log_entry log(level, log_to, log_message);

	_lock->Enter();
    _log_queue.push(log);
    _lock->Leave();
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

        _lock->Enter();
		log_entry log = _log_queue.pop();
        _lock->Leave();
		     
		if (log._log_to & log_to_con)
		{
            switch (log._log_level)
            {
            case log_level_error: // same as log_level_critical
                write_to_console(wtc_red, log._msg.c_str());
                break;
            case log_level_info:
            case log_level_warn: 
                write_to_console(wtc_green, log._msg.c_str());
                break;
            default:
                write_to_console(wtc_none, log._msg.c_str());
            }
		}

		if (log._log_to & log_to_file)
		{
			if (INVALID_HANDLE_VALUE != _log_file_handle)
			{
				write_to_filea(_log_file_handle, "%s", log._msg.c_str());
			}
		}

		if (log._log_to & log_to_ods)
		{			
			OutputDebugStringA(log._msg.c_str());
		}
    }

    // flush all logs to target media only file.
    write_to_console(wtc_green, "[INFO] finalizing logs...");
    _lock->Enter();
        while(true != _log_queue.empty())
        {
            log_entry log = _log_queue.pop();
			if (log._log_to & log_to_file)
			{
				if (INVALID_HANDLE_VALUE != _log_file_handle)
				{
					write_to_filea(_log_file_handle, "%s", log._msg.c_str());
				}
			}
        }
    _lock->Leave();	
    write_to_console(wtc_green, "done.\n");
    //std::cout << boost::format("tid=0x%08x, %s logger thread terminated \n") % GetCurrentThreadId() % __FUNCTION__;
}