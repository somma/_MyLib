/**----------------------------------------------------------------------------
 * RegistryUtil.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 11:11:2011   0:10 created
**---------------------------------------------------------------------------*/
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383



HKEY 
RUOpenKey(
    HKEY    RootKey,
    LPCWSTR SubKey,
    BOOL    ReadOnly
    );

BOOL
RUCloseKey(
    HKEY Key
    );

HKEY
RUCreateKey(
    HKEY    RootKey,
    LPCWSTR SubKey,
    BOOL    ReadOnly
    );


DWORD 
RUReadDword(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   ValueName, 
    DWORD   DefaultValue
    );

BOOL 
RUWriteDword(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   ValueName, 
    DWORD   DefaultValue
    );


BOOL
RUReadString(
    IN HKEY         RootKey,
    IN LPCWSTR      SubKey,
    IN PCWCH        ValueName,
    IN OUT PWCHAR&  Value,
    IN OUT DWORD&   cbValue
    );

BOOL
RUSetString(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   ValueName,
    PWCHAR  Value,           // byte buffer
    DWORD   cbValue          // count byte
    );

BOOL
RUSetExpandString(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   value_name,
    PWCHAR  value,           // byte buffer
    DWORD   cbValue          // count byte
    );


// 이건 그냥 string 으로 읽으면 됨
// 
//BOOL
//RUReadExpandString(
//    HKEY    key,
//    PCWCH   value_name,
//    PWCHAR  value,           // byte buffer
//    DWORD   cbValue          // count byte
//    )


BOOL
RUIsKeyExists(
    HKEY RootKey, 
    PCWCH TargetKey
    );
