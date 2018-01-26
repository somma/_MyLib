/**
 * @file    mm_queue.h
 * @brief   Queue implementation using Memory Mapped File
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2018.01.25 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "mm_queue.h"

#include <crtdbg.h>
#include "Win32Utils.h"


MmQueue::MmQueue():
	_optimized_io_size(0), 
	_mmf_size(0),
	_mmf_handle(INVALID_HANDLE_VALUE),
	_map_handle(NULL), 
	_mmf(nullptr),
	_push_offset(0),
	_pop_offset(0), 
	_remain(0)
{
}

MmQueue::~MmQueue()
{
	finalize();
}

/// @brief	
bool MmQueue::initialize(_In_ uint32_t size, _In_ const wchar_t* file_path)
{
	_ASSERTE(0 < size);
	_ASSERTE(nullptr != file_path);
	if (!(size > 0) || nullptr == file_path) return false;

	//
	//	I/O �� ����ȭ�� ������� ���Ѵ�.
	//	��Ȯ�� ��ġ�� �𸣰����� SYSTEM_INFO.dwAllocationGranularity(64k) * 8 ������ 
	//	���� ������ ������ �����°� ����. (win7, win10 ���� �׽�Ʈ)
	//
	SYSTEM_INFO si = { 0 };
	GetSystemInfo(&si);
	if (0 == si.dwAllocationGranularity)
	{
		_optimized_io_size = ((64 * 1024) * 8);
	}
	else
	{
		_optimized_io_size = (si.dwAllocationGranularity * 8);
	}

	//
	//	Memory mapped I/O �� ����� ������ �����Ѵ�.
	//	�̹� ������ �����Ѵٸ� ����ó���Ѵ�. 
	// 
	if (true == is_file_existsW(file_path))
	{
		log_err "File already exists. path=%ws",
			file_path
			log_end;
		return false;
	}

	//
	//	FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE �ɼ��� ����ؼ�
	//	������ �� file �� flush ���� �ʵ��� �ϰ�, handle �� ���� �� MMF ������
	//	�����Ѵ�.
	//
	_ASSERTE(INVALID_HANDLE_VALUE == _mmf_handle);
	_mmf_handle = CreateFileW(file_path,
							  GENERIC_READ | GENERIC_WRITE,
#ifdef _DEBUG
							  FILE_SHARE_READ,
#else
							  0, // no share
#endif//_DEBUG
							  nullptr,
							  CREATE_ALWAYS,
							  FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
							  NULL);
	if (INVALID_HANDLE_VALUE == _mmf_handle)
	{
		log_err "CreateFile() failed. file=%ws, gle=%u",
			file_path,
			GetLastError()
			log_end;
		return false;
	}

	//
	//	��û�� size ��ŭ ���ϻ���� Ű���.
	//
	if (true != set_file_size(_mmf_handle, size))
	{
		log_err "set_file_size() failed. requested size=%u",
			_mmf_size
			log_end;
		return false;
	}

	//
	//	��û�� �������� mapping object �� �����ϰ�, 
	//	mapping object ��ü�� process �ּҰ����� �����Ѵ�. 
	//
	_ASSERTE(nullptr == _mmf);
	_ASSERTE(NULL == _map_handle);
	_map_handle = CreateFileMappingW(_mmf_handle,
									 nullptr,
									 PAGE_READWRITE,
									 0,
									 0,
									 nullptr);
	if (NULL == _map_handle)
	{
		log_err "CreateFileMappingW() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	_mmf = (char*)MapViewOfFile(_map_handle,
								PAGE_READWRITE,
								0,
								0,
								0);
	if (nullptr == _mmf)
	{
		log_err "MapViewOfFile() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	//
	//	size ������ ������Ʈ
	//
	_mmf_size = size;
	_remain = size;

	return true;
}

bool MmQueue::push_back(_In_ uint32_t size, _In_ const char* const buf)
{
	_ASSERTE(nullptr != _mmf);
	_ASSERTE(size > 0);
	if (nullptr == _mmf) return false;
	if (!(size > 0)) return false;

	_ASSERTE(size < _mmf_size);
	_ASSERTE(nullptr != buf);
	if (size >= _mmf_size) return false;	
	if (nullptr == buf) return false;

	//
	//	���� �������� �� ū����� ��û�� ��� 
	//
	uint32_t size_need = size + sizeof(q_entry_header);
	if (_remain < size_need)
	{
		log_err "Not enough space left. remain=%u, req size=%u",
			_remain,
			size_need
			log_end;
		return false;
	}

	//
	//	_remain ������ ����
	//
	_ASSERTE(_remain >= size_need);
	_remain -= size_need;

	return true;
}

char* MmQueue::pop_front()
{
	return nullptr;
}

void MmQueue::release(_In_ const char* p)
{
	_ASSERTE(nullptr != p);
	if (nullptr == p) return;

	q_entry* entry = CONTAINING_RECORD(p, q_entry, buf);
	_ASSERTE(true == entry->h.busy);
	if (true != entry->h.busy)
	{
		entry->h.busy = false;
	}
}

void MmQueue::finalize()
{
	if (nullptr != _mmf)
	{
		UnmapViewOfFile(_mmf);
		_mmf = nullptr;
	}

	if (NULL != _map_handle)
	{
		CloseHandle(_map_handle);
		_map_handle = NULL;
	}

	if (INVALID_HANDLE_VALUE == _mmf_handle)
	{
		CloseHandle(_mmf_handle);
		_mmf_handle = INVALID_HANDLE_VALUE;
	}
}

char*
MmQueue::get_ptr(
	_In_ bool read_only,
	_In_ uint64_t Offset,
	_In_ uint32_t Size
	)
{

	return nullptr;
}

void MmQueue::release_ptr(_In_ char* ptr)
{
	_ASSERTE(nullptr != ptr);
	if (nullptr != ptr)
	{
		UnmapViewOfFile(ptr);
	}
}