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

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383



HKEY 
RUOpenKey(
    HKEY    RootKey,
    LPCWSTR SubKey,
    bool    ReadOnly
    );

bool
RUCloseKey(
    HKEY Key
    );

HKEY
RUCreateKey(
    HKEY    RootKey,
    LPCWSTR SubKey,
    bool    ReadOnly
    );


DWORD 
RUReadDword(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   ValueName, 
    DWORD   DefaultValue
    );

bool 
RUWriteDword(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   ValueName, 
    DWORD   DefaultValue
    );


bool
RUReadString(
    IN HKEY				RootKey,
    IN LPCWSTR			SubKey,
    IN PCWCH			ValueName,
    OUT std::wstring&	Value
    );

bool
RUSetString(
    HKEY		RootKey,
    LPCWSTR		SubKey,
    PCWCH		ValueName,
    PCWCHAR		Value,           // byte buffer
    DWORD		cbValue          // count byte
    );

bool
RUSetExpandString(
    HKEY    RootKey,
    LPCWSTR SubKey,
    PCWCH   value_name,
    PWCHAR  value,           // byte buffer
    DWORD   cbValue          // count byte
    );


// 이건 그냥 string 으로 읽으면 됨
// 
//bool
//RUReadExpandString(
//    HKEY    key,
//    PCWCH   value_name,
//    PWCHAR  value,           // byte buffer
//    DWORD   cbValue          // count byte
//    )


bool
RUDeleteValue(
	_In_ HKEY		RootKey,
	_In_ LPCWSTR	SubKey,
	_In_ LPCWSTR	ValueName
	);

bool 
RUDeleteKey(
	_In_ HKEY		RootKey,
	_In_ LPCWSTR	SubKey
	);


bool
RUIsKeyExists(
    HKEY RootKey, 
    PCWCH TargetKey
    );

