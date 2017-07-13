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

#include "StatusCode.h"

bool 
aes256_encrypt(
	_In_ unsigned char *key,
	_In_ std::wstring& target_file_path,
	_In_ std::wstring& encrypt_file_path
	);

bool 
aes256_decrypt(
	_In_ unsigned char *key,
	_In_ std::wstring& encrypt_file_path,
	_In_ std::wstring& decrypt_file_path
);

DTSTATUS 
AirCryptBuffer(
	_In_ unsigned char* PassPhrase,
	_In_ UINT32 PassPhraseLen,
	_In_ unsigned char* Input, 
	_In_ UINT32 InputLength, 
	_Out_ unsigned char*& Output, 
	_Out_ UINT32& OutputLength, 
	_In_ BOOL Encrypt
	);