/**
 * @file main.c
 * @brief Telechips signing tool main file
 * @defgroup tcsb_main Telechips Signing Tool Main
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <impl/tcc_util.h>

#define ERROR_NUM (-__LINE__)

/**
 * @ingroup cmd_file
 * @brief transfer string to long_64
 *
 * @param a string
 * @param len string length
 * @param v integer
 *
 * @return 0 Success
 * @return else Fail
 */
int32_t tcc_atoi64(int8_t *a, uint64_t *v)
{
	int32_t len;
	uint64_t long_64;
	int i;
	char c;

	if (a == NULL || v == NULL) {
		return -1;
	}

	len = strlen(a);

	long_64 = 0;

	if (a[0] == '0' && (a[1] == 'x' || a[1] == 'X')) {
		/* hexadecimal */
		for (i = 2; i < len; i++) {
			c = a[i];
			long_64 *= 16;
			if ('0' <= c && c <= '9') {
				long_64 += (uint32_t)(c - '0');
			} else if ('A' <= c && c <= 'F') {
				long_64 += (uint32_t)(c - 'A') + 10;
			} else if ('a' <= c && c <= 'f') {
				long_64 += (uint32_t)(c - 'a') + 10;
			} else {
				return -1;
			}
		}
	} else {
		/* decimal */
		for (i = 0; i < len; i++) {
			c = a[i];
			long_64 *= 10;
			if ('0' <= c && c <= '9') {
				long_64 += (uint32_t)(c - '0');
			} else {
				return -1;
			}
		}
	}

	*v = long_64;

	return 0;
}

/**
 * @ingroup cmd_file
 * @brief transfer string to integer
 *
 * @param a string
 * @param len string length
 * @param v integer
 *
 * @return 0 Success
 * @return else Fail
 */
int32_t tcc_atoi(int8_t *a, uint32_t *v)
{
	int32_t len;
	uint32_t integer;
	int i;
	char c;

	if (a == NULL || v == NULL) {
		return -1;
	}

	len = strlen(a);

	integer = 0;

	if (a[0] == '0' && (a[1] == 'x' || a[1] == 'X')) {
		/* hexadecimal */
		for (i = 2; i < len; i++) {
			c = a[i];
			integer *= 16;
			if ('0' <= c && c <= '9') {
				integer += (uint32_t)(c - '0');
			} else if ('A' <= c && c <= 'F') {
				integer += (uint32_t)(c - 'A') + 10;
			} else if ('a' <= c && c <= 'f') {
				integer += (uint32_t)(c - 'a') + 10;
			} else {
				return -1;
			}
		}
	} else {
		/* decimal */
		for (i = 0; i < len; i++) {
			c = a[i];
			integer *= 10;
			if ('0' <= c && c <= '9') {
				integer += (uint32_t)(c - '0');
			} else {
				return -1;
			}
		}
	}

	*v = integer;

	return 0;
}

/**
 * @ingroup tcc_util
 * @brief return directory name of file
 *
 * @param filename file name
 *
 * @return directory name
 */
int8_t* tcc_dirname(int8_t *filename)
{
	int8_t *dirname;
	int32_t len;

	len = strlen(filename);
	while (len > 0) {
		if (filename[len-1] == '/') {
			break;
		}
		len--;
	}
	if (len == 0) {
		dirname = malloc(2);
		dirname[0] = '.';
		dirname[1] = 0;
	} else {
		dirname = malloc(len);
		memcpy(dirname, filename, len);
		dirname[len-1] = 0;
	}

	return dirname;
}

/**
 * @ingroup tcc_util
 * @brief get config value
 *
 * @param h parameters for config
 * @param s config buffer
 * @param slen config buffer size
 * @param r buffer pointer
 *
 * @return string value
 */
int8_t *tcc_config_value(tcc_config_t *h, int8_t *s, int32_t slen, int32_t *r)
{
	int8_t *n;
	int32_t i, j, k, l;

	*r = 0;

	/* Calculate the size of buffer for name. */
	j = 0;
	for (i = 0; i < slen; i++) {
		if (s[i] == ';') {
			break;
		} else if (s[i] == '$') {
			if (s[i+1] == 'd') {
				j += strlen(h->dirname);
			} else {
				k = (int32_t)(s[i + 1] - '0');
				if (0 <= k && k < h->argc) {
					j += strlen(h->argv[k]);
				}
			}
		}
	}

	if (i >= slen) {
		return 0;
	}

	/* Allocate the buffer for name. */
	n = calloc(i + j, 1);

	// Set the buffer for name.
	j = 0;
	for (i = 0; i < slen; i++) {
		if (s[i] == ';') {
			n[j] = 0;
			break;
		} else if (s[i] == '$' && i + 1 < slen) {
			if (s[i+1] == 'd') {
				l = strlen(h->dirname);
				memcpy(&n[j], h->dirname, l);
				j += l;
				i++;
			} else {
				k = (int32_t)(s[i + 1] - '0');
				if (0 <= k && k < h->argc) {
					l = strlen(h->argv[k]);
					memcpy(&n[j], h->argv[k], l);
					j += l;
					i++;
				}
			}
		} else {
			n[j] = s[i];
			j++;
		}
	}

	*r = i;

	return n;
}

/**
 * @ingroup tcc_util
 * @brief get next '=' value
 *
 * @param p input buffer
 *
 * @return string value
 */
char *next_string(char *p)
{
	while (*p != 0) {
		if (*p++ == '=') {
			return p;
		}
	}
	return NULL;
}

/**
 * @ingroup tcc_util
 * @brief Copy file
 *
 * @param src source file
 * @param out target file
 * @param length length
 *
 * @return Error status
 */
int32_t tcc_copyfile(FILE *src, FILE *out, uint64_t length)
{
	char buf[4096];
	uint32_t current;

	if (length == 0) {
		current = ftell(src);
		fseek(src, 0, SEEK_END);
		length = (uint64_t)(ftell(src) - current);
		fseek(src, current, SEEK_SET);
	}

	while (length >= 4096) {
		if (fread(buf, 4096, 1, src) != 1) {
			printf("%s\n", strerror(ferror(src)));
			return ECODE_UNKNOWNERROR;
		}
		if (fwrite(buf, 4096, 1, out) != 1) {
			printf("%s\n", strerror(ferror(out)));
			return ECODE_UNKNOWNERROR;
		}
		length -= 4096;
	}
	if (length > 0) {
		if (fread(buf, length, 1, src) != 1) {
			printf("%s\n", strerror(ferror(src)));
			return ECODE_UNKNOWNERROR;
		}
		if (fwrite(buf, length, 1, out) != 1) {
			printf("%s\n", strerror(ferror(out)));
			return ECODE_UNKNOWNERROR;
		}
	}
	return ECODE_NOERROR;
}