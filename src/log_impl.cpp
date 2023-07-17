/**
 * @file    Logging module
 * @brief   initialize_log() �Լ��� ��������� ȣ���ϸ�, log level, log target
 *			(file, debugger, console, etc) ����/���� ����
 *
 *			log format ����/���� ����
 *			multi thread ȯ�濡�� serialization �� ��
 *			log_err, log_err ���� ��ũ�θ� ����ϸ� debugger, console �� �޼��� ��� ����
 *
 *			Ŀ�θ�� log ���� user mode log ����� ��� DPFLTR_IHVDRIVER_ID ��
 *			����ϱ� ������ WinDbg ���� User mode �޼����� ���͸� �ϰ� ���� ���
 *			`.ofilter` �� ��� ��
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
		// �α� ���ϸ��� ��õǾ���, ������ Ȯ���ڰ� ���� ��� 
		// ������ .log Ȯ���ڸ� �ٿ��ش�. (Ȯ���� ���� ������ �׳� �Ⱦ)
		// 
		std::wstring ext;
		if (false == get_file_extensionw(_log_file_path.c_str(), ext))
		{
			std::wstringstream strm;
			strm << _log_file_path << L".log";
			_log_file_path = strm.str();
		}

		//
		//	���丮�� �������� ������ �����Ѵ�.
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
		// �̹� �����ϴ� �������� �� �α� ���� ����� ���� �ð� ������ �����Ѵ�.
		//
		if (true != enum_old_log_files(_log_file_path.c_str()))
		{
			return false;
		}

		//
		//	������ �ִ� �α� ������ �������� �Ѵ�. 
		//
		if (true != rotate_log_file(_log_file_path.c_str()))
		{
			return false;
		}
		_ASSERTE(INVALID_HANDLE_VALUE != _log_file_handle);

		//
		// ������ �α� ������ �ִٸ� �����Ѵ�. 
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
 * @brief	log ť�� �α׸� push �Ѵ�.
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
		// invalid character �� �����ִ� ��� �߻� �� �� ����
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
	// �� rotate_log_file() �Լ��� �������� ���� �ֱ⶧����
	// �α������� �����ϴ��� Ȯ���Ѵ�. 
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
		// ������Ʈ �� ������ ��Ͽ� �߰��Ѵ�.
		// _log_files ����Ʈ�� ������ ���� ������ ����� ��������Ƿ�
		// ���� ����� �����ϴ� ���� (enum_old_log_files()�Լ�)�� ������ ������������
		// file �� ctime �� ��Ȯ�� ���� �ʿ����. 
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

/// @brief	�̹� ������Ʈ �� �α� ������ �ִٸ� ctime �������� ����� �����Ѵ�.
///			`���ϸ�.2018-11-12_23-55-12.Ȯ����` ���·� ��������Ƿ� 
///			`���ϸ�.*.Ȯ����` ������ ������ ��� �������� �� �α����Ϸ� �����Ѵ�. 
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
						   // ������ ���� ���� 
						   // 
						   return false;
					   }

					   FILETIME ctime;
					   if (!GetFileTime(f.get(), &ctime, nullptr, nullptr))
					   {
						   //
						   // ���� �����ð��� ������ ���ϸ�, ó�� �Ұ���
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
		// log file ����Ʈ�� ctime �������� �����Ѵ�.
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
		// �������� �� ���� ����Ʈ���� �����ߴ� �ϴ��� �α׸�� ������ �����ϰ� 
		// �������� �ʴ´�. 
		//
	}
	return true;
}

/// @brief	log ������ ������ ���� �� ���� ���� ��� ���� ������ �α����Ϻ��� �����Ѵ�.
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
				//	rotate_log_file() �� �����ϸ� _log_count �� �ʱ�ȭ���� �ʴ´�. 
				//	� �����ε� rotate_log_file() ������ ��� _log_count �� 
				//	�ʱ�ȭ ���� �ʾұ� ������ ��� �� �õ��ϰ� �ȴ�. 
				// 
				//	rotate_log_file() �� �����Ҷ� ���� file log �Ѱ��� ���ǵ�����
				//	�������� �׳� �����Ѵ�.
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
					// ������ �α� ������ �ִٸ� �����Ѵ�.
					//
					remove_old_log_files(_log_files);

					//
					// ���ο� �α� ���Ͽ� �α׸� ����.
					//
					_log_count++;
					write_to_filea(_log_file_handle, "%s", log->_msg.c_str());
				}
			}
			else
			{
				//
				// �� �������� _log_file_handle �� �׻� ��ȿ�ϴ�. 
				//
				_ASSERTE(INVALID_HANDLE_VALUE != _log_file_handle);
				_log_count++;
				write_to_filea(_log_file_handle, "%s", log->_msg.c_str());
			}

			//
			//	30 �� ���� File �� Flush �Ѵ�. 
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
	//	logger ���� ��û�� ���� ����.
	//	�α� ť�� �ִ� �α׵��� ���Ͽ� ��� ����(log_to_file �� ������ ���).
	//	console, ods ���� �׳� ������.
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
					//	rotate_log_file() �� �����ϸ� _log_count �� �ʱ�ȭ���� �ʴ´�. 
					//	� �����ε� rotate_log_file() ������ ��� _log_count �� 
					//	�ʱ�ȭ ���� �ʾұ� ������ ��� �� �õ��ϰ� �ȴ�. 
					// 
					//	rotate_log_file() �� �����Ҷ� ���� file log �Ѱ��� ���ǵ�����
					//	�������� �׳� �����Ѵ�.
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
						// ������ �α� ������ �ִٸ� �����Ѵ�.
						//
						remove_old_log_files(_log_files);

						//
						// ���ο� �α� ���Ͽ� �α׸� ����.
						//
						_log_count++;
						write_to_filea(_log_file_handle, "%s", log->_msg.c_str());
					}
				}
				else
				{
					//
					// �� �������� _log_file_handle �� �׻� ��ȿ�ϴ�. 
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