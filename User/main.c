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
volatile u16 current_adc_val;  // 检测到充电电流对应的电压值

volatile u8 flag_is_light_adjust_time_come = 0; // 调节灯光的时间到来，目前为1s

volatile u16 light_adjust_time_cnt = 0;

volatile u8 flag_is_charging_adjust_time_come = 0; // 调节充电的时间到来

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

    while (1)
    {
        adc_sel_pin(ADC_PIN_DETECT_BATTERY);
        bat_adc_val = adc_getval(); // 采集电池电压对应的ad值
        // adc_sel_pin(ADC_PIN_DETECT_CHARGE);
        // adc_sel_pin_charge(CUR_ADC_REF_3_0_VOL);
        // charging_adc_val = adc_getval();
        adc_sel_pin(ADC_PIN_DETECT_CURRENT);
        current_adc_val = adc_getval();

        // printf("current_adc_val %u\n", current_adc_val);


#if 1

        if (flag_is_charging_adjust_time_come) // 一定要加入缓慢调节，不能迅速调节，否则会等不到电压稳定
        {
            u16 current = 0; // 充电电流，单位：mA
            // u16 voltage_of_charging = 0; // 充电电压，单位：mV
            u16 voltage_of_bat = 0; // 电池电压
            u32 power = 0;          // 功率，单位：mW 毫瓦
            static u8 pwm_duty = 0; //
            u16 pwm_reg = 0;

            flag_is_charging_adjust_time_come = 0;
 
            /*
                检测电流引脚检测到的电压 == ad值 / 4096 * 参考电压
                current_adc_val * 3 / 4096

                检测电流的引脚检测到的充电电流 == 检测电流引脚检测到的电压 / 110(运放放大倍数) / 0.005R，
                逐步换成单片机可以计算的形式：
                current_adc_val * 3 / 4096 / 110 / 0.005
                current_adc_val * 3 / 4096 / 110 * 1000 / 5
                current_adc_val * 3 * 1000 / 5 / 4096 / 110
                得到的是以A为单位的电流，需要再转换成以mA为单位：
                current_adc_val * 3 * 1000 * 1000 / 5 / 4096 / 110，计算会溢出，需要再化简
                (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110
                current =  (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110;
            */ 
            current = (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 76; // 

            /*
                计算充电电压
            */
            // voltage_of_charging = (u32)charging_adc_val * 3 * 1000 * 11 / 4096;
            // 计算电池电压
            voltage_of_bat = (u32)bat_adc_val * 2 * 1000 * 2 / 4096; 

            // 如果检测到电流的ad值已经爆表
            if (current_adc_val >= 4095)
            // if (current >= 5400) // 如果电流值已经爆表，超过单片机能检测的值：5454.54
            {
                // printf("current overflow\n");
                if (pwm_duty > 0)
                {
                    pwm_duty--;
                }
            }
            // // else if (charging_adc_val > 3723) // 充电电压超过30V，关闭PWM输出
            // else if (bat_adc_val > 3686) // 电池电压超过3.6V，关闭PWM输出
            // {
            //     // printf("voltage of charging overflow\n");
            //     pwm_duty = 0;
            // }
            else
            {
                // power = voltage_of_charging * current / 1000; // 1000mV * 1000mA == 1000000 (1 Watt)
                power = voltage_of_bat * current / 1000; // 1000mV * 1000mA == 1000000 (1 Watt)

                // printf("power %lu \n", power);
                // if (power < 30000) // 30 * 1000 mW
                if (power < 25000) // xx * 1000 mW
                // if (power < 20000) // xx * 1000 mW
                {
                    if (pwm_duty < 100)
                    {
                        pwm_duty++;
                    }
                }
                // else if (power > 30000)
                else if (power > 25000)
                // else if (power > 20000)
                {
                    if (pwm_duty > 0)
                    {
                        pwm_duty--;
                    }
                }
            }

            // printf("pwm_duty : %bu %%\n", pwm_duty);
            pwm_reg = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * pwm_duty / 100; // 最终的占空比值

            //     // printf("pwm_duty :%u\n", pwm_duty);
            TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
            TMR1_PWML = pwm_reg & 0xFF;
        }
#endif

        // {
        //     u32 charging_voltage_mV = 0;
        //     u16 pwm_duty = 0;

        //     // charging_adc_val = 1117; // 测试用
        //     charging_voltage_mV = (u32)charging_adc_val * 2 * 11 * 1000 / 4096;
        //     // charging_voltage_mV = 6000; // 测试用
        //     pwm_duty = calculate_duty(charging_voltage_mV);
        //     pwm_duty = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * pwm_duty / 100; // 最终的占空比值

        //     // printf("pwm_duty :%u\n", pwm_duty);
        //     TMR1_PWMH = (pwm_duty >> 8) & 0xFF;
        //     TMR1_PWML = pwm_duty & 0xFF;
        // }

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
