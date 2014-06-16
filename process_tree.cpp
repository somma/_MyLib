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
#include "process_tree.h"


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
cprocess_tree::build_process_tree()
{
	_proc_map.clear();

	bool ret = false;

	PROCESSENTRY32W proc_entry = {0};
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE)
	{
		log_err L"CreateToolhelp32Snapshot() failed, gle = %u", GetLastError() log_end
		return false;
	}


	set_privilege(SE_DEBUG_NAME, TRUE);


	do
	{
		proc_entry.dwSize = sizeof(PROCESSENTRY32W);
		if (!Process32First(snap, &proc_entry))
		{
			log_err L"CreateToolhelp32Snapshot() failed, gle = %u", GetLastError() log_end
			break;
		}

		do
		{
			FILETIME create_time={0};
			HANDLE process_handle = OpenProcess(
										PROCESS_QUERY_INFORMATION, 
										FALSE, 
										proc_entry.th32ProcessID
										);
			if(NULL == process_handle)
			{
				log_err 
					L"OpenProcess() failed, pid = %u, proc = %s, gle = %u", 
					proc_entry.th32ProcessID, 
					proc_entry.szExeFile, 
					GetLastError() 
				log_end
				// use create time 0!
			}
			else
			{
				FILETIME dummy_time;
				if (!GetProcessTimes(process_handle, &create_time, &dummy_time, &dummy_time, &dummy_time))
				{
					log_err L"GetProcessTimes() failed, gle = %u", GetLastError() log_end
					// use create time 0!
				}

				CloseHandle(process_handle); process_handle = NULL;
			}			
						
			add_process(proc_entry.th32ParentProcessID, proc_entry.th32ProcessID, create_time, proc_entry.szExeFile);

		} while (Process32Next(snap, &proc_entry));

	} while (false);


	CloseHandle(snap);
	set_privilege(SE_DEBUG_NAME, FALSE);

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
DWORD cprocess_tree::find_process(_In_ const wchar_t* process_name)
{
	_ASSERTE(NULL != process_name);
	if (NULL == process_name) return false;

	process_map::iterator it = _proc_map.begin();
	process_map::iterator ite = _proc_map.end();
	for(; it != ite; ++it)
	{
		if (0 == it->second.process_name().compare(process_name)) 
		{
			// found
			return it->second.pid();
		}
	}

	return 0;
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
bool cprocess_tree::kill_process_tree(_In_ DWORD root_pid)
{
	if (root_pid == 0 || root_pid == 4) return false;

	process_map::iterator it = _proc_map.find(root_pid);
	if (it == _proc_map.end()) return true;
	process root = it->second;
	
	// terminate child processes
	process_map::iterator its = _proc_map.begin();
	process_map::iterator ite= _proc_map.end();
	for(; its != ite; ++its)
	{
		// ppid 의 값은 동일하지만 ppid 프로세스는 이미 종료되고, 새로운 프로세스가 생성되고, ppid 를 할당받은 경우가 
		// 발생할 수 있다. 따라서 ppid 값이 동일한 경우 ppid 를 가진 프로세스의 생성 시간이 pid 의 생성시간 값이 더 커야 한다. 
		if ( its->second.ppid() == root.pid() && 
			 its->second.creation_time() > root.creation_time())
		{
			kill_process(its->second.pid(), 0);
		}
	}

	// terminate root process
	kill_process(root_pid, 0);
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

	DWORD pid = find_process(root_process_name);
	if (0 != pid)
	{
		print_process_tree(pid);
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
void 
cprocess_tree::add_process(
	_In_ DWORD ppid, 
	_In_ DWORD pid, 
	_In_ FILETIME& creation_time, 
	_In_ const wchar_t* process_name
	)
{
	process p(process_name, ppid, pid, *(uint64_t*)&creation_time);
	_proc_map.insert( std::make_pair(pid, p) );
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
void cprocess_tree::print_process_tree(_In_ process& p, _In_ DWORD& depth)
{
	std::wstringstream prefix;
	for(DWORD i = 0; i < depth; ++i)
	{
		prefix << L"    ";
	}

	log_info 
		L"%spid = %u (ppid = %u), %s ", prefix.str().c_str(), 
		p.pid(), 
		p.ppid(), 
		p.process_name().c_str() 
	log_end

	// p._pid 를 ppid 로 갖는 item 을 찾자
	process_map::iterator it = _proc_map.begin();
	process_map::iterator ite= _proc_map.end();
	for(; it != ite; ++it)
	{
		// ppid 의 값은 동일하지만 ppid 프로세스는 이미 종료되고, 새로운 프로세스가 생성되고, ppid 를 할당받은 경우가 
		// 발생할 수 있다. 따라서 ppid 값이 동일한 경우 ppid 를 가진 프로세스의 생성 시간이 pid 의 생성시간 값이 더 커야 한다. 
		if ( it->second.ppid() == p.pid() && 
			(uint64_t)it->second.creation_time() > (uint64_t)p.creation_time())
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
void cprocess_tree::kill_process(_In_ DWORD pid, _In_ DWORD exit_code)
{
	set_privilege(SE_DEBUG_NAME, TRUE);
	
	HANDLE h = NULL;
	do 
	{
		h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
		if (NULL == h)
		{
			log_err 
				L"OpenProcess() failed, pid = %u, gle = %u", 
				pid,
				GetLastError()
			log_end
			break;
		}
	
		if (!TerminateProcess(h, exit_code))
		{
			log_err 
				L"TerminateProcess() failed, pid = %u, gle = %u", 
				pid,
				GetLastError()
			log_end
			break;			
		}
	
		log_dbg L"pid = %u, terminated", pid log_end
	} while (false);
	
	if (NULL!=h) 
	{
		CloseHandle(h); // TerminateProcess() is asynchronous, so must call CloseHandle()
	}

	set_privilege(SE_DEBUG_NAME, FALSE);                     
}
