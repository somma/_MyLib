/**
 * @file    _MyLib test 
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2017/08/13 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include "process_tree.h"
#include <vector>

/**
 * @brief	test for cprocess_tree class 
			
			테스트를 위해서는 
			cmd.exe -> procexp.exe -> procexp64.exe(자동으로 만들어짐) -> notepad.exe
			순서로 프로세스를 생성해 두고 해야 한다. 
**/
bool proc_tree_callback(_In_ process& process_info, _In_ DWORD_PTR callback_tag)
{
    UNREFERENCED_PARAMETER(callback_tag);
	log_info "pid = %u, name = %ws, path = %ws", 
		process_info.pid(), 
		process_info.process_name(),
		process_info.process_path()
		log_end
	return true;
}

/// @brief	
bool test_iterate_process_tree()
{
	cprocess_tree proc_tree;
	if (!proc_tree.build_process_tree(true)) return false;

	///	부모 프로세스가 없는 프로세스 목록을 먼저 생성한다. 
	std::vector<pprocess> top_level_procs;
	proc_tree.iterate_process([&](_In_ process& process_info, _In_ DWORD_PTR callback_tag)->bool
	{
		UNREFERENCED_PARAMETER(callback_tag);
		const process* p = proc_tree.get_parent(process_info);
		if (nullptr == p)
		{
			top_level_procs.push_back(&process_info);
		}

		return true;
	}, 0);

	/// top_level_proces 와 그 자식 프로세스들을 부모->자식 순으로 iterate 한다. 
	size_t count = 0;
	for (auto top_level_proc : top_level_procs)
	{
		log_info "\n\n" log_end;
		proc_tree.iterate_process_tree(*top_level_proc, [&](_In_ process& process_info, _In_ DWORD_PTR callback_tag)->bool 
		{
			UNREFERENCED_PARAMETER(callback_tag);
			log_info "pid = %u, name = %ws, path = %ws",
				process_info.pid(),
				process_info.process_name(),
				process_info.process_path()
				log_end;

			count++;
			return true;
		}, 0);
	}
	_ASSERTE(proc_tree.size() == count);
	return true;
}

bool test_process_tree()
{
	cprocess_tree proc_tree;
	if (!proc_tree.build_process_tree(true)) return false;

	// 프로세스 열거 테스트 (by callback)
	proc_tree.iterate_process(proc_tree_callback, 0);
	proc_tree.iterate_process_tree(proc_tree.find_process(L"cmd.exe"), proc_tree_callback, 0);

	// 프로세스 열거 테스트 (by lambda)
	proc_tree.iterate_process([](_In_ process& process_info, _In_ DWORD_PTR callback_tag)->bool 
	{
		UNREFERENCED_PARAMETER(callback_tag);
		log_info "pid = %u, name = %ws, path = %ws",
			process_info.pid(),
			process_info.process_name(),
			process_info.process_path()
			log_end
			return true;
	},
	0);

	// 프로세스 열거 테스트 (by boost::function, lambda with capture)
	int count = 0;
	boost::function<bool(_In_ process& process_info, _In_ DWORD_PTR callback_tag)> callback = [&count](_In_ process& process_info, _In_ DWORD_PTR callback_tag)->bool
	{
		UNREFERENCED_PARAMETER(callback_tag);
		log_info "pid = %u, name = %ws, path = %ws",
			process_info.pid(),
			process_info.process_name(),
			process_info.process_path()
			log_end;
		++count;
		return true;
	};
	proc_tree.iterate_process(callback, 0);
	_ASSERTE(count > 0);

	// 프로세스 열거 테스트 (by lambda with capture local variable)
	count = 0;
	proc_tree.iterate_process([&count](_In_ process& process_info, _In_ DWORD_PTR callback_tag)->bool
	{
		UNREFERENCED_PARAMETER(callback_tag);
		log_info "pid = %u, name = %ws, path = %ws",
			process_info.pid(),
			process_info.process_name(),
			process_info.process_path()
			log_end;
		count++;
		return true;
	},
	0);
	_ASSERTE(count > 0);

	// 
	//	run notepad.exe & kill & ...
	// 
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFOW si = { 0 }; si.cb = sizeof(si);
	if (!CreateProcessW(L"c:\\windows\\system32\\notepad.exe",
						nullptr,
						nullptr,
						nullptr,
						FALSE,
						0,
						nullptr,
						nullptr,
						&si,
						&pi))
	{
		log_err "CreateProcessW() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}
	Sleep(1000);

	log_info "kill notepad..." log_end;
	_ASSERTE(true == proc_tree.build_process_tree(false));
	proc_tree.print_process_tree(L"notepad.exe");
	_ASSERTE(true == proc_tree.kill_process_tree(proc_tree.find_process(L"notepad.exe"), 
												 false));

	//
	//	print process tree
	//
	log_info "print explorer family..." log_end;
	proc_tree.print_process_tree(L"explorer.exe");
	
	return true;
}

/// @brief	
bool test_image_path_by_pid()
{
	cprocess_tree proc_tree;
	if (!proc_tree.build_process_tree(true)) return false;

	DWORD pid = proc_tree.find_process(L"explorer.exe");

	std::wstring win32_path;
	std::wstring native_path;
	if (!image_path_by_pid(pid, true, win32_path)) return false;
	if (!image_path_by_pid(pid, false, native_path)) return false;

	log_info "pid=%u, explorer.exe, \nwin32_path=%ws\nnative_path=%ws",
		pid,
		win32_path.c_str(),
		native_path.c_str()
		log_end;
	return true;
}

/// @brief	
bool test_get_process_creation_time()
{
	cprocess_tree proc_tree;
	if (!proc_tree.build_process_tree(true)) return false;

	DWORD pid = proc_tree.find_process(L"explorer.exe");

	FILETIME creation_time;
	if (!get_process_creation_time(pid, &creation_time)) return false;

	log_info "pid=%u, explorer.exe, creation_time=%s",
		pid,
		file_time_to_str(&creation_time, true, true).c_str()
		log_end;

	return true;
}