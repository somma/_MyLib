/**
 * @file    Tests for net_util module
 * @brief
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2018/09/19 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include <vector>
#include "_MyLib/src/Win32Utils.h"
#include "_MyLib/src/net_util.h"

void dump_adapter(_In_ const PInetAdapter adapter)
{

	//
	//	Aapter information
	//
	log_msg
		"\nAdapter, type=%u, friendly name=%ws, name=%s, desc=%ws, mac=%s",
		adapter->interface_type,
		adapter->friendly_name.c_str(),
		adapter->name.c_str(),
		adapter->desc.c_str(),
		mac_to_str(adapter->physical_address).c_str()
		log_end;	

	//
	//	DNS list
	//
	log_msg "+ Dump DNS lists" log_end;
	for (auto& dns : adapter->dns_list)
	{
		log_msg "  - dns=%s", ipv4_to_str(dns).c_str() log_end;
	}

	//
	//	Gateway list
	//
	log_msg "+ Dump gateway lists" log_end;
	for (auto& gw : adapter->gateway_list)
	{
		
		log_msg "  - gw=%s", ipv4_to_str(gw).c_str() log_end;
	}

	//
	//	IP information list
	//
	log_msg "+ Dump IP assignments" log_end;
	for (auto& ip : adapter->ip_info_list)
	{
		log_msg
			"  - %s/%s",
			ipv4_to_str(ip->ip).c_str(),
			ipv4_to_str(ip->mask).c_str()
			log_end;
	}
}


bool test_get_adapters()
{
	_mem_check_begin
	{
		uint32_t log_to = get_log_to();
		set_log_to(log_to_con | log_to_ods);

		init_net_util();
		do
		{
			std::vector<PInetAdapter> adapters;
			_ASSERTE(true == get_inet_adapters(adapters));

			for (auto& adapter : adapters)
			{
				dump_adapter(adapter);
			}

			for (auto& item : adapters)
			{
				delete item;
			}
			adapters.clear();
		} while (false);
		cleanup_net_util();

		

		set_log_to(log_to);
	}
	_mem_check_end;
	return true;
}


bool test_get_addr_info()
{
	_mem_check_begin 
	{
		wchar_t* host_names[] ={
			L"DESKTOP-IQPVKKP",
			L"update.somma.kr",
			L"api.somma.kr", 
			L"www.naver.com",
			L"www.google.com"
		};		
		
		for (int i = 0; i < sizeof(host_names) / sizeof(wchar_t*); ++i)
		{
			std::vector<in_addr> ipv4_list;
			std::vector<in_addr6> ipv6_list;

			_ASSERT(true == get_addr_infow(host_names[i],
										   &ipv4_list,
										   &ipv6_list));

			log_info "host=%ws", host_names[i] log_end;
			for (auto& ip4 : ipv4_list)
			{
				log_info "  ipv4=%s", ipv4_to_str(ip4).c_str() log_end;
			}

			for (auto& ip6 : ipv6_list)
			{
				log_info "  ipv6=%s", ipv6_to_str(ip6).c_str() log_end;
			}
			log_info "" log_end;
		}
	} 
	_mem_check_end;

	return true;
}
