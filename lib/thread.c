/*
 * Simple "thread" fork/join functions for PIOS.
 * Since the PIOS doesn't actually allow multiple threads
 * to run in one process and share memory directly,
 * these functions use the kernel's SNAP and MERGE features
 * to emulate threads in a deterministic consistency model.
 *
 * Copyright (C) 2010 Yale University.
 * See section "MIT License" in the file LICENSES for licensing terms.
 *
 * Primary author: Bryan Ford
 */

#include <inc/stdio.h>
#include <inc/stdlib.h>
#include <inc/string.h>
#include <inc/unistd.h>
#include <inc/assert.h>
#include <inc/syscall.h>
#include <inc/vm.h>
#include <inc/file.h>
#include <inc/errno.h>

#define ALLVA		((void*) VM_USERLO)
#define ALLSIZE		(VM_USERHI - VM_USERLO)

#define SHAREVA		((void*) VM_SHARELO)
#define SHARESIZE	(VM_SHAREHI - VM_SHARELO)


#define EXIT_BARRIER 0x80000000 // Highest bit set to indicate sys_ret at barrier.

static int thread_id;


// Fork a child process/thread, returning 0 in the child and 1 in the parent.
int
tfork(uint16_t child)
{
	// Set up the register state for the child
	struct procstate ps;
	memset(&ps, 0, sizeof(ps));

	// Use some assembly magic to propagate registers to child
	// and generate an appropriate starting eip
	int isparent;
	asm volatile(
		"	movl	%%esi,%0;"
		"	movl	%%edi,%1;"
		"	movl	%%ebp,%2;"
		"	movl	%%esp,%3;"
		"	movl	$1f,%4;"
		"	movl	$1,%5;"
		"1:	"
		: "=m" (ps.tf.regs.esi),
		  "=m" (ps.tf.regs.edi),
		  "=m" (ps.tf.regs.ebp),
		  "=m" (ps.tf.esp),
		  "=m" (ps.tf.eip),
		  "=a" (isparent)
		:
		: "ebx", "ecx", "edx");
	if (!isparent) {
		return 0;	// in the child
	}

	// Fork the child, copying our entire user address space into it.
	ps.tf.regs.eax = 0;	// isparent == 0 in the child
	sys_put(SYS_REGS | SYS_COPY | SYS_SNAP | SYS_START, child,
 		&ps, ALLVA, ALLVA, ALLSIZE);

	return 1;
}

void
tjoin(uint16_t child)
{
	// Wait for the child and retrieve its CPU state.
	// If merging, leave the highest 4MB containing the stack unmerged,
	// so that the stack acts as a "thread-private" memory area.
	struct procstate ps;
	sys_get(SYS_MERGE | SYS_REGS, child, &ps, SHAREVA, SHAREVA, SHARESIZE);

	// Make sure the child exited with the expected trap number
	if (ps.tf.trapno != T_SYSCALL) {
		cprintf("  eip  0x%08x\n", ps.tf.eip);
		cprintf("  esp  0x%08x\n", ps.tf.esp);
		panic("tjoin: unexpected trap %d, expecting %d\n",
			ps.tf.trapno, T_SYSCALL);
	}
}

