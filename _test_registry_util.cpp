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
#include "RegistryUtil.h"

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
	if (true != RUWriteDword(HKEY_CURRENT_USER, L"_test_key", L"TestValue", 1000)) return false;	

	// write string 
	if (true != RUSetString(HKEY_CURRENT_USER, L"_test_key", L"TestValueString", L"abc", (DWORD)((wcslen(L"abc") + 1) * sizeof(wchar_t))) ) return false;
		
	
	// read dword value 
	if (1000 != RUReadDword(HKEY_CURRENT_USER, L"_test_key", L"TestValue", 0)) return false;

	std::wstring str;
	if (true != RUReadString(HKEY_CURRENT_USER, L"_test_key", L"TestValueString", str)) return false;
	if (0 != str.compare(L"abc")) return false;
	
	// Delete DWORD Value
	if (true != RUDeleteValue(HKEY_CURRENT_USER, L"_test_key", L"TestValue")) return false;

	// Delete key
	// -- key 내부의 value 들도 함께 삭제됨
	if (true != RUDeleteKey(HKEY_CURRENT_USER, L"_test_key")) return false;

	return true;
}