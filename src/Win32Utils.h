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

#include <conio.h>
#include "BaseWindowsHeader.h"
#include "boost/algorithm/string.hpp"	// to_uppper, to_lower
#include "log.h"

//
//  _pointer �� _alignment �ٿ������ �ִ��� Ȯ�� (fltKernel.h �� ���ǵǾ�����)
//	
#define IS_ALIGNED(_pointer, _alignment) \
	((((ULONG_PTR) (_pointer)) & ((_alignment) - 1)) == 0)


// | size | _______data_______ | ������ �޸� ����
typedef struct _continuous_memory
{
    DWORD   size;   // buf size
    CHAR    buf[1];
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

#define ADD_OFFSET(ptr, offset) (void*)((uintptr_t)ptr + (uintptr_t)offset)
#define SUB_OFFSET(ptr, offset) (void*)((uintptr_t)ptr - (uintptr_t)offset)


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

#define free_and_nil(p)	do{if (nullptr != p) { free(p); p = nullptr;} } while(false);

#define _pause  _getch()

#define _mem_check_begin \
	_CrtMemState memoryState = { 0 }; \
	_CrtMemCheckpoint(&memoryState); 

#define _mem_check_break(ord) \
	_CrtSetBreakAlloc(ord);

#define _mem_check_end \
	_CrtMemDumpAllObjectsSince(&memoryState)


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

std::string FAT2Str(IN FATTIME& fat);

__inline uint64_t file_time_to_int(_In_ const PFILETIME file_time);
__inline void int_to_file_time(_In_ uint64_t file_time_int, _Out_ PFILETIME const file_time);
__inline void large_int_to_file_time(_In_ const PLARGE_INTEGER large_int, _Out_ PFILETIME const file_time);

void unixtime_to_filetime(_In_ uint32_t unix_time, _Out_ PFILETIME const file_time);

int64_t file_time_delta_msec(_In_ const PFILETIME ftl, _In_ const PFILETIME ftr);
int64_t file_time_delta_sec(_In_ const PFILETIME ftl, _In_ const PFILETIME ftr);
int64_t file_time_delta_day(_In_ const PFILETIME ftl, _In_ const PFILETIME ftr);
FILETIME add_sec_to_file_time(_In_ const PFILETIME file_time, _In_ int32_t secs);
FILETIME add_day_to_file_time(_In_ const PFILETIME file_time, _In_ int32_t day);



std::string	time_now_to_str(_In_ bool localtime, _In_ bool show_misec);
std::string	time_now_to_str2();

std::string file_time_to_str(_In_ const PFILETIME file_time, _In_ bool localtime, _In_ bool show_misec = false);
std::string file_time_to_str(_In_ uint64_t file_time, _In_ bool localtime, _In_ bool show_misec = false);

std::string sys_time_to_str(_In_ const PSYSTEMTIME utc_sys_time, _In_ bool localtime, _In_ bool show_misec = false);
std::string sys_time_to_str2(_In_ const PSYSTEMTIME utc_sys_time);



/******************************************************************************
 * file, disk, volumes
******************************************************************************/
bool is_file_existsW(_In_ std::wstring& file_path);
bool is_file_existsA(_In_ std::string& file_path);
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
bool get_file_version(_In_ const wchar_t* file_path, _Out_ std::wstring& file_version);
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
bool
LoadFileToMemory(
	_In_ const LPCWSTR  FilePath,
	_Out_ DWORD&  MemorySize,
	_Outptr_ PBYTE&  Memory
	);

bool
SaveBinaryFile(
	_In_ const LPCWSTR  Directory,
	_In_ const LPCWSTR  FileName,
	_In_ DWORD    Size,
	_In_ PBYTE    Data
	);

bool 
get_file_hash_by_filepath(
	_In_ const wchar_t* file_path,
	_Out_opt_ const std::string* md5,
	_Out_opt_ const std::string* sha2
);

bool 
get_file_hash_by_filehandle(
	_In_ HANDLE file_handle,
	_Out_opt_ const std::string* md5,
	_Out_opt_ const std::string* sha2
);


/// �ݹ� ��� ���ٸ� ����� �� ����
//	if (true != find_files(root,
//							(DWORD_PTR)&file_list,
//							true, 
//						    [](_In_ DWORD_PTR tag, _In_ const wchar_t* path)->bool
//   					    {
//						       std::list<std::wstring>* files = (std::list<std::wstring>*)(tag);
//							   files->push_back(path);
//							   return true;
//							}))
//	{
//		// error
//	}
//	else
//	{
//		// success
//	}
//
typedef boost::function<
	bool (_In_ DWORD_PTR tag, _In_ const wchar_t* path)
> fnFindFilesCallback;

bool
find_files(
	_In_ const wchar_t* root, 	
	_In_ DWORD_PTR tag, 
	_In_ bool recursive, 
	_In_ fnFindFilesCallback cb
	);

BOOL 
FindSubDirectory(
	IN LPCWSTR RootPath,
	IN OUT std::vector<std::wstring>& DirectoryList, 
	IN BOOL Recursive
	);

/******************************************************************************
 * directory management 
******************************************************************************/
bool WUGetCurrentDirectoryW(IN OUT std::wstring& CurrentDir);
bool WUGetCurrentDirectoryA(IN OUT std::string& CurrentDir);

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

/// @brief	full path ��θ��� `���ϸ�.Ȯ����[:ADS]` �κи� �����. 
std::wstring file_name_from_file_pathw(_In_ const wchar_t* file_path);
std::string file_name_from_file_patha(_In_ const char* file_path);


/// @brief	full path ��θ��� `���ϸ�.Ȯ����` �� ������ ���丮 �κи� �����. 
std::wstring directory_from_file_pathw(_In_ const wchar_t* file_path);
std::string directory_from_file_patha(_In_ const char* file_path);


bool WUCreateDirectory(_In_ std::wstring& DirectoryPath);
bool WUDeleteDirectoryW(_In_ std::wstring& DirctoryPathToDelete);
bool WUCreateDirectory(_In_ const wchar_t* DirectoryPath);
bool WUDeleteDirectoryW(_In_ const wchar_t* DirctoryPathToDelete);
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

#if _WIN32_WINNT >= 0x0600	// after vista
bool 
image_path_by_pid(
	_In_ DWORD process_id, 
	_In_ bool win32_format, 
	_Out_ std::wstring& image_path
	);
#endif

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
bool rstrnicmp(_In_ const wchar_t* src, _In_ const wchar_t* fnd, _In_ bool case_insensitive = true);
bool rstrnicmpa(_In_ const char* src, _In_ const char* fnd, _In_ bool case_insensitive = true);
bool lstrnicmp(_In_ const wchar_t* src, _In_ const wchar_t* fnd, _In_ bool case_insensitive = true);
bool lstrnicmpa(_In_ const char* src, _In_ const char* fnd, _In_ bool case_insensitive = true);

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

bool get_file_extensionw(_In_ const wchar_t* file_path, _Out_ std::wstring& ext);

std::string  trima(std::string& s, const std::string& drop = " ");
std::string rtrima(std::string& s, const std::string& drop = " ");
std::string ltrima(std::string& s, const std::string& drop = " ");

std::wstring  trimw(std::wstring& s, const std::wstring& drop = L" ");
std::wstring rtrimw(std::wstring& s, const std::wstring& drop = L" ");
std::wstring ltrimw(std::wstring& s, const std::wstring& drop = L" ");



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
typedef std::unique_ptr<char, void(*)(char*)> char_ptr;
typedef std::unique_ptr<wchar_t, void(*)(wchar_t*)> wchar_ptr;
typedef std::unique_ptr<std::remove_pointer<HANDLE>::type, void(*)(HANDLE)> handle_ptr;
typedef std::unique_ptr<std::remove_pointer<void*>::type, void(*)(void*)> void_ptr;
typedef std::unique_ptr<std::remove_pointer<HKEY>::type, void(*)(HKEY)> hkey_ptr;
typedef std::unique_ptr<std::remove_pointer<SC_HANDLE>::type, void(*)(SC_HANDLE)> schandle_ptr;


/******************************************************************************
 * API wrapper
******************************************************************************/

bool create_guid(_Out_ GUID& guid);
bool create_guid(_Out_ std::string& guid);
bool create_guid(_Out_ std::wstring& guid);
bool string_to_guid(_In_ const char* guid_string, _Out_ GUID& guid);
bool wstring_to_guid(_In_ const wchar_t* guid_string, _Out_ GUID& guid);
std::string guid_to_string(_In_ GUID& guid);
std::wstring guid_to_stringw(_In_ GUID& guid);


std::string Win32ErrorToStringA(IN DWORD ErrorCode);
std::wstring Win32ErrorToStringW(IN DWORD ErrorCode);

BOOL	DumpMemory(DWORD Length, BYTE* Buf);
BOOL	DumpMemory(FILE* stream,DWORD Length,BYTE* Buf);
bool	dump_memory(_In_ uint64_t base_offset, _In_ unsigned char* buf, _In_ UINT32 buf_len, _Out_ std::vector<std::string>& dump);

BOOL	GetTimeStringA(OUT std::string& TimeString);
BOOL	GetTimeStringW(IN std::wstring& TimeString);

bool	set_privilege(_In_z_ const wchar_t* privilege, _In_ bool enable);

bool    get_active_window_pid(_Out_ DWORD& pid, _Out_ DWORD& tid);
DWORD	get_active_console_session_id();
bool	get_session_id_by_pid(_In_ DWORD process_id, _Out_ DWORD& session_id);
bool	process_in_console_session(_In_ DWORD process_id);

bool create_process(_In_ const wchar_t* cmdline, _In_ DWORD creation_flag, _In_opt_z_ const wchar_t* current_dir, _Out_opt_ HANDLE& process_handle, _Out_opt_ DWORD& process_id);
bool create_process_and_wait(_In_ const wchar_t* cmdline, _In_ DWORD creation_flag, _In_opt_z_ const wchar_t* current_dir, _In_ DWORD timeout_secs, _Out_ DWORD& exit_code);
bool create_process_as_login_user(_In_ uint32_t session_id, _In_ const wchar_t* cmdline, _Out_ PROCESS_INFORMATION& pi);

bool set_security_attributes_type1(_Out_ SECURITY_ATTRIBUTES& sa);
bool set_security_attributes_type2(_Out_ SECURITY_ATTRIBUTES& sa);

bool suspend_process_by_pid(_In_ DWORD pid);
bool resume_process_by_pid(_In_ DWORD pid);
bool terminate_process_by_pid(_In_ DWORD pid, _In_ DWORD exit_code);
bool suspend_process_by_handle(_In_ HANDLE handle);
bool resume_process_by_handle(_In_ HANDLE handle);
bool terminate_process_by_handle(_In_ HANDLE handle, _In_ DWORD exit_code);

bool get_process_creation_time(_In_ DWORD pid, _Out_ PFILETIME const creation_time);
bool get_process_creation_time(_In_ HANDLE process_handle, _Out_ PFILETIME const creation_time);

/// @brief NtCreateFile `CreationDisposition`���ڿ��� �����Ѵ�.
void dump_file_create_disposition(_In_ uint32_t NtCreateFile_CreateDisposition);
/// @brief NtCreateFile `CreateOptions`�� ���ڿ��� �����Ѵ�.
void dump_file_create_options(_In_ uint32_t NtCreateFile_CreateOptions);
/// @brief process group ���� �� `attributes`�� ���ڿ��� �����Ѵ�.
void dump_group_attributes(_In_ uint32_t group_attributes);
/// @brief process privilege ���� �� `attributes`�� ���ڿ��� �����Ѵ�.
void dump_privilege_attributes(_In_ uint32_t privilege_attributes);

/// @brief	user/group ����
///			sid		S-1-5-18, S-1-5-21-2224222141-402476733-2427282895-1001 ��
///			domain	NT AUTHORITY, DESKTOP-27HJ3RS ��
///			user_name	somma, guest ��
typedef class sid_info
{
public: 
	sid_info(_In_ const wchar_t* sid,
			 _In_ const wchar_t* domain,
			 _In_ const wchar_t* name,
			 _In_ SID_NAME_USE sid_name_use) :
		_sid(sid),
		_domain(nullptr == domain ? L"" : domain),
		_name(nullptr == name ? L"" : name),
		_sid_name_use(sid_name_use)
	{}

	sid_info(const sid_info* const cls) : 
		_sid(cls->_sid),
		_domain(cls->_domain),
		_name(cls->_name),
		_sid_name_use(cls->_sid_name_use)
	{		
	}
	
	/// �׻� ��ȿ�� ���ڿ�
	std::wstring _sid;

	/// _domain, _name �� ���� ���� ����
	std::wstring _domain;
	std::wstring _name;

	/// GROUP SID �� ��� _sid_name_use �� ��� ����
	SID_NAME_USE _sid_name_use;



} *psid_info;

psid_info get_sid_info(_In_ PSID sid);

psid_info get_process_user(_In_ DWORD pid);
psid_info get_process_user(_In_ HANDLE process_query_token);

psid_info get_file_owner(_In_ const wchar_t* file_name);

/// @brief	GROUP �� sid ������ attribute �� ���� ( TOKEN_GROUPS )
///			TOKEN_GROUPS::_sid_info::_sid_name_use �� ����
typedef class group_sid_info
{
public:
	group_sid_info(_In_ psid_info sid_info,_In_ DWORD attribute) :
		_sid_info(sid_info),
		_attribute(attribute)
	{}
	~group_sid_info()
	{
		if (nullptr != _sid_info)
		{
			delete _sid_info;
		}
	}

	group_sid_info(const group_sid_info* const cls) :
		_attribute(cls->_attribute)
	{
		_sid_info = new sid_info(cls->_sid_info);
	}

public:
	psid_info _sid_info;
	DWORD _attribute;
}*pgroup_sid_info;

bool get_process_group(_In_ DWORD pid, _Out_ std::list<pgroup_sid_info>& group);
bool get_process_group(_In_ HANDLE process_query_token, _Out_ std::list<pgroup_sid_info>& group);

typedef class privilege_info
{
public:
	privilege_info(_In_ const wchar_t* privilege_name, _In_ DWORD attribute) :
		_name(privilege_name),
		_attribute(attribute)
	{}

	privilege_info(const privilege_info* const cls):
		_name(cls->_name),
		_attribute(cls->_attribute)
	{}

public:
	std::wstring _name;
	DWORD _attribute;
} *pprivilege_info;

pprivilege_info get_privilege_info(_In_ LUID_AND_ATTRIBUTES privileges);

bool
get_process_privilege(
	_In_ DWORD pid,
	_Out_ std::list<pprivilege_info>& privileges
	);
bool
get_process_privilege(
	_In_ HANDLE process_query_token,
	_Out_ std::list<pprivilege_info>& privileges
	);

/// @brief ���μ��� integrity level�� �����´�.
bool
get_process_integrity_level(
	_In_ DWORD pid,
	_Out_ DWORD& integrity_level
	);

bool
get_process_integrity_level(
	_In_ HANDLE process_query_token,
	_Out_ DWORD& integrity_level
	);

/// @brief ���μ��� token elevation type�� �����´�.
bool
get_process_token_elevation_type(
	_In_ DWORD pid,
	_Out_ DWORD& token_elevation_type
	);

bool
get_process_token_elevation_type(
	_In_ HANDLE process_query_token,
	_Out_ DWORD& token_elevation_type
	);

/// @brief ���μ��� toekn elevation�� �����´�.
bool
get_process_token_elevated(
	_In_ DWORD pid,
	_Out_ bool& token_is_elevated
	);

bool
get_process_token_elevated(
	_In_ HANDLE process_query_token,
	_Out_ bool& token_is_elevated
	);

/// @brief ���α׷� ����
///
typedef class program
{
public:
	program(_In_ const wchar_t* id,
			_In_ const wchar_t* name,
			_In_ const wchar_t* vender,
			_In_ const wchar_t* version, 
			_In_ const wchar_t* uninstaller) :
		_id(id),
		_name(name),
		_vender(vender),
		_version(version), 
		_uninstaller(uninstaller)
	{}
public:
	const wchar_t* id() { return _id.c_str(); }
	const wchar_t* name() { return _name.c_str(); }
	const wchar_t* vendor() { return _vender.c_str(); }
	const wchar_t* version() { return _version.c_str(); }
	const wchar_t* uninstaller() { return _uninstaller.c_str(); }
private:
	std::wstring _id;
	std::wstring _name;
	std::wstring _vender;
	std::wstring _version;
	std::wstring _uninstaller;

} *pprogram;

#define sub_key_uninstall L"Software\\Microsoft\\Windows\\CurrentVersion"\
						  L"\\Uninstall\\"

#define sub_key_uninstall_x64 L"Software\\WOW6432Node\\Microsoft\\Windows\\"\
							  L"CurrentVersion\\Uninstall\\"

/// @brief `sub_key_uninstall` or `sub_key_uninstall_x64`�� ����Ű�� `value`
///        ���� �д´�. �̋�, ����Ű�� ���α׷��� Ȥ�� product code�̸� `value`
///        �� ���α׷� �������̴�.
///
pprogram
get_installed_program_info(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key_name
	);

/// @brief ��ġ�� ���α׷� ������ �о� �´�.
///
///
bool get_installed_programs(_Out_ std::list<pprogram>& installed_programs);

bool setup_wer(_In_ const wchar_t* dump_dir);


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

enum console_font_color
{
	fc_none,
	fc_red,
	fc_green
};

bool attach_console(_In_ bool create=true);
void destroy_console();
void write_to_console(_In_ console_font_color color, _In_z_ const char* log_message);
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
#define IT_UNKNOWN	0		// unknown or not executable or invalid image
#define IT_DLL		1		// dll, ocx file    
#define IT_EXE_GUI	2		// gui app
#define IT_EXE_CUI	3		// cui app
#define IT_EXE_BOOT	4		// boot app
#define IT_NATIVE_APP	5	// ntoskrnl.exe, win32k.sys, csrss.exe

typedef uint32_t IMAGE_TYPE;
IMAGE_TYPE get_image_type(_In_ const wchar_t* path);
IMAGE_TYPE get_image_type(_In_ HANDLE file_handle);
LPCWSTR  image_type_to_string(IMAGE_TYPE type);


/******************************************************************************
 * type cast
******************************************************************************/
const 
char* 
get_int_to_char_table(
	_In_ bool uppercase
	);

bool
bin_to_hexa_fast(
	_In_ uint32_t size,
	_In_reads_bytes_(size) uint8_t* buffer,
	_In_ bool upper_case,
	_Out_ std::string& hex_string
	);

bool
bin_to_hexw_fast(
	_In_ uint32_t size,
	_In_reads_bytes_(size) uint8_t* buffer,
	_In_ bool upper_case,
	_Out_ std::wstring& hex_string
	);

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

inline uint16_t swap_endian_16(_In_ uint16_t value)
{
	return (value >> 8) | (value << 8);
}

inline uint32_t swap_endian_32(_In_ uint32_t value)
{
	return	(value >> 24) |
		((value << 8) & 0x00FF0000) |
		((value >> 8) & 0x0000FF00) |
		(value << 24);
}

inline uint64_t swap_endian_64(_In_ uint64_t value)
{
	return  (value >> 56) |
		((value << 40) & 0x00FF000000000000) |
		((value << 24) & 0x0000FF0000000000) |
		((value << 8) & 0x000000FF00000000) |
		((value >> 8) & 0x00000000FF000000) |
		((value >> 24) & 0x0000000000FF0000) |
		((value >> 40) & 0x000000000000FF00) |
		(value << 56);
}

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
// widows 10 ���� (MDSN ���� �϶�´��) VersionHelpers.h �� �ִ� 
// IsWindows10OrGreater() �Լ��� ȣ���ص� false �� ������
// ���� ���⵵ �����ϰ�, ȿ���� �������� RtlGetVersion() wrapper �� �����
// 
// from https://indidev.net/forum/viewtopic.php?f=5&t=474 
// 
enum OSVER
{
    // Operating System Version 
    // https://docs.microsoft.com/ko-kr/windows-hardware/drivers/ddi/content/wdm/ns-wdm-_osversioninfoexw

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
	OSV_2016,			// windows 10 server 
    OSV_UNKNOWN,
};

const char* get_archtecture();
const wchar_t*  osver_to_str(_In_ OSVER os);
OSVER get_os_version();

#endif//_win32_utils_