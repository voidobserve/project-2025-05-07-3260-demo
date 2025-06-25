#include "timer0.h"

#include "ir_handle.h"
#include "user_config.h"

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

#if 1  // 控制LED指示灯
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
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR0_IRQn);
}