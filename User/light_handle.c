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

// 灯光控制
void light_handle(void)
{
    // 如果正在充电，直接返回

    // 如果未在充电

    if (flag_is_light_adjust_time_come)
    {
        flag_is_light_adjust_time_come = 0;
        light_adjust_time_cnt++;
    }

    if (1 == cur_discharge_rate)
    {
        /*
            速度为M1，
            1200s后变化一次占空比，
            3600s后再变化一次，
            7200s后再变化一次，
            ...
            每次变化约10%占空比
        */
        static u8 phase ;
        if (light_adjust_time_cnt >= (1200 * (phase + 1)))
        {
            phase++;

            if (expect_light_pwm_duty_val >= (255 * 480 / 1000))
            {
                expect_light_pwm_duty_val -= (255 * 10 / 100);
            }
            else
            {
                expect_light_pwm_duty_val = ()
            }

            
            // SET_LIGHT_PWM_DUTY(cur_light_pwm_duty_val);
        }
    }
    else // 2 == cur_discharge_rate || 3 == cur_discharge_rate
    {
        /*
            一开始每40s降低一次占空比
            从47%开始，每240s降低一次占空比
            从42%开始，每420s降低一次占空比
        */
    }
}
