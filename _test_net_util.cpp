/**
 * @file    Tests for net_util module
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2018/09/19 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "Win32Utils.h"
#include "net_util.h"

#include <vector>

bool test_get_adapters();


/// @brief	net_util tests
bool test_net_util()
{
	bool ret = false;

	_mem_check_begin;
	//_mem_check_break(137);

	ret = test_get_adapters();
	
	_mem_check_end;	

	return ret;
}


bool test_get_adapters()
{
	uint32_t log_to = get_log_to();
	set_log_to(log_to_con | log_to_ods);

	InitializeWinsock();
	std::vector<PNetAdapter> adapters;
	_ASSERTE(true == get_net_adapters(AF_INET, adapters));
	
	for (auto adapter : adapters)
	{
		adapter->dump();
	}

	for (auto item : adapters)
	{
		delete item;
	}
	adapters.clear();

	set_log_to(log_to);
	return false;
}