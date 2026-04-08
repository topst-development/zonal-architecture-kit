/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_KEY_H
#define TCC_KEY_H

#define TCC_KEY_ENCKEY_LENGTH 16
#define TCC_KEY_AESKEY_LENGTH 16
#define TCC_KEY_PRIKEY_LENGTH 32
#define TCC_KEY_PUBKEY_LENGTH 64

typedef struct {
	uint8_t enckey[TCC_KEY_ENCKEY_LENGTH]; /* This is used for AES256 (enckey + aeskey) */
	uint8_t aeskey[TCC_KEY_AESKEY_LENGTH];
	uint8_t prikey[TCC_KEY_PRIKEY_LENGTH];
	uint8_t pubkey[TCC_KEY_PUBKEY_LENGTH];
} tcc_key_t;

#define TCC_KEYS_MARKER_LENGTH 12
#define TCC_KEYS_DESC_LENGTH 48

typedef struct {
	uint8_t marker[TCC_KEYS_MARKER_LENGTH];
	uint32_t count;
	uint8_t desc[TCC_KEYS_DESC_LENGTH];
	tcc_key_t *key;
} tcc_keys_t;

enum
{
	TCC_KEY_ECDSA = 0,
	TCC_KEY_CBCMAC,
	TCC_KEY_ECDSA224,
};

/**
 * @brief
 *
 * @param name
 * @param keys
 * @return int32_t
 */
int32_t tcc_key_load(int8_t *name, tcc_keys_t *keys);

/**
 * @brief
 *
 * @param keys
 * @param keycount
 * @return int32_t
 */
int32_t tcc_key_init(tcc_keys_t *keys, int32_t keycount);

/**
 * @brief
 *
 * @param keys
 * @param dst
 * @return int32_t
 */
int32_t tcc_key_generate(tcc_keys_t *keys, int32_t algorithm);

/**
 * @brief
 *
 * @param keys
 * @param dst
 * @return int32_t
 */
int32_t tcc_key_save(tcc_keys_t *keys, FILE *dst);

/**
 * @brief
 *
 * @param keys
 */
void tcc_key_free(tcc_keys_t *keys);

/**
 * @brief
 *
 * @param dst
 * @param algorithm
 * @param desc
 * @param key_num
 * @return int32_t
 */
int32_t tcc_key_make(FILE *dst, int32_t algorithm, uint8_t *desc, int32_t key_num);

#endif /* TCC_KEY_H */
