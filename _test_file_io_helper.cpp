/**
 * @file    _test_file_io_helper.cpp
 * @brief   unit test for FileIoHelper class.
 *
 * This file contains test code for `FileIoHelper` class.
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2017.01.0229 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "StopWatch.h"
#include "FileIoHelperClass.h"



bool 
file_copy_mmio(
	_In_ const wchar_t* src_file, 
	_In_ const wchar_t* dst_file, 
	_In_ const uint32_t block_size
	)
{
	//
	//	�ҽ� ������ �б� ���� ����/�����Ѵ�.
	// 
	FileIoHelper srcFile;
	if (!srcFile.OpenForRead(src_file))
	{
		log_err "OpenForRead() failed." log_end;
		return false;
	}

	std::wstring nt_name;
	std::wstring dos_name;
	if (!srcFile.GetMappedFileName(false, nt_name))
	{
		log_err "srcFile.GetMappedFileName() failed" log_end;
		return false;
	}
	log_info "nt_name=%ws", nt_name.c_str() log_end;

	if (!srcFile.GetMappedFileName(true, dos_name))
	{
		log_err "srcFile.GetMappedFileName() failed" log_end;
		return false;
	}
	log_info "dos_name=%ws", dos_name.c_str() log_end;

	//
	//	������ ������ �ҽ����� ������� �����ϰ� �����Ѵ�.
	// 
	FileIoHelper dstFile;
	if (!dstFile.OpenForWrite(dst_file, srcFile.FileSize()))
	{
		log_err "OpenForWrite() failed." log_end;
		return false;
	}

	// 
	//	�ҽ� ������ ������ ���Ϸ� �����Ѵ�. 
	// 
	bool err = false;

	uint64_t file_size = srcFile.FileSize();
	uint64_t pos = 0;

	while (pos < file_size)
	{
		if (err) break;

		//
		//	min() �� �����ϴ� ���� �׻� uint32_t �������� ���̹Ƿ� 
		//	���� ĳ�����ص� ������.
		//
		uint32_t map_size = (uint32_t)min(block_size, file_size - pos);
		uint8_t* dst_ptr = dstFile.GetFilePointer(false, pos, map_size);

		if (true != srcFile.ReadFromFile(pos, map_size, dst_ptr))
		{
			log_err "srcFile.ReadFromFile() failed. offset=%I64d, size=%u",
				pos,
				map_size
				log_end;
			err = true;
		}

		dstFile.ReleaseFilePointer();

		//
		//	file offset ����
		//
		pos += block_size;
	}
	return !err;
}

/// @brief	test main
bool test_file_io_helper()
{
	const wchar_t* src_file = L"c:\\dbg\\ucrtbased.dll";
	const wchar_t* dst_file = L"c:\\dbg\\ucrtbased.dll.copy";

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	StopWatch sw;
	typedef struct block_size_test
	{
		uint32_t byby;
		uint32_t block_size;
		float total_millisec;
	}*pblock_size_test;

	block_size_test blocks[] = {
		{ 0, 1024, 0},
		{ 1, si.dwAllocationGranularity, 0},
		{ 2, si.dwAllocationGranularity * 2, 0 },
		{ 4, si.dwAllocationGranularity * 4, 0 },
		{ 8, si.dwAllocationGranularity * 8, 0 },
		{ 16, si.dwAllocationGranularity * 16, 0 },
		{ 32, si.dwAllocationGranularity * 32, 0 },
		{ 64, si.dwAllocationGranularity * 64, 0 },
		{ 128, si.dwAllocationGranularity * 128, 0 },
		{ 8, FileIoHelper::GetOptimizedBlockSize(), 0}

	};
			
	for (int x = 0; x < 4; ++x)
	{
		for (int i = 0; i < sizeof(blocks) / sizeof(block_size_test); ++i)
		{
			DeleteFileW(dst_file);

			sw.Start();
			if (!file_copy_mmio(src_file, dst_file, blocks[i].block_size))
			{
				log_err "err..." log_end;
				break;
			}
			sw.Stop();
			blocks[i].total_millisec += sw.GetDurationMilliSecond();
		};
		log_info "iteration %u done.", x log_end;
	}	

	for (int i = 0; i < sizeof(blocks) / sizeof(block_size_test); ++i)
	{
		log_info "block size by=%u, time=%f", blocks[i].byby, blocks[i].total_millisec log_end;
	}
	
	
	// 
	//	����� ���ҽ����� �����Ѵ�.
	// 

	return true;
}

/// @brief	ū �������� MMF ������ �����ϰ�, ���� ��ü�� ����������
///			���μ����� �޸� �������� Ȯ���غ��� ���� �׽�Ʈ
bool test_file_io_helper2()
{
	FileIoHelper mmf;

	//
	//	2GB ¥�� ���� ����
	//

	uint32_t size = (1024 * 1024 * 1024); // 2gb
	_ASSERTE(true == mmf.OpenForWrite(L"c:\\dbg\\mmf_test.dat", size));
	log_info "mmf file creaed." log_end;
	_pause;
	uint8_t* p = mmf.GetFilePointer(false, 0, size);
	log_info "view created." log_end; 
	_pause;

	for (uint32_t i = 0; i < size; ++i)
	{
		p[i] = 'A';
	}

	mmf.ReleaseFilePointer();
	log_info "view relased." log_end;
	_pause;
	mmf.close();
	log_info "mmf relased." log_end;
	_pause;
	return true;
}