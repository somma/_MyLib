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

#include <inttypes.h>
#include "boost/algorithm/string.hpp"	// to_uppper, to_lower

#include <conio.h>
#include <winioctl.h>

//> reported as vs 2010 bug, ms says that will be patch this bug next major vs release, vs2012.
//
//1>C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\include\intsafe.h(171): warning C4005: 'INT16_MAX' : macro redefinition
//1>          C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\stdint.h(77) : see previous definition of 'INT16_MAX'
#pragma warning(disable:4005)
#include <intsafe.h>
#pragma warning(default:4005)

#include "log.h"


// | size | _______data_______ | 형태의 메모리 구조
// byte align 때문에 일부러 buf[4] 로 선언함
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


/// @brief  val 변수의 pos 번째 비트가 1 이면 1 아니면 0
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

/// @brief	상수 이름->문자열 변환
///			#define love 0
///			#define	you 1
///			...
///			TO_STR(love)  --> "love"
///			TO_STR(you)  --> "you"
///			TO_STRS(love,you)  --> "loveyou"
#define	TO_STR( v )	#v
#define TO_STRS(x,y) TO_STR(x)##TO_STR(y)



/**	-----------------------------------------------------------------------
	빌드시에 TODO 메세지를 출력하기 위한 매크로 

	#pragma TODO( "요기 구현하셈	\n" )

	요렇게 사용하면 됨 
	IT EXPERT, 윈도우 프로그래머를 이한 MFC 구조와 원리, 서진택 저
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
	x64 에서는 inline asm 을 사용할 수 없으므로 호환성을 위해 제거
	대신 StopWatch.h 파일에 정의된 클래스를 사용하면 됨


// out-of-order execution 을 막기 위해 cpuid 호출
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
// 시간 관련
//=============================================================================

// FILETIME (1601년 1월 1일부터 100-nanosecond 단위 카운트)
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

uint64_t file_time_to_int(_In_ const PFILETIME file_time);
void int_to_file_time(_In_ uint64_t file_time_int, _Out_ PFILETIME const file_time);

void unixtime_to_filetime(_In_ uint32_t unix_time, _Out_ PFILETIME const file_time);

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

/// 콜백 대신 람다를 사용할 수 있음
//	if (true != find_files(root,
//						   [](_In_ DWORD_PTR tag, _In_ const wchar_t* path)->bool
//   					   {
//						       std::list<std::wstring>* files = (std::list<std::wstring>*)(tag);
//							   files->push_back(path);
//							   return true;
//							},
//							(DWORD_PTR)&file_list,
//							true))
//	{
//		// error
//	}
//	else
//	{
//		// success
//	}
//
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

/// @brief  nt_name 에서 device name 부분만 떼어내서 리턴한다.
///
///         "\\Device\\HarddiskVolume4\\Windows"    -> "\\Device\\HarddiskVolume4"
///         "\\Device\\HarddiskVolume4"             -> "\\Device\\HarddiskVolume4"
///         "\\Device\\HarddiskVolume4\\"           -> "\\Device\\HarddiskVolume4"
///         "\\Device\\HarddiskVolume455\\xyz"      -> "\\Device\\HarddiskVolume455"
std::wstring device_name_from_nt_name(_In_ const wchar_t* nt_name);

/// @brief	full path 경로명에서 `파일명.확장자[:ADS]` 부분만 떼어낸다. 
std::wstring file_name_from_file_pathw(_In_ const wchar_t* file_path);
std::string file_name_from_file_patha(_In_ const char* file_path);


/// @brief	full path 경로명에서 `파일명.확장자` 를 제외한 디렉토리 부분만 떼어낸다. 
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

/// @brief  system direcotry 경로 리턴 (c:\windows\system32 )
bool get_system_dir(_Out_ std::wstring& system_dir);        

/// @brief  %systemroot% 경로 리턴 ( c:\windows )
bool get_windows_dir(_Out_ std::wstring& windows_dir);
bool get_environment_value(_In_ const wchar_t* env_variable, _Out_ std::wstring& env_value);
bool get_short_file_name(_In_ const wchar_t* long_file_name, _Out_ std::wstring& short_file_name);


/******************************************************************************
 * 문자열 처리
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

/// @brief  src 의 뒤에서부터 fnd 문자열을 찾는다. 
///         fnd 가 src 의 꽁무니와 정확히 일치하면 true, 아니면 false 리턴
///         - 확장자 검사같은거 할때 사용
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



/// @brief	source 에서 find 문자열을 모두 찾아 replace 문자열로 변경하고, 변경한 횟수를 리턴한다.
///         source 문자열 객체를 직접 변경한다.
template <typename T> int		
find_and_replace_string(IN T& source, IN T& find, IN T replace)
{
    uint32_t adv = (uint32_t)replace.size();
    int count = 0;
	size_t pos = source.find(find, 0);
	while (T::npos != pos)
	{
		source.replace(pos, find.length(), replace);
        pos += adv;     // replace 가 find 문자열을 포함하고 있는 더 긴 문자열인 경우 
                        // adv 만큼 pos 를 보정해 주지 않으면 무한루프에 빠진다. 
                        // e.g. find = ',' replace = '\,' 
		++count;
		pos = source.find(find, pos);
	}
	return count;
}

/// @brief  source 에서 find 를 찾아 replace 로 변경해서, 새로운 문자열 객체를 생성/리턴한다.
///         실패시 _null_string_a 객체를 리턴한다.
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
bool string_to_guid(_In_ const char* guid_string, _Out_ GUID& guid);
bool guid_to_string(_In_ GUID& guid, _Out_ std::string& guid_string);
bool wstring_to_guid(_In_ const wchar_t* guid_string, _Out_ GUID& guid);
bool guid_to_wstring(_In_ GUID& guid, _Out_ std::wstring& guid_string);

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

bool	create_process_as_login_user(_In_ uint32_t session_id, _In_ const wchar_t* cmdline, _Out_ PROCESS_INFORMATION& pi);

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

/// @brief NtCreateFile `CreationDisposition`문자열로 덤프한다.
void dump_file_create_disposition(_In_ uint32_t NtCreateFile_CreateDisposition);
/// @brief NtCreateFile `CreateOptions`을 문자열로 덤프한다.
void dump_file_create_options(_In_ uint32_t NtCreateFile_CreateOptions);
/// @brief process group 정보 중 `attributes`을 문자열로 덤프한다.
void dump_group_attributes(_In_ uint32_t group_attributes);
/// @brief process privilege 정보 중 `attributes`을 문자열로 덤프한다.
void dump_privilege_attributes(_In_ uint32_t privilege_attributes);

#pragma todo("sid 관련코드-> windows_security.h 같은 거 하나 만들어서 이동시키자")

/// @brief	user/group 정보
///			sid		S-1-5-18, S-1-5-21-2224222141-402476733-2427282895-1001 등
///			domain	NT AUTHORITY, DESKTOP-27HJ3RS 등
///			user_name	somma, guest 등
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
	
	/// 항상 유효한 문자열
	std::wstring _sid;

	/// _domain, _name 은 없을 수도 있음
	std::wstring _domain;
	std::wstring _name;

	/// GROUP SID 인 경우 _sid_name_use 는 사용 안함
	SID_NAME_USE _sid_name_use;



} *psid_info;

psid_info get_sid_info(_In_ PSID sid);

psid_info get_process_user(_In_ DWORD pid);
psid_info get_process_user(_In_ HANDLE process_query_token);

psid_info get_file_owner(_In_ const wchar_t* file_name);

/// @brief	GROUP 의 sid 정보와 attribute 를 저장 ( TOKEN_GROUPS )
///			TOKEN_GROUPS::_sid_info::_sid_name_use 은 무시
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
	_Out_ std::list<pprivilege_info>& privileges);


/// @brief 프로그램 정보
///
typedef class program
{
public:
	program(_In_ const wchar_t* product_id,
			_In_ const wchar_t* product_name,
			_In_ const wchar_t* product_vendor,
			_In_ const wchar_t* product_version) :
		_product_id(product_id),
		_product_name(product_name),
		_product_vendor(product_vendor),
		_product_version(product_version)
	{}
public:
	std::wstring id() { return _product_id; }
	std::wstring name() { return _product_name; }
	std::wstring vendor() { return _product_vendor; }
	std::wstring version() { return _product_version; }
private:
	std::wstring _product_id;
	std::wstring _product_name;
	std::wstring _product_vendor;
	std::wstring _product_version;

} *pprogram;

#define sub_key_uninstall L"Software\\Microsoft\\Windows\\CurrentVersion"\
						  L"\\Uninstall\\"

#define sub_key_uninstall_x64 L"Software\\WOW6432Node\\Microsoft\\Windows\\"\
							  L"CurrentVersion\\Uninstall\\"

/// @brief `sub_key_uninstall` or `sub_key_uninstall_x64`의 서브키의 `value`
///        들을 읽는다. 이떄, 서브키는 프로그램명 혹은 product code이며 `value`
///        는 프로그램 정보들이다.
///
pprogram
get_installed_program_info(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key_name
	);

/// @brief 설치된 프로그램 정보를 읽어 온다.
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

bool create_console();
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
typedef enum _IMAGE_TYPE 
{
    IT_DLL,                     // dll, ocx file    
    IT_EXE_GUI,                 // gui app
    IT_EXE_CUI,                 // cui app
	IT_EXE_BOOT,				// boot app
    IT_NATIVE_APP,              // ntoskrnl.exe, win32k.sys, csrss.exe
    IT_NORMAL                   // unknown or not executable or invalid image
} IMAGE_TYPE;

bool    is_executable_file_w(_In_ const wchar_t* path, _Out_ IMAGE_TYPE& type);
bool	is_executable_file_w(_In_ HANDLE file_handle, _Out_ IMAGE_TYPE& type);
LPCWSTR  FileTypeToString(IMAGE_TYPE type);


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
// widows 10 에서 (MDSN 에서 하라는대로) VersionHelpers.h 에 있는 IsWindows10OrGreater() 함수를 
// 호출해도 false 가 떨어짐
// 괜히 쓰기도 복잡하고, 효율도 떨어져서 RtlGetVersion() wrapper 를 사용함
// 
// from https://indidev.net/forum/viewtopic.php?f=5&t=474 
// 

enum OSVER
{
    // Operating System Version 
    //  https://msdn.microsoft.com/en-us/library/windows/desktop/ms724832(v=vs.85).aspx

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