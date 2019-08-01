/**
 * @file    sched_client.h
 * @brief	This module contains a functions that create task scheduler.
 *
 * @author  JaeHyeon, Park (jaehyeon@somma.kr)
 * @date    2019/7/26 11:17 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"
#include "sched_client.h"

SchedClient::SchedClient()
	:_initialized(false), _task_name(_null_stringw), _svc(nullptr), _folder(nullptr),
	_definition(nullptr), _registration_info(nullptr), _trigger_collection(nullptr),
	_trigger(nullptr),_daily_trigger(nullptr),_repetition(nullptr),
	_action_collection(nullptr), _exec_action(nullptr), _action(nullptr)
{
}

SchedClient::~SchedClient()
{
	finalize();
}

bool
SchedClient::initialize()
{
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
		//	CoInitializedSecurity() �Լ��� ���μ������� �ѹ��� ȣ���� �� �ִ�.
		//	�ι� ȣ�� �Ǹ� RPC_E_TOO_LATE �� ������
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

	// #5 Get the root task folder.
	hres = _svc->GetFolder(_bstr_t(L"\\"),
						   &_folder);

	if (!SUCCEEDED(hres))
	{
		log_err "_svc->GetFolder() failed. hres=%u", hres log_end;
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
		
	_ASSERTE(nullptr != _svc);
	_ASSERTE(nullptr != _folder);
		
	_svc->Release();
	_svc = nullptr;
	_folder->Release();
	_folder = nullptr;

	if (nullptr != _definition)
	{
		_definition->Release();
		_definition = nullptr;
	}
	if (nullptr != _registration_info)
	{
		_registration_info->Release();
		_registration_info = nullptr;
	}
	if (nullptr != _trigger_collection)
	{
		_trigger_collection->Release();
		_trigger_collection = nullptr;
	}
	if (nullptr != _trigger)
	{
		_trigger->Release();
		_trigger = nullptr;
	}
	if (nullptr != _daily_trigger)
	{
		_daily_trigger->Release();
		_daily_trigger = nullptr;
	}
	if (nullptr != _repetition)
	{
		_repetition->Release();
		_repetition = nullptr;
	}
	if (nullptr != _action_collection)
	{
		_action_collection->Release();
		_action_collection = nullptr;
	}
	if (nullptr != _exec_action)
	{
		_exec_action->Release();
		_exec_action = nullptr;
	}
	if(nullptr != _action)
	{
		_action->Release();
		_action = nullptr;
	}

	CoUninitialize();

	_initialized = false;
}

/// @breif	SchedClient::create�� �۾������ٷ��� ����� task �̸��� �Է¹޾� task�� �����Ѵ�.
///			task�� �����ϴ��� SchedClient::save�� ȣ������ ������ �۾� �����ٷ���	��� ���� 
///			�����Ƿ� ���� �۾�(author, trigger, execute) �� SchedClient::save�� ȣ���Ͽ� 
///			�۾� �����ٷ��� ����Ѵ�.
bool
SchedClient::create(_In_ const wchar_t* task_name)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != task_name);
	if (nullptr == task_name) return false;

	_task_name = task_name;
	
	HRESULT hres = _svc->NewTask(0, &_definition);
	if (!SUCCEEDED(hres))
	{
		log_err "_svc->NewTask() failed. hres=%u", hres log_end;
		return false;
	}

	hres = _definition->get_RegistrationInfo(&_registration_info);
	if (!SUCCEEDED(hres))
	{
		log_err "_definition->get_RegistrationInfo() failed. hres=%u", hres log_end;
		return false;
	}

	hres = _definition->get_Triggers(&_trigger_collection);
	if (!SUCCEEDED(hres))
	{
		log_err "_definition->get_RegistrationInfo() failed. hres=%u", hres log_end;
		return false;
	}

	return true;
}

///	@brief
bool SchedClient::easy_create(_In_ const wchar_t* task_name,
							  _In_ const wchar_t* execute_path,
							  _In_ const wchar_t* arguments,
							  _In_ uint32_t interval
	)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != task_name);
	_ASSERTE(nullptr != execute_path);
	_ASSERTE(0 < interval && 24 >= interval);

	if (nullptr == task_name) return false;
	if (nullptr == execute_path) return false;
	if (0 > interval && 24 < interval) return false;

	if (true != create(task_name))
	{
		log_err "task create() failed." log_end;
		return false;
	}

	if (true != set_trigger_daily(nullptr,
								  nullptr,
								  1,
								  true))
	{
		log_err "task set_trigger_daily() failed. " log_end;
		return false;
	}

	std::wstringstream time;
	time << L"PT" << interval << L"H";

	if (true != set_daily_repet(time.str().c_str(), L"PT24H"))
	{
		log_err "task set_daily_repet() failed." log_end;
		return false;
	}

	if (true != set_execute(execute_path,arguments))
	{
		log_err " task set_execute() failed." log_end;
		return false;
	}

	if (true != save(TASK_CREATE_OR_UPDATE))
	{
		log_err "task save() failed." log_end;
		return false;
	}

	return true;
}
/// @brief	SchedClinet::set_author�� task�� ������ ���� �̸��� ����
bool
SchedClient::set_author(
	_In_ const wchar_t* author
	)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;
	
	_ASSERTE(nullptr != author);
	if (nullptr == author) return false;

	HRESULT hres = _registration_info->put_Author(_bstr_t(author));
	if (!SUCCEEDED(hres))
	{
		log_err "_registraioninfo->put_Author() failed. hres=%u", hres log_end;
		return false;
	}

	return true;
}

/// @brief	task�� daily Ʈ���Ÿ� ���, start_time�� end_time�� "YYYY-MM-DDTHH-MM-SS"��
///			���� ���� ������ ������.
///			start_time�� nullptr ��� task ��Ͻð��� localtime���� ����ϰ� �ǰ� 
///			end_time�� nullptr ��쿡 expire�� ��Ȱ��ȭ�Ǿ� task�� ���������� �����ϰ� �ȴ�.
bool
SchedClient::set_trigger_daily(
	_In_ const wchar_t* start_time,
	_In_ const wchar_t* end_time,
	_In_ short days_interval,
	_In_ bool enabled
	)
{
	_ASSERTE(true == _initialized);	
	if (true != _initialized) return false;

	if (0 > days_interval || 31 < days_interval) return false;

	HRESULT hres = _trigger_collection->Create(TASK_TRIGGER_DAILY, &_trigger);
	if (!SUCCEEDED(hres))
	{
		log_err "_triggercollection->create() failed. hres=%u", hres log_end;
		return false;
	}

	hres = _trigger->QueryInterface(IID_IDailyTrigger,
		                            (PVOID *)&_daily_trigger);
	if (!SUCCEEDED(hres))
	{
		log_err "_trigger->QueryInterface() failed. hres=%u", hres log_end;
		return false;
	}

	std::wstring start_boundary;
	if (nullptr == start_time)
	{
		start_boundary = MbsToWcsEx(time_now_to_str2().c_str());
	}
	else
	{
		start_boundary = start_time;
	}

	hres = _daily_trigger->put_StartBoundary(_bstr_t(start_boundary.c_str()));
	if (!SUCCEEDED(hres))
	{
		log_err "_dayilytrigger->put_StartBoundary() failed. hres=%u", hres log_end;
		return false;
	}

	if (nullptr != end_time)
	{
		hres = _daily_trigger->put_EndBoundary(_bstr_t(end_time));
		if (!SUCCEEDED(hres))
		{
			log_err "_dailytrigger->put_EndBoundary() failed. hres=%u", hres log_end;
			return false;
		}
	}

	hres = _daily_trigger->put_DaysInterval(days_interval);
	if (!SUCCEEDED(hres))
	{
		log_err "_dailytrigger->put_DaysInterval() failed. hres=%u", hres log_end;
		return false;
	}

	hres = _daily_trigger->put_Enabled(enabled);
	if (!SUCCEEDED(hres))
	{
		log_err "_dailytrigger->put_enabled() failed. hres=%u", hres log_end;
		return false;
	}
	return true;
}

///	@brief	daily Ʈ���ſ��� �ݺ� �۾��� ����Ѵ�. interval�� duration�� "PT00{H,M}" ������ ������
///			1H�� 1�ð�, 1M�� 1���� ��Ÿ����. interval�� duration�� nullptr�� ��� �⺻������ 
///			interval�� 1�ð� , duration�� 24�ð����� ���� �ȴ�.
bool
SchedClient::set_daily_repet(
	_In_ const wchar_t* interval,
	_In_ const wchar_t* duration
	)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;
	
	_ASSERTE(nullptr != interval);
	_ASSERTE(nullptr != duration);
	if (nullptr == interval) return false;
	if (nullptr == duration) return false;

	HRESULT hres = _daily_trigger->get_Repetition(&_repetition);
	if (!SUCCEEDED(hres))
	{
		log_err "_dailytrigger->get_Repetition() fialed. hres=%u", hres log_end;
		return false;
	}
	
	hres = _repetition->put_Duration(_bstr_t(duration));
	if (!SUCCEEDED(hres))
	{
		log_err "_dailytrigger->put_Duration() fialed. hres=%u", hres log_end;
		return false;
	}

	hres = _repetition->put_Interval(_bstr_t(interval));
	if (!SUCCEEDED(hres))
	{
		log_err "_dailytrigger->put_Interval() fialed. hres=%u", hres log_end;
		return false;
	}

	return true;
}

///	@brief	�۾������ٷ��� Ʈ���Ű� �۵��Ҷ� ���� �� ������ ��θ� �����Ѵ�.
bool
SchedClient::set_execute(_In_ const wchar_t* execute_path,
						 _In_ const wchar_t* arguments)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != execute_path);
	if (nullptr == execute_path) return false;

	HRESULT hres = _definition->get_Actions(&_action_collection);
	if (!SUCCEEDED(hres))
	{
		log_err "_definition->get_Actions() failed. hres=%u", hres log_end;
		return false;
	}

	hres = _action_collection->Create(TASK_ACTION_EXEC, &_action);
	if (!SUCCEEDED(hres))
	{
		log_err "_actioncollection->Create() failed. hres=%u", hres log_end;
		return false;
	}

	hres = _action->QueryInterface(IID_IExecAction, (PVOID*)&_exec_action);
	if (!SUCCEEDED(hres))
	{
		log_err "_action->QueryInterface() failed. hres=%u", hres log_end;
		return false;
	}

	hres = _exec_action->put_Path(_bstr_t(execute_path));
	if (!SUCCEEDED(hres))
	{
		log_err "_exec_action->put_Path() failed. hres=%u", hres log_end;
		return false;
	}

	if ( nullptr != arguments)
	{
		hres = _exec_action->put_Arguments(_bstr_t(arguments));
		if (!SUCCEEDED(hres))
		{
			log_err "_exec_action->put_Arguments() failed. hres=%u", hres log_end;
			return false;
		}
	}
	
	return true;
}

///	@brief
bool
SchedClient::save(_In_ TASK_CREATION flag)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;
	
	IRegisteredTask* _registered_task = nullptr;

	HRESULT hres = _folder->RegisterTaskDefinition(_bstr_t(_task_name.c_str()),
												   _definition,
												   flag,
												   _variant_t(L"SYSTEM"),
												   _variant_t(),
												   TASK_LOGON_INTERACTIVE_TOKEN,
												   _variant_t(L""),
												   &_registered_task);
	clear();
	if (!SUCCEEDED(hres))
	{
		log_err "_folder->RegisterTaskDefinition() failed. hres=%u", hres log_end;
		return false;
	}
	_registered_task->Release();
	_registered_task = nullptr;

	return true;
}

///	@brief
bool
SchedClient::remove(_In_ const wchar_t* task_name)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != task_name);
	if (nullptr == task_name) return false;

	HRESULT hres = _folder->DeleteTask(_bstr_t(task_name), 0);
	if (!SUCCEEDED(hres))
	{
		log_err "_folder->DeleteTask() failed. hres=%u", hres log_end;
		return false;
	}

	return true;
}

bool
SchedClient::enabled(_In_ const wchar_t* task_name)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != task_name);
	if (nullptr == task_name) return false;

	IRegisteredTask* _registered_task = nullptr;
	HRESULT hres = _folder->GetTask(_bstr_t(task_name),
									&_registered_task);

	if (!SUCCEEDED(hres))
	{
		log_err "_folder->GetTask() failed. hres=%u", hres log_end;
		return false;
	}
	//
	// put_Enabled�� VARIANT_BOOL Ÿ���� �Ķ���ͷ� �޴µ� VARIANT_BOOL�� -1�� ��,
	// 0�� �����̴�. �׷��� task�� enabled�ϰ� �ٲܷ��� -1, task�� disabled�ϰ� �ٲܷ���
	// 0���� �Ű������� �Ѱ��ָ� �ȴ�.
	//
	_registered_task->put_Enabled(-1);

	if (!SUCCEEDED(hres))
	{
		log_err "_rigistered_task->put_Enabled() failed. hres=%u", hres log_end;
		return false;
	}
	_registered_task->Release();
	_registered_task = nullptr;

	return true;
}

bool
SchedClient::disabled(_In_ const wchar_t* task_name)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != task_name);
	if (nullptr == task_name) return false;

	IRegisteredTask* _registered_task = nullptr;
	HRESULT hres = _folder->GetTask(_bstr_t(task_name),
									&_registered_task);
	if (!SUCCEEDED(hres))
	{
		log_err "_folder->GetTask() failed. hres=%u", hres log_end;
		return false;
	}
	//
	// put_Enabled�� VARIANT_BOOL Ÿ���� �Ķ���ͷ� �޴µ� VARIANT_BOOL�� -1�� ��,
	// 0�� �����̴�. �׷��� task�� enabled�ϰ� �ٲܷ��� -1, task�� disabled�ϰ� �ٲܷ���
	// 0���� �Ű������� �Ѱ��ָ� �ȴ�.
	//
	_registered_task->put_Enabled(0);

	if (!SUCCEEDED(hres))
	{
		log_err "_rigistered_task->put_Enabled() failed. hres=%u", hres log_end;
		return false;
	}
	_registered_task->Release();
	_registered_task = nullptr;

	return true;
}

bool
SchedClient::is_task_existW(_In_ const wchar_t* task_name)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	_ASSERTE(nullptr != task_name);
	if (nullptr == task_name) return false;

	IRegisteredTask* _registered_task = nullptr;
	HRESULT hres = _folder->GetTask(_bstr_t(task_name),
									&_registered_task);
	if (!SUCCEEDED(hres))
		return false;
	else
		_registered_task->Release();
		return true;
}

void SchedClient::clear()
{
	if (nullptr != _definition)
	{
		_definition->Release();
		_definition = nullptr;
	}
	if (NULL != _registration_info)
	{
		_registration_info->Release();
		_registration_info = nullptr;
	}
	if (nullptr != _trigger_collection)
	{
		_trigger_collection->Release();
		_trigger_collection = nullptr;
	}
	if (nullptr != _trigger)
	{
		_trigger->Release();
		_trigger = nullptr;
	}
	if (nullptr != _daily_trigger)
	{
		_daily_trigger->Release();
		_daily_trigger = nullptr;
	}
	if (nullptr != _repetition)
	{
		_repetition->Release();
		_repetition = nullptr;
	}
	if (nullptr != _action_collection)
	{
		_action_collection->Release();
		_action_collection = nullptr;
	}
	if (nullptr != _exec_action)
	{
		_exec_action->Release();
		_exec_action = nullptr;
	}
	if (nullptr != _action)
	{
		_action->Release();
		_action = nullptr;
	}
}