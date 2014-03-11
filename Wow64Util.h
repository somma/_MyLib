///////////////////////////////////////////////////////////////////////////////
// (C) Copyright 2004. Yoo Byung In.
//
// Any part of this source code can not be copied with
// any method without prior written permission from
// the author or authorized person.
//
// File Name	: Wow64Util.h
// Author		: Yoo, Byung In (hanlife7@hanmail.net)
// Descrtption	: interface for the CWow64Util class.
//
// Revision History
// Date         Name								Description
// 2004-02-18   Yoo, Byung In (hanlife7@hanmail.net)  Created
//
///////////////////////////////////////////////////////////////////////////////

#if !defined _Wow64Util_H_
#define _Wow64Util_H_

#include <Windows.h>

///////////////////////////////////////////////////////////////////
// Type Definition of WOW64 Util API Function Pointer
///////////////////////////////////////////////////////////////////
typedef VOID	(WINAPI *LPFN_GetNativeSystemInfo) ( LPSYSTEM_INFO lpSystemInfo );
typedef	BOOL	(WINAPI *LPFN_IsWow64Process) ( HANDLE hProcess, PBOOL Wow64Process );
typedef BOOLEAN (WINAPI *LPFN_Wow64EnableWow64FsRedirection) ( BOOLEAN Wow64FsEnableRedirection );
typedef	UINT	(WINAPI *LPFN_GetSystemWow64Directory) ( LPTSTR lpBuffer,  UINT uSize );
typedef	BOOL	(WINAPI	*LPFN_IsWow64Message) ( void );

///////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////
class CWow64Util  
{
	public:				
		CWow64Util();
		virtual ~CWow64Util();

		// Is..
		BOOL IsWow64Process();
		BOOL IsWow64Process( HANDLE hProcess );
		BOOL IsWow64Message();
		BOOL IsProcessorIA64();
		BOOL IsProcessorAMD64();

		// Redirection On/Off
		BOOL TurnOnWow64FSRedirection();
		BOOL TurnOffWow64FSRedirection();

		// Get Information
		UINT GetWow64SystemDirectory( LPTSTR lpBuffer,  UINT uSize );
		VOID GetWow64SystemInfo( LPSYSTEM_INFO lpSystemInfo );

	private:		
		//
		LPFN_GetNativeSystemInfo			m_pfnGetNativeSystemInfo;
		LPFN_IsWow64Process					m_pfnIsWow64Process;
		LPFN_Wow64EnableWow64FsRedirection	m_pfnWow64EnableWow64FsRedirection;
		LPFN_GetSystemWow64Directory		m_pfnGetSystemWow64Directory;
		LPFN_IsWow64Message					m_pfnIsWow64Message;

		//
		HMODULE	m_hKernel32;
		HMODULE	m_hUser32;
};

#endif
