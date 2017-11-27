/**
 * @file    Windows registry api wrapper
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011/11/11 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include "RegistryUtil.h"


#define NO_SHLWAPI_STRFCNS
#include "Shlwapi.h"
#pragma comment(lib, "Shlwapi.lib")


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
    HKEY RootKey,
    const wchar_t* SubKey, 
    bool ReadOnly
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
    HKEY Key
    )
{
    DWORD ret = RegCloseKey(Key);
    if (ERROR_SUCCESS != ret)
    {
        //log_err "RegCloseKey() failed, ret = %u", ret log_end
    }
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
    HKEY RootKey,
    const wchar_t* SubKey,
    bool ReadOnly
    )
{
    DWORD ret = ERROR_SUCCESS;
    DWORD disposition=0;
    HKEY sub_key_handle = NULL;
    REGSAM sam = (true == ReadOnly) ? KEY_READ : KEY_ALL_ACCESS;

    ret = RegCreateKeyExW(RootKey, SubKey, 0, NULL, 0, sam, NULL, &sub_key_handle, &disposition);
    if (ERROR_SUCCESS != ret)
    {
        log_err "RegCreateKeyExW(%ws) failed, ret = %u", SubKey, ret log_end        
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
		log_err "RegQueryValueExW(%ws) failed, ret = %u",
			value_name,
			ret
			log_end;
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
		log_err "RegSetValueExW(%ws) failed, ret = %u",
			value_name,
			ret
			log_end;
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
		log_err "RegQueryValueExW(%ws) failed, ret = %u",
			value_name,
			ret
			log_end;
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
		log_err "RegSetValueExW(%ws) failed, ret = %u",
			value_name,
			ret
			log_end;
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
		log_err "RegSetValueExW(%ws) failed, ret = %u",
			value_name,
			ret 
			log_end;
        return false;
    }

    return true;
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
		log_err "RegSetValueExW(%ws) failed, ret = %u",
			value_name,
			ret
			log_end;
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
		log_err "RegQueryValueExW(%ws) failed, ret = %u",
			value_name,
			ret
			log_end;
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
		log_err "RegDeleteValueW( %ws ) failed. ret = %u",
			value_name,
			ret
			log_end;
		return false;
	}
	return true;
}

/// @brief	sub_key 와 value 를 삭제한다. 
/// 
///			recursive 가 true 인 경우 sub key 의 하위키도 몽땅 삭제한다. 
///			sub key 의 하위 키가 존재하고, recursive 가 flase 인 경우 false 를 리턴한다. 
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
		//	key_handle\sub_key 를 삭제한다. 
		//	key_handle\sub_key\value 가 있다면 함께 삭제한다. 
		//	key_handle\sub_key\sub_key2 가 있다면 에러를 리턴한다. 
		// 
		DWORD ret = RegDeleteKeyW(key_handle, sub_key);
		if (ERROR_SUCCESS != ret)
		{
			log_err "RegDeleteKeyW( sub = %ws ) failed. ret = %u",
				sub_key,
				ret
				log_end;
			return false;
		}
		return true;
	}
	else
	{
		// 
		//	key_handle\sub_key 를 삭제한다. 
		//	하위 key/value 까지 몽땅 삭제한다. 
		// 
		LSTATUS ls = SHDeleteKeyW(key_handle, sub_key);
		if (ERROR_SUCCESS != ls)
		{
			log_err "SHDeleteKeyW( sub = %ws ) failed. lstatus = %u",
				sub_key,
				ls
				log_end;
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
///						'\' 로 시작하면 안됨
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
		log_err "RegQueryInfoKeyW() failed. ret = %u", ret log_end;
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
					log_err "RegEnumKeyEx() failed. ret = %u", ret log_end;
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

			for (DWORD i = 0; i < value_count; i++)
			{
				RtlZeroMemory(value_name, (max_value_name_cc) * sizeof(wchar_t));
				RtlZeroMemory(value_data, max_value_data_byte);

				DWORD value_type      = REG_NONE;
				DWORD value_name_cc   = max_value_name_cc;
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

			free_and_nil(value_name);
			free_and_nil(value_data);
		}
	}

	return true;
}