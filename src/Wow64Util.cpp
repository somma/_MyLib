/**
 * @file    WOW64 util
 * @brief
 *
 * @author  Yonghwan, Roh (somma@somma.kr)
 * @date    07.14.2022 21:59 created
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"
#include <wow64apiset.h>
#include "Wow64Util.h"

/// @brief	
bool is_wow64_process(HANDLE hProcess)
{
	BOOL wow = FALSE;
	if (NULL == hProcess) hProcess = GetCurrentProcess();
	if (!IsWow64Process(GetCurrentProcess(), &wow))
	{
		// 함수 호출 실패시 Wow 프로세스가 아닌걸로 
		return false;
	}
	else
	{
		return wow ? true : false;
	}
}

/// @brief	
bool is_wow64_process(DWORD pid)
{
	HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (NULL == proc)
	{
		return false;
	}

	return is_wow64_process(proc);
}

/// @brief	
bool is_wow64_message()
{
	return IsWow64Message();
}


/// @brief	현재 스레드의 file system redirection 기능을 끈다. 
///			다시 켤때는 revert_handle 을 통해 revert_wow64_fs_redirection()
//			함수를 사용해야 한다. 
bool disable_wow64_fs_redirection(_Out_ void**& revert_handle)
{
	return Wow64DisableWow64FsRedirection(revert_handle) ? true : false;
}

/// @brief	현재 스레드의 file system redirection 기능을 다시 켠다.
///			revert_handle 을 통해 disable_wow64_fs_redirection() 함수가
///			리턴한 값을 사용해야 한다.
bool revert_wow64_fs_redirection(_In_ void* revert_handle)
{
	return Wow64RevertWow64FsRedirection(revert_handle) ? true : false;
}


