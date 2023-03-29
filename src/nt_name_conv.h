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
#include <mutex>
#include "_MyLib/src/Win32Utils.h"
#include "_MyLib/src/RegistryUtil.h"
#include <Winnetwk.h>
#pragma comment(lib, "Mpr.lib")

#define _mup_path		L"\\Device\\Mup"
#define _device_path	L"\\Device\\"


typedef class DosDeviceInfo
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

} *PDosDeviceInfo;

/// @brief	iterate callback, callback 함수가 false 를 리턴하면 iterate 를 중지한다. 
typedef bool(*iterate_dos_device_callback)(_In_ const DosDeviceInfo* ddi, 
										   _In_ ULONG_PTR tag);

/// @brief  
typedef class NameConverter
{
public:
	NameConverter():_loaded(false) {}
    virtual ~NameConverter() 
	{
		if (_loaded)
		{
			//
			//
			//
		}
		_dos_devices.clear();
		_mup_devices.clear();
	}

	bool load(_In_ bool force_reload);

    bool get_canon_name(_In_ const wchar_t* file_name, _Out_ std::wstring& canonical_file_name);
	bool get_nt_path_by_dos_path(_In_ const wchar_t* dos_path, _Out_ std::wstring& nt_device_path);
	bool get_device_name_by_drive_letter(_In_ const wchar_t* drive_letter, _Out_ std::wstring& device_name);
	bool get_drive_letter_by_device_name(_In_ const wchar_t* device_name, _Out_ std::wstring& drive_letter);
	
	bool is_removable_drive(_In_ const wchar_t* nt_name);
	bool is_network_path(_In_ const wchar_t* nt_name);

	bool iterate_dos_devices(_In_ iterate_dos_device_callback callback, _In_ ULONG_PTR tag);
    
private:
	bool _loaded;
    std::mutex _lock;

	//	%SystemDrive% (e.g. c: )
	//
	//	`/Program Files`, `/Windows` 형태로 drive 를 제외하고, 상대 root 경로로
	//	접근하는 경우 (예. cd \dbg 같은 경우), current drive 의 root 경로에 대한 
	//	접근이다. 그러나 current drive 를 알 수 없는 경우가 대부분이기 때문에, 
	//	정확한 변환은 어렵다. 이런 경우 그나마 가장 확률이 높은 %SystemDrive% 를 붙여준다. 
	//	_system_drive 는 항상 소문자를 사용 함
	//
	//	current path 정보는 프로세스의 PEB 를 읽으면 알아낼 수는 있으나 
	//	그렇게까지 할 필요는 없어보여서, 이정도만...
	std::wstring _system_drive;

	// %SystemRoot% (e.g. c:\windows )	
	std::wstring _system_root;

	/// 

    std::list<std::unique_ptr<DosDeviceInfo>> _dos_devices;
    std::list<std::wstring> _mup_devices;

    bool resolve_device_prefix(_In_ const wchar_t* file_name, _Out_ std::wstring& resolved_file_name);
	bool load_env_values();
    bool update_dos_device_prefixes();
    bool update_mup_device_prefixes();
} *PNameConverter;


//
//	Singleton 객체를 이용한 C api 
// 
//	nc_xxxx() API 를 사용하기 전에 반드시 nc_initialize() 를 호출하고, 
//	API 를 더이상 사용하지 않을 경우 nc_finalize() 를 호출해 주어야 한다.
// 

bool nc_initialize();
void nc_finalize();

bool nc_reload();
bool nc_get_canon_name(_In_ const wchar_t* file_name, _Out_ std::wstring& canonical_file_name);
bool nc_get_nt_path_by_dos_path(_In_ const wchar_t* dos_path, _Out_ std::wstring& nt_device_path);
bool nc_get_device_name_by_drive_letter(_In_ const wchar_t* drive_letter, _Out_ std::wstring& device_name);
bool nc_get_drive_letter_by_device_name(_In_ const wchar_t* device_name, _Out_ std::wstring& drive_letter);

bool nc_is_removable_drive(_In_ const wchar_t* nt_name);
bool nc_is_network_path(_In_ const wchar_t* nt_name);

bool nc_iterate_dos_devices(_In_ iterate_dos_device_callback callback, _In_ ULONG_PTR tag);




