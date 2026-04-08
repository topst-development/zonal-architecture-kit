// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <lib/tcsblib/tcccipher.h>
#include "tcc_mbed.h"

int ECDSA224_SignInitial(ECDSACtx_t *pCtx, const uint8_t *ucKey)
{
    return tcc_mbed_ecdsa_sign_initial((tcc_mbed_t *)pCtx, TCC_MBED_ECDSA224_BITS, ucKey);
}

int ECDSA224_SignUpdate(ECDSACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength)
{
    return tcc_mbed_ecdsa_sign_update((tcc_mbed_t *)pCtx, ucData, ulLength);
}

int ECDSA224_SignFinalize(ECDSACtx_t *pCtx, uint8_t *ucSignature)
{
    return tcc_mbed_ecdsa_sign_finalize((tcc_mbed_t *)pCtx, ucSignature);
}

int ECDSA224_VerifyInitial(ECDSACtx_t *pCtx, const uint8_t *ucKey)
{
    return tcc_mbed_ecdsa_verify_initial((tcc_mbed_t *)pCtx, TCC_MBED_ECDSA224_BITS, ucKey);
}

int ECDSA224_VerifyUpdate(ECDSACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength)
{
    return tcc_mbed_ecdsa_verify_update((tcc_mbed_t *)pCtx, ucData, ulLength);
}

int ECDSA224_VerifyFinalize(ECDSACtx_t *pCtx, const uint8_t *ucSignature)
{
    return tcc_mbed_ecdsa_verify_finalize((tcc_mbed_t *)pCtx, ucSignature);
}

int ECDSA256_SignInitial(ECDSACtx_t *pCtx, const uint8_t *ucKey)
{
    return tcc_mbed_ecdsa_sign_initial((tcc_mbed_t *)pCtx, TCC_MBED_ECDSA256_BITS, ucKey);
}

int ECDSA256_SignUpdate(ECDSACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength)
{
    return tcc_mbed_ecdsa_sign_update((tcc_mbed_t *)pCtx, ucData, ulLength);
}

int ECDSA256_SignFinalize(ECDSACtx_t *pCtx, uint8_t *ucSignature)
{
    return tcc_mbed_ecdsa_sign_finalize((tcc_mbed_t *)pCtx, ucSignature);
}

int ECDSA256_VerifyInitial(ECDSACtx_t *pCtx, const uint8_t *ucKey)
{
    return tcc_mbed_ecdsa_verify_initial((tcc_mbed_t *)pCtx, TCC_MBED_ECDSA256_BITS, ucKey);
}

int ECDSA256_VerifyUpdate(ECDSACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength)
{
    return tcc_mbed_ecdsa_verify_update((tcc_mbed_t *)pCtx, ucData, ulLength);
}

int ECDSA256_VerifyFinalize(ECDSACtx_t *pCtx, const uint8_t *ucSignature)
{
    return tcc_mbed_ecdsa_verify_finalize((tcc_mbed_t *)pCtx, ucSignature);
}

int AES128_EncryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV)
{
    return tcc_mbed_aes_encrypt_initial((tcc_mbed_t *)pCtx, eMode, TCC_MBED_AES128_BITS, ucKey, ucIV);
}

int AES128_EncryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut)
{
    return tcc_mbed_aes_encrypt_update((tcc_mbed_t *)pCtx, ucData, ucOut, TCC_MBED_AES_BLOCK_BYTES);
}

int AES128_EncryptDone(AESCtx_t *pCtx)
{
    return tcc_mbed_aes_encrypt_done((tcc_mbed_t *)pCtx);
}

int AES128_DecryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV)
{
    return tcc_mbed_aes_decrypt_initial((tcc_mbed_t *)pCtx, eMode, TCC_MBED_AES128_BITS, ucKey, ucIV);
}

int AES128_DecryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut)
{
    return tcc_mbed_aes_decrypt_update((tcc_mbed_t *)pCtx, ucData, ucOut, TCC_MBED_AES_BLOCK_BYTES);
}

int AES128_DecryptDone(AESCtx_t *pCtx)
{
    return tcc_mbed_aes_decrypt_done((tcc_mbed_t *)pCtx);
}

int AES192_EncryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV)
{
    return tcc_mbed_aes_encrypt_initial((tcc_mbed_t *)pCtx, eMode, TCC_MBED_AES192_BITS, ucKey, ucIV);
}

int AES192_EncryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut)
{
    return tcc_mbed_aes_encrypt_update((tcc_mbed_t *)pCtx, ucData, ucOut, TCC_MBED_AES_BLOCK_BYTES);
}

int AES192_EncryptDone(AESCtx_t *pCtx)
{
    return tcc_mbed_aes_encrypt_done((tcc_mbed_t *)pCtx);
}

int AES192_DecryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV)
{
    return tcc_mbed_aes_decrypt_initial((tcc_mbed_t *)pCtx, eMode, TCC_MBED_AES192_BITS, ucKey, ucIV);
}

int AES192_DecryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut)
{
    return tcc_mbed_aes_decrypt_update((tcc_mbed_t *)pCtx, ucData, ucOut, TCC_MBED_AES_BLOCK_BYTES);
}

int AES192_DecryptDone(AESCtx_t *pCtx)
{
    return tcc_mbed_aes_decrypt_done((tcc_mbed_t *)pCtx);
}

int AES256_EncryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV)
{
    return tcc_mbed_aes_encrypt_initial((tcc_mbed_t *)pCtx, eMode, TCC_MBED_AES256_BITS, ucKey, ucIV);
}

int AES256_EncryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut)
{
    return tcc_mbed_aes_encrypt_update((tcc_mbed_t *)pCtx, ucData, ucOut, TCC_MBED_AES_BLOCK_BYTES);
}

int AES256_EncryptDone(AESCtx_t *pCtx)
{
    return tcc_mbed_aes_encrypt_done((tcc_mbed_t *)pCtx);
}

int AES256_DecryptInitial(AESCtx_t *pCtx, BMode_t eMode, const uint8_t *ucKey, const uint8_t *ucIV)
{
    return tcc_mbed_aes_decrypt_initial((tcc_mbed_t *)pCtx, eMode, TCC_MBED_AES256_BITS, ucKey, ucIV);
}

int AES256_DecryptBlockUpdate(AESCtx_t *pCtx, const uint8_t *ucData, uint8_t *ucOut)
{
    return tcc_mbed_aes_decrypt_update((tcc_mbed_t *)pCtx, ucData, ucOut, TCC_MBED_AES_BLOCK_BYTES);
}

int AES256_DecryptDone(AESCtx_t *pCtx)
{
    return tcc_mbed_aes_decrypt_done((tcc_mbed_t *)pCtx);
}

int AES256_CMAC_Initial(AESCtx_t *pCtx, uint8_t *ucKey)
{
    return tcc_mbed_cmac_initial((tcc_mbed_t *)pCtx, TCC_MBED_AES256_BITS, ucKey);
}

int AES256_CMAC_Update(AESCtx_t *pCtx, const uint8_t *ucBuf, uint32_t ulLength)
{
    return tcc_mbed_cmac_update((tcc_mbed_t *)pCtx, ucBuf, ulLength);
}

int AES256_CMAC_Finalize(AESCtx_t *pCtx, uint8_t *ucOut)
{
    return tcc_mbed_cmac_finalize((tcc_mbed_t *)pCtx, ucOut);
}

int SHA224_Initial(SHACtx_t *pCtx)
{
    return tcc_mbed_sha_initial((tcc_mbed_t *)pCtx, TCC_MBED_SHA224_BITS);
}

int SHA224_Update(SHACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength)
{
    return tcc_mbed_sha_update((tcc_mbed_t *)pCtx, ucData, ulLength);
}

int SHA224_Finalize(SHACtx_t *pCtx, uint8_t *ucHash)
{
    return tcc_mbed_sha_finalize((tcc_mbed_t *)pCtx, ucHash);
}

int SHA256_Initial(SHACtx_t *pCtx)
{
    return tcc_mbed_sha_initial((tcc_mbed_t *)pCtx, TCC_MBED_SHA256_BITS);
}

int SHA256_Update(SHACtx_t *pCtx, const uint8_t *ucData, uint32_t ulLength)
{
    return tcc_mbed_sha_update((tcc_mbed_t *)pCtx, ucData, ulLength);
}

int SHA256_Finalize(SHACtx_t *pCtx, uint8_t *ucHash)
{
    return tcc_mbed_sha_finalize((tcc_mbed_t *)pCtx, ucHash);
}

int GenerateAES128Key(uint8_t *ucKey)
{
    return tcc_mbed_generate_random_value(ucKey, TCC_MBED_AES128_BYTES);
}

int GenerateAES192Key(uint8_t *ucKey)
{
    return tcc_mbed_generate_random_value(ucKey, TCC_MBED_AES192_BYTES);
}

int GenerateAES256Key(uint8_t *ucKey)
{
    return tcc_mbed_generate_random_value(ucKey, TCC_MBED_AES256_BYTES);
}

int GenerateECDSA224KeyPair(uint8_t *ucPrivKey, uint8_t *ucPubKey)
{
    return tcc_mbed_generate_ecdsa_keypair(TCC_MBED_ECDSA224_BITS, ucPrivKey, ucPubKey);
}

int GenerateECDSA256KeyPair(uint8_t *ucPrivKey, uint8_t *ucPubKey)
{
    return tcc_mbed_generate_ecdsa_keypair(TCC_MBED_ECDSA256_BITS, ucPrivKey, ucPubKey);
}
