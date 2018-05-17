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

/**	-----------------------------------------------------------------------
	\brief	

	\param	
	\return	
	\code
	
	\endcode		
-------------------------------------------------------------------------*/
#define MAX_UNSIGNED_LONG	0xFFFFFFFF	
unsigned long CStream::ChangeCursor(_In_ const unsigned long offset, _In_ unsigned long from)
{
	_ASSERTE(0 <= offset);
	_ASSERTE(0 <= from);
	_ASSERTE(m_pos >= from);
	_ASSERTE(MAX_UNSIGNED_LONG >= from); 
	_ASSERTE(MAX_UNSIGNED_LONG >= offset); 
	_ASSERTE(MAX_UNSIGNED_LONG >= from + offset);

	if (	
			!(0 <= offset) ||
			!(0 <= from) ||
			!(m_pos >= from) ||
			!(MAX_UNSIGNED_LONG >= from) ||
			!(MAX_UNSIGNED_LONG >= offset) || 
			!(MAX_UNSIGNED_LONG >= from + offset)
		)
	{
		return MAX_UNSIGNED_LONG;
	}

	unsigned long newPosition = from + offset;		

	// position 이 스트림의 크기보다 크면 마지막으로 커서를 옮긴다. 
	//
	if (newPosition > m_size)
	{
		newPosition = m_size;		
	}

	setPos(newPosition);		
	return newPosition;
};


/**	-----------------------------------------------------------------------
	\brief	memory stream 의 크기를 변경한다.

	\param	
	\return	
	\code
	
	\endcode		
-------------------------------------------------------------------------*/
unsigned long CMemoryStream::SetSize(_In_ unsigned long newSize)
{
	unsigned long oldPosition = GetCurrentCusor();
	char *ptr=NULL;

	if (0 == newSize)
	{
		free(m_pMemory); m_pMemory=NULL;		
	}
	else
	{
		ptr = (char *) realloc(m_pMemory, newSize);
		if (NULL == ptr)
		{
			// 메모리가 부족함.
			// m_pMemory 는 변경되지 않음. 
			// 
			char log[512]={0};
			StringCbPrintfA(log, sizeof(log), "%s(), can not reallocate memory, new memory size=%u bytes", __FUNCTION__, newSize);
			OutputDebugStringA(log);
			return MAX_UNSIGNED_LONG;
		}
		
		m_pMemory = ptr;
	}
	
	m_size = newSize;
	if (oldPosition > newSize) ChangeCursor(0, newSize);
	
	return newSize;
}



 
/**	-----------------------------------------------------------------------
	\brief	스트림으로 부터 데이터를 읽어서 버퍼에 쓴다.

	\param	
	\return			
			성공시 읽은 바이트 수 리턴 
			(스트림이 Count 보다 작은 경우 포함)
			실패시 -1 리턴
	\code
	
	\endcode		
-------------------------------------------------------------------------*/
unsigned long CMemoryStream::ReadFromStream(_Out_ void *Buffer, _In_ unsigned long Count)
{
	_ASSERTE(nullptr != Buffer);
	if (nullptr == Buffer) return 0;

	if ( (m_pos >= 0) && (Count >= 0) )	
	{
		unsigned long ret = m_size - m_pos;
		if (0 < ret)
		{
			if (ret > Count) ret = Count;

			memmove(Buffer, (char *)(DWORD_PTR(m_pMemory) + m_pos), ret);
			ChangeCursor(ret, m_pos);
			return ret;
		}
	}

	return 0;
}

/**	-----------------------------------------------------------------------
	\brief	버퍼로부터 데이터를 읽어 스트림의 현재 포지션에 쓴다.

	\param	
	\return			
			성공시 Write 한 바이트 수
			실패시 -1
	\code
	
	\endcode		
-------------------------------------------------------------------------*/
unsigned long CMemoryStream::WriteToStream(const void *Buffer, unsigned long Count)
{
	if ( (m_pos >= 0) && (Count >= 0) )
	{
		unsigned long pos = m_pos + Count;
		if (pos > 0)
		{
			if (pos > m_size)
			{
				if (MAX_UNSIGNED_LONG == SetSize(pos))
				{
					return MAX_UNSIGNED_LONG;
				}
			}

			memmove(&m_pMemory[m_pos], Buffer, Count);
			this->m_pos = pos;
			return Count;
		}	  
	}

	return 0;	// write 한 바이트가 0 이므로
}



/// @brief	
unsigned long CMemoryStream::ReadUint16FromStream(_Out_ uint16_t& value)
{
	return ReadFromStream((void*)&value, sizeof(uint16_t));
}

/// @brief	
unsigned long CMemoryStream::WriteUint16ToStream(_In_ uint16_t value)
{
	return WriteToStream(&value, sizeof(uint16_t));
}


/// @brief	
unsigned long CMemoryStream::ReadUint32FromStream(_Out_ uint32_t& value)
{
	return ReadFromStream((void*)&value, sizeof(uint32_t));
}

/// @brief	
unsigned long CMemoryStream::WriteUint32ToStream(_In_ uint32_t value)
{
	return WriteToStream(&value, sizeof(uint32_t));
}



