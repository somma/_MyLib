/**
 * @file    net_util_typedef.h
 * @brief   This module defines custom types that used by net_util.h/cpp module.
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2017/12/29 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once

using MacAddrType = unsigned char[6];

typedef class Ipv4Info
{
public:
	Ipv4Info(uint32_t ip, uint32_t mask) : ip(ip), mask(mask) {};

	uint32_t ip;
	uint32_t mask;

} *PIpv4Info;


typedef class InetAdapter
{
public:
	InetAdapter(IFTYPE interface_type): interface_type(interface_type)
	{
	}

	virtual ~InetAdapter()
	{
		for (const auto& ip : ip_info_list)
		{
			_ASSERTE(nullptr != ip);
			delete ip;
		}
		ip_info_list.clear();
	}

	IFTYPE interface_type;
	std::wstring friendly_name;		// Wi-Fi
	std::string name;				// {7F158482-83C5-4C7F-B47C-4CE15F1899CA}
	std::wstring desc;				// Marvell AVASTAR Wireless-AC Network Controller
	MacAddrType physical_address;

	std::vector<PIpv4Info> ip_info_list;
	std::vector<uint32_t> dns_list;
	std::vector<uint32_t> gateway_list;

} *PInetAdapter;


//
//	AF_INET6
//
typedef struct Ipv6Info
{
	uint32_t dumy;
} *PIpv6Info;

typedef class Inet6Adapter
{
private:
	Inet6Adapter() {}
	virtual ~Inet6Adapter() {}
} *PInet6Adapter;