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

volatile u8 cur_light_pwm_duty_val = 0;    // 当前灯光对应的占空比值
volatile u8 expect_light_pwm_duty_val = 0; // 期望调节到的、灯光对应的占空比值

volatile u16 bat_adc_val;

volatile u8 flag_is_light_adjust_time_come = 0; // 调节灯光的时间到来，目前为1s

volatile u16 light_adjust_time_cnt = 0;

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
    timer0_config();
    timer1_pwm_config();
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

    printf("sys reset\n");

    // adc_sel_pin(ADC_PIN_DETECT_CHARGE); // 测试通过，可以正常检测对应的电压值
    // adc_sel_pin(ADC_PIN_DETECT_BATTERY); // 测试通过，可以正常检测对应的电压值

    // PWM_CTL_FOR_CHARGING_SET_HIGH_FEQ();
    // 用下面的占空比充电，电流会非常大：
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 10 / 100) >> 8) & 0xFF; // 10%
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 10 / 100) & 0xFF;

    // 用这一栏充电（充电电压5.5~32V），电源一侧显示不出电流
    // PWM_CTL_FOR_CHARGING_SET_HIGH_FEQ();
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 1 / 100) >> 8) & 0xFF; // 1%，实际测试有时是0.88%，有时是1.3%
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 1 / 100) & 0xFF;
    // TMR1_PWMH = 0 & 0xFF;
    // TMR1_PWML = 1 & 0xFF;

    // 充电电压到 7-8V时，电流就会到1A，电压越高，电流越大
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 5 / 100) >> 8) & 0xFF; // 5%
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 5 / 100) & 0xFF;

    // 充电电压到11.3V时，电流会到1A，电压越高，电流越大
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 2 / 100) >> 8) & 0xFF; // 2%
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 2 / 100) & 0xFF;

    // 5V，电流会到10A
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 80 / 100) >> 8) & 0xFF; //
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 80 / 100) & 0xFF;

    // 在5V附近，电流5A左右
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 60 / 100) >> 8) & 0xFF; //
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 60 / 100) & 0xFF;

    // {
    //     u32 tmp = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * 90 / 100;

    //     TMR1_PWMH = (tmp >> 8) & 0xFF;
    //     TMR1_PWML = tmp & 0xFF;
    // }

    // LED_1_ON();
    // LED_2_ON();
    // LED_3_ON();
    // LED_4_ON();
    // LED_5_ON();

    // TODO:
    // 上电后，需要先点亮红色指示灯，再变为电池电量指示模式
    LED_1_ON();
    delay_ms(1000);

    // cur_led_mode = CUR_LED_MODE_INITIAL_DISCHARGE_GEAR;
    cur_led_mode = CUR_LED_MODE_BAT_INDICATOR; // 电池电量指示模式
    cur_initial_discharge_gear = 5;
    cur_discharge_rate = 3;

    while (1)
    {
        adc_sel_pin(ADC_PIN_DETECT_BATTERY);
        bat_adc_val = adc_getval(); // 采集电池电压对应的ad值

        ir_handle();
        // charge_handle();
        led_handle_update_percent_of_bat();

#if 1 // 缓慢调节驱动灯光的pwm占空比

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

#endif // 缓慢调节驱动灯光的pwm占空比
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/
