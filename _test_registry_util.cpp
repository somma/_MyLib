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
#include "_MyLib/src/RegistryUtil.h"
#include "_MyLib/src/log.h"

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

	
	uint64_t test64 = 1234567890;
	uint64_t ret_test64 = 0;
	// Write QWORD Value
	_ASSERTE(true == RUWriteQword(key, L"test_Qword", test64));
	// Read QWORD Value
	_ASSERTE(test64 == RUReadQword(key, L"test_Qword", 0));

	// Delete key
	// -- key 내부의 value 들도 함께 삭제됨
	_ASSERTE(true == RUDeleteKey(HKEY_CURRENT_USER, L"_test_key", true));
	    
	return true;
}

bool
reg_key_callback(
	_In_ uint32_t index,
	_In_ const wchar_t* base_name,
	_In_ const wchar_t* sub_key_name,
	_In_ const wchar_t* class_name,
	_In_ DWORD_PTR tag
	)
{
	UNREFERENCED_PARAMETER(base_name);
	UNREFERENCED_PARAMETER(tag);

	log_dbg
		"index = %u, sub_key_name = %ws, class_name = %ws",
		index, sub_key_name, class_name
		log_end;
	return true;
}

bool
reg_value_callback(
	_In_ uint32_t index,
	_In_ uint32_t value_type,
	_In_ const wchar_t* value_name,
	_In_ uint32_t value_data_size,
	_In_ const uint8_t* value_data,
	_In_ DWORD_PTR tag
	)
{
	UNREFERENCED_PARAMETER(index);
	UNREFERENCED_PARAMETER(value_type);
	UNREFERENCED_PARAMETER(value_data_size);
	UNREFERENCED_PARAMETER(value_data);
	UNREFERENCED_PARAMETER(tag);

	log_dbg
		"value name=%ws",
		value_name
		log_end;

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
    
    reg_enum_key_values(key,
						nullptr,
						reg_key_callback,
						NULL,
						reg_value_callback,
						NULL);
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

bool test_reg_multi_value()
{
	_mem_check_begin
	{
		// Create key	
		hkey_ptr reg_key(RUCreateKey(HKEY_CURRENT_USER,
									 L"test_key",
									 false),
						 [](HKEY key) {
			if (nullptr != key)
			{
				RUCloseKey(key);
			}
		});

		_ASSERTE(nullptr != reg_key.get());

		std::vector<std::wstring> set_vec;
		set_vec.push_back(L"Hello");
		set_vec.push_back(L"World");
		_ASSERTE(true == RUSetMultiString(reg_key.get(), L"TestValue", set_vec));

		std::vector<std::wstring> read_vec;
		_ASSERTE(true == RUReadMultiString(reg_key.get(), L"TestValue", read_vec));

		for (DWORD i = 0; i < set_vec.size(); ++i)
		{
			_ASSERTE(0 == read_vec.at(i).compare(set_vec.at(i).c_str()));
		}

		// Delete key
		// -- key 내부의 value 들도 함께 삭제됨
		_ASSERTE(true == RUDeleteKey(HKEY_CURRENT_USER, L"test_key", true));
	}
	_mem_check_end;

	return true;
}


