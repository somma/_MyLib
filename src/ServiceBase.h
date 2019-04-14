/****************************** Module Header ******************************\
* Module Name:  ServiceBase.h
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* Provides a base class for a service that will exist as part of a service 
* application. CServiceBase must be derived from when creating a new service 
* class.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once

#include <windows.h>


class CServiceBase
{
public:

	// Register the executable for a service with the Service Control Manager 
	// (SCM). After you call Run(ServiceBase), the SCM issues a Start command, 
	// which results in a call to the OnStart method in the service. This 
	// method blocks until the service has stopped.
	static BOOL Run(CServiceBase &service);
	static BOOL RunEx(_In_ CServiceBase &service);

	// Service object constructor. The optional parameters (fCanStop, 
	// fCanShutdown and fCanPauseContinue) allow you to specify whether the 
	// service can be stopped, paused and continued, or be notified when 
	// system shutdown occurs.
	CServiceBase(PWSTR pszServiceName,
				 BOOL fCanStop = TRUE,
				 BOOL fCanShutdown = TRUE,
				 BOOL fCanPauseContinue = FALSE,
				 BOOL fCanSessionChange = FALSE);

	// Service object destructor. 
	virtual ~CServiceBase(void);

	// Stop the service.
	void Stop();

	// Get service name
	const wchar_t* ServiceName() { return m_name; }

protected:

	// When implemented in a derived class, executes when a Start command is 
	// sent to the service by the SCM or when the operating system starts 
	// (for a service that starts automatically). Specifies actions to take 
	// when the service starts.
	virtual bool OnStart(DWORD dwArgc, PWSTR *pszArgv);

	// When implemented in a derived class, executes when a Stop command is 
	// sent to the service by the SCM. Specifies actions to take when a 
	// service stops running.
	virtual void OnStop();

	// When implemented in a derived class, executes when a Pause command is 
	// sent to the service by the SCM. Specifies actions to take when a 
	// service pauses.
	virtual void OnPause();

	// When implemented in a derived class, OnContinue runs when a Continue 
	// command is sent to the service by the SCM. Specifies actions to take 
	// when a service resumes normal functioning after being paused.
	virtual void OnContinue();

	// When implemented in a derived class, executes when the system is 
	// shutting down. Specifies what should occur immediately prior to the 
	// system shutting down.
	virtual void OnShutdown();

	//	RegisterDeviceNotification() 콜백	 
	//
	//	PURPOSE: RegisterServiceCtrlHandlerEx( _handler_ex ) callback의 dwControlCode 값이 
	//	SERVICE_CONTROL_DEVICEEVENT 인 경우 호출되는 가상함수.
	//	
	//	가능한 dwEventType 은 아래와 같다. 
	//	https://msdn.microsoft.com/en-us/library/windows/desktop/ms683241(v=vs.85).aspx
	// 
	//		DBT_DEVICEARRIVAL
	//		DBT_DEVICEREMOVECOMPLETE
	//		DBT_DEVICEQUERYREMOVE
	//		DBT_DEVICEQUERYREMOVEFAILED
	//		DBT_DEVICEREMOVEPENDING
	//		DBT_CUSTOMEVENT
	/*
		void 
		MonsterGcsService::OnDeviceEvent(
			_In_ DWORD dwEventType,
			_In_ LPVOID lpEventData
			)
		{
		#ifdef _DEBUG
			//
			//	NOTE by somma (2017/3/29)
			// 
			//	아래 코드는 더이상 사용되지 않으나 코멘트만 남겨둠.
			// 
			//	CServiceBase::RunEx() 를 통해서 RegisterDeviceNotification 를 
			//	서비스에서 호출, device notification 을 서비스에서 받을 수 있다. 
			//	응용프로그램에서는 DBT_DEVTYP_VOLUME 을 받을 수 있으나 
			//	서비스에서는 DBT_DEVTYP_VOLUME 가 전달되지 않는다. 
			//	(최상위 윈도우에게만 윈도우 메세지를 전송하기 때문이라고)
			//	
			//	DBT_DEVICEARRIVAL 또는 DBT_DEVICEREMOVECOMPLETE 이 발생했을때
			//	드라이브 목록을 갱신하는 방법을 취할 수 있으나 해당 이벤트가 동일장치 
			//	연결 및 해제시 어러번 발생하기때문에 정확한 포인트를 잡기 어렵다. 
			//
			//	결국 Monster 서비스에서 device change 이벤트를 받아서 처리하는 방법을
			//	사용하지 않기로 하고, 사용자 로그인이 성공하면 해당 세션에 응용프로그램을
			//	띄우고, 응용프로그램에서 device change 이벤트를 받아서 처리하기로 한다. 
			// 
			_ASSERTE(nullptr != lpEventData);
			if (nullptr == lpEventData) return;
	
			PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lpEventData;

			if (dwEventType == DBT_DEVICEARRIVAL)	
			{
				log_info "device type=%u, size=%u, connected", 
					pHdr->dbch_devicetype, 
					pHdr->dbch_size
					log_end;
			}
			else if (dwEventType == DBT_DEVICEREMOVECOMPLETE)
			{
				log_info "device type=%u, size=%u, disconnected",
					pHdr->dbch_devicetype,
					pHdr->dbch_size
					log_end;
			}
			else
			{
				log_info "event_type=%u, device type=%u, size=%u, unknown",
					dwEventType,
					pHdr->dbch_devicetype,
					pHdr->dbch_size
					log_end;
			}
		#else 
			return;
		#endif//_DEBUG
		}
	*/
	virtual void OnDeviceEvent(DWORD dwEventType, LPVOID lpEventData)
	{
		UNREFERENCED_PARAMETER(dwEventType);
		UNREFERENCED_PARAMETER(lpEventData);
	}

	//	SERVICE_CONTROL_SESSIONCHANGE 콜백
	// 
	/*
		void 
		MonsterGcsService::OnSessionChange(
			DWORD dwEventType, 
			LPVOID lpEventData
			)
		{
		#ifdef _DEBUG
			//
			//	특별히 사용하고 있지 않으나 그냥 남겨둠
			//
			_ASSERTE(nullptr != lpEventData);
			if (nullptr == lpEventData) return;

			PWTSSESSION_NOTIFICATION session_noti = (PWTSSESSION_NOTIFICATION)lpEventData;
			std::string event_type;
			switch (dwEventType)
			{
			case WTS_CONSOLE_CONNECT: event_type = "WTS_CONSOLE_CONNECT"; break;
			case WTS_CONSOLE_DISCONNECT: event_type = "WTS_CONSOLE_DISCONNECT"; break;
			case WTS_REMOTE_CONNECT: event_type = "WTS_REMOTE_CONNECT"; break;
			case WTS_REMOTE_DISCONNECT: event_type = "WTS_REMOTE_DISCONNECT"; break;
			case WTS_SESSION_LOGON: 
			{
				//
				//	로그인
				// 
				event_type = "WTS_SESSION_LOGON"; break;		
			}
			case WTS_SESSION_LOGOFF: 
			{
				//
				//	로그아웃
				// 
				event_type = "WTS_SESSION_LOGOFF"; break;
			}

			case WTS_SESSION_LOCK: 
			{
				//
				//	화면 잠금
				// 
				event_type = "WTS_SESSION_LOCK"; break;
			}
			case WTS_SESSION_UNLOCK: event_type = "WTS_SESSION_UNLOCK"; break;
			case WTS_SESSION_REMOTE_CONTROL: event_type = "WTS_SESSION_REMOTE_CONTROL"; break;
			case WTS_SESSION_CREATE: event_type = "WTS_SESSION_CREATE"; break;
			case WTS_SESSION_TERMINATE: event_type = "WTS_SESSION_TERMINATE"; break;
			default: event_type = "unknown"; break;
			}

	
			log_info "SERVICE_CONTROL_SESSIONCHANGE, session id=%u, event=%s",
				session_noti->dwSessionId,
				event_type.c_str()
				log_end;	
		#else
			return;
		#endif//_DEBUG
		}
	*/
	virtual void OnSessionChange(DWORD dwEventType, LPVOID lpEventData)
	{
		UNREFERENCED_PARAMETER(dwEventType);
		UNREFERENCED_PARAMETER(lpEventData);
	}

	// Set the service status and report the status to the SCM.
    void SetServiceStatus(DWORD dwCurrentState, 
        DWORD dwWin32ExitCode = NO_ERROR, 
        DWORD dwWaitHint = 0);

    // Log a message to the Application event log.
    void WriteEventLogEntry(_In_ const wchar_t* pszMessage, _In_ WORD wType);

    // Log an error message to the Application event log.
    void WriteErrorLogEntry(_In_ const wchar_t* pszFunction, DWORD dwError = GetLastError());

private:

    // Entry point for the service. It registers the handler function for the 
    // service and starts the service.
    static void WINAPI ServiceMain(DWORD dwArgc, LPWSTR *pszArgv);

    // The function is called by the SCM whenever a control code is sent to 
    // the service.
    static void WINAPI ServiceCtrlHandler(DWORD dwCtrl);


	static void WINAPI ServiceMainEx(DWORD dwArgc, LPWSTR *pszArgv);
	static DWORD WINAPI ServiceCtrlHandlerEx(DWORD    dwControl,
											 DWORD    dwEventType,
											 LPVOID   lpEventData,
											 LPVOID   lpContext);	

    // Start the service.
    void Start(DWORD dwArgc, PWSTR *pszArgv);
    
    // Pause the service.
    void Pause();

    // Resume the service after being paused.
    void Continue();

    // Execute when the system is shutting down.
    void Shutdown();

    // The singleton service instance.
    static CServiceBase *s_service;
	
    // The name of the service
    PWSTR m_name;

    // The status of the service
    SERVICE_STATUS m_status;

    // The service status handle
    SERVICE_STATUS_HANDLE m_statusHandle;
};




