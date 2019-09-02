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
// �ֻ��� ��Ʈ�� �������̽� 
// 
class CStream
{
private:	
protected:
	unsigned long m_size;
	unsigned long m_pos;

	// ChangeCursor() method ���� setPos() �� ȣ���ϸ� ��� ��ȿ�� �˻�� 
	// ChangeCursor() method ������ �����Ѵ�. 
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


	// ��Ʈ���� ����� �ڿ��� �Ҹ��Ѵ�. 
	//	(�����̵�, �޸𸮵�...)
	// 
	virtual void ClearStream(void) = 0;

	// ��Ʈ������ ���� �����͸� �о ���ۿ� ����.
	virtual unsigned long ReadFromStream(_Out_ void *Buffer, _In_ unsigned long Count) = 0;

	// ���۷κ��� �����͸� �о� ��Ʈ���� ���� �����ǿ� ����.
	virtual unsigned long WriteToStream(_In_ const void *Buffer, _In_ unsigned long Count) = 0;
	
	virtual unsigned long ReadUint16FromStream(_Out_ uint16_t& value) = 0;
	virtual unsigned long WriteUint16ToStream(_In_ uint16_t value) = 0;
	virtual unsigned long ReadUint32FromStream(_Out_ uint32_t& value) = 0;
	virtual unsigned long WriteUint32ToStream(_In_ uint32_t value) = 0;
};





//
// �޸� ��Ʈ�� Ŭ���� 
// 
// WARNING !
//		�� Ŭ�������� �ٸ� Ŭ������ �Ļ����� ����.
//		����/����/�Ҹ��� ���.. ���� �Ļ� ��ü�� ������� �ʾ���. ������ :-)
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


	// ��Ʈ���� ����� �ڿ��� �Ҹ��Ѵ�. 
	//	(�����̵�, �޸𸮵�...)
	//	-- inline method �� - cpp ������ �ѱ��� ����
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

	// ByteToRead ��ŭ ���� �� �ִ��� �˻�
	bool CanReadFromStream(_In_ unsigned long ByteToRead)
	{
		if (m_pos + ByteToRead > m_size)
			return false;
		else
			return true;
	}

	// ��Ʈ�� ���۸� �̹� �Ҵ�� �޸� ���۷� �Ҵ��Ѵ�. 
	// 
	void SetStreamBuffer(_In_ void* Buffer, _In_ unsigned long BufferSize)
	{
		ClearStream();

		m_pos = 0;
		m_size = BufferSize;
		m_pMemory = (char*)Buffer;
	}

	// ��Ʈ������ ���� �����͸� �о ���ۿ� ����.
	virtual unsigned long ReadFromStream(_Out_ void *Buffer, 
										 _In_ unsigned long Count);

	// ���۷κ��� �����͸� �о� ��Ʈ���� ���� �����ǿ� ����.
	virtual unsigned long WriteToStream(_In_ const void *Buffer, 
										_In_ unsigned long Count);
		
	virtual unsigned long ReadUint16FromStream(_Out_ uint16_t& value);
	virtual unsigned long WriteUint16ToStream(_In_ uint16_t value);
	virtual unsigned long ReadUint32FromStream(_Out_ uint32_t& value);
	virtual unsigned long WriteUint32ToStream(_In_ uint32_t value);
	
	// �޸� ����� ���� ������ �ϱ� ���� �޼ҵ�
	const void *GetMemory() { return m_pMemory; };

} *PMemoryStream;



#endif