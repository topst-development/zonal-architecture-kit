// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "tcc_sw_cipher.h"
#include <lib/tcsblib/tcc_key.h>

#define TCC_KEYS_MARKER "TCSB_KEYFILE"

#define ERROR_NUM (-__LINE__)

int32_t tcc_key_load(int8_t *name, tcc_keys_t *keys)
{
	FILE *src;

	src = fopen(name, "rb");
	if (src == NULL) {
		return ERROR_NUM;
	}

	if (fread(keys->marker, TCC_KEYS_MARKER_LENGTH, 1, src) != 1) {
		fclose(src);
		return ERROR_NUM;
	}

	if (fread(&keys->count, sizeof(uint32_t), 1, src) != 1) {
		fclose(src);
		return ERROR_NUM;
	}

	if (fread(keys->desc, TCC_KEYS_DESC_LENGTH, 1, src) != 1) {
		fclose(src);
		return ERROR_NUM;
	}

	keys->key = malloc(keys->count * sizeof(tcc_key_t));
	if (!keys->key) {
		fclose(src);
		return ERROR_NUM;
	}
	if (fread(keys->key, sizeof(tcc_key_t), keys->count, src) != keys->count) {
		free(keys->key);
		fclose(src);
		return ERROR_NUM;
	}
	fclose(src);
	return 0;
}

int32_t tcc_key_init(tcc_keys_t *keys, int32_t keycount)
{
	memset(keys, 0, sizeof(tcc_keys_t));
	memcpy(keys->marker, TCC_KEYS_MARKER, TCC_KEYS_MARKER_LENGTH);
	keys->count = keycount;
	keys->key = malloc(keys->count * sizeof(tcc_key_t));
	if (!keys->key) {
		return ERROR_NUM;
	}
	memset(keys->key, 0, keys->count * sizeof(tcc_key_t));
	return 0;
}

int32_t tcc_key_generate(tcc_keys_t *keys, int32_t algorithm)
{
	tcc_key_t *key;
	uint32_t i;

	memset(keys->key, 0, keys->count * sizeof(tcc_key_t));
	for (i = 0; i < keys->count; i++) {
		key = &keys->key[i];

		/* generating authentication key */
		if (algorithm == TCC_KEY_ECDSA) {
			/* ecdsa key */
			if (tcc_api_sw_GenerateKeyPair(key->prikey, key->pubkey) != 0) {
				printf("tcc_api_sw_GenerateKeyPair error\n");
				return -1;
			}
		} else if (algorithm == TCC_KEY_CBCMAC) {
			/* cbcmac key */
			if (tcc_api_sw_GenerateAESKey(key->prikey) != 0) {
				printf("tcc_api_sw_GenerateAESKey error\n");
				return -1;
			}
			/* symetric key */
			memcpy(key->pubkey, key->prikey, TCC_KEY_PRIKEY_LENGTH);
		} else if (algorithm == TCC_KEY_ECDSA224) {
			/* ecdsa224 key */
			if (tcc_api_sw_GenerateKeyPair224(key->prikey, key->pubkey) != 0) {
				printf("tcc_api_sw_GenerateKeyPair224 error\n");
				return -1;
			}
		} else {
			return -1;
		}

		/* generating cryption key */
		if (tcc_api_sw_GenerateAESKey(key->enckey) != 0) {
			printf("tcc_api_sw_GenerateAESKey error\n");
			return -1;
		}

		/* generating cryption key */
		if (tcc_api_sw_GenerateAESKey(key->aeskey) != 0) {
			printf("tcc_api_sw_GenerateAESKey error\n");
			return -1;
		}
	}
	return 0;
}

int32_t tcc_key_save(tcc_keys_t *keys, FILE *dst)
{
	uint32_t i;

	if (fwrite(keys->marker, TCC_KEYS_MARKER_LENGTH, 1, dst) != 1) {
		return -1;
	}

	if (fwrite(&keys->count, sizeof(uint32_t), 1, dst) != 1) {
		return -1;
	}

	if (fwrite(keys->desc, TCC_KEYS_DESC_LENGTH, 1, dst) != 1) {
		return -1;
	}

	for (i = 0; i < keys->count; i++) {
		/* storing a created key */
		if (fwrite(&keys->key[i], sizeof(tcc_key_t), 1, dst) != 1) {
			return -1;
		}
	}
	return 0;
}

void tcc_key_free(tcc_keys_t *keys)
{
	if (keys->key) {
		free(keys->key);
	}
}

int32_t tcc_key_make(FILE *dst, int32_t algorithm, uint8_t *desc, int32_t key_num)
{
	tcc_keys_t keys;
	tcc_key_t key;
	int32_t i;

	/* clear */
	memset(&keys, 0, sizeof(tcc_keys_t));

	/* marker */
	memcpy(keys.marker, TCC_KEYS_MARKER, TCC_KEYS_MARKER_LENGTH);

	/* key count */
	keys.count = key_num;

	if (fwrite(keys.marker, TCC_KEYS_MARKER_LENGTH, 1, dst) != 1) {
		return -1;
	}

	if (fwrite(&keys.count, sizeof(uint32_t), 1, dst) != 1) {
		return -1;
	}

	if (fwrite(keys.desc, TCC_KEYS_DESC_LENGTH, 1, dst) != 1) {
		return -1;
	}

	memset(&key, 0, sizeof(tcc_key_t));
	for (i = 0; i < key_num; i++) {
		/* generating authentication key */
		if (algorithm == TCC_KEY_ECDSA) {
			/* ecdsa key */
			if (tcc_api_sw_GenerateKeyPair(key.prikey, key.pubkey) != 0) {
				printf("tcc_api_sw_GenerateKeyPair error\n");
				return -1;
			}
		} else if (algorithm == TCC_KEY_CBCMAC) {
			/* cbcmac key */
			if (tcc_api_sw_GenerateAESKey(key.prikey) != 0) {
				printf("tcc_api_sw_GenerateAESKey error\n");
				return -1;
			}
			/* symetric key */
			memcpy(key.pubkey, key.prikey, TCC_KEY_PRIKEY_LENGTH);
		} else if (algorithm == TCC_KEY_ECDSA224) {
			/* ecdsa224 key */
			if (tcc_api_sw_GenerateKeyPair224(key.prikey, key.pubkey) != 0) {
				printf("tcc_api_sw_GenerateKeyPair224 error\n");
				return -1;
			}
		} else {
			return -1;
		}

		/* generating cryption key */
		if (tcc_api_sw_GenerateAESKey(key.enckey) != 0) {
			printf("tcc_api_sw_GenerateAESKey error\n");
			return -1;
		}

		/* generating cryption key */
		if (tcc_api_sw_GenerateAESKey(key.aeskey) != 0) {
			printf("tcc_api_sw_GenerateAESKey error\n");
			return -1;
		}

		/* storing a created key */
		if (fwrite(&key, sizeof(tcc_key_t), 1, dst) != 1) {
			return -1;
		}
	}
	return 0;
}
