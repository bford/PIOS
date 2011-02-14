#
# This makefile system follows the structuring conventions
# recommended by Peter Miller in his excellent paper:
#
#	Recursive Make Considered Harmful
#	http://aegis.sourceforge.net/auug97.pdf
#
# Copyright (C) 2003 Massachusetts Institute of Technology 
# See section "MIT License" in the file LICENSES for licensing terms.
# Primary authors: Bryan Ford, Eddie Kohler, Austin Clemens
#
OBJDIR := obj

ifdef LAB
SETTINGLAB := true
else
-include conf/lab.mk
endif

-include conf/env.mk

ifndef SOL
SOL := 0
endif
ifndef LABADJUST
LABADJUST := 0
endif


TOP = .

# Cross-compiler toolchain
#
# This Makefile will automatically use a cross-compiler toolchain installed
# as 'pios-*' or 'i386-elf-*', if one exists.  If the host tools ('gcc',
# 'objdump', and so forth) compile for a 32-bit x86 ELF target, that will
# be detected as well.  If you have the right compiler toolchain installed
# using a different name, set GCCPREFIX explicitly in conf/env.mk

# try to infer the correct GCCPREFIX
ifndef GCCPREFIX
GCCPREFIX := $(shell sh misc/gccprefix.sh)
endif

# try to infer the correct QEMU
ifndef QEMU
QEMU := $(shell sh misc/which-qemu.sh)
endif

# try to generate unique GDB and network port numbers
GDBPORT	:= $(shell expr `id -u` % 5000 + 25000)
NETPORT := $(shell expr `id -u` % 5000 + 30000)

# Correct option to enable the GDB stub and specify its port number to qemu.
# First is for qemu versions <= 0.10, second is for later qemu versions.
# QEMUPORT := -s -p $(GDBPORT)
QEMUPORT := -gdb tcp::$(GDBPORT)

CC	:= $(GCCPREFIX)gcc -pipe
AS	:= $(GCCPREFIX)as
AR	:= $(GCCPREFIX)ar
LD	:= $(GCCPREFIX)ld
OBJCOPY	:= $(GCCPREFIX)objcopy
OBJDUMP	:= $(GCCPREFIX)objdump
NM	:= $(GCCPREFIX)nm
GDB	:= $(GCCPREFIX)gdb

# Native commands
NCC	:= gcc $(CC_VER) -pipe
TAR	:= gtar
PERL	:= perl

# If we're not using the special "PIOS edition" of GCC,
# reconfigure the host OS's compiler for our purposes.
ifneq ($(GCCPREFIX),pios-)
CFLAGS += -nostdinc -m32
LDFLAGS += -nostdlib -m elf_i386
USER_LDFLAGS += -e start -Ttext=0x40000100
endif

# Where does GCC have its libgcc.a and libgcc's include directory?
GCCDIR := $(dir $(shell $(CC) $(CFLAGS) -print-libgcc-file-name))

# x86-64 systems may put libgcc's include directory with the 64-bit compiler
GCCALTDIR := $(dir $(shell $(CC) -print-libgcc-file-name))

# Compiler flags
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -O1 to discourage inlining, which complicates backtraces.
CFLAGS += $(DEFS) $(LABDEFS) -fno-builtin -I$(TOP) -I$(TOP)/inc \
		-I$(GCCDIR)/include -I$(GCCALTDIR)/include \
		-MD -Wall -Wno-unused -Werror -gstabs

# Add -fno-stack-protector if the option exists.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 \
		&& echo -fno-stack-protector)

LDFLAGS += -L$(OBJDIR)/lib -L$(GCCDIR)

# Compiler flags that differ for kernel versus user-level code.
KERN_CFLAGS += $(CFLAGS) -DPIOS_KERNEL
KERN_LDFLAGS += $(LDFLAGS) -nostdlib -Ttext=0x00100000 -L$(GCCDIR)
KERN_LDLIBS += $(LDLIBS) -lgcc

USER_CFLAGS += $(CFLAGS) -DPIOS_USER
USER_LDFLAGS += $(LDFLAGS)
USER_LDINIT += $(OBJDIR)/lib/crt0.o
USER_LDDEPS += $(USER_LDINIT) $(OBJDIR)/lib/libc.a
USER_LDLIBS += $(LDLIBS) -lc -lgcc

# Lists that the */Makefrag makefile fragments will add to
OBJDIRS :=

# Make sure that 'all' is the first target
all:

# Eliminate default suffix rules
.SUFFIXES:

# Delete target files if there is an error (or make is interrupted)
.DELETE_ON_ERROR:

# make it so that no intermediate .o files are ever deleted
.PRECIOUS: %.o $(OBJDIR)/boot/%.o $(OBJDIR)/kern/%.o \
	   $(OBJDIR)/lib/%.o $(OBJDIR)/fs/%.o $(OBJDIR)/net/%.o \
	   $(OBJDIR)/user/%.o




# Include Makefrags for subdirectories
include boot/Makefrag
include kern/Makefrag
include lib/Makefrag
include user/Makefrag



NCPUS = 2
IMAGES = $(OBJDIR)/kern/kernel.img
QEMUOPTS = -smp $(NCPUS) -hda $(OBJDIR)/kern/kernel.img -serial mon:stdio \
		-k en-us -m 1100M
#QEMUNET = -net socket,mcast=230.0.0.1:$(NETPORT) -net nic,model=i82559er
QEMUNET1 = -net nic,model=i82559er,macaddr=52:54:00:12:34:01 \
		-net socket,connect=:$(NETPORT) -net dump,file=node1.dump
QEMUNET2 = -net nic,model=i82559er,macaddr=52:54:00:12:34:02 \
		-net socket,listen=:$(NETPORT) -net dump,file=node2.dump

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

ifneq ($(LAB),5)
# Launch QEMU and run PIOS. Labs 1-4 need only one instance of QEMU.
qemu: $(IMAGES)
	$(QEMU) $(QEMUOPTS)
else
# Lab 5 is a distributed system, so we need (at least) two instances.
# Only one instance gets input from the terminal, to avoid confusion.
qemu: $(IMAGES)
	@rm -f node?.dump
	$(QEMU) $(QEMUOPTS) $(QEMUNET2) </dev/null | sed -e 's/^/2: /g' &
	@sleep 1
	$(QEMU) $(QEMUOPTS) $(QEMUNET1)
endif

# Launch QEMU without a virtual VGA display (use when X is unavailable).
qemu-nox: $(IMAGES)
	echo "*** Use Ctrl-a x to exit"
	$(QEMU) -nographic $(QEMUOPTS)

ifneq ($(LAB),5)
# Launch QEMU for debugging. Labs 1-4 need only one instance of QEMU.
qemu-gdb: $(IMAGES) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) $(QEMUOPTS) -S $(QEMUPORT)
else
# Launch QEMU for debugging the 2-node distributed system in Lab 5.
qemu-gdb: $(IMAGES) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	@rm -f node?.dump
	$(QEMU) $(QEMUOPTS) $(QEMUNET2) </dev/null | sed -e 's/^/2: /g' &
	@sleep 1
	$(QEMU) $(QEMUOPTS) $(QEMUNET1) -S $(QEMUPORT)
endif

# Launch QEMU for debugging, without a virtual VGA display.
qemu-gdb-nox: $(IMAGES) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUPORT)

# For deleting the build
clean:
	rm -rf $(OBJDIR) grade-out

realclean: clean
	rm -rf lab$(LAB).tar.gz

distclean: realclean
	rm -rf conf/gcc.mk

grade: misc/grade-lab$(LAB).sh
	$(V)$(MAKE) clean >/dev/null 2>/dev/null
	$(MAKE) all
	sh misc/grade-lab$(LAB).sh

tarball: realclean
	tar cf - `find . -type f | grep -v '^\.*$$' | grep -v '/CVS/' | grep -v '/\.svn/' | grep -v '/\.git/' | grep -v 'lab[0-9].*\.tar\.gz'` | gzip > lab$(LAB)-handin.tar.gz


# This magic automatically generates makefile dependencies
# for header files included from C source files we compile,
# and keeps those dependencies up-to-date every time we recompile.
# See 'misc/mergedep.pl' for more information.
$(OBJDIR)/.deps: $(foreach dir, $(OBJDIRS), $(wildcard $(OBJDIR)/$(dir)/*.d))
	@mkdir -p $(@D)
	@$(PERL) misc/mergedep.pl $@ $^

-include $(OBJDIR)/.deps

always:
	@:

.PHONY: all always \
	handin tarball clean realclean clean-labsetup distclean grade labsetup

