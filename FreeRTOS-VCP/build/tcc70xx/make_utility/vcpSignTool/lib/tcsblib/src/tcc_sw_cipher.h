/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_SW_CIPHER_H
#define TCC_SW_CIPHER_H

/**
 * @ingroup tcsb_crypto
 * AES block size in byte unit
 */
#define AES_BLOCK_SIZE   16

/**
 * @ingroup tcsb_crypto
 * AES key size in byte unit
 */
#define AES_KEY_SIZE     16

/**
 * @ingroup tcsb_crypto
 * ECDSA keypair private key size in byte unit
 */
#define PRIVATE_KEY_SIZE 32

/**
 * @ingroup tcsb_crypto
 * ECDSA keypair private key size in byte unit
 */
#define PUBLIC_KEY_SIZE  64

/**
 * @ingroup tcsb_crypto
 * CBC MAC size in byte unit
 */
#define CBC_MAC_SIZE     16

/**
 * @ingroup tcsb_crypto
 * This enumeration is used to determine operation mode of the AES
 */
typedef enum {
	CIPHER_AES_ECB = 0,  /**< Electronic Codebook */
	CIPHER_AES_CBC = 1   /**< Cipher Block Chaining */
} eCipherAES;

typedef struct {
	uint8_t rev[1024];
} sw_AES_t, sw_ECDSA_t;

/**
 * @ingroup tcsb_crypto
 * @brief Initialize ECDSA signing with key
 *
 * @param[in] pKey A pointer to the key
 * @param[in] iKeyLen Length of the key
 */
void tcc_api_sw_ECDSASignInitial(sw_ECDSA_t *h, const uint8_t *pKey, uint32_t iKeyLen);

/**
 * @ingroup tcsb_crypto
 * @brief ECDSA signing update in block unit
 *
 * @param[in] ucData A pointer to the data
 * @param[in] iDataLen Length of the data
 */
void tcc_api_sw_ECDSASignUpdate(sw_ECDSA_t *h, uint8_t *ucData, uint32_t iDataLen);

/**
 * @ingroup tcsb_crypto
 * @brief Finalize ECDSA signing
 *
 * @param[in] ucSignature A pointer to the signature
 *
 * @return 0 Success
 * @return else Fail
 */
int  tcc_api_sw_ECDSASignFinalize(sw_ECDSA_t *h, uint8_t *ucSignature);

/**
 * @ingroup tcsb_crypto
 * @brief Initialize ECDSA verifying with key
 *
 * @param[in] pKey A pointer to the key
 * @param[in] iKeyLen Length of the key
 */
void tcc_api_sw_ECDSAVerifyInitial(sw_ECDSA_t *h, uint8_t *pKey, uint32_t iKeyLen);

/**
 * @ingroup tcsb_crypto
 * @brief ECDSA verifying update in block unit
 *
 * @param[in] ucData A pointer to the data
 * @param[in] iDataLen Length of the data
 */
void tcc_api_sw_ECDSAVerifyUpdate(sw_ECDSA_t *h, uint8_t *ucData, uint32_t iDataLen);

/**
 * @ingroup tcsb_crypto
 * @brief Finalize ECDSA verifying
 *
 * @param[in] ucSignature A pointer to the signature
 *
 * @return 0 Success
 * @return else Fail
 */
int  tcc_api_sw_ECDSAVerifyFinalize(sw_ECDSA_t *h, uint8_t *ucSignature);

/**
 * @ingroup tcsb_crypto
 * @brief Initialize AES encryption with operation mode, key and initial vector
 *
 * @param[in] nOpMode Operation mode refer to #eCipherAES
 * @param[in] pKey A pointer to the key
 * @param[in] pIV A pointer to the initial vector
 */
void tcc_api_sw_AESEncryptInitial(sw_AES_t *h, uint32_t nMode, const uint8_t *pKey, uint8_t *pIV);

/**
 * @ingroup tcsb_crypto
 * @brief AES encryption in block unit
 *
 * @param[in] pDataIn A pointer to the input data
 * @param[in] pDataOut A pointer to the output data.\nIt can be same as pDataIn
 */
void tcc_api_sw_AESEncryptBlockUpdate(sw_AES_t *h, const uint8_t *pDataIn, uint8_t *pDataOut);

/**
 * @ingroup tcsb_crypto
 * @brief Deinitialize AES encryption
 */
void tcc_api_sw_AESEncryptDone(sw_AES_t *h);

/**
 * @ingroup tcsb_crypto
 * @brief Initialize AES decryption with operation mode, key and initial vector
 *
 * @param[in] nOpMode Operation mode refer to #eCipherAES
 * @param[in] pKey A pointer to the key
 * @param[in] pIV A pointer to the initial vector
 */
void tcc_api_sw_AESDecryptInitial(sw_AES_t *h, uint32_t nMode, uint8_t *pKey, uint8_t *pIV);

/**
 * @ingroup tcsb_crypto
 * @brief AES decryption in block unit
 *
 * @param[in] pDataIn A pointer to the input data
 * @param[in] pDataOut A pointer to the output data.\nIt can be same as pDataIn
 */
void tcc_api_sw_AESDecryptBlockUpdate(sw_AES_t *h, const uint8_t *pDataIn, uint8_t *pDataOut);

/**
 * @ingroup tcsb_crypto
 * @brief Deinitialize AES decryption
 */
void tcc_api_sw_AESDecryptDone(sw_AES_t *h);

/**
 * @ingroup tcsb_crypto
 * @brief Wrap the key with AES-ECB encryption
 *
 * @param[in] pWrapKey A pointer to wrap key
 * @param[in] pEncKey A pointer to the key
 * @param[in] pWrappedKey A pointer to wrapped key
 *
 * @return 0 Success
 * @return else Fail
 */
int tcc_api_sw_WrapKey(uint8_t *pWrapKey, uint8_t *pEncKey, uint8_t *pWrappedKey);

/**
 * @ingroup tcsb_crypto
 * @brief Unrap the key with AES-ECB decryption
 *
 * @param pWrapKey A pointer to wrap key
 * @param pWrappedKey A pointer to wrapped key
 * @param pEncKey A pointer to the key
 *
 * @return 0 Success
 * @return else Fail
 */
int tcc_api_sw_UnWrapKey(uint8_t *pWrapKey, uint8_t *pWrappedKey, uint8_t *pEncKey);

/**
 * @ingroup tcsb_crypto
 * @brief Generate random bytes
 *
 * @param[in] pData A pointer to the random bytes
 * @param[in] iDataLen Length of the random bytes
 *
 * @return 0 Success
 * @return else Fail
 */
int tcc_api_sw_GenerateRandom(uint8_t *pData, int32_t iDataLen);

/**
 * @ingroup tcsb_crypto
 * @brief Generate random AES key
 *
 * @param[in] pKey A pointer to the AES key
 *
 * @return 0 Success
 * @return else Fail
 */
int tcc_api_sw_GenerateAESKey(uint8_t *pKey);

/**
 * @ingroup tcsb_crypto
 * @brief Generate random ECDSA256 key pair
 *
 * @param[in] pPriKey A pointer to the private key
 * @param[in] pPubKey A pointer to the public key
 *
 * @return 0 Success
 * @return else Fail
 */
int tcc_api_sw_GenerateKeyPair(uint8_t *pPriKey, uint8_t *pPubKey);

/**
 * @ingroup tcsb_crypto
 * @brief Generate random ECDSA224 key pair
 *
 * @param[in] pPriKey A pointer to the private key
 * @param[in] pPubKey A pointer to the public key
 *
 * @return 0 Success
 * @return else Fail
 */
int tcc_api_sw_GenerateKeyPair224(uint8_t *pPriKey, uint8_t *pPubKey);

/**
 * @ingroup tcsb_crypto
 * @brief Initialize random number generator
 *
 * @return 0 Success
 * @return else Fail
 */
int RNG_Initialize(void);

/**
 * @ingroup tcsb_crypto
 * @brief Finalize random number generator
 *
 * @return 0 Success
 * @return else Fail
 */
int RNG_Finalize(void);

/**
 * @ingroup tcsb_crypto
 * @brief Generate random bytes
 *
 * @param[in] RandomData_p A pointer to the random bytes.
 * @param[in] RandomLen Length of the random bytes.
 *
 * @return 0 Success
 * @return else Fail
 */
int RNG_GenerateRandom(char *RandomData_p, int  RandomLen);

#endif /* TCC_SW_CIPHER_H */
