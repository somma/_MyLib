/**
 * @file    Windows registry api wrapper
 * @brief	
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011/11/11 created.
 * @copyright All rights reserved by Yonghwan, Roh.
 *
 *			Registry I/O �� ��� Ű �Ǵ� Value �� ���� ��찡 ���� �����α׸� �� ������
 *			�ʹ� ���� ���ʿ��� �αװ� ���´�. RegistryUtil ��� �������� �����α׸� ������ �ʴ´�.
**/

#include "stdafx.h"
#include <crtdbg.h>
#include "log.h"
#include "RegistryUtil.h"
#include "CStream.h"


#define NO_SHLWAPI_STRFCNS
#include "Shlwapi.h"
#pragma comment(lib, "Shlwapi.lib")



/// @brief	������Ʈ�� Ű�� �����Ѵ�. ������ nullptr �� �����Ѵ�.
/// @param	RootKey	RegCreateKey, RegOpenKey ���� ������ HKEY �Ǵ� �Ʒ� ������ Predefined key.
///					HKEY_CLASSES_ROOT
///                 HKEY_CURRENT_CONFIG
///                 HKEY_CURRENT_USER
///                 HKEY_LOCAL_MACHINE
///                 HKEY_USERS
///	@param	SubKey	Rootkey ���� ����Ű ���. `\` ���ڿ��� �����ϸ� �ȵ�
///					SOFTWARE\Microsoft\Windows\CurrentVersion\Run 
HKEY 
RUOpenKey(
    _In_ HKEY RootKey,
	_In_ const wchar_t* SubKey,
	_In_ bool ReadOnly
    )
{
    HKEY hSubKey = nullptr;
    REGSAM sam = (true == ReadOnly) ? KEY_READ : KEY_ALL_ACCESS;

    DWORD ret = RegOpenKeyExW(RootKey, SubKey, 0, sam, &hSubKey);
    if (ERROR_SUCCESS != ret)
    {
        //log_err "RegOpenKeyExW(%ws) failed, ret = %u", SubKey, ret log_end        
        return nullptr;
    }

    return hSubKey;
}


/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
void
RUCloseKey(
	_In_ HKEY Key
    )
{
    DWORD ret = RegCloseKey(Key);
    if (ERROR_SUCCESS != ret)
    {
        //log_err "RegCloseKey() failed, ret = %u", ret log_end
    }
}

/// @brief	SubKey �� �����Ѵ�. �̹� �����ϴ� ��� �����Ѵ�.
/// @param	RootKey	RegCreateKey, RegOpenKey ���� ������ HKEY �Ǵ� �Ʒ� ������ Predefined key.
///					HKEY_CLASSES_ROOT
///                 HKEY_CURRENT_CONFIG
///                 HKEY_CURRENT_USER
///                 HKEY_LOCAL_MACHINE
///                 HKEY_USERS
///	@param	SubKey	Rootkey ���� ����Ű ���. `\` ���ڿ��� �����ϸ� �ȵ�
///					SOFTWARE\Microsoft\Windows\CurrentVersion\Run     
HKEY
RUCreateKey(
	_In_ HKEY RootKey,
	_In_ const wchar_t* SubKey,
	_In_ bool ReadOnly
    )
{
    DWORD ret = ERROR_SUCCESS;
    DWORD disposition=0;
    HKEY sub_key_handle = NULL;
    REGSAM sam = (true == ReadOnly) ? KEY_READ : KEY_ALL_ACCESS;

    ret = RegCreateKeyExW(RootKey, 
						  SubKey, 
						  0, 
						  NULL, 
						  0, 
						  sam, 
						  NULL, 
						  &sub_key_handle, 
						  &disposition);
    if (ERROR_SUCCESS != ret)
    {
		//log_err "RegCreateKeyExW(%ws) failed, ret = %u",
		//	SubKey,
		//	ret
		//	log_end
        return NULL;
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
								 NULL, 
								 NULL, 
								 (PBYTE)&value, 
								 &value_size);
    if (ERROR_SUCCESS != ret)
    {
		//log_err "RegQueryValueExW(%ws) failed, ret = %u",
		//	value_name,
		//	ret
		//	log_end;
        return DefaultValue;
    }

    return value;
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
		//log_err "RegSetValueExW(%ws) failed, ret = %u",
		//	value_name,
		//	ret
		//	log_end;
        return false;
    }

    return true;
}

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

    DWORD ret = RegQueryValueExW(
                        key_handle,
                        value_name, 
                        NULL, 
                        NULL, 
                        (LPBYTE) buffer, 
                        &cbValue
                        );
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

        ret = RegQueryValueExW(
                        key_handle, 
                        value_name, 
                        NULL, 
                        NULL, 
                        (LPBYTE) buffer, 
                        &cbValue
                        );
    }

    if (ERROR_SUCCESS != ret)
    {
		//log_err "RegQueryValueExW(%ws) failed, ret = %u",
		//	value_name,
		//	ret
		//	log_end;
        free(buffer); buffer=NULL;
        return false;    
    }
    
	// buffer -> wstring 
	value = buffer;
	free(buffer); buffer = NULL;
    return true;
}

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
		//log_err "RegSetValueExW(%ws) failed, ret = %u",
		//	value_name,
		//	ret
		//	log_end;
		return false;
    }

    return true;
}

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
		//log_err "RegSetValueExW(%ws) failed, ret = %u",
		//	value_name,
		//	ret 
		//	log_end;
        return false;
    }

    return true;
}

///	@brief Multi_SZ type�� ������Ʈ�� value�� �����Ѵ�.
///
///			Multi_SZ�� �������� ���ڿ��� �����ϴµ� ���ڿ��� �����Ҷ� '\' null terminator�� ����.
///			�׸��� �׻� ������ null terminator�� 2���� �;��Ѵ�.

///			���ڿ� 1. L"Welcome"
///			���ڿ� 2. L"to"
///			���ڿ� 3. L"Hello"
///			���ڿ� 4. L"World"
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
	for (auto& value : values)
	{
		strm.WriteToStream(value.c_str(), (unsigned long)value.size() * sizeof(wchar_t));
		strm.WriteToStream(&null_term, sizeof(uint16_t));
	}
	strm.WriteToStream(&null_term, sizeof(uint16_t));

	DWORD ret = RegSetValueExW(key_handle,
							   value_name,
							   NULL,
							   REG_MULTI_SZ,
							   (LPBYTE)strm.GetMemory(),
							   strm.GetSize());
	if (ERROR_SUCCESS != ret)
	{
		//log_err "RegSetValueExW(%ws) failed, ret = %u",
		//	value_name,
		//	ret
		//	log_end;
		return false;
	}

	return true;
}

///	@brief Multi_SZ type�� ������Ʈ�� value���� �д´�.
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
		//log_err "RegQueryValueExW(%ws) failed. ret = %u",
		//	value_name,
		//	ret
		//	log_end;
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
		//log_err "RegQueryValueExW(%ws) failed. ret = %u",
		//	value_name,
		//	ret
		//	log_end;
		return false;
	}

	//
	// buffer = L"Welcome\0to\0Hello\0World\0\0"
	//
	// 1. L"\0\0"�� ���� ���Ͽ� ���������� üũ�ϰ� wstring���� vector�� �����Ѵ�.
	// 2. L"\0" null terminator�� ã���� wstring���� vector�� �����Ѵ�.

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

/// @remark	value ������ ���ѿ� ���� ������ ��ũ ����
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
		//log_err "RegSetValueExW(%ws) failed, ret = %u",
		//	value_name,
		//	ret
		//	log_end;
		return false;
    }

    return true;
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
    uint8_t* buffer = (uint8_t*) malloc(cbValue);
    if (nullptr == buffer)
    {
        cbValue = 0;
        return nullptr;
    }
    RtlZeroMemory(buffer, cbValue);

    DWORD ret = RegQueryValueExW(
                        key_handle, 
                        value_name, 
                        NULL, 
                        NULL,
                        (LPBYTE) buffer, 
                        &cbValue
                        );
    while (ERROR_MORE_DATA  == ret)
    {
        cbValue *= 2;
        old = buffer;        // save pointer for realloc faild

        buffer = (uint8_t*) realloc(buffer, cbValue);
        if (NULL == buffer)
        {
            free(old);  cbValue = 0;
            return nullptr;
        }
		RtlZeroMemory(buffer, cbValue);

        ret = RegQueryValueExW(
                        key_handle, 
                        value_name, 
						nullptr,
						nullptr,
                        (LPBYTE) buffer, 
                        &cbValue
                        );
    }

    if (ERROR_SUCCESS != ret)
    {
		//log_err "RegQueryValueExW(%ws) failed, ret = %u",
		//	value_name,
		//	ret
		//	log_end;
		free(buffer); buffer= nullptr;
        return nullptr;    
    }
    
	return buffer;
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
		//log_err "RegDeleteValueW( %ws ) failed. ret = %u",
		//	value_name,
		//	ret
		//	log_end;
		return false;
	}
	return true;
}

/// @brief	sub_key �� value �� �����Ѵ�. 
/// 
///			recursive �� true �� ��� sub key �� ����Ű�� ���� �����Ѵ�. 
///			sub key �� ���� Ű�� �����ϰ�, recursive �� flase �� ��� false �� �����Ѵ�. 
bool 
RUDeleteKey(
	_In_ HKEY key_handle,
	_In_ const wchar_t* sub_key, 
	_In_ bool recursive
	)
{
	_ASSERTE(nullptr != key_handle);
	_ASSERTE(nullptr != sub_key);
	if (nullptr == key_handle || nullptr == sub_key) return false;
		
	if (true != recursive)
	{
		//
		//	key_handle\sub_key �� �����Ѵ�. 
		//	key_handle\sub_key\value �� �ִٸ� �Բ� �����Ѵ�. 
		//	key_handle\sub_key\sub_key2 �� �ִٸ� ������ �����Ѵ�. 
		// 
		DWORD ret = RegDeleteKeyW(key_handle, sub_key);
		if (ERROR_SUCCESS != ret)
		{
			//log_err "RegDeleteKeyW( sub = %ws ) failed. ret = %u",
			//	sub_key,
			//	ret
			//	log_end;
			return false;
		}
		return true;
	}
	else
	{
		// 
		//	key_handle\sub_key �� �����Ѵ�. 
		//	���� key/value ���� ���� �����Ѵ�. 
		// 
		LSTATUS ls = SHDeleteKeyW(key_handle, sub_key);
		if (ERROR_SUCCESS != ls)
		{
			//log_err "SHDeleteKeyW( sub = %ws ) failed. lstatus = %u",
			//	sub_key,
			//	ls
			//	log_end;
			return false;
		}
		return true;
	}	
}

/// @prarm	root_key	HKEY_CLASSES_ROOT
///						HKEY_CURRENT_CONFIG
///						HKEY_CURRENT_USER
///						HKEY_LOCAL_MACHINE
///						HKEY_USERS
///						
/// @param	sub_key		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
///						'\' �� �����ϸ� �ȵ�
bool
RUIsKeyExists(
	_In_ HKEY root_key,
	_In_ const wchar_t* sub_key
    )
{
	_ASSERTE(nullptr != root_key);
	_ASSERTE(nullptr != sub_key);
	if (nullptr == root_key || nullptr == sub_key) return false;


    RegHandle reg (RUOpenKey(root_key, sub_key, true));    
    return (NULL == reg.get()) ? false : true;
}


/// @brief  `key` �� key, value ���� enumerate �Ѵ�. 
///         sub key �� `key_cb` �� ���� caller ���� ���޵ȴ�.
///			����, sub key�� �θ� ��ΰ� �ʿ��� ��� base_name��
///         �Բ� ���� �Ѵ�. �׷��� ������ base_name�� NULL �� ��
///			�ִ�.
///         value �� `value_cb` �� ���� caller ���� ���޵ȴ�. 
///         key_cb, value_cb �� NULL �� �� �ִ�. 
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

	// xxx_cc ���� null terminate �� �������� �ʴ� character count �̴�. 
	// RegEnumKeyEx() ���� �Լ����� �Ķ���͸� ������ null terminate �� ������ 
	// ���ۿ� ������ ����� �Ķ���ͷ� �ޱ� ������ ���ǻ� xxx_cc �� +1 �� ���ش�. 
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