/*
 * Entrypoint code for PIOS user-space programs.
 * This code must always be the first object file linked into any program,
 * and must always be linked at the same memory address (see below).
 *
 * Copyright (C) 1997 Massachusetts Institute of Technology
 * See section "MIT License" in the file LICENSES for licensing terms.
 *
 * Derived from the MIT Exokernel and JOS.
 * Adapted for PIOS by Bryan Ford at Yale University.
 */

#include <inc/syscall.h>
#include <inc/vm.h>


	.text

// Start entrypoint - this is where the kernel (or our parent process)
// starts us running when we are initially loaded into a new process.
	.globl start
start:

	call	main	// run the program
	pushl	%eax	// use with main's return value as exit status
        movl	$SYS_RET, %eax
        int	$T_SYSCALL
1:	jmp 1b



