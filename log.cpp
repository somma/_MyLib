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
static slogger*		_logger = NULL;
static bool			_show_process_name = true;
static bool			_show_pid_tid = true;
static bool			_show_function_name = true;


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
	_In_ LogLevel log_level, 
	_In_opt_z_ const wchar_t* log_file_path
	)
{
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
	InterlockedExchangePointer((PVOID*)&_logger, local_slogger);
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
	_show_process_name = show_process_name;
	_show_pid_tid = show_pid_tid;
	_show_function_name = show_function_name;
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
log_write(
    _In_ LogLevel log_level, 
	_In_ uint32_t log_to,
    _In_z_ const char* function,
    _In_z_ const char* log_message
    )
{
	_ASSERTE(NULL != function);
	_ASSERTE(NULL != log_message);
	if (NULL == function || NULL == log_message) return;
	
	// check base log level
	if (NULL != _logger && _logger->slog_get_base_log_level() > log_level) return;


	// show log level prefix
	std::stringstream log_strm;
	switch (log_level)
	{
	case log_level_debug:   log_strm << "[DBG ] "; break;
	case log_level_info:    log_strm << "[INFO] "; break;
	case log_level_warn:    log_strm << "[WARN] "; break;
	case log_level_error:   log_strm << "[ERR ] "; break;
	case log_level_msg:		log_strm << ""; break;
	default:
		_ASSERTE(!"never reach here!");
		return;
	}

	//> show process name
	if (true == _show_process_name)
	{
		std::string module_file = WcsToMbsEx(get_current_module_fileEx().c_str());
		log_strm << module_file;
	}

	//> show pid, tid
	if (true == _show_pid_tid)
	{
		log_strm << 
			boost::format( "(%+5u:%+5u) : " ) 
			% GetCurrentProcessId() 
			% GetCurrentThreadId();
	}

	//> show function name
	if (true == _show_function_name)
	{
		log_strm << boost::format( "%s : " ) % function;
	}

	//> add new line
	log_strm << log_message;
	log_strm << "\n";


	if (NULL != _logger)
	{
		_logger->slog_write(log_level, log_to, log_strm.str().c_str());
	}
	else
	{		
		if (log_to & log_to_con)
		{
			write_to_console(log_strm.str().c_str());
		}

		if (log_to & log_to_ods)
		{			
			OutputDebugStringA(log_strm.str().c_str());
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
void
log_write_fmt(
    _In_ LogLevel log_level, 
	_In_ uint32_t log_to,
	_In_z_ const char* function,
    _In_z_ const char* fmt, 
    _In_ ...
    )
{
    if (NULL == fmt) return;

	char log_buffer[2048];
    size_t remain = sizeof(log_buffer);
    size_t remain_b = remain;
    char* pos = log_buffer;
    char* pos_b = pos;    
    va_list args;

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
            pos_b, 
            remain_b, 
            &pos_b, 
            &remain_b, 
            0, 
            "invalid function call parameters"
            );		
        remain = remain_b;
        pos = pos_b;
    }    
    va_end(args);

	log_write(log_level, log_to, function, log_buffer);
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
	_In_ LogLevel base_log_level, 	
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
	_In_ LogLevel level, 
	_In_ uint32_t log_to, 
	_In_z_ const char* log_message
	)
{
	_ASSERTE(NULL != log_message);
	if (NULL == log_message) return;

	// check log level
	if (slog_get_base_log_level() > level) return;

	// enqueue log
	log_entry log(log_to, log_message);

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
        // 큐에서 pop 을 하는 스레드는 현재 스레드가 유일하므로 empty 검사시에는
        // lock 이 필요없다.
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
			write_to_console(log._msg.c_str());
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

    //std::cout << boost::format("tid=0x%08x, %s finalizing... \n") % GetCurrentThreadId() % __FUNCTION__;
    _lock->Enter();
        while(true != _log_queue.empty())
        {
            log_entry log = _log_queue.pop();
			if (log._log_to & log_to_con)
			{
				write_to_console(log._msg.c_str());
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
    _lock->Leave();	
    //std::cout << boost::format("tid=0x%08x, %s logger thread terminated \n") % GetCurrentThreadId() % __FUNCTION__;
}