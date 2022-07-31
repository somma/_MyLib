/**
 * @file    Windows registry api wrapper
 * @brief	
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011/11/11 created.
 * @copyright All rights reserved by Yonghwan, Roh.
 *
 *			Registry I/O 의 경우 키 또는 Value 가 없는 경우가 많아 에러로그를 다 찍으면
 *			너무 많은 불필요한 로그가 남는다. RegistryUtil 모듈 내에서는 에러로그를 남기지 않는다.
**/

#include "stdafx.h"
#include <crtdbg.h>
#include "log.h"
#include "RegistryUtil.h"
#include "CStream.h"
#include "Wow64Util.h"


/// @brief	레지스트리 키를 오픈한다. 없으면 nullptr 을 리턴한다.
/// @param	RootKey	RegCreateKey, RegOpenKey 등이 리턴한 HKEY 또는 아래 나열된 Predefined key.
///					HKEY_CLASSES_ROOT
///                 HKEY_CURRENT_CONFIG
///                 HKEY_CURRENT_USER
///                 HKEY_LOCAL_MACHINE
///                 HKEY_USERS
///	@param	SubKey	Rootkey 이하 서브키 경로. `\` 문자열로 시작하면 안됨
///					SOFTWARE\Microsoft\Windows\CurrentVersion\Run 
HKEY 
RUOpenKey(
    _In_ HKEY RootKey,
	_In_ const wchar_t* SubKey,
	_In_ bool ReadOnly, 
	_In_ bool DisableWow
    )
{
    HKEY hSubKey = nullptr;
    REGSAM sam = (true == ReadOnly) ? KEY_READ : KEY_ALL_ACCESS;

	// 현재 프로세스가 WOW64 프로세스라면 64bit registry 경로로 변경한다. 
	if (DisableWow)
	{		
		if (is_wow64_process(GetCurrentProcess()))
		{
			sam |= KEY_WOW64_64KEY;
		}
	}

	DWORD ret = RegOpenKeyExW(RootKey,
							  SubKey,
							  0,
							  sam,
							  &hSubKey);
	if (ERROR_SUCCESS != ret)
	{
		return nullptr;
	}    

    return hSubKey;
}

/// @brief	
void
RUCloseKey(
	_In_ HKEY Key
    )
{
    RegCloseKey(Key);
}

/// @brief	SubKey 를 생성한다. 이미 존재하는 경우 오픈한다.
/// @param	RootKey	RegCreateKey, RegOpenKey 등이 리턴한 HKEY 또는 아래 나열된 Predefined key.
///					HKEY_CLASSES_ROOT
///                 HKEY_CURRENT_CONFIG
///                 HKEY_CURRENT_USER
///                 HKEY_LOCAL_MACHINE
///                 HKEY_USERS
///	@param	SubKey	Rootkey 이하 서브키 경로. `\` 문자열로 시작하면 안됨
///					SOFTWARE\Microsoft\Windows\CurrentVersion\Run     
HKEY
RUCreateKey(
	_In_ HKEY RootKey,
	_In_ const wchar_t* SubKey,
	_In_ bool ReadOnly, 
	_In_ bool DisableWow
    )
{
    DWORD ret = ERROR_SUCCESS;
    DWORD disposition=0;
    HKEY sub_key_handle = NULL;
    REGSAM sam = (true == ReadOnly) ? KEY_READ : KEY_ALL_ACCESS;

	// 현재 프로세스가 WOW64 프로세스라면 64bit registry 경로로 변경한다. 
	if (DisableWow)
	{
		if (is_wow64_process(GetCurrentProcess()))
		{
			sam |= KEY_WOW64_64KEY;
		}
	}

    ret = RegCreateKeyExW(RootKey, 
						  SubKey, 
						  0, 
						  nullptr,
						  0, 
						  sam, 
						  nullptr,
						  &sub_key_handle, 
						  &disposition);
    if (ERROR_SUCCESS != ret)
    {
        return nullptr;
    }
    else
    {
        return sub_key_handle;
    }    
}

DWORD 
RUReadDword(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_In_ DWORD DefaultValue
    )
{
	_ASSERTE(nullptr != value_name);
	_ASSERTE(nullptr != key_handle);
	if (nullptr == key_handle || nullptr == value_name) return DefaultValue;

    DWORD value = DefaultValue;
    DWORD value_size = sizeof(value);

	DWORD ret = RegQueryValueExW(key_handle, 
								 value_name, 
								 nullptr,
								 nullptr,
								 (PBYTE)&value, 
								 &value_size);
    if (ERROR_SUCCESS != ret)
    {
        return DefaultValue;
    }

    return value;
}

DWORD
RUReadDwordEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ DWORD default_value, 
	_In_ bool disable_wow
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle ||
		nullptr == sub_key || 
		nullptr == value_name)
	{
		return default_value;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, true, disable_wow));
	if (nullptr == rh.get())
	{
		return default_value;
	}

	return RUReadDword(rh.get(), value_name, default_value);
}

bool 
RUWriteDword(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,	
    _In_ DWORD value
    )
{
	_ASSERTE(nullptr != key_handle); 
	_ASSERTE(nullptr != value_name);	
	if (nullptr == key_handle || nullptr == value_name) return false;

    DWORD ret = RegSetValueExW(key_handle, 
							   value_name, 
							   0, 
							   REG_DWORD, 
							   (PBYTE)&value, 
							   sizeof(DWORD));
    if (ERROR_SUCCESS != ret)
    {
        return false;
    }

    return true;
}

bool
RUWriteDwordEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ DWORD value,
	_In_ bool disable_wow
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name)
	{
		return false;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, false, disable_wow));
	if (nullptr == rh.get())
	{
		return false;
	}

	return RUWriteDword(rh.get(), value_name, value);
}

uint64_t 
RUReadQword(
	_In_ HKEY key_handle, 
	_In_ const wchar_t * value_name,
	_In_ uint64_t DefaultValue
)
{
	_ASSERTE(nullptr != value_name);
	_ASSERTE(nullptr != key_handle);
	if (nullptr == key_handle || nullptr == value_name) return DefaultValue;

	uint64_t value = DefaultValue;
	DWORD value_size = sizeof(value);

	DWORD ret = RegQueryValueExW(key_handle,
								 value_name,
								 nullptr,
								 nullptr,
								 (PBYTE)&value,
								 &value_size);
	if (ERROR_SUCCESS != ret)
	{
		return DefaultValue;
	}

	return value;
}

uint64_t
RUReadQwordEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ uint64_t default_value,
	_In_ bool disable_wow = false
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name)
	{
		return default_value;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, false, disable_wow));
	if (nullptr == rh.get())
	{
		return default_value;
	}

	return RUReadQword(rh.get(), value_name, default_value);
}

bool 
RUWriteQword(
	_In_ HKEY key_handle, 
	_In_ const wchar_t * value_name, 
	_In_ uint64_t value
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle || nullptr == value_name) return false;

	DWORD ret = RegSetValueExW(key_handle,
							   value_name,
							   0,
							   REG_QWORD,
							   (PBYTE)&value,
							   sizeof(uint64_t));

	if (ERROR_SUCCESS != ret)
	{
		return false;
	}

	return true;
}

bool
RUWriteQwordEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ int64_t value,
	_In_ bool disable_wow
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name)
	{
		return false;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, false, disable_wow));
	if (nullptr == rh.get())
	{
		return false;
	}

	return RUWriteQword(rh.get(), value_name, value);
}

////

bool
RUReadString(
    _In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_Out_ std::wstring& value
    )
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle || nullptr == value_name) return false;

    void* old = NULL;	
    DWORD cbValue = 1024;
    wchar_t* buffer = (PWCHAR) malloc(cbValue);
    if (NULL == buffer) return false;
    RtlZeroMemory(buffer, cbValue);

	DWORD ret = RegQueryValueExW(key_handle,
								 value_name,
								 nullptr,
								 nullptr,
								 (LPBYTE)buffer,
								 &cbValue);
    while (ERROR_MORE_DATA  == ret)
    {
        cbValue *= 2;
        old = buffer;        // save pointer for realloc faild

        buffer = (PWCHAR) realloc(buffer, cbValue);
        if (NULL == buffer)
        {
            free(old); cbValue = 0;
			return false;
        }
		RtlZeroMemory(buffer, cbValue);

		ret = RegQueryValueExW(key_handle,
							   value_name,
							   nullptr,
							   nullptr,
							   (LPBYTE)buffer,
							   &cbValue
		);
    }

    if (ERROR_SUCCESS != ret)
    {
        free(buffer); buffer=nullptr;
        return false;    
    }
    
	// buffer -> wstring 
	value = buffer;
	free(buffer); buffer = nullptr;
    return true;
}

bool
RUReadStringEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ bool disable_wow,
	_Out_ std::wstring& value
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name)
	{
		return false;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, true, disable_wow));
	if (nullptr == rh.get())
	{
		return false;
	}

	return RUReadString(rh.get(), value_name, value);
}

///

bool
RUSetString(
    _In_ HKEY key_handle,	
	_In_ const wchar_t* value_name,
	_In_ const wchar_t* value
    )
{    
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != value_name);
	_ASSERTE(nullptr != value);
	if (nullptr == key_handle || nullptr == value_name || nullptr == value) return false;
	
    DWORD ret = RegSetValueExW(key_handle,
							   value_name, 
							   0, 
							   REG_SZ, 
							   (LPBYTE)value, 
							   static_cast<uint32_t>(((wcslen(value) + 1) * sizeof(wchar_t))) );
    if (ERROR_SUCCESS != ret)
    {
		return false;
    }

    return true;
}

bool
RUSetStringEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ bool disable_wow,
	_In_ const wchar_t* value
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name)
	{
		return false;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, false, disable_wow));
	if (nullptr == rh.get())
	{
		return false;
	}

	return RUSetString(rh.get(), value_name, value);
}

///

bool
RUSetExpandString(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_In_ const wchar_t* value,	// byte buffer
	_In_ DWORD cbValue			// count byte
    )
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != value_name);
	_ASSERTE(nullptr != value);
	if (nullptr == key_handle || nullptr == value_name || nullptr == value) return false;

    DWORD ret = RegSetValueExW(key_handle, 
							   value_name, 
							   0, 
							   REG_EXPAND_SZ, 
							   (LPBYTE)value, 
							   cbValue);
    if (ERROR_SUCCESS != ret)
    {
		return false;
    }

    return true;
}

bool
RUSetExpandStringEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ bool disable_wow,
	_In_ const wchar_t* value,
	_In_ DWORD cbValue
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	_ASSERTE(nullptr != value);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name ||
		nullptr == value)
	{
		return false;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, false, disable_wow));
	if (nullptr == rh.get())
	{
		return false;
	}

	return RUSetExpandString(rh.get(), value_name, value, cbValue);
}

///	@brief Multi_SZ type의 레지스트리 value값을 읽는다.
bool
RUReadMultiString(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_Out_ std::vector<std::wstring>& value
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle || nullptr == value_name) return false;

	DWORD type = REG_NONE;
	DWORD cbValue = 0;
	DWORD ret = RegQueryValueExW(key_handle,
								 value_name,
								 nullptr,
								 &type,
								 nullptr,
								 &cbValue);

	if (ERROR_SUCCESS != ret)
	{
		return false;
	}

	_ASSERTE(REG_MULTI_SZ == type);
	if (REG_MULTI_SZ != type)
	{
		log_err
			"Type of requested value is not REG_MULTI_SZ. value=%ws",
			value_name
			log_end;
		return false;
	}

	char_ptr ptr((char*)malloc(cbValue),
				 [](char* p) {
		free_and_nil(p);
	});
	if (nullptr == ptr.get())
	{
		log_err "No resources for value. value=%ws, size=%u",
			value_name,
			cbValue
			log_end;
		return false;
	}

	uint8_t* buffer = (uint8_t*)ptr.get();
	RtlZeroMemory(buffer, cbValue);

	ret = RegQueryValueExW(key_handle,
						   value_name,
						   nullptr,
						   nullptr,
						   buffer,
						   &cbValue);

	if (ERROR_SUCCESS != ret)
	{
		return false;
	}

	//
	// buffer = L"Welcome\0to\0Hello\0World\0\0"
	//
	// 1. L"\0\0"을 먼저 비교하여 마지막인지 체크하고 wstring값을 vector에 복사한다.
	// 2. L"\0" null terminator를 찾으면 wstring값을 vector에 복사한다.

	size_t str_pos = 0;
	size_t pos = 0;
	while (pos < cbValue)
	{
		if (0 == wmemcmp((wchar_t*)(buffer + pos), L"\0\0", 2))
		{
			value.push_back((wchar_t*)(buffer + str_pos));
			break;
		}
		if (buffer[pos] == L'\0')
		{
			value.push_back((wchar_t*)(buffer + str_pos));
			str_pos = pos + sizeof(wchar_t);
		}
		pos += sizeof(wchar_t);
	}

	return true;
}

bool
RUReadMultiStringEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ bool disable_wow,
	_Out_ std::vector<std::wstring>& values
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name)
	{
		return false;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, true, disable_wow));
	if (nullptr == rh.get())
	{
		return false;
	}

	return RUReadMultiString(rh.get(), value_name, values);
}


///	@brief Multi_SZ type의 레지스트리 value를 생성한다.
///
///			Multi_SZ는 여러개의 문자열을 저장하는데 문자열을 구분할때 '\' null terminator가 들어간다.
///			그리고 항상 마지막 null terminator는 2개가 와야한다.

///			문자열 1. L"Welcome"
///			문자열 2. L"to"
///			문자열 3. L"Hello"
///			문자열 4. L"World"
///
///			buffer = L"Welcome\0to\0Hello\0World\0\0"
bool
RUSetMultiString(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_In_ std::vector<std::wstring>& values
	)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != value_name);
	_ASSERTE(true != values.empty());
	if (nullptr == key_handle ||
		nullptr == value_name ||
		true == values.empty())
	{
		return false;
	}

	uint16_t null_term = 0;
	CMemoryStream strm;
	for (const auto& value : values)
	{
		strm.WriteToStream((char*)value.c_str(), (unsigned long)value.size() * sizeof(wchar_t));
		strm.WriteToStream((char*)&null_term, sizeof(uint16_t));
	}
	strm.WriteToStream((char*)&null_term, sizeof(uint16_t));

	DWORD ret = RegSetValueExW(key_handle,
							   value_name,
							   0,
							   REG_MULTI_SZ,
							   (LPBYTE)strm.GetMemory(),
							   (DWORD)strm.GetSize());
	if (ERROR_SUCCESS != ret)
	{
		return false;
	}

	return true;
}

bool
RUSetMultiString(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ bool disable_wow,
	_In_ std::vector<std::wstring>& values
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name)
	{
		return false;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, false, disable_wow));
	if (nullptr == rh.get())
	{
		return false;
	}

	return RUSetMultiString(rh.get(), value_name, values);
}


/// @remark	caller must free returned buffer pointer.
uint8_t*
RUReadBinaryData(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_Out_ DWORD& cbValue			// count byte
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle || nullptr == value_name) return false;

	void* old = nullptr;
	cbValue = 1024;
	uint8_t* buffer = (uint8_t*)malloc(cbValue);
	if (nullptr == buffer)
	{
		cbValue = 0;
		return nullptr;
	}
	RtlZeroMemory(buffer, cbValue);

	DWORD ret = RegQueryValueExW(key_handle,
								 value_name,
								 nullptr,
								 nullptr,
								 (LPBYTE)buffer,
								 &cbValue);
	while (ERROR_MORE_DATA == ret)
	{
		cbValue *= 2;
		old = buffer;        // save pointer for realloc faild

		buffer = (uint8_t*)realloc(buffer, cbValue);
		if (nullptr == buffer)
		{
			free(old);  cbValue = 0;
			return nullptr;
		}
		RtlZeroMemory(buffer, cbValue);
		ret = RegQueryValueExW(key_handle,
							   value_name,
							   nullptr,
							   nullptr,
							   (LPBYTE)buffer,
							   &cbValue);
	}

	if (ERROR_SUCCESS != ret)
	{
		free(buffer); buffer = nullptr;
		return nullptr;
	}

	return buffer;
}

uint8_t*
RUReadBinaryDataEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ bool disable_wow,
	_Out_ DWORD& cbValue
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name)
	{
		return false;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, true, disable_wow));
	if (nullptr == rh.get())
	{
		return nullptr;
	}

	return RUReadBinaryData(rh.get(), value_name, cbValue);
}

/// @remark	value 사이즈 제한에 관한 정보는 링크 참조
///			https://msdn.microsoft.com/en-us/library/windows/desktop/ms724872(v=vs.85).aspx
bool
RUSetBinaryData(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_In_ const uint8_t* value,	// byte buffer
	_In_ DWORD cbValue			// count byte
	)	
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != value_name);
	_ASSERTE(nullptr != value);
	if (nullptr == key_handle || nullptr == value_name || nullptr == value) return false;

	DWORD ret = RegSetValueExW(key_handle, 
							   value_name, 
							   0, 
							   REG_BINARY, 
							   (LPBYTE)value, 
							   cbValue);
    if (ERROR_SUCCESS != ret)
    {
		return false;
    }

    return true;
}

bool
RUSetBinaryDataEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ bool disable_wow,
	_In_ const uint8_t* value,
	_In_ DWORD cbValue
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	_ASSERTE(nullptr != value);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name ||
		nullptr == value)
	{
		return false;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, false, disable_wow));
	if (nullptr == rh.get())
	{
		return false;
	}

	return RUSetBinaryData(rh.get(), value_name, value, cbValue);
}

bool
RUDeleteValue(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name
	)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle || nullptr == value_name) return false;

	DWORD ret = RegDeleteValueW(key_handle, value_name);
	if (ERROR_SUCCESS != ret)
	{
		return false;
	}
	return true;
}

bool
RUDeleteValueEx(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ const wchar_t* value_name,
	_In_ bool disable_wow
)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	_ASSERTE(nullptr != value_name);
	if (nullptr == key_handle ||
		nullptr == sub_key ||
		nullptr == value_name)
	{
		return false;
	}

	RegHandle rh(RUOpenKey(key_handle, sub_key, false, disable_wow));
	if (nullptr == rh.get())
	{
		return false;
	}

	return RUDeleteValue(rh.get(), value_name);
}

/// @brief	sub_key 와 value 를 삭제한다. 
/// 
///			recursive 가 true 인 경우 sub key 의 하위키도 몽땅 삭제한다. 
///			sub key 의 하위 키가 존재하고, recursive 가 flase 인 경우 false 를 리턴한다. 
bool 
RUDeleteKey(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key, 
	_In_ bool recursive,
	_In_ bool DisableWow
	)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	if (nullptr == key_handle || nullptr == sub_key) return false;

	//
	// recursive 인 경우 하위 키/값들을 먼저 삭제
	//
	if (recursive)
	{
		// 
		//	key_handle\sub_key 를 삭제한다. 
		//	하위 key/value 까지 몽땅 삭제한다. 
		//
		//	(참고)
		//	WOW 문제가 있기때문에 sub_key 를 열어서 핸들을 구한후
		//	RegDeleteTreeW 함수로 sub_key 를 제외한 하위 키/값들을 
		//	먼저 삭제한다. 
		// 
		hkey_ptr hkey(RUOpenKey(key_handle,
								sub_key,
								false,
								DisableWow),
					  [](HKEY key)
		{
			if (nullptr != key)
			{
				RegCloseKey(key);
			}
		});
		if (!hkey) { return false; }

		LSTATUS status = RegDeleteTreeW(hkey.get(), nullptr);
		if (ERROR_SUCCESS != status)
		{
			log_err
				"RegDeleteTreeW() failed. sub=%ws, err=%u",
				sub_key,
				status
				log_end;
			return false;
		}
	}

	//
	//	sub_key 를 삭제한다. 
	//
	REGSAM sam = 0;
	if (DisableWow)
	{
		// 현재 프로세스가 WOW64 프로세스라면 64bit registry 경로로 변경. 
		if (is_wow64_process(GetCurrentProcess()))
		{
			sam |= KEY_WOW64_64KEY;
		}
	}

	LSTATUS status = RegDeleteKeyExW(key_handle, sub_key, sam, 0);
	if (ERROR_SUCCESS != status)
	{
		log_err
			"RegDeleteKeyExW() failed. sub=%ws, err=%u",
			sub_key,
			status
			log_end;
		return false;
	}

	return true;
}

/// @prarm	root_key	HKEY_CLASSES_ROOT
///						HKEY_CURRENT_CONFIG
///						HKEY_CURRENT_USER
///						HKEY_LOCAL_MACHINE
///						HKEY_USERS
///						
/// @param	sub_key		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
///						'\' 로 시작하면 안됨
bool
RUIsKeyExists(
	_In_ HKEY root_key,
	_In_ const wchar_t* sub_key, 
	_In_ bool DisableWow
    )
{
	_ASSERTE(nullptr != root_key);
	_ASSERTE(nullptr != sub_key);
	if (nullptr == root_key || nullptr == sub_key) return false;

    RegHandle reg (RUOpenKey(root_key, sub_key, true, DisableWow));    
    return (NULL == reg.get()) ? false : true;
}


/// @brief  `key` 의 key, value 들을 enumerate 한다. 
///         sub key 는 `key_cb` 를 통해 caller 에게 전달된다.
///			또한, sub key의 부모 경로가 필요한 경우 base_name을
///         함께 전달 한다. 그렇기 때문에 base_name은 NULL 일 수
///			있다.
///         value 는 `value_cb` 를 통해 caller 에게 전달된다. 
///         key_cb, value_cb 는 NULL 일 수 있다. 
///			
bool
reg_enum_key_values(
	_In_ HKEY key,
	_In_ const wchar_t* base_name,
	_In_ fn_key_callback_tag key_cb,
	_In_ DWORD_PTR key_cb_tag,
	_In_ fn_key_value_callback_tag value_cb,
	_In_ DWORD_PTR value_cb_tag
	)
{
	_ASSERTE(nullptr != key);
	if (nullptr == key) return false;

	DWORD	 sub_key_count				 = 0;
	DWORD	 max_sub_key_name_cc		 = 0;
	DWORD	 max_class_name_cc			 = 0;
	DWORD	 value_count				 = 0;
	DWORD	 max_value_name_cc			 = 0;
	DWORD	 max_value_data_byte		 = 0;
	DWORD	 max_security_desciptor_byte = 0;
	FILETIME last_write_time			 = { 0 };

	LSTATUS ret = RegQueryInfoKeyW(key,
								   nullptr,
								   nullptr,
								   nullptr,
								   &sub_key_count,
								   &max_sub_key_name_cc,
								   &max_class_name_cc,
								   &value_count,
								   &max_value_name_cc,
								   &max_value_data_byte,
								   &max_security_desciptor_byte,
								   &last_write_time);
	if (ERROR_SUCCESS != ret)
	{
		//log_err "RegQueryInfoKeyW() failed. ret = %u", 
		//	ret 
		//	log_end;
		return false;
	}

	//log_dbg
	//    "sub_key_count = %u, \n"\
	//    "max_sub_key_name_size = %u, \n"\
    //    "max_class_name_size = %u, \n"\
    //    "value_count = %u, \n"\
    //    "max_value_name_size = %u, \n"\
    //    "max_value_data_size = %u, \n"\
    //    "max_security_desciptor_size = %u",
	//    sub_key_count,
	//    max_sub_key_name_cc,
	//    max_class_name_cc,
	//    value_count,
	//    max_value_name_cc,
	//    max_value_data_byte,
	//    max_security_desciptor_byte
	//    log_end;

	// xxx_cc 값은 null terminate 를 포함하지 않는 character count 이다. 
	// RegEnumKeyEx() 같은 함수들은 파라미터를 받을때 null terminate 를 포함한 
	// 버퍼와 버퍼의 사이즈를 파라미터로 받기 때문에 편의상 xxx_cc 에 +1 을 해준다. 
	++max_sub_key_name_cc;
	++max_class_name_cc;
	++max_value_name_cc;

	// enum reg key
	if (NULL != key_cb)
	{
		if (0 < sub_key_count)
		{
			for (DWORD i = 0; i < sub_key_count; ++i)
			{
				wchar_t* sub_key_name = (wchar_t*)malloc(max_sub_key_name_cc * sizeof(wchar_t));
				wchar_t* class_name = (wchar_t*)malloc((max_class_name_cc) * sizeof(wchar_t));

				if (nullptr == sub_key_name)
				{
					log_err "Not enough memory. size=%u",
						max_sub_key_name_cc * sizeof(wchar_t)
						log_end;
					continue;
				}

				if (nullptr == class_name)
				{
					log_err "Not enough memory. size=%u",
						max_class_name_cc * sizeof(wchar_t)
						log_end;
					continue;
				}

				DWORD sub_key_name_cc = max_sub_key_name_cc;
				DWORD class_name_cc   = max_class_name_cc;

				ret = RegEnumKeyEx(key,
								   i,
								   sub_key_name,
								   &sub_key_name_cc,
								   nullptr,
								   class_name,
								   &class_name_cc,
								   &last_write_time);
				if (ERROR_SUCCESS != ret)
				{
					log_err "RegEnumKeyEx() failed. ret = %u", 
						ret 
						log_end;
				}
				else
				{
					if (true != key_cb(i, 
									   base_name, 
									   sub_key_name, 
									   class_name, 
									   key_cb_tag))
					{
						// caller canceled;
						break;
					}
				}

				free_and_nil(sub_key_name);
				free_and_nil(class_name);
			}
		}
	}

	// enum reg values
	if (NULL != value_cb)
	{
		if (0 < value_count)
		{
			//log_dbg "value count = %u", value_count log_end;
			wchar_t* value_name = (wchar_t*)malloc(max_value_name_cc * sizeof(wchar_t));
			BYTE*    value_data = (BYTE*)malloc(max_value_data_byte);
			if (nullptr != value_name && nullptr != value_data)
			{
				for (DWORD i = 0; i < value_count; i++)
				{
					RtlZeroMemory(value_name, (max_value_name_cc) * sizeof(wchar_t));
					RtlZeroMemory(value_data, max_value_data_byte);

					DWORD value_type = REG_NONE;
					DWORD value_name_cc = max_value_name_cc;
					DWORD value_data_size = max_value_data_byte;

					ret = RegEnumValue(key,
									   i,
									   value_name,
									   &value_name_cc,
									   NULL,
									   &value_type,
									   value_data,
									   &value_data_size);
					if (ret == ERROR_SUCCESS)
					{
						//log_dbg
						//    "index = %u, value_name = %ws, value_type = %u, data = %ws", 
						//    i, 
						//    value_name, 
						//    value_type, 
						//    (char*)value_data
						//    log_end;
						if (true != value_cb(i,
											 value_type,
											 value_name,
											 value_data_size,
											 value_data,
											 value_cb_tag))
						{
							// caller calceld.
							break;
						}
					}
					else
					{
						log_err "RegEnumValue() failed. ret = %u", ret log_end;
					}
				}
			}

			free_and_nil(value_name);
			free_and_nil(value_data);
		}
	}

	return true;
}

///	root key 
bool
str_to_reg_key(
	_In_ const wchar_t* key_path,
	_Out_ HKEY& key,
	_Out_ std::wstring& sub_key
)
{
	_ASSERTE(nullptr != key_path);
	if (nullptr == key_path) return false;

	std::wstring root_key_str = extract_first_tokenExW(key_path,
													   L"\\",
													   true);
	to_lower_string(root_key_str);

	bool ret = false;
	do
	{
		if (0 == root_key_str.compare(_hklm_str) || 0 == root_key_str.compare(_hklm_full_str))
		{
			key = HKEY_LOCAL_MACHINE;
		}
		else if(0 == root_key_str.compare(_hkcu_str) || 0 == root_key_str.compare(_hkcu_full_str))
		{
			key = HKEY_CURRENT_USER;
		}
		else if (0 == root_key_str.compare(_hkcr_str) || 0 == root_key_str.compare(_hkcr_full_str))
		{
			key = HKEY_CLASSES_ROOT;
		}
		else
		{
			break;
		}
		sub_key = extract_first_tokenExW(key_path,
										 L"\\",
										 false);
		ret = true;
	} while (false);

	return ret;
}