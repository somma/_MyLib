#include "stdafx.h"
#include "AirCrypto.h"

//전송 설정 파일(암호화 되어있음) 입력 받아서 복호화하고 읽고 키 지움, 키가 남아있으면 안됨
bool aes256_read_only(
	IN unsigned char *key,
	IN const wchar_t *path,
	IN const wchar_t *encrypt_file_name,
	IN  unsigned char * encrypt
)
{
	_ASSERTE(NULL != key);
	_ASSERTE(NULL != path);
	_ASSERTE(NULL != encrypt_file_name);
	_ASSERTE(NULL == encrypt);
	if (NULL == key || NULL == path || NULL == encrypt_file_name || NULL != encrypt)
		return FALSE;

	PBYTE buffer = nullptr;
	DWORD file_size = 0;
	UINT32 length = 0;

	//복호화할 파일를 메모리에 로드
	if (LoadFileToMemory(encrypt_file_name, file_size, buffer))
	{
		//복호화
		AirCryptBuffer(key, strlen((const char*)key), buffer, file_size, encrypt, length, FALSE);
		//파일 사이즈만큼 읽고 문자열 출력
		char *read = (char*)malloc(sizeof(char)*length+1);
		RtlZeroMemory(read, sizeof(char)*length + 1);
		RtlCopyMemory(read, encrypt, length);
		printf("%s\n", read);
		free(read);

		RtlSecureZeroMemory(key, sizeof(key));
		free(buffer);
		free(encrypt);
	}
	else
	{
		log_err "decrypt failed!!!" log_end;
		return FALSE;
	}

	return TRUE;
}


bool key_recover(
	IN unsigned char *in_key,
	OUT unsigned char *&out_key
)
{
	_ASSERTE(NULL != in_key);
	_ASSERTE(NULL == out_key);
	if (NULL == in_key || NULL != out_key)
		return FALSE;

	//unsigned char origin_key[] = "xinfolab"; //하드 코딩으로 암호화 되어 있음
	PBYTE buffer = nullptr;
	DWORD file_size = 0;
	unsigned char *encrypt_data = nullptr;
	UINT32 length = 0;
	//임시 입력 값, 하드코딩으로 테스트
	unsigned char decrypt_data[] = { 0x82 ,0x34 ,0xae ,0xef ,0x57 ,0x74 ,0x70 ,0xe8 ,0x70 ,0x99 ,0x76 ,0x17 ,0x2b ,0x04 ,0x2a ,0xdb };

	//원본키 암호화시킨 뒤 값 확인용
	//AirCryptBuffer(in_key, strlen((const char*)in_key), origin_key, strlen((const char*)origin_key) + 1, encrypt_data, length, TRUE);
	//unsigned char *test = encrypt_data;

	AirCryptBuffer(in_key, strlen((const char*)in_key), decrypt_data, strlen((const char*)decrypt_data) + 1, encrypt_data, length, FALSE);

	out_key	= encrypt_data;
	
	return TRUE;
}

bool test_aes256()
{
	wchar_t *path = (wchar_t*)malloc(sizeof(wchar_t)*MAX_PATH);
	wchar_t *target_file_path = L"ase256_test_before.conf";
	wchar_t * encrypt_file_name = L"ase256_test_file";
	wchar_t * decrypt_file_name = L"ase256_test_after.conf";
//	unsigned char origin_key[] = "xinfolab";	//원본키
	unsigned char *origin_key = nullptr;		//key_recover에서 키가 복호화해서 나오기 때문에 NULL,
	unsigned char second_key[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890*!=&?&/";
	unsigned char *encrypt = nullptr;

	//현재 디렉토리 확인
	if (GetCurrentDirectoryW(MAX_PATH, path) == 0)
	{
		log_err "GetCurrentDirectoryW() failed. gle = %u", GetLastError() log_end;
		return FALSE;
	}	

	//테스트 파일 존재 확인
	if (!is_file_existsW(target_file_path))
	{
		log_info "no search file!!" log_end;
		return FALSE;
	}

	//암호화 파일 경로
	WCHAR encrypt_file[MAX_PATH + 1] = { 0 };
	if (TRUE != SUCCEEDED(StringCbPrintfW(
		encrypt_file,
		sizeof(encrypt_file),
		L"%s\\%s",
		path,
		encrypt_file_name)))
	{
		log_err "Can not generate target path, dir=%s, file=%s" log_end;
		return FALSE;
	}

	//이미 암호화 파일이 존재하면 삭제
	if (is_file_existsW(encrypt_file))
	{
		log_err "same file exits, delete file!!" log_end;
		DeleteFileW(encrypt_file);
	}

	key_recover(second_key, origin_key);

	//aes256 암호화
	if (!aes256_encrypt(origin_key, path, target_file_path, encrypt_file_name))
	{
		log_err "aes256_encrypt err" log_end;
		free(path);
		return FALSE;
	}

	//aes256 복호화
	if (!aes256_decrypt(origin_key, path, encrypt_file_name, decrypt_file_name))
	{
		log_err "aes256_encrypt err" log_end;
		free(encrypt);
		free(path);
		return FALSE;
	}

	if (!aes256_read_only(origin_key, path, encrypt_file_name, encrypt))
	{
		log_err "aes256_read_only err" log_end;
		free(path);
		return FALSE;
	}

	free(origin_key);
	free(path);

	return TRUE;
}
