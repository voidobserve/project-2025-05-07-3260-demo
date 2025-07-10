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
volatile u16 charging_adc_val; // 检测到充电电压的ad值

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

// // 绝对值函数
// float my_fabs(float x) {
//     return (x < 0) ? -x : x;
// }

// 泰勒展开近似 ln(x)，收敛范围：x > 0.1
float my_ln(float x)
{
    u8 i;
    float t = (x - 1) / (x + 1);
    float sum = 0.0f;
    float term = t;

    for (i = 0; i < 50; i++)
    {
        sum += term / (2 * i + 1);
        term *= t * t;
    }

    return 2 * sum;
}

// 泰勒展开近似 exp(x)
float my_exp(float x)
{
    u8 i;
    float sum = 1.0f;
    float term = 1.0f;

    for (i = 1; i < 20; i++)
    {
        term *= x / i;
        sum += term;
    }

    return sum;
}

// 幂函数：x^y = e^(y * ln(x))
float my_pow(float base, float exponent)
{
    if (base <= 0.0f)
    {
        return 0.0f; // 不支持负底数或0的负次幂
    }
    if (exponent == 0.0f)
        return 1.0f;
    if (base == 1.0f)
        return 1.0f;

    return my_exp(exponent * my_ln(base));
}

// 返回值：占空比，单位：百分之一（例如 75 表示 75%）
u8 calculate_duty(u32 voltage_mV)
{
    // y = 538 / (x ^ 1.178)
    // const float A = 538.0f;
    // const float n = 1.178f;

    // y = 680 / (x ^ 1.32)
    const float A = 680.0F;
    const float n = 1.32F;

    // y = k / x 
    // const float A = 376.26f;
    // const float A = 395.2f;
    // const float A = 348.32f;
    // const float A = 288.0F; 
    // const float n = 1.0f;
    float result;

    if (voltage_mV < 100)
        return 0; // 防止除零或过大输出

    voltage_mV /= 1000;

    result = A / my_pow(voltage_mV, n);

    if (result < 0.0f)
        result = 0.0f;
    if (result > 100.0f)
        result = 100.0f;

    return (u8)(result + 0.5f); // 四舍五入
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

    printf("sys reset\n");

// LED_1_ON();
// LED_2_ON();
// LED_3_ON();
// LED_4_ON();
// LED_5_ON();

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

    timer1_set_pwm_high_feq();
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 9 / 100) >> 8) & 0xFF; //
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 9 / 100) & 0xFF;

    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 75 / 100) >> 8) & 0xFF; //
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 75 / 100) & 0xFF;

    // {
    //     u8 tmp = 0;
    //     tmp = calculate_duty(5300);

    //     printf("tmp %bu\n", tmp);

    //     tmp = calculate_duty(11000);
    //     printf("tmp %bu\n", tmp);

    //     tmp = calculate_duty(32000);
    //     printf("tmp %bu\n", tmp);
    // }

    // {
    //     // u32 charging_voltage_mV = 0;
    //     // u16 pwm_duty = 0;
    //     // charging_voltage_mV = charging_adc_val * 2 * 11 * 1000 / 4096;
    //     // pwm_duty = calculate_duty(charging_voltage_mV);
    //     // pwm_duty = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * pwm_duty  / 100; // 最终的占空比值
    //     // TMR1_PWMH = (pwm_duty >> 8) & 0xFF;
    //     // TMR1_PWML = pwm_duty & 0xFF;

    //     u32 charging_voltage_mV = 0;
    //     u16 pwm_duty = 0;
    //     u8 i;

    //     for (i = 1; i <= 32; i++)
    //     {
    //         pwm_duty = calculate_duty((u32)i * 1000);
    //         printf("pwm_duty :%bu %%\n", (u8)pwm_duty);
    //         pwm_duty = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * pwm_duty / 100; // 最终的占空比值
    //         printf("reg val %u \n", (u16)pwm_duty);
    //     }
    // }

    while (1)
    {
        // adc_sel_pin(ADC_PIN_DETECT_BATTERY);
        // bat_adc_val = adc_getval(); // 采集电池电压对应的ad值
        adc_sel_pin(ADC_PIN_DETECT_CHARGE);
        charging_adc_val = adc_getval();

        {
            u32 charging_voltage_mV = 0;
            u16 pwm_duty = 0;

            // charging_adc_val = 1117; // 测试用
            charging_voltage_mV = (u32)charging_adc_val * 2 * 11 * 1000 / 4096;
            // charging_voltage_mV = 6000; // 测试用
            pwm_duty = calculate_duty(charging_voltage_mV);
            pwm_duty = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * pwm_duty / 100; // 最终的占空比值

            // printf("pwm_duty :%u\n", pwm_duty);
            TMR1_PWMH = (pwm_duty >> 8) & 0xFF;
            TMR1_PWML = pwm_duty & 0xFF;
        }

#if 0
        {
            u32 tmp = 0;

            // tmp = (u32)124286 - (u32)charging_adc_val * 7857 * 2 * 11 / 4096;
            tmp = (u32)88101 - (u32)charging_adc_val * 2471 * 2 * 11 / 4096;

            // tmp = (((u32)TMR1_PRH << 8) | (u32)TMR1_PRL) * (u32)tmp / 1000; // 最终的占空比值（用这个计算，得出的值是0）
            tmp = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * tmp / 1000 / 100; // 最终的占空比值

            /*
                由于公式限制，可能充电电压大于15V就会导致占空比变为0，现在不让它到0%
            */
            if (0 == (u16)tmp)
            // if (0 == tmp)
            {
                tmp = 0;
            }

            TMR1_PWMH = (tmp >> 8) & 0xFF;
            TMR1_PWML = tmp & 0xFF;
        }
#endif

        // ir_handle();
        // charge_handle();
        // led_handle_update_percent_of_bat();

#if 0 // 缓慢调节驱动灯光的pwm占空比

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
