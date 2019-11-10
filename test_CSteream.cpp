/**
 * @file    test_CStream.cpp
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2019/10/05 09:51 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include "_MyLib/src/CStream.h"

bool test_cstream()
{
	_mem_check_begin
	{
		CMemoryStream strm;

		// 스트림이 아직 할당되지 않음
		_ASSERTE(!strm.SetPos(0));

		_ASSERTE(true == strm.Reserve(1024));
		_ASSERTE(strm.GetCapacity() >= 1024);
		_ASSERTE(0 == strm.GetPos());
		_ASSERTE(strm.GetSize() == strm.GetPos());

		//
		//	문자열 쓰기
		//
		const char* test_string = "0123456789";
		size_t size = strlen(test_string) * sizeof(char);

		_ASSERTE(strm.WriteInt<size_t>(size));
		_ASSERTE(size == strm.WriteToStream(test_string, size));
		
		size_t pos = sizeof(size) + size;
		_ASSERTE(pos == strm.GetPos());
		_ASSERTE(strm.GetSize() == strm.GetPos());
		_ASSERTE(strm.GetCapacity() >= pos);

		//
		//	정수형 쓰기/읽기
		//
		_ASSERTE(strm.WriteInt<uint8_t> (0x11));
		_ASSERTE(strm.WriteInt<uint16_t>(0x1122));
		_ASSERTE(strm.WriteInt<uint32_t>(0x11223344));
		_ASSERTE(strm.WriteInt<uint64_t>(0x1122334411223344));

		// 스트림 영역 밖으로 이동시 실패
		_ASSERTE(false == strm.SetPos(strm.GetPos() + 1));

		//
		//	읽기 & 검증 (참조, 읽기)
		//
		{
			_ASSERTE(strm.SetPos(0));
			size = strm.ReadInt<size_t>();

			//	스트림 문자열 참조
			pos = strm.GetPos();

			const char* p = nullptr;
			_ASSERTE(size >= strm.RefFromStream(p, size));
			_ASSERTE(strm.GetPos() > pos);

			std::string s_ref((char*)p, size);
			log_info "ref from stream=%s", s_ref.c_str() log_end;

			_ASSERTE(0x11 == strm.ReadInt<uint8_t>());
			_ASSERTE(0x1122 == strm.ReadInt<uint16_t>());
			_ASSERTE(0x11223344 == strm.ReadInt<uint32_t>());
			_ASSERTE(0x1122334411223344 == strm.ReadInt<uint64_t>());
		}

		//
		//	읽기 & 검증 (복사, 읽기)
		//
		{
			_ASSERTE(strm.SetPos(0));
			size = strm.ReadInt<size_t>();

			auto ptr = std::make_unique<char[]>(size);
			_ASSERTE(ptr);
			_ASSERTE(size >= strm.ReadFromStream(ptr.get(), size));
			std::string s(ptr.get(), size);
			log_info "read from stream=%s", s.c_str() log_end;

			_ASSERTE(0x11 == strm.ReadInt<uint8_t>());
			_ASSERTE(0x1122 == strm.ReadInt<uint16_t>());
			_ASSERTE(0x11223344 == strm.ReadInt<uint32_t>());
			_ASSERTE(0x1122334411223344 == strm.ReadInt<uint64_t>());
		}

		
		

		strm.ClearStream();
		_ASSERTE(0 == strm.GetPos());
		_ASSERTE(0 == strm.GetSize());
		_ASSERTE(0 == strm.GetCapacity());
		_ASSERTE(nullptr == strm.GetMemory());
	}
	_mem_check_end;

	return true;
}



bool test_cstream_read_only()
{
	_mem_check_begin
	{
		CMemoryStream strm(1024, (const char* const)GetModuleHandleW(nullptr));

		// 스트림이 할당되었으므로 SetPos 가능
		_ASSERTE(strm.SetPos(1024));
		_ASSERTE(!strm.SetPos(1024 + 1));
		_ASSERTE(strm.SetPos(0));

		_ASSERTE(true != strm.Reserve(1024));
		_ASSERTE(strm.GetCapacity() == 1024);
		_ASSERTE(0 == strm.GetPos());
		_ASSERTE(strm.GetSize() == 1024);

		//
		//	문자열 쓰기 실패
		//
		const char* test_string = "0123456789";
		size_t size = strlen(test_string) * sizeof(char);

		_ASSERTE(!strm.WriteInt<size_t>(size));
		_ASSERTE(size != strm.WriteToStream(test_string, size));

		//
		//	정수형 쓰기/읽기 실패 
		//
		_ASSERTE(!strm.WriteInt<uint8_t>(0x11));
		_ASSERTE(!strm.WriteInt<uint16_t>(0x1122));
		_ASSERTE(!strm.WriteInt<uint32_t>(0x11223344));
		_ASSERTE(!strm.WriteInt<uint64_t>(0x1122334411223344));

		//
		//	읽기 & 검증
		//
		_ASSERTE(strm.SetPos(0));

		_ASSERTE(strm.ReadInt<uint8_t>() > 0);
		_ASSERTE(strm.ReadInt<uint16_t>() > 0);
		_ASSERTE(strm.ReadInt<uint32_t>() > 0);
		_ASSERTE(strm.ReadInt<uint64_t>() > 0);


		strm.ClearStream();
		_ASSERTE(0 == strm.GetPos());
		_ASSERTE(0 == strm.GetSize());
		_ASSERTE(0 == strm.GetCapacity());
		_ASSERTE(nullptr == strm.GetMemory());
	}
	_mem_check_end;

	return true;
}