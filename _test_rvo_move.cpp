/**
 * @file    test for std::move, RVO and ...
 * @brief
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2020/07/19 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"

#define _buf_size 16
#define _size	4


///	@brief	Move semantic 이 적용되지 않은 클래스
class my
{
public:
	my(char* buf) : _buf(buf)
	{
		//log_info "" log_end;
	}

	~my()
	{
		if (nullptr != _buf) free(_buf);
		//log_info "" log_end;
	}

	//
	//	copy constructor
	//
	my(const my& cls)
	{
		size_t len = strlen(cls._buf);		
		_buf = (char*)malloc(len+sizeof(char*));
		RtlCopyMemory(_buf, cls._buf, len);
		_buf[len] = 0x00;
				
		log_info 
			"copy called. 0x%p, %s -> 0x%p, %s", 
			&cls._buf, cls._buf, 
			&_buf, _buf
			log_end;
	}

public:
	char* _buf;
};



///	@brief	Move semantic 이 적용된 클래스
class my_move
{
public:
	my_move(char* buf) : _buf(buf)
	{
		//log_info "" log_end;
	}

	~my_move()
	{
		if (nullptr != _buf) free(_buf); 
		//log_info "" log_end;
	}

	//
	//	Move constructor
	//
	my_move(my_move&& cls)
	{
		_buf = cls._buf;
		cls._buf = nullptr;
	}

	//
	//	Assignment operator
	//
	my_move& operator=(my_move&& cls)
	{
		if (nullptr != _buf) free(_buf);

		_buf = cls._buf;
		cls._buf = nullptr;
	}
	


private:
	//
	//	Disable copy 
	//
	my_move(const my_move& cls);
	my_move& operator=(const my_move& cls);

public:
	char* _buf;
};











std::vector<my> return_big_container_vector_no_reserve();
std::vector<my> return_big_container_vector();
std::list<my> return_big_container_list();
std::list<my_move> return_big_container_list_move();

bool test_rvo_and_move()
{
	_mem_check_begin
	{
		log_info "== return_big_container_vector_no_reserve() ==" log_end;
		auto bc = return_big_container_vector_no_reserve();
		for (const auto& i : bc) { log_info "%s", i._buf log_end;}
	}
	_mem_check_end;
	
	{
		log_info "== return_big_container_vector() ==" log_end; 
		auto bc = return_big_container_vector();
		for (const auto& i : bc) { log_info "%s", i._buf log_end; }
	}
	_mem_check_end;

	{
		log_info "== return_big_container_list() ==" log_end; 
		auto bc = return_big_container_list();
		for (const auto& i : bc) { log_info "%s", i._buf log_end; }
	}
	_mem_check_end;

	{
		log_info "== return_big_container_list_move() ==" log_end;
		auto bc = return_big_container_list_move();
		for (const auto& i : bc) { log_info "%s", i._buf log_end; }
	}
	_mem_check_end;

	return true;
}


/// @brief	
std::vector<my> return_big_container_vector_no_reserve()
{
	std::vector<my> bc;
	for (uint32_t i = 0; i < _size; ++i)
	{
		char* buf = (char*)malloc(_buf_size);
		StringCbPrintfA(buf, _buf_size, "value=%u", i);
		
		//
		//	my 객체 생성
		//	push_back() 에 파라미터로 넘어갈때 복사
		//	my 객체 소멸자 호출
		//
		bc.push_back(my(buf));
	}

	// 
	//	STL container 는 이미 move semantic 이 적용되었있기때문에 
	//	불필요한 복사는 일어나지 않는다.
	//
	return bc;
}

/// @brief	
std::vector<my> return_big_container_vector()
{
	std::vector<my> bc;

	//
	//	vector 객체에 item 이 추가되면서 vector 사이즈를 키우고
	//	이전 vector 에 있던 item 들을 복사
	//	따라서 vector 사용시 reserve() 는 필수
	//
	bc.reserve(_size);

	for (uint32_t i = 0; i < _size; ++i)
	{
		char* buf = (char*)malloc(_buf_size);
		StringCbPrintfA(buf, _buf_size, "value=%u", i);


		//
		//	my 객체 생성
		//	push_back() 에 파라미터로 넘어갈때 복사
		//	my 객체 소멸자 호출
		//
		bc.push_back(my(buf));
	}


	// 
	//	STL container 는 이미 move semantic 이 적용되었있기때문에 
	//	불필요한 복사는 일어나지 않는다.
	//
	return bc;
}

/// @brief	
std::list<my> return_big_container_list()
{
	std::list<my> bc;

	for (uint32_t i = 0; i < _size; ++i)
	{
		char* buf = (char*)malloc(_buf_size);
		StringCbPrintfA(buf, _buf_size, "value=%u", i);


		//
		//	my 객체 생성
		//	push_back() 에 파라미터로 넘어갈때 복사
		//	my 객체 소멸자 호출
		//
		bc.push_back(my(buf));
	}


	// 
	//	STL container 는 이미 move semantic 이 적용되었있기때문에 
	//	불필요한 복사는 일어나지 않는다.
	//
	return bc;
}

/// @brief	
std::list<my_move> return_big_container_list_move()
{
	std::list<my_move> bc;

	for (uint32_t i = 0; i < _size; ++i)
	{
		char* buf = (char*)malloc(_buf_size);
		StringCbPrintfA(buf, _buf_size, "value=%u", i);


		//
		//	my 객체 생성
		//	push_back() 에 파라미터로 넘어갈때 복사
		//	my 객체 소멸자 호출
		//
		bc.push_back(my_move(buf));
	}


	// 
	//	STL container 는 이미 move semantic 이 적용되었있기때문에 
	//	불필요한 복사는 일어나지 않는다.
	//
	return bc;
}