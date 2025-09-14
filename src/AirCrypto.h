/**----------------------------------------------------------------------------
 * AIRCrypto.h
 *-----------------------------------------------------------------------------
 * OpenSSL 라이브러리를 이용한 암호화 모듈
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 20:9:2011   15:22 created
**---------------------------------------------------------------------------*/
#pragma once

#include <string>

bool 
aes256_encrypt_file(
	_In_ const unsigned char* key,
	_In_ const std::wstring& target_file_path,
	_In_ const std::wstring& encrypt_file_path
	);

bool 
aes256_decrypt_file(
	_In_ const unsigned char* key,
	_In_ const std::wstring& encrypt_file_path,
	_In_ const std::wstring& decrypt_file_path
	);

/// @brief AES256으로 메모리 버퍼를 암호화/복호화함 (보안 강화 버전)
/// @param _In_ PassPhrase 암호화 키 (패스프레이즈)
/// @param _In_ PassPhraseLen 패스프레이즈 길이
/// @param _In_ Input 입력 데이터 버퍼
/// @param _In_ InputLength 입력 데이터 크기
/// @param _Outptr_ Output 출력 데이터 버퍼 (malloc으로 할당됨, 호출자가 free() 해야 함)
/// @param _Out_ OutputLength 출력 데이터 크기
/// @param _In_ Encrypt true=암호화, false=복호화
/// @return 성공 시 true, 실패 시 false
/// @warning Output 포인터는 malloc()으로 할당되므로 호출자가 반드시 free()로 해제해야 함
/// @note SHA-256, salt, 강화된 라운드를 사용하여 보안성이 향상됨 (기존 파일과 호환되지 않음)
bool 
aes256_crypt_buffer_v2(
	_In_ const unsigned char* PassPhrase,
	_In_ const uint32_t PassPhraseLen,
	_In_ const unsigned char* Input, 
	_In_ const uint32_t InputLength,
	_Outptr_ unsigned char*& Output,
	_Out_ uint32_t& OutputLength,
	_In_ bool Encrypt
	);