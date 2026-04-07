// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <lib/tcsblib/tcc_secure.h>
#include "tcc_cryp.h"
#include "tcc_auth.h"
#include <lib/tccscram.h>

#define ERROR_NUM (-__LINE__)

#define SECURE_BLOCK_BYTES (64)
#define ENC_BLOCK_BYTES (16)

/**
 * make attribute.
 */
int32_t tcc_secure_attribute(void *dst, int32_t imgid, int32_t rbid, int32_t ceid, int32_t bodylen)
{
	TC_ATTRIBUTE *attr = (TC_ATTRIBUTE *)dst;

	memset(attr, 0, ATTRIBUTE_BYTES);

	// Secure Boot Version
	attr[ATTR_IDX_VERSION].uTYPE = 0x1;
	attr[ATTR_IDX_VERSION].uVALUE = 0x1;

	// Rollback ID
	attr[ATTR_IDX_RBID].uTYPE = 0x2;
	attr[ATTR_IDX_RBID].uVALUE = (uint32_t)rbid;

	// Customer ID
	attr[ATTR_IDX_CEID].uTYPE = 0x3;
	attr[ATTR_IDX_CEID].uVALUE = (uint32_t)ceid;

	// Image ID
	attr[ATTR_IDX_IMGID].uTYPE = 0x4;
	attr[ATTR_IDX_IMGID].uVALUE = (uint32_t)imgid;

	// Body Size
	attr[ATTR_IDX_BODYSIZE].uTYPE = 0x5;
	attr[ATTR_IDX_BODYSIZE].uVALUE = (uint32_t)bodylen;

	return 0;
}

/**
 * set rbid in attribute.
 */
int32_t tcc_secure_set_rbid(void *dst, int32_t rbid)
{
	TC_ATTRIBUTE *attr = (TC_ATTRIBUTE *)dst;

	// Rollback ID
	attr[1].uTYPE = 0x2;
	attr[1].uVALUE = (uint32_t)rbid;

	return 0;
}

/**
 * set ceid in attribute.
 */
int32_t tcc_secure_set_ceid(void *dst, int32_t ceid)
{
	TC_ATTRIBUTE *attr = (TC_ATTRIBUTE *)dst;

	// Customer ID
	attr[2].uTYPE = 0x3;
	attr[2].uVALUE = (uint32_t)ceid;

	return 0;
}

/**
 * set body size in attribute.
 */
int32_t tcc_secure_set_size(void *dst, int32_t size)
{
	TC_ATTRIBUTE *attr = (TC_ATTRIBUTE *)dst;

	// Body Size
	attr[4].uTYPE = 0x5;
	attr[4].uVALUE = (uint32_t)size;

	return 0;
}

/**
 * set version in attribute.
 */
int32_t tcc_secure_set_version(void *dst, int32_t value)
{
	TC_ATTRIBUTE *attr = (TC_ATTRIBUTE *)dst;

	// Version
	attr[0].uVALUE = value;

	return 0;
}

/**
 * sign and encrypt a source file and store a destination file.
 * @param h handle.
 */
int32_t tcc_secure_image(FILE *src, FILE *dst, uint32_t len, uint8_t* prikey, uint8_t *enckey,
                         void *attribute)
{
	uint8_t signature[SIGNATURE_BYTES];
	uint8_t buf[SECURE_BLOCK_BYTES];
	int32_t ret = 0;
	uint32_t i;
#if ONLYSIGN != 1
	tcc_cryp_t cryp;
#else
	tcc_secure_set_version(attribute, 0); // Only Signed Image Vesion is 0
#endif
	tcc_auth_t auth;

	if (tcc_auth_initial(&auth, prikey, TCC_AUTH_SIGN) != 0) {
		return ERROR_NUM;
	}
#if ONLYSIGN != 1
	if (tcc_cryp_initial(&cryp, enckey, len, TCC_CRYP_ENC) != 0) {
		tcc_auth_finalize(&auth, signature);
		return ERROR_NUM;
	}
#endif

	/* update image body */
	for (i = 0; i < len; i += SECURE_BLOCK_BYTES) {
		if (fread(buf, SECURE_BLOCK_BYTES, 1, src) != 1) {
			ret = ERROR_NUM;
			break;
		}
		if (tcc_auth_update(&auth, buf, SECURE_BLOCK_BYTES) != 0) {
			ret = ERROR_NUM;
			break;
		}
#if ONLYSIGN != 1
		if (tcc_cryp_update(&cryp, buf, SECURE_BLOCK_BYTES) != 0) {
			ret = ERROR_NUM;
			break;
		}
#endif
		if (fwrite(buf, SECURE_BLOCK_BYTES, 1, dst) != 1) {
			ret = ERROR_NUM;
			break;
		}
	}

	/* update attribute (no-encrypt) */
	if (ret == 0) {
		if (tcc_auth_update(&auth, attribute, ATTRIBUTE_BYTES) != 0) {
			ret = ERROR_NUM;
		}
	}

	/* finalize */
	if (tcc_auth_finalize(&auth, signature) != 0) {
#if ONLYSIGN != 1
		tcc_cryp_finalize(&cryp);
#endif
		return ERROR_NUM;
	}
#if ONLYSIGN != 1
	if (tcc_cryp_finalize(&cryp) != 0) {
		return ERROR_NUM;
	}
#endif

	/* write attribute */
	if (fwrite(attribute, ATTRIBUTE_BYTES, 1, dst) != 1) {
		return ERROR_NUM;
	}
	/* write signature */
	if (fwrite(signature, SIGNATURE_BYTES, 1, dst) != 1) {
		return ERROR_NUM;
	}

	return ret;
}

/**
 * encrypt a data.
 * @param buf address of data.
 * @param len valid length of source file including signature.
 * @param key key for verification.
 */
int32_t tcc_secure_m2m_enc(uint8_t *buf, uint32_t len, uint8_t *key)
{
	tcc_cryp_t cryp;
	int32_t ret = 0;
	uint32_t i;

	/* remove attribute and signature size */
	len -= (ATTRIBUTE_BYTES + SIGNATURE_BYTES);

	/* initialize */
	if (tcc_cryp_initial(&cryp, key, len, TCC_CRYP_ENC) != 0) {
		return ERROR_NUM;
	}

	/* update */
	for (i = 0; i < len; i += ENC_BLOCK_BYTES) {
		if (tcc_cryp_update(&cryp, buf + i, ENC_BLOCK_BYTES) != 0) {
			ret = ERROR_NUM;
			break;
		}
	}

	/* finalize */
	if (tcc_cryp_finalize(&cryp) != 0) {
		return ERROR_NUM;
	}

	return ret;
}

int32_t tcc_secure_mcert_enc(uint8_t *buf, uint32_t len, uint8_t *key)
{
	tcc_cryp_t cryp;
	int32_t ret = 0;
	uint32_t i;

	/* remove signature size */
	len -= SIGNATURE_BYTES;

	if (key) {
		/* initialize */
		if (tcc_cryp_initial(&cryp, key, len, TCC_CRYP_ENC) != 0) {
			return ERROR_NUM;
		}

		/* update */
		for (i = 0; i < len; i += ENC_BLOCK_BYTES) {
			if (tcc_cryp_update(&cryp, buf + i, ENC_BLOCK_BYTES) != 0) {
				ret = ERROR_NUM;
				break;
			}
		}

		/* finalize */
		if (tcc_cryp_finalize(&cryp) != 0) {
			return ERROR_NUM;
		}
	}
	else {
		tccscram(buf, len);
	}

	return ret;
}

/**
 * decrypt a data.
 * @param buf address of data.
 * @param len valid length of source file including signature.
 * @param key key for verification.
 */
int32_t tcc_secure_m2m_dec(uint8_t *buf, uint32_t len, uint8_t *key)
{
	tcc_cryp_t cryp;
	int32_t ret = 0;
	uint32_t i;

	/* remove attribute and signature size */
	len -= (ATTRIBUTE_BYTES + SIGNATURE_BYTES);

	/* initialize */
	if (tcc_cryp_initial(&cryp, key, len, TCC_CRYP_DEC) != 0) {
		return ERROR_NUM;
	}

	/* update */
	for (i = 0; i < len; i += ENC_BLOCK_BYTES) {
		if (tcc_cryp_update(&cryp, buf + i, ENC_BLOCK_BYTES) != 0) {
			ret = ERROR_NUM;
			break;
		}
	}

	/* finalize */
	if (tcc_cryp_finalize(&cryp) != 0) {
		return ERROR_NUM;
	}

	return ret;
}

/**
 * sign a memory data.
 * @param buf address of data.
 * @param len valid length of memory including signature.
 * @param key key for verification.
 */
int32_t tcc_secure_m2m_sign(uint8_t *buf, uint32_t len, uint8_t *key)
{
	tcc_auth_t auth;
	int32_t ret = 0;
	uint32_t i;

	/* removing signature size */
	len -= SIGNATURE_BYTES;

	/* initializing */
	if (tcc_auth_initial(&auth, key, TCC_AUTH_SIGN) != 0) {
		return ERROR_NUM;
	}

	/* updating */
	for (i = 0; i < len; i += SECURE_BLOCK_BYTES) {
		if (tcc_auth_update(&auth, buf + i, SECURE_BLOCK_BYTES) != 0) {
			ret = ERROR_NUM;
			break;
		}
	}

	/* finalizing */
	if (tcc_auth_finalize(&auth, buf + len) != 0) {
		return ERROR_NUM;
	}

	return ret;
}

/**
 * verify an authentication of data.
 * @param buf address of data.
 * @param len valid length of data including signature.
 * @param key key for verification.
 */
int32_t tcc_secure_m2m_verify(uint8_t *buf, uint32_t len, uint8_t *key)
{
	return ERROR_NUM;
}

/**
 * encrypt a source file and store a destination file.
 * @param src source file handle.
 * @param src destination file handle.
 * @param len valid length of source file including signature.
 * @param key key for verification.
 */
int32_t tcc_secure_f2f_enc(FILE *src, FILE *dst, uint32_t len, uint8_t *key)
{
	tcc_cryp_t cryp;
	int32_t ret = 0;
	uint32_t i;
	uint8_t buf[SECURE_BLOCK_BYTES];

	/* remove attribute and signature size */
	len -= (ATTRIBUTE_BYTES + SIGNATURE_BYTES);

	/* initialize */
	if (tcc_cryp_initial(&cryp, key, len, TCC_CRYP_ENC) != 0) {
		return ERROR_NUM;
	}

	/* update */
	for (i = 0; i < len; i += ENC_BLOCK_BYTES) {
		if (fread(buf, ENC_BLOCK_BYTES, 1, src) != 1) {
			ret = ERROR_NUM;
			break;
		}
		if (tcc_cryp_update(&cryp, buf, ENC_BLOCK_BYTES) != 0) {
			ret = ERROR_NUM;
			break;
		}
		if (fwrite(buf, ENC_BLOCK_BYTES, 1, dst) != 1) {
			ret = ERROR_NUM;
			break;
		}
	}

	if (tcc_cryp_finalize(&cryp) != 0) {
		return ERROR_NUM;
	}

	/* copy attribute */
	if (fread(buf, ATTRIBUTE_BYTES, 1, src) != 1) {
		return ERROR_NUM;
	}
	if (fwrite(buf, ATTRIBUTE_BYTES, 1, dst) != 1) {
		return ERROR_NUM;
	}

	/* copy signature */
	if (fread(buf, SIGNATURE_BYTES, 1, src) != 1) {
		return ERROR_NUM;
	}
	if (fwrite(buf, SIGNATURE_BYTES, 1, dst) != 1) {
		return ERROR_NUM;
	}

	return ret;
}

/**
 * decrypt a source file and store a destination file.
 * @param src source file handle.
 * @param src destination file handle.
 * @param len valid length of source file including signature.
 * @param key key for verification.
 */
int32_t tcc_secure_f2f_dec(FILE *src, FILE *dst, uint32_t len, uint8_t *key)
{
	tcc_cryp_t cryp;
	int32_t ret = 0;
	uint32_t i;
	uint8_t buf[SECURE_BLOCK_BYTES];

	/* remove attribute and signature size */
	len -= (ATTRIBUTE_BYTES + SIGNATURE_BYTES);

	/* initialize */
	if (tcc_cryp_initial(&cryp, key, len, TCC_CRYP_DEC) != 0) {
		return ERROR_NUM;
	}

	/* update */
	for (i = 0; i < len; i += ENC_BLOCK_BYTES) {
		if (fread(buf, ENC_BLOCK_BYTES, 1, src) != 1) {
			ret = ERROR_NUM;
			break;
		}
		if (tcc_cryp_update(&cryp, buf, ENC_BLOCK_BYTES) != 0) {
			ret = ERROR_NUM;
			break;
		}
		if (fwrite(buf, ENC_BLOCK_BYTES, 1, dst) != 1) {
			ret = ERROR_NUM;
			break;
		}
	}

	if (tcc_cryp_finalize(&cryp) != 0) {
		return ERROR_NUM;
	}

	/* copy attribute */
	if (fread(buf, ATTRIBUTE_BYTES, 1, src) != 1) {
		return ERROR_NUM;
	}
	if (fwrite(buf, ATTRIBUTE_BYTES, 1, dst) != 1) {
		return ERROR_NUM;
	}

	/* copy signature */
	if (fread(buf, SIGNATURE_BYTES, 1, src) != 1) {
		return ERROR_NUM;
	}
	if (fwrite(buf, SIGNATURE_BYTES, 1, dst) != 1) {
		return ERROR_NUM;
	}

	return ret;
}

/**
 * sign a source file and store a destination file.
 * @param buf source file handle.
 * @param dst destination file handle.
 * @param len valid length of source file including signature.
 * @param key key for verification.
 */
int32_t tcc_secure_f2f_sign(FILE *src, FILE *dst, uint32_t len, uint8_t *key)
{
	tcc_auth_t auth;
	int32_t ret = 0;
	uint32_t i;
	uint8_t buf[SECURE_BLOCK_BYTES];
	uint8_t sig[SIGNATURE_BYTES];
	memset(sig, 0, SIGNATURE_BYTES);

	/* removing signature size */
	len -= SIGNATURE_BYTES;

	/* initializing */
	if (tcc_auth_initial(&auth, key, TCC_AUTH_SIGN) != 0) {
		return ERROR_NUM;
	}

	/* updating */
	for (i = 0; i < len; i += SECURE_BLOCK_BYTES) {
		if (fread(buf, SECURE_BLOCK_BYTES, 1, src) != 1) {
			ret = ERROR_NUM;
			break;
		}
		if (fwrite(buf, SECURE_BLOCK_BYTES, 1, dst) != 1) {
			ret = ERROR_NUM;
			break;
		}
		if (tcc_auth_update(&auth, buf, SECURE_BLOCK_BYTES) != 0) {
			ret = ERROR_NUM;
			break;
		}
	}

	/* finalizing */
	if (tcc_auth_finalize(&auth, sig) != 0) {
		return ERROR_NUM;
	}
	if (fwrite(sig, SIGNATURE_BYTES, 1, dst) != 1) {
		return ERROR_NUM;
	}

	return ret;
}

/**
 * verify an authentication of source file.
 * @param src source file handle.
 * @param len valid length of source file including signature.
 * @param key key for verification.
 */
int32_t tcc_secure_f2f_verify(FILE *src, uint32_t len, uint8_t *key)
{
	tcc_auth_t auth;
	int32_t ret = 0;
	uint32_t i;
	uint8_t buf[SECURE_BLOCK_BYTES];
	uint8_t sig[SIGNATURE_BYTES];

	/* removing signature size */
	len -= SIGNATURE_BYTES;

	/* initializing */
	if (tcc_auth_initial(&auth, key, TCC_AUTH_VERIFY) != 0) {
		return ERROR_NUM;
	}

	/* updating */
	for (i = 0; i < len; i += SECURE_BLOCK_BYTES) {
		if (fread(buf, SECURE_BLOCK_BYTES, 1, src) != 1) {
			ret = ERROR_NUM;
			break;
		}
		if (tcc_auth_update(&auth, buf, SECURE_BLOCK_BYTES) != 0) {
			ret = ERROR_NUM;
			break;
		}
	}

	if (fread(sig, SIGNATURE_BYTES, 1, src) != 1) {
		return ERROR_NUM;
	}

	/* finalizing */
	if (tcc_auth_finalize(&auth, sig) != 0) {
		return ERROR_NUM;
	}

	return ret;
}

int32_t tcc_mfmcert_dec(uint8_t *buf, uint32_t len)
{
	return -1;
}
