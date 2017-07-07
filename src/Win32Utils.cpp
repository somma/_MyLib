/**
 * @file    Windows API wrapper and utility routines.
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011/08/26 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"

//	
//	std
//
#include <stdio.h>
#include <tchar.h>
#include <random>

#include "Win32Utils.h"

#include <set>
#include <errno.h>
#include <io.h>			// _setmode()
#include <fcntl.h>		// _O_U8TEXT, ...
#include <VersionHelpers.h>
#include <time.h>
#include <Shellapi.h>
#include <Shlobj.h>
#include <Psapi.h>
#include <guiddef.h>

#include "ResourceHelper.h"
#include "gpt_partition_guid.h"
#include "AirCrypto.h"

#include <sddl.h>
#pragma comment(lib, "Advapi32.lib")

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")


//
//	create_process_as_login_user()
// 
#include <TLHELP32.H>
#include <userenv.h>
#pragma comment (lib, "userenv.lib")
#include <Wtsapi32.h>
#pragma comment (lib, "Wtsapi32.lib")


/// @brief	int type �������� �����Ѵ�.
int get_random_int(_In_ int min, _In_ int max)
{
	std::random_device seed;
	std::default_random_engine re(seed());
	std::uniform_int_distribution<uint32_t> range(min, max);
	return range(re);
}


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

/// @brief	
LPCWSTR FAT2Str(IN FATTIME& fat)
{
    FILETIME	ft={0};
	
	if (0x00 != fat.usDate && 0x00 != fat.usTime)
	{
		DosDateTimeToFileTime(fat.usDate, fat.usTime, &ft);
	}

	return FT2Str(ft);
}

/// @brief  
uint64_t file_time_to_int(_In_ const PFILETIME file_time)
{
	return ((LARGE_INTEGER*)file_time)->QuadPart;
}

/// @brief
void 
int_to_file_time(
	_In_ uint64_t file_time_int, 
	_Out_ PFILETIME const file_time
	)
{
	_ASSERTE(nullptr != file_time);
	if (nullptr == file_time) return;

	file_time->dwLowDateTime = ((PLARGE_INTEGER)&file_time_int)->LowPart;
	file_time->dwHighDateTime = ((PLARGE_INTEGER)&file_time_int)->HighPart;
}

/// @brief  ftl, ftr ���� ���� �ʴ����� �����Ѵ�. 
int64_t 
file_time_delta_sec(
	_In_ const PFILETIME ftl, 
	_In_ const PFILETIME ftr
	)
{
	return (llabs( file_time_to_int(ftl) - file_time_to_int(ftr) ) / _file_time_to_sec);
}

/// @brief	ftl, ftr ���� ���� �ϴ����� �����Ѵ�. 
int64_t 
file_time_delta_day(
	_In_ const PFILETIME ftl, 
	_In_ const PFILETIME ft2
	)
{
	return (llabs( file_time_to_int(ftl) - file_time_to_int(ft2) ) / _file_time_to_day);
}

/// @brief file_time �� day ��ŭ ���� ����Ÿ���� �����Ѵ�.
FILETIME 
add_day_to_file_time(
	_In_ const PFILETIME file_time, 
	_In_ int32_t day
	)
{
	if (day == 0) return *file_time;

	uint64_t file_time_uint64_t = file_time_to_int(file_time) + (day * _file_time_to_day);

	FILETIME added_file_time;
	int_to_file_time(file_time_uint64_t, &added_file_time);
	return added_file_time;
}

/// @brief  FILETIME to `yyyy-mm-dd hh:mi:ss` string representation.
std::string 
file_time_to_str(
	_In_ const PFILETIME file_time, 
	_In_ bool localtime,
	_In_ bool show_misec
	)
{
    SYSTEMTIME utc;
    FileTimeToSystemTime(file_time, &utc);    
    return sys_time_to_str(&utc, localtime, show_misec);
}

/// @brief  FILETIME to `yyyy-mm-dd hh:mi:ss` string representation.
std::string 
file_time_to_str(
	_In_ uint64_t file_time,
	_In_ bool localtime,
	_In_ bool show_misec
	)
{
	FILETIME ft;
	int_to_file_time(file_time, &ft);
	return file_time_to_str(&ft, localtime, show_misec);
}	

/// @brief  SYSTEMTIME (UTC) to `yyyy-mm-dd hh:mi:ss` string representation.
/// 
std::string 
sys_time_to_str(
	_In_ const PSYSTEMTIME sys_time, 
	_In_ bool localtime, 
	_In_ bool show_misec
	)
{
    char buf[32];

    SYSTEMTIME local;
    PSYSTEMTIME time = sys_time;

    if (true == localtime)
    {
        SystemTimeToTzSpecificLocalTime(NULL, sys_time, &local);
        time = &local;
    }

	if (!show_misec)
	{
		StringCbPrintfA(buf, sizeof(buf),
						"%04u-%02u-%02u %02u:%02u:%02u",
						time->wYear,
						time->wMonth,
						time->wDay,
						time->wHour,
						time->wMinute,
						time->wSecond);
	}
	else
	{
		StringCbPrintfA(buf, sizeof(buf),
						"%04u-%02u-%02u %02u:%02u:%02u %u",
						time->wYear,
						time->wMonth,
						time->wDay,
						time->wHour,
						time->wMinute,
						time->wSecond, 
						time->wMilliseconds);
	}
    return std::string(buf);
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
	if (NULL == file_path) return false;

	WIN32_FILE_ATTRIBUTE_DATA info = {0};

	//
	// CreateFile()�� �ƴ� GetFileAttributesEx()�� �̿��ϸ� ������ �ٸ� process�� ���� lock�Ǿ� �־
	// ���� ���翩�θ� ��Ȯ�� üũ�� �� �ִ�.
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

/// @brief  `file_path` �� �����ϰ�, directory �̸� true ����
///         `file_path` �� ���ų�, �����̸� false ����
bool is_dir(_In_ const wchar_t* file_path)
{   
    WIN32_FILE_ATTRIBUTE_DATA info = { 0 };
    if (TRUE == GetFileAttributesExW(file_path, GetFileExInfoStandard, &info))
    {
        // ������ �����ϰ�, ���丮��� true
        if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            return true;
        }
    }

    return false;
}

/// @brief  `file_path` ������ �����ϰ�, ���丮�� �ƴ� �����̸� true ����
///         `file_path` �� �������� �ʰų� ���丮�� ��� false ����
bool is_file(_In_ const wchar_t* file_path)
{
    WIN32_FILE_ATTRIBUTE_DATA info = { 0 };
    if (TRUE == GetFileAttributesExW(file_path, GetFileExInfoStandard, &info))
    {
        // ������ �����ϰ�, ���丮�� �ƴϸ� true
        if (!(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            return true;
        }
    }

    return false;
}

/**
 * @brief	file_handle �� file path �� ���ϴ� �Լ�
 * @param	
 * @see		http://msdn.microsoft.com/en-us/library/windows/desktop/aa366789(v=vs.85).aspx
 * @see		https://msdn.microsoft.com/en-us/library/aa364962(v=vs.85).aspx
 * @remarks	NtQueryObject() �� ����ϴ°� �� ���� �� ���⵵ ��, ���� ������ �߻� �� ���� ������ ������
 * @remarks Ȯ���غ��� ����
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

    // �ý��ۿ� ���εǾ��ִ� ����̺� ����� ���Ѵ�. 
    // 
	// 0               4               8               12
	// +---+---+---+---+---+---+---+---+---+---+---+---+
	// | A | : | \ |NUL| C | : | \ |NUL| D | : | \ |NUL|
	//  "A:\"           "C:\"           "D:\"
	// 
	// ���ε� ����̺긦 ��Ÿ���� ���ڿ� ���۴� 
	// 26 (���ĺ�) * 4 (����̺� ��) = 104 ����Ʈ�� �����
	// �����ڵ��� ��� 208����Ʈ 	
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
	to_lower_string(nt_namel);			// �ҹ��ڷ� ��� ��ȯ
	
	for (DWORD i = 0; i < length / 4; ++i)
	{
		// C:  --> \Device\HarddiskVolume1 ���� ������ ��ȸ 
		// QueryDosDeviceW() ȣ�� �� drive name �� �������� '\' ����� ��		
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

		// nt_name �� device_name �κ��� ��ġ�ϴ��� ��
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

/// @brief  ����Ʈ �� disk �� number ����� ���Ѵ�. 
/// 
///         ȹ���� disk number �� Diskdevice �� ���� ��� NTFS �� ������ �ʰ�
///         I/O �� �����ϴµ� ����� �� �ִ�. 
///             - CreateFile( L"\\\\.\\\Physicaldisk[disk number]" ) ���·� ����� �� ����
///             - `Win32 Device Namespaces` �̱⶧���� file system �� ��ġ�� �ʰ�, io �� �� �� ����
///
///         ������ I/O �뵵�� ����ϴ� ��� 
///             - `\\physicaldisk[disk number]` ���°� �ƴ϶� 
///             - `\\.\c:` ���·ε� ������ �� �ֱ⶧���� ��ư� disk number �� ���� �ʿ䰡 ����. 
///         
bool get_disk_numbers(_Out_ std::vector<uint32_t>& disk_numbers)
{
    // �ý��ۿ� ���εǾ��ִ� ����̺� ����� ���Ѵ�. 
    // 
	// 0               4               8               12
	// +---+---+---+---+---+---+---+---+---+---+---+---+
	// | A | : | \ |NUL| C | : | \ |NUL| D | : | \ |NUL|
	//  "A:\"           "C:\"           "D:\"
	// 
	// ���ε� ����̺긦 ��Ÿ���� ���ڿ� ���۴� 
	// 26 (���ĺ�) * 4 (����̺� ��) = 104 ����Ʈ�� �����
	// �����ڵ��� ��� 208����Ʈ 	
	wchar_t drive_string[128 + 1] = {0};
	DWORD length = GetLogicalDriveStringsW(128, drive_string);
	if ( 0 == length)
	{
		log_err 
			"GetLogicalDriveStringsW(), gle = %u", GetLastError()
		log_end
		return false;
	}

    // �ϳ��� ��ũ�� �������� ��Ƽ��(����)�� �����Ǿ� ���� �� �ֱ⶧����
    // �̹� ���� disk_number ���� Ȯ���Ѵ�.
    std::set<uint32_t> disk_numberz;

    for (DWORD i = 0; i < length / 4; ++i)
    {
        wchar_t* dos_device_name = &(drive_string[i * 4]);
        dos_device_name[1] = 0x0000;
        std::wstringstream path;
        path << L"\\\\.\\" << dos_device_name << ":";
        HANDLE hFile = CreateFileW(
                            path.str().c_str(), //L"\\\\.\\c:", 
                            GENERIC_READ, 
                            FILE_SHARE_READ | FILE_SHARE_WRITE, 
                            NULL, 
                            OPEN_EXISTING,  // for device or file, only if exists.
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            // cdrom �� ����ְų�, network fs �̰ų�,...
            // �������� ��Ȳ���� ���� �� ����
            log_err 
                "CreateFile( %ws ) failed. gle = %u", 
                path.str().c_str(), 
                GetLastError() 
            log_end;
            continue;
        }

        STORAGE_DEVICE_NUMBER sdn = { 0 };
        DWORD bytes_returned = 0;

        // disk number �� �˾Ƴ��� ���ؼ� ȣ���ϴ°��̹Ƿ�
        // hFile ����̽��� ���� ��� volume extents ����� ������ �ʿ����
        // �ϳ��� �������� �ȴ�. 
        // 
        // :�����:
        // use IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS
        //      device number, extent ������ ���ö�
        // 
        // use IOCTL_DISK_GET_DRIVE_LAYOUT_EX  
        //      ����̽��� ��Ƽ�� ������ ���ö� 


        bool ioctrl_succeeded = true;
        if (!DeviceIoControl(
                    hFile,
                    IOCTL_STORAGE_GET_DEVICE_NUMBER,
                    NULL,
                    0,
                    (LPVOID)&sdn,
                    sizeof(sdn),
                    (LPDWORD)&bytes_returned,
                    NULL))
        {
            DWORD gle = GetLastError();
            if (ERROR_MORE_DATA != gle)
            {
                log_err 
                    "DeviceIoControl( IOCTL_STORAGE_GET_DEVICE_NUMBER ) failed. device = %ws, gle = %u", 
                    path.str().c_str(),
                    gle 
                log_end;
                ioctrl_succeeded = false;
            }
        }

        if (true == ioctrl_succeeded)
        {
            if (sdn.DeviceType == FILE_DEVICE_DISK)
            {
                if (disk_numberz.end() == disk_numberz.find(sdn.DeviceNumber))
                {
                    disk_numberz.insert(sdn.DeviceNumber);

                    disk_numbers.push_back(sdn.DeviceNumber);
                    log_dbg "disk number = %u, found.", sdn.DeviceNumber log_end
                }
            }
            
            //uint8_t buf[512] = { 0x00 };
            //if (!ReadFile(hFile, buf, sizeof(buf), &bytes_returned, NULL))
            //{
            //    log_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
            //}
            //else
            //{
            //    std::vector<std::string> dumps;
            //    dump_memory(buf, sizeof(buf), dumps);
            //    for (auto line : dumps)
            //    {
            //        log_info "%s", line.c_str() log_end;
            //    }
            //}
        }
        CloseHandle(hFile);
    }

    return true;
}

/// @brief  
const char* partition_style_to_str(_In_ DWORD partition_style)
{
    switch (partition_style)
    {
    case PARTITION_STYLE_MBR: return "MBR";
    case PARTITION_STYLE_GPT: return "GPT";
    case PARTITION_STYLE_RAW: return "RAW";
    default: 
        return "UNKNOWN";
    }
}
 
/// @brief  
const char* gpt_partition_type_to_str(_In_ GUID& partition_type)
{
    if (IsEqualGUID(partition_type, PARTITION_BASIC_DATA_GUID))
    {
        return "PARTITION_BASIC_DATA_GUID";
    } else if (IsEqualGUID(partition_type, PARTITION_ENTRY_UNUSED_GUID))
    {
        return "PARTITION_ENTRY_UNUSED_GUID";
    }
    else if (IsEqualGUID(partition_type, PARTITION_SYSTEM_GUID))
    {
        return "PARTITION_SYSTEM_GUID";
    }
    else if (IsEqualGUID(partition_type, PARTITION_MSFT_RESERVED_GUID))
    {
        return "PARTITION_MSFT_RESERVED_GUID";
    }
    else if (IsEqualGUID(partition_type, PARTITION_LDM_METADATA_GUID))
    {
        return "PARTITION_LDM_METADATA_GUID";
    }
    else if (IsEqualGUID(partition_type, PARTITION_LDM_DATA_GUID))
    {
        return "PARTITION_LDM_DATA_GUID";
    }
    else if (IsEqualGUID(partition_type, PARTITION_MSFT_RECOVERY_GUID))
    {
        return "PARTITION_MSFT_RECOVERY_GUID";
    }
    else
        return "UNKNOWN GUID";
}

/// @brief  info._disk_number ��ũ�� VBR (Volume Boot Record) �� VBR ������ ���Ѵ�.
///
///         disk �� �������� ����(��Ƽ��)���� ������, �� ������ ���� VBR �� ������. 
///         MBR ��Ƽ���� ��� �� ��Ƽ�� 4��, GPT �� 128��(�³�?)
///         
/// @return ������ true, ���н� false
bool get_disk_volume_info(_Inout_ disk_volume_info& info)
{
    std::wstringstream path;
    path << L"\\\\.\\PhysicalDrive" << info._disk_number;

    // open handle for disk as `WIN32 Device Namespace` foramt.
    HANDLE disk = CreateFileW(
                        path.str().c_str(),
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,  // for device or file, only if exists.
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (INVALID_HANDLE_VALUE == disk)
    {
        log_err
            "CreateFile( %ws ) failed. gle = %u",
            path.str().c_str(),
            GetLastError());
        return false;
    }
    raii_handle handle_guard(disk, raii_CloseHandle);

    DWORD bytes_returned = 0;
    uint32_t layout_info_size = sizeof(DRIVE_LAYOUT_INFORMATION_EX) * 4;
    PDRIVE_LAYOUT_INFORMATION_EX layout_info = (PDRIVE_LAYOUT_INFORMATION_EX)malloc(layout_info_size);
    
    for (;;)
    {
        if (NULL == layout_info)
        {
            log_err
                "insufficient rsrc for DRIVE_LAYOUT_INFORMATION_EX buffer. device = %ws",
                path.str().c_str());

            return false;
        }

        if (!DeviceIoControl(
                    disk,
                    IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                    NULL,
                    0,
                    (LPVOID)layout_info,
                    layout_info_size,
                    &bytes_returned,
                    NULL))
        {
            if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
                layout_info_size *= 2;
                layout_info = (PDRIVE_LAYOUT_INFORMATION_EX)realloc(layout_info, layout_info_size);
                continue;
            }
            else
            {
                log_err
                    "DeviceIoControl( IOCTL_DISK_GET_DRIVE_LAYOUT_EX ) failed. "\
                    "device = %ws, gle = %u, bytes_returned = %u",
                    path.str().c_str(),
                    GetLastError(),
                    bytes_returned);
                return false;
            }
        }
        else
        {
            // ok. we got!
            break;
        }
    }
    raii_void_ptr buf_guard(layout_info, raii_free);

    //
    // ok. DeviceIoControl() succeeded.
    // 
    log_dbg
        "disk = %ws, partition style = %s, partition count = %u, ",
        path.str().c_str(),
        partition_style_to_str(layout_info->PartitionStyle),        
        layout_info->PartitionCount);

    for (DWORD i = 0; i < layout_info->PartitionCount; ++i)
    {
        vbr_info vbr = { 0 };

        PPARTITION_INFORMATION_EX pi = &layout_info->PartitionEntry[i];
        vbr.offset = pi->StartingOffset;
        vbr.partition_length = pi->PartitionLength;
        vbr.partition_number = pi->PartitionNumber;
        vbr.rewrite_partition = (TRUE == pi->RewritePartition) ? true : false;
        
        if (pi->PartitionStyle == PARTITION_STYLE_MBR)
        {
            vbr.is_mbr = true;
            PPARTITION_INFORMATION_MBR mbr = &pi->Mbr;
            if (TRUE == mbr->BootIndicator)
            {
                vbr.is_boot_partition = true;
            }
            vbr.recognized = (TRUE == mbr->RecognizedPartition) ? true : false;
            
            log_dbg
                "    [%u/%u] style = MBR, recognized = %s, boot = %s, offset = 0x%llx, length = %llu, number = %u",
                i + 1,
                layout_info->PartitionCount,
                (vbr.recognized) ? "true" : "false",
                vbr.is_boot_partition ? "true" : "false",
                vbr.offset.QuadPart,
                vbr.partition_length.QuadPart,
                vbr.partition_number);
        }
        else
        {
            vbr.is_mbr = false;
            PPARTITION_INFORMATION_GPT gpt = &pi->Gpt;
            if (TRUE == IsEqualGUID(PARTITION_SYSTEM_GUID, gpt->PartitionType))
            {
                vbr.is_boot_partition = true;
            }

            vbr.recognized = true;
            
            log_dbg
                "    [%u/%u] style = GPT, recognized = true, boot = %s, offset = 0x%llx, length = %llu, number = %u, type = %s",
                i + 1,
                layout_info->PartitionCount,
                vbr.is_boot_partition ? "true" : "false",
                vbr.offset.QuadPart,
                vbr.partition_length.QuadPart,
                vbr.partition_number, 
                gpt_partition_type_to_str(gpt->PartitionType)
                );
        }

        info._vbrs.push_back(vbr);
    }

    return true;
}

/// @brief  sample code about using IOCTL_DISK_GET_DRIVE_LAYOUT_EX
///         disk �� ��Ƽ�� ������ �����Ѵ�. 
bool dump_all_disk_drive_layout()
{
    std::vector<uint32_t> disk_numbers;
    bool ret = get_disk_numbers(disk_numbers);
    if (true != ret)
        return false;

    DWORD bytes_returned = 0;

    for (auto disk_number : disk_numbers)
    {
        std::wstringstream path;
        path << L"\\\\.\\PhysicalDrive" << disk_number;
        
        // open handle for disk as `WIN32 Device Namespace` foramt.
        HANDLE disk = CreateFileW(
                        path.str().c_str(),
                        GENERIC_READ/* | GENERIC_WRITE*/,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,  // for device or file, only if exists.
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if (INVALID_HANDLE_VALUE == disk)
        {
            log_err
                "CreateFile( %ws ) failed. gle = %u",
                path.str().c_str(),
                GetLastError()
            log_end;
            continue;
        }

        // get information about drive layout
        bool break_loop = false;
        uint32_t dli_size = sizeof(DRIVE_LAYOUT_INFORMATION_EX) * 4;
        PDRIVE_LAYOUT_INFORMATION_EX dli = (PDRIVE_LAYOUT_INFORMATION_EX)malloc(dli_size);
        if (NULL == dli)
        {
            log_err
                "insufficient rsrc for DRIVE_LAYOUT_INFORMATION_EX buffer. device = %ws", 
                path.str().c_str()
            log_end;

            CloseHandle(disk);
            continue;
        }

        while(true != break_loop)
        {
            // get needed buffer size
            if (!DeviceIoControl(
                    disk,
                    IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                    NULL,
                    0,
                    (LPVOID)dli,
                    dli_size,
                    (LPDWORD)&bytes_returned,
                    NULL))
            {
                DWORD gle = GetLastError();
                if (ERROR_INSUFFICIENT_BUFFER == gle)
                {
                    dli_size *= 2;
                    dli = (PDRIVE_LAYOUT_INFORMATION_EX)realloc(dli, dli_size);
                }
                else
                {
                    log_err
                        "DeviceIoControl( IOCTL_DISK_GET_DRIVE_LAYOUT_EX ) failed. "\
                        "device = %ws, gle = %u, bytes_returned = %u",
                        path.str().c_str(),
                        gle,
                        bytes_returned
                        log_end;
                    break_loop = true;
                }

                continue;
            }
            else
            {
                // ok. print partition info for the drive
                log_info
                    "===============================================\n"\
                    "device = %ws\npartition style = %u, count = %u",
                    path.str().c_str(),
                    dli->PartitionStyle,
                    dli->PartitionCount
                    log_end;

                for (DWORD i = 0; i < dli->PartitionCount; ++i)
                {
                    PPARTITION_INFORMATION_EX pi = &dli->PartitionEntry[i];
                    if (pi->PartitionStyle == PARTITION_STYLE_MBR)
                    {
                        PPARTITION_INFORMATION_MBR mbr = &pi->Mbr;
                        log_info
                            "partition [%u] info\n"\
                            "   style = PARTITION_STYLE_MBR\n"\
                            "   start offset = 0x%llx\n"\
                            "   length = %llu\n"\
                            "   number = %u\n"\
                            "   [MBR]\n"\
                            "       PartitionType = %u\n"\
                            "       BootIndicator = %s\n"\
                            "       RecognizedPartition = %s\n"\
                            "       HiddenSectors = %u\n",
                            i,
                            pi->StartingOffset,
                            pi->PartitionLength,
                            pi->PartitionNumber,
                            mbr->PartitionType,
                            TRUE == mbr->BootIndicator ? "true" : "false",
                            TRUE == mbr->RecognizedPartition ? "true" : "false",
                            mbr->HiddenSectors
                            log_end;

                        if (mbr->BootIndicator)
                        {
                            uint8_t buf[512] = { 0x00 };
                            LARGE_INTEGER li_new_pos = { 0 };
                            LARGE_INTEGER li_distance = { 0 };
                            
                            // boot ��Ƽ���� ��ġ�� ��ũ�� ù ���ʹ� MBR
                            log_info "[*] dump MBR" log_end
                            li_distance.QuadPart = 0;
                            if (!SetFilePointerEx(disk, li_distance, &li_new_pos, FILE_BEGIN))
                            {
                                log_err
                                    "SetFilePointerEx() failed, gle = %u", GetLastError()
                                    log_end;
                                continue;
                            }
                            
                            if (!ReadFile(disk, buf, sizeof(buf), &bytes_returned, NULL))
                            {
                                log_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
                            }
                            else
                            {
                                std::vector<std::string> dumps;
                                dump_memory(0, buf, sizeof(buf), dumps);
                                for (auto line : dumps)
                                {
                                    log_info "%s", line.c_str() log_end;
                                }
                            }

                            // boot ��Ƽ���� ù ���ʹ� VBR
                            li_distance = pi->StartingOffset;
                            if (!SetFilePointerEx(disk, li_distance, &li_new_pos, FILE_BEGIN))
                            {
                                log_err
                                    "SetFilePointerEx() failed, gle = %u", GetLastError()
                                    log_end;
                                continue;
                            }
                            
                            if (!ReadFile(disk, buf, sizeof(buf), &bytes_returned, NULL))
                            {
                                log_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
                            }
                            else
                            {
                                std::vector<std::string> dumps;
                                dump_memory(0, buf, sizeof(buf), dumps);

                                log_info 
                                    "[*] dump VBR (disk offset 0x%llx)", pi->StartingOffset 
                                log_end
                                for (auto line : dumps)
                                {
                                    log_info "%s", line.c_str() log_end;
                                }
                            }
                        }
                    }
                    else if (pi->PartitionStyle == PARTITION_STYLE_GPT)
                    {
                        PPARTITION_INFORMATION_GPT gpt = &pi->Gpt;

                        std::string p_type;
                        std::string p_id;

                        bin_to_hexa(sizeof(GUID), (uint8_t*)&gpt->PartitionType, false, p_type);
                        bin_to_hexa(sizeof(GUID), (uint8_t*)&gpt->PartitionId, false, p_id);

                        log_info
                            "partition [%u] info\n"\
                            "   style = PARTITION_STYLE_GPT\n"\
                            "   start offset = 0x%llx\n"\
                            "   length = %llu\n"\
                            "   number = %u\n"\
                            "   [GPT]\n"\
                            "       PartitionType = %s\n"\
                            "       PartitionId = %s\n"\
                            "       Attributes = %llx\n"\
                            "       Name = %ws\n",
                            i,
                            pi->StartingOffset,
                            pi->PartitionLength,
                            pi->PartitionNumber,
                            p_type.c_str(),
                            p_id.c_str(),
                            gpt->Attributes,
                            gpt->Name
                            log_end;
                    }
                    else
                    {
                        log_info
                            "partition [%u] info\n"\
                            "   style = PARTITION_STYLE_RAW\n"\
                            "   start offset = 0x%llx\n"\
                            "   length = %llu\n"\
                            "   number = %u\n",
                            i,
                            pi->StartingOffset,
                            pi->PartitionLength,
                            pi->PartitionNumber
                            log_end;
                    }

                    
                }

                break_loop = true;
            }
        }

        free_and_nil(dli);
        CloseHandle(disk);
    }
    return true;
}

/// @brief  c ����̺�� ���ε� ��Ƽ��(����)�� boot_area ������ ����Ѵ�. 
///         �翬�� CreateFile �� open�ϴ� �ڵ��� ������ �ڵ��̾�� �Ѵ�. 
bool dump_boot_area()
{
    DWORD bytes_returned = 0;

    // open handle for disk
    if (true != set_privilege(SE_MANAGE_VOLUME_NAME, true))
    {
        log_err "set_privilege(SE_MANAGE_VOLUME_NAME) failed." log_end;
        return false;
    }

    HANDLE disk = INVALID_HANDLE_VALUE;
    
#pragma warning(disable: 4127)  // conditional expression is constant
    do
    {
        std::wstringstream path;
        path << L"\\\\.\\h:";           // ������ �����ؾ� �Ѵ�. \\.\PhysicalDrive0 ������ �����ϸ� �ȵ�
        disk = CreateFileW(
                        path.str().c_str(),
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,  // for device or file, only if exists.
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if (INVALID_HANDLE_VALUE == disk)
        {
            log_err
                "CreateFile( %ws ) failed. gle = %u",
                path.str().c_str(),
                GetLastError()
                log_end;
            break;
        }

        BOOT_AREA_INFO bai = { 0 };

        if (!DeviceIoControl(
                disk,
                FSCTL_GET_BOOT_AREA_INFO,
                NULL,
                0,
                (LPVOID)&bai,
                sizeof(bai),
                (LPDWORD)&bytes_returned,
                NULL))
        {
            log_err "DeviceIoControl(FSCTL_GET_BOOT_AREA_INFO) failed. gle = %u\n", GetLastError() log_end;

            break;
        }

        log_info
            "===============================================\n"\
            "device = %ws\nBootSectorCount = %u, Offset = 0x%llx, Offset2 = 0x%llx",
            path.str().c_str(),
            bai.BootSectorCount,
            bai.BootSectors[0].Offset.QuadPart,
            bai.BootSectors[1].Offset.QuadPart
            log_end;

        uint8_t buf[512] = { 0x00 };
        LARGE_INTEGER li_new_pos = { 0 };
        LARGE_INTEGER li_distance = { 0 };
        li_distance.LowPart = bai.BootSectors[0].Offset.LowPart;
        li_distance.HighPart = bai.BootSectors[0].Offset.HighPart;
        if (!SetFilePointerEx(disk, li_distance, &li_new_pos, FILE_BEGIN))
        {
            log_err
                "SetFilePointerEx() failed, gle = %u", GetLastError()
                log_end;
            break;
        }

        if (!ReadFile(disk, buf, sizeof(buf), &bytes_returned, NULL))
        {
            log_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
        }
        else
        {
            std::vector<std::string> dumps;
            dump_memory(0, buf, sizeof(buf), dumps);
            for (auto line : dumps)
            {
                log_info "%s", line.c_str() log_end;
            }
        }
    } while (false);
#pragma warning(default: 4127)  // conditional expression is constant

    if (disk != INVALID_HANDLE_VALUE)
    {
        CloseHandle(disk);
    }

    return true;
}

/**
* @brief	������ ������� �����Ѵ�.
*/
HANDLE open_file_to_write(_In_ const wchar_t* file_path)
{
	_ASSERTE(NULL != file_path);
	if (NULL == file_path)
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
            "CreateFile(path=%S), gle=%u", 
            file_path, 
            GetLastError()
        log_end
	}

    return hFile;
}

/**
* @brief	������ �б���� �����Ѵ�.
*/
HANDLE open_file_to_read(LPCWCH file_path)
{
	_ASSERTE(NULL != file_path);
	if (NULL == file_path)
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
            "CreateFile(path=%ws), gle=%u", 
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
bool get_file_size(_In_ HANDLE file_handle, _Out_ int64_t& size)
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
* @brief	���Ͽ� ���˹��ڿ��� ����.
*/
BOOL write_to_filew(LPCWCH file_path, LPCWCH format,...)
{
	_ASSERTE(NULL != file_path);
	if (NULL == file_path)
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
            "CreateFile(%S), gle=%u", 
            file_path, GetLastError()
        log_end
        return FALSE;
	}

	// �ڿ� ���� ��ü ����
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
* @brief	���Ͽ� ���� ���ڿ��� ����
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
* @brief	���Ͽ� ���� ���ڿ��� ����
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
bool get_file_position(_In_ HANDLE file_handle, _Out_ int64_t& position)
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

/// @brief	���������͸� distance �� �̵��Ѵ�.
bool 
set_file_position(
	_In_ HANDLE file_handle, 
	_In_ int64_t distance, 
	_Out_opt_ int64_t* new_position
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

/// @brief	������ ����� �����Ѵ�.
///			- SetFilePointer() -> SetEndOfFile() ���
///			- SetFileInformationByHandle(..., FileAllocationInfo, ...)  ���
bool set_file_size(_In_ HANDLE file_handle, _In_ int64_t new_size)
{
#if 0
	FILE_ALLOCATION_INFO file_alloc_info;
	file_alloc_info.AllocationSize.QuadPart = new_size;

	if (!SetFileInformationByHandle(file_handle,
									FileAllocationInfo,
									&file_alloc_info,
									sizeof(file_alloc_info)))
	{
		log_err
			"SetFileInformationByHandle() failed. class=FileAllocationInfo, gle=%u",
			GetLastError()
			log_end;
		return false;
	}
#endif
	if (!set_file_position(file_handle, new_size, NULL))
	{
		log_err "set_file_position() failed." log_end;
		return false;
	}

	if (!SetEndOfFile(file_handle))
	{
		log_err "SetEndOfFile() failed. gle=%u", GetLastError() log_end;
		return false;
	}

	return true;
}

/**
* @brief	���ڿ��� UTF8 ���� ���Ϸ� �����Ѵ�.
*           http://digitz.tistory.com/303   ANSI�� UTF-8���� ��ȣ ��ȯ
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
* @brief	���ڿ��� UTF8 ���� ���Ϸ� �����Ѵ�.
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
            "WriteFile(BOM) failed, gle=%u", 
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
            "WriteFile(Utf8String) failed, gle=%u", 
            GetLastError()
        log_end
        CloseHandle(hFile); hFile = NULL;
        return FALSE;
    }
    
    CloseHandle(hFile); hFile = NULL;
    return TRUE;
}

/**
* @brief	������ �޸𸮿� �ε��Ѵ�. ��ȯ�Ǵ� �޸𸮴� �����Ҵ�� �޸��̹Ƿ� caller �� �����ؾ� ��
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
            "CreateFile(%ws) failed, gle=%u", 
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
            "%ws, can not get file size, gle=%u", 
            FilePath, 
            GetLastError() 
        log_end
        return FALSE;
    }

	if (0 == fileSize.QuadPart)
	{
		log_err "Can not map zero length file" log_end
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
            "CreateFileMapping(%ws) failed, gle=%u", 
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
            "MapViewOfFile(%ws) failed, gle=%u", 
            FilePath, 
            GetLastError() 
        log_end

        return FALSE;
    }
    SmrtView sfView(ImageView);

    MemorySize = fileSize.LowPart;  // max config fileSize = 4 MB �̹Ƿ� ������
    Memory = (PBYTE) malloc(MemorySize);
    if (NULL == Memory) {return FALSE;}

    RtlZeroMemory(Memory, MemorySize);
    RtlCopyMemory(Memory, ImageView, MemorySize);
    return TRUE;
}

/**
* @brief	���̳ʸ� ���Ϸ� �����͸� �����Ѵ�.
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
    _ASSERTE(NULL != Directory);
    _ASSERTE(NULL != FileName);
    _ASSERTE(0 < Size);
    _ASSERTE(NULL != Data);
    if (NULL == Directory || NULL == FileName || 0 >= Size || NULL == Data)
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
            "Can not generate target path, dir=%S, file=%S", 
            Directory, FileName
        log_end
        return FALSE;
    }

    // ������ ������ �����ϴ� ��� ���� ������ ���� �� ���Ӱ� ������
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
            "Can not create file=%S, check path or privilege", 
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
            "WriteFile(path=%S) failed, gle=%u",
            DataPath, GetLastError()
        log_end
        return FALSE;
    }

    return TRUE;
}

/// @brief	DirectoryPath ���丮�� �����Ѵ�. �߰��� ���� ���丮 ��ΰ� �����ϸ�
///			�����Ѵ�.
bool WUCreateDirectory(const LPCWSTR DirectoryPath)
{
	_ASSERTE(NULL != DirectoryPath);
	if (NULL==DirectoryPath) return false;

	if (true != is_file_existsW(DirectoryPath))
	{
		if (ERROR_SUCCESS != SHCreateDirectoryExW(NULL, DirectoryPath, NULL))
		{
			log_err
				"SHCreateDirectoryExW( path=%ws ) failed. gle=%u", 
				DirectoryPath, GetLastError()
			log_end
			return false;
		}
	}

	return true;
}

/**----------------------------------------------------------------------------
    \brief	������ ���丮(������ ����, ���ϵ����)�� ���� �����ϴ� �Լ�  
    
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
	
	// FileOp.pFrom, FileOp.pTo �� NULL char �� �ΰ��̾�� �� (msdn ����)
	// 
	size_t len = (wcslen(DirctoryPathToDelete) + 2) * sizeof(WCHAR);
	WCHAR* tmp= (WCHAR*)malloc(len);
	if (NULL == tmp) return false;
	RtlZeroMemory(tmp, len);
	if (TRUE != SUCCEEDED( StringCbPrintfW(tmp, len, L"%s", DirctoryPathToDelete) )) return false;

	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_DELETE;       // ���� �Ӽ� ����
	FileOp.pTo = NULL;
	FileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;//FOF_NOCONFIRMATION | FOF_NOERRORUI; // Ȯ�θ޽����� �ȶߵ��� ����
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
BOOL 
GetImageFullPathFromPredefinedPathA(
    IN  LPCSTR ImageName, 
    IN  DWORD  FullPathBufferLen,
    OUT LPSTR  FullPathBuffer
    )
{
    _ASSERTE(NULL != ImageName);
    _ASSERTE(NULL != FullPathBuffer);
    if (	(NULL == ImageName) || 
			(NULL == FullPathBuffer))
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
    if (	(NULL == ImageName) || 
			(NULL == FullPathBuffer))
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
        // �ٽ� ȣ��Ǵ� ��� SearchPathBuf ���۸� �ٽ� ����� ���� ������
        // static ���� ����
        //
        static WCHAR SearchPathBuf[MAX_PATH] = {0,};
        if (0x00 == SearchPathBuf[0])
        {
            std::wstring system_root;
            if (!get_system_dir(system_root))
            {
                log_err "get_system_rootdir() failed." log_end;
                return FALSE;
            }

            if (! SUCCEEDED(StringCbPrintfW(
                                SearchPathBuf, 
                                sizeof(SearchPathBuf), 
                                L"%s\\drivers", 
                                system_root.c_str())))
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

/// @ brief GetProcessImageFileName() wrapper
bool get_process_image_full_path(_In_ HANDLE process_handle, _Out_ std::wstring& full_path)
{
    bool ret = false;
    uint32_t    buf_len = 1024;
    wchar_t*    buf = (wchar_t*)malloc(buf_len);
    if (NULL == buf) return false;
        
    for (int i = 0; i < 3; ++i) // ���� �ø��°� ������...
    {
        DWORD dwret = GetProcessImageFileName(process_handle, buf, buf_len / sizeof(wchar_t));
        if (0 == dwret)
        {
            DWORD gle = GetLastError();
            if (gle == ERROR_INSUFFICIENT_BUFFER)
            {
                buf_len *= 2;
                free(buf);
                buf = (wchar_t*)malloc(buf_len);
                if (NULL == buf) return false;
                continue;
            }
            else
            {
                log_err "GetProcessImageFileName() failed. gle = %u", gle log_end;
                break;
            }
        }
        else
        {
            // ����
            full_path = buf;
            ret = true;
            break;
        }
    }

    free(buf);
    return ret;
}


/// @brief  GetSystemDirectoryW() wrapper (c:\windows\system32)
bool get_system_dir(_Out_ std::wstring& system_dir)
{
    wchar_t     buf[MAX_PATH] = { 0x00 };
    uint32_t    buf_len = sizeof(buf);
    wchar_t*    pbuf = buf;
    
    static  std::wstring _sys_dir;

    if (true != _sys_dir.empty())
    {
        system_dir = _sys_dir;
        return true;
    }

    UINT32 len = GetSystemDirectoryW(pbuf, buf_len);
    if (0 == len)
    {
        log_err "GetSystemDirectoryW() failed. gle=%u", GetLastError() log_end;
        return false;
    }

    if (len < buf_len)
    {
        buf[len] = 0x0000;
        system_dir = buf;
        return true;
    }
    else if (len == buf_len)
    {
        // GetWindowsDirectoryW( ) �� null char �� �������� �ʴ� ���̸� ������
        // ���۰� �� �ʿ��ϴ�.
        buf_len *= 2;            
        pbuf = (wchar_t*)malloc(buf_len);
        if (NULL == buf)
        {
            log_err "not enough memory" log_end;
            return false;
        }

        // try again
        len = GetSystemDirectoryW(pbuf, buf_len);
        if (0 == len)
        {
            log_err "GetSystemDirectoryW() failed. gle=%u", GetLastError() log_end;
            free(pbuf); 
            return false;
        }
        else
        {
            pbuf[len] = 0x0000;
            
            _sys_dir = pbuf;
            system_dir = pbuf;

            free(pbuf); pbuf = NULL;
            return true;
        }
    }

    return true;        // never reach here
}

/**
 * @brief	windows ��θ� �����ϴ� �Լ� (c:\windows)
 * @param	
 * @see		
 * @remarks	��ΰ� c:\ �� ��츦 �����ϰ�, '\' �� ������ ����. (GetWindowsDirectory() �Լ� ����)
 * @code		
 * @endcode	
 * @return	
**/
bool get_windows_dir(_Out_ std::wstring& windows_dir)
{
    wchar_t     buf[MAX_PATH] = { 0x00 };
    uint32_t    buf_len = sizeof(buf);
    wchar_t*    pbuf = buf;

    static  std::wstring _win_dir;

    if (true != _win_dir.empty())
    {
        windows_dir = _win_dir;
        return true;
    }

    UINT32 len = GetWindowsDirectoryW(pbuf, buf_len);
    if (0 == len)
    {
        log_err "GetWindowsDirectoryW() failed. gle=%u", GetLastError() log_end;
        return false;
    }

    if (len < buf_len)
    {
        buf[len] = 0x0000;
        windows_dir = buf;
        return true;
    }
    else if (len == buf_len)
    {
        // GetWindowsDirectoryW( ) �� null char �� �������� �ʴ� ���̸� ������
        // ���۰� �� �ʿ��ϴ�.
        buf_len *= 2;
        pbuf = (wchar_t*)malloc(buf_len);
        if (NULL == buf)
        {
            log_err "not enough memory" log_end;
            return false;
        }

        // try again
        len = GetSystemDirectoryW(pbuf, buf_len);
        if (0 == len)
        {
            log_err "GetSystemDirectoryW() failed. gle=%u", GetLastError() log_end;
            free(pbuf);
            return false;
        }
        else
        {
            pbuf[len] = 0x0000;
            windows_dir = pbuf;
            _win_dir = pbuf;    // !

            free(pbuf); pbuf = NULL;
            return true;
        }
    }

    return true;        // never reach here
}

/**
 * @brief	���� ����� windows ȯ�溯���� �о �����Ѵ�.
			%home% = \Users\somma
			%path% = c:\python27\;xx.....
			%temp% = \Users\somma\AppData\Loal\Temp
			...
 * @param	env_variable	ȯ�� ���� ���ڿ� (e.g. %home%, %path%)
 * @param	env_value		ȯ�� ���� ��     (\Users\somma ���ڿ�)
 * @return	������ true, ���н� false
**/
bool 
get_environment_value(
	_In_ const wchar_t* env_variable, 
	_Out_ std::wstring& env_value
	)
{
	_ASSERTE(NULL != env_variable);
	if (NULL == env_variable) return false;

	wchar_t* buf = NULL;
	uint32_t buf_len = 0;
	DWORD char_count_plus_null = ExpandEnvironmentStrings(env_variable, buf, buf_len);
	if (0 == char_count_plus_null)
	{
		log_err 
			"ExpandEnvironmentStrings( %ws ) failed. gle = %u", 
			env_variable, 
			GetLastError() 
		log_end
		return false;
	}

	buf_len = char_count_plus_null * sizeof(wchar_t);
	buf = (wchar_t*) malloc(buf_len);
	if (NULL == buf)
	{
		log_err "malloc() failed." log_end
		return false;
	}

	char_count_plus_null = ExpandEnvironmentStrings(env_variable, buf, buf_len);
	if (0 == char_count_plus_null)
	{
		log_err 
			"ExpandEnvironmentStrings( %ws ) failed. gle = %u", 
			env_variable, 
			GetLastError() 
		log_end

		free(buf);
		return false;
	}

	env_value = buf;
	free(buf);
	return true;
}

/**
 * @brief	�̹� �����ϴ� ������ short file name �� ���Ѵ�. 
 * @param	
 * @see		
 * @remarks	https://msdn.microsoft.com/en-us/library/aa365247(v=vs.85).aspx
 * @code		
 * @endcode	
 * @return	
**/
bool get_short_file_name(_In_ const wchar_t* long_file_name, _Out_ std::wstring& short_file_name)
{
	_ASSERTE(NULL != long_file_name);
	if (NULL == long_file_name) return false;

	wchar_t* short_path = NULL;
	uint32_t char_count_and_null = 0;
	char_count_and_null  = GetShortPathNameW(long_file_name, short_path, char_count_and_null);
	if (0 == char_count_and_null)
	{
		log_err "GetShortPathNameW( %ws ) failed. gle = %u", long_file_name, GetLastError() log_end
		return false;
	}

	short_path = (wchar_t*) malloc( sizeof(wchar_t*) * char_count_and_null );
	if (NULL == short_path) 
	{
		log_err "malloc() failed." log_end
		return false;
	}

	if (0 == GetShortPathNameW(long_file_name, short_path, char_count_and_null))
	{
		log_err "GetShortPathNameW( %ws ) failed.", long_file_name log_end
		free(short_path);
		return false;
	}

	short_file_name = short_path;
	free(short_path);
	return true;
}



/**
 * @brief      ���� ���丮�� �����ϴ� ��� ���ϵ��� enum �ϴ� �Լ�

				�Ʒ� ���� 4������ ��� ������ ����� �����	
				"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData",
				"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData\\",
				"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData\\*",
				"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData\\*.*"

				Ȯ���� ���͸� �����͵� ������
				"d:\\Work\\AFirstIRF\\trunk\\AIRF\\debug\\AIRSData\\*.txt"
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
    if (NULL == root) return false;


	std::wstring root_dir(root);

    if (root[wcslen(root)-1] == L'\\')
    {
		//
		//	root �Ķ���Ͱ� '\' �� ������ �ȵǹǷ� `\*` �� ���� ����
		//
        root_dir.append(L"*");
    }
	else 
	{
		//	`d:\dir\` �� ���� �̹� �˻������Ƿ� root �� �Ʒ� �� Ÿ�� �� �ϳ��ϰ��̴�.
		// 
		//	d:\dir
		//	d:\dir\*, d:\dir\*.*, d:\dir\*.txt, ...
		//	
		//	`d:\dir\*` ���¶�� ������ is_dir() ���� false �� �����ϹǷ� 
		//	is_dir() �� true �� �����ϴ� ����� ������ `\*` �� �ٿ��ش�. 
		// 
		if (true == is_dir(root))
		{
			root_dir.append(L"\\*");
		}		
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
		DWORD gle = GetLastError();

		if (ERROR_ACCESS_DENIED != gle)
		{
			log_err
				"FindFirstFileW(path=%S) failed, gle=%u", 
				root_dir.c_str(), GetLastError()
			log_end
		}
        
        return false;
    }

	_wsplitpath_s(root_dir.c_str(), drive, _MAX_DRIVE, dir, MAX_PATH, NULL, NULL, NULL, NULL);

    while (bResult) 
    {
        WaitForSingleObject(NULL, 0);

        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) 
        {
            // symbolic link �� ó������ ����
            // 
            if (wfd.dwReserved0 & IO_REPARSE_TAG_SYMLINK)
            {                
				bResult=FindNextFile(hSrch,&wfd);
                continue;                
            }
        }
        else if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
			if (true == recursive)
			{
				if (0 != _wcsnicmp(&wfd.cFileName[0], L".", 1))
				{
					StringCbPrintfW(newpath, sizeof(newpath), L"%s%s%s\\*.*", drive, dir, wfd.cFileName);
					find_files(newpath, cb, tag, recursive);
				}
			}			            
        } 
        else 
        {
			//
			//	���� ��θ� callback ���� �����Ѵ�. ���丮�� �������� ����.
			// 
            
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
    \brief  RootPath �������丮 ��θ� enum �ϴ� �Լ�
			
			�Ʒ� ���� 4������ ��� ������ ����� �����			
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
    if (NULL == RootPath) return FALSE;

    HANDLE hSrch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd = {0};
    BOOL bResult=TRUE;
    WCHAR drive[_MAX_DRIVE+1] = {0};
    WCHAR dir[MAX_PATH+1] = {0};

	// RootPath �Ķ���Ͱ� c:\dbg ������ ��� c:\dbg ��(RootPath �ڽ�)�� �߰���
	// ���� '\', '\*', '\*.*' �� �ƴ� ��� ������ '\*' �� �ٿ� RootPath ��δ� ������
	// 
	// ���� RootPath �Ķ���Ͱ� '\' �� ������ �ȵǹǷ� \* �� ���� ����
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
            "FindFirstFileW(path=%S) failed, gle=%u", 
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
* @brief	aes-gcm�� ���� ����� 16����� �ƴϸ� �������� ������.16����� ���� ������ ������ ��.
* @param
* @see
* @remarks
* @code
* @endcode
* @return
**/
bool file_size_padding(
	IN wchar_t* file_path,
	IN OUT int64_t &padding
)
{
	_ASSERTE(NULL != file_path);
	_ASSERTE(NULL == padding);
	if (NULL == file_path || NULL != padding)
		return FALSE;

	HANDLE hfile = open_file_to_write(file_path);
	int64_t file_size = 0;

	if (!get_file_size(hfile, file_size))
	{
		log_err "get_file_size err" log_end;
		CloseHandle(hfile);
		return FALSE;
	}

	//16 �����ŭ ������
	//16 ����� ����
	if (file_size % 16)
		padding = 16 - file_size % 16;
	else
		return TRUE;

	if (!set_file_size(hfile, file_size + padding))
	{
		log_err "set_file_size err" log_end;
		CloseHandle(hfile);
		return FALSE;
	}

	CloseHandle(hfile);
	return TRUE;
}

/**
* @brief	aes256 ���� ��ȣȭ
* @param
* @see
* @remarks
* @code
* @endcode
* @return
**/
bool aes256_encrypt(
	IN unsigned char *key,
	IN const wchar_t *path,
	IN const wchar_t *target_file_path,
	IN const wchar_t *encrypt_file_name,
	OUT unsigned char * out_encrypt
)
{
	_ASSERTE(NULL != key);
	_ASSERTE(NULL != path);
	_ASSERTE(NULL != target_file_path);
	_ASSERTE(NULL != encrypt_file_name);
	_ASSERTE(NULL == out_encrypt);
	if (NULL == key || NULL == path || NULL == target_file_path || NULL == encrypt_file_name || NULL != out_encrypt)
		return FALSE;


	PBYTE buffer = nullptr;
	DWORD file_size = 0;
	unsigned char *encrypt_data = nullptr;
	UINT32 length = 0;

	//��ȣȭ�� ���ϸ� �޸𸮿� �ε�
	if (LoadFileToMemory(target_file_path, file_size, buffer))
	{

		AirCryptBuffer(key, strlen((const char*)key), buffer, file_size + 1, encrypt_data, length, TRUE);
		//��ȣȭ ������ ���ο� ���Ϸ� ����
		SaveBinaryFile(path, encrypt_file_name, file_size, encrypt_data);
		free(buffer);
		//free(encrypt_data);	//�׽�Ʈ �ڵ忡���� decrypt �� �� �Ҵ� ����
		//���߿� ����� �� free ����
		out_encrypt = encrypt_data;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/**
* @brief	aes256 ���� ��ȣȭ
* @param
* @see
* @remarks
* @code
* @endcode
* @return
**/
bool aes256_decrypt(
	IN unsigned char *key,
	IN const wchar_t *path,
	IN const wchar_t *target_file_path,
	IN const wchar_t *encrypt_file_name,
	IN const wchar_t *decrypt_file_name,
	IN  unsigned char * encrypt
)
{
	_ASSERTE(NULL != key);
	_ASSERTE(NULL != path);
	_ASSERTE(NULL != target_file_path);
	_ASSERTE(NULL != encrypt_file_name);
	_ASSERTE(NULL == encrypt);
	if (NULL == key || NULL == path || NULL == target_file_path || NULL == encrypt_file_name || NULL != encrypt)
		return FALSE;

	PBYTE buffer = nullptr;
	DWORD file_size = 0;
	UINT32 length = 0;

	//��ȣȭ�� ���ϸ� �޸𸮿� �ε�
	if (LoadFileToMemory(encrypt_file_name, file_size, buffer))
	{
		//��ȣȭ
		AirCryptBuffer(key, strlen((const char*)key), buffer, file_size + 1, encrypt, length, FALSE);
		//��ȣȭ ������ ���ο� ���Ϸ� ����
		SaveBinaryFile(path, decrypt_file_name, file_size, encrypt);
		free(buffer);
		free(encrypt);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * @brief	ASCII(Multibyte) --> WIDE CHAR �� ��ȯ, caller �� ���ϵǴ� �����͸� �Ҹ�����־�� ��
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
 * @brief	WIDE CHAR --> ASCII(Multibyte) �� ��ȯ, caller �� ���ϵǴ� �����͸� �Ҹ�����־�� ��
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
 * @brief	wide char -> utf8 ��ȯ, caller �� ���ϵǴ� �����͸� �Ҹ�����־�� �� 
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
    if (NULL == tmp.get())
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

/// @brief  src �� �ڿ������� fnd ���ڿ��� ã�´�. 
///         fnd �� src �� �ǹ��Ͽ� ��Ȯ�� ��ġ�ϸ� true, �ƴϸ� false ����
///         - Ȯ���� �˻簰���� �Ҷ� ���
bool rstrnicmp(_In_ const wchar_t* src, _In_ const wchar_t* fnd)
{
    _ASSERTE(NULL != src);
    _ASSERTE(NULL != fnd);
    if (NULL == src || NULL == fnd) return false;
    
    uint32_t src_len = (uint32_t)wcslen(src);
    uint32_t fnd_len = (uint32_t)wcslen(fnd);
    if (fnd_len > src_len) return false;

    int sidx = src_len - 1; // uint32_t Ÿ�� ���� �ȵ�!
    int fidx = fnd_len - 1;
    while (fidx >= 0)
    {
        if (towlower(fnd[fidx--]) != towlower(src[sidx--])) return false;

    }
    return true;
}

bool rstrnicmpa(_In_ const char* src, _In_ const char* fnd)
{
    _ASSERTE(NULL != src);
    _ASSERTE(NULL != fnd);
    if (NULL == src || NULL == fnd) return false;

    uint32_t src_len = (uint32_t)strlen(src);
    uint32_t fnd_len = (uint32_t)strlen(fnd);
    if (fnd_len > src_len) return false;

    int sidx = src_len - 1; // uint32_t Ÿ�� ���� �ȵ�!
    int fidx = fnd_len - 1;
    while (fidx >= 0)
    {
        if (tolower(fnd[fidx--]) != tolower(src[sidx--])) return false;

    }
    return true;
}


/// @brief  src �� �տ������� fnd ���ڿ��� ã�´�. 
bool lstrnicmp(_In_ const wchar_t* src, _In_ const wchar_t* fnd)
{
    _ASSERTE(NULL != src);
    _ASSERTE(NULL != fnd);
    if (NULL == src || NULL == fnd) return false;

    return (0 == _wcsnicmp(src, fnd, wcslen(fnd))) ? true : false;
}

/// @brief  src �� �տ������� fnd ���ڿ��� ã�´�. 
bool lstrnicmpa(_In_ const char* src, _In_ const char* fnd)
{
    _ASSERTE(NULL != src);
    _ASSERTE(NULL != fnd);
    if (NULL == src || NULL == fnd) return false;

    return (0 == _strnicmp(src, fnd, strlen(fnd))) ? true : false;
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
//	// unicode �� �ƴ� ��� �ѱ۰��� wide char �� 2����Ʈ�� �����
//	// �׷��� _bstr_t::length() �� ������ ������ �����ϹǷ� 
//	// CharCount * sizeof(TCHAR) * 2 �� �� ��� �Ѵ�. 
//	// ���� unicode �� ���� �ƹ� ���� ����
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
 * \brief	org_string ���� token �� �˻��ؼ� ���ڿ��� �߶󳽴�. 
			(org_string �� �տ������� token �� �˻�)

			ABCDEFG.HIJ.KLMN	: org_string
				   .			: token
			ABCDEFG             : out_string if forward = TRUE
					HIJ.KLMN	: out_string if forward = FALSE

			delete_token �� True �� ��� org_string ���� out_string + token �� ����

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
 * \brief	org_string ���� token �� �˻��ؼ� ���ڿ��� �߶󳽴�. 
			(org_string �� �տ������� token �� �˻�)

			ABCDEFG.HIJ.KLMN	: org_string
				   .			: token
			ABCDEFG             : out_string if forward = TRUE
					HIJ.KLMN	: out_string if forward = FALSE
 * @param	
 * @see		
 * @remarks ���� �� �и��� ���ڿ��� ��Ʈ�� ��ü�� ����
			���� �� _nullstringw ����
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
 * @brief	org_string ���� token �� �˻��ؼ� ���ڿ��� �߶󳽴�. 
			(org_string �� �տ������� token �� �˻�)
			
			ABCDEFG.HIJ.KLMN	: org_string
			       .			: token
		    ABCDEFG             : out_string if forward = TRUE
			        HIJ.KLMN	: out_string if forward = FALSE

            delete_token �� True �� ��� org_string ���� out_string + token �� ����
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
 * @brief	org_string ���� token �� �˻��ؼ� ���ڿ��� �߶󳽴�. 
			(org_string �� �տ������� token �� �˻�)
			
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
 * @brief	org_string ���� token �� �˻��ؼ� ���ڿ��� �߶󳽴�. 
			(org_string �� �ڿ������� token �� �˻�)

			ABCDEFG.HIJ.KLMN	: org_string
					   .		: token
			ABCDEFG.HIJ			: out_string if forward = TRUE
					    KLMN	: out_string if forward = FALSE

			delete_token �� True �� ��� org_string ���� out_string + token �� ����
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
 * @brief	org_string ���� token �� �˻��ؼ� ���ڿ��� �߶󳽴�. 
			(org_string �� �ڿ������� token �� �˻�)

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
 * @brief	org_string ���� token �� �˻��ؼ� ���ڿ��� �߶󳽴�. 
			(org_string �� �ڿ������� token �� �˻�)
			
			ABCDEFG.HIJ.KLMN	: org_string
			           .		: token
		    ABCDEFG.HIJ			: out_string if forward = TRUE
						KLMN	: out_string if forward = FALSE

            delete_token �� True �� ��� org_string ���� out_string + token �� ����
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
* @brief	trim �Լ���

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

/// @brief  sprit `str` using `seps` and save each token into `tokens`. 
bool 
split_stringa(
	_In_ const char* str, 
	_In_ const char* seps, 
	_Out_ std::vector<std::string>& tokens
	)
{
#define max_str_len     2048

    _ASSERTE(NULL != str);
    if (NULL == str) return false;

    tokens.clear();

    // strtok_s() modifies the `str` buffer.
    // so we should make copy.
    size_t buf_len = (strlen(str) * sizeof(char)) + sizeof(char);
    if (max_str_len < buf_len)
    {
        return false;
    }

	raii_char_ptr buf((char*)malloc(buf_len), raii_free);
	if (nullptr == buf.get())
	{
		return false;
	}

	StringCbPrintfA(buf.get(), buf_len, "%s", str);

    char* next_token = NULL;
    char* token = strtok_s(buf.get(), seps, &next_token);
    while (NULL != token)
    {
        tokens.push_back(token);
        token = strtok_s(NULL, seps, &next_token);
    }   
	
    return true;
}

bool 
split_stringw(
	_In_ const wchar_t* str, 
	_In_ const wchar_t* seps, 
	_Out_ std::vector<std::wstring>& tokens
	)
{
#define max_str_len     2048

    _ASSERTE(NULL != str);
    if (NULL == str) return false;

    tokens.clear();

    // strtok_s() modifies the `str` buffer.
    // so we should make copy.
    size_t buf_len = (wcslen(str) * sizeof(wchar_t)) + sizeof(wchar_t);
    if (max_str_len < buf_len)
    {
        return false;
    }

	raii_wchar_ptr buf((wchar_t*)malloc(buf_len));
    if (nullptr == buf.get())
    {
        return false;
    }

    StringCbPrintfW(buf.get(), buf_len, L"%ws", str);

    wchar_t* next_token = NULL;
    wchar_t* token = wcstok_s(buf.get(), seps, &next_token);
    while (NULL != token)
    {
        tokens.push_back(token);
        token = wcstok_s(NULL, seps, &next_token);
    }
    return true;
}

/// @brief  string to hash
///         http://stackoverflow.com/questions/98153/whats-the-best-hashing-algorithm-to-use-on-a-stl-string-when-using-hash-map?answertab=active
///         �������غ���, �� ����, �׳� ��
uint32_t hash_string32(_In_ const char* s, _In_opt_ uint32_t seed)
{
    _ASSERTE(NULL != s);
    if (NULL == s) return 0;

    uint32_t hash = seed;
    while (*s)
    {
        hash = hash * 101 + *s++;
    }
    return hash;
}

uint64_t hash_string64(_In_ const char* s, _In_opt_ uint64_t seed)
{
    _ASSERTE(NULL != s);
    if (NULL == s) return 0;

    uint64_t hash = seed;
    while (*s)
    {
        hash = hash * 101 + *s++;
    }
    return hash;
}

uint32_t hash_string32w(_In_ const wchar_t* s, _In_opt_ uint32_t seed)
{
    _ASSERTE(NULL != s);
    if (NULL == s) return 0;

    uint32_t ret = 0;
    char* mbs = WcsToMbs(s);
    if (NULL != mbs)
    {
        ret = hash_string32(mbs, seed);
        free(mbs);
    }

    return ret;
}

uint64_t hash_string64w(_In_ const wchar_t* s, _In_opt_ uint64_t seed)
{
    _ASSERTE(NULL != s);
    if (NULL == s) return 0;

    uint64_t ret = 0;
    char* mbs = WcsToMbs(s);
    if (NULL != mbs)
    {
        ret = hash_string64(mbs, seed);
        free(mbs);
    }

    return ret;
}

/// @brief  source ���� find �� ã�� replace �� �����ؼ�, ���ο� ���ڿ� ��ü�� ����/�����Ѵ�.
///         ���н� _null_string_a ��ü�� �����Ѵ�.
std::string
find_and_replace_string_exa(
    _In_ const char* source,
    _In_ const char* find,
    _In_ const char* replace
    )
{
    _ASSERTE(NULL != source);
    _ASSERTE(NULL != find);
    _ASSERTE(NULL != replace);

    if (NULL != source && NULL != find && NULL != replace)
    {
        std::string s(source);
        std::string f(find);
        std::string r(replace);
        find_and_replace_string(s, f, r);
        return s;
    }
    else
        return _null_stringa;
}

std::wstring
find_and_replace_string_exw(
    _In_ const wchar_t* source,
    _In_ const wchar_t* find,
    _In_ const wchar_t* replace
    )
{
    _ASSERTE(NULL != source);
    _ASSERTE(NULL != find);
    _ASSERTE(NULL != replace);

    if (NULL != source && NULL != find && NULL != replace)
    {
        std::wstring s(source);
        std::wstring f(find);
        std::wstring r(replace);
        find_and_replace_string(s, f, r);
        return s;
    }
    else
        return _null_stringw;
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
* @brief	���� ���丮�� �����ϴ� �Լ� (e.g. c:\debug )
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
			"GetCurrentDirectoryW() failed. gle=%u", 
			GetLastError()
		log_end
		return FALSE;
	}

	// buflen : NULL ĳ���͸� ������ �ʿ��� ������ ������ in char.
	// 
	buf = (PWSTR) malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		log_err
			"GetCurrentDirectoryW() failed, gle=%u", 
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
			%TMP% > %TEMP% > %USERPROFILE% ȯ�溯�� ������� ������
			�������� '\' �ٿ��� �����Ѵ�.
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
			%TMP% > %TEMP% > %USERPROFILE% ȯ�溯�� ������� ������
			�������� '\' �ٿ��� �����Ѵ�.
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
 * @brief	���� ����� full path �� ���Ѵ�. 
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
    wchar_t* buf = (wchar_t*)malloc(buf_len * sizeof(wchar_t));
	if (NULL == buf) return false;
	
	for(;;)
	{
		ret = GetModuleFileNameW(GetModuleHandleW(module_name), buf, buf_len);
		if (ret == buf_len)
		{
			// buf �� ���� ��� buf_len ��ŭ ���۰� �߸���, buf_len ���� (������ ����)
			// ���� ����� 2�� �÷��� �� �õ�
			free(buf);

			buf_len *= 2;
            buf = (wchar_t*)malloc(buf_len * sizeof(wchar_t));
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
 * @brief	���� ����� full path �� ���Ѵ�. 
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
 * @brief	���� ����� ���ϸ��� ������ ���丮 ��θ� ���Ѵ�. ('\' ���ڴ� ����)
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
 * @brief	���� ����� ���ϸ��� ���Ѵ�. (�������)
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

/// @brief  nt_name ���� device name �κи� ����� �����Ѵ�.
///
///         "\Device\HarddiskVolume4\Windows"    -> "\Device\HarddiskVolume4\"
///         "\Device\HarddiskVolume4"             -> "\Device\HarddiskVolume4"   (!)
///         "\Device\HarddiskVolume4\"           -> "\Device\HarddiskVolume4\"
///         "\Device\HarddiskVolume455\xyz"      -> "\Device\HarddiskVolume455\"
std::wstring device_name_from_nt_name(_In_ const wchar_t* nt_name)
{
    _ASSERTE(NULL != nt_name);
    if (NULL == nt_name) return false;

    // ���ڿ� ���̸� ���
    // input: \Device\HarddiskVolume4\
    //        ^      ^               ^  : `\` �� 3�� ������������ ���̸� ���Ѵ�. (������ `\` ����)
    uint32_t cmp_count = 0;
    uint32_t met_count = 0;
    uint32_t max_count = (uint32_t)wcslen(nt_name);
    for (cmp_count = 0; cmp_count <= max_count; ++cmp_count)
    {
        if (met_count == 3) break;
        if (nt_name[cmp_count] == L'\\')
        {
            ++met_count;
        }
    }
    
    // �׳� ���� ¥��...����..
    if (cmp_count < 256)
    {
        wchar_t buf[256] = { 0x00 };
        RtlCopyMemory(buf, nt_name, sizeof(wchar_t) * cmp_count);
        return std::wstring(buf);
    }
    else
    {
        // oh?! 
        return _null_stringw;
    }
}

/// @brief	full path ��θ��� `���ϸ�.Ȯ����` �κи� �����. 
std::wstring file_name_from_file_path(_In_ const wchar_t* file_path)
{
	_ASSERTE(nullptr != file_path);
	if (nullptr == file_path) return L"";

	return extract_last_tokenExW(file_path, L"\\", false);
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
	if ( (0 < Length) && (NULL != Buf))
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
	if ( (0 < Length) && (NULL != Buf) )
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

	_ftprintf(stream, TEXT("err ] invalid parameters \n"));
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
bool dump_memory(_In_ uint64_t base_offset, _In_ unsigned char* buf, _In_ UINT32 buf_len, _Out_ std::vector<std::string>& dump)
{
	_ASSERTE(NULL!=buf);
	_ASSERTE(0 < buf_len);
	if (NULL == buf || 0 == buf_len) return false;
	
	// !����! - �� ������ line_dump ���� ū ��� (���� �׷�����...?!) ������ �߻� �� �� ����
	char line_dump[1024];

	if ( (0 < buf_len) && (NULL != buf) )
	{
        // useless, uh?
		//StringCbPrintfA(line_dump, sizeof(line_dump), "buf_len = %u, buffer=0x%08x", buf_len, buf);
		//dump.push_back(line_dump);
		
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
								base_offset + i)))
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

	return false;
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
                        newtime.tm_mon + 1,			// 1�� = 0, 2�� = 1, ... ��
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

#if (NTDDI_VERSION >= NTDDI_VISTA)

///	@brief	
std::string ipv4_to_str(_In_ uint32_t ip_netbyte_order)
{
	in_addr ia;
	ia.s_addr = ip_netbyte_order;
	return ipv4_to_str(ia);
}

/// @brief 
std::string ipv6_to_str(_In_ uint64_t ip_netbyte_order)
{
	in_addr6 ia;
	RtlCopyMemory(ia.s6_addr, &ip_netbyte_order, sizeof(ip_netbyte_order));
	return ipv6_to_str(ia);
}

/// @brief  
std::string ipv4_to_str(_In_ in_addr& ipv4)
{
    char ipv4_buf[16 + 1] = { 0 };
    if (NULL == InetNtopA(AF_INET, &ipv4, ipv4_buf, sizeof(ipv4_buf)))
    {
        log_err "InetNtopA( ) failed. wsa_gle = %u", WSAGetLastError() log_end;
        return std::string("0.0.0.0");
    }
    return std::string(ipv4_buf);
}

/// @brief  
std::string ipv6_to_str(_In_ in6_addr& ipv6)
{
    char ipv6_buf[46 + 1] = { 0 };
    if (NULL == InetNtopA(AF_INET6, &ipv6, ipv6_buf, sizeof(ipv6_buf)))
    {
        log_err "InetNtopA( ) failed. wsa_gle = %u", WSAGetLastError() log_end;
        return std::string("0.0.0.0");
    }
    return std::string(ipv6_buf);
}

/// @brief  
bool str_to_ipv4(_In_ const wchar_t* ipv4, _Out_ in_addr& ipv4_addr)
{
    _ASSERTE(NULL != ipv4);
    if (NULL != ipv4)
    {
        int ret = InetPtonW(AF_INET, ipv4, &ipv4_addr);
        switch (ret)
        {
        case 1: 
            return true;    // success
        case 0:
            log_err "invalid ipv4 string. input = %ws", ipv4 log_end;
            return false;
        case -1: 
            log_err "InetPtonW() failed. input = %ws, wsa gle = %u", ipv4, WSAGetLastError() log_end;
            return false;
        }
    }

    return false;
}

/// @brief  
bool str_to_ipv6(_In_ const wchar_t* ipv6, _Out_ in6_addr& ipv6_addr)
{
    _ASSERTE(NULL != ipv6);
    if (NULL != ipv6)
    {
        int ret = InetPtonW(AF_INET6, ipv6, &ipv6_addr);
        switch (ret)
        {
        case 1:
            return true;    // success
        case 0:
            log_err "invalid ipv4 string. input = %ws", ipv6 log_end;
            return false;
        case -1:
            log_err "InetPtonW() failed. input = %ws, wsa gle = %u", ipv6, WSAGetLastError() log_end;
            return false;
        }
    }

    return false;
}
#endif//#if (NTDDI_VERSION >= NTDDI_VISTA)


/// @brief  
bool get_ip_by_hostname(_In_ const wchar_t* host_name, _Out_ std::wstring& ip_string)
{
    _ASSERTE(NULL != host_name);
    if (NULL == host_name) return false;

    bool ret = false;

    ADDRINFOW hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    do
    {
        PADDRINFOW Result = NULL;
        if (0 != GetAddrInfoW(host_name, NULL, &hints, &Result) || NULL == Result)
        {
            log_err 
                "GetAddrInfoW(host_name = %s) failed, wserr=0x%08x", 
                host_name, 
                WSAGetLastError() log_end;
            break;
        }

        for (PADDRINFOW p = Result; p != NULL; p = p->ai_next)
        {
            if (AF_INET == p->ai_family)
            {
                sockaddr_in* sa = (sockaddr_in*)p->ai_addr;

#if (NTDDI_VERSION >= NTDDI_VISTA)
                wchar_t addr[16] = { 0 };
                if (NULL == InetNtop(AF_INET, (PVOID)&sa->sin_addr, addr, 16))
                {
                    log_err "InetNtop() failed. gle = %u", WSAGetLastError() log_end;
                    continue;
                }
                ip_string = addr;                
#else
                ip_string = MbsToWcsEx(inet_ntoa(sa->sin_addr)));
#endif

                ret = true;
                break;
            }
        }
        
        FreeAddrInfoW(Result);    
    } while (false);
    WSACleanup();

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
			// GetComputerNameExW() �� �����ϴ� NetbiosNameLen �� null �� �����Ѵ�.
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
				netbios_name[NetbiosNameLen] = 0x00;
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

#if (NTDDI_VERSION >= NTDDI_VISTA)
            wchar_t addr[16] = {0};
            if (NULL == InetNtop(AF_INET, (PVOID)&sa->sin_addr, addr, 16))
            {
                log_err "InetNtop() failed. gle = %u", WSAGetLastError() log_end
                continue;
            }            
            ip_list.push_back( addr );
#else
            ip_list.push_back( MbsToWcsEx(inet_ntoa(sa->sin_addr)) );
#endif
		}            
	}

	FreeAddrInfoW(Result); 
	WSACleanup();
	return true;
}

/// @brief  `ip_str` �ּҸ� ���� interface �� mac �ּҸ� �˾Ƴ���. 
///         ����Ʈ �ý����� mac �ּҸ� �˾ƿ����� ������ �ƴϰ�, ���� �ý��ۿ� �����
///         interface �� �� �ּҸ� �˾ƿ��� �뵵�� �Լ���. 
///         `127.0.0.1` �� �ȵ�
/// 
///         GetAdaptersAddresses() �Լ��� ����� ���� ������ �̰� �� �����ؼ� ...
///         cf. IP_ADAPTER_ADDRESSES  
///         https://msdn.microsoft.com/en-us/library/windows/desktop/aa366058(v=vs.85).aspx
/// 
/// @param  ip_str
bool get_local_mac_by_ipv4(_In_ const wchar_t* ip_str, _Out_ std::wstring& mac_str)
{
    _ASSERTE(NULL != ip_str);
    if (NULL == ip_str) return false;

    IN_ADDR src = { 0 };
    if (0 == InetPtonW(AF_INET, ip_str, &src))
    {
        log_err "InetPtonW( %ws ) failed." log_end;
        return false;
    }

    uint8_t     mac[8] = { 0 };
    uint32_t    cb_mac = sizeof(mac);
    DWORD ret = SendARP((IPAddr)src.S_un.S_addr, (IPAddr)src.S_un.S_addr, mac, (PULONG)&cb_mac);
    if (NO_ERROR != ret)
    {
        log_err "SendARP( %ws ) failed. ret = %u", ip_str, ret log_end;
        return false;
    }

    wchar_t buf[18] = { 0 };   // 01-34-67-9a-cd-f0[null]

    StringCbPrintfW(&buf[0], 6, L"%02x", mac[0]);
    StringCbPrintfW(&buf[2], 4, L"-");

    StringCbPrintfW(&buf[3], 6, L"%02x", mac[1]);
    StringCbPrintfW(&buf[5], 4, L"-");

    StringCbPrintfW(&buf[6], 6, L"%02x", mac[2]);
    StringCbPrintfW(&buf[8], 4, L"-");

    StringCbPrintfW(&buf[9], 6, L"%02x", mac[3]);
    StringCbPrintfW(&buf[11], 4, L"-");

    StringCbPrintfW(&buf[12], 6, L"%02x", mac[4]);
    StringCbPrintfW(&buf[14], 4, L"-");

    StringCbPrintfW(&buf[15], 6, L"%02x", mac[5]);
    
    mac_str = buf;
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
    if (IsWindowsXPOrGreater())
	{
		HANDLE hToken;
		if ( TRUE != OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken) )
		{
			if (GetLastError() == ERROR_NO_TOKEN)
			{
				if (ImpersonateSelf(SecurityImpersonation)	!= TRUE ) 
				{
					log_err "ImpersonateSelf( ) failed. gle=%u", GetLastError() log_end
					return false;
				}

				if (TRUE != OpenThreadToken(GetCurrentThread(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,FALSE,&hToken))
				{
					log_err "OpenThreadToken() failed. gle=%u", GetLastError() log_end
					return false;
				}
			}
			else
			{
				log_err "OpenThread() failed. gle=%u", GetLastError() log_end
				return false;
			}
		}  

		TOKEN_PRIVILEGES tp = { 0 };		
		LUID luid = {0};
		DWORD cb = sizeof(TOKEN_PRIVILEGES);
		if(!LookupPrivilegeValue( NULL, privilege, &luid ))
		{		
			log_err "LookupPrivilegeValue() failed. gle=%u", GetLastError() log_end
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
				log_err "AdjustTokenPrivileges() failed. gle=%u", GetLastError() log_end
				CloseHandle(hToken);				
			    return false;
		    }		
        }        

		CloseHandle(hToken);
	}

	return true;
}

/**
* @brief	raise_privilege �� true �� ��� SE_DEBUG_NAME ������ ȹ���� �� ���μ����� �����Ѵ�. 
			ȹ���� ������ ��������� �������ٶ����� ��� ���� �ȴ�. 
			SE_DEBUG_NAME �� ���� �־��� ���� �־��� ������ �����ϸ� ������ ���� �� �ְ�,
			�� �Լ��� ������ �ڵ鿡 I/O �� �߻��� �� ������ �� �ִ� SE_DEBUG_NAME �� ��� ������ 
			���� �� �ִ� (���� ���� ����...)
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
    #pragma warning(disable:4127)
	do 
	{
		// open the process
		proc_handle = OpenProcess(rights, FALSE, pid);
		if (NULL != proc_handle) break;		// ����!

		// SeDebugPrivilege	������ Ȱ��ȭ�ϰ� �� �õ� �غ���. (������ ������ �ȵ� ���� ����)
		if (true != raise_privilege) break;	// ����!

		if (true != set_privilege(SE_DEBUG_NAME, TRUE) ) break;
		
		// SeDebugPrivilege �� ����ٸ� �ٽ� ���� �õ�
		proc_handle = OpenProcess(rights, FALSE, pid);
		if (NULL == proc_handle)
		{
			//log_info "OpenProcess( pid = %u ) failed. gle = %u", pid, GetLastError() log_end
		}
		
	} while(false);
    #pragma warning(default: 4127)

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
 * @param	process_id �� PROCESS_QUERY_INFORMATION ������ �ʿ���
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
 * @brief	process_id �� �ּܼ��ǿ��� �������� ��� true ����
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

/// @brief	active console session �� �α��ε� ����� �������� ���μ����� �����Ѵ�.
///	@remark �̰� ������� ������ �Լ��̹Ƿ� ��������. 
bool 
create_process_as_login_user(
	_In_ uint32_t session_id,
	_In_ const wchar_t* cmdline, 
	_Out_ PROCESS_INFORMATION& pi
	)
{
	_ASSERTE(NULL != cmdline);
	if (NULL == cmdline) return false;

	if (session_id == 0xffffffff)
	{
		session_id = WTSGetActiveConsoleSessionId();
	}
	DWORD explorer_pid = 0xFFFFFFFF;

	// 
	//	Ÿ�� ������ explorer.exe ���μ����� ã��, 
	//	�ش� ���μ����� ��ū���� ���μ����� �����Ѵ�.
	// 
	PROCESSENTRY32 proc_entry = { 0 };
	DWORD creation_flag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
	LPVOID env_block = NULL;
	size_t cmd_len = 0;
	wchar_t* cmd = NULL;	
	STARTUPINFO si = { 0 }; 
	TOKEN_PRIVILEGES tp = { 0 };
	LUID luid = { 0 };	
	HANDLE snap = INVALID_HANDLE_VALUE;
	HANDLE primary_token = NULL;
	HANDLE duplicated_token = NULL;
	HANDLE process_handle = NULL;

	bool ret = false;
	do
	{
		snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snap == INVALID_HANDLE_VALUE)
		{
			
			log_err "CreateToolhelp32Snapshot(), gle=%u", GetLastError() log_end;
			break;
		}

		do
		{
			proc_entry.dwSize = sizeof(PROCESSENTRY32);
			if (!Process32First(snap, &proc_entry))
			{
				log_err "Process32First(), gle=%u", GetLastError() log_end;
				break;
			}

			do
			{
				if(true == rstrnicmp(proc_entry.szExeFile, L"explorer.exe"))
				{
					DWORD explorer_sessio_id = 0;
					if (ProcessIdToSessionId(proc_entry.th32ProcessID, &explorer_sessio_id) &&
						explorer_sessio_id == session_id)
					{
						explorer_pid = proc_entry.th32ProcessID;
						break;
					}
				}
			} while (Process32Next(snap, &proc_entry));
		} while (false);

		CloseHandle(snap);

		if (0xFFFFFFFF == explorer_pid)
		{
			log_err "Can not find 'explorer.exe'" log_end;			
			break;
		}

		si.cb = sizeof(si);
		si.lpDesktop = L"winsta0\\default";

		process_handle = OpenProcess(MAXIMUM_ALLOWED, 
									 FALSE, 
									 explorer_pid);
		if (NULL == process_handle)
		{
			log_err "OpenProcess(), gle=%u", GetLastError() log_end;
			break;
		}

		if (TRUE != OpenProcessToken(process_handle,
									 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID | TOKEN_READ | TOKEN_WRITE,
									 &primary_token))
		{
			log_err "OpenProcessToken(), gle=%u", GetLastError() log_end;
			break;
		}

		if (TRUE != LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
		{
			log_err "LookupPrivilegeValue(), gle=%u", GetLastError() log_end;
			break;
		}

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (TRUE != DuplicateTokenEx(primary_token, 
									 MAXIMUM_ALLOWED, 
									 NULL, 
									 SecurityIdentification, 
									 TokenPrimary, 
									 &duplicated_token))
		{
			log_err "DuplicateTokenEx(), gle=%u", GetLastError() log_end;
			break;
		}

		//> adjust token privilege
		if (TRUE != SetTokenInformation(duplicated_token, 
										TokenSessionId, 
										(void*)(DWORD_PTR)session_id, 
										sizeof(DWORD)))
		{
			//log_err L"SetTokenInformation(), gle=0x%08x", GetLastError() log_end		
			// note - �� ������ �����ص� �ȴ�. 
		}

		if (TRUE != AdjustTokenPrivileges(duplicated_token, 
										  FALSE, 
										  &tp, 
										  sizeof(TOKEN_PRIVILEGES), 
										  (PTOKEN_PRIVILEGES)NULL, 
										  NULL))
		{
			DWORD err = GetLastError();
			if (ERROR_NOT_ALL_ASSIGNED != err)
			{
				log_err "AdjustTokenPrivileges(), gle=%u", err log_end;
				break;
			}

			// ERROR_NOT_ALL_ASSIGNED is OK!
		}

		if (TRUE == CreateEnvironmentBlock(&env_block, 
										   duplicated_token, 
										   TRUE))
		{
			creation_flag |= CREATE_UNICODE_ENVIRONMENT;
		}

		//> create process in the consol session
		cmd_len = (wcslen(cmdline) + 1) * sizeof(wchar_t);
		cmd = (wchar_t*)malloc(cmd_len);
		if (NULL == cmd)
		{
			log_err "insufficient memory for cmdline" log_end;
			break;
		}
		StringCbPrintfW(cmd, cmd_len, L"%s", cmdline);

		si.wShowWindow = SW_HIDE;

		if (TRUE != CreateProcessAsUser(duplicated_token,
										NULL,
										cmd,
										NULL,
										NULL,
										FALSE,
										creation_flag,
										env_block,
										NULL,
										&si,
										&pi))
		{
			log_err "CreateProcessAsUserW(), gle=%u", GetLastError() log_end;
			break;
		}

		if (NULL == pi.hProcess)
		{
			log_err "CreateProcessAsUserW() failed. gle=%u", GetLastError() log_end;
			break;
		}

		//
		//	OK!!!
		// 
		ret = true;

	} while (FALSE);
	

	if (NULL != cmd)
	{
		free(cmd);
	}

	if (NULL != process_handle)
	{
		CloseHandle(process_handle);
	}
	if (NULL != primary_token)
	{
		CloseHandle(primary_token);
	}
	if (NULL != duplicated_token)
	{
		CloseHandle(duplicated_token);
	}
	
	if (NULL != env_block)
	{
		DestroyEnvironmentBlock(env_block);
	}

	return ret;
}

/// @brief	authenticated user �� ���ٰ�����  DACL �� �����Ѵ�. 
bool set_security_attributes(_Out_ SECURITY_ATTRIBUTES& sa)
{
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = FALSE;

	//	Creating a DACL
	//	https://msdn.microsoft.com/en-us/library/windows/desktop/ms717798(v=vs.85).aspx
	// 
	//	Security Descriptor String Format
	//	https://msdn.microsoft.com/en-us/library/windows/desktop/aa379570(v=vs.85).aspx
	//
	//	ACE Strings
	//	https://msdn.microsoft.com/en-us/library/windows/desktop/aa374928(v=vs.85).aspx
	//
	//	Define the SDDL for the DACL.This example sets the following access:
	//     Built-in guests are denied all access.
	//     Anonymous logon is denied all access.
	//     Authenticated users are allowed read/write/execute access.
	//     Administrators are allowed full control.
	//
	//	Modify these values as needed to generate the proper DACL for your application. 

	wchar_t* dacl_text = \
		L"D:"					// Discretionary ACL
		L"(D;OICI;GA;;;BG)"		// Deny access to built-in guests
		L"(D;OICI;GA;;;AN)"     // Deny access to anonymous logon
		L"(A;OICI;GRGWGX;;;AU)" // Allow 
								// read/write/execute 
								// to authenticated 
								// users
		L"(A;OICI;GA;;;BA)";    // Allow full control to administrators

	BOOL ret = ConvertStringSecurityDescriptorToSecurityDescriptorW(dacl_text,
																	SDDL_REVISION_1,
																	&(sa.lpSecurityDescriptor),
																	NULL);
	return ret ? true : false;
}

/**
 * @brief	pid �� ���μ����� ��ü �̸��� ���Ѵ�. (vista �̻�)
			ZwQuerySystemInformation ó�� length �� 0 �� �Ѱ��ָ� �ʿ��� ������ ���̸�...
			������ ���� �ʴ´�. ŭ���ϰ� ���� ��Ƽ� ȣ���ؾ� ��
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	c:\dbg\sound.dll ���� (Win32 format) string ����
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
	
	for(int i = 0; i < 3; ++i)		// name buffer �� �ι辿 Ű��°͵� 3ȸ�� �õ��Ѵ�.
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
void write_to_console(_In_ uint32_t color, _In_z_ const char* log_message)
{
	_ASSERTE(NULL != log_message);
	if (NULL == log_message) return;

	static HANDLE	con_stdout_handle = INVALID_HANDLE_VALUE;
	static WORD		con_old_color = 0;

	if (INVALID_HANDLE_VALUE == con_stdout_handle)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		con_stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(con_stdout_handle, &csbi);
		con_old_color = csbi.wAttributes;
	}

	DWORD len;
    switch (color)
    {
    case wtc_red:
        SetConsoleTextAttribute(con_stdout_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteConsoleA(con_stdout_handle, log_message, (DWORD)strlen(log_message), &len, NULL);
        SetConsoleTextAttribute(con_stdout_handle, con_old_color);
        break;
    case wtc_green:
        SetConsoleTextAttribute(con_stdout_handle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        WriteConsoleA(con_stdout_handle, log_message, (DWORD)strlen(log_message), &len, NULL);
        SetConsoleTextAttribute(con_stdout_handle, con_old_color);
        break;
    case wtc_none:
    default:
        WriteConsoleA(con_stdout_handle, log_message, (DWORD)strlen(log_message), &len, NULL);
    }
}

/// @brief  clear console
///         https://msdn.microsoft.com/en-us/library/windows/desktop/ms682022(v=vs.85).aspx
void clear_console()
{
    COORD coordScreen = { 0, 0 };    // home for the cursor 
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get the number of character cells in the current buffer. 
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)){ return; }
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire screen with blanks.
    if (!FillConsoleOutputCharacter(hConsole,        // Handle to console screen buffer 
                                    (TCHAR) ' ',     // Character to write to the buffer
                                    dwConSize,       // Number of cells to write 
                                    coordScreen,     // Coordinates of first cell 
                                    &cCharsWritten))// Receive number of characters written
    {
        return;
    }

    // Get the current text attribute.
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) { return; }

    // Set the buffer's attributes accordingly.
    if (!FillConsoleOutputAttribute(hConsole,         // Handle to console screen buffer 
                                    csbi.wAttributes, // Character attributes to use
                                    dwConSize,        // Number of cells to set attribute 
                                    coordScreen,      // Coordinates of first cell 
                                    &cCharsWritten)) // Receive number of characters written
    {
        return;
    }

    // Put the cursor at its home coordinates.
    SetConsoleCursorPosition(hConsole, coordScreen);
}

/** ---------------------------------------------------------------------------
    \brief  ���� ������ ������ ��� TRUE ����

    \param  path            exe ������ full path    
            type            file type
    \return         
            ������ TRUE ����
            ���н� FALSE ����, 
                
                
    \code
    \endcode        
-----------------------------------------------------------------------------*/
bool is_executable_file_w(_In_ const wchar_t* path, _Out_ IMAGE_TYPE& type)
{
	type = IT_NORMAL;

    HANDLE hFile = CreateFileW(
                    path, 
                    GENERIC_READ, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL, 
                    OPEN_EXISTING, 
                    FILE_ATTRIBUTE_NORMAL, 
                    NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        //log_err "access denied or invalid path, %ws, gle = %u", path, GetLastError() log_end
        return false;
    }
    SmrtHandle sfFile(hFile);

    // check file size 
    // 
    LARGE_INTEGER fileSize;
    if (TRUE != GetFileSizeEx(hFile, &fileSize))
    {
        log_err "%ws, can not get file size, errorcode=0x%08x", path, GetLastError() log_end
        return false;
    }
    if (sizeof(IMAGE_DOS_HEADER) > fileSize.QuadPart) return false;

    
    HANDLE hImageMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (NULL == hImageMap){return false;}
    SmrtHandle sfMap(hImageMap);

    PBYTE ImageView = (LPBYTE) MapViewOfFile(
                            hImageMap, 
                            FILE_MAP_READ, 
                            0, 
                            0, 
                            0
                            );
    if(ImageView == NULL){return false;}
    SmrtView sfView(ImageView);


	//
	// PE �� offset ������ �ŷ��� �� ���� ������ SEH �� �̿��Ѵ�. 
	// ������ IsBadReadPtr() �Լ����� �� �̻� ������� �ʴ´�. 
	//
	try
	{
		PIMAGE_DOS_HEADER idh = (PIMAGE_DOS_HEADER)ImageView;
		if (idh->e_magic != IMAGE_DOS_SIGNATURE) return false;

		// check DOS file size
		// 
		DWORD dosSize = (idh->e_cp * 512);
		if (dosSize > fileSize.QuadPart)
		{
			log_err "%ws, invalid file size, size=%lu", path, fileSize.QuadPart log_end;
			return false;
		}

		PIMAGE_NT_HEADERS inh = (PIMAGE_NT_HEADERS)((DWORD_PTR)idh + idh->e_lfanew);
		if (IMAGE_NT_SIGNATURE != inh->Signature) return false;

		WORD subsys = inh->OptionalHeader.Subsystem;
		WORD characteristics = inh->FileHeader.Characteristics;

		// characteristics check
		if ((characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) != IMAGE_FILE_EXECUTABLE_IMAGE) return false;   // not executable

		if ((characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL)
		{
			type = IT_DLL;
			return true;
		}

		// never called.. ???
		// if ((characteristics & IMAGE_FILE_SYSTEM) == IMAGE_FILE_SYSTEM){OutputDebugStringW(" IMAGE_FILE_SYSTEM");}


		// subsystem check
		// 
		if ((subsys & IMAGE_SUBSYSTEM_NATIVE) == IMAGE_SUBSYSTEM_NATIVE)
		{
			type = IT_NATIVE_APP;
			return true;
		}

		if ((subsys & IMAGE_SUBSYSTEM_WINDOWS_GUI) == IMAGE_SUBSYSTEM_WINDOWS_GUI)
		{
			type = IT_EXE_GUI;
			return true;
		}
		if ((subsys & IMAGE_SUBSYSTEM_WINDOWS_CUI) == IMAGE_SUBSYSTEM_WINDOWS_CUI)
		{
			type = IT_EXE_CUI;
			return true;
		}
	}
	catch(std::exception& e)
	{
		log_warn "Invalid/Malformed pe file, exception=%s", e.what() log_end;
		return false;
	}

    return false;
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
bool str_to_int32(_In_ const char* int32_string, _Out_ int32_t& int32_val)
{
	if (NULL == int32_string) return false;
	
	errno = 0;
	int32_val = strtol(int32_string, NULL, 10);
	if (ERANGE == errno) { return false; }
	
	return true;
}

bool str_to_uint32(_In_ const char* uint32_string, _Out_ uint32_t& uint32_val)
{
	if (NULL == uint32_string) return false;

	//> ���ڿ� ���� ���� ��� ����, '-' �� �����ϴ� ��� ����ó��
	std::string str = uint32_string;
	trima(str);
	if (str[0] == '-') return false;
	
	errno = 0;
	uint32_val = strtoul(str.c_str(), NULL, 10);
	if (ERANGE == errno) { return false; }
	return true;	
}

bool str_to_int64(_In_ const char* int64_string, _Out_ int64_t& int64_val)
{
	if (NULL == int64_string) return false;

	errno = 0; 
	int64_val = _strtoi64(int64_string, NULL, 10);
	if (ERANGE == errno) 
	{ 
		// msdn ���� ERANGE �� �����Ѵٴ� ������ ����, _I64_MAX, _I64_MIN �� �����Ѵٰ� �ϴµ�,
		// ������ ������ �Ѵ� input �� �־�� ERANGE �� �������� Ȯ������
		//
		// test_str_to_xxx() �׽�Ʈ Ȯ��		
		return false; 
	}

	if (EINVAL == errno)
	{		
		return false;
	}

	return true;
}

bool str_to_uint64(_In_ const char* uint64_string, _Out_ uint64_t& uint64_val)
{
	if (NULL == uint64_string) return false;

	//> ���ڿ� ���� ���� ��� ����, '-' �� �����ϴ� ��� ����ó��
	std::string str = uint64_string;
	trima(str);
	if (str[0] == '-') return false;

	errno = 0; 
	uint64_val = _strtoui64(str.c_str(), NULL, 10);
	if (ERANGE == errno) 
	{ 
		// msdn ���� ERANGE �� �����Ѵٴ� ������ ����, _UI64_MAX �� �����Ѵٰ� �ϴµ�,
		// ������ ������ �Ѵ� input �� �־�� ERANGE �� �������� Ȯ������
		//
		// test_str_to_xxx() �׽�Ʈ Ȯ��
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

bool wstr_to_int32(_In_ const wchar_t* int32_string, _Out_ int32_t& int32_val)
{
	if (NULL == int32_string) return false;
	return str_to_int32(WcsToMbsEx(int32_string).c_str(), int32_val);	
}

bool wstr_to_uint32(_In_ const wchar_t* uint32_string, _Out_ uint32_t& uint32_val)
{
	if (NULL == uint32_string) return false;
	return str_to_uint32(WcsToMbsEx(uint32_string).c_str(), uint32_val);
}

bool wstr_to_int64(_In_ const wchar_t* int64_string, _Out_ int64_t& int64_val)
{
	if (NULL == int64_string) return false;
	return str_to_int64(WcsToMbsEx(int64_string).c_str(), int64_val);
}

bool wstr_to_uint64(_In_ const wchar_t* uint64_string, _Out_ uint64_t& uint64_val)
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

/**
* @brief	cpu ������ �����Ѵ�. 
* @param	
* @see		
* @remarks  xp sp3 �̻󿡼��� ��� ����
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


/// @brief 
const wchar_t*  osver_to_str(_In_ OSVER os)
{
    switch (os)
    {
    case OSV_UNDEF: return L"Undef";
    case OSV_2000: return L"Windows 2000";
    case OSV_XP: return L"Windows XP";
    case OSV_XPSP1: return L"Windows XP (SP1)";
    case OSV_XPSP2: return L"Windows XP (SP2)";
    case OSV_XPSP3: return L"Windows XP (SP3)";
    case OSV_2003: return L"Windows 2000";
    case OSV_VISTA: return L"Windows Vista";
    case OSV_VISTASP1: return L"Windows Vista (SP1)";
    case OSV_VISTASP2: return L"Windows Vista (SP2)";
    case OSV_2008: return L"Windows Server 2008";
    case OSV_7: return L"Windows 7";
    case OSV_7SP1: return L"Windows 7 (SP1)";
    case OSV_2008R2: return L"Windows Server 2008 R2";
    case OSV_8: return L"Windows 8";
    case OSV_81: return L"Windows 8.1";
    case OSV_2012R2: return L"Windows Server 2012 R2";
    case OSV_10: return L"Windows 10";
    case OSV_UNKNOWN:
    default:
        return L"Unknown OS";
    }
}

/// @brief  RtlGetVersion() wrapper
OSVER get_os_version()
{
    static OSVER os = OSV_UNDEF;
    if (os != OSV_UNDEF) return os;
    os = OSV_UNKNOWN;

    // https://github.com/DarthTon/Blackbone/blob/master/contrib/VersionHelpers.h 
    RTL_OSVERSIONINFOEXW osv = { sizeof(osv), 0, };    
    typedef uint32_t(__stdcall *fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);
    fnRtlGetVersion RtlGetVersion = (fnRtlGetVersion)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");

    if (RtlGetVersion != 0 && RtlGetVersion((PRTL_OSVERSIONINFOW)&osv) == 0 && osv.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        switch (osv.dwMajorVersion)
        {
        case 5:
            os = OSV_2000;
            if (osv.dwMinorVersion == 1)
            {
                os = OSV_XP;
                if (osv.wServicePackMajor == 1) os = OSV_XPSP1;
                else if (osv.wServicePackMajor == 2) os = OSV_XPSP2;
                else if (osv.wServicePackMajor == 3) os = OSV_XPSP3;
            }
            else if (osv.dwMinorVersion == 2)   os = OSV_2003;
            break;

        case 6:
            if (osv.dwMinorVersion == 0)
            {
                os = OSV_VISTA;
                if (osv.wProductType == VER_NT_WORKSTATION)
                {
                    if (osv.wServicePackMajor == 1) os = OSV_VISTASP1;
                    else if (osv.wServicePackMajor == 2) os = OSV_VISTASP2;
                }
                else
                    os = OSV_2008;
            }
            else if (osv.dwMinorVersion == 1)
            {
                if (osv.wProductType == VER_NT_WORKSTATION)
                {
                    os = OSV_7;
                    if (osv.wServicePackMajor == 1) os = OSV_7SP1;
                }
                else
                    os = OSV_2008R2;
            }
            else if (osv.dwMinorVersion == 2)
                os = OSV_8;
            else if (osv.dwMinorVersion == 3)
            {
                if (osv.wProductType == VER_NT_WORKSTATION)
                    os = OSV_81;
                else
                    os = OSV_2012R2;
            }
            break;

        case 10:
            os = OSV_10;
            break;
        }
    }

    return os;
}