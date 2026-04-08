// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tcc_sw_cipher.h"
#include "tcc_auth.h"

/* ecdsa bytes */
#define ECDSA256_BYTES (32)
#define ECDSA224_BYTES (28)
#define ECDSA256_PUB_BYTES (ECDSA256_BYTES * 2)
#define ECDSA224_PUB_BYTES (ECDSA224_BYTES * 2)

/* CBCMAC bytes */
#define CBCMAC_BYTES      (16)

/* authentication algorithm */
enum
{
	AUTH_ECDSA = 0,
	AUTH_CBCMAC,
	AUTH_ECDSA224,
};

/* zero key */
static const uint8_t zerokey[16] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x99, 0x2E, 0x80, 0x72, 0xA9, 0xBF, 0x2A, 0xFB, 0x03, 0x4E, 0xE4, 0x6C, 0xF0, 0x3B, 0x6C, 0x57,
};

typedef struct {
	struct {
		uint8_t rev[SIGNATURE_BYTES - CBCMAC_BYTES];
		uint8_t mac[CBCMAC_BYTES];
	} sig;
	sw_AES_t aes;
} cbcmac_ctx_t;

static void swap_endian(void *buf)
{
	int32_t *d32 = (int32_t *)buf;
	int32_t i;

	for (i = 0; i < 4; i++) {
		d32[i] = ((d32[i] & 0x000000FF) << 24) | ((d32[i] & 0x0000FF00) << 8)
			| ((d32[i] & 0x00FF0000) >> 8) | ((d32[i] & 0xFF000000) >> 24);
	}
}

static int32_t cbcmac_initial(cbcmac_ctx_t *ctx, uint8_t *key)
{
#if (1) /* change endian */
	uint8_t revkey[CBCMAC_BYTES];
	int i;

	if (key) {
		for (i = 0; i < CBCMAC_BYTES; i++) {
			revkey[i] = key[CBCMAC_BYTES-i-1];
		}
		key = revkey;
	}
#endif
	memset(ctx->sig.rev, (key) ? 0xFF : 0x00, SIGNATURE_BYTES - CBCMAC_BYTES);
	memset(ctx->sig.mac, 0x00, CBCMAC_BYTES);
	tcc_api_sw_AESEncryptInitial(&ctx->aes, CIPHER_AES_CBC, (key) ? key : zerokey, ctx->sig.mac);
	return 0;
}

static int32_t cbcmac_update(cbcmac_ctx_t *ctx, uint8_t *buf, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i += CBCMAC_BYTES) {
		tcc_api_sw_AESEncryptBlockUpdate(&ctx->aes, buf + i, ctx->sig.mac);
	}
	return 0;
}

static int32_t cbcmac_sign_finalize(cbcmac_ctx_t *ctx, uint8_t *sig)
{
	uint32_t i;
	for (i = 0; i < SIGNATURE_BYTES - CBCMAC_BYTES; i += CBCMAC_BYTES) {
		tcc_api_sw_AESEncryptBlockUpdate(&ctx->aes, ctx->sig.rev + i, ctx->sig.mac);
	}
	tcc_api_sw_AESEncryptDone(&ctx->aes);
	swap_endian(ctx->sig.mac);
	memcpy(sig, &ctx->sig, SIGNATURE_BYTES);
	return 0;
}

static int32_t cbcmac_verify_finalize(cbcmac_ctx_t *ctx, uint8_t *sig)
{
	uint32_t i;
	for (i = 0; i < SIGNATURE_BYTES - CBCMAC_BYTES; i += CBCMAC_BYTES) {
		tcc_api_sw_AESEncryptBlockUpdate(&ctx->aes, ctx->sig.rev + i, ctx->sig.mac);
	}
	tcc_api_sw_AESEncryptDone(&ctx->aes);
	swap_endian(ctx->sig.mac);
	return memcmp(sig, &ctx->sig, SIGNATURE_BYTES);
}

static int32_t cbcmac_finalize(cbcmac_ctx_t *ctx, uint8_t *sig, uint32_t sign)
{
	if (sign == TCC_AUTH_SIGN) {
		return cbcmac_sign_finalize(ctx, sig);
	} else {
		return cbcmac_verify_finalize(ctx, sig);
	}
}

typedef struct {
	uint8_t sig[SIGNATURE_BYTES];
	sw_ECDSA_t ecdsa;
} ecdsa_ctx_t;

static int32_t ecdsa_initial(ecdsa_ctx_t *ctx, uint8_t *key, uint32_t sign)
{
	if (sign == TCC_AUTH_SIGN) {
		tcc_api_sw_ECDSASignInitial(&ctx->ecdsa, key, ECDSA256_BYTES);
	} else {
		tcc_api_sw_ECDSAVerifyInitial(&ctx->ecdsa, key, (ECDSA256_BYTES * 2));
	}

	return 0;
}

static int32_t ecdsa224_initial(ecdsa_ctx_t *ctx, uint8_t *key, uint32_t sign)
{
	if (sign == TCC_AUTH_SIGN) {
		tcc_api_sw_ECDSASignInitial(&ctx->ecdsa, key, ECDSA224_BYTES);
	} else {
		tcc_api_sw_ECDSAVerifyInitial(&ctx->ecdsa, key, (ECDSA224_BYTES * 2));
	}

	return 0;
}

static int32_t ecdsa_update(ecdsa_ctx_t *ctx, uint8_t *buf, uint32_t len, uint32_t sign)
{
	if (sign == TCC_AUTH_SIGN) {
		tcc_api_sw_ECDSASignUpdate(&ctx->ecdsa, buf, len);
	} else {
		tcc_api_sw_ECDSAVerifyUpdate(&ctx->ecdsa, buf, len);
	}

	return 0;
}

static int32_t ecdsa_finalize(ecdsa_ctx_t *ctx, uint8_t *sig, uint32_t sign)
{
	if (sign == TCC_AUTH_SIGN) {
		return tcc_api_sw_ECDSASignFinalize(&ctx->ecdsa, sig);
	} else {
		return tcc_api_sw_ECDSAVerifyFinalize(&ctx->ecdsa, sig);
	}
}

static int32_t algorithm(uint8_t *key)
{
	int32_t i;

	/* if key is null, return AUTH_CBCMAC*/
	if (key == NULL) {
		return AUTH_CBCMAC;
	}

	/* if key length is lager than ECDSA224_BYTES, return AUTH_ECDSA */
	for (i = ECDSA224_BYTES; i < ECDSA256_BYTES; i++) {
		if (key[i] != 0) {
			return AUTH_ECDSA;
		}
	}
	/* else if key length is lager than CBCMAC_BYTES, return AUTH_ECDSA224 */
	for (i = CBCMAC_BYTES; i < ECDSA224_BYTES; i++) {
		if (key[i] != 0) {
			return AUTH_ECDSA224;
		}
	}
	return AUTH_CBCMAC;
}

static int32_t pubkey_algorithm(uint8_t *key)
{
	int32_t i;

	if (key == NULL) {
		return -1;
	}

	/* if key length is lager than ECDSA224_PUB_BYTES, return AUTH_ECDSA */
	for (i = ECDSA224_PUB_BYTES; i < ECDSA256_PUB_BYTES; i++) {
		if (key[i] != 0) {
			return AUTH_ECDSA;
		}
	}
	/* else if key length is lager than CBCMAC_BYTES, return AUTH_ECDSA224 */
	for (i = CBCMAC_BYTES; i < ECDSA224_PUB_BYTES; i++) {
		if (key[i] != 0) {
			return AUTH_ECDSA224;
		}
	}
	return AUTH_CBCMAC;
}

int32_t tcc_auth_initial(tcc_auth_t *h, uint8_t *key, uint32_t sign)
{
	int32_t ret = -1;

	if (sign == TCC_AUTH_SIGN) {
		h->algorithm = algorithm(key);
	} else if (sign == TCC_AUTH_VERIFY) {
		h->algorithm = pubkey_algorithm(key);
	}
	h->ctx = NULL;
	h->sign = sign;

	switch (h->algorithm) {
		case AUTH_ECDSA: {
			h->ctx = calloc(1, sizeof(ecdsa_ctx_t));
			if (h->ctx) {
				ret = ecdsa_initial(h->ctx, key, h->sign);
			}
			break;
		}
		case AUTH_CBCMAC: {
			h->ctx = calloc(1, sizeof(cbcmac_ctx_t));
			if (h->ctx) {
				ret = cbcmac_initial(h->ctx, key);
			}
			break;
		}
		case AUTH_ECDSA224: {
			h->ctx = calloc(1, sizeof(ecdsa_ctx_t));
			if (h->ctx) {
				ret = ecdsa224_initial(h->ctx, key, h->sign);
			}
			break;
		}
	}
	return ret;
}

int32_t tcc_auth_update(tcc_auth_t *h, uint8_t *buf, uint32_t len)
{
	int32_t ret = -1;

	if (h->ctx) {
		switch (h->algorithm) {
		case AUTH_ECDSA:
		case AUTH_ECDSA224:
			ret = ecdsa_update(h->ctx, buf, len, h->sign);
			break;
		case AUTH_CBCMAC:
			ret = cbcmac_update(h->ctx, buf, len);
			break;
		}
	}
	return ret;
}

int32_t tcc_auth_finalize(tcc_auth_t *h, uint8_t *sig)
{
	int32_t ret = -1;

	if (h->ctx) {
		switch (h->algorithm) {
		case AUTH_ECDSA:
		case AUTH_ECDSA224:
			ret = ecdsa_finalize(h->ctx, sig, h->sign);
			free(h->ctx);
			break;
		case AUTH_CBCMAC:
			ret = cbcmac_finalize(h->ctx, sig, h->sign);
			free(h->ctx);
			break;
		}
	}
	return ret;
}
