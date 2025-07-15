// encoding UTF-8
// 灯光控制源程序
#include "light_handle.h"

void light_blink(void)
{
    u8 i;
    for (i = 0; i < 3; i++)
    {
        LIGHT_ON();
        delay_ms(161);
        LIGHT_OFF();
        delay_ms(161);
    }
}

// 灯光控制（放电控制）
void light_handle(void)
{
    // 如果正在充电，直接返回

    // 如果未在充电

    // if (flag_is_light_adjust_time_come)
    // {
    //     flag_is_light_adjust_time_come = 0;
    //     light_adjust_time_cnt++;
    // }

    if (1 == cur_discharge_rate) // 放电速率1档，M1
    {
        /*
            速度为M1，
            1200s后变化一次占空比，(1200 * 1)
            3600s后再变化一次，    (1200 * 3)
            7200s后再变化一次，    (1200 * 6)
            ...
            假设之后是：
            (1200 * 9)
            (1200 * 12)
            (1200 * 15)
            ...
            每次变化约10%占空比
        */

        if (light_adjust_time_cnt >= (1200 * light_ctl_phase_in_rate_1)) // 如果到了调节时间
        {
            light_adjust_time_cnt = 0;

            if (1 == light_ctl_phase_in_rate_1)
            {
                light_ctl_phase_in_rate_1 = 3;
            }
            else
            {
                light_ctl_phase_in_rate_1 += 3;
            }

            // 定时器对应的重装载值最大值 对应 100%占空比
            if (expect_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 10 / 100))
            {
                // 如果仍大于 4.8% + 10%， 减少10%占空比
                expect_light_pwm_duty_val -= (u32)TIMER2_FEQ * 10 / 100;
            }
            else
            {
                // 4.8%占空比
                expect_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
            }
        }
    }
    else // 2 == cur_discharge_rate || 3 == cur_discharge_rate
    {
        /*
            一开始每40s降低一次占空比
            从47%开始，每240s降低一次占空比
            从42%开始，每420s降低一次占空比

            暂定每次降低 0.6%
        */

        if (cur_light_pwm_duty_val > (u32)TIMER2_FEQ * 47 / 100)
        {
            if (light_adjust_time_cnt >= 40)
            {
                light_adjust_time_cnt = 0;

                if (expect_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 6 / 1000))
                {
                    // 如果仍大于 4.8% + xx %， 减少 xx %占空比
                    expect_light_pwm_duty_val -= (u32)TIMER2_FEQ * 6 / 1000;
                }
                else
                { 
                    // 4.8%占空比
                    expect_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
                }
            }
        }
        else if (cur_light_pwm_duty_val > (u32)TIMER2_FEQ * 42 / 100)
        {
            if (light_adjust_time_cnt >= 240)
            {
                light_adjust_time_cnt = 0;

                if (expect_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 6 / 1000))
                {
                    // 如果仍大于 4.8% + xx %， 减少 xx %占空比
                    expect_light_pwm_duty_val -= (u32)TIMER2_FEQ * 6 / 1000;
                }
                else
                { 
                    // 4.8%占空比
                    expect_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
                }
            }
        }
        else
        {
            if (light_adjust_time_cnt >= 420)
            {
                light_adjust_time_cnt = 0;

                if (expect_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 6 / 1000))
                {
                    // 如果仍大于 4.8% + xx %， 减少 xx %占空比
                    expect_light_pwm_duty_val -= (u32)TIMER2_FEQ * 6 / 1000;
                }
                else
                { 
                    // 4.8%占空比
                    expect_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
                }
            }
        }
    }
}
