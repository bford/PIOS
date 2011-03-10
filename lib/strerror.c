#include <inc/stdio.h>

char *
strerror(int err)
{
	static char *errtab[] = {
		"(no error)",
		"Invalid argument",			// EINVAL
		"No such file or directory",
		"File too large",
		"Too many open files",
		"Not a directory",
		"File name too long",
		"No space left on device",
		"Resource temporarily unavailable",
		"No child processes",
		"Conflict detected",
	};
	static char errbuf[64];

	const int tablen = sizeof(errtab)/sizeof(errtab[0]);
	if (err >= 0 && err < sizeof(errtab)/sizeof(errtab[0]))
		return errtab[err];

	sprintf(errbuf, "Unknown error code %d", err);
	return errbuf;
}

