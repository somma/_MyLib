/**
 * @file    test_unique_ptr.cpp
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2017/10/18 09:51 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"

typedef class Foo
{
public:
	Foo(const char* str_ptr) : foo_str(str_ptr)
	{
	}

	~Foo()
	{
		log_info "destructor, %s", foo_str.c_str() log_end;
	}

	std::string foo_str;
} *PFoo;


bool test_unique_ptr()
{
	_mem_check_begin
	{
		std::map<std::string, std::unique_ptr<Foo> > fooz;

		char buf[128];
		for (int i = 0; i < 10; ++i)
		{
			StringCbPrintfA(buf, sizeof(buf), "index=%d", i);

			fooz.insert(std::make_pair(std::string(buf),
									   std::make_unique<Foo>(buf)));
		}

		//
		// for (auto entry : fooz)  처럼 entry 를 참조자로 받지 않으면
		// `attempting to reference a deleted function` 컴파일 에러 발생
		// unique_ptr 이 복사가 되지 않기 때문인듯
		//
		for (auto& entry : fooz)
		{
			log_info "%s", entry.second.get()->foo_str.c_str() log_end;
		}
	}
	_mem_check_end;

	return true;
}


bool test_unique_ptr_assign()
{
	_mem_check_begin
	{
		std::unique_ptr<char[]> empty_ptr = nullptr;	
		_ASSERTE(empty_ptr.get() == nullptr);

		std::unique_ptr<char[]> y = std::make_unique<char[]>(1024);
		std::unique_ptr<char[]> x = std::make_unique<char[]>(512);

		// unique_ptr 은 복사가 안됨, std::move 를 해야 함
		// y 에 이미 할당되어있던 메모리는 x 를 이동할때 소멸 됨
		y = std::move(x);
		
		//
		//	동일한 테스트............
		//	소멸자는 정상적으로 호출됨
		//
		std::unique_ptr<Foo> yyyy = std::make_unique<Foo>("yyyy");
		std::unique_ptr<Foo> xxxx = std::make_unique<Foo>("xxxx");

		// unique_ptr 은 복사가 안됨, std::move 를 해야 함
		// y 에 이미 할당되어있던 메모리는 x 를 이동할때 소멸 됨
		yyyy = std::move(xxxx);
	}
	_mem_check_end;

	return true;	
}

bool test_unique_ptr_list()
{
	_mem_check_begin
	{
		//
		//	std::list<std::unique_ptr<Obj>> objs 형태의 사용
		//
		std::list<std::unique_ptr<Foo>> fooz;

		for (int i = 0; i < 4; ++i)
		{
			fooz.push_back(std::make_unique<Foo>(std::to_string(i).c_str()));
		}

		// 얘는 호출 안해도 그만...
		fooz.clear();

		//
		//	No leaks!!
		//
	}
	_mem_check_end;

	return true;
}

bool test_unique_ptr_list_remove()
{
	_mem_check_begin
	{
		//
		//	std::list<std::unique_ptr<Obj>> objs 형태에서 아이템  삭제하기
		//
		std::list<std::unique_ptr<Foo>> fooz;

		for (int i = 0; i < 4; ++i)
		{
			fooz.push_back(std::make_unique<Foo>(std::to_string(i).c_str()));
		}

		for (auto& foo : fooz)		
		{
			if (foo->foo_str == "2")
			{
				//
				//	remove 해서 foo 를 날리는 순간 unique_ptr 의 소멸자가 
				//	호출되어 정상적으로 메모리는 해제된다.
				//
				fooz.remove(foo);
				break;
			}
		}

		_ASSERTE(3 == fooz.size());

		//
		//	No leaks
		//

	}
	_mem_check_end;

	return true;
}

bool test_make_unique_struct_allocate()
{
	typedef struct test_struct
	{
		int a;
		int b;
		int c;

	}*ptest_struct;

	_mem_dump_console
	_mem_check_begin
	{
		//
		//	allocate struct
		//
		auto p_rec = std::make_unique<test_struct>();
		p_rec->a = 1;
		p_rec->b = 2;
		p_rec->c = 3;
	}
	_mem_check_end;	

	return true;
}


std::unique_ptr<Foo> return_uniq()
{
	return std::make_unique<Foo>("test");
}

std::unique_ptr<Foo> return_uniq_empty()
{
	auto u = std::make_unique<Foo>("test");
	delete u.release();
	return u;
}

std::unique_ptr<Foo> return_empty()
{
	return {};		// 원래는 요렇게 리턴하는게 정석이라고 함(잘모르겠음)
}

std::unique_ptr<Foo> return_empty2()
{
	return nullptr;	// 이렇게 리턴해도 괜찮음
}

void func_foo_param(_In_ Foo* foo_param)
{
	log_info "str=%s", foo_param->foo_str.c_str() log_end;	
}

void call_func_foo_param()
{
	_mem_check_begin
	{
		// 임시 unique_ptr 객체를 만들어서 포인터만 파라미터로 전달
		func_foo_param(std::make_unique<Foo>("func_foo_param").get());
	}
	_mem_check_end;	
}





bool test_return_unique_ptr()
{
	_mem_check_begin
	{
		auto u = return_uniq();
		_ASSERTE(u);
		auto u2 = return_uniq_empty();
		_ASSERTE(!u2);

		auto u3 = return_empty();
		_ASSERTE(!u3);

		auto u4 = return_empty2();
		_ASSERTE(!u4);

		call_func_foo_param();
	}
	_mem_check_end;

	return true;
}