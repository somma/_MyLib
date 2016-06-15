///////////////////////////////////////////////////////////////////////////////
// (C) Copyright 2004. Yoo Byung In.
//
// Any part of this source code can not be copied with
// any method without prior written permission from
// the author or authorized person.
//
// File Name	: Wow64Util.cpp
// Author		: Yoo, Byung In (hanlife7@hanmail.net)
// Descrtption	: implementation of the CWow64Util class.
//
// Revision History
// Date         Name								Description
// 2004-02-18   Yoo, Byung In (hanlife7@hanmail.net)  Created
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Wow64Util.h"

//////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////
CWow64Util::~CWow64Util()
{	
	if( m_hKernel32 ) FreeLibrary( m_hKernel32	);
	if( m_hUser32	) FreeLibrary( m_hUser32	);
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
CWow64Util::CWow64Util()
{	
	m_pfnGetNativeSystemInfo			= NULL;
	m_pfnIsWow64Process					= NULL;
	m_pfnWow64EnableWow64FsRedirection	= NULL;
	m_pfnGetSystemWow64Directory		= NULL;
	m_pfnIsWow64Message					= NULL;

	m_hKernel32	= LoadLibrary(	TEXT("Kernel32.DLL"));
	m_hUser32	= LoadLibrary(	TEXT("User32.DLL")	);	
	
	if( m_hKernel32 )
	{			
		m_pfnGetNativeSystemInfo			= (LPFN_GetNativeSystemInfo)GetProcAddress(m_hKernel32, "GetNativeSystemInfo");
		m_pfnIsWow64Process					= (LPFN_IsWow64Process)GetProcAddress(m_hKernel32, "IsWow64Process");
		m_pfnWow64EnableWow64FsRedirection	= (LPFN_Wow64EnableWow64FsRedirection)GetProcAddress(m_hKernel32, "Wow64EnableWow64FsRedirection");
		m_pfnGetSystemWow64Directory		= (LPFN_GetSystemWow64Directory)GetProcAddress(m_hKernel32, "GetSystemWow64Directory");
	}

	if( m_hUser32 )
	{
		m_pfnIsWow64Message					= (LPFN_IsWow64Message)GetProcAddress(m_hUser32, "IsWow64Message");
	}
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
BOOL CWow64Util::IsWow64Process( VOID )
{
	BOOL bRtVal = FALSE;

	if( m_pfnIsWow64Process )
	{
		m_pfnIsWow64Process(GetCurrentProcess(), &bRtVal);
	}

	return bRtVal;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
BOOL CWow64Util::IsWow64Process( HANDLE hProcess )
{
	BOOL bRtVal = FALSE;

	if( NULL == hProcess ) hProcess = GetCurrentProcess();

	if( m_pfnIsWow64Process)
	{
		m_pfnIsWow64Process(hProcess, &bRtVal);
	}

	return bRtVal;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
BOOL CWow64Util::IsWow64Process( DWORD pid )
{
	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (INVALID_HANDLE_VALUE == hProc)
	{
		return FALSE;
	}

	return IsWow64Process(hProc);
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
BOOL CWow64Util::IsWow64Message( )
{
	BOOL bRtVal = FALSE;

	if( m_pfnIsWow64Message)
	{
		bRtVal = m_pfnIsWow64Message();
	}

	return bRtVal;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
BOOL CWow64Util::IsProcessorIA64( )
{
	BOOL bRtVal = FALSE;
	SYSTEM_INFO SystemInfo;

	GetWow64SystemInfo( &SystemInfo );

	if( SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
	{
		bRtVal = TRUE;
	}


	return bRtVal;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
BOOL CWow64Util::IsProcessorAMD64( )
{
	BOOL bRtVal = FALSE;
	SYSTEM_INFO SystemInfo;

	GetWow64SystemInfo( &SystemInfo );

	if( SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
	{
		bRtVal = TRUE;
	}

	return bRtVal;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
BOOL CWow64Util::TurnOnWow64FSRedirection()
{
	BOOL bRtVal = FALSE;

	if( m_pfnWow64EnableWow64FsRedirection )
	{
		bRtVal = m_pfnWow64EnableWow64FsRedirection(TRUE);
	}

	return bRtVal;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
BOOL CWow64Util::TurnOffWow64FSRedirection()
{
	BOOL bRtVal = FALSE;
	
	if( m_pfnWow64EnableWow64FsRedirection )
	{
		bRtVal = m_pfnWow64EnableWow64FsRedirection(FALSE);
	}

	return bRtVal;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
UINT CWow64Util::GetWow64SystemDirectory( LPTSTR lpBuffer,  UINT uSize )
{
	UINT uRtVal = 0;
		
	if( m_pfnGetSystemWow64Directory )
	{			
		uRtVal = m_pfnGetSystemWow64Directory ( lpBuffer, uSize );		
	}
	
	return uRtVal;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
VOID CWow64Util::GetWow64SystemInfo( LPSYSTEM_INFO lpSystemInfo )
{
	if(m_pfnGetNativeSystemInfo)
	{
		m_pfnGetNativeSystemInfo(lpSystemInfo);
	}
}
//////////////////////////////////////////////////////////////////////
//	END OF SOURCE
//////////////////////////////////////////////////////////////////////