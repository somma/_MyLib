/**
 * @file    _test_call_by_value_container.cpp
 * @brief   
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2019/09/12 13:51 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include <vector>

class Widget
{
public: 
	Widget(int val) : _val(val)
	{
		log_info "constructor, val=%u", _val log_end;
	}

	Widget(Widget const& other): _val(other._val)
	{
		log_info "copy constructor, val=%u", _val log_end;
	}

	~Widget()
	{
		log_info "destructor, val=%u", _val log_end;
	}


private:
	uint32_t _val;

};


void func_call_by_value(std::vector<Widget> wlist)
{
	for (Widget w : wlist)
	{
		w = w;
	}
}

void func_call_by_ref(std::vector<Widget>& wlist)
{
	for (Widget& w : wlist)
	{
		w = w;
	}
}


bool test_callby_value_container()
{
	_mem_check_begin
	{
		std::vector<Widget> wlist;
		for (int i = 0; i < 4; ++i)
		{
			wlist.push_back(Widget(i));
		}

		// 
		log_info "=== func_call_by_value ===" log_end;
		func_call_by_value(wlist);
		
		//
		log_info "=== func_call_by_ref ===" log_end;
		func_call_by_ref(wlist);

		log_info "=== test finished ===" log_end;

	}
	_mem_check_end;

	return true;
}
