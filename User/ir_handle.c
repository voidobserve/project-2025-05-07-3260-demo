#include "ir_handle.h"
#include "user_config.h"

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
            // =================================================================
            // 大摇控器的红色按键，小遥控器的绿色按键                             //
            // =================================================================
        case IR_KEY_RED:

            if (flag_is_in_setting_mode)
            {
                // 正处于设置模式，不响应
                return;
            } 

            led_all_off(); // 关闭所有led
            led_struction_mode_exit_times_cnt = 0; // 清空led指示模式退出时间

            if (CUR_LED_MODE_OFF == cur_led_mode)
            {
                /*
                    如果之前已经关掉，又按下了该按键，一段时间后退出
                */
                flag_is_led_mode_exit_enable = 1;
            }  

            if (cur_led_mode < CUR_LED_MODE_IN_INSTRUCTION_MODE) // 如果不处于指示模式
            {
                cur_led_mode = CUR_LED_MODE_BAT_INDICATIOR_IN_INSTRUCTION_MODE; // 指示模式 子模式 电池电量指示
            }
            else // 如果处于指示模式
            {
                cur_led_mode++;
                if (cur_led_mode > CUR_LED_MODE_DISCHARGE_RATE_IN_INSTRUCTION_MODE)
                {
                    cur_led_mode = CUR_LED_MODE_BAT_INDICATIOR_IN_INSTRUCTION_MODE;
                }
            }

            break;

            // =================================================================
            // 数字1                                                           //
            // =================================================================
        case IR_KEY_NUM_1:

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, 1);

            break;

            // =================================================================
            // 亮度减，也是小遥控器的数字2                                      //
            // =================================================================
        case IR_KEY_BRIGHTNESS_SUB_OR_NUM_2:

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, 2);

            // 如果不在设置模式，才调节亮度：
            if (0 == flag_is_in_setting_mode)
            {
                // 查表，找到当前亮度对应表格中的位置
                u8 i;
                for (i = 0; i < (u8)(ARRAY_SIZE(light_pwm_sub_table) - 1); i++)
                {
                    if (cur_light_pwm_duty_val > light_pwm_sub_table[i])
                    {
                        break;
                    }
                }

                cur_light_pwm_duty_val = light_pwm_sub_table[i];
                LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // 这个操作应该可以统一放到主函数中来更新
            }

            // printf("light pwm sub\n");

            break;

        case IR_KEY_3H_OR_NUM_3:
            // 3H，也是小遥控器的数字3
            // TODO：待完善功能

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, 3);

            break;

            // =================================================================
            // 亮度加，也是小遥控器的数字4                                      //
            // =================================================================
        case IR_KEY_BRIGHTNESS_ADD_OR_NUM_4:

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, 4);

            // 如果不在设置模式，才调节亮度：
            if (0 == flag_is_in_setting_mode)
            {
                // 查表，找到当前亮度对应表格中的位置
                u8 i;
                for (i = 0; i < ARRAY_SIZE(light_pwm_add_table) - 1; i++)
                {
                    if (cur_light_pwm_duty_val < light_pwm_add_table[i])
                    {
                        break;
                    }
                }

                // 亮度增加时，不能超过当前的初始挡位
                if (light_pwm_add_table[i] > light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1])
                {
                    cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
                }
                else
                {
                    cur_light_pwm_duty_val = light_pwm_add_table[i];
                }

                LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // 这个操作应该可以统一放到主函数中来更新
            }

            break;

            // =================================================================
            // 自动模式 ，也是小遥控器的数字5                                   //
            // =================================================================
        case IR_KEY_AUTO_OR_NUM_5:

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, 5);

            // 如果不在设置模式，才调节：
            if (0 == flag_is_in_setting_mode)
            {
                // 直接设置为当前初始挡位对应的亮度：
                cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
                // 清空调节时间计时值：
                light_adjust_time_cnt = 0;
                LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val);
                light_blink(3);
            }

            break;

        case IR_KEY_5H_OR_M1:
            // 5H，也是小遥控器的M1
            // TODO：待完善功能

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE, 1);

            break;

            // =================================================================
            // 小遥控器的M2                                                    //
            // =================================================================
        case IR_KEY_M2:

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE, 2);

            break;

        case IR_KEY_8H_OR_M3:
            // 8H，也是小遥控器的M3
            // TODO：待完善功能

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE, 3);

            break;

        case IR_KEY_SET:
            // SET 模式设置

            // if (cur_led_mode == CUR_LED_MODE_SETTING ||
            //     cur_led_mode == CUR_LED_MODE_INITIAL_DISCHARGE_GEAR ||
            //     cur_led_mode == CUR_LED_MODE_DISCHARGE_RATE)
            if (flag_is_in_setting_mode)
            {
            }
            else // 不处于设置模式下，才进入
            {
                // 如果之前已经关机，这里要设置退出控制，设置结束后退出设置模式
                if (CUR_LED_MODE_OFF == cur_led_mode)
                {
                    flag_allow_light_in_setting_mode = 1;
                }

                flag_is_in_setting_mode = 1; // 表示进入设置模式
                // led_status_refresh();
                // cur_led_mode = CUR_LED_MODE_SETTING; // 交给定时器处理，让所有指示灯都闪烁
                led_mode_alter(CUR_LED_MODE_SETTING);
                light_blink(cur_initial_discharge_gear); // 第一次进入设置模式，主灯光闪烁，闪烁次数对应初始放电档位
            }

            break;

        case IR_KEY_ON:
            // 开灯

            led_init();
            light_init();
            led_mode_alter(CUR_LED_MODE_BAT_INDICATOR);

            break;

        case IR_KEY_OFF:

            // if (expect_light_pwm_duty_val > 0 &&           /* 如果灯光还是亮的 */
            //     CUR_CHARGE_PHASE_NONE == cur_charge_phase) /* 当前未在充电 */
            if (cur_light_pwm_duty_val > 0 &&              /* 如果灯光还是亮的 */
                CUR_CHARGE_PHASE_NONE == cur_charge_phase) /* 当前未在充电 */
            {
                cur_led_mode = CUR_LED_MODE_OFF;
                light_blink(2);
                // LIGHT_OFF() // light_blink() 最后就是关灯操作，可以不写这一句

                // expect_light_pwm_duty_val = 0;
                cur_light_pwm_duty_val = 0; //
                // timer2_set_pwm_duty(0);
                // LIGHT_OFF();
            }

            break;
        } // switch (ir_data)
    } // if (flag_is_recved_data)

    // 收到重复码
    if (flag_is_recv_ir_repeat_code)
    {
        flag_is_recv_ir_repeat_code = 0;

        if (IR_KEY_BRIGHTNESS_ADD_OR_NUM_4 == last_ir_data)
        {
            // 亮度加 每次变化 2.59%
            if (cur_light_pwm_duty_val <
                (light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1] -
                 (u16)((u32)TIMER2_FEQ * 259 / 10000))) /* 当前亮度值小于初始亮度值减去 2.59% */
            {
                cur_light_pwm_duty_val += (u16)((u32)TIMER2_FEQ * 259 / 10000);
            }
            else
            {
                /*
                    当前亮度值不小于初始亮度值减去 2.59%，直接变为初始亮度值
                */
                cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
            }

            LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // 这个操作应该可以统一放到主函数中来更新
        }
        else if (IR_KEY_BRIGHTNESS_SUB_OR_NUM_2 == last_ir_data)
        {
            // 亮度减 每次变化 2.59%
            if (cur_light_pwm_duty_val >
                (light_pwm_sub_table[ARRAY_SIZE(light_pwm_sub_table) - 1] +
                 (u16)((u32)TIMER2_FEQ * 259 / 10000))) /* 当前亮度值大于最小亮度值加 2.59% */
            {
                cur_light_pwm_duty_val -= (u16)((u32)TIMER2_FEQ * 259 / 10000);
            }
            else
            {
                /*
                    当前亮度值不大于最小亮度值加 2.59%，直接变为最小亮度值
                */
                cur_light_pwm_duty_val = light_pwm_sub_table[ARRAY_SIZE(light_pwm_sub_table) - 1];

                LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // 这个操作应该可以统一放到主函数中来更新
            }
        }
    }
}
