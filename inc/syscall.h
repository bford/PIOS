/*
 * PIOS system call definitions.
 *
 * Copyright (C) 2010 Yale University.
 * See section "MIT License" in the file LICENSES for licensing terms.
 *
 * Primary author: Bryan Ford
 */

#ifndef PIOS_INC_SYSCALL_H
#define PIOS_INC_SYSCALL_H

#include <trap.h>


// System call command codes (passed in EAX)
#define SYS_TYPE	0x0000000f	// Basic operation type
#define SYS_CPUTS	0x00000000	// Write debugging string to console
#define SYS_PUT		0x00000001	// Push data to child and start it
#define SYS_GET		0x00000002	// Pull results from child
#define SYS_RET		0x00000003	// Return to parent

#define SYS_START	0x00000010	// Put: start child running

#define SYS_REGS	0x00001000	// Get/put register state
#define SYS_FPU		0x00002000	// Get/put FPU state (with SYS_REGS)
#define SYS_MEM		0x00004000	// Get/put memory mappings

#define SYS_MEMOP	0x00030000	// Get/put memory operation
#define SYS_ZERO	0x00010000	// Get/put fresh zero-filled memory
#define SYS_COPY	0x00020000	// Get/put virtual copy
#define SYS_MERGE	0x00030000	// Get: diffs only from last snapshot
#define SYS_SNAP	0x00040000	// Put: snapshot child state

#define SYS_PERM	0x00000100	// Set memory permissions on get/put
#define SYS_READ	0x00000200	// Read permission (NB: in PTE_AVAIL)
#define SYS_WRITE	0x00000400	// Write permission (NB: in PTE_AVAIL)
#define SYS_RW		0x00000600	// Both read and write permission


// Register conventions for CPUTS system call (write to debug console):
//	EAX:	System call command
//	EBX:	User pointer to string to output to debug console,
//		up to CPUTS_MAX characters long (see inc/assert.h)


// Register conventions on GET/PUT system call entry:
//	EAX:	System call command/flags (SYS_*)
//	EDX:	bits 15-8: Node number to migrate to, 0 for current
//		bits 7-0: Child process number on above node to get/put
//	EBX:	Get/put CPU state pointer for SYS_REGS and/or SYS_FPU)
//	ECX:	Get/put memory region size
//	ESI:	Get/put local memory region start
//	EDI:	Get/put child memory region start
//	EBP:	reserved


#ifndef __ASSEMBLER__

// Process state save area format for GET/PUT with SYS_REGS flags
typedef struct procstate {
	trapframe	tf;		// general registers
	uint32_t	pff;		// process feature flags - see below
	fxsave		fx;		// x87/MMX/XMM registers
} procstate;

// process feature enable/status flags
#define PFF_USEFPU	0x0001		// process has used the FPU
#define PFF_NONDET	0x0100		// enable nondeterministic features
#define PFF_ICNT	0x0200		// enable instruction count/recovery


static void gcc_inline
sys_cputs(const char *s)
{
	// Pass system call number and flags in EAX,
	// parameters in other registers.
	// Interrupt kernel with vector T_SYSCALL.
	//
	// The "volatile" tells the assembler not to optimize
	// this instruction away just because it doesn't
	// look to the compiler like it computes useful values.
	// 
	// The last clause tells the assembler that this can
	// potentially change the condition codes and arbitrary
	// memory locations.

	asm volatile("int %0" :
		: "i" (T_SYSCALL),
		  "a" (SYS_CPUTS),
		  "b" (s)
		: "cc", "memory");
}

static void gcc_inline
sys_put(uint32_t flags, uint16_t child, procstate *save,
		void *localsrc, void *childdest, size_t size)
{
	asm volatile("int %0" :
		: "i" (T_SYSCALL),
		  "a" (SYS_PUT | flags),
		  "b" (save),
		  "d" (child),
		  "S" (localsrc),
		  "D" (childdest),
		  "c" (size)
		: "cc", "memory");
}

static void gcc_inline
sys_get(uint32_t flags, uint16_t child, procstate *save,
		void *childsrc, void *localdest, size_t size)
{
	asm volatile("int %0" :
		: "i" (T_SYSCALL),
		  "a" (SYS_GET | flags),
		  "b" (save),
		  "d" (child),
		  "S" (childsrc),
		  "D" (localdest),
		  "c" (size)
		: "cc", "memory");
}

static void gcc_inline
sys_ret(void)
{
	asm volatile("int %0" : :
		"i" (T_SYSCALL),
		"a" (SYS_RET));
}


#endif /* !__ASSEMBLER__ */

#endif /* !PIOS_INC_SYSCALL_H */
