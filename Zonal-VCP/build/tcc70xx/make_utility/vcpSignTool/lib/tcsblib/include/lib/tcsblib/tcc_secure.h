/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_SECURE_H
#define TCC_SECURE_H

/* The size of attribute */
#define ATTRIBUTE_BYTES 64

/* Index of attribute */
enum
{
	ATTR_IDX_VERSION = 0,
	ATTR_IDX_RBID,
	ATTR_IDX_CEID,
	ATTR_IDX_IMGID,
	ATTR_IDX_BODYSIZE,
};

/* The format of attribute */
typedef struct
{
	uint32_t uTYPE;
	uint32_t uVALUE;
} TC_ATTRIBUTE;

/**
 * \brief
 *
 * \param dst
 * \param imgid
 * \param rbid
 * \param ceid
 * \param bodylen
 * \return int32_t
 */
int32_t tcc_secure_attribute(void *dst, int32_t imgid, int32_t rbid, int32_t ceid, int32_t bodylen);

/**
 * \brief
 *
 * \param dst
 * \param imgid
 * \param rbid
 * \param ceid
 * \param bodylen
 * \param r5bc
 * \return int32_t
 */
int32_t tcc_secure_attribute_v2(void *dst, int32_t imgid, int32_t rbid, int32_t ceid, int32_t bodylen, int32_t r5bc);

/**
 * \brief
 *
 * \param dst
 * \param rbid
 * \return int32_t
 */
int32_t tcc_secure_set_rbid(void *dst, int32_t rbid);

/**
 * \brief
 *
 * \param dst
 * \param ceid
 * \return int32_t
 */
int32_t tcc_secure_set_ceid(void *dst, int32_t ceid);

/**
 * \brief
 *
 * \param dst
 * \param size
 * \return int32_t
 */
int32_t tcc_secure_set_size(void *dst, int32_t size);

/**
 * \brief
 *
 * \param dst
 * \param value
 * \return int32_t
 */
int32_t tcc_secure_set_version(void *dst, int32_t value);

/**
 * \brief
 *
 * \param src
 * \param dst
 * \param len
 * \param prikey
 * \param enckey
 * \param attribute
 * \return int32_t
 */
int32_t tcc_secure_image(FILE *src, FILE *dst, uint32_t len, uint8_t* prikey, uint8_t *enckey,
                         void *attribute);

/**
 * \brief
 *
 * \param buf
 * \param len
 * \param key
 * \return int32_t
 */
int32_t tcc_secure_m2m_enc(uint8_t *buf, uint32_t len, uint8_t *key);
int32_t tcc_secure_mcert_enc(uint8_t *buf, uint32_t len, uint8_t *key);

/**
 * \brief
 *
 * \param buf
 * \param len
 * \param key
 * \return int32_t
 */
int32_t tcc_secure_m2m_dec(uint8_t *buf, uint32_t len, uint8_t *key);

/**
 * \brief
 *
 * \param buf
 * \param len
 * \param key
 * \return int32_t
 */
int32_t tcc_secure_m2m_sign(uint8_t *buf, uint32_t len, uint8_t *key);

/**
 * \brief
 *
 * \param buf
 * \param len
 * \param key
 * \return int32_t
 */
int32_t tcc_secure_m2m_verify(uint8_t *buf, uint32_t len, uint8_t *key);

/**
 * \brief
 *
 * \param src
 * \param dst
 * \param len
 * \param key
 * \return int32_t
 */
int32_t tcc_secure_f2f_enc(FILE *src, FILE *dst, uint32_t len, uint8_t *key);

/**
 * \brief
 *
 * \param src
 * \param dst
 * \param len
 * \param key
 * \return int32_t
 */
int32_t tcc_secure_f2f_dec(FILE *src, FILE *dst, uint32_t len, uint8_t *key);

/**
 * \brief
 *
 * \param src
 * \param dst
 * \param len
 * \param key
 * \return int32_t
 */
int32_t tcc_secure_f2f_sign(FILE *src, FILE *dst, uint32_t len, uint8_t *key);

/**
 * \brief
 *
 * \param src
 * \param dst
 * \param len
 * \param key
 * \return int32_t
 */
int32_t tcc_secure_f2f_verify(FILE *src, uint32_t len, uint8_t *key);

/**
 * \brief
 *
 * \param buf
 * \param len
 * \return int32_t
 */
int32_t tcc_mfmcert_dec(uint8_t *buf, uint32_t len);

#endif /* TCC_SECURE_H */
