#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <lib/tcsblib/tcc_secure.h>
#include <lib/tcsblib/tcc_key.h>
#include <lib/tcsblib/tcccipher.h>
#include <impl/tcc_util.h>

#define CRT_SIZE 1024

/* command context */
typedef struct
{
	FILE *L1;
	FILE *L2;
	FILE *MCU_L3;
	FILE *HSM_L3;
	FILE *dst;
} cmd_ctx_t;

typedef struct
{
	uint8_t L1[CRT_SIZE];
	uint8_t L2[CRT_SIZE];
	uint8_t MCU_L3[CRT_SIZE];
	uint8_t HSM_L3[CRT_SIZE];
} tcc_vcpcert;

static int32_t cmd_process(cmd_ctx_t *h)
{
	tcc_vcpcert *vcpcert;
	uint32_t filesize;

	vcpcert = malloc(sizeof(tcc_vcpcert));
	memset(vcpcert, 0, sizeof(tcc_vcpcert));

	// get file size
	fseek(h->L1, 0, SEEK_END);
	filesize = ftell(h->L1);

	fseek(h->L1, 0, SEEK_SET);
	if (fread(vcpcert->L1, filesize, 1, h->L1) != 1) {
		return -1;
	}

	// get file size
	fseek(h->L2, 0, SEEK_END);
	filesize = ftell(h->L2);

	fseek(h->L2, 0, SEEK_SET);
	if (fread(vcpcert->L2, filesize, 1, h->L2) != 1) {
		return -1;
	}

	// get file size
	fseek(h->MCU_L3, 0, SEEK_END);
	filesize = ftell(h->MCU_L3);

	fseek(h->MCU_L3, 0, SEEK_SET);
	if (fread(vcpcert->MCU_L3, filesize, 1, h->MCU_L3) != 1) {
		return -1;
	}

	// get file size
	fseek(h->HSM_L3, 0, SEEK_END);
	filesize = ftell(h->HSM_L3);

	fseek(h->HSM_L3, 0, SEEK_SET);
	if (fread(vcpcert->HSM_L3, filesize, 1, h->HSM_L3) != 1) {
		return -1;
	}

	if (fwrite(vcpcert, sizeof(tcc_vcpcert), 1, h->dst) != 1) {
		return -1;
	}

	return 0;
}

/**
 * @ingroup cmd_vcpcert
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
	int8_t *L1 = NULL, *L2 = NULL, *MCU_L3 = NULL, *HSM_L3 = NULL, *out = NULL;
	int32_t i, count = 0;

	memset(h, 0x0, sizeof(cmd_ctx_t));

	for (i = 2; i < argc; i++) {
		if (count == 0) {
			L1 = argv[i];
			count++;
		} else if (count == 1) {
			L2 = argv[i];
			count++;
		} else if (count == 2) {
			MCU_L3 = argv[i];
			count++;
		} else if (count == 3) {
			HSM_L3 = argv[i];
			count++;
		} else if (count == 4) {
			out = argv[i];
			count++;
		} else {
			return ECODE_INVALID_ARGUMENT;
		}
	}

	h->L1 = fopen(L1, "rb");
	if (!h->L1) {
		printf("Can NOT open L1 file\n");
		return ECODE_UNKNOWNERROR;
	}

	h->L2 = fopen(L2, "rb");
	if (!h->L2) {
		printf("Can NOT open L2 file\n");
		return ECODE_UNKNOWNERROR;
	}

	h->MCU_L3 = fopen(MCU_L3, "rb");
	if (!h->MCU_L3) {
		printf("Can NOT open MCU_L3 file\n");
		return ECODE_UNKNOWNERROR;
	}

	h->HSM_L3 = fopen(HSM_L3, "rb");
	if (!h->HSM_L3) {
		printf("Can NOT open HSM_L3 file\n");
		return ECODE_UNKNOWNERROR;
	}

	h->dst = fopen(out, "wb");
	if (!h->dst) {
		printf("Can NOT open output file\n");
		return ECODE_UNKNOWNERROR;
	}

	return ECODE_NOERROR;
}

/**
 * @ingroup cmd_vcpcert
 * @brief finalizing command process
 *
 * @param h context
 *
 * @return 0 Success
 * @return else Fail
 */
static int32_t cmd_finalize(cmd_ctx_t *h)
{
	if (h->L1) {
		fclose(h->L1);
	}
	if (h->L2) {
		fclose(h->L2);
	}
	if (h->MCU_L3) {
		fclose(h->MCU_L3);
	}
	if (h->HSM_L3) {
		fclose(h->HSM_L3);
	}
	if (h->dst) {
		fclose(h->dst);
	}
	return 0;
}

/**
 * @ingroup cmd_vcpcert
 * @brief printing usage.
 */
static void print_usage(void)
{
	printf("\n");
	printf("Usage: vcpSignTool vcpcert [L1] [L2] [MCU_L3] [HSM_L3] [OUT_FILE]\n");
	printf("\n");
}

/**
 * @ingroup cmd_vcpcert
 * @brief tcsbfw command main function
 *
 * @param argc Count of parameters
 * @param argv[] Parameters
 * @param envp Environment information
 *
 * @return 0 Success
 * @retrun else Fail
 */
int32_t cmd_vcpcert(int32_t argc, int8_t *argv[], int8_t **envp)
{
	cmd_ctx_t ctx;
	int32_t ret = -1;

	if (argc != 7) {
		print_usage();
		return -1;
	}
	if (cmd_initial(&ctx, argc, argv) == 0) {
		ret = cmd_process(&ctx);
		if (ret == 0) {
			printf("Successful VCP CERT creation.\n");
		}
	} else {
		print_usage();
	}
	cmd_finalize(&ctx);
	return ret;
}
