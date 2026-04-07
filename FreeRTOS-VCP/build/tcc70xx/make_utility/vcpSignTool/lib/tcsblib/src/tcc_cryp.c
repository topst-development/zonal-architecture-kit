// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tcc_sw_cipher.h"
#include "tcc_cryp.h"

typedef struct {
	uint32_t enc;
	sw_AES_t aes;
} aes_ctx_t;

static int32_t encrypt_initial(aes_ctx_t *ctx, uint8_t *key, uint32_t len)
{
	uint32_t iv[AES_BLOCK_SIZE >> 2] = {0, 0, 0, len};

	tcc_api_sw_AESEncryptInitial(&ctx->aes, CIPHER_AES_ECB, key, NULL);
	tcc_api_sw_AESEncryptBlockUpdate(&ctx->aes, (uint8_t *)iv, (uint8_t *)iv);
	tcc_api_sw_AESEncryptDone(&ctx->aes);

	tcc_api_sw_AESEncryptInitial(&ctx->aes, CIPHER_AES_CBC, key, (uint8_t *)iv);

	return 0;
}

static int32_t encrypt_update(aes_ctx_t *ctx, uint8_t *buf, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i += AES_BLOCK_SIZE) {
		tcc_api_sw_AESEncryptBlockUpdate(&ctx->aes, buf + i, buf + i);
	}
	return 0;
}

static int32_t encrypt_finalize(aes_ctx_t *ctx)
{
	tcc_api_sw_AESEncryptDone(&ctx->aes);
	return 0;
}

static int32_t decrypt_initial(aes_ctx_t *ctx, uint8_t *key, uint32_t len)
{
	uint32_t iv[AES_BLOCK_SIZE >> 2] = {0, 0, 0, len};

	tcc_api_sw_AESEncryptInitial(&ctx->aes, CIPHER_AES_ECB, key, NULL);
	tcc_api_sw_AESEncryptBlockUpdate(&ctx->aes, (uint8_t *)iv, (uint8_t *)iv);
	tcc_api_sw_AESEncryptDone(&ctx->aes);

	tcc_api_sw_AESDecryptInitial(&ctx->aes, CIPHER_AES_CBC, key, (uint8_t *)iv);

	return 0;
}

static int32_t decrypt_update(aes_ctx_t *ctx, uint8_t *buf, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i += AES_BLOCK_SIZE) {
		tcc_api_sw_AESDecryptBlockUpdate(&ctx->aes, buf + i, buf + i);
	}
	return 0;
}

static int32_t decrypt_finalize(aes_ctx_t *ctx)
{
	tcc_api_sw_AESDecryptDone(&ctx->aes);
	return 0;
}

int32_t tcc_cryp_initial(tcc_cryp_t *h, uint8_t *key, uint32_t len, uint32_t enc)
{
	int32_t ret = -1;

	h->ctx = calloc(1, sizeof(aes_ctx_t));
	if (h->ctx) {
		h->enc = enc;
		if (h->enc == TCC_CRYP_ENC) {
			ret = encrypt_initial(h->ctx, key, len);
		} else {
			ret = decrypt_initial(h->ctx, key, len);
		}
	}
	return ret;
}

int32_t tcc_cryp_update(tcc_cryp_t *h, uint8_t *buf, uint32_t len)
{
	int32_t ret = -1;

	if (h->ctx) {
		if (h->enc == TCC_CRYP_ENC) {
			ret = encrypt_update(h->ctx, buf, len);
		} else {
			ret = decrypt_update(h->ctx, buf, len);
		}
	}
	return ret;
}

int32_t tcc_cryp_finalize(tcc_cryp_t *h)
{
	int32_t ret = -1;

	if (h->ctx) {
		if (h->enc == TCC_CRYP_ENC) {
			ret = encrypt_finalize(h->ctx);
		} else {
			ret = decrypt_finalize(h->ctx);
		}
		free(h->ctx);
	}
	return ret;
}
