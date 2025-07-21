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

// =================================================================
// ���������ر���                                                //
// =================================================================
volatile u8 ir_data = 0;
volatile bit flag_is_recv_ir_repeat_code = 0;
volatile bit flag_is_recved_data = 0;

// =================================================================
// ��������ر���                                                 //
// =================================================================
volatile u16 bat_adc_val;                                           // ��ص�ѹ���Ųɼ�����adֵ
volatile u16 charging_adc_val;                                      // ����ѹ���Ųɼ���adֵ
volatile u16 current_adc_val;                                       // ���������Ųɼ���adֵ
volatile u8 flag_is_charging_adjust_time_come = 0;                  // ���ڳ���ʱ�䵽��
volatile u8 cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE; // ���Ƴ���PWM״̬
volatile u8 cur_charge_phase = CUR_CHARGE_PHASE_NONE;               // ��¼��ǰ���׶�

// =================================================================
// ָʾ�ƿ�����ر���                                               //
// =================================================================
volatile u8 cur_initial_discharge_gear; // ��ʼ�ŵ絲λ����Ҫ���䣩
volatile u8 cur_discharge_rate;         // ��ʼ�ŵ����ʣ���Ҫ���䣩
volatile u8 cur_led_mode;               // ��ǰ��LEDģʽ
// volatile u8 last_led_mode; // �ϴε�ledģʽ
volatile u8 cur_led_gear;             // ��ǰled��λ
volatile u8 last_led_gear;            // �ϴ�led��λ��ֻ���ڸ��ϵ�ʱ���㸳��ʼֵ��
volatile u8 cur_led_gear_in_charging; // ���ָʾ����Ӧ�ĵ�λ

volatile bit flag_is_in_setting_mode = 0;              // �Ƿ�������ģʽ
volatile u8 flag_led_setting_mode_exit_times_come = 0; // ��־λ��led�˳�����ģʽ��ʱ�䵽��
volatile u16 led_setting_mode_exit_times_cnt = 0;      // �����LEDģʽ���˳�ʱ�����

volatile bit flag_is_in_struction_mode = 0;               // �Ƿ���ָʾģʽ
volatile bit flag_led_struction_mode_exit_times_come = 0; // �˳�ָʾ��ָʾģʽ��ʱ�䵽��
volatile u16 led_struction_mode_exit_times_cnt = 0;       // �˳�ָʾ��ָʾģʽʱ�����

// ��־λ���Ƿ�Ҫ�ص� led_off ģʽ
volatile bit flag_is_led_off_enable = 0;

// =================================================================
// ���ƹ������ر���                                               //
// =================================================================
volatile u32 light_adjust_time_cnt = 0;    // ���ڵƹ��ʱ��������ݶ�Ϊÿ1s��һ
volatile u8 light_ctl_phase_in_rate_1 = 1; // �ڷŵ�����M1ʱ��ʹ�õ��ı������ڼ��㹫ʽ��������ϵ����ÿ�λ���ʱ��Ҫ��ʼ��Ϊ1

// TODO��3260ʹ��16λ�Ĵ�����7361ʹ��8λ�Ĵ�����Ҫ���������޸�
volatile u16 cur_light_pwm_duty_val = 0; // ��ǰ�ƹ��Ӧ��ռ�ձ�ֵ
// volatile u16 expect_light_pwm_duty_val = 0;                  // �������ڵ��ġ��ƹ��Ӧ��ռ�ձ�ֵ
volatile u8 flag_is_light_adjust_time_come = 0;              // ���ڵƹ��ʱ�䵽����ĿǰΪ1s
volatile u8 flag_is_light_pwm_duty_val_adjust_time_come = 0; // �ƹ�ռ�ձ�ֵ����ʱ�䵽��

volatile u8 flag_is_ctl_light_blink = 0; // �Ƿ�������ƹ���˸
volatile u8 light_ctl_blink_times = 0;   // Ҫ�������ƹ���˸�Ĵ���
/*
    �Ƿ�Ҫ������ģʽ�ڼ�ر����ƹ�

    ����Ѿ��صƣ�������ģʽ�ڼ䣬������˸��ɺ�ֱ�ӹص�
*/
volatile bit flag_allow_light_in_setting_mode = 0;

// �Ƿ�Ҫ�����������ƹ��ռ�ձ�
volatile bit flag_is_adjust_light_slowly = 0;
volatile u16 expect_light_pwm_duty_val = 0; // �����������ڵ��ġ����ƹ��Ӧ��ռ�ձ�ֵ

// �Ƿ����˶�ʱ�ػ����ܣ�
volatile bit flag_is_auto_shutdown_enable = 0;
volatile u32 light_auto_shutdown_time_cnt = 0;     // ��ʱ�ػ����ܵĶ�ʱ����������λ��ms
volatile bit flag_is_auto_shutdown_times_come = 0; // ��ʱ�ػ���ʱ�䵽��

// �̰���С�ƹ����ȣ���Ӧ������λ���ȵ�ռ�ձ�ֵ
const u16 light_pwm_sub_table[9] = {
    (u16)((u32)TIMER2_FEQ * 8367 / 10000), // 83.67 %
    (u16)((u32)TIMER2_FEQ * 7371 / 10000), // 73.71 %
    (u16)((u32)TIMER2_FEQ * 6375 / 10000), // 63.75 %
    (u16)((u32)TIMER2_FEQ * 5379 / 10000), // 53.79 %
    (u16)((u32)TIMER2_FEQ * 4383 / 10000), // 43.83 %
    (u16)((u32)TIMER2_FEQ * 3387 / 10000), // 33.87 %
    (u16)((u32)TIMER2_FEQ * 2391 / 10000), // 23.91 %
    (u16)((u32)TIMER2_FEQ * 1395 / 10000), // 13.95 %
    (u16)((u32)TIMER2_FEQ * 478 / 10000),  // 4.78 %
};

// �̰����ӵƹ����ȣ���Ӧ������λ���ȵ�ռ�ձ�ֵ
const u16 light_pwm_add_table[9] = {
    (u16)((u32)TIMER2_FEQ * 478 / 10000),  // 4.78 %
    (u16)((u32)TIMER2_FEQ * 1474 / 10000), // 14.74 %
    (u16)((u32)TIMER2_FEQ * 2470 / 10000), // 24.70 %
    (u16)((u32)TIMER2_FEQ * 3466 / 10000), // 34.66 %
    (u16)((u32)TIMER2_FEQ * 4462 / 10000), // 44.62 %
    (u16)((u32)TIMER2_FEQ * 5458 / 10000), // 54.58 %
    (u16)((u32)TIMER2_FEQ * 6554 / 10000), // 65.54 %
    (u16)((u32)TIMER2_FEQ * 7450 / 10000), // 74.50 %
    (u16)((u32)TIMER2_FEQ * 8367 / 10000), // 83.67 %
};

const u16 light_pwm_duty_init_val_table[5] = {
    (u16)((u32)TIMER2_FEQ * 8367 / 10000), // 83.67 %
    (u16)((u32)TIMER2_FEQ * 7411 / 10000), // 74.11 %
    (u16)((u32)TIMER2_FEQ * 6455 / 10000), // 64.55 %
    (u16)((u32)TIMER2_FEQ * 5698 / 10000), // 56.98 %
    (u16)((u32)TIMER2_FEQ * 4980 / 10000), // 49.80 %
};

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

/*
    ��������������ʼ��

    ����ǵ�һ���ϵ磬��Ҫ������ŵ�����
*/
void param_init(void)
{
    light_ctl_phase_in_rate_1 = 1;

    cur_initial_discharge_gear = 5; // ��ʼ�ŵ絲λ����Ҫ���䣩
    cur_discharge_rate = 2;         // ��ʼ�ŵ����ʣ���Ҫ���䣩
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

    // cur_led_mode = CUR_LED_MODE_CHARGING;

    // bat_adc_val = 2000;
    // led_status_refresh();
    // cur_led_mode = CUR_LED_MODE_BAT_INDICATOR;

    param_init();

    light_init();
    led_init();
    led_mode_alter(CUR_LED_MODE_BAT_INDICATOR); // ��ص���ָʾģʽ

    // light_ctl_blink_times = 3;
    // flag_is_ctl_light_blink = 1;

    while (1)
    {

#if 1

        charge_handle();
        ir_handle(); // �����ڲ����ж��Ƿ��ڳ�磬����ڳ�����˳�

        /*
            ���ǳ��ģʽ�� -> �����ģʽ��

            �����ǰ���ڳ�磬����ָʾ��û���л������ָʾģʽ�����л���
        */
        if (CUR_CHARGE_PHASE_NONE != cur_charge_phase)
        {
            // if (cur_led_mode != CUR_LED_MODE_CHARGING && /* ָʾ�Ʋ����ڳ��ģʽ */
            //     cur_led_mode != CUR_LED_MODE_OFF)
            if (cur_led_mode != CUR_LED_MODE_CHARGING) /* ָʾ�Ʋ����ڳ��ģʽ */
            {
                // ��ն�ʱ�ػ���صı���
                flag_is_auto_shutdown_enable = 0;
                led_status_clear();
                led_mode_alter(CUR_LED_MODE_CHARGING);
            }

            // ��Ҫ�ر����ƹ�
            LIGHT_OFF();
        } // if (CUR_CHARGE_PHASE_NONE != cur_charge_phase)
        else // CUR_CHARGE_PHASE_NONE == cur_charge_phase
        {
            /*
                �����ģʽ�� -> ���ŵ硢�������ƹ⡢ָʾ�ƶ�Ӧ��ص���ָʾ��
                �����ǰû���ڳ�磬����ָʾ�ƴ��ڳ��ָʾģʽ��
                �л��ص�ص���ָʾģʽ

                ����ʱ���ִӳ�絽�Ͽ���磬ledָʾ�ƻ�����˸����Ҫ�����ⲹ��
            */
            if (cur_led_mode == CUR_LED_MODE_CHARGING)
            {
                led_status_clear();
                led_mode_alter(CUR_LED_MODE_BAT_INDICATOR);
                // ��Ҫ�����ƹ�

                // �����õ�λ��Ӧ��ռ�ձ�ֵ
                cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];

                LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // ���̸���PWMռ�ձ�
                LIGHT_ON();                                 // ʹ�� PWM ���
            }
        }

        // �����ʱ�ػ���ʱ�䵽��
        if (flag_is_auto_shutdown_times_come)
        {
            flag_is_auto_shutdown_times_come = 0; // ��ն�ʱ�ػ���־
            flag_is_auto_shutdown_enable = 0;     // �������Զ��ػ�
            led_status_clear();
            cur_led_mode = CUR_LED_MODE_OFF;
            cur_light_pwm_duty_val = 0;
            LIGHT_OFF();

            // printf("power off\n");
        }

        adc_update_bat_adc_val();
        led_handle();
        light_handle();

#if 0  // ÿ��һ��ʱ�䣬��ӡ������Ϣ��

        {
            static u8 cnt = 0;
            cnt++;
            if (cnt >= 200)
            {
                cnt = 0;
                // printf("bat_adc_val %u\n", bat_adc_val);
                // printf("cur_light_pwm_duty_val %u\n", cur_light_pwm_duty_val);
                // printf("cur light pwm percent %bu %%\n", (u8)((u32)cur_light_pwm_duty_val * 100 / TIMER2_FEQ));

                switch (cur_led_mode)
                {
                case CUR_LED_MODE_OFF:
                    printf("led mode off\n");
                    break;

                case CUR_LED_MODE_BAT_INDICATOR:
                    printf("led mode bat indicator\n");
                    break;

                case CUR_LED_MODE_CHARGING:
                    printf("led mode charging\n");
                    break;

                case CUR_LED_MODE_SETTING:
                    printf("led mode setting\n");
                    break;

                case CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE:
                    printf("led mode initial discharge gear in setting mode\n");
                    break;

                case CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE:
                    printf("led mode discharge rate in setting mode\n");
                    break;

                case CUR_LED_MODE_BAT_INDICATIOR_IN_INSTRUCTION_MODE:
                    printf("led mode bat indicator in instruction mode\n");
                    break;

                case CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_INSTRUCTION_MODE:
                    printf("led mode initial discharge gear in instruction mode\n");
                    break;

                case CUR_LED_MODE_DISCHARGE_RATE_IN_INSTRUCTION_MODE:
                    printf("led mode discharge rate in instruction mode\n");
                    break;

                default:
                    break;
                }
            }
        }
#endif // ÿ��һ��ʱ�䣬��ӡ������Ϣ

#if 1 // demo��ʹ�õ��ĵ���ָʾ��
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
#endif // demo��ʹ�õ��ĵ���ָʾ��

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
