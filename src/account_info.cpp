/**
 * @file    account_info.h
 * @brief   
 * @ref     
 * @author  Taehong, Kim (taehong@somma.kr)
 * @date    2017/12/12 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "Win32Utils.h"
#include <sddl.h>
#include "account_info.h"

#include <LM.h>
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "netapi32.lib")


/// @brief	
account::account(_In_ const wchar_t * name, 
				 _In_ DWORD password_age,
				 _In_ DWORD privilege,
				 _In_ DWORD flags,
				 _In_ const wchar_t * script_path,
				 _In_ DWORD last_logon,
				 _In_ DWORD num_logons,
				 _In_ const wchar_t * sid) :
	_name(nullptr == name ? L"" : name),
	_password_age(password_age),
	_privilege(privilege),
	_flags(flags),
	_script_path(script_path),
	_last_logon(last_logon),
	_num_logons(num_logons),
	_sid(sid)
{}

/// @brief	계정 타입(Administrator, User, Guest)
const wchar_t* account::privilege() const 
{
	switch (_privilege)
	{
	case USER_PRIV_ADMIN: return L"Administrator";
	case USER_PRIV_USER: return L"User";
	case USER_PRIV_GUEST: return L"Guest";
	default:
		_ASSERTE(!"Never reach here");
		return L"Unknown";
	}
}

/// @brief	계정 속성
void account::dump_account_flags()
{
	char buf[256];
	char* pos = buf;
	size_t remain = sizeof(buf);
	bool add_lf = false;

	if (FlagOn(_flags, UF_SCRIPT))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  "%s",
						  "UF_SCRIPT");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_ACCOUNTDISABLE))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_ACCOUNTDISABLE");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_HOMEDIR_REQUIRED))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_HOMEDIR_REQUIRED");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_PASSWD_NOTREQD))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_PASSWD_NOTREQD");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_PASSWD_CANT_CHANGE))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_PASSWD_CANT_CHANGE");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_DONT_EXPIRE_PASSWD))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_DONT_EXPIRE_PASSWD");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_NOT_DELEGATED))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_NOT_DELEGATED");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_SMARTCARD_REQUIRED))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_SMARTCARD_REQUIRED");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_USE_DES_KEY_ONLY))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_USE_DES_KEY_ONLY");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_DONT_REQUIRE_PREAUTH))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_DONT_REQUIRE_PREAUTH");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_TRUSTED_FOR_DELEGATION))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_TRUSTED_FOR_DELEGATION");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_PASSWORD_EXPIRED))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_PASSWORD_EXPIRED");
		add_lf = true;
	}
	if (FlagOn(_flags, UF_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION))
	{
		StringCbPrintfExA(pos,
						  remain,
						  &pos,
						  &remain,
						  0,
						  (true == add_lf) ? ", %s" : "%s",
						  "UF_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION");
		add_lf = true;
	}


	if (add_lf == true)
	{
		log_info "options=%s", buf log_end;
	}
	else
	{
		log_info "options=None" log_end;
	}
}

/// @brief `PSID`를 문자열로 변환 한다. 
bool
psid_to_wstr_sid(
	_In_ PSID sid,
	_Out_ std::wstring& sid_string
	)
{
	_ASSERTE(nullptr != sid);
	
	sid_string = L"";
	if (nullptr == sid) return false;

	//
	//	ConvertSidToStringSidW() 가 리턴한 버퍼는 반드시 LocalFree 로
	//	소멸해야 한다.
	//
	wchar_t* sid_buf = nullptr;	
	wchar_ptr sid_ptr(sid_buf, [](wchar_t* p) 
	{ 
		if (nullptr != p) LocalFree(p); 
	});

	if (TRUE != ConvertSidToStringSidW(sid, &sid_buf))
	{
		DWORD gle = GetLastError();
		const char* gles = nullptr;
		switch (gle)
		{
		case ERROR_NOT_ENOUGH_MEMORY: gles = "ERROR_NOT_ENOUGH_MEMORY"; break;
		case ERROR_INVALID_SID:		  gles = "ERROR_INVALID_SID"; break;
		case ERROR_INVALID_PARAMETER: gles = "ERROR_INVALID_PARAMETER"; break;
		}

		if (nullptr != gles)
		{
			log_err "ConvertSidToStringSidW() failed. gle=%s",
				gles
				log_end;
			return false;
		}
		else
		{
			log_err "ConvertSidToStringSidW() failed. gle=%u",
				gle
				log_end;
			return false;
		}
	}

	sid_string = sid_buf;
	return true;
}

/// @brief  계정 이름을 가지고 계정 정보를 조회해서 반환한다.
bool
get_account_info_by_name(
	_In_ wchar_t* user_name,
	_Out_ LPUSER_INFO_4* user_info
	)
{
	_ASSERTE(nullptr != user_name);
	if (nullptr == user_name) return false;

	//
	// NetUserGetInfo 함수에서 `infomation level`에 따라 읽어 올 수 있는
	// 사용자 정보가 다르다. 사용자의 상세 정보를 읽어 오는건 3 또는 4레벨 
	// 이면 충분 하고, MSDN에 의하면 level 3보다는 4를 사용하는걸 권장 하고
	// 있다.
	// 참고 URL : https://msdn.microsoft.com/ko-kr/library/windows/desktop/aa370654(v=vs.85).aspx
	//
	DWORD status = NetUserGetInfo(NULL,
								  user_name,
								  4,
								  (LPBYTE*)user_info);

	if (NERR_Success != status)
	{
		const char* status_str = nullptr;
		switch (status)
		{
		case ERROR_ACCESS_DENIED:  status_str = "ERROR_ACCESS_DENIED"; break;
		case ERROR_BAD_NETPATH:    status_str = "ERROR_BAD_NETPATH"; break;
		case ERROR_INVALID_LEVEL:  status_str = "ERROR_INVALID_LEVEL"; break;
		case NERR_InvalidComputer: status_str = "NERR_InvalidComputer"; break;
		case NERR_UserNotFound:    status_str = "NERR_UserNotFound"; break;
		}

		if (nullptr != status_str)
		{
			log_err
				"GetLocalAccountInfos::NetUserGetInfo failed. status=%s",
				status_str
				log_end;
		}
		{
			log_err
				"GetLocalAccountInfos::NetUserGetInfo failed. status=%d",
				status
				log_end;
		}

		return false;
	}

	return true;
}

/// @brief  시스템의 모든 계정 정보를 읽어 온다.
bool
get_account_infos(
	_Out_ std::list<paccount>& accounts
	)
{
	uint8_t* buffer = nullptr;
	DWORD entries_read, total_entries;
	DWORD resume_handle = 0;
	NET_API_STATUS ret;
	do
	{
		//
		// 시스템의 모든 계정 정보를 읽는다. 이때 계정 정보 관련 레벨은 `level` 
		// 인자값으로 조절 가능하며 계정 정보를 iterate할 때 사용자 계정 타입별
		// 필터를 줄 수 있다. 현재 필요한 사용자 목록은 윈도우 설치시 설정 되어 있는 계정 
		// 과 사용자가 생성한 계정만 읽어 오도록(`FILTER_NORMAL_ACCOUNT`) 되어 있다.
		// 참고 : `Active Directory`환경에서 프로그램이 동작 할 때 ACL에 의해서
		//       `NetUserEnum`을 이용한 계정 조회가 허용 되거나 거부 될 수 있다.
		//       ACL 기본 정책은 허용된 사용자 그리고 `Pre-Windows 2000 compat-
		//       ible access` 그룹에 속한 계정으로 프로그램이 실행 된다면 계정 조
		//       회에 문제가 없다.
		// 참고 : https://msdn.microsoft.com/ko-kr/library/windows/desktop/aa370652(v=vs.85).aspx
		//
		ret = NetUserEnum(NULL,
						  0,
						  FILTER_NORMAL_ACCOUNT,
						  &buffer,
						  MAX_PREFERRED_LENGTH,
						  &entries_read,
						  &total_entries,
						  &resume_handle);
		if ((NERR_Success != ret) && (ERROR_MORE_DATA != ret))
		{
			log_err
				"NetUserEnum Error. NetApiStatus=%u",
				ret
				log_end;
			break;
		}
		else
		{
			USER_INFO_0* user_info_0 = (USER_INFO_0*)buffer;
			//
			// 시스템에 있는 계정을 iterate하면서 계정 정보를 가져온다.
			//
			for (DWORD count = 0; count < total_entries; count++)
			{
				//
				// 계정명을 이용해서 계정에 대한 상세 정보를 획득한다.
				//
				LPUSER_INFO_4 user_info = NULL;
				if (!get_account_info_by_name(user_info_0->usri0_name,
											  &user_info))
				{
					log_err
						"get_user_info_by_name failed. name(%ws)",
						user_info_0->usri0_name
						log_end;
					user_info_0++;
					continue;
				}

				//
				// psid를 문자열 sid로 변환을 한다.
				// 
				std::wstring sid_str;
				psid_to_wstr_sid(user_info->usri4_user_sid, sid_str);
				paccount ac = new account(user_info->usri4_name,
										  user_info->usri4_password_age,
										  user_info->usri4_priv,
										  user_info->usri4_flags,
										  user_info->usri4_script_path,
										  user_info->usri4_last_logon,
										  user_info->usri4_num_logons,
										  sid_str.c_str());
				accounts.push_back(ac);

				if (user_info) { NetApiBufferFree(user_info); user_info = nullptr; }

				user_info_0++;
			}
		}
		if (buffer) { NetApiBufferFree(buffer); buffer = nullptr; }
	} while (ERROR_MORE_DATA == ret);

	if (buffer) { NetApiBufferFree(buffer); buffer = nullptr; }
	return true;
}