/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_CRYP_H
#define TCC_CRYP_H

typedef struct {
	uint32_t algorithm;
	uint32_t enc;
	void *ctx;
} tcc_cryp_t;

enum {
	TCC_CRYP_DEC = 0,
	TCC_CRYP_ENC,
};

/**
 * \brief Initialize
 * 
 * \param h Handle
 * \param key Key
 * \param iv Initial Vector
 * \param enc TCC_CRYP_DEC or TCC_CRYP_ENC
 * \return int32_t Error status
 */
int32_t tcc_cryp_initial(tcc_cryp_t *h, uint8_t *key, uint32_t iv, uint32_t enc);

/**
 * \brief Update
 * 
 * \param h Handle
 * \param buf Buffer
 * \param len Length
 * \return int32_t Error status
 */
int32_t tcc_cryp_update(tcc_cryp_t *h, uint8_t *buf, uint32_t len);

/**
 * \brief Finalize
 * 
 * \param h Handle
 * \return int32_t Error status
 */
int32_t tcc_cryp_finalize(tcc_cryp_t *h);

#endif /* TCC_CRYP_H */
