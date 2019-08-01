/**
 * @file    Windows API wrapper and utility routines.
 * @brief
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011/08/26 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "BaseWindowsHeader.h"

#include <random>

#include <errno.h>
#include <io.h>			// _setmode()
#include <fcntl.h>		// _O_U8TEXT, ...
#include <VersionHelpers.h>
#include <time.h>
#include <Shellapi.h>
#include <Shlobj.h>
#include <Psapi.h>
#include <sddl.h>
#include <Objbase.h>	// CoCreateGuid()

#include <TLHELP32.H>
#include <userenv.h>
#include <Wtsapi32.h>
#pragma comment (lib, "userenv.lib")
#pragma comment (lib, "Wtsapi32.lib")

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "Ole32.lib")

#include "Win32Utils.h"
#include "RegistryUtil.h"
#include "Wow64Util.h"

#include "md5.h"
#include "sha2.h"
#include "FileIoHelperClass.h"
#include "ResourceHelper.h"
#include "gpt_partition_guid.h"


char _int_to_char_table[] = {
	"0123456789" /* 0 - 9 */
	"abcdefghijklmnopqrstuvwxyz" /* 10 - 35 */
	" !\"#$%&'()*+,-./" /* 36 - 51 */
	":;<=>?@" /* 52 - 58 */
	"[\\]^_`" /* 59 - 64 */
	"{|}~" /* 65 - 68 */
};

char _int_to_uchar_table[] = {
	"0123456789" /* 0 - 9 */
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ" /* 10 - 35 */
	" !\"#$%&'()*+,-./" /* 36 - 51 */
	":;<=>?@" /* 52 - 58 */
	"[\\]^_`" /* 59 - 64 */
	"{|}~" /* 65 - 68 */
};


/// @brief	int type �������� �����Ѵ�.
int get_random_int(_In_ int min, _In_ int max)
{
	std::random_device seed;
	std::default_random_engine re(seed());
	std::uniform_int_distribution<uint32_t> range(min, max);
	return range(re);
}

/// @brief	
std::string FAT2Str(IN FATTIME& fat)
{
	FILETIME	ft = { 0 };

	if (0x00 != fat.usDate && 0x00 != fat.usTime)
	{
		DosDateTimeToFileTime(fat.usDate, fat.usTime, &ft);
	}

	return file_time_to_str(&ft, true, false);
}

/// @brief	FILETIME -> uint64_t
///	@remark	Do not cast a pointer to a FILETIME structure to either a ULARGE_INTEGER* or __int64*
///			https://msdn.microsoft.com/en-us/library/ms724284(VS.85).aspx
///			https://devblogs.microsoft.com/oldnewthing/20040825-00/?p=38053
/*
	
		typedef struct _FILETIME {
			DWORD dwLowDateTime;
			DWORD dwHighDateTime;
		} FILETIME, *PFILETIME, *LPFILETIME;

	FILETIME ����ü�� 4����Ʈ ���� 2���� �����Ǿ� �����Ƿ� 4 byte alignment �Ǿ�� �Ѵ�.
	�� ��� FILETIME -> __int64 �� ��ȯ�ص� ������ �߻������� �ʴ´�.

	+----------------------------------------------------------------------------+

	typedef struct _WIN32_FIND_DATAW {
		DWORD dwFileAttributes;
		FILETIME ftCreationTime;
		FILETIME ftLastAccessTime;
		FILETIME ftLastWriteTime;
		DWORD nFileSizeHigh;
		DWORD nFileSizeLow;
		...
	} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;

	WIN32_FIND_DATAW ����ü�� ��� dwFileAttributes �� 4����Ʈ�̹Ƿ�, 
		- ftCreationTime   = 0x00000004 ~ 0x0000000C 
		- ftLastAccessTime = 0x0000000c ~ 0x00000020 
	�� ��ġ�ϰ� �ȴ�. 

	������ ������ pointer alignment(8����Ʈ)�� �Ǿ�� �ϴµ�, ���� ftCreationTime �� 
	�ּ�(0x00000004)�� __int64_t* Ÿ���� �����ͷ� ĳ�����ؼ� �������ϰ� �Ǵ� ��� 
	__int64_t �����ʹ� pointer alignment(8����Ʈ)�� �ƴ϶� 4 byte alignment �޸� ������
	�����ϰ� �Ǳ� ������ alignment �� ���Ѿ� �ϴ� ��� ������ �߻��� �� �ִ�. 

	Ŀ���� �޼��� ����ó���� �Ʒ��� ���� �ڵ���� ���� �� �� �ִ�. 

		if (!IS_ALIGNED(OutputBuffer, sizeof(uint64_t*))) {
			return STATUS_DATATYPE_MISALIGNMENT;
		}

	���� ftCreateTime �� __uint64_t* �� ����ĳ�����ؼ� Ŀ�μ��� ��ƾ�� ȣ���ߴٸ� 
	STATUS_DATATYPE_MISALIGNMENT ������ ���ϵ� ���̴�. 

	+----------------------------------------------------------------------------+

		typedef union _LARGE_INTEGER 
		{
			struct {
				DWORD LowPart;
				LONG HighPart;
			} DUMMYSTRUCTNAME;

			struct {
				DWORD LowPart;
				LONG HighPart;
			} u;

			LONGLONG QuadPart;
		} LARGE_INTEGER;

	LARGE_INTEGER, ULARGE_INTEGER �� union ���� ���ǵǾ��ְ� LONGLONG Ÿ�� ������ �ֱ⶧����
	�⺻���� 8byte align �� ����ϱ� ������ ���� ������ �߻����� �ʴ´�. 

	����� LowPart, HighPart �� ������ ������ ����ü�� union �ȿ� �ι� ������ ������ ���� �ڵ���� 
	ȣȯ���� ���� ���̶�� �� (�̸� ���� ����ü�� ������ ���Ͽ��� �����ϴ��� ���³� �� �׷�...)
	������ ���Ͽ��� ������ �θ� �Ʒ� �ΰ��� ������ �ڵ带 ��� ����� �� �ֱ� �����̶�� ��
	
		LargeInteger.u.Lowpart = 0
		LargeInteger.LowPart = 0 		
*/
__inline 
uint64_t
file_time_to_int(
	_In_ const PFILETIME file_time
)
{
	_ASSERTE(nullptr != file_time);
	if (nullptr == file_time) { return (uint64_t)(-1); }

	LARGE_INTEGER li;
	li.LowPart = file_time->dwLowDateTime;
	li.HighPart = file_time->dwHighDateTime;
	return li.QuadPart;
}

/// @brief	uint64_t -> FILETIME
///	@remark	Do not cast a pointer to a FILETIME structure to either a ULARGE_INTEGER* or __int64*
///			https://msdn.microsoft.com/en-us/library/ms724284(VS.85).aspx
///			https://devblogs.microsoft.com/oldnewthing/20040825-00/?p=38053
///	
__inline 
void
int_to_file_time(
	_In_ uint64_t file_time_int,
	_Out_ PFILETIME const file_time
)
{
	_ASSERTE(nullptr != file_time);
	if (nullptr == file_time) return;

	large_int_to_file_time(reinterpret_cast<PLARGE_INTEGER>(&file_time_int), file_time);
}

/// @brief	LARGE_INTEGER -> FILETIME ���� ��ȯ
///			** NOTE **
///			Ư���� ��찡 �ƴ϶�� large_int_to_file_time() �Լ��� ����� �ʿ���� 
///			`PFILETIME ft = (PFILETIME)&large_int` ���·� �ٷ� ĳ�����ؼ� ����ϵ� ��������. 
///			
__inline 
void
large_int_to_file_time(
	_In_ const PLARGE_INTEGER large_int,
	_Out_ PFILETIME const file_time
)
{
	_ASSERTE(nullptr != large_int);
	if (nullptr == large_int) return;

	file_time->dwLowDateTime = large_int->LowPart;
	file_time->dwHighDateTime = large_int->HighPart;
}

/// @brief	unixtime(DWORD) -> FILETIME
///	@remark	Do not cast a pointer to a FILETIME structure to either a ULARGE_INTEGER* or __int64*
///			https://msdn.microsoft.com/en-us/library/ms724284(VS.85).aspx
///			https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
void
unixtime_to_filetime(
	_In_ uint32_t unix_time,
	_Out_ PFILETIME const file_time)
{
	_ASSERTE(nullptr != file_time);
	if (nullptr == file_time) return;

	//
	// time_t to filetime
	//
	LONGLONG ll = Int32x32To64(unix_time, 10000000) + 116444736000000000;
	file_time->dwLowDateTime = (DWORD)ll;
	file_time->dwHighDateTime = ll >> 32;
}

/// @brief	ftl, ftr ���� ���� �и������� ������ �����Ѵ�. 
int64_t
file_time_delta_msec(
	_In_ const PFILETIME ftl,
	_In_ const PFILETIME ftr
)
{
	return (llabs(file_time_to_int(ftl) - file_time_to_int(ftr)) / _file_time_to_msec);
}

/// @brief  ftl, ftr ���� ���� �ʴ����� �����Ѵ�. 
int64_t
file_time_delta_sec(
	_In_ const PFILETIME ftl,
	_In_ const PFILETIME ftr
)
{
	return (llabs(file_time_to_int(ftl) - file_time_to_int(ftr)) / _file_time_to_sec);
}

/// @brief	ftl, ftr ���� ���� �ϴ����� �����Ѵ�. 
int64_t
file_time_delta_day(
	_In_ const PFILETIME ftl,
	_In_ const PFILETIME ft2
)
{
	return (llabs(file_time_to_int(ftl) - file_time_to_int(ft2)) / _file_time_to_day);
}

/// @brief file_time �� secs �ʸ� ���� ����Ÿ���� �����Ѵ�.
FILETIME
add_sec_to_file_time(
	_In_ const PFILETIME file_time,
	_In_ int32_t secs
)
{
	if (secs == 0) return *file_time;

	uint64_t file_time_uint64_t = file_time_to_int(file_time) + (secs * _file_time_to_sec);

	FILETIME added_file_time;
	int_to_file_time(file_time_uint64_t, &added_file_time);
	return added_file_time;
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


/// @brief	���� �ð��� `2017-05-23 21:23:24.821` ���� ���ڿ��� ����Ѵ�. 
std::string	time_now_to_str(_In_ bool localtime, _In_ bool show_misec)
{
	SYSTEMTIME utc_system_time;
	GetSystemTime(&utc_system_time);

	return sys_time_to_str(&utc_system_time, localtime, show_misec);
}


/// @brief	���� �ð��� `2017-05-23T21:23:24.821+09:00` ���� ���ڿ��� ����Ѵ�. 
std::string	time_now_to_str2()
{
	SYSTEMTIME utc_system_time;
	GetSystemTime(&utc_system_time);

	return sys_time_to_str2(&utc_system_time);
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
std::string
sys_time_to_str(
	_In_ const PSYSTEMTIME utc_sys_time,
	_In_ bool localtime,
	_In_ bool show_misec
)
{
	char buf[32];

	SYSTEMTIME local;
	PSYSTEMTIME time = utc_sys_time;

	if (true == localtime)
	{
		SystemTimeToTzSpecificLocalTime(NULL, utc_sys_time, &local);
		time = &local;
	}

	HRESULT hr;

	if (!show_misec)
	{
		hr = StringCbPrintfA(buf, sizeof(buf),
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
		hr = StringCbPrintfA(buf, sizeof(buf),
							 "%04u-%02u-%02u %02u:%02u:%02u %u",
							 time->wYear,
							 time->wMonth,
							 time->wDay,
							 time->wHour,
							 time->wMinute,
							 time->wSecond,
							 time->wMilliseconds);
	}

	if (!SUCCEEDED(hr))
	{
		log_err "Can't make time string. hr=%u",
			hr
			log_end;
		return std::string("1601-01-01 00:00:000");
	}

	return std::string(buf);
}


/// @brief  `2017-05-23T21:23:24.821+09:00` ���� �ð� ���ڿ��� �����Ѵ�. 
///			[localtime][time zone] �����̹Ƿ� utc_sys_time �Ķ���͸� 
///			�ݵ�� localtime ���� ��ȯ�ؾ� �Ѵ�. 
///
///			https://www.w3.org/TR/NOTE-datetime ����
std::string
sys_time_to_str2(
	_In_ const PSYSTEMTIME utc_sys_time
)
{
	char buf[32];

	//
	//	local time ���� ��ȯ
	// 
	SYSTEMTIME local_sys_time;
	SystemTimeToTzSpecificLocalTime(NULL, utc_sys_time, &local_sys_time);


	//
	//	Ÿ���� ����
	//	UTC+xxx �� ��� TimeZone.Bias ���� ������ �����Ƿ� +/- ��ȣ�� �����Ѵ�.
	TIME_ZONE_INFORMATION lpTimeZone = { 0 };
	GetTimeZoneInformation(&lpTimeZone);

	char tz_sign = '-';
	if (0 > lpTimeZone.Bias)
	{
		tz_sign = '+';
	}


	HRESULT hr = StringCbPrintfA(buf,
								 sizeof(buf),
								 "%04u-%02u-%02uT%02u:%02u:%02u.%u%c%02u:%02u",
								 local_sys_time.wYear,
								 local_sys_time.wMonth,
								 local_sys_time.wDay,
								 local_sys_time.wHour,
								 local_sys_time.wMinute,
								 local_sys_time.wSecond,
								 local_sys_time.wMilliseconds,
								 tz_sign,
								 labs(lpTimeZone.Bias / 60),
								 labs(lpTimeZone.Bias % 60));
	if (!SUCCEEDED(hr))
	{
		log_err "Can't make time string. hr=%u",
			hr
			log_end;
		return std::string("1601-01-01T00:00:000+00:00");
	}

	return std::string(buf);
}


bool is_file_existsW(_In_ std::wstring& file_path)
{
	return is_file_existsW(file_path.c_str());
}

bool is_file_existsA(_In_ std::string& file_path)
{
	return is_file_existsA(file_path.c_str());
}

bool is_file_existsW(_In_ const wchar_t* file_path)
{
	_ASSERTE(NULL != file_path);
	if (NULL == file_path) return false;

	WIN32_FILE_ATTRIBUTE_DATA info = { 0 };

	//
	//	CreateFile()�� �ƴ� GetFileAttributesEx()�� �̿��ϸ� ������ 
	//	�ٸ� process�� ���� lock�Ǿ� �־
	//	���� ���翩�θ� ��Ȯ�� üũ�� �� �ִ�.
	//
	if (FALSE == GetFileAttributesExW(file_path,
									  GetFileExInfoStandard,
									  &info))
		return false;
	else
		return true;
}

bool is_file_existsA(_In_ const char* file_path)
{
	WCHAR* wcs = MbsToWcs(file_path);
	if (NULL == wcs) { return false; }

	bool ret = is_file_existsW(wcs); free(wcs);
	return ret;
}

/// @brief  `file_path` �� �����ϰ�, directory �̸� true ����
///         `file_path` �� ���ų�, �����̸� false ����
bool is_dir(_In_ const wchar_t* file_path)
{
	WIN32_FILE_ATTRIBUTE_DATA info = { 0 };
	if (TRUE == GetFileAttributesExW(file_path,
									 GetFileExInfoStandard,
									 &info))
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
	if (TRUE == GetFileAttributesExW(file_path,
									 GetFileExInfoStandard,
									 &info))
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

	LARGE_INTEGER file_size = { 0 };
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

	handle_ptr map_handle(
		CreateFileMapping(file_handle, NULL, PAGE_READONLY, 0, 1, NULL),
		[](HANDLE h) {
		if (INVALID_HANDLE_VALUE == h) return;
		if (NULL == h) return;
		CloseHandle(h);
	});

	if (NULL == map_handle.get())
	{
		log_err "CreateFileMapping(), gle = %u", GetLastError() log_end
			return false;
	}

	void_ptr map_ptr(
		MapViewOfFile(map_handle.get(), FILE_MAP_READ, 0, 0, 1),
		[](void* p) {
		if (NULL != p)
		{
			UnmapViewOfFile(p);
		}
	});

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

	for (;;)
	{
		if (NULL != buf) free(buf);

		buf = (wchar_t*)malloc((cch_buf + 1) * sizeof(wchar_t));	// add NULL 
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

	if (true == ret) file_name = buf;

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
	wchar_t drive_string[128 + 1] = { 0 };
	DWORD length = GetLogicalDriveStringsW(128, drive_string);
	if (0 == length)
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
	for (;;)
	{
		if (NULL != buf) free(buf);

		buf = (wchar_t*)malloc((cch_buf + 1) * sizeof(wchar_t));
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
			if (ERROR_INSUFFICIENT_BUFFER != gle)
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

	if (true == ret) nt_device = buf;

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
	wchar_t drive_string[128 + 1] = { 0 };
	DWORD length = GetLogicalDriveStringsW(128, drive_string);
	if (0 == length)
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

		//
		//	A: �� �ǳʶڴ�.
		// 
		if (dos_device_name[0] == 'A' || dos_device_name[0] == 'a') continue;

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
			log_warn
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
					//log_dbg "disk number = %u, found.", sdn.DeviceNumber log_end
				}
			}
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
	}
	else if (IsEqualGUID(partition_type, PARTITION_ENTRY_UNUSED_GUID))
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
	HANDLE disk = CreateFileW(path.str().c_str(),
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
	handle_ptr handle_guard(disk, [](HANDLE h) {
		CloseHandle(h);
	});

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

		if (!DeviceIoControl(disk,
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
	void_ptr buf_guard(layout_info, [](void* p) {
		free(p);
	});

	//
	// ok. DeviceIoControl() succeeded.
	// 
	//log_dbg
	//    "disk = %ws, partition style = %s, partition count = %u, ",
	//    path.str().c_str(),
	//    partition_style_to_str(layout_info->PartitionStyle),        
	//    layout_info->PartitionCount);

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

			//log_dbg
			//    "    [%u/%u] style = MBR, recognized = %s, boot = %s, offset = 0x%llx, length = %llu, number = %u",
			//    i + 1,
			//    layout_info->PartitionCount,
			//    (vbr.recognized) ? "true" : "false",
			//    vbr.is_boot_partition ? "true" : "false",
			//    vbr.offset.QuadPart,
			//    vbr.partition_length.QuadPart,
			//    vbr.partition_number);
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

			//log_dbg
			//    "    [%u/%u] style = GPT, recognized = true, boot = %s, offset = 0x%llx, length = %llu, number = %u, type = %s",
			//    i + 1,
			//    layout_info->PartitionCount,
			//    vbr.is_boot_partition ? "true" : "false",
			//    vbr.offset.QuadPart,
			//    vbr.partition_length.QuadPart,
			//    vbr.partition_number, 
			//    gpt_partition_type_to_str(gpt->PartitionType)
			//    );
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

		while (true != break_loop)
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
							pi->StartingOffset.QuadPart,
							pi->PartitionLength.QuadPart,
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
									"[*] dump VBR (disk offset 0x%llx)", pi->StartingOffset.QuadPart
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
							pi->StartingOffset.QuadPart,
							pi->PartitionLength.QuadPart,
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
							pi->StartingOffset.QuadPart,
							pi->PartitionLength.QuadPart,
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
	if (hFile == INVALID_HANDLE_VALUE)
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
	if (hFile == INVALID_HANDLE_VALUE)
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
bool
get_file_size(
	_In_ HANDLE file_handle,
	_Out_ int64_t& size
)
{
	_ASSERTE(INVALID_HANDLE_VALUE != file_handle);
	if (INVALID_HANDLE_VALUE == file_handle) return false;

	LARGE_INTEGER size_tmp = { 0 };

	if (TRUE != GetFileSizeEx(file_handle,
							  &size_tmp))
	{
		log_err "GetFileSizeEx( file = 0x%p ), gle = %u",
			file_handle,
			GetLastError()
			log_end;
		return false;
	}

	size = size_tmp.QuadPart;
	return true;
}

///
///	@brief	������ ������ ���� ������ �����ϴ� �Լ�
/// @return ����		true
///					(���Ͽ� ���ҽ� ������ ���°�쿡��,
///					 file_version�� ""�� true�� return �Ѵ�.)
/// @return ����		false
///
bool
get_file_version(
	_In_ const wchar_t* file_path,
	_Out_ std::wstring& file_version
)
{
	_ASSERTE(nullptr != file_path);
	if (nullptr == file_path)
	{
		return false;
	}

	DWORD size = GetFileVersionInfoSize(file_path,
										0);
	if (0 == size)
	{
		DWORD err = GetLastError();
		if (ERROR_RESOURCE_DATA_NOT_FOUND == err ||
			ERROR_RESOURCE_TYPE_NOT_FOUND == err)
		{
			//
			//	������ ���ҽ� ������ ���°�� get_file_version()�� �׻� �����ϰ� �ȴ�.
			//	������, file_version�� ""�� �����ϰ� true�� return �Ѵ�.
			//
			file_version = L"";
			return true;
		}

		log_err "GetFileVersionInfoSize() failed, file=%ws, gle=%u",
			file_path,
			err
			log_end;
		return false;
	}

	wchar_ptr buffer((wchar_t*)malloc(size),
					 [](_In_ wchar_t* ptr)
	{
		if (nullptr != ptr) free(ptr);
	});
	if (nullptr == buffer.get())
	{
		log_err "Not enough memory. malloc size=%u",
			size
			log_end;
		return false;
	}

	if (TRUE != GetFileVersionInfo(file_path,
								   0,
								   size,
								   buffer.get()))
	{
		log_err "GetFileVersionInfo() failed, file=%ws, gle=%u",
			file_path,
			GetLastError()
			log_end;
		return false;
	}

	UINT len;
	VS_FIXEDFILEINFO *vs_ffi;
	if (TRUE != VerQueryValue(buffer.get(),
							  L"\\",
							  (LPVOID*)&vs_ffi,
							  &len))
	{
		log_err "VerQueryValue() failed, file=%ws, gle=%u",
			file_path,
			GetLastError()
			log_end;
		return false;
	}

	std::wstringstream strm;
	strm << HIWORD(vs_ffi->dwFileVersionMS) << L".";
	strm << LOWORD(vs_ffi->dwFileVersionMS) << L".";
	strm << HIWORD(vs_ffi->dwFileVersionLS) << L".";
	strm << LOWORD(vs_ffi->dwFileVersionLS);
	file_version = strm.str();

	return true;
}

/**
* @brief	���Ͽ� ���˹��ڿ��� ����.
*/
BOOL write_to_filew(LPCWCH file_path, LPCWCH format, ...)
{
	_ASSERTE(NULL != file_path);
	if (NULL == file_path)
	{
		return FALSE;
	}

	HANDLE hFile = CreateFileW(file_path,
							   GENERIC_WRITE,
							   FILE_SHARE_READ,
							   NULL,
							   OPEN_ALWAYS,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL);
	if (hFile == INVALID_HANDLE_VALUE)
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
	DWORD pos = SetFilePointer(hFile, 0, NULL, FILE_END);
	if (INVALID_SET_FILE_POINTER == pos) return FALSE;

	va_list args;
	DWORD dw = 0;
	WCHAR temp[1024] = { 0 };
	WCHAR* pszDestEnd = temp;
	size_t cbRemaining = sizeof(temp);

	va_start(args, format);
	if (TRUE != SUCCEEDED(StringCbVPrintfExW(pszDestEnd,
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

	if (TRUE != WriteFile(hFile,
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
BOOL write_to_filew(HANDLE hFile, LPCWCH format, ...)
{
	DWORD pos = SetFilePointer(hFile, 0, NULL, FILE_END);
	if (INVALID_SET_FILE_POINTER == pos) return FALSE;

	va_list args;
	DWORD dw = 0;
	WCHAR temp[1024] = { 0 };
	WCHAR* pszDestEnd = temp;
	size_t cbRemaining = sizeof(temp);


	va_start(args, format);
	if (TRUE != SUCCEEDED(StringCbVPrintfExW(pszDestEnd,
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

	if (TRUE != WriteFile(hFile,
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
BOOL write_to_filea(HANDLE hFile, LPCCH format, ...)
{
	DWORD pos = SetFilePointer(hFile, 0, NULL, FILE_END);
	if (INVALID_SET_FILE_POINTER == pos) return FALSE;

	va_list args;
	DWORD dw = 0;
	CHAR temp[5120] = { 0 };
	CHAR* pszDestEnd = temp;
	size_t cbRemaining = sizeof(temp);

	va_start(args, format);
	if (TRUE != SUCCEEDED(StringCbVPrintfExA(pszDestEnd,
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

	if (TRUE != WriteFile(hFile,
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

	LARGE_INTEGER li_new_pos = { 0 };
	LARGE_INTEGER li_distance = { 0 };
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

	LARGE_INTEGER li_distance = { 0 }; li_distance.QuadPart = distance;
	LARGE_INTEGER li_new_pos = { 0 };

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
	DWORD cbWritten = 0;
	BYTE ByteOrderMark[] = { 0xEF, 0xBB, 0xBF };
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
bool
LoadFileToMemory(
	_In_ const LPCWSTR  FilePath,
	_Out_ DWORD&  MemorySize,
	_Outptr_ PBYTE&  Memory
)
{
	_ASSERTE(nullptr != FilePath);
	_ASSERTE(true == is_file_existsW(FilePath));
	if (nullptr == FilePath ||
		true != is_file_existsW(FilePath))
	{
		return false;
	}
	HANDLE hFile = CreateFileW((LPCWSTR)FilePath,
							   GENERIC_READ,
							   FILE_SHARE_READ/* | FILE_SHARE_WRITE*/,
							   NULL,
							   OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		log_err
			"CreateFile(%ws) failed, gle=%u",
			FilePath,
			GetLastError()
			log_end
			return false;
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
			return false;
	}

	if (0 == fileSize.QuadPart)
	{
		log_err "Can not map zero length file" log_end
			return false;
	}

	HANDLE hImageMap = CreateFileMapping(hFile,
										 NULL,
										 PAGE_READONLY,
										 0,
										 0,
										 NULL);
	if (NULL == hImageMap)
	{
		log_err
			"CreateFileMapping(%ws) failed, gle=%u",
			FilePath,
			GetLastError()
			log_end

			return false;
	}
	SmrtHandle sfMap(hImageMap);

	PBYTE ImageView = (LPBYTE)MapViewOfFile(hImageMap,
											FILE_MAP_READ,
											0,
											0,
											0);
	if (ImageView == nullptr)
	{
		log_err
			"MapViewOfFile(%ws) failed, gle=%u",
			FilePath,
			GetLastError()
			log_end
			return false;
	}
	SmrtView sfView(ImageView);

	MemorySize = fileSize.LowPart;  // max config fileSize = 4 MB �̹Ƿ� ������
	Memory = (PBYTE)malloc(MemorySize);
	if (nullptr == Memory) { return false; }

	RtlZeroMemory(Memory, MemorySize);
	RtlCopyMemory(Memory, ImageView, MemorySize);
	return true;
}

/**
 * @brief	���̳ʸ� ���Ϸ� �����͸� �����Ѵ�.
 */
bool
SaveBinaryFile(
	_In_ const LPCWSTR  Directory,
	_In_ const LPCWSTR  FileName,
	_In_ DWORD    Size,
	_In_ PBYTE    Data
)
{
	_ASSERTE(nullptr != Directory);
	_ASSERTE(nullptr != FileName);
	_ASSERTE(0 < Size);
	_ASSERTE(nullptr != Data);
	if (nullptr == Directory ||
		nullptr == FileName ||
		0 >= Size ||
		nullptr == Data)
	{
		return false;
	}
	// create data directory
	//
	int ret = SHCreateDirectoryExW(NULL, Directory, NULL);
	if (ERROR_SUCCESS != ret && ERROR_ALREADY_EXISTS != ret)
	{
		log_err
			"SHCreateDirectoryExW(path=%S) failed, ret=0x%08x",
			Directory, ret
			log_end
			return false;
	}

	WCHAR DataPath[MAX_PATH + 1] = { 0 };
	if (true != SUCCEEDED(StringCbPrintfW(DataPath,
										  sizeof(DataPath),
										  L"%s\\%s",
										  Directory,
										  FileName)))
	{
		log_err
			"Can not generate target path, dir=%S, file=%S",
			Directory, FileName
			log_end
			return false;
	}

	// ������ ������ �����ϴ� ��� ���� ������ ���� �� ���Ӱ� ������
	// 
	if (true == is_file_existsW(DataPath))
	{
		//log_err
		//	"same file exists, file=%S will be replaced by new file",
		//	DataPath
		//	log_end
		::DeleteFileW(DataPath);
	}

	HANDLE hFile = open_file_to_write(DataPath);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		log_err
			"Can not create file=%S, check path or privilege",
			DataPath
			log_end
			return false;
	}
	SmrtHandle sh(hFile);

	DWORD cbWritten = 0;
	if (TRUE != ::WriteFile(hFile,
							Data,
							Size,
							&cbWritten,
							NULL))
	{
		log_err
			"WriteFile(path=%S) failed, gle=%u",
			DataPath, GetLastError()
			log_end
			return false;
	}

	return true;
}

/// @brief	������ �ؽø� ����Ѵ�.
bool
get_file_hash_by_filepath(
	_In_ const wchar_t* file_path,
	_Out_opt_ const std::string* md5,
	_Out_opt_ const std::string* sha2
)
{
	_ASSERTE(nullptr != file_path);
	_ASSERTE(!(nullptr == md5 && nullptr == sha2));
	if (nullptr == file_path || (nullptr == md5 && nullptr == sha2))
	{
		return false;
	}

	handle_ptr file_handle(
		CreateFileW(file_path,
					GENERIC_READ,
					FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL),
		[](HANDLE h)
	{
		if (INVALID_HANDLE_VALUE != h)
		{
			CloseHandle(h);
		}
	});

	if (INVALID_HANDLE_VALUE == file_handle.get())
	{
		log_err
			"CreateFileW() failed. path=%ws, gle = %u",
			file_path,
			GetLastError()
			log_end;
		return false;
	}

	return get_file_hash_by_filehandle(file_handle.get(),
									   md5,
									   sha2);
}


/// @brief	������ �ؽø� ����Ѵ�.
bool
get_file_hash_by_filehandle(
	_In_ HANDLE file_handle,
	_Out_opt_ const std::string* md5,
	_Out_opt_ const std::string* sha2
)
{
	_ASSERTE(nullptr != file_handle);
	_ASSERTE(!(nullptr == md5 && nullptr == sha2));
	if (nullptr == file_handle || (nullptr == md5 && nullptr == sha2))
	{
		return false;
	}
	
	bool ret = false;
	MD5_CTX* ctx_md5 = nullptr;
	sha256_ctx* ctx_sha2 = nullptr;
	uint8_t* sha2_buf = nullptr;
	do
	{
		if (nullptr != md5)
		{
			ctx_md5 = (MD5_CTX*)malloc(sizeof(MD5_CTX));
			if (nullptr == ctx_md5) break;
		}

		if (nullptr != sha2)
		{
			ctx_sha2 = (sha256_ctx*)malloc(sizeof(sha256_ctx));
			sha2_buf = (uint8_t*)malloc(32);
			if (nullptr == ctx_sha2 || nullptr == sha2_buf) break;
		}
		
		_ASSERTE(!(nullptr == ctx_md5 && nullptr == ctx_sha2));

		/// NOTE by somma
		/// ReadFile() �� �о �ؽø� ���ϴ� �ڵ带 FileIoHelper �� ����ؼ�
		/// MMIO �� ���������� ���ɻ��� ū �̵��� ���°� ����. 
		/// ������ ���� ���� ����� �۾Ƽ� ũ�� �̵��� ���� �� �ʹ�.
		FileIoHelper fio;
		if (true != fio.OpenForRead(file_handle))
		{
			log_err "fio.OpenForRead() failed. file handle=0x%p",
				file_handle
				log_end;
			break;
		}

		if (nullptr != ctx_md5) { MD5Init(ctx_md5, 0); }
		if (nullptr != ctx_sha2) { sha256_begin(ctx_sha2); }

		bool err = false;
		uint64_t file_size = fio.FileSize();
		uint64_t pos = 0;
		while (pos < file_size)
		{
			if (err) break;

			uint32_t size = (uint32_t)min(fio.GetOptimizedBlockSize(), file_size - pos);
			uint8_t* ptr = fio.GetFilePointer(true, pos, size);

			if (nullptr != ctx_md5) { MD5Update(ctx_md5, ptr, size); }
			if (nullptr != ctx_sha2) { sha256_hash(ptr, size, ctx_sha2); }

			fio.ReleaseFilePointer();
			pos += size;
		}

		if (nullptr != ctx_md5) { MD5Final(ctx_md5); }
		if (nullptr != ctx_sha2){ sha256_end(sha2_buf, ctx_sha2); }

		//
		//	Hash ���̳ʸ� ���۸� hex ���ڿ��� ��ȯ
		//
		std::string tmp;
		if (nullptr != ctx_md5)
		{
			if (true != bin_to_hexa_fast(sizeof(ctx_md5->digest),
										 ctx_md5->digest,
										 false,
										 (std::string&)*md5))
			{
				log_err "bin_to_hexa_fast() failed. " log_end;
				break;
			}			
		}
		
		if (nullptr != ctx_sha2)
		{
			if (true != bin_to_hexa_fast(32,
										 sha2_buf,
										 false,
										 (std::string&)*sha2))
			{
				log_err "bin_to_hexa_fast() failed. " log_end;
				break;
			}
		}		

		ret = true;
	} while (false);

	free_and_nil(ctx_md5);
	free_and_nil(ctx_sha2);
	free_and_nil(sha2_buf);
	return ret;
}


/// @brief	DirectoryPath ���丮�� �����Ѵ�. 
///			�߰��� ���� ���丮 ��ΰ� �����ϸ� �����Ѵ�.
bool WUCreateDirectory(_In_ std::wstring& DirectoryPath)
{
	return WUCreateDirectory(DirectoryPath.c_str());
}

/// @brief	������ ���丮(������ ����, ���ϵ����)�� ���� �����Ѵ�. 
bool WUDeleteDirectoryW(_In_ std::wstring& DirctoryPathToDelete)
{
	return WUDeleteDirectoryW(DirctoryPathToDelete.c_str());
}

/// @brief	DirectoryPath ���丮�� �����Ѵ�. 
///			�߰��� ���� ���丮 ��ΰ� �����ϸ� �����Ѵ�.
bool WUCreateDirectory(_In_ const wchar_t* DirectoryPath)
{
	_ASSERTE(NULL != DirectoryPath);
	if (NULL == DirectoryPath) return false;

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

/// @brief	������ ���丮(������ ����, ���ϵ����)�� ���� �����Ѵ�. 
bool WUDeleteDirectoryW(_In_ const wchar_t* DirctoryPathToDelete)
{
	_ASSERTE(NULL != DirctoryPathToDelete);
	if (NULL == DirctoryPathToDelete) return false;
	if (!is_file_existsW(DirctoryPathToDelete)) return true;


	SHFILEOPSTRUCTW FileOp = { 0 };

	// FileOp.pFrom, FileOp.pTo �� NULL char �� �ΰ��̾�� �� (msdn ����)
	// 
	size_t len = (wcslen(DirctoryPathToDelete) + 2) * sizeof(WCHAR);
	wchar_ptr tmp((wchar_t*)malloc(len), [](wchar_t* p) {
		if (nullptr != p)
		{
			free(p);
		}
	});
	if (NULL == tmp.get()) return false;
	RtlZeroMemory(tmp.get(), len);
	if (TRUE != SUCCEEDED(StringCbPrintfW(tmp.get(),
										  len,
										  L"%s",
										  DirctoryPathToDelete)))
	{
		return false;
	}

	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_DELETE;       // ���� �Ӽ� ����
	FileOp.pTo = NULL;
	FileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;//FOF_NOCONFIRMATION | FOF_NOERRORUI; // Ȯ�θ޽����� �ȶߵ��� ����
	FileOp.fAnyOperationsAborted = false;
	FileOp.hNameMappings = NULL;
	FileOp.lpszProgressTitle = tmp.get();
	FileOp.pFrom = tmp.get();

	int ret = SHFileOperation(&FileOp);
	if (0 != ret)
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
	if ((NULL == ImageName) ||
		(NULL == FullPathBuffer))
	{
		return FALSE;
	}


	WCHAR* wcs = MbsToWcs(ImageName);
	if (NULL == wcs) { return FALSE; }
	SmrtPtr<WCHAR*> smpt(wcs);

	WCHAR buf[MAX_PATH] = { 0 };
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
	if ((NULL == ImageName) ||
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
		static WCHAR SearchPathBuf[MAX_PATH] = { 0, };
		if (0x00 == SearchPathBuf[0])
		{
			std::wstring system_root;
			if (!get_system_dir(system_root))
			{
				log_err "get_system_rootdir() failed." log_end;
				return FALSE;
			}

			if (!SUCCEEDED(StringCbPrintfW(
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

/// @brief	Process handle �� full path �� ���Ѵ�. 
///	@return	nt device name format �� ���μ����� �̹��� ���
///			( e.g. \Device\Harddisk0\Partition1\Windows\System32\Ctype.nls )
bool get_process_image_full_path(_In_ HANDLE process_handle, _Out_ std::wstring& full_path)
{
	bool ret = false;
	uint32_t    buf_len = 1024;
	wchar_t*    buf = (wchar_t*)malloc(buf_len);
	if (NULL == buf) return false;

	for (int i = 0; i < 3; ++i) // ���� �ø��°� ������...
	{
		DWORD dwret = GetProcessImageFileNameW(process_handle, buf, buf_len / sizeof(wchar_t));
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


/// @brief	pid �� ���μ����� ��ü �̸��� ���Ѵ�. (vista �̻�)
///	@param	win32_format
///				true �� ��� `c:\dbg\sound.dll` ���� (Win32 format) string ����
///				false �� ��� `\Device\HarddiskVolume1\dbg\sound.dll` ���� ����		
#if _WIN32_WINNT >= 0x0600	// after vista
bool
image_path_by_pid(
	_In_ DWORD process_id,
	_In_ bool win32_format,
	_Out_ std::wstring& image_path
)
{
	HANDLE phandle = NULL;
	do
	{
		phandle = OpenProcess(PROCESS_QUERY_INFORMATION,
							  FALSE,
							  process_id);
		if (NULL != phandle) break;

		if (!set_privilege(SE_DEBUG_NAME, true))
		{
			log_err "set_privilege( SE_DEBUG_NAME ) failed. " log_end;
			break;
		}

		phandle = OpenProcess(PROCESS_QUERY_INFORMATION,
							  FALSE,
							  process_id);
	} while (false);

	if (NULL == phandle)
	{
		//log_err "Can not access process. pid=%u",
		//	pid
		//	log_end;
		return false;
	}
	handle_ptr process_handle(phandle, [](HANDLE h) {CloseHandle(h); });

	wchar_t*	name = NULL;
	DWORD		name_len = MAX_PATH;
	DWORD		ret = 0;

	//
	//	ZwQuerySystemInformation ó�� length �� 0 �� �Ѱ��ָ� �ʿ��� ������ ���̸�
	//	������ ���� �ʴ´�. ŭ���ϰ� ���� ��Ƽ� ȣ���ؾ� ��
	//
	for (int i = 0; i < 3; ++i)		// name buffer �� �ι辿 Ű��°͵� 3ȸ�� �õ��Ѵ�.
	{
		if (NULL != name) free(name);
		name = (wchar_t*)malloc((name_len + 1) * sizeof(wchar_t));
		if (NULL == name) return false;

		ret = QueryFullProcessImageNameW(process_handle.get(),
			(win32_format == true) ? 0 : PROCESS_NAME_NATIVE,
										 name,
										 &name_len);
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
				log_err "QueryFullProcessImageName() failed. gle = %u",
					gle
					log_end;
				return false;
			}
		}
	}

	image_path = name;	// copy
	if (NULL != name) free(name);
	return true;
}

#endif

/// @brief  GetSystemDirectoryW() wrapper (c:\windows\system32)
bool get_system_dir(_Out_ std::wstring& system_dir)
{
	wchar_t     buf[MAX_PATH] = { 0x00 };
	uint32_t    buf_len = sizeof(buf);
	wchar_t*    pbuf = buf;

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

	DWORD char_count_plus_null = ExpandEnvironmentStrings(env_variable, NULL, 0);
	if (0 == char_count_plus_null)
	{
		log_err
			"ExpandEnvironmentStrings( %ws ) failed. gle = %u",
			env_variable,
			GetLastError()
			log_end
			return false;
	}

	uint32_t buf_len = char_count_plus_null * sizeof(wchar_t);
	wchar_ptr buf(
		(wchar_t*)malloc(buf_len),
		[](wchar_t* p) {if (nullptr != p) free(p); }
	);
	if (NULL == buf.get())
	{
		log_err "malloc() failed." log_end
			return false;
	}

	char_count_plus_null = ExpandEnvironmentStrings(env_variable,
													buf.get(),
													buf_len);
	if (0 == char_count_plus_null)
	{
		log_err
			"ExpandEnvironmentStrings( %ws ) failed. gle = %u",
			env_variable,
			GetLastError()
			log_end
			return false;
	}

	env_value = buf.get();
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
	char_count_and_null = GetShortPathNameW(long_file_name,
											short_path,
											char_count_and_null);
	if (0 == char_count_and_null)
	{
		log_err "GetShortPathNameW( %ws ) failed. gle = %u",
			long_file_name,
			GetLastError()
			log_end;
		return false;
	}

	short_path = (wchar_t*)malloc(sizeof(wchar_t*) * char_count_and_null);
	if (NULL == short_path)
	{
		log_err "malloc() failed." log_end
			return false;
	}

	if (0 == GetShortPathNameW(long_file_name,
							   short_path,
							   char_count_and_null))
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
	_In_ DWORD_PTR tag,
	_In_ bool recursive,
	_In_ fnFindFilesCallback cb
)
{
	_ASSERTE(NULL != root);
	if (NULL == root) return false;


	std::wstring root_dir(root);

	if (root[wcslen(root) - 1] == L'\\')
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
	WIN32_FIND_DATAW wfd = { 0 };
	WCHAR fname[MAX_PATH + 1] = { 0 };
	BOOL bResult = TRUE;
	WCHAR drive[_MAX_DRIVE + 1] = { 0 };
	WCHAR dir[MAX_PATH + 1] = { 0 };
	WCHAR newpath[MAX_PATH + 1] = { 0 };

	hSrch = FindFirstFileW(root_dir.c_str(), &wfd);
	if (INVALID_HANDLE_VALUE == hSrch)
	{
		DWORD gle = GetLastError();

		if (ERROR_ACCESS_DENIED == gle || ERROR_FILE_NOT_FOUND == gle)
		{
			//
			// ��Ī�Ǵ� ������ ���ų�(* �� ����� find_files() ȣ���� ���)
			// ������ ������ ��� 
			// �������� �����Ѵ�. 
			// 
			return true;
		}
		else
		{
			log_err
				"FindFirstFileW(path=%S) failed, gle=%u",
				root_dir.c_str(), GetLastError()
				log_end;
			return false;
		}
	}

	_wsplitpath_s(root_dir.c_str(), drive, _MAX_DRIVE, dir, MAX_PATH, NULL, NULL, NULL, NULL);

	while (bResult)
	{
		Sleep(100);

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		{
			// symbolic link �� ó������ ����
			// 
			if (wfd.dwReserved0 & IO_REPARSE_TAG_SYMLINK)
			{
				bResult = FindNextFile(hSrch, &wfd);
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
					find_files(newpath, tag, recursive, cb);
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
				if (TRUE != cb(tag, fname)) break;
			}
		}
		bResult = FindNextFile(hSrch, &wfd);
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
	WIN32_FIND_DATAW wfd = { 0 };
	BOOL bResult = TRUE;
	WCHAR drive[_MAX_DRIVE + 1] = { 0 };
	WCHAR dir[MAX_PATH + 1] = { 0 };

	// RootPath �Ķ���Ͱ� c:\dbg ������ ��� c:\dbg ��(RootPath �ڽ�)�� �߰���
	// ���� '\', '\*', '\*.*' �� �ƴ� ��� ������ '\*' �� �ٿ� RootPath ��δ� ������
	// 
	// ���� RootPath �Ķ���Ͱ� '\' �� ������ �ȵǹǷ� \* �� ���� ����
	// 
	std::wstring RootDir(RootPath);
	if (RootPath[wcslen(RootPath) - 1] == L'\\')
	{
		RootDir.append(L"*");
	}
	else if (RootPath[wcslen(RootPath) - 1] == L'*')
	{
		// do nothing
	}
	else
	{
		RootDir.append(L"\\*");
	}


	hSrch = FindFirstFileW(RootDir.c_str(), &wfd);
	if (INVALID_HANDLE_VALUE == hSrch)
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
		Sleep(100);

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
		bResult = FindNextFile(hSrch, &wfd);
	}
	FindClose(hSrch);
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
	_ASSERTE(nullptr != mbs);
	_ASSERTE(0x00 != mbs[0]);
	if (nullptr == mbs) return nullptr;
	if (0x00 == mbs[0]) return nullptr;

	int outLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mbs, -1, nullptr, 0);
	if (0 == outLen) return nullptr;

	wchar_t* outWchar = (wchar_t*)malloc(outLen * (sizeof(wchar_t)));  // outLen contains NULL char 
	if (NULL == outWchar) return nullptr;
	RtlZeroMemory(outWchar, outLen);

	if (0 == MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mbs, -1, outWchar, outLen))
	{
		log_err "MultiByteToWideChar() failed, errcode=0x%08x", GetLastError() log_end

			free(outWchar);
		return nullptr;
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
	_ASSERTE(nullptr != wcs);
	_ASSERTE(0x00 != wcs[0]);
	if (nullptr == wcs) return nullptr;
	if (0x00 == wcs[0]) return nullptr;

	int outLen = WideCharToMultiByte(CP_ACP, 0, wcs, -1, nullptr, 0, nullptr, nullptr);
	if (0 == outLen) return nullptr;

	char* outChar = (char*)malloc(outLen * sizeof(char));
	if (nullptr == outChar) return nullptr;
	RtlZeroMemory(outChar, outLen);

	if (0 == WideCharToMultiByte(CP_ACP, 0, wcs, -1, outChar, outLen, nullptr, nullptr))
	{
		log_err "WideCharToMultiByte() failed, errcode=0x%08x", GetLastError() log_end
			free(outChar);
		return nullptr;
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
	_ASSERTE(nullptr != wcs);
	_ASSERTE(0x00 != wcs[0]);
	if (nullptr == wcs) return nullptr;
	if (0x00 == wcs[0]) return nullptr;

	int outLen = WideCharToMultiByte(CP_UTF8, 0, wcs, -1, nullptr, 0, nullptr, nullptr);
	if (0 == outLen) return nullptr;

	char* outChar = (char*)malloc(outLen * sizeof(char));
	if (nullptr == outChar) return nullptr;
	RtlZeroMemory(outChar, outLen);

	if (0 == WideCharToMultiByte(CP_UTF8, 0, wcs, -1, outChar, outLen, nullptr, nullptr))
	{
		log_err "WideCharToMultiByte() failed, errcode=0x%08x", GetLastError() log_end

			free(outChar);
		return nullptr;
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
	_ASSERTE(nullptr != utf8);
	_ASSERTE(0x00 != utf8[0]);
	if (nullptr == utf8) return nullptr;
	if (0x00 == utf8[0]) return nullptr;

	int outLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, nullptr, 0);
	if (0 == outLen) return nullptr;

	wchar_t* outWchar = (wchar_t*)malloc(outLen * (sizeof(wchar_t)));  // outLen contains nullptr char 
	if (nullptr == outWchar) return nullptr;
	RtlZeroMemory(outWchar, outLen);

	if (0 == MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, outWchar, outLen))
	{
		log_err "MultiByteToWideChar() failed, errcode=0x%08x", GetLastError() log_end

			free(outWchar);
		return nullptr;
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
	_ASSERTE(nullptr != mbs);
	if (nullptr == mbs) return _null_stringw;
	if (0x00 == mbs[0]) return _null_stringw;

	wchar_ptr tmp(MbsToWcs(mbs), [](wchar_t* p) {
		if (nullptr != p)
		{
			free(p);
		}
	});

	if (nullptr == tmp.get())
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
	_ASSERTE(nullptr != wcs);
	if (nullptr == wcs) return _null_stringa;
	if (0x00 == wcs[0]) return _null_stringa;

	char_ptr tmp(WcsToMbs(wcs), [](char* p) {
		if (nullptr != p)
		{
			free(p);
		}
	});

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
	_ASSERTE(nullptr != wcs);
	if (nullptr == wcs) return _null_stringa;
	if (0x00 == wcs[0]) return _null_stringa;

	char_ptr tmp(WcsToMbsUTF8(wcs), [](char*p) {
		if (nullptr != p)
		{
			free(p);
		}
	});

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
	_ASSERTE(nullptr != utf8);
	if (nullptr == utf8) return _null_stringw;
	if (0x00 == utf8[0]) return _null_stringw;

	wchar_ptr tmp(Utf8MbsToWcs(utf8), [](wchar_t* p) {
		if (nullptr != p)
		{
			free(p);
		}
	});

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
bool 
rstrnicmp(
	_In_ const wchar_t* src, 
	_In_ const wchar_t* fnd, 
	_In_ bool case_insensitive
	)
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
		if (true == case_insensitive)
		{
			if (towlower(fnd[fidx--]) != towlower(src[sidx--])) return false;
		}
		else
		{
			if (fnd[fidx--] != src[sidx--]) return false;
		}
	}
	return true;
}

bool 
rstrnicmpa(
	_In_ const char* src, 
	_In_ const char* fnd, 
	_In_ bool case_insensitive
	)
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
		if (true == case_insensitive)
		{
			if (towlower(fnd[fidx--]) != towlower(src[sidx--])) return false;
		}
		else
		{
			if (fnd[fidx--] != src[sidx--]) return false;
		}
	}
	return true;
}

/// @brief  src �� �տ������� fnd ���ڿ��� ã�´�. 
bool 
lstrnicmp(
	_In_ const wchar_t* src, 
	_In_ const wchar_t* fnd, 
	_In_ bool case_insensitive
	)
{
	_ASSERTE(NULL != src);
	_ASSERTE(NULL != fnd);
	if (NULL == src || NULL == fnd) return false;

	if (true == case_insensitive)
	{
		return (0 == _wcsnicmp(src, fnd, wcslen(fnd))) ? true : false;
	}
	else
	{
		return (0 == wcsncmp(src, fnd, wcslen(fnd))) ? true : false;
	}
}

/// @brief  src �� �տ������� fnd ���ڿ��� ã�´�. 
bool 
lstrnicmpa(
	_In_ const char* src, 
	_In_ const char* fnd, 
	_In_ bool case_insensitive
	)
{
	_ASSERTE(NULL != src);
	_ASSERTE(NULL != fnd);
	if (NULL == src || NULL == fnd) return false;

	if (true == case_insensitive)
	{
		return (0 == _strnicmp(src, fnd, strlen(fnd))) ? true : false;
	}
	else
	{
		return (0 == strncmp(src, fnd, strlen(fnd))) ? true : false;
	}
}

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

	size_t pos = org_string.rfind(token);
	if (std::wstring::npos == pos)
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

/// @brief	������ Ȯ���ڸ� �����Ѵ�. 
///
///			c:\dbg\abc.txt -> txt 
///			\offsymxl.ttf:WofCompressedData -> ttf
/// 
///			Ȯ���ڰ� ���� ��� false �� �����Ѵ�.
bool
get_file_extensionw(
	_In_ const wchar_t* file_path,
	_Out_ std::wstring& ext
)
{
	_ASSERTE(nullptr != file_path);
	if (nullptr == file_path) return false;

	//
	// c:\dbg\.\sub\abc_no_ext ������ ��θ��� ��� `\sub\abc_no_ext` �� 
	// Ȯ���ڷ� �νĵȴ�. ���� �ڿ��� `\` ���ڿ��� ���� ã�� �� `\` ���� 
	// ���ڿ����� `.` �� ã�´�.
	//
	std::wstring org_string = extract_last_tokenExW(file_path, L"\\", false);
	size_t pos = org_string.rfind(L".");
	if (std::wstring::npos == pos)
	{
		return false;
	}
	org_string = org_string.substr(pos + 1, org_string.size());

	//
	//	ADS �� �߶󳽴�.
	// 
	pos = org_string.find(L":");
	if (std::wstring::npos == pos)
	{
		ext = org_string;
		return true;
	}

	ext = org_string.substr(0, pos);
	return true;
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
*/
std::string trima(std::string& s, const std::string& drop)
{
	std::string r = s.erase(s.find_last_not_of(drop) + 1);
	return r.erase(0, r.find_first_not_of(drop));
}
std::string rtrima(std::string& s, const std::string& drop)
{
	return s.erase(s.find_last_not_of(drop) + 1);
}
std::string ltrima(std::string& s, const std::string& drop)
{
	return s.erase(0, s.find_first_not_of(drop));
}
std::wstring  trimw(std::wstring& s, const std::wstring& drop)
{
	std::wstring r = s.erase(s.find_last_not_of(drop) + 1);
	return r.erase(0, r.find_first_not_of(drop));
}
std::wstring rtrimw(std::wstring& s, const std::wstring& drop)
{
	return s.erase(s.find_last_not_of(drop) + 1);
}
std::wstring ltrimw(std::wstring& s, const std::wstring& drop)
{
	return s.erase(0, s.find_first_not_of(drop));
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

	//
	//	strtok_s() modifies the `str` buffer.
	//	So we have to use copy `str` for out use only.
	// 
	size_t buf_len = (strlen(str) * sizeof(char)) + sizeof(char);
	if (max_str_len < buf_len)
	{
		return false;
	}

	char_ptr buf((char*)malloc(buf_len), [](char* p) {
		if (nullptr != p)
		{
			free(p);
		}
	});

	if (nullptr == buf.get())
	{
		return false;
	}

	StringCbPrintfA(buf.get(), buf_len, "%s", str);

	//	
	//	wcstok_s() �Լ����� separator ���ڿ��� ���ӵǴ� ��� �˾Ƽ� �ǳʶڴ�.
	//
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

	//
	//	strtok_s() modifies the `str` buffer.
	//	so we should make copy.
	//
	size_t buf_len = (wcslen(str) * sizeof(wchar_t)) + sizeof(wchar_t);
	if (max_str_len < buf_len)
	{
		return false;
	}

	wchar_ptr buf((wchar_t*)malloc(buf_len), [](wchar_t* p) {
		if (nullptr != p)
		{
			free(p);
		}
	});

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
///
///			test_GeneralHashFunctions(), test_GeneralHashFunctions2()
///			�Լ��� �ۼ��ؼ� c:\windows ���� ���ϵ鿡 ���ؼ� �׽�Ʈ�غôµ�, 
///			�� ����. ��� ����.
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
* @brief	���� ���丮�� �����ϴ� �Լ� (e.g. c:\debug )
* @param
* @code
* @endcode
* @return
*/
bool WUGetCurrentDirectoryW(_Out_ std::wstring& CurrentDir)
{
	UINT32 buflen = 0;
	PWSTR buf = NULL;

	buflen = GetCurrentDirectoryW(buflen, buf);
	if (0 == buflen)
	{
		log_err
			"GetCurrentDirectoryW() failed. gle=%u",
			GetLastError()
			log_end
			return false;
	}

	// buflen : NULL ĳ���͸� ������ �ʿ��� ������ ������ in char.
	// 
	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		log_err
			"GetCurrentDirectoryW() failed, gle=%u",
			GetLastError()
			log_end

			free(buf);
		return false;
	}

	CurrentDir = buf;
	free(buf);
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
*/
bool WUGetCurrentDirectoryA(_Out_ std::string& CurrentDir)
{
	std::wstring _cur;
	if (!WUGetCurrentDirectoryW(_cur))
	{
		return false;
	}
	std::string _cura = WcsToMbsEx(_cur.c_str());
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
	WCHAR path[MAX_PATH + 1] = { 0 };

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
	char path[MAX_PATH + 1] = { 0 };

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
 * @brief	����� full path �� ���Ѵ�.
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool
get_module_path(
	_In_ const wchar_t* module_name,
	_Out_ std::wstring& module_path
)
{
	DWORD  ret = 0;
	DWORD  buf_len = MAX_PATH;
	wchar_t* buf = (wchar_t*)malloc(buf_len * sizeof(wchar_t));
	if (NULL == buf) return false;

	for (;;)
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

	if (true != extract_last_tokenW(module_path,
									L"\\",
									module_dir,
									true,
									false))
	{
		log_err "extract_last_tokenW( org=%s )",
			module_path.c_str()
			log_end
			module_dir = L"";
		return false;
	}

	return true;
}

/**
 * @brief	���� ����� ���ϸ��� ���Ѵ�. (�������)
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
 * @brief	���� ����� full path �� ���Ѵ�.
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
 * @brief	���� ����� ���ϸ��� ���Ѵ�. (�������)
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

/// @brief	full path ��θ��� `���ϸ�.Ȯ����:ADS` �κи� �����. 
std::wstring file_name_from_file_pathw(_In_ const wchar_t* file_path)
{
	_ASSERTE(nullptr != file_path);
	if (nullptr == file_path) return _null_stringw;

	return extract_last_tokenExW(file_path, L"\\", false);
}

std::string file_name_from_file_patha(_In_ const char* file_path)
{
	_ASSERTE(nullptr != file_path);
	if (nullptr == file_path) return _null_stringa;

	return extract_last_tokenExA(file_path, "\\", false);
}

/// @brief	full path ��θ��� `���ϸ�.Ȯ����` �� ������ ���丮 �κи� �����. 
std::wstring directory_from_file_pathw(_In_ const wchar_t* file_path)
{
	_ASSERTE(nullptr != file_path);
	if (nullptr == file_path) return _null_stringw;

	return extract_last_tokenExW(file_path, L"\\", true);
}

std::string directory_from_file_patha(_In_ const char* file_path)
{
	_ASSERTE(nullptr != file_path);
	if (nullptr == file_path) return _null_stringa;

	return extract_last_tokenExA(file_path, "\\", true);
}

/// @brief	GUID �� �����Ѵ�. 
bool create_guid(_Out_ GUID& guid)
{
	return SUCCEEDED(CoCreateGuid(&guid));
}

bool create_guid(_Out_ std::string& guid)
{
	GUID g;
	if (true != create_guid(g)) return false;
	guid = guid_to_string(g);
	return true;
}

bool create_guid(_Out_ std::wstring& guid)
{
	GUID g;
	if (true != create_guid(g)) return false;
	guid = guid_to_stringw(g);
	return true;
}

/// @brief	
bool
string_to_guid(
	_In_ const char* guid_string,
	_Out_ GUID& guid
)
{
	_ASSERTE(nullptr != guid_string);
	if (nullptr == guid_string) return false;

	return wstring_to_guid(MbsToWcsEx(guid_string).c_str(), guid);
}

/// @brief	
bool
wstring_to_guid(
	_In_ const wchar_t* guid_string,
	_Out_ GUID& guid
)
{
	_ASSERTE(nullptr != guid_string);
	if (nullptr == guid_string) return false;

	return SUCCEEDED(CLSIDFromString(guid_string, &guid));
}

/// @brief	
std::string
guid_to_string(
	_In_ GUID& guid
)
{
	std::wstring guid_stringw = guid_to_stringw(guid);
	return WcsToMbsEx(guid_stringw.c_str());
}

/// @brief	
std::wstring
guid_to_stringw(
	_In_ GUID& guid
)
{
	const wchar_t* null_guid = L"{00000000-0000-0000-0000-000000000000}";
	wchar_t buf[40];

	//
	//	ret �� buf �� ������ ������ ���� + NULL 
	//	�߰��� NULL Terminate �� �ʿ� ����. 
	// 
	int ret = StringFromGUID2(guid, buf, sizeof(buf) / sizeof(wchar_t));
	if (0 == ret)
	{
		// 
		//	not enough buffer
		// 
		wchar_t* buf2 = (wchar_t*)malloc(256);
		if (nullptr == buf2) return null_guid;

		ret = StringFromGUID2(guid, buf2, 256 / sizeof(wchar_t));
		if (0 == ret)
		{
			//
			//	Give up
			// 
			return null_guid;
		}

		return std::wstring(buf2);
	}
	else
	{
		return std::wstring(buf);
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
std::string Win32ErrorToStringA(IN DWORD ErrorCode)
{
	char* msg = NULL;
	if (0 == FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		ErrorCode,
		0,
		(CHAR*)&msg,
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
	wchar_t* msg = NULL;
	if (0 == FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		ErrorCode,
		0,
		(wchar_t*)&msg,
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
	if ((0 < Length) && (NULL != Buf))
	{
		log_info "length = %u, buffer=0x%08x", Length, Buf log_end

			CHAR print_buf[128 * sizeof(CHAR)] = { 0 };
		DWORD i = 0, x = 0, ib = 0;
		UCHAR*  Addr = Buf;
		CHAR*	Pos = NULL;
		size_t	Remain = 0;
		for (;;)
		{
			if (i >= Length) break;
			ib = i;

			// reset all
			//
			Pos = print_buf;
			Remain = sizeof(print_buf);

			if (!SUCCEEDED(StringCbPrintfExA(
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

				if (!SUCCEEDED(StringCbPrintfExA(
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
				if (!SUCCEEDED(StringCbPrintfExA(
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

				if (!SUCCEEDED(StringCbPrintfExA(
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

			char tmp[64] = { 0 };
			Pos = tmp;
			Remain = sizeof(tmp) - sizeof(char);
			for (DWORD p = 0; p < 16; ++p)
			{
				if (p == Length) break;

				if (0x20 <= Addr[ib] && 0x7F > Addr[ib])
				{
					if (!SUCCEEDED(StringCbPrintfExA(
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
					if (!SUCCEEDED(StringCbPrintfExA(
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
BOOL DumpMemory(FILE* stream, DWORD Length, BYTE* Buf)
{
	if ((0 < Length) && (NULL != Buf))
	{
		_ftprintf(stream, TEXT("\n  00 01 02 03 04 05 06 07   08 09 0A 0B 0C 0D 0E 0F\n"));
		_ftprintf(stream, TEXT("  -- -- -- -- -- -- -- --   -- -- -- -- -- -- -- --\n"));

		TCHAR print_buf[128 * sizeof(TCHAR)] = { 0 };
		DWORD i = 0, x = 0;
		UCHAR*  Addr = Buf;
		TCHAR*	Pos = NULL;
		size_t	Remain = 0;
		for (;;)
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

				if (!SUCCEEDED(StringCbPrintfEx(
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

			if (x == Length) break;

			// insert space between first 8bytes and last 8 bytes.
			//
			if (!SUCCEEDED(StringCbPrintfEx(
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

				if (!SUCCEEDED(StringCbPrintfEx(
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
	_ASSERTE(NULL != buf);
	_ASSERTE(0 < buf_len);
	if (NULL == buf || 0 == buf_len) return false;

	// !����! - �� ������ line_dump ���� ū ��� (���� �׷�����...?!) ������ �߻� �� �� ����
	char line_dump[1024];

	if ((0 < buf_len) && (NULL != buf))
	{
		// useless, uh?
		//StringCbPrintfA(line_dump, sizeof(line_dump), "buf_len = %u, buffer=0x%08x", buf_len, buf);
		//dump.push_back(line_dump);

		CHAR print_buf[128 * sizeof(CHAR)] = { 0 };
		DWORD i = 0, x = 0, ib = 0;
		UCHAR*  Addr = buf;
		CHAR*	Pos = NULL;
		size_t	Remain = 0;
		for (;;)
		{
			if (i >= buf_len) break;
			ib = i;

			// reset all
			//
			Pos = print_buf;
			Remain = sizeof(print_buf);

			if (!SUCCEEDED(StringCbPrintfExA(
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

				if (!SUCCEEDED(StringCbPrintfExA(
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
				if (!SUCCEEDED(StringCbPrintfExA(
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

				if (!SUCCEEDED(StringCbPrintfExA(
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

			char tmp[64] = { 0 };
			Pos = tmp;
			Remain = sizeof(tmp) - sizeof(char);
			for (DWORD p = 0; p < 16; ++p)
			{
				if (p == buf_len) break;

				if (0x20 <= Addr[ib] && 0x7F > Addr[ib])
				{
					if (!SUCCEEDED(StringCbPrintfExA(
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
					if (!SUCCEEDED(StringCbPrintfExA(
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
		StringCbPrintfA(line_dump, sizeof(line_dump), "%s", print_buf);
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
	__time64_t long_time = 0;
	struct tm newtime = { 0 };

	// Get time as 64-bit integer.
	_time64(&long_time);

	errno_t err = _localtime64_s(&newtime, &long_time);
	if (err)
	{
		log_err "_localtime64_s() failed" log_end
			return FALSE;
	}

	// e.g. 2009.07.06 23:04:33
	//
	CHAR buf[20] = { 0 };
	if (!SUCCEEDED(StringCbPrintfA(
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

	TimeString = buf;
	return TRUE;
}

BOOL GetTimeStringW(IN std::wstring& TimeString)
{
	__time64_t long_time = 0;
	struct tm newtime = { 0 };

	// Get time as 64-bit integer.
	//
	_time64(&long_time);

	errno_t err = _localtime64_s(&newtime, &long_time);
	if (err)
	{
		log_err "_localtime64_s() failed" log_end
			return FALSE;
	}

	// e.g. 2009.07.06 23:04:33
	//
	WCHAR buf[20] = { 0 };
	if (!SUCCEEDED(StringCbPrintfW(
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

	TimeString = buf;
	return TRUE;
}

/**
* @brief	http://support.microsoft.com/kb/131065/EN-US/
* @param	privilege	e.g. SE_DEBUG_NAME
* @see
* @remarks
* @code
* @endcode
* @return
*/
bool set_privilege(_In_z_ const wchar_t* privilege, _In_ bool enable)
{
	//
	//    set_privilege(SE_DEBUG_NAME, true) ȣ�� �� set_privilege(SE_DEBUG_NAME, false) �� 
	//    ȣ���ϸ� set_privilege(SE_DEBUG_NAME, false) ȣ�� ������ OpenProcess() �� ����
	//    ������ �������� �ұ��ϰ�, �����ϴ� ��찡 �߻��Ѵ�. 
	//    ��Ȯ�� ���� �м��� �غ��� �ʾƼ� �� �𸣰���. ���߿� �ð����� �ѹ�...
	//    �ϴ� set_privilege(SE_DEBUG_NAME, false) �� ȣ������ �ʵ��� �ϰ� �ϱ� ����
	//    �����ڵ常 �߰��� ��
	//
	_ASSERTE(true == enable);
	if (true != enable) return false;

	if (IsWindowsXPOrGreater())
	{
		HANDLE hToken;
		if (TRUE != OpenThreadToken(GetCurrentThread(),
									TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
									FALSE,
									&hToken))
		{
			if (GetLastError() == ERROR_NO_TOKEN)
			{
				//
				//    Impersonate ���� ���� ������ ��� Thread �� Access token �� ������ ���� 
				//    ������ �׻� ERROR_NO_TOKEN �� �����Ѵ�.
				//
				//    �� ��� OpenProcessToken() �� ȣ���ؼ� ���μ����� Access token �� ��
				//    ���μ����� Privilige �� ������ �� �ִ�. 
				// 
				//    �ƴϸ� ImpersonateSelf() �� ȣ���ؼ� ���� �������� Access token �� �����ϰ�
				//    (������ token �� ���� �����忡���� ��ȿ) ���� �������� Privilege �� ������ 
				//    �� �ִ�. Privilege ���� �� ������ ������ RevertToSelf() �� ȣ���ؼ� 
				//    Impersion �� ���� �� �ִ�.
				//
				if (ImpersonateSelf(SecurityImpersonation) != TRUE)
				{
					log_err "ImpersonateSelf( ) failed. gle=%u",
						GetLastError()
						log_end;
					return false;
				}

				if (TRUE != OpenThreadToken(GetCurrentThread(),
											TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
											FALSE,
											&hToken))
				{
					log_err "OpenThreadToken() failed. gle=%u",
						GetLastError()
						log_end;
					return false;
				}
			}
			else
			{
				log_err "OpenThreadToken() failed. gle=%u",
					GetLastError()
					log_end;
				return false;
			}
		}
		handle_ptr token_ptr(hToken, [](_In_ HANDLE h) {CloseHandle(h); });

		TOKEN_PRIVILEGES tp = { 0 };
		TOKEN_PRIVILEGES tpPrevious = { 0 };
		LUID luid = { 0 };
		DWORD cb = sizeof(TOKEN_PRIVILEGES);
		if (!LookupPrivilegeValue(NULL,
								  privilege,
								  &luid))
		{
			log_err "LookupPrivilegeValue() failed. priv=%ws, gle=%u",
				privilege,
				GetLastError()
				log_end
				return false;
		}

		// 
		// first pass.  get current privilege setting
		// 
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = 0;
		if (!AdjustTokenPrivileges(hToken,
								   FALSE,
								   &tp,
								   sizeof(TOKEN_PRIVILEGES),
								   &tpPrevious,
								   &cb))
		{
			log_err "AdjustTokenPrivileges() failed. priv=%ws, gle=%u",
				privilege,
				GetLastError()
				log_end;
			return false;
		}

		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
			//
			//    luid �� ��Ī�Ǵ� Privilige �� ���� ����̹Ƿ� 
			//    tpPrevious �� count �� 0 �̾�� �Ѵ�. 
			//
			_ASSERTE(tpPrevious.PrivilegeCount == 00);
			log_err "Token does not have specified privilege. priv=%ws",
				privilege
				log_end;
			return false;
		}

		// 
		// second pass.  set privilege based on previous setting
		// 
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		if (enable)
		{
			tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
		}
		else
		{
			tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED &
													tpPrevious.Privileges[0].Attributes);
		}

		if (!AdjustTokenPrivileges(hToken,
								   FALSE,
								   &tpPrevious,
								   cb,
								   NULL,
								   NULL))
		{
			log_err "AdjustTokenPrivileges() failed. priv=%ws, gle=%u",
				privilege,
				GetLastError()
				log_end;
			return false;
		}

		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
			log_err "Token does not have specified privilege. priv=%ws",
				privilege
				log_end;
			return false;
		}
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

/// @brief	cmdline �� �����ϴ� ���μ����� �����ϴ� CreateProcessW �Լ� wrapper
bool 
create_process(
	_In_z_ const wchar_t* cmdline,
	_In_ DWORD creation_flag,
	_In_opt_z_ const wchar_t* current_dir,
	_Out_ HANDLE& process_handle,
	_Out_ DWORD& process_id)
{
	_ASSERTE(nullptr != cmdline);
	if (nullptr == cmdline) return false;

	// CreateProcessW �Լ��� cmdline �� ���� ������ �����̾�� �Ѵ�. 
	// ���� �Է����� ���� cmdline �� ���� ���۸� �Ҵ��ϰ�, �����ؼ�
	// ����Ѵ�.
	size_t buf_size = ((wcslen(cmdline) + 1) * sizeof(wchar_t));
	wchar_ptr cmdline_buf((wchar_t*)malloc(buf_size), [](wchar_t* p)
	{
		if (nullptr != p) free(p);
	});

	if (!cmdline_buf)
	{
		log_err "No memory for cmdline. size=%u",
			buf_size
			log_end;
		return false;
	}		
	RtlCopyMemory(cmdline_buf.get(), cmdline, wcslen(cmdline) * sizeof(wchar_t));
	cmdline_buf.get()[wcslen(cmdline)] = 0x0000;
	
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);

	if (!CreateProcessW(nullptr,
						cmdline_buf.get(),
						nullptr,
						nullptr,
						FALSE,
						creation_flag,
						nullptr,
						nullptr != current_dir ? current_dir : nullptr,
						&si,
						&pi))
	{
		log_err "CreateProcessW() failed. cmd=%ws, gle=%u",
			cmdline_buf.get(),
			GetLastError()
			log_end;
		return false;
	}

	process_handle = pi.hProcess;
	process_id = pi.dwProcessId;

	CloseHandle(pi.hThread);
	return true;
}

/// @brief	���μ����� �����ϰ�, ����ñ��� ��ٸ���. 
bool 
create_process_and_wait(
	_In_ const wchar_t* cmdline,
	_In_ DWORD creation_flag,
	_In_opt_z_ const wchar_t* current_dir,
	_In_ DWORD timeout_secs,
	_Out_ DWORD& exit_code
)
{
	HANDLE process_handle;
	DWORD process_id;

	if (!create_process(cmdline, creation_flag, current_dir, process_handle, process_id))
	{
		log_err "create_process() failed. cmdline=%ws", cmdline log_end;
		return false;
	}

	//
	//	Wait for the process
	//
	DWORD wr = WaitForSingleObject(process_handle, timeout_secs*1000);
	if (WAIT_OBJECT_0 != wr)
	{
		switch (wr)
		{
		case WAIT_ABANDONED:
			log_err 
				"WaitForSingleObject() failed for process. pid=%u, handle=0x%p, wr=WAIT_ABANDONED", 
				process_id, 
				process_handle
				log_end;
			break;
		case WAIT_TIMEOUT:
			log_err "WaitForSingleObject() failed for process. pid=%u, handle=0x%p, wr=WAIT_TIMEOUT",
				process_id,
				process_handle
				log_end;
			break;
		case WAIT_FAILED:
			log_err "WaitForSingleObject() failed for process. pid=%u, handle=0x%p, wr=WAIT_FAILED, gle=%u",
				process_id,
				process_handle, 
				GetLastError()
				log_end;
			break;
		default:
			_ASSERTE(!"oops! Unknown wait result code.");
		}

		//
		//	����Ǿ��ų� ������ ���μ����� ��������Ǿ��ٴ� ������ �����Ƿ�
		//	���� ���� �õ��Ѵ�. 
		//
		TerminateProcess(process_handle, 0xffffffff);
	}

	if (!GetExitCodeProcess(process_handle, &exit_code))
	{
		log_err "GetExitCodeProcess() failed. gle=%u", GetLastError() log_end;
		exit_code = 0xffffffff;		// exit_code -1 �� ����
	}

	//
	//	Cleanup
	//
	CloseHandle(process_handle);
	return true;
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
				if (true == rstrnicmp(proc_entry.szExeFile, L"explorer.exe"))
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
		si.lpDesktop = (LPWSTR)L"winsta0\\default";

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

/// @brief	���񽺿��� ������ Ŀ�ο�����Ʈ�� �α��� ����� ���α׷����� 
///			���ٰ����ϵ��� DACL �� �����Ѵ�. 
/// 
///			Built-in guests are denied all access.
///			Anonymous logon is denied all access.
///			Authenticated users are allowed read/write/execute access.
///			Administrators are allowed full control.
///
///			Creating a DACL
///			https://msdn.microsoft.com/en-us/library/windows/desktop/ms717798(v=vs.85).aspx
/// 
///			Security Descriptor String Format
///			https://msdn.microsoft.com/en-us/library/windows/desktop/aa379570(v=vs.85).aspx
///
///			ACE Strings
///			https://msdn.microsoft.com/en-us/library/windows/desktop/aa374928(v=vs.85).aspx
bool set_security_attributes_type1(_Out_ SECURITY_ATTRIBUTES& sa)
{
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = FALSE;

	const wchar_t* dacl_text = \
		L"D:"					// Discretionary ACL
		L"(D;OICI;GA;;;BG)"		// Deny access to built-in guests
		L"(D;OICI;GA;;;AN)"     // Deny access to anonymous logon

		//	ace_type; ace_flags; rights; object_guid; inherit_object_guid; account_sid; (resource_attribute)
		//
		//	A : SDDL_ACCESS_ALLOWED, D: SDDL_ACCESS_DENIED
		//	OI: SDDL_OBJECT_INHERIT
		//	CI: SDDL_CONTAINER_INHERIT
		//	GR: SDDL_GENERIC_READ
		//	GW: SDDL_GENERIC_WRITE
		//	GX: SDDL_GENERIC_EXECUTE
		//	n/a
		//	n/a
		//	AU: SDDL_AUTHENTICATED_USERS
		// 
		//	Allow 
		//		- read/write/execute 
		//		- to authenticated users
		L"(A;OICI;GRGWGX;;;AU)"

		//	ace_type; ace_flags; rights; object_guid; inherit_object_guid; account_sid; (resource_attribute)
		//
		//	A : SDDL_ACCESS_ALLOWED
		//	OI: SDDL_OBJECT_INHERIT
		//	CI: SDDL_CONTAINER_INHERIT
		//	GA: SDDL_GENERIC_ALL
		//	n/a
		//	n/a
		//	BA: SDDL_BUILTIN_ADMINISTRATORS, SY: SDDL_LOCAL_SYSTEM
		// 
		//	Allow full control to administrators
		L"(A;OICI;GA;;;BA)";

	if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(dacl_text,
															  SDDL_REVISION_1,
															  &(sa.lpSecurityDescriptor),
															  NULL))
	{
		log_err "ConvertStringSecurityDescriptorToSecurityDescriptorW() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	return true;
}

/// @brief	LOCAL_SYSTEM �������θ� ���� ������ DACL �� �����Ѵ�. 
/// 
///			Built-in guests are denied all access.
///			Anonymous logon is denied all access.
///			Authenticated users denied all access.
///			Local system are allowed full control.
///
///			Creating a DACL
///			https://msdn.microsoft.com/en-us/library/windows/desktop/ms717798(v=vs.85).aspx
/// 
///			Security Descriptor String Format
///			https://msdn.microsoft.com/en-us/library/windows/desktop/aa379570(v=vs.85).aspx
///
///			ACE Strings
///			https://msdn.microsoft.com/en-us/library/windows/desktop/aa374928(v=vs.85).aspx
bool set_security_attributes_type2(_Out_ SECURITY_ATTRIBUTES& sa)
{
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = FALSE;

	const wchar_t* dacl_text = \
		L"D:"					// Discretionary ACL
		L"(D;OICI;GA;;;BG)"		// Deny access to built-in guests
		L"(D;OICI;GA;;;AN)"     // Deny access to anonymous logon
		L"(D;OICI;GA;;;AU)"		// Deny access to authenticated user

		//	ace_type; ace_flags; rights; object_guid; inherit_object_guid; account_sid; (resource_attribute)
		//
		//	A : SDDL_ACCESS_ALLOWED
		//	OI: SDDL_OBJECT_INHERIT
		//	CI: SDDL_CONTAINER_INHERIT
		//	GA: SDDL_GENERIC_ALL
		//	n/a
		//	n/a
		//	SY: SDDL_LOCAL_SYSTEM
		// 
		//	Allow full control to LOCAL_SYSTE
		L"(A;OICI;GA;;;SY)";

	if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(dacl_text,
															  SDDL_REVISION_1,
															  &(sa.lpSecurityDescriptor),
															  NULL))
	{
		log_err "ConvertStringSecurityDescriptorToSecurityDescriptorW() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	return true;
}

/// @brief	Suspend process
bool suspend_process_by_pid(_In_ DWORD pid)
{
	HANDLE proc_handle = NULL;
	do
	{
		proc_handle = OpenProcess(PROCESS_SUSPEND_RESUME,
								  FALSE,
								  pid);
		if (NULL != proc_handle) break;

		if (!set_privilege(SE_DEBUG_NAME, true))
		{
			log_err "set_privilege( SE_DEBUG_NAME ) failed. " log_end;
			break;
		}

		proc_handle = OpenProcess(PROCESS_SUSPEND_RESUME,
								  FALSE,
								  pid);
	} while (false);

	if (NULL == proc_handle)
	{
		log_err "Can not access process. pid=%u",
			pid
			log_end;
		return false;
	}

	if (true != suspend_process_by_handle(proc_handle))
	{
		log_err "suspend_process_by_handle() failed. pid=%u",
			pid
			log_end;
	}

	CloseHandle(proc_handle);
	return true;
}

/// @brief	Resume process
bool resume_process_by_pid(_In_ DWORD pid)
{
	HANDLE proc_handle = NULL;
	do
	{
		proc_handle = OpenProcess(PROCESS_SUSPEND_RESUME,
								  FALSE,
								  pid);
		if (NULL != proc_handle) break;

		if (!set_privilege(SE_DEBUG_NAME, true))
		{
			log_err "set_privilege( SE_DEBUG_NAME ) failed. " log_end;
			break;
		}

		proc_handle = OpenProcess(PROCESS_SUSPEND_RESUME,
								  FALSE,
								  pid);
		set_privilege(SE_DEBUG_NAME, false);

	} while (false);

	if (NULL == proc_handle)
	{
		log_err "Can not access process. pid=%u",
			pid
			log_end;
		return false;
	}

	if (true != resume_process_by_handle(proc_handle))
	{
		log_err "resume_process_by_handle() failed. pid=%u",
			pid
			log_end;
	}

	CloseHandle(proc_handle);
	return true;
}

/// @brief	Terminate process
bool terminate_process_by_pid(_In_ DWORD pid, _In_ DWORD exit_code)
{
	HANDLE proc_handle = NULL;
	do
	{
		proc_handle = OpenProcess(PROCESS_TERMINATE,
								  FALSE,
								  pid);
		if (NULL != proc_handle) break;

		if (!set_privilege(SE_DEBUG_NAME, true))
		{
			log_err "set_privilege( SE_DEBUG_NAME ) failed. " log_end;
			break;
		}

		proc_handle = OpenProcess(PROCESS_TERMINATE,
								  FALSE,
								  pid);
	} while (false);

	if (NULL == proc_handle)
	{
		log_err "Can not access process. pid=%u",
			pid
			log_end;
		return false;
	}

	return terminate_process_by_handle(proc_handle, exit_code) ? true : false;
}

bool suspend_process_by_handle(_In_ HANDLE handle)
{
	typedef LONG NTSTATUS;
#define NT_SUCCESS(Status)  (((NTSTATUS)(Status)) >= 0)

	typedef NTSTATUS(NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
	NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtSuspendProcess");
	if (NULL == pfnNtSuspendProcess)
	{
		log_err "No NtSuspendProcess api." log_end;
		return false;
	}

	NTSTATUS status = pfnNtSuspendProcess(handle);
	if (!NT_SUCCESS(status))
	{
		log_err "NtSuspendProcess() failed., status=0x%08x",
			status
			log_end;
		return false;
	}

	return true;
}

bool resume_process_by_handle(_In_ HANDLE handle)
{
	typedef LONG NTSTATUS;
#define NT_SUCCESS(Status)  (((NTSTATUS)(Status)) >= 0)

	typedef NTSTATUS(NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);
	NtResumeProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtResumeProcess");
	if (NULL == pfnNtResumeProcess)
	{
		log_err "No NtResumeProcess api." log_end;
		return false;
	}

	NTSTATUS status = pfnNtResumeProcess(handle);
	if (!NT_SUCCESS(status))
	{
		log_err "NtResumeProcess() failed., status=0x%08x",
			status
			log_end;
		return false;
	}

	return true;
}

bool terminate_process_by_handle(_In_ HANDLE handle, _In_ DWORD exit_code)
{
	if (0 == TerminateProcess(handle, exit_code))
	{
		log_err "TerminateProcess() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	return true;
}

/// @brief	���μ����� ����ð��� ���Ѵ�.
bool
get_process_creation_time(
	_In_ DWORD pid,
	_Out_ PFILETIME const creation_time
)
{
	_ASSERTE(0 != pid);
	_ASSERTE(4 != pid);
	_ASSERTE(nullptr != creation_time);
	if (0 == pid || 4 == pid || nullptr == creation_time) return false;

	HANDLE process_handle = NULL;
	do
	{
		process_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
		if (NULL != process_handle)
		{
			break;
		}
		if (!set_privilege(SE_DEBUG_NAME, true)) { break; }
		process_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	} while (false);


	if (NULL == process_handle)
	{
		log_err "Can not access process. pid=%u",
			pid
			log_end;
		return false;
	}

	handle_ptr hg(process_handle, [](HANDLE h) {CloseHandle(h); });
	return get_process_creation_time(process_handle, creation_time);
}

/// @brief	���μ����� ����ð��� ���Ѵ�.
bool
get_process_creation_time(
	_In_ HANDLE process_handle,
	_Out_ PFILETIME const creation_time
)
{
	_ASSERTE(nullptr != creation_time);
	if (nullptr == creation_time) return false;

	FILETIME CreationTime;
	FILETIME ExitTime;
	FILETIME KernelTime;
	FILETIME UserTime;

	if (!GetProcessTimes(process_handle,
						 &CreationTime,
						 &ExitTime,
						 &KernelTime,
						 &UserTime))
	{
		log_err "GetProcessTimes() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	creation_time->dwLowDateTime = CreationTime.dwLowDateTime;
	creation_time->dwHighDateTime = CreationTime.dwHighDateTime;
	return true;
}

/// @brief 
void dump_file_create_disposition(_In_ uint32_t NtCreateFile_CreateDisposition)
{
	char buf[256];
	char* pos = buf;
	size_t remain = sizeof(buf);
	bool add_lf = false;

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005

	if (NtCreateFile_CreateDisposition & FILE_SUPERSEDE)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "%s",
						  "FILE_SUPERSEDE, ");
		add_lf = true;
	}
	if (NtCreateFile_CreateDisposition & FILE_OPEN)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OPEN");
		add_lf = true;
	}
	if (NtCreateFile_CreateDisposition & FILE_CREATE)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_CREATE");
		add_lf = true;
	}
	if (NtCreateFile_CreateDisposition & FILE_OPEN_IF)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OPEN_IF");
		add_lf = true;
	}
	if (NtCreateFile_CreateDisposition & FILE_OVERWRITE)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OVERWRITE");
		add_lf = true;
	}
	if (NtCreateFile_CreateDisposition & FILE_OVERWRITE_IF)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OVERWRITE_IF");
		add_lf = true;
	}

	if (add_lf == true)
	{
		log_info "disposition=%s", buf log_end;
	}
	else
	{
		log_info "disposition=None" log_end;
	}
}

void dump_file_create_options(_In_ uint32_t NtCreateFile_CreateOptions)
{
#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_REMOTE_INSTANCE               0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000

#define FILE_OPEN_REQUIRING_OPLOCK              0x00010000
#define FILE_DISALLOW_EXCLUSIVE                 0x00020000
#define FILE_SESSION_AWARE                      0x00040000

#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000

	char buf[256];
	char* pos = buf;
	size_t remain = sizeof(buf);
	bool add_lf = false;

	if (NtCreateFile_CreateOptions & FILE_DIRECTORY_FILE)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "%s",
						  "FILE_DIRECTORY_FILE");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_WRITE_THROUGH)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_WRITE_THROUGH");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_SEQUENTIAL_ONLY)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_SEQUENTIAL_ONLY");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_NO_INTERMEDIATE_BUFFERING)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_NO_INTERMEDIATE_BUFFERING");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_SYNCHRONOUS_IO_ALERT)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_SYNCHRONOUS_IO_ALERT");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_SYNCHRONOUS_IO_NONALERT)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_SYNCHRONOUS_IO_NONALERT");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_NON_DIRECTORY_FILE)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_NON_DIRECTORY_FILE");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_CREATE_TREE_CONNECTION)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_CREATE_TREE_CONNECTION");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_COMPLETE_IF_OPLOCKED)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_COMPLETE_IF_OPLOCKED");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_NO_EA_KNOWLEDGE)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_NO_EA_KNOWLEDGE");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_OPEN_REMOTE_INSTANCE)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OPEN_REMOTE_INSTANCE");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_RANDOM_ACCESS)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_RANDOM_ACCESS");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_DELETE_ON_CLOSE)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_DELETE_ON_CLOSE");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_OPEN_BY_FILE_ID)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OPEN_BY_FILE_ID");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_OPEN_FOR_BACKUP_INTENT)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OPEN_FOR_BACKUP_INTENT");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_NO_COMPRESSION)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_NO_COMPRESSION");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_OPEN_REQUIRING_OPLOCK)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OPEN_REQUIRING_OPLOCK");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_DISALLOW_EXCLUSIVE)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_DISALLOW_EXCLUSIVE");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_SESSION_AWARE)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_SESSION_AWARE");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_RESERVE_OPFILTER)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_RESERVE_OPFILTER");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_OPEN_REPARSE_POINT)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OPEN_REPARSE_POINT");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_OPEN_NO_RECALL)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OPEN_NO_RECALL");
		add_lf = true;
	}
	if (NtCreateFile_CreateOptions & FILE_OPEN_FOR_FREE_SPACE_QUERY)
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "FILE_OPEN_FOR_FREE_SPACE_QUERY");
		add_lf = true;
	}

	if (add_lf == true)
	{
		log_info "options=%s", buf log_end;
	}
	else
	{
		log_info "options=None" log_end;
	}
}

void dump_group_attributes(_In_ uint32_t group_attributes)
{
	char buf[256];
	char* pos = buf;
	size_t remain = sizeof(buf);
	bool add_lf = false;

	if (FlagOn(group_attributes, SE_GROUP_MANDATORY))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "%s",
						  "SE_GROUP_MANDATORY");
		add_lf = true;
	}

	if (FlagOn(group_attributes, SE_GROUP_ENABLED_BY_DEFAULT))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "SE_GROUP_ENABLED_BY_DEFAULT");
		add_lf = true;
	}
	if (FlagOn(group_attributes, SE_GROUP_ENABLED))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "SE_GROUP_ENABLED");
		add_lf = true;
	}
	if (FlagOn(group_attributes, SE_GROUP_OWNER))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "SE_GROUP_OWNER");
		add_lf = true;
	}
	if (FlagOn(group_attributes, SE_GROUP_USE_FOR_DENY_ONLY))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "SE_GROUP_USE_FOR_DENY_ONLY");
		add_lf = true;
	}
	if (FlagOn(group_attributes, SE_GROUP_INTEGRITY))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "SE_GROUP_INTEGRITY");
		add_lf = true;
	}
	if (FlagOn(group_attributes, SE_GROUP_INTEGRITY_ENABLED))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "SE_GROUP_INTEGRITY_ENABLED");
		add_lf = true;
	}
	if (FlagOn(group_attributes, SE_GROUP_LOGON_ID))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "SE_GROUP_LOGON_ID");
		add_lf = true;
	}
	if (FlagOn(group_attributes, SE_GROUP_RESOURCE))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "SE_GROUP_RESOURCE");
		add_lf = true;
	}

	if (add_lf == true)
	{
		log_info "options=%s", buf log_end;
	}
	else
	{
		log_info "options=None" log_end;
	}
}

void dump_privilege_attributes(_In_ uint32_t privilege_attributes)
{
	bool add_lf = false;
	char buf[256];
	char* pos = buf;
	size_t remain = sizeof(buf);

	if (FlagOn(privilege_attributes, SE_PRIVILEGE_ENABLED))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "%s",
						  "SE_PRIVILEGE_ENABLED");
		add_lf = true;
	}

	if (FlagOn(privilege_attributes, SE_PRIVILEGE_ENABLED_BY_DEFAULT))
	{

		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "SE_PRIVILEGE_ENABLED_BY_DEFAULT");
		add_lf = true;
	}
	if (FlagOn(privilege_attributes, SE_PRIVILEGE_REMOVED))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "%s",
						  "SE_PRIVILEGE_REMOVED");
		add_lf = true;
	}
	if (FlagOn(privilege_attributes, SE_PRIVILEGE_USED_FOR_ACCESS))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "%s",
						  "SE_PRIVILEGE_USED_FOR_ACCESS");
		add_lf = true;
	}

	if (add_lf == true)
	{
		log_info "options=%s", buf log_end;
	}
	else
	{
		log_info "options=None" log_end;
	}
}

/// @brief	
psid_info get_sid_info(_In_ PSID sid)
{
	_ASSERTE(nullptr != sid);
	if (nullptr == sid) return nullptr;

	//
	//	SID  
	// 
	wchar_t* sid_str = nullptr;
	if (TRUE != ConvertSidToStringSidW(sid, &sid_str))
	{
		DWORD gle = GetLastError();
		const char* gles = nullptr;
		switch (gle)
		{
		case ERROR_NOT_ENOUGH_MEMORY: gles = "ERROR_NOT_ENOUGH_MEMORY"; break;
		case ERROR_INVALID_SID: gles = "ERROR_INVALID_SID"; break;
		case ERROR_INVALID_PARAMETER: gles = "ERROR_INVALID_PARAMETER"; break;
		}
		
		if (nullptr != gles)
		{
			log_err "ConvertSidToStringSidW() failed. gle=%s",
				gles
				log_end;
		}
		else
		{
			log_err "ConvertSidToStringSidW() failed. gle=%u",
				gle
				log_end;
		}
		return nullptr;
	}

	//
	//	sid_string ���۴� �ݵ�� LocalFree() �� �Ҹ��ؾ� �Ѵ�. 
	//	
	wchar_ptr sid_ptr(sid_str, [](wchar_t* p) {LocalFree(p); });

	//
	//	 User/Group, Reference domain name mapped with this SID
	//
	DWORD cch_name = 0;
	wchar_t* name = nullptr;
	DWORD cch_domain = 0;
	wchar_t* domain = nullptr;
	SID_NAME_USE sid_name_use = SidTypeUnknown;
	LookupAccountSid(nullptr,
					 sid,
					 nullptr,
					 &cch_name,
					 nullptr,
					 &cch_domain,
					 &sid_name_use);
	if (cch_name > 0)
	{
		name = (wchar_t*)malloc((cch_name + 1) * sizeof(wchar_t));
		if (nullptr == name)
		{
			log_err "Not enough memory. " log_end;
			return nullptr;
		}
	}

	if (cch_domain > 0)
	{
		domain = (wchar_t*)malloc((cch_domain + 1) * sizeof(wchar_t));
		if (nullptr == name)
		{
			log_err "Not enough memory. " log_end;
			return nullptr;
		}
	}
	wchar_ptr name_ptr(name, [](_In_ wchar_t* ptr) {if (nullptr != ptr) { free(ptr); }});
	wchar_ptr domain_ptr(domain, [](_In_ wchar_t* ptr) {if (nullptr != ptr) { free(ptr); }});

	if (TRUE != LookupAccountSid(nullptr,
								 sid,
								 name_ptr.get(),
								 &cch_name,
								 domain_ptr.get(),
								 &cch_domain,
								 &sid_name_use))
	{
		// 
		//	�α��� ��ī��Ʈ�� ���ε��� ���� SID(e.g. logon SID)�� ��� �Ǵ� 
		//	�α��� �̸��� ã�ٰ� ��Ʈ��ũ Ÿ�Ӿƿ��� �߻��� ��� �Ǵ� ���ε� �α��� ������
		//	���°��(�α��� ������ ���� Group SID �� ���) � ERROR_NONE_MAPPED �� 
		//	������ �� �ִ�. 
		//
		//log_err "LookupAccountSid() failed. gle=%u",
		//	GetLastError()
		//	log_end;
		//
	}

	return new sid_info(sid_ptr.get(),
						domain_ptr.get(),
						name_ptr.get(),
						sid_name_use);
}

/// @brief	pid ���μ����� ����� ������ ���Ѵ�. 
psid_info
get_process_user(
	_In_ DWORD pid
)
{
	//
	//	Open process handle with READ token access
	//
	handle_ptr proc_handle(
		OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
					FALSE,
					pid),
		[](_In_ HANDLE handle)
	{
		if (NULL != handle) { CloseHandle(handle); }
	});
	if (NULL == proc_handle.get())
	{
		log_err "OpenProcess() failed. pid=%u, gle=%u",
			pid,
			GetLastError()
			log_end;
		return false;
	}

	//
	//	Open token handle
	//
	HANDLE th;
	if (TRUE != OpenProcessToken(proc_handle.get(),
								 TOKEN_QUERY,
								 &th))
	{
		log_err "OpenProcessToken() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}
	handle_ptr token_handle(th, [](_In_ HANDLE th) {CloseHandle(th); });
	return get_process_user(token_handle.get());
}

/// @brief	
psid_info
get_process_user(
	_In_ HANDLE process_query_token
)
{
	_ASSERTE(NULL != process_query_token);
	if (NULL == process_query_token) return nullptr;

	//
	//	Get token information
	//
	DWORD return_length;
	GetTokenInformation(process_query_token,
						TokenUser,
						nullptr,
						0,
						&return_length);
	DWORD gle = GetLastError();
	if (gle != ERROR_INSUFFICIENT_BUFFER)
	{
		log_err "GetTokenInformation() failed. gle=%u",
			gle
			log_end;
		return nullptr;
	}

	char_ptr ptr(
		(char*)malloc(return_length),
		[](_In_ char* ptr)
	{
		if (nullptr != ptr) free(ptr);
	});
	if (nullptr == ptr.get())
	{
		log_err "Not enough memory. malloc size=%u",
			return_length
			log_end;
		return nullptr;
	}

	if (TRUE != GetTokenInformation(process_query_token,
									TokenUser,
									(PTOKEN_USER)ptr.get(),
									return_length,
									&return_length))
	{
		log_err "GetTokenInformation() failed. gle=%u",
			GetLastError()
			log_end;
		return nullptr;
	}

	return get_sid_info(((PTOKEN_USER)ptr.get())->User.Sid);
}

/// @brief	
bool
get_process_group(
	_In_ DWORD pid,
	_Out_ std::list<pgroup_sid_info>& group
)
{
	//
	//	Open process handle with READ token access
	//
	handle_ptr proc_handle(
		OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid),
		[](_In_ HANDLE handle)
	{
		if (NULL != handle) { CloseHandle(handle); }
	});
	if (NULL == proc_handle.get())
	{
		log_err "OpenProcess() failed. pid=%u, gle=%u",
			pid,
			GetLastError()
			log_end;
		return false;
	}

	//
	//	Open token handle
	//
	HANDLE th;
	if (TRUE != OpenProcessToken(proc_handle.get(),
								 TOKEN_QUERY,
								 &th))
	{
		log_err "OpenProcessToken() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}
	handle_ptr token_handle(th, [](_In_ HANDLE th) {CloseHandle(th); });

	return get_process_group(token_handle.get(), group);
}

bool
get_process_group(
	_In_ HANDLE process_query_token,
	_Out_ std::list<pgroup_sid_info>& group
)
{
	_ASSERTE(NULL != process_query_token);
	if (NULL == process_query_token) return false;

	//
	//	Get token information
	//
	DWORD return_length;
	GetTokenInformation(process_query_token,
						TokenGroups,
						nullptr,
						0,
						&return_length);
	DWORD gle = GetLastError();
	if (gle != ERROR_INSUFFICIENT_BUFFER)
	{
		log_err "GetTokenInformation() failed. gle=%u",
			gle
			log_end;
		return false;
	}

	char_ptr ptr(
		(char*)malloc(return_length),
		[](_In_ char* ptr)
	{
		if (nullptr != ptr) free(ptr);
	});
	if (nullptr == ptr.get())
	{
		log_err "Not enough memory. malloc size=%u",
			return_length
			log_end;
		return false;
	}

	if (TRUE != GetTokenInformation(process_query_token,
									TokenGroups,
									(PTOKEN_GROUPS)ptr.get(),
									return_length,
									&return_length))
	{
		log_err "GetTokenInformation() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	PTOKEN_GROUPS token_groups = (PTOKEN_GROUPS)ptr.get();
	for (uint32_t i = 0; i < token_groups->GroupCount; ++i)
	{
		psid_info group_sid = get_sid_info(token_groups->Groups[i].Sid);
		if (nullptr != group_sid)
		{
			pgroup_sid_info g = new group_sid_info(group_sid,
												   token_groups->Groups[i].Attributes);
			if (nullptr == g)
			{
				delete group_sid; group_sid = nullptr;
			}
			else
			{
				group.push_back(g);
			}
		}
	}

	return true;
}

pprivilege_info
get_privilege_info(
	_In_ LUID_AND_ATTRIBUTES privileges
)
{
	DWORD cch_name = 0;
	wchar_t* name = nullptr;
	LookupPrivilegeNameW(nullptr,
						 &privileges.Luid,
						 nullptr,
						 &cch_name);
	if (0 < cch_name)
	{
		name = (wchar_t*)malloc((cch_name + 1) * sizeof(wchar_t));
		if (nullptr == name)
		{
			log_err "Not enough memory. " log_end;
			return nullptr;
		}
	}

	wchar_ptr name_ptr(name, [](_In_ wchar_t* ptr) {if (nullptr != ptr) { free(ptr); }});

	if (TRUE != LookupPrivilegeNameW(nullptr,
									 &privileges.Luid,
									 name_ptr.get(),
									 &cch_name))
	{
		log_err
			"LookupPrivilegeNameW failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}


	return new privilege_info(name_ptr.get(),
							  privileges.Attributes);
}

/// @brief ���μ��� ������ ȹ��
bool
get_process_privilege(
	_In_ DWORD pid,
	_Out_ std::list<pprivilege_info>& privileges
	)
{
	//
	//	Open process handle with READ token access
	//
	handle_ptr proc_handle(
		OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid),
		[](_In_ HANDLE handle)
	{
		if (NULL != handle) { CloseHandle(handle); }
	});
	if (NULL == proc_handle.get())
	{
		log_err "OpenProcess() failed. pid=%u, gle=%u",
			pid,
			GetLastError()
			log_end;
		return false;
	}

	//
	//	Open token handle
	//
	HANDLE th;
	if (TRUE != OpenProcessToken(proc_handle.get(),
								 TOKEN_QUERY,
								 &th))
	{
		log_err "OpenProcessToken() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}
	handle_ptr token_handle(th, [](_In_ HANDLE th) {CloseHandle(th); });

	return get_process_privilege(token_handle.get(), privileges);
}

bool
get_process_privilege(
	_In_ HANDLE process_query_token,
	_Out_ std::list<pprivilege_info>& privileges
	)
{
	_ASSERTE(NULL != process_query_token);
	if (NULL == process_query_token) return false;

	//
	//	Get token information
	//
	DWORD return_length;
	GetTokenInformation(process_query_token,
						TokenPrivileges,
						nullptr,
						0,
						&return_length);
	DWORD gle = GetLastError();
	if (gle != ERROR_INSUFFICIENT_BUFFER)
	{
		log_err "GetTokenInformation() failed. gle=%u",
			gle
			log_end;
		return false;
	}

	char_ptr ptr(
		(char*)malloc(return_length),
		[](_In_ char* ptr)
	{
		if (nullptr != ptr) { free(ptr); }
	});

	if (nullptr == ptr.get())
	{
		log_err "Not enough memory. malloc size=%u",
			return_length
			log_end;
		return false;
	}

	if (TRUE != GetTokenInformation(process_query_token,
									TokenPrivileges,
									(PTOKEN_PRIVILEGES)ptr.get(),
									return_length,
									&return_length))
	{
		log_err "GetTokenInformation() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	PTOKEN_PRIVILEGES token_privileges = (PTOKEN_PRIVILEGES)ptr.get();
	for (uint32_t i = 0; i < token_privileges->PrivilegeCount; ++i)
	{
		pprivilege_info privilege_info = get_privilege_info(
			token_privileges->Privileges[i]);
		if (nullptr != privilege_info)
		{
			privileges.push_back(privilege_info);
		}
	}

	return true;
}

/// @brief ���μ��� integrity level�� �����´�.
bool
get_process_integrity_level(
	_In_ DWORD pid,
	_Out_ DWORD& integrity_level
	)
{
	//
	//	Open process handle with READ token access
	//
	handle_ptr proc_handle(
		OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid),
		[](_In_ HANDLE handle)
	{
		if (NULL != handle) { CloseHandle(handle); }
	});
	if (NULL == proc_handle.get())
	{
		log_err "OpenProcess() failed. pid=%u, gle=%u",
			pid,
			GetLastError()
			log_end;
		return false;
	}

	//
	//	Open token handle
	//
	HANDLE th;
	if (TRUE != OpenProcessToken(proc_handle.get(),
		TOKEN_QUERY,
		&th))
	{
		log_err "OpenProcessToken() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}
	handle_ptr token_handle(th, [](_In_ HANDLE th) {CloseHandle(th); });

	return get_process_integrity_level(token_handle.get(), integrity_level);
}

bool
get_process_integrity_level(
	_In_ HANDLE process_query_token,
	_Out_ DWORD& integrity_level
	)
{
	_ASSERTE(NULL != process_query_token);
	if (NULL == process_query_token) return false;

	//
	//	Get token information
	//
	DWORD return_length;
	GetTokenInformation(process_query_token,
						TokenIntegrityLevel,
						nullptr,
						0,
						&return_length);
	DWORD gle = GetLastError();
	if (gle != ERROR_INSUFFICIENT_BUFFER)
	{
		log_err "GetTokenInformation() failed. gle=%u",
			gle
			log_end;
		return false;
	}

	char_ptr ptr(
		(char*)malloc(return_length),
		[](_In_ char* ptr)
	{
		if (nullptr != ptr) free(ptr);
	});
	if (nullptr == ptr.get())
	{
		log_err "Not enough memory. malloc size=%u",
			return_length
			log_end;
		return false;
	}

	if (TRUE != GetTokenInformation(process_query_token,
									TokenIntegrityLevel,
									(PTOKEN_MANDATORY_LABEL)ptr.get(),
									return_length,
									&return_length))
	{
		log_err "GetTokenInformation() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	PTOKEN_MANDATORY_LABEL mandatory_label = (PTOKEN_MANDATORY_LABEL)ptr.get();

	integrity_level = *GetSidSubAuthority(mandatory_label->Label.Sid,
										  ((PISID)mandatory_label->Label.Sid)->SubAuthorityCount - 1);

	return true;
}

/// @brief ���μ��� token elevation type�� �����´�.
bool
get_process_token_elevation_type(
	_In_ DWORD pid,
	_Out_ DWORD& token_elevation_type
	)
{
	//
	//	Open process handle with READ token access
	//
	handle_ptr proc_handle(
		OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid),
		[](_In_ HANDLE handle)
	{
		if (NULL != handle) { CloseHandle(handle); }
	});
	if (NULL == proc_handle.get())
	{
		log_err "OpenProcess() failed. pid=%u, gle=%u",
			pid,
			GetLastError()
			log_end;
		return false;
	}

	//
	//	Open token handle
	//
	HANDLE th;
	if (TRUE != OpenProcessToken(proc_handle.get(),
		TOKEN_QUERY,
		&th))
	{
		log_err "OpenProcessToken() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}
	handle_ptr token_handle(th, [](_In_ HANDLE th) {CloseHandle(th); });

	return get_process_token_elevation_type(token_handle.get(), token_elevation_type);
}

bool
get_process_token_elevation_type(
	_In_ HANDLE process_query_token,
	_Out_ DWORD& token_elevation_type
	)
{
	_ASSERTE(NULL != process_query_token);
	if (NULL == process_query_token) return false;

	//
	//	Get token information
	//
	DWORD return_length;
	if (TRUE != GetTokenInformation(process_query_token,
									TokenElevationType,
									&token_elevation_type,
									sizeof(DWORD),
									&return_length))
	{
		log_err "GetTokenInformation() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}
	
	return true;
}
/// @brief ���μ��� toekn elevation�� �����´�.
bool
get_process_token_elevated(
	_In_ DWORD pid,
	_Out_ bool& token_is_elevated
	)
{
	//
	//	Open process handle with READ token access
	//
	handle_ptr proc_handle(
		OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid),
		[](_In_ HANDLE handle)
	{
		if (NULL != handle) { CloseHandle(handle); }
	});
	if (NULL == proc_handle.get())
	{
		log_err "OpenProcess() failed. pid=%u, gle=%u",
			pid,
			GetLastError()
			log_end;
		return false;
	}

	//
	//	Open token handle
	//
	HANDLE th;
	if (TRUE != OpenProcessToken(proc_handle.get(),
		TOKEN_QUERY,
		&th))
	{
		log_err "OpenProcessToken() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}
	handle_ptr token_handle(th, 
							[](_In_ HANDLE th) {CloseHandle(th); });

	return get_process_token_elevated(token_handle.get(), 
									  token_is_elevated);
}

bool
get_process_token_elevated(
	_In_ HANDLE process_query_token,
	_Out_ bool& token_is_elevated
	)
{
	_ASSERTE(NULL != process_query_token);
	if (NULL == process_query_token) return false;

	//
	//	Get token information
	//
	DWORD return_length = 0;
	TOKEN_ELEVATION te = {0x00};
	if (TRUE != GetTokenInformation(process_query_token,
									TokenElevation,
									&te,
									sizeof(te),
									&return_length))
	{
		log_err "GetTokenInformation() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	token_is_elevated = (0 != te.TokenIsElevated ? true : false);
	return true;
}

/// @brief ��ġ�� ���α׷��� ����(���α׷���, ����, ������) �о� �´�.
///
#pragma todo("���� ������� ��ġ�� ���α׷� ������ �о� ���� ����� �߰� �ؾ��Ѵ�.")
pprogram
get_installed_program_info(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key_name
)
{
	DWORD is_system_component = RUReadDword(key_handle,
											L"SystemComponent",
											0);
	// if system component flags
	if (1 == is_system_component)
	{
		//
		// SystemComponent flags�� ������ ��� ���α׷� �߰�/����
		// Ȥ�� App & features�� ��Ͽ��� ������ �ʴ´�. ���� ���α׷�
		// �߰�/���� Ȥ�� App & features ��� �������� ������ �ϱ� ��
		// ���� �ش� �÷��װ� ������ ��� ���� ���� �ʴ´�.
		//
		return nullptr;
	}
	else if (0 == is_system_component)
	{
		std::wstring name;
		if (!RUReadString(key_handle,
						  L"DisplayName",
						  name))
		{
			//log_err
			//	"RUReadString failed(valu=`DisplayName`). key=%ws",
			//	sub_key_name
			//	log_end;
		}

		std::wstring version;
		if (!RUReadString(key_handle,
						  L"DisplayVersion",
						  version))
		{
			//log_err
			//	"RUReadString failed(valu=`DisplayVersion`). key=%ws",
			//	sub_key_name
			//	log_end;
		}

		std::wstring publisher;
		if (!RUReadString(key_handle,
						  L"Publisher",
						  publisher))
		{
			//log_err
			//	"RUReadString failed(valu=`DisplayVersion`). key=%ws",
			//	sub_key_name
			//	log_end;
		}

		//
		//	Uninstall String
		//
		std::wstring uninstaller;
		if (!RUReadString(key_handle,
						  L"UninstallString",
						  uninstaller))
		{
			//log_err
			//	"RUReadString failed(valu=`DisplayVersion`). key=%ws",
			//	sub_key_name
			//	log_end;
		}

		if (true == name.empty())
		{
			//
			// ���α׷����� �� �� ���� ��쿡�� ó�� ���� �ʴ´�.
			//
			return nullptr;
		}
		else
		{
			return new program(sub_key_name,
							   name.c_str(),
							   publisher.c_str(),
							   version.c_str(), 
							   uninstaller.c_str());
		}
	}
	_ASSERTE(!"oops nerver reach");
	return nullptr;
}

/// @brief ��ġ�� ���α׷��� ������ �о� ���� ���� `callback` �Լ� �̴�.
///
bool
sub_key_iterate_callback(
	_In_ uint32_t index,
	_In_ const wchar_t* base_name,
	_In_ const wchar_t* sub_key_name,
	_In_ const wchar_t* class_name,
	_In_ DWORD_PTR tag
)
{
	UNREFERENCED_PARAMETER(index);
	UNREFERENCED_PARAMETER(class_name);

	std::list<pprogram>* softwares = (std::list<pprogram>*)tag;

	//
	// sub key�� ��ü ��θ� �����.
	// eg. base name: HKLM\SoftwareL\Software\Microsoft
	//                 \Windows\CurrentVersion\Uninstall\
	//     sub_key_name: Everything
	//     full path: HKLM\SoftwareL\Software\Microsoft\Windows\Current
	//                Version\Uninstall\Everything
	//
	std::wstringstream strm;
	strm << base_name << sub_key_name;
	HKEY key_handle = RUOpenKey(HKEY_LOCAL_MACHINE,
								strm.str().c_str(),
								true);

	if (nullptr == key_handle)
	{
		//log_err "RUOpenKey() failed. key=%ws",
		//	strm.str().c_str()
		//	log_end;
		return false;
	}

	pprogram sf = get_installed_program_info(key_handle,
											 sub_key_name);
	if (nullptr != sf)
	{
		softwares->push_back(sf);
	}

	RUCloseKey(key_handle);
	return true;
}


/// @brief ��ġ�� ���α׷� ������ �о� �´�.
///
bool
get_installed_programs(
	_Out_ std::list<pprogram>& installed_programs
)
{
	//
	// HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\
	// �� �ִ� ���α׷� ������ �о� �´�.
	// 64bit �ü���� ��� �ش� ��ο� �ִ� ���ø����̼� ������ 64��Ʈ
	// ���α׷��� ���� ���� �̴�.
	//
	HKEY key_handle = RUOpenKey(HKEY_LOCAL_MACHINE,
								sub_key_uninstall,
								true);

	if (nullptr == key_handle)
	{
		//log_err "RUOpenKey() failed. key=%ws",
		//	sub_key_uninstall
		//	log_end;
		return false;
	}

	//
	// ��ġ�� ���α׷��� ���� list�� base_key(`sub_key_uninstall`)
	// �� �ݹ� �Լ��� �ѱ��.
	//
	reg_enum_key_values(key_handle,
						sub_key_uninstall,
						sub_key_iterate_callback,
						(DWORD_PTR)&installed_programs,
						NULL,
						NULL);
	RUCloseKey(key_handle);

	//
	// HKLM\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\
	// ��ΰ� ���� �ϴ� ��� �ش� ������ �о� �´�. `WOW6432Node`�� �ִ� ���ø����̼�
	// ������ 64��Ʈ �ü������ 32��Ʈ ���α׷� ������ ���� �ϰ� �ִ� ����̴�.
	// 64��Ʈ �ü���� ��� ���� �ش� ��� ���� ������ üũ �� �� ������ �а� �� ��
	// 64��Ʈ ���α׷� ������ �о� �´�.
	//
	if (true == RUIsKeyExists(HKEY_LOCAL_MACHINE,
							  sub_key_uninstall_x64))
	{
		HKEY wow64_key_handle = RUOpenKey(HKEY_LOCAL_MACHINE,
										  sub_key_uninstall_x64,
										  true);

		if (nullptr == wow64_key_handle)
		{
			//log_err "RUOpenKey() failed. key=%ws",
			//	sub_key_uninstall_x64
			//	log_end;
			return false;
		}

		//
		// ��ġ�� ���α׷��� ���� list�� base_key(`sub_key_uninstall_x64`)
		// �� �ݹ� �Լ��� �ѱ��.
		//
		reg_enum_key_values(wow64_key_handle,
							sub_key_uninstall_x64,
							sub_key_iterate_callback,
							(DWORD_PTR)&installed_programs,
							NULL,
							NULL);
		RUCloseKey(wow64_key_handle);
	}


	return true;
}

/// @brief	������ ���� ������ ���Ѵ�. 
///			(��ȯ �� psid_info�� �ݵ�� �����ؾ��Ѵ�.)
psid_info
get_file_owner(
	_In_ const wchar_t* file_name
)
{
	_ASSERTE(nullptr != file_name);
	if (nullptr == file_name) return nullptr;

	//
	//	Get the required size for the security descriptor.
	//
	DWORD return_length;
	GetFileSecurity(file_name,
					OWNER_SECURITY_INFORMATION,
					nullptr,
					0,
					&return_length);
	DWORD gle = GetLastError();
	if (gle != ERROR_INSUFFICIENT_BUFFER)
	{
		log_err "GetFileSecurity() failed. gle=%u",
			gle
			log_end;
		return nullptr;
	}

	char_ptr ptr((char*)malloc(return_length),
				 [](_In_ char* ptr)
	{
		if (nullptr != ptr) free(ptr);
	});
	if (nullptr == ptr.get())
	{
		log_err "Not enough memory. malloc size=%u",
			return_length
			log_end;
		return nullptr;
	}

	//
	//	get information about the security of a file
	//
	if (TRUE != GetFileSecurity(file_name,
								OWNER_SECURITY_INFORMATION,
								(PSECURITY_DESCRIPTOR)ptr.get(),
								return_length,
								&return_length))
	{
		log_err "GetFileSecurity() failed. gle=%u",
			GetLastError()
			log_end;
		return nullptr;
	}

	//
	//	Find the name of the owner and owning group.
	//
	BOOL owner_def;
	PSID owner_sid;
	if (TRUE != GetSecurityDescriptorOwner((PSECURITY_DESCRIPTOR)ptr.get(),
										   &owner_sid,
										   &owner_def))
	{
		log_err "GetSecurityDescriptorOwner() failed. gle=%u",
			GetLastError()
			log_end;
		return nullptr;
	}

	return get_sid_info(owner_sid);
}

/// @brief	 Windows Error Reporting ȯ�� ����
bool setup_wer(_In_ const wchar_t* dump_dir)
{
	_ASSERTE(nullptr != dump_dir);
	if (nullptr == dump_dir)
	{
		return false;
	}

	//
	//	Setup WER(Windows Error Reporting)
	//	"/wer c:\dump"
	//
	//	option::DumpType(2:FullDump)�� ����
	//		0 : CustomDump
	//		1 : MiniDump (default)
	//		2 : FullDump
	//
	//	option::DumpFolder
	//		���� ���� ���(�Էµ� ��η� ����)
	// 
	std::wstring key_path;
	key_path = L"Software\\Microsoft\\Windows\\Windows Error Reporting\\LocalDumps";
	RegHandle KeyHandle(RUCreateKey(HKEY_LOCAL_MACHINE,
									key_path.c_str(),
									false));
	if (NULL == KeyHandle.get())
	{
		log_err "RUCreateKey() failed. key=%ws",
			key_path.c_str()
			log_end;
		return false;
	}

	//	set option - DumpType
	if (true != RUWriteDword(KeyHandle.get(),
							 L"DumpType",
							 2))
	{
		log_err "RUWriteDword(HKLM, %ws, %ws) failed.",
			key_path.c_str(),
			L"DumpType"
			log_end;
		return false;
	}

	//	set option - CustomDumpFlags
	if (true != RUWriteDword(KeyHandle.get(),
							 L"CustomDumpFlags",
							 0))
	{
		log_err "RUWriteDword(HKLM, %ws, %ws) failed.",
			key_path.c_str(),
			L"CustomDumpFlags"
			log_end;
		return false;
	}

	//	������ ������ �����Ѵ�. 
	if (!WUCreateDirectory(dump_dir))
	{
		log_err "WUCreateDirectory() failed. dir=%ws",
			dump_dir
			log_end;
		return false;
	}

	DWORD out_file_size = (DWORD)((wcslen(dump_dir) + 1) * sizeof(wchar_t));
	if (true != RUSetExpandString(KeyHandle.get(),
								  L"DumpFolder",
								  dump_dir,
								  out_file_size))
	{
		log_err "RUSetExpandString(HKLM, %ws, %ws, %ws) failed.",
			key_path.c_str(),
			L"DumpFolder",
			dump_dir
			log_end;
		return false;
	}

	log_info "Reboot system to apply WER configuration." log_end;
	return true;
}

/// @brief 
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

/// @brief	 �ܼ��� �����Ѵ�.
bool attach_console(_In_ bool create)
{
	//
	//	�θ� ���μ����� �ܼ��ڵ鿡 ����õ��Ѵ�.
	//
	if (AttachConsole(ATTACH_PARENT_PROCESS))
	{
		//
		//	Console Attach �� ��� ������Ʈ�� ���κп��� �ٷ� ����� ���۵Ǿ�
		//	�Ʒ�ó�� �� ����ϱ� ������, 
		//
		//	C:\x64_release>[INFO] Monster.
		//
		//	������ \n �� �ѹ� �߰����ش�.
		//
		write_to_console(fc_none, "\n");
		return true;
	}

	if (create == true)
	{
		//
		//	�θ� ���μ����� console handle �� ���� ��� �����Ѵ�.
		//
		HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
		if (NULL == h_console)
		{
			//
			//	Console �� ����, service �Ǵ� gui �� ��� NULL �� ����
			//	

			if (!AllocConsole())
			{
				log_err "AllocConsole() failed. gle=%u",
					GetLastError()
					log_end;
				return false;
			}

			h_console = GetStdHandle(STD_OUTPUT_HANDLE);
			if (NULL == h_console)
			{
				log_err "Can not get STD_OUTPUT_HANDLE (No console allocated)."
					log_end;
				return false;
			}
			else if (INVALID_HANDLE_VALUE == h_console)
			{
				log_err "GetStdHandle() failed. gle=%u",
					GetLastError()
					log_end;
				return false;
			}
		}
		else if (INVALID_HANDLE_VALUE == h_console)
		{
			log_err "GetStdHandle() failed. gle=%u",
				GetLastError()
				log_end;
			return false;
		}
	}
	
	return true;
}

void destroy_console()
{
	FreeConsole();
}

void write_to_console(_In_ console_font_color color, _In_z_ const char* log_message)
{
	_ASSERTE(NULL != log_message);
	if (NULL == log_message) return;

	static HANDLE	con_stdout_handle = NULL;
	static WORD		con_old_color = 0;

	if (NULL == con_stdout_handle)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		con_stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (NULL == con_stdout_handle || INVALID_HANDLE_VALUE == con_stdout_handle)
		{
			//
			//	NULL : console handle �� ���� ���μ��� 
			//	INVALID_HANDLE_VALUE : GetStdHandle() ���� 
			// 
			return;
		}

		GetConsoleScreenBufferInfo(con_stdout_handle, &csbi);
		con_old_color = csbi.wAttributes;
	}

	DWORD len;
	switch (color)
	{
	case fc_red:
		SetConsoleTextAttribute(con_stdout_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
		WriteConsoleA(con_stdout_handle, log_message, (DWORD)strlen(log_message), &len, NULL);
		SetConsoleTextAttribute(con_stdout_handle, con_old_color);
		break;
	case fc_green:
		SetConsoleTextAttribute(con_stdout_handle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		WriteConsoleA(con_stdout_handle, log_message, (DWORD)strlen(log_message), &len, NULL);
		SetConsoleTextAttribute(con_stdout_handle, con_old_color);
		break;
	case fc_none:
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
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) { return; }
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

/// @brief	������ ������������ �ƴ��� Ȯ���Ѵ�. 
/// @return	������ true
IMAGE_TYPE get_image_type(_In_ const wchar_t* path)
{
	_ASSERTE(nullptr != path);
	if (nullptr == path) return IT_UNKNOWN;
	if (true != is_file_existsW(path)) return IT_UNKNOWN;

	HANDLE hFile = CreateFileW(path,
							   GENERIC_READ,
							   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							   NULL,
							   OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		//log_err "access denied or invalid path, %ws, gle = %u", path, GetLastError() log_end
		return IT_UNKNOWN;
	}
	SmrtHandle sfFile(hFile);
	return get_image_type(hFile);
}

IMAGE_TYPE get_image_type(_In_ HANDLE file_handle)
{
	// check file size 
	// 
	LARGE_INTEGER fileSize;
	if (TRUE != GetFileSizeEx(file_handle, &fileSize))
	{
		log_err "can not get file size, errorcode=0x%08x",
			GetLastError()
			log_end;
		return IT_UNKNOWN;
	}
	if (sizeof(IMAGE_DOS_HEADER) > fileSize.QuadPart)
	{
		//
		//	Not a PE file
		//
		return IT_UNKNOWN;
	}

	HANDLE hImageMap = CreateFileMapping(file_handle,
										 NULL,
										 PAGE_READONLY,
										 0,
										 0,
										 NULL);
	if (NULL == hImageMap)
	{
		log_err "CreateFileMapping() failed. gle=%u",
			GetLastError()
			log_end;
		return IT_UNKNOWN;
	}
	SmrtHandle sfMap(hImageMap);
	PBYTE ImageView = (LPBYTE)MapViewOfFile(hImageMap,
											FILE_MAP_READ,
											0,
											0,
											0);
	if (ImageView == NULL)
	{
		log_err "MapViewOfFile() failed. gle=%u",
			GetLastError()
			log_end;
		return IT_UNKNOWN;
	}
	SmrtView sfView(ImageView);

	//
	// PE �� offset ������ �ŷ��� �� ���� ������ SEH �� �̿��Ѵ�. 
	// ������ IsBadReadPtr() �Լ����� �� �̻� ������� �ʴ´�. 
	//
	try
	{
		PIMAGE_DOS_HEADER idh = (PIMAGE_DOS_HEADER)ImageView;
		if (idh->e_magic != IMAGE_DOS_SIGNATURE)
		{
			//
			//	Not a PE file
			//
			return IT_UNKNOWN;
		}

		//
		//	PE �� ��ϵ� PE ������ ������� ���� ���� ����� ��
		// 
		DWORD dosSize = (idh->e_cp * 512);
		if (dosSize > fileSize.QuadPart)
		{
			log_dbg "(not a pe) invalid file size, size=%llu", fileSize.QuadPart log_end;
			return IT_UNKNOWN;
		}

		//
		//	IMAGE_NT_HEADER �����Ͱ� PE ���� ����(+/- ���� ���)�� �ִ��� Ȯ��
		//
		#define IMAGE_DOS_SIGNATURE_SIZE 2
		PIMAGE_NT_HEADERS inh = (PIMAGE_NT_HEADERS)((uintptr_t)idh + idh->e_lfanew);
		if ((uintptr_t)inh < ((uintptr_t)idh + IMAGE_DOS_SIGNATURE_SIZE))
		{
			log_dbg "(not a pe) invalid idh->e_lfanew (negative value)" log_end;
			return IT_UNKNOWN;
		}
		
		//
		//	IMAGE_NT_HEADER ����ü�� PE ���� ���̳��� ��� �ִ��� Ȯ��
		//
		if ( (uintptr_t)inh > (uintptr_t)idh + fileSize.QuadPart ||			
			 (uintptr_t)inh + sizeof(IMAGE_NT_HEADERS) > (uintptr_t)idh + fileSize.QuadPart )
		{
			log_dbg "(not a pe) inh is out of pe file." log_end;
			return IT_UNKNOWN;
		}

		if (IMAGE_NT_SIGNATURE != inh->Signature) return IT_UNKNOWN;;

		WORD subsys = inh->OptionalHeader.Subsystem;
		WORD characteristics = inh->FileHeader.Characteristics;

		// characteristics check
		if ((characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) != IMAGE_FILE_EXECUTABLE_IMAGE) return IT_UNKNOWN;   // not executable

		if ((characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL)
		{
			return IT_DLL;
		}

		// never called.. ???
		// if ((characteristics & IMAGE_FILE_SYSTEM) == IMAGE_FILE_SYSTEM){OutputDebugStringW(" IMAGE_FILE_SYSTEM");}

		// subsystem check
		// 
		if ((subsys & IMAGE_SUBSYSTEM_NATIVE) == IMAGE_SUBSYSTEM_NATIVE)
		{
			return IT_NATIVE_APP;
		}

		if ((subsys & IMAGE_SUBSYSTEM_WINDOWS_GUI) == IMAGE_SUBSYSTEM_WINDOWS_GUI)
		{
			return IT_EXE_GUI;
		}
		if ((subsys & IMAGE_SUBSYSTEM_WINDOWS_CUI) == IMAGE_SUBSYSTEM_WINDOWS_CUI)
		{
			return IT_EXE_CUI;
		}
		if ((subsys & IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION) == IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION)
		{
			return IT_EXE_BOOT;
		}

	}
	catch (std::exception& e)
	{
		log_warn "Invalid/Malformed pe file, exception=%s", e.what() log_end;
		return IT_UNKNOWN;
	}

	//
	//	Not a PE file
	//
	return IT_UNKNOWN;
}

/// @brief
LPCWSTR  image_type_to_string(_In_ IMAGE_TYPE type)
{
	switch (type)
	{
	case IT_DLL:
		return L"DLL";
	case IT_EXE_GUI:
		return L"GUI APP";
	case IT_EXE_CUI:
		return L"CUI APP";
	case IT_EXE_BOOT:
		return L"BOOT APP";
	case IT_NATIVE_APP:
		return L"NATIVE";
	case IT_UNKNOWN:
	default:
		return L"Unknown";
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

	char_ptr hex_buf((char*)malloc(buffer_size), [](char* p) {
		if (nullptr != p)
		{
			free(p);
		}
	});
	if (NULL == hex_buf.get()) return false;
	RtlZeroMemory(hex_buf.get(), buffer_size);

	char*	pos = hex_buf.get();
	size_t	remain = buffer_size;
	for (UINT32 i = 0; i < code_size; ++i)
	{
		HRESULT hr = StringCbPrintfExA(pos, remain, &pos, &remain, 0, "%02x", code[i]);
		if (!SUCCEEDED(hr))
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


const
char*
get_int_to_char_table(
	_In_ bool uppercase
)
{
	if (true == uppercase)
	{
		return _int_to_uchar_table;
	}
	else
	{
		return _int_to_char_table;
	}
}

bool
bin_to_hexa_fast(
	_In_ uint32_t size,
	_In_reads_bytes_(size) uint8_t* buffer,
	_In_ bool upper_case,
	_Out_ std::string& hex_string
)
{
	char* table = _int_to_char_table;
	if (true == upper_case)
	{
		table = _int_to_uchar_table;
	}

	uint32_t buffer_size = ((size * 2) + 1) * sizeof(char);
	char_ptr hex_buffer((char*)malloc(buffer_size), [](char* p) {
		if (nullptr != p)
		{
			free(p);
		}
	});

	if (nullptr == hex_buffer.get()) return false;

	for (uint32_t i = 0; i < size; ++i)
	{
		hex_buffer.get()[i * 2] = table[buffer[i] >> 4];
		hex_buffer.get()[i * 2 + 1] = table[buffer[i] & 0xf];
	}
	hex_buffer.get()[buffer_size - 1] = 0x00;

	hex_string = hex_buffer.get();
	return true;
}

bool
bin_to_hexw_fast(
	_In_ uint32_t size,
	_In_reads_bytes_(size) uint8_t* buffer,
	_In_ bool upper_case,
	_Out_ std::wstring& hex_string
)
{
	char* table = _int_to_char_table;
	if (true == upper_case)
	{
		table = _int_to_uchar_table;
	}

	uint32_t buffer_size = ((size * 2) + 1) * sizeof(wchar_t);
	wchar_ptr hex_buffer((wchar_t*)malloc(buffer_size), [](wchar_t* p) {
		if (nullptr != p)
		{
			free(p);
		}
	});
	if (nullptr == hex_buffer.get()) return false;

	for (uint32_t i = 0; i < size; ++i)
	{
		hex_buffer.get()[i * 2] = table[buffer[i] >> 4];
		hex_buffer.get()[i * 2 + 1] = table[buffer[i] & 0xf];
	}
	hex_buffer.get()[(buffer_size / sizeof(wchar_t)) - 1] = 0x00;

	hex_string = hex_buffer.get();
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

	DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
	DWORD bitSetCount = 0;
	ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
	DWORD i;

	for (i = 0; i <= LSHIFT; ++i)
	{
		bitSetCount += ((bitMask & bitTest) ? 1 : 0);
		bitTest /= 2;
	}

	return bitSetCount;
}

BOOL WUGetProcessorInfo(IN OUT WU_PROCESSOR_INFO& CpuInfo)
{
	BOOL done = FALSE;
	DWORD ReturnedLength = 0;
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
	DWORD offset = 0;
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
				CpuInfo.L3Cache++;
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
const char* get_archtecture()
{
#ifdef _WIN64
	return "x64";
#else
	CWow64Util wow;
	if (wow.IsWow64Process())
	{
		return "x64";
	}
	else
	{
		return "x86";
	}
#endif
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
	case OSV_2016: return L"Windows Server 2016";
	case OSV_UNKNOWN:
	default:
		return L"Unknown OS";
	}
}

/// @brief  RtlGetVersion() wrapper
///			https://docs.microsoft.com/ko-kr/windows-hardware/drivers/ddi/content/wdm/ns-wdm-_osversioninfoexw
OSVER get_os_version()
{
	OSVER os = OSV_UNKNOWN;

	RTL_OSVERSIONINFOEXW osv = { sizeof(osv), 0, };
	typedef uint32_t(__stdcall *fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);
	fnRtlGetVersion RtlGetVersion = (fnRtlGetVersion)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");

	if (RtlGetVersion != 0 &&
		RtlGetVersion((PRTL_OSVERSIONINFOW)&osv) == 0 &&
		osv.dwPlatformId == VER_PLATFORM_WIN32_NT)
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
			else if (osv.dwMinorVersion == 2) os = OSV_2003;
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
			if (osv.dwMinorVersion == 0)
			{
				if (osv.wProductType == VER_NT_WORKSTATION)
				{
					os = OSV_10;
				}
				else
				{
					os = OSV_2016;
				}
			}
			else
			{
				// unknown or unsupported
				break;
			}

			break;
		}
	}

	return os;
}