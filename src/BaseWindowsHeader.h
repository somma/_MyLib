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

// std::string 을 strsafe.h 보다 먼저 선언해줘야 워닝이 없음
// 
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <set>
#include <thread>
#include <memory>

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
//	include 순서 주의 (바꾸면 안됨)
// 
#include <winioctl.h>
#include <initguid.h>
