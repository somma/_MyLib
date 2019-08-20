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

//
//	control services
//

bool install_win32_service(_In_z_ const wchar_t* bin_path,
						   _In_z_ const wchar_t* service_name,
						   _In_z_ const wchar_t* service_display_name,
						   _In_ bool auto_start);

bool install_fs_filter(_In_z_ const wchar_t* bin_path,
					   _In_z_ const wchar_t* service_name,
					   _In_z_ const wchar_t* service_display_name,
					   _In_z_ const wchar_t* altitude,
					   _In_ uint32_t fs_filter_flag);

bool uninstall_service(_In_ const wchar_t* service_name);
bool start_service(_In_ const wchar_t* service_name);
bool stop_service(_In_ const wchar_t* service_name, _In_ uint32_t wait_for_n_secs = 10);
bool service_installed(_In_ const wchar_t* service_name);
bool service_started(_In_ const wchar_t* service_name);
const char* service_status_to_str(_In_ uint32_t service_status);

HANDLE open_driver(_In_ const wchar_t* service_name);
bool io_control(_In_ HANDLE driver_handle,
				_In_ uint32_t io_code,
				_In_ uint32_t input_buffer_size,
				_In_bytecount_(input_buffer_size) void* input_buffer,
				_In_ uint32_t output_buffer_size,
				_In_bytecount_(output_buffer_size) void* output_buffer,
				_Out_ uint32_t* bytes_returned);
