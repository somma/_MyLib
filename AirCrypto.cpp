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

#include "slogger.h"
#include "Win32Utils.h"

#include "AIRCrypto.h"
#pragma comment(lib, "libeay32.lib")


/**----------------------------------------------------------------------------
    \brief  
			Create an 256 bit key and IV using the supplied key_data. 
			salt can be added for taste.
			Fills in the encryption and decryption ctx objects and returns 0 on success
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
DTSTATUS 
aes_init(
	IN unsigned char *key_data, 
	IN int key_data_len, 
	OUT EVP_CIPHER_CTX *ctx, 
	BOOL encrypt
	)
{
	unsigned char key[EVP_MAX_KEY_LENGTH]={0};
	unsigned char iv[EVP_MAX_IV_LENGTH]={0};
	//unsigned char salt[8]={0};

	//int ret = RAND_pseudo_bytes(salt, sizeof(salt));
	//if (ret < 0)
	//{
	//	DBG_OPN
	//		"[ERR ]", "RAND_pseudo_bytes() failed, ssl_err=%s",
	//		ERR_error_string(ERR_get_error(), NULL)
	//	DBG_END

	//	return DTS_OPENSSL_PRNG_FAIL;
	//}
	

	/*
	* Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
	* nrounds is the number of times the we hash the material. More rounds are more secure but
	* slower.
	*/
	int ret = EVP_BytesToKey(
				EVP_aes_256_cbc(), 
				EVP_sha1(), 
				NULL, //salt, 
				key_data, 
				key_data_len, 
				5, 
				key, 
				iv
				);
	if (ret != 32) 
	{
		DBG_ERR "Key size is %d bits - should be 256 bits", ret DBG_END
		return DTS_OPENSSL_KEYGEN_FAIL;
	}

	if (TRUE == encrypt)
	{
		EVP_CIPHER_CTX_init(ctx);
		EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
	}
	else
	{
		EVP_CIPHER_CTX_init(ctx);
		EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);	
	}

	return DTS_SUCCESS;
}

/**----------------------------------------------------------------------------
    \brief  
				Encrypt *len bytes of data    
				All data going in & out is considered binary (unsigned char[])
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
unsigned char*
aes_encrypt(
	IN EVP_CIPHER_CTX *e, 
	IN unsigned char *plaintext, 
	IN OUT int *len
	)
{
	int BlockSize = EVP_CIPHER_CTX_block_size(e);
	
	/* max ciphertext len for a n bytes of plaintext is n + AES_BLOCK_SIZE -1 bytes */
	/*int c_len = *len + AES_BLOCK_SIZE, f_len = 0;*/
	int c_len = *len + BlockSize;
	int f_len = 0;
	unsigned char *ciphertext = (unsigned char *) malloc(c_len);

	/* allows reusing of 'e' for multiple encryption cycles */
//	EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

	/* update ciphertext, c_len is filled with the length of ciphertext generated,
     * len is the size of plaintext in bytes 
	 */
	EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, *len);

	/* update ciphertext with the final remaining bytes */
	EVP_EncryptFinal_ex(e, ciphertext+c_len, &f_len);

	*len = c_len + f_len;
	return ciphertext;
}

/**----------------------------------------------------------------------------
    \brief  Decrypt *len bytes of ciphertext
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
unsigned char *
aes_decrypt(
	IN EVP_CIPHER_CTX *e, 
	IN unsigned char *ciphertext, 
	IN OUT int *len
	)
{
  /* plaintext will always be equal to or lesser than length of ciphertext*/
  int p_len = *len, f_len = 0;
  unsigned char *plaintext = (unsigned char *) malloc(p_len);
  
//  EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
  EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
  EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);

  *len = p_len + f_len;
  return plaintext;
}


/**----------------------------------------------------------------------------
    \brief  
    
    \param  
    \return
    \code
    
    \endcode        
-----------------------------------------------------------------------------*/
DTSTATUS 
AirCryptBuffer(
	IN unsigned char* PassPhrase,
	IN UINT32 PassPhraseLen,
	IN unsigned char* Input, 
	IN UINT32 InputLength, 
	OUT unsigned char*& Output, 
	OUT UINT32& OutputLength, 
	IN BOOL Encrypt
	)
{
	_ASSERTE(NULL != PassPhrase);
	_ASSERTE(0 < PassPhraseLen);
	_ASSERTE(NULL != Input);
	_ASSERTE(NULL == Output);
	if (NULL == PassPhrase || 0 > PassPhraseLen || NULL == Input || NULL != Output) 
	{
		return DTS_INVALID_PARAMETER;
	}

	ERR_load_crypto_strings();

	/* "opaque" encryption, decryption ctx structures that libcrypto uses to record
     * status of enc/dec operations 
	 */

	EVP_CIPHER_CTX ctx={0};
	DTSTATUS status = aes_init(PassPhrase, PassPhraseLen, &ctx, Encrypt);
	if (TRUE != DT_SUCCEEDED(status))
	{
		DBG_ERR "aes_init() failed, status=0x%08x", status DBG_END
		ERR_free_strings();
		return status;
	}


	unsigned char* out=NULL;
	int outlen=InputLength;
	
	if (TRUE == Encrypt)
	{
		out = aes_encrypt(&ctx, Input, &outlen);
		if (NULL == out)
		{
			DBG_ERR "%s", "aes_encrypt() failed" DBG_END
			ERR_free_strings();
			return DTS_OPENSSL_ERROR;
		}		
	}
	else
	{
		out = aes_decrypt(&ctx, Input, &outlen);
		if (NULL == out)
		{
			DBG_ERR "%s", "aes_encrypt() failed" DBG_END
			ERR_free_strings();
			return DTS_OPENSSL_ERROR;
		}
	}

	Output = out;
	OutputLength = outlen;

	EVP_CIPHER_CTX_cleanup(&ctx);
	ERR_free_strings();

	DBG_INFO "encrypt=%s, input len=%u, output len=%u",  TRUE == Encrypt ? "true" : "false", InputLength, OutputLength DBG_END
	return DTS_SUCCESS;
}

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
//  unsigned char *key_data;
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