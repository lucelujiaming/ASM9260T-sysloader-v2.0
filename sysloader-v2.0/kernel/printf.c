/*
 *  printf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */
 
#include <common.h>
#include <kernel/printf.h>

#define CFG_PBSIZE 256

void sl_printf (const char *fmt, ...){
    alp_printf (fmt);
    alp_printf("AS3310$ ");
}

void alp_printf (const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CFG_PBSIZE];

	va_start (args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf (printbuffer, fmt, args);
	va_end (args);

	/* Print the string */
	puts (printbuffer);
}

void alp_vsprintf (char * printbuffer,const char *fmt, ...)
{
	va_list args;
	uint i;
	va_start (args, fmt);
	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf (printbuffer, fmt, args);
	va_end (args);
}

#if CONFIG_GPIO_UART
void alp_gpio_printf (const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CFG_PBSIZE];

	va_start (args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf (printbuffer, fmt, args);
	va_end (args);

	/* Print the string */
	gpio_uart_puts (printbuffer);
}
#endif //CONFIG_GPIO_UART

void panic (const char *fmt, ...){
    alp_printf (fmt);
    while(1);
}

//void alp_vprintf (const char *fmt, va_list args)
//{
//	uint i;
//	char printbuffer[CFG_PBSIZE];
//
//	/* For this to work, printbuffer must be larger than
//	 * anything we ever want to print.
//	 */
//	i = vsprintf (printbuffer, fmt, args);
//
//	/* Print the string */
//	puts (printbuffer);
//}
