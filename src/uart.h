/**
 * @file   uart.h
 * @author cy023
 * @date   2020.08.04
 * @brief  uart 功能實現
 */

#ifndef __UART_H__
#define __UART_H__

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 11059200UL

int stdio_putchar(char c, FILE *stream);
int stdio_getchar(FILE *stream);
int UART_init(void);

#endif