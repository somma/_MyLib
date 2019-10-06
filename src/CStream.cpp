/*-----------------------------------------------------------------------------
 * CStream.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh Yong Hwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * Revision History:
 * Date					Who					What
 * ----------------		----------------	----------------
 * 22/03/2007			Noh Yong Hwan		birth
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "CStream.h"
#include <strsafe.h>

/// @brief	Constructor
CMemoryStream::CMemoryStream() 
	:
	_capacity(0),
	m_pMemory(nullptr), 
	m_size(0), 
	m_pos(0), 
	_page_size(0)
{
	SYSTEM_INFO si = { 0 };
	GetSystemInfo(&si);
	_page_size = si.dwPageSize;

	_ASSERTE(_page_size > 0);
	if (_page_size == 0)
	{
		_page_size = 4096;
	}
}

/// @brief	메모리 버퍼를 size 만큼 미리 할당해둔다.
///			메모리 버퍼가 이미 할당되었고, 사용중인 스트림에 대해서는 
///			Reserve() 를 호출 할 수 없다. 
bool CMemoryStream::Reserve(_In_ size_t size)
{
	_ASSERTE(0 == _capacity);
	if (_capacity > 0 || m_size > 0 || m_pos > 0 || nullptr != m_pMemory)
	{
		return false;
	}

	return (IncreseSize(size) >= size ? true : false);
}

/// @brief	Destructor
CMemoryStream::~CMemoryStream()
{
	ClearStream();
}

/// @brief	내부 버퍼의 의 크기 newSize 로 키운다. 
///			성공시 변경한 사이즈(newSize)를 리턴하고, 
///			실패시 0 을 리턴하고, 원래 내부 버퍼는 그대로 둔다.
size_t 
CMemoryStream::IncreseSize(
	_In_ size_t newSize
)
{
	if (0 == newSize)
	{
		if (nullptr == m_pMemory)
		{
			free(m_pMemory); 
			m_pMemory = nullptr;
		}

		_capacity = 0;
		m_size = 0;
		m_pos = 0;
	}
	else
	{
		//
		//	스트림의 사이즈를 줄이는 것은 불가능함
		//
		if (m_pos > newSize)
		{
			return 0;
		}

		//
		//	요청한 사이즈를 페이지 사이즈로 올림한다.
		//
		size_t new_size = newSize;
		_ASSERTE(_page_size > 0);			
		uint32_t remain = newSize % _page_size;
		if (0 != remain)
		{
			new_size += (_page_size - remain);
		}		
		
		char *ptr = (char *) realloc(m_pMemory, new_size);
		if (nullptr == ptr)
		{
			// 메모리 부족, m_pMemory 는 변경되지 않음. 
			char log[512]={0};
			StringCbPrintfA(log, 
							sizeof(log), 
							"%s(), can not reallocate memory, new memory size=%u bytes", 
							__FUNCTION__, 
							newSize);
			OutputDebugStringA(log);
			return 0;
		}
		else
		{
			_capacity = new_size;
			m_pMemory = ptr;			
		}
	}
	
	return _capacity;
}

/// @brief	스트림으로 부터 데이터를 읽어서 버퍼에 쓴다.
///			성공시 읽은 바이트 수 리턴 (스트림이 size 보다 작은 경우 포함)
///			실패시 0 리턴
size_t 
CMemoryStream::ReadFromStream(
	_Out_ void *Buffer, 
	_In_ size_t size
)
{
	_ASSERTE(nullptr != Buffer);
	if (nullptr == Buffer) return 0;

	if ( (m_pos >= 0) && (size >= 0) )	
	{
		size_t cb_read = m_size - m_pos;
		if (cb_read > 0)
		{
			if (cb_read > size) cb_read = size;
		
			RtlCopyMemory(Buffer, (char *)(DWORD_PTR(m_pMemory) + m_pos), cb_read);
			m_pos += cb_read;
			return cb_read;
		}
	}

	return 0;
}

/// @brief	버퍼로부터 데이터를 읽어 스트림의 현재 포지션에 쓴다.
///			성공시 Write 한 바이트 수
///			실패시 0
size_t 
CMemoryStream::WriteToStream(
	const void *Buffer, 
	size_t size
)
{
	_ASSERTE(nullptr != Buffer);
	_ASSERTE(size > 0);
	if (nullptr == Buffer || size == 0 || (size_t)(-1) == size)
	{
		return 0;
	}

	if ( (m_pos >= 0) && (size >= 0) )
	{
		size_t pos = m_pos + size;
		if (pos > 0)
		{
			if (pos > m_size)
			{
				if (0 == IncreseSize(pos))
				{
					return 0;
				}
			}

			RtlCopyMemory(&m_pMemory[m_pos], Buffer, size);

			m_pos = pos;
			m_size = m_pos;
			return size;
		}	  
	}

	return 0;	// write 한 바이트가 0 이므로
}

