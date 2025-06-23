#include "charge_handle.h"

volatile u8 cur_charge_status = CUR_CHARGE_STATUS_NONE;

// 充电控制
void charge_handle(void)
{
    // static volatile u8 cur_charge_status = CUR_CHARGE_STATUS_NONE; // 测试用

    /*
        标志位，是否使能充电，用于控制充满电后，等到充电电压小于一定值之后，再使能充电
        0 -- 不使能充电
        1 -- 使能充电
    */
    static volatile u8 flag_is_enable_charging = 0;

    volatile u16 charging_adc_val;
    volatile u16 bat_adc_val;

    adc_sel_pin(ADC_PIN_DETECT_CHARGE);
    charging_adc_val = adc_getval(); // 采集充电输入对应的ad值
    adc_sel_pin(ADC_PIN_DETECT_BATTERY);
    bat_adc_val = adc_getval(); // 采集电池电压对应的ad值

#if 0
    {
        static u16 cnt = 0;
        cnt++;
        if (cnt >= 100)
        {
            cnt = 0;
            printf("cur_charging_adc_val : %u\n", charging_adc_val);
            // printf("cur_bat_adc_val : %u\n", bat_adc_val);

            // printf("detect vol of charge: %u mV\n", (u16)((u32)charging_adc_val * 2 * 11000 / 4096));
            // printf("detect vol of bat: %u mV\n", (u16)((u32)bat_adc_val * 2 * 2 * 1000 / 4096));
        }
    }
#endif

#if 1
    // 如果当前未在充电
    if (CUR_CHARGE_STATUS_NONE == cur_charge_status)
    {

        if (charging_adc_val <= ADC_VAL_DISABLE_IN_CHARGE_END) /* 如果充电电压过小 */
        {
            flag_is_enable_charging = 1; // 使能充电
        }

        if (charging_adc_val >= ADC_VAL_ENABLE_IN_CHARGE_END)
        {
            // 如果在未充电时，检测到充电输入电压而使能充电

            if (flag_is_enable_charging) // 如果使能了充电
            {
                cur_charge_status = CUR_CHARGE_STATUS_IN_CHARGING;
                printf("in charging\n");
            }
        }
    }
    else // 如果当前正在充电
    {
        static u8 cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;

        if (charging_adc_val <= ADC_VAL_DISABLE_IN_CHARGE_END || /* 如果充电电压过小 */
            // charging_adc_val >= 2420 || /* 充电输入电压大于 13V ，断开充电 ，公式还有缺陷*/
            // charging_adc_val >= 2973 || /* 充电输入电压大于 15V ，断开充电 ，公式还有缺陷 */
            bat_adc_val >= ADC_VAL_BAT_IS_FULL) /* 如果电池已经满电 */
        {
            // 充电输入的电压小于一定值，断开充电
            // 电池 已经满电， 断开充电
            // timer0_pwm_disable();
            PWM_CTL_FOR_CHARGING_DISABLE();
            cur_charge_status = CUR_CHARGE_STATUS_NONE;
            cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;

            printf("disable charge\n");
            return;
        }

        // 进行涓流充电：
        if (bat_adc_val <= ADC_VAL_BAT_IS_LOW || /* 如果电池电压小于一定值 */
            // bat_adc_val >= ADC_VAL_BAT_IS_NEAR_FULL || /* 如果电池电压大于一定值 */
            charging_adc_val < 930) /* 充电电压小于5V */
        {
            if (CUR_CHARGING_PWM_STATUS_LOW_FEQ != cur_charging_pwm_status)
            {
                // timer0_pwm_set_low_feq(); // 函数内部设定了固定频率和固定占空比
                PWM_CTL_FOR_CHARGING_SET_LOW_FEQ();
                cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_LOW_FEQ;

                printf("low feq charge\n");
            }
        }
        else // 如果电池电压不是在涓流充电的范围
        {
            u32 tmp;

            // 如果电池电压不在涓流充电的区间，则正常充电
            // 注意这里不能随意改变 PWM占空比，最好设置PWM占空比为0
            if (CUR_CHARGING_PWM_STATUS_HIGH_FRQ != cur_charging_pwm_status)
            {
                // timer0_pwm_set_high_feq();
                PWM_CTL_FOR_CHARGING_SET_HIGH_FEQ();
                cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_HIGH_FRQ;
            }

            // 根据充电电压调整pwm占空比

            // 正常充电时，才调节控制充电的PWM占空比
            if (CUR_CHARGING_PWM_STATUS_HIGH_FRQ == cur_charging_pwm_status)
            {
                /*
                       测得样机充电控制的功能：
                       5V输入，pwm 105Khz，90%，（可能要使用 105Khz，85%）；12V输入，pwm 105Khz，30%
                       用（5V，85%）和（12V，30%）这两个点来计算，
                       已知两点坐标(5,85),(12,30)，求对应的方程，
                       对应直线的方程： y  =  -7.857x  + 124.286
                       y 对应占空比，单位：%；x 对应充电输入电压，单位：V
                       在公式上去掉小数点：
                       1000y = -7857x + 124286

                       单片机adc使用内部2.0V参考电压，
                       将公式转换成 占空比 和 ad值 的关系:
                       外部充电输入的电压 1/11 分压后，到单片机检测脚
                       外部输入电压 / 11 / 单片机adc参考电压 * 4096 == 单片机采集到的ad值
                       外部输入电压 == 单片机采集到的ad值 / 4096 * 单片机adc参考电压 * 11
                       换算成单片机可以计算的形式：
                       外部输入电压 == 单片机采集到的ad值 * 单片机adc参考电压 * 11 / 4096
                       x = adc_val * 2 * 11 / 4096

                       得到的公式：
                       1000y = 124286 - 7857 (adc_val * 2 * 11 / 4096)
                       1000y = 124286 - adc_val * 7857 * 2 * 11 / 4096
                       这个公式运算不会超出 2^32 范围
                   */
                tmp = (u32)124286 - (u32)charging_adc_val * 7857 * 2 * 11 / 4096;
                // 得到的 tmp 是1000倍的占空比值，需要除以1000， 再乘以 定时器重载值/100,得到定时器对应的占空比值
                // T1DATA = (u32)T1LOAD * (u32)tmp / 1000 / 100; // 最终的占空比值
                {
                    static u16 cnt = 0;
                    cnt++;
                    if (cnt >= 100)
                    {
                        cnt = 0;
                        printf("cur pwm percent : %lu %%\n", (u32)tmp / 1000);
                        // printf("cal val  : %lu \n", (u32)charging_adc_val * 7857 * 2 * 11 / 4096);
                    }
                }

                // tmp = (((u32)TMR1_PRH << 8) | (u32)TMR1_PRL) * (u32)tmp / 1000; // 最终的占空比值（用这个计算，得出的值是0）
                tmp = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * tmp / 1000 / 100; // 最终的占空比值

                /*
                    由于公式限制，可能充电电压大于15V就会导致占空比变为0，现在不让它到0%
                */
                if (0 == (u16)tmp)
                // if (0 == tmp)
                {
                    tmp = 1;
                }

#if 0
                {
                    static u16 cnt = 0;
                    cnt++;
                    if (cnt >= 100)
                    {
                        cnt = 0;
                        // printf("reg val : %u \n", (u16)tmp);
                        // printf("reg val : %u \n", (u16)(((u32)TMR1_PRH << 8) | (u32)TMR1_PRL));
                        // printf("reg val : %u \n", (u16)((u32)TIMER1_HIGH_FEQ_PEROID_VAL * tmp / 1000 / 100));
                    }
                }
#endif

                TMR1_PWMH = (tmp >> 8) & 0xFF;
                TMR1_PWML = tmp & 0xFF;
            }
        }
    }
#endif
}
