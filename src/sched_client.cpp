/**
 * @file    sched_client.cpp
 * @brief	This module contains a functions that create task scheduler.
 *
 * @author  JaeHyeon, Park (jaehyeon@somma.kr)
 * @date    2019/7/26 11:17 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"

#include "_MyLib/src/log.h"
#include "_MyLib/src/Win32Utils.h"
#include "_MyLib/src/sched_client.h"


SchedClient::SchedClient()
	:_initialized(false), _svc(nullptr)
{
}

SchedClient::~SchedClient()
{
	finalize();
}

///	@brief COM object intialize
bool
SchedClient::initialize()
{
	if (true == _initialized)
	{
		return true;
	}

	// #1, init COM
	HRESULT hres = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	if (!SUCCEEDED(hres))
	{
		log_err "CoInitializeEx() falied. hres=%u", hres log_end;
		return false;
	}

	// #2 set general COM security levels
	hres = CoInitializeSecurity(nullptr,
								-1,								// COM authentication
								nullptr,						// Authentication services
								nullptr,						// Reserved
								RPC_C_AUTHN_LEVEL_DEFAULT,		// Default authentication
								RPC_C_IMP_LEVEL_IMPERSONATE,	// Default Impoersonation
								nullptr,						// Authentication info
								EOAC_NONE,						// Additional capabilities
								nullptr);						// Reserved

	if (!SUCCEEDED(hres))
	{
		//
		//	CoInitializedSecurity() 함수는 프로세스에서 한번만 호출할 수 있다.
		//	두번 호출 되면 RPC_E_TOO_LATE 를 리턴함
		//
		if (RPC_E_TOO_LATE != hres)
		{
			log_err "CoInitializeSecurity() failed. hres=%u", hres log_end;
			CoUninitialize();
			return false;
		}
	}

	// #3 Obtain the initial task service to task scheduler
	hres = CoCreateInstance(CLSID_TaskScheduler,
							nullptr,
							CLSCTX_INPROC_SERVER,
							IID_ITaskService,
							(LPVOID*)&_svc);

	if (!SUCCEEDED(hres))
	{
		log_err "CoCreateInstance() failed. hres=%u", hres log_end;
		CoUninitialize();
		return false;
	}

	// #4, Connect to Task service through ITaskService::Connect
	//
	// _variant_t() -> intialized VARIANT struct
	//				   VARINT.vt = VT_EMPTY;
	//
	hres = _svc->Connect(_variant_t(),			// Server Name.
						 _variant_t(),			// User Name.
						 _variant_t(),			// Domain.
						 _variant_t());			// Password.

	if (!SUCCEEDED(hres))
	{
		log_err "_svc->Connect() failed. hres=%u", hres log_end;
		_svc->Release();
		_svc = nullptr;

		CoUninitialize();
		return false;
	}

	_initialized = true;
	return true;
}

void SchedClient::finalize()
{
	if (true != _initialized) return;
	
	if (nullptr != _svc)
	{
		_svc->Release();
		_svc = nullptr;
	}

	CoUninitialize();

	_initialized = false;
}

bool
SchedClient::initialized()
{
	return _initialized;
}

///	@brief	작업 스케줄러에 등록할 folder_name,과 task_name로 daily task를 생성하고 
///			주기적으로 실행시킬 파일의 경로를 등록한다.
bool
SchedClient::create_daily_task(
	_In_ const wchar_t* task_name,
	_In_ const wchar_t* execute_path,
	_In_ const wchar_t* arguments,
	_In_opt_ const wchar_t* folder_name,
	_In_opt_ short day_interval,
	_In_opt_ const wchar_t* task_interval,
	_In_opt_ const wchar_t* task_duration
)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != task_name || nullptr != execute_path);
	if (nullptr == task_name || nullptr == execute_path) return false;

	ITaskFolder*		folder = nullptr;
	ITaskDefinition*	definition = nullptr;
	IRegistrationInfo*	registration_info = nullptr;
	ITriggerCollection*	trigger_collection = nullptr;
	IRegisteredTask*	registered_task = nullptr;

	HRESULT hres = S_FALSE;
	bool ret = false;
	
	do
	{
		if (true != get_folder_info(folder_name, &folder))
		{
			//log_err "get_folder_info() failed. folder=%ws" log_end;
			break;
		}

		hres = _svc->NewTask(0, &definition);
		if (!SUCCEEDED(hres))
		{
			log_err "_svc->newTask() failed. hres=%u", hres log_end;
			break;
		}

		hres = definition->get_RegistrationInfo(&registration_info);
		if (!SUCCEEDED(hres))
		{
			log_err "definition->get_RegistrationInfo() failed. hres=%u", hres log_end;
			break;
		}

		hres = definition->get_Triggers(&trigger_collection);
		if (!SUCCEEDED(hres))
		{
			log_err "definition->get_Triggers() failed. hres=%u", hres log_end;
			break;
		}

		if (true != make_daily_trigger(trigger_collection, day_interval, task_interval, task_duration))
		{
			log_err "make_daily_trigger() failed." log_end;
			break;
		}

		if (true != make_actions(definition, execute_path, arguments))
		{
			log_err "make_actions() failed. execute_path=%ws, arguments=%ws",
				execute_path,
				arguments
				log_end;
			break;
		}

		hres = folder->RegisterTaskDefinition(_bstr_t(task_name),
											  definition,
											  _default_task_create_flag,
											  _variant_t(_system_level_task_logon_user),
											  _variant_t(),
											  _default_task_logon_type,
											  _variant_t(L""),
											  &registered_task);
		if (!SUCCEEDED(hres))
		{
			log_err "_folder->RegisterTaskDefinition() failed. hres=%u", hres log_end;
			break;
		}
		ret = true;
	} while (false);

	if (folder != nullptr)
	{
		folder->Release(); folder = nullptr;
	}
	if (definition != nullptr)
	{
		definition->Release(); definition = nullptr;
	}
	if (registration_info != nullptr)
	{
		registration_info->Release(); registration_info = nullptr;
	}
	if (trigger_collection != nullptr)
	{
		trigger_collection->Release(); trigger_collection = nullptr;
	}
	if (registered_task != nullptr)
	{
		registered_task->Release(); registered_task = nullptr;
	}
	
	return ret;
}

///	@brief task를 삭제한다.
bool
SchedClient::remove_task(
	_In_ const wchar_t* task_name,
	_In_opt_ const wchar_t* folder_name
)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != task_name || nullptr != folder_name);
	if (nullptr == task_name || nullptr == folder_name) return false;

	ITaskFolder* folder = nullptr;

	HRESULT hres = S_FALSE;
	bool ret = false;
	do
	{
		if (true != get_folder_info(folder_name, &folder))
		{
			//log_err "get_folder_info() failed. folder=%ws" log_end;
			break;
		}
		hres = folder->DeleteTask(_bstr_t(task_name), 0);
		if (!SUCCEEDED(hres))
		{
			log_err "folder->DeleteTask() failed. hres=%u", hres log_end;
			break;
		}
		ret = true;
	} while (false);

	if (nullptr != folder)
	{
		folder->Release(); folder = nullptr;
	}
	return ret;
}

///	@brief task를 활성화 하거나 비활성화 한다.
bool 
SchedClient::change_task_status(
	_In_ const wchar_t* task_name,
	_In_opt_ bool enabled_task,
	_In_opt_ const wchar_t* folder_name
)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != task_name || nullptr != folder_name);
	if (nullptr == task_name || nullptr == folder_name) return false;

	//
	// put_Enabled는 VARIANT_BOOL 타입을 파라미터로 받는데 VARIANT_BOOL은 -1이 참,
	// 0이 거짓이다. 그래서 task를 enabled하게 바꿀려면 enabled_task를 -1,
	// task를 disabled하게 바꿀려면 0으로 매개변수를 넘겨주면 된다. 이외의 값들은 InValied한 값이다.
	//

	IRegisteredTask* registered_task = nullptr;

	HRESULT hres = S_FALSE;
	bool ret = false;
	do
	{
		if (true != get_task_register_info(task_name,
										   &registered_task,
										   folder_name))
		{
			//log_err "get_task_register_info() failed." log_end;
			break;
		}

		hres = registered_task->put_Enabled(enabled_task);
		if (!SUCCEEDED(hres))
		{
			log_err "_rigistered_task->put_Enabled() failed. hres=%u", hres log_end;
			break;
		}
		ret = true;
	} while (false);

	if (nullptr != registered_task)
	{
		registered_task->Release(); registered_task = nullptr;
	}
	
	return ret;
}

///	@brief	태스크가 등록되어있는지 확인한다.
bool
SchedClient::is_task_existW(
	_In_ const wchar_t* task_name,
	_In_opt_ const wchar_t* folder_name
)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != task_name);
	if (nullptr == task_name) return false;

	IRegisteredTask*	registered_task = nullptr;

	if (true != get_task_register_info(task_name,
									   &registered_task, 
									   folder_name))
	{
		return false;
	}
	if (nullptr != registered_task)
	{
		registered_task->Release(); registered_task = nullptr;
	}
	return true;
}

/// @brief	등록된 task의 실행 파일들의 경로를 구한다.
bool
SchedClient::get_task_action_exec_programs(
	_In_ const wchar_t* task_name,
	_Out_ std::vector<std::wstring>& task_exec_path,
	_In_opt_ const wchar_t* folder_name
)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != folder_name || nullptr != task_name);
	if (nullptr == folder_name || nullptr == task_name) return false;

	IRegisteredTask*	registered_task = nullptr;
	ITaskDefinition*	definition = nullptr;
	IActionCollection*	action_collection = nullptr;

	HRESULT hres = S_FALSE;
	bool ret = false;
	do
	{
		if (true != get_task_register_info(task_name, &registered_task, folder_name))
		{
			log_err "get_task_register_info() failed. folder_name=%ws, task_name=%ws",
				folder_name,
				task_name
				log_end;
			break;
		}

		hres = registered_task->get_Definition(&definition);
		if (!SUCCEEDED(hres))
		{
			log_err "registered_task->get_Definition() failed. hres=%u",
				hres
				log_end;
			break;
		}

		hres = definition->get_Actions(&action_collection);
		if (!SUCCEEDED(hres))
		{
			log_err "definition->get_actions() failed. hres=%u",
				hres
				log_end;
			break;
		}
		long task_action_count = 0;
		hres = action_collection->get_Count(&task_action_count);
		if (!SUCCEEDED(hres) || task_action_count < 1)
		{
			log_err "action_collection->get_count() failed. hres=%u",
				hres
				log_end;
			break;
		}

		for (int i = 1; i <= task_action_count; ++i)
		{
			IAction*		action = nullptr;
			IExecAction*	exec_action = nullptr;

			hres = action_collection->get_Item(i, &action);

			if (!SUCCEEDED(hres))
			{
				log_err "action_collection->get_item() failed. hres=%u",
					hres
					log_end;
				continue;
			}

			hres = action->QueryInterface(IID_IExecAction, (PVOID*)&exec_action);
			if (!SUCCEEDED(hres))
			{
				log_err "action->QueryInterface() failed. hres=%u", hres log_end;
				action->Release();
				continue;
			}
			BSTR path;
			hres = exec_action->get_Path(&path);

			if (!SUCCEEDED(hres))
			{
				log_err "action->get_path() failed. hres=%u", hres log_end;
				action->Release();
				exec_action->Release();
				continue;
			}
			_bstr_t action_path;
			action_path.Assign(path);
			task_exec_path.push_back((LPCWSTR)(action_path));
			ret = true;

			::SysFreeString(path);
			action->Release();
			exec_action->Release();
		}
	} while (false);

	if (nullptr != registered_task)
	{
		registered_task->Release(); registered_task = nullptr;
	}
	if (nullptr != definition)
	{
		definition->Release(); definition = nullptr;
	}
	if (nullptr != action_collection)
	{
		action_collection->Release(); action_collection = nullptr;
	}

	return ret;
}

/// @brief	task에 daily 트리거를 등록, start_time과 end_time은 "YYYY-MM-DDTHH-MM-SS"과
///			같은 포멧 형식을 따른다.
///			day_interval
bool
SchedClient::make_daily_trigger(
	_In_ ITriggerCollection* trigger_collection,
	_In_opt_ short day_interval,
	_In_opt_ const wchar_t* task_interval,
	_In_opt_ const wchar_t* task_duration
)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != trigger_collection);
	if (nullptr == trigger_collection) return false;
	
	ITrigger*			trigger = nullptr;
	IDailyTrigger*		daily_trigger = nullptr;
	IRepetitionPattern*	repetition = nullptr;

	HRESULT hres = S_FALSE;
	bool ret = false;
	do
	{
		hres = trigger_collection->Create(TASK_TRIGGER_DAILY, &trigger);
		if (!SUCCEEDED(hres))
		{
			log_err "triggercollection->create() failed. hres=%u", hres log_end;
			break;
		}

		hres = trigger->QueryInterface(IID_IDailyTrigger, (PVOID *)&daily_trigger);
		if (!SUCCEEDED(hres))
		{
			log_err "trigger->QueryInterface() failed. hres=%u", hres log_end;
			break;
		}

		std::wstring start_boundary = MbsToWcsEx(time_now_to_str2().c_str());
		hres = daily_trigger->put_StartBoundary(_bstr_t(start_boundary.c_str()));
		if (!SUCCEEDED(hres))
		{
			log_err "dayilytrigger->put_StartBoundary() failed. hres=%u", hres log_end;
			break;
		}

		hres = daily_trigger->put_DaysInterval(day_interval);
		if (!SUCCEEDED(hres))
		{
			log_err "dailytrigger->put_DaysInterval() failed. hres=%u", hres log_end;
			break;
		}

		hres = daily_trigger->put_Enabled(true);
		if (!SUCCEEDED(hres))
		{
			log_err "dailytrigger->put_enabled() failed. hres=%u", hres log_end;
			break;
		}

		hres = daily_trigger->get_Repetition(&repetition);
		if (!SUCCEEDED(hres))
		{
			log_err "dailytrigger->get_Repetition() fialed. hres=%u", hres log_end;
			break;
		}

		// interval과 duration은 "PT00{H,M}" 포멧을 가지고 1H는 1시간, 1M은 1분을 나타낸다.
		// interval은 기본값으로 30분 , duration은 기본값으로 24시간으로 설정 된다.
		hres = repetition->put_Duration(_bstr_t(task_duration));
		if (!SUCCEEDED(hres))
		{
			log_err "dailytrigger->put_Duration() fialed. hres=%u", hres log_end;
			break;
		}

		hres = repetition->put_Interval(_bstr_t(task_interval));
		if (!SUCCEEDED(hres))
		{
			log_err "dailytrigger->put_Interval() fialed. hres=%u", hres log_end;
			break;
		}
		ret = true;
	} while (false);

	if (nullptr != trigger)
	{
		trigger->Release(); trigger = nullptr;
	}
	if (nullptr != daily_trigger)
	{
		daily_trigger->Release(); daily_trigger = nullptr;
	}
	if (nullptr != repetition)
	{
		repetition->Release(); repetition = nullptr;
	}
	return ret;
}

///	@brief
bool
SchedClient::make_actions(
	_In_ ITaskDefinition* definition,
	_In_ const wchar_t* execute_path,
	_In_ const wchar_t* arguments
)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != definition || nullptr != execute_path);
	if (nullptr == definition || nullptr == execute_path) return false;

	IActionCollection*	action_collection = nullptr;
	IExecAction*		exec_action = nullptr;
	IAction*			action = nullptr;

	HRESULT hres = S_FALSE;
	bool ret = false;
	do
	{
		hres = definition->get_Actions(&action_collection);
		if (!SUCCEEDED(hres))
		{
			log_err "definition->get_Actions() failed. hres=%u", hres log_end;
			break;
		}

		hres = action_collection->Create(TASK_ACTION_EXEC, &action);
		if (!SUCCEEDED(hres))
		{
			log_err "actioncollection->Create() failed. hres=%u", hres log_end;
			break;
		}

		hres = action->QueryInterface(IID_IExecAction, (PVOID*)&exec_action);
		if (!SUCCEEDED(hres))
		{
			log_err "action->QueryInterface() failed. hres=%u", hres log_end;
			break;
		}

		hres = exec_action->put_Path(_bstr_t(execute_path));
		if (!SUCCEEDED(hres))
		{
			log_err "exec_action->put_Path() failed. hres=%u", hres log_end;
			break;
		}

		if (nullptr != arguments)
		{
			hres = exec_action->put_Arguments(_bstr_t(arguments));
			if (!SUCCEEDED(hres))
			{
				log_err "_exec_action->put_Arguments() failed. hres=%u", hres log_end;
				break;
			}
		}
		ret = true;
	} while (false);

	if (nullptr != action_collection)
	{
		action_collection->Release(); action_collection = nullptr;
	}
	if (nullptr != exec_action)
	{
		exec_action->Release(); exec_action = nullptr;
	}
	if (nullptr != action)
	{
		action->Release(); action = nullptr;
	}
		
	return ret;
}

///	@brief
bool
SchedClient::get_folder_info(
	_In_ const wchar_t* folder_name,
	_Out_ ITaskFolder** folder
)
{
	_ASSERTE(nullptr != folder_name);
	if (nullptr == folder_name) return false;

	HRESULT hres = S_FALSE;
	hres = _svc->GetFolder(_bstr_t(folder_name), folder);
	if (!SUCCEEDED(hres))
	{
		log_dbg "_svc->GetFolder() failed. folder=%ws", folder_name log_end;
		return false;
	}

	return true;
}

///	@brief
bool
SchedClient::get_task_register_info(
	_In_ const wchar_t* task_name,
	_Out_ IRegisteredTask** registered_task,
	_In_opt_ const wchar_t* folder_name
)
{
	_ASSERTE(nullptr != folder_name || nullptr != task_name);
	if (nullptr == folder_name || nullptr == task_name) return false;

	ITaskFolder* folder = nullptr;
	
	HRESULT hres = S_FALSE;
	bool ret = false;
	do
	{
		if (true != get_folder_info(folder_name, &folder))
		{
			log_dbg "get_folder_info() failed. folder=%ws" log_end;
			break;
		}

		hres = folder->GetTask(_bstr_t(task_name), registered_task);
		if (!SUCCEEDED(hres))
		{
			log_dbg "folder->GetTask() failed. task_name=%ws", task_name log_end;
			break;
		}
		ret = true;
	} while (false);

	if (nullptr != folder)
	{
		folder->Release(); folder = nullptr;
	}
	return ret;
}