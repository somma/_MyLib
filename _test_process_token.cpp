#include "stdafx.h"
#include "process_tree.h"


bool test_process_token()
{
	cprocess_tree proc_tree;
	_ASSERTE(true == proc_tree.build_process_tree());
	proc_tree.iterate_process([](_In_ process& proc, _In_ DWORD_PTR tag)->bool
	{
		UNREFERENCED_PARAMETER(tag);

		//
		//	user
		//
		psid_info user_sid =nullptr;
		user_sid = get_process_user(proc.pid());
		if (nullptr == user_sid)
		{
			log_err "get_process_user() failed. pid=%u, image=%ws",
				proc.pid(),
				proc.process_name()
				log_end;
		}
		else
		{
			log_info "== user, pid=%u, image=%ws, %ws\\%ws, sid=%ws, use=%u",
				proc.pid(),
				proc.process_name(),
				user_sid->_domain.c_str(),
				user_sid->_name.c_str(),
				user_sid->_sid.c_str(),
				user_sid->_sid_name_use
				log_end;

			//if (71944 == proc.pid()) __debugbreak();

			//
			//	groups
			//
			std::list<pgroup_sid_info> groups;
			if (true != get_process_group(proc.pid(), groups))
			{
				log_err "get_process_group() failed. pid=%u, image=%ws",
					proc.pid(),
					proc.process_name()
					log_end;
			}
			else
			{
				for (auto group_sid : groups)
				{
					std::wstringstream strm;
					if (true != group_sid->_sid_info->_domain.empty())
					{
						strm << group_sid->_sid_info->_domain << L"\\";
					}

					if (true != group_sid->_sid_info->_name.empty())
					{
						strm << group_sid->_sid_info->_name << L", ";
					}
					
					strm << L"sid=" << group_sid->_sid_info->_sid << L", ";
					strm << L"attribute=" << group_sid->attribute();

					log_info "    %ws", strm.str().c_str() log_end;
					delete group_sid;
				}
				groups.clear();
			}
			//
			// privilege
			//
			std::list<pprivilege_info> privileges;
			if (true != get_process_privilege(proc.pid(), privileges))
			{
				log_err "get_process_privilege() failed. pid=%u, image=%ws",
					proc.pid(),
					proc.process_name()
					log_end;
			}
			else
			{
				for (auto privilege : privileges)
				{
					std::wstringstream strm;
					strm << L"privilege=" << privilege->_name;
					if (0 != privilege->_attribute)
					{
						strm << L", attribute=" << privilege->attribute();
					}
					log_info "        %ws", strm.str().c_str() log_end;
					delete privilege;
				}
				privileges.clear();
			}
		}
		delete user_sid; user_sid = nullptr;
		return true;		
	}, 0);


	return true;

}