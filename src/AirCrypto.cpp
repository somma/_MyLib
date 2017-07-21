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

#include "AIRCrypto.h"
#pragma comment(lib, "libeay32.lib")

/**
* @brief	aes256 파일 암호화
* @param 	target_file_path ex) C:\\test_folder\\a.txt
*			encrypt_file_path ex) C:\\test_folder\\a.txt.crypto
**/
bool 
aes256_encrypt(
	_In_ const unsigned char* key,
	_In_ const std::wstring& target_file_path,
	_In_ const std::wstring& encrypt_file_path
	)
{
	_ASSERTE(nullptr != key);
	_ASSERTE(nullptr != target_file_path.c_str());
	_ASSERTE(nullptr != encrypt_file_path.c_str());
	_ASSERTE(target_file_path.compare(encrypt_file_path));
	if (nullptr == key ||
		nullptr == target_file_path.c_str() ||
		nullptr == encrypt_file_path.c_str() ||
		!target_file_path.compare(encrypt_file_path))
	{
		return false;
	}

	PBYTE buffer = nullptr;
	DWORD file_size = 0;
	unsigned char* encrypt_data = nullptr;
	uint32_t length = 0;

	std::wstring encrypt_file_name;
	if (!extract_last_tokenW(const_cast<std::wstring&>(encrypt_file_path),
							 L"\\",
							 encrypt_file_name,
							 false,
							 false))
	{
		log_err "extract_last_tokenW(%ws) failed.", encrypt_file_path log_end;
		return false;
	}

	std::wstring target_file_directory;
	if (!extract_last_tokenW(const_cast<std::wstring&>(target_file_path),
							 L"\\",
							 target_file_directory,
							 true,
							 false))
	{
		log_err "extract_last_tokenW(path=%ws) failed.", target_file_path log_end;
		return false;
	}

	if (LoadFileToMemory(target_file_path.c_str(), file_size, buffer))
	{
		if (!AirCryptBuffer(const_cast<unsigned char*>(key),
							(uint32_t)strlen((const char*)key),
							buffer,
							file_size,
							encrypt_data,
							length,
							true))
		{
			log_err "AirCryptBuffer() failed." log_end;
			return false;
		}
		if (!SaveBinaryFile(target_file_directory.c_str(),
							encrypt_file_name.c_str(),
							file_size,
							encrypt_data))
		{
			log_err "SaveBinaryFile() failed." log_end;
			return false;
		}
		free(buffer);
		free(encrypt_data);
	}
	else
	{
		log_err "LoadFileToMemory() failed" log_end;
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
aes256_decrypt(
	_In_ const unsigned char* key,
	_In_ const std::wstring& encrypt_file_path,
	_In_ const std::wstring& decrypt_file_path
	)
{
	_ASSERTE(nullptr != key);
	_ASSERTE(nullptr != encrypt_file_path.c_str());
	_ASSERTE(nullptr != decrypt_file_path.c_str());
	_ASSERTE(encrypt_file_path.compare(decrypt_file_path));
	if (nullptr == key ||
		nullptr == decrypt_file_path.c_str() ||
		nullptr == encrypt_file_path.c_str() ||
		!encrypt_file_path.compare(decrypt_file_path))
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
		log_err "extract_last_tokenW(path=%ws) failed.", decrypt_file_path log_end;
		return false;
	}
	std::wstring decrypt_file_directory;
	if(!extract_last_tokenW(const_cast<std::wstring&>(decrypt_file_path),
							L"\\", 
							decrypt_file_directory, 
							true, 
							false))
	{
		log_err "extract_last_tokenW(path=%ws) failed.", decrypt_file_path log_end;
		return false;
	}

	if (LoadFileToMemory(encrypt_file_path.c_str(), file_size, buffer))
	{
		if (!AirCryptBuffer(const_cast<unsigned char*>(key),
							(uint32_t)strlen((char*)key),
							buffer,
							file_size,
							encrypt_data,
							length,
							false))
		{
			log_err "AirCryptBuffer() failed." log_end;
			return false;
		}
		if (!SaveBinaryFile(decrypt_file_directory.c_str(),
							decrypt_file_name.c_str(),
							file_size,
							encrypt_data))
		{
			log_err "SaveBinaryFile() failed." log_end;
			return false;
		}
		free(buffer);
		free(encrypt_data);
	}
	else
	{
		log_err "LoadFileToMemory err" log_end;
		return false;
	}

	return true;
}


/**----------------------------------------------------------------------------
    \brief  
			Create an 256 bit key and IV using the supplied key_data. 
			salt can be added for taste.
			Fills in the encryption and decryption ctx objects and returns 0 on success  
-----------------------------------------------------------------------------*/
bool 
aes_init(
	_In_ const unsigned char* key_data, 
	_In_ const int key_data_len, 
	_Outptr_ EVP_CIPHER_CTX* ctx, 
	bool encrypt
	)
{
	unsigned char key[EVP_MAX_KEY_LENGTH]={0};
	unsigned char iv[EVP_MAX_IV_LENGTH]={0};

	/*
	 * Gen key & IV for AES 256 GCM mode. A SHA1 digest is used to hash the supplied key material.
	 * nrounds is the number of times the we hash the material. More rounds are more secure but
	 * slower.
	 */
	int ret = EVP_BytesToKey(EVP_aes_256_gcm(),
							 EVP_sha1(),
							 NULL, //salt, 
							 key_data,
							 key_data_len,
							 5,
							 key,
							 iv);
	if (ret != 32) 
	{
		log_err "Key size is %d bits - should be 256 bits", ret log_end;
		return false;
	}

	if (true == encrypt)
	{
		EVP_CIPHER_CTX_init(ctx);
		EVP_EncryptInit_ex(ctx,
						   EVP_aes_256_gcm(), 
						   NULL, 
						   key, 
						   iv);
	}
	else
	{
		EVP_CIPHER_CTX_init(ctx);
		EVP_DecryptInit_ex(ctx, 
						   EVP_aes_256_gcm(), 
						   NULL, 
						   key, 
						   iv);
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
	unsigned char *ciphertext = (unsigned char *) malloc(c_len);

	/* allows reusing of 'e' for multiple encryption cycles */
	//	EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

	/* update ciphertext, c_len is filled with the length of ciphertext generated,
     * len is the size of plaintext in bytes 
	 */
	EVP_EncryptUpdate(e, 
					  ciphertext, 
					  &c_len, 
					  plaintext, 
					  *len);

	/* update ciphertext with the final remaining bytes */
	EVP_EncryptFinal_ex(e, 
						ciphertext+c_len, 
						&f_len);

	*len = c_len + f_len;
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
  unsigned char *plaintext = (unsigned char *) malloc(p_len);
  
//  EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
  EVP_DecryptUpdate(e, 
				    plaintext, 
				    &p_len, 
				    ciphertext, 
				    *len);

  EVP_DecryptFinal_ex(e, 
					  plaintext+p_len, 
					  &f_len);

  *len = p_len + f_len;
  return plaintext;
}

bool 
AirCryptBuffer(
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
	_ASSERTE(nullptr == Output);
	if (nullptr == PassPhrase || 
		0 > PassPhraseLen || 
		nullptr == Input || 
		nullptr != Output) 
		return false;

	ERR_load_crypto_strings();

	/* "opaque" encryption, decryption ctx structures that libcrypto uses to record
     * status of enc/dec operations 
	 */

	EVP_CIPHER_CTX ctx;
	SecureZeroMemory(&ctx, sizeof(EVP_CIPHER_CTX));

	if (!aes_init(PassPhrase, 
				  PassPhraseLen, 
				  &ctx, 
				  Encrypt))
	{
		log_err "aes_init() failed." log_end;
		ERR_free_strings();
		return false;
	}

	unsigned char* out=nullptr;
	int outlen=InputLength;

	if (true == Encrypt)
	{
		out = aes_encrypt(&ctx, 
			  			  Input, 
						  &outlen);
		if (nullptr == out)
		{
			log_err "aes_encrypt() failed" log_end;
			ERR_free_strings();
			return false;
		}		
	}
	else
	{
		out = aes_decrypt(&ctx, 
						  Input, 
						  &outlen);
		if (nullptr == out)
		{
			log_err "aes_encrypt() failed" log_end;
			ERR_free_strings();
			return false;
		}
	}

	Output = out;
	OutputLength = outlen;

	EVP_CIPHER_CTX_cleanup(&ctx);
	ERR_free_strings();

	log_info "encrypt=%s, input len=%u, output len=%u",  
		true == Encrypt ? "true" : "false", 
		InputLength, 
		OutputLength 
		log_end;

	return true;
}

//AirCryptBuffer 사용하지 않고 aes 암호화
//
//
//int main(int argc, char **argv)
//{
//  /* "opaque" encryption, decryption ctx structures that libcrypto uses to record
//     status of enc/dec operations */
//  EVP_CIPHER_CTX en, de;
//
//  /* 8 bytes to salt the key_data during key generation. This is an example of
//     compiled in salt. We just read the bit pattern created by these two 4 byte 
//     integers on the stack as 64 bits of contigous salt material - 
//     ofcourse this only works if sizeof(int) >= 4 */
//  unsigned int salt[] = {12345, 54321};
//  unsigned char* key_data;
//  int key_data_len, i;
//  char *input[] = {"a", "abcd", "this is a test", "this is a bigger test", 
//                   "\nWho are you ?\nI am the 'Doctor'.\n'Doctor' who ?\nPrecisely!",
//                   NULL};
//
//  /* the key_data is read from the argument list */
//  key_data = (unsigned char *)argv[1];
//  key_data_len = strlen(argv[1]);
//  
//  /* gen key and iv. init the cipher ctx object */
//  if (aes_init(key_data, key_data_len, (unsigned char *)&salt, &en, &de)) {
//    printf("Couldn't initialize AES cipher\n");
//    return -1;
//  }
//
//  /* encrypt and decrypt each input string and compare with the original */
//  for (i = 0; input[i]; i++) {
//    char *plaintext;
//    unsigned char *ciphertext;
//    int olen, len;
//    
//    /* The enc/dec functions deal with binary data and not C strings. strlen() will 
//       return length of the string without counting the '\0' string marker. We always
//       pass in the marker byte to the encrypt/decrypt functions so that after decryption 
//       we end up with a legal C string */
//    olen = len = strlen(input[i])+1;
//    
//    ciphertext = aes_encrypt(&en, (unsigned char *)input[i], &len);
//    plaintext = (char *)aes_decrypt(&de, ciphertext, &len);
//
//    if (strncmp(plaintext, input[i], olen)) 
//      printf("FAIL: enc/dec failed for \"%s\"\n", input[i]);
//    else 
//      printf("OK: enc/dec ok for \"%s\"\n", plaintext);
//    
//    free(ciphertext);
//    free(plaintext);
//  }
//
//  EVP_CIPHER_CTX_cleanup(&en);
//  EVP_CIPHER_CTX_cleanup(&de);
//
//  return 0;
//}
//  