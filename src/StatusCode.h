/*-----------------------------------------------------------------------------
 * StatusCode.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * - 2011.01.12 Created
**---------------------------------------------------------------------------*/

#pragma once

typedef unsigned long DTSTATUS;

#define     DTS_SUCCESS                 0x00000000
#define     DT_SUCCEEDED(status)        ((DTSTATUS)(status) == DTS_SUCCESS)

#define DTCHECKS   if (TRUE != DT_SUCCEEDED(status)){
#define DTCHECKE   }


#define		DTS_GENERAL_ERROR			0x00000001
#define		DTS_INVALID_PARAMETER		DTS_GENERAL_ERROR + 1
#define		DTS_INVALID_OBJECT_STATUS	DTS_GENERAL_ERROR + 2
#define		DTS_WINAPI_FAILED			DTS_GENERAL_ERROR + 3		// 요청한 동작이 OS 에서 동작하지 않습니다.
#define     DTS_MORE_BUFFER_NEEDED      DTS_GENERAL_ERROR + 4
#define		DTS_INSUFFICIENT_RESOURCES	DTS_GENERAL_ERROR + 5
#define		DTS_EXCEPTION_RAISED		DTS_GENERAL_ERROR + 6

#define		DTS_NO_CONFIG_FILE			DTS_GENERAL_ERROR + 6		// 설정 파일이 존재하지 않습니다.
#define		DTS_INVALID_CONFIG_FILE		DTS_GENERAL_ERROR + 7		// 설정 파일에 오류가 있습니다.
#define		DTS_INVALID_FILE_PATH		DTS_GENERAL_ERROR + 8		// 유효하지 않는 파일 경로입니다.
#define		DTS_NO_FILE_EXIST			DTS_GENERAL_ERROR + 9		// 파일이 존재하지 않습니다.

#define		DTS_NO_ARCIVE_FILE			DTS_GENERAL_ERROR + 10		// 아카이브 파일이 존재하지 않습니다.
#define		DTS_NO_ITEM_FOUND			DTS_GENERAL_ERROR + 11		// 요청한 요소가 존재하지 않습니다.


// OpenSSL error
// 
#define		DTS_CRYPTO_ERROR_BASE		DTS_GENERAL_ERROR + 256
#define		DTS_OPENSSL_ERROR			DTS_CRYPTO_ERROR_BASE + 1
#define		DTS_OPENSSL_PRNG_FAIL		DTS_CRYPTO_ERROR_BASE + 2	// 난수 생성에 실패했습니다.
#define		DTS_OPENSSL_KEYGEN_FAIL		DTS_CRYPTO_ERROR_BASE + 3	// 키 생성에 실패했습니다.

// UDT / Network 에러
#define		DTS_NET_ERROR_BASE			DTS_GENERAL_ERROR + 512
#define		DTS_NET_API_FAILED			DTS_NET_ERROR_BASE + 1		// network 관련 api 오류입니다.
#define		DTS_NET_CONNECT_FAILED		DTS_NET_ERROR_BASE + 2		// 서버에 접속 할 수 없습니다. 
#define		DTS_NET_SEND_FAILED			DTS_NET_ERROR_BASE + 3		// 데이터 전송 중 오류가 발생했습니다.
#define		DTS_NET_RECV_FAILED			DTS_NET_ERROR_BASE + 4		// 데이터 수신 중 오류가 발생했습니다.


// DATA PARSER
// 
#define		DTS_PARSER_ERROR_BASE       DTS_GENERAL_ERROR + 768
#define     DTS_INVALID_DATA_FORMAT     DTS_PARSER_ERROR_BASE + 1   // 데이터의 포맷이 유효하지 않습니다.


// DATABASE
// 
#define		DTS_DATABASE_BASE			DTS_GENERAL_ERROR + 1024
#define		DTS_DB_OPEN_FAILED			DTS_DATABASE_BASE + 1		// 데이터베이스를 열 수 없습니다. 
#define		DTS_DB_EXEC_FAILED			DTS_DATABASE_BASE + 2		// 데이터베이스를 질의어 수행중 오류가 발생했습니다.


#define		DTS_NOT_IMPLEMENTED			0xFFFFFFFF