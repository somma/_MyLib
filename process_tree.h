/**----------------------------------------------------------------------------
 * process_tree.h
 *-----------------------------------------------------------------------------
 * module that manage running process
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com)
 *-----------------------------------------------------------------------------
 * 2014:6:16 8:48 created
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
	:_process_name(L""), _ppid(0), _pid(0), _creation_time(0)
	{
		
	}

	process(_In_ const wchar_t* process_name, _In_ DWORD ppid, _In_ DWORD pid, _In_ uint64_t creation_time)
	: _process_name(process_name), _ppid(ppid), _pid(pid), _creation_time(creation_time)
	{
	}

	bool build_process_tree();

	std::wstring&	process_name()	{ return _process_name; }
	DWORD			ppid()			{ return _ppid; }
	DWORD			pid()			{ return _pid; }
	uint64_t		creation_time() { return _creation_time; }

private:
	std::wstring	_process_name;
	DWORD			_ppid;
	DWORD			_pid;
	uint64_t		_creation_time;
};




/**
 * @brief	place holder for running processes
**/
typedef std::map< DWORD, process >	process_map;

class cprocess_tree
{
public:
	bool  build_process_tree();
	DWORD find_process(_In_ const wchar_t* process_name);

	bool  kill_process_tree(_In_ DWORD root_pid);
	
	void  print_process_tree(_In_ DWORD root_pid);
	void  print_process_tree(_In_ const wchar_t* root_process_name);


private:
	void add_process(_In_ DWORD ppid, _In_ DWORD pid, _In_ FILETIME& creation_time, _In_ const wchar_t* process_name);
	void print_process_tree(_In_ process& p, _In_ DWORD& depth);
	void kill_process(_In_ DWORD pid, _In_ DWORD exit_code);

private:
	process_map _proc_map;
};

