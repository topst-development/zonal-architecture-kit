#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <lib/tcsblib/tcccipher.h>
#include <lib/tcsblib/tcc_secure.h>
#include <lib/tcsblib/tcc_key.h>
#include <impl/tcc_util.h>

#define CERTIFICATE_SIZE 0x100
#define CERTIFICATE_MARKER "CERT"
#define HEADER_SIZE 0x100
#define HEADER_MARKER "HDR0"

#define IMAGE_NAME_MAX_NUM (sizeof(image_id)/sizeof(uint32_t))

static const char *image_name[] = {"R5-FW"};

static const uint32_t image_id[] = {0x2FF};

uint8_t prikey[TCC_KEY_PRIKEY_LENGTH] = {0xd3, 0x74, 0x4a, 0xac, 0xe8, 0x4d, 0x67, 0x78, 0x65,
	0x0c, 0x1f, 0x9c, 0x24, 0xfe, 0xc8, 0xb7, 0xdf, 0x3b, 0x0a, 0x9f, 0xb4, 0x32, 0x46, 0x9c, 0x3c,
	0xb1, 0x1e, 0xd4, 0x00, 0xd6, 0xce, 0x09}; // test key

/* command context */
typedef struct
{
	FILE *src;
	FILE *dst;
	uint32_t mkid;
	uint32_t rbid;
} cmd_ctx_t;

/* header struct */
typedef struct
{
	char marker[4];
	uint32_t bodyLength;
	uint32_t bodyOffset;
	uint8_t headerformatversion[4];
	char socName[4];
	char imageName[12];
	char imageVersion[16];
	uint64_t targetAddr;
	char executionStates[4];

	uint8_t reserved2[36];
	uint8_t bodyhash[32];

	uint32_t mkid;
	uint32_t imgid;
	uint32_t rbid;
	uint32_t imageSize;
	uint32_t imageIsEncrypted;
	uint32_t r5fwBootConf;
	uint8_t reserved3[40];
	uint8_t signature[64];
} tcc_header;

static int32_t make_sign_image(uint8_t *buf, uint32_t len, uint8_t *prikey)
{
	if (len > 0) {
		if (tcc_secure_m2m_sign(buf, len, prikey) != 0) {
			return -1;
		}
	}
	return 0;
}

static uint32_t get_imgid(tcc_header *header)
{
	uint32_t i;

	for (i = 0; i < IMAGE_NAME_MAX_NUM; i++) {
		if (strcmp(header->imageName, image_name[i]) == 0) {
			printf("Signing %s Image.\n", image_name[i]);
			return image_id[i];
		}
	}

	return 0;
}

static int make_tcc_header(cmd_ctx_t *h, uint32_t *bodyOffset, uint32_t *bodySize, uint32_t *imgid)
{
	uint8_t *buf;
	uint32_t len, offset;
	tcc_header *header;
	int32_t ret;

	/* Copy Certificate */
	len = CERTIFICATE_SIZE;
	buf = malloc(len);
	if (buf == NULL) {
		return -1;
	}
	offset = 0;
	fseek(h->src, offset, SEEK_SET);
	if (fread(buf, len, 1, h->src) != 1) {
		free(buf);
		return -1;
	}

	if (fwrite(buf, len, 1, h->dst) != 1) {
		free(buf);
		return -1;
	}
	free(buf);

	/* Copy Header */
	len = HEADER_SIZE;
	buf = malloc(len);
	if (buf == NULL) {
		return -1;
	}
	offset = CERTIFICATE_SIZE;
	fseek(h->src, offset, SEEK_SET);
	if (fread(buf, len, 1, h->src) != 1) {
		free(buf);
		return -1;
	}

	header = (tcc_header *)buf;

	*bodySize = header->bodyLength;
	*bodyOffset = header->bodyOffset;
	*imgid = get_imgid(header);

	header->rbid = h->rbid;
	header->mkid = h->mkid;
	header->imageSize = *bodySize - 0x80;
	header->imgid = *imgid;

	/* Make secure Header */
	ret = make_sign_image(buf, len, prikey);
	if (ret != 0) {
		free(buf);
		return -1;
	}

	if (fwrite(buf, len, 1, h->dst) != 1) {
		free(buf);
		return -1;
	}
	free(buf);
	return 0;
}

static int make_header(cmd_ctx_t *h, uint32_t *bodyOffset, uint32_t *bodySize, uint32_t *imgid)
{
	if (make_tcc_header(h, bodyOffset, bodySize, imgid) != 0) {
		return -1;
	}

	return 0;
}

static int32_t cmd_process(cmd_ctx_t *h)
{
	uint8_t *buf;
	uint32_t len, offset, bodySize, bodyOffset;
	uint32_t imgid;
	int32_t ret;

	if (make_header(h, &bodyOffset, &bodySize, &imgid) != 0) {
		return -1;
	}

	/* DUMMY Image */
	if (bodyOffset == 0) {
		return 0;
	}

	/* Copy PADDING */
	len = bodyOffset - (CERTIFICATE_SIZE + HEADER_SIZE);

	if (len > 0) {
		buf = malloc(len);
		if (buf == NULL) {
			return -1;
		}
		offset = CERTIFICATE_SIZE + HEADER_SIZE;
		fseek(h->src, offset, SEEK_SET);
		if (fread(buf, len, 1, h->src) != 1) {
			free(buf);
			return -1;
		}
		if (fwrite(buf, len, 1, h->dst) != 1) {
			free(buf);
			return -1;
		}
		free(buf);
	}

	/* Copy Body */
	buf = malloc(bodySize);
	if (buf == NULL) {
		return -1;
	}

	fseek(h->src, bodyOffset, SEEK_SET);
	if (fread(buf, bodySize, 1, h->src) != 1) {
		free(buf);
		return -1;
	}

	/* Set attribute */
	tcc_secure_attribute((buf + bodySize - 0x80), imgid, h->rbid, h->mkid, (bodySize - 0x80));

	/* Make secure Image */
	ret = make_sign_image(buf, bodySize, prikey);
	if (ret != 0) {
		free(buf);
		return -1;
	}

	if (fwrite(buf, bodySize, 1, h->dst) != 1) {
		free(buf);
		return -1;
	}
	free(buf);

	return ECODE_NOERROR;
}

/**
 * @ingroup cmd_vcpmcu
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
	int8_t *in = NULL, *out = NULL, *rbid = NULL, *mkid = NULL;
	int32_t i, count = 0;

	memset(h, 0x0, sizeof(cmd_ctx_t));

	for (i = 2; i < argc; i++) {
		if (count == 0) {
			in = argv[i];
			count++;
		} else if (count == 1) {
			out = argv[i];
			count++;
		} else if (count == 2) {
			mkid = argv[i];
			count++;
		} else if (count == 3) {
			rbid = argv[i];
			count++;
		} else {
			return ECODE_INVALID_ARGUMENT;
		}
	}

	if (in == NULL || out == NULL) {
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

	if (mkid) {
		if (tcc_atoi(mkid, &h->mkid)) {
			printf("MKID(%s) Error\n", mkid);
			return ECODE_UNKNOWNERROR;
		}
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
 * @ingroup cmd_vcpmcu
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
	return ECODE_NOERROR;
}

/**
 * @ingroup cmd_vcpmcu
 * @brief printing usage.
 */
static void print_usage(void)
{
	printf("\n");
	printf("Usage: vcpSignTool vcpmcu [IN_FILE] [OUT_FILE]\n");
	printf("                           {[MKID] [RBID]} [OPTIONS...]\n");
	printf("\n");
}

/**
 * @ingroup cmd_vcpmcu
 * @brief secfw command main function
 *
 * @param argc Count of parameters
 * @param argv[] Parameters
 * @param envp Environment information
 *
 * @return 0 Success
 * @retrun else Fail
 */
int32_t cmd_vcpmcu(int32_t argc, int8_t *argv[], int8_t **envp)
{
	cmd_ctx_t ctx;
	int32_t ret = -1;

	if (argc < 4) {
		print_usage();
		return -1;
	}

	ret = cmd_initial(&ctx, argc, argv);
	if (ret == ECODE_NOERROR) {
		ret = cmd_process(&ctx);
		if (ret == 0) {
			printf("Successful signed VCP F/W image creation.\n");
		}
	}
	cmd_finalize(&ctx);

	if (ret == ECODE_INVALID_ARGUMENT) {
		print_usage();
	}
	printf("\n");

	return ret;
}
