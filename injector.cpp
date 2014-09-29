/**----------------------------------------------------------------------------
 * injector.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 26:8:2011   15:31 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "injector.h"
#include "Win32Utils.h"

bool 
my_create_remote_thread(
	_In_ HANDLE process_handle, 
	_In_ LPTHREAD_START_ROUTINE thread_start_routine, 
	_In_ LPVOID thread_param
	);

typedef DWORD (WINAPI *pNtCreateThreadEx)
( 
	OUT PHANDLE hThread,
	IN ACCESS_MASK DesiredAccess,
	IN LPVOID ObjectAttributes,
	IN HANDLE ProcessHandle,
	IN LPTHREAD_START_ROUTINE lpStartAddress,
	IN LPVOID lpParameter,
	IN BOOL CreateSuspended, 
	IN ULONG StackZeroBits,
	IN ULONG SizeOfStackCommit,
	IN ULONG SizeOfStackReserve,
	OUT LPVOID lpBytesBuffer
); 


/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
bool inject_thread(_In_ DWORD pid, _In_z_ const char* dll_path)
{
	// 타겟 프로세스 오픈	
	HANDLE process_handle = privileged_open_process(pid, PROCESS_ALL_ACCESS, true);
	if (NULL==process_handle)
	{
		log_err "privileged_open_process(pid=%u) failed", pid log_end
		return false;
	}


	// 대상 프로세스 메모리에 dll 경로명 만큼의 버퍼 할당 (파라미터 영역)
	unsigned int buffer_size = (unsigned int)strlen(dll_path) + sizeof(char);
	char* buffer = (char*) VirtualAllocEx(
								process_handle, 
								NULL, 
								buffer_size, 
								MEM_COMMIT, 
								PAGE_READWRITE
								);	
	if (NULL == buffer)
	{
		CloseHandle(process_handle);
		return false;
	}

	// dll 경로명 (문자열) 복사
	SIZE_T cbWritten=0;
	if (TRUE != WriteProcessMemory(
								process_handle, 
								buffer, 
								dll_path, 
								(DWORD)strlen(dll_path), 
								&cbWritten))
	{
		log_err "WriteProcessMemory() failed. gle=0x%08x", GetLastError() log_end

		VirtualFreeEx(process_handle, buffer, buffer_size, MEM_DECOMMIT);
		CloseHandle(process_handle);
		return false;
	}

	// LoadLibraryA 주소 구하기
	HMODULE kernel32_handle = GetModuleHandleW(L"kernel32.dll");
	LPTHREAD_START_ROUTINE load_library_ptr = 
		(LPTHREAD_START_ROUTINE) GetProcAddress(kernel32_handle, "LoadLibraryW");

	// create remote-thread 
	bool ret = my_create_remote_thread(
								process_handle, 
								load_library_ptr, 
								buffer
								);
	
	VirtualFreeEx(process_handle, buffer, buffer_size, MEM_DECOMMIT);
	CloseHandle(process_handle);
	
	
	if (true != ret)
	{
		log_err "my_create_remote_thread( pid = %u ) failed.", pid log_end		
		return false;
	}
	else
	{
		return true;
	}
}


/**
 * @brief	
 * @param	
 * @see		
 * @remarks		이승원 책임 코드 (05_64비트_&_Windows_Kernel_6\43_DLL_Injection_in_Kernel_6\src\InjectDll_new\InjectDll_new.cpp)
				는 debug 모드에서는 실패, release 모드에서는 성공함
				이유를 모르겠네... 일단 바쁘니 NtCreateThreadEx 사용하는건 나중에 하자.
 * @code		
 * @endcode	
 * @return	
**/
bool 
my_create_remote_thread(
	_In_ HANDLE process_handle, 
	_In_ LPTHREAD_START_ROUTINE thread_start_routine, 
	_In_ LPVOID thread_param
	)
{
	HANDLE thread_handle = NULL;

	/*
	if (true == is_vista_later(OsVersion()))
	{
		pNtCreateThreadEx func = (pNtCreateThreadEx)GetProcAddress(
														GetModuleHandleW(L"ntdll.dll"), 
														"NtCreateThreadEx");
		if(NULL == func)
		{
			log_err "GetProcAddress( NtCreateThreadEx ) failed. gle = %u", GetLastError() log_end
			return false;
		}
		
		log_info "func = 0x%16x, tsr = 0x%16x, param = 0x%16x", func, thread_start_routine, thread_param log_end
		
		DWORD ret = func(
						&thread_handle, 
						THREAD_ALL_ACCESS,
                        NULL,
                        process_handle,
                        thread_start_routine,
                        thread_param,
                        FALSE,
                        NULL,
                        NULL,
                        NULL,
                        NULL
						);
		if (NULL == thread_handle)
		{
			log_err "NtCreateThreadEx() failed. ret = %u", ret log_end
			return false;
		}
	}
	else
	{
    */
		thread_handle = CreateRemoteThread(process_handle, NULL, 0, thread_start_routine, thread_param, 0, NULL);
        if( thread_handle == NULL )
        {
            log_err "CreateRemoteThread() failed. gle = %u", GetLastError() log_end
            return false;
        }
	/*
	}
	*/
	log_info "new remote thread created, wait for thread load dll." log_end
	WaitForSingleObject(thread_handle, INFINITE);		// dll loading 이 완료될 때 까지 대기
	return true;
}

