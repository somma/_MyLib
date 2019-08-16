/**
 * @file    service control manager helper class
 * @brief
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011/11/30 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include "scm_context.h"
#include "log.h"
#include <memory>				// std::shared_ptr

#include "RegistryUtil.h"




/// @brief	win32 서비스를 설치한다. 
bool
install_win32_service(
	_In_z_ const wchar_t* bin_path,
	_In_z_ const wchar_t* service_name,
	_In_z_ const wchar_t* service_display_name,
	_In_ bool auto_start
)
{
	_ASSERTE(nullptr != bin_path);
	_ASSERTE(nullptr != service_name);
	_ASSERTE(nullptr != service_display_name);
	if (nullptr == bin_path ||
		nullptr == service_name ||
		nullptr == service_display_name)
	{
		return false;
	}

	if (true != is_file_existsW(bin_path))
	{
		log_err "No file exists. path=%ws", bin_path log_end;
		return false;
	}

	if (wcslen(service_name) > 256)
	{
		log_err "service name is too long. max=256" log_end;
		return false;
	}

	if (wcslen(service_display_name) > 256)
	{
		log_err "service display name is too long. max=256" log_end;
		return false;
	}

	//
	//	이미 설치된 서비스가 있다면 그냥 리턴한다.
	//
	if (true == service_installed(service_name))
	{
		log_dbg
			"%ws service already installed.",
			service_name
			log_end;
		return true;
	}

	//
	//	Open service manager for create new service
	//
	schandle_ptr scm_handle(OpenSCManagerW(NULL,
										   NULL,
										   SC_MANAGER_CREATE_SERVICE),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});

	if (!scm_handle)
	{
		log_err
			"OpenSCManagerW() faield. gle = %u",
			GetLastError()
			log_end;
		return false;
	}

	//
	//	Install service 
	// 
	schandle_ptr svc_handle(CreateServiceW(scm_handle.get(),
										   service_name,
										   service_display_name,
										   GENERIC_READ,
										   SERVICE_WIN32_OWN_PROCESS,
										   true == auto_start ? SERVICE_AUTO_START : SERVICE_DEMAND_START,
										   SERVICE_ERROR_NORMAL,
										   bin_path,
										   nullptr,
										   nullptr,
										   nullptr,
										   nullptr,
										   nullptr),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});

	if (!svc_handle)
	{
		log_err
			"CreateServcieW() failed. path=%ws, svc_name=%ws, gle = %u",
			bin_path,
			service_name,
			GetLastError()
			log_end;
		return false;
	}
	else
	{
		log_dbg "Service installed. service=%ws",
			service_name
			log_end;
		return true;
	}
}

/// @brief	Filesystem Filter (서비스)를 설치한다. 
bool
install_fs_filter(
	_In_z_ const wchar_t* bin_path,
	_In_z_ const wchar_t* service_name,
	_In_z_ const wchar_t* service_display_name, 
	_In_z_ const wchar_t* altitude,
	_In_ uint32_t fs_filter_flag
)
{
	_ASSERTE(nullptr != bin_path);
	_ASSERTE(nullptr != service_name);
	_ASSERTE(nullptr != service_display_name);
	if (nullptr == bin_path ||
		nullptr == service_name ||
		nullptr == service_display_name)
	{
		return false;
	}

	if (true != is_file_existsW(bin_path))
	{
		log_err "No file exists. path=%ws", bin_path log_end;
		return false;
	}

	if (wcslen(service_name) > 256)
	{
		log_err "service name is too long. max=256" log_end;
		return false;
	}

	if (wcslen(service_display_name) > 256)
	{
		log_err "service display name is too long. max=256" log_end;
		return false;
	}

	//
	//	이미 설치된 서비스가 있다면 그냥 리턴한다.
	//
	if (true == service_installed(service_name))
	{
		log_dbg
			"%ws service already installed.",
			service_name
			log_end;
		return true;
	}

	//
	//	Open service manager for create new service
	//
	schandle_ptr scm_handle(OpenSCManagerW(NULL,
										   NULL,
										   SC_MANAGER_CREATE_SERVICE),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});

	if (!scm_handle)
	{
		log_err
			"OpenSCManagerW() faield. gle = %u",
			GetLastError()
			log_end;
		return false;
	}

	//
	//	Install Kernel driver service 
	//
	std::wstringstream sys_path;

#ifdef _DEBUG
	sys_path << bin_path;
#else
	//
	//	Release 버전에서는 `SERVICE_BOOT_START` 타입으로 설치한다.
	// 
	//	StartType 을 `SERVICE_BOOT_START` 으로 지정하려면 image file 의 
	//	경로가 반드시 system32 에 있어야하므로 드라이버 파일을 복사하고
	//	복사된 경로의 드라이버를 서비스로 설치한다.
	// 
	std::wstring windows_dir;
	if (!get_windows_dir(windows_dir))
	{
		log_err "get_windows_dir() failed. " log_end;
		return false;
	}
		
	sys_path
		<< windows_dir
		<< L"\\system32\\"
		<< file_name_from_file_pathw(bin_path);
	if (!CopyFile(bin_path, sys_path.str().c_str(), FALSE))
	{
		log_err
			"Can not copy driver file to system directory. gle=%u",
			GetLastError()
			log_end;
		return false;
	}
#endif//_DEBUG
	schandle_ptr svc_handle(CreateServiceW(scm_handle.get(),
										   service_name,
										   service_display_name,
										   GENERIC_READ,
										   SERVICE_KERNEL_DRIVER,
										   SERVICE_BOOT_START, 
										   SERVICE_ERROR_NORMAL,
										   sys_path.str().c_str(),
										   L"FSFilter Activity Monitor",
										   nullptr,
										   L"FltMgr",
										   nullptr,
										   nullptr),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});

	if (!svc_handle)
	{
		log_err
			"CreateServcieW() failed. path=%ws, svc_name=%ws, gle = %u",
			sys_path.str().c_str(),
			service_name,
			GetLastError()
			log_end;
		return false;
	}

	//
	//	미니필터용 추가 정보 설정 #1
	//
	//	key  : HKLM\SYSTEM\CurrentControlSet\Services\[xxx]\Instances
	//	value: "DefaultInstance" = "AltitudeAndFlags"
	//
	bool ret = false;
	HKEY key_handle = NULL;
	std::wstringstream key_path;
	do
	{
#define	MF_DEFAULT_INSTANCE	L"DefaultInstance"
#define	MF_ALTITUDE_N_FLAGS	L"AltitudeAndFlags"			
		key_path
			<< L"SYSTEM\\CurrentControlSet\\Services\\"
			<< service_name
			<< L"\\Instances";

		//
		//	Instances 키가 없으므로 생성한다 (만일 있다면 open 한다).
		// 
		key_handle = RUCreateKey(HKEY_LOCAL_MACHINE,
								 key_path.str().c_str(),
								 false);
		if (NULL == key_handle)
		{
			log_err "RUCreateKey() failed. key=%ws",
				key_path.str().c_str()
				log_end;
			break;
		}

		if (!RUSetString(key_handle,
						 MF_DEFAULT_INSTANCE,
						 MF_ALTITUDE_N_FLAGS))
		{
			log_err "RUSetString(HKLM, %ws, %ws) failed.",
				key_path.str().c_str(),
				MF_DEFAULT_INSTANCE
				log_end;
			break;
		}

		//
		//	ok
		//
		ret = true;

	} while (false);

	RUCloseKey(key_handle);
	key_handle = NULL;

	if (!ret)
	{
		return false;
	}

	//	미니필터용 추가 정보 설정 #2
	// 
	//	key  : HKLM\SYSTEM\CurrentControlSet\Services\scanner\Instances\AltitudeAndFlags
	//	value:	"Altitude" = "0"
	//			"Flags" = dword:00000000
	//
	ret = false;
	do
	{
		_ASSERTE(key_handle == NULL);

#define	MF_ALTITUDE		L"Altitude"
#define	MF_FLAGS		L"Flags"
		clear_str_stream_w(key_path);
		key_path
			<< L"SYSTEM\\CurrentControlSet\\Services\\"
			<< service_name
			<< L"\\Instances\\AltitudeAndFlags";

		key_handle = RUCreateKey(HKEY_LOCAL_MACHINE,
								 key_path.str().c_str(),
								 false);
		if (nullptr == key_handle)
		{
			log_err "RUCreateKey() failed. key=%ws",
				key_path.str().c_str()
				log_end;
			break;
		}

		if (!RUSetString(key_handle, MF_ALTITUDE, altitude))
		{
			log_err "RUSetString(HKLM, %ws, %ws) failed.",
				key_path.str().c_str(),
				MF_ALTITUDE
				log_end;
			break;
		}

		if (!RUWriteDword(key_handle, MF_FLAGS, fs_filter_flag))
		{
			log_err "RUWriteDword(HKLM, %ws, %ws) failed.",
				key_path.str().c_str(),
				MF_FLAGS
				log_end;
			break;
		}

		//
		//	OK
		// 
		ret = true;

	} while (false);

	RUCloseKey(key_handle);
	key_handle = NULL;

	return ret;
}

/// @brief	서비스가 실행중이라면 중지하고, 서비스를 삭제한다. 
///			성공시 true 를 리턴한다.
bool
uninstall_service(
	_In_ const wchar_t* service_name
)
{
	_ASSERTE(nullptr != service_name);
	if (nullptr == service_name)
	{
		return false;
	}

	if (wcslen(service_name) > 256)
	{
		log_err "service name is too long. max=256" log_end;
		return false;
	}

	//
	//	설치된 서비스가 없다면 그냥 리턴한다.
	//
	if (true != service_installed(service_name))
	{
		log_dbg
			"No service installed. service=%ws",
			service_name
			log_end;
		return true;
	}

	//
	//	서비스가 실행중이라면 중지한다.
	//
	if (true != stop_service(service_name))
	{
		log_warn
			"stop_service() failed, but mark it for deletion."
			log_end

			//> stop_service() 가 실패해도 삭제한다.
			//    - driver :: DriverEntry() 에서 STATUS_SUCCESS 를 리턴했으나 
			//		아무짓도 안하고, 리턴한 경우
			//    - driver handle 을 누군가 물고 있는 경우
			//  
			//	강제로 서비스를 삭제 (registry 에서 서비스 제거)하고, 리부팅하면 
			//	서비스가 제거된 상태로 (정상) 돌아올 수 있다. 
			//
			//return false;
	}

	//
	//	Open service manager for delete service
	//
	schandle_ptr scm_handle(OpenSCManagerW(NULL,
										   NULL,
										   SC_MANAGER_ALL_ACCESS),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});

	if (!scm_handle)
	{
		log_err
			"OpenSCManagerW() faield. gle = %u",
			GetLastError()
			log_end;
		return false;
	}

	//
	//	서비스 핸들을 오픈한다. 
	// 
	schandle_ptr svc_handle(OpenServiceW(scm_handle.get(),
										 service_name,
										 SERVICE_ALL_ACCESS),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});
	if (!svc_handle)
	{
		log_err
			"OpenServiceW() failed. service_name=%ws, gle = %u",
			service_name,
			GetLastError()
			log_end
			return false;
	}

	//
	//	서비스를 제거한다. 
	// 
	if (FALSE == DeleteService(svc_handle.get()))
	{
		DWORD err = GetLastError();
		if (ERROR_SERVICE_MARKED_FOR_DELETE == err)
		{
			log_warn
				"Service marked for deletion(need reboot). service=%ws",
				service_name
				log_end;
			return true;
		}
		else
		{
			log_err
				"DeleteService() failed, service name=%ws, gle = %u",
				service_name,
				err
				log_end;
			return false;
		}
	}

	log_dbg
		"Service deleted successfully. service=%ws",
		service_name
		log_end;
	return true;
}

/// @brief	이미 설치된 서비스를 시작한다. 
///			설치된 서비스가 없거나 시작에 실패하면 false 를 리턴한다.
bool
start_service(
	_In_ const wchar_t* service_name
)
{
	_ASSERTE(nullptr != service_name);
	if (nullptr == service_name)
	{
		return false;
	}

	if (wcslen(service_name) > 256)
	{
		log_err "service name is too long. max=256" log_end;
		return false;
	}

	//
	//	설치된 서비스가 없다면 실패를 리턴한다.
	//
	if (true != service_installed(service_name))
	{
		log_err
			"No service installed. service=%ws",
			service_name
			log_end;
		return false;
	}

	//
	//	이미 실행중이라면 성공을 리턴
	//
	if (true == service_started(service_name))
	{
		log_dbg
			"Service already running. service=%ws",
			service_name
			log_end;
		return true;
	}

	//
	//	Open service manager for delete service
	//
	schandle_ptr scm_handle(OpenSCManagerW(NULL,
										   NULL,
										   SC_MANAGER_ALL_ACCESS),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});

	if (!scm_handle)
	{
		log_err
			"OpenSCManagerW() faield. gle = %u",
			GetLastError()
			log_end;
		return false;
	}

	//
	//	서비스 핸들을 오픈한다. 
	// 
	schandle_ptr svc_handle(OpenServiceW(scm_handle.get(),
										 service_name,
										 SERVICE_ALL_ACCESS),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});
	if (!svc_handle)
	{
		log_err
			"OpenServiceW() failed. service_name=%ws, gle = %u",
			service_name,
			GetLastError()
			log_end
			return false;
	}

	//
	//	서비스 시작 명령을 내리고, 서비스가 시작될 때까지 기다린다. 
	//
	if (!StartService(svc_handle.get(), 0, nullptr))
	{
		DWORD err = GetLastError();
		if (err != ERROR_SERVICE_ALREADY_RUNNING)
		{
			log_err
				"start_service() failed. service name=%ws, gle=%u",
				service_name,
				GetLastError()
				log_end;
			return false;
		}
	}

	SERVICE_STATUS svc_status = { 0 };
	while (QueryServiceStatus(svc_handle.get(), &svc_status))
	{
		if (svc_status.dwCurrentState == SERVICE_START_PENDING)
		{
			Sleep(1000);
		}
		else
		{
			break;
		}
	}

	if (svc_status.dwCurrentState != SERVICE_RUNNING)
	{
		log_err "service start failed. status=%u",
			svc_status.dwCurrentState
			log_end;
		return false;
	}

	log_dbg
		"Service started. service=%ws",
		service_name
		log_end;
	return true;
}


/// @brief	서비스를 중지한다. service_handle 은 SERVICE_STOP 권한이 
///			필요하다. 
bool
stop_service(
	_In_ const wchar_t* service_name,
	_In_ uint32_t wait_for_n_secs
)
{
	_ASSERTE(nullptr != service_name);
	if (nullptr == service_name)
	{
		return false;
	}

	if (wcslen(service_name) > 256)
	{
		log_err "service name is too long. max=256" log_end;
		return false;
	}

	//
	//	설치된 서비스가 없다면 실패를 리턴한다.
	//
	if (true != service_installed(service_name))
	{
		log_err
			"No service installed. service=%ws",
			service_name
			log_end;
		return false;
	}

	//
	//	이미 중지된 경우 성공을 리턴
	//
	if (true != service_started(service_name))
	{
		log_dbg
			"Service is not running. service=%ws",
			service_name
			log_end;
		return true;
	}

	//
	//	Open service manager for delete service
	//
	schandle_ptr scm_handle(OpenSCManagerW(NULL,
										   NULL,
										   SC_MANAGER_ALL_ACCESS),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});

	if (!scm_handle)
	{
		log_err
			"OpenSCManagerW() faield. gle = %u",
			GetLastError()
			log_end;
		return false;
	}

	//
	//	서비스 핸들을 오픈한다. 
	// 
	schandle_ptr svc_handle(OpenServiceW(scm_handle.get(),
										 service_name,
										 SERVICE_ALL_ACCESS),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});
	if (!svc_handle)
	{
		log_err
			"OpenServiceW() failed. service_name=%ws, gle = %u",
			service_name,
			GetLastError()
			log_end
			return false;
	}

	//
	//	2007.05.17 by somma
	//	다른 프로세스가 SCM 을 통해서 SERVICE_CONTROL_STOP 을 이미요청한 경우
	//	여기서 호출한 ControlService() 함수는 FALSE 를 리턴한다.
	//	그러나 서비스는 정상 종료된다.
	//

	SERVICE_STATUS service_status = { 0 };
	if (FALSE == ControlService(svc_handle.get(),
								SERVICE_CONTROL_STOP,
								&service_status))
	{
		log_err
			"ControlService() failed, service name=%ws, gle = %u",
			service_name,
			GetLastError()
			log_end;
		return false;
	}

	//
	//	서비스가 종료될 때까지 기다린다. 
	// 
	SERVICE_STATUS svc_status = { 0 };
	uint32_t wait_for_secs = 0;
	while (QueryServiceStatus(svc_handle.get(), &svc_status))
	{
		if (svc_status.dwCurrentState == SERVICE_STOP_PENDING)
		{
			if (wait_for_secs < wait_for_n_secs)
			{
				Sleep(1000);
				wait_for_secs++;
			}
		}
		else
		{
			break;
		}
	}

	if (svc_status.dwCurrentState == SERVICE_STOPPED)
	{
		log_dbg "Service has stopped. service=%ws",
			service_name
			log_end;
		return true;
	}
	else
	{
		log_dbg "Service stop failed. service=%ws, status=%s",
			service_name,
			service_status_to_str(svc_status.dwCurrentState)
			log_end;

		return false;
	}
}

/// @brief	
bool
service_installed(
	_In_ const wchar_t* service_name
)
{
	_ASSERTE(nullptr != service_name);
	if (nullptr == service_name)
	{
		return false;
	}

	if (wcslen(service_name) > 256)
	{
		log_err "service name is too long. max=256" log_end;
		return false;
	}

	//
	//	Open service manager for delete service
	//
	schandle_ptr scm_handle(OpenSCManagerW(NULL,
										   NULL,
										   SC_MANAGER_ALL_ACCESS),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});

	if (!scm_handle)
	{
		log_err
			"OpenSCManagerW() faield. gle = %u",
			GetLastError()
			log_end;
		return false;
	}

	//
	//	서비스 핸들을 오픈한다. 
	// 
	schandle_ptr svc_handle(OpenServiceW(scm_handle.get(),
										 service_name,
										 SERVICE_QUERY_STATUS),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});
	if (!svc_handle)
	{
		DWORD gle = GetLastError();
		if (ERROR_SERVICE_DOES_NOT_EXIST == gle)
		{
			return false;
		}
		else
		{
			log_err
				"OpenServiceW() failed. service_name=%ws, gle = %u",
				service_name,
				GetLastError()
				log_end;
			return false;
		}
	}
	else
	{
		return true;
	}
}

/// @brief	
bool service_started(_In_ const wchar_t* service_name)
{
	_ASSERTE(nullptr != service_name);
	if (nullptr == service_name)
	{
		return false;
	}

	if (wcslen(service_name) > 256)
	{
		log_err "service name is too long. max=256" log_end;
		return false;
	}

	//
	//	Open service manager for delete service
	//
	schandle_ptr scm_handle(OpenSCManagerW(NULL,
										   NULL,
										   SC_MANAGER_ALL_ACCESS),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});

	if (!scm_handle)
	{
		log_err
			"OpenSCManagerW() faield. gle = %u",
			GetLastError()
			log_end;
		return false;
	}

	//
	//	서비스 핸들을 오픈한다. 
	// 
	schandle_ptr svc_handle(OpenServiceW(scm_handle.get(),
										 service_name,
										 SERVICE_QUERY_STATUS),
							[](SC_HANDLE handle) {
		if (nullptr != handle)
		{
			CloseServiceHandle(handle);
		}
	});
	if (!svc_handle)
	{
		log_err
			"OpenServiceW() failed. service_name=%ws, gle = %u",
			service_name,
			GetLastError()
			log_end;
		return false;
	}

	//
	//	서비스 상태를 조회한다. 
	// 
	SERVICE_STATUS svc_status = { 0 };
	if (!QueryServiceStatus(svc_handle.get(), &svc_status))
	{
		log_err "QueryServiceStatus() failed. service=%ws, gle=%u",
			service_name,
			GetLastError()
			log_end;
		return false;
	}

	if (svc_status.dwCurrentState == SERVICE_RUNNING)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/// @brief	
HANDLE open_driver(_In_ const wchar_t* service_name)
{
	std::wstringstream driver_object_name;
	driver_object_name << L"\\\\.\\" << service_name;
	HANDLE driver_handle = CreateFileW(driver_object_name.str().c_str(),
									   GENERIC_READ | GENERIC_WRITE,
									   0, // exclusive open
									   nullptr,
									   OPEN_EXISTING,
									   FILE_ATTRIBUTE_NORMAL,
									   nullptr);
	if (INVALID_HANDLE_VALUE == driver_handle)
	{
		log_err
			"CreateFileW() failed, driver name=%ws, gle = %u",
			driver_object_name.str().c_str(),
			GetLastError()
			log_end;
		return INVALID_HANDLE_VALUE;
	}

	return driver_handle;
}

/// @brief	
bool
io_control(
	_In_ HANDLE driver_handle,
	_In_ uint32_t io_code,
	_In_ uint32_t input_buffer_size,
	_In_bytecount_(input_buffer_size) void* input_buffer,
	_In_ uint32_t output_buffer_size,
	_In_bytecount_(output_buffer_size) void* output_buffer,
	_Out_ uint32_t* bytes_returned
)
{
	if (nullptr == driver_handle || INVALID_HANDLE_VALUE == driver_handle)
	{
		_ASSERTE(!"Oops, Invalid handle");
		return false;
	}

	BOOL ret = DeviceIoControl(driver_handle,
							   io_code,
							   input_buffer,
							   input_buffer_size,
							   output_buffer,
							   output_buffer_size,
							   (LPDWORD)bytes_returned,
							   nullptr);
	if (TRUE != ret)
	{
		log_err
			"DeviceIoControl() failed, io_code=0x%08x, gle=%u",
			io_code,
			GetLastError()
			log_end;
		return false;
	}

	return true;
}

/// @brief 
const char*
service_status_to_str(
	_In_ uint32_t service_status
)
{
	switch (service_status)
	{
	case SERVICE_STOPPED: return "SERVICE_STOPPED";
	case SERVICE_START_PENDING: return "SERVICE_START_PENDING";
	case SERVICE_STOP_PENDING: return "SERVICE_STOP_PENDING";
	case SERVICE_RUNNING: return "SERVICE_RUNNING";
	case SERVICE_CONTINUE_PENDING: return "SERVICE_CONTINUE_PENDING";
	case SERVICE_PAUSE_PENDING: return "SERVICE_PAUSE_PENDING";
	case SERVICE_PAUSED: return "SERVICE_PAUSED";
	default:
		return "UNKNOWN";
	}
}

/// @brief	서비스가 실행중이라면 true 를 리턴하고, 조회중 오류가 
///			발생하거나 실행중이지 않은 경우 false 를 리턴한다.
///			service_handle 은 SERVICE_QUERY_STATUS 권한이 있어야 한다.
bool service_started(_In_ SC_HANDLE service_handle)
{
	_ASSERTE(nullptr != service_handle);
	if (nullptr == service_handle) return false;

	SERVICE_STATUS svc_status = { 0 };
	if (!QueryServiceStatus(service_handle, &svc_status))
	{
		log_err "QueryServiceStatus() failed. gle=%u",
			GetLastError()
			log_end;
		return false;
	}

	if (svc_status.dwCurrentState == SERVICE_RUNNING)
	{
		return true;
	}
	else
	{
		return false;
	}
}

