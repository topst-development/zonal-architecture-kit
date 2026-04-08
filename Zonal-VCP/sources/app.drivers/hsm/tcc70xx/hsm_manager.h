// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : hsm_manager.h
*
*   Copyright (c) Telechips Inc.
*
*   Description : HSM MANAGER
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_HSM_MANAGER_HEADER
#define MCU_BSP_HSM_MANAGER_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_MBOX != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_MBOX value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
// clang-format off
#define HSM_REQ_SB_VERIFY                   (0x00020018U)

#define HSM_REQ_RUN_AES                     (0x10010000U)
#define HSM_REQ_RUN_AES_BY_KT               (0x10020000U)

#define REQ_HSM_GEN_CMAC                    (0x10120000u)
#define REQ_HSM_GEN_CMAC_BY_KT              (0x10130000u)
#define REQ_HSM_VERIFY_CMAC                 (0x10140000u)
#define HSM_REQ_GEN_HMAC                    (0x10150000U)
#define HSM_REQ_GEN_HMAC_BY_KT              (0x10160000U)
#define HSM_REQ_VERIFY_HMAC                 (0x10170000u)

#define HSM_REQ_GEN_SHA                     (0x10210000U)

#define HSM_REQ_RUN_ECDSA_SIGN              (0x10310000U)
#define HSM_REQ_RUN_ECDSA_VERIFY            (0x10320000U)
#define REQ_HSM_RUN_ECDH_PUBKEY_COMPUTE     (0x10330000u)
#define REQ_HSM_RUN_ECDH_PHASE_I            (0x10340000u)
#define REQ_HSM_RUN_ECDH_PHASE_II           (0x10350000u)
#define REQ_HSM_RUN_ECDSA_SIGN_BY_KT        (0x10360000u)
#define REQ_HSM_RUN_ECDSA_VERIFY_BY_KT      (0x10370000u)

#define REQ_HSM_RUN_RSASSA_PKCS_SIGN        (0x10410000u)
#define REQ_HSM_RUN_RSASSA_PKCS_VERIFY      (0x10420000u)
#define REQ_HSM_RUN_RSASSA_PSS_SIGN         (0x10430000u)
#define REQ_HSM_RUN_RSASSA_PSS_VERIFY       (0x10440000u)
#define REQ_HSM_RUN_RSASSA_PKCS_SIGN_BY_KT  (0x10450000u)
#define REQ_HSM_RUN_RSASSA_PKCS_VERIFY_BY_KT    (0x10460000u)
#define REQ_HSM_RUN_RSASSA_PSS_SIGN_BY_KT       (0x10470000u)
#define REQ_HSM_RUN_RSASSA_PSS_VERIFY_BY_KT     (0x10480000u)

#define REQ_HSM_RUN_PBKDF2                      (0x10510000u)
#define REQ_HSM_CERTIFICATE_PARSE               (0x10520000u)
#define REQ_HSM_CERTIFICATE_VERIFY              (0x10530000u)

#define HSM_REQ_GET_TRNG                     (0x10610000U)
#define HSM_REQ_GET_PRNG                     (0x10620000U)

#define HSM_REQ_WRITE_NVM                   (0x10710000U)

#define HSM_REQ_SET_KEY_FROM_NVM            (0x10810000U)

#define HSM_REQ_SELECT_BANK                 (0x10910000U)

#define HSM_REQ_GET_VER                     (0x20010000U)

#define HSM_REQ_PROGRAM_LDT                 (0x20050000u)
#define HSM_REQ_READ_LDT                    (0x20060000U)

#define HSM_REQ_VERIFY_HSM                  (0x20070000U)
#define HSM_REQ_VERIFY_MCU                  (0x20080000U)
// clang-format on

#define HSM_CMD_FIFO_SIZE (0x0008U)
#define HSM_DATA_FIFO_CNT (128U)
#define HSM_MBOX_LOCATION_DATA (0x0400U)
#define HSM_MBOX_LOCATION_CMD (0x0000U)
#define HSM_MBOX_ID_HSM (0x4D5348) /* 0x4D5348 = "HSM" */

#define HSM_MBOX_CID_A72 (0x7200U)
#define HSM_MBOX_CID_A53 (0x5300U)
#define HSM_MBOX_CID_SC (0xD300U)
#define HSM_MBOX_CID_HSM (0xA000U)
#define HSM_MBOX_CID_R5 (0xFF00U)

#define HSM_MBOX_BSID_BL0 (0x0042U)
#define HSM_MBOX_BSID_BL1 (0x0043U)
#define HSM_MBOX_BSID_BL3 (0x0045U)
#define HSM_MBOX_BSID_KERNEL (0x0046U)

#define HSM_MBOX_HSM_CMD0 (HSM_MBOX_CID_R5 | HSM_MBOX_BSID_KERNEL)
#define HSM_RSASSA_PSS_OID_HASH (HSM_OID_SHA2_256)

#define HSM_MC_SUB_FW_IMAGE_ID (0x000003FFU)
#define HSM_MICOM_SRAM1_BASEADDR (0xC1000000U)

/* HSM NVM key index */
#define HSM_R5_AES_KEY_INDEX (0x0000)
#define HSM_R5_MAC_KEY_INDEX (0x0001)

/* HSM NVM key address */
#define HSM_DF_AESKEY_ADDR (0x00000000)
#define HSM_DF_MACKEY_ADDR (0x00000010)

/* HSM Size */
#define HSM_MAX_AES_BLOCK_SIZE (16u)
#define HSM_MAX_AES_SRC_SIZE (256u)
#define HSM_MAX_AES_KEY_SIZE (32u)
#define HSM_MAX_AES_IV_SIZE (16u)
#define HSM_MAX_AES_TAG_SIZE (32u)
#define HSM_MAX_AES_AAD_SIZE (32u)
#define HSM_MAX_MAC_KEY_SIZE (64u)
#define HSM_MAX_CMAC_SIZE (16u)
#define HSM_MAX_CMAC_SRC_SIZE (8192u)
#define HSM_MAX_DIGEST_SIZE (64u)
#define HSM_MAX_HMAC_SIZE (32u)

#define HSM_MAX_ASYM_KEY_SIZE (512u)
#define HSM_MAX_ASYM_MODN_SIZE (512u)
#define HSM_MAX_ASYM_SIGN_SIZE (512u)

#define HSM_MAX_SEED_SIZE (64u)
#define HSM_MAX_RAND_SIZE (128u)

#define HSM_SHA2_256_SIZE (32u)
#define HSM_CMAC_AES_128_SIZE (16u)
#define HSM_HMAC_SHA2_256_SIZE (32u)
#define HSM_ECDH_P256_KEY_SIZE (32u)
#define HSM_ECDH_P256_SIG_SIZE (64u)

#define HSM_KDF_MAX_PW_SIZE (64u)
#define HSM_KDF_MAX_SALT_SIZE (64u)
#define HSM_KDF_MAX_KEY_SIZE (64u)

#define HSM_MBOX_TIMEOUT_GENERAL (2000) // 2000 ms
#define HSM_MBOX_TIMEOUT_RSASSA (25000) // 25 s

/* HSM Err code */
#define HSM_DATA_FIFO_OK (0U)
#define HSM_OK (0)
#define HSM_ERR (-1)
#define HSM_ERR_INVALID_PARAM (-2)
#define HSM_ERR_INVALID_STATE (-3)
#define HSM_ERR_INVALID_MEMORY (-4)
#define HSM_ERR_UNSUPPORT_FUNC (-5)
#define HSM_ERR_SOTB_CIPHER (-6)
#define HSM_ERR_OCCUPIED_RESOURCE (-7)

/* HSM OID CMAC */
#define HSM_OID_CMAC_AES128 (0x00021100U)
#define HSM_OID_CMAC_DES (0x00021200U)

/* HSM OID HMAC */
#define HSM_OID_HMAC_SHA1_160 (0x00011100U)
#define HSM_OID_HMAC_SHA2_224 (0x00012200U)
#define HSM_OID_HMAC_SHA2_256 (0x00012300U)
#define HSM_OID_HMAC_SHA2_384 (0x00012400U)
#define HSM_OID_HMAC_SHA2_512 (0x00012500U)
#define HSM_OID_HMAC_SHA3_224 (0x00013200U)
#define HSM_OID_HMAC_SHA3_256 (0x00013300U)
#define HSM_OID_HMAC_SHA3_384 (0x00013400U)
#define HSM_OID_HMAC_SHA3_512 (0x00013500U)

/* HSM OID SHA/SM3 */
#define HSM_OID_SHA1_160 (0x00001100U)
#define HSM_OID_SHA2_224 (0x00002200U)
#define HSM_OID_SHA2_256 (0x00002300U)
#define HSM_OID_SHA2_384 (0x00002400U)
#define HSM_OID_SHA2_512 (0x00002500U)
#define HSM_OID_SHA3_224 (0x00003200U)
#define HSM_OID_SHA3_256 (0x00003300U)
#define HSM_OID_SHA3_384 (0x00003400U)
#define HSM_OID_SHA3_512 (0x00003500U)
#define HSM_OID_SM3_256 (0x01002300U)

#define HSM_KDF_SHA2_256 (0x4U)

/* HSM ECC Code */
#define HSM_OID_ECC_P256 (0x00000013U)
#define HSM_OID_ECC_P384 (0x00000014U)
#define HSM_OID_ECC_P521 (0x00000015U)
#define HSM_OID_ECC_BP256 (0x00000053U)
#define HSM_OID_ECC_BP384 (0x00000054U)
#define HSM_OID_ECC_BP512 (0x00000055U)
#define HSM_OID_SM2_256_SM3_256 (0x010023A3U)

/* HSM DMA type */
#define HSM_NONE_DMA (0)
#define HSM_DMA (1)

#define HSM_AES_ENCRYPT (0x00000000U)
#define HSM_AES_DECRYPT (0x01000000U)
#define HSM_AES_ECB_128 (0x00100008U)
#define HSM_AES_ECB_192 (0x00180008U)
#define HSM_AES_ECB_256 (0x00200008U)
#define HSM_AES_CBC_128 (0x00100108U)
#define HSM_AES_CBC_192 (0x00180108U)
#define HSM_AES_CBC_256 (0x00200108U)
#define HSM_AES_CTR_128 (0x00100208U)
#define HSM_AES_CTR_192 (0x00180208U)
#define HSM_AES_CTR_256 (0x00200208U)
#define HSM_AES_XTS_128 (0x00100308U)
#define HSM_AES_XTS_256 (0x00200308U)
#define HSM_AES_CCM_128 (0x00101008U)
#define HSM_AES_CCM_192 (0x00181008U)
#define HSM_AES_CCM_256 (0x00201008U)
#define HSM_AES_GCM_128 (0x00101108U)
#define HSM_AES_GCM_192 (0x00181108U)
#define HSM_AES_GCM_256 (0x00201108U)

/* HSM OID SM4 */
#define HSM_OID_SM4_ENCRYPT (0x00000000U)
#define HSM_OID_SM4_DECRYPT (0x01000000U)
#define HSM_OID_SM4_ECB_128 (0x00100008U)
#define HSM_OID_SM4_CBC_128 (0x00100108U)

/* Operation Mode is "Start" */
/* Operation Mode is "Update". Used to calculate intermediate results. */
/* Operation Mode is "Finish". The calculations shall be finalized. */
/* Operation Mode is "Single Call". Mixture of "Start", "Update" and "Finish". */
#define OPMODE_START (0x1u)
#define OPMODE_UPDATE (0x2u)
#define OPMODE_FINISH (0x4u)
#define OPMODE_SINGLECALL (0x7u)

#define CRT_SIZE 1024
#define L1_HASH_ADDR 0x40

/*
***************************************************************************************************
*                                         STRUCT
***************************************************************************************************
*/
typedef struct
{
    uint32 dfAddr;
    uint32 keySize;
    uint8 *keydata;
    uint32 keyIdx;
} hsmKey;

typedef struct
{
    uint32 objId;
    uint32 opMode;
    uint32 keyIdx;
    uint8 aesKey[HSM_MAX_AES_KEY_SIZE];
    uint32 keySize;
    uint8 aesIv[HSM_MAX_AES_IV_SIZE];
    uint32 ivSize;
    uint8 *tag;
    uint32 tagSize;
    uint8 *aad;
    uint32 aadSize;
    uint8 *src;
    uint32 srcSize;
    uint8 *dst;
    uint32 dstSize;
} hsmAes;

typedef struct
{
    uint32 req;
    uint32 objId;
    uint32 opMode;
    uint32 keyIdx;
    uint8 macKey[HSM_MAX_MAC_KEY_SIZE];
    uint32 keySize;
    uint8 macIv[HSM_MAX_MAC_KEY_SIZE];
    uint32 ivSize;
    uint8 *src;
    uint32 srcSize;
    uint8 mac[HSM_MAX_HMAC_SIZE];
    uint32 macSize;
} hsmMac;

typedef struct
{
    uint32 objId;
    uint32 opMode;
    uint8 *src;
    uint32 srcSize;
    uint8 dig[HSM_SHA2_256_SIZE];
    uint32 digSize;
} hsmSha;

typedef struct
{
    uint32 objId;
    uint32 keyType;
    uint8 pubKey[HSM_ECDH_P256_KEY_SIZE * 2u];
    uint32 pubSize;
    uint8 priKey[HSM_ECDH_P256_KEY_SIZE];
    uint32 priSize;
    uint8 secKey[HSM_ECDH_P256_KEY_SIZE];
    uint32 secSize;
    uint8 seed[HSM_MAX_SEED_SIZE];
    uint32 seedSize;
} hsmAsymKey;

typedef struct
{
    uint32 req;
    uint32 objId;
    uint32 opMode;
    uint8 *dig;
    uint32 digSize;
    uint8 *pubKey;
    uint32 pubSize;
    uint8 *priKey;
    uint32 priSize;
    uint8 *modulus;
    uint32 modSize;
    uint8 *src;
    uint32 srcSize;
    uint8 *sig;
    uint32 sigSize;
} hsmAsym;

typedef struct
{
    uint32 mdAlg;
    uint32 pSize;
    uint8 pw[HSM_KDF_MAX_PW_SIZE];
    uint32 saltSize;
    uint8 salt[HSM_KDF_MAX_SALT_SIZE];
    uint32 iteration;
    uint32 keySize;
    uint8 key[HSM_KDF_MAX_KEY_SIZE];
} hsmKdf;

typedef struct
{
    uint8 *crt;
    uint32 crtSize;
    uint8 *pubKey;
    uint32 pubSize;
    uint8 *sig;
    uint32 sigSize;
} hsmCerti;

typedef struct
{
    uint32 rng[HSM_MAX_RAND_SIZE / 4u];
    uint32 rngSize;
    uint32 seed[HSM_MAX_SEED_SIZE / 4u];
    uint32 seedSize;
} hsmRng;

typedef struct
{
    uint32 x;
    uint32 y;
    uint32 z;
} hsmVer;

typedef struct
{
    uint8 L1[CRT_SIZE];
    uint8 L2[CRT_SIZE];
    uint8 MCU_L3[CRT_SIZE];
    uint8 HSM_L3[CRT_SIZE];
} vcpCert;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
int32 HSM_Init(void);
int32 HSM_UpdateVerify(uint32 storage);
int32 HSM_VerifyFw(
    uint32 uiHeaderAddr, uint32 uiHeaderSize, uint32 uiImgAddr, uint32 uiImgSize, uint32 uiImgId);
int32 HSM_SetKey(hsmKey *param);
int32 HSM_RunAes(hsmAes *param);
int32 HSM_RunAesByKt(hsmAes *param);
int32 HSM_GenMac(hsmMac *param);
int32 HSM_GenMacByKt(hsmMac *param);
int32 HSM_Verifymac(hsmMac *param);
int32 HSM_GenHash(hsmSha *param);
int32 HSM_RunEcdsa(hsmAsym *param);
int32 HSM_RunRsassa(hsmAsym *param);
int32 HSM_RunEcdhPhaseI(hsmAsymKey *param);
int32 HSM_RunEcdhPhaseII(hsmAsymKey *param);
int32 HSM_ComputeEcdhPubKey(hsmAsymKey *param);
int32 HSM_Pbkdf2(hsmKdf *param);
int32 HSM_CertiParse(hsmCerti *param);
int32 HSM_CertiVerify(hsmCerti *root, hsmCerti *target);
int32 HSM_DfWrite(hsmKey *param);
int32 HSM_TRNG(hsmRng *param);
int32 HSM_PRNG(hsmRng *param);
int32 HSM_SelectBank(uint32 uiBankIndex);
int32 HSM_GetVersion(hsmVer *param);
int32 HSM_ProgramLDT(uint32 addr, uint32 data1, uint32 data2, uint32 data3, uint32 data4, uint32* dataCRC);
int32 HSM_ReadLDT(uint32 addr, uint32* dataCRC);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )

#endif  // MCU_BSP_HSM_MANAGER_HEADER

