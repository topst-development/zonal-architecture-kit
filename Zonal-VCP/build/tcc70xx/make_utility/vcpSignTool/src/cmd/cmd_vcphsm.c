#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <lib/tcsblib/tcc_secure.h>
#include <lib/tcsblib/tcc_key.h>
#include <lib/tcsblib/tcccipher.h>
#include <impl/tcc_util.h>
#include <mbedtls/x509_crt.h>

#define MODE_FACTORY (1U << 1)
#define MODE_FACTORY_HSMENC (1U << 2)
#define MODE_ENCRYPT (1U << 4)

#define MFCERT_MARKER "MFCERT"
#define MFCERT_MARKER_SIZE 6
#define MFCERT_SIZE 0x100

#define ATTR_SIG_SIZE 0x80    // 0x40(Attr), 0x40(Signature)
#define SIG_SIZE 0x40    // 0x40(Signature)
#define SHA256_SIZE 0x20
#define	VCP_HSMFW_ID	0x3030

#define CRT_SIZE 1024

uint8_t boot_prikey[TCC_KEY_PRIKEY_LENGTH] = {0x50, 0xc1, 0x08, 0xf7, 0x83, 0x12, 0xcd, 0x95, 0xfc,
	0x34, 0x07, 0x62, 0x69, 0x02, 0xd0, 0x8c, 0xf1, 0xcd, 0x91, 0xb1, 0xfa, 0xbb, 0xe9, 0x07, 0x77,
	0x1f, 0xb9, 0xf6, 0xed, 0x47, 0x64, 0x51}; // test key

uint8_t upgrade_prikey[TCC_KEY_PRIKEY_LENGTH] = {0x0a, 0x00, 0x21, 0x21, 0x90, 0xb4, 0x4d, 0x62,
	0x81, 0x2c, 0x6d, 0xdb, 0x25, 0x5a, 0x0e, 0x29, 0x1b, 0x6a, 0xaf, 0x42, 0x31, 0xc4, 0x3b, 0x57,
	0xde, 0x22, 0x61, 0x32, 0xa8, 0xd6, 0x31, 0xa0}; // test key

/* command context */
typedef struct
{
	FILE *src;
	FILE *dst;
	FILE *L1;
	FILE *HSMboot_L3;
	FILE *FWDN_ROM;
	uint32_t mkid;
	uint32_t rbid;
	uint32_t mode;
	uint32_t r5bc; // r5-fw boot config
} cmd_ctx_t;

typedef struct
{
	uint8_t L1_hash[SHA256_SIZE];
	uint8_t HSMboot_pub[TCC_KEY_PUBKEY_LENGTH];
	uint8_t dummy[32]; //for align
	uint8_t upgrade_sig[SIG_SIZE];
} tcc_hsm_upgrade_footer;

static int CertiParse_get_pubkey(mbedtls_pk_context *pk, uint8_t *pubkey, uint32_t pubSize)
{
	uint32_t keySize = 0;
	int32_t ret = -1;

	/* Get key length and Covert bits to bytes */
	keySize = (mbedtls_pk_get_bitlen(pk) / 8u);

	/* Check sigSize To prevent buffer overflow */
	if (pubSize < keySize) {
		goto out;
	}

	mbedtls_ecp_keypair *ecp = NULL;

	/* Store actual keySize(pubX + pubY) */
	pubSize = (keySize * 2u);

	ecp = mbedtls_pk_ec(*pk);
	if (ecp == NULL) {
		goto out;
	}
	ret = mbedtls_mpi_write_binary(&ecp->Q.X, &pubkey[0], keySize);
	ret = mbedtls_mpi_write_binary(&ecp->Q.Y, &pubkey[keySize], keySize);

out:
	return ret;
}

static int32_t make_sign_image(uint8_t *buf, uint32_t len, uint8_t *prikey)
{
	if (len > 0) {
		if (tcc_secure_m2m_sign(buf, len, prikey) != 0) {
			return -1;
		}
	}
	return 0;
}

static int32_t cmd_process(cmd_ctx_t *h)
{
	uint8_t *buf;
	uint8_t *fwdn_buf;
	uint32_t imagesize, bodysize;
	uint32_t crt_imagesize, fwdn_size;
	uint8_t temp_crt[CRT_SIZE] = {0};
	SHACtx_t ctx;
	tcc_hsm_upgrade_footer *hsmupgrade;
	mbedtls_x509_crt crt;

	hsmupgrade = malloc(sizeof(tcc_hsm_upgrade_footer));
	memset(hsmupgrade, 0, sizeof(tcc_hsm_upgrade_footer));

	// get file size
	fseek(h->src, 0, SEEK_END);
	imagesize = ftell(h->src);
	bodysize = imagesize - ATTR_SIG_SIZE;

	/*Prepare Buffer */
	buf = malloc(imagesize + sizeof(tcc_hsm_upgrade_footer));
	if (buf == NULL) {
		free(hsmupgrade);
		return -1;
	}

	fseek(h->src, 0, SEEK_SET);
	if (fread(buf, imagesize, 1, h->src) != 1) {
		free(hsmupgrade);
		free(buf);
		return -1;
	}

	/* copy fwdn.rom hash */
	fseek(h->FWDN_ROM, 0, SEEK_END);
	fwdn_size = ftell(h->FWDN_ROM);

	fwdn_buf = malloc(fwdn_size);
	if (fwdn_buf == NULL) {
		free(hsmupgrade);
		free(buf);
		return -1;
	}

	fseek(h->FWDN_ROM, 0, SEEK_SET);
	if (fread(fwdn_buf, fwdn_size, 1, h->FWDN_ROM) != 1) {
		free(hsmupgrade);
		free(buf);
		return -1;
	}
	SHA256_Initial(&ctx);
	SHA256_Update(&ctx, fwdn_buf, fwdn_size);
	SHA256_Finalize(&ctx, (buf + bodysize - SHA256_SIZE));

	/* Set attribute */
	tcc_secure_attribute_v2(
		(buf + bodysize), VCP_HSMFW_ID, h->rbid, h->mkid, bodysize, h->r5bc);

	/* Set boot sig */
	if (make_sign_image(buf, imagesize, boot_prikey) != 0) {
		free(hsmupgrade);
		free(buf);
		return -1;
	}

	/* copy L1 hash */
	fseek(h->L1, 0, SEEK_END);
	crt_imagesize = ftell(h->L1);
	fseek(h->L1, 0, SEEK_SET);
	if (fread(temp_crt, crt_imagesize, 1, h->L1) != 1) {
		free(hsmupgrade);
		free(buf);
		return -1;
	}
	SHA256_Initial(&ctx);
	SHA256_Update(&ctx, temp_crt, CRT_SIZE);
	SHA256_Finalize(&ctx, hsmupgrade->L1_hash);

	/* copy public key */
	fseek(h->HSMboot_L3, 0, SEEK_END);
	crt_imagesize = ftell(h->HSMboot_L3);
	fseek(h->HSMboot_L3, 0, SEEK_SET);
	memset(temp_crt, 0, sizeof(temp_crt));
	if (fread(temp_crt, crt_imagesize, 1, h->HSMboot_L3) != 1) {
		free(hsmupgrade);
		free(buf);
		return -1;
	}

	mbedtls_x509_crt_init(&crt);
	if (mbedtls_x509_crt_parse(&crt, (const uint8_t *)temp_crt, CRT_SIZE) != 0) {
		return -1;
	}
	if (CertiParse_get_pubkey(&crt.pk, hsmupgrade->HSMboot_pub, TCC_KEY_PUBKEY_LENGTH) != 0) {
		return -1;
	}

	/* copy hsm upgrade */
	memcpy((buf + imagesize), hsmupgrade, sizeof(tcc_hsm_upgrade_footer));

	/* Set upgrade sig */
	if (make_sign_image(buf, (imagesize + sizeof(tcc_hsm_upgrade_footer)), upgrade_prikey) != 0) {
		free(hsmupgrade);
		free(buf);
		return -1;
	}

	if (fwrite(buf, (imagesize + sizeof(tcc_hsm_upgrade_footer)), 1, h->dst) != 1) {
		free(hsmupgrade);
		free(buf);
		return -1;
	}
	free(hsmupgrade);
	free(buf);

	return 0;
}

/**
 * @ingroup cmd_secfw
 * @brief initialing command process
 *
 * @param h context
 * @param argc Count of parameters
 * @param argv[] Parameters
 *
 * @return 0 Success
 * @return else Fail
 */
static int32_t cmd_initial(cmd_ctx_t *h, int32_t argc, int8_t *argv[])
{
	int8_t *in = NULL, *out = NULL, *L1 = NULL, *HSMboot_L3 = NULL, *FWDN_ROM = NULL, *rbid = NULL;
	int32_t i, count = 0;
	char *ptr = NULL;

	memset(h, 0x0, sizeof(cmd_ctx_t));

	for (i = 2; i < argc; i++) {
		if (argv[i][0] == '-') {
			ptr = strstr(argv[i], "--r5-bc");
			if (ptr != NULL) {
				ptr = next_string(argv[i]);
				tcc_atoi(ptr, &h->r5bc);
				if (h->r5bc > 3) {
					printf("ERROR : --r5-bc <value> must be less than 4.\n");
					return ECODE_INVALID_ARGUMENT;
				}
			} else if (strcmp(argv[i], "--factory") == 0) {
				h->mode |= MODE_FACTORY;
				h->mode |= MODE_FACTORY_HSMENC;
			} else if (strcmp(argv[i], "--factory-noenc") == 0) {
				h->mode |= MODE_FACTORY;
			} else {
				return ECODE_INVALID_ARGUMENT;
			}
		} else {
			if (count == 0) {
				in = argv[i];
				count++;
			} else if (count == 1) {
				out = argv[i];
				count++;
			} else if (count == 2) {
				L1 = argv[i];
				count++;
			} else if (count == 3) {
				HSMboot_L3 = argv[i];
				count++;
			} else if (count == 4) {
				FWDN_ROM = argv[i];
				count++;
			} else if (count == 5) {
				rbid = argv[i];
				count++;
			} else {
				return ECODE_INVALID_ARGUMENT;
			}
		}
	}

	if (in == NULL || out == NULL || L1 == NULL || HSMboot_L3 == NULL || FWDN_ROM == NULL) {
		return ECODE_INVALID_ARGUMENT;
	}

	h->src = fopen(in, "rb");
	if (!h->src) {
		printf("Can NOT open input file\n");
		return ECODE_UNKNOWNERROR;
	}

	h->dst = fopen(out, "wb");
	if (!h->dst) {
		printf("Can NOT open output file\n");
		return ECODE_UNKNOWNERROR;
	}

	h->L1 = fopen(L1, "rb");
	if (!h->L1) {
		printf("Can NOT open L1 file\n");
		return ECODE_UNKNOWNERROR;
	}

	h->HSMboot_L3 = fopen(HSMboot_L3, "rb");
	if (!h->HSMboot_L3) {
		printf("Can NOT open HSMboot_L3 file\n");
		return ECODE_UNKNOWNERROR;
	}

	h->FWDN_ROM = fopen(FWDN_ROM, "rb");
	if (!h->FWDN_ROM) {
		printf("Can NOT open FWDN_ROM file\n");
		return ECODE_UNKNOWNERROR;
	}

	if (rbid) {
		if (tcc_atoi(rbid, &h->rbid)) {
			printf("RBID(%s) Error\n", rbid);
			return ECODE_UNKNOWNERROR;
		}
	}

	return ECODE_NOERROR;
}

/**
 * @ingroup cmd_secfw
 * @brief finalizing command process
 *
 * @param h context
 *
 * @return 0 Success
 * @return else Fail
 */
static int32_t cmd_finalize(cmd_ctx_t *h)
{
	if (h->src) {
		fclose(h->src);
	}
	if (h->dst) {
		fclose(h->dst);
	}
	if (h->L1) {
		fclose(h->L1);
	}
	return 0;
}

/**
 * @ingroup cmd_secfw
 * @brief printing usage.
 */
static void print_usage(void)
{
	printf("\n");
	printf("Usage: vcpSignTool vcphsm [IN_FILE] [OUT_FILE] [L1] [HSM_BOOT_L3] [FWDN_ROM]\n");
	printf("                                  {[RBID] [OPTIONS...]}\n");
	printf("\n");
	printf("    --r5-bc=<value>  R5-FW Boot Config (Only use for R5-FW)\n");
	printf("                     <value> [0 : default]\n");
	printf("                             [1 : Runtime Verification Enable]\n");
	printf("                             [2 : Parallel Boot Enable]\n");
	printf("                             [3 : Runtime Verification & Parallel Boot Enable]\n");
	printf("\n");
}

/**
 * @ingroup cmd_secfw
 * @brief tcsbfw command main function
 *
 * @param argc Count of parameters
 * @param argv[] Parameters
 * @param envp Environment information
 *
 * @return 0 Success
 * @retrun else Fail
 */
int32_t cmd_vcphsm(int32_t argc, int8_t *argv[], int8_t **envp)
{
	cmd_ctx_t ctx;
	int32_t ret = -1;

	if (argc < 7) {
		print_usage();
		return -1;
	}
	if (cmd_initial(&ctx, argc, argv) == 0) {
		ret = cmd_process(&ctx);
		if (ret == 0) {
			printf("Successful signed HSM F/W image creation.\n");
		}
	} else {
		print_usage();
	}
	cmd_finalize(&ctx);
	return ret;
}
