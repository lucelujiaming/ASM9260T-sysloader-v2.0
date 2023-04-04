/*
 *  printf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */
 
#include <stdarg.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>

#ifndef __PRINTF_H__
#define __PRINTF_H__

//#ifdef CONFIG_COMMAND_ONLY
#define printf alp_printf
//#endif
//#ifdef CONFIG_COMMAND_AND_TASK
//#define printf sl_printf
//#endif

#define vprintf alp_vprintf
//
void sl_printf (const char *fmt, ...);
void serial_printf (const char *fmt, ...);
void alp_printf (const char *fmt, ...);
//void alp_vprintf (const char *fmt, va_list args);
void alp_vsprintf (char * printbuffer,const char *fmt, ...);
void panic (const char *fmt, ...);

#endif //__PRINTF_H__

