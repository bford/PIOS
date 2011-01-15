/*
 * Generic type definitions and macros used throughout PIOS.
 * Most are C/Unix standard, with a few PIOS-specific exceptions.
 *
 * Copyright (c) 1982, 1986, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * See section "BSD License" in the file LICENSES for licensing terms.
 *
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 * Adapted for PIOS by Bryan Ford at Yale University.
 */

#ifndef PIOS_INC_TYPES_H
#define PIOS_INC_TYPES_H

#ifndef NULL
#define NULL ((void*) 0)
#endif

// Represents true-or-false values
typedef int bool;
#define false 0
#define true 1

// Explicitly-sized versions of integer types
typedef signed char		int8_t;
typedef unsigned char		uint8_t;
typedef short			int16_t;
typedef unsigned short		uint16_t;
typedef int			int32_t;
typedef unsigned int		uint32_t;
typedef long long		int64_t;
typedef unsigned long long	uint64_t;

// Pointers and addresses are 32 bits long.
// We use pointer types to represent virtual addresses,
// and [u]intptr_t to represent the numerical values of virtual addresses.
typedef int			intptr_t;	// pointer-size signed integer
typedef unsigned		uintptr_t;	// pointer-size unsigned integer
typedef int			ptrdiff_t;	// difference between pointers

// size_t is used for memory object sizes, and ssize_t is a signed analog.
typedef unsigned		size_t;
typedef int			ssize_t;

// intmax_t and uintmax_t represent the maximum-size integers supported.
typedef long long		intmax_t;
typedef unsigned long long	uintmax_t;

// Floating-point types matching the size at which the compiler
// actually evaluates floating-point expressions of a given type. (math.h)
typedef	double			double_t;
typedef	float			float_t;

// Unix API compatibility types
typedef int			off_t;		// file offsets and lengths
typedef int			pid_t;		// process IDs
typedef int			ino_t;		// file inode numbers
typedef int			mode_t;		// file mode flags

// Efficient min and max operations
#define MIN(_a, _b)						\
({								\
	typeof(_a) __a = (_a);					\
	typeof(_b) __b = (_b);					\
	__a <= __b ? __a : __b;					\
})
#define MAX(_a, _b)						\
({								\
	typeof(_a) __a = (_a);					\
	typeof(_b) __b = (_b);					\
	__a >= __b ? __a : __b;					\
})

// Rounding operations (efficient when n is a power of 2)
// Round down to the nearest multiple of n
#define ROUNDDOWN(a, n)						\
({								\
	uint32_t __a = (uint32_t) (a);				\
	(typeof(a)) (__a - __a % (n));				\
})
// Round up to the nearest multiple of n
#define ROUNDUP(a, n)						\
({								\
	uint32_t __n = (uint32_t) (n);				\
	(typeof(a)) (ROUNDDOWN((uint32_t) (a) + __n - 1, __n));	\
})

// Return the offset of 'member' relative to the beginning of a struct type
#define offsetof(type, member)  ((size_t) (&((type*)0)->member))

// Make the compiler think a value is getting used, even if it isn't.
#define USED(x)		(void)(x)


#endif /* !PIOS_INC_TYPES_H */
