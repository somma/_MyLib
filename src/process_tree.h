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

#include <sstream>
#include <string>
#include <map>

#include "Win32Utils.h"

#include <TlHelp32.h>

/**
 * @brief	class for running process
**/
class process
{
public:
	process()
	:_process_name(L""), _ppid(0), _pid(0), _creation_time(0), _full_path(L""), _killed(false)
	{

	}

	process(_In_ const wchar_t* process_name, _In_ DWORD ppid, _In_ DWORD pid, _In_ uint64_t creation_time, _In_ std::wstring& full_path, _In_ bool killed)
	: _process_name(process_name), _ppid(ppid), _pid(pid), _creation_time(creation_time), _full_path(full_path), _killed(killed)
	{
	}

	bool kill(_In_ DWORD exit_code);
	bool suspend() { /* not implemented yet */ return true; }
	bool resume()  { /* not implemented yet */ return true; }

	const wchar_t*	process_name()	{ return _process_name.c_str(); }
    const wchar_t*  process_path()  { return _full_path.c_str(); }
	DWORD			ppid()			{ return _ppid; }
	DWORD			pid()			{ return _pid; }
	uint64_t		creation_time() { return _creation_time; }
	bool			killed()		{ return _killed; }

private:
	std::wstring	_process_name;
	DWORD			_ppid;
	DWORD			_pid;
	uint64_t		_creation_time;
    std::wstring    _full_path;
	bool			_killed;
};




/**
 * @brief	place holder for running processes
**/
typedef std::map< DWORD, process >	process_map;
typedef bool (*fnproc_tree_callback)(_In_ process& process_info, _In_ DWORD_PTR callback_tag);


class cprocess_tree
{
public:
	bool	clear_process_tree() { _proc_map.clear(); }
	bool	build_process_tree();

	DWORD			find_process(_In_ const wchar_t* process_name);
	const wchar_t*	get_process_name(_In_ DWORD pid);
    const wchar_t*  get_process_path(_In_ DWORD pid);
    uint64_t        get_process_time(_In_ DWORD pid);

	DWORD			get_parent_pid(_In_ DWORD child_pid);
	const wchar_t*	get_parent_name(_In_ DWORD child_pid);

	void	iterate_process(_In_ fnproc_tree_callback callback, _In_ DWORD_PTR callback_tag);
	void 	iterate_process_tree(_In_ DWORD root_pid, _In_ fnproc_tree_callback callback, _In_ DWORD_PTR callback_tag);

	void	print_process_tree(_In_ DWORD root_pid);
	void	print_process_tree(_In_ const wchar_t* root_process_name);

	bool	kill_process(_In_ DWORD pid);
	bool	kill_process(_In_ const wchar_t* process_name);

	bool	kill_process_tree(_In_ DWORD root_pid);

private:
	void	add_process(_In_ DWORD ppid, _In_ DWORD pid, _In_ FILETIME& creation_time, _In_ const wchar_t* process_name, _In_ std::wstring& full_path);
	void	print_process_tree(_In_ process& p, _In_ DWORD& depth);
	void	kill_process_tree(_In_ process& root);
	void	iterate_process_tree(_In_ process& root, _In_ fnproc_tree_callback callback, _In_ DWORD_PTR callback_tag);

private:
	process_map _proc_map;
};





