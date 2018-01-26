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
	_map_handle(NULL)
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
	//	I/O 에 최적화된 블럭사이즈를 구한다.
	//	정확한 수치는 모르겠지만 SYSTEM_INFO.dwAllocationGranularity(64k) * 8 정도가 
	//	가장 무난한 성능이 나오는것 같음. (win7, win10 에서 테스트)
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
	//	Memory mapped I/O 에 사용할 파일을 생성한다.
	//	이미 파일이 존재한다면 에러처리한다. 
	// 
	if (true == is_file_existsW(file_path))
	{
		log_err "File already exists. path=%ws",
			file_path
			log_end;
		return false;
	}

	//
	//	FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE 옵션을 사용해서
	//	가능한 한 file 로 flush 하지 않도록 하고, handle 을 닫을 때 MMF 파일을
	//	삭제한다.
	//
	_ASSERTE(INVALID_HANDLE_VALUE == _mmf_handle);
	_mmf_handle = CreateFileW(file_path,
							  GENERIC_READ | GENERIC_WRITE,
							  0, // no share
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
	//	_mmf_size 만큼 파일사이즈를 키운다.
	//
	if (true != set_file_size(_mmf_handle, _mmf_size))
	{
		log_err "set_file_size() failed. requested size=%u",
			_mmf_size
			log_end;
		return false;
	}

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

	return true;
}

void MmQueue::finalize()
{
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

char* MmQueue::mmf_alloc(_In_ uint32_t size)
{
	return nullptr;
}

void MmQueue::mmf_free(_In_ char* p)
{
	
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