/**----------------------------------------------------------------------------
 * process_tree.cpp
 *-----------------------------------------------------------------------------
 * module that manage running process
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com)
 *-----------------------------------------------------------------------------
 * 2014:6:15 22:23 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "_MyLib/src/log.h"
#include "_MyLib/src/Win32Utils.h"
#include "_MyLib/src/process_tree.h"

/// @brief	Constructor
process::process()
	:
	_process_name(L""),
	_ppid(0),
	_pid(0),
	_creation_time(0),
	_is_wow64(false),
	_full_path(L""),
	_killed(false)
{
}

/// @brief	Destructor
process::process(
	_In_ const wchar_t* process_name,
	_In_ DWORD ppid,
	_In_ DWORD pid,
	_In_ DWORD session_id,
	_In_ uint64_t creation_time,
	_In_ bool is_wow64,
	_In_ std::wstring& full_path,
	_In_ bool killed)
	:
	_process_name(process_name),
	_ppid(ppid),
	_pid(pid),
	_session_id(session_id),
	_creation_time(creation_time),
	_is_wow64(is_wow64),
	_full_path(full_path),
	_killed(killed)
{
	_ASSERTE(nullptr != process_name);
	if (nullptr == process_name || 0 == wcslen(process_name))
	{
		_process_name = L"N/A";
	}
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool process::kill(_In_ DWORD exit_code, _In_ bool enable_debug_priv)
{
	_ASSERTE(true != _killed);
	if (true == _killed) return true;

	//
	//	first try without set privilege
	//
	HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, _pid);
	if (NULL == h && true == enable_debug_priv)
	{
		DWORD gle = GetLastError();
		if (true != set_privilege(SE_DEBUG_NAME, true))
		{
			log_err
				"OpenProcess() failed and no debug privilege), pid = %u, gle=%u",
				_pid,
				gle
				log_end;
			return false;
		}

		//
		//	re-try with debug privilege
		//	
		h = OpenProcess(PROCESS_TERMINATE, FALSE, _pid);
	}

	if (NULL == h)
	{
		log_err
			"OpenProcess() failed, pid = %u, gle=%u",
			_pid,
			GetLastError()
			log_end;
		return false;
	}

	if (!TerminateProcess(h, exit_code))
	{
		log_err
			"TerminateProcess() failed, pid = %u, gle=%u",
			_pid,
			GetLastError()
			log_end;
	}
	else
	{
		_killed = true;
		log_dbg "pid = %u, %ws terminated", _pid, _process_name.c_str() log_end;
	}

	_ASSERTE(NULL != h);
	CloseHandle(h); // TerminateProcess() is asynchronous, so must call CloseHandle()
	return (true == _killed) ? true : false;
}













/// @brief	Destructor
cprocess_tree::~cprocess_tree()
{
	clear_process_tree();
}

/// @brief	
void cprocess_tree::clear_process_tree()
{
	for (const auto& entry : _proc_map)
	{
		_ASSERTE(entry.second);
		if (nullptr != entry.second)
		{
			delete entry.second;
		}
	}
	_proc_map.clear();
}


/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool
cprocess_tree::build_process_tree(_In_ bool enable_debug_priv)
{
	//
	// 권한(SeDebugPrivilege)이 없는 경우 특정 프로세스(csrss, WmiPrvSE 등)를
	// 오픈 할려고 하는 경우 실패를 하기 때문에 디버그 권한 설정이 필요하다.
	// `set_debug_privilege` 값이 true인 경우 디버그 권한을 설정을 하고 아닌 경우 설정
	// 하지 않는다.
	//
	if (true == enable_debug_priv)
	{
		if (true != set_privilege(SE_DEBUG_NAME, true))
		{
			log_err "set_privilege(SE_DEBUG_NAME) failed." log_end;
			//
			// `SeDebugPrivilege`를 활성화 하지 못한 경우 실패를 반환
			// 하고 함수를 빠져 나가지 않는 이유는 `SeDebugPrivilege`
			// 권한이 없다고 해서 프로세스 정보를 수집 못하는게 아니기 문이다.
			//
		}
	}

	clear_process_tree();

	PROCESSENTRY32W proc_entry = { 0 };
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE)
	{
		log_err "CreateToolhelp32Snapshot() failed, gle=%u", GetLastError() log_end;
		return false;
	}

#pragma warning(disable: 4127)
	do
	{
		proc_entry.dwSize = sizeof(PROCESSENTRY32W);
		if (!Process32First(snap, &proc_entry))
		{
			log_err 
				"CreateToolhelp32Snapshot() failed, gle=%u", GetLastError() log_end;
			break;
		}

		//
		// system, idle 프로세스는 생성 시간을 획득 할 수 없으므로
		// 현재 시각을 프로세스 생성 시간으로 간주한다.
		//
		FILETIME now;
		GetSystemTimeAsFileTime(&now);

		do
		{
			BOOL IsWow64 = FALSE;
			FILETIME create_time(now);
			std::wstring full_path(_null_stringw);

			//
			// System Idle Process, System Process
			//
			if (0 == proc_entry.th32ProcessID || 4 == proc_entry.th32ProcessID)
			{
				// already initialized with default values.
				// 
				//full_path = _null_stringw;
				//create_time = now;
			}
			else
			{
				auto process_handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
												  FALSE,
												  proc_entry.th32ProcessID);
				if (NULL == process_handle)
				{
					// too much logs.
					//log_err 
					//	"OpenProcess() failed, pid = %u, proc = %s, gle=%u", 
					//	proc_entry.th32ProcessID, 
					//	WcsToMbsEx(proc_entry.szExeFile).c_str(),
					//	GetLastError() 
					//log_end;					
				}
				else
				{
					if (!get_process_image_full_path(process_handle, 
													 full_path))
					{
						log_err 
							"get_process_image_full_path() failed. pid=%u, process=%ws",
							proc_entry.th32ProcessID,
							proc_entry.szExeFile
							log_end;
					}

					FILETIME dummy_time;
					if (!GetProcessTimes(process_handle,
										 &create_time,
										 &dummy_time,
										 &dummy_time,
										 &dummy_time))
					{
						log_err 
							"GetProcessTimes() failed, pid=%u, process=%ws, gle=%u",
							proc_entry.th32ProcessID,
							proc_entry.szExeFile,
							GetLastError()
							log_end;
					}

#ifdef _WIN64
					//
					// Is WoW64 process?
					//					
					if (!IsWow64Process(process_handle, 
										&IsWow64))
					{
						log_err 
							"IsWow64Process() failed. pid=%u, process=%ws, gle=%u",
							proc_entry.th32ProcessID,
							proc_entry.szExeFile,
							GetLastError()
							log_end;
						// assume process is not WoW64
					}
#endif
					CloseHandle(process_handle); 
					process_handle = nullptr;
				}
			}

			add_process(proc_entry.th32ParentProcessID,
						proc_entry.th32ProcessID,
						create_time,
						IsWow64,
						proc_entry.szExeFile,
						full_path);
		} while (Process32Next(snap, &proc_entry));

	} while (false);
#pragma warning(default: 4127)

	CloseHandle(snap);
	return true;
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool
cprocess_tree::find_process(
	_In_ const wchar_t* process_name, 
	_In_ on_proc_walk callback
)
{
	_ASSERTE(NULL != process_name);
	if (NULL == process_name) return false;

	for (const auto& process : _proc_map)
	{
		if (rstrnicmp(process.second->process_name(), process_name))
		{
			// found
			if (!callback(process.second))
			{
				// stop iterating
				return false;
			}
		}
	}

	return true;
}

const process* cprocess_tree::get_process(_In_ DWORD pid)
{
	auto p = _proc_map.find(pid);
	if (_proc_map.end() == p)
		return nullptr;
	else
		return p->second;
}

const wchar_t* cprocess_tree::get_process_name(_In_ DWORD pid)
{
	const process* p = get_process(pid);
	if (nullptr != p)
	{
		return p->process_name();
	}
	else
	{
		return nullptr;
	}
}

const wchar_t* cprocess_tree::get_process_path(_In_ DWORD pid)
{
	const process* p = get_process(pid);
	if (nullptr != p)
	{
		return p->process_path();
	}
	else
	{
		return nullptr;
	}
}

uint64_t cprocess_tree::get_process_time(DWORD pid)
{
	const process* p = get_process(pid);
	if (nullptr != p)
	{
		return p->creation_time();
	}
	else
	{
		return 0;
	}
}

/// @brief	부모 프로세스 객체를 리턴한다.
const 
process* 
cprocess_tree::get_parent(
	_In_ const process* const process
)
{
	if (process->pid() == _pt_idle_proc_pid || process->pid() == _pt_system_proc_pid)
	{
		return nullptr;
	}

	auto p = _proc_map.find(process->ppid());
	if (p == _proc_map.end()) return nullptr;

	if (p->second->creation_time() <= process->creation_time())
	{
		return p->second;
	}
	else
	{
		return nullptr;
	}
}

const process* cprocess_tree::get_parent(_In_ DWORD pid) 
{
	const process* me = get_process(pid);
	if (nullptr == me) return nullptr;

	return get_parent(me);
}

/// @brief
DWORD cprocess_tree::get_parent_pid(_In_ DWORD pid)
{
	const process* p = get_parent(pid);
	if (nullptr == p) return 0xffffffff;

	return p->ppid();
}

/// @brief 
const wchar_t* cprocess_tree::get_parent_name(_In_ DWORD pid)
{
	if (0 == pid || 4 == pid) return nullptr;

	const process* p = get_parent(pid);
	if (nullptr == p) return nullptr;

	return p->process_name();
}

/// @brief	모든 process 를 iterate 한다. 
///			callback 에서 false 를 리턴하면 iterate 를 중지한다.
void
cprocess_tree::iterate_process(
	_In_ on_proc_walk callback
)
{
	_ASSERTE(NULL != callback);
	if (NULL == callback) return;

	for (const auto& process : _proc_map)
	{
		if (true != callback(process.second))
		{
			return;
		}
	}

}

/// @brief	지정된 process tree 를 iterate 한다. 
///			callback 에서 false 를 리턴(iterate 중지/취소) 하거나,
///			지정된 프로세스를 찾을 수 없거나,
///			유효하지 않는 파라미터 입력시 iterate 를 중지한다.
void
cprocess_tree::iterate_process_tree(
	_In_ DWORD root_pid,
	_In_ on_proc_walk callback
)
{
	_ASSERTE(NULL != callback);
	if (NULL == callback) return;

	process_map::const_iterator it = _proc_map.find(root_pid);
	if (it == _proc_map.end()) return;

	const pprocess root = it->second;
	return iterate_process_tree(root, callback);
}

/// @brief	process map 을 순회하면서 콜백함수를 호출한다. 
///			콜백함수가 false 를 리턴하면 순회를 즉시 멈춘다.
void
cprocess_tree::iterate_process_tree(
	_In_ process* const root,
	_In_ on_proc_walk callback
)
{
	// parent first
	if (true != callback(root)) return;

	//
	//	pid == 0 인 프로세스라면 recursive call 을 하지 않는다. 
	//	win10 에서 toolhelp 를 이용한 경우 
	// 
	//	`[System Process]` : 
	//	`System` 
	// 
	//	이렇게 두개의 프로세스 정보가 리턴되는데, [System Process] 의 경우
	//	pid, ppid, create time 등이 모두 0 이다. 
	//	[System Process] 의 자식 프로세스는 없으므로 recursive call 을 하지 않는다.
	//	
	if (0 == root->pid())
	{
		return;
	}

	// childs
	process_map::const_iterator its = _proc_map.begin();
	process_map::const_iterator ite = _proc_map.end();
	for (; its != ite; ++its)
	{
		//	ppid 의 값은 동일하지만 ppid 프로세스는 이미 종료되고, 새로운 프로세스가 생성되고, 
		//	ppid 를 할당받은 경우가 발생할 수 있다. 
		//	따라서 ppid 값이 동일한 경우 ppid 를 가진 프로세스의 생성 시간이 pid 의 생성시간 값이 
		//	더 커야 한다.
		// 
		//	수정: Jang, Hyowon (jang.hw73@gmail.com)
		//	생성시간이 동일한 경우 print되지 않는 프로세스가 존재하기 때문에(ex. creation_time == 0) 
		//	생성시간의 값이 더 크거나 같은 값으로 해야 한다.
		if (its->second->ppid() == root->pid() &&
			its->second->creation_time() >= root->creation_time())
		{
			iterate_process_tree(its->second, callback);
		}
	}
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
void cprocess_tree::print_process_tree(_In_ DWORD root_pid)
{
	process_map::iterator it = _proc_map.find(root_pid);
	if (it != _proc_map.end())
	{
		DWORD depth = 0;
		print_process_tree(it->second, depth);
		_ASSERTE(0 == depth);
	}
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
void cprocess_tree::print_process_tree(_In_ const wchar_t* root_process_name)
{
	_ASSERTE(NULL != root_process_name);
	if (NULL == root_process_name) { return; }

	find_process(root_process_name, 
				 [&](_In_ const process* const process_info)->bool 
	{
		DWORD depth = 0;
		print_process_tree(process_info, depth);
		return true;
	});
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
void
cprocess_tree::print_process_tree(
	_In_ const process* const p,
	_In_ DWORD& depth
)
{
	std::stringstream prefix;
	for (DWORD i = 0; i < depth; ++i)
	{
		prefix << "+";
	}

	log_info
		"%spid = %u (ppid = %u), %ws ",
		prefix.str().c_str(),
		p->pid(),
		p->ppid(),
		p->process_name()
		log_end;

	// p._pid 를 ppid 로 갖는 item 을 찾자
	process_map::const_iterator it = _proc_map.begin();
	process_map::const_iterator ite = _proc_map.end();
	for (; it != ite; ++it)
	{
		// ppid 의 값은 동일하지만 ppid 프로세스는 이미 종료되고, 새로운 프로세스가 생성되고, ppid 를 할당받은 경우가 
		// 발생할 수 있다. 따라서 ppid 값이 동일한 경우 ppid 를 가진 프로세스의 생성 시간이 pid 의 생성시간 값이 더 커야 한다.
		// 수정: Jang, Hyowon (jang.hw73@gmail.com)
		// 생성시간이 동일한 경우 print되지 않는 프로세스가 존재하기 때문에(ex. creation_time == 0) 생성시간의 값이 더 크거나 같은 값으로 해야 한다.
		if (it->second->ppid() == p->pid() &&
			(uint64_t)it->second->creation_time() >= (uint64_t)p->creation_time())
		{
			print_process_tree(it->second, ++depth);
			--depth;
		}
	}
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool cprocess_tree::kill_process(_In_ DWORD pid, _In_ bool enable_debug_priv)
{
	if (pid == 0 || pid == 4) return false;

	process_map::iterator it = _proc_map.find(pid);
	if (it == _proc_map.end()) return true;
	process* const prcs = it->second;
	return prcs->kill(0, enable_debug_priv);
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool 
cprocess_tree::kill_process(
	_In_ const wchar_t* process_name, 
	_In_ bool enable_debug_priv
)
{
	_ASSERTE(NULL != process_name);
	if (NULL == process_name) return false;

	return find_process(process_name, 
						[&](_In_ const process* const process_info)->bool 
	{
		const_cast<process*>(process_info)->kill(0, enable_debug_priv);
		return true;
	});
}

/**
 * @brief	root_pid 와 그 자식 프로세스를 모두 종료한다.
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool
cprocess_tree::kill_process_tree(
	_In_ DWORD root_pid,
	_In_ bool enable_debug_priv
)
{
	if (root_pid == 0 || root_pid == 4) return false;

	process_map::iterator it = _proc_map.find(root_pid);
	if (it == _proc_map.end()) return true;
	process* const root = it->second;

	// check process is already killed.
	if (true == root->killed())
	{
		log_info
			"already killed. pid = %u, %ws",
			root->pid(),
			root->process_name()
			log_end;
		return true;
	}

	// kill process tree include root.
	kill_process_tree(root, enable_debug_priv);
	return true;
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
void
cprocess_tree::add_process(
	_In_ DWORD ppid,
	_In_ DWORD pid,	
	_In_ FILETIME& creation_time,
	_In_ BOOL is_wow64,
	_In_ const wchar_t* process_name,
	_In_ std::wstring& full_path
)
{
	DWORD session_id = 0xffffffff;
	ProcessIdToSessionId(pid, &session_id);

	auto ret = _proc_map.insert(std::make_pair(pid, nullptr));
	if (!ret.second)
	{
		log_warn
			"process (pid=%u, %ws) exists and will be replaced with pid=%u, image=%ws",
			ret.first->second->pid(),
			ret.first->second->process_name(),
			pid,
			process_name
			log_end;

		_ASSERTE(nullptr != ret.first->second);
		if (nullptr != ret.first->second)
		{
			delete ret.first->second;
		}
	}
	else
	{
		ret.first->second = new process(process_name,
										ppid,
										pid,
										session_id,
										*(uint64_t*)&creation_time,
										(is_wow64 ? true : false),
										full_path,
										false);
	}
}

/**
 * @brief	자식의 자식의 자식까지... 재귀적으로 죽이고, 나도 죽는다. :-)

			중간에 권한 문제라든지 뭔가 문제가 있는 프로세스가 있을 수도 있다.
			못죽이는 놈은 냅두고, 죽일 수 있는 놈은 다 죽이려고, 리턴값을 void 로 했음
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
void
cprocess_tree::kill_process_tree(
	_In_ process* const root,
	_In_ bool enable_debug_priv
)
{
	// terminate child processes first if exists.
	process_map::const_iterator its = _proc_map.begin();
	process_map::const_iterator ite = _proc_map.end();
	for (; its != ite; ++its)
	{
		// ppid 의 값은 동일하지만 ppid 프로세스는 이미 종료되고, 새로운 프로세스가 생성되고, ppid 를 할당받은 경우가 
		// 발생할 수 있다. 따라서 ppid 값이 동일한 경우 ppid 를 가진 프로세스의 생성 시간이 pid 의 생성시간 값이 더 커야 한다.
		// 수정: Jang, Hyowon (jang.hw73@gmail.com)
		// 생성시간이 동일한 경우 print되지 않는 프로세스가 존재하기 때문에(ex. creation_time == 0) 생성시간의 값이 더 크거나 같은 값으로 해야 한다.
		if (its->second->ppid() == root->pid() &&
			its->second->creation_time() >= root->creation_time())
		{
			kill_process_tree(its->second, enable_debug_priv);
		}
	}

	// terminate parent process
	root->kill(0, enable_debug_priv);
}

