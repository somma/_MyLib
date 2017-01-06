/**----------------------------------------------------------------------------
 * RegistryUtil.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 11:11:2011   0:43 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "RegistryUtil.h"


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




/**----------------------------------------------------------------------------
    \brief      
    \param      RootKey     HKEY_CLASSES_ROOT
                            HKEY_CURRENT_CONFIG
                            HKEY_CURRENT_USER
                            HKEY_LOCAL_MACHINE
                            HKEY_USERS
				SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
							'\' 로 시작하면 안됨
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
HKEY 
RUOpenKey(
    HKEY RootKey,
    const wchar_t* SubKey, 
    bool ReadOnly
    )
{
    HKEY hSubKey = NULL;
    REGSAM sam = (true == ReadOnly) ? KEY_READ : KEY_ALL_ACCESS;

    DWORD ret = RegOpenKeyExW(RootKey, SubKey, 0, sam, &hSubKey);
    if (ERROR_SUCCESS != ret)
    {
        log_err "RegOpenKeyExW(%ws) failed, ret = %u", SubKey, ret log_end        
        return NULL;
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
bool
RUCloseKey(
    HKEY Key
    )
{
    DWORD ret = RegCloseKey(Key);
    if (ERROR_SUCCESS != ret)
    {
        log_err "RegCloseKey() failed, ret = %u", ret log_end        
        return false;
    }

    return true;
}

/**----------------------------------------------------------------------------
    \brief	SubKey 를 생성한다. 이미 존재하는 경우 오픈한다.
    \param      RootKey     HKEY_CLASSES_ROOT
                            HKEY_CURRENT_CONFIG
                            HKEY_CURRENT_USER
                            HKEY_LOCAL_MACHINE
                            HKEY_USERS
				SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
							'\' 로 시작하면 안됨
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
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

/**----------------------------------------------------------------------------
    \brief  
    
    \param      RootKey     HKEY_CLASSES_ROOT
                            HKEY_CURRENT_CONFIG
                            HKEY_CURRENT_USER
                            HKEY_LOCAL_MACHINE
                            HKEY_USERS
				SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
							'\' 로 시작하면 안됨
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
DWORD 
RUReadDword(
    HKEY RootKey,
    const wchar_t* SubKey,
    const wchar_t* ValueName, 
    DWORD DefaultValue
    )
{
    DWORD value = DefaultValue;
    DWORD value_size = sizeof(value);

    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, true);
    if (NULL == sub_key_handle)
    {
        log_err "RUOpenKey(%ws) failed", SubKey log_end
        return DefaultValue;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

    DWORD ret = RegQueryValueExW(sub_key_handle, ValueName, NULL, NULL, (PBYTE)&value, &value_size);
    if (ERROR_SUCCESS != ret)
    {
        log_err "RegQueryValueExW(%ws) failed, ret = %u", SubKey, ret log_end
        return DefaultValue;
    }

    return value;
}


/** ---------------------------------------------------------------------------
    \brief  
    \param      RootKey     HKEY_CLASSES_ROOT
                            HKEY_CURRENT_CONFIG
                            HKEY_CURRENT_USER
                            HKEY_LOCAL_MACHINE
                            HKEY_USERS
				SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
							'\' 로 시작하면 안됨
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
bool 
RUWriteDword(
    HKEY RootKey,
    const wchar_t* SubKey,
    const wchar_t* ValueName, 
    DWORD Value
    )
{
    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, false);
    if (NULL == sub_key_handle)
    {
        log_err "RUOpenKey(%ws) failed", SubKey log_end        
        return false;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

    DWORD ret = RegSetValueExW(sub_key_handle, ValueName, 0, REG_DWORD, (PBYTE)&Value, sizeof(DWORD));
    if (ERROR_SUCCESS != ret)
    {
        log_err "RegSetValueExW(%ws) failed, ret = %u", ValueName, ret log_end
        return false;
    }

    return true;
}

/**
 * @brief	
 * @param	RootKey     HKEY_CLASSES_ROOT
                            HKEY_CURRENT_CONFIG
                            HKEY_CURRENT_USER
                            HKEY_LOCAL_MACHINE
                            HKEY_USERS
			SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
						'\' 로 시작하면 안됨
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool
RUReadString(
    IN HKEY				RootKey,
    IN const wchar_t*   SubKey,
    IN const wchar_t* 	ValueName,
	OUT std::wstring&	Value
    )
{
    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, true);
    if (NULL == sub_key_handle)
    {
        log_err "RUOpenKey(%ws) failed", SubKey log_end
        return false;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);
    void* old = NULL;	
    DWORD cbValue = 1024;
    wchar_t* buffer = (PWCHAR) malloc(cbValue);
    if (NULL == buffer) return false;
    RtlZeroMemory(buffer, cbValue);

    DWORD ret = RegQueryValueExW(
                        sub_key_handle, 
                        ValueName, 
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
            free(old); 

            cbValue = 0;
            return false;
        }
		RtlZeroMemory(buffer, cbValue);

        ret = RegQueryValueExW(
                        sub_key_handle, 
                        ValueName, 
                        NULL, 
                        NULL, 
                        (LPBYTE) buffer, 
                        &cbValue
                        );
    }

    if (ERROR_SUCCESS != ret)
    {
        // Value 가 없는 경우
        //
        //log_err "RegQueryValueExW(%ws) failed, ret = %u", ValueName, ret log_end
        free(buffer); buffer=NULL;
        return false;    
    }
    
	// buffer -> wstring 
	Value = buffer;
	free(buffer); buffer = NULL;
    return true;
}

/**
 * @brief	SubKey\ValueName 문자열 Value 를 생성한다. 만일 SubKey 가 존재하지 않는 다면 생성하고, 값을 생성한다.
 * @param	RootKey     HKEY_CLASSES_ROOT
                            HKEY_CURRENT_CONFIG
                            HKEY_CURRENT_USER
                            HKEY_LOCAL_MACHINE
                            HKEY_USERS
			SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
						'\' 로 시작하면 안됨
			cbValue		null 을 포함하는 문자열의 바이트 사이즈
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool
RUSetString(
    HKEY RootKey,
    const wchar_t* SubKey,
    const wchar_t* ValueName,
    const wchar_t* Value,	// byte buffer
    DWORD cbValue			// count byte
    )
{
    HKEY sub_key_handle = RUCreateKey(RootKey, SubKey, false);
    if (NULL == sub_key_handle)
    {
        log_err "RUCreateKey(%ws) failed", SubKey log_end        
        return false;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

    DWORD ret = RegSetValueExW(sub_key_handle,
							   ValueName, 
							   0, 
							   REG_SZ, 
							   (LPBYTE)Value, cbValue);
    if (ERROR_SUCCESS != ret)
    {
        //log_err "RegSetValueExW(%ws) failed, ret = %u", ValueName, ret log_end
        return false;
    }

    return true;
}

/**
 * @brief	
 * @param	RootKey     HKEY_CLASSES_ROOT
                            HKEY_CURRENT_CONFIG
                            HKEY_CURRENT_USER
                            HKEY_LOCAL_MACHINE
                            HKEY_USERS
			SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
						'\' 로 시작하면 안됨
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool
RUSetExpandString(
    HKEY RootKey,
    const wchar_t* SubKey,
    const wchar_t* value_name,
    const wchar_t* value,           // byte buffer
    DWORD cbValue          // count byte
    )
{
    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, false);
    if (NULL == sub_key_handle)
    {
        log_err "RUOpenKey(%ws) failed", SubKey log_end
        return false;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

    DWORD ret = RegSetValueExW(sub_key_handle, value_name, 0, REG_EXPAND_SZ, (LPBYTE)value, cbValue);
    if (ERROR_SUCCESS != ret)
    {
        log_err "RegSetValueExW(%ws) failed, ret = %u", value_name, ret log_end
        return false;
    }

    return true;
}

/**
 * @brief	
 * @param	RootKey     HKEY_CLASSES_ROOT
                        HKEY_CURRENT_CONFIG
                        HKEY_CURRENT_USER
                        HKEY_LOCAL_MACHINE
                        HKEY_USERS
			SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
						'\' 로 시작하면 안됨
 * @see		
 * @remarks	value 사이즈 제한에 관한 정보는 링크 참조
 *          https://msdn.microsoft.com/en-us/library/windows/desktop/ms724872(v=vs.85).aspx
 * @code		
 * @endcode	
 * @return	
**/
bool
RUSetBinaryData(
    HKEY RootKey,
    const wchar_t* SubKey,
    const wchar_t* value_name,
    const uint8_t* value,
    DWORD cbValue)
{
    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, false);
    if (NULL == sub_key_handle)
    {
        log_err "RUOpenKey(%ws) failed", SubKey log_end
        return false;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

    DWORD ret = RegSetValueExW(sub_key_handle, value_name, 0, REG_BINARY, (LPBYTE)value, cbValue);
    if (ERROR_SUCCESS != ret)
    {
        log_err "RegSetValueExW(%ws) failed, ret = %u", value_name, ret log_end
        return false;
    }

    return true;
}

/**
 * @brief	
 * @param	RootKey     HKEY_CLASSES_ROOT
                        HKEY_CURRENT_CONFIG
                        HKEY_CURRENT_USER
                        HKEY_LOCAL_MACHINE
                        HKEY_USERS
			SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
						'\' 로 시작하면 안됨
 * @see		
 * @remarks	caller must free returned buffer pointer.
 * @code		
 * @endcode	
 * @return	
**/
uint8_t*
RUReadBinaryData(
    _In_ HKEY RootKey,
    _In_ const wchar_t* SubKey,
    _In_ const wchar_t* value_name,
    _Out_ DWORD& cbValue
    )
{
    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, true);
    if (NULL == sub_key_handle)
    {
        log_err "RUOpenKey(%ws) failed", SubKey log_end
        return NULL;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);
    void* old = NULL;	
    cbValue = 1024;
    uint8_t* buffer = (uint8_t*) malloc(cbValue);
    if (NULL == buffer)
    {
        cbValue = 0;
        return NULL;
    }
    RtlZeroMemory(buffer, cbValue);

    DWORD ret = RegQueryValueExW(
                        sub_key_handle, 
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
            free(old); 

            cbValue = 0;
            return NULL;
        }
		RtlZeroMemory(buffer, cbValue);

        ret = RegQueryValueExW(
                        sub_key_handle, 
                        value_name, 
                        NULL, 
                        NULL,
                        (LPBYTE) buffer, 
                        &cbValue
                        );
    }

    if (ERROR_SUCCESS != ret)
    {
        // Value 가 없는 경우
        //
        //log_err "RegQueryValueExW(%ws) failed, ret = %u", ValueName, ret log_end
        free(buffer); buffer=NULL;
        return NULL;    
    }
    
	return buffer;
}


/**
 * @brief	
 * @param	RootKey     HKEY_CLASSES_ROOT
                            HKEY_CURRENT_CONFIG
                            HKEY_CURRENT_USER
                            HKEY_LOCAL_MACHINE
                            HKEY_USERS
			SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
						'\' 로 시작하면 안됨
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool
RUDeleteValue(
	_In_ HKEY RootKey,
	_In_ const wchar_t* SubKey,
	_In_ const wchar_t* ValueName
	)
{
	HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, false);
    if (NULL == sub_key_handle)
    {
        log_err "RUOpenKey(%ws) failed", SubKey log_end
        return false;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

	DWORD ret = RegDeleteValueW(sub_key_handle, ValueName);
	if (ERROR_SUCCESS != ret)
	{
		log_err "RegDeleteValueW( %ws ) failed. ret = %u", ValueName, ret log_end
		return false;
	}

	return true;
}

/**
 * @brief	key 를 삭제한다. 대상 키의 내부에 있는 value 들도 함께 삭제된다.
 * @param	RootKey     HKEY_CLASSES_ROOT
                            HKEY_CURRENT_CONFIG
                            HKEY_CURRENT_USER
                            HKEY_LOCAL_MACHINE
                            HKEY_USERS
			SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
						'\' 로 시작하면 안됨
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool 
RUDeleteKey(
	_In_ HKEY RootKey,
	_In_ const wchar_t* SubKey
	)
{
	DWORD ret = RegDeleteKeyW(RootKey, SubKey);
	if (ERROR_SUCCESS != ret)
	{
		log_err "RegDeleteKeyW( sub = %ws ) failed. ret = %u", SubKey, ret log_end
		return false;
	}

	return true;
}

/**
 * @brief	
 * @param	RootKey     HKEY_CLASSES_ROOT
                            HKEY_CURRENT_CONFIG
                            HKEY_CURRENT_USER
                            HKEY_LOCAL_MACHINE
                            HKEY_USERS
			SubKey		SOFTWARE\Microsoft\Windows\CurrentVersion\Run
						'\' 로 시작하면 안됨
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool
RUIsKeyExists(
    HKEY RootKey, 
    const wchar_t* TargetKey
    )
{
    RegHandle reg (RUOpenKey(RootKey, TargetKey, true));    
    return (NULL == reg.get()) ? false : true;
}


/// @brief  `key` 의 key, value 들을 enumerate 한다. 
///         sub key 는 `key_cb` 를 통해 caller 에게 전달되고, 
///         value 는 `value_cb` 를 통해 caller 에게 전달된다. 
///         key_cb, value_cb 는 NULL 일 수 있다. 
bool reg_enum_key_values(_In_ HKEY key, _In_ fn_key_callback key_cb, _In_ fn_value_callback value_cb)
{
    DWORD sub_key_count = 0;
    DWORD max_sub_key_name_cc = 0;
    DWORD max_class_name_cc = 0;
    DWORD value_count = 0;
    DWORD max_value_name_cc = 0;
    DWORD max_value_data_byte = 0;
    DWORD max_security_desciptor_byte = 0;
    FILETIME last_write_time = { 0 };

    // xxx_size 값은 UNICODE character 의 사이즈이며 
    // null terminate character 를 포함하지 않은 값이다. 

    LSTATUS ret = RegQueryInfoKeyW(
                    key,
                    NULL,
                    NULL,
                    NULL,
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


    // enum reg keyz
    if (NULL != key_cb)
    {
        if (sub_key_count > 0)
        {
            //log_dbg "sub key count = %u", sub_key_count log_end;
            wchar_t* sub_key_name = (wchar_t*)malloc((max_sub_key_name_cc)* sizeof(wchar_t));
            wchar_t* class_name = (wchar_t*)malloc((max_class_name_cc)* sizeof(wchar_t));

            for (DWORD i = 0; i < sub_key_count; ++i)
            {
                RtlZeroMemory(sub_key_name, (max_sub_key_name_cc)* sizeof(wchar_t));
                RtlZeroMemory(class_name, (max_class_name_cc)* sizeof(wchar_t));

                DWORD sub_key_name_cc = max_sub_key_name_cc;
                DWORD class_name_cc = max_class_name_cc;

                ret = RegEnumKeyEx(
                        key,
                        i,
                        sub_key_name,
                        &sub_key_name_cc,
                        NULL,
                        class_name,
                        &class_name_cc,
                        &last_write_time);
                if (ret == ERROR_SUCCESS)
                {
                    //log_dbg
                    //    "index = %u, sub_key_name = %ws, class_name = %ws",
                    //    i, sub_key_name, class_name
                    //    log_end;
                
                    if (true != key_cb(i, sub_key_name, class_name))
                    {
                        // caller canceled.
                        break;
                    }
                }
                else
                {
                    log_err "RegEnumKeyEx() failed. ret = %u", ret log_end;
                }
            }

            free_and_nil(sub_key_name);
            free_and_nil(class_name);
        }
    }

    // enum reg values
    if (NULL != value_cb)
    {
        if (value_count > 0)
        {
            //log_dbg "value count = %u", value_count log_end;
            wchar_t* value_name = (wchar_t*)malloc((max_value_name_cc)* sizeof(wchar_t));
            BYTE* value_data = (BYTE*)malloc(max_value_data_byte);

            for (DWORD i = 0; i < value_count; i++)
            {
                RtlZeroMemory(value_name, (max_value_name_cc)* sizeof(wchar_t));
                RtlZeroMemory(value_data, max_value_data_byte);

                DWORD value_type = REG_NONE;
                DWORD value_name_cc = max_value_name_cc;
                DWORD value_data_size = max_value_data_byte;
            
                ret = RegEnumValue(
                            key,
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
                    if (true != value_cb(i, value_type, value_name, value_data_size, value_data))
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