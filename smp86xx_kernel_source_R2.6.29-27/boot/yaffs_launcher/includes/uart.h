/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

#ifndef _UART_H_
#define _UART_H_

void uart_putc(const char c);
void uart_puts(const char *s);
void uart_init_port(int baud, int fifo);
int uart_printf(const char *fmt, ...);
int uart_peekc(void);
int uart_getc(void);
int uart_gets(char *bufptr, int size, int echo);

#define PRINTF(format...) do { if (LOCALDBG) uart_printf(format); } while (0)

#endif

