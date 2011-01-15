/*
 * Standard I/O definitions, mostly inline with the standard C/Unix API
 * (except for the PIOS-specific "console printing" routines cprintf/cputs,
 * which are intended for debugging purposes only).
 *
 * Copyright (C) 1997 Massachusetts Institute of Technology
 * See section "MIT License" in the file LICENSES for licensing terms.
 *
 * Derived from the MIT Exokernel and JOS.
 * Adapted for PIOS by Bryan Ford at Yale University.
 */

#ifndef PIOS_INC_STDIO_H
#define PIOS_INC_STDIO_H

#include <types.h>
#include <stdarg.h>

#ifndef NULL
#define NULL	((void *) 0)
#endif /* !NULL */

// Primitive formatted printing functions: lib/printfmt.c
void	printfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...);
void	vprintfmt(void (*putch)(int, void*), void *putdat,
		const char *fmt, va_list);

// Debug console output functions.
// These are available in both the PIOS kernel and in user space,
// but are implemented differently in user space and in the kernel.
void	cputs(const char *str);			// lib/cputs.c or kern/cons.c
int	cprintf(const char *fmt, ...);		// lib/cprintf.c
int	vcprintf(const char *fmt, va_list);	// lib/cprintf.c

#endif /* !PIOS_INC_STDIO_H */
