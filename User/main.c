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

volatile bit_flag flag1 = {0};
volatile bit_flag flag2 = {0};

// TODO：3260使用16位寄存器，7361使用8位寄存器，要进行适配修改 
volatile u16 cur_light_pwm_duty_val = 0;    // 当前灯光对应的占空比值 
volatile u16 expect_light_pwm_duty_val = 0; // 期望调节到的、灯光对应的占空比值 

volatile u8 flag_is_light_adjust_time_come = 0; // 调节灯光的时间到来，目前为1s 

// =================================================================
// 充电控制相关变量                                                 //
// =================================================================
volatile u16 bat_adc_val;                                           // 电池电压检测脚采集到的ad值
volatile u16 charging_adc_val;                                      // 充电电压检测脚采集的ad值
volatile u16 current_adc_val;                                       // 充电电流检测脚采集的ad值
volatile u8 flag_is_charging_adjust_time_come = 0;                  // 调节充电的时间到来
volatile u8 cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE; // 控制充电的PWM状态
volatile u8 cur_charge_phase = CUR_CHARGE_PHASE_NONE;               // 记录当前充电阶段

volatile u32 light_adjust_time_cnt = 0;    // 调节灯光的时间计数，暂定为每1s加一
volatile u8 light_ctl_phase_in_rate_1 = 1; // 在放电速率M1时，使用到的变量，在计算公式里面用作系数，每次唤醒时需要初始化为1
// volatile u8 flag_is_tim_turn_off_pwm = 0; // 标志位，在涓流充电期间，定时器是否关闭了PWM输出

// =================================================================
// 指示灯控制相关变量                                               //
// =================================================================
volatile u8 cur_initial_discharge_gear = 5; // 初始放电挡位（需要记忆）
volatile u8 cur_discharge_rate = 1;         // 初始放电速率（需要记忆）
volatile u8 cur_led_mode; // 当前的LED模式
volatile u8 cur_led_gear;  // 当前led挡位
volatile u8 last_led_gear; // 上次led挡位
volatile u8 cur_led_gear_in_charging; // 充电指示，对应的挡位

//
void led_pin_config(void)
{
    P1_MD0 &= ~GPIO_P11_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P11_MODE_SEL(0x01);
    FOUT_S11 = GPIO_FOUT_AF_FUNC;
    P11 = 0; // 如果不给初始值，上电之后，指示灯会闪一下

    P1_MD0 &= ~GPIO_P12_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P12_MODE_SEL(0x01);
    FOUT_S12 = GPIO_FOUT_AF_FUNC;
    P12 = 0;

    P1_MD0 &= ~GPIO_P13_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P13_MODE_SEL(0x01);
    FOUT_S13 = GPIO_FOUT_AF_FUNC;
    P13 = 0;

    P1_MD1 &= ~GPIO_P14_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P14_MODE_SEL(0x01);
    FOUT_S14 = GPIO_FOUT_AF_FUNC;
    P14 = 0;

    P1_MD1 &= ~GPIO_P15_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P15_MODE_SEL(0x01);
    FOUT_S15 = GPIO_FOUT_AF_FUNC;
    P15 = 0;
}

// 变量、参数，初始化
void param_init(void)
{
    light_ctl_phase_in_rate_1 = 1;
}

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
    my_debug_led_config();

    timer0_config();
    timer1_pwm_config(); // 控制充电的PWM
    timer1_pwm_disable();
    timer2_pwm_config(); // 控制灯光的pwm
    timer2_pwm_disable();

    // timer1_set_pwm_high_feq();
    // TODO: 7361不用加这个引脚配置:
    led_pin_config();

    // 红外接收引脚：
    P2_MD0 &= ~(GPIO_P23_MODE_SEL(0x03)); // 输入模式
    P2_PU |= GPIO_P23_PULL_UP(0x01);      // 上拉

    adc_config();

    printf("sys reset\n"); // 打印至少占用1012字节空间

    // TODO:
    // 上电后，需要先点亮红色指示灯，再变为电池电量指示模式
    // LED_1_ON();
    // delay_ms(1000);

    // cur_led_mode = CUR_LED_MODE_INITIAL_DISCHARGE_GEAR;

#if 0
    cur_led_mode = CUR_LED_MODE_BAT_INDICATOR; // 电池电量指示模式
    cur_initial_discharge_gear = 5;
    cur_discharge_rate = 3;
#endif

    // timer1_set_pwm_high_feq();

    // {
    //     u16 pwm_reg = 0;                                     // 存放要写入到寄存器中的占空比值
    //     pwm_reg = (u32)TIMER1_LOW_FEQ_PEROID_VAL * 13 / 100; // 最终的占空比值
    //     TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
    //     TMR1_PWML = pwm_reg & 0xFF;
    //     timer1_set_pwm_low_feq();
    //     cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_LOW_FEQ;
    //     // cur_charge_phase = CUR_CHARGE_PHASE_;
    // }

    while (1)
    {
#if 1
        // adc_sel_pin(ADC_PIN_DETECT_BATTERY);
        // bat_adc_val = adc_getval(); // 采集电池电压对应的ad值
        // adc_sel_pin(ADC_PIN_DETECT_CHARGE);
        // adc_sel_pin_charge(CUR_ADC_REF_3_0_VOL);
        // charging_adc_val = adc_getval();
        // adc_sel_pin(ADC_PIN_DETECT_CURRENT);
        // current_adc_val = adc_getval(); // 采集流入电池的电流对应的ad值

        // printf("current_adc_val %u\n", current_adc_val);

        charge_handle();

        // ir_handle();
        // charge_handle();
        // led_handle_update_percent_of_bat();

        if (CUR_CHARGE_PHASE_NONE == cur_charge_phase)
        {
            my_debug_led_2_off();
            my_debug_led_3_off();
            my_debug_led_4_off();

            my_debug_led_1_on();
        }
        else if (CUR_CHARGE_PHASE_NORMAL_CHARGE == cur_charge_phase)
        {
            my_debug_led_1_off();
            my_debug_led_3_off();
            my_debug_led_4_off();

            my_debug_led_2_on();
        }
        else if (CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE == cur_charge_phase)
        {
            my_debug_led_1_off();
            my_debug_led_2_off();
            my_debug_led_4_off();

            my_debug_led_3_on();
        }
        else if (CUR_CHARGE_PHASE_FULLY_CHARGE == cur_charge_phase)
        {
            my_debug_led_1_off();
            my_debug_led_2_off();
            my_debug_led_3_off();

            my_debug_led_4_on();
        }

        // my_debug_led_1_on();
        // my_debug_led_2_on();
        // my_debug_led_3_on();
        // my_debug_led_4_on();

#if 0 // 缓慢调节驱动灯光的pwm占空比（还未调试完成）

        {
            // static u8 cnt =0;

            // 暂定每100us调节一次

            if (cur_light_pwm_duty_val > expect_light_pwm_duty_val)
            {
                cur_light_pwm_duty_val--;
            }
            else if (cur_light_pwm_duty_val < expect_light_pwm_duty_val)
            {
                cur_light_pwm_duty_val++;
            }

            SET_LIGHT_PWM_DUTY(cur_light_pwm_duty_val);
            // timer2_set_pwm_duty(cur_light_pwm_duty_val);
        }

#endif // 缓慢调节驱动灯光的pwm占空比（还未调试完成）

#endif
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/
