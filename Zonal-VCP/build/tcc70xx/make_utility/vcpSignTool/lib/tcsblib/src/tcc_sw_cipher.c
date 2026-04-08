// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <string.h>
#include "tcc_sw_cipher.h"
#include "tcc_mbed.h"

#define AES_KEY_SWAP

static void uint8_to_sb_sw_number(const uint8_t *src, uint8_t *dst, uint32_t bytes)
{
	uint32_t i;
	for (i = 0; i < bytes; i++) {
		dst[bytes - i - 1] = src[i];
	}
}

static int WRAP(sw_AES_t *h, int8_t *KeyIn_p, int8_t *WrappedOut_p, uint32_t KeyBits)
{
	uint32_t R[256 * 8 / 64][2];
	union
	{
		/* A and B variables, combined storage */
		uint8_t A_bytes[16];
		uint32_t A[2];
		uint8_t B_bytes[16];
		uint32_t B[4];
	} u;
	uint32_t i, j;
	uint32_t n = KeyBits / 64;
	uint16_t cnt = 0;

	u.A[0] = u.A[1] = 0xA6A6A6A6;

	memcpy(R, KeyIn_p, n * 8);

	for (j = 0; j <= 5; j++) {
		for (i = 1; i <= n; i++) {
			u.B[2] = R[i - 1][0];
			u.B[3] = R[i - 1][1];
			tcc_api_sw_AESEncryptBlockUpdate(h, u.B_bytes, u.B_bytes);
			cnt++;
			u.A_bytes[7] ^= cnt & 255;
			u.A_bytes[6] ^= cnt >> 8;
			R[i - 1][0] = u.B[2];
			R[i - 1][1] = u.B[3];
		}
	}

	memcpy(WrappedOut_p, u.A, 8);
	memcpy(((char *)WrappedOut_p) + 8, R, n * 8);

	return 0;
}

static int UNWRAP(sw_AES_t *h, int8_t *WrappedIn_p, int8_t *KeyOut_p, uint32_t KeyBits)
{
	uint32_t R[256 * 8 / 64][2];
	union
	{
		/* A and B variables, combined storage */
		uint8_t A_bytes[16];
		uint32_t A[2];
		uint8_t B_bytes[16];
		uint32_t B[4];
	} u;
	int32_t i;
	int32_t j;
	uint32_t n = KeyBits / 64;
	uint16_t cnt;

	memcpy(u.A, WrappedIn_p, 8);

	memcpy(R, WrappedIn_p + 8, n * 8);

	for (j = 5; j >= 0; j--) {
		for (i = n; i >= 1; i--) {
			u.B[2] = R[i - 1][0];
			u.B[3] = R[i - 1][1];
			cnt = (uint16_t)(n * j + i);
			u.A_bytes[7] ^= cnt & 255;
			u.A_bytes[6] ^= cnt >> 8;
			tcc_api_sw_AESDecryptBlockUpdate(h, u.B_bytes, u.B_bytes);
			R[i - 1][0] = u.B[2];
			R[i - 1][1] = u.B[3];
		}
	}

	memcpy(KeyOut_p, R, n * 8);

	if (u.A[0] == u.A[1] && u.A[0] == 0xA6A6A6A6) {
		return 0;
	}
	return -1;
}

void tcc_api_sw_ECDSASignInitial(sw_ECDSA_t *h, const uint8_t *pKey, uint32_t iKeyLen)
{
    tcc_mbed_ecdsa_sign_initial((tcc_mbed_t *)h, iKeyLen << 3, pKey);
}

void tcc_api_sw_ECDSASignUpdate(sw_ECDSA_t *h, uint8_t *ucData, uint32_t iDataLen)
{
    tcc_mbed_ecdsa_sign_update((tcc_mbed_t *)h, ucData, iDataLen);
}

int tcc_api_sw_ECDSASignFinalize(sw_ECDSA_t *h, uint8_t *ucSignature)
{
    return tcc_mbed_ecdsa_sign_finalize((tcc_mbed_t *)h, ucSignature);
}

void tcc_api_sw_ECDSAVerifyInitial(sw_ECDSA_t *h, uint8_t *pKey, uint32_t iKeyLen)
{
    tcc_mbed_ecdsa_verify_initial((tcc_mbed_t *)h, iKeyLen << 2, pKey);
}

void tcc_api_sw_ECDSAVerifyUpdate(sw_ECDSA_t *h, uint8_t *ucData, uint32_t iDataLen)
{
    tcc_mbed_ecdsa_verify_update((tcc_mbed_t *)h, ucData, iDataLen);
}

int  tcc_api_sw_ECDSAVerifyFinalize(sw_ECDSA_t *h, uint8_t *ucSignature)
{
    return tcc_mbed_ecdsa_verify_finalize((tcc_mbed_t *)h, ucSignature);
}

void tcc_api_sw_AESEncryptInitial(sw_AES_t *h, uint32_t nMode, const uint8_t *pKey, uint8_t *pIV)
{
#if defined(AES_KEY_SWAP)
	uint8_t pSwap[TCC_MBED_AES128_BYTES];

    uint8_to_sb_sw_number(pKey, pSwap, TCC_MBED_AES128_BYTES);
	pKey = pSwap;
#endif
    tcc_mbed_aes_encrypt_initial((tcc_mbed_t *)h, nMode, TCC_MBED_AES128_BITS, pKey, pIV);
}

void tcc_api_sw_AESEncryptBlockUpdate(sw_AES_t *h, const uint8_t *pDataIn, uint8_t *pDataOut)
{
    tcc_mbed_aes_encrypt_update((tcc_mbed_t *)h, pDataIn, pDataOut, TCC_MBED_AES_BLOCK_BYTES);
}

void tcc_api_sw_AESEncryptDone(sw_AES_t *h)
{
    tcc_mbed_aes_encrypt_done((tcc_mbed_t *)h);
}

void tcc_api_sw_AESDecryptInitial(sw_AES_t *h, uint32_t nMode, uint8_t *pKey, uint8_t *pIV)
{
#if defined(AES_KEY_SWAP)
	uint8_t pSwap[TCC_MBED_AES128_BYTES];

    uint8_to_sb_sw_number(pKey, pSwap, TCC_MBED_AES128_BYTES);
	pKey = pSwap;
#endif
    tcc_mbed_aes_decrypt_initial((tcc_mbed_t *)h, nMode, TCC_MBED_AES128_BITS, pKey, pIV);
}

void tcc_api_sw_AESDecryptBlockUpdate(sw_AES_t *h, const uint8_t *pDataIn, uint8_t *pDataOut)
{
    tcc_mbed_aes_decrypt_update((tcc_mbed_t *)h, pDataIn, pDataOut, TCC_MBED_AES_BLOCK_BYTES);
}

void tcc_api_sw_AESDecryptDone(sw_AES_t *h)
{
    tcc_mbed_aes_encrypt_done((tcc_mbed_t *)h);
}

int tcc_api_sw_WrapKey(uint8_t *pWrapKey, uint8_t *pEncKey, uint8_t *pWrappedKey)
{
	sw_AES_t aes;
	char pData[AES_BLOCK_SIZE];
	int ret = -1;

	uint8_to_sb_sw_number(pWrapKey, pData, AES_BLOCK_SIZE);

	tcc_api_sw_AESEncryptInitial(&aes, CIPHER_AES_ECB, pData, NULL);

#if defined(AES_KEY_SWAP)
	uint8_to_sb_sw_number(pEncKey, pData, AES_BLOCK_SIZE);
#else
	memcpy(pData, pEncKey, AES_BLOCK_SIZE);
#endif

	ret = WRAP(&aes, pData, pWrappedKey, AES_BLOCK_SIZE << 3);

	tcc_api_sw_AESEncryptDone(&aes);

	return ret;
}

int tcc_api_sw_UnWrapKey(uint8_t *pWrapKey, uint8_t *pWrappedKey, uint8_t *pEncKey)
{
	sw_AES_t aes;
	char pData[AES_BLOCK_SIZE];
	int ret = -1;

	uint8_to_sb_sw_number(pWrapKey, pData, AES_BLOCK_SIZE);

	tcc_api_sw_AESDecryptInitial(&aes, CIPHER_AES_ECB, pData, NULL);

	ret = UNWRAP(&aes, pWrappedKey, pData, AES_BLOCK_SIZE << 3);

#if defined(AES_KEY_SWAP)
	uint8_to_sb_sw_number(pData, pEncKey, AES_BLOCK_SIZE);
#else
	memcpy(pEncKey, pData, AES_BLOCK_SIZE);
#endif

	tcc_api_sw_AESDecryptDone(&aes);

	return ret;
}

int tcc_api_sw_GenerateRandom(uint8_t *pData, int32_t iDataLen)
{
    return tcc_mbed_generate_random_value(pData, iDataLen);
}

int tcc_api_sw_GenerateAESKey(uint8_t *pKey)
{
    return tcc_mbed_generate_random_value(pKey, TCC_MBED_AES128_BYTES);
}

int tcc_api_sw_GenerateKeyPair(uint8_t *pPriKey, uint8_t *pPubKey)
{
    return tcc_mbed_generate_ecdsa_keypair(TCC_MBED_ECDSA256_BYTES, pPriKey, pPubKey);
}

int tcc_api_sw_GenerateKeyPair224(uint8_t *pPriKey, uint8_t *pPubKey)
{
    return tcc_mbed_generate_ecdsa_keypair(TCC_MBED_ECDSA224_BYTES, pPriKey, pPubKey);
}

int RNG_Initialize(void)
{
    return 0;
}

int RNG_Finalize(void)
{
    return 0;
}

int RNG_GenerateRandom(char *RandomData_p, int  RandomLen)
{
    return tcc_mbed_generate_random_value(RandomData_p, RandomLen);
}
