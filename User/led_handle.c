#include "led_handle.h"

// 3.5V以上（1）（2）（3）（4）点亮
// 3.1~3.5V，（1）（2）（3）点亮
// 2.8~3.1V，（1）（2）点亮
// <2.8V，（1）点亮

// 指示灯从左往右数,分为1~5
// 电池电压检测脚检测到的电压,为电池的1/2分压
// 定义电池各个电压下对应的AD值:
enum
{
    BAT_ADC_VAL_1 = 2687, /* 2687.2 -- 电池2.8V对应的ad值 */
    BAT_ADC_VAL_2 = 3174, /* 3174.4 -- 电池3.1V对应的ad值 */
    BAT_ADC_VAL_3 = 3584, /* 3584 -- 电池3.5V对应的ad值 */
    // BAT_AD_VAL_4 = , /*  */
    // BAT_AD_VAL_5 = , /*  */
};

#define BAT_ADC_VAL_DEAD_ZONE (50) // 电池电压对应的ad值死区

enum
{
    CUR_LED_MODE_BAT_INDICATOR = 0,      // 电池电量指示模式
    CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, // 初始放电挡位 -- 从 xx% PWW开始放电
    CUR_LED_MODE_DISCHARGE_RATE,         // 放电速率
};

volatile u8 cur_led_mode; // 当前的LED模式

/* 滑动平均 */
#define BAT_ADC_VAL_SAMPLE_COUNT 20 // 样本计数
// static volatile u16 samples[SAMPLE_COUNT] = {0};
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

void led_handle_update_percent_of_bat(void)
{
    u16 bat_adc_val;

    adc_sel_pin(ADC_PIN_DETECT_BATTERY);
    bat_adc_val = adc_getval(); // 采集电池电压对应的ad值
    bat_adc_val = get_filtered_bat_adc_val(bat_adc_val); // 得到滑动平均滤波后的值

    // 如果当前处于电池电量指示模式
    // TODO: 需要加上一分钟更新一次的操作,否则会有灯光频繁闪烁
    if (CUR_LED_MODE_BAT_INDICATOR == cur_led_mode)
    {
        /* 点亮指示灯1 */
        LED_1_ON();

        /* 点亮指示灯1 指示灯2 */
        if (bat_adc_val >= BAT_ADC_VAL_1 + BAT_ADC_VAL_DEAD_ZONE)
        {
            LED_2_ON();
        }
        else
        {
            LED_2_OFF();
        }

        /* 点亮指示灯1 指示灯2 指示灯3 */
        if (bat_adc_val >= BAT_ADC_VAL_2 + BAT_ADC_VAL_DEAD_ZONE)
        {
            LED_3_ON();
        }
        else
        {
            LED_3_OFF();
        }

        /* 点亮指示灯1 指示灯2 指示灯3 指示灯4 */
        if (bat_adc_val >= BAT_ADC_VAL_3 + BAT_ADC_VAL_DEAD_ZONE)
        {
            LED_4_ON();
        } 
        else
        {
            LED_4_OFF();
        }
    }
}
