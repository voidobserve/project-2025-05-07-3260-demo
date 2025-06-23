#include "uart0.h"

// 重写putchar()函数
char putchar(char uart_data)
{
    UART0_DATA = uart_data;
    while (!(UART0_STA & UART_TX_DONE(0x1)))
        ; // 等待发送完成

    return uart_data;
}

void uart0_config(void)
{
    // 初始化打印
    // TX
    P0_MD0 &= ~GPIO_P02_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P02_MODE_SEL(0x01);
    FOUT_S02 = GPIO_FOUT_UART0_TX;

    // P2_MD1 &= ~GPIO_P26_MODE_SEL(0x03);
    // P2_MD1 |= GPIO_P26_MODE_SEL(0x01);
    // FOUT_S26 = GPIO_FOUT_UART0_TX;
    UART0_BAUD1 = (((SYSCLK - UART0_BAUD) / UART0_BAUD) >> 8) & 0xFF;
    UART0_BAUD0 = ((SYSCLK - UART0_BAUD) / UART0_BAUD) & 0xFF;
    UART0_CON0 = UART_STOP_BIT(0x0) |
                 UART_EN(0x1); // 8bit数据，1bit停止位
}