#include "adc.h"

volatile u16 adc_val = 0;

void adc_config(void)
{
    // P1_MD0 |= GPIO_P13_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P16_MODE_SEL(0x03);

    P2_MD0 |= GPIO_P22_MODE_SEL(0x03);

    // P2_MD1 |= GPIO_P23_MODE_SEL(0x3); // 模拟模式
    // P2_MD1 |= GPIO_P24_MODE_SEL(0x3); // 模拟模式

    // P1_MD1 |= GPIO_P16_MODE_SEL(0x03); // 模拟模式
    // P3_MD0 |= GPIO_P31_MODE_SEL(0x3);

    ADC_CFG1 |= (0x0F << 3); // ADC时钟分频为16分频，为系统时钟/16
    ADC_CFG2 = 0xFF;         // 通道0采样时间配置为256个采样时钟周期

    ADC_ACON1 &= ~((0x01 << 5) |   /* 关闭ADC外部参考选择信号 */
                   (0x07 << 0));   /* 清空ADC内部参考电压的选择配置 */
    ADC_ACON1 |= (0x01 << 6) |     /* 使能adc内部参考 */
                 (0x03 << 3) |     /* 关闭测试信号 */
                 (0x01 << 0);      /* 选择 内部 2.0V 作为参考电压 */
    ADC_ACON0 = ADC_CMP_EN(0x1) |  // 打开ADC中的CMP使能信号
                ADC_BIAS_EN(0x1) | // 打开ADC偏置电流能使信号
                ADC_BIAS_SEL(0x1); // 偏置电流选择：1x
    ADC_TRGS0 |= (0x07 << 4);      // 通道 0DLY 的 ADC 时钟个数选择，配置为 4n+1，4 * 29 + 1
    ADC_CHS0 |= (0x01 << 6);       // 使能 通道 0DLY 功能
}

void adc_sel_pin(u8 adc_pin)
{
    ADC_CFG0 &= ~((0x01 << 6) | (0x01 << 3)); // 关闭adc，不使能通道0转换
    ADC_CHS0 &= ~((0x01 << 7) |               /* 数据格式左对齐 */
                  (0x01 << 5) |               // 选择内部通道
                  (0x01 << 4) |
                  (0x01 << 3) |
                  (0x01 << 2) |
                  (0x01 << 1) |
                  (0x01 << 0)); // 选择外部通路，清空选择的adc0通路

    if (ADC_PIN_DETECT_CHARGE == adc_pin)
    {
        //  ADC_CHS0 |= (0x0B << 0); // P13 对应的模拟通道
        ADC_CHS0 |= (0x0E << 0); // P16 对应的模拟通道
        // ADC_CHS0 |= (0x14 << 0); // P24 对应的模拟通道
        // ADC_CHS0 = ADC_ANALOG_CHAN(0x19) | // 选则引脚对应的通道（0x19--P31）
        //            ADC_EXT_SEL(0x0);       // 选择外部通道
    }
    else if (ADC_PIN_DETECT_BATTERY == adc_pin)
    {
        ADC_CHS0 |= (0x12 << 0); // P22 对应的模拟通道
        // ADC_CHS0 |= (0x13 << 0); // P23 对应的模拟通道
    }

    ADC_CFG0 |= ADC_CHAN0_EN(0x1) | // 使能通道0转换
                ADC_EN(0x1);        // 使能A/D转换
    delay_ms(1);                    // 等待adc稳定
}

// adc采集+滤波
u16 adc_getval(void)
{
    u8 i = 0; // adc采集次数的计数
    volatile u16 g_temp_value = 0;
    volatile u32 g_tmpbuff = 0;
    volatile u16 g_adcmax = 0;
    volatile u16 g_adcmin = 0xFFFF;

    // 采集20次，去掉前两次采样，再去掉一个最大值和一个最小值，再取平均值
    for (i = 0; i < 20; i++)
    {
        ADC_CFG0 |= ADC_CHAN0_TRG(0x1); // 触发ADC0转换
        while (!(ADC_STA & ADC_CHAN0_DONE(0x1)))
            ;                                                 // 等待转换完成
        g_temp_value = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4); // 读取 channel0 的值
        ADC_STA = ADC_CHAN0_DONE(0x1);                        // 清除ADC0转换完成标志位

        if (i < 2)
            continue; // 丢弃前两次采样的
        if (g_temp_value > g_adcmax)
            g_adcmax = g_temp_value; // 最大
        if (g_temp_value < g_adcmin)
            g_adcmin = g_temp_value; // 最小

        g_tmpbuff += g_temp_value;
    }

    g_tmpbuff -= g_adcmax;           // 去掉一个最大
    g_tmpbuff -= g_adcmin;           // 去掉一个最小
    g_temp_value = (g_tmpbuff >> 4); // 除以16，取平均值

    return g_temp_value;
}
