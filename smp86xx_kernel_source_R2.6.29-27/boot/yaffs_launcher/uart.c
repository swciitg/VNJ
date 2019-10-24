/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

#include "yaffs_launcher.h"

#include "includes/addrspace.h"
#include "includes/rmem86xxid.h"

#include "includes/uart.h"
#include "includes/util.h"
#include "includes/vsprintf.h"

extern unsigned long tangox_get_sysclock(void);
extern unsigned long cpu_uart_base;

void uart_putc(const char c)
{
	if (c != '\n') {
		while ((gbus_readl(cpu_uart_base + CPU_UART_LSR) & 0x20) == 0);
		gbus_writel(cpu_uart_base + CPU_UART_THR, (unsigned long)c);
	} else {
		while ((gbus_readl(cpu_uart_base + CPU_UART_LSR) & 0x20) == 0);
		gbus_writel(cpu_uart_base + CPU_UART_THR, 0x0d);
		while ((gbus_readl(cpu_uart_base + CPU_UART_LSR) & 0x20) == 0);
		gbus_writel(cpu_uart_base + CPU_UART_THR, 0x0a);
	}
}

void uart_puts(const char *s)
{
	for (; *s != '\0'; s++)
		uart_putc(*s);
}

int uart_printf(const char *fmt, ...)
{
	char buf[256];
	va_list args;
	int i;
                                                                                
	va_start(args, fmt);
	i = vsprintf(buf, fmt, args); /* hopefully i < sizeof(buf) */
	va_end(args);
                                                                                
	uart_puts(buf);
	return(i);
}

int uart_peekc(void)
{
	if ((gbus_readl(cpu_uart_base + CPU_UART_LSR) & 0x01) != 0)
		return((int)gbus_readl(cpu_uart_base + CPU_UART_RBR));
	else
		return(-1);
}
                                                                                
int uart_getc(void)
{
	while ((gbus_readl(cpu_uart_base + CPU_UART_LSR) & 0x01) == 0)
		;
	return((int)gbus_readl(cpu_uart_base + CPU_UART_RBR));
}

int uart_gets(char *bufptr, int size, int echo)
{ 
    char c, *buf = bufptr;
    int i;
    for (i = 0; i < (size - 1); ) {
	c = uart_getc();
	if (c == '\b') {
	    buf--;
	    i--;
	    if (buf >= bufptr) {
		if (echo) {
		    uart_putc(c);
		    uart_putc(' ');
		    uart_putc(c);
		}
	    } else {
		buf++;
		i++;
	    }
	    continue;
	}
	if (echo)
	    uart_putc(c);
	if ((c != '\n') && (c != '\a') && (c != '\r'))
	    *buf = c;
	else
	    break;
	i++;
	buf++;
    }
    *buf = '\0';
    uart_putc('\n');
    return(i);
}

