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
		std::unique_ptr<char[]> y = std::make_unique<char[]>(1024);
		std::unique_ptr<char[]> x = std::make_unique<char[]>(512);

		// unique_ptr 은 복사가 안됨
		// std::move 를 해야 함
		y = std::move(x);
			// y 에 이미 할당되어있던 메모리는 x 를 이동할때 소멸 됨
			// x 포이터는 y 로 이동되었고, y 소멸시 해제 됌

		
		//
		//	동일한 테스트............
		//	소멸자 호출이 정상적으로 되는지 확인
		//

		std::unique_ptr<Foo> yyyy = std::make_unique<Foo>("yyyy");
		std::unique_ptr<Foo> xxxx = std::make_unique<Foo>("xxxx");

		// unique_ptr 은 복사가 안됨
		// std::move 를 해야 함
		yyyy = std::move(xxxx);
		// y 에 이미 할당되어있던 메모리는 x 를 이동할때 소멸 됨
		// x 포이터는 y 로 이동되었고, y 소멸시 해제 됌
	}
	_mem_check_end;

	return true;	
}