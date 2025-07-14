#include "charge_handle.h"

// volatile u8 cur_charge_status = CUR_CHARGE_STATUS_NONE;

// 充电控制
void charge_handle(void)
{

    if (flag_is_charging_adjust_time_come) // 一定要加入缓慢调节，不能迅速调节，否则会等不到电压稳定
    {
        u16 current = 0; // 充电电流，单位：mA
        // u16 voltage_of_charging = 0; // 充电电压，单位：mV
        u16 voltage_of_bat = 0; // 电池电压
        u32 power = 0;          // 功率，单位：mW 毫瓦
        static u8 pwm_duty = 0; //
        u16 pwm_reg = 0;        // 存放要写入到寄存器中的占空比值

        u16 expected_power = 0; // 期望功率

        flag_is_charging_adjust_time_come = 0; // 清除标志位

        // 如果当前没有在充电
        if (CUR_CHARGE_PHASE_NONE == cur_charge_phase)
        {
            adc_sel_ref_voltage(ADC_REF_2_0_VOL); // 2V参考电压
            adc_sel_pin(ADC_PIN_DETECT_CHARGE);
            charging_adc_val = adc_getval();

            // 如果充电输入电压大于4.9V，使能充电
            if (charging_adc_val >= (u16)((u32)4900 * 4096 / 11 / 2 / 1000))
            {
                cur_charge_phase = CUR_CHARGE_PHASE_TRICKLE_CHARGE; // 刚进入充电，默认是电池电量低对应的涓流充电
            }

            adc_sel_pin(ADC_PIN_DETECT_BATTERY);
            bat_adc_val = adc_getval();
            if (bat_adc_val >= (u16)((u32)(3600 - 50) * 4096 / 2 / 2 / 1000))
            {
                // 检测到电池电压大于3.6 - 0.05V
                // 不使能充电
                cur_charge_phase = CUR_CHARGE_PHASE_NONE;
            }

            // 如果充电电压未满足使能充电的条件，会进入下面的语句块
            if (CUR_CHARGE_PHASE_NONE == cur_charge_phase)
            {
                return;
            }
        }
        else
        {
            // 如果当前在充电
            adc_sel_ref_voltage(ADC_REF_3_0_VOL); // 3V参考电压
            adc_sel_pin(ADC_PIN_DETECT_CHARGE);
            charging_adc_val = adc_getval();

            // 如果充电电压过大，PWM百分比设置为0，等到电压变小才打开
            // if (charging_adc_val >= (u16)((u32)30000 * 4096 / 11 / 3 / 1000)) // 充电电压超过30V
            if (charging_adc_val >= (u16)((u32)28000 * 4096 / 11 / 3 / 1000)) // 充电电压超过 xx V
            {
                pwm_reg = 0;
                TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
                TMR1_PWML = pwm_reg & 0xFF;
                return;
            }

            // 如果充电输入电压小于4V，断开充电
            if (charging_adc_val <= (u16)((u32)4000 * 4096 / 11 / 3 / 1000))
            {
                pwm_reg = 0;
                TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
                TMR1_PWML = pwm_reg & 0xFF;
                timer1_pwm_disable();

                cur_charge_phase = CUR_CHARGE_PHASE_NONE;
                cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;
                return;
            }

            // 如果已经充满电，直接退出
            if (CUR_CHARGE_PHASE_FULLY_CHARGE == cur_charge_phase)
            {
                return;
            }

            // 可能是对应的MOS管未使能，导致充电电流小，不是涓流充电：
            // if (charging_adc_val <= (u16)((u32)4900 * 4096 / 11 / 3 / 1000)) // 小于4.9V，进行涓流充电
            // {
            //     pwm_reg = (u32)TIMER1_LOW_FEQ_PEROID_VAL * 13 / 100; // 最终的占空比值
            //     TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
            //     TMR1_PWML = pwm_reg & 0xFF;
            //     timer1_set_pwm_low_feq();
            //     cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_LOW_FEQ;
            //     return;
            // }
        }

        // 进入到这里，说明正在充电，且充电电压在 4V~30V之间，不包括4V和30V

        // 检测电池电压，使用内部2.0V参考电压
        adc_sel_ref_voltage(ADC_REF_2_0_VOL);
        adc_sel_pin(ADC_PIN_DETECT_BATTERY);
        bat_adc_val = adc_getval();

        // 刚进入充电，会进入下面这个语句块：
        if (CUR_CHARGE_PHASE_TRICKLE_CHARGE == cur_charge_phase)
        {
            // 如果电池电压小于2.7V，进行涓流充电
            if (bat_adc_val <= (u16)((u32)2700 * 4096 / 2 / 2 / 1000))
            {
                if (CUR_CHARGING_PWM_STATUS_LOW_FEQ != cur_charging_pwm_status)
                {
                    pwm_reg = (u32)TIMER1_LOW_FEQ_PEROID_VAL * 13 / 100; // 最终的占空比值
                    TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
                    TMR1_PWML = pwm_reg & 0xFF;
                    timer1_set_pwm_low_feq();
                    cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_LOW_FEQ;
                }

                return;
            }

            // 如果电池电压不小于2.7V，进行正常充电
            cur_charge_phase = CUR_CHARGE_PHASE_NORMAL_CHARGE;
        }

        // TODO:
        // 电池电压大于 xx V，从正常充电变为涓流充电
        if ((bat_adc_val >= (u16)((u32)3700 * 4096 / 2 / 2 / 1000)) &&
            (CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE != cur_charge_phase))
        {
            pwm_reg = (u32)TIMER1_LOW_FEQ_PEROID_VAL * 13 / 100; // 最终的占空比值
            TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
            TMR1_PWML = pwm_reg & 0xFF;
            timer1_set_pwm_low_feq();
            cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_LOW_FEQ;
            cur_charge_phase = CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE;
        }

        // 如果充电阶段已经到了电池接近满电的阶段
        if (CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE == cur_charge_phase)
        {
            static u8 fully_charge_cnt = 0;

            // adc_sel_ref_voltage(ADC_REF_2_0_VOL);

            if (flag_is_tim_turn_off_pwm)
            {
                // 如果在涓流充电的 断开充电期间
                bat_adc_val = adc_getval();
                if (bat_adc_val >= (u16)((u32)3600 * 4096 / 2 / 2 / 1000))
                {
                    // 检测到电池电压大于3.6V
                    fully_charge_cnt++;
                }

                if (fully_charge_cnt >= 10)
                {
                    fully_charge_cnt = 0;

                    pwm_reg = 0;
                    TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
                    TMR1_PWML = pwm_reg & 0xFF;
                    timer1_pwm_disable();

                    cur_charge_phase = CUR_CHARGE_PHASE_FULLY_CHARGE; // 表示已经充满电，接下来需要等充电电压低于4.0V
                    cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;
                }
            }
            else
            {
                fully_charge_cnt = 0;
            }

            // TODO：
            if (bat_adc_val >= (u16)((u32)3700 * 4096 / 2 / 2 / 1000))
            {
                // 如果之前检测到电池电压大于 xx V，并且还在涓流充电，立即停止充电
                pwm_reg = 0;
                TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
                TMR1_PWML = pwm_reg & 0xFF;
                timer1_pwm_disable();

                cur_charge_phase = CUR_CHARGE_PHASE_FULLY_CHARGE; // 表示已经充满电，接下来需要等充电电压低于4.0V
                cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;
                fully_charge_cnt = 0;
            }

            return;
        }

        // ===================================================================
        // 以下都是正常充电对应的控制程序，cur_charge_phase == CUR_CHARGE_PHASE_NORMAL_CHARGE
        // ===================================================================

        // 不是正常充电，提前返回
        if (CUR_CHARGE_PHASE_NORMAL_CHARGE != cur_charge_phase)
        {
            return;
        }

        // 如果PWM未切换到高频
        if (CUR_CHARGING_PWM_STATUS_HIGH_FRQ != cur_charging_pwm_status)
        {
            pwm_reg = 0;
            TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
            TMR1_PWML = pwm_reg & 0xFF;
            timer1_set_pwm_high_feq();
            cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_HIGH_FRQ;
        }

        // 如果有一次电池电压超过3.5V
        // if (bat_adc_val >= (u16)((u32)3500 * 4096 / 2 / 2 / 1000))
        if (bat_adc_val >= (u16)((u32)3400 * 4096 / 2 / 2 / 1000))
        {
            // expected_power = 24000; // 实际测试20W不到，接近20W
            // expected_power = 20000; //
            // expected_power = 10000; // 电流很大
            expected_power = 5000; // 调节后PWM的占空比太小，可能连 1% 都不到
            // expected_power = 1000; // 电流太小，PWM占空比也小
        }
        else // 如果电池电压不超过3.5V
        {
            // if (expected_power != 24000)
            {
                // expected_power = 27000; // 实际测试约30W
                expected_power = 26500; 
            }
        }

        adc_sel_ref_voltage(ADC_REF_3_0_VOL);
        adc_sel_pin(ADC_PIN_DETECT_CURRENT);
        current_adc_val = adc_getval();

        /*
            检测电流引脚检测到的电压 == ad值 / 4096 * 参考电压
            current_adc_val * 3 / 4096

            检测电流的引脚检测到的充电电流 == 检测电流引脚检测到的电压 / 110(运放放大倍数) / 0.005R，
            逐步换成单片机可以计算的形式：
            current_adc_val * 3 / 4096 / 110 / 0.005
            current_adc_val * 3 / 4096 / 110 * 1000 / 5
            current_adc_val * 3 * 1000 / 5 / 4096 / 110
            得到的是以A为单位的电流，需要再转换成以mA为单位：
            current_adc_val * 3 * 1000 * 1000 / 5 / 4096 / 110，计算会溢出，需要再化简
            (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110
            current =  (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110;
        */
        current = (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 76; // 

        /*
            计算充电电压
        */
        // voltage_of_charging = (u32)charging_adc_val * 3 * 1000 * 11 / 4096;
        /*
            计算电池电压
            电池电压（mV） == 采集到的ad值 / 4096 * 参考电压 * 分压系数 * 1000（mV）
        */
        voltage_of_bat = (u32)bat_adc_val * 2 * 1000 * 2 / 4096;

        // 如果检测到电流的ad值已经爆表
        if (current_adc_val >= 4095)
        // if (current >= 5400) // 如果电流值已经爆表，超过单片机能检测的值：5454.54
        {
            // printf("current overflow\n");
            if (pwm_duty > 0)
            {
                pwm_duty--;
            }
        }
        else //
        {
            // power = voltage_of_charging * current / 1000; // 1000mV * 1000mA == 1000000 (1 Watt)
            power = (u32)voltage_of_bat * current / 1000; // 1000mV * 1000mA == 1000000 (1 Watt)

            // 假设充电有20%的损耗，充电输出30W，流入电池24W
            // printf("power %lu \n", power);
            // if (power < 30000) // 30 * 1000 mW（实际测得这里要比30W还大，未考虑上压降）
            // if (power < 27000) // xx * 1000 mW（实际测试约24.6W）
            // if (power < 24000) // xx * 1000 mW（实际测试不到20W，接近20W）
            // if (power < 20000) // xx * 1000 mW
            // if (power == 0) // 这种情况未考虑
            // {

            // }
            // else if (power < expected_power)
            
            if (power < expected_power)
            {
                if (pwm_duty < 100)
                {
                    pwm_duty++;
                }
            }
            // else if (power > 30000)
            // else if (power > 27000) // （实际测试约24.6W）
            // else if (power > 24000) // （实际测试不到20W，接近20W）
            // else if (power > 20000)
            else if (power > expected_power)
            {
                if (pwm_duty > 0)
                {
                    pwm_duty--;
                }
            }
            else // power == 目标值，不做调节
            {
            }
        }

        // printf("pwm_duty : %bu %%\n", pwm_duty);
        pwm_reg = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * pwm_duty / 100; // 最终的占空比值

        //     // printf("pwm_duty :%u\n", pwm_duty);
        TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
        TMR1_PWML = pwm_reg & 0xFF;
    }
}
