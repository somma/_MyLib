/**
 * @file    account_info.h
 * @brief   
 * @ref     
 * @author  Taehong, Kim (taehong@somma.kr)
 * @date    2017/12/12 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once
#include <string>
#include <list>

#include <sal.h>
#include <minwindef.h>
#include <LMaccess.h>


/// @brief account 정보
typedef class account
{
public:
	account(_In_ const wchar_t* name,
			_In_ DWORD password_age,
			_In_ DWORD privilege,
			_In_ DWORD flags,
			_In_ const wchar_t* script_path,
			_In_ DWORD last_logon,
			_In_ DWORD num_logons,
			_In_ const wchar_t* sid);

public:
	// 계정명
	std::wstring name() { return _name; }

	// 마지막 패스워드 변경된 날짜부터 현재 까지의 시간(초)
	DWORD password_age() { return _password_age; }

	// 계정 타입(Administrator, User, Guest)
	const wchar_t* privilege() const;

	// 계정 속성
	std::wstring attribute_to_string();

	// 사용자 로그온시 동작하는 스크립트의 경로
	std::wstring script_path() { return _script_path; };

	// 마지막 로그인 시간
	DWORD last_logon_timestamp() { return _last_logon; }

	// 로그인 성공 횟수
	DWORD num_logons() { return _num_logons; }

	// 계정별 고유 ID
	std::wstring sid() { return _sid; }

private:
	std::wstring _name;
	DWORD		 _privilege;
	DWORD		 _password_age;
	DWORD		 _flags;
	std::wstring _script_path;
	DWORD		 _last_logon;
	DWORD		 _num_logons;
	std::wstring _sid;
} *paccount;




/// @brief `PSID`를 SID 문자열로 변환 한다.
bool 
psid_to_wstr_sid(
	_In_ PSID sid, 
	_Out_ std::wstring& sid_string
	);

/// @brief  사용자 이름을 가지고 사용자 정보를 조회 한다.
bool
get_account_info_by_name(
	_In_ wchar_t* user_name,
	_Out_ LPUSER_INFO_4* user_info
	);

/// @brief  시스템의 모든 계정 정보를 읽어 온다. 
bool 
get_account_infos(
	_Out_ std::list<paccount>& accounts
	);