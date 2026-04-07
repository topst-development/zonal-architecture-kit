/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_AUTH_H
#define TCC_AUTH_H

/* signature bytes */
#define SIGNATURE_BYTES   (64)

/* authentication handle */
typedef struct {
	uint32_t algorithm;
	uint32_t sign;
	void *ctx;
} tcc_auth_t;

enum
{
	TCC_AUTH_SIGN = 0,
	TCC_AUTH_VERIFY,
};

/**
 * \brief Initialize
 *
 * \param h Handle
 * \param key Key
 * \return int32_t Error status
 */
int32_t tcc_auth_initial(tcc_auth_t *h, uint8_t *key, uint32_t sign);

/**
 * \brief Update
 *
 * \param h Handle
 * \param buf Buffer
 * \param len Length
 * \return int32_t Error status
 */
int32_t tcc_auth_update(tcc_auth_t *h, uint8_t *buf, uint32_t len);

/**
 * \brief Finalize
 *
 * \param h Handle
 * \param sig Signature
 * \return int32_t Error status
 */
int32_t tcc_auth_finalize(tcc_auth_t *h, uint8_t *sig);

#endif /* TCC_AUTH_H */
