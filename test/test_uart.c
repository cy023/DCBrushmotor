/**
 * @file   test_uart.c
 * @author cy023
 * @date   2020.08.04
 * @brief  uart 測試程式
 */

#include "../src/uart.h"

int main() {
    UART_init();
    for(int i = 0; i < 100; i++) {
        printf("%d\n", i);
    }
    return 0;
}