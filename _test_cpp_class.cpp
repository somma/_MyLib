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

class test_base
{
public:
	test_base(int p1, int p2): member1(p1), pmember1(p2)
	{
		log_dbg "" log_end
	}

	virtual ~test_base()
	{
		log_dbg "" log_end
	}

	/// @brief	Copy constructor
	test_base(const test_base& cls) :
		member1(cls.member1),
		pmember1(cls.pmember1)
	{
		log_dbg "copy constructor using `test_base&`" log_end
	}

	test_base(const test_base* const cls) :
		member1(cls->member1),
		pmember1(cls->pmember1)
	{
		log_dbg "copy constructor using `test_base*`" log_end
	}

	/// @brief	Assign operator
	test_base& operator = (const test_base& cls)
	{
		log_dbg "assign op using `test_base&`" log_end

		member1 = cls.member1;
		pmember1 = cls.pmember1;
		return *this;
	}

	test_base& operator = (const test_base* cls)
	{
		log_dbg "assign op using `test_base*`" log_end

		member1 = cls->member1;
		pmember1 = cls->pmember1;
		return *this;

	}

public:
	int member1;

private:
	int pmember1;
};

class child : public test_base
{
public:
	child(int param1):test_base(param1, 100)
	{
		log_dbg "" log_end
	}
	virtual ~child()
	{
		log_dbg "" log_end
	}
};


class child_has_diff_creator : public test_base
{
public:
	child_has_diff_creator(int param1, int param2, int param3): test_base(param1, param2)
	{
        UNREFERENCED_PARAMETER(param2);
		UNREFERENCED_PARAMETER(param3);
		log_dbg "" log_end
	}
	virtual ~child_has_diff_creator()
	{
		log_dbg "" log_end
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
bool test_call_order()
{
	/* expected output 
		[INFO] test_base::test_base,
		[INFO] child::child,
		[INFO] child::~child,
		[INFO] test_base::~test_base,
	*/
	test_base* obj = new child(1);
	delete obj;

	/*
		test_base::test_base,
		child::child,
		child::~child,
		test_base::~test_base,
	*/
	child* obj2 = new child(1);
	delete obj2;

	/*
		test_base::test_base,
		child_has_diff_creator::child_has_diff_creator,
		child_has_diff_creator::~child_has_diff_creator,
		test_base::~test_base,
	*/
	child_has_diff_creator* obj3 = new child_has_diff_creator(1,2, 3);
	obj3->member1 = 1;
	delete obj3;

	

	return true;
}

//
//	복사생성자/연산자 오버로딩
//
bool test_copy_assign()
{
	test_base* base = new test_base(1, 2);

	// 
	// copy constructor
	//
	test_base base_copy = *base;
	test_base base_copy2(base);
	test_base base_copy3 = *base;
	test_base base_copy4 = base;

	// 
	// assignment
	//
	test_base* base_assign = new test_base(1, 2);
	
	//	둘다 포인터인 경우 대입연산자 호출이 아니라, raw 포인터 
	//	assignment 가 되어버린다. 
	base_assign = base;

	
	test_base base_assign2(1, 2);
	test_base base_assign3(1, 3);
	base_assign2 = base;	
	base_assign2 = base_assign3;
	return true;
}



bool test_cpp_class()
{
	bool b1, b2, b3;
	get_log_format(b1, b2, b3);
	set_log_format(false, false, true);

	if (true != test_call_order()) return false;
	if (true != test_copy_assign()) return false;
	
	set_log_format(b1, b2, b3);

	return true;
}

