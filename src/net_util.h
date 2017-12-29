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
	std::string name;
	std::string desc;
	std::string ip;

} *PNetAdapter;

typedef class NetConfig
{
public:
	NetConfig() {}
	~NetConfig() {}

	std::wstring _host_name;
	std::wstring _domain;
	std::vector<std::string> _dns;
	std::vector<NetAdapter> _adapters;
public:
	

} *PNetConfig;



/// SOCKET_ADDRESS
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

