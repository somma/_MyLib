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
#include "stdafx.h"

#include "Win32Utils.h"
#include "FileIoHelperClass.h"

FileIoHelper::FileIoHelper()
:	mReadOnly(TRUE), 
	mFileHandle(INVALID_HANDLE_VALUE), 
	mFileSize(0),
	mFileMap(NULL), 
	mFileView(NULL)
{
}

FileIoHelper::~FileIoHelper()
{
	this->close();
}

/// @brief	I/O 에 최적화된 블럭사이즈를 리턴한다. 
///			정확한 수치는 모르겠지만 SYSTEM_INFO.dwAllocationGranularity(64k) * 8 정도가 가장 
///			무난한 성능이 나오는것 같음. (win7, win10 에서 테스트)
//static
uint32_t FileIoHelper::GetOptimizedBlockSize()
{
	static DWORD AllocationGranularity = 0;
	if (0 == AllocationGranularity)
	{
		SYSTEM_INFO si = { 0 };
		GetSystemInfo(&si);
		AllocationGranularity = si.dwAllocationGranularity;
	}

	if (0 == AllocationGranularity)
	{
		return (64 * 1024) * 8;
	}
	else
	{
		return AllocationGranularity * 8;
	}
}

/// @brief	파일을 읽기모드로 오픈한다. 
bool FileIoHelper::OpenForRead(_In_ const wchar_t* file_path)
{
	_ASSERTE(nullptr != file_path);
	if (nullptr == file_path) return false;

	if (TRUE != is_file_existsW(file_path))
	{
		log_err "no file exists. file=%ws",
			file_path
			log_end;
		return false;
	}
	
	if (TRUE == Initialized()) { close(); }
	
	HANDLE file_handle = CreateFileW(file_path,
									 GENERIC_READ,
									 NULL,
									 NULL,
									 OPEN_EXISTING,
									 FILE_ATTRIBUTE_NORMAL,
									 NULL);
	if (INVALID_HANDLE_VALUE == file_handle)
	{
		log_err
			"CreateFile() failed, file=%ws, gle=%u",
			file_path,
			GetLastError()
			log_end
			return false;
	}

	if (true != OpenForRead(file_handle))
	{
		CloseHandle(file_handle);		
		return false;
	}

	_ASSERTE(mFileHandle == file_handle);
	_ASSERTE(mReadOnly == TRUE);
	_ASSERTE(mFileMap != NULL);
	return true;
}

/// @brief	파일을 읽기모드로 오픈한다. 
bool 
FileIoHelper::OpenForRead(
	_In_ const HANDLE file_handle
	)
{
	_ASSERTE(INVALID_HANDLE_VALUE != file_handle);	
	if (INVALID_HANDLE_VALUE == file_handle)
	{
		return false;
	}

	if (TRUE == Initialized())
	{
		close();
	}
		
	uint64_t file_size = 0;
	HANDLE file_map = NULL;

#pragma warning(disable: 4127)
	bool ret = false;
	do
	{
		// check file size 
		// 
		if (TRUE != GetFileSizeEx(file_handle, (PLARGE_INTEGER)&file_size))
		{
			log_err
				"GetFileSizeEx() failed. file_handle=0x%p, gle=%u",
				file_handle,
				GetLastError()
				log_end
				break;
		}

		if (file_size == 0)
		{
			log_err "Empty file specified." log_end;
			break;
		}
		
		file_map = CreateFileMapping(file_handle,
									 NULL,
									 PAGE_READONLY,
									 0,
									 0,
									 NULL);
		if (NULL == file_map)
		{
			log_err
				"CreateFileMapping() failed, file_handle=0x%p, gle=%u",
				file_handle,
				GetLastError()
				log_end
				break;
		}

		ret = true;
	} while (FALSE);
#pragma warning(default: 4127)

	if (true != ret)
	{
		if (NULL != file_map)
		{
			CloseHandle(file_map);			
		}
	}
	else
	{
		mReadOnly = TRUE;
		mFileSize = file_size;
		mFileHandle = file_handle;
		mFileMap = file_map;
	}
	return ret;
}

/// @brief	file_size 바이트 짜리 파일을 생성한다.
bool 
FileIoHelper::OpenForWrite(
	_In_ const wchar_t* file_path, 
	_In_ uint64_t file_size
	)
{
	if (TRUE == Initialized()) { close(); }
	if (file_size == 0) return false;

	mReadOnly = FALSE;	
	
#pragma warning(disable: 4127)
	bool ret = false;
    do 
    {
		mFileSize = file_size;

        mFileHandle = CreateFileW(file_path,
								  GENERIC_READ | GENERIC_WRITE, 
								  FILE_SHARE_READ,		// write 도중 다른 프로세스에서 읽기가 가능
								  NULL, 
								  CREATE_ALWAYS,
								  FILE_ATTRIBUTE_NORMAL, 
								  NULL);
        if (INVALID_HANDLE_VALUE == mFileHandle)
        {
            log_err
                "CreateFile() failed, file=%ws, gle=%u", 
                file_path,
                GetLastError()
            log_end
            break;
        }

		//
		//	요청된 크기만큼 파일사이즈를 늘린다.
		// 
		if (TRUE != SetFilePointerEx(mFileHandle, 
									 *(PLARGE_INTEGER)&mFileSize, 
									 NULL, 
									 FILE_BEGIN))
		{
			log_err
				"SetFilePointerEx() failed, file=%ws, size=%I64d, gle=%u", 
				file_path, 
				file_size,
				GetLastError()				
			log_end
			break;
		}
		
		if (TRUE != SetEndOfFile(mFileHandle))
		{
			log_err "SetEndOfFile() failed, file=%ws, gle=%u", 
				file_path,
				GetLastError() 
				log_end
			break;
		}
        
        mFileMap = CreateFileMapping(mFileHandle, 
									 NULL, 
									 PAGE_READWRITE,
									 0, 
									 0, 
									 NULL);
        if (NULL == mFileMap)
        {
            log_err
                "CreateFileMapping() failed, file=%ws, gle=%u", 
                file_path,
                GetLastError() 
            log_end
            break;
        }
				
		ret = true;
    } while (FALSE);
#pragma warning(default: 4127)

    if (true != ret)
    {
        if (INVALID_HANDLE_VALUE!=mFileHandle) 
		{
			CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;
		}
        if (NULL!= mFileMap) CloseHandle(mFileMap);
    }	

	return ret;
}

/// @brief	파일을 읽기/쓰기모드로 오픈한다. 
bool FileIoHelper::OpenForReadWrite(_In_ const wchar_t* file_path)
{
	_ASSERTE(nullptr != file_path);
	if (nullptr == file_path) return false;

	if (TRUE != is_file_existsW(file_path))
	{
		log_err "no file exists. file=%ws",
			file_path
			log_end;
		return false;
	}
	
	if (TRUE == Initialized()) { close(); }
	
	HANDLE file_handle = CreateFileW(file_path,
									 GENERIC_READ | GENERIC_WRITE,
									 NULL,
									 NULL,
									 OPEN_EXISTING,
									 FILE_ATTRIBUTE_NORMAL,
									 NULL);
	if (INVALID_HANDLE_VALUE == file_handle)
	{
		log_err
			"CreateFile() failed, file=%ws, gle=%u",
			file_path,
			GetLastError()
			log_end
			return false;
	}

	if (true != OpenForReadWrite(file_handle))
	{
		CloseHandle(file_handle);		
		return false;
	}

	_ASSERTE(mFileHandle == file_handle);
	_ASSERTE(mReadOnly == false);
	_ASSERTE(mFileMap != NULL);
	return true;
}

/// @brief	파일을 읽기/쓰기모드로 오픈한다. 
bool 
FileIoHelper::OpenForReadWrite(
	_In_ const HANDLE file_handle
	)
{
	_ASSERTE(INVALID_HANDLE_VALUE != file_handle);	
	if (INVALID_HANDLE_VALUE == file_handle)
	{
		return false;
	}

	if (TRUE == Initialized()) { close(); }
		
	uint64_t file_size = 0;
	HANDLE file_map = NULL;

#pragma warning(disable: 4127)
	bool ret = false;
	do
	{
		// check file size 
		// 
		if (TRUE != GetFileSizeEx(file_handle, (PLARGE_INTEGER)&file_size))
		{
			log_err
				"GetFileSizeEx() failed. file_handle=0x%p, gle=%u",
				file_handle,
				GetLastError()
				log_end
				break;
		}

		if (file_size == 0)
		{
			log_err "Empty file specified." log_end;
			break;
		}
		
		file_map = CreateFileMapping(file_handle,
									 NULL,
									 PAGE_READWRITE,
									 0,
									 0,
									 NULL);
		if (NULL == file_map)
		{
			log_err
				"CreateFileMapping() failed, file_handle=0x%p, gle=%u",
				file_handle,
				GetLastError()
				log_end
				break;
		}

		ret = true;
	} while (FALSE);
#pragma warning(default: 4127)

	if (true != ret)
	{
		if (NULL != file_map)
		{
			CloseHandle(file_map);			
		}
	}
	else
	{
		mReadOnly = false;
		mFileSize = file_size;
		mFileHandle = file_handle;
		mFileMap = file_map;
	}
	return ret;
}

/// @brief	모든 리소스를 제거한다.
void FileIoHelper::close()
{
    if (TRUE != Initialized()) return;

    ReleaseFilePointer();
	if (NULL != mFileMap)
	{
		CloseHandle(mFileMap); 
		mFileMap = NULL;
	}

	if (INVALID_HANDLE_VALUE != mFileHandle)
	{
		CloseHandle(mFileHandle); 
		mFileHandle = INVALID_HANDLE_VALUE;
	}
}

/// @beief	메모리 매핑된 파일의 경로를 구한다. 
///			convert_to_dosname == false 인 경우 
///				\Device\harddiskVolume1\Windows\System32\notepad.exe 경로 리턴
///
///			convert_to_dosname == true 인 경우 디바이스 네임을 볼륨명으로 변경
///				c:\Windows\System32\notepad.exe 경로 리턴
bool FileIoHelper::GetMappedFileName(_In_ bool convet_to_dosname, _Out_ std::wstring& file_path)
{
	if (TRUE != Initialized()) return false;
	if (NULL != mFileView)
	{
		log_err "mapped file pointer is busy. " log_end;
		return false;
	}

	uint8_t* ptr = GetFilePointer(true, 0, 8);
	if (NULL == ptr)
	{
		log_err "GetFilePointer() failed." log_end;
		return false;
	}


	bool ret = false;
	do
	{
		std::wstring nt_file_name;
		if (true != get_mapped_file_name(GetCurrentProcess(), ptr, nt_file_name))
		{
			log_err "get_mapped_file_name() failed." log_end;
			break;
		}

		if (true == convet_to_dosname)
		{
			std::wstring dos_file_name;
			if (!nt_name_to_dos_name(nt_file_name.c_str(), dos_file_name))
			{
				log_err "nt_name_to_dos_name() failed. nt_name=%ws",
					nt_file_name.c_str()
					log_end;
				break;
			}

			file_path = dos_file_name;
		}
		else
		{
			file_path = nt_file_name;
		}

		ret = true;
	} while (false);

	ReleaseFilePointer();
	return ret;

}

/// @brief	지정된 파일의 Offset 위치를 Size 만큼 매핑하고, 해당 메모리 참조를 리턴한다.
///			Offset 은 SYSTEM_INFO.dwAllocationGranularity 의 배수로 지정해야 한다. 
///			그렇지 않은 경우 자동으로 SYSTEM_INFO.dwAllocationGranularity 값으로 조정해서
///			파일을 매핑하고, pointer 를 적당히 보정해서 리턴한다.
uint8_t* 
FileIoHelper::GetFilePointer(
	_In_ bool read_only, 
	_In_ uint64_t Offset, 
	_In_ uint32_t Size
	)
{
	_ASSERTE(NULL == mFileView);
	if (NULL != mFileView)
	{
		log_err "ReleaseFilePointer() first!" log_end;
		return NULL;
	}

	if (TRUE != Initialized()) return false;
	if (IsReadOnly() && !read_only)
	{
		log_err "file mapped read only." log_end;
		return NULL;
	}

	// 
	//	요청한 offset 이 파일 사이즈보다 크다면 오류를 리턴한다.
	// 
	if (Offset >= mFileSize)
	{
		log_err "Req offset > File size. req offset=0x%I64x, file size=0x%I64x",
			Offset,
			mFileSize
			log_end;
		return NULL;
	}

	//
	//	요청한 사이즈가 파일사이즈보다 크다면 파일사이즈만큼만 매핑한다. 
	//
	uint32_t adjusted_size = Size;
	if (Offset + Size > mFileSize)
	{
		adjusted_size = (uint32_t)(mFileSize - Offset);
	}
	
	//
	//	MapViewOfFile() 함수의 dwFileOffsetLow 파라미터는 
	//	SYSTEM_INFO::dwAllocationGranularity 값의 배수이어야 한다.
	//	혹시라도 오류가 나면 64k 로 설정한다. 
	// 
	static DWORD AllocationGranularity = 0;
	if (0 == AllocationGranularity)
	{
		SYSTEM_INFO si = { 0 };
		GetSystemInfo(&si);
		AllocationGranularity = si.dwAllocationGranularity;
	}
	
	_ASSERTE(0 != AllocationGranularity);
	if (0 == AllocationGranularity)
	{
		AllocationGranularity = (64 * 1024);		
	}

	//
	//	AllocationGranularity 이하의 값을 버린다. 
	//	결국 매핑해야 할 사이즈는 버려진 사이즈 만큼 커져야 한다.
	// 	
	uint64_t AdjustMask = (uint64_t)(AllocationGranularity - 1);
	uint64_t adjusted_offset = Offset & ~AdjustMask;
	adjusted_size = (DWORD)(Offset & AdjustMask) + adjusted_size;

	mFileView = (PUCHAR)MapViewOfFile(mFileMap,
								      (TRUE == read_only) ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE,
									  ((PLARGE_INTEGER)&adjusted_offset)->HighPart,
									  ((PLARGE_INTEGER)&adjusted_offset)->LowPart,
									  adjusted_size);
	if (NULL == mFileView)
	{
		log_err
			"MapViewOfFile(high=0x%08x, low=0x%08x, bytes to map=%u) failed, gle=%u",
			((PLARGE_INTEGER)&adjusted_offset)->HighPart,
			((PLARGE_INTEGER)&adjusted_offset)->LowPart,
			adjusted_size,
			GetLastError()
			log_end;
		return NULL;
	}

	//
	//	매핑은 adjust offset 으로 하지만 리턴하는 메모리 포인터는 
	//	요청한 대로 리턴해주어야 한다.
	// 
	return &mFileView[Offset & AdjustMask];
}


/// @brief	매핑된 파일포인터를 릴리즈한다. 
void FileIoHelper::ReleaseFilePointer()
{
	if (NULL != mFileView)
	{
		UnmapViewOfFile(mFileView);
		mFileView = NULL;
	}
}

/// @brief	파일의 Offset 에서 Size 만큼 읽어서 Buffer 에 리턴한다.
bool 
FileIoHelper::ReadFromFile(
	_In_ uint64_t Offset, 
	_In_ DWORD Size, 
	_Inout_updates_bytes_(Size) PUCHAR Buffer
	)
{
	_ASSERTE(NULL != Buffer);
	if (NULL == Buffer) return false;

	uint8_t* src_ptr = GetFilePointer(true, Offset, Size);
	if (NULL == src_ptr)
	{
		log_err "GetFilePointer() failed. offset=0x%I64d, size=%u",
			Offset,
			Size
			log_end;
		return false;
	}

	bool ret = false;
	__try
	{
		RtlCopyMemory(Buffer, src_ptr, Size);
		ret = true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		log_err
			"exception. offset=0x%I64d, size=%u, code=0x%08x",
			Offset,
			Size,
			GetExceptionCode()
		log_end		
	}

	ReleaseFilePointer();
	return ret;
	
}

/// @brief	Buffer 를 파일의 Offset 에 Size 만큼 쓴다.
bool 
FileIoHelper::WriteToFile(
	_In_ uint64_t Offset, 
	_In_ DWORD Size, 
	_In_reads_bytes_(Size) PUCHAR Buffer
	)
{
	_ASSERTE(NULL != Buffer);
	_ASSERTE(0 != Size);
	_ASSERTE(NULL != Buffer);
	if (NULL == Buffer || 0 == Size || NULL == Buffer) return false;

	uint8_t* dst_ptr = GetFilePointer(false, Offset, Size);
	if (NULL == dst_ptr)
	{
		log_err "GetFilePointer() failed. offset=0x%I64d, size=%u",
			Offset,
			Size
			log_end;
		return false;
	}

	bool ret = false;
	__try
	{
		RtlCopyMemory(dst_ptr, Buffer, Size);
		ret = true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		log_err
			"exception. offset=0x%I64d, size=%u, code=0x%08x",
			Offset,
			Size,
			GetExceptionCode()
			log_end
	}

	ReleaseFilePointer();
	return ret;
}
