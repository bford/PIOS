/*
 * Stripped-down primitive printf-style formatting routines,
 * used in common by printf, sprintf, fprintf, etc.
 * This code is also used by both the kernel and user programs.
 *
 * Copyright (c) 1986, 1988, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * See section "BSD License" in the file LICENSES for licensing terms.
 *
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Adapted for PIOS by Bryan Ford at Yale University.
 */

#include <inc/types.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/stdarg.h>
#include <inc/assert.h>

typedef struct printstate {
	void (*putch)(int ch, void *putdat);	// character output function
	void *putdat;		// data for above function
	int padc;		// left pad character, ' ' or '0'
	int width;		// field width, -1=none
	int prec;		// numeric precision or string length, -1=none
	int signc;		// sign character: '+', '-', ' ', or -1=none
	int flags;		// flags below
	int base;		// base for numeric output
} printstate;

#define	F_L	0x01		// (at least) one 'l' specified
#define	F_LL	0x02		// (at least) two 'l's specified
#define F_ALT	0x04		// '#' alternate format flag specified
#define F_DOT	0x08		// '.' separating width from precision seen
#define F_RPAD	0x10		// '-' indiciating right padding seen

// Get an unsigned int of various possible sizes from a varargs list,
// depending on the lflag parameter.
static uintmax_t
getuint(printstate *st, va_list *ap)
{
	if (st->flags & F_LL)
		return va_arg(*ap, unsigned long long);
	else if (st->flags & F_L)
		return va_arg(*ap, unsigned long);
	else
		return va_arg(*ap, unsigned int);
}

// Same as getuint but signed - can't use getuint
// because of sign extension
static intmax_t
getint(printstate *st, va_list *ap)
{
	if (st->flags & F_LL)
		return va_arg(*ap, long long);
	else if (st->flags & F_L)
		return va_arg(*ap, long);
	else
		return va_arg(*ap, int);
}

// Print padding characters, and an optional sign before a number.
static void
putpad(printstate *st)
{
	while (--st->width >= 0)
		st->putch(st->padc, st->putdat);
}

// Print a string with a specified maximum length (-1=unlimited),
// with any appropriate left or right field padding.
static void
putstr(printstate *st, const char *str, int maxlen)
{
	const char *lim;		// find where the string actually ends
	if (maxlen < 0)
		lim = strchr(str, 0);	// find the terminating null
	else if ((lim = memchr(str, 0, maxlen)) == NULL)
		lim = str + maxlen;
	st->width -= (lim-str);		// deduct string length from field width

	if (!(st->flags & F_RPAD))	// print left-side padding
		putpad(st);		// (also leaves st->width == 0)
	while (str < lim) {
		char ch = *str++;
			st->putch(ch, st->putdat);
	}
	putpad(st);			// print right-side padding
}

// Generate a number (base <= 16) in reverse order into a string buffer.
static char *
genint(printstate *st, char *p, uintmax_t num)
{
	// first recursively print all preceding (more significant) digits
	if (num >= st->base)
		p = genint(st, p, num / st->base);	// output higher digits
	else if (st->signc >= 0)
		*p++ = st->signc;			// output leading sign
	*p++ = "0123456789abcdef"[num % st->base];	// output this digit
	return p;
}

// Print an integer with any appropriate field padding.
static void
putint(printstate *st, uintmax_t num, int base)
{
	char buf[30], *p = buf;		// big enough for any 64-bit int in octal
	st->base = base;		// select base for genint
	p = genint(st, p, num);		// output to the string buffer
	putstr(st, buf, p-buf);		// print it with left/right padding
}


// Main function to format and print a string.
void
vprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, va_list ap)
{
	register int ch, err;

	printstate st = { .putch = putch, .putdat = putdat };
	while (1) {
		while ((ch = *(unsigned char *) fmt++) != '%') {
			if (ch == '\0')
				return;
			putch(ch, putdat);
		}

		// Process a %-escape sequence
		st.padc = ' ';
		st.width = -1;
		st.prec = -1;
		st.signc = -1;
		st.flags = 0;
		st.base = 10;
		uintmax_t num;
	reswitch:
		switch (ch = *(unsigned char *) fmt++) {

		// modifier flags
		case '-': // pad on the right instead of the left
			st.flags |= F_RPAD;
			goto reswitch;

		case '+': // prefix positive numeric values with a '+' sign
			st.signc = '+';
			goto reswitch;

		case ' ': // prefix signless numeric values with a space
			if (st.signc < 0)	// (but only if no '+' is specified)
				st.signc = ' ';
			goto reswitch;

		// width or precision field
		case '0':
			if (!(st.flags & F_DOT))
				st.padc = '0'; // pad with 0's instead of spaces
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			for (st.prec = 0; ; ++fmt) {
				st.prec = st.prec * 10 + ch - '0';
				ch = *fmt;
				if (ch < '0' || ch > '9')
					break;
			}
			goto gotprec;

		case '*':
			st.prec = va_arg(ap, int);
		gotprec:
			if (!(st.flags & F_DOT)) {	// haven't seen a '.' yet?
				st.width = st.prec;	// then it's a field width
				st.prec = -1;
			}
			goto reswitch;

		case '.':
			st.flags |= F_DOT;
			goto reswitch;

		case '#':
			st.flags |= F_ALT;
			goto reswitch;

		// long flag (doubled for long long)
		case 'l':
			st.flags |= (st.flags & F_L) ? F_LL : F_L;
			goto reswitch;

		// character
		case 'c':
			putch(va_arg(ap, int), putdat);
			break;

		// string
		case 's': {
			const char *s;
			if ((s = va_arg(ap, char *)) == NULL)
				s = "(null)";
			putstr(&st, s, st.prec);
			break;
		    }

		// (signed) decimal
		case 'd':
			num = getint(&st, &ap);
			if ((intmax_t) num < 0) {
				num = -(intmax_t) num;
				st.signc = '-';
			}
			putint(&st, num, 10);
			break;

		// unsigned decimal
		case 'u':
			putint(&st, getuint(&st, &ap), 10);
			break;

		// (unsigned) octal
		case 'o':
			// Replace this with your code.
			putch('X', putdat);
			putch('X', putdat);
			putch('X', putdat);
			break;

		// (unsigned) hexadecimal
		case 'x':
			putint(&st, getuint(&st, &ap), 16);
			break;

		// pointer
		case 'p':
			putch('0', putdat);
			putch('x', putdat);
			putint(&st, (uintptr_t) va_arg(ap, void *), 16);
			break;


		// escaped '%' character
		case '%':
			putch(ch, putdat);
			break;

		// unrecognized escape sequence - just print it literally
		default:
			putch('%', putdat);
			for (fmt--; fmt[-1] != '%'; fmt--)
				/* do nothing */;
			break;
		}
	}
}

