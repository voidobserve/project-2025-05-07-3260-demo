#ifndef __TIMER_2_H
#define __TIMER_2_H

#include "include.h"
#include "user_config.h"

// duty_val 占空比值，不是以百分比为单位
#define SET_LIGHT_PWM_DUTY(duty_val)   \
    do                                 \
    {                                  \
        timer2_set_pwm_duty(duty_val); \
    } while (0);

#define LIGHT_ON()           \
    do                       \
    {                        \
        timer2_pwm_enable(); \
    } while (0);

#define LIGHT_OFF()           \
    do                        \
    {                         \
        timer2_pwm_disable(); \
    } while (0);

void timer2_pwm_config(void);

// 开启PWM，引脚复用为pwm输出
void timer2_pwm_enable(void);

// 关闭pwm，引脚输出低电平
void timer2_pwm_disable(void);

void timer2_set_pwm_duty(u8 pwm_duty_val);

#endif
