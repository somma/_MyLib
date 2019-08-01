/**
 * @file    Tests for MyLib
 * @brief
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2014/01/25 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
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
#include "StopWatch.h"
#include "GeneralHashFunctions.h"
#include <unordered_map>
#include "Singleton.h"
#include "FileInfoCache.h"
#include "account_info.h"
#include "curl_client.h"
#include "CStream.h"
#include "sched_client.h"

// _test_log.cpp
extern bool test_log_rotate();

// _test_steady_timer.cpp
extern bool test_steady_timer();


// _test_dns_query.cpp
extern bool test_ip_to_dns();
extern bool test_dns_to_ip();

// _test_net_util.cpp
extern bool test_get_adapters();
extern bool test_get_addr_info();

// test_iphelp_api.cpp
extern bool test_iphelp_api();

bool test_create_guid();
bool test_file_info_cache();

// _test_process_token.cpp
extern bool test_process_token();

bool test_is_executable_file_w();
bool test_singleton();

// _test_std_move.cpp
bool test_std_move();

bool test_log_xxx();
bool test_set_security_attributes();

bool test_GeneralHashFunctions();
bool test_GeneralHashFunctions2();

bool test_get_file_extension();
bool test_raii_xxx();
bool test_suspend_resume_process();
bool test_to_str();
bool test_convert_file_time();

// _test_ppl.cpp
extern bool test_ppl();

// _test_file_io_helper.cpp
bool test_file_io_helper();
bool test_file_io_helper2();

// _test_scm.cpp
extern bool test_scm_context();

bool test_alignment_error_test();
bool test_crc64();

bool test_NameConverter_iterate();
bool test_NameConverter_get_canon_name();
bool test_NameConverter_dosname_to_devicename();

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
//bool test_const_position();	// ������ �Ұ� �׽�Ʈ
bool test_initialize_string();

bool test_base64();
bool test_random();
bool test_ip_mac();
bool test_ip_to_str();

bool test_strtok();

// test_process_tree.cpp
extern bool test_process_tree();
extern bool test_iterate_process_tree();
extern bool test_image_path_by_pid();
extern bool test_get_process_creation_time();

// _test_cpp_test.cpp
bool test_cpp_class();


// win32utils.cpp
bool test_find_files();
bool test_get_filepath_by_handle();
bool test_nt_name_to_dos_name();
bool test_query_dos_device();
bool test_bin_to_hex();
bool test_str_to_xxx();
bool test_set_get_file_position();
bool test_get_module_path();
bool test_dump_memory();
bool test_get_environment_value();
bool test_get_account_infos();
bool test_get_installed_programs();
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
extern bool test_map_insert_swap();


// _test_regstry_util.cpp
extern bool test_registry_util();
extern bool test_read_mouted_device();
extern bool test_set_binary_data();

// _test_curl_https
bool test_curl_https();
bool test_curl_http();
bool test_curl_http_upload();

// thread_pool.h
bool test_thread_pool();

// _test_boost_thread.cpp
extern bool test_boost_thread();

//_test_aes256.cpp
bool test_aes256();

bool test_trivia();
bool test_alignment();
bool test_create_string_from_buffer();
bool test_stop_watch();

bool test_boost_function();

bool test_bit_field();

bool test_sched_client();

void run_test()
{
	UINT32 _pass_count = 0;
	UINT32 _fail_count = 0;

	bool ret = false;
	//assert_bool(true, test_log_rotate);
	//assert_bool(true, test_steady_timer);
	//assert_bool(true, test_get_adapters);
	//assert_bool(true, test_get_addr_info);
	//assert_bool(true, test_ip_to_dns);
	//assert_bool(true, test_dns_to_ip);
	//assert_bool(true, test_iphelp_api);
	//assert_bool(true, test_create_guid);
	//assert_bool(true, test_file_info_cache);
	//
	//assert_bool(true, test_process_token);
	//assert_bool(true, test_is_executable_file_w);
	//assert_bool(true, test_singleton);
	//assert_bool(true, test_std_move);
	//assert_bool(true, test_log_xxx);
	//assert_bool(true, test_set_security_attributes);
	//assert_bool(true, test_GeneralHashFunctions);
	//assert_bool(true, test_GeneralHashFunctions2);
	//assert_bool(true, test_get_file_extension);
	//assert_bool(true, test_raii_xxx);
	//assert_bool(true, test_suspend_resume_process);
	//assert_bool(true, test_convert_file_time);

	//assert_bool(true, test_ppl);

	//assert_bool(true, test_find_and_replace);
	//assert_bool(true, test_file_io_helper);
	//assert_bool(true, test_file_io_helper2);

	//assert_bool(true, test_scm_context);

	//assert_bool(true, test_regexp);
	//assert_bool(true, test_ping);
	//assert_bool(true, test_alignment_error_test);
	//assert_bool(true, test_crc64);


	//assert_bool(true, test_NameConverter_iterate);
	//assert_bool(true, test_NameConverter_get_canon_name);
	//assert_bool(true, test_NameConverter_dosname_to_devicename);

	//assert_bool(true, test_wmi_client);
	//assert_bool(true, test_NtCreateFile);
	//assert_bool(true, test_device_name_from_nt_name);
	//assert_bool(true, test_rstrnicmp);

	//assert_bool(true, test_get_drive_type);
	//assert_bool(true, test_os_version);

	//assert_bool(true, test_boost_thread);
	//assert_bool(true, test_thread_pool);
 //
	//assert_bool(true, test_boost_asio_timer);
	//assert_bool(true, test_for_each);
	//assert_bool(true, test_enum_physical_drive);
	//assert_bool(true, test_get_disk_volume_info);
	//assert_bool(true, test_dump_xxx);

	//assert_bool(true, test_asm_func);
	//assert_bool(true, test_x64_calling_convension);
	//assert_bool(true, test_2_complement);
	//assert_bool(true , test_print_64int);
	//assert_bool(true, test_std_string_find_and_substr);
	//assert_bool(true, test_to_lower_uppper_string);

	//assert_bool(true, test_initialize_string);

	//uint32_t lt = get_log_to();
	//set_log_to(log_to_con | lt);
	////assert_bool(true, test_process_tree);
	//assert_bool(true, test_iterate_process_tree);
	//set_log_to(lt);

	//assert_bool(true, test_image_path_by_pid);
	//assert_bool(true, test_get_process_creation_time);
	//assert_bool(true, test_base64);
	//assert_bool(true, test_random);
	//assert_bool(true, test_ip_mac);
	//assert_bool(true, test_ip_to_str);

	//assert_bool(true, test_strtok);
	//assert_bool(true, test_cpp_class);
	//assert_bool(true, test_nt_name_to_dos_name);

	//assert_bool(true, test_query_dos_device);
	//assert_bool(true, test_get_filepath_by_handle);
	//assert_bool(true, test_find_files);
	//
	//assert_bool(true, test_bin_to_hex);
	//assert_bool(true, test_str_to_xxx);
	//assert_bool(true, test_set_get_file_position);
	//assert_bool(true, test_get_module_path);
	//assert_bool(true, test_dump_memory);
	//assert_bool(true, test_get_environment_value);
	//assert_bool(true, test_get_account_infos);
	//assert_bool(true, test_get_installed_programs);
	//assert_bool(true, test_rc4_encrypt);
	//assert_bool(true, test_md5_sha2);

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
	//assert_bool(true, test_map_insert_swap);


	//assert_bool(true, test_registry_util);
	//assert_bool(true, test_read_mouted_device);
	//assert_bool(true, test_set_binary_data);
	//assert_bool(true, test_aes256);

	//assert_bool(true, test_curl_https);
	//assert_bool(true, test_curl_http);
	assert_bool(true, test_curl_http_upload);
	//assert_bool(true, test_alignment);
	//assert_bool(true, test_create_string_from_buffer);
	//assert_bool(true, test_stop_watch);
	//assert_bool(true, test_boost_function);
	//assert_bool(true, test_bit_field);
	//assert_bool(true, test_sched_client);

//
//	�����׽�Ʈ�� ���Ե��� �ʴ� �׳� �׽�Ʈ�� �ڵ�
//
	//assert_bool(true, test_write_mbr_vbr);		// Ȥ�ö� �׽�Ʈ �� mbr ���� �� �����Ƿ� ����.
	//assert_bool(true, test_const_position);		// ������ �Ұ� �׽�Ʈ



	log_info
		"----------------------------------------------------"
		log_end;

	//_pause;

	log_info
		"total test = %u, pass = %u, fail = %u",
		_pass_count + _fail_count,
		_pass_count,
		_fail_count
	log_end

	finalize_log();
}

bool test_curl_https()
{
	pcurl_client _curl_client = new curl_client();
	if (nullptr == _curl_client)
	{
		log_err "not enought memory" log_end;
		return false;
	}

	std::stringstream http_header;
	http_header << "authorization: Bearer "
				<< "FOO";

	if (true != _curl_client->initialize(10, 90, 0))
	{
		log_err "curl client initialize() failed." log_end;
		return false;
	}

	_curl_client->append_header("authorization", http_header.str().c_str());

	const char* url = "https://api.somma.kr:55550/api/v1100/host-info";

	long http_response_code = 404;
	std::string res;
	_ASSERTE(true == _curl_client->http_get(url, http_response_code, res));

	_ASSERTE(_curl_client != nullptr);
	delete _curl_client; _curl_client = nullptr;
	return true;
}

bool test_curl_http()
{
	pcurl_client _curl_client = new curl_client();
	if (nullptr == _curl_client)
	{
		log_err "not enought memory" log_end;
		return false;
	}

	if (true != _curl_client->initialize())
	{
		log_err "curl client initialize() failed." log_end;
		return false;
	}

	const char* url = "http://192.168.10.200:5601/app/kibana#/monster?_g=()";
	long http_response_code = 404;
	CMemoryStream stream;
	_ASSERTE(true == _curl_client->http_get(url, http_response_code, stream));

	std::vector<std::string> dumps;
	dump_memory(0LL, (unsigned char*)(stream.GetMemory()), stream.GetSize(), dumps);
	std::for_each(dumps.begin(), dumps.end(),[](std::string& dump){printf("%s\n", dump.c_str());});

	_ASSERTE(_curl_client != nullptr);
	delete _curl_client; _curl_client = nullptr;
	return true;
}

bool test_curl_http_upload()
{
	pcurl_client client = new curl_client();

	const wchar_t* path = L"C:\\Windows\\System32\\notepad.exe";
	const char*  url = "http://localhost:33330/upload";
	long response_code;
	std::string response;
	client->initialize();

	std::map<std::string, std::string> forms;
	forms["group_guid"] = "invalid_group_guid";
	forms["host_guid"] = "invalid_host_guid";
	forms["full_path"] = "invalid_full_path";

	bool ret = false;
	do
	{
		if (true != client->http_file_upload(url, path, forms, response_code, response))
		{
			log_err "http_file_upload() failed. path=%ws, url=%s, status_code=%u",
				path,
				url,
				response_code
				log_end;
			break;
		}

		ret = true;
	} while (false);

	if (true == ret)
	{
		log_info "file upload success. path=%ws, url=%s, status_code=%u",
			path,
			url,
			response_code
			log_end;
	}

	if (nullptr != client)
	{
		delete client; client = nullptr;
	}
	return false;
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
    log_info "%ws", osver_to_str(os) log_end;
    return true;
}

/// @brief
bool test_find_and_replace()
{
    std::string src = "0123456789,Version=v4,5";
    std::string find = ",";
    std::string replace = " ";


    log_info "before find_and_replace, %s", src.c_str() log_end;
    std::string str_mod = find_and_replace_string_exa(src.c_str(), ",", "\\,");
    _ASSERTE(0 == str_mod.compare("0123456789\\,Version=v4\\,5"));
    log_info "after find_and_replace, %s", str_mod.c_str() log_end;

    std::wstring srcw = L"0123456789,Version=v4,5";
    log_info "before find_and_replace, %ws", srcw.c_str() log_end;
    std::wstring str_modw = find_and_replace_string_exw(srcw.c_str(), L",", L"\\,");
    _ASSERTE(0 == str_modw.compare(L"0123456789\\,Version=v4\\,5"));
    log_info "after find_and_replace, %ws", str_mod.c_str() log_end;

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
	std::wstring nt_device_name2(L"\\DEVICE\\HarddiskVolume1");		// �빮��


	//> ��ҹ��ڰ� ��ġ�ϴ� ��� string::find �� ���� ���� ��
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

	//> ��ҹ��� ���� ���� find �Ϸ��� win32util::to_lower_string() ȣ�� �� ���ؾ� ��
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

	// ���: %d = -1, %x = ffffffff
	// 1 = 0000 0001
	//     1111 1110 + 1 (������ ǥ���ϱ� ���� 2�� ������ ���ϸ�...)
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
 * @brief	const ��ġ / �ǹ�
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
	//> (const char*) msg : char* �� const, �� msg �� ����Ű�� ������ ���� �Ұ�
    char* Function1(const char* msg)
    {
        msg[0] = 't'; // error
        return m_msg;
    }

	//> char* (const msg) : msg ������ const, �� msg ������ ���� ���� �Ұ�
    char* Function2(char* const msg)
    {
        msg = m_msg; //error
        return m_msg;
    }

	//> �޼ҵ� ���ȭ, �� �޼ҵ�� Ŭ���� ����� ���� ���� ������ ���� �� ���� ����
    char* Function3(char* msg) const
    {
        m_msg = msg; //error
        return m_msg;
    }

	//> (const char*) : ���� ���� const char* �̹Ƿ� ���� �޴� ������ const char* �̾�� ��
	//> ���� ���ϵǴ� �����Ͱ� ����Ű�� ������ ���� �Ұ�
    const char* Function4(char* msg)
    {
        m_msg = msg;
        return m_msg; //��ȯ �޴� Ÿ���� const�� �ƴ� ��� error
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

	//> invalid null point exception �߻�
	//> try-except �� �� ����...
	//> �ʱ�ȭ�� NULL �̸� "" �� �ٲ㼭 �ʱ�ȭ �ؾ� ��
/*
	try
	{
		std::wstring str2 = NULL;
		log_dbg "str2 = %s", str2.c_str() log_end
	}
	catch (...)
	{
		log_err "oops" log_end
	}
*/

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

	std::wstring string_to_encodeW = L"���ѹα�";
	std::string string_to_encodeA = "���ѹα�";

	std::wstring wide_str;
	std::string utf8_str;
	std::string base64_str;

	// base 64 encode
	//
	// #1) multibyte -> ucs16 -> utf8 -> base64 ������...
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
    log_info "var = %d", var log_end;
	return true;
}

/**
 * @brief
**/
bool test_ip_mac()
{
	std::wstring host_name;
	std::vector<std::string> ip_list;

	log_info "representative ip v4=%s",
		get_representative_ip_v4().c_str()
		log_end;

	_ASSERTE(true == get_host_name(host_name));
	_ASSERTE(true == get_ip_list_v4(ip_list));

	log_info "host_name = %ws", host_name.c_str() log_end
	std::for_each(
        ip_list.begin(),
        ip_list.end(),
		[](std::string& ip)
		{
			log_info "ip=%s, mac=%s",
				ip.c_str(),
				get_mac_by_ip_v4(ip.c_str()).c_str()
				log_end
		});


	return true;
}

/// @brief
bool test_ip_to_str()
{
	const wchar_t* ip_str = L"1.2.3.4";

    uint32_t addr = { 0 };
    if (true != str_to_ipv4(ip_str, addr)) return false;
    log_info "ip = %ws -> %lu", ip_str, addr log_end;
    log_info "ip = %lu -> %s", addr, ipv4_to_str(addr).c_str() log_end;


	in_addr inaddr;
    inaddr.S_un.S_addr = 0x0100007f;
    log_info "ip = %lu -> %s", inaddr.S_un.S_addr, ipv4_to_str(inaddr).c_str() log_end;
    return true;
}

/// @brief
bool
split_string(
	_In_ const char* str,
	_In_ const char* seps,
	_Out_ std::vector<std::string>& tokens
	)
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

    char_ptr buf((char*)malloc(len + sizeof(char)), [](char* p) {
		if (nullptr != p)
		{
			free(p);
		}
	});
    if (NULL == buf.get())
    {
        return false;
    }

    RtlCopyMemory(buf.get(), str, len);
    buf.get()[len] = 0x00;

    char* next_token = NULL;
    char* token = strtok_s(buf.get(), seps, &next_token);
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

	//
    //	Establish string and get the first token:
	//
    if (false == split_string(string1, seps, tokens1) ||
        false == split_string(string2, seps, tokens2))
    {
        return false;
    }

	log_info "tokens1 : %s\n", string1 log_end;
    for (auto token: tokens1)
    {
		log_info "\t%s", token.c_str() log_end;
    }
	log_info "\n" log_end;


	log_info "tokens2 : %s\n", string2 log_end;
    for (auto token : tokens2)
    {
		log_info "\t%s", token.c_str() log_end;
    }
	log_info "\n" log_end;

    return true;
}

/// @brief	c:\temp\dbg			X
///			c:\temp\dbg\		O
///			c:\temp\dbg\*		O
///			c:\temp\dbg\*.*		O
///			c:\temp\dbg\*.exe	O
///			c:\temp\dbg\*.txt	O
bool WINAPI ffcb(_In_ DWORD_PTR tag, _In_ const wchar_t* path)
{
	std::list<std::wstring>* files = (std::list<std::wstring>*)tag;
	files->push_back(path);
	return true;
}

bool test_find_files()
{
	//
	//	�׽�Ʈ�� ���丮/���� ����
	//
	{
		if (true == is_file_existsW(L"c:\\temp\\dbg"))
		{
			if (!WUDeleteDirectoryW(L"c:\\temp\\dbg"))
			{
				return false;
			}
		}

		if (true != WUCreateDirectory(L"c:\\temp\\dbg"))
		{
			return false;
		}

		HANDLE h = open_file_to_write(L"c:\\temp\\dbg\\1111.txt");
		if (INVALID_HANDLE_VALUE == h)
			return false;
		else
			CloseHandle(h);

		h = open_file_to_write(L"c:\\temp\\dbg\\1112.txt");
		if (INVALID_HANDLE_VALUE == h)
			return false;
		else
			CloseHandle(h);


		h = open_file_to_write(L"c:\\temp\\dbg\\1111.exe");
		if (INVALID_HANDLE_VALUE == h)
			return false;
		else
			CloseHandle(h);

		h = open_file_to_write(L"c:\\temp\\dbg\\1111.dll");
		if (INVALID_HANDLE_VALUE == h)
			return false;
		else
			CloseHandle(h);

		h = open_file_to_write(L"c:\\temp\\dbg\\1111");
		if (INVALID_HANDLE_VALUE == h)
			return false;
		else
			CloseHandle(h);
	}


	typedef struct root_and_count
	{
		wchar_t* root_path;
		size_t count;
	} *proot_and_count;

	root_and_count roots[] = {
		{ L"c:\\temp\\dbg\\1112.txt", 1 },
		{ L"c:\\temp\\dbg\\1111.*", 4 },
		{ L"c:\\temp\\dbg\\1111", 1 },
		{L"c:\\temp\\dbg", 5},
		{ L"c:\\temp\\dbg\\", 5},
		{ L"c:\\temp\\dbg\\*", 5 },
		{ L"c:\\temp\\dbg\\*.*", 5},
		{ L"c:\\temp\\dbg\\*.exe", 1},
		{ L"c:\\temp\\dbg\\*.txt", 2}
	};

	for (int i = 0; i < sizeof(roots) / sizeof(root_and_count); ++i)
	{
		std::list<std::wstring> files;
		_ASSERTE(true == find_files(roots[i].root_path,
									reinterpret_cast<DWORD_PTR>(&files),
									false,
									ffcb));
		_ASSERTE(roots[i].count == files.size());

		std::wstringstream strm;
		for (auto file: files)
		{
			strm << file << std::endl;
		}
		log_info "root=%ws\n%ws", roots[i].root_path, strm.str().c_str() log_end;
	}

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
			"nt_name=%ws -> dos_name=%ws",
			nt_name,
			dos_name.c_str()
		log_end
	}
	else
	{
		log_err
			"nt_name=%ws -> dos_name=failed.",
			nt_name
			log_end;
	}
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

	/*std::string hexa;
	if (true != bin_to_hexa(sizeof(code), code, true, hexa)) return false;
	if (0 != hexa.compare(orgau)) return false;

	if (true != bin_to_hexa(sizeof(code), code, false, hexa)) return false;
	if (0 != hexa.compare(orgal)) return false;

	std::wstring hexw;
	if (true != bin_to_hexw(sizeof(code), code, true, hexw)) return false;
	if (0 != hexw.compare(orgwu)) return false;

	if (true != bin_to_hexw(sizeof(code), code, false, hexw)) return false;
	if (0 != hexw.compare(orgwl)) return false;

	*/
	std::string hexa;
	std::string hexa_fast;
	StopWatch sw; sw.Start();
	if (true != bin_to_hexa(sizeof(code), code, true, hexa)) return false;
	sw.Stop();
	log_info "%f, bin_to_hexa()", sw.GetDurationMilliSecond() log_end;

	sw.Start();
	if (true != bin_to_hexa_fast(sizeof(code), code, true, hexa_fast)) return false;
	sw.Stop();
	log_info "%f, bin_to_hexa_fast()", sw.GetDurationMilliSecond() log_end;

	if (0 != hexa.compare(orgau)) return false;
	if (0 != hexa_fast.compare(orgau)) return false;

	sw.Start();
	if (true != bin_to_hexa(sizeof(code), code, false, hexa)) return false;
	sw.Stop();
	log_info "%f, bin_to_hexa()", sw.GetDurationMilliSecond() log_end;

	sw.Start();
	if (true != bin_to_hexa_fast(sizeof(code), code, false, hexa_fast)) return false;
	sw.Stop();
	log_info "%f, bin_to_hexa_fast()", sw.GetDurationMilliSecond() log_end;

	if (0 != hexa.compare(orgal)) return false;
	if (0 != hexa_fast.compare(orgal)) return false;





	std::wstring hexw;
	std::wstring hexw_fast;
	sw.Start();
	if (true != bin_to_hexw(sizeof(code), code, true, hexw)) return false;
	sw.Stop();
	log_info "%f, bin_to_hexw()", sw.GetDurationMilliSecond() log_end;

	sw.Start();
	if (true != bin_to_hexw_fast(sizeof(code), code, true, hexw_fast)) return false;
	sw.Stop();
	log_info "%f, bin_to_hexw_fast()", sw.GetDurationMilliSecond() log_end;

	if (0 != hexw.compare(orgwu)) return false;
	if (0 != hexw_fast.compare(orgwu)) return false;

	sw.Start();
	if (true != bin_to_hexw(sizeof(code), code, false, hexw)) return false;
	sw.Stop();
	log_info "%f, bin_to_hexw()", sw.GetDurationMilliSecond() log_end;

	sw.Start();
	if (true != bin_to_hexw_fast(sizeof(code), code, false, hexw_fast)) return false;
	sw.Stop();
	log_info "%f, bin_to_hexw_fast()", sw.GetDurationMilliSecond() log_end;

	if (0 != hexw.compare(orgwl)) return false;
	if (0 != hexw_fast.compare(orgwl)) return false;




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
bool test_set_get_file_position()
{
	std::wstringstream test_file;
	test_file << get_current_module_dirEx() << L"\\testfile.dat";
	HANDLE file_handle = open_file_to_write(test_file.str().c_str());
	if (INVALID_HANDLE_VALUE == file_handle) return false;

	do
	{
		DWORD bytes_written = 0;
		for (int i = 0; i < 254; ++i)
		{
			if (!WriteFile(file_handle,
						   &i,
						   1,
						   &bytes_written,
						   NULL))
			{
				break;
			}
		}

		int64_t pos = 0;
		int64_t new_pos = 0;

		// get file position
		if (true != get_file_position(file_handle, pos)) break;
		if (254 != pos) break;

		// set file position
		pos = 128;
		if (true != set_file_position(file_handle, pos, &new_pos)) break;
		if (true != get_file_position(file_handle, pos)) break;

		if (128 != pos) break;

	} while (false);

	CloseHandle(file_handle);
	_ASSERTE(DeleteFileW(test_file.str().c_str()));

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
 * @brief �ý����� ���� ������ �о� ���� �׽�Ʈ
**/
bool test_get_account_infos()
{
	std::list<paccount> accounts;
	_ASSERTE(true == get_account_infos(accounts));

	_ASSERTE(1 < accounts.size());

	for (auto account : accounts)
	{
		FILETIME logon_filetime;
		unixtime_to_filetime(account->last_logon_timestamp(), &logon_filetime);
		std::wstring last_logon_kst = MbsToWcsEx(file_time_to_str(&logon_filetime, true, false).c_str());
		log_info
			"name(%ws) : sid(%ws) priv(%ws) attrib(%d) last_logon(%ws) log_on_count(%u) last_password_change(%u)",
			account->name().c_str(),
			account->sid().c_str(),
			account->privilege(),
			account->flags(),
			last_logon_kst.c_str(),
			account->num_logons(),
			account->password_age()
			log_end;

		delete account; account = nullptr;
	}

	accounts.clear();

	return true;
}

/**
 * @brief
**/
bool test_get_installed_programs()
{
	std::list<pprogram> softwares;
	_ASSERTE(true == get_installed_programs(softwares));

	for (auto software : softwares)
	{
		log_info
			"product code: %ws, name(%ws)-vender(%ws)-version(%ws)",
			software->id(),
			software->name(),
			software->vendor(),
			software->version()
			log_end;
		delete software;
	}

	return true;
}

/**
 * @brief
**/
bool test_rc4_encrypt()
{
	const char* key = "coresecurity";
	const char plain[] = "abcdefghijklmnop1234567890!@#$%^&*()�����ٶ󸶹ٻ������īŸ����";
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


    wchar_t* file_path = L"c:\\windows\\system32\\notepad.exe";

    HANDLE file_handle = CreateFileW(
                            file_path,
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
            log_err
                "ReadFile( %ws ) failed. gle = %u",
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
	log_info "tid = %u, running", GetCurrentThreadId() log_end;
};

struct worker
{
    void operator()()
    {
		log_info "tid = %u, running", GetCurrentThreadId() log_end;
    };
};

void more_work( int v)
{
	log_info "tid = %u, running = %d", GetCurrentThreadId(), v log_end;
    //getchar();
};

class RunClass
{
public:
	bool CalledByThread(_In_ const char* msg)
	{
		log_info "tid=%u, msg=%s",
			GetCurrentThreadId(),
			msg
			log_end;
		return true;
	}
};


bool test_thread_pool()
{
    thread_pool pool(8);
    pool.run_task( work );                        // Function pointer.
    pool.run_task( worker() );                    // Callable object.
    pool.run_task( boost::bind( more_work, 5 ) ); // Callable object.
    pool.run_task( worker() );                    // Callable object.

	pool.run_task([]()								// lambda
	{
		log_info "tid=%u",GetCurrentThreadId() log_end;
	});


	RunClass rc;
	pool.run_task([&]()
	{
		if (true != rc.CalledByThread("test msg"))
		{
			log_err "rc.CalledByThread() failed." log_end;
		}
		else
		{
			log_info "rc.CalledByThread() succeeded." log_end;
		}
	});


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
                log_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
            }
            else
            {
                std::vector<std::string> dumps;
                dump_memory(0, buf, sizeof(buf), dumps);
                //for (auto line : dumps)
                //{
                //    log_info "%s", line.c_str() log_end;
                //}
                log_info "%ws, \n%s", path.str().c_str(), dumps[dumps.size() - 2].c_str() log_end;
            }
        }
        else
        {
            log_err
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
            log_err "get_vbr_offset( disk_number = %u ) failed.", disk_number);
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
            log_err
                "CreateFile( %ws ) failed. gle = %u",
                path.str().c_str(),
                GetLastError());
            return false;
        }
		handle_ptr handle_guard(disk, [](HANDLE h) {CloseHandle(h);});

        // dump MBR
        li_distance.QuadPart = 0;
        if (!SetFilePointerEx(disk, li_distance, &li_new_pos, FILE_BEGIN))
        {
            log_err
                "SetFilePointerEx() failed, gle = %u", GetLastError()
                log_end;
            break;
        }

        if (!ReadFile(disk, buf, sizeof(buf), &bytes_returned, NULL))
        {
            log_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
        }
        else
        {
            std::vector<std::string> dump;
            dump_memory(0, buf, sizeof(buf), dump);
            log_info "[*] MBR" log_end
            for (auto line : dump)
            {
                log_info "%s", line.c_str() log_end
            }
        }

        // dump VBRs
        for (auto vbr_info : dvi._vbrs)
        {
            if (true != vbr_info.recognized) { continue; }

            li_distance = vbr_info.offset;
            if (!SetFilePointerEx(disk, li_distance, &li_new_pos, FILE_BEGIN))
            {
                log_err
                    "SetFilePointerEx() failed, gle = %u", GetLastError()
                    log_end;
                break;
            }

            if (!ReadFile(disk, buf, sizeof(buf), &bytes_returned, NULL))
            {
                log_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
            }
            else
            {
                log_info "[*] VBR" log_end
                std::vector<std::string> dump;
                dump_memory(0, buf, sizeof(buf), dump);
                for (auto line : dump)
                {
                    log_info "%s", line.c_str() log_end
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
    log_err
        "This test writes MBR and VBR, may cause serious system damage. really want to do this? ok~"
        log_end
    _pause;

    ///
    /// buf �� offset �� 512 (���� ������)�� align �Ǿ����� ������
    /// Read/WriteFile �Լ����� ���� (87) �߻���
    /// ���߿� �ѹ� Ȯ���غ�
    ///
    uint8_t buf_read[512] = { 0x00 };
    uint8_t buf_write[512] = { 0x41 };

    std::vector<uint32_t> disk_numbers;
    bool ret = get_disk_numbers(disk_numbers);
    if (true != ret)
    {
        log_err "get_disk_numbers() failed." log_end;
        return false;
    }

    for (auto disk_number : disk_numbers)
    {
        disk_volume_info dvi(disk_number);

        if (true != get_disk_volume_info(dvi))
        {
            log_err "get_vbr_offset( disk_number = %u ) failed.", disk_number);
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
            log_err
                "CreateFile( %ws ) failed. gle = %u",
                path.str().c_str(),
                GetLastError());
            return false;
        }
        handle_ptr handle_guard(disk, [](HANDLE h) {CloseHandle(h);});

        // #1 write mbr - backup
        if (true != read_file_offset(disk, 0, buf_read, sizeof(buf_read)))
        {
            log_err "read_file_offset() failed" log_end;
        }

        log_info "[*] %uth disk MBR - before write", disk_number log_end;
        dump_file_offset(disk, 0, sizeof(buf_read));

        // #2 write mbr - write
        if (true != write_file_offset(disk, 0, buf_write, sizeof(buf_write)))
        {
            log_err "write_file_offset(mbr) failed." log_end;
            return false;
        }
        log_info "[*] %uth disk MBR - after write", disk_number log_end;
        dump_file_offset(disk, 0, sizeof(buf_read));

        // #3 write mbr - restore
        if (true != write_file_offset(disk, 0, buf_read, sizeof(buf_read)))
        {
            log_err "write_file_offset(mbr) failed. (restore failed)" log_end;
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
                log_err "read_file_offset() failed" log_end;
            }

            log_info "[*] %uth disk, %uth VBR - before write", disk_number, ix log_end;
            dump_file_offset(disk, vbr_info.offset.QuadPart, sizeof(buf_read));

            // #2 write vbr - write
            if (true != write_file_offset(disk, vbr_info.offset.QuadPart, buf_write, sizeof(buf_write)))
            {
                log_err "write_file_offset(mbr) failed." log_end;
                return false;
            }
            log_info "[*] %uth disk, %uth VBR - after write", disk_number, ix log_end;
            dump_file_offset(disk, vbr_info.offset.QuadPart, sizeof(buf_read));

            // #3 write vbr - restore
            if (true != write_file_offset(disk, vbr_info.offset.QuadPart, buf_read, sizeof(buf_read)))
            {
                log_err "write_file_offset(mbr) failed. (restore failed)" log_end;
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
        log_err
            "SetFilePointerEx() failed, gle = %u", GetLastError()
            log_end;
        return false;
    }

    RtlZeroMemory(buf, size);
    if (!ReadFile(file_handle, buf, size, &bytes_rw, NULL))
    {
        log_err "ReadFile( ) failed. gle = %u", GetLastError() log_end;
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
        log_err
            "SetFilePointerEx() failed, gle = %u", GetLastError()
            log_end;
        return false;
    }

    if (!WriteFile(file_handle, buf, size, &bytes_rw, NULL))
    {
        log_err
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
        log_err "read_file_offset() failed." log_end;
        return;
    }

    std::vector<std::string> dump;
    if (true != dump_memory(0, buf, size, dump))
    {
        log_err "dump_memory(0,  ) failed." log_end;
        return;
    }

    for (auto line : dump)
    {
        log_info "%s", line.c_str() log_end
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

        // std::regexp �� `\` �� Ư�����ڷ� �ν�, �ѹ��� escape �ϱ� ������
        // `\device` ���ڿ��� ��Ī�ϱ� ���ؼ���
        // `\\device` ������ ����ؾ� �Ѵ�.
        //
        // c++ ������ `\` �� escape �ϹǷ� �ᱹ `\` ���ڸ� ��Ī�ϱ� ���ؼ���
        // `\\\\device` ������ ����ؾ� �Ѵ�.
        //
        // c++11 raw string, `LR"( )"` �� ����ϸ� c++ �� escape �� ���� �� �����ϱ�
        // LR"(\\device)" ������ ����ؼ� ��Ī�� �� �ִ�.
        //
        std::wstring str(L"\\device");
        std::wregex exp(L"\\\\device");   // wregex �� `\` �� �ѹ��� �� escape �ϹǷ� `\\` �� ����
        std::wregex expr(LR"(\\device)");

        std::wsmatch wsm;
        _ASSERTE(true == std::regex_match(str, wsm, exp));  // ��Ȯ�� ��ġ�ϸ� ��Ī
        _ASSERTE(true == std::regex_match(str, wsm, expr)); // ��Ȯ�� ��ġ�ϸ� ��Ī



        //wchar_t* r = LR"(\\device\\mup\\(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}))";
        //std::wregex exp(r, std::regex_constants::ECMAScript | std::regex_constants::icase);

        //for (auto s : strs)
        //{
        //    std::wstring ws(s);
        //    std::wsmatch wsm;
        //    if (std::regex_search(ws, wsm, exp))        // sub string ��Ī
        //    {
        //        for (size_t i = 0; i < wsm.size(); i++)
        //        {
        //            std::wcout << i << "��° : " << wsm[i] << std::endl;
        //        }
        //    }
        //}

        //std::wstring str(L"\\device");
        //std::wregex exp(L"\\\\de");   // wregex �� `\` �� �ѹ��� �� escape �ϹǷ� `\\` �� ����
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
        log_err "regex_error caught: %s", e.what() log_end;

        if (e.code() == std::regex_constants::error_brack)
        {
            log_err "The code was error_brack" log_end;
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
            if (std::regex_search(str, wcm, exp))        // sub string ��Ī
            {
                _ASSERTE(2 == wcm.size());
                log_info "str=%ws, match=%ws, ip=%ws", str, wcm[0].str().c_str(), wcm[1].str().c_str() log_end;
            }
        }

    }
    catch (const std::regex_error& e)
    {
        log_err "regex_error caught: %s", e.what() log_end;

        if (e.code() == std::regex_constants::error_brack)
        {
            log_err "The code was error_brack" log_end;
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

bool test_NameConverter_iterate()
{
	NameConverter nc;
	_ASSERTE(true == nc.load(false));

	uint32_t count = 0;
	_ASSERTE(true == nc.iterate_dos_devices(
		[](_In_ const DosDeviceInfo* ddi, _In_ ULONG_PTR tag)
		{
			if (nullptr == ddi) { return false; }

			uint32_t* count = (uint32_t*)tag;
			++(*count);

			log_info "dos device, %ws (%ws)",
				ddi->_logical_drive.c_str(),
				ddi->_device_name.c_str()
				log_end;
			return true;
		},
		(ULONG_PTR)&count));
	_ASSERTE(count > 0);	// logical driver �� 1�� �̻��� �ݵ�� �����Ƿ�

	// callback �� false �� �����ϸ� iterate �� �����ϰ�, true �� �����Ѵ�.
	count = 0;
	_ASSERTE(true == nc.iterate_dos_devices(
		[](_In_ const DosDeviceInfo* ddi, _In_ ULONG_PTR tag)
		{
			UNREFERENCED_PARAMETER(ddi);
			UNREFERENCED_PARAMETER(tag);
			return false;
		},
		(ULONG_PTR)&count));
	_ASSERTE(count == 0);

	//_ASSERTE(false == nc.iterate_dos_devices(nullptr, 0));

	return true;
}

bool test_NameConverter_get_canon_name()
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

	//[INFO] \Device\WebDavRedirector\192.168.0.1\DavWWWRoot\ -> \\192.168.0.1\davwwwroot\
	//[INFO] \Device\Mup\192.168.0.1\ -> \\192.168.0.1\
	//[INFO] \Device\Mup\192.168.0.1\temp.*\ -> \\192.168.0.1\temp.*\
	//[INFO] \Device\Mup\192.168.59.134\ADMIN$\PSEXESVC.EXE -> \\192.168.59.134\admin$\psexesvc.exe
	//[INFO] \Device\Mup\; Csc\.\.\ -> \Device\csc\.\.\
	//[INFO] \Device\Mup\;          WebDavRedirector\ -> \Device\webdavredirector\
	//[INFO] \Device\Mup\; WebDavRedirector\192.168.0.1\DavWWWRoot\ -> \Device\webdavredirector\192.168.0.1\davwwwroot\
	//[INFO] \Device\Mup\; RdpDr\; :1\192.168.59.134\IPC$\ -> \\192.168.59.134\ipc$\
	//[INFO] \Device\Floppy0\temp\123.txt -> \Device\Floppy0\temp\123.txt
	//[INFO] \Device\CdRom0\temp\123.txt -> \Device\CdRom0\temp\123.txt

	struct path_in_out_pair
	{
		bool ret;
		const wchar_t* in;
		const wchar_t* out;
	}
	file_names[] =
	{
		{ true, L"\\??\\c:\\windows\\system32\\abc.exe", L"c:\\windows\\system32\\abc.exe"},
		{ true, L"\\Systemroot\\abc.exe", L"c:\\Windows\\abc.exe"},
		{ true, L"system32\\abc.exe", L"c:\\windows\\system32\\abc.exe"},
		{ true, L"\\Device\\Mup\\1.1.1.1\\abc.exe", L"\\\\1.1.1.1\\abc.exe"},
		{ false, L"\\Device\\Unknown\\aaaa.exe", nullptr },

        // net use x: \\10.10.10.10\\dbg\\ /user:vmuser * ������� x ����̺� ������ �ξ�� ���������� �׽�Ʈ ����
        //
		{ true, L"\\Device\\Mup\\; lanmanredirector\\; x:000000000008112d\\10.10.10.10\\dbg\\", L"\\\\10.10.10.10\\dbg\\"},
		{ false, L"x:\\", nullptr},
		{ true, L"\\Device\\Mup\\10.10.10.10\\dbg\\", L"\\\\10.10.10.10\\dbg\\"},
		{ true, L"\\Device\\WebDavRedirector\\192.168.0.1\\DavWWWRoot\\", L"\\\\192.168.0.1\\DavWWWRoot\\"},
		{ true, L"\\Device\\Mup\\192.168.0.1\\", L"\\\\192.168.0.1\\"},
		{ true, L"\\Device\\Mup\\192.168.0.1\\temp.*\\", L"\\\\192.168.0.1\\temp.*\\"},
		{ true, L"\\Device\\Mup\\192.168.59.134\\ADMIN$\\PSEXESVC.EXE", L"\\\\192.168.59.134\\ADMIN$\\PSEXESVC.EXE"},
		{ true, L"\\Device\\Mup\\; Csc\\.\\.\\", L"\\Device\\Csc\\.\\.\\"},
		{ true, L"\\Device\\Mup\\;          WebDavRedirector\\", L"\\Device\\WebDavRedirector\\"},
		{ true, L"\\Device\\Mup\\; WebDavRedirector\\192.168.0.1\\DavWWWRoot\\", L"\\Device\\WebDavRedirector\\192.168.0.1\\DavWWWRoot\\"},
		{ true, L"\\Device\\Mup\\; RdpDr\\; :1\\192.168.59.134\\IPC$\\", L"\\\\192.168.59.134\\IPC$\\"},
		{ false, L"\\Device\\Floppy0\\temp\\123.txt", nullptr },// �ý��ۿ� ��Ź���� ���� ����̽�
		{ false, L"\\Device\\CdRom0\\temp\\123.txt", nullptr },// �ý��ۿ� ��Ź���� ���� ����̽�
		{ false, L"\\Device\\HarddiskVolume100\\boot", nullptr },// �ý��ۿ� ��Ź���� ���� ����̽�

		{ true, L"\\windows\\system32\\abc.exe", L"c:\\windows\\system32\\abc.exe"},
		{ true, L"\\Program Files\\", L"c:\\Program Files\\"},
		{ true, L"\\Program Files (x86)", L"c:\\Program Files (x86)"},
		{ true, L"\\Users\\Unsor", L"c:\\Users\\Unsor"}
    };

    NameConverter nc;
	_ASSERTE(true == nc.load(false));

	for (int i = 0; i < sizeof(file_names) / sizeof(path_in_out_pair); ++i)
	{
		std::wstring name;
		bool ret = nc.get_canon_name(file_names[i].in, name);
		bool is_natwork = nc.is_network_path(file_names[i].in);
		bool is_removable = nc.is_removable_drive(file_names[i].in);
		log_info "[ret=%d][net=%d][removable=%d] %ws -> %ws",
			ret,
			is_natwork,
			is_removable,
			file_names[i].in,
			name.c_str()
			log_end;

		_ASSERTE(ret == file_names[i].ret);
		if (file_names[i].ret == true)
		{
			_ASSERTE(0 == _wcsnicmp(file_names[i].out,
									name.c_str(),
									wcslen(file_names[i].out)));
		}
	}

    return true;
}

bool test_NameConverter_dosname_to_devicename()
{
	typedef struct in_out_str
	{
		wchar_t* in;
		wchar_t* out;
	}*pin_out_str;

	//
	//	c: ����̺�(�ɺ�����ũ)�� ����̽� �̸��� ���´�.
	//
	wchar_t device_name[MAX_PATH];
	_ASSERTE(QueryDosDeviceW(L"C:", device_name, MAX_PATH));

	{
		//
		//	get_nt_path_by_dos_path test
		//
		in_out_str in_out_pair[] = {
			{ L"c:\\windows\\system32\\abc.exe",	L"%ws\\windows\\system32\\abc.exe" },
			{ L"c:\\", L"%ws\\" }
		};

		NameConverter nc;
		_ASSERTE(true == nc.load(false));
		for (int i = 0; i < sizeof(in_out_pair) / sizeof(in_out_str); ++i)
		{
			std::wstring nt_path;
			_ASSERTE(true == nc.get_nt_path_by_dos_path(in_out_pair[i].in,
														nt_path));

			wchar_t buf[128];
			StringCbPrintfW(buf, sizeof(buf), in_out_pair[i].out, device_name);
			_ASSERTE(0 == nt_path.compare(buf));
			log_info "%ws -> %ws", in_out_pair[i].in, nt_path.c_str() log_end;
		}
	}

	{
		//
		//	get_device_name_by_drive_letter test
		//
		NameConverter nc;
		_ASSERTE(true == nc.load(false));

		std::wstring device_name_str;
		_ASSERTE(true == nc.get_device_name_by_drive_letter(L"c:", device_name_str));
		_ASSERTE(0 == device_name_str.compare(device_name));

		// �߸��� drive letter ����
		_ASSERTE(true != nc.get_device_name_by_drive_letter(L"c", device_name_str));
		_ASSERTE(true != nc.get_device_name_by_drive_letter(L"c:\\", device_name_str));

	}

	{
		//
		//	get_drive_letter_by_device_name test
		//
		NameConverter nc;
		_ASSERTE(true == nc.load(false));

		// \Device\HarddiskVolume1 ���� �Է�
		std::wstring drive_letter;
		_ASSERTE(true == nc.get_drive_letter_by_device_name(device_name, drive_letter));
		_ASSERTE(0 == _wcsnicmp(L"c:", drive_letter.c_str(), 2));

		// \Device\HarddiskVolume1\ ���� �Է�
		std::wstringstream strm;
		strm << device_name << L"\\";
		_ASSERTE(true == nc.get_drive_letter_by_device_name(strm.str().c_str(), drive_letter));
		_ASSERTE(0 == _wcsnicmp(L"c:", drive_letter.c_str(), 2));

		// \Device\HarddiskVolume1\windows\... ���� �Է�
		clear_str_stream_w(strm);
		strm << device_name << L"\\windows\\system32\\aaa.txt";
		_ASSERTE(true == nc.get_drive_letter_by_device_name(strm.str().c_str(), drive_letter));
		_ASSERTE(0 == _wcsnicmp(L"c:", drive_letter.c_str(), 2));

		// ���� ����̽� (����)
		clear_str_stream_w(strm);
		strm << L"\\Device\\not_exist\\";
		_ASSERTE(true != nc.get_drive_letter_by_device_name(strm.str().c_str(), drive_letter));

		// �߸��� device name ����
		_ASSERTE(true != nc.get_drive_letter_by_device_name(L"\\invalid_device", drive_letter));
		_ASSERTE(true != nc.get_drive_letter_by_device_name(L"invalid_device", drive_letter));

	}

    return true;
}


typedef struct WIN32_FIND_DATAW_ALIGNTEST
{
	DWORD dwFileAttributes;
	uint64_t ftCreationTime;
	uint64_t ftLastAccessTime;
	uint64_t ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD dwReserved0;
	DWORD dwReserved1;
	_Field_z_ WCHAR  cFileName[MAX_PATH];
	_Field_z_ WCHAR  cAlternateFileName[14];
}*PWIN32_FIND_DATAW_ALIGNTEST;

///	@brief	����ü ���� ���� �׽�Ʈ
///			https://msdn.microsoft.com/en-us/library/windows/desktop/ms724284(v=vs.85).aspx
///			https://blogs.msdn.microsoft.com/oldnewthing/20040825-00/?p=38053
///			https://msdn.microsoft.com/en-us/library/aa290049(v=vs.71).aspx
///			http://www.ibm.com/developerworks/library/pa-dalign/
bool test_alignment_error_test()
{
	const wchar_t* root = L"c:\\dbg";

	WIN32_FIND_DATAW wfd = { 0 };
	HANDLE hSrch = FindFirstFileW(root, &wfd);

	//	������ 1
	//
	//	PWIN32_FIND_DATAW ����ü�� �����ϴ� �������� ����� 4����Ʈ�̹Ƿ�
	//	4����Ʈ ���ĵȴ�. (FILE_TIME ����ü�� 4����Ʈ �ΰ��̹Ƿ�)
	//	PWIN32_FIND_DATAW_ALIGNTEST ����ü�� ��� uint64_t �� �����Ƿ� 8����Ʈ
	//	������ �ȴ�.
	//
	//	����  PWIN32_FIND_DATAW �� PWIN32_FIND_DATAW_ALIGNTEST �� Ÿ��ĳ�����ϰ�
	//	�Ǹ� ������ �߻��Ѵ�. DWORD dwFileAttributes ���� 4����Ʈ�� �е����� �νĵǾ�
	//	4����Ʈ�� �з����� �ȴ�.
	//
	PWIN32_FIND_DATAW_ALIGNTEST at = (PWIN32_FIND_DATAW_ALIGNTEST)&wfd;
	at = at;

	//	������ 2
	//
	//	wfd.ftCreationTime �� �ּҸ� uint64_t �� ĳ�����ؼ� ����ϱ� ������
	//	�е��̳� ���Ļ��� ������ ����. ������ 8����Ʈ ������ ����ϴ� �ý����� ���(x64)
	//	ftCreationTime �ּҴ� 4����Ʈ ���ĵ� �ּ��̱� ������ 8����Ʈ�� �б� ����
	//	8����Ʈ�� �ι� �о 4����Ʈ�� �ɰ��� ���ľ� �ϴ°�찡 �߻��� �� �ִ�.
	//
	//	0123 4567 | 789a bcde
	//       ----   ----
	//		  (1)   (2)
	//
	//	x86 ó�� 4����Ʈ ������ �ϴ� �ý����̶�� ������ ���� �� �ְ�����
	//	x64 ó�� 8����Ʈ ������ �ϴ� �ý����̶�� (1) �ּ��� 8����Ʈ�� �б� ����
	//	�ι��� �б� ������ �ʿ��ϴ�.
	//
	//	�̰� üũ�� �ִ� ��ũ�ΰ� IS_ALIGNED() ��ũ����.
	//	�ᱹ FILE_TIME �� uint64_t �� ĳ�����ϴ°��� �ſ� ������ �����̸�
	//	�׷��� Ÿ�� ĳ���� ���� ����, high/low part �� LARGE_INTEGER �� �����Ѵ㿡
	//	LARGE_INTEGER.QuadPart �� ���ؼ� ������ �϶�� �ϴ°��̴�.
	//
	uint64_t* p = (uint64_t*)&wfd.ftCreationTime;
	p = p;

	hSrch = hSrch;
	return true;
}

bool test_crc64()
{
    log_info "e9c6d914c4b8d9ca == %016llx",
        (unsigned long long) crc64(0, (unsigned char*)"123456789", 9)
        log_end;
    return true;
}

class TestClass
{
public:
	TestClass() {}
	uintptr_t addr() { return (uintptr_t)this; }
private:
	int _v1;
	int _v2;
};

bool test_file_info_cache()
{
	const wchar_t* test_file_1 = L"c:\\windows\\system32\\notepad.exe";
	std::wstringstream db;
	db << get_current_module_dirEx() << L"\\file_info_cache.db";
	{
		FileInfoCache cache;
		_ASSERTE(true == cache.initialize(db.str().c_str(), true));

		//
		//	FileInfoCache Ŭ���� �̿�
		//
		FileInformation fi1;
		FileInformation fi2;
		_ASSERTE(true == cache.get_file_information(test_file_1, fi1));
		_ASSERTE(true == cache.get_file_information(test_file_1, fi2));
		_ASSERTE(1 == cache.size());
		_ASSERTE(1 == cache.hit_count());
		_ASSERTE(0 == fi1.sha2.compare(fi2.sha2));
	}
	DeleteFileW(db.str().c_str());

	//
	//	FileInfoCache C api �̿�
	//
	_ASSERTE(true == fi_initialize());
	FileInformation fic1;
	_ASSERTE(true == fi_get_file_information(test_file_1, fic1));
	FileInformation fic2;
	_ASSERTE(true == fi_get_file_information(test_file_1, fic2));
	_ASSERTE(0 == fic1.sha2.compare(fic2.sha2));
	fi_finalize();		///<!
	return true;
}

bool test_create_guid()
{
	GUID guid;
	GUID guid2;
	_ASSERTE(true == create_guid(guid));

	// guid -> string -> guid
	std::string guid_string = guid_to_string(guid);
	_ASSERTE(string_to_guid(guid_string.c_str(), guid2));
	_ASSERTE(IsEqualGUID(guid, guid2));

	log_info "guid=%s",
		guid_string.c_str()
		log_end;
	return true;
}

bool test_is_executable_file_w()
{
	_ASSERTE(IT_UNKNOWN == get_image_type(L"c:\\work.mylib\\test_upx_invalid_lfanew.exe"));
	_ASSERTE(IT_EXE_GUI == get_image_type(L"c:\\windows\\system32\\notepad.exe"));
	return true;
}

TestClass* test_singleton_subcall()
{
	return Singleton<TestClass>::GetInstancePointer();
}

bool test_singleton()
{
	TestClass* o = Singleton<TestClass>::GetInstancePointer();
	TestClass* o2 = Singleton<TestClass>::GetInstancePointer();
	TestClass* o3 = test_singleton_subcall();
	_ASSERTE(o == o2);
	_ASSERTE(o == o3);
	log_info "o=0x%p, o2=0x%p, o3=0x%p",
		o->addr(),
		o2->addr(),
		o3->addr()
		log_end;

	Singleton<TestClass>::ReleaseInstance();
	Singleton<TestClass>::ReleaseInstance();
	Singleton<TestClass>::ReleaseInstance();
	return true;
}


bool test_log_xxx()
{
	_ASSERTE(attach_console());

	set_log_env(log_mask_all, log_level_debug, log_to_con);
	log_dbg "you can see this log, console" log_end;
	log_info "you can see this log, console" log_end;
	log_warn "you can see this log, console" log_end;
	log_err "you can see this log, console" log_end;

	set_log_env(log_mask_all, log_level_error, log_to_ods);
	log_dbg "you can't see this log, ods" log_end;
	log_info "you can't see this log, ods" log_end;
	log_warn "you can't see this log, ods" log_end;
	log_err "you can see this log, ods" log_end;

	set_log_env(log_mask_all, log_level_error, log_to_file);
	log_dbg "you can't see this log, file" log_end;
	log_info "you can't see this log, file" log_end;
	log_warn "you can't see this log, file" log_end;
	log_err "you can see this log, file" log_end;

	set_log_env(log_mask_all, log_level_debug, log_to_all);
	return true;
}

bool test_set_security_attributes()
{
	//
	//	security_attributes.txt ������ �����Ѵ�.
	//
	//	������ ������ local system �������θ� ���� �����ϱ� ������ �׳� ������ �� ����.
	//	����->�Ӽ�->����->���->�����ں��� ���� ���� ���� ����ڿ��� �ʿ��� ������ �־��
	//	�Ѵ�.
	//
	std::wstringstream file_path;
	file_path << get_current_module_dirEx() << L"\\security_attributes.txt";
	if (is_file_existsW(file_path.str().c_str()))
	{
		DeleteFileW(file_path.str().c_str());
	}

	SECURITY_ATTRIBUTES sa;
	_ASSERTE(true == set_security_attributes_type2(sa));

	HANDLE file_handle = CreateFileW(file_path.str().c_str(),
									 FILE_ALL_ACCESS,
									 FILE_SHARE_READ | FILE_SHARE_WRITE,
									 &sa,
									 CREATE_NEW,
									 FILE_ATTRIBUTE_NORMAL,
									 NULL);
	if (INVALID_HANDLE_VALUE == file_handle)
	{
		log_err "CreateFileW() failed. file=%ws, gle=%u",
			file_path.str().c_str(),
			GetLastError()
			log_end;
		return false;
	}

	CloseHandle(file_handle);
	return true;
}

bool test_GeneralHashFunctions()
{
	char* _known_exts[] = {
		"crt", "pptm", "ppt", "pdb",
		"p7c", "xps", "ptx", "js",
		"xls", "nrw", "pdd", "ini",
		"docm", "ogg", "tiff", "exe",
		"pyd", "png", "odt", "nc",
		"arw", "crw", "sys", "key",
		"class", "mkv", "accdb", "pptx",
		"eps", "xlsm", "gif", "mp4",
		"ai", "rc", "x3f", "psd",
		"py", "one", "pef", "nef",
		"dbf", "rtf", "config", "dng",
		"rdp", "cs", "cer", "lnk",
		"vmx", "css", "db3", "dl",
		"sqlite", "p7b", "db", "wma",
		"asp", "bay", "cdr", "rw2",
		"mdb", "doc", "rar", "cr2",
		"pst", "lic", "csv", "wps",
		"mdf", "tap", "p12", "xm",
		"xlsx", "mp3", "jpeg", "docx",
		"flac", "dxg", "dxf", "java",
		"dwg", "h", "ods", "asm",
		"pdf", "c", "kdc", "hwp",
		"mef", "cpp", "raw", "pyc",
		"vmdk", "dcr", "srw", "txt",
		"der", "7z", "lib", "flv",
		"inf", "pfx", "tif", "wb2",
		"odb", "def", "pem", "srf",
		"dat", "zip", "wmv", "ovf",
		"uti", "sq", "avi", "cat",
		"php", "xlsb", "odp", "bmp",
		"sln", "jpg"
	};

	//
	//	GernalHashFunctions ���� �浹���� Ȯ��
	//
	struct functions
	{
		hash_function func;
		char* name;
	} _functions[] = {
		{ RSHash, "RSHash" },
		{ JSHash, "JSHash" },
		{ PJWHash, "PJWHash" },
		{ ELFHash, "ELFHash" },
		{ BKDRHash, "BKDRHash" },
		{ SDBMHash, "SDBMHash" },
		{ DJBHash, "DJBHash" },
		{ DEKHash, "DEKHash" },
		{ BPHash, "BPHash" },
		{ FNVHash, "FNVHash" },
		{ APHash, "APHash" },
		{ [](char* str, unsigned int len)->unsigned int
			{
				UNREFERENCED_PARAMETER(len);
				return hash_string32(str);
			},
			"hash_string32" }
	};


	for (int i = 0; i < sizeof(_functions) / sizeof(functions); ++i)
	{
		uint32_t collision_count = 0;
		std::unordered_map<uint32_t, char*> _tbl;
		StopWatch sw;
		sw.Start();
		for (int j = 0; j < sizeof(_known_exts) / sizeof(char*); ++j)
		{
			uint32_t hash = _functions[i].func(_known_exts[j], (uint32_t)strlen(_known_exts[j]));
			auto entry = _tbl.find(hash);
			if (entry == _tbl.end())
			{
				_tbl.insert(std::make_pair<uint32_t, char*>(std::move(hash),
															std::move(_known_exts[j])));
			}
			else
			{
				// �浹
				log_info "collision, prev=%s, curr=%s, hash=%u",
					entry->second,
					_known_exts[j],
					hash
					log_end;
				collision_count++;
			}
		}
		sw.Stop();
		log_info "func=%s, count=%u, collision=%u, time elapsed=%f msecs",
			_functions[i].name,
			sizeof(_known_exts) / sizeof(char*),
			collision_count,
			sw.GetDurationMilliSecond()
			log_end;
	}


	return true;
}

/*

### ��� ###
std::unordered_map<std::string, std::string>  �� ����� ������


����� ���������� std::map �� std::unordered_map ���� ��������,
������ ���������� std::unordered_map �� Ȯ���� ������.

�浹������ BPHash �� �浹�� �ʹ� ����. ����...�������� ������
hash_string32() �� ����ܷ� ������!

std::unordered_map<std::string, std::string> �� ����ϸ� �˾Ƽ�
�浹ó���� ���� (collision = 0). �ӵ����� 2�� ���� ���������� ���̺���
ī��Ʈ�� �� 14�������� �����ϸ� �� ����� �����ϴ� (���ϰ�!)




# ������ ����

[INFO] # std::map<uint32_t, std::string>, file=145718
[INFO] func=RSHash, collision=3, time elapsed=108.565140 msecs
[INFO] func=JSHash, collision=11, time elapsed=118.081841 msecs
[INFO] func=PJWHash, collision=254, time elapsed=122.064140 msecs
[INFO] func=ELFHash, collision=254, time elapsed=117.629372 msecs
[INFO] func=BKDRHash, collision=3, time elapsed=108.026054 msecs
[INFO] func=SDBMHash, collision=2, time elapsed=102.761192 msecs
[INFO] func=DJBHash, collision=2, time elapsed=106.163803 msecs
[INFO] func=DEKHash, collision=66, time elapsed=98.259743 msecs
[INFO] func=BPHash, collision=141966, time elapsed=30.714870 msecs
[INFO] func=FNVHash, collision=3, time elapsed=103.456375 msecs
[INFO] func=APHash, collision=4, time elapsed=120.516907 msecs
[INFO] func=hash_string32, collision=1, time elapsed=121.895775 msecs

[INFO] # std::unordered_map<uint32_t, std::string>, file=145718
[INFO] func=RSHash, collision=3, time elapsed=74.703918 msecs
[INFO] func=JSHash, collision=11, time elapsed=74.806114 msecs
[INFO] func=PJWHash, collision=254, time elapsed=83.417625 msecs
[INFO] func=ELFHash, collision=254, time elapsed=85.153160 msecs
[INFO] func=BKDRHash, collision=3, time elapsed=72.944366 msecs
[INFO] func=SDBMHash, collision=2, time elapsed=76.123161 msecs
[INFO] func=DJBHash, collision=2, time elapsed=72.482445 msecs
[INFO] func=DEKHash, collision=66, time elapsed=70.576500 msecs
[INFO] func=BPHash, collision=141966, time elapsed=27.657179 msecs
[INFO] func=FNVHash, collision=3, time elapsed=73.280594 msecs
[INFO] func=APHash, collision=4, time elapsed=82.607208 msecs
[INFO] func=hash_string32, collision=1, time elapsed=81.336922 msecs

[INFO] # std::unordered_map<std::string, std::string>, file=145718
[INFO] func=std::unordeded_map, collision=0, time elapsed=140.758255 msecs



# ����� ����
[INFO] # std::map<uint32_t, std::string>, file=145718
[INFO] func=RSHash, collision=3, time elapsed=2783.447021 msecs
[INFO] func=JSHash, collision=11, time elapsed=2711.636963 msecs
[INFO] func=PJWHash, collision=254, time elapsed=2768.272705 msecs
[INFO] func=ELFHash, collision=254, time elapsed=2814.179932 msecs
[INFO] func=BKDRHash, collision=3, time elapsed=2766.507324 msecs
[INFO] func=SDBMHash, collision=2, time elapsed=2753.156982 msecs
[INFO] func=DJBHash, collision=2, time elapsed=2693.644531 msecs
[INFO] func=DEKHash, collision=66, time elapsed=2767.681641 msecs
[INFO] func=BPHash, collision=141966, time elapsed=1065.056396 msecs
[INFO] func=FNVHash, collision=3, time elapsed=2765.506348 msecs
[INFO] func=APHash, collision=4, time elapsed=2695.671387 msecs
[INFO] func=hash_string32, collision=1, time elapsed=2740.690674 msecs

[INFO] # std::unordered_map<uint32_t, std::string>, file=145718
[INFO] func=RSHash, collision=3, time elapsed=2868.202393 msecs
[INFO] func=JSHash, collision=11, time elapsed=2942.227783 msecs
[INFO] func=PJWHash, collision=254, time elapsed=2929.952393 msecs
[INFO] func=ELFHash, collision=254, time elapsed=2964.694824 msecs
[INFO] func=BKDRHash, collision=3, time elapsed=2932.376709 msecs
[INFO] func=SDBMHash, collision=2, time elapsed=2912.048096 msecs
[INFO] func=DJBHash, collision=2, time elapsed=2880.731689 msecs
[INFO] func=DEKHash, collision=66, time elapsed=2923.981689 msecs
[INFO] func=BPHash, collision=141966, time elapsed=862.678833 msecs
[INFO] func=FNVHash, collision=3, time elapsed=2875.431152 msecs
[INFO] func=APHash, collision=4, time elapsed=2892.405273 msecs
[INFO] func=hash_string32, collision=1, time elapsed=2874.231689 msecs

[INFO] # std::unordered_map<std::string, std::string>, file=145718
[INFO] func=std::unordeded_map, collision=0, time elapsed=3553.741699 msecs

*/
bool test_GeneralHashFunctions2()
{
	std::list<std::string> _files;

	if (!find_files(L"c:\\windows",
					reinterpret_cast<DWORD_PTR>(&_files),
					true,
					[](_In_ DWORD_PTR tag, _In_ const wchar_t* path)
	{
		char_ptr cptr(WcsToMbs(path), [](char* p) {if (nullptr != p) free(p); });
		if (nullptr == cptr) return true;

		std::list<std::string>* files = (std::list<std::string>*)(tag);
		files->push_back(cptr.get());
		return true;

	}))
	{
		return false;
	}

	//
	//	GernalHashFunctions ���� �浹���� Ȯ��
	//
	struct functions
	{
		hash_function func;
		char* name;
	} _functions[] = {
		{ RSHash, "RSHash" },
		{ JSHash, "JSHash" },
		{ PJWHash, "PJWHash" },
		{ ELFHash, "ELFHash" },
		{ BKDRHash, "BKDRHash" },
		{ SDBMHash, "SDBMHash" },
		{ DJBHash, "DJBHash" },
		{ DEKHash, "DEKHash" },
		{ BPHash, "BPHash" },
		{ FNVHash, "FNVHash" },
		{ APHash, "APHash" },
		{ [](char* str, unsigned int len)->unsigned int
			{
				UNREFERENCED_PARAMETER(len);
				return hash_string32(str);
			},
			"hash_string32" }
	};


	//
	//	std::map ���
	//
	log_info "# std::map<uint32_t, std::string>, file=%u", _files.size() log_end;
	for (int i = 0; i < sizeof(_functions) / sizeof(functions); ++i)
	{
		uint32_t collision_count = 0;
		std::map<uint32_t, std::string> _tbl;
		StopWatch sw;
		sw.Start();
		for (auto file : _files)
		{
			uint32_t hash = _functions[i].func(const_cast<char*>(file.c_str()),
											   (uint32_t)strlen(file.c_str()));
			auto entry = _tbl.find(hash);
			if (entry == _tbl.end())
			{
				_tbl.insert(std::make_pair<uint32_t, std::string>(std::move(hash),
															std::move(file)));
			}
			else
			{
				// �浹
				//if (0 == _strnicmp(_functions[i].name, "hash_string32", strlen("hash_string32")))
				//{
				//	log_info "collision, prev=%s, curr=%s, hash=%u",
				//		entry->second.c_str(),
				//		file.c_str(),
				//		hash
				//		log_end;
				//}
				collision_count++;
			}
		}
		sw.Stop();
		log_info "func=%s, collision=%u, time elapsed=%f msecs",
			_functions[i].name,
			collision_count,
			sw.GetDurationMilliSecond()
			log_end;
	}

	//
	//	std::unordered_map ���
	//
	log_info "# std::unordered_map<uint32_t, std::string>, file=%u", _files.size() log_end;
	for (int i = 0; i < sizeof(_functions) / sizeof(functions); ++i)
	{
		uint32_t collision_count = 0;
		std::unordered_map<uint32_t, std::string> _tbl;
		StopWatch sw;
		sw.Start();
		for (auto file : _files)
		{
			uint32_t hash = _functions[i].func(const_cast<char*>(file.c_str()),
				(uint32_t)strlen(file.c_str()));
			auto entry = _tbl.find(hash);
			if (entry == _tbl.end())
			{
				_tbl.insert(std::make_pair<uint32_t, std::string>(std::move(hash),
																  std::move(file)));
			}
			else
			{
				// �浹
				//if (0 == _strnicmp(_functions[i].name, "hash_string32", strlen("hash_string32")))
				//{
				//	log_info "collision, prev=%s, curr=%s, hash=%u",
				//		entry->second.c_str(),
				//		file.c_str(),
				//		hash
				//		log_end;
				//}
				collision_count++;
			}
		}
		sw.Stop();
		log_info "func=%s, collision=%u, time elapsed=%f msecs",
			_functions[i].name,
			collision_count,
			sw.GetDurationMilliSecond()
			log_end;
	}

	//
	//	std::unordered_map<std::string, std::string> ���
	//
	log_info "# std::unordered_map<std::string, std::string>, file=%u", _files.size() log_end;
	{
		uint32_t collision_count = 0;
		std::unordered_map<std::string, std::string> _tbl;
		StopWatch sw;
		sw.Start();
		for (auto file : _files)
		{
			auto entry = _tbl.find(file);
			if (entry == _tbl.end())
			{
				_tbl.insert(std::make_pair<std::string, std::string>(std::move(file),
																	 std::move(file)));
			}
			else
			{
				// �浹
				//if (0 == _strnicmp(_functions[i].name, "hash_string32", strlen("hash_string32")))
				//{
				//	log_info "collision, prev=%s, curr=%s, hash=%u",
				//		entry->second.c_str(),
				//		file.c_str(),
				//		hash
				//		log_end;
				//}
				collision_count++;
			}
		}
		sw.Stop();
		log_info "func=std::unordeded_map, collision=%u, time elapsed=%f msecs",
			collision_count,
			sw.GetDurationMilliSecond()
			log_end;
	}

	return true;
}

bool test_get_file_extension()
{
	//
	//	Ȯ���ڸ� ���� ��θ� �� ó���ϴ��� Ȯ��
	//
	wchar_t* file_with_ext[] = {
		L"abc.txt",
		L"c:\\windows\\xbadsad\\aaaa.txt",
		L"aaaa.txt",
		L"aaaa.doc.txt",
		L"\\offsymxl.txt:WofCompressedData"		// ttf
	};

	for (int i = 0; i < sizeof(file_with_ext) / sizeof(wchar_t*); ++i)
	{
		std::wstring ext;
		_ASSERTE(true == get_file_extensionw(file_with_ext[i], ext));
		_ASSERTE(0 == ext.compare(L"txt"));
		log_info "file=%ws, ext=%ws", file_with_ext[i], ext.c_str() log_end;
	}

	//
	//	Ȯ���ڰ� ���� ���� ó�� Ȯ��
	//
	wchar_t* file_without_ext[] = {
		L"c:\\windows\\xbadsad\\aaaa",
		L"aaaa",
		L"aaaadoc_txt"

	};

	for (int i = 0; i < sizeof(file_without_ext) / sizeof(wchar_t*); ++i)
	{
		std::wstring ext;
		_ASSERTE(false == get_file_extensionw(file_without_ext[i], ext));

		log_info "file=%ws, ext=no ext", file_without_ext[i] log_end;
	}

	return true;
}


bool test_raii_xxx()
{
	//
	//	char* Ÿ�� lambda custom deleter
	//
	do
	{
		char_ptr pchar((char*)malloc(1024), [](char* p) {
			if (nullptr != p)
			{
				free(p);
			}
		});
		if (nullptr == pchar.get()) break;

		RtlFillMemory(pchar.get(), 1024, 0xcc);


		if (nullptr != pchar)
		{
			//
			//	pchar = nullptr �� �����ϴ� ���� deleter �� ȣ���
			//
			pchar = nullptr;
		}

		//
		//	�̹� deleter �� ȣ��Ǿ�����, pchar == nullptr ��
		//
		if (pchar == nullptr)
		{
			break;
		}

	} while (false);


	//
	//	HANDLE Ÿ�� lambda custom deleter
	//
	do
	{
		handle_ptr hptr(open_file_to_read(L"c:\\windows\\system32\\notepad.exe"), [](HANDLE h) {
			if (INVALID_HANDLE_VALUE == h) return;
			if (NULL == h) return;

			CloseHandle(h);
		});

		if (INVALID_HANDLE_VALUE == hptr.get())
		{
			break;
		}

		DWORD bytes_read = 0;
		uint8_t buf[128] = { 0 };
		if (!ReadFile(hptr.get(), buf, sizeof(buf), &bytes_read, NULL))
		{
			log_err "ReadFile() failed. gle=%u",
				GetLastError()
				log_end;
			break;
		}

		std::vector<std::string> dumps;
		if (!dump_memory(0, buf, sizeof(buf), dumps))
		{
			log_err "dump_memory() failed." log_end;
			break;
		}

		for (auto dump : dumps)
		{
			log_info "%s", dump.c_str() log_end;
		}

	} while (false);

	return true;

}

bool test_suspend_resume_process()
{
	//
	//	run notepad.exe
	//
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFOW si = { 0 };
	if (!CreateProcessW(L"c:\\windows\\system32\\notepad.exe",
						nullptr,
						nullptr,
						nullptr,
						FALSE,
						0,
						nullptr,
						nullptr,
						&si,
						&pi))
	{
		log_err "CreateProcessW() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}


	//
	//	suspend notepad
	//
	if (true != suspend_process_by_handle(pi.hProcess)) return false;
	log_info "suspended..." log_end;
	Sleep(1000);

	//
	//	resume notepad
	//
	if (true != resume_process_by_handle(pi.hProcess)) return false;
	log_info "resumed..." log_end;
	Sleep(1000);

	//
	//	terminate notepd
	//
	terminate_process_by_handle(pi.hProcess, 0);
	log_info "terminated..." log_end;
	Sleep(1000);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}


bool test_to_str()
{
#define love 0
#define	you 1

	if (0 != _strnicmp(TO_STR(love), "love", 4)) return false;
	if (0 != _strnicmp(TO_STR(you), "you", 3)) return false;
	if (0 != _strnicmp(TO_STRS(love,you), "loveyou", 7)) return false;
	return true;
}

///
bool test_convert_file_time()
{
	//   FILETIME ft1, ft2, ft3;
	//   GetSystemTimeAsFileTime(&ft1);
	//   Sleep(1000);
	//   GetSystemTimeAsFileTime(&ft2);
	//   log_info "delta = %u", file_time_delta_sec(ft2, ft1) log_end;

	//uint64_t file_time_int = file_time_to_int(ft1);
	//int_to_file_time(file_time_int, &ft3);
	//_ASSERTE(file_time_int == file_time_to_int(ft3));



	//
	//	��¥�� �� ����ؼ� ������Ű�� (SYSTEMTIME �� ���� ��ġ�� �ȵ�)
	//
	SYSTEMTIME system_time_one;
	SYSTEMTIME system_time_two;

	GetSystemTime(&system_time_one);
	system_time_two = system_time_one;

	std::string st_one = sys_time_to_str(&system_time_one, false);
	log_info "st_one=%s", st_one.c_str() log_end;

	system_time_two.wDay++;
	std::string st_two = sys_time_to_str(&system_time_two, false);
	log_info "st_two=%s", st_two.c_str() log_end;

	system_time_two.wDay++;
	st_two = sys_time_to_str(&system_time_two, false);
	log_info "st_two=%s", st_two.c_str() log_end;

	system_time_two.wDay++;
	st_two = sys_time_to_str(&system_time_two, false);
	log_info "st_two=%s", st_two.c_str() log_end;

	system_time_two.wDay++;
	st_two = sys_time_to_str(&system_time_two, false);
	log_info "st_two=%s", st_two.c_str() log_end;

	log_info ".... " log_end;


	//
	//	��¥�� �� ����ؼ� ������Ű��
	//
	//	FILETIME ���� ��ȯ�� �� ���� ����, SYSTEMTIME ���� ��ȯ���ָ� ��
	//

	FILETIME file_time;
	FILETIME file_time_added;

	SystemTimeToFileTime(&system_time_one, &file_time);
	st_two = file_time_to_str(&file_time, false);
	log_info "ft_one=%s", st_two.c_str() log_end;

	file_time_added = add_day_to_file_time(&file_time, 1);
	st_two = file_time_to_str(&file_time_added, false);
	log_info "ft_one=%s", st_two.c_str() log_end;

	file_time_added = add_day_to_file_time(&file_time_added, 1);
	st_two = file_time_to_str(&file_time_added, false);
	log_info "ft_one=%s", st_two.c_str() log_end;

	file_time_added = add_day_to_file_time(&file_time_added, 1);
	st_two = file_time_to_str(&file_time_added, false);
	log_info "ft_one=%s", st_two.c_str() log_end;

	file_time_added = add_day_to_file_time(&file_time_added, 1);
	st_two = file_time_to_str(&file_time_added, false);
	log_info "ft_one=%s", st_two.c_str() log_end;

	_ASSERTE(4 == file_time_delta_day(&file_time_added, &file_time));


	return true;
}

bool test_trivia()
{
	class aaa
	{
	public:
	    aaa(bool value) : _value(value) { }
	    virtual ~aaa() { log_info "..." log_end }

	    void run() { log_info "%s", _value == true ? "Ture" : "False"  log_end;}
	protected:
	    bool _value;
	};

	class bbb : public aaa
	{
	public:
	    bbb(bool value): aaa(value) { }
	    virtual ~bbb() { }
	    void run() { log_info "%s", _value == true ? "Ture" : "False"  log_end;}
	};

	class ccc : public bbb
	{
	public:
	    ccc(bool value) : bbb(value) { }
	    virtual ~ccc() { }
	    void run() { log_info "%s", _value == true ? "Ture" : "False"  log_end;}
	};


	//{
	//	boost::wformat f = boost::wformat(L"%s\\%s") % get_current_module_dirEx().c_str() % L"_MyLib_tst.log";
	//	if (true != initialize_log(log_mask_all,
	//							   log_level_debug,
	//							   log_to_all,
	//							   f.str().c_str())) return;
	//	set_log_format(false, false, true);
	//}
	set_log_format(false, false, false, false);

	//
	//	std::string ����
	//
	std::wstring wstr = L"12345";
	log_info "wstr.size() = %u, wcslen(wstr.c_str()) = %u  (same size)",
		wstr.size(), wcslen(wstr.c_str())
		log_end;
	_ASSERTE(wstr.size() == wcslen(wstr.c_str()));

	std::wstring s = L"";
	_ASSERTE(true == s.empty());

	std::string sy = WcsToMbsEx(s.c_str());
	sy = sy;



	// empty-string.c_str() �� nullptr �� �ƴϴ�!
	const wchar_t* p = nullptr;
	p = s.c_str();
	_ASSERTE(nullptr != p);

	std::wstring ss = L" ";
	_ASSERTE(true != ss.empty());

	//
	//	64��Ʈ ���� ���
	//
	uint64_t t = 0xffffffff00112233;
	log_info "0x%llx, 0x%016llx", t, t log_end;

	uint64_t y = 0x00112233;
	log_info "0x%llx, 0x%016llx", y, y log_end;

	//	list ����
	std::list<int> li;
	li.push_back(1);
	li.push_back(2);
	li.push_back(3);
	for (auto v : li)
	{
		log_info "%d", v log_end;
	}

	//
	//	������/�Ҹ��� ȣ��
	//
	ccc c(true);
	c.run();

	// log :: rotate_log_file() �Լ� �׽�Ʈ
	//for (int i=0; ; ++i)
	//{
	//	log_info "%d", i log_end;
	//	Sleep(500);
	//}


	return true;
}

/// @brief	STATUS_DATATYPE_MISALIGNMENT related test
///			https://devblogs.microsoft.com/oldnewthing/20040826-00/?p=38043
///			https://devblogs.microsoft.com/oldnewthing/20040825-00/?p=38053
bool test_alignment()
{
	WIN32_FIND_DATAW data = { 0 };
	//GetSystemTimeAsFileTime();

	log_info"\n"
		"ftCreationTime     =0x%08x\n"
		"ftLastAccessTime   =0x%08x\n"
		"ftLastWriteTime    =0x%08x\n"
		"nFileSizeHigh      =0x%08x\n",
		(DWORD_PTR)&data.ftCreationTime - (DWORD_PTR)&data,
		(DWORD_PTR)&data.ftLastAccessTime - (DWORD_PTR)&data,
		(DWORD_PTR)&data.ftLastWriteTime - (DWORD_PTR)&data,
		(DWORD_PTR)&data.nFileSizeHigh - (DWORD_PTR)&data
		log_end;

	uint64_t* p = (uint64_t*)&data.ftCreationTime;
	log_info "0x%p, %llu, %s",
		p,
		*p,
		IS_ALIGNED(p, sizeof(uint64_t*)) ? "aligned" : "not aligned"
		log_end;

	return true;
}

/// @brief
bool test_create_string_from_buffer()
{
	//
	//	char �迭�κ��� ������ �����ŭ�� �����ؼ� string ��ü �����ϱ�
	//
	char buf[128] = { 0x00 };
	RtlCopyMemory(&buf[0], "abcd", strlen("abcd"));
	RtlCopyMemory(&buf[16], "defg", strlen("defg"));

	std::string abcde(&buf[0], strlen("abcd"));
	std::string empty(&buf[8], _null_stringa.size() * sizeof(char));
	std::string defg(&buf[16], strlen("abcd"));
	log_info "%s(%u), %s(%u), %s(%u)",
		abcde.c_str(), abcde.size(),
		empty.c_str(), empty.size(),
		defg.c_str(), defg.size()
		log_end;

	//
	//
	//

	wchar_t bufw[128] = { 0x00 };
	RtlCopyMemory(&bufw[0], L"abcd", wcslen(L"abcd") * sizeof(wchar_t));
	RtlCopyMemory(&bufw[16], L"defg", wcslen(L"defg") * sizeof(wchar_t));

	std::wstring wabcde(&bufw[0], wcslen(L"abcd"));
	std::wstring wempty(&bufw[8], wcslen(_null_stringw.c_str()));
	std::wstring wdefg(&bufw[16], wcslen(L"defg"));
	log_info "%ws(%u), %ws(%u), %ws(%u)",
		wabcde.c_str(), wabcde.size(),
		wempty.c_str(), wempty.size(),
		wdefg.c_str(), wdefg.size()
		log_end;

	//
	//	std::string.length(), size()
	//		- ������ count �� ����, �θ޼ҵ� ����
	//
	std::wstring tests = L"0123456789";
	log_info "%ws, length=%u, size=%u",
		tests.c_str(),
		tests.length(),
		tests.size()
		log_end;

	//
	//	std::string.assign()
	//
	char* src = "0123456789";
	std::string assignee;
	std::string r = assignee.assign(&src[0], 4);
	log_info
		"[*] assignee.assign(src[0], 4)\n"
		"	return	=%s \n"
		"	assignee=%s",
		r.c_str(),
		assignee.c_str()
		log_end;

	wchar_t* wsrc = L"0123456789";
	std::wstring wassignee;
	std::wstring wr = wassignee.assign(&wsrc[0], 4);
	log_info
		"[*] assignee.assign(src[0], 4)\n"
		"	return	=%ws \n"
		"	assignee=%ws",
		wr.c_str(),
		wassignee.c_str()
		log_end;

	return true;
}

/// StopWatch Ŭ���� �׽�Ʈ
bool test_stop_watch()
{
	StopWatch sw;
	sw.Start();
	Sleep(1000);
	sw.Stop();
	log_info "%f secs, %f msecs elapsed.",
		sw.GetDurationSecond(),
		sw.GetDurationMilliSecond()
		log_end;
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

/// @brief	bitfield structure memory layout Ȯ��
bool test_bit_field()
{
	typedef struct _bfs
	{
		BYTE    a: 1;
		BYTE    b: 1;
		BYTE    c: 1;
		BYTE    d: 4;
		BYTE    e: 1;

		BYTE    f: 4;
		BYTE    g: 1;
		BYTE    h: 1;
		BYTE    i: 1;
		BYTE    j: 1;
	} bfs;

	bfs v = {
		0b0,
		0b1,
		0b0,
		0b1111,
		0b0,

		0b1111,
		0b0,
		0b1,
		0b0,
		0b1
	};

	// memory layout
	// 0 1 1 1 1 0 1 0		1 0 1 0 1 1 1 1
	// e ---d--- c b a      j i h g ---f---

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
int _tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	_CrtMemState memoryState = { 0 };
	_CrtMemCheckpoint(&memoryState);
	//_CrtSetBreakAlloc(152);

	if (argc == 1)
	{
		set_log_to(log_to_ods|log_to_con);
		set_log_level(log_level_info);

		run_test();
	}
	else
	{
		set_log_to(log_to_con);
		set_log_level(log_level_info);

		if (argv[1][0] != L'/' ||
			argv[1][1] == L'?')
		{
			log_err
				"\nUsage:\n\n"\
				"%ws /?	show help \n"\
				"%ws /filetime_to_str 131618627540824506\n",
				argv[0],
				argv[0]
				log_end;
			return 0;
		}
		//
		//	mylib.exe /filetime_to_str 131618627540824506
		//
		else if (argc == 3 && (0 == _wcsicmp(&argv[1][1], L"filetime_to_str")))
		{
			uint64_t ftime;
			if (true != wstr_to_uint64(argv[2], ftime))
			{
				log_err "wstr_to_uint64() failed. str=%ws",
					argv[2]
					log_end;
				return -1;
			}

			log_info "Input=%llu, Local Time=%s",
				ftime,
				file_time_to_str(ftime, true, true).c_str()
				log_end;
			return 0;
		}
	}

	_CrtMemDumpAllObjectsSince(&memoryState);
	return 0;
}

