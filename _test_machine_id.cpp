/**
 * @file    test_machine_id.cpp
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    07.22.2022 15:13 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "_MyLib/src/machine_id.h"

/// @brief	
bool test_generate_machine_id()
{
	bool ret = false;
	_mem_check_begin
	{
		std::string machine_id;
		ret = generate_machine_id(machine_id);
		log_info "machine_id=%s", machine_id.c_str() log_end;
	}
	_mem_check_end;
	
	return ret;
}