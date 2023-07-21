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
#pragma once
#include "_MyLib/src/BaseWindowsHeader.h"
#include "_MyLib/src/log.h"
#include <atomic>

typedef class slogger
{
public:
	explicit slogger(_In_ uint32_t log_id,
					 _In_ uint32_t log_mask,
					 _In_ uint32_t log_level,
					 _In_ uint32_t log_to,
					 _In_opt_z_ const wchar_t*log_file_path,
					 _In_ uint32_t max_log_count = _max_log_count_def,
					 _In_ uint32_t max_log_files = _max_log_files_def);
	virtual ~slogger();

	bool slog_start();
	void slog_stop();

	uint32_t log_level()  { return _log_level; }	
	uint32_t log_to()  { return _log_to; }

	void slog_write(
		_In_ uint32_t log_mask,
		_In_ uint32_t log_level,
		_In_z_ const char* function,
		_In_z_ const char* fmt,
		_In_ va_list args
	);

	void slog_write(
		_In_ uint32_t log_mask,
		_In_ uint32_t log_level,
		_In_z_ const char* function,
		_In_z_ const char* fmt,
		...
	);
	
	void set_log_format(
		_In_ bool show_level,
		_In_ bool show_current_time,
		_In_ bool show_process_name,
		_In_ bool show_pid_tid,
		_In_ bool show_function_name
	);
	void get_log_format(
		_Out_ bool& show_level,
		_Out_ bool& show_current_time,
		_Out_ bool& show_process_name,
		_Out_ bool& show_pid_tid,
		_Out_ bool& show_function_name
	);
	
	void set_log_env(
		_In_ uint32_t log_mask,
		_In_ uint32_t log_level		
	);

	void get_log_env(
		_Out_ uint32_t& log_mask,
		_Out_ uint32_t& log_level
	);

private:
	std::atomic<bool> _stop_logger; 
	
	uint32_t _id;
	std::wstring _process_name;

	
	std::atomic<bool> _show_level = true;
	std::atomic<bool> _show_current_time = true;
	std::atomic<bool> _show_process_name = true;
	std::atomic<bool> _show_pid_tid = true;
	std::atomic<bool> _show_function_name = true;
	
	std::atomic<uint32_t> _log_mask;

	std::atomic<uint32_t> _log_level; 
	std::atomic<uint32_t> _log_to;
		
	std::atomic<uint32_t> _max_log_count;
	std::atomic<uint32_t> _max_log_files;
	
	class log_file_and_ctime
	{
	public:
		log_file_and_ctime() : ctime{ 0, 0 } { }
		log_file_and_ctime(const wchar_t* log_file_path, FILETIME& log_file_ctime) :
			path(log_file_path),
			ctime(log_file_ctime)
		{}

		std::wstring path;
		FILETIME ctime;
	};
	
	std::list<log_file_and_ctime> _log_files;
	int64_t _log_count;
	std::wstring _log_file_path;
	volatile HANDLE _log_file_handle;


	typedef class log_entry
	{
	public:
		log_entry(_In_ uint32_t log_level, const char* log_message)
			: _log_level(log_level), _msg(log_message)
		{
		}

		uint32_t    _log_level;
		std::string _msg;
	} *plog_entry;
	
	std::mutex _lock;
	Queue<plog_entry>	_log_queue;
	std::thread*		_logger_thread;	

private:
	bool rotate_log_file(_In_ const wchar_t* log_file_path);
	bool enum_old_log_files(_In_ const wchar_t* log_file_path);
	void remove_old_log_files(_In_ std::list<log_file_and_ctime>& log_files);

	void slog_thread();

#ifdef MYLIB_TEST
	friend bool test_log_rotate_with_ext();
	friend bool test_log_rotate_without_ext();
#endif

} *pslogger;


