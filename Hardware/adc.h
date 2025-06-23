#ifndef __ADC_H
#define __ADC_H

#include "include.h"
#include "user_config.h"

enum
{
    ADC_PIN_NONE = 0x00,
    ADC_PIN_DETECT_CHARGE,  // 检测充电分压后的电压（1/11分压） P12 AN7
    ADC_PIN_DETECT_BATTERY, // 检测电池分压后的电压（1/2分压）  P05 AN4
};

/*
    在充电一端检测到可以给电池充电的电压
    样机是检测到充电输入端大于4.9V，使能给电池的充电

    adc使用 内部 2.0V作为参考电压
    单片机检测脚检测到 大于 0.44545V （4.9V 经过 1/11分压后）,
    使能给电池的充电

    对应的ad值 912.29
*/
#define ADC_VAL_ENABLE_IN_CHARGE_END ((u16)912)
/*
    在充电一端检测到 断开给电池充电的电压
    样机是检测到充电输入端小于4V，断开给电池的充电

    adc使用 内部 2.0V作为参考电压
    单片机检测脚检测到 小于 0.40V （4.0V 经过 1/11分压后）,
    断开给电池的充电

    对应的ad值 744.7272
*/
#define ADC_VAL_DISABLE_IN_CHARGE_END ((u16)744)
/*
    电池低电量时，对应的ad值
    样机是低于 2.85V ，进行涓流充电

    adc使用 内部 2.0V作为参考电压
    单片机检测脚检测到 小于 1.425V （2.85V 经过 1/2分压后），
    进行涓流充电

    对应ad值 2918.4
*/
#define ADC_VAL_BAT_IS_LOW ((u16)2918)
/*
    电池快满电时，对应的ad值
    样机是大于 3.55V ，进行涓流充电（只测试了一次，认为快满电3.6V前应该进行涓流充电）

    adc使用 内部 2.0V作为参考电压
    单片机检测脚检测到 大于 1.775 V （3.55 V 经过 1/2分压后），
    进行涓流充电

    对应ad值 3635.2
*/
#define ADC_VAL_BAT_IS_NEAR_FULL ((u16)3635)
/*
    充电期间，电池充满电时对应的ad值
    目前只知道样机充满电后的电池电压约为3.6V

    adc使用 内部 2.0V作为参考电压
    单片机检测脚检测到 大于 1.8 V （3.6 V 经过 1/2分压后），
    断开给电池的充电，不使能对应的PWM

    对应的ad值为 3686.4
*/
#define ADC_VAL_BAT_IS_FULL ((u16)3686)

extern volatile u16 adc_val;

void adc_config(void);
void adc_sel_pin(u8 adc_pin);
u16 adc_getval(void);

#endif
