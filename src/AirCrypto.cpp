/**----------------------------------------------------------------------------
 * AirCrypto.cpp
 *-----------------------------------------------------------------------------
 * AES encryption/decryption demo program using OpenSSL EVP apis
 * gcc -Wall openssl_aes.c -lcrypto
 *
 *  this is public domain code.
 *
 *  Saju Pillai (saju.pillai@gmail.com)
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 20:9:2011   15:35 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include <crtdbg.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include "_MyLib/src/log.h"
#include "_MyLib/src/Win32Utils.h"
#include "_MyLib/src/AirCrypto.h"

#pragma comment(lib, "libcrypto.lib")

/**
* @brief	aes256 파일 암호화
* @param 	target_file_path ex) C:\\test_folder\\a.txt
*			encrypt_file_path ex) C:\\test_folder\\a.txt.crypto
**/
bool 
aes256_encrypt_file(
	_In_ const unsigned char* key,
	_In_ const std::wstring& target_file_path,
	_In_ const std::wstring& encrypt_file_path
)
{
	_ASSERTE(nullptr != key);
	_ASSERTE(nullptr != target_file_path.c_str());
	_ASSERTE(nullptr != encrypt_file_path.c_str());
	_ASSERTE(0 != target_file_path.compare(encrypt_file_path));
	if (nullptr == key ||
		nullptr == target_file_path.c_str() ||
		nullptr == encrypt_file_path.c_str() ||
		0 == target_file_path.compare(encrypt_file_path))
	{
		return false;
	}

	DWORD file_size = 0;	
	std::wstring encrypt_file_name;
	if (!extract_last_tokenW(const_cast<std::wstring&>(encrypt_file_path),
							 L"\\",
							 encrypt_file_name,
							 false,
							 false))
	{
		log_err
			"extract_last_tokenW(%ws) failed.",
			encrypt_file_path.c_str()
			log_end;
		return false;
	}

	std::wstring encrypt_file_directory;
	if (!extract_last_tokenW(const_cast<std::wstring&>(encrypt_file_path),
							 L"\\",
							 encrypt_file_directory,
							 true,
							 false))
	{
		log_err "extract_last_tokenW(path=%ws) failed.",
			encrypt_file_path.c_str()
			log_end;
		return false;
	}


	PBYTE buffer = nullptr;	
	if (!LoadFileToMemory(target_file_path.c_str(), file_size, buffer))
	{
		log_err "LoadFileToMemory() failed. path=%ws", target_file_path.c_str() log_end;
		return false;
	}
	_ASSERTE(nullptr != buffer);
	if (nullptr == buffer) return false;
	char_ptr buffer_ptr((char *)buffer, [](char* ptr) { free(ptr); });

	uint32_t length = 0;
	unsigned char* encrypt_data = nullptr;	
    if (!aes256_crypt_buffer_v2(const_cast<unsigned char*>(key),
                                (uint32_t)strlen((const char*)key),
                                buffer,
                                file_size,
                                encrypt_data,
                                length,
                                true))
	{
		log_err "aes256_crypt_buffer() failed." log_end;
		return false;
	}
	_ASSERTE(nullptr != encrypt_data);
	if (nullptr == encrypt_data) return false;
	char_ptr edata_ptr((char*)encrypt_data, [](char* ptr) { free(ptr); });

	if (!SaveBinaryFile(encrypt_file_directory.c_str(),
						encrypt_file_name.c_str(),
						length,
						encrypt_data))
	{
		log_err "SaveBinaryFile() failed." log_end;
		return false;
	}	

	return true;
}

/**
* @brief	aes256 파일 복호화
* @param 	encrypt_file_path ex) C:\\test_folder\\a.txt.crypto
*			decrypt_file_path ex) C:\\test_folder\\a.txt
**/
bool 
aes256_decrypt_file(
	_In_ const unsigned char* key,
	_In_ const std::wstring& encrypt_file_path,
	_In_ const std::wstring& decrypt_file_path
	)
{
	_ASSERTE(nullptr != key);
	_ASSERTE(nullptr != encrypt_file_path.c_str());
	_ASSERTE(nullptr != decrypt_file_path.c_str());
	_ASSERTE(0 != encrypt_file_path.compare(decrypt_file_path));
	if (nullptr == key ||
		nullptr == decrypt_file_path.c_str() ||
		nullptr == encrypt_file_path.c_str() ||
		0 == encrypt_file_path.compare(decrypt_file_path))
	{
		return false;
	}

	PBYTE buffer = nullptr;
	DWORD file_size = 0;
	uint32_t length = 0;
	unsigned char* encrypt_data = nullptr;

	std::wstring decrypt_file_name;
	if(!extract_last_tokenW(const_cast<std::wstring&>(decrypt_file_path),
							L"\\", 
							decrypt_file_name, 
							false, 
							false))
	{
		log_err "extract_last_tokenW(path=%ws) failed.", 
			decrypt_file_path.c_str() 
			log_end;
		return false;
	}
	std::wstring decrypt_file_directory;
	if(!extract_last_tokenW(const_cast<std::wstring&>(decrypt_file_path),
							L"\\", 
							decrypt_file_directory, 
							true, 
							false))
	{
		log_err "extract_last_tokenW(path=%ws) failed.", 
			decrypt_file_path.c_str()
			log_end;
		return false;
	}

	if (LoadFileToMemory(encrypt_file_path.c_str(), file_size, buffer))
	{
		// RAII를 위한 버퍼 가드
		auto buffer_guard = [](void* p) { free(p); };
		std::unique_ptr<void, decltype(buffer_guard)> smart_buffer(buffer, buffer_guard);

        if (!aes256_crypt_buffer_v2(const_cast<unsigned char*>(key),
                                    (uint32_t)strlen((char*)key),
                                    buffer,
                                    file_size,
                                    encrypt_data,
                                    length,
                                    false))
		{
			log_err "aes256_crypt_buffer() failed." log_end;
			return false;
			// buffer는 smart_buffer 소멸자에서 자동으로 해제됨
		}

		// RAII를 위한 encrypt_data 가드
		auto encrypt_data_guard = [](void* p) { free(p); };
		std::unique_ptr<void, decltype(encrypt_data_guard)> smart_encrypt_data(encrypt_data, encrypt_data_guard);

		if (!SaveBinaryFile(decrypt_file_directory.c_str(),
							decrypt_file_name.c_str(),
							length,  
							encrypt_data))
		{
			log_err "SaveBinaryFile() failed." log_end;
			return false;			
		}
	}
	else
	{
		log_err "LoadFileToMemory err" log_end;
		return false;
	}

	return true;
}


/// @brief 보안이 강화된 새로운 AES 초기화 함수 (SHA-256, salt 사용)
bool 
aes_init_v2(
	_In_ const unsigned char* key_data, 
	_In_ const int key_data_len, 
	_Outptr_ EVP_CIPHER_CTX* ctx, 
	bool encrypt
	)
{
	unsigned char key[EVP_MAX_KEY_LENGTH]={0};
	unsigned char iv[EVP_MAX_IV_LENGTH]={0};

	// 고정 salt 사용
	static const unsigned char salt[8] = {0x41, 0x69, 0x72, 0x43, 0x72, 0x79, 0x70, 0x74};

	/*
	 * Gen key & IV for AES 256 CBC mode. A SHA-256 digest is used to hash the supplied key material.
	 * nrounds is the number of times the we hash the material. More rounds are more secure but
	 * slower.
	 */
	int ret = EVP_BytesToKey(EVP_aes_256_cbc(),
							 EVP_sha256(),
							 salt,
							 key_data,
							 key_data_len,
							 10,
							 key,
							 iv);
	if (ret != 32) 
	{
		log_err "Key size is %d bits - should be 256 bits", ret log_end;
		return false;
	}

	if (true == encrypt)
	{
		if (!EVP_EncryptInit_ex(ctx,
						        EVP_aes_256_cbc(),
						        NULL, 
						        key, 
						        iv))
		{
			log_err "EVP_EncryptInit_ex() failed" log_end;
			return false;
		}
	}
	else
	{
		if (!EVP_DecryptInit_ex(ctx, 
						        EVP_aes_256_cbc(),
						        NULL, 
						        key, 
						        iv))
		{
			log_err "EVP_DecryptInit_ex() failed" log_end;
			return false;
		}
	}

	return true;
}

/**----------------------------------------------------------------------------
    \brief  
				Encrypt *len bytes of data    
				All data going in & out is considered binary (unsigned char[])      
-----------------------------------------------------------------------------*/
unsigned char*
aes_encrypt(
	_In_ EVP_CIPHER_CTX* e, 
	_In_ const unsigned char* plaintext, 
	_Inout_  int* len
	)
{
	int BlockSize = EVP_CIPHER_CTX_block_size(e);
	
	int c_len = *len + BlockSize;
	int f_len = 0;
		
	unsigned char* ciphertext = (unsigned char*)malloc(c_len);
	if (nullptr == ciphertext)
	{
		log_err "malloc() failed for ciphertext buffer" log_end;
		return nullptr;
	}	
	auto ciphertext_guard = [](void* p) { free(p); };
	std::unique_ptr<void, decltype(ciphertext_guard)> smart_ciphertext(ciphertext, ciphertext_guard);

	/* allows reusing of 'e' for multiple encryption cycles */
	//	EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

	/* update ciphertext, c_len is filled with the length of ciphertext generated,
     * len is the size of plaintext in bytes 
	 */
	if (!EVP_EncryptUpdate(e, 
					       ciphertext, 
					       &c_len, 
					       plaintext, 
					       *len))
	{
		log_err "EVP_EncryptUpdate() failed" log_end;
		return nullptr;		
	}

	/* update ciphertext with the final remaining bytes */
	if (!EVP_EncryptFinal_ex(e, 
						     ciphertext+c_len, 
						     &f_len))
	{
		log_err "EVP_EncryptFinal_ex() failed" log_end;
		return nullptr;		
	}

	*len = c_len + f_len;
	
	// 소유권을 호출자에게 전달 (release 호출로 가드 해제)
	smart_ciphertext.release();
	return ciphertext;
}

/**----------------------------------------------------------------------------
    \brief  Decrypt *len bytes of ciphertext     
-----------------------------------------------------------------------------*/
unsigned char*
aes_decrypt(
	_In_ EVP_CIPHER_CTX* e, 
	_In_ const unsigned char* ciphertext, 
	_Inout_ int* len
	)
{
	/* plaintext will always be equal to or lesser than length of ciphertext*/
	int p_len = *len, f_len = 0;
	
	// malloc 사용하되 RAII 패턴 적용
	unsigned char* plaintext = (unsigned char*)malloc(p_len);
	if (nullptr == plaintext)
	{
		log_err "malloc() failed for plaintext buffer" log_end;
		return nullptr;
	}
	
	// RAII를 위한 스마트 포인터 가드
	auto plaintext_guard = [](void* p) { free(p); };
	std::unique_ptr<void, decltype(plaintext_guard)> smart_plaintext(plaintext, plaintext_guard);

	//  EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
	if (!EVP_DecryptUpdate(e,
					       plaintext,
					       &p_len,
					       ciphertext,
					       *len))
	{
		log_err "EVP_DecryptUpdate() failed" log_end;
		return nullptr;
	}

	if (!EVP_DecryptFinal_ex(e,
						     plaintext + p_len,
						     &f_len))
	{
		log_err "EVP_DecryptFinal_ex() failed" log_end;
		return nullptr;
	}

	*len = p_len + f_len;
	
	smart_plaintext.release();
	return plaintext;
}

/// @brief 보안이 강화된 AES256 버퍼 암호화/복호화 함수
bool 
aes256_crypt_buffer_v2(
	_In_ const unsigned char* PassPhrase,
	_In_ const uint32_t PassPhraseLen,
	_In_ const unsigned char* Input, 
	_In_ const uint32_t InputLength,
	_Outptr_ unsigned char*& Output,
	_Out_ uint32_t& OutputLength,
	_In_ bool Encrypt
	)
{
	_ASSERTE(nullptr != PassPhrase);
	_ASSERTE(0 < PassPhraseLen);
	_ASSERTE(nullptr != Input);
	_ASSERTE(0 < InputLength);
	
	if (nullptr == PassPhrase || 
		0 >= PassPhraseLen || 
		nullptr == Input ||
		0 >= InputLength) 
	{
		log_err "aes256_crypt_buffer_v2() invalid parameters" log_end;
		return false;
	}

	Output = nullptr;
	OutputLength = 0;

	/* "opaque" encryption, decryption ctx structures that libcrypto uses to record
     * status of enc/dec operations 
	 */
	auto ctx_guard = [](EVP_CIPHER_CTX* p) { EVP_CIPHER_CTX_free(p); };
	std::unique_ptr<EVP_CIPHER_CTX, decltype(ctx_guard)> ctx(EVP_CIPHER_CTX_new(), ctx_guard);
	if (nullptr == ctx.get())
	{
		log_err "EVP_CIPHER_CTX_new() failed." log_end;
		return false;
	}

	if (!aes_init_v2(PassPhrase,   // 보안 강화된 버전 사용
				     PassPhraseLen, 
				     ctx.get(), 
				     Encrypt))
	{
		log_err "aes_init_v2() failed." log_end;
		return false;		
	}

	unsigned char* out = nullptr;
	int outlen = InputLength;
	if (true == Encrypt)
	{
		out = aes_encrypt(ctx.get(), Input, &outlen);
		if (nullptr == out)
		{
			log_err "aes_encrypt() failed" log_end;
			return false;
		}		
	}
	else
	{
		out = aes_decrypt(ctx.get(), Input, &outlen);
		if (nullptr == out)
		{
			log_err "aes_decrypt() failed" log_end;
			return false;
		}
	}

	Output = out;
	OutputLength = outlen;
	return true;
}