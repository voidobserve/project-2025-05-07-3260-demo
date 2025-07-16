#include "timer0.h"

#include "ir_handle.h"
#include "user_config.h"

// 特殊的LED模式，退出时间计数
u16 special_led_mode_times_cnt = 0;

void timer0_config(void)
{
    __EnableIRQ(TMR0_IRQn); // 使能timer0中断
    IE_EA = 1;              // 使能总中断

    // 设置timer0的计数功能，配置一个频率为1kHz的中断
    TMR_ALLCON = TMR0_CNT_CLR(0x1);                               // 清除计数值
    TMR0_PRH = TMR_PERIOD_VAL_H((TIMER0_PERIOD_VAL >> 8) & 0xFF); // 周期值
    TMR0_PRL = TMR_PERIOD_VAL_L((TIMER0_PERIOD_VAL >> 0) & 0xFF);
    TMR0_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);                          // 计数等于周期时允许发生中断
    TMR0_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x7) | TMR_MODE_SEL(0x1); // 选择系统时钟，128分频，计数模式
}

void TIMR0_IRQHandler(void) interrupt TMR0_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR0_IRQn);

    // ---------------- 用户函数处理 -------------------

    // 周期中断
    if (TMR0_CONH & TMR_PRD_PND(0x1))
    {
        TMR0_CONH |= TMR_PRD_PND(0x1); // 清除pending

#if 1     // 红外解码【需要放到100us的定时器中断来处理】
        { // 红外解码
            // static volatile u8 ir_fliter;
            static volatile u16 ir_level_cnt; // 红外信号的下降沿时间间隔计数
            static volatile u32 __ir_data;    //
            static volatile bit last_level_in_ir_pin = 0;
            // static volatile u16 ir_long_press_cnt; // 檢測紅外遙控長按的計數值

            // 对每个下降沿进行计数
            if (ir_level_cnt <= 1300)
                ir_level_cnt++;

            // 滤波操作
            // ir_fliter <<= 1;
            // if (IR_RECV_PIN)
            // {
            //     ir_fliter |= 1;
            // }
            // ir_fliter &= 0x07;

            // if (ir_fliter == 0x07)
            //     filter_level = 1;
            // else if (ir_fliter == 0x00)
            //     filter_level = 0;

            // if (filter_level)
            if (IR_RECV_PIN)
            {
                last_level_in_ir_pin = 1; // 表示接收到的是高电平

                // 如果之前也是高电平
                if (ir_level_cnt > 1200) // 超时时间(120000us,120ms)
                {
                    // if (__ir_data != 0) // 超时，且接收到数据(红外接收处理函数中会把ir_data清零)
                    if (__ir_data != 0) // 超时，且接收到数据(现在是在中断服务函数中把__ir_data自动清零)
                    {
                        // // 带校验的版本：
                        // if ((u8)(__ir_data >> 8) == (u8)(~__ir_data)) // 如果键值的原码和反码相等
                        // {
                        // flag_is_recved_data = 1;
                        // }

                        // 不带校验的版本
                        if (0 == flag_is_recved_data)
                        {
                            // if ((__ir_data & 0xFF0000) == 0xFF0000)
                            {
                                ir_data = ~__ir_data;
                                __ir_data = 0;
                                flag_is_recved_data = 1;
                            }
                        }
                    }

                    flag_is_recv_ir_repeat_code = 0; // 认为遥控器按键已经按下，然后松开
                }
            }
            else
            {
                if (last_level_in_ir_pin)
                {
                    // 如果之前检测到的是高电平，现在检测到了低电平
                    if (ir_level_cnt <= 8) // 小于800us，说明接收到无效的数据，重新接收
                    {
                        // FLAG_IS_RECVED_ERR_IR_DATA = 1;
                        flag_is_recv_ir_repeat_code = 0;
                    }
                    else if (ir_level_cnt <= 18) // 小于1800us,说明接收到了逻辑0
                    {
                        __ir_data <<= 1;

                        // P15D = 0; // 测试红外解码
                        // P15D = ~P15D; // 测试红外解码
                    }
                    else if (ir_level_cnt <= 26) // 小于2600us,说明接收到了逻辑1
                    {
                        __ir_data <<= 1;
                        __ir_data |= 0x01;

                        // P15D = 1; // 测试红外解码
                    }
                    else if (ir_level_cnt <= 150) // 小于15000us,说明接收到了格式头
                    {
                        // FLAG_IS_RECVED_ERR_IR_DATA = 1;
                        // ir_long_press_cnt = 0; // 加上这一条会检测不到长按
                    }
                    else if (ir_level_cnt <= 420) // 小于42ms,说明接收完一次完整的红外信号
                    {
#if 0 // 带校验的版本，命令源码与命令反码进行校验
    
                if ((u8)(__ir_data >> 8) == (u8)(~__ir_data)) // 如果键值的原码和反码相等
                {
                    flag_is_recved_data = 1;
                    flag_is_recv_ir_repeat_code = 1; //
                    ir_long_press_cnt = 0;
                }

#else // 不带校验的版本

                        if (0 == flag_is_recved_data) // 如果之前未接收到数据/接收到的数据已经处理完毕
                        {
                            // if ((__ir_data & 0xFF0000) == 0xFF0000)
                            {
                                ir_data = ~__ir_data;
                                __ir_data = 0;
                                flag_is_recved_data = 1;
                                // flag_is_recv_ir_repeat_code = 1; //
                            }
                        }

#endif // 不带校验的版本
                    }
                    else if (ir_level_cnt <= 1200) // 小于120000,120ms,说明接收到了重复码
                    {
                        // if (ir_long_press_cnt < 65535)
                        //     ir_long_press_cnt++;

                        flag_is_recv_ir_repeat_code = 1;
                    }
                    // else // 超过120000,说明接收到无效的数据
                    // {
                    // }

                    ir_level_cnt = 0;
                }

                last_level_in_ir_pin = 0; // 表示接收到的是低电平
            }
        } // 红外解码
#endif // 红外解码【需要放到100us的定时器中断来处理】

#if 1 // 控制LED指示灯

        {                   // 控制LED指示灯--需要放在100us的中断
            static u16 cnt; // 用软件实现PWM驱动LED的相关变量
            cnt++;

            if (cnt <= 20) // 20 * 100us
            {
                if (flag_is_led_1_enable)
                {
                    LED_1_PIN = LED_ON_LEVEL;
                }

                if (flag_is_led_2_enable)
                {
                    LED_2_PIN = LED_ON_LEVEL;
                }

                if (flag_is_led_3_enable)
                {
                    LED_3_PIN = LED_ON_LEVEL;
                }

                if (flag_is_led_4_enable)
                {
                    LED_4_PIN = LED_ON_LEVEL;
                }

                if (flag_is_led_5_enable)
                {
                    LED_5_PIN = LED_ON_LEVEL;
                }
            }
            else
            {
                LED_1_PIN = LED_OFF_LEVEL;
                LED_2_PIN = LED_OFF_LEVEL;
                LED_3_PIN = LED_OFF_LEVEL;
                LED_4_PIN = LED_OFF_LEVEL;
                LED_5_PIN = LED_OFF_LEVEL;
            }

            if (cnt >= 100) // 100 * 100us
            {
                cnt = 0;
            }

        } // 控制LED指示灯--需要放在100us的中断
#endif // 控制LED指示灯

        {
            static u8 cnt = 0;
            static u8 blink_cnt = 0;
            static bit flag_blink_dir = 0;
            cnt++;

            if (cnt >= 10) // 10 * 100us == 1ms
            {
                cnt = 0;

#if 1 // 控制LED指示灯的闪烁效果

                blink_cnt++;
                if (blink_cnt >= 200)
                {
                    blink_cnt = 0;

                    // 处于初始放电挡位指示模式，才进入
                    if (CUR_LED_MODE_INITIAL_DISCHARGE_GEAR == cur_led_mode)
                    {
                        if (0 == flag_blink_dir)
                        {
                            switch (cur_initial_discharge_gear)
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
                            case 4:
                                LED_4_ON();
                                break;
                            case 5:
                                LED_5_ON();
                                break;
                            }

                            flag_blink_dir = 1;
                        }
                        else
                        {
                            switch (cur_initial_discharge_gear)
                            {
                            case 1:
                                LED_1_OFF();
                                break;
                            case 2:
                                LED_2_OFF();
                                break;
                            case 3:
                                LED_3_OFF();
                                break;
                            case 4:
                                LED_4_OFF();
                                break;
                            case 5:
                                LED_5_OFF();
                                break;
                            }

                            flag_blink_dir = 0;
                        }
                    } // if (CUR_LED_MODE_INITIAL_DISCHARGE_GEAR == cur_led_mode)
                    // 刚按下遥控器的 SET 按键，会进入 设置模式，5个指示灯一起闪烁
                    else if (CUR_LED_MODE_SETTING == cur_led_mode)
                    {
                        if (0 == flag_blink_dir)
                        {
                            LED_1_ON();
                            LED_2_ON();
                            LED_3_ON();
                            LED_4_ON();
                            LED_5_ON();
                            flag_blink_dir = 1;
                        }
                        else
                        {
                            LED_1_OFF();
                            LED_2_OFF();
                            LED_3_OFF();
                            LED_4_OFF();
                            LED_5_OFF();
                            flag_blink_dir = 0;
                        }
                    }
                    // 指示灯处于充电指示模式
                    else if (CUR_LED_MODE_CHARGING == cur_led_mode)
                    {
                        if (cur_led_gear_in_charging <= 2)
                        {
                            if (0 == flag_blink_dir)
                            {
                                LED_2_ON();
                                flag_blink_dir = 1;
                            }
                            else
                            {
                                LED_2_OFF();
                                flag_blink_dir = 0;
                            }
                        }
                        else if (cur_led_gear_in_charging <= 3)
                        {
                            if (0 == flag_blink_dir)
                            {
                                LED_3_ON();
                                flag_blink_dir = 1;
                            }
                            else
                            {
                                LED_3_OFF();
                                flag_blink_dir = 0;
                            }
                        }
                        else if (cur_led_gear_in_charging <= 4)
                        {
                            if (0 == flag_blink_dir)
                            {
                                LED_4_ON();
                                flag_blink_dir = 1;
                            }
                            else
                            {
                                LED_4_OFF();
                                flag_blink_dir = 0;
                            }
                        }
                        else if (cur_led_gear_in_charging <= 5)
                        {
                            if (0 == flag_blink_dir)
                            {
                                LED_5_ON();
                                flag_blink_dir = 1;
                            }
                            else
                            {
                                LED_5_OFF();
                                flag_blink_dir = 0;
                            }
                        }
                    }

                } // if (blink_cnt >= 200)

#endif // 控制LED指示灯的闪烁效果

#if 1 // 退出特殊的led指示模式的时间计数
      // if (cur_led_mode != CUR_LED_MODE_BAT_INDICATOR) // 不处于电池电量指示模式，开始累计时间
                {
                    special_led_mode_times_cnt++;
                    if (special_led_mode_times_cnt >= 5000)
                    {
                        special_led_mode_times_cnt = 0;
                        flag_is_update_led_mode_times_come = 1;
                    }
                }
#endif // 退出特殊的led指示模式的时间计数

                {
                    static u8 cnt = 0;
                    // static u16 cnt = 0;
                    cnt++;
                    if (cnt >= 200)
                    // if (cnt >= 1000) // 延长时间并不会影响电池快满电后，调节PWM的大小，还是会过冲，电流过大
                    {
                        cnt = 0;
                        flag_is_charging_adjust_time_come = 1; // 调节充电电流/功率
                    }
                }

#if 1 // 在涓流充电时，负责每段时间断开一会PWM输出
                {
                    static u16 cnt = 0;

                    if (CUR_CHARGING_PWM_STATUS_LOW_FEQ == cur_charging_pwm_status)
                    {
                        // 如果在涓流充电
                        cnt++;

                        if (cnt < 22000) // 22 s
                        {
                            timer1_pwm_enable();
                            // flag_is_tim_turn_off_pwm = 0;
                        }
                        else if (cnt <= (22000 + 60)) // 22s + 60ms
                        {
                            // 累计涓流充电22s后，关闭控制充电的PWM，之后可以在这期间检测电池是否满电
                            timer1_pwm_disable();
                            // flag_is_tim_turn_off_pwm = 1;
                            // cnt = 0;
                        }
                        else
                        {
                            cnt = 0;
                        }
                    }
                    else
                    {
                        cnt = 0;
                    }
                }
#endif // 在涓流充电时，负责每段时间断开一会PWM输出

            } // if (cnt >= 10) // 10 * 100us == 1ms
        }

#if 1 // 放电时间控制

        // TODO:如果不在充电，设备没有关机，才进行放电时间累计
        if (CUR_CHARGE_PHASE_NONE == cur_charge_phase) // 如果不在充电
        {
            static u16 cnt = 0;
            cnt++;
            if (cnt >= 10000) // 10000 * 100us，1s
            {
                cnt = 0;
                // flag_is_light_adjust_time_come = 1;

                if (light_adjust_time_cnt < 4294967295) // 防止计数溢出
                {
                    light_adjust_time_cnt++;
                }
            }
        }

#endif // 放电时间控制

#if 0 // 缓慢调节驱动灯光的pwm占空比

        {
            // static u8 cnt =0;

            // 暂定每100us调节一次

            if (cur_light_pwm_duty_val > expect_light_pwm_duty_val)
            {
                cur_light_pwm_duty_val--;
            }
            else if (cur_light_pwm_duty_val < expect_light_pwm_duty_val)
            {
                cur_light_pwm_duty_val++;
            }

            // SET_LIGHT_PWM_DUTY(cur_light_pwm_duty_val);
            // timer2_set_pwm_duty(cur_light_pwm_duty_val);
        }

#endif // 缓慢调节驱动灯光的pwm占空比
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR0_IRQn);
}