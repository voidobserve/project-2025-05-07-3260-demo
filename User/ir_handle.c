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

/**
 * @brief 在设置模式，设置初始放电挡位或放电速率
 *          函数内部会判断 当前 是否处于设置模式
 *
 * @param set_led_mode
 *          CUR_LED_MODE_INITIAL_DISCHARGE_GEAR 要设置的是初始放电挡位
 *          CUR_LED_MODE_DISCHARGE_RATE     要设置的是放电速率
 * @param val 初始放电挡位 或 放电速率对应的值（需要注意不能超过范围）
 */
void set_led_mode_status(u8 set_led_mode, u8 val)
{
    // 如果在设置模式，才会进入
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
    static u8 last_ir_data = 0;

    if (cur_charge_phase != CUR_CHARGE_PHASE_NONE)
    {
        // 如果当前正在充电，直接返回
        flag_is_recved_data = 0;
        flag_is_recv_ir_repeat_code = 0;
        last_ir_data = 0;
        return;
    }

    if (flag_is_recved_data)
    {
        flag_is_recved_data = 0;

        last_ir_data = ir_data;

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

            /*
                不在设置模式，每次按下按键，在
                电池电量指示 -> 初始放电挡位 -> 放电速率 -> 电池电量指示 之间切换
            */
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
            // TODO：待完善功能

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, 2);

            break;

        case IR_KEY_3H_OR_NUM_3:
            // 3H，也是小遥控器的数字3
            // TODO：待完善功能

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, 3);

            break;

        case IR_KEY_BRIGHTNESS_ADD_OR_NUM_4:
            // 亮度加，也是小遥控器的数字4
            // TODO：待完善功能

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, 4);

            break;

        case IR_KEY_AUTO_OR_NUM_5:
            // 自动模式 ，也是小遥控器的数字5
            // TODO：待完善功能

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, 5);

            break;

        case IR_KEY_5H_OR_M1:
            // 5H，也是小遥控器的M1
            // TODO：待完善功能

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE, 1);

            break;

        case IR_KEY_M2:
            // 小遥控器的M2

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE, 2);

            break;

        case IR_KEY_8H_OR_M3:
            // 8H，也是小遥控器的M3
            // TODO：待完善功能

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE, 3);

            break;

        case IR_KEY_SET:
            // SET 模式设置

            flag_is_in_setting_mode = 1; // 表示进入设置模式
            refresh_led_mode_status();
            cur_led_mode = CUR_LED_MODE_SETTING; // 交给定时器处理，让所有指示灯都闪烁

            break;

        case IR_KEY_ON:
            // 开灯

            /* 根据初始的放电挡位来设定灯光对应的pwm占空比 */
            switch (cur_initial_discharge_gear)
            {
            case 1:
                // 初始放电挡位 1档，刚开始是 83.67%开始放电

                // 定时器对应的重装载值最大值 对应 100%占空比
                expect_light_pwm_duty_val = ((u32)TIMER2_FEQ * 8367 / 10000);

                break;

            case 2:
                // 初始放电挡位 2档，刚开始是 74.11%开始放电

                // 定时器对应的重装载值最大值 对应 100%占空比
                expect_light_pwm_duty_val = ((u32)TIMER2_FEQ * 7411 / 10000);

                break;

            case 3:
                // 初始放电挡位 3档，刚开始是 64.55%开始放电

                // 定时器对应的重装载值最大值 对应 100%占空比
                expect_light_pwm_duty_val = ((u32)TIMER2_FEQ * 6455 / 10000);

                break;

            case 4:
                // 初始放电挡位 4档，刚开始是 56.98%开始放电

                // 定时器对应的重装载值最大值 对应 100%占空比
                expect_light_pwm_duty_val = ((u32)TIMER2_FEQ * 5698 / 10000);

                break;

            case 5:
                // 初始放电挡位 5档，刚开始是 49.8%开始放电

                // 定时器对应的重装载值最大值 对应 100%占空比
                expect_light_pwm_duty_val = ((u32)TIMER2_FEQ * 4980 / 10000);

                break;
            }

            cur_light_pwm_duty_val = expect_light_pwm_duty_val;
            // SET_LIGHT_PWM_DUTY(cur_light_pwm_duty_val);
            timer2_set_pwm_duty(cur_light_pwm_duty_val); // 立刻更新PWM占空比
            LIGHT_ON();                                  // 使能PWM输出
            light_blink();
            light_adjust_time_cnt = 0; 
            refresh_led_mode_status(); 
            cur_led_mode = CUR_LED_MODE_BAT_INDICATOR;

            break;

        case IR_KEY_OFF:

            // TODO: 如果未关机
            if (expect_light_pwm_duty_val > 0 &&           /* 如果灯光还是亮的 */
                CUR_CHARGE_PHASE_NONE == cur_charge_phase) /* 当前未在充电 */
            {
                light_blink();
                // LIGHT_OFF() // light_blink() 最后就是关灯操作，可以不写这一句
                expect_light_pwm_duty_val = 0;
                cur_light_pwm_duty_val = 0;
                timer2_set_pwm_duty(0);
                LIGHT_OFF();
                cur_led_mode = CUR_LED_MODE_OFF;
            }

            break;

        } // switch (ir_data)
    } // if (flag_is_recved_data)

    if (flag_is_recv_ir_repeat_code)
    {
        flag_is_recv_ir_repeat_code = 0;
    }
}
