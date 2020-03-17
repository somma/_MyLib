/**
 * @file    net_util.h
 * @brief   This module implements wrapper functions for Winsock.
 *			Call init_net_util() before use these functions
 *			and call cleanup_net_util() after use.
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2017/12/29 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once

#include <string>
#include <vector>


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <WinDNS.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Dnsapi.lib")

#include "net_util_typedef.h"


bool init_net_util();
void cleanup_net_util();


bool
SocketAddressToStr(
	_In_ const SOCKET_ADDRESS* addr, 
	_Out_ std::string& addr_str
);

bool
SocketAddressToStr(
	_In_ const SOCKADDR* addr,
	_Out_ std::string& addr_str
);


bool
get_host_name(
	_Out_ std::wstring& host_name
);

bool
get_inet_adapters(
	_Out_ std::vector<PInetAdapter>& adapters
);

__inline 
bool
get_inet6_adapters(
	_Out_ std::vector<PInet6Adapter>& adapters
)
{
	UNREFERENCED_PARAMETER(adapters);
	return true;
}

bool
ip_to_dns(
	_In_ uint32_t ip_netbyte_order,
	_In_ bool cache_only,
	_Out_ std::wstring& domain_name
);

bool
dns_to_ip(
	_In_ const wchar_t* domain_name,
	_In_ bool cache_only,
	_Out_ std::vector<uint32_t>& ip_list
);

//
//	Win32Util 에 있던 Winsock 관련 함수들 
//	중복되는 내용도 있고, deprecated 된 함수들도 있고, ...
//	정리를 좀 해야 하는데, 귀찮다. 
// 

std::string
ipv4_to_str(
	_In_ uint32_t ip_netbyte_order
);

std::string
ipv4_to_str(
	_In_ in_addr& ipv4
);

std::string
ipv6_to_str(
	_In_ in_addr6& ipv6
);

bool
str_to_ipv4(
	_In_ const char* const ipv4,
	_Out_ uint32_t& ip_netbyte_order
);

bool
str_to_ipv6(
	_In_ const char* const ipv6,
	_Out_ in_addr6& ip_netbyte_order
);

bool
str_to_ipv4(
	_In_ const wchar_t* const ipv4,
	_Out_ uint32_t& ip_netbyte_order
);

bool
str_to_ipv6(
	_In_ const wchar_t* const ipv6,
	_Out_ in_addr6& ip_netbyte_order
);

bool
get_ip_list_v4(
	_Out_ std::vector<std::string>& ip_list
);

bool
get_broadcast_list_v4(
	_Out_ std::vector<uint32_t>& broadcast_list
);

std::string
get_representative_ip_v4(
);

std::string
get_mac_by_ip_v4(
	_In_ const char* ip_str
);

std::string
mac_to_str(
	_In_ const MacAddrType mac
);

bool
get_addr_infow(
	_In_ const wchar_t* host_name,
	_Out_opt_ std::vector<in_addr>* const ipv4_addrs,
	_Out_opt_ std::vector<in6_addr>* const ipv6_addrs
);



bool
is_reserved_ipv4(
	_In_ uint32_t ip_netbyte_order
);
