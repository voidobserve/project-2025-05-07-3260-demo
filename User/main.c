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
    // adc_sel_pin(ADC_PIN_DETECT_BATTERY); // ����ͨ����������������Ӧ�ĵ�ѹֵ

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
