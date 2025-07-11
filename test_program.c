#include "user_config.h"

{
    u16 current = 0; // 充电电流，单位：mA

    /*
        检测电流引脚检测到的电压 == ad值 / 4096 * 参考电压
        current_adc_val * 3 / 4096

        检测电流的引脚检测到的充电电流 == 检测电流引脚检测到的电压 / 110 / 0.005R，
        逐步换成单片机可以计算的形式：
        current_adc_val * 3 / 4096 / 110 / 0.005
        current_adc_val * 3 / 4096 / 110 * 1000 / 5
        current_adc_val * 3 * 1000 / 5 / 4096 / 110
        得到的是以A为单位的电流，需要再转换成以mA为单位：
        current_adc_val * 3 * 1000 * 1000 / 5 / 4096 / 110，计算会溢出，需要再化简
        (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110
    */
    current_adc_val = 4095;
    current = (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110;
    printf("current1 %umA\n", current); // 打印出来是 5453mA

    current_adc_val = 546;
    current = (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110;
    printf("current2 %umA\n", current); // 打印出来是 727mA
}