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

#ifndef _INCLUDE_C_STREAM_H_
#define _INCLUDE_C_STREAM_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sal.h>
#include <stdint.h>
#include <crtdbg.h>

//
// 최상위 스트림 인터페이스 
// 
class CStream
{
private:	
protected:
	unsigned long m_size;
	unsigned long m_pos;

	// ChangeCursor() method 만이 setPos() 를 호출하며 모든 유효성 검사는 
	// ChangeCursor() method 내에서 수행한다. 
	//
	unsigned long setPos(_In_ const unsigned long newPosition)
	{
		m_pos = newPosition;
		return m_pos;
	};


	virtual unsigned long SetSize(_In_ unsigned long newSize) = 0;
public:
	CStream(): m_pos(0),m_size(0){};
	virtual ~CStream(){};


	unsigned long GetSize() { return m_size; };
	unsigned long GetCurrentCusor() { return m_pos; };
	unsigned long ChangeCursor(_In_ const unsigned long offset, _In_ unsigned long from);


	// 스트림이 사용한 자원을 소멸한다. 
	//	(파일이든, 메모리든...)
	// 
	virtual void ClearStream(void) = 0;

	// 스트림으로 부터 데이터를 읽어서 버퍼에 쓴다.
	virtual unsigned long ReadFromStream(_Out_ void *Buffer, _In_ unsigned long Count) = 0;

	// 버퍼로부터 데이터를 읽어 스트림의 현재 포지션에 쓴다.
	virtual unsigned long WriteToStream(_In_ const void *Buffer, _In_ unsigned long Count) = 0;
	
	virtual unsigned long ReadUint16FromStream(_Out_ uint16_t& value) = 0;
	virtual unsigned long WriteUint16ToStream(_In_ uint16_t value) = 0;
	virtual unsigned long ReadUint32FromStream(_Out_ uint32_t& value) = 0;
	virtual unsigned long WriteUint32ToStream(_In_ uint32_t value) = 0;
};





//
// 메모리 스트림 클래스 
// 
// WARNING !
//		이 클래스에서 다른 클래스를 파생하지 말것.
//		복사/대입/소멸자 등등.. 전혀 파생 객체를 고려하지 않았음. 쿠쿠쿠 :-)
// 
typedef class CMemoryStream : public CStream
{
private:
	char *m_pMemory;
	virtual unsigned long SetSize(_In_ unsigned long newSize);
protected:
public:
	CMemoryStream():m_pMemory(NULL){};
	~CMemoryStream()			
	{
		if (NULL != m_pMemory)
		{
			// free and change cursor			
			//
			if (0 != SetSize(0))
			{
				_ASSERTE(!"SetSize() error");				
			}
		}
	};


	// 스트림이 사용한 자원을 소멸한다. 
	//	(파일이든, 메모리든...)
	//	-- inline method 임 - cpp 쪽으로 넘기지 말것
	//
	virtual void ClearStream(void)
	{
		// free and change cursor			
		//
		if (0 != SetSize(0))
		{
			_ASSERTE(!"SetSize() error");			
		}		
	}

	// ByteToRead 만큼 읽을 수 있는지 검사
	bool CanReadFromStream(_In_ unsigned long ByteToRead)
	{
		if (m_pos + ByteToRead > m_size)
			return false;
		else
			return true;
	}

	// 스트림 버퍼를 이미 할당된 메모리 버퍼로 할당한다. 
	// 
	void SetStreamBuffer(_In_ void* Buffer, _In_ unsigned long BufferSize)
	{
		ClearStream();

		m_pos = 0;
		m_size = BufferSize;
		m_pMemory = (char*)Buffer;
	}

	// 스트림으로 부터 데이터를 읽어서 버퍼에 쓴다.
	virtual unsigned long ReadFromStream(_Out_ void *Buffer, 
										 _In_ unsigned long Count);

	// 버퍼로부터 데이터를 읽어 스트림의 현재 포지션에 쓴다.
	virtual unsigned long WriteToStream(_In_ const void *Buffer, 
										_In_ unsigned long Count);
		
	virtual unsigned long ReadUint16FromStream(_Out_ uint16_t& value);
	virtual unsigned long WriteUint16ToStream(_In_ uint16_t value);
	virtual unsigned long ReadUint32FromStream(_Out_ uint32_t& value);
	virtual unsigned long WriteUint32ToStream(_In_ uint32_t value);
	
	// 메모리 멤버에 직접 엑세스 하기 위한 메소드
	const void *GetMemory() { return m_pMemory; };

} *PMemoryStream;



#endif