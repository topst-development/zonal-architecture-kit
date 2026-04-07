// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : hsm_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : HSM TEST
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )

#if 0
#define NDEBUG
#endif

#define TLOG_LEVEL (TLOG_DEBUG)
#include "hsm_log.h"

#include <bsp.h>
#include <fmu.h>
#include <debug.h>
#include <stdlib.h>
#include <sal_internal.h>

#include "hsm_test.h"
#include "hsm_manager.h"

#define ENABLE_RSA (0) /* VCP HSM dose not support RSA Algorithm */
// clang-format off
/* Common */
static uint8 plain_data[32] = {
        0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08,
        0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08
};
static uint8 aes_key[16] = {
        0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04
};

static uint8 aead_iv[12] = {
        0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78
};
static uint8 aes_iv[16] = {
        0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78
};

/* AES Test */
static uint8 aes_aad[16] = {
        0x89, 0xAB, 0xCD, 0xEF, 0x89, 0xAB, 0xCD, 0xEF, 0x89, 0xAB, 0xCD, 0xEF, 0x89, 0xAB, 0xCD, 0xEF
};

/* MAC Test */
static uint8 mac_key[32] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};
static uint8 cmac_out[16] = {
        0xB9, 0xDB, 0x1E, 0x12, 0xAA, 0x66, 0xA1, 0x95, 0xAC, 0x26, 0x8E, 0x92, 0x0E, 0x63, 0x50, 0x89,

};
static uint8 hmac_out[32] = {
        0x8B, 0xD2, 0x76, 0xD9, 0xD0, 0x4F, 0x34, 0x7E, 0x80, 0xF3, 0xC0, 0xD8, 0x9A, 0x18, 0x31, 0xD9,
        0x12, 0xCD, 0x8E, 0x0D, 0x16, 0xEF, 0x27, 0xC4, 0xF7, 0x28, 0xB8, 0x9A, 0x90, 0x0B, 0xA3, 0x08,
};

/* ECDSA Test */
/* NIST P256 Test key pair */
/*static uint8 gzu08_p256_priv[32] = {
    0x10, 0xF5, 0x48, 0x49, 0x8B, 0x91, 0xF6, 0xCB, 0x02, 0xC2, 0xB9, 0x11, 0x3B, 0xEC, 0x36,
    0xED, 0xA5, 0xEE, 0x53, 0x61, 0xFE, 0x54, 0x6C, 0x4E, 0x01, 0xF0, 0x57, 0x34, 0xB6, 0x5E,
    0x05, 0xD7
};*/

/*static uint8 gzu08_p256_pub[64] = {
    0x03, 0x20, 0x41, 0x50, 0x42, 0x3D, 0x8A, 0x9D, 0xC2, 0x0F, 0xCF, 0x6A, 0x64, 0x8C, 0x23,
    0x67, 0x38, 0x13, 0x22, 0xBC, 0x7A, 0x51, 0x43, 0x1D, 0x4D, 0xA6, 0x4F, 0xFD, 0x4E, 0xD5,
    0xEE, 0xE7, 0x9A, 0x3D, 0x92, 0xDC, 0x75, 0xC5, 0x46, 0x43, 0xCE, 0xFA, 0xD7, 0x9C, 0xD3,
    0xB9, 0x8B, 0xCA, 0xF8, 0xB6, 0x2F, 0x02, 0xA0, 0x10, 0xB9, 0x3D, 0x8E, 0xA3, 0x80, 0x3F,
    0x19, 0xDD, 0x7F, 0x17
};*/

/* BP256 Test key pair */

/*static uint8 gzu08_bp256_priv[32] = {
     0x36,0x74,0x4D,0x09, 0x45,0xCE,0x41,0xC7, 0xD1,0x6A,0x09,0xD8, 0xF2,0xE4,0xED,0x86,
     0xD8,0x6C,0x9F,0x95, 0xF4,0xEB,0x77,0x8F, 0x98,0x51,0x10,0x58, 0x54,0xAD,0xA5,0x18
};*/

/*static uint8 gzu08_bp256_pub[64] = {
      0x80,0x90,0xFB,0xFD, 0x65,0x6E,0x74,0xB4, 0xBC,0x0C,0x6E,0xD6, 0x28,0x4A,0xA5,0x0E,
     0x8B,0x31,0x7A,0xBE, 0x6E,0xD8,0xC5,0xA1, 0x8C,0x18,0xDA,0x62, 0xB1,0x4F,0xB9,0x7E,
      0x98,0x64,0x5F,0xAF, 0x3A,0x87,0xB0,0xF8, 0x24,0xA7,0xE4,0x3E, 0x20,0xFC,0xCD,0x3C,
      0x3F,0xED,0xD4,0xD0, 0xAF,0x98,0xDA,0xD3, 0xC7,0x17,0xCB,0xA4, 0x20,0x56,0x97,0x1E
};*/
static uint8 gzu08_digest[32] = {
        0x44, 0xAC, 0xF6, 0xB7, 0xE3, 0x6C, 0x13, 0x42, 0xC2, 0xC5, 0x89, 0x72,
        0x04, 0xFE, 0x09, 0x50, 0x4E, 0x1E, 0x2E, 0xFB, 0x1A, 0x90, 0x03, 0x77,
        0xDB, 0xC4, 0xE7, 0xA6, 0xA1, 0x33, 0xEC, 0x56
};

#if (ENABLE_RSA)
/* RSA_SSA_PKCS_v1.5_1024 Test key pair */
static uint8 rsassa_n[128] = {
    0xB6, 0x74, 0x4D, 0x09, 0x45, 0xCE, 0x41, 0xC7, 0xD1, 0x6A, 0x09, 0xD8, 0xF2, 0xE4, 0xED, 0x86, 0xD8, 0x6C, 0x9F,
    0x95, 0xF4, 0xEB, 0x77, 0x8F, 0x98, 0x51, 0x10, 0x58, 0x54, 0xAD, 0xA5, 0x18, 0xAB, 0xFE, 0x6D, 0x34, 0x21, 0xD0,
    0x1D, 0xA5, 0x91, 0x69, 0xDD, 0x37, 0x7F, 0x08, 0x8B, 0xEC, 0xB3, 0xF7, 0xD6, 0x7B, 0xFA, 0xB9, 0xF7, 0x0E, 0x5E,
    0x87, 0x9C, 0xC4, 0xD2, 0x77, 0x7C, 0xD1, 0x4D, 0x8A, 0x24, 0x1C, 0x31, 0xC3, 0x67, 0x25, 0x71, 0xF2, 0x57, 0xB9,
    0xB2, 0xE8, 0xE6, 0xB6, 0xF3, 0xA1, 0x9A, 0x10, 0xD4, 0xA9, 0xC9, 0xB4, 0x83, 0x46, 0xA5, 0x29, 0xC9, 0xF1, 0x03,
    0x87, 0xCD, 0xA4, 0xEF, 0x16, 0xBC, 0x3E, 0x1C, 0x28, 0xAC, 0x24, 0x3E, 0xEC, 0x40, 0x2E, 0x24, 0x8C, 0x58, 0x88,
    0x3C, 0x7B, 0x90, 0x67, 0xE6, 0x93, 0x07, 0xEA, 0x54, 0x5B, 0x0A, 0x1F, 0x30, 0x33,
};
static uint8 rsassa_d[128] = {
    0x44, 0x17, 0xF5, 0xA4, 0x4F, 0x4C, 0x22, 0x1A, 0xBC, 0x1A, 0x52, 0x20, 0x88, 0xCE, 0x4D, 0xA3, 0xB4, 0xE4, 0x1D,
    0x4F, 0x2F, 0x00, 0x2E, 0x38, 0x06, 0x45, 0xC2, 0xCE, 0xE4, 0x93, 0x85, 0xF9, 0x46, 0x3F, 0x87, 0x6C, 0x1C, 0x6E,
    0x7A, 0x92, 0x0F, 0x4A, 0x2E, 0x48, 0x86, 0xFD, 0x9E, 0x93, 0x31, 0xBB, 0x7E, 0x6D, 0xF8, 0xEE, 0x2C, 0xB9, 0x4D,
    0x28, 0x59, 0xA6, 0xCF, 0x51, 0xCE, 0xE0, 0x00, 0x45, 0xF3, 0x11, 0x9F, 0xEF, 0xD2, 0x72, 0x05, 0x21, 0x2D, 0x81,
    0x7D, 0xF7, 0xB4, 0x64, 0xE2, 0xE2, 0xF1, 0xF0, 0x25, 0x48, 0x3E, 0x5F, 0xF8, 0xCC, 0x9E, 0x09, 0x61, 0xA3, 0x8D,
    0x34, 0x83, 0x16, 0x6D, 0x53, 0xDC, 0xE4, 0x33, 0x8B, 0x8E, 0x32, 0x0E, 0x85, 0xE8, 0xF0, 0xE4, 0xF7, 0xDF, 0x5B,
    0x8E, 0x60, 0x8B, 0x3E, 0xB4, 0xDE, 0xC3, 0x45, 0x73, 0x61, 0xAD, 0xC9, 0x97, 0xD9,
};
static uint8 rsassa_e[3] = {0x01, 0x00, 0x01};
#endif
static uint8 g_NIST_P256_SHA256_pri_key[32u] = {
        0x51u, 0x9bu, 0x42u, 0x3du, 0x71u, 0x5fu, 0x8bu, 0x58u, 0x1fu, 0x4fu, 0xa8u, 0xeeu, 0x59u,
        0xf4u, 0x77u, 0x1au, 0x5bu, 0x44u, 0xc8u, 0x13u, 0x0bu, 0x4eu, 0x3eu, 0xacu, 0xcau,
        0x54u, 0xa5u, 0x6du, 0xdau, 0x72u, 0xb4u, 0x64u
};

static uint8 g_NIST_P256_pri_key[32u] = {
        0x7du, 0x7du, 0xc5u, 0xf7u, 0x1eu, 0xb2u, 0x9du, 0xdau, 0xf8u, 0x0du, 0x62u, 0x14u, 0x63u,
        0x2eu, 0xeau, 0xe0u, 0x3du, 0x90u, 0x58u, 0xafu, 0x1fu, 0xb6u, 0xd2u, 0x2eu, 0xd8u,
        0x0bu, 0xadu, 0xb6u, 0x2bu, 0xc1u, 0xa5u, 0x34u
};

static uint8 g_NIST_P256_shared_pubKey[64u] ={
        0x1cu, 0xcbu, 0xe9u, 0x1cu, 0x07u, 0x5fu, 0xc7u, 0xf4u, 0xf0u, 0x33u, 0xbfu, 0xa2u, 0x48u,
        0xdbu, 0x8fu, 0xccu, 0xd3u, 0x56u, 0x5du, 0xe9u, 0x4bu, 0xbfu, 0xb1u, 0x2fu, 0x3cu,
        0x59u, 0xffu, 0x46u, 0xc2u, 0x71u, 0xbfu, 0x83u, 0xceu, 0x40u, 0x14u, 0xc6u, 0x88u,
        0x11u, 0xf9u, 0xa2u, 0x1au, 0x1fu, 0xdbu, 0x2cu, 0x0eu, 0x61u, 0x13u, 0xe0u, 0x6du,
        0xb7u, 0xcau, 0x93u, 0xb7u, 0x40u, 0x4eu, 0x78u, 0xdcu, 0x7cu, 0xcdu, 0x5cu, 0xa8u,
        0x9au, 0x4cu, 0xa9u
};

static uint8 g_NIST_P256_shared_secretKey[32u] =
{ 0x64u, 0xb9u, 0xacu, 0xffu, 0xa6u, 0x55u, 0xdcu, 0xc4u, 0x64u, 0xf1u, 0xdcu, 0x90u, 0xdcu,
        0x55u, 0x43u, 0xfcu, 0xecu, 0xe9u, 0x76u, 0x2eu, 0x75u, 0x3bu, 0xd1u, 0x43u, 0xbcu,
        0xb3u, 0x27u, 0x5eu, 0x3au, 0x4fu, 0x13u, 0x70u, };

/* HASH Test */
static uint8 digest_sha2_256[32] = {
        0x59, 0x47, 0xDB, 0x68, 0x5E, 0xC0, 0x33, 0xAD, 0x3C, 0xBE, 0x68, 0x60,
        0x23, 0x5A, 0x36, 0xCA, 0xC9, 0x89, 0xFD, 0x8E, 0x23, 0x04, 0x60, 0xDF,
        0x28, 0x1F, 0x18, 0xFA, 0xA0, 0x4A, 0x21, 0xB3
};
static uint8 digest_sha2_256_benchmark[32] = {
    0x25,0x4B,0xCC,0x3F,0xC4,0xF2,0x71,0x72,0x63,0x6D,0xF4,0xBF,0x32,0xDE,0x9F,0x10,
    0x7F,0x62,0x0D,0x55,0x9B,0x20,0xD7,0x60,0x19,0x7E,0x45,0x2B,0x97,0x45,0x39,0x17,
};
static uint8 cmac_mac_benchmar[16] = {
    0x78,0xDA,0x82,0xFD,0xAA,0x4B,0xFF,0xE4,0x8B,0x26,0x12,0x33,0x80,0x03,0x5F,0x55,
};
static uint8 hmac_mac_benchmar[32] = {
    0xDF,0xC9,0x41,0xD9,0x73,0x8A,0xB0,0x5D,0x7A,0x28,0x06,0xB1,0x1F,0xBA,0x20,0x32,
    0x1E,0x6E,0x61,0x1F,0x4E,0x15,0x24,0xB5,0xFC,0x39,0xCA,0xE6,0x67,0x41,0xB7,0x59,
};
static uint8 hash_message[32] = {
    0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06,
    0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08,
    0x05, 0x06, 0x07, 0x08
};

/* PRNG Seed Test */
static uint8 rng_seed[64] = {
        0x1cu, 0xcbu, 0xe9u, 0x1cu, 0x07u, 0x5fu, 0xc7u, 0xf4u, 0xf0u, 0x33u, 0xbfu, 0xa2u,
        0xdbu, 0x8fu, 0xccu, 0xd3u, 0x56u, 0x5du, 0xe9u, 0x4bu, 0xbfu, 0xb1u, 0x2fu, 0x3cu,
        0x59u, 0xffu, 0x46u, 0xc2u, 0x71u, 0xbfu, 0x83u, 0xceu, 0x40u, 0x14u, 0xc6u, 0x88u,
        0x11u, 0xf9u, 0xa2u, 0x1au, 0x1fu, 0xdbu, 0x2cu, 0x0eu, 0x61u, 0x13u, 0xe0u, 0x6du,
        0xb7u, 0xcau, 0x93u, 0xb7u, 0x40u, 0x4eu, 0x78u, 0xdcu, 0x7cu, 0xcdu, 0x5cu, 0xa8u,
        0x9au, 0x4cu, 0xa9u, 0x48u
};

static const uint32 pwLen = 24;
static const uint8 pw_data[25] = {"passwordPASSWORDpassword"};
static uint32 saltLen = 36;
static const uint8 salt_data[37] = {"saltSALTsaltSALTsaltSALTsaltSALTsalt"};
static const uint32 kdf_iteration = 16;
static const uint8 result_key_data[64] = {
    0x39,0x7F,0xD4,0x14,0xCA,0x96,0x92,0x78,0x99,0x1F,0x82,0x18,0xC3,0x1F,0xDA,0xBB,
    0x04,0xD8,0x62,0x41,0x6A,0x6B,0xA1,0x6C,0xF8,0x87,0x3C,0x02,0xBA,0x8F,0x43,0xF5,
    0x24,0xB2,0xDF,0x43,0xA7,0x33,0xE7,0x05,0x38,0x2A,0x3C,0x11,0x54,0x50,0x0E,0x0C,
    0x6D,0x92,0x37,0xFA,0xEB,0x92,0x22,0x59,0xEA,0x86,0x1F,0x03,0x6B,0xD4,0x99,0xED,
};

#if (ENABLE_RSA)
static sint8 root_rsa_crt[] = "-----BEGIN CERTIFICATE-----\r\n"
                              "MIIFZzCCA0+gAwIBAgIJAOtvwk6RfbrHMA0GCSqGSIb3DQEBCwUAMEoxCzAJBgNV\r\n"
                              "BAYTAktSMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxEjAQBgNVBAoMCVRlbGVjaGlw\r\n"
                              "czEQMA4GA1UEAwwHaGouamVvbjAeFw0yMjEyMTYwMDE5NDdaFw0zMjEyMTMwMDE5\r\n"
                              "NDdaMEoxCzAJBgNVBAYTAktSMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxEjAQBgNV\r\n"
                              "BAoMCVRlbGVjaGlwczEQMA4GA1UEAwwHaGouamVvbjCCAiIwDQYJKoZIhvcNAQEB\r\n"
                              "BQADggIPADCCAgoCggIBALNQZgbUCfIZg0lZ/G8ExSJI3+kR6WZiCO17MI6ftzXr\r\n"
                              "UbGPHmmRQ3RAs1DiZoVkGJyy79zIjJuXi6vwMWjbNJQlxJgW+3Cz0zZgFjLgDqBq\r\n"
                              "kMqXfr5kEc2QMCI3JmITpFGtUZlcU+OaFzS/0OtBtY/VTEuX3DHVFYPWUitCA3lI\r\n"
                              "MFB4eoT0c1acYjGK1Gsu3f054M6VteDy+gzdko8T5aNr2toNIo7pUq3RHtczODhO\r\n"
                              "29zOTxyP5jai+cML3Nu34ajyKzAlDzS4nWk6m5U7oZnuE220aq4X/AJqt7kezpon\r\n"
                              "/4/ijtXkRh3jq+W8BSF8YczMZTSPlYwN1bnh6tl9S22K/sb1NrKUPei23TOFyCou\r\n"
                              "YGrNU3zoL7Q2YEPio0Cj9PnhXpZfbEZS3owidei0lkCHGv4n3N6icDWsIgiuEVsi\r\n"
                              "TCwMMR4jc++WMyIZ0yQITer+DqK3lK+lQ8ZQrz0qp1fU6uCFJMDoNLN+t8UMF7LD\r\n"
                              "i5GxIdcRPxVcFMPg0rvq7KzQxTxW1ycFefbHWsdluf9xFWKDci3XtnmXW7bAR7Qd\r\n"
                              "z6zuMKMNuDCMK4f6OqHqE9UPyZEJcx/gHIxWQMILft0vAa1ys7a7JIYMQUYN4Etl\r\n"
                              "xWhztd8Fsb/a1qjzXKVSQWWcBI6jii2VfrRbJZ8ZK87yHEW6phaZyGIKdryb7bJT\r\n"
                              "AgMBAAGjUDBOMB0GA1UdDgQWBBR6MTH9YKuhm9DniBX3CKABogRamDAfBgNVHSME\r\n"
                              "GDAWgBR6MTH9YKuhm9DniBX3CKABogRamDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3\r\n"
                              "DQEBCwUAA4ICAQB1DCedjck4kAUoleygeyaazsi1nTl5kuzq4x4/pyccrTUeQLdb\r\n"
                              "DOrQjirEhniiCRST2vYjz70/uZO2lYBSqNkQuK9SHAQEFdEV2dqatA43hvHbDmTP\r\n"
                              "d9srNaNtFneMLCRrxhcEWZ9S/Y2FAVJLZjKZnFEw+df1KO1ESub8QUkmb8FeGRb8\r\n"
                              "69ptcCufIe7sT2rltA5IOEwV8n4dPgXrRDpX+yOAoboQC+3m9nLgIvT564zaJuVL\r\n"
                              "wH/u2hMSKD/2i11bTQbSVkNS7m6Zve9x684NmozB0coJlOwA8pChRkWOXkX2NR1U\r\n"
                              "lFALekXlbNCYVqBztnYIH5ybdH8Og5nR8X9XI2E+Sxr+f1QY/Do37NGeAOw10Iba\r\n"
                              "lmK5LYj1ViLcjO5XnZwqa3b/dZublzoGyQPGeIEPvaRItK1m9hYb+avFv+KaJosJ\r\n"
                              "J4EE0nkBJ08nxnDjXyphw2PomhG0M7DAUYb1F61HuW1aMEQ79SD0Qdg8VoNTvqgH\r\n"
                              "sYdMARogPrwlLdWre7L9TLGuE5qf5bNFj/qLxe7Woojlg+yMeK2pwmcTQEAYjvfU\r\n"
                              "pxb8sq9c+YFy1/aaOL577hts7aj3ksusrRzAHJSYAzT+GCWsfdm3grhADhJKEmKj\r\n"
                              "18XP12cu4rb+OJ2GcVT45g9Q0yqFwXEEkk2ZDMHNMA9v531iaUrARe8G1g==\r\n"
                              "-----END CERTIFICATE-----\r\n";

static sint8 host_rsa_crt[] = "-----BEGIN CERTIFICATE-----\r\n"
                              "MIIFETCCAvkCCQCCtIDOe4H2tTANBgkqhkiG9w0BAQsFADBKMQswCQYDVQQGEwJL\r\n"
                              "UjEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRIwEAYDVQQKDAlUZWxlY2hpcHMxEDAO\r\n"
                              "BgNVBAMMB2hqLmplb24wHhcNMjIxMjE2MDAyMTM1WhcNMjMxMjE2MDAyMTM1WjBL\r\n"
                              "MQswCQYDVQQGEwJLcjEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRIwEAYDVQQKDAlU\r\n"
                              "ZWxlY2hpcHMxETAPBgNVBAMMCGh5ZW9uanVuMIICIjANBgkqhkiG9w0BAQEFAAOC\r\n"
                              "Ag8AMIICCgKCAgEA2YRAWtIDLIk4K8Ujs/HATtKso2k9Xs72BKhvpAR2e1U6aXWS\r\n"
                              "65cXK25mSokTIcauIWD4Nj4PCaYo6hn0TYCDIfwfBP/u/Ir/SB5IyeyRoiy81xVj\r\n"
                              "7qSZue2zH7dk3RAl1BkQe2rqMvxX3N4N11ZABf11Pvgzr9uemSChhIrt7sW0gnWH\r\n"
                              "XXlqNEEpLGwFQVuGzAsLkaS4MXwEfJhLMwyvA42xYh9SXKeHi9uP0lGOzo+A0ghR\r\n"
                              "U9EEVFtYFA27MT/9mbO51FPK0KO3ltYscOVS2HeQ13JFUH6UE8xanhObDtKDq0DV\r\n"
                              "YdNzPWGbYlqPyD/TpxQX7LF2L4DA5yXMvuk533XR3MiAqSXmWzB2sCQmk8W/aEMk\r\n"
                              "sAxGXqSZ/vLElqK4EbkX1DyuWj2t+bUfo+Z1OmtZFR1/wMJub3n41tAmsBzELmP3\r\n"
                              "vPNkpggb/KA3TCtAyyQrPC/nUMDpGyx9FFr9jGOVj2dSvl/e408iPyHxpDBnqOFk\r\n"
                              "r/UooF9zNNMYQeBvzudv9JWpZwPJbX3PYGOdIRurVUqZprVFbG/S2TwUJcjgNU4K\r\n"
                              "wdrepnCkn8ck4yWGTsTYWaE6k+UnI2dqP4CE4ycJEEpft5T3lPI6z6IkwWcTGDOb\r\n"
                              "LY7Df8CjiyY2MceeeLVE322wkK+gJ2rJzBy7B4vu8WpQjWqWyoKkFC2ioCkCAwEA\r\n"
                              "ATANBgkqhkiG9w0BAQsFAAOCAgEAWEe/MnmcqRK1JrmUsEYqMZHm782cjybIbHRO\r\n"
                              "P2GlwHT81mpG5LQrxdBWiHBQ2+c9tdQu8jMqh+i2utrubmU9bADCCfROYOTWXzPi\r\n"
                              "fxdKf3voDExIB2ovY+It4E/s7ZKywtSy6qRCjPkiCUGr9Uwmu6n4EdZE1wQal2/A\r\n"
                              "+alSIdEjwvwJCSkCeLzvtGYyfiA9YAIDVYJSrKM9szz/xPBwL0FHPLG+bKzQGVhk\r\n"
                              "/6EckcNMFvtMUnCswADfojXvWFJBuXWySFEIyU6jY0jWUy5GOM8V3VNT8T0mDjjL\r\n"
                              "zzK5p5gwxUBzO8evjdtR8yoL0h/nIots2yviOeXyAHJwhgfW50/53s7YYJdJBgNm\r\n"
                              "MUbRJvZp//gFmurNbIt4o/UlZlQ6HUagZabl9kydnris95NJaq7wWXBNAv4N0mTs\r\n"
                              "VMmvPcRVFEWGBonwjSMAcX7I2MIpIXsL8gIfutUQ3u/rf5QrfV97IH4FNixPqquU\r\n"
                              "nrAIgtVBM2z8oIeVduX6SGkTwliPqYIfvxoy2eYQaPHstwOmPZD7r8YMYveBb/XE\r\n"
                              "aPk5G3QY7CaMmB186xZBAjuCwpQbe3yO84AQpvDrFYLKBr4KSPWWXXmLlS2M041V\r\n"
                              "Atk0ae4MQfSyb/mp4y6aHOYSQz+Xbo1/bguOz1LPBLhljk7zbdyMtW0wLVJEBgvx\r\n"
                              "FrPLqYc=\r\n"
                              "-----END CERTIFICATE-----\r\n";

static const uint8 host_rsa_pub[512] = {
    0xd9, 0x84, 0x40, 0x5a, 0xd2, 0x03, 0x2c, 0x89, 0x38, 0x2b, 0xc5, 0x23, 0xb3, 0xf1, 0xc0, 0x4e, 0xd2, 0xac, 0xa3,
    0x69, 0x3d, 0x5e, 0xce, 0xf6, 0x04, 0xa8, 0x6f, 0xa4, 0x04, 0x76, 0x7b, 0x55, 0x3a, 0x69, 0x75, 0x92, 0xeb, 0x97,
    0x17, 0x2b, 0x6e, 0x66, 0x4a, 0x89, 0x13, 0x21, 0xc6, 0xae, 0x21, 0x60, 0xf8, 0x36, 0x3e, 0x0f, 0x09, 0xa6, 0x28,
    0xea, 0x19, 0xf4, 0x4d, 0x80, 0x83, 0x21, 0xfc, 0x1f, 0x04, 0xff, 0xee, 0xfc, 0x8a, 0xff, 0x48, 0x1e, 0x48, 0xc9,
    0xec, 0x91, 0xa2, 0x2c, 0xbc, 0xd7, 0x15, 0x63, 0xee, 0xa4, 0x99, 0xb9, 0xed, 0xb3, 0x1f, 0xb7, 0x64, 0xdd, 0x10,
    0x25, 0xd4, 0x19, 0x10, 0x7b, 0x6a, 0xea, 0x32, 0xfc, 0x57, 0xdc, 0xde, 0x0d, 0xd7, 0x56, 0x40, 0x05, 0xfd, 0x75,
    0x3e, 0xf8, 0x33, 0xaf, 0xdb, 0x9e, 0x99, 0x20, 0xa1, 0x84, 0x8a, 0xed, 0xee, 0xc5, 0xb4, 0x82, 0x75, 0x87, 0x5d,
    0x79, 0x6a, 0x34, 0x41, 0x29, 0x2c, 0x6c, 0x05, 0x41, 0x5b, 0x86, 0xcc, 0x0b, 0x0b, 0x91, 0xa4, 0xb8, 0x31, 0x7c,
    0x04, 0x7c, 0x98, 0x4b, 0x33, 0x0c, 0xaf, 0x03, 0x8d, 0xb1, 0x62, 0x1f, 0x52, 0x5c, 0xa7, 0x87, 0x8b, 0xdb, 0x8f,
    0xd2, 0x51, 0x8e, 0xce, 0x8f, 0x80, 0xd2, 0x08, 0x51, 0x53, 0xd1, 0x04, 0x54, 0x5b, 0x58, 0x14, 0x0d, 0xbb, 0x31,
    0x3f, 0xfd, 0x99, 0xb3, 0xb9, 0xd4, 0x53, 0xca, 0xd0, 0xa3, 0xb7, 0x96, 0xd6, 0x2c, 0x70, 0xe5, 0x52, 0xd8, 0x77,
    0x90, 0xd7, 0x72, 0x45, 0x50, 0x7e, 0x94, 0x13, 0xcc, 0x5a, 0x9e, 0x13, 0x9b, 0x0e, 0xd2, 0x83, 0xab, 0x40, 0xd5,
    0x61, 0xd3, 0x73, 0x3d, 0x61, 0x9b, 0x62, 0x5a, 0x8f, 0xc8, 0x3f, 0xd3, 0xa7, 0x14, 0x17, 0xec, 0xb1, 0x76, 0x2f,
    0x80, 0xc0, 0xe7, 0x25, 0xcc, 0xbe, 0xe9, 0x39, 0xdf, 0x75, 0xd1, 0xdc, 0xc8, 0x80, 0xa9, 0x25, 0xe6, 0x5b, 0x30,
    0x76, 0xb0, 0x24, 0x26, 0x93, 0xc5, 0xbf, 0x68, 0x43, 0x24, 0xb0, 0x0c, 0x46, 0x5e, 0xa4, 0x99, 0xfe, 0xf2, 0xc4,
    0x96, 0xa2, 0xb8, 0x11, 0xb9, 0x17, 0xd4, 0x3c, 0xae, 0x5a, 0x3d, 0xad, 0xf9, 0xb5, 0x1f, 0xa3, 0xe6, 0x75, 0x3a,
    0x6b, 0x59, 0x15, 0x1d, 0x7f, 0xc0, 0xc2, 0x6e, 0x6f, 0x79, 0xf8, 0xd6, 0xd0, 0x26, 0xb0, 0x1c, 0xc4, 0x2e, 0x63,
    0xf7, 0xbc, 0xf3, 0x64, 0xa6, 0x08, 0x1b, 0xfc, 0xa0, 0x37, 0x4c, 0x2b, 0x40, 0xcb, 0x24, 0x2b, 0x3c, 0x2f, 0xe7,
    0x50, 0xc0, 0xe9, 0x1b, 0x2c, 0x7d, 0x14, 0x5a, 0xfd, 0x8c, 0x63, 0x95, 0x8f, 0x67, 0x52, 0xbe, 0x5f, 0xde, 0xe3,
    0x4f, 0x22, 0x3f, 0x21, 0xf1, 0xa4, 0x30, 0x67, 0xa8, 0xe1, 0x64, 0xaf, 0xf5, 0x28, 0xa0, 0x5f, 0x73, 0x34, 0xd3,
    0x18, 0x41, 0xe0, 0x6f, 0xce, 0xe7, 0x6f, 0xf4, 0x95, 0xa9, 0x67, 0x03, 0xc9, 0x6d, 0x7d, 0xcf, 0x60, 0x63, 0x9d,
    0x21, 0x1b, 0xab, 0x55, 0x4a, 0x99, 0xa6, 0xb5, 0x45, 0x6c, 0x6f, 0xd2, 0xd9, 0x3c, 0x14, 0x25, 0xc8, 0xe0, 0x35,
    0x4e, 0x0a, 0xc1, 0xda, 0xde, 0xa6, 0x70, 0xa4, 0x9f, 0xc7, 0x24, 0xe3, 0x25, 0x86, 0x4e, 0xc4, 0xd8, 0x59, 0xa1,
    0x3a, 0x93, 0xe5, 0x27, 0x23, 0x67, 0x6a, 0x3f, 0x80, 0x84, 0xe3, 0x27, 0x09, 0x10, 0x4a, 0x5f, 0xb7, 0x94, 0xf7,
    0x94, 0xf2, 0x3a, 0xcf, 0xa2, 0x24, 0xc1, 0x67, 0x13, 0x18, 0x33, 0x9b, 0x2d, 0x8e, 0xc3, 0x7f, 0xc0, 0xa3, 0x8b,
    0x26, 0x36, 0x31, 0xc7, 0x9e, 0x78, 0xb5, 0x44, 0xdf, 0x6d, 0xb0, 0x90, 0xaf, 0xa0, 0x27, 0x6a, 0xc9, 0xcc, 0x1c,
    0xbb, 0x07, 0x8b, 0xee, 0xf1, 0x6a, 0x50, 0x8d, 0x6a, 0x96, 0xca, 0x82, 0xa4, 0x14, 0x2d, 0xa2, 0xa0, 0x29};

static const uint8 host_rsa_sig[512] = {
    0x58, 0x47, 0xbf, 0x32, 0x79, 0x9c, 0xa9, 0x12, 0xb5, 0x26, 0xb9, 0x94, 0xb0, 0x46, 0x2a, 0x31, 0x91, 0xe6, 0xef,
    0xcd, 0x9c, 0x8f, 0x26, 0xc8, 0x6c, 0x74, 0x4e, 0x3f, 0x61, 0xa5, 0xc0, 0x74, 0xfc, 0xd6, 0x6a, 0x46, 0xe4, 0xb4,
    0x2b, 0xc5, 0xd0, 0x56, 0x88, 0x70, 0x50, 0xdb, 0xe7, 0x3d, 0xb5, 0xd4, 0x2e, 0xf2, 0x33, 0x2a, 0x87, 0xe8, 0xb6,
    0xba, 0xda, 0xee, 0x6e, 0x65, 0x3d, 0x6c, 0x00, 0xc2, 0x09, 0xf4, 0x4e, 0x60, 0xe4, 0xd6, 0x5f, 0x33, 0xe2, 0x7f,
    0x17, 0x4a, 0x7f, 0x7b, 0xe8, 0x0c, 0x4c, 0x48, 0x07, 0x6a, 0x2f, 0x63, 0xe2, 0x2d, 0xe0, 0x4f, 0xec, 0xed, 0x92,
    0xb2, 0xc2, 0xd4, 0xb2, 0xea, 0xa4, 0x42, 0x8c, 0xf9, 0x22, 0x09, 0x41, 0xab, 0xf5, 0x4c, 0x26, 0xbb, 0xa9, 0xf8,
    0x11, 0xd6, 0x44, 0xd7, 0x04, 0x1a, 0x97, 0x6f, 0xc0, 0xf9, 0xa9, 0x52, 0x21, 0xd1, 0x23, 0xc2, 0xfc, 0x09, 0x09,
    0x29, 0x02, 0x78, 0xbc, 0xef, 0xb4, 0x66, 0x32, 0x7e, 0x20, 0x3d, 0x60, 0x02, 0x03, 0x55, 0x82, 0x52, 0xac, 0xa3,
    0x3d, 0xb3, 0x3c, 0xff, 0xc4, 0xf0, 0x70, 0x2f, 0x41, 0x47, 0x3c, 0xb1, 0xbe, 0x6c, 0xac, 0xd0, 0x19, 0x58, 0x64,
    0xff, 0xa1, 0x1c, 0x91, 0xc3, 0x4c, 0x16, 0xfb, 0x4c, 0x52, 0x70, 0xac, 0xc0, 0x00, 0xdf, 0xa2, 0x35, 0xef, 0x58,
    0x52, 0x41, 0xb9, 0x75, 0xb2, 0x48, 0x51, 0x08, 0xc9, 0x4e, 0xa3, 0x63, 0x48, 0xd6, 0x53, 0x2e, 0x46, 0x38, 0xcf,
    0x15, 0xdd, 0x53, 0x53, 0xf1, 0x3d, 0x26, 0x0e, 0x38, 0xcb, 0xcf, 0x32, 0xb9, 0xa7, 0x98, 0x30, 0xc5, 0x40, 0x73,
    0x3b, 0xc7, 0xaf, 0x8d, 0xdb, 0x51, 0xf3, 0x2a, 0x0b, 0xd2, 0x1f, 0xe7, 0x22, 0x8b, 0x6c, 0xdb, 0x2b, 0xe2, 0x39,
    0xe5, 0xf2, 0x00, 0x72, 0x70, 0x86, 0x07, 0xd6, 0xe7, 0x4f, 0xf9, 0xde, 0xce, 0xd8, 0x60, 0x97, 0x49, 0x06, 0x03,
    0x66, 0x31, 0x46, 0xd1, 0x26, 0xf6, 0x69, 0xff, 0xf8, 0x05, 0x9a, 0xea, 0xcd, 0x6c, 0x8b, 0x78, 0xa3, 0xf5, 0x25,
    0x66, 0x54, 0x3a, 0x1d, 0x46, 0xa0, 0x65, 0xa6, 0xe5, 0xf6, 0x4c, 0x9d, 0x9e, 0xb8, 0xac, 0xf7, 0x93, 0x49, 0x6a,
    0xae, 0xf0, 0x59, 0x70, 0x4d, 0x02, 0xfe, 0x0d, 0xd2, 0x64, 0xec, 0x54, 0xc9, 0xaf, 0x3d, 0xc4, 0x55, 0x14, 0x45,
    0x86, 0x06, 0x89, 0xf0, 0x8d, 0x23, 0x00, 0x71, 0x7e, 0xc8, 0xd8, 0xc2, 0x29, 0x21, 0x7b, 0x0b, 0xf2, 0x02, 0x1f,
    0xba, 0xd5, 0x10, 0xde, 0xef, 0xeb, 0x7f, 0x94, 0x2b, 0x7d, 0x5f, 0x7b, 0x20, 0x7e, 0x05, 0x36, 0x2c, 0x4f, 0xaa,
    0xab, 0x94, 0x9e, 0xb0, 0x08, 0x82, 0xd5, 0x41, 0x33, 0x6c, 0xfc, 0xa0, 0x87, 0x95, 0x76, 0xe5, 0xfa, 0x48, 0x69,
    0x13, 0xc2, 0x58, 0x8f, 0xa9, 0x82, 0x1f, 0xbf, 0x1a, 0x32, 0xd9, 0xe6, 0x10, 0x68, 0xf1, 0xec, 0xb7, 0x03, 0xa6,
    0x3d, 0x90, 0xfb, 0xaf, 0xc6, 0x0c, 0x62, 0xf7, 0x81, 0x6f, 0xf5, 0xc4, 0x68, 0xf9, 0x39, 0x1b, 0x74, 0x18, 0xec,
    0x26, 0x8c, 0x98, 0x1d, 0x7c, 0xeb, 0x16, 0x41, 0x02, 0x3b, 0x82, 0xc2, 0x94, 0x1b, 0x7b, 0x7c, 0x8e, 0xf3, 0x80,
    0x10, 0xa6, 0xf0, 0xeb, 0x15, 0x82, 0xca, 0x06, 0xbe, 0x0a, 0x48, 0xf5, 0x96, 0x5d, 0x79, 0x8b, 0x95, 0x2d, 0x8c,
    0xd3, 0x8d, 0x55, 0x02, 0xd9, 0x34, 0x69, 0xee, 0x0c, 0x41, 0xf4, 0xb2, 0x6f, 0xf9, 0xa9, 0xe3, 0x2e, 0x9a, 0x1c,
    0xe6, 0x12, 0x43, 0x3f, 0x97, 0x6e, 0x8d, 0x7f, 0x6e, 0x0b, 0x8e, 0xcf, 0x52, 0xcf, 0x04, 0xb8, 0x65, 0x8e, 0x4e,
    0xf3, 0x6d, 0xdc, 0x8c, 0xb5, 0x6d, 0x30, 0x2d, 0x52, 0x44, 0x06, 0x0b, 0xf1, 0x16, 0xb3, 0xcb, 0xa9, 0x87
};
#endif
static sint8 root_ecdsa_crt[] = "-----BEGIN CERTIFICATE-----\r\n"
                                "MIICCTCCAa+gAwIBAgIJAJaJ7IReddC6MAoGCCqGSM49BAMCMGExCzAJBgNVBAYT\r\n"
                                "AktSMQ8wDQYDVQQIDAZQYW5neW8xDjAMBgNVBAcMBVNlb3VsMRIwEAYDVQQKDAlU\r\n"
                                "ZWxlY2hpcHMxDTALBgNVBAsMBEJTUEcxDjAMBgNVBAMMBVNFQlNQMB4XDTIzMDQy\r\n"
                                "NjA3MDkyN1oXDTI0MDQyNTA3MDkyN1owYTELMAkGA1UEBhMCS1IxDzANBgNVBAgM\r\n"
                                "BlBhbmd5bzEOMAwGA1UEBwwFU2VvdWwxEjAQBgNVBAoMCVRlbGVjaGlwczENMAsG\r\n"
                                "A1UECwwEQlNQRzEOMAwGA1UEAwwFU0VCU1AwWTATBgcqhkjOPQIBBggqhkjOPQMB\r\n"
                                "BwNCAAShJDnF2dS/jqdxYxbKobGnvDvb7i7bmiMV/YbOVhM1sdPPs2UPQZfftXmn\r\n"
                                "bfQUGKLmGniylEx+BlMQgA5I4r28o1AwTjAdBgNVHQ4EFgQUucDFQ89CYNlCMu5W\r\n"
                                "OEtXuUizFdowHwYDVR0jBBgwFoAUucDFQ89CYNlCMu5WOEtXuUizFdowDAYDVR0T\r\n"
                                "BAUwAwEB/zAKBggqhkjOPQQDAgNIADBFAiAlNapc/U3AjXMUpmjiLHDCtJPGLGz8\r\n"
                                "zs5WFFxXQTs+oAIhAIIwm3lQvMinWLqHpU2QRUArZvgPE6OAbCWluDsT6p62\r\n"
                                "-----END CERTIFICATE-----\r\n";


#if(1)
/* Test Pass Example */
static sint8 host_ecdsa_crt[] = "-----BEGIN CERTIFICATE-----\r\n"
                                "MIIBtDCCAVoCCQCSUaiN+B7LkDAKBggqhkjOPQQDAjBhMQswCQYDVQQGEwJLUjEP\r\n"
                                "MA0GA1UECAwGUGFuZ3lvMQ4wDAYDVQQHDAVTZW91bDESMBAGA1UECgwJVGVsZWNo\r\n"
                                "aXBzMQ0wCwYDVQQLDARCU1BHMQ4wDAYDVQQDDAVTRUJTUDAeFw0yMzA0MjYwNzEw\r\n"
                                "MTJaFw0yNDA0MjUwNzEwMTJaMGMxCzAJBgNVBAYTAktSMQ8wDQYDVQQIDAZQYW5n\r\n"
                                "eW8xDjAMBgNVBAcMBVNlb3VsMRIwEAYDVQQKDAlUZWxlY2hpcHMxDTALBgNVBAsM\r\n"
                                "BEJTUEcxEDAOBgNVBAMMB2hqLmplb24wWTATBgcqhkjOPQIBBggqhkjOPQMBBwNC\r\n"
                                "AAR91hcTNzT4atVXRpS57oQeowx4MgTCm8L0Bwx/0KjfqWW96cQVRapXCO4zbPc0\r\n"
                                "hQJS74NkDpLKfZPV79iHtrweMAoGCCqGSM49BAMCA0gAMEUCIQDTiKgFNAymgJYO\r\n"
                                "tpyTePlqgM0ghdKOaD4kmkGigtKmmAIgMTTA8+TxQME7a3GS84m2FPi/rAK7eDcT\r\n"
                                "g7paEgm5kmA=\r\n"
                                "-----END CERTIFICATE-----\r\n";
#else
/* Test Fail Example */
static sint8 host_ecdsa_crt[] = "-----BEGIN CERTIFICATE-----\r\n"
                                "MIICCzCCAbCgAwIBAgIJAOsg/b7KOxA1MAoGCCqGSM49BAMCMGMxCzAJBgNVBAYT\r\n"
                                "AktSMQ8wDQYDVQQIDAZQYW5neW8xDjAMBgNVBAcMBVNlb3VsMRIwEAYDVQQKDAlU\r\n"
                                "ZWxlY2hpcHMxDTALBgNVBAsMBEJTUEcxEDAOBgNVBAMMB2hqLmplb24wHhcNMjMw\r\n"
                                "NDI3MDA0NjE4WhcNMzMwNDI0MDA0NjE4WjBjMQswCQYDVQQGEwJLUjEPMA0GA1UE\r\n"
                                "CAwGUGFuZ3lvMQ4wDAYDVQQHDAVTZW91bDESMBAGA1UECgwJVGVsZWNoaXBzMQ0w\r\n"
                                "CwYDVQQLDARCU1BHMRAwDgYDVQQDDAdoai5qZW9uMFYwEAYHKoZIzj0CAQYFK4EE\r\n"
                                "AAoDQgAE03hxA+UI0hdVTuqaDTtJiiPJSiTKUfUTduHc8YvDRrDXbapmnozqQRnZ\r\n"
                                "pkbYj5iiOeeUsi42j2i92ScKZ+5/NKNQME4wHQYDVR0OBBYEFMGZkwZ1QY/RzfLx\r\n"
                                "hV0CAJ72s76BMB8GA1UdIwQYMBaAFMGZkwZ1QY/RzfLxhV0CAJ72s76BMAwGA1Ud\r\n"
                                "EwQFMAMBAf8wCgYIKoZIzj0EAwIDSQAwRgIhALM1tor+/1WWUXzE0lI+ilaRVE8F\r\n"
                                "qvzGWQhL7c23V2MzAiEAzg1YawmnPSyXdVuZl2BChUH7KrdJURPJaJwQCVaXAS4=\r\n"
                                "-----END CERTIFICATE-----\r\n";
#endif

static const uint8 host_ecdsa_pub[] = {
    0x7d, 0xd6, 0x17, 0x13, 0x37, 0x34, 0xf8, 0x6a,
    0xd5, 0x57, 0x46, 0x94, 0xb9, 0xee, 0x84, 0x1e,
    0xa3, 0x0c, 0x78, 0x32, 0x04, 0xc2, 0x9b, 0xc2,
    0xf4, 0x07, 0x0c, 0x7f, 0xd0, 0xa8, 0xdf, 0xa9,
    0x65, 0xbd, 0xe9, 0xc4, 0x15, 0x45, 0xaa, 0x57,
    0x08, 0xee, 0x33, 0x6c, 0xf7, 0x34, 0x85, 0x02,
    0x52, 0xef, 0x83, 0x64, 0x0e, 0x92, 0xca, 0x7d,
    0x93, 0xd5, 0xef, 0xd8, 0x87, 0xb6, 0xbc, 0x1e
};

static const uint8 host_ecdsa_sig[] = {
    0x30, 0x45, 0x02, 0x21, 0x00, 0xd3, 0x88, 0xa8,
    0x05, 0x34, 0x0c, 0xa6, 0x80, 0x96, 0x0e, 0xb6,
    0x9c, 0x93, 0x78, 0xf9, 0x6a, 0x80, 0xcd, 0x20,
    0x85, 0xd2, 0x8e, 0x68, 0x3e, 0x24, 0x9a, 0x41,
    0xa2, 0x82, 0xd2, 0xa6, 0x98, 0x02, 0x20, 0x31,
    0x34, 0xc0, 0xf3, 0xe4, 0xf1, 0x40, 0xc1, 0x3b,
    0x6b, 0x71, 0x92, 0xf3, 0x89, 0xb6, 0x14, 0xf8,
    0xbf, 0xac, 0x02, 0xbb, 0x78, 0x37, 0x13, 0x83,
    0xba, 0x5a, 0x12, 0x09, 0xb9, 0x92, 0x60
};

uint8 benchmarkBuf[0x8000];

#define TESTAPP_MAJOR_VER (2)
#define TESTAPP_MINOR_VER (12)

typedef struct
{
    uint32 start;
    uint32 end;
    uint32 timeDiff;
    uint32 iteration;
    uint32 speed;
} hsmBenchMark;

static const int8 *hsm_cmd[] = {
    "0 set key from nvm",
    "1 run aes",
    "2 run aes by kt",
    "3 gen mac",
    "4 gen mac by kt",
    "5 gen sha",
    "6 run ecdsa sign & verify",
    "7 run rsassa sign & verify",
    "8 compute ecdh pubkey",
    "9 ecdh phase I",
    "10 ecdh phase II",
    "11 kdf",
    "12 certificate parse",
    "13 certificate verify",
    "14 gen random number",
    "15 write nvm",
    "16 mbox ecc",
    "17 select bank",
    "18 get version",
    "19 full test",
    "20 aging(10,000)",
    "21 Benchmark Test",
    NULL
};
// clang-format on

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void HSM_PrintCmd(void)
{
    int32 i = 0;

    BLOG("\ncommand for hsm\n\n");

    for (i = 0; hsm_cmd[i] != NULL; i++) {
        BLOG("  %s\n", hsm_cmd[i]);
    }

    BLOG("\n");

    return;
}

static void HSM_HexDump(uint8 *pData, uint32 uiSize)
{
    uint8 ascii[17] = {0};
    uint32 i = 0;
    uint32 j = 0;
    ascii[16] = 0;
    for (i = 0; i < uiSize; ++i) {
        BLOG("%02X ", ((const uint8 *)pData)[i]);
        if ((((uint8 *)pData)[i] >= 0x20U) && (((uint8 *)pData)[i] <= 0x7EU)) {
            ascii[i % 16U] = ((const uint8 *)pData)[i];
        } else {
            ascii[i % 16U] = 0x2EU;
        }
        if ((((i + 1U) % 8U) == 0U) || ((i + 1U) == uiSize)) {
            BLOG(" ");
            if (((i + 1U) % 16U) == 0U) {
                BLOG("|  %s \n", ascii);
            } else if ((i + 1U) == uiSize) {
                ascii[(i + 1U) % 16U] = 0U;
                if (((i + 1U) % 16U) <= 8U) {
                    BLOG(" ");
                }
                for (j = ((i + 1U) % 16U); j < 16U; ++j) {
                    BLOG("   ");
                }
                BLOG("|  %s \n", ascii);
            } else {
            }
        }
    }
    BLOG("\n");
}

static int32 HSM_SetKeyTest(void)
{
    int32 ret = HSM_ERR;
    hsmKey dfKey = {0};

    /* Set key information */
    dfKey.keyIdx = HSM_R5_AES_KEY_INDEX;
    dfKey.dfAddr = HSM_DF_AESKEY_ADDR;
    dfKey.keySize = sizeof(aes_key);

    ret = HSM_SetKey(&dfKey);
    if (ret != HSM_OK) {
        ELOG("HSM_SetKeyTest Err(%d), aes addr=0x%x\n", ret, dfKey.dfAddr);
    }

    dfKey.keyIdx = HSM_R5_MAC_KEY_INDEX;
    dfKey.dfAddr = HSM_DF_MACKEY_ADDR;
    dfKey.keySize = sizeof(mac_key);
    ret = HSM_SetKey(&dfKey);
    if (ret != HSM_OK) {
        ELOG("HSM_SetKey test fail(%d), mac addr=0x%x\n", ret, dfKey.dfAddr);
        return ret;
    }

    DLOG("HSM_SetKey(aes df addr=0x%x mac df addr=0x%x) Success\n", HSM_DF_AESKEY_ADDR, HSM_DF_MACKEY_ADDR);
    return ret;
}

static int32 HSM_RunAesTest(void)
{
    hsmAes aes = {0};
    uint8 encrypted[32] = {0};
    uint32 enc_size = sizeof(encrypted);
    uint8 decrypted[32] = {0};
    uint32 dec_size = sizeof(decrypted);
    uint8 tag[16] = {0};
    int32 ret = HSM_ERR;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    /* AES ECB Test */
    /* Mode: ECB, ENCRPTION */
    aes.objId = HSM_AES_ECB_128 | HSM_AES_ENCRYPT;
    aes.opMode = OPMODE_SINGLECALL;
    aes.keySize = sizeof(aes_key);
    SAL_MemCopy((void *)aes.aesKey, (const void *)aes_key, aes.keySize);
    aes.ivSize = sizeof(aes_iv);
    SAL_MemCopy((void *)aes.aesIv, (const void *)aes_iv, aes.ivSize);
    aes.srcSize = sizeof(plain_data);
    aes.src = plain_data;
    aes.dstSize = enc_size;
    aes.dst = encrypted;
    ret = HSM_RunAes(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }

    /* Mode: ECB, DECRPTION */
    aes.objId = HSM_AES_ECB_128 | HSM_AES_DECRYPT;
    aes.opMode = OPMODE_SINGLECALL;
    aes.src = encrypted;
    aes.srcSize = enc_size;
    aes.dst = decrypted;
    aes.dstSize = dec_size;
    ret = HSM_RunAes(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }
    sal_ret = SAL_MemCmp((const void *)plain_data, (const void *)aes.dst, (SALSize)dec_size, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump((uint8 *)plain_data, dec_size);
        HSM_HexDump((uint8 *)aes.dst, dec_size);
        ELOG("Wrong cipher data(%d)\n", ret);
        return HSM_ERR;
    }
    /* AES CCM Test */
    /* Mode: CCM, ENCRPTION */
    aes.objId = HSM_AES_CCM_128 | HSM_AES_ENCRYPT;
    aes.opMode = OPMODE_SINGLECALL;
    aes.keySize = sizeof(aes_key);
    SAL_MemCopy((void *)aes.aesKey, (const void *)aes_key, aes.keySize);
    aes.ivSize = sizeof(aead_iv);
    SAL_MemCopy((void *)aes.aesIv, (const void *)aead_iv, aes.ivSize);
    aes.srcSize = sizeof(plain_data);
    aes.src = plain_data;
    aes.dstSize = enc_size;
    aes.dst = encrypted;
    aes.tagSize = sizeof(tag);
    aes.tag = tag;
    aes.aadSize = sizeof(aes_aad);
    aes.aad = aes_aad;
    ret = HSM_RunAes(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }
    /* Mode: CCM, DECRPTION */
    aes.objId = HSM_AES_CCM_128 | HSM_AES_DECRYPT;
    aes.opMode = OPMODE_SINGLECALL;
    aes.src = encrypted;
    aes.srcSize = enc_size;
    aes.dst = decrypted;
    aes.dstSize = dec_size;
    ret = HSM_RunAes(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }
    sal_ret = SAL_MemCmp((const void *)plain_data, (const void *)decrypted, dec_size, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(plain_data, dec_size);
        HSM_HexDump(decrypted, dec_size);
        ELOG("Wrong cipher data(%d)\n", ret);
        return HSM_ERR;
    }

    /* AES GCM Test */
    /* Mode: GCM, ENCRPTION */
    aes.objId = HSM_AES_GCM_128 | HSM_AES_ENCRYPT;
    aes.opMode = OPMODE_SINGLECALL;
    aes.keySize = sizeof(aes_key);
    SAL_MemCopy((void *)aes.aesKey, (const void *)aes_key, aes.keySize);
    aes.ivSize = sizeof(aead_iv);
    SAL_MemCopy((void *)aes.aesIv, (const void *)aead_iv, aes.ivSize);
    aes.srcSize = sizeof(plain_data);
    aes.src = plain_data;
    aes.dstSize = enc_size;
    aes.dst = encrypted;
    aes.tagSize = sizeof(tag);
    aes.tag = tag;
    aes.aadSize = sizeof(aes_aad);
    aes.aad = aes_aad;
    ret = HSM_RunAes(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }
    /* Mode: GCM, DECRPTION */
    aes.objId = HSM_AES_GCM_128 | HSM_AES_DECRYPT;
    aes.opMode = OPMODE_SINGLECALL;
    aes.src = encrypted;
    aes.srcSize = enc_size;
    aes.dst = decrypted;
    aes.dstSize = dec_size;
    ret = HSM_RunAes(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }

    sal_ret = SAL_MemCmp((const void *)plain_data, (const void *)decrypted, dec_size, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(plain_data, dec_size);
        HSM_HexDump(decrypted, dec_size);
        ELOG("Wrong cipher data(%d)\n", ret);
        return HSM_ERR;
    }

    DLOG("RUN_AES Success(%d)\n", ret);

    return ret;
}

static int32 HSM_RunAesByKtTest(void)
{
    hsmAes aes = {0};
    uint8 encrypted[32] = {0};
    uint32 enc_size = sizeof(encrypted);
    uint8 decrypted[32] = {0};
    uint32 dec_size = sizeof(decrypted);
    uint8 tag[16] = {0};
    int32 ret = HSM_ERR;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    /* AES ECB */
    /* Mode: ECB, ENCRPTION */
    aes.objId = HSM_AES_ECB_128 | HSM_AES_ENCRYPT;
    aes.keyIdx = HSM_R5_AES_KEY_INDEX;
    aes.ivSize = sizeof(aes_iv);
    SAL_MemCopy((void *)aes.aesIv, (const void *)aes_iv, aes.ivSize);
    aes.srcSize = sizeof(plain_data);
    aes.src = plain_data;
    aes.dstSize = enc_size;
    aes.dst = encrypted;

    ret = HSM_RunAesByKt(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAesByKt test fail(%d)\n", ret);
        return HSM_ERR;
    }
    /* Mode: ECB, DECRPTION */
    aes.objId = HSM_AES_ECB_128 | HSM_AES_DECRYPT;
    aes.src = encrypted;
    aes.srcSize = enc_size;
    aes.dst = decrypted;
    aes.dstSize = dec_size;
    ret = HSM_RunAesByKt(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAesByKt test fail(%d)\n", ret);
        return HSM_ERR;
    }
    sal_ret = SAL_MemCmp((const void *)plain_data, (const void *)aes.dst, (SALSize)dec_size, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump((uint8 *)plain_data, dec_size);
        HSM_HexDump((uint8 *)aes.dst, dec_size);
        ELOG("Wrong cipher data(%d)\n", ret);
        return HSM_ERR;
    }

    /* AES CCM */
    /* Mode: CCM, ENCRPTION */
    aes.objId = HSM_AES_CCM_128 | HSM_AES_ENCRYPT;
    aes.keyIdx = HSM_R5_AES_KEY_INDEX;
    aes.ivSize = sizeof(aead_iv);
    SAL_MemCopy((void *)aes.aesIv, (const void *)aead_iv, aes.ivSize);
    aes.srcSize = sizeof(plain_data);
    aes.src = plain_data;
    aes.dstSize = enc_size;
    aes.dst = encrypted;
    aes.tagSize = sizeof(tag);
    aes.tag = tag;
    aes.aadSize = sizeof(aes_aad);
    aes.aad = aes_aad;

    ret = HSM_RunAesByKt(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAesByKt test fail(%d)\n", ret);
        return HSM_ERR;
    }
    /* Mode: CCM, DECRPTION */
    aes.objId = HSM_AES_CCM_128 | HSM_AES_DECRYPT;
    aes.keyIdx = HSM_R5_AES_KEY_INDEX;
    aes.src = encrypted;
    aes.srcSize = enc_size;
    aes.dst = decrypted;
    aes.dstSize = dec_size;
    ret = HSM_RunAesByKt(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAesByKt test fail(%d)\n", ret);
        return HSM_ERR;
    }
    sal_ret = SAL_MemCmp((const void *)plain_data, (const void *)aes.dst, dec_size, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(plain_data, dec_size);
        HSM_HexDump(aes.dst, dec_size);
        ELOG("Wrong cipher data(%d)\n", ret);
        return HSM_ERR;
    }

    /* AES GCM */
    /* Mode: GCM, ENCRPTION */
    aes.objId = HSM_AES_GCM_128 | HSM_AES_ENCRYPT;
    aes.keyIdx = HSM_R5_AES_KEY_INDEX;
    aes.ivSize = sizeof(aead_iv);
    SAL_MemCopy((void *)aes.aesIv, (const void *)aead_iv, aes.ivSize);
    aes.srcSize = sizeof(plain_data);
    aes.src = plain_data;
    aes.dstSize = enc_size;
    aes.dst = encrypted;
    aes.tagSize = sizeof(tag);
    aes.tag = tag;
    aes.aadSize = sizeof(aes_aad);
    aes.aad = aes_aad;

    ret = HSM_RunAesByKt(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAesByKt test fail(%d)\n", ret);
        return HSM_ERR;
    }
    /* Mode: GCM, DECRPTION */
    aes.objId = HSM_AES_GCM_128 | HSM_AES_DECRYPT;
    aes.src = encrypted;
    aes.srcSize = enc_size;
    aes.dst = decrypted;
    aes.dstSize = dec_size;
    ret = HSM_RunAesByKt(&aes);
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }

    sal_ret = SAL_MemCmp((const void *)plain_data, (const void *)aes.dst, dec_size, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(plain_data, dec_size);
        HSM_HexDump(aes.dst, dec_size);
        ELOG("Wrong cipher data(%d)\n", ret);
        return HSM_ERR;
    }

    DLOG("RUN_AES_BY_KT Success(%d)\n", ret);

    return ret;
}

static int32 HSM_GenMacTest(void)
{
    hsmMac mac = {0};
    int32 ret = HSM_ERR;

    mac.req = REQ_HSM_GEN_CMAC;
    mac.opMode = OPMODE_SINGLECALL;
    mac.objId = HSM_OID_CMAC_AES128;
    mac.srcSize = sizeof(plain_data);
    mac.src = plain_data;
    mac.keySize = sizeof(aes_key);
    SAL_MemCopy((void *)mac.macKey, (const void *)aes_key, mac.keySize);
    mac.ivSize = sizeof(aes_iv);
    SAL_MemCopy((void *)mac.macIv, (const void *)aes_iv, mac.ivSize);
    mac.macSize = HSM_MAX_CMAC_SIZE;

    ret = HSM_GenMac(&mac);
    if (ret != HSM_OK) {
        ELOG("hsm_gen_cmac test fail(%d)\n", ret);
        return HSM_ERR;
    }
    mac.req = REQ_HSM_VERIFY_CMAC;
    ret = HSM_Verifymac(&mac);
    if (ret != HSM_OK) {
        ELOG("HSM_Verify cmac test fail(%d)\n", ret);
        return HSM_ERR;
    }

    DLOG("TCCHSM_GEN_CMAC and TCCHSM_VERIFY_CMAC Success(%d)\n", ret);

    mac.req = HSM_REQ_GEN_HMAC;
    mac.opMode = OPMODE_SINGLECALL;
    mac.objId = HSM_OID_HMAC_SHA2_256;
    mac.srcSize = sizeof(plain_data);
    mac.src = plain_data;
    mac.keySize = sizeof(aes_key);
    SAL_MemCopy((void *)mac.macKey, (const void *)aes_key, mac.keySize);
    mac.ivSize = sizeof(aes_iv);
    SAL_MemCopy((void *)mac.macIv, (const void *)aes_iv, mac.ivSize);
    mac.macSize = HSM_MAX_HMAC_SIZE;

    ret = HSM_GenMac(&mac);
    if (ret != HSM_OK) {
        ELOG("hsm_gen_hmac test fail(%d)\n", ret);
        return HSM_ERR;
    }

    mac.req = HSM_REQ_VERIFY_HMAC;
    mac.opMode = OPMODE_SINGLECALL;
    ret = HSM_Verifymac(&mac);
    if (ret != HSM_OK) {
        ELOG("HSM_Verify hmac test fail(%d)\n", ret);
        return HSM_ERR;
    }

    DLOG("HSM_Verify hmac Success(%d)\n", ret);

    return ret;
}

static int32 HSM_GenMacByKtTest(void)
{
    hsmMac mac = {0};
    int32 ret = HSM_ERR;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    mac.req = REQ_HSM_GEN_CMAC_BY_KT;
    mac.opMode = OPMODE_SINGLECALL;
    mac.objId = HSM_OID_CMAC_AES128;
    mac.keyIdx = HSM_R5_AES_KEY_INDEX;
    mac.ivSize = sizeof(aes_iv);
    SAL_MemCopy((void *)mac.macIv, (const void *)aes_iv, mac.ivSize);
    mac.srcSize = sizeof(plain_data);
    mac.src = plain_data;
    mac.macSize = HSM_MAX_CMAC_SIZE;

    ret = HSM_GenMacByKt(&mac);
    if (ret != HSM_OK) {
        ELOG("HSM_GenMacByKt test fail(%d)\n", ret);
        return HSM_ERR;
    }
    sal_ret = SAL_MemCmp((const void *)cmac_out, (const void *)mac.mac, mac.macSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(cmac_out, mac.macSize);
        HSM_HexDump(mac.mac, mac.macSize);
        ELOG("Wrong cipher data(%d)\n", ret);
        return HSM_ERR;
    }
    DLOG("TCCHSM_GEN_CMAC_BY_KT Success(%d)\n", ret);

    mac.req = HSM_REQ_GEN_HMAC_BY_KT;
    mac.opMode = OPMODE_SINGLECALL;
    mac.objId = HSM_OID_HMAC_SHA2_256;
    mac.keyIdx = HSM_R5_MAC_KEY_INDEX;
    mac.ivSize = sizeof(aes_iv);
    SAL_MemCopy((void *)mac.macIv, (const void *)aes_iv, mac.ivSize);
    mac.srcSize = sizeof(plain_data);
    mac.src = plain_data;
    mac.macSize = HSM_MAX_HMAC_SIZE;

    ret = HSM_GenMacByKt(&mac);
    if (ret != HSM_OK) {
        ELOG("HSM_GenMacByKt test fail(%d)\n", ret);
        return HSM_ERR;
    }
    sal_ret = SAL_MemCmp((const void *)hmac_out, (const void *)mac.mac, mac.macSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(hmac_out, mac.macSize);
        HSM_HexDump(mac.mac, mac.macSize);
        ELOG("Wrong cipher data(%d)\n", ret);
        return HSM_ERR;
    }

    DLOG("TCCHSM_GEN_HMAC_BY_KT Success(%d)\n", ret);

    return ret;
}

static int32 HSM_GenHashTest(void)
{
    hsmSha sha = {0};
    int32 ret = HSM_ERR;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    sha.objId = HSM_OID_SHA2_256; // Only support SHA256
    sha.opMode = OPMODE_SINGLECALL;
    sha.srcSize = sizeof(hash_message);
    sha.src = hash_message;
    sha.digSize = sizeof(digest_sha2_256);
    ret = HSM_GenHash(&sha);
    if (ret != HSM_OK) {
        ELOG("HSM_GenHash test fail(%d)\n", ret);
        return HSM_ERR;
    }
    sal_ret = SAL_MemCmp((const void *)digest_sha2_256, (const void *)sha.dig, sha.digSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(digest_sha2_256, sha.digSize);
        HSM_HexDump(sha.dig, sha.digSize);
        ELOG("Wrong cipher data(%d)\n", ret);
        return HSM_ERR;
    }

    DLOG("TCCHSM_GEN_SHA Success(%d)\n", ret);

    return ret;
}

static int32 HSM_RunEcdsaTest(void)
{
    hsmAsymKey asymKey = {0};
    hsmAsym ecdsa = {0};
    uint8 sigBuf[HSM_ECDH_P256_SIG_SIZE] = {0};
    int32 ret = HSM_ERR;

    /* Generate key */
    asymKey.objId = HSM_OID_ECC_P256;
    asymKey.keyType = 0u;
    asymKey.priSize = HSM_ECDH_P256_KEY_SIZE;
    asymKey.pubSize = (HSM_ECDH_P256_KEY_SIZE * 2u);
    asymKey.seedSize = 0; // Not mandatory
    ret = HSM_RunEcdhPhaseI(&asymKey);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa signing test fail(%d)\n", ret);
        return ret;
    }

    /*NIST P256 TEST*/
    /* Signing */
    ecdsa.req = HSM_REQ_RUN_ECDSA_SIGN;
    ecdsa.objId = (HSM_OID_SHA2_256 | HSM_OID_ECC_P256);
    ecdsa.opMode = OPMODE_SINGLECALL;
    ecdsa.priSize = asymKey.priSize;
    ecdsa.priKey = asymKey.priKey;
    ecdsa.pubSize = asymKey.pubSize;
    ecdsa.pubKey = asymKey.pubKey;
    ecdsa.digSize = sizeof(gzu08_digest);
    ecdsa.dig = gzu08_digest;
    ecdsa.sigSize = HSM_ECDH_P256_SIG_SIZE;
    ecdsa.sig = sigBuf;

    ret = HSM_RunEcdsa(&ecdsa);

    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa nist p256 signing test fail(%d)\n", ret);
        return ret;
    }
    /* Verify */
    ecdsa.req = HSM_REQ_RUN_ECDSA_VERIFY;
    ecdsa.objId = (HSM_OID_SHA2_256 | HSM_OID_ECC_P256);

    ret = HSM_RunEcdsa(&ecdsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa nist p256 verify test fail(%d)\n", ret);
        return ret;
    }

    DLOG("nist p256 ecdsa sign and verify success(%d) \n", ret);

    /*BP256 TEST*/
    /* Generate key */
    asymKey.objId = HSM_OID_ECC_BP256;
    asymKey.keyType = 0u;
    asymKey.priSize = HSM_ECDH_P256_KEY_SIZE;
    asymKey.pubSize = (HSM_ECDH_P256_KEY_SIZE * 2u);
    asymKey.seedSize = 0; // Not mandatory
    ret = HSM_RunEcdhPhaseI(&asymKey);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa signing test fail(%d)\n", ret);
        return ret;
    }

    /* Signing */
    ecdsa.req = HSM_REQ_RUN_ECDSA_SIGN;
    ecdsa.objId = (HSM_OID_SHA2_256 | HSM_OID_ECC_BP256);
    ecdsa.opMode = OPMODE_SINGLECALL;
    ecdsa.priSize = asymKey.priSize;
    SAL_MemCopy((void *)ecdsa.priKey, (const void *)asymKey.priKey, asymKey.priSize);
    ecdsa.pubSize = asymKey.pubSize;
    SAL_MemCopy((void *)ecdsa.pubKey, (const void *)asymKey.pubKey, asymKey.pubSize);
    ecdsa.digSize = sizeof(gzu08_digest);
    ecdsa.dig = gzu08_digest;
    ecdsa.sigSize = HSM_ECDH_P256_SIG_SIZE;

    ret = HSM_RunEcdsa(&ecdsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa bp256 signing test fail(%d)\n", ret);
        return ret;
    }
    /* Verify */
    ecdsa.req = HSM_REQ_RUN_ECDSA_VERIFY;
    ecdsa.objId = (HSM_OID_SHA2_256 | HSM_OID_ECC_BP256);

    ret = HSM_RunEcdsa(&ecdsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa bp256 verify test fail(%d)\n", ret);
        return ret;
    }
    DLOG("BP256 ecdsa sign and verify success(%d) \n", ret);

    return ret;
}
#if (ENABLE_RSA)
static int32 HSM_RunRsassaTest(void)
{
    hsmAsym rsa = {0};
    uint8 sigBuf[256] = {0};
    int32 ret = HSM_ERR;

    /* Sign - PKCS 1024bit*/
    rsa.req = REQ_HSM_RUN_RSASSA_PKCS_SIGN;
    rsa.objId = HSM_OID_SHA2_256;
    rsa.opMode = OPMODE_SINGLECALL;
    rsa.priSize = sizeof(rsassa_d);
    rsa.priKey = rsassa_d;
    rsa.modSize = sizeof(rsassa_n);
    rsa.modulus = rsassa_n;
    rsa.pubSize = sizeof(rsassa_e);
    rsa.pubKey = rsassa_e;
    rsa.digSize = sizeof(gzu08_digest);
    rsa.dig = gzu08_digest;
    rsa.sigSize = sizeof(sigBuf);
    rsa.sig = sigBuf;

    ret = HSM_RunRsassa(&rsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunRsassa pkcs1 sign test fail(%d)\n", ret);
        return ret;
    }

    /* Verify - PKCS 1024bit*/
    rsa.req = REQ_HSM_RUN_RSASSA_PKCS_VERIFY;
    ret = HSM_RunRsassa(&rsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunRsassa pkcs1 verify test fail(%d)\n", ret);
        return ret;
    }
    DLOG("RSASSA(1024bit) pkcs1 sign and verify success(%d) \n", ret);

    /* Sign - PSS 1024bit*/
    rsa.req = REQ_HSM_RUN_RSASSA_PSS_SIGN;
    rsa.objId = HSM_OID_SHA2_256;
    rsa.opMode = OPMODE_SINGLECALL;
    rsa.priSize = sizeof(rsassa_d);
    rsa.priKey = rsassa_d;
    rsa.modSize = sizeof(rsassa_n);
    rsa.modulus = rsassa_n;
    rsa.pubSize = sizeof(rsassa_e);
    rsa.pubKey = rsassa_e;
    rsa.digSize = sizeof(gzu08_digest);
    rsa.dig = gzu08_digest;
    rsa.sigSize = sizeof(sigBuf);
    rsa.sig = sigBuf;

    ret = HSM_RunRsassa(&rsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunRsassa pss sign test fail(%d)\n", ret);
        return ret;
    }

    /* Verify - PSS 1024bit*/
    rsa.req = REQ_HSM_RUN_RSASSA_PSS_VERIFY;
    ret = HSM_RunRsassa(&rsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunRsassa pss verify test fail(%d)\n", ret);
        return ret;
    }
    DLOG("RSASSA(1024bit) pss sign and verify success(%d) \n", ret);

    return ret;
}
#endif
static int32 HSM_ComputeEcdhPubKeyTest(void)
{
    hsmAsymKey asymKey = {0};
    int32 ret = HSM_ERR;
    int32 sal_ret = HSM_ERR;

    /* Generate key */
    asymKey.keyType = 0u;
    asymKey.objId = HSM_OID_ECC_P256;
    asymKey.priSize = sizeof(g_NIST_P256_SHA256_pri_key);
    asymKey.pubSize = sizeof(g_NIST_P256_shared_pubKey);
    SAL_MemCopy((void *)asymKey.priKey, (const void *)g_NIST_P256_SHA256_pri_key, asymKey.priSize);
    ret = HSM_ComputeEcdhPubKey(&asymKey);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa signing test fail(%d)\n", ret);
        return ret;
    }

    sal_ret = SAL_MemCmp(
        (const void *)asymKey.pubKey, (const void *)g_NIST_P256_shared_pubKey, (SALSize)asymKey.pubSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump((uint8 *)asymKey.pubKey, asymKey.pubSize);
        HSM_HexDump((uint8 *)g_NIST_P256_shared_pubKey, asymKey.pubSize);
        ELOG("Wrong pubkey data(%d)\n", ret);
        return HSM_ERR;
    }

    DLOG("HSM_ComputeEcdhPubKeyTest success(%d)\n", ret);

    return ret;
}

static int32 HSM_EcdhPhaseITest(void)
{
    hsmAsymKey asymKey = {0};
    int32 ret = HSM_ERR;

    /* Generate key */
    asymKey.objId = HSM_OID_ECC_P256;
    asymKey.keyType = 0u;
    asymKey.priSize = HSM_ECDH_P256_KEY_SIZE;
    asymKey.pubSize = (HSM_ECDH_P256_KEY_SIZE * 2u);
    asymKey.seedSize = 0; // Not mandatory
    ret = HSM_RunEcdhPhaseI(&asymKey);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdhPhaseI signing test fail(%d)\n", ret);
        return ret;
    }

    DLOG("HSM_EcdhPhaseI Test success(%d)\n", ret);

    return ret;
}

static int32 HSM_EcdhPhaseIITest(void)
{
    hsmAsymKey asymKey = {0};
    int32 ret = HSM_ERR;
    int32 sal_ret = HSM_ERR;

    /* Generate key */
    asymKey.keyType = 0u;
    asymKey.objId = HSM_OID_ECC_P256;
    asymKey.pubSize = (HSM_ECDH_P256_KEY_SIZE * 2u);
    SAL_MemCopy((void *)asymKey.pubKey, (const void *)g_NIST_P256_shared_pubKey, asymKey.pubSize);
    asymKey.priSize = HSM_ECDH_P256_KEY_SIZE;
    SAL_MemCopy((void *)asymKey.priKey, (const void *)g_NIST_P256_pri_key, asymKey.priSize);
    asymKey.secSize = HSM_ECDH_P256_KEY_SIZE;

    ret = HSM_RunEcdhPhaseII(&asymKey);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdhPhaseII test fail(%d)\n", ret);
        return ret;
    }

    sal_ret = SAL_MemCmp(
        (const void *)asymKey.secKey, (const void *)g_NIST_P256_shared_secretKey, (SALSize)asymKey.secSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump((uint8 *)asymKey.secKey, asymKey.secSize);
        HSM_HexDump((uint8 *)g_NIST_P256_shared_secretKey, asymKey.secSize);
        ELOG("Wrong pubkey data(%d)\n", ret);
        return HSM_ERR;
    }

    DLOG("HSM_EcdhPhaseIITest success(%d)\n", ret);

    return ret;
}

static int32 HSM_Pbkdf2Test(void)
{
    hsmKdf kdf = {0};
    int32 ret = HSM_ERR;
    int32 sal_ret = HSM_ERR;

    /* Generate key */
    kdf.mdAlg = HSM_KDF_SHA2_256;
    kdf.pSize = pwLen;
    SAL_MemCopy((void *)kdf.pw, (const void *)pw_data, kdf.pSize);

    kdf.saltSize = saltLen;
    SAL_MemCopy((void *)kdf.salt, (const void *)salt_data, kdf.saltSize);

    kdf.iteration = kdf_iteration;
    kdf.keySize = sizeof(result_key_data);

    ret = HSM_Pbkdf2(&kdf);
    if (ret != HSM_OK) {
        ELOG("HSM_Pbkdf2 test fail(%d)\n", ret);
        return ret;
    }

    sal_ret = SAL_MemCmp((const void *)kdf.key, (const void *)result_key_data, (SALSize)kdf.keySize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump((uint8 *)kdf.key, kdf.keySize);
        HSM_HexDump((uint8 *)result_key_data, kdf.keySize);
        ELOG("Wrong out data(%d)\n", ret);
        return HSM_ERR;
    }

    DLOG("HSM_Pbkdf2 Test success(%d)\n", ret);

    return ret;
}

static int32 HSM_CertiParseTest(void)
{
    hsmCerti certi = {0};
    uint8 pubKey[64] = {0};
    uint8 signature[72] = {0};
    int32 ret = HSM_ERR;
    int32 sal_ret = HSM_ERR;

    certi.crtSize = sizeof(host_ecdsa_crt);
    certi.crt = (uint8 *)host_ecdsa_crt;
    certi.pubSize = sizeof(pubKey);
    certi.pubKey = pubKey;
    certi.sigSize = 0;
    certi.sig = NULL;

    ret = HSM_CertiParse(&certi);
    if (ret != HSM_OK) {
        ELOG("HSM_CertiParse test fail(%d)\n", ret);
        return ret;
    }

    sal_ret = SAL_MemCmp(
        (const void *)certi.pubKey, (const void *)host_ecdsa_pub, (SALSize)certi.pubSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0) || (certi.pubSize == 0)) {
        HSM_HexDump((uint8 *)certi.pubKey, certi.pubSize);
        HSM_HexDump((uint8 *)host_ecdsa_pub, certi.pubSize);
        ELOG("Wrong pub data(%d)\n", ret);
        return HSM_ERR;
    }
    DLOG("HSM_CertiParse only Pubkey success(%d)\n", ret);

    certi.sigSize = sizeof(signature);
    certi.sig = signature;

    ret = HSM_CertiParse(&certi);
    if (ret != HSM_OK) {
        ELOG("HSM_CertiParse test fail(%d)\n", ret);
        return ret;
    }

    sal_ret = SAL_MemCmp(
        (const void *)certi.pubKey, (const void *)host_ecdsa_pub, (SALSize)certi.pubSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0) || (certi.pubSize == 0)) {
        HSM_HexDump((uint8 *)certi.pubKey, certi.pubSize);
        HSM_HexDump((uint8 *)host_ecdsa_pub, certi.pubSize);
        ELOG("Wrong pub data(%d)\n", ret);
        return HSM_ERR;
    }

    if (certi.sig != NULL && certi.sigSize != 0) {
        sal_ret = SAL_MemCmp(
            (const void *)certi.sig, (const void *)host_ecdsa_sig, (SALSize)certi.sigSize, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0) || (certi.sigSize == 0)) {
            HSM_HexDump((uint8 *)certi.sig, certi.sigSize);
            HSM_HexDump((uint8 *)host_ecdsa_sig, certi.sigSize);
            ELOG("Wrong sig data(%d)\n", ret);
            return HSM_ERR;
        }
    }

    DLOG("HSM_CertiParse Test success(%d)\n", ret);

    return ret;
}

static int32 HSM_CertiVerifyTest(void)
{
    hsmCerti root = {0};
    hsmCerti target = {0};
    int32 ret = HSM_ERR;

    root.crt = (uint8 *)root_ecdsa_crt;
    root.crtSize = sizeof(root_ecdsa_crt);
    target.crt = (uint8 *)host_ecdsa_crt;
    target.crtSize = sizeof(host_ecdsa_crt);

    ret = HSM_CertiVerify(&root, &target);
    if (ret != HSM_OK) {
        ELOG("HSM_CertiVerify test fail(%d)\n", ret);
        return ret;
    }

    DLOG("HSM_CertiVerify Test success(%d)\n", ret);

    return ret;
}

static int32 HSM_GetRngTest(void)
{
    hsmRng rng = {0};
    int32 ret = HSM_ERR;

    rng.rngSize = HSM_MAX_RAND_SIZE;
    ret = HSM_TRNG(&rng);
    if (ret != HSM_OK) {
        ELOG("HSM_GetTRNG test fail(%d)\n", ret);
        return ret;
    }
    DLOG("HSM_GetTRNG Test Success\n");
    HSM_HexDump((uint8 *)rng.rng, rng.rngSize);

    ret = HSM_PRNG(&rng);
    if (ret != HSM_OK) {
        ELOG("HSM_GetPRNG test fail(%d)\n", ret);
        return ret;
    }

    DLOG("HSM_GetPRNG Test Success\n");
    HSM_HexDump((uint8 *)rng.rng, rng.rngSize);

    rng.rngSize = HSM_MAX_RAND_SIZE;
    rng.seedSize = sizeof(rng_seed);
    SAL_MemCopy((void *)rng.seed, (const void *)rng_seed, rng.seedSize);
    ret = HSM_PRNG(&rng);
    if (ret != HSM_OK) {
        ELOG("HSM_GetPRNG_seed test fail(%d)\n", ret);
        return ret;
    }

    DLOG("HSM_GetPRNG_seed Test Success\n");
    HSM_HexDump((uint8 *)rng.rng, rng.rngSize);

    BLOG("\n");
    return ret;
}

static int32 HSM_WriteDFTest(void)
{
    hsmKey dfKey = {0};
    int32 ret = HSM_ERR;

    // write aes test key
    dfKey.dfAddr = HSM_DF_AESKEY_ADDR;
    dfKey.keySize = sizeof(aes_key);
    dfKey.keydata = aes_key;

    ret = HSM_DfWrite(&dfKey);
    if (ret != HSM_OK) {
        ELOG("HSM_WriteDFTest fail(%d) addr=0x%x\n", ret, dfKey.dfAddr);
    }

    // write mac test key
    dfKey.dfAddr = HSM_DF_MACKEY_ADDR;
    dfKey.keySize = sizeof(mac_key);
    dfKey.keydata = mac_key;

    ret = HSM_DfWrite(&dfKey);
    if (ret != HSM_OK) {
        ELOG("HSM_WriteDFTest fail(%d) addr=0x%x\n", ret, dfKey.dfAddr);
        return ret;
    }

    DLOG("HSM_DfWrite(aeskey_addr=0x%x mackey_addr=0x%x) Success\n", HSM_DF_AESKEY_ADDR, HSM_DF_MACKEY_ADDR);

    return ret;
}



static int32 HSM_SelectBankTest(int32 ibankIdx)
{
    int32 ret = HSM_ERR;
    uint32 bankIndex = 0xffffffff;

    if (ibankIdx < 2)
    {
        bankIndex = ibankIdx;
    }

    ret = HSM_SelectBank(bankIndex);
    if (ret != HSM_OK) {
        ELOG("HSM_SelectBank fail(%d) bank_index=0x%x\n", ret, bankIndex);
    }

    DLOG("HSM_SelectBankTest (bank_index=0x%x) Success\n", bankIndex);

    return ret;
}
static int32 HSM_GetVersionTest(void)
{
    hsmVer ver = {0};
    int32 ret = HSM_ERR;

    ret = HSM_GetVersion(&ver);
    if (ret != HSM_OK) {
        ELOG("HSM_GetVersion Err(%d)\n", ret);
        return ret;
    }

    DLOG("HSM FW Version %d.%d.%d\n", ver.x, ver.y, ver.z);

    return ret;
}
static void HSM_FullTest(int32 iCnt)
{
    int32 i = 0;
    int32 ret = HSM_OK;

    for (i = 0; i < iCnt; i++) {
        ret = HSM_WriteDFTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_SetKeyTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_RunAesTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_RunAesByKtTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_GenMacTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_GenMacByKtTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_GenHashTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_RunEcdsaTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
#if (ENABLE_RSA)
        ret = HSM_RunRsassaTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
#endif
        ret = HSM_ComputeEcdhPubKeyTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_EcdhPhaseITest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_EcdhPhaseIITest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_Pbkdf2Test();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_CertiParseTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_CertiVerifyTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_GetRngTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
        ret = HSM_GetVersionTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return;
        }
    }

    DLOG("Success test(%d, %d)\n", i, iCnt);

    return;
}

static int32 HSM_BenchmarkAesTest(void)
{
    hsmAes aes = {0};
    hsmBenchMark bench = {0};
    uint8 expectedDst[16] = {0};
    SALRetCode_t sal_ret = SAL_RET_FAILED;
    int32 ret = 0;
    uint32 i = 0;

    DLOG("BENCHMARK AES TEST\n");

    /* AES ECB Test */
    /* Mode: ECB, ENCRPTION */
    aes.objId = HSM_AES_ECB_128 | HSM_AES_ENCRYPT;
    aes.opMode = OPMODE_SINGLECALL;
    aes.keySize = sizeof(aes_key);
    SAL_MemCopy((void *)aes.aesKey, (const void *)aes_key, aes.keySize);
    aes.ivSize = sizeof(aes_iv);
    SAL_MemCopy((void *)aes.aesIv, (const void *)aes_iv, aes.ivSize);
    aes.srcSize = sizeof(benchmarkBuf);
    aes.src = benchmarkBuf;
    aes.dstSize = sizeof(benchmarkBuf);
    aes.dst = benchmarkBuf;

    bench.iteration = 512;
    SAL_GetTickCount(&bench.start);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_RunAes(&aes);
    }
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }
    SAL_GetTickCount(&bench.end);
    bench.timeDiff = (bench.end - bench.start);
    bench.speed = (((aes.srcSize * bench.iteration)) / bench.timeDiff);

    DLOG(
        "[AES128-ECB-Encryption]Input Size:%d MB(%d Kbyte * %d times)  Total Time:%d ms  Performance: %d Kbyte/s\n",
        (aes.srcSize * bench.iteration) / (1024u * 1024u), (aes.srcSize / 1024u), bench.iteration, bench.timeDiff,
        bench.speed);

    aes.objId = HSM_AES_ECB_128 | HSM_AES_DECRYPT;
    SAL_GetTickCount(&bench.start);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_RunAes(&aes);
    }
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }
    SAL_GetTickCount(&bench.end);
    bench.timeDiff = (bench.end - bench.start);
    bench.speed = (((aes.srcSize * bench.iteration)) / bench.timeDiff);

    sal_ret = SAL_MemCmp((const void *)benchmarkBuf, (const void *)expectedDst, sizeof(expectedDst), &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(benchmarkBuf, sizeof(expectedDst));
        HSM_HexDump(expectedDst, sizeof(expectedDst));
        ELOG("Wrong cipher data(%d)\n", ret);
        // return HSM_ERR;
    }

    DLOG(
        "[AES128-ECB-Decryption]Input Size:%d MB(%d Kbyte * %d times)  Total Time:%d ms  Performance: %d Kbyte/s\n",
        (aes.srcSize * bench.iteration) / (1024u * 1024u), (aes.srcSize / 1024u), bench.iteration, bench.timeDiff,
        bench.speed);

    aes.objId = HSM_AES_CBC_128 | HSM_AES_ENCRYPT;
    SAL_GetTickCount(&bench.start);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_RunAes(&aes);
    }
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }
    SAL_GetTickCount(&bench.end);
    bench.timeDiff = (bench.end - bench.start);
    bench.speed = (((aes.srcSize * bench.iteration)) / bench.timeDiff);

    DLOG(
        "[AES128-CBC-Encryption]Input Size:%d MB(%d Kbyte * %d times)  Total Time:%d ms  Performance: %d Kbyte/s\n",
        (aes.srcSize * bench.iteration) / (1024u * 1024u), (aes.srcSize / 1024u), bench.iteration, bench.timeDiff,
        bench.speed);

    aes.objId = HSM_AES_CBC_128 | HSM_AES_DECRYPT;
    SAL_GetTickCount(&bench.start);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_RunAes(&aes);
    }
    if (ret != HSM_OK) {
        ELOG("HSM_RunAes test fail(%d)\n", ret);
        return HSM_ERR;
    }
    SAL_GetTickCount(&bench.end);
    bench.timeDiff = (bench.end - bench.start);
    bench.speed = (((aes.srcSize * bench.iteration)) / bench.timeDiff);

    sal_ret = SAL_MemCmp((const void *)benchmarkBuf, (const void *)expectedDst, sizeof(expectedDst), &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(benchmarkBuf, sizeof(expectedDst));
        HSM_HexDump(expectedDst, sizeof(expectedDst));
        ELOG("Wrong cipher data(%d)\n", ret);
        // return HSM_ERR;
    }

    DLOG(
        "[AES128-CBC-Decryption]Input Size:%d MB(%d Kbyte * %d times)  Total Time:%d ms  Performance: %d Kbyte/s\n",
        (aes.srcSize * bench.iteration) / (1024u * 1024u), (aes.srcSize / 1024u), bench.iteration, bench.timeDiff,
        bench.speed);

    BLOG("\n");
    return ret;
}

static int32 HSM_BenchmarkHashTest(void)
{
    hsmSha sha = {0};
    hsmBenchMark bench = {0};
    uint32 i = 0;
    int32 ret = 0;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    DLOG("BENCHMARK HAST TEST\n");

    SAL_MemSet((void *)benchmarkBuf, 0, sizeof(benchmarkBuf));

    sha.objId = HSM_OID_SHA2_256; // Only support SHA256
    sha.src = benchmarkBuf;
    sha.digSize = sizeof(digest_sha2_256);

    /* START */
    bench.iteration = 4096u;
    SAL_GetTickCount(&bench.start);
    sha.srcSize = (sizeof(benchmarkBuf) * bench.iteration);
    sha.opMode = OPMODE_START;
    ret = HSM_GenHash(&sha);

    /* UPDATE */
    sha.opMode = OPMODE_UPDATE;
    sha.src = benchmarkBuf;
    sha.srcSize = sizeof(benchmarkBuf);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_GenHash(&sha);
    }

    /* FINISH */
    sha.opMode = OPMODE_FINISH;
    sha.srcSize = 0;
    sha.digSize = sizeof(digest_sha2_256);
    ret |= HSM_GenHash(&sha);
    SAL_GetTickCount(&bench.end);

    bench.timeDiff = (bench.end - bench.start);
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHashTest test fail(%d)\n", ret);
        return HSM_ERR;
    }

    bench.speed = (((sizeof(benchmarkBuf) * bench.iteration)) / bench.timeDiff);

    sal_ret = SAL_MemCmp((const void *)digest_sha2_256_benchmark, (const void *)sha.dig, sha.digSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(digest_sha2_256_benchmark, sha.digSize);
        HSM_HexDump(sha.dig, sha.digSize);
        ELOG("Wrong cipher data(%d)\n", ret);
        return HSM_ERR;
    }

    DLOG(
        "[HASH-SHA256]Input Size:%d MB(%d Kbyte * %d times)  Total Time:%d ms  Performance: %d Kbyte/s\n",
        (sizeof(benchmarkBuf) * bench.iteration) / (1024u * 1024u), (sizeof(benchmarkBuf) / 1024u), bench.iteration,
        bench.timeDiff, bench.speed);

    /* START */
    bench.iteration = 8192u;
    SAL_GetTickCount(&bench.start);
    sha.srcSize = ((sizeof(benchmarkBuf) * bench.iteration) / 2u);
    sha.opMode = OPMODE_START;
    ret = HSM_GenHash(&sha);

    /* UPDATE */
    sha.opMode = OPMODE_UPDATE;
    for (i = 0; i < bench.iteration; i++) {
        sha.src = benchmarkBuf;
        sha.srcSize = (sizeof(benchmarkBuf) / 2u);
        ret |= HSM_GenHash(&sha);
    }

    /* FINISH */
    sha.opMode = OPMODE_FINISH;
    sha.srcSize = 0;
    sha.digSize = sizeof(digest_sha2_256);
    ret |= HSM_GenHash(&sha);
    SAL_GetTickCount(&bench.end);

    bench.timeDiff = (bench.end - bench.start);
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHashTest test fail(%d)\n", ret);
        return HSM_ERR;
    }

    bench.speed = ((((sizeof(benchmarkBuf) / 2u) * bench.iteration)) / bench.timeDiff);

    sal_ret = SAL_MemCmp((const void *)digest_sha2_256_benchmark, (const void *)sha.dig, sha.digSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(digest_sha2_256_benchmark, sha.digSize);
        HSM_HexDump(sha.dig, sha.digSize);
        ELOG("Wrong cipher data(%d)\n", ret);
        // return HSM_ERR;
    }

    DLOG(
        "[HASH-SHA256]Input Size:%d MB(%d Kbyte * %d times)  Total Time:%d ms  Performance: %d Kbyte/s\n",
        ((sizeof(benchmarkBuf) / 2u) * bench.iteration) / (1024u * 1024u), ((sizeof(benchmarkBuf) / 2u) / 1024u),
        bench.iteration, bench.timeDiff, bench.speed);

    BLOG("\n");
    return ret;
}

static int32 HSM_BenchmarkMacTest(void)
{
    hsmMac mac = {0};
    hsmBenchMark bench = {0};
    uint32 i = 0;
    int32 ret = HSM_OK;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    DLOG("BENCHMARK MAC TEST\n");

    SAL_MemSet((void *)benchmarkBuf, 0, sizeof(benchmarkBuf));

    /* CMAC is supported as only single call*/
    mac.req = REQ_HSM_GEN_CMAC;
    mac.opMode = OPMODE_SINGLECALL;
    mac.objId = HSM_OID_CMAC_AES128;
    mac.keySize = sizeof(aes_key);
    SAL_MemCopy((void *)mac.macKey, (const void *)aes_key, mac.keySize);
    mac.ivSize = sizeof(aes_iv);
    SAL_MemCopy((void *)mac.macIv, (const void *)aes_iv, mac.ivSize);
    mac.macSize = HSM_MAX_CMAC_SIZE;

    /* Test1) [MAC-CMAC-GEN] SINGLECALL */
    mac.src = benchmarkBuf;
    mac.srcSize = HSM_MAX_CMAC_SRC_SIZE;
    bench.iteration = 1024;
    SAL_GetTickCount(&bench.start);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_GenMac(&mac);
    }
    SAL_GetTickCount(&bench.end);
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkCMacTest generate test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }

    bench.timeDiff = (bench.end - bench.start);
    bench.speed = ((mac.srcSize * bench.iteration) / bench.timeDiff);
    sal_ret = SAL_MemCmp((const void *)cmac_mac_benchmar, (const void *)mac.mac, mac.macSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(cmac_mac_benchmar, mac.macSize);
        HSM_HexDump(mac.mac, mac.macSize);
        ELOG("Wrong cipher data(%d)\n", ret);
        // return HSM_ERR;
    }
    DLOG(
        "[CMAC-AES128-GEN]Input Size:%d MB(%d Kbyte * %d times) Total Time:%d ms  Performance: %d Kbyte/s\n",
        (mac.srcSize * bench.iteration) / (1024u * 1024u), (mac.srcSize / 1024u), bench.iteration, bench.timeDiff,
        bench.speed);

    /* Test2) [MAC-CMAC-VERIFY] SINGLECALL */
    bench.iteration = 1024;
    mac.req = REQ_HSM_VERIFY_CMAC;
    SAL_GetTickCount(&bench.start);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_Verifymac(&mac);
    }
    SAL_GetTickCount(&bench.end);
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkCMacTest verify test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }

    bench.timeDiff = (bench.end - bench.start);
    bench.speed = ((mac.srcSize * bench.iteration) / bench.timeDiff);
    DLOG(
        "[CMAC-AES128-VERIFY]Input Size:%d MB(%d Kbyte * %d times) Total Time:%d ms  Performance: %d Kbyte/s\n",
        (mac.srcSize * bench.iteration) / (1024u * 1024u), (mac.srcSize / 1024u), bench.iteration, bench.timeDiff,
        bench.speed);

    /* Test3) [MAC-HMAC-SHA256-GEN] 32KB x 1024  */
    mac.req = HSM_REQ_GEN_HMAC;
    mac.objId = HSM_OID_HMAC_SHA2_256;
    mac.src = benchmarkBuf;
    bench.iteration = 1024;
    mac.macSize = HSM_MAX_HMAC_SIZE;
    SAL_GetTickCount(&bench.start);

    /* START */
    mac.opMode = OPMODE_START;
    mac.srcSize = (sizeof(benchmarkBuf) * bench.iteration);
    ret = HSM_GenMac(&mac);
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHMacTest generate test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }
    /* UPDATE */
    mac.opMode = OPMODE_UPDATE;
    mac.srcSize = sizeof(benchmarkBuf);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_GenMac(&mac);
    }
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHMacTest generate test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }
    /* FINISH */
    mac.opMode = OPMODE_FINISH;
    mac.srcSize = 0;
    ret |= HSM_GenMac(&mac);
    SAL_GetTickCount(&bench.end);
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHMacTest generate test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }

    sal_ret = SAL_MemCmp((const void *)hmac_mac_benchmar, (const void *)mac.mac, mac.macSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(hmac_mac_benchmar, mac.macSize);
        HSM_HexDump(mac.mac, mac.macSize);
        ELOG("Wrong cipher data(%d)\n", ret);
        // return HSM_ERR;
    }

    bench.timeDiff = (bench.end - bench.start);
    bench.speed = (((sizeof(benchmarkBuf) * bench.iteration)) / bench.timeDiff);
    DLOG(
        "[HMAC-SHA256-GEN]Input Size:%d MB(%d Kbyte * %d times) Total Time:%d ms  Performance: %d Kbyte/s\n",
        (mac.srcSize * bench.iteration) / (1024u * 1024u), (mac.srcSize / 1024u), bench.iteration, bench.timeDiff,
        bench.speed);

    /* Test4) [MAC-HMAC-SHA256-GEN] 16KB x 2048  */
    mac.req = HSM_REQ_GEN_HMAC;
    mac.objId = HSM_OID_HMAC_SHA2_256;
    mac.src = benchmarkBuf;
    bench.iteration = 2048;
    mac.macSize = HSM_MAX_HMAC_SIZE;
    SAL_GetTickCount(&bench.start);

    /* START */
    mac.opMode = OPMODE_START;
    mac.srcSize = ((sizeof(benchmarkBuf) / 2u) * bench.iteration);
    ret = HSM_GenMac(&mac);
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHMacTest generate test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }
    /* UPDATE */
    mac.opMode = OPMODE_UPDATE;
    mac.srcSize = (sizeof(benchmarkBuf) / 2u);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_GenMac(&mac);
    }
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHMacTest generate test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }
    /* FINISH */
    mac.opMode = OPMODE_FINISH;
    mac.srcSize = 0;
    ret |= HSM_GenMac(&mac);
    SAL_GetTickCount(&bench.end);
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHMacTest generate test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }

    bench.timeDiff = (bench.end - bench.start);
    bench.speed = ((((sizeof(benchmarkBuf) / 2u) * bench.iteration)) / bench.timeDiff);
    sal_ret = SAL_MemCmp((const void *)hmac_mac_benchmar, (const void *)mac.mac, mac.macSize, &ret);
    if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
        HSM_HexDump(hmac_mac_benchmar, mac.macSize);
        HSM_HexDump(mac.mac, mac.macSize);
        ELOG("Wrong cipher data(%d)\n", ret);
        // return HSM_ERR;
    }
    DLOG(
        "[HMAC-SHA256-GEN]Input Size:%d MB(%d Kbyte * %d times) Total Time:%d ms  Performance: %d Kbyte/s\n",
        ((sizeof(benchmarkBuf) / 2u) * bench.iteration) / (1024u * 1024u), ((sizeof(benchmarkBuf) / 2u) / 1024u),
        bench.iteration, bench.timeDiff, bench.speed);

    /* Test5) [MAC-HMAC-SHA256-Verify] 32KB x 1024  */
    mac.req = HSM_REQ_VERIFY_HMAC;
    mac.objId = HSM_OID_HMAC_SHA2_256;
    mac.src = benchmarkBuf;
    bench.iteration = 1024;
    mac.macSize = HSM_MAX_HMAC_SIZE;
    SAL_GetTickCount(&bench.start);

    /* START */
    mac.opMode = OPMODE_START;
    mac.srcSize = (sizeof(benchmarkBuf) * bench.iteration);
    ret = HSM_Verifymac(&mac);
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHMacTest generate test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }
    /* UPDATE */
    mac.opMode = OPMODE_UPDATE;
    mac.srcSize = sizeof(benchmarkBuf);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_Verifymac(&mac);
    }
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHMacTest generate test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }
    /* FINISH */
    mac.opMode = OPMODE_FINISH;
    mac.srcSize = 0;
    ret |= HSM_Verifymac(&mac);
    SAL_GetTickCount(&bench.end);
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkHMacTest generate test fail(%d) iteration=%d\n", ret, i);
        return HSM_ERR;
    }

    bench.timeDiff = (bench.end - bench.start);
    bench.speed = (((sizeof(benchmarkBuf) * bench.iteration)) / bench.timeDiff);
    DLOG(
        "[HMAC-SHA256-VERIFY]Input Size:%d MB(%d Kbyte * %d times) Total Time:%d ms  Performance: %d Kbyte/s\n",
        (sizeof(benchmarkBuf) * bench.iteration) / (1024u * 1024u), (sizeof(benchmarkBuf) / 1024u), bench.iteration,
        bench.timeDiff, bench.speed);

    BLOG("\n");
    return ret;
}

static int32 HSM_BenchmarkEcdsaTest(void)
{
    hsmAsymKey asymKey = {0};
    hsmBenchMark bench = {0};
    hsmAsym ecdsa = {0};
    uint32 cnt = 0;
    uint8 sigBuf[HSM_ECDH_P256_SIG_SIZE] = {0};
    int32 ret = HSM_ERR;

    DLOG("BENCHMARK ECDSA TEST\n");

    /* Test1) NIST P256 TEST */
    /* Generate key */
    asymKey.objId = HSM_OID_ECC_P256;
    asymKey.keyType = 0u;
    asymKey.priSize = HSM_ECDH_P256_KEY_SIZE;
    asymKey.pubSize = (HSM_ECDH_P256_KEY_SIZE * 2u);
    asymKey.seedSize = 0; // Not mandatory
    ret = HSM_RunEcdhPhaseI(&asymKey);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa signing test fail(%d)\n", ret);
        return ret;
    }
    ecdsa.req = HSM_REQ_RUN_ECDSA_SIGN;
    ecdsa.objId = (HSM_OID_SHA2_256 | HSM_OID_ECC_P256);
    ecdsa.opMode = OPMODE_SINGLECALL;
    ecdsa.priSize = asymKey.priSize;
    ecdsa.priKey = asymKey.priKey;
    ecdsa.pubSize = asymKey.pubSize;
    ecdsa.pubKey = asymKey.pubKey;
    ecdsa.digSize = sizeof(gzu08_digest);
    ecdsa.dig = gzu08_digest;
    ecdsa.sigSize = HSM_ECDH_P256_SIG_SIZE;
    ecdsa.sig = sigBuf;
    bench.iteration = 4096u;

    /* ECDSA-P256-SIGN */
    SAL_GetTickCount(&bench.start);
    while (bench.timeDiff < 1000) {
        ret |= HSM_RunEcdsa(&ecdsa);
        SAL_GetTickCount(&bench.end);
        bench.timeDiff = (bench.end - bench.start);
        cnt++;
    }
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkEcdsaTest nist p256 signing test fail(%d)\n", ret);
        return ret;
    }
    ecdsa.req = HSM_REQ_RUN_ECDSA_VERIFY;
    ret = HSM_RunEcdsa(&ecdsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa nist p256 verify test fail(%d)\n", ret);
        return ret;
    }
    DLOG("[ECDSA-P256-SIGN] %d sign/sec\n", cnt);

    /* ECDSA-P256-VERIFY */
    cnt = 0u;
    bench.timeDiff = 0u;
    ecdsa.req = HSM_REQ_RUN_ECDSA_VERIFY;
    SAL_GetTickCount(&bench.start);
    while (bench.timeDiff < 1000) {
        ret |= HSM_RunEcdsa(&ecdsa);
        SAL_GetTickCount(&bench.end);
        bench.timeDiff = (bench.end - bench.start);
        cnt++;
    }
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkEcdsaTest nist p256 verifying test fail(%d)\n", ret);
        return ret;
    }
    DLOG("[ECDSA-P256-VERIFY] %d sign/sec\n", cnt);

    /* Test2) NIST BP256 TEST */
    /* Generate key */
    asymKey.objId = HSM_OID_ECC_BP256;
    asymKey.keyType = 0u;
    asymKey.priSize = HSM_ECDH_P256_KEY_SIZE;
    asymKey.pubSize = (HSM_ECDH_P256_KEY_SIZE * 2u);
    asymKey.seedSize = 0; // Not mandatory
    ret = HSM_RunEcdhPhaseI(&asymKey);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa signing test fail(%d)\n", ret);
        return ret;
    }

    ecdsa.req = HSM_REQ_RUN_ECDSA_SIGN;
    ecdsa.objId = (HSM_OID_SHA2_256 | HSM_OID_ECC_BP256);
    ecdsa.opMode = OPMODE_SINGLECALL;
    ecdsa.priSize = asymKey.priSize;
    SAL_MemCopy((void *)ecdsa.priKey, (const void *)asymKey.priKey, asymKey.priSize);
    ecdsa.pubSize = asymKey.pubSize;
    SAL_MemCopy((void *)ecdsa.pubKey, (const void *)asymKey.pubKey, asymKey.pubSize);
    ecdsa.digSize = sizeof(gzu08_digest);
    ecdsa.dig = gzu08_digest;
    ecdsa.sigSize = HSM_ECDH_P256_SIG_SIZE;

    cnt = 0u;
    bench.timeDiff = 0u;
    SAL_GetTickCount(&bench.start);
    while (bench.timeDiff < 1000) {
        ret |= HSM_RunEcdsa(&ecdsa);
        SAL_GetTickCount(&bench.end);
        bench.timeDiff = (bench.end - bench.start);
        cnt++;
    }
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkEcdsaTest nist bp256 signing test fail(%d)\n", ret);
        return ret;
    }
    ecdsa.req = HSM_REQ_RUN_ECDSA_VERIFY;
    ret = HSM_RunEcdsa(&ecdsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa nist bp256 verify test fail(%d)\n", ret);
        return ret;
    }
    DLOG("[ECDSA-BP256-VERIFY] %d sign/sec\n", cnt);

    /* ECDSA-P256-VERIFY */
    cnt = 0u;
    bench.timeDiff = 0u;
    ecdsa.req = HSM_REQ_RUN_ECDSA_VERIFY;
    SAL_GetTickCount(&bench.start);
    while (bench.timeDiff < 1000) {
        ret |= HSM_RunEcdsa(&ecdsa);
        SAL_GetTickCount(&bench.end);
        bench.timeDiff = (bench.end - bench.start);
        cnt++;
    }
    if (ret != HSM_OK) {
        ELOG("HSM_BenchmarkEcdsaTest nist bp256 verifying test fail(%d)\n", ret);
        return ret;
    }

    DLOG("[ECDSA-BP256-VERIFY] %d sign/sec\n", cnt);
    BLOG("\n");

    return ret;
}
#if (ENABLE_RSA)
static int32 HSM_BenchmarkRsassaTest(void)
{
    hsmAsym rsa = {0};
    hsmBenchMark bench = {0};
    uint8 sigBuf[256] = {0};
    int32 ret = HSM_ERR;

    DLOG("BENCHMARK RSASSA TEST\n");

    /* Sign - PKCS 1024bit*/
    rsa.req = REQ_HSM_RUN_RSASSA_PKCS_SIGN;
    rsa.objId = HSM_OID_SHA2_256;
    rsa.opMode = OPMODE_SINGLECALL;
    rsa.priSize = sizeof(rsassa_d);
    rsa.priKey = rsassa_d;
    rsa.modSize = sizeof(rsassa_n);
    rsa.modulus = rsassa_n;
    rsa.pubSize = sizeof(rsassa_e);
    rsa.pubKey = rsassa_e;
    rsa.digSize = sizeof(gzu08_digest);
    rsa.dig = gzu08_digest;
    rsa.sigSize = sizeof(sigBuf);
    rsa.sig = sigBuf;

    SAL_GetTickCount(&bench.start);
    ret = HSM_RunRsassa(&rsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunRsassa pkcs1 sign test fail(%d)\n", ret);
        return ret;
    }

    SAL_GetTickCount(&bench.end);
    bench.timeDiff = (bench.end - bench.start);

    DLOG("[RSASSA(1024bit)-PKCS-SIGN] %d ms per run\n", bench.timeDiff);

    /* Verify - PKCS 1024bit*/
    rsa.req = REQ_HSM_RUN_RSASSA_PKCS_VERIFY;
    SAL_GetTickCount(&bench.start);
    ret = HSM_RunRsassa(&rsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunRsassa pkcs1 verify test fail(%d)\n", ret);
        return ret;
    }
    SAL_GetTickCount(&bench.end);
    bench.timeDiff = (bench.end - bench.start);
    DLOG("[RSASSA(1024bit)-PKCS-Verify] %d ms per run\n", bench.timeDiff);

    /* Sign - PSS 1024bit*/
    rsa.req = REQ_HSM_RUN_RSASSA_PSS_SIGN;
    rsa.objId = HSM_OID_SHA2_256;
    rsa.opMode = OPMODE_SINGLECALL;
    rsa.priSize = sizeof(rsassa_d);
    rsa.priKey = rsassa_d;
    rsa.modSize = sizeof(rsassa_n);
    rsa.modulus = rsassa_n;
    rsa.pubSize = sizeof(rsassa_e);
    rsa.pubKey = rsassa_e;
    rsa.digSize = sizeof(gzu08_digest);
    rsa.dig = gzu08_digest;
    rsa.sigSize = sizeof(sigBuf);
    rsa.sig = sigBuf;

    SAL_GetTickCount(&bench.start);
    ret = HSM_RunRsassa(&rsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunRsassa pkcs1 sign test fail(%d)\n", ret);
        return ret;
    }

    SAL_GetTickCount(&bench.end);
    bench.timeDiff = (bench.end - bench.start);
    DLOG("[RSASSA(1024bit)-PSS-SIGN] %d ms per run\n", bench.timeDiff);

    /* Verify - PSS 1024bit*/
    rsa.req = REQ_HSM_RUN_RSASSA_PSS_VERIFY;
    SAL_GetTickCount(&bench.start);
    ret = HSM_RunRsassa(&rsa);
    if (ret != HSM_OK) {
        ELOG("HSM_RunRsassa pss verify test fail(%d)\n", ret);
        return ret;
    }
    SAL_GetTickCount(&bench.end);
    bench.timeDiff = (bench.end - bench.start);
    DLOG("[RSASSA(1024bit)-PSS-VERIFY] %d ms per run\n", bench.timeDiff);
    BLOG("\n");

    return ret;
}
#endif

static int32 HSM_BenchmarkRngTest(void)
{
    hsmRng rng = {0};
    hsmBenchMark bench = {0};
    uint32 i = 0;
    int32 ret = 0;

    DLOG("BENCHMARK RNG TEST\n");

    rng.rngSize = HSM_MAX_RAND_SIZE;
    bench.iteration = 4096u;

    SAL_GetTickCount(&bench.start);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_TRNG(&rng);
    }
    SAL_GetTickCount(&bench.end);
    if (ret != HSM_OK) {
        ELOG("HSM_GetTRNG test fail(%d)\n", ret);
        return ret;
    }
    bench.timeDiff = (bench.end - bench.start);
    bench.speed = ((rng.rngSize * bench.iteration) / bench.timeDiff);

    DLOG(
        "[RNG-TRNG]Input Size:%d KB(%d Byte * %d times)  Total Time:%d ms  Performance: %d Kbyte/s\n",
        (rng.rngSize * bench.iteration) / 1024u, rng.rngSize, bench.iteration, bench.timeDiff, bench.speed);

    rng.rngSize = HSM_MAX_RAND_SIZE;
    rng.seedSize = sizeof(rng_seed);
    SAL_MemCopy((void *)rng.seed, (const void *)rng_seed, rng.seedSize);

    SAL_GetTickCount(&bench.start);
    for (i = 0; i < bench.iteration; i++) {
        ret |= HSM_PRNG(&rng);
    }
    SAL_GetTickCount(&bench.end);
    if (ret != HSM_OK) {
        ELOG("HSM_GetTRNG test fail(%d)\n", ret);
        return ret;
    }
    bench.timeDiff = (bench.end - bench.start);
    bench.speed = ((rng.rngSize * bench.iteration) / bench.timeDiff);

    DLOG(
        "[RNG-PRNG]Input Size:%d KB(%d Byte * %d times)  Total Time:%d ms  Performance: %d Kbyte/s\n",
        (rng.rngSize * bench.iteration) / 1024u, rng.rngSize, bench.iteration, bench.timeDiff, bench.speed);
    BLOG("\n");

    return ret;
}

static int32 HSM_BenchmarkCertVerifyTest(void)
{
    hsmCerti root = {0};
    hsmCerti target = {0};
    hsmBenchMark bench = {0};
    int32 ret = HSM_ERR;

    DLOG("BENCHMARK Certificate Verify TEST\n");

    root.crt = (uint8 *)root_ecdsa_crt;
    root.crtSize = sizeof(root_ecdsa_crt);
    target.crt = (uint8 *)host_ecdsa_crt;
    target.crtSize = sizeof(host_ecdsa_crt);

    SAL_GetTickCount(&bench.start);
    ret = HSM_CertiVerify(&root, &target);
    if (ret != HSM_OK) {
        ELOG("HSM_CertiVerify test fail(%d)\n", ret);
        return ret;
    }

    SAL_GetTickCount(&bench.end);
    bench.timeDiff = (bench.end - bench.start);

    DLOG("[CERTIFICATE-VERIFY] %d ms per run\n", bench.timeDiff);

    BLOG("\n");

    return ret;
}

static void HSM_BenchmarkTest(void)
{
    int32 ret = HSM_OK;

    ret = HSM_BenchmarkAesTest();
    ret |= HSM_BenchmarkHashTest();
    ret |= HSM_BenchmarkMacTest();
    ret |= HSM_BenchmarkEcdsaTest();
#if (ENABLE_RSA)
    ret |= HSM_BenchmarkRsassaTest();
#endif
    ret |= HSM_BenchmarkRngTest();
    ret = HSM_BenchmarkCertVerifyTest();
    if (ret != 0) {
        ELOG("HSM_BenchmarkTest fail(%d)\n", ret);
    }

    return;
}

static void MBOX_FmuIsr(void)
{
    FMUErrTypes_t err = FMU_OK;

    DLOG("***************************\n");
    DLOG("MBOX_FmuIsr\n");
    DLOG("***************************\n");

    err = FMU_IsrClr(FMU_ID_MBOX0_S_ECC);
    if (err != FMU_OK) {
        ELOG("FMU ISR ERROR\n");
    }
}

static void MBOX_EnableFmu(void)
{
    //    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_MBOX0_S_ECC,
    //    (FMUSeverityLevelType_t)FMU_SVL_LOW,
    //            (FMUIntFnctPtr)&MBOX_FmuIsr, NULL_PTR);
    //    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_MBOX0_S_ECC,
    //    (FMUSeverityLevelType_t)FMU_SVL_MID,
    //            (FMUIntFnctPtr)&MBOX_FmuIsr, NULL_PTR);
    (void)FMU_IsrHandler(
        (FMUFaultid_t)FMU_ID_MBOX0_S_ECC, (FMUSeverityLevelType_t)FMU_SVL_HIGH,
        (FMUIntFnctPtr)&MBOX_FmuIsr, NULL_PTR);

    (void)FMU_Set((FMUFaultid_t)FMU_ID_MBOX0_S_ECC);

    DLOG("MBOX_EnableFmu\n");
}

static void MBOX_TrigerFmu(void)
{
    volatile int32 *MBOX_DATA_FIFO_RX = (volatile int32 *)0xA09F0070UL;
    volatile uint32 data = 0;
    int32 i;

    for (i = 0; i < 128; i++) {
        data = *MBOX_DATA_FIFO_RX;
        DLOG("MBOX_DATA_FIFO_RX(%d)\n", data);
    }
}

int32 HSM_Test(void *pCommand, void *pValue)
{
    int32 ret = HSM_ERR;
    uint32 cmd = 0;
    uint32 value = 0;

    DLOG("Test App Version:%d.%d\n", TESTAPP_MAJOR_VER, TESTAPP_MINOR_VER);

    cmd = atoi(pCommand);
    value = atoi(pValue);
    DLOG(" uiCmd:%d, uiValue:%d\n", cmd, value);

    if (HSM_Init() != HSM_OK) {
        return HSM_ERR;
    }

    switch (cmd) {
    case HSM_SET_KEY_FROM_NVM:
        ret = HSM_SetKeyTest();
        break;

    case HSM_RUN_AES:
        ret = HSM_RunAesTest();
        break;

    case HSM_RUN_AES_BY_KT:
        ret = HSM_RunAesByKtTest();
        break;

    case HSM_GEN_MAC:
        ret = HSM_GenMacTest();
        break;

    case HSM_GEN_MAC_BY_KT:
        ret = HSM_GenMacByKtTest();
        break;

    case HSM_GEN_SHA:
        ret = HSM_GenHashTest();
        break;

    case HSM_RUN_ECDSA:
        ret = HSM_RunEcdsaTest();
        break;
#if (ENABLE_RSA)
    case HSM_RUN_RSASSA:
        ret = HSM_RunRsassaTest();
        break;
#endif
    case HSM_ECDH_COMPUTE_PUBKEY:
        ret = HSM_ComputeEcdhPubKeyTest();
        break;

    case HSM_ECDH_PHASE_I:
        ret = HSM_EcdhPhaseITest();
        break;

    case HSM_ECDH_PHASE_II:
        ret = HSM_EcdhPhaseIITest();
        break;

    case HSM_PBKDF2:
        ret = HSM_Pbkdf2Test();
        break;

    case HSM_CERTIFICATE_PARSE:
        ret = HSM_CertiParseTest();
        break;

    case HSM_CERTIFICATE_VERIFY:
        ret = HSM_CertiVerifyTest();
        break;

    case HSM_GET_RNG:
        ret = HSM_GetRngTest();
        break;

    case HSM_WRITE_NVM:
        ret = HSM_WriteDFTest();
        break;

    case HSM_SELECT_BANK:
        ret = HSM_SelectBankTest(value);
        break;

    case HSM_GET_VER:
        ret = HSM_GetVersionTest();
        break;

    case HSM_MBOX_ECC_TEST:
        MBOX_EnableFmu();
        MBOX_TrigerFmu();
        break;

    case HSM_FULL:
        HSM_FullTest(1);
        break;

    case HSM_AGING:
        HSM_FullTest(10000);
        break;

    case HSM_BECHMARK:
        HSM_BenchmarkTest();
        break;

    default:
        ELOG("unknown command(%d)\n", cmd);
        break;
    }

    HSM_PrintCmd();
    return ret;
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )

