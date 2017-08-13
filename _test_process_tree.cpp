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


/**
 * @brief	test for cprocess_tree class 
			
			테스트를 위해서는 
			cmd.exe -> procexp.exe -> procexp64.exe(자동으로 만들어짐) -> notepad.exe
			순서로 프로세스를 생성해 두고 해야 한다. 
**/
bool proc_tree_callback(_In_ process& process_info, _In_ DWORD_PTR callback_tag)
{
    UNREFERENCED_PARAMETER(callback_tag);
	con_info "pid = %u, %ws", 
		process_info.pid(), 
		process_info.process_name() 
		log_end
	return true;
}

bool test_process_tree()
{
	cprocess_tree proc_tree;
	if (!proc_tree.build_process_tree()) return false;

	// 프로세스 열거 테스트 (by callback)
	proc_tree.iterate_process(proc_tree_callback, 0);
	proc_tree.iterate_process_tree(proc_tree.find_process(L"cmd.exe"), proc_tree_callback, 0);
	
	// print 
	proc_tree.print_process_tree(L"cmd.exe");

	// 프로세스 종료 테스트	
	proc_tree.kill_process_tree( proc_tree.find_process(L"cmd.exe") );	

    proc_tree.print_process_tree(L"explorer.exe");

	return true;
}

/// @brief	
bool test_image_path_by_pid()
{
	cprocess_tree proc_tree;
	if (!proc_tree.build_process_tree()) return false;

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
	if (!proc_tree.build_process_tree()) return false;

	DWORD pid = proc_tree.find_process(L"explorer.exe");

	FILETIME creation_time;
	if (!get_process_creation_time(pid, &creation_time)) return false;

	log_info "pid=%u, explorer.exe, creation_time=%s",
		pid,
		file_time_to_str(&creation_time, true, true).c_str()
		log_end;

	return true;
}