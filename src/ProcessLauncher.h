/**
 * @file    ProcessLauncher.h
 * @brief   Cross-session 프로세스 생성 모듈
 *          다양한 권한 레벨(User/Admin/System)과 환경 설정을 지원하며,
 *          UAC 환경과 PAM 환경 모두에서 안정적으로 동작함
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2024-11-29 created
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#pragma once

#include "BaseWindowsHeader.h"
#include <cstdint>
#include <string>

//=============================================================================
// Constants
//=============================================================================

constexpr uint32_t LAUNCHER_SESSION_ACTIVE_CONSOLE = 0xffffffff;

//=============================================================================
// Enumerations
//=============================================================================

/// @brief 프로세스 실행 권한 레벨
enum class launcher_privilege_level
{
	/// 로그인된 사용자 권한 (Medium Integrity Level)
	/// explorer.exe 토큰을 복제하여 사용
	user_privilege,

	/// 관리자 권한 (High Integrity Level)
	/// UAC 환경에서 Linked Token(Elevated Token) 사용
	/// Linked Token이 없으면 실패 (PAM 환경 등)
	administrator_privilege,

	/// 시스템 권한 (System Integrity Level)
	/// LocalSystem 토큰을 대상 세션에 바인딩
	/// PAM 환경에서 관리자 권한이 필요할 때 사용
	system_privilege
};

/// @brief system_privilege 사용 시 환경 블록 전략
/// @details LocalSystem 토큰으로 프로세스를 생성할 때
///          환경 변수를 어떻게 설정할지 결정함
enum class environment_strategy
{
	/// SYSTEM 환경 변수 사용
	/// %USERPROFILE% = C:\Windows\System32\config\systemprofile
	/// 주의: 사용자 환경에 접근 불가
	system_environment,

	/// 로그인 사용자 환경 변수 사용 (권장)
	/// %USERPROFILE% = C:\Users\<username>
	/// SYSTEM 권한 + 사용자 환경 변수 조합
	user_environment,

	/// SYSTEM 환경을 기본으로 하고 사용자 경로만 병합
	/// PATH, TEMP 등 특정 변수만 사용자 값으로 오버라이드
	merged_environment
};

/// @brief 프로세스 생성 결과 코드
enum class launcher_result
{
	success,                    ///< 성공
	invalid_parameter,          ///< 잘못된 파라미터
	session_not_found,          ///< 세션을 찾을 수 없음
	token_source_not_found,     ///< 토큰 소스(explorer.exe 등) 없음
	token_acquisition_failed,   ///< 토큰 획득 실패
	token_duplication_failed,   ///< 토큰 복제 실패
	elevation_not_available,    ///< 권한 상승 불가 (Linked Token 없음)
	session_binding_failed,     ///< 세션 바인딩 실패
	environment_creation_failed,///< 환경 블록 생성 실패
	process_creation_failed,    ///< CreateProcessAsUser 실패
	wait_failed,                ///< 프로세스 대기 실패
	internal_error              ///< 내부 오류
};

//=============================================================================
// Structures
//=============================================================================

/// @brief system_privilege 사용 시 추가 옵션
struct system_privilege_options
{
	/// 환경 블록 전략
	environment_strategy env_strategy = environment_strategy::user_environment;

	/// 사용자 환경을 가져올 세션 ID
	/// LAUNCHER_SESSION_ACTIVE_CONSOLE: 대상 세션과 동일
	uint32_t user_env_session_id = LAUNCHER_SESSION_ACTIVE_CONSOLE;

	system_privilege_options() = default;

	system_privilege_options(environment_strategy strategy)
		: env_strategy(strategy)
		, user_env_session_id(LAUNCHER_SESSION_ACTIVE_CONSOLE)
	{}
};

/// @brief 세션 권한 분석 결과
/// @details analyze_session_capabilities()로 세션 환경을 분석한 결과
struct session_capability_info
{
	/// 분석된 세션 ID
	uint32_t session_id = 0;

	/// 로그인 사용자 정보
	std::wstring username;
	std::wstring domain;
	std::wstring user_sid;

	/// 관리자 그룹 멤버십
	bool is_admin_group_member = false;

	/// Linked Token(Elevated Token) 존재 여부
	/// true: UAC 환경에서 관리자 권한 상승 가능
	/// false: 표준 사용자이거나 UAC 비활성화
	bool has_linked_token = false;

	/// Token Elevation Type
	/// TokenElevationTypeDefault(1): UAC 비활성화 또는 표준 사용자
	/// TokenElevationTypeFull(2): 이미 Elevated 상태
	/// TokenElevationTypeLimited(3): UAC 활성화, 제한된 토큰
	DWORD token_elevation_type = 0;

	/// PAM 환경 추정
	/// 관리자 그룹이 아니고 Linked Token도 없는 경우
	bool is_pam_environment = false;

	/// 관리자 권한이 필요할 때 권장되는 권한 레벨
	/// - 일반 환경: administrator_privilege
	/// - PAM 환경: system_privilege
	launcher_privilege_level recommended_admin_level = launcher_privilege_level::user_privilege;
};

/// @brief 프로세스 생성 요청 파라미터 (구조체 기반 API용)
struct process_launch_request
{
	/// 권한 레벨
	launcher_privilege_level privilege_level = launcher_privilege_level::user_privilege;

	/// 대상 세션 ID (LAUNCHER_SESSION_ACTIVE_CONSOLE: 활성 콘솔 세션)
	uint32_t session_id = LAUNCHER_SESSION_ACTIVE_CONSOLE;

	/// 실행할 명령줄 (NULL 종료 문자열)
	const wchar_t* cmdline = nullptr;

	/// 작업 디렉토리 (nullptr: 현재 디렉토리)
	const wchar_t* working_directory = nullptr;

	/// 프로세스 종료 대기 여부
	bool wait_for_exit = false;

	/// system_privilege일 때 추가 옵션
	system_privilege_options sys_options;

	process_launch_request() = default;

	process_launch_request(
		launcher_privilege_level level,
		uint32_t sess_id,
		const wchar_t* cmd
	)
		: privilege_level(level)
		, session_id(sess_id)
		, cmdline(cmd)
		, working_directory(nullptr)
		, wait_for_exit(false)
	{}
};

/// @brief 프로세스 생성 결과 정보
struct process_launch_response
{
	/// 결과 코드
	launcher_result result = launcher_result::internal_error;

	/// 생성된 프로세스 정보
	/// result가 success일 때 유효
	PROCESS_INFORMATION pi = { 0 };

	/// 프로세스 종료 코드
	/// wait_for_exit=true이고 result가 success일 때 유효
	DWORD exit_code = 0;

	/// 실패 시 GetLastError() 값
	DWORD last_error = 0;

	/// 사용된 토큰 소스 프로세스 이름 (디버깅용)
	std::wstring token_source_process;
};

//=============================================================================
// Main API Functions
//=============================================================================

/// @brief 세션의 권한 상승 가능 여부 및 환경 분석
/// @details 대상 세션의 로그인 사용자가 관리자 그룹인지,
///          UAC Linked Token이 있는지 등을 분석하여
///          적절한 권한 레벨을 추천함
/// 
/// @param session_id 대상 세션 ID (LAUNCHER_SESSION_ACTIVE_CONSOLE: 활성 콘솔 세션)
/// @param info [OUT] 분석 결과
/// @return 성공 시 true
/// 
/// @code
/// session_capability_info info;
/// if (analyze_session_capabilities(LAUNCHER_SESSION_ACTIVE_CONSOLE, info))
/// {
///     if (info.is_pam_environment)
///     {
///         // PAM 환경: system_privilege 사용 권장
///         launch_process(launcher_privilege_level::system_privilege, ...);
///     }
///     else if (info.has_linked_token)
///     {
///         // UAC 환경: administrator_privilege 사용 가능
///         launch_process(launcher_privilege_level::administrator_privilege, ...);
///     }
/// }
/// @endcode
bool analyze_session_capabilities(
	_In_ uint32_t session_id,
	_Out_ session_capability_info& info
);

/// @brief 지정된 권한으로 프로세스 생성 (구조체 기반 API)
/// @details 가장 유연한 API로, 모든 옵션을 구조체로 전달
/// 
/// @param request 생성 요청 파라미터
/// @param response [OUT] 생성 결과
/// @return 성공 시 true
/// 
/// @code
/// process_launch_request req;
/// req.privilege_level = launcher_privilege_level::system_privilege;
/// req.session_id = target_session;
/// req.cmdline = L"C:\\MyApp\\tool.exe";
/// req.sys_options.env_strategy = environment_strategy::user_environment;
/// 
/// process_launch_response resp;
/// if (launch_process(req, resp))
/// {
///     // resp.pi에서 프로세스 핸들 사용
///     CloseHandle(resp.pi.hProcess);
///     CloseHandle(resp.pi.hThread);
/// }
/// @endcode
bool launch_process(
	_In_ const process_launch_request& request,
	_Out_ process_launch_response& response
);

/// @brief 지정된 권한으로 프로세스 생성 (간편 API)
/// @details 자주 사용되는 파라미터만 받는 간편한 버전
/// 
/// @param privilege_level 권한 레벨
/// @param session_id 대상 세션 ID (LAUNCHER_SESSION_ACTIVE_CONSOLE: 활성 콘솔 세션)
/// @param cmdline 실행할 명령줄
/// @param pi [OUT] 생성된 프로세스 정보 (nullptr 가능)
/// @param wait 프로세스 종료 대기 여부
/// @param exit_code [OUT] 프로세스 종료 코드 (nullptr 가능, wait=true일 때 유효)
/// @return 성공 시 true
/// 
/// @code
/// PROCESS_INFORMATION pi;
/// if (launch_process(
///         launcher_privilege_level::user_privilege,
///         LAUNCHER_SESSION_ACTIVE_CONSOLE,
///         L"notepad.exe",
///         &pi))
/// {
///     CloseHandle(pi.hProcess);
///     CloseHandle(pi.hThread);
/// }
/// @endcode
bool launch_process(
	_In_ launcher_privilege_level privilege_level,
	_In_ uint32_t session_id,
	_In_ const wchar_t* cmdline,
	_Out_opt_ PROCESS_INFORMATION* pi = nullptr,
	_In_ bool wait = false,
	_Out_opt_ DWORD* exit_code = nullptr
);

/// @brief system_privilege 옵션을 포함한 프로세스 생성
/// @details PAM 환경에서 사용자 환경 변수가 필요할 때 사용
/// 
/// @param privilege_level 권한 레벨
/// @param session_id 대상 세션 ID
/// @param cmdline 실행할 명령줄
/// @param sys_options system_privilege 옵션
/// @param pi [OUT] 생성된 프로세스 정보 (nullptr 가능)
/// @param wait 프로세스 종료 대기 여부
/// @param exit_code [OUT] 프로세스 종료 코드 (nullptr 가능)
/// @return 성공 시 true
bool launch_process_ex(
	_In_ launcher_privilege_level privilege_level,
	_In_ uint32_t session_id,
	_In_ const wchar_t* cmdline,
	_In_ const system_privilege_options& sys_options,
	_Out_opt_ PROCESS_INFORMATION* pi = nullptr,
	_In_ bool wait = false,
	_Out_opt_ DWORD* exit_code = nullptr
);

//=============================================================================
// Utility Functions
//=============================================================================

/// @brief 결과 코드를 문자열로 변환
/// @param result 결과 코드
/// @return 결과 코드에 해당하는 문자열 (정적 문자열)
const wchar_t* get_launcher_result_string(
	_In_ launcher_result result
);

/// @brief 권한 레벨을 문자열로 변환
/// @param level 권한 레벨
/// @return 권한 레벨에 해당하는 문자열 (정적 문자열)
const wchar_t* get_privilege_level_string(
	_In_ launcher_privilege_level level
);

/// @brief 환경 전략을 문자열로 변환
/// @param strategy 환경 전략
/// @return 환경 전략에 해당하는 문자열 (정적 문자열)
const wchar_t* get_environment_strategy_string(
	_In_ environment_strategy strategy
);

//=============================================================================
// Helper Functions (Advanced Usage)
//=============================================================================

/// @brief 세션의 로그인 사용자 환경 블록 생성
/// @details explorer.exe 토큰을 사용하여 사용자 환경 블록 생성
/// 
/// @param session_id 대상 세션 ID
/// @param env_block [OUT] 생성된 환경 블록
///                  사용 후 DestroyEnvironmentBlock()으로 해제 필요
/// @return 성공 시 true
bool create_user_environment_for_session(
	_In_ uint32_t session_id,
	_Out_ LPVOID* env_block
);

/// @brief LocalSystem 토큰을 대상 세션에 바인딩
/// @details 현재 프로세스(서비스) 토큰을 복제하고 세션 ID 설정
/// 
/// @param session_id 대상 세션 ID
/// @param bound_token [OUT] 세션 바인딩된 토큰
///                    사용 후 CloseHandle()로 해제 필요
/// @return 성공 시 true
/// 
/// @note 호출자는 SE_TCB_PRIVILEGE 권한이 필요함 (LocalSystem 서비스)
bool create_session_bound_system_token(
	_In_ uint32_t session_id,
	_Out_ HANDLE* bound_token
);

/// @brief 토큰이 Administrators 그룹에 속하는지 확인
/// @param token 검사할 토큰 핸들
/// @param is_admin [OUT] 관리자 그룹 멤버십 여부
/// @return 성공 시 true (실패 시 is_admin은 변경되지 않음)
bool is_token_in_admin_group(
	_In_ HANDLE token,
	_Out_ bool& is_admin
);

/// @brief 토큰에 Linked Token이 있는지 확인
/// @param token 검사할 토큰 핸들
/// @param has_linked [OUT] Linked Token 존재 여부
/// @return 성공 시 true
bool has_linked_token(
	_In_ HANDLE token,
	_Out_ bool& has_linked
);

/// @brief 세션 ID 검증 및 변환
/// @param session_id 입력 세션 ID (LAUNCHER_SESSION_ACTIVE_CONSOLE 가능)
/// @param validated_session_id [OUT] 검증된 실제 세션 ID
/// @return 성공 시 true
bool validate_session_id(
	_In_ uint32_t session_id,
	_Out_ uint32_t& validated_session_id
);

//=============================================================================
// Local Process Creation (Simple CreateProcessW wrapper)
//=============================================================================

/// @brief 현재 세션/권한으로 프로세스 생성 (CreateProcessW wrapper)
/// @details Cross-session이 필요 없는 경우 간단히 프로세스를 생성함.
///          기존 create_process() 함수의 대체 API.
/// 
/// @param cmdline 실행할 명령줄 (NULL 종료 문자열)
/// @param creation_flags 프로세스 생성 플래그 (CREATE_NO_WINDOW 등)
/// @param working_directory 작업 디렉토리 (nullptr: 현재 디렉토리)
/// @param process_handle [OUT] 생성된 프로세스 핸들
/// @param process_id [OUT] 생성된 프로세스 ID
/// @return 성공 시 true
/// 
/// @code
/// HANDLE hProc = nullptr;
/// DWORD pid = 0;
/// if (launch_process_local(L"notepad.exe", 0, nullptr, hProc, pid))
/// {
///     // 프로세스 핸들 사용 후 해제
///     CloseHandle(hProc);
/// }
/// @endcode
bool launch_process_local(
	_In_z_ const wchar_t* cmdline,
	_In_ DWORD creation_flags,
	_In_opt_z_ const wchar_t* working_directory,
	_Out_ HANDLE& process_handle,
	_Out_ DWORD& process_id
);

/// @brief 프로세스를 생성하고 종료될 때까지 대기
/// @details 기존 create_process_and_wait() 함수의 대체 API.
/// 
/// @param cmdline 실행할 명령줄 (NULL 종료 문자열)
/// @param creation_flags 프로세스 생성 플래그 (CREATE_NO_WINDOW 등)
/// @param working_directory 작업 디렉토리 (nullptr: 현재 디렉토리)
/// @param timeout_secs 대기 타임아웃 초 단위 (INFINITE: 무한 대기)
/// @param exit_code [OUT] 프로세스 종료 코드 (nullptr 가능)
/// @return 성공 시 true (프로세스가 정상 종료된 경우)
/// 
/// @code
/// DWORD exit_code = 0;
/// if (launch_process_local_and_wait(
///         L"netcfg.exe -v -u component",
///         CREATE_NO_WINDOW,
///         nullptr,
///         30,  // 30초 대기
///         &exit_code))
/// {
///     if (exit_code == 0)
///     {
///         // 성공적으로 완료됨
///     }
/// }
/// @endcode
bool launch_process_local_and_wait(
	_In_z_ const wchar_t* cmdline,
	_In_ DWORD creation_flags,
	_In_opt_z_ const wchar_t* working_directory,
	_In_ DWORD timeout_secs,
	_Out_opt_ DWORD* exit_code
);
