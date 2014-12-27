/**----------------------------------------------------------------------------
 * Win32Utils.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 26:8:2011   15:34 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include <errno.h>

#include <io.h>			// _setmode()
#include <fcntl.h>		// _O_U8TEXT, ...

#include "Win32Utils.h"

#include <time.h>
#include <Shellapi.h>
#include <Shlobj.h>
#pragma comment(lib, "Shell32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <Psapi.h>
#pragma comment(lib, "psapi.lib")

#include "ResourceHelper.h"



/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
LPCWSTR FT2Str(IN FILETIME& ft)
{
    static WCHAR	_time[32] = {0x00, };
	SYSTEMTIME		st, stLocal;

	if (0 != ft.dwHighDateTime || 0 != ft.dwLowDateTime)
	{
		FileTimeToSystemTime(&ft, &st);
		SystemTimeToTzSpecificLocalTime(NULL, &st, &stLocal);

		StringCchPrintfW(
            _time, 
            RTL_NUMBER_OF(_time), 
            TEXT("%04d-%02d-%02d %02d:%02d:%02d"), 
            stLocal.wYear, stLocal.wMonth, stLocal.wDay, 
            stLocal.wHour, stLocal.wMinute, stLocal.wSecond
            );
	}
	else
	{
		StringCchPrintfW(_time, RTL_NUMBER_OF(_time), TEXT("N/A"));
	}

	return _time;
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
LPCWSTR FAT2Str(IN FATTIME& fat)
{
    FILETIME	ft={0};
	
	if (0x00 != fat.usDate && 0x00 != fat.usTime)
	{
		DosDateTimeToFileTime(fat.usDate, fat.usTime, &ft);
	}

	return FT2Str(ft);
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool is_file_existsW(_In_ const wchar_t* file_path)
{
	_ASSERTE(NULL != file_path);
	_ASSERTE(TRUE != IsBadStringPtrW(file_path, MAX_PATH));
	if ((NULL == file_path) || (TRUE == IsBadStringPtrW(file_path, MAX_PATH))) return false;

	WIN32_FILE_ATTRIBUTE_DATA info = {0};

	//
	// CreateFile()이 아닌 GetFileAttributesEx()를 이용하면 파일이 다른 process에 의해 lock되어 있어도
	// 파일 존재여부를 정확히 체크할 수 있다.
	//
	if (GetFileAttributesExW(file_path, GetFileExInfoStandard, &info)==0) 
		return false;
	else
		return true;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool is_file_existsA(_In_ const char* file_path)
{
    WCHAR* wcs=MbsToWcs(file_path);
    if(NULL==wcs) { return false;}

	bool ret = is_file_existsW(wcs); free(wcs);
	return ret;
}

/**
 * @brief	file_handle 로 file path 를 구하는 함수
 * @param	
 * @see		http://msdn.microsoft.com/en-us/library/windows/desktop/aa366789(v=vs.85).aspx
 * @remarks	NtQueryObject() 를 사용하는게 더 좋을 것 같기도 함, 권한 문제가 발생 할 수도 있을것 같으나
 * @remarks 확인해본적 없음
 * @code		
 * @endcode	
 * @return	
**/
bool 
get_filepath_by_handle(
	_In_ HANDLE file_handle, 
	_Out_ std::wstring& file_path
	)
{
	_ASSERTE(NULL != file_handle);
	_ASSERTE(INVALID_HANDLE_VALUE != file_handle);	
	if (NULL == file_handle || INVALID_HANDLE_VALUE == file_handle) return false;
	
	LARGE_INTEGER file_size = {0};
	if (TRUE != GetFileSizeEx(file_handle, &file_size))
	{
		log_err
			"GetFileSize( file handle = 0x%08x ), gle = %u", 
			file_handle, 
			GetLastError()
		log_end
		return false;
	}

	if (0 == file_size.QuadPart)
	{
		log_err "zero length file" log_end
		return false;
	}
	
	raii_handle map_handle(
					CreateFileMapping(file_handle, NULL, PAGE_READONLY, 0, 1, NULL), 
					raii_CloseHandle
					);
	if (NULL == map_handle.get())
	{
		log_err "CreateFileMapping(), gle = %u", GetLastError() log_end
		return false;
	}

	raii_void_ptr map_ptr(
					MapViewOfFile(map_handle.get(), FILE_MAP_READ, 0, 0, 1), 
					raii_UnmapViewOfFile
					);
	if (NULL == map_ptr.get())
	{
		log_err "MapViewOfFile(), gle = %u", GetLastError() log_end
		return false;
	}

	std::wstring nt_device_name;
	if (true != get_mapped_file_name(
					GetCurrentProcess(), 
					map_ptr.get(), 
					nt_device_name))
	{
		log_err "get_mapped_file_name()" log_end
		return false;
	}

	if (true != nt_name_to_dos_name(nt_device_name.c_str(), file_path))
	{
		log_err "nt_name_to_dos_name( nt name = %s )", nt_device_name.c_str() log_end
		return false;
	}
	
	return true;
}

/**
 * @brief	wrapper function for GetMappedFileName() 
 * @param	
 * @see		http://msdn.microsoft.com/en-us/library/windows/desktop/ms683195(v=vs.85).aspx
 * @remarks	
 * @code		
 * @endcode	
 * @return	true if succeeded.
 * @return	file_name is nt device name (e.g. Device\HarddiskVolume2\Windows\System32\drivers\etc\hosts)
 * @return	if you want use dos device name, use nt_name_to_dos_name() function.
**/
bool 
get_mapped_file_name(
	_In_ HANDLE process_handle, 
	_In_ const void* mapped_addr, 
	_Out_ std::wstring& file_name
	)
{
	bool		ret = false;
	DWORD		ret_cch_buf = 0;
	DWORD		cch_buf = MAX_PATH;
	wchar_t*	buf = NULL; 
	
	for(;;)
	{
		if (NULL != buf) free(buf);

		buf = (wchar_t*) malloc((cch_buf + 1) * sizeof(wchar_t));	// add NULL 
		if (NULL == buf) 
		{
			log_err 
				"insufficient memory, malloc( %u )", 
				(cch_buf + 1) * sizeof(wchar_t) 
			log_end
			return false;
		}

		ret_cch_buf = GetMappedFileNameW(
							process_handle, 
							const_cast<void*>(mapped_addr), 
							buf, 
							cch_buf
							);
		if (0 == ret_cch_buf)
		{
			log_err 
				"GetMappedFileNameW( process handle = 0x%08x, addr = 0x%p ), gle = %u",
				process_handle, 
				mapped_addr, 
				GetLastError()
			log_end
			
			break;
		}

		if (ret_cch_buf < cch_buf)
		{
			// OK!
			ret = true;
			buf[ret_cch_buf] = L'\0';
			break;
		}
		else if (ret_cch_buf == cch_buf)
		{
			// we need more buffer
			cch_buf *= 2;
			continue;
		}
		else
		{
			log_err 
				"unexpected ret_cch_buf(%u) : cch_buf(%u), GetMappedFileNameW()",
				ret_cch_buf,
				cch_buf
			log_end
			break;
		}
	}

	if(true == ret) file_name = buf;
	
	free(buf); buf = NULL;
	return ret;
}

/**
 * @brief	convert nt name -> dos name
 * @param	nt_name		ex) \Device\HarddiskVolume2\Windows\System32\drivers\etc\hosts
 * @param	dos_name	ex) c:\Windows\System32\drivers\etc\hosts
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	true if succeeded, dos_name string is always lower case.
**/
bool 
nt_name_to_dos_name(
	_In_ const wchar_t* nt_name, 
	_Out_ std::wstring& dos_name
	)
{
	_ASSERTE(NULL != nt_name);
	if (NULL == nt_name) return false;
	
	bool ret = false;

	// 0               4               8               12
	// +---+---+---+---+---+---+---+---+---+---+---+---+
	// | A | : | \ |NUL| C | : | \ |NUL| D | : | \ |NUL|
	//  "A:\"           "C:\"           "D:\"
	// 
	// 매핑된 드라이브를 나타내는 문자열 버퍼는 
	// 26 (알파벳) * 4 (드라이브 명) = 104 바이트면 충분함
	// 유니코드인 경우 208바이트 	
	wchar_t drive_string[128 + 1] = {0};
	DWORD length = GetLogicalDriveStringsW(128, drive_string);
	if ( 0 == length)
	{
		log_err 
			"GetLogicalDriveStringsW(), gle = %u", GetLastError()
		log_end
		return false;
	}

	std::wstring nt_namel(nt_name);
	to_lower_string(nt_namel);			// 소문자로 모두 변환
	
	for (DWORD i = 0; i < length / 4; ++i)
	{
		// C:  --> \Device\HarddiskVolume1 매핑 정보를 조회 
		// QueryDosDeviceW() 호출 시 drive name 에 역슬래시 '\' 없어야 함		
		wchar_t* dos_device_name = &(drive_string[i * 4]);
		dos_device_name[2] = 0x0000;

		std::wstring nt_device;	
		if (true != query_dos_device(dos_device_name, nt_device))
		{
			log_err 
				"query_dos_device( dos_device_name = %s )", 
				dos_device_name
			log_end
			return false;
		}
		to_lower_string(nt_device);

		// nt_name 의 device_name 부분이 일치하는지 비교
		// 
		// nt_device= \\device\\harddiskvolume1
		// nt_namel = \\device\\harddiskvolume1\\windows\\system32\\drivers
		//            -------------------------
		// dos_device_name=                   c:
		// dos_name =                         c:\\windows\\system32\\drivers
		size_t pos = nt_namel.find(nt_device);
		if (std::wstring::npos == pos)
		{
			continue;
		}
		else
		{
			// we found!
			dos_name = dos_device_name;	
			dos_name += nt_namel.substr(pos + nt_device.size(), nt_namel.size());
			
			ret = true;
			break;
		}
	}

	return ret;
}

/**
 * @brief	wrapper function fro QueryDosDevice()
 * @param	dos_device	e.g. c:, d:  (no back slash)
 * @see		http://msdn.microsoft.com/en-us/library/windows/desktop/aa365461(v=vs.85).aspx
 * @remarks	
 * @code		
 * @endcode	
 * @return	true if succeeded. nt_device = \Device\HarddiskVolume1
**/
bool query_dos_device(_In_ const wchar_t* dos_device, _Out_ std::wstring& nt_device)
{
	_ASSERTE(NULL != dos_device);
	if (NULL == dos_device) return false;

	bool		ret = false;
	DWORD		cch_buf = MAX_PATH;
	wchar_t*	buf = NULL;
	for(;;)
	{
		if (NULL != buf) free(buf);

		buf = (wchar_t*) malloc( (cch_buf+1) * sizeof(wchar_t) );
		if (NULL == buf)
		{
			log_err
				"insufficient memory, malloc( %u )", 
				(cch_buf + 1) * sizeof(wchar_t)
			log_end
			return false;
		}
		
		DWORD cch_ret = QueryDosDevice(dos_device, buf, cch_buf);
		if (0 == cch_ret)
		{
			DWORD gle = GetLastError();
			if(ERROR_INSUFFICIENT_BUFFER != gle)
			{
				log_err 
					"QueryDosDevice( dos = %s ), gle = %u", 
					dos_device, 
					GetLastError()
				log_end
				break;
			}

			// we need more buffer
			cch_buf *= 2;
			continue;
		} 
		else if (cch_ret <= cch_buf)
		{
			// success, add null and break the loop
			buf[cch_ret] = L'\0';
			ret = true;
			break;
		}
		else
		{
			// opps! unknown error 
			log_err 
				"unexpected cch_ret(%u) : cch_buf(%u), QueryDosDevice( %s )",
				cch_ret,
				cch_buf, 
				dos_device
			log_end
			break;
		}
	}
	
	if(true == ret) nt_device = buf;

	free(buf); buf = NULL;
	return ret;
}

/**
* @brief	파일을 쓰기모드로 오픈한다.
*/
HANDLE open_file_to_write(_In_ const wchar_t* file_path)
{
	_ASSERTE(NULL != file_path);
	_ASSERTE(TRUE != IsBadStringPtrW(file_path, MAX_PATH));
	if ( (NULL == file_path) || (TRUE == IsBadStringPtrW(file_path, MAX_PATH)) )
	{
		return INVALID_HANDLE_VALUE;
	}

	HANDLE hFile = CreateFileW(file_path, 
						GENERIC_WRITE, 
						FILE_SHARE_READ, 
						NULL, 
						OPEN_ALWAYS, 
						FILE_ATTRIBUTE_NORMAL, 
						NULL);	
	if(hFile == INVALID_HANDLE_VALUE)
	{						
        log_err
            "CreateFile(path=%S), gle=0x%08x", 
            file_path, 
            GetLastError()
        log_end
	}

    return hFile;
}

/**
* @brief	파일을 읽기모드로 오픈한다.
*/
HANDLE open_file_to_read(LPCWCH file_path)
{
	_ASSERTE(NULL != file_path);
	_ASSERTE(TRUE != IsBadStringPtrW(file_path, MAX_PATH));
	if ( (NULL == file_path) || (TRUE == IsBadStringPtrW(file_path, MAX_PATH)) )
	{
		return INVALID_HANDLE_VALUE;
	}

	HANDLE hFile = CreateFileW(file_path, 
						GENERIC_READ, 
						FILE_SHARE_READ, 
						NULL, 
						OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL, 
						NULL);	
	if(hFile == INVALID_HANDLE_VALUE)
	{						
        log_err
            "CreateFile(path=%S), gle=0x%08x", 
            file_path, 
            GetLastError()
        log_end
	}

    return hFile;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_file_size(_In_ HANDLE file_handle, _Out_ uint64_t& size)
{
	_ASSERTE(INVALID_HANDLE_VALUE != file_handle);
	if (INVALID_HANDLE_VALUE == file_handle) return false;

	LARGE_INTEGER size_tmp = {0};
	
	if (TRUE != GetFileSizeEx(file_handle, &size_tmp))
	{
		log_err 
			"GetFileSizeEx( file = 0x%p ), gle = %u", file_handle, GetLastError() 
		log_end
		return false;
	}

	size = size_tmp.QuadPart;
	return true;
}

/**
* @brief	파일에 포맷문자열을 쓴다.
*/
BOOL write_to_filew(LPCWCH file_path, LPCWCH format,...)
{
	_ASSERTE(NULL != file_path);
	_ASSERTE(TRUE != IsBadStringPtrW(file_path, MAX_PATH));
	if ( (NULL == file_path) || (TRUE == IsBadStringPtrW(file_path, MAX_PATH)) )
	{
		return FALSE;
	}

	HANDLE hFile = CreateFileW(
                        file_path, 
						GENERIC_WRITE, 
						FILE_SHARE_READ,
						NULL, 
						OPEN_ALWAYS, 
						FILE_ATTRIBUTE_NORMAL, 
						NULL);	
	if(hFile == INVALID_HANDLE_VALUE)
	{						
        log_err
            "CreateFile(%S), gle=0x%08x", 
            file_path, GetLastError()
        log_end
        return FALSE;
	}

	// 자원 관리 객체 생성
	//
	SmrtHandle file_handle(hFile);

	DWORD pos = SetFilePointer (hFile, 0, NULL, FILE_END);
	if (INVALID_SET_FILE_POINTER == pos) return FALSE;

	va_list args;
	DWORD dw = 0;
	WCHAR temp[5120] = {0};
	WCHAR* pszDestEnd = temp;
	size_t cbRemaining = sizeof(temp);
		
	va_start(args, format);	
	if (TRUE != SUCCEEDED(StringCbVPrintfExW(
                                pszDestEnd, 
								cbRemaining,
								&pszDestEnd, 
								&cbRemaining, 
								0, 
								format, 
								args)))
	{
		_ASSERTE(!"StringCbVPrintf");
		va_end(args);
		return FALSE;
	}
	va_end(args);

	if (TRUE != WriteFile(
					hFile, 
					temp, 
					(DWORD)((DWORD_PTR)pszDestEnd - (DWORD_PTR)temp), 
					&dw, 
					NULL
					))
	{
		return FALSE;
	}

	return TRUE;		// for success
}

/**
* @brief	파일에 포맷 문자열을 쓴다
*/
BOOL write_to_filew(HANDLE hFile,LPCWCH format,...)
{
	DWORD pos = SetFilePointer (hFile, 0, NULL, FILE_END);
	if (INVALID_SET_FILE_POINTER == pos) return FALSE;

	va_list args;
	DWORD dw = 0;
	WCHAR temp[5120] = {0};
	WCHAR* pszDestEnd = temp;
	size_t cbRemaining = sizeof(temp);
		

	va_start(args, format);	
	if (TRUE != SUCCEEDED(StringCbVPrintfExW(
                                pszDestEnd, 
								cbRemaining,
								&pszDestEnd, 
								&cbRemaining, 
								0, 
								format, 
								args)))
	{
		_ASSERTE(!"StringCbVPrintf");
		va_end(args);
		return FALSE;
	}
	va_end(args);

	if (TRUE != WriteFile(
					hFile, 
					temp, 
					(DWORD)((DWORD_PTR)pszDestEnd - (DWORD_PTR)temp), 
					&dw, 
					NULL))
	{
		return FALSE;
	}
	
	return TRUE;		// for success
}

/**
* @brief	파일에 포맷 문자열을 쓴다
*/
BOOL write_to_filea(HANDLE hFile,LPCCH format,...)
{
	DWORD pos = SetFilePointer (hFile, 0, NULL, FILE_END);
	if (INVALID_SET_FILE_POINTER == pos) return FALSE;
		
	va_list args;
	DWORD dw = 0;
	CHAR temp[5120] = {0};
	CHAR* pszDestEnd = temp;
	size_t cbRemaining = sizeof(temp);	

	va_start(args, format);	
	if (TRUE != SUCCEEDED(StringCbVPrintfExA(
                                pszDestEnd, 
								cbRemaining,
								&pszDestEnd, 
								&cbRemaining, 
								0, 
								format, 
								args)))
	{
		_ASSERTE(!"StringCbVPrintf");
		va_end(args);
		return FALSE;
	}
	va_end(args);

    if (!SUCCEEDED(StringCbPrintfExA(
                                pszDestEnd, 
                                cbRemaining,
                                &pszDestEnd, 
                                &cbRemaining, 
                                0, 
                                "%s", 
                                "\r\n")))
    {
        _ASSERTE(!"StringCbPrintfEx()");
        return FALSE;
    }	

	if(TRUE != WriteFile(	
								hFile, 
								temp, 
								(DWORD)((DWORD_PTR)pszDestEnd - (DWORD_PTR)temp), 
								&dw, 
								NULL))
	{
		return FALSE;
	}
	
	return TRUE;		// for success
}

/**
 * @brief	retrieve current file position
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_file_position(_In_ HANDLE file_handle, _Out_ uint64_t& position)
{
	_ASSERTE(INVALID_HANDLE_VALUE != file_handle);
	if (INVALID_HANDLE_VALUE == file_handle) return false;

	DWORD file_type = GetFileType(file_handle);
	if (FILE_TYPE_DISK != file_type)
	{
		log_err 
			"invalid file type = %u, FILE_TYPE_DISK (1) only", file_type 
		log_end
		return false;
	}

	LARGE_INTEGER li_new_pos = {0};
	LARGE_INTEGER li_distance= {0};
	if (!SetFilePointerEx(file_handle, li_distance, &li_new_pos, FILE_CURRENT))
	{
		log_err
			"SetFilePointerEx() failed, gle = %u", GetLastError()
		log_end
		return false;
	}

	position = li_new_pos.QuadPart;
	return true;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool 
set_file_position(
	_In_ HANDLE file_handle, 
	_In_ uint64_t distance, 
	_Out_opt_ uint64_t* new_position
	)
{
	_ASSERTE(INVALID_HANDLE_VALUE != file_handle);
	if (INVALID_HANDLE_VALUE == file_handle) return false;

	DWORD file_type = GetFileType(file_handle);
	if (FILE_TYPE_DISK != file_type)
	{
		log_err 
			"invalid file type = %u, FILE_TYPE_DISK (1) only", file_type 
		log_end
		return false;
	}

	LARGE_INTEGER li_distance = {0}; li_distance.QuadPart = distance;
	LARGE_INTEGER li_new_pos = {0};

	if (!SetFilePointerEx(file_handle, li_distance, &li_new_pos, FILE_BEGIN))
	{
		log_err
			"SetFilePointerEx() failed, gle = %u", GetLastError()
		log_end
		return false;
	}

	if (NULL != new_position) { *new_position = li_new_pos.QuadPart; }
	return true;
}

/**
* @brief	문자열을 UTF8 형식 파일로 저장한다.
*           http://digitz.tistory.com/303   ANSI와 UTF-8과의 상호 변환
*           http://en.wikipedia.org/wiki/Byte_Order_Mark  
*           http://golbenge.wordpress.com/2009/12/24/stl%EC%9D%84-%EC%9D%B4%EC%9A%A9%ED%95%9C-unicode-%ED%85%8D%EC%8A%A4%ED%8A%B8-%ED%8C%8C%EC%9D%BC-%EC%B6%9C%EB%A0%A5/
*/
BOOL 
SaveToFileAsUTF8A(
    IN LPCWSTR FilePathDoesNotExists, 
    IN LPCSTR NullTerminatedAsciiString
    )
{
    _ASSERTE(NULL != FilePathDoesNotExists);
    if (NULL == FilePathDoesNotExists) return FALSE;    
    if (TRUE == is_file_existsW(FilePathDoesNotExists)) return FALSE;

    // ASCII ==> WIDE CHAR (UCS-2)
    // 
    PWSTR WideString = MbsToWcs(NullTerminatedAsciiString);
    if (NULL == WideString) return FALSE;
    SmrtPtr<PWSTR> spw(WideString);

    return SaveToFileAsUTF8W(
                FilePathDoesNotExists, 
                WideString
                );
}


/**
* @brief	문자열을 UTF8 형식 파일로 저장한다.
*/
BOOL 
SaveToFileAsUTF8W(
    IN LPCWSTR FilePathDoesNotExists, 
    IN LPCWSTR NullTerminatedWideString
    )
{
    _ASSERTE(NULL != FilePathDoesNotExists);
    if (NULL == FilePathDoesNotExists) return FALSE;    
    if (TRUE == is_file_existsW(FilePathDoesNotExists)) return FALSE;

    // UCS-2 ==> UTF-8
    // 
    PSTR Utf8String = WcsToMbsUTF8(NullTerminatedWideString);
    if (NULL == NullTerminatedWideString) return FALSE;
    SmrtPtr<PSTR> spu(Utf8String);

    HANDLE hFile = open_file_to_write(FilePathDoesNotExists);
    if (INVALID_HANDLE_VALUE == hFile) 
    {
        log_err
            "OpenFileToWrite(path=%S) failed",
            FilePathDoesNotExists
        log_end
        return FALSE;
    }

    // write file 
    // 
    DWORD cbWritten=0;
    BYTE ByteOrderMark[] = {0xEF, 0xBB, 0xBF};
    if (TRUE != WriteFile(
                    hFile, 
                    ByteOrderMark, 
                    sizeof(ByteOrderMark), 
                    &cbWritten, 
                    NULL))
    {
        log_err
            "WriteFile(BOM) failed, gle=0x%08x", 
            GetLastError()
        log_end

        CloseHandle(hFile); hFile = NULL;
        return FALSE;
    }

    if (TRUE != WriteFile(
                    hFile, 
                    Utf8String, 
                    (DWORD)strlen(Utf8String), 
                    &cbWritten, 
                    NULL))
    {
        log_err
            "WriteFile(Utf8String) failed, gle=0x%08x", 
            GetLastError()
        log_end
        CloseHandle(hFile); hFile = NULL;
        return FALSE;
    }
    
    CloseHandle(hFile); hFile = NULL;
    return TRUE;
}

/**
* @brief	파일을 메모리에 로드한다. 반환되는 메모리는 동적할당된 메모리이므로 caller 가 해제해야 함
*/
BOOL 
LoadFileToMemory(
    IN LPCWSTR  FilePath, 
    OUT DWORD&  MemorySize, 
    OUT PBYTE&  Memory
    )
{
    _ASSERTE(NULL != FilePath);
    _ASSERTE(TRUE == is_file_existsW(FilePath));
    if (NULL == FilePath || TRUE != is_file_existsW(FilePath)) return FALSE;

    HANDLE hFile = CreateFileW(
                        (LPCWSTR)FilePath, 
                        GENERIC_READ, 
                        FILE_SHARE_READ/* | FILE_SHARE_WRITE*/,
                        NULL, 
                        OPEN_EXISTING, 
                        FILE_ATTRIBUTE_NORMAL, 
                        NULL
                        );
    if (INVALID_HANDLE_VALUE == hFile)
    {
        log_err
            "CreateFile(%ws) failed, gle=0x%08x", 
            FilePath, 
            GetLastError()
        log_end
        return FALSE;
    }
    SmrtHandle sfFile(hFile);

    // check file size 
    // 
    LARGE_INTEGER fileSize;
    if (TRUE != GetFileSizeEx(hFile, &fileSize))
    {
        log_err
            "%ws, can not get file size, gle=0x%08x", 
            FilePath, 
            GetLastError() 
        log_end
        return FALSE;
    }

	if (0 == fileSize.QuadPart)
	{
		log_err "can not map zero length file" log_end
		return FALSE;
	}
   
    HANDLE hImageMap = CreateFileMapping(
                            hFile, 
                            NULL, 
                            PAGE_READONLY, 
                            0, 
                            0, 
                            NULL
                            );
    if (NULL == hImageMap)
    {
        log_err
            "CreateFileMapping(%ws) failed, gle=0x%08x", 
            FilePath, 
            GetLastError() 
        log_end

        return FALSE;
    }
    SmrtHandle sfMap(hImageMap);

    PBYTE ImageView = (LPBYTE) MapViewOfFile(
                                    hImageMap, 
                                    FILE_MAP_READ, 
                                    0, 
                                    0, 
                                    0
                                    );
    if(ImageView == NULL)
    {
        log_err
            "MapViewOfFile(%ws) failed, gle=0x%08x", 
            FilePath, 
            GetLastError() 
        log_end

        return FALSE;
    }
    SmrtView sfView(ImageView);

    MemorySize = fileSize.LowPart;  // max config fileSize = 4 MB 이므로 안전함
    Memory = (PBYTE) malloc(MemorySize);
    if (NULL == Memory) {return FALSE;}

    RtlZeroMemory(Memory, MemorySize);
    RtlCopyMemory(Memory, ImageView, MemorySize);
    return TRUE;
}

/**
* @brief	바이너리 파일로 데이터를 저장한다.
* @return	
*/
BOOL 
SaveBinaryFile(
    IN LPCWSTR  Directory,
    IN LPCWSTR  FileName, 
    IN DWORD    Size,
    IN PBYTE    Data
    )
{
    _ASSERT(NULL != Directory);
    _ASSERT(NULL != FileName);
    _ASSERT(0 < Size);
    _ASSERT(NULL != Data);
    _ASSERT(TRUE != IsBadReadPtr(Data, Size));
    if (NULL == Directory || NULL == FileName || 0 >= Size || NULL == Data ||
        TRUE == IsBadReadPtr(Data, Size))
    {
        return FALSE;
    }

    // create data directory
    //
    int ret=SHCreateDirectoryExW(NULL, Directory, NULL);
    if (ERROR_SUCCESS != ret && ERROR_ALREADY_EXISTS != ret)
    {
        log_err
            "SHCreateDirectoryExW(path=%S) failed, ret=0x%08x",
            Directory, ret
        log_end
        return FALSE;
    }

    WCHAR DataPath[MAX_PATH + 1] = {0};
    if (TRUE != SUCCEEDED(StringCbPrintfW(
                                DataPath, 
                                sizeof(DataPath), 
                                L"%s\\%s", 
                                Directory, 
                                FileName
                                )))
    {
        log_err
            "can not generate target path, dir=%S, file=%S", 
            Directory, FileName
        log_end
        return FALSE;
    }

    // 동일한 파일이 존재하는 경우 기존 파일을 삭제 후 새롭게 생성함
    // 
    if (TRUE == is_file_existsW(DataPath))
    {
        log_err
            "same file exists, file=%S will be replaced by new file",
            DataPath
        log_end
        
        ::DeleteFileW(DataPath);
    }

    HANDLE hFile = open_file_to_write(DataPath);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        log_err
            "can not create file=%S, check path or privilege", 
            DataPath
        log_end
        return FALSE;
    }
    SmrtHandle sh(hFile);

    DWORD cbWritten=0;
    if (TRUE != ::WriteFile(
                        hFile, 
                        Data, 
                        Size, 
                        &cbWritten, 
                        NULL))
    {
        log_err
            "WriteFile(path=%S) failed, gle=0x%08x",
            DataPath, GetLastError()
        log_end
        return FALSE;
    }

    return TRUE;
}

/**----------------------------------------------------------------------------
    \brief  DirectoryPath 디렉토리를 생성한다. 중간에 없는 디렉토리 경로가 존재하면
	\brief  생성한다. 
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
bool WUCreateDirectory(const LPCWSTR DirectoryPath)
{
	_ASSERTE(NULL != DirectoryPath);
	if (NULL==DirectoryPath) return false;

	if (true != is_file_existsW(DirectoryPath))
	{
		if (ERROR_SUCCESS != SHCreateDirectoryExW(NULL, DirectoryPath, NULL))
		{
			log_err
				"SHCreateDirectoryExW( path=%ws ) failed. gle=0x%08x", 
				DirectoryPath, GetLastError()
			log_end
			return false;
		}
	}

	return true;
}

/**----------------------------------------------------------------------------
    \brief	지정된 디렉토리(내부의 폴더, 파일등까지)를 몽땅 삭제하는 함수  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
bool WUDeleteDirectoryW(IN LPCWSTR  DirctoryPathToDelete)
{
	_ASSERTE(NULL != DirctoryPathToDelete);
	if (NULL == DirctoryPathToDelete) return false;

	SHFILEOPSTRUCTW FileOp={0};
	
	// FileOp.pFrom, FileOp.pTo 는 NULL char 가 두개이어야 함 (msdn 참고)
	// 
	size_t len = (wcslen(DirctoryPathToDelete) + 2) * sizeof(WCHAR);
	WCHAR* tmp= (WCHAR*)malloc(len);
	if (NULL == tmp) return false;
	RtlZeroMemory(tmp, len);
	if (TRUE != SUCCEEDED( StringCbPrintfW(tmp, len, L"%s", DirctoryPathToDelete) )) return false;

	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_DELETE;       // 삭제 속성 설정
	FileOp.pTo = NULL;
	FileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;//FOF_NOCONFIRMATION | FOF_NOERRORUI; // 확인메시지가 안뜨도록 설정
	FileOp.fAnyOperationsAborted = false;
	FileOp.hNameMappings = NULL;
	FileOp.lpszProgressTitle = tmp;
	FileOp.pFrom = tmp;

	int ret = SHFileOperation(&FileOp);
	if(0!=ret)
	{
		log_err
			"SHFileOperation(path=%S) failed, ret=0x%08x", 
			DirctoryPathToDelete, ret
		log_end
		return false;
	}

	return true;
}

/**
* @brief	
*/
BOOL GetSystemRootDirectory(DWORD Len,LPTSTR Buffer)
{
    _ASSERTE(NULL != Buffer);
    _ASSERTE(TRUE != IsBadWritePtr(Buffer, Len));
    if ( (NULL == Buffer) || (TRUE == IsBadWritePtr(Buffer, Len)) )
    {
        return FALSE;
    }

    if (0 != GetSystemDirectory(Buffer, Len))
        return TRUE;
    else
        return FALSE;
}

/**
* @brief	
*/
BOOL 
GetImageFullPathFromPredefinedPathA(
    IN  LPCSTR ImageName, 
    IN  DWORD  FullPathBufferLen,
    OUT LPSTR  FullPathBuffer
    )
{
    _ASSERTE(NULL != ImageName);
    _ASSERTE(NULL != FullPathBuffer);
    _ASSERTE(TRUE != IsBadWritePtr(FullPathBuffer, FullPathBufferLen));
    if (	(NULL == ImageName) || 
			(NULL == FullPathBuffer) || 
			(TRUE == IsBadWritePtr(FullPathBuffer, FullPathBufferLen)))
    {
        return FALSE;
    }

    
    WCHAR* wcs=MbsToWcs(ImageName);    
    if(NULL==wcs) { return FALSE;}
    SmrtPtr<WCHAR*> smpt(wcs);

    WCHAR buf[MAX_PATH] = {0};
    if (TRUE != GetImageFullPathFromPredefinedPathW(
                            wcs, 
                            MAX_PATH, 
                            buf))
    {
        return FALSE;
    }

    size_t len = wcslen(buf);
    if (FullPathBufferLen <= len)
    {
        log_err "buffer overflow" log_end
        return FALSE;
    }

    RtlZeroMemory(FullPathBuffer, FullPathBufferLen);
    RtlCopyMemory(FullPathBuffer, buf, len);
    return TRUE;
}

/**
* @brief	
*/
BOOL 
GetImageFullPathFromPredefinedPathW(
    IN  LPCWSTR ImageName, 
    IN  DWORD   FullPathBufferLen,
    OUT LPWSTR  FullPathBuffer
    )
{
    _ASSERTE(NULL != ImageName);
    _ASSERTE(NULL != FullPathBuffer);
    _ASSERTE(TRUE != IsBadWritePtr(FullPathBuffer, FullPathBufferLen));
    if (	(NULL == ImageName) || 
			(NULL == FullPathBuffer) || 
			(TRUE == IsBadWritePtr(FullPathBuffer, FullPathBufferLen)))
    {
        return FALSE;
    }    
    
    WCHAR* lpFilePart = NULL;    
    if (0 != SearchPathW(
                NULL,
                ImageName,
                NULL,
                FullPathBufferLen,
                FullPathBuffer,
                &lpFilePart
                ))
    {
        return TRUE;
    }
    else
    {
        // 다시 호출되는 경우 SearchPathBuf 버퍼를 다시 만드는 일이 없도록
        // static 으로 만듬
        //
        static WCHAR SearchPathBuf[MAX_PATH] = {0,};
        if (0x00 == SearchPathBuf[0])
        {
            WCHAR   SysRootDir[MAX_PATH] = {0,};
            if (TRUE != GetSystemRootDirectory(
								    sizeof(SysRootDir), 
								    SysRootDir))
            {
                log_err "GetSystemRootDirectory() failed" log_end                    
                return FALSE;
            }    

            if (! SUCCEEDED(StringCbPrintfW(
                                SearchPathBuf, 
                                sizeof(SearchPathBuf), 
                                L"%s\\drivers", 
                                SysRootDir)) )
            {
                log_err "StringCbPrintf() failed" log_end
                return FALSE;
            }    
        }    

        if (0 != SearchPathW(
                    SearchPathBuf,
                    ImageName,
                    NULL,
                    FullPathBufferLen,
                    FullPathBuffer,
                    &lpFilePart
                    ))
        {
            return TRUE;
        }
        else
        {   
            return FALSE;
        }
    }
}

/**
 * @brief      하위 디렉토리에 존재하는 모든 파일들을 enum 하는 함수

				아래 형태 4가지는 모두 동일한 결과를 출력함
				"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData",
				"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData\\",
				"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData\\*",
				"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData\\*.*"	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool
find_files(
	_In_ const wchar_t* root, 
	_In_ fnFindFilesCallback cb, 
	_In_ DWORD_PTR tag, 
	_In_ bool recursive
	)
{ 
    _ASSERTE(NULL != root);
    _ASSERTE(TRUE != IsBadStringPtr(root, MAX_PATH));
    if ((NULL == root) || (TRUE == IsBadStringPtr(root, MAX_PATH))) return false;


	// root 파라미터가 '\' 로 끝나면 안되므로 \* 로 강제 변경
	// 
	std::wstring root_dir(root);
    if (root[wcslen(root)-1] == L'\\')
    {
        root_dir.append(L"*");
    }

    HANDLE hSrch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd = {0};
    WCHAR fname[MAX_PATH+1] = {0};
    BOOL bResult=TRUE;
    WCHAR drive[_MAX_DRIVE+1] = {0};
    WCHAR dir[MAX_PATH+1] = {0};
    WCHAR newpath[MAX_PATH+1] = {0};
    
    hSrch=FindFirstFileW(root_dir.c_str(),&wfd);
    if(INVALID_HANDLE_VALUE == hSrch) 
    {
        log_err
            "FindFirstFileW(path=%S) failed, gle=0x%08x", 
            root_dir.c_str(), GetLastError()
        log_end
        return false;
    }

    while (bResult) 
    {
        WaitForSingleObject(NULL, 0);

        _wsplitpath_s(root_dir.c_str(), drive, _MAX_DRIVE, dir, MAX_PATH, NULL, NULL, NULL, NULL);		

        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) 
        {
            // symbolic link 는 처리하지 않음
            // 
            if (wfd.dwReserved0 & IO_REPARSE_TAG_SYMLINK)
            {                
				bResult=FindNextFile(hSrch,&wfd);
                continue;                
            }
        }
        else if ( (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  && true == recursive )
        {
			if (0 != _wcsnicmp(&wfd.cFileName[0], L".", 1))
			{
				StringCbPrintfW(newpath, sizeof(newpath), L"%s%s%s\\*.*", drive, dir, wfd.cFileName);
				find_files(newpath, cb, tag, recursive);
			}            
        } 
        else 
        {
            StringCbPrintfW(fname, sizeof(fname), L"%s%s%s", drive, dir, wfd.cFileName);

            if (NULL != cb)
            {
                if(TRUE != cb(tag, fname)) break;
            }
        }
        bResult=FindNextFile(hSrch,&wfd);
    }
    FindClose(hSrch);
    return true;
}


/**----------------------------------------------------------------------------
    \brief  RootPath 하위디렉토리 경로를 enum 하는 함수
			
			아래 형태 4가지는 모두 동일한 결과를 출력함
			"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData",
			"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData\\",
			"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData\\*",
			"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData\\*.*"

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
BOOL 
FindSubDirectory(
    IN LPCWSTR RootPath, 
    IN OUT std::vector<std::wstring>& DirectoryList, 
	IN BOOL Recursive
    )
{ 
    _ASSERTE(NULL != RootPath);
    _ASSERTE(TRUE != IsBadStringPtr(RootPath, MAX_PATH));
    if ((NULL == RootPath) || (TRUE == IsBadStringPtr(RootPath, MAX_PATH))) return FALSE;

    HANDLE hSrch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd = {0};
    BOOL bResult=TRUE;
    WCHAR drive[_MAX_DRIVE+1] = {0};
    WCHAR dir[MAX_PATH+1] = {0};

	// RootPath 파라미터가 c:\dbg 형태인 경우 c:\dbg 값(RootPath 자신)이 추가됨
	// 따라서 '\', '\*', '\*.*' 가 아닌 경우 강제로 '\*' 를 붙여 RootPath 경로는 제외함
	// 
	// 또한 RootPath 파라미터가 '\' 로 끝나면 안되므로 \* 로 강제 변경
	// 
	std::wstring RootDir(RootPath);
	if (RootPath[wcslen(RootPath)-1] == L'\\')
	{
		RootDir.append(L"*");
	}
	else if (RootPath[wcslen(RootPath)-1] == L'*')
	{
		// do nothing
	}
	else
	{
		RootDir.append(L"\\*");
	}


    hSrch=FindFirstFileW(RootDir.c_str(),&wfd);
    if(INVALID_HANDLE_VALUE == hSrch) 
    {
        log_err
            "FindFirstFileW(path=%S) failed, gle=0x%08x", 
            RootDir.c_str(), GetLastError()
        log_end
        return FALSE;
    }
	
    std::wstringstream s;
    while (bResult) 
    {
        WaitForSingleObject(NULL, 0);

        _wsplitpath_s(RootDir.c_str(), drive, _MAX_DRIVE, dir, MAX_PATH, NULL, NULL, NULL, NULL);
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
        {
            if (0 != _wcsnicmp(&wfd.cFileName[0], L".", 1))
            {
                s.str(L"");
                s << drive << dir << wfd.cFileName;
                std::wstring NewDir = s.str();
                DirectoryList.push_back(NewDir);

				if (TRUE == Recursive)
				{
					s << L"\\*.*";
					NewDir = s.str();
					FindSubDirectory(NewDir.c_str(), DirectoryList, TRUE);
				}
            }
        } 
        bResult=FindNextFile(hSrch,&wfd);
    }
    FindClose(hSrch);
    return TRUE;
}



/**
 * @brief	ASCII(Multibyte) --> WIDE CHAR 로 변환, caller 는 리턴되는 포인터를 소멸시켜주어야 함
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
wchar_t* MbsToWcs(_In_ const char* mbs)
{
    _ASSERTE(NULL!=mbs);
    if(NULL==mbs) return NULL;

    int outLen=MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mbs, -1, NULL, 0);
    if(0==outLen) return NULL;

    wchar_t* outWchar=(wchar_t*) malloc(outLen * (sizeof(wchar_t)));  // outLen contains NULL char 
    if(NULL==outWchar) return NULL;
    RtlZeroMemory(outWchar, outLen);

    if(0==MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mbs, -1, outWchar, outLen))
    {
        log_err "MultiByteToWideChar() failed, errcode=0x%08x", GetLastError() log_end

        free(outWchar);
        return NULL;
    }

    return outWchar;
}

/**
 * @brief	WIDE CHAR --> ASCII(Multibyte) 로 변환, caller 는 리턴되는 포인터를 소멸시켜주어야 함
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
char* WcsToMbs(_In_ const wchar_t* wcs)
{
    _ASSERTE(NULL!=wcs);
    if(NULL==wcs) return NULL;

    int outLen=WideCharToMultiByte(CP_ACP, 0, wcs, -1, NULL, 0, NULL, NULL);
    if(0==outLen) return NULL;

    char* outChar=(char*) malloc(outLen * sizeof(char));
    if(NULL==outChar) return NULL;
    RtlZeroMemory(outChar, outLen);

    if(0==WideCharToMultiByte(CP_ACP, 0, wcs, -1, outChar, outLen, NULL, NULL))
    {
        log_err "WideCharToMultiByte() failed, errcode=0x%08x", GetLastError() log_end
        free(outChar);
        return NULL;
    }

    return outChar;
}
    
/**
 * @brief	wide char -> utf8 변환, caller 는 리턴되는 포인터를 소멸시켜주어야 함 
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
char* WcsToMbsUTF8(_In_ const wchar_t* wcs)
{
    _ASSERTE(NULL!=wcs);
    if(NULL==wcs) return NULL;

    int outLen=WideCharToMultiByte(CP_UTF8, 0, wcs, -1, NULL, 0, NULL, NULL);
    if(0==outLen) return NULL;

    char* outChar=(char*) malloc(outLen * sizeof(char));
    if(NULL==outChar) return NULL;
    RtlZeroMemory(outChar, outLen);

    if(0==WideCharToMultiByte(CP_UTF8, 0, wcs, -1, outChar, outLen, NULL, NULL))
    {
        log_err "WideCharToMultiByte() failed, errcode=0x%08x", GetLastError() log_end

        free(outChar);
        return NULL;
    }

    return outChar;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
wchar_t* Utf8MbsToWcs(_In_ const char* utf8)
{
    _ASSERTE(NULL!=utf8);
    if(NULL==utf8) return NULL;

    int outLen=MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, NULL, 0);
    if(0==outLen) return NULL;

    wchar_t* outWchar=(wchar_t*) malloc(outLen * (sizeof(wchar_t)));  // outLen contains NULL char 
    if(NULL==outWchar) return NULL;
    RtlZeroMemory(outWchar, outLen);

    if(0==MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, outWchar, outLen))
    {
        log_err "MultiByteToWideChar() failed, errcode=0x%08x", GetLastError() log_end

        free(outWchar);
        return NULL;
    }

    return outWchar;
}

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
std::wstring MbsToWcsEx(_In_ const char *mbs)
{
    raii_wchar_ptr tmp( MbsToWcs(mbs), raii_free );
    if (NULL == tmp)
    {
        return _null_stringw;
    }
    else
    {
        return std::wstring(tmp.get());
    }

}

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
std::string WcsToMbsEx(_In_ const wchar_t *wcs)
{
	raii_char_ptr tmp( WcsToMbs(wcs), raii_free );
    if (NULL == tmp.get())
    {
        return _null_stringa;
    }
    else
    {
        return std::string(tmp.get());
    }
}

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
std::string WcsToMbsUTF8Ex(_In_ const wchar_t *wcs)
{
    raii_char_ptr tmp( WcsToMbsUTF8(wcs), raii_free);
    if (NULL == tmp.get())
    {
        return _null_stringa;
    }
    else
    {
        return std::string(tmp.get());
    }
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
std::wstring Utf8MbsToWcsEx(_In_ const char* utf8)
{
    std::auto_ptr<wchar_t> tmp( Utf8MbsToWcs(utf8) );
    if (NULL == tmp.get())
    {
        return _null_stringw;
    }
    else
    {
        return std::wstring(tmp.get());
    }
}

/**	---------------------------------------------------------------------------
	\brief	

			#include <comutil.h>
			#pragma comment(lib, "comsuppw.lib")
			
            #define STRSAFE_NO_DEPRECATE
            #include <strsafe.h>
	\param	
	\return	success : allocate null terminated TCHAR string and return pointer
					  (must free TCHAR string)
			fail    : return NULL
	\code	
	\endcode
-----------------------------------------------------------------------------*/
//LPTSTR BstrToTchar(IN const BSTR bstr)
//{
//	_ASSERTE(NULL != bstr);
//	if (NULL == bstr){return NULL;}
//	
//	_bstr_t tmp(bstr);
//	DWORD	CharCount = tmp.length();
//
//#ifdef  UNICODE
//	DWORD	c_str_buf_len = (sizeof(TCHAR) * CharCount ) + sizeof(TCHAR);
//#else
//	// unicode 가 아닌 경우 한글같은 wide char 는 2바이트를 사용함
//	// 그러나 _bstr_t::length() 는 문자의 갯수만 리턴하므로 
//	// CharCount * sizeof(TCHAR) * 2 를 해 줘야 한다. 
//	// 물론 unicode 인 경우는 아무 문제 없음
//	//
//	DWORD	c_str_buf_len = (( sizeof(TCHAR) * CharCount ) * 2 ) + sizeof(TCHAR);
//#endif
//	LPTSTR	c_str = (LPTSTR) malloc(c_str_buf_len);
//	RtlZeroMemory(c_str, c_str_buf_len);
//	if (!SUCCEEDED(StringCbPrintf(c_str, c_str_buf_len, TEXT("%s"), (LPCTSTR)tmp)))
//	{
//		free(c_str); 
//		c_str = NULL;
//	}
//
//	return c_str;
//}


/**
 * \brief	org_string 에서 token 을 검색해서 문자열을 잘라낸다. 
			(org_string 의 앞에서부터 token 을 검색)

			ABCDEFG.HIJ.KLMN	: org_string
				   .			: token
			ABCDEFG             : out_string if forward = TRUE
					HIJ.KLMN	: out_string if forward = FALSE

			delete_token 가 True 인 경우 org_string 에서 out_string + token 을 삭제

 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool
extract_first_tokenW(
	_In_ std::wstring& org_string,
	_In_ const std::wstring& token,
	_Out_ std::wstring& out_string, 
	_In_ bool forward,
	_In_ bool delete_token
	)
{
    if (true == delete_token)
    {
        if (&org_string == &out_string) 
        {
#ifndef TEST_EXPORTS
            _ASSERTE(!"prarameters conflict! ");
#endif
            return false;
        }
    }

	size_t pos = org_string.find(token);
	if (std::wstring::npos == pos)
	{
        out_string = org_string;
        return true;
	}

	if (true== forward)
	{
		out_string = org_string.substr(0, pos);
        if (delete_token) org_string.erase(0, pos + token.size());
	}
	else
	{
		out_string = org_string.substr(pos + token.size(), org_string.size());
        if (delete_token) org_string.erase(pos, org_string.size());
	}
    return true;
}

/**
 * \brief	org_string 에서 token 을 검색해서 문자열을 잘라낸다. 
			(org_string 의 앞에서부터 token 을 검색)

			ABCDEFG.HIJ.KLMN	: org_string
				   .			: token
			ABCDEFG             : out_string if forward = TRUE
					HIJ.KLMN	: out_string if forward = FALSE
 * @param	
 * @see		
 * @remarks 성공 시 분리된 문자열을 스트링 객체에 리턴
			실패 시 _nullstringw 리턴
 * @code		
 * @endcode	
 * @return	
**/
std::wstring 
extract_first_tokenExW(
	_In_ const wchar_t* org,
	_In_ const wchar_t* token,	
	_In_ bool forward
	)
{
	_ASSERTE(NULL != org);
	_ASSERTE(NULL != token);
	if (NULL == org || NULL == token) return _null_stringw;

	std::wstring org_string = org;
	std::wstring out_string;
	if (true != extract_first_tokenW(org_string, token, out_string, forward, false)) 
		return _null_stringw;
	else
		return out_string;
}


/**
 * @brief	org_string 에서 token 을 검색해서 문자열을 잘라낸다. 
			(org_string 의 앞에서부터 token 을 검색)
			
			ABCDEFG.HIJ.KLMN	: org_string
			       .			: token
		    ABCDEFG             : out_string if forward = TRUE
			        HIJ.KLMN	: out_string if forward = FALSE

            delete_token 가 True 인 경우 org_string 에서 out_string + token 을 삭제
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool
extract_first_tokenA(
	_In_ std::string& org_string,
	_In_ const std::string& token,
	_Out_ std::string& out_string, 
	_In_ bool forward,
	_In_ bool delete_token
    )	
{
    if (true == delete_token)
    {
        if (&org_string == &out_string) 
        {
#ifndef TEST_EXPORTS
            _ASSERTE(!"prarameters conflict! ");
#endif
            return false;;
        }
    }

	size_t pos = org_string.find(token);
	if (std::string::npos == pos)
	{
        out_string = org_string;
        return true;
	}

	if (TRUE == forward)
	{
		out_string = org_string.substr(0, pos);
        if (delete_token) org_string.erase(0, pos + token.size());
	}
	else
	{
		out_string = org_string.substr(pos + token.size(), org_string.size());
        if (delete_token) org_string.erase(pos, org_string.size());
	}
    return true;
}

/**
 * @brief	org_string 에서 token 을 검색해서 문자열을 잘라낸다. 
			(org_string 의 앞에서부터 token 을 검색)
			
			ABCDEFG.HIJ.KLMN	: org_string
			       .			: token
		    ABCDEFG             : out_string if forward = TRUE
			        HIJ.KLMN	: out_string if forward = FALSE

 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
std::string
extract_first_tokenExA(
		_In_ const char* org,
		_In_ const char* token,		
		_In_ bool forward
		)
{
	_ASSERTE(NULL != org);
	_ASSERTE(NULL != token);
	if (NULL == org || NULL == token) return _null_stringa;

	std::string org_string = org;
	std::string out_string;
	if (true != extract_first_tokenA(org_string, token, out_string, forward, false)) 
		return _null_stringa;
	else
		return out_string;
}

/**
 * @brief	org_string 에서 token 을 검색해서 문자열을 잘라낸다. 
			(org_string 의 뒤에서부터 token 을 검색)

			ABCDEFG.HIJ.KLMN	: org_string
					   .		: token
			ABCDEFG.HIJ			: out_string if forward = TRUE
					    KLMN	: out_string if forward = FALSE

			delete_token 가 True 인 경우 org_string 에서 out_string + token 을 삭제
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool 
extract_last_tokenW(
	_In_ std::wstring& org_string,
	_In_ const std::wstring& token,
	_Out_ std::wstring& out_string, 
	_In_ bool forward,
	_In_ bool delete_token
	)
{
    if (true== delete_token)
    {
        if (&org_string == &out_string) 
        {
#ifndef TEST_EXPORTS
            _ASSERTE(!"prarameters conflict! ");
#endif
            return false;
        }
    }

    size_t pos = org_string.rfind(token);
	if (std::wstring::npos == pos)
	{
        out_string = org_string;
        return true;
	}

	if (true== forward)
	{
		out_string = org_string.substr(0, pos);
        if (delete_token) org_string.erase(0, pos + token.size());
	}
	else
	{
        out_string = org_string.substr(pos + token.size(), org_string.size());
        if (delete_token) org_string.erase(pos, org_string.size());
	}
    return true;
}

/**
 * @brief	org_string 에서 token 을 검색해서 문자열을 잘라낸다. 
			(org_string 의 뒤에서부터 token 을 검색)

			ABCDEFG.HIJ.KLMN	: org_string
					   .		: token
			ABCDEFG.HIJ			: out_string if forward = TRUE
					    KLMN	: out_string if forward = FALSE
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
std::wstring
extract_last_tokenExW(
	_In_ const wchar_t* org,
	_In_ const wchar_t* token,	
	_In_ bool forward
	)
{
	_ASSERTE(NULL != org);
	_ASSERTE(NULL != token);
	if (NULL == org || NULL == token) return _null_stringw;

	std::wstring org_string = org;
	std::wstring out_string;
	if (true != extract_last_tokenW(org_string, token, out_string, forward, false))
		return _null_stringw;
	else
		return out_string;
}

/**
 * @brief	org_string 에서 token 을 검색해서 문자열을 잘라낸다. 
			(org_string 의 뒤에서부터 token 을 검색)
			
			ABCDEFG.HIJ.KLMN	: org_string
			           .		: token
		    ABCDEFG.HIJ			: out_string if forward = TRUE
						KLMN	: out_string if forward = FALSE

            delete_token 가 True 인 경우 org_string 에서 out_string + token 을 삭제
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool 
extract_last_tokenA(
	_In_ std::string& org_string,
	_In_ const std::string& token,
	_Out_ std::string& out_string, 
	_In_ bool forward,
	_In_ bool delete_token
    )
{
    if (TRUE == delete_token)
    {
        if (&org_string == &out_string) 
        {
#ifndef TEST_EXPORTS
            _ASSERTE(!"prarameters conflict! ");
#endif
            return false;
        }
    }

	size_t pos = org_string.rfind(token);
	if (std::string::npos == pos)
	{
        out_string = org_string;
        return true;
	}

	if (true == forward)
	{
		out_string = org_string.substr(0, pos);
        if (delete_token) org_string.erase(0, pos + token.size());
	}
	else
	{
        out_string = org_string.substr(pos + token.size(), org_string.size());
        if (delete_token) org_string.erase(pos, org_string.size());
	}
    return true;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
std::string
extract_last_tokenExA(
	_In_ const char* org,
	_In_ const char* token,
	_In_ bool forward
    )
{
	_ASSERTE(NULL != org);
	_ASSERTE(NULL != token);
	if (NULL == org || NULL == token) return _null_stringa;

	std::string org_string = org;
	std::string out_string;
	if (true != extract_last_tokenA(org_string, token, out_string, forward, false))
		return _null_stringa;
	else
		return out_string;

}



/**
* @brief	trim 함수들

            string szbuf="__12345_____";  
            cout<<"Before trim: "<<szbuf<<endl;  
            cout<<"After  trim: "<<trim(szbuf,"_")<<endl;
            cout<<"After rtrim: "<<rtrim(szbuf,"_")<<endl;
            cout<<"After ltrim: "<<ltrim(szbuf,"_")<<endl;
            ----------------------------------------------
            Before trim: __12345_____
            After  trim: 12345
            After rtrim: __12345
            After ltrim: 12345
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
std::string trima(std::string& s, const std::string& drop)
{
    std::string r=s.erase(s.find_last_not_of(drop)+1);
    return r.erase(0,r.find_first_not_of(drop));
}
std::string rtrima(std::string& s, const std::string& drop)
{
  return s.erase(s.find_last_not_of(drop)+1); 
}
std::string ltrima(std::string& s, const std::string& drop)
{ 
  return s.erase(0,s.find_first_not_of(drop)); 
}
std::wstring  trimw(std::wstring& s, const std::wstring& drop)
{
    std::wstring r=s.erase(s.find_last_not_of(drop)+1);
    return r.erase(0,r.find_first_not_of(drop));
}
std::wstring rtrimw(std::wstring& s, const std::wstring& drop)
{
    return s.erase(s.find_last_not_of(drop)+1); 
}
std::wstring ltrimw(std::wstring& s, const std::wstring& drop)
{
    return s.erase(0,s.find_first_not_of(drop)); 
}


/**
 * @brief	custom destructor for Windows HANDLE
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void raii_CloseHandle(_In_ HANDLE handle)
{
	if (NULL == handle || INVALID_HANDLE_VALUE == handle) return;
	CloseHandle(handle);
}

/**
 * @brief	custom destructor for void pointer
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void raii_free(_In_ void* void_ptr)
{
	if (NULL == void_ptr) return;
	free(void_ptr);
}

/**
 * @brief	custom destructor for MapViewOfFile( )
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void raii_UnmapViewOfFile(_In_ void* void_ptr)
{
	if (NULL != void_ptr)
	{
		UnmapViewOfFile(void_ptr);
	}
}

/**
* @brief	현재 디렉토리를 리턴하는 함수 (e.g. c:\debug )
* @param	
* @code		
* @endcode	
* @return	
*/
BOOL WUGetCurrentDirectoryW(IN OUT std::wstring& CurrentDir)
{
	UINT32 buflen=0;
	PWSTR buf=NULL;

	buflen = GetCurrentDirectoryW(buflen, buf);
	if (0 == buflen)
	{
		log_err
			"GetCurrentDirectoryW() failed. gle=0x%08x", 
			GetLastError()
		log_end
		return FALSE;
	}

	// buflen : NULL 캐릭터를 포함한 필요한 버퍼의 사이즈 in char.
	// 
	buf = (PWSTR) malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		log_err
			"GetCurrentDirectoryW() failed, gle=0x%08x", 
			GetLastError()
		log_end

		free(buf);
		return FALSE;
	}

    CurrentDir = buf;
    free(buf);
    return TRUE;
}

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
BOOL WUGetCurrentDirectoryA(IN OUT std::string& CurrentDir)
{
	std::wstring _cur;
	if (TRUE != WUGetCurrentDirectoryW(_cur))
	{
		return FALSE;
	}
	std::string _cura = WcsToMbsEx(_cur.c_str() );
	CurrentDir = _cura;
	return true;
}

/**
 * @brief	GetTempPath() wrapper.
			%TMP% > %TEMP% > %USERPROFILE% 환경변수 순서대로 가져옴
			마지막에 '\' 붙여서 리턴한다.
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_temp_dirW(_Out_ std::wstring& temp_dir)
{
	WCHAR path[MAX_PATH + 1] = {0};

	DWORD ret = GetTempPathW(MAX_PATH, path);
	if (ret > MAX_PATH || 0 == ret)
	{
		log_err "GetTempPathW() failed. gle = %u", GetLastError() log_end
		return false;
	}

	temp_dir = path;
	return true;
}

/**
 * @brief	GetTempPath() wrapper.
			%TMP% > %TEMP% > %USERPROFILE% 환경변수 순서대로 가져옴
			마지막에 '\' 붙여서 리턴한다.
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_temp_dirA(_Out_ std::string& temp_dir)
{
	char path[MAX_PATH + 1] = {0};

	DWORD ret = GetTempPathA(MAX_PATH, path);
	if (ret > MAX_PATH || 0 == ret)
	{
		log_err "GetTempPathA() failed. gle = %u", GetLastError() log_end
		return false;
	}

	temp_dir = path;
	return true;	
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_module_path(_In_ const wchar_t* module_name, _Out_ std::wstring& module_path)
{
	DWORD  ret = 0;
	DWORD  buf_len = MAX_PATH;
	LPTSTR buf = (LPTSTR) malloc( buf_len * sizeof(TCHAR) );
	if (NULL == buf) return false;
	
	for(;;)
	{
		ret = GetModuleFileName(GetModuleHandle(module_name), buf, buf_len);
		if (ret == buf_len)
		{
			// buf 가 작은 경우 buf_len 만큼 버퍼가 잘리고, buf_len 리턴 (에러로 간주)
			// 버퍼 사이즈를 2배 늘려서 재 시도
			free(buf);

			buf_len *= 2;
			buf = (LPTSTR) malloc( buf_len * sizeof(TCHAR) );
			if (NULL == buf) return false;
		}
		else
		{
			module_path = buf;
			free(buf);
			return true;
		}
	}

	//return false;	// never reach here!
}

/**
 * @brief	현재 모듈의 full path 를 구한다. 
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_current_module_path(_Out_ std::wstring& module_path)
{
	return get_module_path(NULL, module_path);
}

/**
 * @brief	현재 모듈의 파일명을 제외한 디렉토리 경로를 구한다. ('\' 문자는 제외)
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_current_module_dir(_Out_ std::wstring& module_dir)
{
	std::wstring module_path;
	if (true != get_current_module_path(module_path))
	{
		log_err "get_current_module_path()" log_end
		return false;
	}
	
	if (true != extract_last_tokenW(module_path, L"\\", module_dir, true, false))
	{
		log_err "extract_last_tokenW( org=%s )", module_path.c_str() log_end
		module_dir = L"";
		return false;
	}

	return true;
}

/**
 * @brief	현재 모듈의 파일명을 구한다. (경로제외)
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_current_module_file(_Out_ std::wstring& module_file)
{
	std::wstring module_path;
	if (true != get_current_module_path(module_path))
	{
		log_err "get_current_module_path()" log_end
		return false;
	}
	
	if (true != extract_last_tokenW(module_path, L"\\", module_file, false, false))
	{
		log_err "extract_last_tokenW( org=%s )", module_path.c_str() log_end
		module_file = L"";
		return false;
	}

	return true;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
std::wstring get_module_pathEx(_In_ const wchar_t* module_name)
{
	std::wstring out;
	if (true != get_module_path(module_name, out))
	{
		return _null_stringw;
	}
	else
	{
		return out;
	}
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
std::wstring get_module_dirEx(_In_ const wchar_t* module_name)
{
	std::wstring module_path = get_module_pathEx(module_name);
	return extract_last_tokenExW(module_path.c_str(), L"\\", true);
}
	


/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
std::wstring get_current_module_pathEx()
{
	std::wstring out;
	if (true != get_current_module_path(out))
	{
		return _null_stringw;
	}
	else
	{
		return out;
	}
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
std::wstring get_current_module_dirEx()
{
	std::wstring out;
	if (true != get_current_module_dir(out))
	{
		return _null_stringw;
	}
	else
	{
		return out;
	}
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
std::wstring get_current_module_fileEx()
{
	std::wstring out;
	if (true != get_current_module_file(out))
	{
		return _null_stringw;
	}
	else
	{
		return out;
	}
}

/**
 * @brief	system directory 경로를 리턴하는 함수
 * @param	
 * @see		
 * @remarks	경로가 c:\ 인 경우를 제외하고, '\' 를 붙이지 않음. (GetWindowsDirectory() 함수 스펙)
 * @code		
 * @endcode	
 * @return	
**/
bool get_system_directory(_Out_ std::wstring& system_dir)
{
	bool ret = false;
	UINT32 buf_len = MAX_PATH;
	wchar_t* buf = (wchar_t*) malloc(buf_len);
	if (NULL == buf) return false;
	RtlZeroMemory(buf, buf_len);

	for(;;)
	{
		UINT32 len = GetWindowsDirectoryW(buf, buf_len);
		if (0 == len) 
		{
			break;				// error
		}

		if (len == buf_len)
		{
			// GetWindowsDirectoryW( ) 는 null char 를 포함하지 않는 길이를 리턴함
			// 버퍼가 더 필요하다.
			buf_len *= 2;
			free(buf);
			buf = (wchar_t*) malloc(buf_len);
			if (NULL == buf) return false;			
			RtlZeroMemory(buf, buf_len);
			
			continue;				// re-try
		}
		else
		{
			system_dir = buf;
			ret = true;				// success
			break;
		}
	}

	free(buf); buf = NULL;
	return ret;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
std::string Win32ErrorToStringA(IN DWORD ErrorCode)
{
    char* msg=NULL;
	if(0 == FormatMessageA(
		        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		        NULL, 
		        ErrorCode, 
		        0, 
		        (CHAR*) &msg, 
		        0, 
		        NULL))
    {
        return std::string("error code conversion failed");
    }
    
    std::string ret(msg);    
    LocalFree(msg);
    return ret;
}

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
std::wstring Win32ErrorToStringW(IN DWORD ErrorCode)
{
    wchar_t* msg=NULL;
	if(0 == FormatMessageW(
		        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		        NULL, 
		        ErrorCode, 
		        0, 
		        (wchar_t*) &msg, 
		        0, 
		        NULL))
    {
        return std::wstring(L"error code conversion failed");
    }
    
    std::wstring ret(msg);    
    LocalFree(msg);
    return ret;
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
BOOL DumpMemory(DWORD Length, BYTE* Buf)
{
	if ( (0 < Length) && (NULL != Buf) && (TRUE != IsBadReadPtr(Buf, Length)) )
	{
        log_info "length = %u, buffer=0x%08x", Length, Buf log_end

		CHAR print_buf[128 * sizeof(CHAR)] = {0};
		DWORD i = 0, x = 0, ib = 0;		
		UCHAR*  Addr = Buf;
		CHAR*	Pos = NULL;
		size_t	Remain = 0;
		for(;;)
		{
			if (i >= Length) break;
            ib = i;

			// reset all
			//
			Pos = print_buf;
			Remain = sizeof(print_buf);

			if (! SUCCEEDED(StringCbPrintfExA(
                                Pos, 
								Remain, 
								&Pos, 
								&Remain, 
								0, 
								"0x%08p    ", 
								&Addr[i])))
			{
                log_err "StringCbPrintfEx() failed" log_end
				break;
			}	

			// first 8 bytes
			//
			for (x = 0; x < 8; x++, i++)
			{
                if (x == Length) break;

				if (! SUCCEEDED(StringCbPrintfExA(
                                    Pos, 
									Remain, 
									&Pos, 
									&Remain, 
									0, 
									"%02X ", 
									Addr[i])))
				{
                    log_err "StringCbPrintfEx() failed" log_end
					break;
				}		
			}

			if (x != Length)
            {
                // insert space between first 8bytes and last 8 bytes.
			    //
			    if (! SUCCEEDED(StringCbPrintfExA(
                                    Pos, 
								    Remain, 
								    &Pos, 
								    &Remain, 
								    0,
								    "%s",
								    "  ")))
			    {
                    log_err "StringCbPrintfEx() failed" log_end
				    break;
			    }
            }
            
            // last 8 bytes
			//
			for (x = 0; x < 8; x++, i++)
			{
                if (x == Length) break;

				if (! SUCCEEDED(StringCbPrintfExA(
                                    Pos, 
									Remain, 
									&Pos, 
									&Remain, 
									0, 
									"%02X ", 
									Addr[i])))
				{
                    log_err "StringCbPrintfEx() failed" log_end
					break;
				}		
			}

            char tmp[64] = {0};
            Pos = tmp;
            Remain = sizeof(tmp) - sizeof(char);
            for(DWORD p = 0; p < 16; ++p)
            {
                if (p == Length) break;

                if (0x20 <= Addr[ib] &&  0x7F > Addr[ib])
                {
                    if(!SUCCEEDED(StringCbPrintfExA(
                                        Pos, 
                                        Remain, 
                                        &Pos, 
                                        &Remain, 
                                        0, 
                                        "%c", 
                                        Addr[ib])))
                    {
                        log_err "StringCbPrintfEx() failed" log_end
					    break;
                    }
                }
                else
                {
                    if(!SUCCEEDED(StringCbPrintfExA(
                                        Pos, 
                                        Remain, 
                                        &Pos, 
                                        &Remain, 
                                        0,
                                        "%c",
                                        '.')))                                        
                    {
                        log_err "StringCbPrintfEx() failed" log_end
					    break;
                    }                
                }
                
                ++ib;
            }
            
			// print string..
			//
            log_info "  %s   %s", print_buf, tmp log_end            
			memset(print_buf, 0x00, sizeof(print_buf));
		}

		log_info "  %s\n\n", print_buf log_end
		return TRUE;
	}

    log_err "invalid parameters" log_end
	return FALSE;
}

/** ---------------------------------------------------------------------------
    \brief  

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
BOOL DumpMemory(FILE* stream,DWORD Length,BYTE* Buf)
{
	if ( (0 < Length) && (NULL != Buf) && (TRUE != IsBadReadPtr(Buf, Length)) )
	{
		_ftprintf(stream, TEXT("\n  00 01 02 03 04 05 06 07   08 09 0A 0B 0C 0D 0E 0F\n"));
		_ftprintf(stream, TEXT("  -- -- -- -- -- -- -- --   -- -- -- -- -- -- -- --\n"));

		TCHAR print_buf[128 * sizeof(TCHAR)] = {0};
		DWORD i = 0, x = 0;		
		UCHAR*  Addr = Buf;
		TCHAR*	Pos = NULL;
		size_t	Remain = 0;
		for(;;)
		{
			if (i >= Length) break;

			// reset all
			//
			Pos = print_buf;
			Remain = sizeof(print_buf);

			// first 8 bytes
			//
			for (x = 0; x < 8; x++, i++)
			{
                if (x == Length) break; 

				if (! SUCCEEDED(StringCbPrintfEx(
                                    Pos, 
									Remain, 
									&Pos, 
									&Remain, 
									0, 
									TEXT("%02X "), 
									Addr[i])))
				{
					_ftprintf(stream, TEXT("StringCbPrintfEx() failed \n"));
					break;
				}		
			}

            if(x == Length) break;
                        
            // insert space between first 8bytes and last 8 bytes.
			//
			if(!SUCCEEDED(StringCbPrintfEx(
                                Pos, 
								Remain, 
								&Pos, 
								&Remain, 
								0,
								TEXT("%s"),
								TEXT("  "))))
			{
				_ftprintf(stream, TEXT("StringCbPrintfEx() failed \n"));
				break;
			}

            // last 8 bytes
			//
			for (x = 0; x < 8; x++, i++)
			{
                if ((x + 8) == Length) break;

				if (! SUCCEEDED(StringCbPrintfEx(
                                    Pos, 
									Remain, 
									&Pos, 
									&Remain, 
									0, 
									TEXT("%02X "), 
									Addr[i])))
				{
					_ftprintf(stream, TEXT("StringCbPrintfEx() failed \n"));
					break;
				}		
			}			
            
			// print string..
			//
			_ftprintf(stream, TEXT("  %s\n"), print_buf);
			memset(print_buf, 0x00, sizeof(print_buf));
		}

		_ftprintf(stream, TEXT("  %s\n\n"), print_buf);	
		return TRUE;
	}

	_ftprintf(stream, TEXT("%s > invalid parameters \n"), __FUNCTION__);
	return FALSE;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool dump_memory(_In_ unsigned char* buf, _In_ UINT32 buf_len, _Out_ std::vector<std::string>& dump)
{
	_ASSERTE(NULL!=buf);
	_ASSERTE(0 < buf_len);
	if (NULL == buf || 0 == buf_len) return false;
	
	// !주의! - 한 라인이 line_dump 보다 큰 경우 (설마 그런일이...?!) 문제가 발생 할 수 있음
	char line_dump[1024];

	if ( (0 < buf_len) && (NULL != buf) && (TRUE != IsBadReadPtr(buf, buf_len)) )
	{
		StringCbPrintfA(line_dump, sizeof(line_dump), "buf_len = %u, buffer=0x%08x", buf_len, buf);
		dump.push_back(line_dump);
		
		CHAR print_buf[128 * sizeof(CHAR)] = {0};
		DWORD i = 0, x = 0, ib = 0;		
		UCHAR*  Addr = buf;
		CHAR*	Pos = NULL;
		size_t	Remain = 0;
		for(;;)
		{
			if (i >= buf_len) break;
            ib = i;

			// reset all
			//
			Pos = print_buf;
			Remain = sizeof(print_buf);

			if (! SUCCEEDED(StringCbPrintfExA(
                                Pos, 
								Remain, 
								&Pos, 
								&Remain, 
								0, 
								"0x%08p    ", 
								&Addr[i])))
			{
                log_err "StringCbPrintfEx() failed" log_end
				break;
			}	

			// first 8 bytes
			//
			for (x = 0; x < 8; x++, i++)
			{
                if (x == buf_len) break;

				if (! SUCCEEDED(StringCbPrintfExA(
                                    Pos, 
									Remain, 
									&Pos, 
									&Remain, 
									0, 
									"%02X ", 
									Addr[i])))
				{
                    log_err "StringCbPrintfEx() failed" log_end
					break;
				}		
			}

			if (x != buf_len)
            {
                // insert space between first 8bytes and last 8 bytes.
			    //
			    if (! SUCCEEDED(StringCbPrintfExA(
                                    Pos, 
								    Remain, 
								    &Pos, 
								    &Remain, 
								    0,
								    "%s",
								    "  ")))
			    {
                    log_err "StringCbPrintfEx() failed" log_end
				    break;
			    }
            }
            
            // last 8 bytes
			//
			for (x = 0; x < 8; x++, i++)
			{
                if (x == buf_len) break;

				if (! SUCCEEDED(StringCbPrintfExA(
                                    Pos, 
									Remain, 
									&Pos, 
									&Remain, 
									0, 
									"%02X ", 
									Addr[i])))
				{
                    log_err "StringCbPrintfEx() failed" log_end
					break;
				}		
			}

            char tmp[64] = {0};
            Pos = tmp;
            Remain = sizeof(tmp) - sizeof(char);
            for(DWORD p = 0; p < 16; ++p)
            {
                if (p == buf_len) break;

                if (0x20 <= Addr[ib] &&  0x7F > Addr[ib])
                {
                    if(!SUCCEEDED(StringCbPrintfExA(
                                        Pos, 
                                        Remain, 
                                        &Pos, 
                                        &Remain, 
                                        0, 
                                        "%c", 
                                        Addr[ib])))
                    {
                        log_err "StringCbPrintfEx() failed" log_end
					    break;
                    }
                }
                else
                {
                    if(!SUCCEEDED(StringCbPrintfExA(
                                        Pos, 
                                        Remain, 
                                        &Pos, 
                                        &Remain, 
                                        0,
                                        "%c",
                                        '.')))                                        
                    {
                        log_err "StringCbPrintfEx() failed" log_end
					    break;
                    }                
                }
                
                ++ib;
            }
            
			// add line dump string..
			StringCbPrintfA(line_dump, sizeof(line_dump), "%s   %s", print_buf, tmp);
			dump.push_back(line_dump);
            
			memset(print_buf, 0x00, sizeof(print_buf));
		}

		// add rest of dump
		StringCbPrintfA(line_dump, sizeof(line_dump), "%s", print_buf );
		dump.push_back(line_dump);
		
		return true;
	}

	return FALSE;
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
BOOL GetTimeStringA(OUT std::string& TimeString)
{
    __time64_t long_time=0;
    struct tm newtime={0};

    // Get time as 64-bit integer.
    _time64(&long_time);

    errno_t err=_localtime64_s(&newtime, &long_time ); 
    if (err)
    {
        log_err "_localtime64_s() failed" log_end
        return FALSE;
    }

    // e.g. 2009.07.06 23:04:33
    //
	CHAR buf[20]={0};
    if(!SUCCEEDED(StringCbPrintfA(
                        buf, 
                        20 * sizeof(CHAR), 
                        "%.4d.%.2d.%.2d_%.2d.%.2d.%.2d", 
                        newtime.tm_year + 1900,
                        newtime.tm_mon,
                        newtime.tm_mday,
                        newtime.tm_hour,
                        newtime.tm_min,
                        newtime.tm_sec)))
    {
        log_err "StringCbPrintfEx() failed" log_end
        return FALSE;
    }

	TimeString=buf;
	return TRUE;
}

BOOL GetTimeStringW(IN std::wstring& TimeString)
{
    __time64_t long_time=0;
    struct tm newtime={0};

    // Get time as 64-bit integer.
    //
    _time64(&long_time);

    errno_t err=_localtime64_s(&newtime, &long_time ); 
    if (err)
    {
        log_err "_localtime64_s() failed" log_end
        return FALSE;
    }

    // e.g. 2009.07.06 23:04:33
    //
	WCHAR buf[20]={0};
    if(!SUCCEEDED(StringCbPrintfW(
                        buf, 
                        20 * sizeof(WCHAR), 
                        L"%.4d.%.2d.%.2d_%.2d.%.2d.%.2d", 
                        newtime.tm_year + 1900,
                        newtime.tm_mon,
                        newtime.tm_mday,
                        newtime.tm_hour,
                        newtime.tm_min,
                        newtime.tm_sec)))
    {
        log_err "StringCbPrintfEx() failed" log_end
        return FALSE;
    }

	TimeString=buf;
	return TRUE;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_local_ip_list(_Out_ std::wstring& host_name, _Out_ std::vector<std::wstring>& ip_list)
{
	WSADATA wsaData={0};
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	DWORD NetbiosNameLen = 0;
	wchar_t* netbios_name = NULL;

	if(0 == GetComputerNameExW(ComputerNameNetBIOS, netbios_name, &NetbiosNameLen))
	{
		if(ERROR_MORE_DATA == GetLastError())
		{
			// GetComputerNameExW() 가 리턴하는 NetbiosNameLen 은 null 을 포함한다.
			netbios_name = (wchar_t*) malloc(NetbiosNameLen * sizeof(wchar_t));
			if (NULL == netbios_name) return false;

			if(0 == GetComputerNameExW(ComputerNameNetBIOS, netbios_name, &NetbiosNameLen))
			{
				log_err "GetComputerNameExW( ComputerNameNetBIOS ) failed, gle = %u", GetLastError() log_end

				free(netbios_name); netbios_name = NULL;
				WSACleanup();
				return false;
			}
			else
			{
				// enforce null terminate string.
				netbios_name[NetbiosNameLen] = 0x0000;
			}
		}
	}

	host_name = netbios_name;
	free(netbios_name); netbios_name = NULL;

	ADDRINFOW hints={0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	PADDRINFOW Result=NULL;
	if( 0 != GetAddrInfoW(host_name.c_str(), NULL, &hints, &Result) || NULL == Result )
	{
		log_err "GetAddrInfoW(host_name = %s) failed, wserr=0x%08x", host_name.c_str(), WSAGetLastError() log_end
		WSACleanup();
		return false;
	}

	for (PADDRINFOW p = Result; p != NULL; p = p->ai_next)
	{
		if (AF_INET == p->ai_family)
		{
			sockaddr_in* sa = (sockaddr_in*) p->ai_addr;			
			ip_list.push_back( MbsToWcsEx(inet_ntoa(sa->sin_addr)) );
		}            
	}

	FreeAddrInfoW(Result); 
	WSACleanup();
	return true;
}

/**
* @brief	http://support.microsoft.com/kb/131065/EN-US/
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
bool set_privilege(_In_z_ const wchar_t* privilege, _In_ bool enable)
{
	if ( is_nt_family( OsVersion() ) )
	{
		HANDLE hToken;
		if ( TRUE != OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken) )
		{
			if (GetLastError() == ERROR_NO_TOKEN)
			{
				if (ImpersonateSelf(SecurityImpersonation)	!= TRUE ) 
				{
					log_err "ImpersonateSelf( ) failed. gle=0x%08x", GetLastError() log_end
					return false;
				}

				if (TRUE != OpenThreadToken(GetCurrentThread(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,FALSE,&hToken))
				{
					log_err "OpenThreadToken() failed. gle=0x%08x", GetLastError() log_end
					return false;
				}
			}
			else
			{
				log_err "OpenThread() failed. gle=0x%08x", GetLastError() log_end
				return false;
			}
		}  

		TOKEN_PRIVILEGES tp = { 0 };		
		LUID luid = {0};
		DWORD cb = sizeof(TOKEN_PRIVILEGES);
		if(!LookupPrivilegeValue( NULL, privilege, &luid ))
		{		
			log_err "LookupPrivilegeValue() failed. gle=0x%08x", GetLastError() log_end
			CloseHandle(hToken);
			return false;
		}
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		if(enable) 
		{
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		} 
		else 
		{
			tp.Privileges[0].Attributes = 0;
		}
		
        if (FALSE == AdjustTokenPrivileges( hToken, FALSE, &tp, cb, NULL, NULL ))
        {
            DWORD gle=GetLastError();
		    if (gle != ERROR_SUCCESS)
		    {		
				log_err "AdjustTokenPrivileges() failed. gle=0x%08x", GetLastError() log_end
				CloseHandle(hToken);				
			    return false;
		    }		
        }        

		CloseHandle(hToken);
	}

	return true;
}

/**
* @brief	raise_privilege 가 true 인 경우 SE_DEBUG_NAME 권한을 획득한 후 프로세스를 오픈한다. 
			획득한 권한은 명시적으로 제거해줄때까지 계속 유지 된다. 
			SE_DEBUG_NAME 가 원래 있었을 수도 있었기 때문에 제거하면 문제가 생길 수 있고,
			이 함수가 리턴한 핸들에 I/O 가 발생할 때 오픈할 때 있던 SE_DEBUG_NAME 가 없어서 문제가 
			생길 수 있다 (빡신 버그 생성...)
* @param	
* @see		
* @remarks	
* @code		HANDLE process_handle = privileged_open_process(pid, PROCESS_ALL_ACCESS, true);
* @endcode	
* @return	
*/
HANDLE privileged_open_process(_In_ DWORD pid, _In_ DWORD rights, _In_ bool raise_privilege)
{
	HANDLE proc_handle = NULL;
	do 
	{
		// open the process
		proc_handle = OpenProcess(rights, FALSE, pid);
		if (NULL != proc_handle) break;		// 성공!

		// SeDebugPrivilege	권한을 활성화하고 재 시도 해본다. (권한이 없으면 안될 수도 있음)
		if (true != raise_privilege) break;	// 실패!

		if (true != set_privilege(SE_DEBUG_NAME, TRUE) ) break;
		
		// SeDebugPrivilege 를 얻었다면 다시 오픈 시도
		proc_handle = OpenProcess(rights, FALSE, pid);
		if (NULL == proc_handle)
		{
			//log_info "OpenProcess( pid = %u ) failed. gle = %u", pid, GetLastError() log_end
		}
		
	} while(false);

	return proc_handle;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_active_window_pid(_Out_ DWORD& pid, _Out_ DWORD& tid)
{
	HWND active = GetForegroundWindow();
	if (NULL == active) return false;

	tid = 0;
	tid = GetWindowThreadProcessId(active, &pid);
	return (0 != tid) ? true : false;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
DWORD	get_active_console_session_id()
{
	return WTSGetActiveConsoleSessionId();
}

/**
 * @brief	get session id of specified process.
 * @param	process_id 는 PROCESS_QUERY_INFORMATION 권한이 필요함
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool get_session_id_by_pid(_In_ DWORD process_id, _Out_ DWORD& session_id)
{
	if (!ProcessIdToSessionId(process_id, &session_id))
	{
		log_err "ProcessIdToSessionId( pid = %u ) failed. gle = %u", process_id, GetLastError() log_end
		return false;
	}

	return true;
}

/**
 * @brief	process_id 가 콘솔세션에서 실행중인 경우 true 리턴
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool	process_in_console_session(_In_ DWORD process_id)
{
	DWORD console_session = get_active_console_session_id();
	DWORD session_id = 0xffffffff;
	if (!get_session_id_by_pid(process_id, session_id)) 
	{
		log_err "get_session_id_by_pid() failed." log_end
		return false;
	}

	if (console_session == session_id) 
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief	pid 로 프로세스의 전체 이름을 구한다. (vista 이상)
			ZwQuerySystemInformation 처럼 length 를 0 을 넘겨주면 필요한 버퍼의 길이를...
			리턴해 주지 않는다. 큼직하게 버퍼 잡아서 호출해야 함
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	c:\dbg\sound.dll 형태 (Win32 format) string 리턴
**/
#if _WIN32_WINNT >= 0x0600	// after vista

std::wstring get_process_name_by_pid(_In_ DWORD process_id)
{
	raii_handle process_handle(
					privileged_open_process(process_id, PROCESS_QUERY_INFORMATION, true),
					raii_CloseHandle
					);
	if (NULL == process_handle.get()) return _null_stringw;

	// get size
	wchar_t*	name = NULL;
	DWORD		name_len = 20;//MAX_PATH;
	DWORD		ret = 0;
	
	for(int i = 0; i < 3; ++i)		// name buffer 를 두배씩 키우는것도 3회만 시도한다.
	{
		if (NULL != name) free(name);
		name = (wchar_t*) malloc((name_len + 1) * sizeof(wchar_t));
		if (NULL == name) return _null_stringw;

		ret = QueryFullProcessImageNameW(
					process_handle.get(), 
					0,	// Win32 Format
					name,
					&name_len
					);	
		if (ret > 0)
		{
			name[name_len] = 0x00;
			break;
		}
		else
		{
			DWORD gle = GetLastError();
			if (ret == 0 && ERROR_INSUFFICIENT_BUFFER == gle)
			{
				name_len *= 2;
				continue;
			}
			else 
			{
				log_err "QueryFullProcessImageName() failed. gle = %u", gle log_end
				return _null_stringw;
			}
		}		
	}

	std::wstring name_str = name;
	if (NULL != name) free(name); name = NULL;
	return name_str;	
}

#endif

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
void SetConsoleCoords(COORD xy)
{
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), xy);
}

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
COORD GetCurCoords(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.dwCursorPosition;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void write_log(_In_ LOG_TO_XXX log_to, _In_ DWORD log_level, _In_ const char* function, _In_ const char* format, ...)
{
	// 현재 모듈의 이름을 구하고 16글자만 잘라서 로그의 prefix 로 사용한다. 
	std::wstring module_file;
	get_current_module_file(module_file);
	
	char module_filea[16];
	StringCbPrintfA(module_filea, sizeof(module_filea), "%ws", module_file.c_str());
	
	// [DEBG] notepad.exe (100:1111), InternalFunction(), xxxxx
	std::stringstream log_stream;
	switch (log_level)
	{
	case LL_DEBG: 
		{
			log_stream << "[DEBG] " << WcsToMbsEx(module_file.c_str()) << " (" << GetCurrentProcessId() << ":" << GetCurrentThreadId() << "), " << function << "(), "; 
			break;
		}
	case LL_INFO: 
		{
			log_stream << "[INFO] " << WcsToMbsEx(module_file.c_str()) << " (" << GetCurrentProcessId() << ":" << GetCurrentThreadId() << "), " << function << "(), "; 
			break;
		}
	case LL_ERRR: 
		{
			log_stream << "[ERR ] " << WcsToMbsEx(module_file.c_str()) << " (" << GetCurrentProcessId() << ":" << GetCurrentThreadId() << "), " << function << "(), "; 
			break;
		}
	case LL_NONE:
	default:
		{		
			break;
		}
	}


	
	va_list args;
	char msg[4096];

	va_start(args, format);
	if(TRUE != SUCCEEDED(StringCchVPrintfA(msg, sizeof(msg), format, args))){ return; }
	va_end(args);

	log_stream << msg << std::endl;

	switch(log_to)
	{
	case LOG_TO_DEBUGGER:
		OutputDebugStringA(log_stream.str().c_str());
		break;
	case LOG_TO_CONSOLE:
		write_to_console(log_level, log_stream.str().c_str());
		break;
	default:
		//oops!
		break;
	}	
}


/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void write_to_console(_In_ DWORD log_level, _In_ const char* log_text)
{
	UNREFERENCED_PARAMETER(log_level);

	static HANDLE	_stdout_handle = INVALID_HANDLE_VALUE;
	static WORD		_old_color = 0x0000;
	
	//> initialization for console text color manipulation.
	if (INVALID_HANDLE_VALUE == _stdout_handle)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		_stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(_stdout_handle, &csbi);
		_old_color = csbi.wAttributes;
	}
		
	DWORD len = 0;
	WriteConsole(_stdout_handle, log_text, (DWORD)((strlen(log_text) + 1) * sizeof(char)), &len, NULL);	
	WriteConsole(_stdout_handle, "\n", (DWORD)strlen("\n"), &len, NULL);
	
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), _old_color);
}

/** ---------------------------------------------------------------------------
    \brief  실행 가능한 파일인 경우 TRUE 리턴

    \param  path            exe 파일의 full path    
            type            file type
    \return         
            성공시 TRUE 리턴
            실패시 FALSE 리턴, 
                
                
    \code
    \endcode        
-----------------------------------------------------------------------------*/
BOOL IsExecutableFile(LPCTSTR path, IMAGE_TYPE& type)
{
    type = IT_NORMAL;

    HANDLE hFile = CreateFile(
                    (LPCTSTR)path, 
                    GENERIC_READ, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, 
                    OPEN_EXISTING, 
                    FILE_ATTRIBUTE_NORMAL, 
                    NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        log_err "access denied or invalid path, %ws", path log_end
        return FALSE;
    }
    SmrtHandle sfFile(hFile);

    // check file size 
    // 
    LARGE_INTEGER fileSize;
    if (TRUE != GetFileSizeEx(hFile, &fileSize))
    {
        log_err "%ws, can not get file size, errorcode=0x%08x", path, GetLastError() log_end
        return FALSE;
    }
    if (sizeof(IMAGE_DOS_HEADER) > fileSize.QuadPart) return TRUE;

    
    HANDLE hImageMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (NULL == hImageMap){return FALSE;}
    SmrtHandle sfMap(hImageMap);

    PBYTE ImageView = (LPBYTE) MapViewOfFile(
                            hImageMap, 
                            FILE_MAP_READ, 
                            0, 
                            0, 
                            0
                            );
    if(ImageView == NULL){return FALSE;}
    SmrtView sfView(ImageView);


    //
    // parse PE header
    //

    PIMAGE_DOS_HEADER idh = (PIMAGE_DOS_HEADER)ImageView;
    if(idh->e_magic != IMAGE_DOS_SIGNATURE) return TRUE;


    // check DOS file size
    // 
    DWORD dosSize = (idh->e_cp * 512);
    if (dosSize > fileSize.QuadPart) 
    {
        log_err "%ws, invalid file size, size=%lu", path, fileSize.QuadPart log_end
        return TRUE;
    }


    // check e_lfanew offset
    // 
    DWORD_PTR p = (DWORD_PTR)idh + idh->e_lfanew;
    if (TRUE == IsBadReadPtr((void*)p, sizeof(DWORD)/* sizeof (IMAGE_NT_SIGNATURE) */))
    {
        log_err
            "%s, 0x%08x, invalid e_lfanew offset, idh=0x%08x, e_lfanew=0x%08x", 
            path, idh, idh->e_lfanew 
        log_end        
        return TRUE;    
    }


    PIMAGE_NT_HEADERS inh = (PIMAGE_NT_HEADERS) ((DWORD_PTR)idh + idh->e_lfanew);
    if (IMAGE_NT_SIGNATURE != inh->Signature) return TRUE;


    WORD subsys = inh->OptionalHeader.Subsystem;
    WORD characteristics = inh->FileHeader.Characteristics;
    
    // characteristics check
    //  - 
    ///if ((characteristics & IMAGE_FILE_32BIT_MACHINE) != IMAGE_FILE_32BIT_MACHINE) return FALSE;
    if ((characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) != IMAGE_FILE_EXECUTABLE_IMAGE) return TRUE;   // not executable
    
    if ((characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL) 
    { 
        type = IT_DLL;
        return TRUE;
    }

    // never called.. ???
    // if ((characteristics & IMAGE_FILE_SYSTEM) == IMAGE_FILE_SYSTEM){OutputDebugStringW(" IMAGE_FILE_SYSTEM");}

    
    // subsystem check
    // 
    if ((subsys & IMAGE_SUBSYSTEM_NATIVE) == IMAGE_SUBSYSTEM_NATIVE)
    {
        type = IT_NATIVE_APP;
        return TRUE;
    }

    if ((subsys & IMAGE_SUBSYSTEM_WINDOWS_GUI) == IMAGE_SUBSYSTEM_WINDOWS_GUI)
    {
        type = IT_EXE_GUI;
        return TRUE;
    }
    if ((subsys & IMAGE_SUBSYSTEM_WINDOWS_CUI) == IMAGE_SUBSYSTEM_WINDOWS_CUI)
    {
        type = IT_EXE_CUI;
        return TRUE;
    }        
        
    return TRUE;
}

/** ---------------------------------------------------------------------------
    \brief  

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
LPCWSTR  FileTypeToString(IMAGE_TYPE type)
{
    switch(type)
    {
    case IT_DLL:
        return L"DLL";
    case IT_EXE_GUI:
        return L"GUI APP";
    case IT_EXE_CUI:
        return L"CUI APP";
    case IT_NATIVE_APP: 
        return L"NATIVE";    
    case IT_NORMAL:
    default:
        return L"NORMAL FILE";
    }
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool 
bin_to_hexa(
	_In_ UINT32 code_size, 
	_In_ const UINT8* code, 
	_In_ bool upper_case, 
	_Out_ std::string& hex_string
	)
{
	_ASSERTE(NULL != code);
	if (NULL == code) return false;

	//> allocate memory
	UINT32			buffer_size = ((code_size * 2) + 1) * sizeof(char);
	raii_char_ptr	hex_buf( (char*)malloc(buffer_size), raii_free);
	if (NULL == hex_buf.get()) return false;
	RtlZeroMemory(hex_buf.get(), buffer_size);

	char*	pos = hex_buf.get();
	size_t	remain = buffer_size;
	for(UINT32 i = 0; i < code_size; ++i)
	{	
		HRESULT hr = StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%02x", code[i]);
		if(!SUCCEEDED(hr))
		{
			log_err 
				"StringCbPrintfExW(pos = 0x%p, remain = %u), hr = 0x%08x",
				pos, remain, hr
			log_end
			return false;
		}
	}
		
	hex_string = hex_buf.get();
	if (true == upper_case)
	{
		to_upper_string(hex_string);
	}
	else
	{
		to_lower_string(hex_string);
	}

	return true;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool 
bin_to_hexw(
	_In_ UINT32 code_size, 
	_In_ const UINT8* code, 
	_In_ bool upper_case, 
	_Out_ std::wstring& hex_string
	)
{
	std::string hexa;
	if (true != bin_to_hexa(code_size, code, upper_case, hexa)) 
	{
		return false;
	}

	hex_string = MbsToWcsEx(hexa.c_str());
	return true;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool str_to_int32(_In_ const char* int32_string, _Out_ INT32& int32_val)
{
	if (NULL == int32_string) return false;
	
	errno = 0;
	int32_val = strtol(int32_string, NULL, 10);
	if (ERANGE == errno) { return false; }
	
	return true;
}

bool str_to_uint32(_In_ const char* uint32_string, _Out_ UINT32& uint32_val)
{
	if (NULL == uint32_string) return false;

	//> 문자열 양쪽 공백 모두 제거, '-' 로 시작하는 경우 에러처리
	std::string str = uint32_string;
	trima(str);
	if (str[0] == '-') return false;
	
	errno = 0;
	uint32_val = strtoul(str.c_str(), NULL, 10);
	if (ERANGE == errno) { return false; }
	return true;	
}

bool str_to_int64(_In_ const char* int64_string, _Out_ INT64& int64_val)
{
	if (NULL == int64_string) return false;

	errno = 0; 
	int64_val = _strtoi64(int64_string, NULL, 10);
	if (ERANGE == errno) 
	{ 
		// msdn 에는 ERANGE 를 리턴한다는 내용이 없고, _I64_MAX, _I64_MIN 를 리턴한다고 하는데,
		// 실제로 범위를 넘는 input 을 넣어보면 ERANGE 를 리턴함을 확인했음
		//
		// test_str_to_xxx() 테스트 확인		
		return false; 
	}

	if (EINVAL == errno)
	{		
		return false;
	}

	return true;
}

bool str_to_uint64(_In_ const char* uint64_string, _Out_ UINT64& uint64_val)
{
	if (NULL == uint64_string) return false;

	//> 문자열 양쪽 공백 모두 제거, '-' 로 시작하는 경우 에러처리
	std::string str = uint64_string;
	trima(str);
	if (str[0] == '-') return false;

	errno = 0; 
	uint64_val = _strtoui64(str.c_str(), NULL, 10);
	if (ERANGE == errno) 
	{ 
		// msdn 에는 ERANGE 를 리턴한다는 내용이 없고, _UI64_MAX 를 리턴한다고 하는데,
		// 실제로 범위를 넘는 input 을 넣어보면 ERANGE 를 리턴함을 확인했음
		//
		// test_str_to_xxx() 테스트 확인
		errno = 0; 
		return false; 
	}

	if (EINVAL == errno)
	{
		errno = 0;
		return false;
	}


	return true;
}

bool wstr_to_int32(_In_ const wchar_t* int32_string, _Out_ INT32& int32_val)
{
	if (NULL == int32_string) return false;
	return str_to_int32(WcsToMbsEx(int32_string).c_str(), int32_val);	
}

bool wstr_to_uint32(_In_ const wchar_t* uint32_string, _Out_ UINT32& uint32_val)
{
	if (NULL == uint32_string) return false;
	return str_to_uint32(WcsToMbsEx(uint32_string).c_str(), uint32_val);
}

bool wstr_to_int64(_In_ const wchar_t* int64_string, _Out_ INT64& int64_val)
{
	if (NULL == int64_string) return false;
	return str_to_int64(WcsToMbsEx(int64_string).c_str(), int64_val);
}

bool wstr_to_uint64(_In_ const wchar_t* uint64_string, _Out_ UINT64& uint64_val)
{
	if (NULL == uint64_string) return false;
	return str_to_uint64(WcsToMbsEx(uint64_string).c_str(), uint64_val);
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
UINT16 swap_endian_16(_In_ UINT16 value)
{
	return (value >> 8) | (value << 8);
}

UINT32 swap_endian_32(_In_ UINT32 value)
{
	return	( value >> 24)				| 
			((value << 8) & 0x00FF0000) | 
			((value >> 8) & 0x0000FF00) | 
			( value << 24);
}

UINT64 swap_endian_64(_In_ UINT64 value)
{
	return  ( value >> 56)						|
            ((value << 40) & 0x00FF000000000000)|
            ((value << 24) & 0x0000FF0000000000)|
            ((value << 8 ) & 0x000000FF00000000)|
            ((value >> 8 ) & 0x00000000FF000000)|
            ((value >> 24) & 0x0000000000FF0000)|
            ((value >> 40) & 0x000000000000FF00)|
            ( value << 56);
}


/**	---------------------------------------------------------------------------
	\brief	
                http://msdn.microsoft.com/en-us/library/ms724833(v=VS.85).aspx

                Operating system	                Version number	dwMajorVersion	dwMinorVersion	Other
				Windows 8	                        6.2	            6	            2	            OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
				Windows Server 2012                 6.2             6               2				OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
                Windows 7	                        6.1	            6	            1	            OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
                Windows Server 2008 R2	            6.1		        6	            1	            OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
                Windows Server 2008	                6.0		        6	            0	            OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
                Windows Vista	                    6.0		        6	            0	            OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
                Windows Server 2003 R2	            5.2		        5	            2	            GetSystemMetrics(SM_SERVERR2) != 0
                Windows Home Server	                5.2		        5	            2	            OSVERSIONINFOEX.wSuiteMask & VER_SUITE_WH_SERVER
                Windows Server 2003	                5.2		        5	            2	            GetSystemMetrics(SM_SERVERR2) == 0
                Windows XP Professional x64 Edition	5.2		        5	            2	            (OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION) && (SYSTEM_INFO.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
                Windows XP	                        5.1		        5	            1	            Not applicable
                Windows 2000	                    5.0		        5	            0	            Not applicable
	\param	
	\return			
	\code
	\endcode		
-----------------------------------------------------------------------------*/
WORD OsVersion(void)
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.
	//
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
	if( bOsVersionInfoEx == FALSE )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )  return 0;
	}

	switch (osvi.dwPlatformId)
	{
		// Test for the Windows NT product family.
	case VER_PLATFORM_WIN32_NT:
		if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2 )
        {
            if(osvi.wProductType != VER_NT_WORKSTATION)
            {
                return OSTYPE_WIN_SERVER_2012;
            }
            else
            {
                return OSTYPE_WIN_8;
            }
        }
		
        if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 )
        {
            if(osvi.wProductType != VER_NT_WORKSTATION)
            {
                return OSTYPE_WIN_SERVER_2008_R2;
            }
            else
            {
                return OSTYPE_WIN_7;
            }
        }

		if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
        {
            if (osvi.wProductType == VER_NT_WORKSTATION)
            {
                return OSTYPE_WIN_VISTA;
            }
            else
            {
                return OSTYPE_WIN_SERVER_2008;
            }
        }		
        if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
        {
            SYSTEM_INFO siSysInfo;
            GetSystemInfo(&siSysInfo);

            if (osvi.wProductType == VER_NT_WORKSTATION && 
                siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            {
                return OSTYPE_WIN_XP_64;
            }
            else if (GetSystemMetrics(SM_SERVERR2) != 0)
            {
                return OSTYPE_WIN_2003;
            }
            else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER)
            {
                return OSTYPE_WIN_HOME_SERVER;
            }
            else 
                return OSTYPE_WIN_XP;            
        }	

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
			return OSTYPE_WIN_XP ;

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
			return OSTYPE_WIN_2000 ;

		if ( osvi.dwMajorVersion <= 4 )
			return OSTYPE_WIN_NT ;			

	case VER_PLATFORM_WIN32_WINDOWS:

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			/*
			printf ("Microsoft Windows 95 ");
			if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
				printf("OSR2 " );
			*/
			return OSTYPE_WIN_95 ;
		} 

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			
			// printf ("Microsoft Windows 98 ");
			
			if ( osvi.szCSDVersion[1] == 'A' ) {
				return OSTYPE_WIN_SE ;
				// printf("SE " );
			}
			
			return OSTYPE_WIN_98 ;
		} 

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
		{
			return OSTYPE_WIN_ME ;
			/*
			printf ("Microsoft Windows Millennium Edition\n");
			*/
		} 
		break;

	case VER_PLATFORM_WIN32s:

		/*
		printf ("Microsoft Win32s\n");
		*/
		break;
	}

	return 0;
}


/**
* @brief	cpu 정보를 수집한다. 
* @param	
* @see		
* @remarks  xp sp3 이상에서만 사용 가능
* @code		
* @endcode	
* @return	
*/

DWORD CountSetBits(ULONG_PTR bitMask)
{
    // Helper function to count set bits in the processor mask.
    // 

    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
    DWORD i;
    
    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}

BOOL WUGetProcessorInfo(IN OUT WU_PROCESSOR_INFO& CpuInfo)
{
    BOOL done=FALSE;
    DWORD ReturnedLength=0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buf = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    PCACHE_DESCRIPTOR Cache = NULL;

    while (FALSE == done)
    {
        done = GetLogicalProcessorInformation(buf, &ReturnedLength);
        if (TRUE != done)
        {
            if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
            {
                return FALSE;
            }

            if (NULL != buf) free(buf);
            buf = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(ReturnedLength);
            if (NULL == buf) return FALSE;
        }
    }    

    ptr = buf;
    DWORD offset=0;
    while (offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= ReturnedLength)
    {
        switch (ptr->Relationship) 
        {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            CpuInfo.NumaNodeCount++;
            break;

        case RelationProcessorCore:
            CpuInfo.PhysicalCpuCoreCount++;

            // A hyperthreaded core supplies more than one logical processor.
            CpuInfo.LogicalCpuCount += CountSetBits(ptr->ProcessorMask);
            break;

        case RelationCache:
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
            Cache = &ptr->Cache;
            if (Cache->Level == 1)
            {
                CpuInfo.L1Cache++;
            }
            else if (Cache->Level == 2)
            {
                CpuInfo.L2Cache++;
            }
            else if (Cache->Level == 3)
            {
                CpuInfo.L3Cache ++;
            }
            break;

        case RelationProcessorPackage:
            // Logical processors share a physical package.
            CpuInfo.PhysicalCpuPackageCount++;
            break;

        default:            
            break;
        }
        offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    free(buf);
    return TRUE;
}