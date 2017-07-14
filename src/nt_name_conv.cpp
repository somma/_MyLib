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
#include "stdafx.h"
#include "nt_name_conv.h"


/// @brief  
bool NameConverter::reload()
{
    boost::lock_guard<boost::mutex> lock(_lock);
    if (true != update_dos_device_prefixes())
    {
        log_err "update_dos_device_prefixes() failed." log_end;
        return false;
    }

    if (true != update_mup_device_prefixes())
    {
        log_err "update_mup_device_prefixes() failed." log_end;
        return false;
    }

    return true;
}

/// @brief  nt path name 을 dos path name 으로 변환한다. 
///			\??\c:\windows\system32\abc.exe->c:\windows\system32\abc.exe
///			\Systemroot\abc.exe->C:\WINDOWS\abc.exe
///			system32\abc.exe->C:\WINDOWS\system32\abc.exe
///			\windows\system32\abc.exe->C:\WINDOWS\system32\abc.exe
///			\Device\Mup\1.1.1.1\abc.exe -> \\1.1.1.1\abc.exe
///			\Device\Unknown\aaaa.exe -> \Device\Unknown\aaaa.exe
///			\device\lanmanredirector\;x:000000000008112d\192.168.153.144\sfr022\ -> \\192.168.153.144\sfr022\
///			x:\->x:\
///			\Device\Mup\192.168.153.144\sfr022\ -> \\192.168.153.144\sfr022\
///
std::wstring 
NameConverter::get_canon_name(
	_In_ const wchar_t* file_name
	)
{
    _ASSERTE(NULL != file_name);
    if (NULL == file_name) return std::wstring(L"");

    uint32_t cch_file_name = (uint32_t)wcslen(file_name);
    uint32_t cch_canon_file = 0;
    wchar_t* canon_file = NULL;
    raii_wchar_ptr wp(canon_file, raii_free);

    // "\??\" refers to \GLOBAL??\. Just remove it.
    if (true == lstrnicmp(file_name, L"\\??\\"))
    {
        cch_canon_file = cch_file_name - 4;
        canon_file = (wchar_t*)malloc((cch_canon_file + 1) * sizeof(wchar_t));
        if (NULL == canon_file) return std::wstring(file_name);

        RtlCopyMemory(canon_file, &file_name[4], (cch_canon_file * sizeof(wchar_t)));
        canon_file[cch_canon_file] = 0x0000;
        return std::wstring(canon_file);
    }

    // "\SystemRoot" means "C:\Windows".    
    else if (true == lstrnicmp(file_name, L"\\SystemRoot"))
    {
        std::wstring windows_dir;                    // c:\windows
        if (true != get_windows_dir(windows_dir))
        {
            log_err "get_windows_dir() failed." log_end;
            return std::wstring(file_name);
        }

        cch_canon_file = (cch_file_name - 11) + (uint32_t)windows_dir.size();
        canon_file = (wchar_t*)malloc((cch_canon_file + 1) * sizeof(wchar_t));
        if (NULL == canon_file) return std::wstring(file_name);

        RtlCopyMemory(canon_file, windows_dir.c_str(), (windows_dir.size() * sizeof(wchar_t)));
        RtlCopyMemory(&canon_file[windows_dir.size()], &file_name[11], (cch_file_name - 11) * sizeof(wchar_t));
        canon_file[cch_canon_file] = 0x0000;
        return std::wstring(canon_file);
    }
    // "system32\" means "C:\Windows\system32\".
    else if (true == lstrnicmp(file_name, L"system32\\"))
    {
        std::wstring windows_dir;                    // c:\windows
        if (true != get_windows_dir(windows_dir))
        {
            log_err "get_windows_dir() failed." log_end;
            return std::wstring(file_name);
        }
        windows_dir += L"\\";   // `c:\windows` => `c:\windows\\`

        cch_canon_file = cch_file_name + (uint32_t)windows_dir.size();
        canon_file = (wchar_t*)malloc((cch_canon_file + 1) * sizeof(wchar_t));
        if (NULL == canon_file) return std::wstring(file_name);

        RtlCopyMemory(canon_file, windows_dir.c_str(), (windows_dir.size() * sizeof(wchar_t)));
        RtlCopyMemory(&canon_file[windows_dir.size()], file_name, cch_file_name * sizeof(wchar_t));
        canon_file[cch_canon_file] = 0x0000;
        return std::wstring(canon_file);
    }
    else if (file_name[0] == L'\\')
    {
        std::wstring resolved_file_name;
        if (true == resolve_device_prefix(file_name, resolved_file_name))
        {
            return resolved_file_name;
        }
        else
        {
            // We didn't find a match.
            // If the file name starts with "\Windows", prepend the system drive.
            if (true == lstrnicmp(file_name, L"\\windows"))
            {
                std::wstring windows_dir;                    // c:\windows
                if (true != get_windows_dir(windows_dir))
                {
                    log_err "get_windows_dir() failed." log_end;
                    return std::wstring(file_name);
                }

                cch_canon_file = (cch_file_name - 8) + (uint32_t)windows_dir.size();
                canon_file = (wchar_t*)malloc((cch_canon_file + 1) * sizeof(wchar_t));
                if (NULL == canon_file) return std::wstring(file_name);

                RtlCopyMemory(canon_file, windows_dir.c_str(), (windows_dir.size() * sizeof(wchar_t)));
                RtlCopyMemory(&canon_file[windows_dir.size()], &file_name[8], (cch_file_name - 8) * sizeof(wchar_t));
                canon_file[cch_canon_file] = 0x0000;
                return std::wstring(canon_file);
            }
            else
            {
                // unknown
                return std::wstring(file_name);
            }
        }
    }
    else
    {
        // unknown
        return std::wstring(file_name);
    }
}

/// @brief	c:\windows\system32\test.txt -> \Device\HarddiskVolume1\windows\test.txt 로 변환
bool
NameConverter::get_nt_path_by_dos_path(
	_In_ const wchar_t* dos_path, 
	_Out_ std::wstring& nt_device_path
)
{
	_ASSERTE(dos_path);
	if (nullptr == dos_path) return false;

	// 
	// dos_path 는 최소한 `c:\` 형식이어야 한다. 
	// 
	if (3 > wcslen(dos_path) || dos_path[2] != L'\\')
	{
		log_err "Invalid dos_path format. dos_path=%ws",
			dos_path
			log_end;
		return false;
	}

	bool found = false;
	std::wstringstream out_path;
	{
		boost::lock_guard<boost::mutex> lock(_lock);
		for (auto& ddi : _dos_devices)
		{
			//
			//	`c:` 두 글자만 비교
			// 
			if (0 == _wcsnicmp(ddi._logical_drive.c_str(), dos_path, 2))
			{
				out_path << ddi._device_name;
				found = true;
				break;
			}
		}
	}
	
	if (true != found)
	{
		log_err "Can not find device name. dos_path=%ws",
			dos_path
			log_end;
		return false;
	}

	// 
	//	dos_path =                      c:\windows\system32\test.txt
	//	out_path = \device\harddiskvolume1\
	// 
	out_path << &dos_path[3];
	nt_device_path = out_path.str();
	return true;
}

/// @brief  device prefix 를 인식, 적절히 변환해서 리턴한다. 
/// @remark dos_device prefix 의 경우
///             \device\harddiskvolume1\abc.exe -> c:\abc.exe
///
///          local drive 로 매핑된 경우 (DRIVE_REMOTE 타입 드라이브인 경우)
///             x: -> \\192.168.153.144\share 
///             
///         mup_device 의 경우
///             \device\mup\1.1.1.1\share\abc.exe -> \\1.1.1.1\share\abc.exe
bool 
NameConverter::resolve_device_prefix(
	_In_ const wchar_t* file_name, 
	_Out_ std::wstring& resolved_file_name
	)
{
    _ASSERTE(NULL != file_name);
    if (NULL == file_name || file_name[0] != L'\\') return false;


    // file_name 문자열 정규화 
    //
    //  \\Device, Device 등 '\' 가 2개 미만이면 invalid input
    //  \\Device\\HarddiskVolume4    라면 \\Device\\HarddiskVolume4\\ 로 변환
    //  \\Device\\HarddiskVolume4\\, \\Device\\HarddiskVolume4\\aaa 라면 그대로 사용
    uint32_t cmp_count = 0;
    uint32_t met_count = 0;
    uint32_t max_count = (uint32_t)wcslen(file_name);
    for (cmp_count = 0; cmp_count <= max_count; ++cmp_count)
    {
        if (met_count == 3) break;
        if (file_name[cmp_count] == L'\\')
        {
            ++met_count;
        }
    }

    std::wstringstream strm;
    switch (met_count)
    {
    case 0:
    case 1:
        log_err "invalid nt_name, nt_name=%ws", file_name log_end;
        return false;
    case 2:
        // '\' 하나 더 붙여준다.            
        strm << file_name << L"\\";
        break;
    case 3:
        strm << file_name;
        break;
    }
    std::wstring qualified_file_name = strm.str();


    boost::lock_guard<boost::mutex> lock(this->_lock);

    // #1, is this a dos device?
    for (auto dos_device : _dos_devices)
    {
        // \Device\HarddiskVolume1, \Device\HarddiskVolume11 이 매칭되지 않도록
        // dos_device._device_name 필드는 `\Device\HarddiskVolume1\` 처럼 `\` 로 끝난다.
        size_t pos = qualified_file_name.find(dos_device._device_name);
        if (pos == 0)
        {
            if (DRIVE_REMOTE == dos_device._drive_type)
            {
                resolved_file_name = dos_device._network_name;
                resolved_file_name += L"\\";
                resolved_file_name += qualified_file_name.substr(dos_device._device_name.size(),
                                                                 qualified_file_name.size());

            }
            else
            {
                resolved_file_name = dos_device._logical_drive;
                resolved_file_name += L"\\";
                resolved_file_name += qualified_file_name.substr(dos_device._device_name.size(),
                                                                 qualified_file_name.size());
            }

            return true;
        }
    }

    // #2. is this a mup device? (network providers)
    for (auto mup_device : _mup_devices)
    {
        size_t pos = qualified_file_name.find(mup_device);
        if (pos == 0)
        {
            resolved_file_name = L'\\';
            resolved_file_name += qualified_file_name.substr(mup_device.size(), 
															 qualified_file_name.size());
            return true;
        }
    }

    // #3, no match    
    return false;
}

/// @brief  
bool NameConverter::update_dos_device_prefixes()
{
    this->_dos_devices.clear();

    // 시스템에 매핑되어 있는 드라이브 목록을 구한다. (c:\, d:\, ...)    
    wchar_t drive_string[128 + 1] = { 0 };
    DWORD length = GetLogicalDriveStringsW(128, drive_string);
    if (0 == length)
    {
        log_err "GetLogicalDriveStringsW(), gle = %u", GetLastError() log_end;
        return false;
    }

    // 매핑된 각 드라이브의 device name 을 구한다. 
    for (DWORD i = 0; i < length / 4; ++i)
    {
        wchar_t* dos_device_name = &(drive_string[i * 4]);
        dos_device_name[2] = 0x0000;        // `c:` 형태로 만듬
                                            // floppy 는 무시한다. 안그러면 메세지 박스 떠서 뭐 준비가 안되었느니 어쩌느니 함.
        if (dos_device_name[0] == 'A' || dos_device_name[0] == 'a') { continue; }

        std::wstring nt_device;
        if (true != query_dos_device(dos_device_name, nt_device))
        {
            log_err
                "query_dos_device( dos_device_name = %s )",
                dos_device_name
                log_end;
            return false;
        }
        nt_device += L"\\";

        // drive type 을 구한다. 
        std::wstring network_name;
        std::wstring drive_root = dos_device_name;
        drive_root += L"\\";
        uint32_t drive_type = GetDriveTypeW(drive_root.c_str());
        if (DRIVE_REMOTE == drive_type)
        {
            DWORD cch_rmt = 0;
            wchar_t* rmt = NULL;

            DWORD ret = WNetGetConnection(dos_device_name, NULL, &cch_rmt);
            if (ERROR_MORE_DATA == ret)
            {
                rmt = (wchar_t*)malloc((cch_rmt + 1) * sizeof(wchar_t));
                if (NULL != rmt)
                {
                    if (NO_ERROR == WNetGetConnection(dos_device_name, rmt, &cch_rmt))
                    {
                        network_name = rmt;
						free(rmt);
                    }
                }
            }
        }

        // _drive_table 에 등록한다. 
        DosDeviceInfo ddi(dos_device_name, nt_device.c_str(), drive_type, network_name.c_str());
        _dos_devices.push_back(ddi);
    }

    return true;
}

/// @brief  등록된 MUP device 이름들을 읽어온다. 
///         https://msdn.microsoft.com/windows/hardware/drivers/ifs/support-for-unc-naming-and-mup
bool NameConverter::update_mup_device_prefixes()
{
    // #1, HKLM\System\CurrentControlSet\Control\NetworkProvider\Order :: ProviderOrder (REG_SZ) 값을 읽는다. 
    std::wstring provider_order;
	HKEY key_handle = RUOpenKey(HKEY_LOCAL_MACHINE,
								L"System\\CurrentControlSet\\Control\\NetworkProvider\\Order",
								true);
	if (nullptr == key_handle)
	{
		log_err "RUOpenKey() failed. key=%ws",
			L"System\\CurrentControlSet\\Control\\NetworkProvider\\Order"
			log_end;
		return false;
	}
	
    if (!RUReadString(key_handle, 
					  L"ProviderOrder",
					  provider_order))
    {
        log_err "RUReadString( value=ProviderOrder ) failed." log_end;

		RUCloseKey(key_handle);
        return false;
    }
	RUCloseKey(key_handle);



    _mup_devices.push_back(L"\\Device\\Mup");       // default mup device

    // #2, provider 의 device 이름을 가져온다. 
    // 
    //  provider_order = RDPNP, LanmanWorkstation, webclient
    // 
    //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\[provider]\NetworkProvider :: DeviceName (REG_SZ)
    // 
    std::vector<std::wstring> providers;
    if (!split_stringw(provider_order.c_str(), L",", providers))
    {
        log_err "split_stringw( provider_order=%ws ) failed.", provider_order.c_str() log_end;
        return false;
    }

    for (auto provider : providers)
    {
        std::wstring device_name;
        std::wstringstream key;
        key << L"SYSTEM\\CurrentControlSet\\Services\\" << provider << L"\\NetworkProvider";

		key_handle = RUOpenKey(HKEY_LOCAL_MACHINE,
							   key.str().c_str(),
							   true);
		if (nullptr == key_handle)
		{
			log_err "RUOpenKey() failed. key=%ws",
				key.str().c_str()				
				log_end;
			continue;
		}

        if (!RUReadString(key_handle, 
						  L"DeviceName",
						  device_name))
        {
            log_err "RUReadString( DeviceName ) failed." log_end;

			RUCloseKey(key_handle);
            continue;
        }

        this->_mup_devices.push_back(device_name);
    }
	
    return true;
}
