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
#include "BaseWindowsHeader.h"
#include "log.h"

//
// 메모리 스트림 클래스 
//
typedef class CMemoryStream
{
private:
	size_t _capacity; 
	char *m_pMemory;	
	size_t m_size;
	size_t m_pos;

	DWORD _page_size;

	bool _read_only;

public:
	CMemoryStream();
	CMemoryStream(size_t size, const char* const read_only_ptr);
	
	virtual ~CMemoryStream();
	
	bool Reserve(_In_ size_t size);
	void ClearStream(void);

	// 스트림 버퍼의 할당된 사이즈 (데이터 + 여유공간)
	size_t GetCapacity() { return _capacity; }

	// 스트림의 사이즈 (스트림 버퍼내 할당된 데이터의 크기)
	size_t GetSize() { return m_size; };

	// 스트림의 포지션을 리턴한다.
	size_t GetPos() { return m_pos; };

	// 스트림의 포지션을 이동한다. 
	bool SetPos(_In_ size_t new_pos);
		
	// 스트림 버퍼 포인터를 리턴한다.
	const void *GetMemory() { return m_pMemory; };
	
	// 스트림으로 부터 데이터를 읽어서 버퍼에 쓴다.	
	size_t ReadFromStream(_Out_ void *Buffer, _In_ size_t size);

	// 버퍼로부터 데이터를 읽어 스트림의 현재 포지션에 쓴다.
	size_t WriteToStream(_In_ const void *Buffer, _In_ size_t size);

	/// @brief	스트림으로부터 integer type 값을 읽고, 읽은 값을 리턴한다.
	///
	///			참고.
	///			에러가 발생한 경우와 실제 값이 0 인 경우가 구분이 안되는데, 
	///			스트림 읽기의 경우 에러가 없다고 간주한다. 
	template <typename int_type> int_type ReadInt()
	{
		int_type value;
		if (sizeof(value) != ReadFromStream((void*)&value, sizeof(value)))
		{
			return 0;
		}
		else
		{
			return value;
		}
	}

	/// @brief	스트림에 integer type 값을 쓰고, 성공시 true 를 리턴한다.
	template <typename int_type> bool WriteInt(const int_type value)
	{
		if (_read_only)
		{
			log_err "Never call on read only mode stream." log_end;
			return 0;
		}

		if (sizeof(value) != WriteToStream((const void*)&value, sizeof(value)))
		{
			return false;
		}
		else
		{
			return true;
		}
	}
private:	
	size_t IncreseSize(_In_ size_t newSize);

} *PMemoryStream;




/// @brief	스트림이 사용한 자원을 소멸한다. 
inline void CMemoryStream::ClearStream(void)
{	
	if (!_read_only && nullptr != m_pMemory)
	{
		free(m_pMemory);
	}

	_capacity = 0;
	m_pMemory = nullptr;
	m_size = 0;
	m_pos = 0;
	_read_only = false;
}

/// @brief	스트림의 포지션을 이동한다. 
///			스트림이 유효하고, 요청한 위치가 스트림 범위내인 경우 이동이 가능하다. 
inline bool CMemoryStream::SetPos(_In_ size_t new_pos)
{
	if (m_size > 0 && m_size >= new_pos)
	{
		m_pos = new_pos;
		return true;
	}
	else
	{
		return false;
	}
}
