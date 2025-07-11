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
 * ��Ȩ˵����������
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include <stdio.h>

#include "user_config.h"

volatile bit_flag flag1 = {0};
volatile bit_flag flag2 = {0};

volatile u8 cur_light_pwm_duty_val = 0;    // ��ǰ�ƹ��Ӧ��ռ�ձ�ֵ
volatile u8 expect_light_pwm_duty_val = 0; // �������ڵ��ġ��ƹ��Ӧ��ռ�ձ�ֵ

volatile u16 bat_adc_val;
volatile u16 charging_adc_val; // ��⵽����ѹ��adֵ
volatile u16 current_adc_val;  // ��⵽��������Ӧ�ĵ�ѹֵ

volatile u8 flag_is_light_adjust_time_come = 0; // ���ڵƹ��ʱ�䵽����ĿǰΪ1s

volatile u16 light_adjust_time_cnt = 0;

volatile u8 flag_is_charging_adjust_time_come = 0; // ���ڳ���ʱ�䵽��

//
void led_pin_config(void)
{
    P1_MD0 &= ~GPIO_P11_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P11_MODE_SEL(0x01);
    FOUT_S11 = GPIO_FOUT_AF_FUNC;
    P11 = 0; // ���������ʼֵ���ϵ�֮��ָʾ�ƻ���һ��

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

// // ����ֵ����
// float my_fabs(float x) {
//     return (x < 0) ? -x : x;
// }

// ̩��չ������ ln(x)��������Χ��x > 0.1
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

// ̩��չ������ exp(x)
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

// �ݺ�����x^y = e^(y * ln(x))
float my_pow(float base, float exponent)
{
    if (base <= 0.0f)
    {
        return 0.0f; // ��֧�ָ�������0�ĸ�����
    }
    if (exponent == 0.0f)
        return 1.0f;
    if (base == 1.0f)
        return 1.0f;

    return my_exp(exponent * my_ln(base));
}

// ����ֵ��ռ�ձȣ���λ���ٷ�֮һ������ 75 ��ʾ 75%��
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
        return 0; // ��ֹ�����������

    voltage_mV /= 1000;

    result = A / my_pow(voltage_mV, n);

    if (result < 0.0f)
        result = 0.0f;
    if (result > 100.0f)
        result = 100.0f;

    return (u8)(result + 0.5f); // ��������
}

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
void main(void)
{
    // ���Ź�Ĭ�ϴ�, ��λʱ��2s
    WDT_KEY = WDT_KEY_VAL(0xDD); //  �رտ��Ź� (�������ÿ��Ź���鿴��WDT\WDT_Reset��ʾ��)

    system_init();

    // �ر�HCK��HDA�ĵ��Թ���
    WDT_KEY = 0x55;  // ���д����
    IO_MAP &= ~0x01; // �������Ĵ�����ֵ��ʵ�ֹر�HCK��HDA���ŵĵ��Թ��ܣ����ӳ�䣩
    WDT_KEY = 0xBB;  // дһ����Ч�����ݣ�����д����

    uart0_config();
    timer0_config();
    timer1_pwm_config(); // ���Ƴ���PWM
    timer1_pwm_disable();
    timer2_pwm_config(); // ���Ƶƹ��pwm
    timer2_pwm_disable();

    // timer1_set_pwm_high_feq();
    // TODO: 7361���ü������������:
    led_pin_config();

    // ����������ţ�
    P2_MD0 &= ~(GPIO_P23_MODE_SEL(0x03)); // ����ģʽ
    P2_PU |= GPIO_P23_PULL_UP(0x01);      // ����

    adc_config();

    printf("sys reset\n"); 

// TODO:
// �ϵ����Ҫ�ȵ�����ɫָʾ�ƣ��ٱ�Ϊ��ص���ָʾģʽ
// LED_1_ON();
// delay_ms(1000);

// cur_led_mode = CUR_LED_MODE_INITIAL_DISCHARGE_GEAR;
#if 0
    cur_led_mode = CUR_LED_MODE_BAT_INDICATOR; // ��ص���ָʾģʽ
    cur_initial_discharge_gear = 5;
    cur_discharge_rate = 3;
#endif

    timer1_set_pwm_high_feq();  

    while (1)
    {
        adc_sel_pin(ADC_PIN_DETECT_BATTERY);
        bat_adc_val = adc_getval(); // �ɼ���ص�ѹ��Ӧ��adֵ
        // adc_sel_pin(ADC_PIN_DETECT_CHARGE);
        // adc_sel_pin_charge(CUR_ADC_REF_3_0_VOL);
        // charging_adc_val = adc_getval();
        adc_sel_pin(ADC_PIN_DETECT_CURRENT);
        current_adc_val = adc_getval();

        // printf("current_adc_val %u\n", current_adc_val);


#if 1

        if (flag_is_charging_adjust_time_come) // һ��Ҫ���뻺�����ڣ�����Ѹ�ٵ��ڣ������Ȳ�����ѹ�ȶ�
        {
            u16 current = 0; // ����������λ��mA
            // u16 voltage_of_charging = 0; // ����ѹ����λ��mV
            u16 voltage_of_bat = 0; // ��ص�ѹ
            u32 power = 0;          // ���ʣ���λ��mW ����
            static u8 pwm_duty = 0; //
            u16 pwm_reg = 0;

            flag_is_charging_adjust_time_come = 0;
 
            /*
                ���������ż�⵽�ĵ�ѹ == adֵ / 4096 * �ο���ѹ
                current_adc_val * 3 / 4096

                �����������ż�⵽�ĳ����� == ���������ż�⵽�ĵ�ѹ / 110(�˷ŷŴ���) / 0.005R��
                �𲽻��ɵ�Ƭ�����Լ������ʽ��
                current_adc_val * 3 / 4096 / 110 / 0.005
                current_adc_val * 3 / 4096 / 110 * 1000 / 5
                current_adc_val * 3 * 1000 / 5 / 4096 / 110
                �õ�������AΪ��λ�ĵ�������Ҫ��ת������mAΪ��λ��
                current_adc_val * 3 * 1000 * 1000 / 5 / 4096 / 110��������������Ҫ�ٻ���
                (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110
                current =  (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110;
            */ 
            current = (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 76; // 

            /*
                �������ѹ
            */
            // voltage_of_charging = (u32)charging_adc_val * 3 * 1000 * 11 / 4096;
            // �����ص�ѹ
            voltage_of_bat = (u32)bat_adc_val * 2 * 1000 * 2 / 4096; 

            // �����⵽������adֵ�Ѿ�����
            if (current_adc_val >= 4095)
            // if (current >= 5400) // �������ֵ�Ѿ�����������Ƭ���ܼ���ֵ��5454.54
            {
                // printf("current overflow\n");
                if (pwm_duty > 0)
                {
                    pwm_duty--;
                }
            }
            // // else if (charging_adc_val > 3723) // ����ѹ����30V���ر�PWM���
            // else if (bat_adc_val > 3686) // ��ص�ѹ����3.6V���ر�PWM���
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
            pwm_reg = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * pwm_duty / 100; // ���յ�ռ�ձ�ֵ

            //     // printf("pwm_duty :%u\n", pwm_duty);
            TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
            TMR1_PWML = pwm_reg & 0xFF;
        }
#endif

        // {
        //     u32 charging_voltage_mV = 0;
        //     u16 pwm_duty = 0;

        //     // charging_adc_val = 1117; // ������
        //     charging_voltage_mV = (u32)charging_adc_val * 2 * 11 * 1000 / 4096;
        //     // charging_voltage_mV = 6000; // ������
        //     pwm_duty = calculate_duty(charging_voltage_mV);
        //     pwm_duty = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * pwm_duty / 100; // ���յ�ռ�ձ�ֵ

        //     // printf("pwm_duty :%u\n", pwm_duty);
        //     TMR1_PWMH = (pwm_duty >> 8) & 0xFF;
        //     TMR1_PWML = pwm_duty & 0xFF;
        // }

#if 0
        {
            u32 tmp = 0;

            // tmp = (u32)124286 - (u32)charging_adc_val * 7857 * 2 * 11 / 4096;
            tmp = (u32)88101 - (u32)charging_adc_val * 2471 * 2 * 11 / 4096;

            // tmp = (((u32)TMR1_PRH << 8) | (u32)TMR1_PRL) * (u32)tmp / 1000; // ���յ�ռ�ձ�ֵ����������㣬�ó���ֵ��0��
            tmp = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * tmp / 1000 / 100; // ���յ�ռ�ձ�ֵ

            /*
                ���ڹ�ʽ���ƣ����ܳ���ѹ����15V�ͻᵼ��ռ�ձȱ�Ϊ0�����ڲ�������0%
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

#if 0 // �������������ƹ��pwmռ�ձ�

        {
            // static u8 cnt =0;

            // �ݶ�ÿ100us����һ��

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

#endif // �������������ƹ��pwmռ�ձ�
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/
