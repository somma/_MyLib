#include "stdafx.h"
#include "process_tree.h"
#include <sddl.h>


bool test_process_token()
{
	cprocess_tree proc_tree;
	_ASSERTE(true == proc_tree.build_process_tree());
	uint32_t pid = proc_tree.find_process(L"explorer.exe");
	_ASSERTE(0 != pid);

	//
	//	Open process handle with READ token access
	//
	handle_ptr proc_handle(
		OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid),
		[](_In_ HANDLE handle) 
	{
		if (NULL == handle){CloseHandle(handle);}
	});
	if (NULL == proc_handle.get())
	{
		log_err "OpenProcess() failed. pid=%u, gle=%u",
			pid, 
			GetLastError()
			log_end;
		return false;
	}

	//
	//	Open token handle
	//
	HANDLE th;
	if (TRUE != OpenProcessToken(proc_handle.get(),
								 TOKEN_QUERY,
								 &th))
	{
		log_err "OpenProcessToken() failed. pid=%u, gle=%u",
			pid, 
			GetLastError()
			log_end;
		return false;
	}
	handle_ptr token_handle(th, [](_In_ HANDLE th) {CloseHandle(th); });

	//
	//	Get token information
	//
	DWORD return_length;
	GetTokenInformation(token_handle.get(),
						TokenUser,
						nullptr,
						0,
						&return_length);
	DWORD gle = GetLastError();
	if (gle != ERROR_INSUFFICIENT_BUFFER)
	{
		log_err "GetTokenInformation() failed. pid=%u, gle=%u",
			pid,
			gle
			log_end;
		return false;
	}

	char_ptr ptr(
		(char*)malloc(return_length),
		[](_In_ char* ptr) 
	{
		if (nullptr != ptr) free(ptr); 
	});

	if (nullptr == ptr.get())
	{
		log_err "Not enough memory. malloc size=%u", 
			return_length
			log_end;
		return false;
	}
	
	if (TRUE != GetTokenInformation(token_handle.get(),
									TokenUser,
									(PTOKEN_USER)ptr.get(),
									return_length, 
									&return_length))
	{
		log_err "GetTokenInformation() failed. pid=%u, gle=%u",
			pid, 
			GetLastError()
			log_end;
		return false;
	}

	PTOKEN_USER token_user = (PTOKEN_USER)ptr.get();

	//
	//	SID  
	// 
	wchar_t* sid_string = nullptr;
	if (TRUE != ConvertSidToStringSidW(token_user->User.Sid, &sid_string))
	{
		gle = GetLastError();
		const char* gles = nullptr;
		switch (gle)
		{
		case ERROR_NOT_ENOUGH_MEMORY: gles = "ERROR_NOT_ENOUGH_MEMORY"; break;
		case ERROR_INVALID_SID: gles = "ERROR_INVALID_SID"; break;
		case ERROR_INVALID_PARAMETER: gles = "ERROR_INVALID_PARAMETER"; break;
		}

		if (nullptr != gles)
		{
			log_err "ConvertSidToStringSidW() failed. pid=%, gle=%s",
				pid,
				gles
				log_end;
		}
		else
		{
			log_err "ConvertSidToStringSidW() failed. pid=%, gle=%u",
				pid,
				gle
				log_end;
		}
		return false;
	}
	_ASSERTE(nullptr != sid_string);
	wchar_ptr ss(
		sid_string, 
		[](_In_ wchar_t* ptr)
	{ 
		//
		//	sid_string 버퍼는 반드시 LocalFree() 로 
		//	소멸한다. 
		//
		LocalFree(ptr); 
	});
	
	log_info "pid=%u, sid=%ws",
		pid,
		sid_string
		log_end;

	//
	//	 User name
	//
	SID_NAME_USE sid_name_use = SidTypeUser;
	DWORD cch_name = 0;
	DWORD cch_domain = 0;
	LookupAccountSid(nullptr,
					 token_user->User.Sid,
					 nullptr,
					 &cch_name,
					 nullptr,
					 &cch_domain,
					 &sid_name_use);
	
	wchar_ptr name((wchar_t*)malloc((cch_name + 1) * sizeof(wchar_t)),
				   [](_In_ wchar_t* ptr) 
	{
		if (nullptr != ptr)
		{
			free(ptr);
		}
	});
	wchar_ptr domain((wchar_t*)malloc((cch_domain + 1) * sizeof(wchar_t)),
					 [](_In_ wchar_t* ptr) 
	{
		if (nullptr != ptr)
		{
			free(ptr);
		}
	});
	if (nullptr == name.get() || nullptr == domain.get())
	{
		log_err "Not enough memory. pid=%u",
			pid
			log_end;
		return false;
	}

	if (TRUE != LookupAccountSid(nullptr,
								 token_user->User.Sid,
								 name.get(),
								 &cch_name,
								 domain.get(),
								 &cch_domain,
								 &sid_name_use))
	{
		log_err "LookupAccountSid() failed. pid=%u",
			pid
			log_end;
		return false;
	}

	log_info "pid=%u, domain=%ws, user=%ws, use=%u",
		pid,
		domain.get(),
		name.get(),
		sid_name_use
		log_end;

	return true;
}