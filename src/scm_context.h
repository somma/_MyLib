/**
 * @file    service control manager helper class
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011/11/30 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once
#include <Windows.h>
#include <sal.h>
#include <string>

typedef class scm_context
{
public:
	scm_context(
		_In_z_ const wchar_t* bin_path, 
		_In_z_ const wchar_t* service_name, 
		_In_z_ const wchar_t* service_display_name, 
		_In_ bool win32_service,
		_In_ bool uninstall_service_on_free
		);
	scm_context(
		_In_z_ const wchar_t* bin_path,
		_In_z_ const wchar_t* service_name,
		_In_z_ const wchar_t* service_display_name,
		_In_z_ const wchar_t* altitude, 
		_In_ uint32_t flags, 
		_In_ bool uninstall_service_on_free
		);
	~scm_context();

	bool install_service(_In_ bool auto_start);
	
	bool uninstall_service();
	bool uninstall_service(_In_ SC_HANDLE scm_handle);

	bool start_service();
	bool start_service(_In_ SC_HANDLE service_handle);

	bool stop_service();
	bool stop_service(_In_ SC_HANDLE service_handle);

	bool service_installed(_Out_ bool& installed);
	bool service_installed(_In_ SC_HANDLE scm_handle, _Out_ bool& installed);

	bool service_started(_Out_ bool& started);
	bool service_started(_In_ SC_HANDLE service_handle, _Out_ bool& started);

	bool	
	send_command(
		_In_ uint32_t io_code, 
		_In_ uint32_t input_buffer_size,
		_In_bytecount_(input_buffer_size) void* input_buffer,
		_In_ uint32_t output_buffer_size,
		_In_bytecount_(output_buffer_size) void* output_buffer,
		_Out_ uint32_t* bytes_returned
		);

	const wchar_t* service_name() { return _service_name.c_str(); }
private:
	bool			_uninstall_service_on_free;
	HANDLE			_driver_handle;
	std::wstring	_bin_path;
	std::wstring	_service_name;
	std::wstring	_service_display_name;

	uint32_t		_service_type;

	bool			_is_minifilter;
	std::wstring	_altitude;	// for minifilter
	uint32_t		_flags;		// for minifilter

	HANDLE			open_driver();

} *pscm_context;

