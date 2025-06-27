
#ifndef __USER_CONFIG_H
#define __USER_CONFIG_H

#include "include.h"
#include <stdio.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#include "uart0.h"
#include "timer0.h"
#include "timer1.h"
#include "adc.h"
#include "charge_handle.h"
#include "led_handle.h" 

#define LED_1_PIN P11
#define LED_2_PIN P12
#define LED_3_PIN P13
#define LED_4_PIN P14
#define LED_5_PIN P15

// LED点亮或关闭时，对应的引脚电平
#define LED_ON_LEVEL 1
#define LED_OFF_LEVEL 0

typedef union
{
    unsigned char byte;
    struct
    {
        u8 bit0 : 1;
        u8 bit1 : 1;
        u8 bit2 : 1;
        u8 bit3 : 1;
        u8 bit4 : 1;
        u8 bit5 : 1;
        u8 bit6 : 1;
        u8 bit7 : 1;
    } bits;
} bit_flag;
extern volatile bit_flag flag1;
extern volatile bit_flag flag2;

#define flag_is_led_1_enable flag2.bits.bit0 // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
#define flag_is_led_2_enable flag2.bits.bit1 // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
#define flag_is_led_3_enable flag2.bits.bit2 // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
#define flag_is_led_4_enable flag2.bits.bit3 // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
#define flag_is_led_5_enable flag2.bits.bit4 // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮

#define LED_1_ON()                    \
    {                                 \
        do                            \
        {                             \
            flag_is_led_1_enable = 1; \
        } while (0);                  \
    }
#define LED_1_OFF()                   \
    {                                 \
        do                            \
        {                             \
            flag_is_led_1_enable = 0; \
        } while (0);                  \
    }
#define LED_2_ON()                    \
    {                                 \
        do                            \
        {                             \
            flag_is_led_2_enable = 1; \
        } while (0);                  \
    }
#define LED_2_OFF()                   \
    {                                 \
        do                            \
        {                             \
            flag_is_led_2_enable = 0; \
        } while (0);                  \
    }
#define LED_3_ON()                    \
    {                                 \
        do                            \
        {                             \
            flag_is_led_3_enable = 1; \
        } while (0);                  \
    }
#define LED_3_OFF()                   \
    {                                 \
        do                            \
        {                             \
            flag_is_led_3_enable = 0; \
        } while (0);                  \
    }
#define LED_4_ON()                    \
    {                                 \
        do                            \
        {                             \
            flag_is_led_4_enable = 1; \
        } while (0);                  \
    }
#define LED_4_OFF()                   \
    {                                 \
        do                            \
        {                             \
            flag_is_led_4_enable = 0; \
        } while (0);                  \
    }
#define LED_5_ON()                    \
    {                                 \
        do                            \
        {                             \
            flag_is_led_5_enable = 1; \
        } while (0);                  \
    }
#define LED_5_OFF()                   \
    {                                 \
        do                            \
        {                             \
            flag_is_led_5_enable = 0; \
        } while (0);                  \
    }

#endif