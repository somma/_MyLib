/**
 * @file    net_util_typedef.h
 * @brief   This module defines custom types that used by net_util.h/cpp module.
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2017/12/29 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once

typedef class NetIpInfo
{
public:
	NetIpInfo() {}
	virtual ~NetIpInfo() {};
public:
	std::string ip;
	uint32_t subnet_mask;

} *PNetIpInfo;


typedef class NetAdapter
{
public:
	NetAdapter()
	{
	}

	virtual ~NetAdapter()
	{
		for (auto ip : ip_info_list)
		{
			_ASSERTE(nullptr != ip);
			delete ip;
		}
		ip_info_list.clear();
	}

	std::wstring friendly_name;		// Wi-Fi
	std::string name;				// {7F158482-83C5-4C7F-B47C-4CE15F1899CA}
	std::wstring desc;				// Marvell AVASTAR Wireless-AC Network Controller
	std::string physical_address;	// BC-83-85-2D-8A-91

	std::vector<PNetIpInfo> ip_info_list;
	std::vector<std::string> dns_list;
	std::vector<std::string> gateway_list;

} *PNetAdapter;
