/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_MBED_H
#define TCC_MBED_H

#define TCC_MBED_AES_BLOCK_BYTES (16)
#define TCC_MBED_AES128_BITS (128)
#define TCC_MBED_AES192_BITS (192)
#define TCC_MBED_AES256_BITS (256)
#define TCC_MBED_AES128_BYTES ((TCC_MBED_AES128_BITS) >> 3)
#define TCC_MBED_AES192_BYTES ((TCC_MBED_AES192_BITS) >> 3)
#define TCC_MBED_AES256_BYTES ((TCC_MBED_AES256_BITS) >> 3)

#define TCC_MBED_ECDSA224_BITS (224)
#define TCC_MBED_ECDSA256_BITS (256)
#define TCC_MBED_ECDSA224_BYTES ((TCC_MBED_ECDSA224_BITS) >> 3)
#define TCC_MBED_ECDSA256_BYTES ((TCC_MBED_ECDSA256_BITS) >> 3)

#define TCC_MBED_SHA224_BITS (224)
#define TCC_MBED_SHA256_BITS (256)
#define TCC_MBED_SHA224_BYTES ((TCC_MBED_SHA224_BITS) >> 3)
#define TCC_MBED_SHA256_BYTES ((TCC_MBED_SHA256_BITS) >> 3)

typedef enum {
	TCC_MBED_CIPHER_ECB = 0, /**< Electronic Codebook */
	TCC_MBED_CIPHER_CBC,     /**< Cipher Block Chaining */
} tcc_mbed_mode_t;

typedef struct {
	uint8_t rev[1024];
} tcc_mbed_t;

/* ECDSA */
int tcc_mbed_ecdsa_sign_initial(tcc_mbed_t *ctx, uint32_t bitsize, const uint8_t *key);
int tcc_mbed_ecdsa_sign_update(tcc_mbed_t *ctx, const uint8_t *buf, uint32_t length);
int tcc_mbed_ecdsa_sign_finalize(tcc_mbed_t *ctx, uint8_t *sig);
int tcc_mbed_ecdsa_verify_initial(tcc_mbed_t *ctx, uint32_t bitsize, const uint8_t *key);
int tcc_mbed_ecdsa_verify_update(tcc_mbed_t *ctx, const uint8_t *buf, uint32_t length);
int tcc_mbed_ecdsa_verify_finalize(tcc_mbed_t *ctx, const uint8_t *sig);

/* AES */
int tcc_mbed_aes_encrypt_initial(tcc_mbed_t *ctx, tcc_mbed_mode_t mode, uint32_t bitsize,
        const uint8_t *key, const uint8_t *iv);
int tcc_mbed_aes_encrypt_update(tcc_mbed_t *ctx, const uint8_t *srcbuf, uint8_t *outbuf,
        uint32_t length);
int tcc_mbed_aes_encrypt_done(tcc_mbed_t *ctx);
int tcc_mbed_aes_decrypt_initial(tcc_mbed_t *ctx, tcc_mbed_mode_t mode, uint32_t bitsize,
        const uint8_t *key, const uint8_t *iv);
int tcc_mbed_aes_decrypt_update(tcc_mbed_t *ctx, const uint8_t *srcbuf, uint8_t *outbuf,
        uint32_t length);
int tcc_mbed_aes_decrypt_done(tcc_mbed_t *ctx);

/* CMAC */
int tcc_mbed_cmac_initial(tcc_mbed_t *ctx, uint32_t bitsize, const uint8_t *key);
int tcc_mbed_cmac_update(tcc_mbed_t *ctx, const uint8_t *buf, uint32_t length);
int tcc_mbed_cmac_finalize(tcc_mbed_t *ctx, uint8_t *mac);

/* SHA */
int tcc_mbed_sha_initial(tcc_mbed_t *ctx, uint32_t bitsize);
int tcc_mbed_sha_update(tcc_mbed_t *ctx, const uint8_t *buf, uint32_t length);
int tcc_mbed_sha_finalize(tcc_mbed_t *ctx, uint8_t *buf);

/* Key */
int tcc_mbed_generate_random_value(uint8_t *buf, uint32_t length);
int tcc_mbed_generate_ecdsa_keypair(uint32_t bitsize, uint8_t *privkey, uint8_t *pubkey);

#endif /* TCC_MBED_H */
