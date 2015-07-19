/**----------------------------------------------------------------------------
 * FileIoHelperClass.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 13:10:2011   17:04 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"

#include "Win32Utils.h"
#include "FileIoHelperClass.h"

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
FileIoHelper::FileIoHelper()
:	mReadOnly(TRUE), 
	mFileHandle(INVALID_HANDLE_VALUE), 
	mFileMap(NULL), 
	mFileView(NULL)
{
	mFileSize.QuadPart=0;
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
FileIoHelper::~FileIoHelper()
{
	this->FIOClose();
}

/**----------------------------------------------------------------------------
    \brief  파일 IO 를 위해 파일을 오픈한다.
    			
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
DTSTATUS	
FileIoHelper::FIOpenForRead(
	IN std::wstring FilePath
	)	
{
	if (TRUE == Initialized()) { FIOClose(); }

	mReadOnly = TRUE;	
	if (TRUE != is_file_existsW(FilePath.c_str()))
	{
		log_err
			"no file exists. file=%ws", FilePath.c_str()
		log_end
		return DTS_NO_FILE_EXIST;
	}

#pragma warning(disable: 4127)
	DTSTATUS status=DTS_WINAPI_FAILED;
    do 
    {
        mFileHandle = CreateFileW(
                            FilePath.c_str(), 
							GENERIC_READ,
                            NULL,
                            NULL, 
							OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL
                            );
        if (INVALID_HANDLE_VALUE == mFileHandle)
        {
            log_err
                "CreateFile(%ws) failed, gle=0x%08x", 
                FilePath.c_str(), 
                GetLastError()
            log_end
            break;
        }

        // check file size 
        // 
		if (TRUE != GetFileSizeEx(mFileHandle, &mFileSize))
        {
            log_err
                "%ws, can not get file size, gle=0x%08x", 
                FilePath.c_str(), 
                GetLastError() 
            log_end
            break;
        }
        
        mFileMap = CreateFileMapping(
                                mFileHandle, 
                                NULL, 
                                PAGE_READONLY,
                                0, 
                                0, 
                                NULL
                                );
        if (NULL == mFileMap)
        {
            log_err
                "CreateFileMapping(%ws) failed, gle=0x%08x", 
                FilePath.c_str(), 
                GetLastError() 
            log_end
            break;
        }
				
        status = DTS_SUCCESS;
    } while (FALSE);
#pragma warning(default: 4127)

    if (TRUE != DT_SUCCEEDED(status))
    {
        if (INVALID_HANDLE_VALUE!=mFileHandle) 
		{
			CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;
		}
        if (NULL!= mFileMap) CloseHandle(mFileMap);
    }

	return status;
}

/**----------------------------------------------------------------------------
    \brief		FileSize 바이트 짜리 파일을 생성한다.
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
DTSTATUS	
FileIoHelper::FIOCreateFile(
	IN std::wstring FilePath, 
	IN LARGE_INTEGER FileSize
	)
{
	if (TRUE == Initialized()) { FIOClose(); }
	if (FileSize.QuadPart == 0) return DTS_INVALID_PARAMETER;

	mReadOnly = FALSE;	
	
#pragma warning(disable: 4127)
	DTSTATUS status=DTS_WINAPI_FAILED;
    do 
    {
		mFileSize = FileSize;

        mFileHandle = CreateFileW(
                            FilePath.c_str(), 
							GENERIC_READ | GENERIC_WRITE, 
                            FILE_SHARE_READ,		// write 도중 다른 프로세스에서 읽기가 가능
                            NULL, 
							CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL
                            );
        if (INVALID_HANDLE_VALUE == mFileHandle)
        {
            log_err
                "CreateFile(%ws) failed, gle=0x%08x", 
                FilePath.c_str(), 
                GetLastError()
            log_end
            break;
        }

		// increase file size
		// 
		if (TRUE != SetFilePointerEx(mFileHandle, mFileSize, NULL, FILE_BEGIN))
		{
			log_err
				"SetFilePointerEx( move to %I64d ) failed, gle=0x%08x", 
				FileSize.QuadPart, GetLastError()
			log_end
			break;
		}
		
		if (TRUE != SetEndOfFile(mFileHandle))
		{
			log_err "SetEndOfFile( ) failed, gle=0x%08x",  GetLastError() log_end
			break;
		}
        
        mFileMap = CreateFileMapping(
                                mFileHandle, 
                                NULL, 
                                PAGE_READWRITE,
                                0, 
                                0, 
                                NULL
                                );
        if (NULL == mFileMap)
        {
            log_err
                "CreateFileMapping(%ws) failed, gle=0x%08x", 
                FilePath.c_str(), 
                GetLastError() 
            log_end
            break;
        }
				
        status = DTS_SUCCESS;
    } while (FALSE);
#pragma warning(default: 4127)

    if (TRUE != DT_SUCCEEDED(status))
    {
        if (INVALID_HANDLE_VALUE!=mFileHandle) 
		{
			CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;
		}
        if (NULL!= mFileMap) CloseHandle(mFileMap);
    }	

	return status;
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
void		
FileIoHelper::FIOClose(
	)
{
    if (TRUE != Initialized()) return;

    FIOUnreference();
	CloseHandle(mFileMap); mFileMap=NULL;
	CloseHandle(mFileHandle); mFileHandle = INVALID_HANDLE_VALUE;		
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
DTSTATUS	
FileIoHelper::FIOReference(
	IN BOOL ReadOnly, 
	IN LARGE_INTEGER Offset, 
	IN DWORD Size, 
	IN OUT PUCHAR& ReferencedPointer
	)
{
	if (TRUE != Initialized()) return DTS_INVALID_OBJECT_STATUS;
	if (TRUE == IsReadOnly())
	{
		if (TRUE != ReadOnly) 
		{
			log_err "file handle is read-only!" log_end
			return DTS_INVALID_PARAMETER;
		}
	}
	
	_ASSERTE(NULL== mFileView);
	FIOUnreference();

	if (Offset.QuadPart + Size > mFileSize.QuadPart)
	{
		log_err
			"invalid offset. file size=%I64d, req offset=%I64d", 
			mFileSize.QuadPart, Offset.QuadPart
		log_end
		return DTS_INVALID_PARAMETER;
	}

	//
	// MapViewOfFile() 함수의 dwFileOffsetLow 파라미터는 
	// SYSTEM_INFO::dwAllocationGranularity 값의 배수이어야 한다.
	// 
	static DWORD AllocationGranularity=0;
	if(0 == AllocationGranularity)
	{
		SYSTEM_INFO si={0};
		GetSystemInfo(&si);
		AllocationGranularity = si.dwAllocationGranularity;
	}

	DWORD AdjustMask = AllocationGranularity -1;
	LARGE_INTEGER AdjustOffset={0};
	AdjustOffset.HighPart = Offset.HighPart;
	
	// AllocationGranularity 이하의 값을 버림
	// 
	AdjustOffset.LowPart = (Offset.LowPart & ~AdjustMask); 
	
	// 버려진 값만큼 매핑할 사이즈를 증가
	// 
	DWORD BytesToMap = (Offset.LowPart & AdjustMask) + Size;

	mFileView = (PUCHAR) MapViewOfFile(
								mFileMap, 
								(TRUE == ReadOnly) ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE, 
								AdjustOffset.HighPart, 
								AdjustOffset.LowPart, 
								BytesToMap
								);
	if (NULL == mFileView)
	{
		log_err
			"MapViewOfFile(high=0x%08x, log=0x%08x, bytes to map=%u) failed, gle=0x%08x", 
			AdjustOffset.HighPart, AdjustOffset.LowPart, BytesToMap, GetLastError()
		log_end
		return DTS_WINAPI_FAILED;
	}
	
	ReferencedPointer = &mFileView[Offset.LowPart & AdjustMask];
	return DTS_SUCCESS;
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
void		
FileIoHelper::FIOUnreference(
	)
{
	if (NULL != mFileView)
	{
		UnmapViewOfFile(mFileView);
		mFileView=NULL;
	}
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
DTSTATUS	
FileIoHelper::FIOReadFromFile(
	IN LARGE_INTEGER Offset, 
	IN DWORD Size, 
	IN OUT PUCHAR Buffer
	)
{
	_ASSERTE(NULL != Buffer);
	if (NULL == Buffer) return DTS_INVALID_PARAMETER;

	UCHAR* p=NULL;
	DTSTATUS status = FIOReference(TRUE, Offset, Size, p);
	if(TRUE != DT_SUCCEEDED(status))
	{
		log_err
			"FIOReference() failed. status=0x%08x", 
			status
		log_end
		return status;
	}

	__try
	{
		RtlCopyMemory(Buffer, p, Size);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		log_err
			"exception. code=0x%08x", GetExceptionCode()
		log_end		
		status = DTS_EXCEPTION_RAISED;
	}

	FIOUnreference();
	return status;
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
DTSTATUS	
FileIoHelper::FIOWriteToFile(
	IN LARGE_INTEGER Offset, 
	IN DWORD Size, 
	IN PUCHAR Buffer
	)
{
	_ASSERTE(NULL != Buffer);
	_ASSERTE(0 != Size);
	_ASSERTE(NULL != Buffer);
	if (NULL == Buffer || 0 == Size || NULL == Buffer) return DTS_INVALID_PARAMETER;

	UCHAR* p=NULL;
	DTSTATUS status = FIOReference(FALSE, Offset, Size, p);
	if(TRUE != DT_SUCCEEDED(status))
	{
		log_err
			"FIOReference() failed. status=0x%08x", 
			status
		log_end
		return status;
	}

	__try
	{
		RtlCopyMemory(p, Buffer, Size);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		log_err
			"exception. code=0x%08x", GetExceptionCode()
		log_end		
		status = DTS_EXCEPTION_RAISED;
	}

	FIOUnreference();
	return status;
}