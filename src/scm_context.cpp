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
#include <memory>				// std::shared_ptr
#include "RegistryUtil.h"

//> todo - SCM 접근시 권한은 필요한 만큼만 정해서 호출하게 하자. 

struct sc_handle_deleter
{
	void operator()(SC_HANDLE* phandle) const
	{
		CloseServiceHandle(*phandle);
		delete phandle;
	}
};

typedef std::shared_ptr<SC_HANDLE> sc_handle_ptr;

/// @brief	service context manager for legacy dirver or win32 service.
scm_context::scm_context(
	_In_z_ const wchar_t* bin_path, 
	_In_z_ const wchar_t* service_name, 
	_In_z_ const wchar_t* service_display_name,
	_In_ bool win32_service,
	_In_ bool uninstall_service_on_free)
:	_uninstall_service_on_free(uninstall_service_on_free),
	_driver_handle(INVALID_HANDLE_VALUE), 
	_bin_path(bin_path),
	_service_name(service_name),
	_service_display_name(service_display_name),
	_service_type((true == win32_service ? SERVICE_WIN32_OWN_PROCESS : SERVICE_KERNEL_DRIVER)),
	_is_minifilter(false),
	_altitude(L"none"),
	_flags(0)
{
	
}

/// @brief	service context manager for minifilter driver
scm_context::scm_context(
	_In_z_ const wchar_t* bin_path,
	_In_z_ const wchar_t* service_name,
	_In_z_ const wchar_t* service_display_name,
	_In_z_ const wchar_t* altitude,
	_In_ uint32_t flags,
	_In_ bool uninstall_service_on_free)
:	_uninstall_service_on_free(uninstall_service_on_free),
	_driver_handle(INVALID_HANDLE_VALUE),
	_bin_path(bin_path),
	_service_name(service_name),
	_service_display_name(service_display_name),
	_service_type(SERVICE_KERNEL_DRIVER),
	_is_minifilter(true),
	_altitude(altitude),
	_flags(flags)
{
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
scm_context::~scm_context()
{
	if(INVALID_HANDLE_VALUE != _driver_handle) 
	{
		CloseHandle(_driver_handle); _driver_handle = INVALID_HANDLE_VALUE;
	}
		
	if (true == _uninstall_service_on_free)
	{
		stop_service();
		uninstall_service();
	}
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
bool scm_context::install_service()
{
	SC_HANDLE scm_handle = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == scm_handle)
	{
		log_err "OpenSCManagerW() faield. gle = %u", GetLastError() log_end
		return false;
	}
	sc_handle_ptr scm_handle_ptr(new SC_HANDLE(scm_handle), sc_handle_deleter());
	SC_HANDLE service_handle = OpenServiceW(scm_handle, 
											_service_name.c_str(), 
											SERVICE_QUERY_CONFIG);
	if (NULL != service_handle)
	{
		log_dbg "service=%ws. already exists", _service_name.c_str() log_end
		CloseServiceHandle(service_handle);
		return true;
	}


	//
	//	Install service 
	// 

	service_handle = CreateServiceW(scm_handle,
									_service_name.c_str(),
									_service_display_name.c_str(),
									GENERIC_READ, // SERVICE_ALL_ACCESS,
									_service_type,
									SERVICE_DEMAND_START,
									SERVICE_ERROR_NORMAL,
									_bin_path.c_str(),
									NULL,
									NULL,
									NULL,
									NULL,
									NULL);
	if (service_handle == NULL)
	{
		log_err
			"CreateServcieW(path=%ws, svc_name=%ws, svc_display=%ws) failed. gle = %u",
			_bin_path.c_str(), _service_name.c_str(), _service_display_name.c_str(), GetLastError()
			log_end;
		return false;
	}
	CloseServiceHandle(service_handle);
	
	if (true != _is_minifilter)
	{
		return true;
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
			<< _service_name
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
			<< _service_name
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

		if (!RUSetString(key_handle,
						 MF_ALTITUDE,
						 _altitude.c_str()))
		{
			log_err "RUSetString(HKLM, %ws, %ws) failed.",
				key_path.str().c_str(),
				MF_ALTITUDE
				log_end;
			break;
		}

		if (!RUWriteDword(key_handle,
						  MF_FLAGS,
						  _flags))
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

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
bool scm_context::uninstall_service()
{
	if (true != stop_service())
	{
		log_err 
			"scm_context::stop_service() failed, can not uninstall driver..." 
			log_end

			//> stop_service() 가 실패해도 삭제시도를 해야 한다. 
			//    - driver :: DriverEntry() 에서 STATUS_SUCCESS 를 리턴했으나 아무짓도 안하고, 리턴한 경우
			//    - driver handle 을 누군가 물고 있는 경우
			//  강제로 서비스를 삭제 (registry 에서 서비스 제거)하고, 리부팅하면 서비스가 
			//  제거된 상태로 (정상) 돌아올 수 있다. 
			//
			//return false;
	}

	SC_HANDLE scm_handle = OpenSCManagerW(NULL, 
										  NULL, 
										  SC_MANAGER_ALL_ACCESS);
	if (NULL == scm_handle)
	{
		log_err "OpenSCManagerW() faield. gle = %u", 
			GetLastError() 
			log_end
		return false;
	}
	sc_handle_ptr scm_handle_ptr(new SC_HANDLE(scm_handle), sc_handle_deleter());

	SC_HANDLE service_handle = OpenServiceW(scm_handle,
											_service_name.c_str(),
											SERVICE_ALL_ACCESS);
	if (NULL == service_handle)
	{
		log_err 
			"OpenServiceW( service_name=%ws ) failed. gle = %u", 
			_service_name.c_str(), GetLastError() 
		log_end
		return false;
	}
	sc_handle_ptr service_handle_ptr(new SC_HANDLE(service_handle), sc_handle_deleter());

	if (FALSE == DeleteService(service_handle))
	{
		DWORD err = GetLastError();
		if (ERROR_SERVICE_MARKED_FOR_DELETE != err)
		{
			log_err 
				"DeleteService( service name=%ws ) failed, gle = %u", 
				_service_name.c_str(), err
			log_end
			return false;
		}
	}

	log_info "service=%ws deleted successfully", _service_name.c_str() log_end
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
*/
bool scm_context::start_service()
{
	SC_HANDLE scm_handle = OpenSCManagerW(NULL, 
										  NULL, 
										  SC_MANAGER_ALL_ACCESS);
	if (NULL == scm_handle)
	{
		log_err "OpenSCManagerW() faield. gle = %u", 
			GetLastError() 
			log_end
		return false;
	}
	sc_handle_ptr scm_handle_ptr(new SC_HANDLE(scm_handle), sc_handle_deleter());

	SC_HANDLE service_handle = OpenServiceW(
									scm_handle,
									_service_name.c_str(), 
									SERVICE_ALL_ACCESS
									);
	if (NULL == service_handle)
	{
		log_err 
			"OpenServiceW( service_name=%ws ) failed. gle = %u", 
			_service_name.c_str(), GetLastError() 
		log_end
		return false;
	}
	sc_handle_ptr service_handle_ptr(new SC_HANDLE(service_handle), sc_handle_deleter());

	if (TRUE != StartService(service_handle, 0, NULL))
	{
		DWORD err = GetLastError();
		if (err != ERROR_SERVICE_ALREADY_RUNNING)
		{
            log_err 
				"StartService( service name=%ws ) failed, gle = %u", 
				_service_name.c_str(), err
			log_end
			return false;
		}
	}

	//
	//	서비스가 시작될 때까지 기다린다. 
	// 
	SERVICE_STATUS svc_status = { 0 };
	while (QueryServiceStatus(service_handle, &svc_status))
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

	if (svc_status.dwCurrentState == SERVICE_RUNNING)
	{
		log_info "service is started. svc=%ws",
			_service_name.c_str()
			log_end;

		//_running = true;
		return true;
	}
	else
	{
		log_info "service start failed. svc=%ws",
			_service_name.c_str()
			log_end;

		return false;
	}
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
bool scm_context::stop_service()
{
	if (true != started()) return true;

	SC_HANDLE scm_handle = OpenSCManagerW(NULL, 
										  NULL, 
										  SC_MANAGER_ALL_ACCESS);
	if (NULL == scm_handle)
	{
		log_err "OpenSCManagerW() faield. gle = %u", 
			GetLastError() 
			log_end
		return false;
	}
	sc_handle_ptr scm_handle_ptr(new SC_HANDLE(scm_handle), sc_handle_deleter());

	SC_HANDLE service_handle = OpenServiceW(scm_handle,
											_service_name.c_str(),
											SERVICE_ALL_ACCESS);
	if (NULL == service_handle)
	{
		log_err 
			"OpenServiceW( service_name=%ws ) failed. gle = %u", 
			_service_name.c_str(), GetLastError() 
		log_end
		return false;
	}
	sc_handle_ptr service_handle_ptr(new SC_HANDLE(service_handle), sc_handle_deleter());

	// 2007.05.17 by somma
	// 다른 프로세스가 SCM 을 통해서 SERVICE_CONTROL_STOP 을 이미요청한 경우
	// 여기서 호출한 ControlService() 함수는 FALSE 를 리턴한다.
	// 그러나 서비스는 정상 종료된다.
	//
	SERVICE_STATUS service_status = { 0 };
	if (FALSE == ControlService(service_handle, SERVICE_CONTROL_STOP, &service_status))
	{
		log_err
			"ControlService( service name=%ws ) failed, gle = %u",
			_service_name.c_str(), GetLastError()
			log_end
			return false;
	}

	//
	//	서비스가 종료될 때까지 기다린다. 
	// 
	SERVICE_STATUS svc_status = { 0 };
	while (QueryServiceStatus(service_handle, &svc_status))
	{
		if (svc_status.dwCurrentState == SERVICE_STOP_PENDING)
		{
			Sleep(1000);
		}
		else
		{
			break;
		}
	}

	if (svc_status.dwCurrentState == SERVICE_STOPPED)
	{
		log_info "service is stopped. svc=%ws",
			_service_name.c_str()
			log_end;
	}
	else
	{
		log_info "service stop failed. svc=%ws",
			_service_name.c_str()
			log_end;

		//
		//	서비스 종료 상태는 아니지만 어찌되었든 실행 상태는 아니므로
		//	에러처리않고, 진행한다. 
		// 
	}

	//_running = false;
	return true;
}

/// @brief	
bool scm_context::started()
{
	SC_HANDLE scm_handle = OpenSCManagerW(NULL,
										  NULL,
										  SC_MANAGER_ALL_ACCESS);
	if (NULL == scm_handle)
	{
		log_err "OpenSCManagerW() faield. gle = %u",
			GetLastError()
			log_end;
		return false;
	}
	sc_handle_ptr scm_handle_ptr(new SC_HANDLE(scm_handle), sc_handle_deleter());

	SC_HANDLE service_handle = OpenServiceW(scm_handle,
											_service_name.c_str(),
											SERVICE_ALL_ACCESS);
	if (NULL == service_handle)
	{
		log_err
			"OpenServiceW( service_name=%ws ) failed. gle = %u",
			_service_name.c_str(), GetLastError()
			log_end
			return false;
	}
	sc_handle_ptr service_handle_ptr(new SC_HANDLE(service_handle), sc_handle_deleter());

	//
	//	서비스가 상태를 조회한다. 
	// 
	SERVICE_STATUS svc_status = { 0 };
	if(!QueryServiceStatus(service_handle, &svc_status))
	{
		log_err "QueryServiceStatus() failed. service=%ws",
			_service_name.c_str()
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


/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
bool 
scm_context::send_command(
		_In_ uint32_t io_code, 
		_In_ uint32_t input_buffer_size,
		_In_bytecount_(input_buffer_size) void* input_buffer,
		_In_ uint32_t output_buffer_size,
		_In_bytecount_(output_buffer_size) void* output_buffer,
		_Out_ uint32_t* bytes_returned
		)
{
	if (INVALID_HANDLE_VALUE == _driver_handle)
	{
		_driver_handle = open_driver();
		if (INVALID_HANDLE_VALUE == _driver_handle)
		{
			log_err "scm_context::open_driver() failed" log_end
			return false;
		}
	}

	BOOL ret = DeviceIoControl(_driver_handle,
							   io_code,
							   input_buffer,
							   input_buffer_size,
							   output_buffer,
							   output_buffer_size,
							   reinterpret_cast<LPDWORD>(bytes_returned),
							   NULL);
	if(TRUE != ret)
	{
		log_err "DeviceIoControl( io_code=0x%08x ) failed", io_code log_end
		return false;
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
*/
HANDLE scm_context::open_driver()
{
	_ASSERTE(INVALID_HANDLE_VALUE == _driver_handle);	
	if (INVALID_HANDLE_VALUE != _driver_handle) return _driver_handle;

	std::wstring driver_object_name= L"\\\\.\\" + _service_name;
	HANDLE driver_handle = CreateFileW(driver_object_name.c_str(),
									   GENERIC_READ | GENERIC_WRITE,
									   0, // exclusive open
									   NULL,
									   OPEN_EXISTING,
									   FILE_ATTRIBUTE_NORMAL,
									   0);
	if (INVALID_HANDLE_VALUE == driver_handle)
	{
		log_err
			"CreateFileW(driver name=%ws) failed, gle = %u", 
			driver_object_name.c_str(), GetLastError()
		log_end
		return INVALID_HANDLE_VALUE;
	}

	return driver_handle;
}
