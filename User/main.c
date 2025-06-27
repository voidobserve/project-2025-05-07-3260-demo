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


enum
{
    CUR_LED_STATUS_NONE = 0x00,
    CUR_LED_SHOW_BAT_PERCENT, // ��ʾ��ص����ٷֱ�
    CUR_LED_SHOW_DISCHARGE_GEAR, // ��ʾ�ŵ絲λ 1~5��
    // CUR_LED_SHOW_
}
volatile cur_led_status =0 ;

//
void led_pin_config(void)
{
    P1_MD0 &= ~GPIO_P11_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P11_MODE_SEL(0x01);
    FOUT_S11 = GPIO_FOUT_AF_FUNC;

    P1_MD0 &= ~GPIO_P12_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P12_MODE_SEL(0x01);
    FOUT_S12 = GPIO_FOUT_AF_FUNC;

    P1_MD0 &= ~GPIO_P13_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P13_MODE_SEL(0x01);
    FOUT_S13 = GPIO_FOUT_AF_FUNC;

    P1_MD1 &= ~GPIO_P14_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P14_MODE_SEL(0x01);
    FOUT_S14 = GPIO_FOUT_AF_FUNC;

    P1_MD1 &= ~GPIO_P15_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P15_MODE_SEL(0x01);
    FOUT_S15 = GPIO_FOUT_AF_FUNC;
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
    timer1_pwm_config();
    timer1_pwm_disable();

    // timer1_set_pwm_high_feq();
    // TODO: 7361���ü������������:
    led_pin_config();


    adc_config();

    printf("sys reset\n");

    // adc_sel_pin(ADC_PIN_DETECT_CHARGE); // ����ͨ����������������Ӧ�ĵ�ѹֵ
    // adc_sel_pin(ADC_PIN_DETECT_BATTERY); // ����ͨ����������������Ӧ�ĵ�ѹֵ

    // PWM_CTL_FOR_CHARGING_SET_HIGH_FEQ();
    // �������ռ�ձȳ�磬������ǳ���
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 10 / 100) >> 8) & 0xFF; // 10%
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 10 / 100) & 0xFF;

    // ����һ����磨����ѹ5.5~32V������Դһ����ʾ��������
    // PWM_CTL_FOR_CHARGING_SET_HIGH_FEQ();
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 1 / 100) >> 8) & 0xFF; // 1%��ʵ�ʲ�����ʱ��0.88%����ʱ��1.3%
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 1 / 100) & 0xFF;
    // TMR1_PWMH = 0 & 0xFF;
    // TMR1_PWML = 1 & 0xFF;

    // ����ѹ�� 7-8Vʱ�������ͻᵽ1A����ѹԽ�ߣ�����Խ��
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 5 / 100) >> 8) & 0xFF; // 5%
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 5 / 100) & 0xFF;

    // ����ѹ��11.3Vʱ�������ᵽ1A����ѹԽ�ߣ�����Խ��
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 2 / 100) >> 8) & 0xFF; // 2%
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 2 / 100) & 0xFF;

    // 5V�������ᵽ10A
    // TMR1_PWMH = ((TIMER1_HIGH_FEQ_PEROID_VAL * 80 / 100) >> 8) & 0xFF; //
    // TMR1_PWML = (TIMER1_HIGH_FEQ_PEROID_VAL * 80 / 100) & 0xFF;

    // ��5V����������5A����
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

    while (1)
    {
        // charge_handle();
        led_handle_update_percent_of_bat();
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/
