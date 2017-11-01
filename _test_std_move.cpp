/**
 * @file    _test_std_move.cpp
 * @brief   
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2017/10/18 09:51 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include <list>



typedef class test_class
{
public:
	test_class(int v):_v(v)	{log_info "constructor (%d)", v log_end;}
	test_class(const test_class& rhs) 
	{
		_v = rhs._v;
		log_info "copy ctor (%d)", _v log_end;
	}
	~test_class()	{log_info "~test_class(%d)", _v log_end;}

	int _v;
	int k;
	int j;
	int x;
	int z;
} *ptest_class;

void print_test_class(test_class o)
{
	log_info "v = %d", o._v log_end;
}

void print_test_classex(test_class&& o)
{
	log_info "v = %d", o._v log_end;
}






// 헐 머야 똑같네....이런..ㅅㅂ
bool test_std_move()
{
	//for (int i = 0; i < 10; ++i)
	//{
	//	print_test_class(test_class(i));
	//}
	//
	//log_info "" log_end;

	//for (int i = 0; i < 10; ++i)
	//{
	//	print_test_classex(test_class(i));
	//}


	std::list<test_class> ll;
	{
		test_class l(1);
		ll.push_back(std::move(l));
		log_info "v=%d", l._v log_end;
	}

	for (auto l : ll)
	{
		log_info "v=%d", l._v log_end;
	}	
	return true;
}

