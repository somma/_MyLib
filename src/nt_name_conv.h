/**
 * @file    nt path name -> win32 name converter
 * @brief   
 * @ref     https://msdn.microsoft.com/ko-kr/library/windows/desktop/aa385453(v=vs.85).aspx
 *          https://msdn.microsoft.com/windows/hardware/drivers/ifs/support-for-unc-naming-and-mup
 *          http://pdh11.blogspot.kr/2009/05/pathcanonicalize-versus-what-it-says-on.html
 *          https://github.com/processhacker2/processhacker2/blob/master/phlib/native.c
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2016.06.14 15:36 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once
#include "Win32Utils.h"
#include "RegistryUtil.h"
#include <Winnetwk.h>
#pragma comment(lib, "Mpr.lib")

#define _mup_path		L"\\Device\\Mup"
#define _device_path	L"\\Device\\"

class DosDeviceInfo
{
public:
    DosDeviceInfo(_In_ const wchar_t* logical_drive,
                  _In_ const wchar_t* device_name,
                  _In_ uint32_t drive_type,
                  _In_ const wchar_t* network_name) :
        _logical_drive(logical_drive),
        _device_name(device_name),
        _drive_type(drive_type)
    {
        if (NULL == network_name)
            _network_name = L"";
        else
            _network_name = network_name;
    }
	
    std::wstring    _logical_drive; // `c:`, `d:`, ...
    std::wstring    _device_name;   // `\device\harddiskvolume1\`, ... 
    uint32_t        _drive_type;    // DRIVE_UNKNOWN(0), 
                                    // DRIVE_NO_ROOT_DIR(1),
                                    // DRIVE_REMOVABLE(2)
                                    // DRIVE_FIXED(3), 
                                    // DRIVE_REMOTE(4),
                                    // DRIVE_CDROM(5),
                                    // DRIVE_RAMDISK(6)
    std::wstring    _network_name;  // _drive_type == DRIVE_REMOTE 인 경우 네트워크 리소스 이름
                                    // e.x) \\192.168.153.129\share\

};

/// @brief  
typedef class NameConverter
{
public:
    NameConverter() {}
    ~NameConverter() {}

    bool get_canon_name(_In_ const wchar_t* file_name, _Out_ std::wstring& canonical_file_name);
	bool get_nt_path_by_dos_path(_In_ const wchar_t* dos_path, 
								 _Out_ std::wstring& nt_device_path);
	bool get_device_name_by_drive_letter(_In_ const wchar_t* drive_letter,
										 _Out_ std::wstring& device_name);
	bool is_removable_drive(_In_ const wchar_t* nt_name);
	bool is_natwork_path(_In_ const wchar_t* nt_name);
    bool reload();	
private:
    boost::mutex                _lock;
    std::list<DosDeviceInfo>    _dos_devices;
    std::list<std::wstring>     _mup_devices;

    bool resolve_device_prefix(_In_ const wchar_t* file_name, _Out_ std::wstring& resolved_file_name);
    bool update_dos_device_prefixes();
    bool update_mup_device_prefixes();
} *PNameConverter;