#include "led_handle.h"

// TODO:需要初始化为5
// volatile u8 bat_remaining_power_indication = 5; // 电池剩余电量指示挡位

volatile u8 flag_is_update_led_mode_times_come = 0; // 标志位，LED模式的刷新时间到来

#if 0  // 滑动平均
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
#endif // 滑动平均

/**
 * @brief 更新led指示灯相关的状态
 *          当 cur_led_mode 改变时，会调用此函数，
 *          并且要放在 led_handle_update_percent_of_bat() 函数前
 *
 */
void led_stats_update(void)
{
    LED_1_OFF();
    LED_2_OFF();
    LED_3_OFF();
    LED_4_OFF();
    LED_5_OFF();
    last_led_gear = 0; // 赋值为初始值
    cur_led_gear_in_charging = 0;
}

// 更新LED指示灯 
void led_handle_update_percent_of_bat(void)
{

    // if (CUR_LED_MODE_OFF == cur_led_mode)
    // {
    //     refresh_led_mode_status(); // 关闭所有指示灯
    //     return;
    // }

    // 如果当前处于电池电量指示模式
    // 设备处于放电时，电量指示灯只显示电池电量降低的部分
    if (CUR_LED_MODE_BAT_INDICATOR == cur_led_mode)
    {
        /* 点亮指示灯1（放电模式，指示灯1始终点亮） */
        LED_1_ON();

        if (bat_adc_val > BAT_ADC_VAL_4) // 电池电量大于4格
        {
            cur_led_gear = 5; // 亮5格
        }
        else if (bat_adc_val > BAT_ADC_VAL_3) // 电池电量大于3格
        {
            cur_led_gear = 4; // 亮4格
        }
        else if (bat_adc_val > BAT_ADC_VAL_2) // 电池电量大于2格
        {
            cur_led_gear = 3; // 亮3格
        }
        else if (bat_adc_val > BAT_ADC_VAL_1) // 电池电量大于1格
        {
            cur_led_gear = 2; // 亮2格
        }
        else
        {
            cur_led_gear = 1; // 亮1格
        }

        if (0 == last_led_gear)
        {
            // 如果未初始化
            last_led_gear = cur_led_gear;
        }
        else
        {
            // 如果 last_led_gear 不为0，则说明已经初始化过了

            if (cur_led_gear > last_led_gear)
            {
                // 如果当前要显示的指示灯 大于 上次显示的指示灯（样机在电池电压上升的情况下，不会更新显示）
                cur_led_gear = last_led_gear;
            }
        }

        if (cur_led_gear >= 5)
        {
            LED_5_ON();
        }
        else
        {
            LED_5_OFF();
        }

        if (cur_led_gear >= 4)
        {
            LED_4_ON();
        }
        else
        {
            LED_4_OFF();
        }

        if (cur_led_gear >= 3)
        {
            LED_3_ON();
        }
        else
        {
            LED_3_OFF();
        }

        if (cur_led_gear >= 2)
        {
            LED_2_ON();
        }
        else
        {
            LED_2_OFF();
        }

    } // if (CUR_LED_MODE_BAT_INDICATOR == cur_led_mode)
    else if (CUR_LED_MODE_DISCHARGE_RATE == cur_led_mode) // 放电速率指示模式，M1、M2、M3
    {
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
    else if (CUR_LED_MODE_CHARGING == cur_led_mode) // 充电指示模式
    {
        // 在充电指示模式中，如果电池电量降低，不更新显示
        // TODO：样机是在电池电压超过3.55V并且进入涓流充电，绿色指示灯才一直点亮(第5格指示灯)

        /* 点亮指示灯1（指示灯1始终点亮） */
        LED_1_ON();

        if (bat_adc_val > BAT_ADC_VAL_5) // 电池电量大于5格
        {
            cur_led_gear = 6; // 亮5格，并且所有指示灯常亮
        }
        else if (bat_adc_val > BAT_ADC_VAL_4) // 电池电量大于4格
        {
            cur_led_gear = 5; // 亮5格，让第5格闪烁
        }
        else if (bat_adc_val > BAT_ADC_VAL_3) // 电池电量大于3格
        {
            cur_led_gear = 4; // 亮4格，让第4格闪烁
        }
        else if (bat_adc_val > BAT_ADC_VAL_2) // 电池电量大于2格
        {
            cur_led_gear = 3; // 亮3格，让第3格闪烁
        }
        else if (bat_adc_val > BAT_ADC_VAL_1) // 电池电量大于1格
        {
            cur_led_gear = 2; // 亮2格，让第2格闪烁
        }
        else
        {
            cur_led_gear = 1; // 亮1格，让第2格闪烁
        }

        if (0 == last_led_gear)
        {
            last_led_gear = cur_led_gear;
        }
        else
        {
            if (cur_led_gear < last_led_gear)
            {
                // 在充电指示模式中，如果电池电量降低，不更新显示
                cur_led_gear = last_led_gear;
            }
        }

        cur_led_gear_in_charging = cur_led_gear; // 更新数值，给定时器中断使用
        delay_ms(1);

        if (cur_led_gear >= 3)
        {
            LED_2_ON();
        }

        if (cur_led_gear >= 4)
        {
            LED_3_ON();
        }

        if (cur_led_gear >= 5)
        {
            LED_4_ON();
        }

        if (cur_led_gear >= 6)
        {
            LED_5_ON();
        }
    }

    if (flag_is_update_led_mode_times_come) // 时间到来，回到电池电量指示模式
    {
        flag_is_update_led_mode_times_come = 0;

        if (cur_light_pwm_duty_val) // 如果灯光还是亮着的
        {
            cur_led_mode = CUR_LED_MODE_BAT_INDICATOR; // 恢复到电池电量指示模式
        }
        else // 如果灯光已经熄灭
        {
            cur_led_mode = CUR_LED_MODE_OFF; // 恢复到关机模式
        }

        flag_is_in_setting_mode = 0; // 退出设置模式
    }

    if (CUR_LED_MODE_OFF == cur_led_mode)
    {
        LED_1_OFF();
        LED_2_OFF();
        LED_3_OFF();
        LED_4_OFF();
        LED_5_OFF();
    }
}
