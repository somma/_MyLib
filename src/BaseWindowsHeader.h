/**----------------------------------------------------------------------------
 * BaseWindowsHeader.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 26:12:2011   17:53 created
**---------------------------------------------------------------------------*/
#pragma once

// std::string �� strsafe.h ���� ���� ��������� ������ ����
// 
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <set>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include <inttypes.h>
#include <intsafe.h>
#include <strsafe.h>
#include <sal.h>
#include <winsvc.h>


#ifdef _DEBUG
#include <crtdbg.h>
#endif//_DEBUG

//
//	include ���� ���� (�ٲٸ� �ȵ�)
// 
#include <winioctl.h>
#include <initguid.h>
