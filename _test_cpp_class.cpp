/**----------------------------------------------------------------------------
 * _test_cpp_class.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:2:3 20:24 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"

class base
{
public: 
	base(int param1)
	{
		log_msg L"" log_end
	}

	virtual ~base()
	{
		log_msg L"" log_end
	}

public:
	int member1;
};

class child : public base
{
public:
	child(int param1):base(param1)
	{
		log_msg L"" log_end
	}
	virtual ~child()
	{
		log_msg L"" log_end
	}
};


class child_has_diff_creator : public base
{
public:
	child_has_diff_creator(int param1, int param2): base(param1)
	{
		log_msg L"" log_end
	}
	virtual ~child_has_diff_creator()
	{
		log_msg L"" log_end
	}
};

/**
 * @brief	생성자 / 소멸자 호출 순서 확인용 테스트
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool test_cpp_class()
{
	/* expected output 
		[INFO] base::base,
		[INFO] child::child,
		[INFO] child::~child,
		[INFO] base::~base,
	*/
	base* obj = new child(1);
	delete obj;

	/*
		base::base,
		child::child,
		child::~child,
		base::~base,
	*/
	child* obj2 = new child(1);
	delete obj2;

	/*
		base::base,
		child_has_diff_creator::child_has_diff_creator,
		child_has_diff_creator::~child_has_diff_creator,
		base::~base,
	*/
	child_has_diff_creator* obj3 = new child_has_diff_creator(1,2);
	obj3->member1 = 1;
	delete obj3;

	return true;
}