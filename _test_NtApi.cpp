#include "stdafx.h"
#include <winternl.h>


typedef NTSTATUS(NTAPI *PNtCreateFile)(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );


typedef NTSTATUS(NTAPI *PNtOpenFile)(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );

typedef NTSTATUS(NTAPI *PNtClose)(
    IN HANDLE Handle
    );


typedef VOID(NTAPI *PRtlInitUnicodeString)(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );


static bool                     _initialized = false;
static PNtCreateFile            _NtCreateFile = NULL;
static PNtOpenFile              _NtOpenFile = NULL;
static PNtClose                 _NtClose = NULL;
static PRtlInitUnicodeString    _RtlInitUnicodeString = NULL;


bool init_nt_api()
{
    HMODULE nt = GetModuleHandleW(L"ntdll.dll");
    if (NULL == nt) return false;

    _NtCreateFile = (PNtCreateFile)GetProcAddress(nt, "NtCreateFile");
    _NtOpenFile = (PNtOpenFile)GetProcAddress(nt, "NtOpenFile");
    _NtClose = (PNtClose)GetProcAddress(nt, "NtClose");
    _RtlInitUnicodeString = (PRtlInitUnicodeString)GetProcAddress(nt, "RtlInitUnicodeString");
    
    if (NULL == _NtCreateFile || NULL == _NtOpenFile || NULL == _NtClose ||
        NULL == _RtlInitUnicodeString)
    {
        log_err "GetProcAddress() failed." log_end;
        return false;
    }

    _initialized = true;
    return true;
}

/// @brief  
bool test_NtCreateFile()
{
    init_nt_api();


    const wchar_t* files[] = {
        L"\\Device\\HarddiskVolume1\\Program Files (x86)\\Internet Explorer\\IEXPLORE.EXE",
        L"\\Device\\HarddiskVolume1\\Program Files\\Internet Explorer\\iexplore.exe",
        L"\\Device\\HarddiskVolume1\\Users\\vmuser\\AppData\\Roaming\\Microsoft\\Windows\\Cookies\\Low\\QEO7K0HI.txt",
        L"\\Device\\HarddiskVolume1\\Users\\vmuser\\AppData\\Local\\Microsoft\\Windows\\Temporary Internet Files\\Low\\Content.IE5\\3IIO9DKD\\BANDIZIP-SETUP-KR[1].EXE",
        L"\\Device\\HarddiskVolume1\\work._sync\\processhacker-2.36-src\\phlib\\kph.c",
        L"\\Device\\HarddiskVolume0\\work._sync\\processhacker-2.36-src\\phlib\\kph.c",
        L"\\Device\\HarddiskVolume1",
        L"\\Device\\HarddiskVolume0",
        L"\\Device\\DeviceApi"
    };


    NTSTATUS status;
    HANDLE  file_handle;
    UNICODE_STRING on;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK isb;
    

    for (int i = 0; i < sizeof(files) / sizeof(wchar_t*); ++i)
    {
        // \Device\HarddiskVolume1 µð¹ÙÀÌ½º¸¦ ¿ÀÇÂ, ÇÚµéÀ» È¹µæÇÑ´Ù. 
        _RtlInitUnicodeString(&on, files[i]);
        InitializeObjectAttributes(&oa, &on, OBJ_CASE_INSENSITIVE, NULL, NULL);
        status = _NtOpenFile(&file_handle,
							 FILE_GENERIC_READ,
							 &oa,
							 &isb,
							 FILE_SHARE_READ | FILE_SHARE_WRITE,
							 FILE_NON_DIRECTORY_FILE
		);
        if (!NT_SUCCESS(status))
        {
            log_err "NtOpenFile(%ws) failed. status=0x%08x", files[i], status log_end;
            continue;
        }

        log_info "NtOpenFile(%ws) succeeded. handle=0x%p", files[i], file_handle log_end;
        _NtClose(file_handle);
    }

    return true;
}








