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

#include <intsafe.h>
#include "StatusCode.h"

DTSTATUS 
AirCryptBuffer(
	IN unsigned char* PassPhrase,
	IN UINT32 PassPhraseLen,
	IN unsigned char* Input, 
	IN UINT32 InputLength, 
	OUT unsigned char*& Output, 
	OUT UINT32& OutputLength, 
	IN BOOL Encrypt
	);