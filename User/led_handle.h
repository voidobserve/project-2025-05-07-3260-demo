#ifndef __LED_HANDLE_H
#define __LED_HANDLE_H

#include "include.h"
#include "user_config.h"

/*
    3.2V以上，（1）（2）（3）（4）（5）点亮
    3.05V以上，（1）（2）（3）（4）
    2.85V以上，（1）（2）（3）
    2.65V以上，（1）（2）
    2.65V以下，（1）
*/

// 指示灯从左往右数,分为1~5
// 电池电压检测脚检测到的电压,为电池的1/2分压
// 定义电池各个电压下对应的AD值:
enum
{
    BAT_ADC_VAL_1 = (u16)((u32)2650 * 4096 / 2 / 2 / 1000), /* 2713.6 -- 电池2.65V对应的ad值 */
    BAT_ADC_VAL_2 = (u16)((u32)2850 * 4096 / 2 / 2 / 1000),
    BAT_ADC_VAL_3 = (u16)((u32)3050 * 4096 / 2 / 2 / 1000),
    BAT_ADC_VAL_4 = (u16)((u32)3200 * 4096 / 2 / 2 / 1000),
};

// #define BAT_ADC_VAL_DEAD_ZONE (50) // 电池电压对应的ad值死区

enum
{
    CUR_LED_MODE_OFF = 0,                // 关机，指示灯全灭
    CUR_LED_MODE_BAT_INDICATOR,          // 电池电量指示模式
    CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, // 初始放电挡位 -- 从 xx% PWW开始放电
    CUR_LED_MODE_DISCHARGE_RATE,         // 放电速率
    CUR_LED_MODE_CHARGING,               // 充电指示模式
};

#define BAT_ADC_VAL_SAMPLE_COUNT 20 // 滑动平均的样本计数

extern volatile u8 cur_led_mode;                   // 当前的LED模式
extern volatile u8 bat_remaining_power_indication; // 电池剩余电量指示挡位
extern volatile u8 cur_initial_discharge_gear;     // 初始放电挡位
extern volatile u8 cur_discharge_rate;             // 初始放电速率（需要记忆）

extern volatile u8 flag_is_update_led_mode_times_come; // 标志位，LED特殊模式的时间到来

void led_handle_update_percent_of_bat(void);

#endif
