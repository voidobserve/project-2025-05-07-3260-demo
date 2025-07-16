#include "my_debug.h"

void my_debug_led_config(void)
{
    // 公共端 COM
    P2_MD1 &= ~GPIO_P27_MODE_SEL(0x03);
    P2_MD1 |= GPIO_P27_MODE_SEL(0x01);
    FOUT_S27 = GPIO_FOUT_AF_FUNC;
    P27 = 0; // 固定输出低电平

    // LED1
    P0_MD0 &= ~GPIO_P03_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P03_MODE_SEL(0x01);
    FOUT_S03 = GPIO_FOUT_AF_FUNC;
    P03 = 0;

    // LED2
    P3_MD0 &= ~GPIO_P30_MODE_SEL(0x03);
    P3_MD0 |= GPIO_P30_MODE_SEL(0x01);
    FOUT_S03 = GPIO_FOUT_AF_FUNC;
    P30 = 0;

    // LED3
    P1_MD0 &= ~GPIO_P10_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P10_MODE_SEL(0x01);
    FOUT_S10 = GPIO_FOUT_AF_FUNC;
    P10 = 0;

    // LED4
    P0_MD1 &= ~GPIO_P07_MODE_SEL(0x03);
    P0_MD1 |= GPIO_P07_MODE_SEL(0x01);
    FOUT_S07 = GPIO_FOUT_AF_FUNC;
    P07 = 0;
}

void my_debug_led_1_on(void)
{
    P03 = 1;
}

void my_debug_led_1_off(void)
{
    P03 = 0;
}

void my_debug_led_2_on(void)
{
    P30 = 1;
}

void my_debug_led_2_off(void)
{
    P30 = 0;
}

void my_debug_led_3_on(void)
{
    P10 = 1;
}

void my_debug_led_3_off(void)
{
    P10 = 0;
}

void my_debug_led_4_on(void)
{
    P07 = 1;
}

void my_debug_led_4_off(void)
{
    P07 = 0;
}

