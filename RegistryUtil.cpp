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
#include "DebugMessage.h"

#include <crtdbg.h>
#include <stdlib.h>


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
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
HKEY 
RUOpenKey(
    HKEY    RootKey,
    LPCWSTR SubKey, 
    BOOL    ReadOnly
    )
{
    HKEY hSubKey = NULL;
    REGSAM sam = (TRUE == ReadOnly) ? KEY_READ : KEY_ALL_ACCESS;

    DWORD ret = RegOpenKeyExW(RootKey, SubKey, 0, sam, &hSubKey);
    if (ERROR_SUCCESS != ret)
    {
        DBG_ERR
            "RegOpenKeyExW(%ws) failed, ret=0x%08x", SubKey, ret
        DBG_END        
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
BOOL
RUCloseKey(
    HKEY Key
    )
{
    DWORD ret = RegCloseKey(Key);
    if (ERROR_SUCCESS != ret)
    {
        DBG_ERR
            "RegCloseKey() failed, ret=0x%08x", ret
        DBG_END        
        return FALSE;
    }

    return TRUE;
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
HKEY
RUCreateKey(
    HKEY    RootKey,
    LPCWSTR SubKey,
    BOOL    ReadOnly
    )
{
    DWORD ret = ERROR_SUCCESS;
    DWORD disposition=0;
    HKEY sub_key_handle = NULL;
    REGSAM sam = (TRUE == ReadOnly) ? KEY_READ : KEY_ALL_ACCESS;

    ret = RegCreateKeyExW(RootKey, SubKey, 0, NULL, 0, sam, NULL, &sub_key_handle, &disposition);
    if (ERROR_SUCCESS != ret)
    {
        DBG_ERR
            "RegCreateKeyExW(%ws) failed, ret=0x%08x", SubKey, ret
        DBG_END        
        return NULL;
    }
    else
    {
        return sub_key_handle;
    }    
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
DWORD 
RUReadDword(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   ValueName, 
    DWORD   DefaultValue
    )
{
    DWORD value = DefaultValue;
    DWORD value_size = sizeof(value);

    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, TRUE);
    if (NULL == sub_key_handle)
    {
        DBG_ERR "RUOpenKey(%ws) failed", SubKey DBG_END
        return DefaultValue;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

    DWORD ret = RegQueryValueExW(sub_key_handle, ValueName, 0, NULL, (PBYTE)&value, &value_size);
    if (ERROR_SUCCESS != ret)
    {
        DBG_ERR
            "RegQueryValueExW(%ws) failed, ret=0x%08x", SubKey, ret
        DBG_END

        return DefaultValue;
    }

    return value;
}


/** ---------------------------------------------------------------------------
    \brief  

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
BOOL 
RUWriteDword(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   ValueName, 
    DWORD   Value
    )
{
    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, FALSE);
    if (NULL == sub_key_handle)
    {
        DBG_ERR "RUOpenKey(%ws) failed", SubKey DBG_END        
        return FALSE;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

    DWORD ret = RegSetValueExW(sub_key_handle, ValueName, 0, REG_DWORD, (PBYTE)&Value, sizeof(DWORD));
    if (ERROR_SUCCESS != ret)
    {
        DBG_ERR
            "RegSetValueExW(%ws) failed, ret=0x%08x", ValueName, ret
        DBG_END
        return FALSE;
    }

    return TRUE;
}

/**----------------------------------------------------------------------------
    \brief  [ WARN ]
            리턴되는 Value 버퍼는 Caller 가 free 해 주어야 함
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
BOOL
RUReadString(
    IN HKEY         RootKey,
    IN LPCWSTR      SubKey,
    IN PCWCH        ValueName,
    IN OUT PWCHAR&  Value,
    IN OUT DWORD&   cbValue
    )
{
    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, TRUE);
    if (NULL == sub_key_handle)
    {
        DBG_ERR "RUOpenKey(%ws) failed", SubKey DBG_END
        return FALSE;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

    _ASSERTE(NULL == Value);
    if(NULL != Value) free(Value);

    void* old=NULL;

    cbValue = 1024;
    Value = (PWCHAR) malloc(cbValue);
    if (NULL == Value) return FALSE;
    RtlZeroMemory(Value, cbValue);

    DWORD ret = RegQueryValueExW(
                        sub_key_handle, 
                        ValueName, 
                        0, 
                        NULL, 
                        (LPBYTE) Value, 
                        &cbValue
                        );
    while (ERROR_MORE_DATA  == ret)
    {
        cbValue *= 2;
        old = Value;        // save pointer for realloc faild

        Value = (PWCHAR) realloc(Value, cbValue);
        if (NULL == Value)
        {
            free(old); 

            cbValue = 0;
            return FALSE;
        }

        ret = RegQueryValueExW(
                        sub_key_handle, 
                        ValueName, 
                        0, 
                        NULL, 
                        (LPBYTE) Value, 
                        &cbValue
                        );
    }

    if (ERROR_SUCCESS != ret)
    {
        // Value 가 없는 경우
        //
        //DBG_ERR "RegQueryValueExW(%ws) failed, ret=0x%08x", ValueName, ret DBG_END
        free(Value); Value=NULL;cbValue=0;
        return FALSE;    
    }
    
    return TRUE;
}

/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
BOOL
RUSetString(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   ValueName,
    PWCHAR  Value,           // byte buffer
    DWORD   cbValue          // count byte
    )
{
    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, FALSE);
    if (NULL == sub_key_handle)
    {
        DBG_ERR "RUOpenKey(%ws) failed", SubKey DBG_END        
        return FALSE;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

    DWORD ret = RegSetValueExW(sub_key_handle, ValueName, 0, REG_SZ, (LPBYTE)Value, cbValue);
    if (ERROR_SUCCESS != ret)
    {
        //DBG_ERR "RegSetValueExW(%ws) failed, ret=0x%08x", ValueName, ret DBG_END
        return FALSE;
    }

    return TRUE;
}

/** ---------------------------------------------------------------------------
    \brief  

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
BOOL
RUSetExpandString(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   value_name,
    PWCHAR  value,           // byte buffer
    DWORD   cbValue          // count byte
    )
{
    HKEY sub_key_handle = RUOpenKey(RootKey, SubKey, FALSE);
    if (NULL == sub_key_handle)
    {
        DBG_ERR "RUOpenKey(%ws) failed", SubKey DBG_END
        return FALSE;
    }

    // assign key handle
    //
    RegHandle rh(sub_key_handle);

    DWORD ret = RegSetValueExW(sub_key_handle, value_name, 0, REG_EXPAND_SZ, (LPBYTE)value, cbValue);
    if (ERROR_SUCCESS != ret)
    {
        DBG_ERR "RegSetValueExW(%ws) failed, ret=0x%08x", value_name, ret DBG_END
        return FALSE;
    }

    return TRUE;
}


/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
BOOL
RUIsKeyExists(
    HKEY RootKey, 
    PCWCH TargetKey
    )
{
    RegHandle reg (RUOpenKey(RootKey, TargetKey, TRUE));    
    return (NULL == reg.get()) ? FALSE : TRUE;
}