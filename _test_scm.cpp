/**
 * @file    _test_scm.cpp
 * @brief   unit test for `scm_context` class
 *
 * This file contains test code for `scm_context` class.
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2016.12.09 18:38 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "scm_context.h"

bool test_legacy_driver_service();
bool test_minifilter_service();


/// @brief	test function
bool test_scm_context()
{
	//if (!test_legacy_driver_service()) return false;
	if (!test_minifilter_service()) return false;

	return true;
}

/// @brief	test for legacy driver service using scm_context class
bool test_legacy_driver_service()
{
	scm_context scm(L"c:\\dbg\\scanner.sys", L"scanner", L"scanner", false, false);

	//
	// install service
	// 
	if (!scm.install_service())
	{
		log_err "%ws, install_service() failed.", scm.service_name() log_end;
		return false;
	}

	//
	// start service
	// 
	if (!scm.start_service())
	{
		log_err "%ws, start_service() failed.", scm.service_name() log_end;
		return false;
	}

	_pause;

	// 
	// stop service
	//
	if (!scm.stop_service())
	{
		log_err "%ws, stop_service() failed.", scm.service_name() log_end;
		return false;
	}

	//
	// un-install service
	//
	if (!scm.uninstall_service())
	{
		log_err "%ws, uninstall_service() failed.", scm.service_name() log_end;
		return false;
	}

	return true;
}

/// @brief	test for minifilter driver service using scm_context class
bool test_minifilter_service()
{
	scm_context scm(L"c:\\dbg\\blockRS.sys", 
					L"blockRS", 
					L"blockRS", 
					L"0", 
					0x00000000,
					false);

	const wchar_t* svc_name = scm.service_name();
	log_info "%ws", svc_name log_end;

	//
	// install service
	// 
	if (!scm.install_service())
	{
		log_err "%ws, install_service() failed.", scm.service_name() log_end;
		return false;
	}

	//
	// start service
	// 
	if (!scm.start_service())
	{
		log_err "%ws, start_service() failed.", scm.service_name() log_end;
		return false;
	}


	_pause;

	// 
	// stop service
	//
	if (!scm.stop_service())
	{
		log_err "%ws, stop_service() failed.", scm.service_name() log_end;
		return false;
	}

	//
	// un-install service
	//
	if (!scm.uninstall_service())
	{
		log_err "%ws, uninstall_service() failed.", scm.service_name() log_end;
		return false;
	}

	return true;
}






