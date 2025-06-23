/**
 ******************************************************************************
 * @file    main.c
 * @author  HUGE-IC Application Team
 * @version V1.0.0
 * @date    01-05-2021
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 HUGE-IC</center></h2>
 *
 * 版权说明后续补上
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include <stdio.h>

#include "user_config.h"

#if 0

void sys_init(void)
{
    GIE = 0;
    CLR_RAM();
    IO_Init();
    timer0_pwm_config();
    timer1_config();
    // timer2_pwm_config(); // 控制 主灯 灯光 的pwm

    adc_config();

    INTE |= (0x01 << 1); // 使能 timer1中断

    delay_ms(1); // 等待系统稳定
    GIE = 1;
}

// 充电控制
void charge_handle(void)
{
    // static volatile u8 cur_charge_status = CUR_CHARGE_STATUS_NONE;
    u16 charging_adc_val;
    u16 bat_adc_val;

    adc_sel_pin(ADC_PIN_DETECT_CHARGE);
    charging_adc_val = adc_getval(); // 采集充电输入对应的ad值
    adc_sel_pin(ADC_PIN_DETECT_BATTERY);
    bat_adc_val = adc_getval(); // 采集电池电压对应的ad值

    // 如果当前未在充电
    if (CUR_CHARGE_STATUS_NONE == cur_charge_status)
    {
        if (charging_adc_val >= ADC_VAL_ENABLE_IN_CHARGE_END)
        {
            // 如果在未充电时，检测到充电输入电压而使能充电
            cur_charge_status = CUR_CHARGE_STATUS_IN_CHARGING;
        }
    }
    else // 如果当前正在充电
    {
        static u8 cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;

        if (charging_adc_val <= ADC_VAL_DISABLE_IN_CHARGE_END ||
            charging_adc_val >= 2420 || /* 充电输入电压大于 13V ，断开充电 ，公式还有缺陷*/
            bat_adc_val >= ADC_VAL_BAT_IS_FULL)
        {
            // 充电输入的电压小于一定值，断开充电
            // 电池 已经满电， 断开充电
            timer0_pwm_disable();
            cur_charge_status = CUR_CHARGE_STATUS_NONE;
            cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;
            return;
        }

        // 电池电压小于一定值或是大于一定值，进行涓流充电
        if (bat_adc_val <= ADC_VAL_BAT_IS_LOW ||
            bat_adc_val >= ADC_VAL_BAT_IS_NEAR_FULL)
        {
            if (CUR_CHARGING_PWM_STATUS_LOW_FEQ != cur_charging_pwm_status)
            {
                timer0_pwm_set_low_feq(); // 函数内部设定了固定频率和固定占空比
                cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_LOW_FEQ;
            }
        }
        else // 如果电池电压不是在涓流充电的范围
        {
            u32 tmp;

            // 如果电池电压不在涓流充电的区间，则正常充电
            // 注意这里不能随意改变 PWM占空比，最好设置PWM占空比为0
            if (CUR_CHARGING_PWM_STATUS_HIGH_FRQ != cur_charging_pwm_status)
            {
                timer0_pwm_set_high_feq();
                cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_HIGH_FRQ;
            }

            // 根据充电电压调整pwm占空比

            // 正常充电时，才调节控制充电的PWM占空比
            if (CUR_CHARGING_PWM_STATUS_HIGH_FRQ == cur_charging_pwm_status)
            {
                /*
                       测得样机充电控制的功能：
                       5V输入，pwm 105Khz，90%，（可能要使用 105Khz，85%）；12V输入，pwm 105Khz，30%
                       用（5V，85%）和（12V，30%）这两个点来计算，
                       已知两点坐标(5,85),(12,30)，求对应的方程，
                       对应直线的方程： y  =  ?7.857x  + 124.286
                       y 对应占空比，单位：%；x 对应充电输入电压，单位：V
                       在公式上去掉小数点：
                       1000y = -7857x + 124286

                       单片机adc使用内部2.0V参考电压，
                       将公式转换成 占空比 和 ad值 的关系:
                       外部充电输入的电压 1/11 分压后，到单片机检测脚
                       外部输入电压 / 11 / 单片机adc参考电压 * 4096 == 单片机采集到的ad值
                       外部输入电压 == 单片机采集到的ad值 / 4096 * 单片机adc参考电压 * 11
                       换算成单片机可以计算的形式：
                       外部输入电压 == 单片机采集到的ad值 * 单片机adc参考电压 * 11 / 4096
                       x = adc_val * 2 * 11 / 4096

                       得到的公式：
                       1000y = 124286 - 7857 (adc_val * 2 * 11 / 4096)
                       1000y = 124286 - adc_val * 7857 * 2 * 11 / 4096
                       这个公式运算不会超出 2^32 范围
                   */
                tmp = (u32)124286 - (u32)charging_adc_val * 7857 * 2 * 11 / 4096;
                // 得到的 tmp 是1000倍的占空比值，需要除以1000， 再乘以 定时器重载值/100,得到定时器对应的占空比值
                T1DATA = (u32)T1LOAD * (u32)tmp / 1000 / 100; // 最终的占空比值
            }
        }
    }
}

#endif

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
void main(void)
{
    // 看门狗默认打开, 复位时间2s
    WDT_KEY = WDT_KEY_VAL(0xDD); //  关闭看门狗 (如需配置看门狗请查看“WDT\WDT_Reset”示例)

    system_init();

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55;  // 解除写保护
    IO_MAP &= ~0x01; // 清除这个寄存器的值，实现关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;  // 写一个无效的数据，触发写保护

    uart0_config();
    // timer0_config();
    timer1_pwm_config();
    timer1_pwm_disable();

    // timer1_set_pwm_high_feq();

    adc_config();

    printf("sys reset\n");

    // adc_sel_pin(ADC_PIN_DETECT_CHARGE); // 测试通过，可以正常检测对应的电压值
    adc_sel_pin(ADC_PIN_DETECT_BATTERY); // 测试通过，可以正常检测对应的电压值

    // PWM_CTL_FOR_CHARGING_SET_HIGH_FEQ();
    PWM_CTL_FOR_CHARGING_SET_LOW_FEQ();

    // {
    //     u32 tmp = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * 90 / 100;

    //     TMR1_PWMH = (tmp >> 8) & 0xFF;
    //     TMR1_PWML = tmp & 0xFF;
    // }

    while (1)
    {
        // charge_handle();
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/
