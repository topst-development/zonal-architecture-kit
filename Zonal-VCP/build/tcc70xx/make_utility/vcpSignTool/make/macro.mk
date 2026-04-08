# Find the local dir of the make file
GET_LOCAL_DIR    = $(patsubst %/,%,$(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))

# makes sure the target dir exists
MKDIR = if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi

# prepends the BUILD_DIR var to each item in the list
TOBUILDDIR = $(addprefix $(BUILDDIR)/,$(1))

define MAKE_CMD_HEADER
	$(MKDIR); \
	echo -e "[\x1b[1;34mGenerating\x1b[0m] $1"
	rm -f $1.tmp; \
	echo "#include <stdint.h>" >> $1.tmp; \
	echo "" >> $1.tmp; \
	for d in `echo $($2)`; do \
		s=`echo $$d | sed 's/-/_/'`; \
		echo "int32_t cmd_$$s(int32_t argc, int8_t *argv[], int8_t **envp);" >> $1.tmp; \
	done; \
	echo "" >> $1.tmp; \
	echo "#define CMD_DESCS \\" >> $1.tmp; \
	echo "	struct cmd_desc { \\" >> $1.tmp; \
	echo "		const char *name; \\" >> $1.tmp; \
	echo "		int32_t (*fnc)(int32_t, int8_t **, int8_t **); \\" >> $1.tmp; \
	echo "	}; \\" >> $1.tmp; \
	echo "	\\" >> $1.tmp; \
	echo "	static struct cmd_desc cmds[] = { \\" >> $1.tmp; \
	for d in `echo $($2)`; do \
		s=`echo $$d | sed 's/-/_/'`; \
		echo "		{ .name = \"$$d\", .fnc = cmd_$$s, }, \\" >> $1.tmp; \
	done; \
	echo "	};" >> $1.tmp; \
	echo "" >> $1.tmp; \
	echo "#define CMD_NUM (sizeof(cmds)/sizeof(struct cmd_desc))" >> $1.tmp; \
	echo "" >> $1.tmp; \
	cp $1.tmp $1;
endef
