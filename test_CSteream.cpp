/**
 * @file    test_CStream.cpp
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2019/10/05 09:51 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include "CStream.h"

bool test_cstream()
{
	_mem_check_begin
	{
		CMemoryStream strm;
		_ASSERTE(true == strm.Reserve(1024));
		_ASSERTE(0 == strm.GetPos());
		_ASSERTE(strm.GetSize() == strm.GetPos());
		_ASSERTE(1024 == strm.GetCapacity());

		const char* test_string = "0123456789";

		size_t size = strlen(test_string) * sizeof(char);

		_ASSERTE(size == (size_t)(strm.WriteToStream(test_string, (unsigned long)size)));
		_ASSERTE(size == strm.GetPos());
		_ASSERTE(strm.GetSize() == strm.GetPos());
		_ASSERTE(1024 == strm.GetCapacity());

		strm.ClearStream();
		_ASSERTE(0 == strm.GetPos());
		_ASSERTE(0 == strm.GetSize());
		_ASSERTE(nullptr == strm.GetMemory());

		return true;
	}
	_mem_check_end;
}

