#include "led_handle.h"

volatile u8 cur_led_mode; // 当前的LED模式
// TODO:需要初始化为5
volatile u8 bat_remaining_power_indication = 5; // 电池剩余电量指示挡位
volatile u8 cur_initial_discharge_gear = 5;     // 初始放电挡位（需要记忆）
volatile u8 cur_discharge_rate = 1;             // 初始放电速率（需要记忆）

volatile u8 flag_is_update_led_mode_times_come = 0; // 标志位，LED模式的刷新时间到来

#if 0
/* 滑动平均 */
static volatile u16 bat_adc_val_samples[BAT_ADC_VAL_SAMPLE_COUNT];
static volatile u8 bat_adc_val_sample_index = 0;
u16 get_filtered_bat_adc_val(u16 bat_adc_val)
{
    u8 i = 0;
    u32 sum = 0;
    bat_adc_val_samples[bat_adc_val_sample_index++] = bat_adc_val;
    if (bat_adc_val_sample_index >= BAT_ADC_VAL_SAMPLE_COUNT)
        bat_adc_val_sample_index = 0;

    for (i = 0; i < BAT_ADC_VAL_SAMPLE_COUNT; i++)
        sum += bat_adc_val_samples[i];

    return sum / BAT_ADC_VAL_SAMPLE_COUNT;
}
#endif

// 更新LED指示灯
void led_handle_update_percent_of_bat(void)
{
    // bat_adc_val = 4095; // 测试用
    // bat_adc_val = get_filtered_bat_adc_val(bat_adc_val); // 得到滑动平均滤波后的值

    if (CUR_LED_MODE_OFF == cur_led_mode)
    {
        refresh_led_mode_status(); // 关闭所有指示灯
        return;
    }

    // 如果当前处于电池电量指示模式
    // 设备处于放电时，电量指示灯只显示电池电量降低的部分
    if (CUR_LED_MODE_BAT_INDICATOR == cur_led_mode)
    {
        /* 点亮指示灯1（放电模式，指示灯1始终点亮） */
        LED_1_ON();

        /*
            根据电池电压来划分
            如果电池电压小于5格
        */
        if (bat_adc_val > BAT_ADC_VAL_4)
        {
            // 电池电量大于4格
            if (bat_remaining_power_indication >= 5)
            {
                bat_remaining_power_indication = 5;
            }
        }

        if (bat_adc_val < BAT_ADC_VAL_4)
        {
            // 小于5格对应的电量
            if (bat_remaining_power_indication >= 4)
            {
                bat_remaining_power_indication = 4;
            }
        }

        if (bat_adc_val < BAT_ADC_VAL_3)
        {
            // 小于4格对应的电量
            if (bat_remaining_power_indication >= 3)
            {
                bat_remaining_power_indication = 3;
            }
        }

        if (bat_adc_val < BAT_ADC_VAL_2)
        {
            // 小于3格对应的电量
            if (bat_remaining_power_indication >= 2)
            {
                bat_remaining_power_indication = 2;
            }
        }

        if (bat_adc_val < BAT_ADC_VAL_1)
        {
            // 小于2格对应的电量
            // if (bat_remaining_power_indication >= 2)
            {
                bat_remaining_power_indication = 1;
            }
        }

        if (bat_remaining_power_indication >= 5)
        {
            LED_5_ON();
        }
        else
        {
            LED_5_OFF();
        }

        if (bat_remaining_power_indication >= 4)
        {
            LED_4_ON();
        }
        else
        {
            LED_4_OFF();
        }

        if (bat_remaining_power_indication >= 3)
        {
            LED_3_ON();
        }
        else
        {
            LED_3_OFF();
        }

        if (bat_remaining_power_indication >= 2)
        {
            LED_2_ON();
        }
        else
        {
            LED_2_OFF();
        }
    } // if (CUR_LED_MODE_BAT_INDICATOR == cur_led_mode)
    // else if (CUR_LED_MODE_INITIAL_DISCHARGE_GEAR == cur_led_mode)   // 初始放电挡位指示模式
    else // 处于初始放电挡位指示模式或是初始放电速率指示模式
    {
        if (CUR_LED_MODE_DISCHARGE_RATE == cur_led_mode)
        {
            // 放电速率指示模式
            switch (cur_discharge_rate)
            {
            case 1:
                LED_1_ON();
                break;
            case 2:
                LED_2_ON();
                break;
            case 3:
                LED_3_ON();
                break;
            }
        }
    }
    // else if (CUR_LED_MODE_DISCHARGE_RATE == cur_led_mode)
    // {
    //     // 放电速率指示模式

    //     if (flag_is_special_led_mode_times_come) // 时间到来，回到电池电量指示模式
    //     {
    //         cur_led_mode = CUR_LED_MODE_BAT_INDICATOR;
    //         flag_is_special_led_mode_times_come = 0;
    //     }
    // }

    if (flag_is_update_led_mode_times_come) // 时间到来，回到电池电量指示模式
    {
        flag_is_update_led_mode_times_come = 0;

        if (cur_light_pwm_duty_val) // 如果灯光还是亮着的
        {
            cur_led_mode = CUR_LED_MODE_BAT_INDICATOR;
        }
        else // 如果灯光已经熄灭
        {
            cur_led_mode = CUR_LED_MODE_OFF;
        }

        flag_is_in_setting_mode = 0; // 退出设置模式
    }
}
