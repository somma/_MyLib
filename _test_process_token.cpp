#include "stdafx.h"
#include "_MyLib/src/process_tree.h"


bool test_process_token()
{
	cprocess_tree proc_tree;
	_ASSERTE(true == proc_tree.build_process_tree(true));
	proc_tree.iterate_process([](_In_ const process* const proc)->bool
	{
		//
		//	user
		//
		psid_info user_sid =nullptr;
		user_sid = get_process_user(proc->pid());
		if (nullptr == user_sid)
		{
			log_err "get_process_user() failed. pid=%u, image=%ws",
				proc->pid(),
				proc->process_name()
				log_end;
		}
		else
		{
			log_info "== user, pid=%u, image=%ws, %ws\\%ws, sid=%ws, use=%u",
				proc->pid(),
				proc->process_name(),
				user_sid->_domain.c_str(),
				user_sid->_name.c_str(),
				user_sid->_sid.c_str(),
				user_sid->_sid_name_use
				log_end;

			//
			//	groups
			//
			std::list<pgroup_sid_info> groups;
			if (true != get_process_group(proc->pid(), groups))
			{
				log_err "get_process_group() failed. pid=%u, image=%ws",
					proc->pid(),
					proc->process_name()
					log_end;
			}
			else
			{
				for (auto& group_sid : groups)
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
					strm << L"attribute=" << group_sid->_attribute;

					log_info "    %ws", strm.str().c_str() log_end;
					delete group_sid;
				}
				groups.clear();
			}
			//
			// privilege
			//
			std::list<pprivilege_info> privileges;
			if (true != get_process_privilege(proc->pid(), privileges))
			{
				log_err "get_process_privilege() failed. pid=%u, image=%ws",
					proc->pid(),
					proc->process_name()
					log_end;
			}
			else
			{
				for (auto& privilege : privileges)
				{
					std::wstringstream strm;
					strm << L"privilege=" << privilege->_name;
					strm << L", attribute=" << privilege->_attribute;
					log_info "        %ws", strm.str().c_str() log_end;
					delete privilege;
				}
				privileges.clear();
			}
		}
		delete user_sid; user_sid = nullptr;
		return true;		
	});


	return true;

}