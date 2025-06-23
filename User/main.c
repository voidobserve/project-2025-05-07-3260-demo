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

#if 0

void sys_init(void)
{
    GIE = 0;
    CLR_RAM();
    IO_Init();
    timer0_pwm_config();
    timer1_config();
    // timer2_pwm_config(); // ���� ���� �ƹ� ��pwm

    adc_config();

    INTE |= (0x01 << 1); // ʹ�� timer1�ж�

    delay_ms(1); // �ȴ�ϵͳ�ȶ�
    GIE = 1;
}

// ������
void charge_handle(void)
{
    // static volatile u8 cur_charge_status = CUR_CHARGE_STATUS_NONE;
    u16 charging_adc_val;
    u16 bat_adc_val;

    adc_sel_pin(ADC_PIN_DETECT_CHARGE);
    charging_adc_val = adc_getval(); // �ɼ���������Ӧ��adֵ
    adc_sel_pin(ADC_PIN_DETECT_BATTERY);
    bat_adc_val = adc_getval(); // �ɼ���ص�ѹ��Ӧ��adֵ

    // �����ǰδ�ڳ��
    if (CUR_CHARGE_STATUS_NONE == cur_charge_status)
    {
        if (charging_adc_val >= ADC_VAL_ENABLE_IN_CHARGE_END)
        {
            // �����δ���ʱ����⵽��������ѹ��ʹ�ܳ��
            cur_charge_status = CUR_CHARGE_STATUS_IN_CHARGING;
        }
    }
    else // �����ǰ���ڳ��
    {
        static u8 cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;

        if (charging_adc_val <= ADC_VAL_DISABLE_IN_CHARGE_END ||
            charging_adc_val >= 2420 || /* ��������ѹ���� 13V ���Ͽ���� ����ʽ����ȱ��*/
            bat_adc_val >= ADC_VAL_BAT_IS_FULL)
        {
            // �������ĵ�ѹС��һ��ֵ���Ͽ����
            // ��� �Ѿ����磬 �Ͽ����
            timer0_pwm_disable();
            cur_charge_status = CUR_CHARGE_STATUS_NONE;
            cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;
            return;
        }

        // ��ص�ѹС��һ��ֵ���Ǵ���һ��ֵ������������
        if (bat_adc_val <= ADC_VAL_BAT_IS_LOW ||
            bat_adc_val >= ADC_VAL_BAT_IS_NEAR_FULL)
        {
            if (CUR_CHARGING_PWM_STATUS_LOW_FEQ != cur_charging_pwm_status)
            {
                timer0_pwm_set_low_feq(); // �����ڲ��趨�˹̶�Ƶ�ʺ͹̶�ռ�ձ�
                cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_LOW_FEQ;
            }
        }
        else // �����ص�ѹ������������ķ�Χ
        {
            u32 tmp;

            // �����ص�ѹ��������������䣬���������
            // ע�����ﲻ������ı� PWMռ�ձȣ��������PWMռ�ձ�Ϊ0
            if (CUR_CHARGING_PWM_STATUS_HIGH_FRQ != cur_charging_pwm_status)
            {
                timer0_pwm_set_high_feq();
                cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_HIGH_FRQ;
            }

            // ���ݳ���ѹ����pwmռ�ձ�

            // �������ʱ���ŵ��ڿ��Ƴ���PWMռ�ձ�
            if (CUR_CHARGING_PWM_STATUS_HIGH_FRQ == cur_charging_pwm_status)
            {
                /*
                       ������������ƵĹ��ܣ�
                       5V���룬pwm 105Khz��90%��������Ҫʹ�� 105Khz��85%����12V���룬pwm 105Khz��30%
                       �ã�5V��85%���ͣ�12V��30%���������������㣬
                       ��֪��������(5,85),(12,30)�����Ӧ�ķ��̣�
                       ��Ӧֱ�ߵķ��̣� y  =  ?7.857x  + 124.286
                       y ��Ӧռ�ձȣ���λ��%��x ��Ӧ��������ѹ����λ��V
                       �ڹ�ʽ��ȥ��С���㣺
                       1000y = -7857x + 124286

                       ��Ƭ��adcʹ���ڲ�2.0V�ο���ѹ��
                       ����ʽת���� ռ�ձ� �� adֵ �Ĺ�ϵ:
                       �ⲿ�������ĵ�ѹ 1/11 ��ѹ�󣬵���Ƭ������
                       �ⲿ�����ѹ / 11 / ��Ƭ��adc�ο���ѹ * 4096 == ��Ƭ���ɼ�����adֵ
                       �ⲿ�����ѹ == ��Ƭ���ɼ�����adֵ / 4096 * ��Ƭ��adc�ο���ѹ * 11
                       ����ɵ�Ƭ�����Լ������ʽ��
                       �ⲿ�����ѹ == ��Ƭ���ɼ�����adֵ * ��Ƭ��adc�ο���ѹ * 11 / 4096
                       x = adc_val * 2 * 11 / 4096

                       �õ��Ĺ�ʽ��
                       1000y = 124286 - 7857 (adc_val * 2 * 11 / 4096)
                       1000y = 124286 - adc_val * 7857 * 2 * 11 / 4096
                       �����ʽ���㲻�ᳬ�� 2^32 ��Χ
                   */
                tmp = (u32)124286 - (u32)charging_adc_val * 7857 * 2 * 11 / 4096;
                // �õ��� tmp ��1000����ռ�ձ�ֵ����Ҫ����1000�� �ٳ��� ��ʱ������ֵ/100,�õ���ʱ����Ӧ��ռ�ձ�ֵ
                T1DATA = (u32)T1LOAD * (u32)tmp / 1000 / 100; // ���յ�ռ�ձ�ֵ
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
    // ���Ź�Ĭ�ϴ�, ��λʱ��2s
    WDT_KEY = WDT_KEY_VAL(0xDD); //  �رտ��Ź� (�������ÿ��Ź���鿴��WDT\WDT_Reset��ʾ��)

    system_init();

    // �ر�HCK��HDA�ĵ��Թ���
    WDT_KEY = 0x55;  // ���д����
    IO_MAP &= ~0x01; // �������Ĵ�����ֵ��ʵ�ֹر�HCK��HDA���ŵĵ��Թ��ܣ����ӳ�䣩
    WDT_KEY = 0xBB;  // дһ����Ч�����ݣ�����д����

    uart0_config();
    // timer0_config();
    timer1_pwm_config();
    timer1_pwm_disable();

    // timer1_set_pwm_high_feq();

    adc_config();

    printf("sys reset\n");

    // adc_sel_pin(ADC_PIN_DETECT_CHARGE); // ����ͨ����������������Ӧ�ĵ�ѹֵ
    adc_sel_pin(ADC_PIN_DETECT_BATTERY); // ����ͨ����������������Ӧ�ĵ�ѹֵ

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
