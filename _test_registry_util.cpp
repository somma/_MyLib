/**----------------------------------------------------------------------------
 * _test_registry_util.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:9:24 21:10 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "RegistryUtil.h"

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool test_registry_util()
{
	// Create key	
	HKEY key = RUCreateKey(HKEY_CURRENT_USER, L"_test_key", false);
	if (NULL == key) return false;
	RegCloseKey(key);
	
	// key created ?
	if (true != RUIsKeyExists(HKEY_CURRENT_USER, L"_test_key")) return false;

	// write dword value 
	key = RUOpenKey(HKEY_CURRENT_USER, L"_test_key", false);
	_ASSERTE(nullptr != key);
	_ASSERTE(true == RUWriteDword(key, L"TestValue", 1000));

	// write string 
	_ASSERTE(true == RUSetString(key, L"TestValueString", L"abc"));
		
	
	// read dword value 
	_ASSERTE(1000 == RUReadDword(key, L"TestValue", 0));

	std::wstring str;
	_ASSERTE(true == RUReadString(key, L"TestValueString", str));
	_ASSERTE(0 == str.compare(L"abc"));
	
	// Delete DWORD Value
	_ASSERTE(true == RUDeleteValue(key, L"TestValue"));

	// Delete key
	// -- key 내부의 value 들도 함께 삭제됨
	_ASSERTE(true == RUDeleteKey(HKEY_CURRENT_USER, L"_test_key"));
	    
	return true;
}

bool reg_key_callback(
    _In_ uint32_t index,
    _In_ const wchar_t* sub_key_name,
    _In_ const wchar_t* class_name
    )
{
    log_dbg
        "index = %u, sub_key_name = %ws, class_name = %ws",
        index, sub_key_name, class_name
        log_end;
    return true;
}

bool reg_value_callback(
    _In_ uint32_t index,
    _In_ uint32_t value_type,
    _In_ const wchar_t* value_name,
    _In_ uint32_t value_data_size,
    _In_ const uint8_t* value_data
    )
{
    UNREFERENCED_PARAMETER(index);
    static std::wstring token_name = L"\\dosdevices\\";
    static std::wstring token_data = L"_??_usbstor#disk&";
    
    
    // `\Dosdevice\N:` 형태라면 `N` 부분만 떼어낸다. 
    if (REG_BINARY != value_type) return true;
       
    wchar_t      drive_letter;
    std::wstring product;
    std::wstring serial;
    
    
    std::wstring value_name_str = value_name; to_lower_string(value_name_str);
    size_t pos = value_name_str.find(token_name);
    if (std::wstring::npos == pos) return true;

    // drive letter found.
    drive_letter = value_name_str.substr(pos + token_name.size(), value_name_str.size())[0];

    bool extracted = false;
    uint8_t* buf = (uint8_t*)malloc(value_data_size + sizeof(wchar_t));
    do
    {   
        //
        // data 영역의 시그니처가 `_??_usbstor#disk&` 이면 usb 스토리지.
        // product, serial 정보를 추출한다. 
        // 

        if (NULL == buf) break;

        // 대소문자 구분 없이 비교하는게 귀찮아서 string 객체로변경후 처리한다. 

        RtlCopyMemory(buf, value_data, value_data_size);
        *(uint16_t*)&buf[value_data_size] = 0x00;       // null terminate

        std::wstring data_str = (wchar_t*)buf;
        to_lower_string(data_str);

        pos = data_str.find(token_data);
        if (std::wstring::npos == pos) break;

        /*
        _??_usbstor#disk&ven_sandisk&prod_extremepro&rev_0001#aa010811151147340284&0#{53f56307-b6bf-11d0-94f2-00a0c91efb8b}

        [0] _??_usbstor
        [1] disk
        [2] ven_sandisk
        [3] prod_extremepro         <= product
        [4] rev_0001
        [5] aa010811151147340284    <= serial
        [6] 0
        [7] {53f56307-b6bf-11d0-94f2-00a0c91efb8b}
        */

        std::vector<std::wstring> tokens;
        if (true != split_stringw(data_str.c_str(), L"#&", tokens)) break;
        if (tokens.size() < 6) break;

        // product, serial 정보 추출 완료
        product = tokens[3];
        serial = tokens[5];

        extracted = true;
    } while (false);
    free_and_nil(buf);
    
    if (true == extracted)
    {
        log_info 
            "drive letter = %wc, usb product = %ws, usb serial = %ws", 
            drive_letter, product.c_str(), serial.c_str() 
            log_end;
    }

    return true;
}

bool test_read_mouted_device()
{
    // HKEY_LOCAL_MACHINE\SYSTEM\MountedDevices 키 오픈
    HKEY key = RUOpenKey(HKEY_LOCAL_MACHINE, L"SYSTEM\\MountedDevices", true);
    if (NULL == key)
    {
        log_err "RUOpenKey( HKLM\\SYSTEM\\MountedDevices ) failed." log_end;
        return false;
    }
    
    reg_enum_key_values(key, reg_key_callback, reg_value_callback);
    RegCloseKey(key);
    return true;
}

bool test_set_binary_data()
{
    uint8_t value[] = { 0x00,0x01,0x02,0x03,0x04,0x00,0x01,0x02,0x03,0x04,0x00,0x01,0x02,0x03,0x04 };
                        
	HKEY key = RUOpenKey(HKEY_CURRENT_USER, L"Environment", false);
	_ASSERTE(nullptr != key);
	_ASSERTE(true == RUSetBinaryData(key, L"ExtToReg", (uint8_t*)&value, sizeof(value)));
	RUCloseKey(key);

    return true;
}




