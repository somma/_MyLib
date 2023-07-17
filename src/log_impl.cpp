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
#include "_MyLib/src/log_impl.h"


/**
 * @brief	constructor
 */
slogger::slogger(_In_ uint32_t log_id,
				 _In_ uint32_t log_mask,
				 _In_ uint32_t log_level,
				 _In_ uint32_t log_to,
				 _In_opt_z_ const wchar_t* log_file_path,
				 _In_ uint32_t max_log_count,
				 _In_ uint32_t max_log_files) :
	_id(log_id),
	_stop_logger(true),

	_show_level(true),
	_show_current_time(true),
	_show_process_name(true),
	_show_pid_tid(true),
	_show_function_name(true),
	_log_mask(log_mask),
	

	_log_level(log_level),
	_log_to(log_to),
	_max_log_count(max_log_count),
	_max_log_files(max_log_files),

	_log_file_path(nullptr != log_file_path ? log_file_path : L""),
	_log_file_handle(INVALID_HANDLE_VALUE), 
	
	_log_count(0),
	_logger_thread(nullptr)
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
bool slogger::slog_start()
{
	_ASSERTE(nullptr == _logger_thread);
	if (nullptr != _logger_thread) { return false; }

	_process_name = get_current_module_fileEx();

	if (FlagOn(_log_to, log_to_file) && !_log_file_path.empty())
	{
		//
		// 로그 파일명이 명시되었고, 파일의 확장자가 없는 경우 
		// 강제로 .log 확장자를 붙여준다. (확장자 없는 파일이 그냥 싫어서)
		// 
		std::wstring ext;
		if (false == get_file_extensionw(_log_file_path.c_str(), ext))
		{
			std::wstringstream strm;
			strm << _log_file_path << L".log";
			_log_file_path = strm.str();
		}

		//
		//	디렉토리가 존재하지 않으면 생성한다.
		//
		std::wstring dir = directory_from_file_pathw(_log_file_path.c_str());
		if (true != is_file_existsW(dir))
		{
			if (true != WUCreateDirectory(dir))
			{
				return false;
			}
		}

		//
		// 이미 존재하는 로테이팅 된 로그 파일 목록을 생성 시간 순으로 생성한다.
		//
		if (true != enum_old_log_files(_log_file_path.c_str()))
		{
			return false;
		}

		//
		//	이전에 있던 로그 파일을 로테이팅 한다. 
		//
		if (true != rotate_log_file(_log_file_path.c_str()))
		{
			return false;
		}
		_ASSERTE(INVALID_HANDLE_VALUE != _log_file_handle);

		//
		// 오래된 로그 파일이 있다면 삭제한다. 
		//
		remove_old_log_files(_log_files);
	}

	_stop_logger = false;
	_logger_thread = new std::thread(std::bind(&slogger::slog_thread, this));
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
	_In_ uint32_t log_mask,
	_In_ uint32_t log_level,
	_In_z_ const char* function,
	_In_z_ const char* fmt,
	_In_ va_list args
)
{
	_ASSERTE(nullptr != function);
	if (nullptr == function) return;

	if (log_mask != (_log_mask & log_mask)) return;
	if (log_level > _log_level) return;

	char log_buffer[2048];
	size_t remain = sizeof(log_buffer);
	char* pos = log_buffer;
	
	if (true == _show_current_time)
	{
		StringCbPrintfExA(
			pos,
			remain,
			&pos,
			&remain,
			0,
			"%s ",
			time_now_to_str(true, false).c_str());
	}

	// log level
	if (_show_level)
	{
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
	}
	
	//> show process name
	if (_show_process_name)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "%ws",
						  _process_name.c_str());
	}

	//> show pid, tid
	if (_show_pid_tid)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "(%+5u:%+5u) : ",
						  GetCurrentProcessId(),
						  GetCurrentThreadId());
	}

	//> show function name
	if (_show_function_name)
	{
		StringCbPrintfExA(pos, 
						  remain, 
						  &pos, 
						  &remain, 
						  0, 
						  "%s : ", 
						  function);
	}

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

	// line feed
	StringCbPrintfExA(pos, remain, &pos, &remain, 0, "\n");

	// enqueue log
	plog_entry le = new log_entry(log_level, log_buffer);

	std::lock_guard<std::mutex> lock(_lock);
	_log_queue.push(le);
}

/// @brief	
bool slogger::rotate_log_file(_In_ const wchar_t* log_file_path)
{
	//
	// 전 rotate_log_file() 함수가 실패했을 수도 있기때문에
	// 로그파일이 존재하는지 확인한다. 
	//
	if (true == is_file_existsW(log_file_path))
	{
		std::wstring ext;
		get_file_extensionw(log_file_path, ext);

		wchar_t buf[128];
		SYSTEMTIME time; GetLocalTime(&time);
		_ASSERTE(true != ext.empty());
		if (!SUCCEEDED(StringCbPrintfW(buf,
									   sizeof(buf),
									   L"%04u-%02u-%02u_%02u-%02u-%02u.%s",
									   time.wYear,
									   time.wMonth,
									   time.wDay,
									   time.wHour,
									   time.wMinute,
									   time.wSecond,
									   ext.c_str())))
		{
			_ASSERTE(!"oops! need more buffer");
			return false;
		}

		if (INVALID_HANDLE_VALUE != _log_file_handle)
		{
			write_to_filea(_log_file_handle, "> log rotated -> %s", WcsToMbsEx(buf).c_str());

			CloseHandle(_log_file_handle);
			_log_file_handle = INVALID_HANDLE_VALUE;
		}

		std::wstringstream path;
		path << extract_last_tokenExW(log_file_path, L".", true) << L"." << buf;
		if (!MoveFileW(log_file_path, path.str().c_str()))
		{
			return false;
		}

		//
		// 로테이트 된 파일을 목록에 추가한다.
		// _log_files 리스트는 오래된 파일 순서로 목록이 만들어지므로
		// 최초 목록을 생성하는 시점 (enum_old_log_files()함수)을 제외한 나머지에서는
		// file 의 ctime 을 정확히 구할 필요없다. 
		// 
		FILETIME now; GetSystemTimeAsFileTime(&now);
		log_file_and_ctime fc(path.str().c_str(), now);
		_log_files.push_back(fc);
	}

	if (true == is_file_existsW(log_file_path))
	{
		//_ASSERTE(!"oops");
		return false;
	}

	_ASSERTE(INVALID_HANDLE_VALUE == _log_file_handle);
	_log_file_handle = CreateFileW(log_file_path,
								   GENERIC_ALL,
								   FILE_SHARE_WRITE | FILE_SHARE_READ,
								   NULL,
								   CREATE_NEW,
								   FILE_ATTRIBUTE_NORMAL,
								   NULL);
	if (INVALID_HANDLE_VALUE == _log_file_handle)
	{
		return false;
	}

	_log_count = 0;	///<!
	return true;
}

/// @brief	이미 로테이트 된 로그 파일이 있다면 ctime 기준으로 목록을 생성한다.
///			`파일명.2018-11-12_23-55-12.확장자` 형태로 만들어지므로 
///			`파일명.*.확장자` 형태의 파일을 모두 로테이팅 된 로그파일로 간주한다. 
bool slogger::enum_old_log_files(_In_ const wchar_t* log_file_path)
{
	std::wstring ext;
	get_file_extensionw(log_file_path, ext);

	std::wstringstream search;
	_ASSERTE(true != ext.empty());
	search
		<< extract_last_tokenExW(log_file_path, L".", true)
		<< L".*."
		<< ext;

	bool ret = \
		find_files(search.str().c_str(),
				   reinterpret_cast<DWORD_PTR>(&_log_files),
				   false,
				   [](DWORD_PTR tag, const wchar_t* path)->bool
				   {
					   _ASSERTE(0 != tag);
					   std::list<log_file_and_ctime>* files_ptr = (std::list<log_file_and_ctime>*)tag;

					   handle_ptr f(CreateFile(path,
											   GENERIC_READ,
											   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
											   NULL,
											   OPEN_EXISTING,
											   FILE_ATTRIBUTE_NORMAL,
											   NULL),
									[](HANDLE file_handle) {
										if (INVALID_HANDLE_VALUE != file_handle)
										{
											CloseHandle(file_handle);
										}
									});
					   if (INVALID_HANDLE_VALUE == f.get())
					   {
						   //
						   // 파일을 열기 에러 
						   // 
						   return false;
					   }

					   FILETIME ctime;
					   if (!GetFileTime(f.get(), &ctime, nullptr, nullptr))
					   {
						   //
						   // 파일 생성시각을 구하지 못하면, 처리 불가능
						   //
						   return false;
					   }
					   log_file_and_ctime lfc(path, ctime);
					   files_ptr->push_back(lfc);
					   return true;
				   });

	if (ret)
	{
		//
		// log file 리스트를 ctime 기준으로 정렬한다.
		//
		_log_files.sort([](log_file_and_ctime& lhs, log_file_and_ctime& rhs) {
			if (file_time_to_int(&lhs.ctime) < file_time_to_int(&rhs.ctime))
			{
				return true;
			}
			else
			{
				return false;
			}
		});
	}
	else
	{
		//
		// 로테이팅 된 파일 리스트업에 실패했다 하더라도 로그모듈 구동을 실패하게 
		// 만들지는 않는다. 
		//
	}
	return true;
}

/// @brief	log 파일의 갯수가 설정 값 보다 많은 경우 가장 오래된 로그파일부터 삭제한다.
void slogger::remove_old_log_files(_In_ std::list<log_file_and_ctime>& log_files)
{
	int count_to_remove = (int)(log_files.size() - _max_log_files);
	while (count_to_remove > 0)
	{
		const log_file_and_ctime& fc = _log_files.front();
		DeleteFileW(fc.path.c_str());
		_log_files.pop_front();
		--count_to_remove;
	}
}

///	@brief	logger worker thread
///			pops log entry from log queue and writes log to output
void slogger::slog_thread()
{
	FILETIME prev_flushed; GetSystemTimeAsFileTime(&prev_flushed);

	while (true != _stop_logger)
	{
		if (true == _log_queue.empty())
		{
			Sleep(100);
			continue;
		}

		plog_entry log = NULL;
		{
			std::lock_guard<std::mutex> lock(_lock);
			log = _log_queue.pop();
		}

		if (FlagOn(_log_to, log_to_con))
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

		if (FlagOn(_log_to, log_to_ods))
		{
			dbg_print(log->_log_level, log->_msg.c_str());
		}

		if (FlagOn(_log_to, log_to_file))
		{
			_ASSERTE(INVALID_HANDLE_VALUE != _log_file_handle);

			if (_log_count >= _max_log_count)
			{
				//
				//	rotate_log_file() 가 실패하면 _log_count 는 초기화되지 않는다. 
				//	어떤 이유로든 rotate_log_file() 실패한 경우 _log_count 가 
				//	초기화 되지 않았기 때문에 계속 재 시도하게 된다. 
				// 
				//	rotate_log_file() 가 실패할때 마다 file log 한개씩 유실되지만
				//	그정도는 그냥 포기한다.
				// 

				if (true != rotate_log_file(_log_file_path.c_str()))
				{
					if (FlagOn(_log_to, log_to_con))
					{
						write_to_console(fc_red, "[ERR ] rotate_log_file() failed.");
					}
					dbg_print(log_level_error, "[ERR ] rotate_log_file() failed.");
				}
				else
				{
					//
					// 오래된 로그 파일이 있다면 삭제한다.
					//
					remove_old_log_files(_log_files);

					//
					// 새로운 로그 파일에 로그를 쓴다.
					//
					_log_count++;
					write_to_filea(_log_file_handle, "%s", log->_msg.c_str());
				}
			}
			else
			{
				//
				// 이 시점에서 _log_file_handle 은 항상 유효하다. 
				//
				_ASSERTE(INVALID_HANDLE_VALUE != _log_file_handle);
				_log_count++;
				write_to_filea(_log_file_handle, "%s", log->_msg.c_str());
			}

			//
			//	30 초 마다 File 을 Flush 한다. 
			//
			FILETIME now; GetSystemTimeAsFileTime(&now);
			if (file_time_delta_sec(&now, &prev_flushed) > 30)
			{
				prev_flushed = now;
				_ASSERTE(INVALID_HANDLE_VALUE != _log_file_handle);
				if (INVALID_HANDLE_VALUE != _log_file_handle)
				{
					FlushFileBuffers(_log_file_handle);
				}
			}

		}

		delete log;
	}

	//
	//	logger 종료 요청을 받은 상태.
	//	로그 큐에 있는 로그들을 파일에 모두 쓴다(log_to_file 이 설정된 경우).
	//	console, ods 등은 그냥 버린다.
	//
	{
		std::lock_guard<std::mutex> lock(_lock);

		while (true != _log_queue.empty())
		{
			plog_entry log = _log_queue.pop();
			if (FlagOn(_log_to, log_to_file))
			{
				if (_log_count >= _max_log_count)
				{
					//
					//	rotate_log_file() 가 실패하면 _log_count 는 초기화되지 않는다. 
					//	어떤 이유로든 rotate_log_file() 실패한 경우 _log_count 가 
					//	초기화 되지 않았기 때문에 계속 재 시도하게 된다. 
					// 
					//	rotate_log_file() 가 실패할때 마다 file log 한개씩 유실되지만
					//	그정도는 그냥 포기한다.
					// 

					if (true != rotate_log_file(_log_file_path.c_str()))
					{
						if (FlagOn(_log_to, log_to_con))
						{
							write_to_console(fc_red, "[ERR ] rotate_log_file() failed.\n");
						}
						dbg_print(log_level_error, "ERR ] rotate_log_file() failed.\n");
					}
					else
					{
						//
						// 오래된 로그 파일이 있다면 삭제한다.
						//
						remove_old_log_files(_log_files);

						//
						// 새로운 로그 파일에 로그를 쓴다.
						//
						_log_count++;
						write_to_filea(_log_file_handle, "%s", log->_msg.c_str());
					}
				}
				else
				{
					//
					// 이 시점에서 _log_file_handle 은 항상 유효하다. 
					//
					_ASSERTE(INVALID_HANDLE_VALUE != _log_file_handle);
					_log_count++;
					write_to_filea(_log_file_handle, "%s", log->_msg.c_str());
				}
			}

			delete log;
		}
		_ASSERTE(true == _log_queue.empty());
	}
}

/// @brief	
void
slogger::set_log_format(
	_In_ bool show_level,
	_In_ bool show_current_time,
	_In_ bool show_process_name,
	_In_ bool show_pid_tid,
	_In_ bool show_function_name
)
{
	_show_level = show_level;
	_show_current_time = show_current_time;
	_show_process_name = show_process_name;
	_show_pid_tid = show_pid_tid;
	_show_function_name = show_function_name;
}

/// @brief	
void
slogger::get_log_format(
	_Out_ bool& show_level,
	_Out_ bool& show_current_time,
	_Out_ bool& show_process_name,
	_Out_ bool& show_pid_tid,
	_Out_ bool& show_function_name
)
{
	show_level = _show_level;
	show_current_time = _show_current_time;
	show_process_name = _show_process_name;
	show_pid_tid = _show_pid_tid;
	show_function_name = _show_function_name;
}

void slogger::set_log_env(
	_In_ uint32_t log_mask,
	_In_ uint32_t log_level
)
{
	_log_mask = log_mask;
	_log_level = log_level;
}

void slogger::get_log_env(
	_Out_ uint32_t& log_mask,
	_Out_ uint32_t& log_level
)
{
	log_mask = _log_mask;
	log_level = _log_level;
}




// refac - del
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