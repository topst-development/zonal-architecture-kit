// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <stdlib.h>
#include <string.h>
#include <mbedtls/aes.h>
#include <mbedtls/cmac.h>
#include <mbedtls/sha256.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/pk.h>

#include "tcc_mbed.h"

#define TCC_MBED_ECDSA_MAX_BYTES (TCC_MBED_ECDSA256_BYTES)
#define TCC_MBED_SHA_MAX_BYTES (TCC_MBED_SHA256_BYTES)

typedef struct {
	mbedtls_sha256_context *sha;
    uint32_t bytesize;
    uint8_t key[TCC_MBED_ECDSA_MAX_BYTES << 1];
} tcc_mbed_ecdsa_priv_t;

typedef struct {
	mbedtls_aes_context *aes;
    tcc_mbed_mode_t mode;
    uint8_t iv[TCC_MBED_AES_BLOCK_BYTES];
} tcc_mbed_aes_priv_t;

typedef struct {
	mbedtls_cipher_context_t *cmac;
} tcc_mbed_cmac_priv_t;

typedef tcc_mbed_ecdsa_priv_t tcc_mbed_sha_priv_t;

/* ECDSA */
int tcc_mbed_ecdsa_sign_initial(tcc_mbed_t *ctx, uint32_t bitsize, const uint8_t *key)
{
    tcc_mbed_ecdsa_priv_t *priv = (tcc_mbed_ecdsa_priv_t *)ctx;
    priv->bytesize = bitsize >> 3;
    memcpy(priv->key, key, priv->bytesize);
    return tcc_mbed_sha_initial(ctx, bitsize);
}

int tcc_mbed_ecdsa_sign_update(tcc_mbed_t *ctx, const uint8_t *buf, uint32_t length)
{
    return tcc_mbed_sha_update(ctx, buf, length);
}

int tcc_mbed_ecdsa_sign_finalize(tcc_mbed_t *ctx, uint8_t *sig)
{
    tcc_mbed_ecdsa_priv_t *priv = (tcc_mbed_ecdsa_priv_t *)ctx;
    mbedtls_ecp_group grp;
    mbedtls_mpi d, r, s;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	unsigned char hash[TCC_MBED_SHA_MAX_BYTES];
    int ret = -1;

    if (tcc_mbed_sha_finalize(ctx, hash) == 0) {
        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_entropy_init(&entropy);
        if ((mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0)) == 0) {
            mbedtls_ecp_group_init(&grp);
            mbedtls_mpi_init(&d);
            mbedtls_mpi_init(&r);
            mbedtls_mpi_init(&s);

            mbedtls_mpi_read_binary(&d, priv->key, priv->bytesize);

            if (priv->bytesize == TCC_MBED_ECDSA224_BYTES) {
                mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP224R1);
            } else if (priv->bytesize == TCC_MBED_ECDSA256_BYTES) {
                mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
            }

            ret = mbedtls_ecdsa_sign(&grp, &r, &s, &d, hash, priv->bytesize,
                    mbedtls_ctr_drbg_random, &ctr_drbg);
            mbedtls_mpi_write_binary(&r, &sig[0], priv->bytesize);
            mbedtls_mpi_write_binary(&s, &sig[priv->bytesize], priv->bytesize);

            mbedtls_ecp_group_free(&grp);
            mbedtls_mpi_free(&d);
            mbedtls_mpi_free(&r);
            mbedtls_mpi_free(&s);
        }
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
    }
    return ret;
}

int tcc_mbed_ecdsa_verify_initial(tcc_mbed_t *ctx, uint32_t bitsize, const uint8_t *key)
{
    tcc_mbed_ecdsa_priv_t *priv = (tcc_mbed_ecdsa_priv_t *)ctx;
    priv->bytesize = bitsize >> 3;
    memcpy(&priv->key[0], &key[0], priv->bytesize << 1);
    return tcc_mbed_sha_initial(ctx, bitsize);
}

int tcc_mbed_ecdsa_verify_update(tcc_mbed_t *ctx, const uint8_t *buf, uint32_t length)
{
    return tcc_mbed_sha_update(ctx, buf, length);
}

int tcc_mbed_ecdsa_verify_finalize(tcc_mbed_t *ctx, const uint8_t *sig)
{
    tcc_mbed_ecdsa_priv_t *priv = (tcc_mbed_ecdsa_priv_t *)ctx;
    mbedtls_ecp_group grp;
    mbedtls_ecp_point Q;
    mbedtls_mpi r, s;
	unsigned char hash[TCC_MBED_SHA_MAX_BYTES];
    int ret;

    tcc_mbed_sha_finalize(ctx, hash);

    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&Q);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    mbedtls_mpi_read_binary(&Q.X, &priv->key[0], priv->bytesize);
    mbedtls_mpi_read_binary(&Q.Y, &priv->key[priv->bytesize], priv->bytesize);
    mbedtls_mpi_lset(&Q.Z, 1);
    mbedtls_mpi_read_binary(&r, &sig[0], priv->bytesize);
    mbedtls_mpi_read_binary(&s, &sig[priv->bytesize], priv->bytesize);

    if (priv->bytesize == TCC_MBED_ECDSA224_BYTES) {
        mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP224R1);
    } else if (priv->bytesize == TCC_MBED_ECDSA256_BYTES) {
        mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
    }
    ret = mbedtls_ecdsa_verify(&grp, hash, priv->bytesize, &Q, &r, &s);
    mbedtls_ecp_group_free(&grp);
    mbedtls_ecp_point_free(&Q);
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);

    return ret;
}

/* AES */
int tcc_mbed_aes_encrypt_initial(tcc_mbed_t *ctx, tcc_mbed_mode_t mode, uint32_t bitsize,
        const uint8_t *key, const uint8_t *iv)
{
    tcc_mbed_aes_priv_t *priv = (tcc_mbed_aes_priv_t *)ctx;
    int ret = -1;
    priv->aes = malloc(sizeof(mbedtls_aes_context));
    if (priv->aes) {
        priv->mode = mode;
        if (iv) {
            memcpy(priv->iv, iv, TCC_MBED_AES_BLOCK_BYTES);
        } else {
            memset(priv->iv, 0, TCC_MBED_AES_BLOCK_BYTES);
        }
        mbedtls_aes_init(priv->aes);
        ret = mbedtls_aes_setkey_enc(priv->aes, key, bitsize);
    }
    return ret;
}

int tcc_mbed_aes_encrypt_update(tcc_mbed_t *ctx, const uint8_t *srcbuf, uint8_t *outbuf,
        uint32_t length)
{
    tcc_mbed_aes_priv_t *priv = (tcc_mbed_aes_priv_t *)ctx;
    int ret = -1;
    if (priv->aes) {
        if (priv->mode == TCC_MBED_CIPHER_ECB) {
            while (length >= TCC_MBED_AES_BLOCK_BYTES) {
            	ret = mbedtls_aes_crypt_ecb(priv->aes, MBEDTLS_AES_ENCRYPT, srcbuf, outbuf);
                if (ret != 0) {
                    break;
                }
                length -= TCC_MBED_AES_BLOCK_BYTES;
                srcbuf += TCC_MBED_AES_BLOCK_BYTES;
                outbuf += TCC_MBED_AES_BLOCK_BYTES;
            }
        } else if (priv->mode == TCC_MBED_CIPHER_CBC) {
        	ret = mbedtls_aes_crypt_cbc(priv->aes, MBEDTLS_AES_ENCRYPT, length, priv->iv,
                    srcbuf, outbuf);
        }
        ret = 0;
    }
    return ret;
}

int tcc_mbed_aes_encrypt_done(tcc_mbed_t *ctx)
{
    tcc_mbed_aes_priv_t *priv = (tcc_mbed_aes_priv_t *)ctx;
    int ret = -1;
    if (priv->aes) {
        mbedtls_aes_free(priv->aes);
        free(priv->aes);
        priv->aes = NULL;
        ret = 0;
    }
    return ret;
}

int tcc_mbed_aes_decrypt_initial(tcc_mbed_t *ctx, tcc_mbed_mode_t mode, uint32_t bitsize,
        const uint8_t *key, const uint8_t *iv)
{
    tcc_mbed_aes_priv_t *priv = (tcc_mbed_aes_priv_t *)ctx;
    int ret = -1;
    priv->aes = malloc(sizeof(mbedtls_aes_context));
    if (priv->aes) {
        priv->mode = mode;
        if (iv) {
            memcpy(priv->iv, iv, TCC_MBED_AES_BLOCK_BYTES);
        } else {
            memset(priv->iv, 0, TCC_MBED_AES_BLOCK_BYTES);
        }
        mbedtls_aes_init(priv->aes);
        ret = mbedtls_aes_setkey_dec(priv->aes, key, bitsize);
    }
    return ret;
}

int tcc_mbed_aes_decrypt_update(tcc_mbed_t *ctx, const uint8_t *srcbuf, uint8_t *outbuf,
        uint32_t length)
{
    tcc_mbed_aes_priv_t *priv = (tcc_mbed_aes_priv_t *)ctx;
    int ret = -1;
    if (priv->aes) {
        if (priv->mode == TCC_MBED_CIPHER_ECB) {
            while (length >= TCC_MBED_AES_BLOCK_BYTES) {
            	ret = mbedtls_aes_crypt_ecb(priv->aes, MBEDTLS_AES_DECRYPT, srcbuf, outbuf);
                if (ret != 0) {
                    break;
                }
                length -= TCC_MBED_AES_BLOCK_BYTES;
                srcbuf += TCC_MBED_AES_BLOCK_BYTES;
                outbuf += TCC_MBED_AES_BLOCK_BYTES;
            }
        } else if (priv->mode == TCC_MBED_CIPHER_CBC) {
        	ret = mbedtls_aes_crypt_cbc(priv->aes, MBEDTLS_AES_DECRYPT, length, priv->iv,
                    srcbuf, outbuf);
        }
    }
    return ret;
}

int tcc_mbed_aes_decrypt_done(tcc_mbed_t *ctx)
{
    tcc_mbed_aes_priv_t *priv = (tcc_mbed_aes_priv_t *)ctx;
    int ret = -1;
    if (priv->aes) {
        mbedtls_aes_free(priv->aes);
        free(priv->aes);
        priv->aes = NULL;
        ret = 0;
    }
    return ret;
}

/* CMAC */
int tcc_mbed_cmac_initial(tcc_mbed_t *ctx, uint32_t bitsize, const uint8_t *key)
{
    tcc_mbed_cmac_priv_t *priv = (tcc_mbed_cmac_priv_t *)ctx;
    const mbedtls_cipher_info_t *cipher_info = NULL;
    int ret = -1;
    priv->cmac = malloc(sizeof(mbedtls_cipher_context_t));
    if (priv->cmac) {
        mbedtls_cipher_init(priv->cmac);
        if (bitsize == TCC_MBED_AES128_BITS) {
            cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB);
        } else if (bitsize == TCC_MBED_AES192_BITS) {
            cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_192_ECB);
        } else if (bitsize == TCC_MBED_AES256_BITS) {
            cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_ECB);
        }
        if (cipher_info) {
            mbedtls_cipher_setup(priv->cmac, cipher_info);
            ret = mbedtls_cipher_cmac_starts(priv->cmac, key, bitsize);
        }
    }
    return ret;
}

int tcc_mbed_cmac_update(tcc_mbed_t *ctx, const uint8_t *buf, uint32_t length)
{
    tcc_mbed_cmac_priv_t *priv = (tcc_mbed_cmac_priv_t *)ctx;
    int ret = -1;
    if (priv->cmac) {
    	ret = mbedtls_cipher_cmac_update(priv->cmac, buf, length);
    }
    return ret;
}

int tcc_mbed_cmac_finalize(tcc_mbed_t *ctx, uint8_t *mac)
{
    tcc_mbed_cmac_priv_t *priv = (tcc_mbed_cmac_priv_t *)ctx;
    int ret = -1;
    if (priv->cmac) {
        ret = mbedtls_cipher_cmac_finish(priv->cmac, mac);
        mbedtls_cipher_free(priv->cmac);
        free(priv->cmac);
    }
    return ret;
}

/* SHA */
int tcc_mbed_sha_initial(tcc_mbed_t *ctx, uint32_t bitsize)
{
    tcc_mbed_sha_priv_t *priv = (tcc_mbed_sha_priv_t *)ctx;
    int ret = -1;
    priv->sha = malloc(sizeof(mbedtls_sha256_context));
    if (priv->sha) {
        mbedtls_sha256_init(priv->sha);
        ret = 0;
        if (bitsize == TCC_MBED_SHA224_BITS) {
            mbedtls_sha256_starts(priv->sha, 1);
        } else if (bitsize == TCC_MBED_SHA256_BITS) {
            mbedtls_sha256_starts(priv->sha, 0);
        }
    }
    return ret;
}

int tcc_mbed_sha_update(tcc_mbed_t *ctx, const uint8_t *buf, uint32_t length)
{
    tcc_mbed_sha_priv_t *priv = (tcc_mbed_sha_priv_t *)ctx;
    int ret = -1;
    if (priv->sha) {
    	mbedtls_sha256_update(priv->sha, buf, length);
        ret = 0;
    }
    return ret;
}

int tcc_mbed_sha_finalize(tcc_mbed_t *ctx, uint8_t *hash)
{
    tcc_mbed_sha_priv_t *priv = (tcc_mbed_sha_priv_t *)ctx;
    int ret = -1;
    if (priv->sha) {
        mbedtls_sha256_finish(priv->sha, hash);
        mbedtls_sha256_free(priv->sha);
        free(priv->sha);
        ret = 0;
    }
    return ret;
}

int tcc_mbed_generate_random_value(uint8_t *buf, uint32_t length)
{
    mbedtls_entropy_context entropy;
    int ret;

    mbedtls_entropy_init(&entropy);
    ret = mbedtls_entropy_func(&entropy, buf, length);
    mbedtls_entropy_free(&entropy);

    return ret;
}

int tcc_mbed_generate_ecdsa_keypair(uint32_t bitsize, uint8_t *privkey, uint8_t *pubkey)
{
    mbedtls_ecp_group grp;
    mbedtls_ecp_point Q;
    mbedtls_mpi d;
    mbedtls_entropy_context entropy;
    int ret;

    mbedtls_entropy_init(&entropy);
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&Q);
    mbedtls_mpi_init(&d);
    if (bitsize == TCC_MBED_ECDSA224_BITS) {
        mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP224R1);
    } else if (bitsize == TCC_MBED_ECDSA256_BITS) {
        mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
    }
    ret = mbedtls_ecp_gen_keypair(&grp, &d, &Q, &mbedtls_entropy_func, &entropy);

    mbedtls_mpi_write_binary(&d, &privkey[0], bitsize >> 3);
    mbedtls_mpi_write_binary(&Q.X, &pubkey[0], bitsize >> 3);
    mbedtls_mpi_write_binary(&Q.Y, &pubkey[bitsize >> 3], bitsize >> 3);

    mbedtls_entropy_free(&entropy);
    mbedtls_ecp_group_free(&grp);
    mbedtls_ecp_point_free(&Q);
    mbedtls_mpi_free(&d);

    return ret;
}
