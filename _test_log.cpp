/**
 * @file    tests for log.h/cpp
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2018/12/08 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "log.h"


bool test_log_rotate_with_ext();
bool test_log_rotate_without_ext();


bool test_log_rotate()
{
	if (true != test_log_rotate_with_ext()) return false;
	if (true != test_log_rotate_without_ext()) return false;
	return true;
}


bool test_log_rotate_with_ext()
{
	//
	// init file log
	//
	std::wstringstream log_file_path;
	log_file_path
		<< get_current_module_dirEx()
		<< L"\\test_log_rotate.log";
	initialize_log(log_mask_all, 
				   log_level_debug, 
				   log_to_all, 
				   log_file_path.str().c_str(),
				   5,		// 로그 파일당 로그 5줄
				   5		// 로그 파일의 최대 갯수는 5개
				   );

	//
	// rotate log file few times for test
	//
	for (int i = 0; i < (5*5+1); ++i)
	{
		Sleep(300);
		log_info "log rotate test... %d", i log_end;
	}

	//
	// 
	//

	//
	// finalize file log
	//
	log_info "end log rotate test" log_end;
	finalize_log();	
	return true;
}

bool test_log_rotate_without_ext()
{
	//
	// init file log
	//
	std::wstringstream log_file_path;
	log_file_path
		<< get_current_module_dirEx()
		<< L"\\test_log_rotate_no_ext";
	initialize_log(log_mask_all,
				   log_level_debug,
				   log_to_all,
				   log_file_path.str().c_str(),
				   5,		// 로그 파일당 로그 5줄
				   5		// 로그 파일의 최대 갯수는 5개
	);

	//
	// rotate log file few times for test
	//
	for (int i = 0; i < (5 * 5 + 1); ++i)
	{
		Sleep(300);
		log_info "log rotate test... %d", i log_end;
	}

	//
	// 
	//

	//
	// finalize file log
	//
	log_info "end log rotate test" log_end;
	finalize_log();
	return true;
}