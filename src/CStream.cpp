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


/// @brief	Constructor
CMemoryStream::CMemoryStream()
	:
	_capacity(0),
	m_pMemory(nullptr),
	m_size(0),
	m_pos(0),
	_page_size(0),
	_read_only(false)
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

CMemoryStream::CMemoryStream(size_t size, const char* const read_only_ptr)
	:
	_capacity(size),
	m_pMemory(const_cast<char*>(read_only_ptr)),
	m_size(size),
	m_pos(0),
	_page_size(4096),// read_only 모드에서는 필요없어서, 기본값으로 설정
	_read_only(true)
{
}

/// @brief	Destructor
CMemoryStream::~CMemoryStream()
{
	ClearStream();
}

/// @brief	메모리 버퍼를 size 만큼 미리 할당해둔다.
///			메모리 버퍼가 이미 할당되었고, 사용중인 스트림에 대해서는 
///			Reserve() 를 호출 할 수 없다. 
bool CMemoryStream::Reserve(_In_ size_t size)
{
	if (_read_only)
	{
		log_err "Never call on read only mode stream." log_end;
		return 0;
	}
	
	_ASSERTE(0 == _capacity);
	if (_capacity > 0 || m_size > 0 || m_pos > 0 || nullptr != m_pMemory)
	{
		return false;
	}

	return (IncreseSize(size) >= size ? true : false);
}

/// @brief	내부 버퍼의 의 크기 newSize 로 키운다. 
///			성공시 변경한 사이즈(newSize)를 리턴하고, 
///			실패시 0 을 리턴하고, 원래 내부 버퍼는 그대로 둔다.
size_t 
CMemoryStream::IncreseSize(
	_In_ size_t newSize
)
{
	if (_read_only)
	{
		log_err "Never call on read only mode stream." log_end;
		return 0;
	}

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
			log_err
				"No resources for stream. req size=%u",
				new_size
				log_end;
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

/// @brief	`size` 만큼 `Buffer` 에 복사하고, 스트림 포지션을 size 만큼 이동
/// @return	성공시 `size` 를 리턴하고, 스트림의 읽기 가능한 영역이 `size` 보다 
///			작으면 0 (에러)을 리턴
size_t 
CMemoryStream::ReadFromStream(
	_Out_ char* const Buffer, 
	_In_ const size_t size
)
{
	_ASSERTE(size > 0);
	_ASSERTE(size <= m_size);
	if (size == 0 || size > m_size)
	{
		log_err
			"Invalid parameter. =%zu, m_size=%zu",
			size,
			m_size
			log_end;
		return 0;
	}
	
	size_t cb_can_read = m_size - m_pos;
	if (size > cb_can_read)
	{
		log_err
			"Invalid request. available=%zu, size=%zu",
			cb_can_read,
			size
			log_end;
		return 0;
	}
		
	RtlCopyMemory(Buffer, 
		          (char *)(DWORD_PTR(m_pMemory) + m_pos), 
				  size);
	m_pos += size;
	return size;
}

/// @brief	`Buffer` 에 현재 스트림의 포인터를 리턴하고, `size` 만큼 스트림 
///			포지션을 이동
/// @return	성공시 `size` 를 리턴하고, 스트림의 읽기 가능한 영역이 `size` 보다 
///			작으면 0 (에러)을 리턴
size_t
CMemoryStream::RefFromStream(
	_Out_ const char*& Buffer,
	_In_ const size_t size
)
{
	_ASSERTE(size > 0);
	_ASSERTE(size <= m_size);
	if (size == 0 || size > m_size)
	{
		log_err
			"Invalid parameter. =%zu, m_size=%zu",
			size,
			m_size
			log_end;
		return 0;
	}

	size_t cb_can_read = m_size - m_pos;
	if (size > cb_can_read)
	{
		log_err
			"Invalid request. available=%zu, size=%zu",
			cb_can_read,
			size
			log_end;
		return 0;
	}

	Buffer = &m_pMemory[m_pos];
	m_pos += size;
	return size;
}


/// @brief	버퍼로부터 데이터를 읽어 스트림의 현재 포지션에 쓴다.
///			성공시 Write 한 바이트 수
///			실패시 0
size_t 
CMemoryStream::WriteToStream(
	const char* Buffer, 
	size_t size
)
{
	if (_read_only)
	{
		log_err "Never call on read only mode stream." log_end;
		return 0;
	}

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

/// @brief	스트림에 std::string 을 쓴다.
bool CMemoryStream::WriteString(_In_ const std::string& str)
{
	//
	//	빈 string 객체라면 사이즈(0) 값만 스트림에 쓴다.
	//
	if (str.empty())
	{
		return WriteInt<size_t>(0);
	}

	//
	//	스트림에 string 의 바이트 수를 쓴다.
	//	
	if (true != WriteInt<size_t>(str.size() * sizeof(char)))
	{
		return false;
	}

	//
	//	Writes contents of the input string.
	//
	return WriteToStream((char*)str.c_str(), str.size()*sizeof(char));
}

/// @brief	
bool CMemoryStream::WriteWstring(_In_ const std::wstring& wstr)
{
	//
	//	빈 string 객체라면 사이즈(0) 값만 스트림에 쓴다.
	//
	if (wstr.empty())
	{
		return WriteInt<size_t>(0);
	}

	//
	//	스트림에 string 의 바이트 수를 쓴다.
	//	
	if (true != WriteInt<size_t>(wstr.size() * sizeof(wchar_t)))
	{
		return false;
	}

	//
	//	Writes contents of the input string.
	//
	return WriteToStream((char*)wstr.c_str(), wstr.size() * sizeof(wchar_t));
}

/// @brief	
bool CMemoryStream::WriteString(_In_ const char* const str)
{	
	size_t cc = strlen(str);	
	if (nullptr == str || 0 == cc)
	{
		return WriteInt<size_t>(0);
	}

	//
	//	스트림에 string 의 바이트 수를 쓴다.
	//		
	if (true != WriteInt<size_t>(cc * sizeof(char)))
	{
		return false;
	}

	//
	//	Writes contents of the input string.
	//
	return WriteToStream((char*)str, cc * sizeof(char));
}

/// @brief	
bool CMemoryStream::WriteWstring(_In_ const wchar_t* const wstr)
{
	size_t wcc = wcslen(wstr);
	if (nullptr == wstr || 0 == wcc)
	{
		return WriteInt<size_t>(0);
	}

	//
	//	스트림에 string 의 바이트 수를 쓴다.
	//		
	if (true != WriteInt<size_t>(wcc * sizeof(wchar_t)))
	{
		return false;
	}

	//
	//	Writes contents of the input string.
	//
	return WriteToStream((char*)wstr, wcc * sizeof(wchar_t));
}

/// @brief	Reads std::string from stream.
std::string CMemoryStream::ReadString()
{
	size_t size = ReadInt<size_t>();
	if (size > 0)
	{
		const char* p = nullptr;
		if (size != RefFromStream(p, size))
		{
			return _null_stringa;
		}

		return std::string(p, size/sizeof(char));
	}
	else
	{
		return _null_stringa;
	}
}

/// @brief	
std::wstring CMemoryStream::ReadWstring()
{
	size_t size = ReadInt<size_t>();
	if (size > 0)
	{
		_ASSERTE(0 == size % sizeof(wchar_t));
		if (0 != size % sizeof(wchar_t)) return _null_stringw;

		const char* p = nullptr;
		if (size != RefFromStream(p, size))
		{
			return _null_stringw;
		}

		return std::wstring((wchar_t*)p, size / sizeof(wchar_t));
	}
	else
	{
		return _null_stringw;
	}
}
