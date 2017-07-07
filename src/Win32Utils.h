/**
 * @file    Windows API wrapper and utility routines.
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011/08/26 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#ifndef _win32_utils_
#define _win32_utils_

#include "boost/algorithm/string.hpp"	// to_uppper, to_lower

#include <conio.h>
#include <winioctl.h>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>


//> reported as vs 2010 bug, ms says that will be patch this bug next major vs release, vs2012.
//
//1>C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\include\intsafe.h(171): warning C4005: 'INT16_MAX' : macro redefinition
//1>          C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\stdint.h(77) : see previous definition of 'INT16_MAX'
#pragma warning(disable:4005)
#include <intsafe.h>
#pragma warning(default:4005)

#include "log.h"


// | size | _______data_______ | ������ �޸� ����
// byte align ������ �Ϻη� buf[4] �� ������
// 
typedef struct _continuous_memory
{
    DWORD   size;   // buf size
    CHAR    buf[4];
} CTM, *PCTM;
#define CTM_SIZE( _data_size_ )   ((_data_size_) + sizeof(DWORD))

#define WSTRING_SIZE(_std_wstring_)	(DWORD)((_std_wstring_.size() + 1) * sizeof(WCHAR))
#define STRING_SIZE(_std_string_)   (DWORD)((_std_string_.size() + 1) * sizeof(CHAR))


#define _HIGH_PART(_int64_value_)    (UINT32)((_int64_value_) >> 32)
#define _LOW_PART(_int64_value_)     (UINT32)((_int64_value_) & 0x00000000FFFFFFFF)


/// @brief  val ������ pos ��° ��Ʈ�� 1 �̸� 1 �ƴϸ� 0
#define _check_bit(val, pos)  (val & (1 << pos))

/// @brief	from ntifs.h
#ifndef FlagOn
#define FlagOn(_F,_SF)        ((_F) & (_SF))
#endif

#ifndef BooleanFlagOn
#define BooleanFlagOn(F,SF)   ((BOOLEAN)(((F) & (SF)) != 0))
#endif

#ifndef SetFlag
#define SetFlag(_F,_SF)       ((_F) |= (_SF))
#endif

#ifndef ClearFlag
#define ClearFlag(_F,_SF)     ((_F) &= ~(_SF))
#endif

/// @brief	��� �̸�->���ڿ� ��ȯ
///			#define love 0
///			#define	you 1
///			...
///			TO_STR(love)  --> "love"
///			TO_STR(you)  --> "you"
///			TO_STRS(love,you)  --> "loveyou"
#define	TO_STR( v )	#v
#define TO_STRS(x,y) TO_STR(x)##TO_STR(y)



/**	-----------------------------------------------------------------------
	����ÿ� TODO �޼����� ����ϱ� ���� ��ũ�� 

	#pragma TODO( "��� �����ϼ�	\n" )

	�䷸�� ����ϸ� �� 
	IT EXPERT, ������ ���α׷��Ӹ� ���� MFC ������ ����, ������ ��
-------------------------------------------------------------------------*/
#ifndef _TODO_DEFINED_
	#define LINE1(x)	#x
	#define LINE(x)		LINE1(x)
	#define TODO(msg)	message(  __FILE__"(" LINE(__LINE__) ") [TODO] " msg )
	#define todo TODO
#endif

#define free_and_nil(p)     do { if (NULL != p) { free(p); p = NULL;} } while(p != NULL);

#define     _pause  _getch()


/* 
	x64 ������ inline asm �� ����� �� �����Ƿ� ȣȯ���� ���� ����
	��� StopWatch.h ���Ͽ� ���ǵ� Ŭ������ ����ϸ� ��


// out-of-order execution �� ���� ���� cpuid ȣ��
//
static __inline LARGE_INTEGER getticks(void)
{
	LARGE_INTEGER retval={0};
	__asm 
	{
		cpuid
		rdtsc
		mov dword ptr retval.HighPart, edx
		mov dword ptr retval.LowPart, eax
	}
	return retval;
}
 
*/

int get_random_int(_In_ int min, _In_ int max);


//=============================================================================
// �ð� ����
//=============================================================================

// FILETIME (1601�� 1�� 1�Ϻ��� 100-nanosecond ���� ī��Ʈ)
// 1 nano sec = 1/1,000,000,000 (1e-9) sec 
// 100 nonosecond = 1/10,000,000 (1e-7) sec
#define _file_time_to_msec  ((uint64_t) 10000)
#define _file_time_to_sec  ((uint64_t) 10000000)
#define _file_time_to_min	(_file_time_to_sec * 60)
#define _file_time_to_hour	(_file_time_to_min * 60)
#define _file_time_to_day	(_file_time_to_hour * 24)



#ifndef _FAT_TEIME_DEFINDED_
#define _FAT_TEIME_DEFINDED_
typedef struct _FATTIME
{
	USHORT		usDate;
	USHORT		usTime;
} FATTIME;

#endif//_FAT_TEIME_DEFINDED_

LPCWSTR FT2Str(IN FILETIME& ft);
LPCWSTR FAT2Str(IN FATTIME& fat);


uint64_t file_time_to_int(_In_ const PFILETIME file_time);
void int_to_file_time(_In_ uint64_t file_time_int, _Out_ PFILETIME const file_time);

int64_t file_time_delta_sec(_In_ const PFILETIME ftl, _In_ const PFILETIME ftr);
int64_t file_time_delta_day(_In_ const PFILETIME ftl, _In_ const PFILETIME ftr);
FILETIME add_day_to_file_time(_In_ const PFILETIME file_time, _In_ int32_t day);

std::string file_time_to_str(_In_ const PFILETIME file_time, _In_ bool localtime, _In_ bool show_misec = false);
std::string file_time_to_str(_In_ uint64_t file_time, _In_ bool localtime, _In_ bool show_misec = false);
std::string sys_time_to_str(_In_ const PSYSTEMTIME sys_time, _In_ bool localtime, _In_ bool show_misec = false);





/******************************************************************************
 * file, disk, volumes
******************************************************************************/
bool is_file_existsW(_In_ const wchar_t* file_path);
bool is_file_existsA(_In_ const char* file_path);
bool is_dir(_In_ const wchar_t* file_path);
bool is_file(_In_ const wchar_t* file_path);

bool get_filepath_by_handle(_In_ HANDLE file_handle, _Out_ std::wstring& file_name);
bool get_mapped_file_name(_In_ HANDLE process_handle, _In_ const void* mapped_addr, _Out_ std::wstring& file_name);
bool nt_name_to_dos_name(_In_ const wchar_t* nt_name, _Out_ std::wstring& dos_name);

// disk, volume, ...
bool query_dos_device(_In_ const wchar_t* dos_device, _Out_ std::wstring& nt_device);

bool get_disk_numbers(_Out_ std::vector<uint32_t>& disk_numbers);
const char* partition_style_to_str(_In_ DWORD partition_style);
const char* gpt_partition_type_to_str(_In_ GUID& partition_type);
typedef struct _vbr_info
{
    bool            is_boot_partition;
    bool            is_mbr;
    bool            recognized;
    LARGE_INTEGER   offset;
    LARGE_INTEGER   partition_length;
    uint32_t        partition_number;
    bool            rewrite_partition;
} vbr_info, *pvbr_info;

typedef class disk_volume_info
{
public:
    disk_volume_info(_In_ uint32_t disk_number) :_disk_number(disk_number) {}
    void clear() { _disk_number = 0; _vbrs.clear(); }
    
    uint32_t                _disk_number;
    // `mbr` always placed at first 512,
    std::vector<vbr_info>   _vbrs;
}*pdisk_volume_info;
bool get_disk_volume_info(_Inout_ disk_volume_info& info);

bool dump_all_disk_drive_layout();
bool dump_boot_area();

HANDLE open_file_to_write(_In_ const wchar_t* file_path);
HANDLE open_file_to_read(LPCWCH file_path);

BOOL write_to_filew(LPCWCH file_path, LPCWCH format,...);
BOOL write_to_filew(HANDLE hFile, LPCWCH format, ...);
BOOL write_to_filea(HANDLE hFile, LPCCH format, ...);

bool get_file_size(_In_ HANDLE file_handle, _Out_ int64_t& size);
bool get_file_position(_In_ HANDLE file_handle, _Out_ int64_t& position);
bool set_file_position(_In_ HANDLE file_handle, _In_ int64_t distance, _Out_opt_ int64_t* new_position);
bool set_file_size(_In_ HANDLE file_handle, _In_ int64_t new_size);


BOOL SaveToFileAsUTF8A(
                IN LPCWSTR FilePathDoesNotExists, 
                IN LPCSTR NullTerminatedAsciiString
                );
BOOL SaveToFileAsUTF8W(
                IN LPCWSTR FilePathDoesNotExists, 
                IN LPCWSTR NullTerminatedWideString
                );
BOOL LoadFileToMemory(
                IN LPCWSTR FilePath, 
                OUT DWORD& MemorySize, 
                OUT PBYTE& Memory
                );
BOOL SaveBinaryFile(
                IN LPCWSTR  Directory,
                IN LPCWSTR  FileName, 
                IN DWORD    Size,
                IN PBYTE    Data
                );

typedef bool (WINAPI *fnFindFilesCallback)(_In_ DWORD_PTR tag, _In_ const wchar_t* path);

bool
find_files(
	_In_ const wchar_t* root, 
	_In_ fnFindFilesCallback cb, 
	_In_ DWORD_PTR tag, 
	_In_ bool recursive = true
	);

BOOL 
FindSubDirectory(
	IN LPCWSTR RootPath,
	IN OUT std::vector<std::wstring>& DirectoryList, 
	IN BOOL Recursive
	);

bool file_size_padding(
	IN wchar_t* file_path,
	IN OUT int64_t &padding
);

bool aes256_encrypt(
	IN unsigned char *key,
	IN const wchar_t *path,
	IN const wchar_t *target_file_path,
	IN const wchar_t *encrypt_file_name,
	OUT unsigned char * out_encrypt
);

bool aes256_decrypt(
	IN unsigned char *key,
	IN const wchar_t *path,
	IN const wchar_t *target_file_path,
	IN const wchar_t *encrypt_file_name,
	IN const wchar_t *decrypt_file_name,
	IN  unsigned char * encrypt
);

/******************************************************************************
 * directory management 
******************************************************************************/
BOOL WUGetCurrentDirectoryW(IN OUT std::wstring& CurrentDir);
BOOL WUGetCurrentDirectoryA(IN OUT std::string& CurrentDir);

bool get_temp_dirW(_Out_ std::wstring& temp_dir);
bool get_temp_dirA(_Out_ std::string& temp_dir);

bool get_module_path(_In_ const wchar_t* module_name, _Out_ std::wstring& module_path);
bool get_current_module_path(_Out_ std::wstring& module_path);
bool get_current_module_dir(_Out_ std::wstring& module_dir);
bool get_current_module_file(_Out_ std::wstring& module_file);

std::wstring get_module_pathEx(_In_ const wchar_t* module_name);
std::wstring get_module_dirEx(_In_ const wchar_t* module_name);
std::wstring get_current_module_pathEx();
std::wstring get_current_module_dirEx();
std::wstring get_current_module_fileEx();

/// @brief  nt_name ���� device name �κи� ����� �����Ѵ�.
///
///         "\\Device\\HarddiskVolume4\\Windows"    -> "\\Device\\HarddiskVolume4"
///         "\\Device\\HarddiskVolume4"             -> "\\Device\\HarddiskVolume4"
///         "\\Device\\HarddiskVolume4\\"           -> "\\Device\\HarddiskVolume4"
///         "\\Device\\HarddiskVolume455\\xyz"      -> "\\Device\\HarddiskVolume455"
std::wstring device_name_from_nt_name(_In_ const wchar_t* nt_name);

/// @brief	full path ��θ��� `���ϸ�.Ȯ����` �κи� �����. 
std::wstring file_name_from_file_path(_In_ const wchar_t* file_path);

bool WUCreateDirectory(const LPCWSTR DirectoryPath);
bool WUDeleteDirectoryW(IN LPCWSTR  DirctoryPathToDelete);
BOOL GetImageFullPathFromPredefinedPathW(
                IN  LPCWSTR ImageName, 
                IN  DWORD   BufferLen,
                OUT LPWSTR  FullPathBuffer
                );
BOOL GetImageFullPathFromPredefinedPathA(
                IN  LPCSTR ImageName, 
                IN  DWORD  FullPathBufferLen,
                OUT LPSTR  FullPathBuffer
                );
#ifdef UNICODE
#define GetImageFullPathFromPredefinedPath  GetImageFullPathFromPredefinedPathW
#else
#define GetImageFullPathFromPredefinedPath  GetImageFullPathFromPredefinedPathA	
#endif//UNICODE

bool get_process_image_full_path(_In_ HANDLE process_handle, _Out_ std::wstring& full_path);


/// @brief  system direcotry ��� ���� (c:\windows\system32 )
bool get_system_dir(_Out_ std::wstring& system_dir);        

/// @brief  %systemroot% ��� ���� ( c:\windows )
bool get_windows_dir(_Out_ std::wstring& windows_dir);
bool get_environment_value(_In_ const wchar_t* env_variable, _Out_ std::wstring& env_value);
bool get_short_file_name(_In_ const wchar_t* long_file_name, _Out_ std::wstring& short_file_name);


/******************************************************************************
 * ���ڿ� ó��
******************************************************************************/
wchar_t* MbsToWcs(_In_ const char* mbs);
char* WcsToMbs(_In_ const wchar_t* wcs);
char* WcsToMbsUTF8(_In_ const wchar_t* wcs);
wchar_t* Utf8MbsToWcs(_In_ const char* utf8);

static const std::wstring _null_stringw(L"");
static const std::string  _null_stringa("");

std::wstring MbsToWcsEx(_In_ const char *mbs);
std::string WcsToMbsEx(_In_ const wchar_t *wcs);
std::string WcsToMbsUTF8Ex(_In_ const wchar_t *wcs);
std::wstring Utf8MbsToWcsEx(_In_ const char* utf8);

/// @brief  src �� �ڿ������� fnd ���ڿ��� ã�´�. 
///         fnd �� src �� �ǹ��Ͽ� ��Ȯ�� ��ġ�ϸ� true, �ƴϸ� false ����
///         - Ȯ���� �˻簰���� �Ҷ� ���
bool rstrnicmp(_In_ const wchar_t* src, _In_ const wchar_t* fnd);
bool rstrnicmpa(_In_ const char* src, _In_ const char* fnd);
bool lstrnicmp(_In_ const wchar_t* src, _In_ const wchar_t* fnd);
bool lstrnicmpa(_In_ const char* src, _In_ const char* fnd);

inline void clear_str_stream_w(std::wstringstream& stream)
{
	stream.str(L"");
	stream.clear();
}

inline void clear_str_stream_a(std::stringstream& stream)
{
	stream.str("");
	stream.clear();
}


//> T = std::string || std::wstring
template <typename T> void to_upper_string(_Inout_ T& input){ boost::algorithm::to_upper(input);}
template <typename T> void to_lower_string(_Inout_ T& input){ boost::algorithm::to_lower(input);}

bool 
extract_first_tokenW(
	_In_ std::wstring& org_string,
	_In_ const std::wstring& token,
	_Out_ std::wstring& out_string, 
	_In_ bool forward,
	_In_ bool delete_token
	);

std::wstring 
extract_first_tokenExW(
	_In_ const wchar_t* org,
	_In_ const wchar_t* token,	
	_In_ bool forward
	);

bool
extract_first_tokenA(
		_In_ std::string& org_string,
		_In_ const std::string& token,
		_Out_ std::string& out_string, 
		_In_ bool forward,
		_In_ bool delete_token
		);

std::string
extract_first_tokenExA(
		_In_ const char* org,
		_In_ const char* token,		
		_In_ bool forward
		);


bool 
extract_last_tokenW(
	_In_ std::wstring& org_string,
	_In_ const std::wstring& token,
	_Out_ std::wstring& out_string, 
	_In_ bool forward,
	_In_ bool delete_token
	);

std::wstring
extract_last_tokenExW(
	_In_ const wchar_t* org,
	_In_ const wchar_t* token,	
	_In_ bool forward
	);

bool 
extract_last_tokenA(
	_In_ std::string& org_string,
	_In_ const std::string& token,
	_Out_ std::string& out_string, 
	_In_ bool forward,
	_In_ bool delete_token
    );

std::string
extract_last_tokenExA(
	_In_ const char* org,
	_In_ const char* token,
	_In_ bool forward
    );


std::string  trima(std::string& s, const std::string& drop = " ");
std::string rtrima(std::string& s, const std::string& drop = " ");
std::string ltrima(std::string& s, const std::string& drop = " ");

std::wstring  trimw(std::wstring& s, const std::wstring& drop = L" ");
std::wstring rtrimw(std::wstring& s, const std::wstring& drop = L" ");
std::wstring ltrimw(std::wstring& s, const std::wstring& drop = L" ");

#ifdef UNICODE
	#define ExtractFirstToken	ExtractFirstTokenW
	#define ExtractLastToken	ExtractLastTokenW
#else
	#define ExtractFirstToken	ExtractFirstTokenA
	#define ExtractLastToken	ExtractLastTokenA
#endif//UNICODE

bool 
split_stringa(
    _In_ const char* str, 
    _In_ const char* seps, 
    _Out_ std::vector<std::string>& tokens
    );

bool
split_stringw(
    _In_ const wchar_t* str,
    _In_ const wchar_t* seps,
    _Out_ std::vector<std::wstring>& tokens
    );

// string to hash
uint32_t hash_string32(_In_ const char* s, _In_opt_ uint32_t seed = 0);
uint64_t hash_string64(_In_ const char* s, _In_opt_ uint64_t seed = 0);
uint32_t hash_string32w(_In_ const wchar_t* s, _In_opt_ uint32_t seed = 0);
uint64_t hash_string64w(_In_ const wchar_t* s, _In_opt_ uint64_t seed = 0);



/// @brief	source ���� find ���ڿ��� ��� ã�� replace ���ڿ��� �����ϰ�, ������ Ƚ���� �����Ѵ�.
///         source ���ڿ� ��ü�� ���� �����Ѵ�.
template <typename T> int		
find_and_replace_string(IN T& source, IN T& find, IN T replace)
{
    uint32_t adv = (uint32_t)replace.size();
    int count = 0;
	size_t pos = source.find(find, 0);
	while (T::npos != pos)
	{
		source.replace(pos, find.length(), replace);
        pos += adv;     // replace �� find ���ڿ��� �����ϰ� �ִ� �� �� ���ڿ��� ��� 
                        // adv ��ŭ pos �� ������ ���� ������ ���ѷ����� ������. 
                        // e.g. find = ',' replace = '\,' 
		++count;
		pos = source.find(find, pos);
	}
	return count;
}

/// @brief  source ���� find �� ã�� replace �� �����ؼ�, ���ο� ���ڿ� ��ü�� ����/�����Ѵ�.
///         ���н� _null_string_a ��ü�� �����Ѵ�.
std::string
find_and_replace_string_exa(
    _In_ const char* source,
    _In_ const char* find,
    _In_ const char* replace
    );

std::wstring
find_and_replace_string_exw(
    _In_ const wchar_t* source,
    _In_ const wchar_t* find,
    _In_ const wchar_t* replace
    );


/******************************************************************************
 * RAII (Resource Acquisition Is Initialization ), raii
******************************************************************************/
#pragma todo("boost::shared_ptr -> unique_ptr ")
/*	ex)
	raii_handle map_handle(
					CreateFileMapping(file_handle, NULL, PAGE_READONLY, 0, 1, NULL), 
					raii_CloseHandle
					);
	if (NULL == map_handle.get())
	{
		err...
	}
*/
typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type > raii_handle;
void raii_CloseHandle(_In_ HANDLE handle);

/* 
	raii_void_ptr ods_buf( malloc(ods_size), raii_free );
	if (NULL == ods_buf.get())
	{
		err...
	}
	...
	ods_buf.get();
	...


	raii_void_ptr map_ptr(
					MapViewOfFile(map_handle.get(), FILE_MAP_READ, 0, 0, 1), 
					raii_UnmapViewOfFile
					);
	if (NULL == map_ptr.get())
	{
		err...
	}

*/
typedef boost::shared_ptr< boost::remove_pointer<wchar_t*>::type > raii_wchar_ptr;
typedef boost::shared_ptr< boost::remove_pointer<char*>::type > raii_char_ptr;
typedef boost::shared_ptr< boost::remove_pointer<void*>::type > raii_void_ptr;
void raii_free(_In_ void* void_ptr);
void raii_UnmapViewOfFile(_In_ void* void_ptr);



/******************************************************************************
 * API wrapper
******************************************************************************/
std::string Win32ErrorToStringA(IN DWORD ErrorCode);
std::wstring Win32ErrorToStringW(IN DWORD ErrorCode);

BOOL	DumpMemory(DWORD Length, BYTE* Buf);
BOOL	DumpMemory(FILE* stream,DWORD Length,BYTE* Buf);
bool	dump_memory(_In_ uint64_t base_offset, _In_ unsigned char* buf, _In_ UINT32 buf_len, _Out_ std::vector<std::string>& dump);

BOOL	GetTimeStringA(OUT std::string& TimeString);
BOOL	GetTimeStringW(IN std::wstring& TimeString);

#if (NTDDI_VERSION >= NTDDI_VISTA)
std::string ipv4_to_str(_In_ uint32_t ip_netbyte_order);
std::string ipv6_to_str(_In_ uint64_t ip_netbyte_order);

std::string ipv4_to_str(_In_ in_addr& ipv4);
std::string ipv6_to_str(_In_ in6_addr& ipv6);

bool str_to_ipv4(_In_ const wchar_t* ipv4, _Out_ in_addr& ipv4_addr);
bool str_to_ipv6(_In_ const wchar_t* ipv6, _Out_ in6_addr& ipv6_addr);
#endif

bool    get_ip_by_hostname(_In_ const wchar_t* host_name, _Out_ std::wstring& ip_string);
bool	get_local_ip_list(_Out_ std::wstring& host_name, _Out_ std::vector<std::wstring>& ip_list);
bool    get_local_mac_by_ipv4(_In_ const wchar_t* ip_str, _Out_ std::wstring& mac_str);
 
bool	set_privilege(_In_z_ const wchar_t* privilege, _In_ bool enable);
HANDLE	privileged_open_process(_In_ DWORD pid, _In_ DWORD rights, _In_ bool raise_privilege);

bool    get_active_window_pid(_Out_ DWORD& pid, _Out_ DWORD& tid);
DWORD	get_active_console_session_id();
bool	get_session_id_by_pid(_In_ DWORD process_id, _Out_ DWORD& session_id);
bool	process_in_console_session(_In_ DWORD process_id);

bool	create_process_as_login_user(_In_ uint32_t session_id, _In_ const wchar_t* cmdline, _Out_ PROCESS_INFORMATION& pi);

bool set_security_attributes(_Out_ SECURITY_ATTRIBUTES& sa);

#if _WIN32_WINNT >= 0x0600	// after vista
std::wstring get_process_name_by_pid(_In_ DWORD process_id);
#endif
/******************************************************************************
 * console stuff
******************************************************************************/
/*
	int main()
    {
        COORD curXY;
        int i;
        printf("This number will change: ");

        curXY = GetCurCoords();
        for(i = 0; i < 10; i++)
        {
            printf("%d", i);
            Sleep(500);
            SetConsoleCoords(curXY);
        }
        return 0;
    }
*/
void SetConsoleCoords(COORD xy);
COORD GetCurCoords(void);

#define wtc_none    0
#define wtc_red     1
#define wtc_green   2
void write_to_console(_In_ uint32_t color, _In_z_ const char* log_message);
void clear_console();



/******************************************************************************
 * PE stuff
******************************************************************************/
/* 
    cfix_user.dll= IMAGE_FILE_DLL IMAGE_SUBSYSTEM_WINDOWS_GUI     
    C:\WINDOWS\$NtUninstallKB973869$\dhtmled.ocx= IMAGE_FILE_DLL IMAGE_SUBSYSTEM_WINDOWS_GUI 
    c:\windows\system32\kernel32.dll= IMAGE_FILE_DLL IMAGE_SUBSYSTEM_NATIVE IMAGE_SUBSYSTEM_WINDOWS_GUI IMAGE_SUBSYSTEM_WINDOWS_CUI 

    c:\windows\system32\notepad.exe= IMAGE_SUBSYSTEM_WINDOWS_GUI 

    c:\windows\system32\ntoskrnl.exe= IMAGE_SUBSYSTEM_NATIVE 
    c:\windows\system32\win32k.sys= IMAGE_SUBSYSTEM_NATIVE 
    c:\windows\system32\csrss.exe= IMAGE_SUBSYSTEM_NATIVE 
    c:\windows\system32\smss.exe= IMAGE_SUBSYSTEM_NATIVE 

    c:\windows\system32\winlogon.exe= IMAGE_SUBSYSTEM_WINDOWS_GUI 
    c:\windows\system32\services.exe= IMAGE_SUBSYSTEM_WINDOWS_GUI 
    
*/
typedef enum _IMAGE_TYPE 
{
    IT_DLL,                     // dll, ocx file    
    IT_EXE_GUI,                 // gui app
    IT_EXE_CUI,                 // cui app
    IT_NATIVE_APP,              // ntoskrnl.exe, win32k.sys, csrss.exe
    IT_NORMAL                   // unknown or not executable or invalid image
} IMAGE_TYPE;

bool    is_executable_file_w(_In_ const wchar_t* path, _Out_ IMAGE_TYPE& type);
LPCWSTR  FileTypeToString(IMAGE_TYPE type);


/******************************************************************************
 * type cast
******************************************************************************/
bool 
bin_to_hexa(
	_In_ UINT32 code_size, 
	_In_ const UINT8* code, 
	_In_ bool upper_case, 
	_Out_ std::string& hex_string
	);

bool 
bin_to_hexw(
	_In_ UINT32 code_size, 
	_In_ const UINT8* code, 
	_In_ bool upper_case, 
	_Out_ std::wstring& hex_string
	);

bool str_to_int32(_In_ const char* int32_string, _Out_ int32_t& int32_val);
bool str_to_uint32(_In_ const char* uint32_string, _Out_ uint32_t& uint32_val);
bool str_to_int64(_In_ const char* int64_string, _Out_ int64_t& int64_val);
bool str_to_uint64(_In_ const char* uint64_string, _Out_ uint64_t& uint64_val);
    

bool wstr_to_int32(_In_ const wchar_t* int32_string, _Out_ int32_t& int32_val);
bool wstr_to_uint32(_In_ const wchar_t* uint32_string, _Out_ uint32_t& uint32_val);
bool wstr_to_int64(_In_ const wchar_t* int64_string, _Out_ int64_t& int64_val);
bool wstr_to_uint64(_In_ const wchar_t* uint64_string, _Out_ uint64_t& uint64_val);

uint16_t swap_endian_16(_In_ uint16_t value);
uint32_t swap_endian_32(_In_ uint32_t value);
uint64_t swap_endian_64(_In_ uint64_t value);

typedef struct WU_PROCESSOR_INFO
{
    DWORD   NumaNodeCount;
    DWORD   PhysicalCpuPackageCount;
    DWORD   PhysicalCpuCoreCount;
    DWORD   LogicalCpuCount;

    DWORD   L1Cache;
    DWORD   L2Cache;
    DWORD   L3Cache;
} *PWU_PROCESSOR_INFO;

BOOL WUGetProcessorInfo(IN OUT WU_PROCESSOR_INFO& CpuInfo);


//
// os version
// 
// widows 10 ���� (MDSN ���� �϶�´��) VersionHelpers.h �� �ִ� IsWindows10OrGreater() �Լ��� 
// ȣ���ص� false �� ������
// ���� ���⵵ �����ϰ�, ȿ���� �������� RtlGetVersion() wrapper �� �����
// 
// from https://indidev.net/forum/viewtopic.php?f=5&t=474 
// 

enum OSVER
{
    // Operating System Version 
    //  + https://msdn.microsoft.com/en-us/library/windows/desktop/ms724832(v=vs.85).aspx)

    OSV_UNDEF,
    OSV_2000,

    OSV_XP,
    OSV_XPSP1,      
    OSV_XPSP2,      
    OSV_XPSP3,      

    OSV_2003,
    OSV_VISTA, 
    OSV_VISTASP1, 
    OSV_VISTASP2,
    OSV_2008,           // vista server

    OSV_7, 
    OSV_7SP1,
    OSV_2008R2,         // win7 server

    OSV_8, 
    OSV_81,
    OSV_2012R2,         // win 8 server

    OSV_10,
    OSV_UNKNOWN,
};

const wchar_t*  osver_to_str(_In_ OSVER os);
OSVER get_os_version();

#endif//_win32_utils_