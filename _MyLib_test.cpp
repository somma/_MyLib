/**----------------------------------------------------------------------------
 * _MyLib_test.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:25 13:34 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "process_tree.h"
#include "base64.h"
#include "rc4.h"
#include "thread_pool.h"
#include "md5.h"
#include "sha2.h"
#include "Win32Utils.h"
#include "send_ping.h"
#include <regex>
#include "wmi_client.h"
#include "nt_name_conv.h"
#include "crc64.h"

// _test_scm.cpp
extern bool test_scm_context();	

bool test_crc64();
bool test_canonicalize_file_name();
extern bool test_NtCreateFile();
extern bool test_wmi_client();
bool test_ping();
bool test_regexp();
bool test_device_name_from_nt_name();
bool test_rstrnicmp();
bool test_get_drive_type();
bool test_os_version();
bool test_for_each();

bool test_find_and_replace();

// disk, volume stuffs.
bool read_file_offset(_In_ HANDLE file_handle, _In_ uint64_t offset, _In_ uint8_t* buf, _In_ uint32_t size);
bool write_file_offset(_In_ HANDLE file_handle, _In_ uint64_t offset, _In_ uint8_t* buf, _In_ uint32_t size);
void dump_file_offset(_In_ HANDLE file_handle, _In_ uint64_t offset, _In_ uint32_t size);

bool test_enum_physical_drive();
bool test_get_disk_volume_info();
bool test_dump_xxx();
bool test_write_mbr_vbr();

// _test_asm.cpp
bool test_asm_func();

// _test_x64.cpp
bool test_x64_calling_convension();

bool test_2_complement();
bool test_print_64int();
bool test_std_string_find_and_substr();
bool test_to_lower_uppper_string();
//bool test_const_position();
bool test_initialize_string();
bool test_process_tree();     

bool test_base64();
bool test_random();
bool test_get_local_ip_list();
bool test_get_mac_address();
bool test_ip_to_str();

bool test_strtok();

// _test_cpp_test.cpp
bool test_cpp_class();


// win32utils.cpp
bool test_get_filepath_by_handle();
bool test_nt_name_to_dos_name();
bool test_query_dos_device();
bool test_bin_to_hex();
bool test_str_to_xxx();
bool test_set_get_file_position();
bool test_get_module_path();
bool test_dump_memory();
bool test_get_process_name_by_pid();
bool test_get_environment_value();

// rc4.cpp
bool test_rc4_encrypt();

// md5.cpp / sha2.cpp
bool test_md5_sha2();

// _test_boost_asio_timer.cpp
extern bool test_boost_asio_timer();

// _test_boost.cpp
extern bool boost_lexical_cast();
extern bool boost_shared_ptr_void();
extern bool boost_shared_ptr_handle_01();
extern bool boost_shared_ptr_handle_02();
extern bool boost_shared_ptr_handle_03();
extern bool boost_tuple();
extern bool boost_format();

// _test_boost_bind.cpp
extern bool boost_bind();
extern bool boost_bind2();
extern bool boost_bind3();
extern bool boost_bind4();
extern bool boost_bind5();

// _test_std_map.cpp
extern bool test_std_map();
extern bool test_map_plus_algorithm_1();
extern bool test_map_plus_algorithm_2();
extern bool test_map_plus_algorithm_3();
extern bool test_map_plus_algorithm_4();
extern bool test_std_unordered_map();
extern bool test_std_unordered_map_object();
extern bool test_unorded_map_test_move();


// _test_regstry_util.cpp
extern bool test_registry_util();
extern bool test_read_mouted_device();
extern bool test_set_binary_data();


// thread_pool.h
bool test_thread_pool();

// _test_boost_thread.cpp
extern bool test_boost_thread();

class aaa
{
public:
    aaa(bool value) : _value(value) { }
    virtual ~aaa() { con_info "..." log_end }

    void run() { con_info "%s", _value == true ? "Ture" : "False"  con_end;}
protected:
    bool _value;
};

class bbb : public aaa
{
public:
    bbb(bool value): aaa(value) { }
    virtual ~bbb() { }
    void run() { con_info "%s", _value == true ? "Ture" : "False"  con_end;}
};

class ccc : public bbb
{
public:
    ccc(bool value) : bbb(value) { }
    virtual ~ccc() { }
    void run() { con_info "%s", _value == true ? "Ture" : "False"  con_end;}
};






/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
int _tmain(int argc, _TCHAR* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

	bool ret = false;
	UINT32 _pass_count = 0;
	UINT32 _fail_count = 0;

    //ccc c(true);
    //c.run();
    
    //char* p = NULL;
    //p = (char*)realloc(p, 100);
    //p[0] = 1;

    
    boost::wformat f = boost::wformat(L"%s\\%s") % get_current_module_dirEx().c_str() % L"_MyLib_tst.log";
    if (true != initialize_log(log_level_debug, f.str().c_str())) return false;
    set_log_format(false, false, false);
    
    
    //std::wstring wstr = L"12345";
    //con_info "wstr.size() = %u, wcslen(wstr.c_str() = %u",
    //    wstr.size(), wcslen(wstr.c_str())
    //    log_end;



    //FILETIME ft1, ft2;
    //GetSystemTimeAsFileTime(&ft1);
    //Sleep(1000);
    //GetSystemTimeAsFileTime(&ft2);
    //con_info "delta = %u", file_time_delta_sec(ft2, ft1) con_end;

	assert_bool(true, test_scm_context);


    //assert_bool(true, test_regexp);
    //assert_bool(true, test_ping);
    //assert_bool(true, test_canonicalize_file_name);
    //assert_bool(true, test_crc64);
    
    //assert_bool(true, test_wmi_client);
    //assert_bool(true, test_NtCreateFile);
    //assert_bool(true, test_device_name_from_nt_name);
    //assert_bool(true, test_rstrnicmp);

    //assert_bool(true, test_get_drive_type);
    //assert_bool(true, test_os_version);

    //assert_bool(true, test_boost_thread);
	//assert_bool(true, test_thread_pool);
    
    //assert_bool(true, test_boost_asio_timer);
	//assert_bool(true, test_for_each);
    //assert_bool(true, test_enum_physical_drive);
    //assert_bool(true, test_get_disk_volume_info);
    //assert_bool(true, test_dump_xxx);
    //assert_bool(true, test_write_mbr_vbr);
	//assert_bool(true, test_asm_func);
	//assert_bool(true, test_x64_calling_convension);
	//assert_bool(true, test_2_complement);
	//assert_bool(true , test_print_64int);
	//assert_bool(true, test_std_string_find_and_substr);
	//assert_bool(true, test_to_lower_uppper_string);
	////assert_bool(true, test_const_position);		// 컴파일 불가 테스트
	//assert_bool(true, test_initialize_string);
	//assert_bool(true, test_process_tree);
	//assert_bool(true, test_base64);
	//assert_bool(true, test_random);
	//assert_bool(true, test_get_local_ip_list);
    //assert_bool(true, test_get_mac_address);
    //assert_bool(true, test_ip_to_str);

    //assert_bool(true, test_strtok);

	//assert_bool(true, test_cpp_class);
	
	//assert_bool(true, test_nt_name_to_dos_name);
	//assert_bool(true, test_query_dos_device);
	//assert_bool(true, test_get_filepath_by_handle);
	//assert_bool(true, test_bin_to_hex);
	//assert_bool(true, test_str_to_xxx);
	//assert_bool(true, test_set_get_file_position);
	//assert_bool(true, test_get_module_path);
	//assert_bool(true, test_dump_memory);
	//assert_bool(true, test_get_process_name_by_pid);
	//assert_bool(true, test_get_environment_value);
 
	//assert_bool(true, test_rc4_encrypt);
    //assert_bool(true, test_md5_sha2);
	//
	//assert_bool(true, boost_lexical_cast);
	//assert_bool(true, boost_shared_ptr_void);
	//assert_bool(true, boost_shared_ptr_handle_01);
	//assert_bool(true, boost_shared_ptr_handle_02);
	//assert_bool(true, boost_shared_ptr_handle_03);
	//assert_bool(true, boost_tuple);

	//assert_bool(true, boost_format);

	//assert_bool(true, boost_bind);
	//assert_bool(true, boost_bind2);
	//assert_bool(true, boost_bind3);
	//assert_bool(true, boost_bind4);
	//assert_bool(true, boost_bind5);

	//assert_bool(true, test_std_map);
	//assert_bool(true, test_map_plus_algorithm_1);
	//assert_bool(true, test_map_plus_algorithm_2);
	//assert_bool(true, test_map_plus_algorithm_3);
	//assert_bool(true, test_map_plus_algorithm_4);
    //assert_bool(true, test_std_unordered_map);
    //assert_bool(true, test_std_unordered_map_object);
    //assert_bool(true, test_unorded_map_test_move);


	//assert_bool(true, test_registry_util);
    //assert_bool(true, test_read_mouted_device);
    //assert_bool(true, test_set_binary_data);
    

	con_info
		"----------------------------------------------------"
	log_end

	con_info
		"total test = %u, pass = %u, fail = %u", 
		_pass_count + _fail_count, 
		_pass_count, 
		_fail_count
	log_end

	con_info "press any key to terminate..." con_end
	_pause;
	return 0;
}

/// @brief
bool test_get_drive_type()
{
    UINT drive_type = GetDriveTypeW(L"c:\\");
    drive_type = GetDriveTypeW(L"\\Device\\HarddiskVolume1");
    drive_type = GetDriveTypeW(L"c");
    drive_type = GetDriveTypeW(L"\\Device\\HarddiskVolume1\\");
    drive_type = GetDriveTypeW(L"d:\\");
    drive_type = GetDriveTypeW(L"e:\\");
    drive_type = GetDriveTypeW(L"f:\\");

    return true;
}

/// @brief
bool test_os_version()
{
    OSVER os = get_os_version();
    con_info "%ws", osver_to_str(os) log_end;
    return true;
}

/// @brief
bool test_find_and_replace()
{
    std::string src = "0123456789,Version=v4,5";
    std::string find = ",";
    std::string replace = " ";
    
    
    con_info "before find_and_replace, %s", src.c_str() log_end;
    std::string str_mod = find_and_replace_string_exa(src.c_str(), ",", "\\,");
    _ASSERTE(0 == str_mod.compare("0123456789\\,Version=v4\\,5"));
    con_info "after find_and_replace, %s", str_mod.c_str() log_end;
    
    std::wstring srcw = L"0123456789,Version=v4,5";
    con_info "before find_and_replace, %ws", srcw.c_str() log_end;
    std::wstring str_modw = find_and_replace_string_exw(srcw.c_str(), L",", L"\\,");
    _ASSERTE(0 == str_modw.compare(L"0123456789\\,Version=v4\\,5"));
    con_info "after find_and_replace, %ws", str_mod.c_str() log_end;

    return true;
}

/**
 * @brief	 std::for_each, lambda expression
**/ 

// functor that overrides () opeator.
struct Sum 
{
    Sum() { sum = 0; }
    void operator()(int n) { sum += n; }
 
    int sum;
};

bool test_for_each()
{
	std::vector<int> nums;
	for(int i = 0; i < 11; ++i)
	{
		nums.push_back(i);
	}

	std::for_each(
		    nums.begin(), 
		    nums.end(), 
		    [](int& num)
		    {
			    printf("%d\n",num);
		    }
		    );

	Sum s = std::for_each(
		    nums.begin(), 
		    nums.end(),
		    Sum()
		    );

	printf("sum of nums = %u\n", s.sum);

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
bool test_std_string_find_and_substr()
{
	std::wstring nt_name(L"\\Device\\HarddiskVolume1\\Windows\\system32\\drivers");
	std::wstring dos_device_name(L"c:");
	std::wstring nt_device_name(L"\\Device\\HarddiskVolume1");
	std::wstring nt_device_name2(L"\\DEVICE\\HarddiskVolume1");		// 대문자


	//> 대소문자가 일치하는 경우 string::find 가 정상 동작 함
	size_t pos = nt_name.find(nt_device_name);
	if (std::wstring::npos == pos) return false;

	std::wstring out = dos_device_name + 
						nt_name.substr(pos + nt_device_name.size(), nt_name.size());
	
	log_dbg
		"\nnt_name = %ws \ndos_device_name = %ws \nnt_device_name = %ws \nresult = %ws",
		nt_name.c_str(),
		dos_device_name.c_str(),
		nt_device_name.c_str(),
		out.c_str()
	log_end

	//> 대소문자 구분 없이 find 하려면 win32util::to_lower_string() 호출 후 비교해야 함
	pos = nt_name.find(nt_device_name2);
	if (std::wstring::npos == pos) return true;

	return true;
}

/**
 * @brief	2's complement
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool test_2_complement()
{
	int i = -1;

	// 결과: %d = -1, %x = ffffffff
	// 1 = 0000 0001
	//     1111 1110 + 1 (음수를 표현하기 위해 2의 보수를 취하면...)
	//     1111	1111		= -1 = 0xff
	// 
	// 2 = 0000 0010
	//     1111 1101 + 1
	//     1111 1110		= -2 = 0xfe
	//
	// 3 = 0000 0011
	//     1111 1100 + 1
	//     1111 1101		= -3 = 0xfd
	log_dbg "%%d = %d, %%x = %x", i, i log_end

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
bool test_print_64int()
{
	uint64_t val = 0xffffffffffffffff;
	log_dbg "%%I64d = %I64d, %%I64u = %I64u, %%I64x = %I64x", val, val, val log_end

	// %I64d = -1, %I64u = 18446744073709551615, %I64x = ffffffffffffffff

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
bool test_to_lower_uppper_string()
{
	std::wstring str = L"ABCDEFGh1234";	
	log_dbg "str = %s", WcsToMbsEx(str.c_str()).c_str() log_end
	to_lower_string(str);
	log_dbg "after to_lower, str = %s", WcsToMbsEx(str.c_str()).c_str() log_end

	to_upper_string(str);
	log_dbg "after to_upper, str = %s", WcsToMbsEx(str.c_str()).c_str() log_end

	return true;
}

/**
 * @brief	const 위치 / 의미 
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
/*
class ConstPositionTest
{
public:	
	//> (const char*) msg : char* 가 const, 즉 msg 가 가리키는 데이터 변경 불가
    char* Function1(const char* msg)  
    {
        msg[0] = 't'; // error
        return m_msg;
    }

	//> char* (const msg) : msg 변수가 const, 즉 msg 포인터 변수 변경 불가
    char* Function2(char* const msg)  
    {    
        msg = m_msg; //error
        return m_msg;
    }

	//> 메소드 상수화, 이 메소드는 클래스 멤버를 읽을 수는 있으나 변경 할 수는 없음
    char* Function3(char* msg) const 
    {
        m_msg = msg; //error
        return m_msg;  
    }

	//> (const char*) : 리턴 값이 const char* 이므로 리턴 받는 변수도 const char* 이어야 함
	//> 따라서 리턴되는 포인터가 가리키는 데이터 변경 불가
    const char* Function4(char* msg)  
    {
        m_msg = msg;
        return m_msg; //반환 받는 타입이 const가 아닐 경우 error
    }
private:
    char* m_msg;
};

bool test_const_position()
{
	ConstPositionTest test;
	char msg[] = "hello, const!";
 
	 test.Function1(msg);
	 test.Function2(msg);
	 test.Function3(msg);

	 const char* pMessage = test.Function4(msg);
	 pMessage[0] = 0; // error
}
*/

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool test_initialize_string()
{
	std::wstring str = L"";
	log_dbg "str = %ws", str.c_str() log_end

	//> invalid null point exception 발생 
	//> try-except 로 못 잡음... 
	//> 초기화시 NULL 이면 "" 로 바꿔서 초기화 해야 함
/*
	try
	{
		std::wstring str2 = NULL;
		log_dbg "str2 = %s", str2.c_str() log_end
	}
	catch (...)
	{
		con_err "oops" log_end
	}
*/	
	
	return true;
}

/**
 * @brief	test for cprocess_tree class 
			
			테스트를 위해서는 
			cmd.exe -> procexp.exe -> procexp64.exe(자동으로 만들어짐) -> notepad.exe
			순서로 프로세스를 생성해 두고 해야 한다. 
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool proc_tree_callback(_In_ process& process_info, _In_ DWORD_PTR callback_tag)
{
    UNREFERENCED_PARAMETER(callback_tag);
	con_info "pid = %u, %ws", process_info.pid(), process_info.process_name() log_end
	return true;
}

bool test_process_tree()
{
	cprocess_tree proc_tree;
	if (!proc_tree.build_process_tree()) return false;

	// 프로세스 열거 테스트 (by callback)
	proc_tree.iterate_process(proc_tree_callback, 0);
	proc_tree.iterate_process_tree(proc_tree.find_process(L"cmd.exe"), proc_tree_callback, 0);
	
	// print 
	proc_tree.print_process_tree(L"cmd.exe");

	// 프로세스 종료 테스트	
	proc_tree.kill_process_tree( proc_tree.find_process(L"cmd.exe") );	

    proc_tree.print_process_tree(L"explorer.exe");

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
bool test_base64()
{
// http://www.opinionatedgeek.com/dotnet/tools/base64encode/
#define _base64_encoded	"64yA7ZWc66+86rWt"

	std::wstring string_to_encodeW = L"대한민국";
	std::string string_to_encodeA = "대한민국";

	std::wstring wide_str;
	std::string utf8_str;
	std::string base64_str;
	
	// base 64 encode
	// 
	// #1) multibyte -> ucs16 -> utf8 -> base64 순서로...
	// #2) ucs16 -> utf8 -> base64 
	wide_str = MbsToWcsEx(string_to_encodeA.c_str());
	utf8_str = WcsToMbsUTF8Ex(wide_str.c_str());
	base64_str = base64_encode((unsigned char*)utf8_str.c_str(), (int)utf8_str.size());
	if (0 != base64_str.compare(_base64_encoded)) return false;

	wide_str = string_to_encodeW;
	utf8_str = WcsToMbsUTF8Ex(wide_str.c_str());
	base64_str = base64_encode((unsigned char*)utf8_str.c_str(), (int)utf8_str.size());
	if (0 != base64_str.compare(_base64_encoded)) return false;
	
	// base64 decode	
	std::string f = base64_decode(_base64_encoded);
	wide_str = Utf8MbsToWcsEx(f.c_str());
	if (0 != wide_str.compare(string_to_encodeW.c_str())) return false;
	
	return true;
}

/**
 * @brief	
**/
bool test_random()
{
	int var = rand() % 1000 + 1;
    con_info "var = %d", var log_end;
	return true;
}

/**
 * @brief	
**/
bool test_get_local_ip_list()
{
	std::wstring host_name;
	std::vector<std::wstring> ip_list;
	if (true != get_local_ip_list(host_name, ip_list)) return false;

	con_info "host_name = %ws", host_name.c_str() log_end
	/*std::vector<std::wstring>::iterator its = ip_list.begin();
	std::vector<std::wstring>::iterator ite = ip_list.end();
	for(; its != ite; ++its)
	{
		con_info "ip = %ws", its->c_str() log_end
	}*/
    std::for_each(
        ip_list.begin(), 
        ip_list.end(),
		[](std::wstring& ip)
		    {
			    con_info "ip = %ws", ip.c_str() log_end
		    }
		);


	return true;
}

/// @brief
bool test_get_mac_address()
{
    std::wstring host_name;
    std::vector<std::wstring> ip_list;

    if (true != get_local_ip_list(host_name, ip_list)) return false;
    con_info "hot name = %ws", host_name.c_str() log_end;

    for (auto ip : ip_list)
    {
        std::wstring mac_str;
        
        if (true != get_local_mac_by_ipv4(ip.c_str(), mac_str)) return false;

        con_info "ip = %ws, mac = %ws", ip.c_str(), mac_str.c_str() log_end;
    }
    return true;
}

/// @brief
bool test_ip_to_str()
{
    const wchar_t* ip_str = L"211.221.93.88";
 
    in_addr addr = { 0 };
    if (true != str_to_ipv4(ip_str, addr)) return false;
    con_info "ip = %ws -> %lu", ip_str, addr.S_un.S_addr log_end;
    con_info "ip = %lu -> %ws", addr.S_un.S_addr, ipv4_to_str(addr).c_str() log_end;


    addr.S_un.S_addr = 0x0100007f;
    con_info "ip = %lu -> %ws", addr.S_un.S_addr, ipv4_to_str(addr).c_str() log_end;
    return true;
}

/// @brief
bool split_string(_In_ const char* str, _In_ const char* seps, _Out_ std::vector<std::string>& tokens)
{
#define max_str_len     2048

    _ASSERTE(NULL != str);
    if (NULL == str) return false;

    tokens.clear();

    // strtok_s() modifies the `str` buffer.
    // so we should make copy.
    size_t len = strlen(str);
    if (max_str_len <= len + sizeof(char))
    {
        return false;
    }
    
    char* buf = (char*)malloc(len + sizeof(char));
    if (NULL == buf)
    {
        return false;
    }

    RtlCopyMemory(buf, str, len);
    buf[len] = 0x00;    

    char* next_token = NULL;
    char* token = strtok_s(buf, seps, &next_token);
    while (NULL != token)
    {
        tokens.push_back(token);
        token = strtok_s(NULL, seps, &next_token);
    }
    
    return true;
}

bool test_strtok()
{
    char string1[] = "A string\tof ,,tokens\nand some  more tokens";
    char string2[] = "Another string\n\tparsed at the same time.";
    char seps[] = " ,\t\n";

    std::vector<std::string> tokens1;
    std::vector<std::string> tokens2;
    
    // Establish string and get the first token:
    if (false == split_string(string1, seps, tokens1) ||
        false == split_string(string2, seps, tokens2))
    {
        return false;
    }

    printf("tokens1 : %s\n\n", string1);
    for (auto token: tokens1)
    {
        printf("\t%s\n", token.c_str());
    }
    printf("\n");


    printf("tokens2 : %s\n\n", string2);
    for (auto token : tokens2)
    {
        printf("\t%s\n", token.c_str());
    }
    printf("\n");

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
bool test_get_filepath_by_handle()
{
	typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type > shared_file_handle;
	shared_file_handle file_handle(
							open_file_to_read(L"c:\\windows\\system32\\drivers\\etc\\hosts"), 
							CloseHandle
							);
	if(INVALID_HANDLE_VALUE == file_handle.get()) return false;
	std::wstring file_path;
	bool ret = get_filepath_by_handle(file_handle.get(), file_path);
	if (true == ret)
	{
		log_dbg "file path = %s", file_path.c_str() log_end
	}

	return ret;
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
bool test_nt_name_to_dos_name()
{
	WCHAR*	nt_name = L"\\Device\\HarddiskVolume2\\Windows\\System32\\drivers\\etc\\hosts";
	std::wstring dos_name;

	bool ret = nt_name_to_dos_name(nt_name, dos_name);
	if (true == ret) 
	{
		log_dbg 
			"nt_name = %S -> dos_name = %S", 
			nt_name,
			dos_name.c_str()
		log_end
	}
	return ret;
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
bool test_query_dos_device()
{
	wchar_t* dos_device = L"c:";
	std::wstring nt_device;

	bool ret = query_dos_device(dos_device, nt_device);
	if (true == ret) 
	{
		log_dbg 
			"dos_device( %S ) -> nt_device ( %S )", 
			dos_device, 
			nt_device.c_str()
		log_end
	}

	return ret;
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
bool test_bin_to_hex()
{
	/*
	01003689 > $ E8 C5F9FFFF    CALL notepad.01003053
	0100368E   . 6A 58          PUSH 58
	01003690   . 68 A0370001    PUSH notepad.010037A0
	01003695   . E8 72040000    CALL notepad.01003B0C
	0100369A   . 33DB           XOR EBX,EBX
	0100369C   . 895D E4        MOV DWORD PTR SS:[EBP-1C],EBX
	0100369F   . 895D FC        MOV DWORD PTR SS:[EBP-4],EBX
	010036A2   . 8D45 98        LEA EAX,DWORD PTR SS:[EBP-68]
	010036A5   . 50             PUSH EAX                                 ; /pStartupinfo
	010036A6   . FF15 FC100001  CALL DWORD PTR DS:[<&KERNEL32.GetStartup>; \GetStartupInfoA
	=>
	E8C5F9FFFF6A5868A0370001E87204000033DB895DE4895DFC8D459850FF15FC100001
	*/
	const char* orgau = "E8C5F9FFFF6A5868A0370001E87204000033DB895DE4895DFC8D459850FF15FC100001";
	const char* orgal = "e8c5f9ffff6a5868a0370001e87204000033db895de4895dfc8d459850ff15fc100001";
	const wchar_t* orgwu = L"E8C5F9FFFF6A5868A0370001E87204000033DB895DE4895DFC8D459850FF15FC100001";
	const wchar_t* orgwl = L"e8c5f9ffff6a5868a0370001e87204000033db895de4895dfc8d459850ff15fc100001";
	
	UINT8 code[] = {
					0xE8, 0xC5, 0xF9, 0xFF, 
					0xFF, 0x6A, 0x58, 0x68, 
					0xA0, 0x37, 0x00, 0x01, 
					0xE8, 0x72, 0x04, 0x00, 
					0x00, 0x33, 0xDB, 0x89, 
					0x5D, 0xE4, 0x89, 0x5D, 
					0xFC, 0x8D, 0x45, 0x98, 
					0x50, 0xFF, 0x15, 0xFC, 
					0x10, 0x00, 0x01		
					};

	std::string hexa;
	if (true != bin_to_hexa(sizeof(code), code, true, hexa)) return false;	
	if (0 != hexa.compare(orgau)) return false;
	
	if (true != bin_to_hexa(sizeof(code), code, false, hexa)) return false;	
	if (0 != hexa.compare(orgal)) return false;
	
	std::wstring hexw;
	if (true != bin_to_hexw(sizeof(code), code, true, hexw)) return false;	
	if (0 != hexw.compare(orgwu)) return false;

	if (true != bin_to_hexw(sizeof(code), code, false, hexw)) return false;	
	if (0 != hexw.compare(orgwl)) return false;
	
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
bool test_str_to_xxx()
{
	INT32	int32 = 0;
	UINT32	uint32 = 0;
	INT64	int64 = 0;
	UINT64	uint64 = 0;


	//>
	//> str_to_int32()
	//>
	if (true != str_to_int32("-1", int32)) return false;
	if (-1 != int32) return false;

	// NULL
	if (false != str_to_int32(NULL, int32)) return false;

	// invalid str = return true & 0
	if (true != str_to_int32("invalid str", int32)) return false;
	if (0 != int32) return false;

	// -0 
	if (true != str_to_int32("-0", int32)) return false;
	if (0 != int32) return false;

	// INT_MAX + 1 
	if (false != str_to_int32("2147483648", int32)) return false;

	// INT_MIN -1 => (-2147483647 - 1) -1 = -2147483649
	if (false != str_to_int32("-2147483649", int32)) return false;



	//>
	//> str_to_uint32()
	//>

	if (true != str_to_uint32("101010", uint32)) return false;
	if (101010 != uint32) return false;
	
	// -1
	if (false != str_to_uint32("-1", uint32)) return false;
	
	// NULL
	if (false != str_to_uint32(NULL, uint32)) return false;

	// invalid str = return true & 0
	if (true != str_to_uint32("invalid str", uint32)) return false;
	if (0 != uint32) return false;
			
	// UINT_MAX + 1	
	if (false != str_to_uint32("4294967296", uint32)) return false;



	
	//>
	//> str_to_int64()
	//>

	if (true != str_to_int64("-1", int64)) return false;
	if (-1 != int64) return false;

	// NULL
	if (false != str_to_int64(NULL, int64)) return false;

	// invalid str = return true & 0
	if (true != str_to_int64("invalid str", int64)) return false;
	if (0 != int64) return false;

	// -0 
	if (true != str_to_int64("-0", int64)) return false;
	if (0 != int64) return false;

	//INT64_MAX (9223372036854775807) + 1 = ffffffff`ffffffff
	if (false != str_to_int64("9223372036854775808", int64)) return false;

	//INT64_MIN (-9223372036854775807 -1) -1 = 9223372036854775809
	if (false != str_to_int64("-9223372036854775809", int64)) return false;

	

	//>
	//> str_to_uint64()
	//>

	if (false != str_to_uint64("-1", uint64)) return false;
	
	// NULL
	if (false != str_to_uint64(NULL, uint64)) return false;

	// invalid str = return true & 0
	if (true != str_to_uint64("invalid str", uint64)) return false;
	if (0 != uint64) return false;
		
	// UINT64_MAX (0xffffffff`ffffffff) + X
	if (false != str_to_uint64("9999999223372036854775807", uint64)) return false;
	

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
static std::wstring _test_file;

void _CloseHandle(_In_ HANDLE handle)
{
	if (NULL == handle || INVALID_HANDLE_VALUE == handle) return;
	CloseHandle(handle);


	DeleteFileW(_test_file.c_str());
}

bool test_set_get_file_position()
{
	if (!get_current_module_dir(_test_file)) return false;
	_test_file += L"\\testfile.dat";

	raii_handle file_handle(
					open_file_to_write(_test_file.c_str()), 
					_CloseHandle
					);
	if (NULL == file_handle.get()) return false;

	DWORD bytes_written = 0;
	for(int i = 0; i < 254; ++i)
	{
		if (!WriteFile(file_handle.get(), &i, 1, &bytes_written, NULL)) return false;
	}

	uint64_t pos = 0;
	uint64_t new_pos = 0;
	
	// get file position
	if (true != get_file_position(file_handle.get(), pos)) return false;
	if (254 != pos) return false;

	// set file position
	pos = 128;
	if (true != set_file_position(file_handle.get(), pos, &new_pos)) return false;
	if (true != get_file_position(file_handle.get(), pos)) return false;

	if (128 != pos) return false;
	return true;
}

/**
 * @brief	
**/
bool test_get_module_path()
{
	std::wstring module_path;
	if (true != get_module_path(L"ntdll.dll", module_path)) return false;

	to_lower_string(module_path);	
	if (0 != module_path.compare(L"c:\\windows\\system32\\ntdll.dll")) return false;
	return true;
}

/**
 * @brief	
**/
bool test_dump_memory()
{
	unsigned char buf[512] = {0};
	RtlCopyMemory(buf, GetModuleHandle(NULL), 512);

	std::vector<std::string> dump;
	if (true != dump_memory(0, buf, sizeof(buf), dump)) return false;

	std::vector<std::string>::iterator its = dump.begin();
	std::vector<std::string>::iterator ite = dump.end();
	for(; its != ite; ++its)
	{
		log_dbg "%s", its->c_str() log_end
	}

	dump.clear();
	return true;
}

/**
 * @brief	
**/
bool test_get_process_name_by_pid()
{
	std::wstring name = get_process_name_by_pid(GetCurrentProcessId());
	log_dbg "name = %ws", name.c_str() log_end
	return true;
}

/**
 * @brief	
**/
bool test_get_environment_value()
{
	wchar_t* env_variables[] = 
	{
		L"%homepath%",
		L"%temp%",
		L"%username%",
		L"invalid_variable"
	};

	std::wstring env_value;
	for(int i = 0; i < sizeof(env_variables) / sizeof(wchar_t*); ++i)
	{
		if (true != get_environment_value(env_variables[i], env_value)) 
			return false;
		else
			log_dbg "%ws = %ws", env_variables[i], env_value.c_str() log_end
	}

	return true;
}

/**
 * @brief	
**/
bool test_rc4_encrypt()
{
	const char* key = "coresecurity";
	const char plain[] = "abcdefghijklmnop1234567890!@#$%^&*()가나다라마바사아자차카타파하";
	uint8_t enc[1024] = {0};
	uint8_t dec[1024] = {0};
	rc4_state ctx={0};
	
	// encrypt
	rc4_init(&ctx, (uint8_t*)key, (int)strlen(key));
	rc4_crypt(&ctx, (uint8_t*)plain, enc, (int)sizeof(enc));

	// decrypt
	rc4_init(&ctx, (uint8_t*)key, (int)strlen(key));
	rc4_crypt(&ctx, enc, dec, (int)sizeof(dec));

	// compare
	if (0 != memcmp(dec, plain, (int)strlen(plain))) return false;

	return true;
}

// md5.cpp
bool test_md5_sha2()
{
    MD5_CTX ctx_md5 = { 0 };
    sha256_ctx ctx_sha2 = { 0 };

    uint8_t md5[16] = { 0 };
    uint8_t sha2[32] = { 0 };

    const uint32_t read_buffer_size = 4096;
    uint8_t read_buffer[read_buffer_size];
    
    //wchar_t* file_path = L"z:\\Downloads\\ubuntu-14.04.3-desktop-amd64.iso";
    wchar_t* file_path = L"\\Device\\HarddiskVolume2\\Program Files(x86)\\Microsoft Office\\Root\\Office16\\outllib.dll";

    std::wstring dos_name;
    if (true != nt_name_to_dos_name(file_path, dos_name)) return false;

    HANDLE file_handle = CreateFileW(
                            dos_name.c_str(),
                            GENERIC_READ,
                            NULL,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
    if (INVALID_HANDLE_VALUE == file_handle)
    {
        return false;
    }

    DWORD ret = SetFilePointer(file_handle, 0, NULL, FILE_BEGIN);
    if (INVALID_SET_FILE_POINTER == ret)
    {
        CloseHandle(file_handle);
        return false;
    }

    MD5Init(&ctx_md5, 0);
    sha256_begin(&ctx_sha2);

    DWORD read = 4096;
    while (read_buffer_size == read)
    {
        if (FALSE == ::ReadFile(
                            file_handle, 
                            read_buffer, 
                            read_buffer_size, 
                            &read, 
                            NULL))
        {
            con_err 
                "ReadFile( %ws ) failed. gle = 0x%08x", 
                file_path, 
                GetLastError() 
            log_end;
            break;
        }

        if (0 != read)
        {
            MD5Update(&ctx_md5, read_buffer, read);
            sha256_hash(read_buffer, read, &ctx_sha2);
        }
    }

    MD5Final(&ctx_md5);
    
    sha256_end(sha2, &ctx_sha2);
    RtlCopyMemory(md5, ctx_md5.digest, sizeof(md5));

    // hash byte buffer to string...


    CloseHandle(file_handle);
    return true;
}

/**
 * @brief thread_pool test
 */

void work() 
{
    fprintf(stdout, "tid = %u, running\n", GetCurrentThreadId());
};

struct worker
{
    void operator()() 
    {
        fprintf(stdout, "tid = %u, running\n", GetCurrentThreadId() );
    };
};

void more_work( int v) 
{
    fprintf(stdout, "tid = %u, running = %d\n", GetCurrentThreadId(), v);
    //getchar();
};


bool test_thread_pool()
{
    thread_pool pool(4);
    pool.run_task( work );                        // Function pointer.
    pool.run_task( worker() );                    // Callable object.
    pool.run_task( boost::bind( more_work, 5 ) ); // Callable object.
    pool.run_task( worker() );                    // Callable object.
       
    // Wait until all tasks are done.
    while (0 < pool.get_task_count())
    {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
    }

    return true;
}

#include <winioctl.h>


bool test_enum_physical_drive()
{
    std::vector<uint32_t> disk_numbers;
    bool ret = get_disk_numbers(disk_numbers);
    if (true != ret)
        return false;

    DWORD bytes_returned = 0;

    for (auto disk_number : disk_numbers)
    {
        std::wstringstream path;
        path << L"\\\\.\\PhysicalDrive" << disk_number;
        //path << L"\\\\.\\PhysicalDrive0"
        HANDLE disk = CreateFileW(
                        path.str().c_str(),
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,  // for device or file, only if exists.
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if (INVALID_HANDLE_VALUE != disk)
        {
            uint8_t buf[512] = { 0x00 };
            if (!ReadFile(disk, buf, sizeof(buf), &bytes_returned, NULL))
            {
                con_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
            }
            else
            {
                std::vector<std::string> dumps;
                dump_memory(0, buf, sizeof(buf), dumps);
                //for (auto line : dumps)
                //{
                //    con_info "%s", line.c_str() log_end;
                //}                
                con_info "%ws, \n%s", path.str().c_str(), dumps[dumps.size() - 2].c_str() log_end;
            }
        }
        else
        {
            con_err
                "CreateFile( %ws ) failed. gle = %u",
                path.str().c_str(),
                GetLastError()
            log_end;
        }

    }

    return true;
}


bool test_get_disk_volume_info()
{
    std::vector<uint32_t> disk_numbers;
    bool ret = get_disk_numbers(disk_numbers);
    if (true != ret)
        return false;


    for (auto disk_number : disk_numbers)
    {
        uint8_t buf[512] = { 0x00 };
        DWORD bytes_returned = 0;
        LARGE_INTEGER li_new_pos = { 0 };
        LARGE_INTEGER li_distance = { 0 };
        disk_volume_info dvi(disk_number);

        if (true != get_disk_volume_info(dvi))
        {
            con_err "get_vbr_offset( disk_number = %u ) failed.", disk_number);
            return false;
        }

        // dump mbr and vbr
        std::wstringstream path;
        path << L"\\\\.\\PhysicalDrive" << dvi._disk_number;
        HANDLE disk = CreateFileW(
                            path.str().c_str(),
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,  // for device or file, only if exists.
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
        if (INVALID_HANDLE_VALUE == disk)
        {
            con_err
                "CreateFile( %ws ) failed. gle = %u",
                path.str().c_str(),
                GetLastError());
            return false;
        }
        raii_handle handle_guard(disk, raii_CloseHandle);

        // dump MBR
        li_distance.QuadPart = 0;
        if (!SetFilePointerEx(disk, li_distance, &li_new_pos, FILE_BEGIN))
        {
            con_err
                "SetFilePointerEx() failed, gle = %u", GetLastError()
                log_end;
            break;
        }

        if (!ReadFile(disk, buf, sizeof(buf), &bytes_returned, NULL))
        {
            con_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
        }
        else
        {
            std::vector<std::string> dump;
            dump_memory(0, buf, sizeof(buf), dump);
            con_info "[*] MBR" log_end
            for (auto line : dump)
            {
                con_info "%s", line.c_str() log_end
            }
        }

        // dump VBRs
        for (auto vbr_info : dvi._vbrs)
        {
            if (true != vbr_info.recognized) { continue; }

            li_distance = vbr_info.offset;
            if (!SetFilePointerEx(disk, li_distance, &li_new_pos, FILE_BEGIN))
            {
                con_err
                    "SetFilePointerEx() failed, gle = %u", GetLastError()
                    log_end;
                break;
            }

            if (!ReadFile(disk, buf, sizeof(buf), &bytes_returned, NULL))
            {
                con_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
            }
            else
            {
                con_info "[*] VBR" log_end
                std::vector<std::string> dump;
                dump_memory(0, buf, sizeof(buf), dump);
                for (auto line : dump)
                {
                    con_info "%s", line.c_str() log_end
                }
            }
        }
    }
    
    return true;
}

/// @brief 
bool test_dump_xxx()
{
    dump_all_disk_drive_layout();
    //dump_boot_area();
    return true;
}

/// @brief
bool test_write_mbr_vbr()
{
    con_err
        "This test writes MBR and VBR, may cause serious system damage. really want to do this? ok~"
        log_end
    _pause;

    ///
    /// buf 와 offset 이 512 (섹터 사이즈)로 align 되어있지 않으면 
    /// Read/WriteFile 함수에서 에러 (87) 발생함
    /// 나중에 한번 확인해봐
    ///
    uint8_t buf_read[512] = { 0x00 };
    uint8_t buf_write[512] = { 0x41 };
    
    std::vector<uint32_t> disk_numbers;
    bool ret = get_disk_numbers(disk_numbers);
    if (true != ret)
    {
        con_err "get_disk_numbers() failed." log_end;
        return false;
    }

    for (auto disk_number : disk_numbers)
    {
        disk_volume_info dvi(disk_number);

        if (true != get_disk_volume_info(dvi))
        {
            con_err "get_vbr_offset( disk_number = %u ) failed.", disk_number);
            return false;
        }

        // writes mbr and vbr
        std::wstringstream path;
        path << L"\\\\.\\PhysicalDrive" << dvi._disk_number;
        HANDLE disk = CreateFileW(
                            path.str().c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,  // for device or file, only if exists.
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
        if (INVALID_HANDLE_VALUE == disk)
        {
            con_err
                "CreateFile( %ws ) failed. gle = %u",
                path.str().c_str(),
                GetLastError());
            return false;
        }
        raii_handle handle_guard(disk, raii_CloseHandle);

        // #1 write mbr - backup
        if (true != read_file_offset(disk, 0, buf_read, sizeof(buf_read)))
        {
            con_err "read_file_offset() failed" log_end;
        }

        con_info "[*] %uth disk MBR - before write", disk_number log_end;        
        dump_file_offset(disk, 0, sizeof(buf_read));

        // #2 write mbr - write
        if (true != write_file_offset(disk, 0, buf_write, sizeof(buf_write)))
        {
            con_err "write_file_offset(mbr) failed." log_end;
            return false;
        }
        con_info "[*] %uth disk MBR - after write", disk_number log_end;
        dump_file_offset(disk, 0, sizeof(buf_read));
        
        // #3 write mbr - restore
        if (true != write_file_offset(disk, 0, buf_read, sizeof(buf_read)))
        {
            con_err "write_file_offset(mbr) failed. (restore failed)" log_end;
            return false;
        }

        // dump VBRs
        uint32_t ix = 0;
        for (auto vbr_info : dvi._vbrs)
        {
            if (true != vbr_info.recognized) { continue; }

            // #1 write vbr - backup
            if (true != read_file_offset(disk, vbr_info.offset.QuadPart, buf_read, sizeof(buf_read)))
            {
                con_err "read_file_offset() failed" log_end;
            }

            con_info "[*] %uth disk, %uth VBR - before write", disk_number, ix log_end;
            dump_file_offset(disk, vbr_info.offset.QuadPart, sizeof(buf_read));

            // #2 write vbr - write
            if (true != write_file_offset(disk, vbr_info.offset.QuadPart, buf_write, sizeof(buf_write)))
            {
                con_err "write_file_offset(mbr) failed." log_end;
                return false;
            }
            con_info "[*] %uth disk, %uth VBR - after write", disk_number, ix log_end;
            dump_file_offset(disk, vbr_info.offset.QuadPart, sizeof(buf_read));

            // #3 write vbr - restore
            if (true != write_file_offset(disk, vbr_info.offset.QuadPart, buf_read, sizeof(buf_read)))
            {
                con_err "write_file_offset(mbr) failed. (restore failed)" log_end;
                return false;
            }

            ++ix;
        }
    }

    return true;
}


bool read_file_offset(_In_ HANDLE file_handle, _In_ uint64_t offset, _In_ uint8_t* buf, _In_ uint32_t size)
{
    _ASSERTE(0 == offset % 512);
    _ASSERTE(0 == size % 512);

    DWORD bytes_rw = 0;    
    LARGE_INTEGER li_new_pos = { 0 };
    LARGE_INTEGER li_distance = { 0 };
    li_distance.QuadPart = offset;

    if (!SetFilePointerEx(file_handle, li_distance, &li_new_pos, FILE_BEGIN))
    {
        con_err
            "SetFilePointerEx() failed, gle = %u", GetLastError()
            log_end;
        return false;
    }

    RtlZeroMemory(buf, size);
    if (!ReadFile(file_handle, buf, size, &bytes_rw, NULL))
    {
        con_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
        return false;
    }

    return true;
}

bool write_file_offset(_In_ HANDLE file_handle, _In_ uint64_t offset, _In_ uint8_t* buf, _In_ uint32_t size)
{
    _ASSERTE(0 == offset % 512);
    _ASSERTE(0 == size % 512);

    DWORD bytes_rw = 0;
    LARGE_INTEGER li_new_pos = { 0 };
    LARGE_INTEGER li_distance = { 0 };
    li_distance.QuadPart = offset;

    if (!SetFilePointerEx(file_handle, li_distance, &li_new_pos, FILE_BEGIN))
    {
        con_err
            "SetFilePointerEx() failed, gle = %u", GetLastError()
            log_end;
        return false;
    }

    if (!WriteFile(file_handle, buf, size, &bytes_rw, NULL))
    {
        con_err 
            "WriteFile() failed. gle = %u", GetLastError()
            );
        return false;
    }

    return true;
}

void dump_file_offset(_In_ HANDLE file_handle, _In_ uint64_t offset, _In_ uint32_t size)
{
    uint8_t buf[512] = { 0 };

    _ASSERTE(size <= sizeof(buf));
 
    if (true != read_file_offset(file_handle, offset, buf, size))
    {
        con_err "read_file_offset() failed." log_end;
        return;
    }

    std::vector<std::string> dump;
    if (true != dump_memory(0, buf, size, dump))
    {
        con_err "dump_memory(0,  ) failed." log_end;
        return;
    }

    for (auto line : dump)
    {
        con_info "%s", line.c_str() log_end
    }
}

bool test_rstrnicmp()
{
    if (!rstrnicmp(L"abcdef.123", L"123")) return false;
    if (rstrnicmp(L"aaa.def", L"xx")) return false;
    if (rstrnicmp(L"aaa.def", L"xx")) return false;
    if (rstrnicmp(L"aaa.def", L"xxwwwwwwwwwwwww")) return false;
    return true;
}

bool test_device_name_from_nt_name()
{
    std::wstring r = device_name_from_nt_name(L"\\Device\\HarddiskVolume4\\Windows");    
    if (r.compare(L"\\Device\\HarddiskVolume4\\") != 0) return false;
    r = device_name_from_nt_name(L"\\Device\\HarddiskVolume4");
    if (r.compare(L"\\Device\\HarddiskVolume4") != 0) return false;
    r = device_name_from_nt_name(L"\\Device\\HarddiskVolume455\\xyz");
    if (r.compare(L"\\Device\\HarddiskVolume455\\") != 0) return false;

    return true;
}

bool test_regexp()
{
    try
    {

        // std::regexp 가 `\` 를 특수문자로 인식, 한번더 escape 하기 때문에
        // `\device` 문자열을 매칭하기 위해서는 
        // `\\device` 패턴을 사용해야 한다. 
        // 
        // c++ 에서도 `\` 를 escape 하므로 결국 `\` 문자를 매칭하기 위해서는 
        // `\\\\device` 패턴을 사용해야 한다. 
        // 
        // c++11 raw string, `LR"( )"` 을 사용하면 c++ 의 escape 를 막을 수 있으니까
        // LR"(\\device)" 패턴을 사용해서 매칭할 수 있다. 
        //
        std::wstring str(L"\\device");
        std::wregex exp(L"\\\\device");   // wregex 가 `\` 를 한번씩 더 escape 하므로 `\\` 와 동일
        std::wregex expr(LR"(\\device)");

        std::wsmatch wsm;
        _ASSERTE(true == std::regex_match(str, wsm, exp));  // 정확히 일치하면 매칭
        _ASSERTE(true == std::regex_match(str, wsm, expr)); // 정확히 일치하면 매칭



        //wchar_t* r = LR"(\\device\\mup\\(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}))";
        //std::wregex exp(r, std::regex_constants::ECMAScript | std::regex_constants::icase);

        //for (auto s : strs)
        //{
        //    std::wstring ws(s);
        //    std::wsmatch wsm;
        //    if (std::regex_search(ws, wsm, exp))        // sub string 매칭
        //    {
        //        for (size_t i = 0; i < wsm.size(); i++)
        //        {
        //            std::wcout << i << "번째 : " << wsm[i] << std::endl;
        //        }
        //    }
        //}

        //std::wstring str(L"\\device");
        //std::wregex exp(L"\\\\de");   // wregex 가 `\` 를 한번씩 더 escape 하므로 `\\` 와 동일
        //std::wregex expr(LR"(\\de)");
        //
        //std::wsmatch wsm;
        //if (std::regex_search(str, wsm, exp))
        //{
        //    for (size_t i = 0; i < wsm.size(); i++)
        //    {
        //        std::wcout << wsm[i] << std::endl;
        //    }
        //}        
    }
    catch (const std::regex_error& e)
    {
        con_err "regex_error caught: %s", e.what() con_end;
        
        if (e.code() == std::regex_constants::error_brack)
        {
            con_err "The code was error_brack" con_end;
        }
    }

    const wchar_t* strs[] = {
        L"1122",
        L"\\d",
        L"device",
        L"\\device\\mup",
        L"\\Device\\Mup\\192.168.0.197\\sfr002_share\\agent.exe",
        L"\\Device\\Mup\\1.1.1.1\\sfr002_share\\agent.exe",
        L"\\DevicE\\MUP\\192.168.0.197",
        L"\\Device\\Mup\\192.168.0.197\\"
    };




    try
    {
        wchar_t* r = LR"(\\device\\mup\\(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}))";
        std::wregex exp(r, std::regex_constants::ECMAScript | std::regex_constants::icase);
        for (auto str : strs)
        {
            std::wcmatch wcm;
            if (std::regex_search(str, wcm, exp))        // sub string 매칭
            {
                _ASSERTE(2 == wcm.size());
                con_info "str=%ws, match=%ws, ip=%ws", str, wcm[0].str().c_str(), wcm[1].str().c_str() con_end;
            }
        }
    
    }
    catch (const std::regex_error& e)
    {
        con_err "regex_error caught: %s", e.what() con_end;
        
        if (e.code() == std::regex_constants::error_brack)
        {
            con_err "The code was error_brack" con_end;
        }
    }
    
    return true;
}


/// @brief  IcmpSendEcho 
bool test_ping()
{
    for (int i = 0; i < 10; ++i)
    {
        send_ping(L"8.8.8.8");
    }

    return true;
}




bool test_canonicalize_file_name()
{
    //[INFO] \ ? ? \c : \windows\system32\abc.exe->c:\windows\system32\abc.exe
    //[INFO] \Systemroot\abc.exe->C:\WINDOWS\abc.exe
    //[INFO] system32\abc.exe->C:\WINDOWS\system32\abc.exe
    //[INFO] \windows\system32\abc.exe->C:\WINDOWS\system32\abc.exe
    //[INFO] \Device\Mup\1.1.1.1\abc.exe -> \\1.1.1.1\abc.exe
    //[INFO] \Device\Unknown\aaaa.exe -> \Device\Unknown\aaaa.exe
    //[INFO] \device\lanmanredirector\;x:000000000008112d\192.168.153.144\sfr022\ -> \\192.168.153.144\sfr022\
    //[INFO] x:\->x:\
    //[INFO] \Device\Mup\192.168.153.144\sfr022\ -> \\192.168.153.144\sfr022\

    wchar_t* file_names[] = {
        L"\\??\\c:\\windows\\system32\\abc.exe",
        L"\\Systemroot\\abc.exe",
        L"system32\\abc.exe",
        L"\\windows\\system32\\abc.exe",
        L"\\Device\\Mup\\1.1.1.1\\abc.exe",
        L"\\Device\\Unknown\\aaaa.exe",

        // net use x: \\192.168.153.144\\sfr022\\ /user:vmuser * 명령으로 x 드라이브 매핑해둠
        // 
        L"\\device\\lanmanredirector\\;x:000000000008112d\\192.168.153.144\\sfr022\\",
        L"x:\\",
        L"\\Device\\Mup\\192.168.153.144\\sfr022\\"
    };

    NameConverter nc;
    if (true != nc.reload()) return false;

    for (int i = 0; i < sizeof(file_names) / sizeof(wchar_t*); ++i)
    {
        std::wstring name = nc.get_file_name(file_names[i]);
        log_info "%ws -> %ws", file_names[i], name.c_str() log_end;
    }

    return true;
}


bool test_crc64()
{
    log_info "e9c6d914c4b8d9ca == %016llx",
        (unsigned long long) crc64(0, (unsigned char*)"123456789", 9)
        log_end;
    return true;
}