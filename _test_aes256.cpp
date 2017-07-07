#include "stdafx.h"
#include "AirCrypto.h"

#include <stdint.h>

//���� ��������(��ȣȭ �Ǿ�����) �Է� �޾Ƽ� ��ȣȭ�ϰ� �а� Ű ����, Ű�� ���������� �ȵ�
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

	//��ȣȭ�� ���ϸ� �޸𸮿� �ε�
	if (LoadFileToMemory(encrypt_file_name, file_size, buffer))
	{
		//��ȣȭ
		AirCryptBuffer(key, strlen((const char*)key), buffer, file_size + 1, encrypt, length, FALSE);

		printf("%s", encrypt);

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

	//unsigned char origin_key[] = "xinfolab";
	PBYTE buffer = nullptr;
	DWORD file_size = 0;
	unsigned char *encrypt_data = nullptr;
	UINT32 length = 0;
	//�ӽ� �Է� ��, �ϵ��ڵ����� �׽�Ʈ
	unsigned char decrypt_data[] = { 0x82 ,0x34 ,0xae ,0xef ,0x57 ,0x74 ,0x70 ,0xe8 ,0x70 ,0x99 ,0x76 ,0x17 ,0x2b ,0x04 ,0x2a ,0xdb };

	//AirCryptBuffer(in_key, strlen((const char*)in_key), origin_key, strlen((const char*)origin_key) + 1, encrypt_data, length, TRUE);
	
	//unsigned char *test = encrypt_data;

	AirCryptBuffer(in_key, strlen((const char*)in_key), decrypt_data, strlen((const char*)decrypt_data) + 1, encrypt_data, length, FALSE);
	out_key	= encrypt_data;
	
	return TRUE;
}

bool test_aes256()
{
	wchar_t *path = (wchar_t*)malloc(sizeof(wchar_t)*MAX_PATH);
	wchar_t *target_file_path = L"ase256_test_file.conf";
	wchar_t * encrypt_file_name = L"ase256_test_file";
	wchar_t * decrypt_file_name = L"ase256_test_file_.txt";
//	unsigned char origin_key[] = "xinfolab"; //����Ű
	unsigned char *origin_key = nullptr;
	unsigned char second_key[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890*!=&?&/";
	unsigned char *encrypt = nullptr;

	//���� ���丮 Ȯ��
	if (GetCurrentDirectoryW(MAX_PATH, path) == 0)
	{
		log_err "GetCurrentDirectoryW() failed. gle = %u", GetLastError() log_end;
		return FALSE;
	}
	log_info "%ws", path log_end;

	//�׽�Ʈ ���� ���� Ȯ��
	//if (!is_file_existsW(target_file_path))
	//{
	//	log_info "no search file!!" log_end;
	//	return FALSE;
	//}

	//�׽�Ʈ �ڵ忩�� ��ȣȭ ���� ����
	//��ȣȭ ���� ���
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

	//�̹� ��ȣȭ ������ �����ϸ� ����
	//if (is_file_existsW(encrypt_file))
	//{
	//	log_err "same file exits, delete file!!" log_end;
	//	DeleteFileW(encrypt_file);
	//}


	key_recover(second_key, origin_key);
	
	int64_t padding = 0;
	file_size_padding(target_file_path, padding);

	//aes256 ��ȣȭ
	if (!aes256_encrypt(origin_key, path, target_file_path, encrypt_file_name, encrypt))
	{
		log_err "aes256_encrypt err" log_end;
	}

	//aes256 ��ȣȭ
	if(!aes256_decrypt(origin_key, path, target_file_path, encrypt_file_name, decrypt_file_name, encrypt))
	{
		log_err "aes256_encrypt err" log_end;
	}

	if (!aes256_read_only(origin_key, path, encrypt_file_name, encrypt))
	{
		log_err "aes256_read_only err" log_end;
	}

	free(path);

	return TRUE;
}
