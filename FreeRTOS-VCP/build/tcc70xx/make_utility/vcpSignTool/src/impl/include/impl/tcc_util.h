
#define ECODE_NOERROR (0)
#define ECODE_UNKNOWNERROR (-1)
#define ECODE_INVALID_ARGUMENT (-100)

typedef struct {
	int8_t *dirname;
	int8_t **argv;
	int32_t argc;
} tcc_config_t;

int32_t tcc_atoi(int8_t *a, uint32_t *v);
int32_t tcc_atoi64(int8_t *a, uint64_t *v);
int8_t* tcc_dirname(int8_t *filename);
int8_t *tcc_config_value(tcc_config_t *h, int8_t *s, int32_t slen, int32_t *r);
char *next_string(char *p);
int32_t tcc_copyfile(FILE *src, FILE *out, uint64_t length);
