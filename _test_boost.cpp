/**----------------------------------------------------------------------------
 * _test_boost.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:25 14:30 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"

/**
 * @brief	boost::lexical_cast
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool boost_lexical_cast()
{
    // ���� -> ����
    //
	std::string s = boost::lexical_cast<std::string>(1024);
    std::cout << boost::lexical_cast<std::string>(1024) << std::endl;
    std::wcout << boost::lexical_cast<std::wstring>(1024) << std::endl;

    // ���ڿ� -> ����
    // 
    std::cout << boost::lexical_cast<float>("3.14") << std::endl;
    
    // Ÿ�� ��ȯ ����
    // 
    try
    {
        std::cout << boost::lexical_cast<int>("abcd") << std::endl;
    }
    catch (boost::bad_lexical_cast)
    {
        std::cout << "bad lexical cast" << std::endl;
    }

	return true;
}

/**
 * @brief		shared_ptr �� void * ó���ϱ� (HANDLE �� ������)
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void my_free(_In_ void* memory)
{
	if (NULL == memory) return;

	log_dbg "trying to free void pointer at 0x%p", memory log_end
	free(memory);
}

bool boost_shared_ptr_void()
{
	boost::shared_ptr< boost::remove_pointer<void*>::type > void_ptr( malloc(128), my_free );
	if (NULL == void_ptr.get()) return false;

	log_dbg "void pointer allocated at 0x%p", void_ptr.get() log_end
	return true;
}

/**
 * @brief	boost::shared_ptr �� HANDLE ó���ϱ� #1 (�̷��� ����ϸ� �ȵ�!!)
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool boost_shared_ptr_handle_01()
{	
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > file_handle(
		open_file_to_read(L"c:\\windows\\system32\\drivers\\etc\\hosts"), 
		CloseHandle
		);
	if (INVALID_HANDLE_VALUE == file_handle.get()) 
	{
		return false;
	}
	
	DWORD bytes_read=0;
	unsigned char buffer[128]={0};
	if (!ReadFile(file_handle.get(), buffer, 128, &bytes_read, NULL)) return false;
	log_dbg "ReadFile, buffer = %S", buffer log_end

	file_handle.reset();	//!

	//> CloseHandle() �� ȣ��Ǿ����״�... ReadFile() �� �����ؾ� �ϰ�, 
	//> GetLastError() == ERROR_INVALID_HANDLE �̾�� ��
	if (TRUE == ReadFile(file_handle.get(), buffer, 128, &bytes_read, NULL)) return false;
	if (ERROR_INVALID_HANDLE != GetLastError()) return false;
	
	return true;
}

/**
 * @brief	boost::shared_ptr �� HANDLE ó���ϱ� #2
 * @brief	custom destructor �̿��ϱ�
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void MyCloseHandle(_In_ HANDLE file_handle)
{
	//> invalid �� file_handle �� �Ѿ�� ���� �����Ƿ� �� üũ���־�� ��
	if (INVALID_HANDLE_VALUE == file_handle || NULL == file_handle) return;

	log_dbg 
		"file handle = 0x%08x is closing...", 
		file_handle
	log_end
	
	CloseHandle(file_handle);
}

bool boost_shared_ptr_handle_02()
{
	typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type > shared_handle;
	shared_handle file_handle (
		open_file_to_read(L"c:\\windows\\system32\\drivers\\etc\\hosts"), 
		MyCloseHandle
		);
	if (INVALID_HANDLE_VALUE == file_handle.get()) return false;

	DWORD bytes_read=0;
	unsigned char buffer[128]={0};
	if (!ReadFile(file_handle.get(), buffer, 128, &bytes_read, NULL)) return false;
	log_dbg "ReadFile, buffer = %S", buffer log_end

	file_handle.reset();	//!

	// MyCloseHandle() �� ȣ��Ǿ����״�... 
	//> ReadFile() �� �����ؾ� �ϰ�, ERROR_INVALID_HANDLE �̾�� ��
	if (TRUE == ReadFile(file_handle.get(), buffer, 128, &bytes_read, NULL)) return false;
	if (ERROR_INVALID_HANDLE != GetLastError()) return false;

	return true;
}

/**
 * @brief	boost::shared_ptr �� HANDLE ó���ϱ� #3
 * @brief	file_hanele �� NULL �� ��쿡 ���� ó��
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool boost_shared_ptr_handle_03()
{
	typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type > shared_handle;
	shared_handle file_handle (
		open_file_to_read(L"c:\\windows\\system32\\drivers\\etc\\hosts\\file_doesnot_exists"), 
		MyCloseHandle
		);
	if (INVALID_HANDLE_VALUE == file_handle.get()) 
	{
		file_handle.reset();
		return true;
	}

	return false;	// never reach here
}

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
bool boost_tuple()
{
	boost::tuple<bool, bool, std::string> _tuple;

	// tuple �� �� �����ϱ� (I)
	bool &is_timeout = boost::tuples::get<0>(_tuple);
	bool &io_status = boost::tuples::get<1>(_tuple);
	std::string &str = boost::tuples::get<2>(_tuple);
	is_timeout = true;
	io_status = false;
	str = "this is boost::tuple test";

	// tuple �κ��� �� �б�
	std::cout << "is_timeout=" << boost::tuples::get<0>(_tuple) << std::endl
			  << "io_status=" << boost::tuples::get<1>(_tuple) << std::endl
			  << "str =" << boost::tuples::get<2>(_tuple) << std::endl;


	// tuple �� �� �����ϱ� (II)
	boost::tuples::get<0>(_tuple) = true;
	boost::tuples::get<1>(_tuple) = false;
	boost::tuples::get<2>(_tuple) = "this is type2 assignment";
	std::cout << "is_timeout=" << boost::tuples::get<0>(_tuple) << std::endl
			  << "io_status=" << boost::tuples::get<1>(_tuple) << std::endl
			  << "str =" << boost::tuples::get<2>(_tuple) << std::endl;

	return true;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool boost_format()
{
	boost::wformat f = boost::wformat(L"msg = %s, int = %d, float = %f") % L"string..." % 100 % 3.14;

	// to cout
	std::wcout << L"f = " << f << std::endl;

	// to string
	std::wstring fs = f.str();
	std::wcout << L"fs = " << fs << std::endl;				

	return true;

}

/// @brief boost::hash
bool boost_hash()
{
    return true;
}

/// @brief	boost::fucntion �׽�Ʈ
bool test_boost_function()
{
	typedef boost::function<bool(int a, int b)> f_sum;

	// nullptr �� �ʱ�ȭ ����?
	//f_sum f0 = nullptr;				//<! �����Ͽ���

	// 0 ���δ� �ʱ�ȭ ����
	f_sum f1 = 0;
	log_info "f1.empty()=%s", f1.empty() ? "true" : "false" log_end;

	// �⺻ ������ ȣ���ϸ� empty
	f_sum f2 = f_sum();
	log_info "f2.empty()=%s", f2.empty() ? "true" : "false" log_end;

	// �ƹ��͵� �ʱ�ȭ ���ϸ� empty
	f_sum f3;
	log_info "f3.empty()=%s", f3.empty() ? "true" : "false" log_end;

	return true;
}
