/*
 * Unix-compatibility API - error number definitions.
 *
 * Copyright (C) 2010 Yale University.
 * See section "MIT License" in the file LICENSES for licensing terms.
 */
#ifndef PIOS_INC_ERRNO_H
#define PIOS_INC_ERRNO_H

#include <file.h>


// A process/thread's errno variable is in the filestate structure,
// so that it won't get merged and will behave as thread-private data.
#define	errno		(files->err)

// Error numbers - keep consistent with strerror() in lib/string.c!
#define EINVAL		1	/* Invalid argument */
#define ENOENT		2	/* No such file or directory */
#define EFBIG		3	/* File too large */
#define EMFILE		4	/* Too many open files */
#define ENOTDIR		5	/* Not a directory */
#define ENAMETOOLONG	6	/* File name too long */
#define ENOSPC		7	/* No space left on device */
#define EAGAIN		8	/* Resource temporarily unavailable */
#define ECHILD		9	/* No child processes */
#define ECONFLICT	10	/* Conflict detected (PIOS-specific) */

#endif	// !PIOS_INC_ERRNO_H
