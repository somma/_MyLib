/**
 * @file    sched_client.h
 * @brief	This module contains a functions that create task scheduler.
 *
 * @author  JaeHyeon, Park (jaehyeon@somma.kr)
 * @date    2019/7/26 10:15 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#pragma once

#define _WIN32_DCOM
#include <comdef.h>
#include <taskschd.h>

#pragma comment(lib,"taskschd.lib")

#define _default_sched_folder_name L"\\"
#define _default_day_interval 1

#define _default_task_create_flag TASK_CREATE_OR_UPDATE | TASK_DISABLE
#define _default_task_logon_type TASK_LOGON_INTERACTIVE_TOKEN

#define _default_task_interval L"PT01M"
#define _default_task_duration L"PT24H"
#define _system_level_task_logon_user L"SYSTEM"

typedef class SchedClient
{
public:
	SchedClient();
	virtual ~SchedClient();

	bool initialize();
	void finalize();
	
	bool initialized();

	bool create_daily_task(_In_ const wchar_t* task_name,
						   _In_ const wchar_t* execute_path,
						   _In_ const wchar_t* arguments,
						   _In_opt_ const wchar_t* folder_name = _default_sched_folder_name,
						   _In_opt_ short day_interval = _default_day_interval,
						   _In_opt_ const wchar_t* task_interval = _default_task_interval,
						   _In_opt_ const wchar_t* task_duration = _default_task_duration);

	bool remove_task(_In_ const wchar_t* task_name,
					 _In_opt_ const wchar_t* folder_name = _default_sched_folder_name);

	bool change_task_status(_In_ const wchar_t* task_name,
							_In_opt_ bool enabled_task = true,
							_In_opt_ const wchar_t* folder_name = _default_sched_folder_name);

	bool is_task_existW(_In_ const wchar_t* task_name,
						_In_opt_ const wchar_t* folder_name = _default_sched_folder_name);

	bool get_task_action_exec_programs(_In_ const wchar_t* task_name,
									   _Out_ std::vector<std::wstring>& task_exec_path,
									   _In_opt_ const wchar_t* folder_name = _default_sched_folder_name);


private:
	bool make_daily_trigger(_In_ ITriggerCollection* trigger_collection,
							_In_opt_ short day_interval = _default_day_interval,
							_In_opt_ const wchar_t* task_interval = _default_task_interval,
							_In_opt_ const wchar_t* task_duration = _default_task_duration);

	bool make_actions(_In_ ITaskDefinition* definition,
					  _In_ const wchar_t* execute_path,
					  _In_ const wchar_t* arguments);

private:
	bool get_folder_info(_In_ const wchar_t* folder_name,
						 _Out_ ITaskFolder** folder);

	bool get_task_register_info(_In_ const wchar_t* task_name,
								_Out_ IRegisteredTask** registered_task,
								_In_opt_ const wchar_t* folder_name = _default_sched_folder_name);

private:
	bool _initialized;
	ITaskService*		_svc;
} *PSchedClient;