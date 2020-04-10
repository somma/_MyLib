/**----------------------------------------------------------------------------
 * _mini_test.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:3:2 22:51 created
**---------------------------------------------------------------------------*/

#define assert_bool(expected_bool, return_bool_test_func) \
{ \
	ret = return_bool_test_func();	\
	if (expected_bool == ret) \
	{ \
		log_info "[pass] %s", #return_bool_test_func log_end \
		_pass_count ++; \
	} \
	else \
	{ \
		log_err "[fail] %s", #return_bool_test_func log_end \
		_fail_count ++; \
	} \
}

#define assert_bool_with_param(expected_bool, return_bool_test_func, param) \
{ \
	ret = return_bool_test_func(param);	\
	if (expected_bool == ret) \
	{ \
		log_info "[pass] %s", #return_bool_test_func log_end \
		_pass_count ++; \
	} \
	else \
	{ \
		log_err "[fail] %s", #return_bool_test_func log_end \
		_fail_count ++; \
	} \
}