/**
 * @file    DWT_delay.c
 * @brief   使用 Cortex-M 内核的 DWT 单元实现微秒级延时
 * @author  朱治豪
 * @date    2026-02-19
 * @note    适用于 STM32F4 系列及其他 Cortex-M3/M4/M7 内核的 MCU
 */

#include "DWT_delay.h"

/**
 * @brief  初始化 DWT 单元
 * @note   使用 DWT 外设进行精确延时前必须调用此函数。
 *         该函数会：
 *         1. 使能 DWT 和 ITM 的跟踪功能。
 *         2. 复位 DWT 的周期计数器 CYCCNT。
 *         3. 启动 CYCCNT 计数器。
 * @param  None
 * @retval None
 */
void delay_init(void)
{
    // 使能 DWT 外设的跟踪功能 (Trace function)
    // DEMCR: Debug Exception and Monitor Control Register
    // TRCENA: Trace enable bit. Must be set to 1 to enable DWT and ITM.
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // 清零 DWT 周期计数器
    DWT->CYCCNT = 0;

    // 使能 DWT 周期计数器
    // CTRL: Control register
    // CYCCNTENA: Cycle count enable bit
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief  微秒级延时
 * @param  us: 要延时的微秒数
 * @note   此函数依赖于 delay_init() 的正确初始化。
 *         SystemCoreClock 变量必须正确配置为当前的系统主频。
 * @retval None
 */
void delay_us(uint32_t us)
{
    // 获取进入函数时的 DWT 计数值
    uint32_t start_tick = DWT->CYCCNT;
    
    // 计算延时所需的 CPU 周期数
    // SystemCoreClock 是 HAL 库中定义的全局变量，表示核心时钟频率 (Hz)
    // 在 168MHz 下，ticks_per_us = 168
    uint32_t ticks_per_us = (SystemCoreClock / 1000000);
    uint32_t delay_ticks = us * ticks_per_us;

    // 忙等待，直到 DWT 计数器经过了指定的周期数
    while ((DWT->CYCCNT - start_tick) < delay_ticks);
}

