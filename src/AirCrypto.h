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
aes256_encrypt(
	_In_ const unsigned char* key,
	_In_ const std::wstring& target_file_path,
	_In_ const std::wstring& encrypt_file_path
	);

bool 
aes256_decrypt(
	_In_ const unsigned char* key,
	_In_ const std::wstring& encrypt_file_path,
	_In_ const std::wstring& decrypt_file_path
	);

bool 
AirCryptBuffer(
	_In_ const unsigned char* PassPhrase,
	_In_ const uint32_t PassPhraseLen,
	_In_ const unsigned char* Input, 
	_In_ const uint32_t InputLength,
	_Outptr_ unsigned char*& Output,
	_Out_ uint32_t& OutputLength,
	_In_ bool Encrypt
	);