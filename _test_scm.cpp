/**
 * @file    _test_scm.cpp
 * @brief   unit test for `scm_context` class
 *
 * This file contains test code for `scm_context` class.
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2016.12.09 18:38 created.
 * @copyright All rights reserved by Yonghwan, Noh.
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
	scm_context scm(L"c:\\dbg\\scanner.sys", L"scanner", L"scanner", false);

	//
	// install service
	// 
	if (!scm.install_driver())
	{
		log_err "%ws, install_driver() failed.", scm.service_name() log_end;
		return false;
	}

	//
	// start service
	// 
	if (!scm.start_driver())
	{
		log_err "%ws, start_driver() failed.", scm.service_name() log_end;
		return false;
	}

	_pause;

	// 
	// stop service
	//
	if (!scm.stop_driver())
	{
		log_err "%ws, stop_driver() failed.", scm.service_name() log_end;
		return false;
	}

	//
	// un-install service
	//
	if (!scm.uninstall_driver())
	{
		log_err "%ws, uninstall_driver() failed.", scm.service_name() log_end;
		return false;
	}

	return true;
}

/// @brief	test for minifilter driver service using scm_context class
bool test_minifilter_service()
{
	scm_context scm(L"c:\\dbg\\scanner.sys", 
					L"scanner", 
					L"scanner", 
					L"0", 
					0x00000000,
					false);

	//
	// install service
	// 
	if (!scm.install_driver())
	{
		log_err "%ws, install_driver() failed.", scm.service_name() log_end;
		return false;
	}

	//
	// start service
	// 
	if (!scm.start_driver())
	{
		log_err "%ws, start_driver() failed.", scm.service_name() log_end;
		return false;
	}


	_pause;

	// 
	// stop service
	//
	if (!scm.stop_driver())
	{
		log_err "%ws, stop_driver() failed.", scm.service_name() log_end;
		return false;
	}

	//
	// un-install service
	//
	if (!scm.uninstall_driver())
	{
		log_err "%ws, uninstall_driver() failed.", scm.service_name() log_end;
		return false;
	}

	return true;
}






