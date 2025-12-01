/**
 * @file    ProcessLauncher.cpp
 * @brief   Cross-session 프로세스 생성 모듈 구현
 *          다양한 권한 레벨(User/Admin/System)과 환경 설정을 지원
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2024-11-29 created
 * @copyright (C)Somma, Inc. All rights reserved.
**/

#include "stdafx.h"
#include "ProcessLauncher.h"
#include "process_tree.h"
#include "log.h"

#include <UserEnv.h>
#include <WtsApi32.h>
#include <strsafe.h>
#include <sddl.h>

#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "Wtsapi32.lib")

//=============================================================================
// Internal Constants
//=============================================================================

namespace
{
	// Token source process names in priority order
	const wchar_t* g_token_source_processes[] = { L"explorer.exe", L"winlogon.exe" };
	constexpr size_t g_token_source_count = sizeof(g_token_source_processes) / sizeof(g_token_source_processes[0]);

	// Administrators group SID: S-1-5-32-544
	constexpr DWORD ADMIN_GROUP_RID = DOMAIN_ALIAS_RID_ADMINS;

	// Maximum command line length
	constexpr size_t MAX_CMDLINE_LENGTH = MAX_PATH * 2;

	// Process creation retry count
	constexpr int MAX_CREATION_RETRIES = 3;
}

//=============================================================================
// Internal Helper Function Declarations
//=============================================================================

namespace
{
	/// @brief RAII wrapper for handles
	class scoped_handle
	{
	public:
		explicit scoped_handle(HANDLE h = nullptr) : _handle(h) {}
		~scoped_handle() { close(); }

		scoped_handle(const scoped_handle&) = delete;
		scoped_handle& operator=(const scoped_handle&) = delete;

		scoped_handle(scoped_handle&& other) noexcept : _handle(other._handle)
		{
			other._handle = nullptr;
		}

		scoped_handle& operator=(scoped_handle&& other) noexcept
		{
			if (this != &other)
			{
				close();
				_handle = other._handle;
				other._handle = nullptr;
			}
			return *this;
		}

		void close()
		{
			if (_handle != nullptr && _handle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(_handle);
				_handle = nullptr;
			}
		}

		HANDLE get() const { return _handle; }
		HANDLE* addressof() { return &_handle; }
		HANDLE release()
		{
			HANDLE h = _handle;
			_handle = nullptr;
			return h;
		}

		explicit operator bool() const
		{
			return _handle != nullptr && _handle != INVALID_HANDLE_VALUE;
		}

	private:
		HANDLE _handle;
	};

	/// @brief RAII wrapper for environment blocks
	class scoped_environment
	{
	public:
		explicit scoped_environment(LPVOID env = nullptr) : _env(env) {}
		~scoped_environment()
		{
			destroy();
		}

		scoped_environment(const scoped_environment&) = delete;
		scoped_environment& operator=(const scoped_environment&) = delete;

		scoped_environment(scoped_environment&& other) noexcept : _env(other._env)
		{
			other._env = nullptr;
		}

		scoped_environment& operator=(scoped_environment&& other) noexcept
		{
			if (this != &other)
			{
				destroy();
				_env = other._env;
				other._env = nullptr;
			}
			return *this;
		}

		void destroy()
		{
			if (_env != nullptr)
			{
				DestroyEnvironmentBlock(_env);
				_env = nullptr;
			}
		}

		LPVOID get() const { return _env; }
		LPVOID release()
		{
			LPVOID e = _env;
			_env = nullptr;
			return e;
		}

	private:
		LPVOID _env;
	};

	/// @brief Find process in session and get token (uses pre-built process tree)
	bool get_token_from_session_process(
		_In_ cprocess_tree& pt,
		_In_ const wchar_t* process_name,
		_In_ DWORD target_session_id,
		_In_ launcher_privilege_level privilege_level,
		_Out_ HANDLE* out_token
	);

	/// @brief Find process in session and get token (builds process tree internally)
	bool get_token_from_session_process(
		_In_ const wchar_t* process_name,
		_In_ DWORD target_session_id,
		_In_ launcher_privilege_level privilege_level,
		_Out_ HANDLE* out_token
	);

	/// @brief Acquire execution token based on privilege level
	bool acquire_execution_token(
		_In_ DWORD target_session_id,
		_In_ launcher_privilege_level privilege_level,
		_Out_ HANDLE* out_token,
		_Out_opt_ std::wstring* out_source_process
	);

	/// @brief Create process with retry logic
	launcher_result create_process_internal(
		_In_ HANDLE token,
		_In_ wchar_t* cmdline,
		_In_ const wchar_t* working_dir,
		_In_ LPVOID env_block,
		_Out_ PROCESS_INFORMATION* pi
	);
}

//=============================================================================
// Utility Function Implementations
//=============================================================================

/// @brief Get string representation of launcher result
const wchar_t* get_launcher_result_string(_In_ launcher_result result)
{
	switch (result)
	{
	case launcher_result::success:
		return L"success";
	case launcher_result::invalid_parameter:
		return L"invalid_parameter";
	case launcher_result::session_not_found:
		return L"session_not_found";
	case launcher_result::token_source_not_found:
		return L"token_source_not_found";
	case launcher_result::token_acquisition_failed:
		return L"token_acquisition_failed";
	case launcher_result::token_duplication_failed:
		return L"token_duplication_failed";
	case launcher_result::elevation_not_available:
		return L"elevation_not_available";
	case launcher_result::session_binding_failed:
		return L"session_binding_failed";
	case launcher_result::environment_creation_failed:
		return L"environment_creation_failed";
	case launcher_result::process_creation_failed:
		return L"process_creation_failed";
	case launcher_result::wait_failed:
		return L"wait_failed";
	case launcher_result::internal_error:
	default:
		return L"internal_error";
	}
}

/// @brief Get string representation of privilege level
const wchar_t* get_privilege_level_string(_In_ launcher_privilege_level level)
{
	switch (level)
	{
	case launcher_privilege_level::user_privilege:
		return L"user_privilege";
	case launcher_privilege_level::administrator_privilege:
		return L"administrator_privilege";
	case launcher_privilege_level::system_privilege:
		return L"system_privilege";
	default:
		return L"unknown";
	}
}

/// @brief Get string representation of environment strategy
const wchar_t* get_environment_strategy_string(_In_ environment_strategy strategy)
{
	switch (strategy)
	{
	case environment_strategy::system_environment:
		return L"system_environment";
	case environment_strategy::user_environment:
		return L"user_environment";
	case environment_strategy::merged_environment:
		return L"merged_environment";
	default:
		return L"unknown";
	}
}

//=============================================================================
// Helper Function Implementations
//=============================================================================

/// @brief Validate and resolve session ID
bool validate_session_id(
	_In_ uint32_t session_id,
	_Out_ uint32_t& validated_session_id
)
{
	validated_session_id = session_id;

	// If active console session requested, get actual session ID
	if (session_id == LAUNCHER_SESSION_ACTIVE_CONSOLE)
	{
		validated_session_id = WTSGetActiveConsoleSessionId();
		if (validated_session_id == 0xffffffff)
		{
			log_err "WTSGetActiveConsoleSessionId() failed. gle=%u", GetLastError() log_end;
			return false;
		}

		log_dbg "Active console session resolved: %u", validated_session_id log_end;
		return true;
	}

	// Validate specified session exists
	WTS_SESSION_INFO* session_info = nullptr;
	DWORD session_count = 0;
	bool session_valid = false;

	if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &session_info, &session_count))
	{
		for (DWORD i = 0; i < session_count; i++)
		{
			if (session_info[i].SessionId == session_id)
			{
				session_valid = true;
				log_dbg "Session %u is valid (state: %d)", session_id, session_info[i].State log_end;
				break;
			}
		}
		WTSFreeMemory(session_info);
	}

	if (!session_valid)
	{
		log_err "Session %u is not valid or not found", session_id log_end;
		return false;
	}

	return true;
}

/// @brief Check if token belongs to Administrators group
bool is_token_in_admin_group(
	_In_ HANDLE token,
	_Out_ bool& is_admin
)
{
	_ASSERTE(token != nullptr);
	if (token == nullptr)
	{
		return false;
	}

	is_admin = false;

	// Create Administrators group SID
	SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
	PSID admin_group_sid = nullptr;

	if (!AllocateAndInitializeSid(
		&nt_authority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&admin_group_sid))
	{
		log_err "AllocateAndInitializeSid() failed. gle=%u", GetLastError() log_end;
		return false;
	}

	// Check membership
	BOOL is_member = FALSE;
	BOOL result = CheckTokenMembership(token, admin_group_sid, &is_member);
	DWORD gle = GetLastError();

	FreeSid(admin_group_sid);

	if (!result)
	{
		log_warn "CheckTokenMembership() failed. gle=%u", gle log_end;
		return false;
	}

	is_admin = (is_member == TRUE);
	return true;
}

/// @brief Check if token has linked token (UAC elevated token)
bool has_linked_token(
	_In_ HANDLE token,
	_Out_ bool& has_linked
)
{
	_ASSERTE(token != nullptr);
	if (token == nullptr)
	{
		return false;
	}

	has_linked = false;

	// Get token elevation type first
	DWORD elevation_type = 0;
	DWORD return_length = 0;

	if (!GetTokenInformation(token, TokenElevationType, &elevation_type, sizeof(DWORD), &return_length))
	{
		log_warn "GetTokenInformation(TokenElevationType) failed. gle=%u", GetLastError() log_end;
		return false;
	}

	// TokenElevationTypeLimited means there's a linked elevated token
	// TokenElevationTypeFull means already elevated, may have linked limited token
	if (elevation_type == TokenElevationTypeLimited || elevation_type == TokenElevationTypeFull)
	{
		// Try to get linked token to verify
		TOKEN_LINKED_TOKEN linked_token = { 0 };
		DWORD size = sizeof(TOKEN_LINKED_TOKEN);

		if (GetTokenInformation(token, TokenLinkedToken, &linked_token, size, &size))
		{
			if (linked_token.LinkedToken != nullptr)
			{
				CloseHandle(linked_token.LinkedToken);
				has_linked = true;
			}
		}
	}

	return true;
}

/// @brief Create user environment block for session
bool create_user_environment_for_session(
	_In_ uint32_t session_id,
	_Out_ LPVOID* env_block
)
{
	_ASSERTE(env_block != nullptr);
	if (env_block == nullptr)
	{
		return false;
	}

	*env_block = nullptr;

	// Validate session ID
	uint32_t target_session = 0;
	if (!validate_session_id(session_id, target_session))
	{
		return false;
	}

	// Get token from explorer.exe in target session
	scoped_handle user_token;
	if (!get_token_from_session_process(
		L"explorer.exe",
		target_session,
		launcher_privilege_level::user_privilege,
		user_token.addressof()))
	{
		log_warn "Failed to get explorer token for environment block. session=%u", target_session log_end;
		return false;
	}

	// Create environment block from user token
	if (!CreateEnvironmentBlock(env_block, user_token.get(), TRUE))
	{
		log_err "CreateEnvironmentBlock() failed. gle=%u", GetLastError() log_end;
		return false;
	}

	log_dbg "User environment block created for session %u", target_session log_end;
	return true;
}

/// @brief Create LocalSystem token bound to target session
bool create_session_bound_system_token(
	_In_ uint32_t session_id,
	_Out_ HANDLE* bound_token
)
{
	_ASSERTE(bound_token != nullptr);
	if (bound_token == nullptr)
	{
		return false;
	}

	*bound_token = nullptr;

	// Validate session ID
	uint32_t target_session = 0;
	if (!validate_session_id(session_id, target_session))
	{
		return false;
	}

	// Get current process token (should be LocalSystem)
	scoped_handle current_token;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_QUERY, current_token.addressof()))
	{
		log_err "OpenProcessToken() failed for current process. gle=%u", GetLastError() log_end;
		return false;
	}

	// Duplicate token
	scoped_handle duplicated_token;
	if (!DuplicateTokenEx(current_token.get(),
						  MAXIMUM_ALLOWED,
						  nullptr,
						  SecurityImpersonation,
						  TokenPrimary,
						  duplicated_token.addressof()))
	{
		log_err "DuplicateTokenEx() failed. gle=%u", GetLastError() log_end;
		return false;
	}

	// Set session ID on duplicated token
	if (!SetTokenInformation(duplicated_token.get(),
							 TokenSessionId,
							 &target_session,
							 sizeof(DWORD)))
	{
		log_err "SetTokenInformation(TokenSessionId) failed. session=%u, gle=%u",
			target_session, GetLastError() log_end;
		return false;
	}

	*bound_token = duplicated_token.release();
	log_dbg "System token bound to session %u", target_session log_end;
	return true;
}

//=============================================================================
// Internal Helper Function Implementations
//=============================================================================

namespace
{
	/// @brief Get token from process running in target session (uses pre-built process tree)
	bool get_token_from_session_process(
		_In_ cprocess_tree& pt,
		_In_ const wchar_t* process_name,
		_In_ DWORD target_session_id,
		_In_ launcher_privilege_level privilege_level,
		_Out_ HANDLE* out_token
	)
	{
		_ASSERTE(process_name != nullptr && out_token != nullptr);
		if (process_name == nullptr || out_token == nullptr)
		{
			return false;
		}

		*out_token = nullptr;
		bool token_acquired = false;

		// Find process in target session
		pt.find_process(process_name,
			[&](_In_ const process* const proc_info) -> bool
			{
				if (proc_info == nullptr) return false;

				// Check session ID
				DWORD proc_session_id = 0;
				if (!ProcessIdToSessionId(proc_info->pid(), &proc_session_id) ||
					proc_session_id != target_session_id)
				{
					return false; // Continue searching
				}

				// Open process
				scoped_handle proc_handle(OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, proc_info->pid()));
				if (!proc_handle)
				{
					log_warn "OpenProcess() failed for %ws. pid=%u, gle=%u",
						process_name, proc_info->pid(), GetLastError() log_end;
					return false;
				}

				// Open process token
				scoped_handle proc_token;
				if (!OpenProcessToken(proc_handle.get(), TOKEN_QUERY | TOKEN_DUPLICATE, proc_token.addressof()))
				{
					log_warn "OpenProcessToken() failed for %ws. pid=%u, gle=%u",
						process_name, proc_info->pid(), GetLastError() log_end;
					return false;
				}

				// Handle based on privilege level
				if (privilege_level == launcher_privilege_level::administrator_privilege)
				{
					// Try to get linked token (UAC elevated token)
					TOKEN_LINKED_TOKEN linked_token = { 0 };
					DWORD size = sizeof(TOKEN_LINKED_TOKEN);

					if (GetTokenInformation(proc_token.get(), TokenLinkedToken, &linked_token, size, &size))
					{
						// Duplicate linked token
						if (DuplicateTokenEx(
							linked_token.LinkedToken,
							MAXIMUM_ALLOWED,
							nullptr,
							SecurityImpersonation,
							TokenPrimary,
							out_token))
						{
							CloseHandle(linked_token.LinkedToken);
							token_acquired = true;
							log_info "Acquired linked (elevated) token from %ws. pid=%u, session=%u",
								process_name, proc_info->pid(), target_session_id log_end;
							return true; // Stop searching
						}
						else
						{
							log_warn "DuplicateTokenEx() for linked token failed. gle=%u", GetLastError() log_end;
							CloseHandle(linked_token.LinkedToken);
						}
					}
					else
					{
						log_dbg "No linked token available from %ws. pid=%u, gle=%u",
							process_name, proc_info->pid(), GetLastError() log_end;
					}

					// Linked token not available - don't fall back to regular token
					// for administrator_privilege level
					return false;
				}
				else
				{
					// user_privilege or fallback: duplicate standard token
					if (DuplicateTokenEx(
						proc_token.get(),
						MAXIMUM_ALLOWED,
						nullptr,
						SecurityIdentification,
						TokenPrimary,
						out_token))
					{
						// Set session ID on token
						SetTokenInformation(*out_token, TokenSessionId, &target_session_id, sizeof(DWORD));

						token_acquired = true;
						log_info "Acquired standard token from %ws. pid=%u, session=%u",
							process_name, proc_info->pid(), target_session_id log_end;
						return true; // Stop searching
					}
					else
					{
						log_warn "DuplicateTokenEx() for standard token failed. gle=%u", GetLastError() log_end;
					}
				}

				return false; // Continue searching
			});

		return token_acquired;
	}

	/// @brief Get token from process running in target session (builds process tree internally)
	/// @note This overload is provided for convenience when only one process lookup is needed.
	///       For multiple lookups, use the overload that accepts a pre-built cprocess_tree.
	bool get_token_from_session_process(
		_In_ const wchar_t* process_name,
		_In_ DWORD target_session_id,
		_In_ launcher_privilege_level privilege_level,
		_Out_ HANDLE* out_token
	)
	{
		// Build process tree
		cprocess_tree pt;
		if (!pt.build_process_tree(true))
		{
			log_warn "build_process_tree() failed. gle=%u", GetLastError() log_end;
			return false;
		}

		return get_token_from_session_process(pt, process_name, target_session_id, privilege_level, out_token);
	}

	/// @brief Acquire execution token based on privilege level
	bool acquire_execution_token(
		_In_ DWORD target_session_id,
		_In_ launcher_privilege_level privilege_level,
		_Out_ HANDLE* out_token,
		_Out_opt_ std::wstring* out_source_process
	)
	{
		_ASSERTE(out_token != nullptr);
		if (out_token == nullptr)
		{
			return false;
		}

		*out_token = nullptr;
		if (out_source_process) out_source_process->clear();

		// For system_privilege, use current process token
		if (privilege_level == launcher_privilege_level::system_privilege)
		{
			if (create_session_bound_system_token(target_session_id, out_token))
			{
				if (out_source_process) *out_source_process = L"LocalSystem";
				return true;
			}
			return false;
		}

		// Build process tree once for all token source lookups
		cprocess_tree pt;
		if (!pt.build_process_tree(true))
		{
			log_warn "build_process_tree() failed. gle=%u", GetLastError() log_end;
			return false;
		}

		// Try each token source process in priority order
		for (size_t i = 0; i < g_token_source_count; i++)
		{
			const wchar_t* process_name = g_token_source_processes[i];

			log_dbg "Trying to get token from %ws for session %u", process_name, target_session_id log_end;

			if (get_token_from_session_process(pt, process_name, target_session_id, privilege_level, out_token))
			{
				if (out_source_process) *out_source_process = process_name;
				return true;
			}
		}

		log_err "Failed to acquire token from any source process. session=%u, level=%ws",
			target_session_id, get_privilege_level_string(privilege_level) log_end;
		return false;
	}

	/// @brief Create process with retry logic
	launcher_result create_process_internal(
		_In_ HANDLE token,
		_In_ wchar_t* cmdline,
		_In_ const wchar_t* working_dir,
		_In_ LPVOID env_block,
		_Out_ PROCESS_INFORMATION* pi
	)
	{
		_ASSERTE(token != nullptr && cmdline != nullptr && pi != nullptr);
		if (token == nullptr || cmdline == nullptr || pi == nullptr)
		{
			return launcher_result::invalid_parameter;
		}

		ZeroMemory(pi, sizeof(PROCESS_INFORMATION));

		STARTUPINFOW si = { 0 };
		si.cb = sizeof(STARTUPINFOW);
		si.lpDesktop = const_cast<LPWSTR>(L"winsta0\\default");

		DWORD creation_flags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
		if (env_block != nullptr)
		{
			creation_flags |= CREATE_UNICODE_ENVIRONMENT;
		}

		// First attempt: standard creation
		if (CreateProcessAsUserW(
			token,
			nullptr,
			cmdline,
			nullptr,
			nullptr,
			FALSE,
			creation_flags,
			env_block,
			working_dir,
			&si,
			pi))
		{
			log_dbg "CreateProcessAsUserW() succeeded on first attempt. pid=%u", pi->dwProcessId log_end;
			return launcher_result::success;
		}

		DWORD first_error = GetLastError();
		log_dbg "CreateProcessAsUserW() first attempt failed. gle=%u", first_error log_end;

		// Retry 1: without environment block
		if (env_block != nullptr)
		{
			log_dbg "Retrying without environment block" log_end;
			DWORD retry_flags = creation_flags & ~CREATE_UNICODE_ENVIRONMENT;

			if (CreateProcessAsUserW(
				token,
				nullptr,
				cmdline,
				nullptr,
				nullptr,
				FALSE,
				retry_flags,
				nullptr,
				working_dir,
				&si,
				pi))
			{
				log_dbg "CreateProcessAsUserW() succeeded without environment block. pid=%u", pi->dwProcessId log_end;
				return launcher_result::success;
			}
			log_dbg "Retry without environment failed. gle=%u", GetLastError() log_end;
		}

		// Retry 2: without desktop specification
		log_dbg "Retrying without desktop specification" log_end;
		STARTUPINFOW retry_si = si;
		retry_si.lpDesktop = nullptr;

		if (CreateProcessAsUserW(
			token,
			nullptr,
			cmdline,
			nullptr,
			nullptr,
			FALSE,
			NORMAL_PRIORITY_CLASS,
			nullptr,
			working_dir,
			&retry_si,
			pi))
		{
			log_dbg "CreateProcessAsUserW() succeeded without desktop. pid=%u", pi->dwProcessId log_end;
			return launcher_result::success;
		}

		log_err "All CreateProcessAsUserW() attempts failed. first_gle=%u, final_gle=%u",
			first_error, GetLastError() log_end;
		return launcher_result::process_creation_failed;
	}
}

//=============================================================================
// Main API Implementations
//=============================================================================

/// @brief Analyze session capabilities
bool analyze_session_capabilities(
	_In_ uint32_t session_id,
	_Out_ session_capability_info& info
)
{
	// Initialize output
	info = session_capability_info();

	// Validate session ID
	if (!validate_session_id(session_id, info.session_id))
	{
		return false;
	}

	// Get explorer token for analysis
	scoped_handle explorer_token;
	if (!get_token_from_session_process(
		L"explorer.exe",
		info.session_id,
		launcher_privilege_level::user_privilege,
		explorer_token.addressof()))
	{
		log_warn "Failed to get explorer token for session analysis. session=%u", info.session_id log_end;
		return false;
	}

	// Get token elevation type
	DWORD return_length = 0;
	if (GetTokenInformation(
		explorer_token.get(),
		TokenElevationType,
		&info.token_elevation_type,
		sizeof(DWORD),
		&return_length))
	{
		log_dbg "Token elevation type: %u", info.token_elevation_type log_end;
	}

	// Check admin group membership
	is_token_in_admin_group(explorer_token.get(), info.is_admin_group_member);

	// Check for linked token
	has_linked_token(explorer_token.get(), info.has_linked_token);

	// Get user information
	DWORD token_user_size = 0;
	GetTokenInformation(explorer_token.get(), TokenUser, nullptr, 0, &token_user_size);
	if (token_user_size > 0)
	{
		std::vector<BYTE> buffer(token_user_size);
		if (GetTokenInformation(
			explorer_token.get(),
			TokenUser,
			buffer.data(),
			token_user_size,
			&token_user_size))
		{
			PTOKEN_USER token_user = reinterpret_cast<PTOKEN_USER>(buffer.data());

			// Convert SID to string
			LPWSTR sid_string = nullptr;
			if (ConvertSidToStringSidW(token_user->User.Sid, &sid_string))
			{
				info.user_sid = sid_string;
				LocalFree(sid_string);
			}

			// Lookup account name
			wchar_t name[256] = { 0 };
			wchar_t domain[256] = { 0 };
			DWORD name_size = 256;
			DWORD domain_size = 256;
			SID_NAME_USE sid_type;

			if (LookupAccountSidW(
				nullptr,
				token_user->User.Sid,
				name,
				&name_size,
				domain,
				&domain_size,
				&sid_type))
			{
				info.username = name;
				info.domain = domain;
			}
		}
	}

	// Determine environment type and recommended level
	if (!info.is_admin_group_member && !info.has_linked_token)
	{
		// Standard user without linked token - likely PAM environment
		info.is_pam_environment = true;
		info.recommended_admin_level = launcher_privilege_level::system_privilege;
	}
	else if (info.has_linked_token)
	{
		// Has linked token - can use administrator_privilege
		info.recommended_admin_level = launcher_privilege_level::administrator_privilege;
	}
	else if (info.is_admin_group_member)
	{
		// Admin group member but no linked token (UAC disabled?)
		info.recommended_admin_level = launcher_privilege_level::administrator_privilege;
	}
	else
	{
		// Standard user - can only use user_privilege
		info.recommended_admin_level = launcher_privilege_level::user_privilege;
	}

	log_info "Session %u analysis: user=%ws\\%ws, admin_group=%s, linked_token=%s, pam=%s, recommended=%ws",
		info.session_id,
		info.domain.c_str(),
		info.username.c_str(),
		info.is_admin_group_member ? "yes" : "no",
		info.has_linked_token ? "yes" : "no",
		info.is_pam_environment ? "yes" : "no",
		get_privilege_level_string(info.recommended_admin_level)
		log_end;

	return true;
}

/// @brief Launch process (struct-based API)
bool launch_process(
	_In_ const process_launch_request& request,
	_Out_ process_launch_response& response
)
{
	// Initialize response
	response = process_launch_response();

	// Validate parameters
	if (request.cmdline == nullptr)
	{
		log_err "cmdline parameter is null" log_end;
		response.result = launcher_result::invalid_parameter;
		return false;
	}

	size_t cmdline_len = wcslen(request.cmdline);
	if (cmdline_len == 0)
	{
		log_err "cmdline is empty" log_end;
		response.result = launcher_result::invalid_parameter;
		return false;
	}

	if (cmdline_len > MAX_CMDLINE_LENGTH)
	{
		log_err "cmdline too long. len=%zu, max=%zu", cmdline_len, MAX_CMDLINE_LENGTH log_end;
		response.result = launcher_result::invalid_parameter;
		return false;
	}

	log_dbg "Launching process. cmd=%ws, privilege=%ws, session=%u",
		request.cmdline,
		get_privilege_level_string(request.privilege_level),
		request.session_id
		log_end;

	// Validate session ID
	uint32_t target_session = 0;
	if (!validate_session_id(request.session_id, target_session))
	{
		response.result = launcher_result::session_not_found;
		response.last_error = GetLastError();
		return false;
	}

	// Acquire execution token
	scoped_handle exec_token;
	if (!acquire_execution_token(target_session,
								 request.privilege_level,
								 exec_token.addressof(),
								 &response.token_source_process))
	{
		// Determine specific error
		if (request.privilege_level == launcher_privilege_level::administrator_privilege)
		{
			response.result = launcher_result::elevation_not_available;
		}
		else
		{
			response.result = launcher_result::token_acquisition_failed;
		}
		response.last_error = GetLastError();
		return false;
	}

	// Create environment block
	scoped_environment env_block;

	if (request.privilege_level == launcher_privilege_level::system_privilege)
	{
		// Handle environment strategy for system privilege
		switch (request.sys_options.env_strategy)
		{
		case environment_strategy::user_environment:
		case environment_strategy::merged_environment:
		{
			uint32_t env_session = (request.sys_options.user_env_session_id == LAUNCHER_SESSION_ACTIVE_CONSOLE)
				? target_session
				: request.sys_options.user_env_session_id;

			LPVOID user_env = nullptr;
			if (create_user_environment_for_session(env_session, &user_env))
			{
				env_block = scoped_environment(user_env);
				log_dbg "Using user environment from session %u", env_session log_end;
			}
			else
			{
				log_warn "Failed to create user environment, continuing without it" log_end;
			}
			break;
		}

		case environment_strategy::system_environment:
		default:
			// No environment block needed - will use system default
			log_dbg "Using system environment" log_end;
			break;
		}
	}
	else
	{
		// For user/admin privilege, create user environment block
		LPVOID user_env = nullptr;
		if (create_user_environment_for_session(target_session, &user_env))
		{
			env_block = scoped_environment(user_env);
		}
		else
		{
			log_warn "Failed to create environment block, continuing without it" log_end;
		}
	}

	// Allocate command line buffer (CreateProcessAsUser may modify it)
	std::vector<wchar_t> cmd_buffer(cmdline_len + 1);
	if (FAILED(StringCchCopyW(cmd_buffer.data(), cmd_buffer.size(), request.cmdline)))
	{
		log_err "StringCchCopyW() failed. gle=%u", GetLastError() log_end;
		response.result = launcher_result::internal_error;
		response.last_error = GetLastError();
		return false;
	}

	// Create process
	response.result = create_process_internal(
		exec_token.get(),
		cmd_buffer.data(),
		request.working_directory,
		env_block.get(),
		&response.pi
	);

	if (response.result != launcher_result::success)
	{
		response.last_error = GetLastError();
		return false;
	}

	// Wait for process if requested
	if (request.wait_for_exit && response.pi.hProcess != nullptr)
	{
		log_dbg "Waiting for process to exit. pid=%u", response.pi.dwProcessId log_end;

		DWORD wait_result = WaitForSingleObject(response.pi.hProcess, INFINITE);
		if (wait_result == WAIT_OBJECT_0)
		{
			if (!GetExitCodeProcess(response.pi.hProcess, &response.exit_code))
			{
				log_warn "GetExitCodeProcess() failed. gle=%u", GetLastError() log_end;
			}
			else
			{
				log_dbg "Process exited. pid=%u, exit_code=%u",
					response.pi.dwProcessId, response.exit_code log_end;
			}
		}
		else
		{
			log_err "WaitForSingleObject() failed. gle=%u", GetLastError() log_end;
			response.result = launcher_result::wait_failed;
			response.last_error = GetLastError();
		}
	}

	log_info "Process launched successfully. pid=%u, session=%u, cmd=%ws",
		response.pi.dwProcessId, target_session, request.cmdline log_end;

	return (response.result == launcher_result::success);
}

/// @brief Launch process (simple API)
bool launch_process(
	_In_ launcher_privilege_level privilege_level,
	_In_ uint32_t session_id,
	_In_ const wchar_t* cmdline,
	_Out_opt_ PROCESS_INFORMATION* pi,
	_In_ bool wait,
	_Out_opt_ DWORD* exit_code
)
{
	process_launch_request request;
	request.privilege_level = privilege_level;
	request.session_id = session_id;
	request.cmdline = cmdline;
	request.wait_for_exit = wait;

	process_launch_response response;
	bool result = launch_process(request, response);

	if (pi != nullptr)
	{
		*pi = response.pi;
	}
	else if (result)
	{
		// Close handles if caller doesn't want them
		if (response.pi.hProcess != nullptr) CloseHandle(response.pi.hProcess);
		if (response.pi.hThread != nullptr) CloseHandle(response.pi.hThread);
	}

	if (exit_code != nullptr)
	{
		*exit_code = response.exit_code;
	}

	return result;
}

/// @brief Launch process with system privilege options
bool launch_process_ex(
	_In_ launcher_privilege_level privilege_level,
	_In_ uint32_t session_id,
	_In_ const wchar_t* cmdline,
	_In_ const system_privilege_options& sys_options,
	_Out_opt_ PROCESS_INFORMATION* pi,
	_In_ bool wait,
	_Out_opt_ DWORD* exit_code
)
{
	process_launch_request request;
	request.privilege_level = privilege_level;
	request.session_id = session_id;
	request.cmdline = cmdline;
	request.wait_for_exit = wait;
	request.sys_options = sys_options;

	process_launch_response response;
	bool result = launch_process(request, response);

	if (pi != nullptr)
	{
		*pi = response.pi;
	}
	else if (result)
	{
		// Close handles if caller doesn't want them
		if (response.pi.hProcess != nullptr) CloseHandle(response.pi.hProcess);
		if (response.pi.hThread != nullptr) CloseHandle(response.pi.hThread);
	}

	if (exit_code != nullptr)
	{
		*exit_code = response.exit_code;
	}

	return result;
}

//=============================================================================
// Local Process Creation (Simple CreateProcessW wrapper)
//=============================================================================

/// @brief Local process creation using CreateProcessW
bool launch_process_local(
	_In_z_ const wchar_t* cmdline,
	_In_ DWORD creation_flags,
	_In_opt_z_ const wchar_t* working_directory,
	_Out_ HANDLE& process_handle,
	_Out_ DWORD& process_id
)
{
	process_handle = nullptr;
	process_id = 0;

	_ASSERTE(cmdline != nullptr);
	if (cmdline == nullptr)
	{
		log_err "cmdline parameter is null" log_end;
		return false;
	}

	// CreateProcessW requires writable command line buffer
	size_t cmdline_len = wcslen(cmdline);
	if (cmdline_len == 0)
	{
		log_err "cmdline is empty" log_end;
		return false;
	}

	size_t buf_size = (cmdline_len + 1) * sizeof(wchar_t);
	std::unique_ptr<wchar_t[]> cmdline_buf(new (std::nothrow) wchar_t[cmdline_len + 1]);
	if (!cmdline_buf)
	{
		log_err "Failed to allocate cmdline buffer. size=%zu", buf_size log_end;
		return false;
	}

	// Copy command line to writable buffer
	if (FAILED(StringCchCopyW(cmdline_buf.get(), cmdline_len + 1, cmdline)))
	{
		log_err "StringCchCopyW() failed" log_end;
		return false;
	}

	// Prepare startup info and process info
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(STARTUPINFOW);

	PROCESS_INFORMATION pi = { 0 };

	// Create process
	if (!CreateProcessW(
		nullptr,
		cmdline_buf.get(),
		nullptr,
		nullptr,
		FALSE,
		creation_flags,
		nullptr,
		working_directory,
		&si,
		&pi))
	{
		log_err "CreateProcessW() failed. cmdline=%ws, gle=%u",
			cmdline,
			GetLastError()
			log_end;
		return false;
	}

	// Return process handle and close thread handle
	process_handle = pi.hProcess;
	process_id = pi.dwProcessId;

	CloseHandle(pi.hThread);

	log_dbg "Process created locally. cmdline=%ws, pid=%u", cmdline, process_id log_end;
	return true;
}

/// @brief Local process creation with wait
bool launch_process_local_and_wait(
	_In_z_ const wchar_t* cmdline,
	_In_ DWORD creation_flags,
	_In_opt_z_ const wchar_t* working_directory,
	_In_ DWORD timeout_secs,
	_Out_opt_ DWORD* exit_code
)
{
	HANDLE process_handle = nullptr;
	DWORD process_id = 0;

	if (!launch_process_local(
		cmdline,
		creation_flags,
		working_directory,
		process_handle,
		process_id))
	{
		log_err "launch_process_local() failed. cmdline=%ws", cmdline log_end;
		return false;
	}

	// Calculate timeout in milliseconds
	DWORD timeout_ms = (timeout_secs == INFINITE) ? INFINITE : (timeout_secs * 1000);

	// Wait for process to complete
	DWORD wait_result = WaitForSingleObject(process_handle, timeout_ms);
	if (wait_result != WAIT_OBJECT_0)
	{
		switch (wait_result)
		{
		case WAIT_ABANDONED:
			log_err "WaitForSingleObject() returned WAIT_ABANDONED. pid=%u, handle=0x%p",
				process_id, process_handle log_end;
			break;

		case WAIT_TIMEOUT:
			log_err "WaitForSingleObject() timeout. pid=%u, handle=0x%p, timeout=%u secs",
				process_id, process_handle, timeout_secs log_end;
			break;

		case WAIT_FAILED:
			log_err "WaitForSingleObject() failed. pid=%u, handle=0x%p, gle=%u",
				process_id, process_handle, GetLastError() log_end;
			break;

		default:
			log_err "WaitForSingleObject() unexpected result. pid=%u, result=%u",
				process_id, wait_result log_end;
			break;
		}

		// Terminate process on wait failure
		TerminateProcess(process_handle, 0xffffffff);
		CloseHandle(process_handle);
		return false;
	}

	// Get exit code
	if (exit_code != nullptr)
	{
		if (!GetExitCodeProcess(process_handle, exit_code))
		{
			log_err "GetExitCodeProcess() failed. gle=%u", GetLastError() log_end;
			*exit_code = 0xffffffff;
		}
	}

	CloseHandle(process_handle);

	log_dbg "Process completed. cmdline=%ws, pid=%u, exit_code=%u",
		cmdline, process_id, (exit_code ? *exit_code : 0) log_end;
	return true;
}
