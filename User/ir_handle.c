#include "ir_handle.h"
#include "user_config.h"

volatile u8 ir_data = 0;
volatile bit flag_is_recv_ir_repeat_code = 0;
volatile bit flag_is_recved_data = 0;

volatile bit flag_is_in_setting_mode = 0; // 是否处于设置模式

void refresh_led_mode_status(void)
{
    // 每次按下，清除退出时间计时
    special_led_mode_times_cnt = 0;

    // 切换LED要显示的模式前，先关闭所有指示灯
    LED_1_OFF();
    LED_2_OFF();
    LED_3_OFF();
    LED_4_OFF();
    LED_5_OFF();
}

/*
    set_led_mode

    CUR_LED_MODE_BAT_INDICATOR = 0,      // 电池电量指示模式
    CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, // 初始放电挡位 -- 从 xx% PWW开始放电
    CUR_LED_MODE_DISCHARGE_RATE,         // 放电速率
*/
void set_led_mode_status(u8 set_led_mode, u8 val)
{
    if (flag_is_in_setting_mode)
    {
        if (CUR_LED_MODE_INITIAL_DISCHARGE_GEAR == set_led_mode)
        {
            cur_initial_discharge_gear = val;
        }
        else // CUR_LED_MODE_DISCHARGE_RATE == set_led_mode
        {
            cur_discharge_rate = val;
        }

        cur_led_mode = set_led_mode;
        refresh_led_mode_status();
    }
}

void ir_handle(void)
{
    if (flag_is_recved_data)
    {
        flag_is_recved_data = 0;

        switch (ir_data)
        {
        case IR_KEY_RED:
            // 大摇控器的红色按键，小遥控器的绿色按键

            if (flag_is_in_setting_mode)
            {
                // 正处于设置模式，不响应
                return;
            }

            refresh_led_mode_status();
            cur_led_mode++;
            if (cur_led_mode > CUR_LED_MODE_DISCHARGE_RATE)
            {
                cur_led_mode = CUR_LED_MODE_BAT_INDICATOR;
            }

            break;

        case IR_KEY_NUM_1:
            // 数字1

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, 1);

            break;

        case IR_KEY_BRIGHTNESS_SUB_OR_NUM_2:
            // 亮度减，也是小遥控器的数字2

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, 2);

            break;

        case IR_KEY_3H_OR_NUM_3:
            // 3H，也是小遥控器的数字3

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, 3);

            break;

        case IR_KEY_BRIGHTNESS_ADD_OR_NUM_4:
            // 亮度加，也是小遥控器的数字4

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, 4);

            break;

        case IR_KEY_AUTO_OR_NUM_5:
            // 自动模式 ，也是小遥控器的数字5

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, 5);

            break;

        case IR_KEY_5H_OR_M1:
            // 5H，也是小遥控器的M1

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE, 1);

            break;

        case IR_KEY_M2:
            // 小遥控器的M2

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE, 2);

            break;

        case IR_KEY_8H_OR_M3:
            // 8H，也是小遥控器的M3

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE, 3);

            break;

        case IR_KEY_SET:
            // SET 模式设置

            flag_is_in_setting_mode = 1;
            cur_led_mode = CUR_LED_MODE_INITIAL_DISCHARGE_GEAR;
            refresh_led_mode_status();

            break;

        case IR_KEY_ON:
            // 开灯

            /* 根据初始的放电挡位来设定灯光对应的pwm占空比 */
            switch (cur_initial_discharge_gear)
            {
            case 1:

                break;

            case 2:

                break;

            case 3:

                break;

            case 4:
                // 初始放电挡位 4档，刚开始是 56.98%开始放电
                expect_light_pwm_duty_val = (u8)((u32)255 * 5698 / 10000);                

                break;

            case 5:
                // 初始放电挡位 5档，刚开始是 49.8%开始放电
                expect_light_pwm_duty_val = (u8)((u32)255 * 4980 / 10000);
                
                break;
            }

            cur_light_pwm_duty_val = expect_light_pwm_duty_val;
            SET_LIGHT_PWM_DUTY(cur_light_pwm_duty_val);
            light_blink();
            LIGHT_ON();
            cur_led_mode = CUR_LED_MODE_BAT_INDICATOR;

            break;

        case IR_KEY_OFF:

            if (expect_light_pwm_duty_val > 0) // TODO: 如果未关机
            {
                light_blink();
                // LIGHT_OFF() // light_blink() 最后就是关灯操作，可以不写这一句
                expect_light_pwm_duty_val = 0;
                cur_light_pwm_duty_val = 0;                
                cur_led_mode = CUR_LED_MODE_OFF;
            }

            break;

        } // switch (ir_data)
    } // if (flag_is_recved_data)
}
