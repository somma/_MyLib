#include "stdafx.h"
#include "process_tree.h"


bool test_process_token()
{
	cprocess_tree proc_tree;
	_ASSERTE(true == proc_tree.build_process_tree());
	proc_tree.iterate_process([](_In_ process& proc, _In_ DWORD_PTR tag)->bool
	{
		UNREFERENCED_PARAMETER(tag);
		
		std::wstring sid;
		std::wstring domain;
		std::wstring user_name;
		SID_NAME_USE sid_name_use;
		if (true != get_process_sid_and_user(proc.pid(),
											 sid,
											 domain,
											 user_name,
											 sid_name_use))
		{
			log_err "get_process_sid_and_user() failed. pid=%u, image=%ws",
				proc.pid(),
				proc.process_name()
				log_end;
		}
		else
		{
			log_info "pid=%u, image=%ws, %ws\\%ws, sid=%ws, use=%u",
				proc.pid(),
				proc.process_name(),
				domain.c_str(),
				user_name.c_str(),
				sid.c_str(),
				sid_name_use
				log_end;
		}
		return true;		
	}, 0);

	return true;

}