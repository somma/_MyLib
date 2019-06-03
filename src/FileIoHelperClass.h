/**
 * @file    FileIoHelperClass
 * @brief   
 *
 * This file contains test code for `FileIoHelper` class.
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011.10.13 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once

/// @brief	MMIO 용 유틸리티 클래스.
///			_file_view 포인터는 스레드 안정성을 보장하지 않으므로, 
///			멀티스레드 환경에서 사용하면 안됨
typedef class FileIoHelper
{
private:
	bool _do_not_close_handle;
	bool _read_only;
	HANDLE _file_handle;
	uint64_t _file_size;
	HANDLE _map_handle;
	PUCHAR _file_view;
public:
	FileIoHelper();
	~FileIoHelper();

	static uint32_t GetOptimizedBlockSize();

	bool Initialized()	{ return (INVALID_HANDLE_VALUE != _file_handle) ? true : false;}
	bool IsReadOnly()	{ return (true == _read_only) ? true : false;}	

	bool OpenForRead(_In_ const wchar_t* file_path);
	bool OpenForRead(_In_ const HANDLE file_handle);	
	bool OpenForWrite(_In_ const wchar_t* file_path, _In_ uint64_t file_size);
	bool OpenForReadWrite(_In_ const wchar_t* file_path);
	bool OpenForReadWrite(_In_ const HANDLE file_handle);
	void close();

	bool GetMappedFileName(_In_ bool convet_to_dosname, _Out_ std::wstring& file_path);

	uint8_t* GetFilePointer(_In_ bool read_only, _In_ uint64_t Offset, _In_ uint32_t Size);
	void ReleaseFilePointer();

	bool ReadFromFile(_In_ uint64_t Offset, _In_ DWORD Size, _Inout_updates_bytes_(Size) PUCHAR Buffer);
	bool WriteToFile(_In_ uint64_t Offset, _In_ DWORD Size, _In_reads_bytes_(Size) PUCHAR Buffer);

	uint64_t  FileSize(){ return _file_size; }

	

}*PFileIoHelper;