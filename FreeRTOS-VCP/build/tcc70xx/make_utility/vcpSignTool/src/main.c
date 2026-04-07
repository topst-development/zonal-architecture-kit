/**
 * @file main.c
 * @brief Telechips signing tool main file
 * @defgroup tcsb_main Telechips Signing Tool Main
 */
#include <stdio.h>
#include <string.h>

/* CMD Descriptors are generated automatically by makefile */
CMD_DESCS;

/**
 * print usage
 */
static void print_usage(void)
{
	unsigned long i;

	printf("\n");
	printf("Telechips Secure Image Tool v%s.%s", MAJOR_VERSION, MINOR_VERSION);
	printf("\n\n");
	printf("Usage: %s [", TOOL_NAME);
	printf(" %s", cmds[0].name);
	for (i = 1; i < CMD_NUM; i++) {
		printf(" | %s", cmds[i].name);
	}
	printf(" ]");
	printf("\n\n");
}

/**
 * @ingroup tcsb_main
 * @brief main function
 *
 * @param argc Count of parameters
 * @param argv[] Parameters
 * @param envp Environment information
 *
 * @return 0 Success
 * @retrun else Fail
 */
int main(int argc, char *argv[], char **envp)
{
	unsigned long i;

	/* command processing */
	if (argc >= 2) {
		for (i = 0; i < CMD_NUM; i++) {
			if (strcmp(argv[1], cmds[i].name) == 0) {
				return cmds[i].fnc((int32_t)argc, (int8_t**)argv, (int8_t**)envp);
			}
		}
	}

	/* print usage if command is wrong */
	print_usage();

	return -1;
}
