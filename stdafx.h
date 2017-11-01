/**----------------------------------------------------------------------------
 * stdafx.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:25 13:37 created
**---------------------------------------------------------------------------*/
#pragma once


#pragma warning(disable: 4819)	// warning C4819: The file contains a character that cannot be represented in the current code page (949). Save the file in Unicode format to prevent data loss

#include "targetver.h"

#define BOOST_LIB_DIAGNOSTIC
#include "boost/lexical_cast.hpp"
#include "boost/type_traits.hpp"		// boost::remove_pointer
#include "boost/noncopyable.hpp"
#include "boost/format.hpp"
#include "boost/thread.hpp"
#include "boost/shared_ptr.hpp"
#include <sstream>
#include <list>
#include <string>
#include <set>
#include <iostream>

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <VersionHelpers.h>
#include <crtdbg.h>
#include <strsafe.h>

#include "Win32Utils.h"
#include "mini_test.h"

