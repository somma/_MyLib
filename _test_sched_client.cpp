/**
 * @file    sched_client test
 * @brief
 *
 * @author  Jaehyeon, Park (fixbrain@gmail.com)
 * @date    2019.07.29 10:53 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include "_MyLib/src/sched_client.h"
#include "_MyLib/src/log.h"

typedef struct _task_list {
	const wchar_t* task_name;
	const wchar_t* execute_path;
	const wchar_t* arguments;
	uint32_t interval;
}task_list;

///	@brief	create task scheduler and remove
bool create_remove_task(_In_ const wchar_t* task_name,
						_In_ const wchar_t* execute_path,
						_In_ const wchar_t* arguments,
						_In_ uint32_t interval)
{
	SchedClient sched_client;
	if (true != sched_client.initialize())
	{
		log_err "SchedClient::initialize() failed." log_end;
		return false;
	}

	if (true != sched_client.create_daily_task(task_name,
											   execute_path,
											   L"\\",
											   arguments,
											   (short)interval))
	{
		log_err "SchedClient::easy_create() failed." log_end;
		return false;
	}

	log_info "task scheduler create succeeded. task_name=%ws",
		task_name 
		log_end;

	if (true != sched_client.remove_task(task_name,
										 L"\\"))
	{
		log_err "SchedClient::remove() falied. task_name=%ws",
			task_name
			log_end;
		return false;
	}

	log_info "task scheduler remove succeeded. task_name=%ws", 
		task_name 
		log_end;

	return  true;
}

bool test_sched_client()
{
	std::list<task_list> list = {{L"create first daily task", L"c:\\Windows\\System32\\notepad.exe", nullptr, 1},
								 {L"create second daily task", L"c:\\Windows\\System32\\calc.exe", nullptr, 12},
								 {L"create third daily task", L"c:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe", L"1+2", 23}};

	
	std::list<task_list>::iterator iter = list.begin();
	std::list<task_list>::iterator end = list.end();

	
	for (iter; iter != end; ++iter)
	{
		if (true != create_remove_task((*iter).task_name,
									   (*iter).execute_path,
									   (*iter).arguments,
									   (*iter).interval))
		{
			return  false;
		}
	}
	
	return true;
}