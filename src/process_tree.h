/**----------------------------------------------------------------------------
 * process_tree.h
 *-----------------------------------------------------------------------------
 * module that manage running process
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com)
 *-----------------------------------------------------------------------------
 * 2014:6:16 8:48 created
 *
 * + lsass.exe 프로세스 컨텍스트에서 build() 하면 멈추는 현상 확인 (x64 win7 sp1)
**---------------------------------------------------------------------------*/
#pragma once
#include <sstream>
#include <string>
#include <map>

#include "Win32Utils.h"

#include <TlHelp32.h>

#define _pt_system_proc_   L"System"
#define _pt_system_proc_pid 4
#define _pt_explorer_proc_ L"explorer.exe"

#define _pt_registry_proc_ L"Registry"
#define _pt_memcomp_proc_ L"MemCompression"

#define _pt_idle_proc_		L"System Idle Process"
#define _pt_idle_proc_pid	0

#define _pt_winlogon_proc	L"winlogon.exe"

/// @brief	class for running process
typedef class process
{
public:
	process();
	process(_In_ const wchar_t* process_name,
			_In_ DWORD ppid,
			_In_ DWORD pid,
			_In_ DWORD session_id,
			_In_ uint64_t creation_time,
			_In_ bool is_wow64,
			_In_ std::wstring& full_path,
			_In_ bool killed);
	virtual ~process() {}

	bool kill(_In_ DWORD exit_code, _In_ bool enable_debug_priv);
	bool suspend() { /* not implemented yet */ return true; }
	bool resume()  { /* not implemented yet */ return true; }

	const wchar_t*	process_name() const 
	{ 
		return _process_name.empty() ? L"N/A" : _process_name.c_str();
	}

    const wchar_t*  process_path() const 
	{ 
		return _full_path.empty() ? process_name() : _full_path.c_str();
	}

	DWORD			ppid() const { return _ppid; }
	DWORD			pid() const { return _pid; }
	DWORD			session_id() const { return _session_id; }
	uint64_t		creation_time() const { return _creation_time; }	
	bool			is_wow64() const { return _is_wow64; }
	bool			killed() const { return _killed; }

private:
	std::wstring	_process_name;
	DWORD			_ppid;
	DWORD			_pid;
	DWORD			_session_id;
	uint64_t		_creation_time;
	bool			_is_wow64;
    std::wstring    _full_path;
	bool			_killed;
} *pprocess;


///	@brief	place holder for running processes
using process_map = std::map<DWORD, pprocess>;
using on_proc_walk = boost::function<bool(_In_ const process* const process_info)>;

class cprocess_tree
{
public:
	virtual ~cprocess_tree();

	size_t size() const { return _proc_map.size(); }
	void clear_process_tree();
	bool build_process_tree(_In_ bool enable_debug_priv);

	bool find_process(_In_ const wchar_t* process_name, _In_ on_proc_walk callback);

	const process* get_process(_In_ DWORD pid);
	const wchar_t* get_process_name(_In_ DWORD pid);
	const wchar_t* get_process_path(_In_ DWORD pid);
	uint64_t get_process_time(_In_ DWORD pid);

	const process* get_parent(_In_ const process* const process);
	const process* get_parent(_In_ DWORD pid);
	DWORD get_parent_pid(_In_ DWORD pid);
	const wchar_t* get_parent_name(_In_ DWORD pid);

	void iterate_process(_In_ on_proc_walk callback);
	void iterate_process_tree(_In_ DWORD root_pid, _In_ on_proc_walk callback);
	void iterate_process_tree(_In_ const process* const root, _In_ on_proc_walk callback);

	void print_process_tree(_In_ DWORD root_pid);
	void print_process_tree(_In_ const wchar_t* root_process_name);

	bool kill_process(_In_ DWORD pid, _In_ bool enable_debug_priv);
	bool kill_process(_In_ const wchar_t* process_name, _In_ bool enable_debug_priv);

	bool kill_process_tree(_In_ DWORD root_pid, _In_ bool enable_debug_priv);
private:
	void add_process(_In_ DWORD ppid, _In_ DWORD pid, _In_ FILETIME& creation_time, _In_ BOOL is_wow64, _In_ const wchar_t* process_name, _In_ std::wstring& full_path);
	void print_process_tree(_In_ const process* const p, _In_ DWORD& depth);
	void kill_process_tree(_In_ process* const root, _In_ bool enable_debug_priv);
private:
	process_map _proc_map;
};





