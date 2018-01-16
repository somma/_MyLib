/**
 * @file    Network configuration related codes based on IPHelp api
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2017/12/29 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

//
//	std
//
#include <string>
#include <vector>

//
//	windows
//
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

//
//	libs
//
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")


typedef class NetAdapter
{
public:
	std::wstring friendly_name;		// Wi-Fi
	std::string name;				// {7F158482-83C5-4C7F-B47C-4CE15F1899CA}
	std::wstring desc;				// Marvell AVASTAR Wireless-AC Network Controller
	std::string physical_address;	// BC-83-85-2D-8A-91

	std::vector<std::string> ip_list;
	std::vector<uint32_t> subnet_mask_list;
	std::vector<std::string> dns_list;
	std::vector<std::string> gateway_list;

public:
	void dump();

} *PNetAdapter;


bool
InitializeWinsock(
	);

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
get_net_adapters(
	_In_ ULONG net_family, 
	_Out_ std::vector<PNetAdapter>& adapters
	);



//
//	Win32Util �� �ִ� Winsock ���� �Լ��� 
//	�ߺ��Ǵ� ���뵵 �ְ�, deprecated �� �Լ��鵵 �ְ�, ...
//	������ �� �ؾ� �ϴµ�, ������. 
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
	_In_ const wchar_t* ipv4, 
	_Out_ uint32_t& ip_netbyte_order
	);

bool 
str_to_ipv6(
	_In_ const wchar_t* ipv6, 
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
