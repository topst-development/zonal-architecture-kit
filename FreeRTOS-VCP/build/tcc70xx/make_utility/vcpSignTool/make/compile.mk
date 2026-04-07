NOECHO=@

INCS := $(addprefix -I,$(INCS))

OBJS += $(call TOBUILDDIR,$(patsubst %.c,%.o,$(filter %.c,$(SRCS))))
OBJS += $(call TOBUILDDIR,$(patsubst %.s,%.o,$(filter %.s,$(SRCS))))
OBJS += $(call TOBUILDDIR,$(patsubst %.S,%.o,$(filter %.S,$(SRCS))))
OBJS += $(call TOBUILDDIR,$(patsubst %.key,%.o,$(filter %.key,$(SRCS))))

$(BUILDDIR)/%.o: %.s
	$(NOECHO) echo -e "[\x1b[1;34mAS\x1b[0m] $< => $@ (ASSEMBLE)"
	$(NOECHO) $(MKDIR)
	$(NOECHO) $(AS) $(GFLAGS) $(AFLAGS) $(INCS) $< -o $@

$(BUILDDIR)/%.o: %.S
	$(NOECHO) echo -e "[\x1b[1;34mCC\x1b[0m] $< => $@ (ASSEMBLE)"
	$(NOECHO) $(MKDIR)
	$(NOECHO) gcc $(GFLAGS) $(AFLAGS) $(INCS) $(DEFS) -c $< -o $@

$(BUILDDIR)/%.o: %.c
	$(NOECHO) echo -e "[\x1b[1;34mCC\x1b[0m] $< => $@ (C)"
	$(NOECHO) $(MKDIR)
	$(NOECHO) gcc $(GFLAGS) $(CFLAGS) $(INCS) $(DEFS) -c $< -o $@

$(BUILDDIR)/%.o: %.key
	$(NOECHO) echo -e "[\x1b[1;34mCC\x1b[0m] $< => $@ (C)"
	$(NOECHO) $(MKDIR)
	$(NOECHO) ld -r -b binary $< -o $@
