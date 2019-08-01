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

typedef class SchedClient
{
public:
	SchedClient();
	virtual ~SchedClient();

	bool initialize();
	void finalize();
	
	bool create(_In_ const wchar_t* task_name);
	bool easy_create(_In_ const wchar_t* task_name,
					 _In_ const wchar_t* execute_path,
					 _In_ const wchar_t* arguments,
					 _In_ uint32_t interval);

	bool set_author(_In_ const wchar_t* author);
	
	bool set_trigger_daily(_In_ const wchar_t* start_time,
						   _In_ const wchar_t* end_time,
						   _In_ short days_interval,
						   _In_ bool enabled);

	bool set_daily_repet(_In_ const wchar_t* interval = L"PT1H",
						 _In_ const wchar_t* duration = L"PT24H");

	bool set_execute(_In_ const wchar_t* execute_path,
					 _In_ const wchar_t* arguments);
	
	bool save(_In_ TASK_CREATION flag = TASK_CREATE_OR_UPDATE);
	bool remove(_In_ const wchar_t* task_name);
	
	bool enabled(_In_ const wchar_t* task_name);
	bool disabled(_In_ const wchar_t* task_name);

	bool is_task_existW(_In_ const wchar_t* task_name);
	void clear();
private:
	bool _initialized;
	std::wstring _task_name;

	ITaskService*		_svc;
	ITaskFolder*		_folder;
	ITaskDefinition*	_definition;
	IRegistrationInfo*	_registration_info;
	ITriggerCollection*	_trigger_collection;
	ITrigger*			_trigger;
	IDailyTrigger*		_daily_trigger;
	IRepetitionPattern* _repetition;
	IActionCollection*	_action_collection;
	IExecAction*		_exec_action;
	IAction*			_action;
} *PSchedClient;