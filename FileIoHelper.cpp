/*-----------------------------------------------------------------------------
 * FileIoHelper.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * - 01.09.2010 created
**---------------------------------------------------------------------------*/

#include "stdafx.h"
#include <crtdbg.h>

#include "DebugMessage.h"
#include "Win32Utils.h"

#include "FileIoHelper.h"



/**----------------------------------------------------------------------------
    \brief  

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
DTSTATUS OpenFileContext(IN PCWSTR FilePath, IN OUT PFILE_CTX& Ctx)
{
    _ASSERTE(NULL != FilePath);
    if (NULL == FilePath) return DTS_INVALID_PARAMETER;
    if (TRUE != IsFileExistW(FilePath)) return DTS_INVALID_PARAMETER;

	_ASSERT(NULL == Ctx);
	if (NULL != Ctx) return DTS_INVALID_PARAMETER;

    DTSTATUS status = DTS_WINAPI_FAILED;
	Ctx = (PFILE_CTX) malloc(sizeof(FILE_CTX));
	if (NULL == Ctx) return DTS_INSUFFICIENT_RESOURCES;

    RtlZeroMemory(Ctx, sizeof(FILE_CTX));

#pragma warning(disable: 4127)
    do 
    {
        Ctx->FileHandle = CreateFileW(
                            (LPCWSTR)FilePath, 
                            GENERIC_READ, 
                            NULL,
                            NULL, 
                            OPEN_EXISTING, 
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL
                            );
        if (INVALID_HANDLE_VALUE == Ctx->FileHandle)
        {
            DBG_OPN 
                "[ERR ]", 
                "CreateFile(%ws) failed, gle=0x%08x", 
                FilePath, 
                GetLastError()
            DBG_END
            break;
        }

        // check file size 
        // 
        LARGE_INTEGER fileSize;
        if (TRUE != GetFileSizeEx(Ctx->FileHandle, &fileSize))
        {
            DBG_OPN
                "[ERR ]", 
                "%ws, can not get file size, gle=0x%08x", 
                FilePath, 
                GetLastError() 
            DBG_END
            break;
        }

        // [ WARN ]
        // 
        // 4Gb 이상의 파일의 경우 MapViewOfFile()에서 오류가 나거나 
        // 파일 포인터 이동이 문제가 됨
        // FilIoHelperClass 모듈을 이용해야 함
        // 
        _ASSERTE(fileSize.HighPart == 0);
		if (fileSize.HighPart > 0) 
		{
			DBG_ERR
				"file size = %I64d (over 4GB) can not handle. use FileIoHelperClass",
				fileSize.QuadPart
			DBG_END
			break;
		}

        Ctx->FileSize = (DWORD)fileSize.QuadPart;
        Ctx->FileMap = CreateFileMapping(
                                Ctx->FileHandle, 
                                NULL, 
                                PAGE_READONLY, 
                                0, 
                                0, 
                                NULL
                                );
        if (NULL == Ctx->FileMap)
        {
            DBG_OPN
                "[ERR ]", 
                "CreateFileMapping(%ws) failed, gle=0x%08x", 
                FilePath, 
                GetLastError() 
            DBG_END
            break;
        }

        Ctx->FileView = (PCHAR) MapViewOfFile(
                                        Ctx->FileMap, 
                                        FILE_MAP_READ, 
                                        0, 
                                        0, 
                                        0
                                        );
        if(Ctx->FileView == NULL)
        {
            DBG_OPN
                "[ERR ]", 
                "MapViewOfFile(%ws) failed, gle=0x%08x", 
                FilePath, 
                GetLastError() 
            DBG_END
            break;
        }    

        status = DTS_SUCCESS;
    } while (FALSE);
#pragma warning(default: 4127)

    if (TRUE != DT_SUCCEEDED(status))
    {
        if (INVALID_HANDLE_VALUE != Ctx->FileHandle) CloseHandle(Ctx->FileHandle);
        if (NULL!= Ctx->FileMap) CloseHandle(Ctx->FileMap);
        if (NULL!= Ctx->FileView) UnmapViewOfFile(Ctx->FileView);
        free(Ctx);
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
void CloseFileContext(IN PFILE_CTX& Ctx)
{
	_ASSERTE(NULL != Ctx);
	if (NULL == Ctx) return;

    if (INVALID_HANDLE_VALUE!= Ctx->FileHandle) CloseHandle(Ctx->FileHandle);
    if (NULL!= Ctx->FileMap) CloseHandle(Ctx->FileMap);
    if (NULL!= Ctx->FileView) UnmapViewOfFile(Ctx->FileView);
    free(Ctx);
}