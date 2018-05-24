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
			
			�׽�Ʈ�� ���ؼ��� 
			cmd.exe -> procexp.exe -> procexp64.exe(�ڵ����� �������) -> notepad.exe
			������ ���μ����� ������ �ΰ� �ؾ� �Ѵ�. 
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

bool test_process_tree()
{
	
	cprocess_tree proc_tree;
	if (!proc_tree.build_process_tree(true)) return false;

	// ���μ��� ���� �׽�Ʈ (by callback)
	proc_tree.iterate_process(proc_tree_callback, 0);
	proc_tree.iterate_process_tree(proc_tree.find_process(L"cmd.exe"), proc_tree_callback, 0);

	// print 
	proc_tree.print_process_tree(L"cmd.exe");

	// ���μ��� ���� �׽�Ʈ	
	proc_tree.kill_process_tree(proc_tree.find_process(L"cmd.exe"));

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