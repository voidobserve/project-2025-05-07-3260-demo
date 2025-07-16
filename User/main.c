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

// TODO��3260ʹ��16λ�Ĵ�����7361ʹ��8λ�Ĵ�����Ҫ���������޸� 
volatile u16 cur_light_pwm_duty_val = 0;    // ��ǰ�ƹ��Ӧ��ռ�ձ�ֵ 
volatile u16 expect_light_pwm_duty_val = 0; // �������ڵ��ġ��ƹ��Ӧ��ռ�ձ�ֵ 

volatile u8 flag_is_light_adjust_time_come = 0; // ���ڵƹ��ʱ�䵽����ĿǰΪ1s 

// =================================================================
// ��������ر���                                                 //
// =================================================================
volatile u16 bat_adc_val;                                           // ��ص�ѹ���Ųɼ�����adֵ
volatile u16 charging_adc_val;                                      // ����ѹ���Ųɼ���adֵ
volatile u16 current_adc_val;                                       // ���������Ųɼ���adֵ
volatile u8 flag_is_charging_adjust_time_come = 0;                  // ���ڳ���ʱ�䵽��
volatile u8 cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE; // ���Ƴ���PWM״̬
volatile u8 cur_charge_phase = CUR_CHARGE_PHASE_NONE;               // ��¼��ǰ���׶�

volatile u32 light_adjust_time_cnt = 0;    // ���ڵƹ��ʱ��������ݶ�Ϊÿ1s��һ
volatile u8 light_ctl_phase_in_rate_1 = 1; // �ڷŵ�����M1ʱ��ʹ�õ��ı������ڼ��㹫ʽ��������ϵ����ÿ�λ���ʱ��Ҫ��ʼ��Ϊ1
// volatile u8 flag_is_tim_turn_off_pwm = 0; // ��־λ�����������ڼ䣬��ʱ���Ƿ�ر���PWM���

// =================================================================
// ָʾ�ƿ�����ر���                                               //
// =================================================================
volatile u8 cur_initial_discharge_gear = 5; // ��ʼ�ŵ絲λ����Ҫ���䣩
volatile u8 cur_discharge_rate = 1;         // ��ʼ�ŵ����ʣ���Ҫ���䣩
volatile u8 cur_led_mode; // ��ǰ��LEDģʽ
volatile u8 cur_led_gear;  // ��ǰled��λ
volatile u8 last_led_gear; // �ϴ�led��λ
volatile u8 cur_led_gear_in_charging; // ���ָʾ����Ӧ�ĵ�λ

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

// ��������������ʼ��
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
    // ���Ź�Ĭ�ϴ�, ��λʱ��2s
    WDT_KEY = WDT_KEY_VAL(0xDD); //  �رտ��Ź� (�������ÿ��Ź���鿴��WDT\WDT_Reset��ʾ��)

    system_init();

    // �ر�HCK��HDA�ĵ��Թ���
    WDT_KEY = 0x55;  // ���д����
    IO_MAP &= ~0x01; // �������Ĵ�����ֵ��ʵ�ֹر�HCK��HDA���ŵĵ��Թ��ܣ����ӳ�䣩
    WDT_KEY = 0xBB;  // дһ����Ч�����ݣ�����д����

    uart0_config();
    my_debug_led_config();

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

    printf("sys reset\n"); // ��ӡ����ռ��1012�ֽڿռ�

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

    // timer1_set_pwm_high_feq();

    // {
    //     u16 pwm_reg = 0;                                     // ���Ҫд�뵽�Ĵ����е�ռ�ձ�ֵ
    //     pwm_reg = (u32)TIMER1_LOW_FEQ_PEROID_VAL * 13 / 100; // ���յ�ռ�ձ�ֵ
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
        // bat_adc_val = adc_getval(); // �ɼ���ص�ѹ��Ӧ��adֵ
        // adc_sel_pin(ADC_PIN_DETECT_CHARGE);
        // adc_sel_pin_charge(CUR_ADC_REF_3_0_VOL);
        // charging_adc_val = adc_getval();
        // adc_sel_pin(ADC_PIN_DETECT_CURRENT);
        // current_adc_val = adc_getval(); // �ɼ������صĵ�����Ӧ��adֵ

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

#if 0 // �������������ƹ��pwmռ�ձȣ���δ������ɣ�

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

#endif // �������������ƹ��pwmռ�ձȣ���δ������ɣ�

#endif
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/
