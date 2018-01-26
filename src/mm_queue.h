/**
 * @file    mm_queue.h
 * @brief   Queue implementation using Memory Mapped File
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2018.01.25 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <sal.h>
#include <stdint.h>


typedef class MmQueue
{
public:
	MmQueue();
	virtual ~MmQueue();

	bool initialize(_In_ uint32_t size, _In_ const wchar_t* file_path);
	void finalize();

	char* mmf_alloc(_In_ uint32_t size);
	void mmf_free(_In_ char* p);

private:
	char* get_ptr(_In_ bool read_only, _In_ uint64_t Offset, _In_ uint32_t Size);
	void release_ptr(_In_ char* ptr);

private:
	uint32_t _optimized_io_size;
	uint32_t _mmf_size;
	HANDLE _mmf_handle;
	HANDLE _map_handle;


} *PMmQueue;
