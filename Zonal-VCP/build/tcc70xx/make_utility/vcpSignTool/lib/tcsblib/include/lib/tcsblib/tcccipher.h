/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCCCIPHER_H
#define TCCCIPHER_H

typedef enum {
	CIPHER_ECB = 0, /**< Electronic Codebook */
	CIPHER_CBC,     /**< Cipher Block Chaining */
} BMode_t;

typedef struct {
	uint8_t rev[1024];
} AESCtx_t, ECDSACtx_t, SHACtx_t;

/* ECDSA224 */
int ECDSA224_SignInitial(ECDSACtx_t *pCtx, const uint8_t *ucKey);
int ECDSA224_SignUpdate(ECDSACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength);
int ECDSA224_SignFinalize(ECDSACtx_t *pCtx, uint8_t *ucSignature);
int ECDSA224_VerifyInitial(ECDSACtx_t *pCtx, const uint8_t *ucKey);
int ECDSA224_VerifyUpdate(ECDSACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength);
int ECDSA224_VerifyFinalize(ECDSACtx_t *pCtx, const uint8_t *ucSignature);

/* ECDSA256 */
int ECDSA256_SignInitial(ECDSACtx_t *pCtx, const uint8_t *ucKey);
int ECDSA256_SignUpdate(ECDSACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength);
int ECDSA256_SignFinalize(ECDSACtx_t *pCtx, uint8_t *ucSignature);
int ECDSA256_VerifyInitial(ECDSACtx_t *pCtx, const uint8_t *ucKey);
int ECDSA256_VerifyUpdate(ECDSACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength);
int ECDSA256_VerifyFinalize(ECDSACtx_t *pCtx, const uint8_t *ucSignature);

/* AES128 */
int AES128_EncryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV);
int AES128_EncryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut);
#if defined(SUPPORT_MBEDTLS)
int AES128_EncryptDone(AESCtx_t *pCtx);
#else
#define AES128_EncryptDone(x)
#endif
int AES128_DecryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV);
int AES128_DecryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut);
#if defined(SUPPORT_MBEDTLS)
int AES128_DecryptDone(AESCtx_t *pCtx);
#else
#define AES128_DecryptDone(x)
#endif

/* AES192 */
int AES192_EncryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV);
int AES192_EncryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut);
#if defined(SUPPORT_MBEDTLS)
int AES192_EncryptDone(AESCtx_t *pCtx);
#else
#define AES192_EncryptDone(x)
#endif
int AES192_DecryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV);
int AES192_DecryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut);
#if defined(SUPPORT_MBEDTLS)
int AES192_DecryptDone(AESCtx_t *pCtx);
#else
#define AES192_DecryptDone(x)
#endif

/* AES25 */
int AES256_EncryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV);
int AES256_EncryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut);
#if defined(SUPPORT_MBEDTLS)
int AES256_EncryptDone(AESCtx_t *pCtx);
#else
#define AES256_EncryptDone(x)
#endif
int AES256_DecryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV);
int AES256_DecryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut);
#if defined(SUPPORT_MBEDTLS)
int AES256_DecryptDone(AESCtx_t *pCtx);
#else
#define AES256_DecryptDone(x)
#endif

/* CMAC */
int AES256_CMAC_Initial(AESCtx_t *pCtx, uint8_t *ucKey);
int AES256_CMAC_Update(AESCtx_t *pCtx, const uint8_t *ucBuf, uint32_t ulLength);
int AES256_CMAC_Finalize(AESCtx_t *pCtx, uint8_t *ucOut);

/* SHA224 */
int SHA224_Initial(SHACtx_t *pCtx);
int SHA224_Update(SHACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength);
int SHA224_Finalize(SHACtx_t *pCtx, uint8_t *ucHash);

/* SHA256 */
int SHA256_Initial(SHACtx_t *pCtx);
int SHA256_Update(SHACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength);
int SHA256_Finalize(SHACtx_t *pCtx, uint8_t *ucHash);

/* Key */
int GenerateAES128Key(uint8_t *ucKey);
int GenerateAES192Key(uint8_t *ucKey);
int GenerateAES256Key(uint8_t *ucKey);
int GenerateECDSA224KeyPair(uint8_t *ucPrivKey, uint8_t *ucPubKey);
int GenerateECDSA256KeyPair(uint8_t *ucPrivKey, uint8_t *ucPubKey);

#endif /* TCCCIPHER_H */
