/*
 *  vsprintf.h
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include <stdarg.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>

#ifndef __VSPRINTF_H__
#define __VSPRINTF_H__

//unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
//
//long simple_strtol(const char *cp,char **endp,unsigned int base);

/* we use this so that we can do without the ctype library */
#define is_digit(c)	((c) >= '0' && (c) <= '9')

int skip_atoi(const char **s);

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */

#define do_div(n,base) ({ \
	int __res; \
	__res = ((unsigned long) n) % (unsigned) base; \
	n = ((unsigned long) n) / (unsigned) base; \
	__res; \
})

#ifdef CFG_64BIT_VSPRINTF
char * number(char * str, long long num, int base, int size, int precision ,int type);
#else
char * number(char * str, long num, int base, int size, int precision ,int type);
#endif

/* Forward decl. needed for IP address printing stuff... */
int sprintf(char * buf, const char *fmt, ...);

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char * buf, const char *fmt, ...);
//void panic(const char *fmt, ...);

#endif 
