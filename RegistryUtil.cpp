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

    DWORD ret = RegQueryValueExW(sub_key_handle, ValueName, 0, NULL, (PBYTE)&value, &value_size);
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
    IN const wchar_t* 			SubKey,
    IN const wchar_t* 			ValueName,
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
                        0, 
                        NULL, 
                        (LPBYTE) buffer, 
                        &cbValue
                        );
    while (ERROR_MORE_DATA  == ret)
    {
        cbValue *= 2;
        old = buffer;        // save pointer for realloc faild

        Value = (PWCHAR) realloc(buffer, cbValue);
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
                        0, 
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
RUSetString(
    HKEY RootKey,
    const wchar_t* SubKey,
    const wchar_t* ValueName,
    const wchar_t* Value,           // byte buffer
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

    DWORD ret = RegSetValueExW(sub_key_handle, ValueName, 0, REG_SZ, (LPBYTE)Value, cbValue);
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