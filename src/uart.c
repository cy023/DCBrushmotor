/**
 * @file   uart.c
 * @author cy023
 * @date   2020.08.04
 * @brief  uart 功能實現
 */

#include "./uart.h"

static FILE STDIO_BUFFER = FDEV_SETUP_STREAM(stdio_putchar, stdio_getchar, _FDEV_SETUP_RW);

int stdio_putchar (char c, FILE *stream)
{
    if (c == '\n')
        stdio_putchar('\r', stream);

    while((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = c;
    return 0;
}

int stdio_getchar (FILE *stream)
{
    int UDR_Buff;
    while ((UCSR0A & (1<<RXC0)) == 0);
    UDR_Buff = UDR0;
    stdio_putchar(UDR_Buff,stream);
    return UDR_Buff;
}

int UART_init (void)
{
    unsigned int baud;
    baud = F_CPU/16/38400-1;
    UBRR0H = (unsigned char)(baud>>8);
    UBRR0L = (unsigned char)baud;
    UCSR0B |= (1<<RXEN0) | (1<<TXEN0);
    UCSR0C |= (3<<UCSZ00);
    stdout = &STDIO_BUFFER;
    stdin  = &STDIO_BUFFER;
    return 0;
}
