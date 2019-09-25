/*-----------------------------------------------------------------------------
 * CStream.h
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
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sal.h>
#include <stdint.h>
#include <crtdbg.h>

//
// 메모리 스트림 클래스 
//
typedef class CMemoryStream
{
private:
	char *m_pMemory;
	unsigned long m_size;
	unsigned long m_pos; 

public:
	CMemoryStream():m_pMemory(nullptr), m_size(0ll), m_pos(0ll)
	{
	}
	
	virtual ~CMemoryStream()			
	{
		if (nullptr != m_pMemory)
		{
			if (0 != SetSize(0))
			{
				_ASSERTE(!"SetSize() error");				
			}
		}
	}

	// 스트림이 사용한 자원을 소멸한다. 
	void ClearStream(void)
	{
		if (0 != SetSize(0))
		{
			_ASSERTE(!"SetSize() error");			
		}		
	}

	// 메모리 버퍼의 사이즈(할당된)를 리턴한다.
	unsigned long GetSize() { return m_size; };
		
	// 메모리 버퍼 포인터를 리턴한다.
	const void *GetMemory() { return m_pMemory; };
	
	// ByteToRead 만큼 읽을 수 있는지 검사
	bool CanReadFromStream(_In_ unsigned long ByteToRead)
	{
		if (m_pos + ByteToRead > m_size)
			return false;
		else
			return true;
	}

	// 스트림으로 부터 데이터를 읽어서 버퍼에 쓴다.
	unsigned long ReadFromStream(_Out_ void *Buffer, _In_ unsigned long Count);

	// 버퍼로부터 데이터를 읽어 스트림의 현재 포지션에 쓴다.
	unsigned long WriteToStream(_In_ const void *Buffer, _In_ unsigned long Count);		
private:
	
	unsigned long SetSize(_In_ unsigned long newSize);

	unsigned long GetCurrentCusor() { return m_pos; };
	unsigned long ChangeCursor(_In_ const unsigned long offset, _In_ unsigned long from);
	
	// ChangeCursor() method 만이 setPos() 를 호출하며 모든 유효성 검사는 
	// ChangeCursor() method 내에서 수행한다. 
	unsigned long setPos(_In_ const unsigned long newPosition)
	{
		m_pos = newPosition;
		return m_pos;
	}
	
	// 스트림 버퍼를 이미 할당된 메모리 버퍼로 할당한다. 
	void SetStreamBuffer(_In_ void* Buffer, _In_ unsigned long BufferSize)
	{
		ClearStream();

		m_pos = 0;
		m_size = BufferSize;
		m_pMemory = (char*)Buffer;
	}

} *PMemoryStream;



