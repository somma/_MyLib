/******************************************************************************
 * scm_context.cpp
 ******************************************************************************
 * 
 ******************************************************************************
 * All rights reserved by somma (fixbrain@gmail.com)
 ******************************************************************************
 * 2011/11/30   created
******************************************************************************/
#pragma once

typedef class scm_context
{
public:
	scm_context(
		_In_z_ const wchar_t* driver_path, 
		_In_z_ const wchar_t* service_name, 
		_In_z_ const wchar_t* service_display_name, 
		_In_ bool uninstall_service_on_free = true
		);
	scm_context(
		_In_z_ const wchar_t* driver_path,
		_In_z_ const wchar_t* service_name,
		_In_z_ const wchar_t* service_display_name,
		_In_z_ const wchar_t* altitude, 
		_In_ uint32_t flags, 
		_In_ bool uninstall_service_on_free = true
		);
	~scm_context();

	bool	install_driver();
	bool	uninstall_driver();
	bool	start_driver();
	bool	stop_driver();
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
	std::wstring	_driver_path;
	std::wstring	_service_name;
	std::wstring	_service_display_name;

	bool			_is_minifilter;
	std::wstring	_altitude;	// for minifilter
	uint32_t		_flags;		// for minifilter

	bool			_installed;
	bool			_running;

	HANDLE			open_driver();

} *pscm_context;

