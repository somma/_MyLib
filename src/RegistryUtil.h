/**
 * @file    Windows registry api wrapper
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011/11/11 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#pragma once

#include <string>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383


/// 레지스트리 root 문자열
#define _hklm_str		L"hklm"
#define _hklm_full_str	L"hkey_local_machine"
#define _hkcu_str		L"hkcu"
#define _hkcu_full_str	L"hkey_current_user"
#define _hkcr_str		L"hkcr"
#define _hkcr_full_str	L"hkey_classes_root"

HKEY 
RUOpenKey(
	_In_ HKEY RootKey, 
	_In_ const wchar_t* SubKey,
	_In_ bool ReadOnly);

void
RUCloseKey(
	_In_ HKEY Key
	);

HKEY 
RUCreateKey(
	_In_ HKEY RootKey,
	_In_ const wchar_t* SubKey,
	_In_ bool ReadOnly
	);

DWORD 
RUReadDword(	
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_In_ DWORD DefaultValue
	);

bool 
RUWriteDword(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_In_ DWORD value
	);

uint64_t
RUReadQword(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_In_ uint64_t DefaultValue);

bool
RUWriteQword(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_In_ uint64_t value);

bool 
RUReadString(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_Out_ std::wstring& value
	);

bool
RUSetString(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_In_ const wchar_t* value
	);

bool 
RUSetExpandString(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_In_ const wchar_t* value, 
	_In_ DWORD cbValue
	);

bool 
RUSetBinaryData(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_In_ const uint8_t* value, 
	_In_ DWORD cbValue
	);

bool
RUSetMultiString(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_In_ std::vector<std::wstring>& values
	);

bool
RUReadMultiString(
	_In_ HKEY key_handle,
	_In_ const wchar_t* value_name,
	_Out_ std::vector<std::wstring>& values
	);

uint8_t* 
RUReadBinaryData(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_Out_ DWORD& cbValue
	);

bool 
RUDeleteValue(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name
	);

bool
RUDeleteKey(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key,
	_In_ bool recursive
	);

bool 
RUIsKeyExists(
	_In_ HKEY root_key, 
	_In_ const wchar_t* sub_key
	);

typedef
bool(*fn_key_callback_tag)(
	_In_ uint32_t index,
	_In_ const wchar_t* base_name,
	_In_ const wchar_t* sub_key_name,
	_In_ const wchar_t* class_name,
	_In_ DWORD_PTR tag
	);

typedef
bool(*fn_key_value_callback_tag)(
	_In_ uint32_t index,
	_In_ uint32_t value_type,
	_In_ const wchar_t* value_name,
	_In_ uint32_t value_data_size,
	_In_ const uint8_t* value_data,
	_In_ DWORD_PTR tag
	);

bool
reg_enum_key_values(
	_In_ HKEY key,
	_In_ const wchar_t* base_name,
	_In_ fn_key_callback_tag key_cb,
	_In_ DWORD_PTR key_cb_tag,
	_In_ fn_key_value_callback_tag value_cb,
	_In_ DWORD_PTR value_cb_tag
	);

bool
str_to_reg_key(
	_In_ const wchar_t* key_path,
	_Out_ HKEY& key,
	_Out_ std::wstring& sub_key
);

class RegHandle
{
public:
    RegHandle(HKEY key) : m_key(key) {}
	~RegHandle()    { if (NULL != m_key) RegCloseKey(m_key);  }
    HKEY get()      { return m_key; };
protected:
private:
	HKEY m_key;
};
