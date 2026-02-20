#ifndef __DWT_DELAY_H
#define __DWT_DELAY_H

#include "stm32f4xx_hal.h"

/**
 * @brief 初始化 DWT 延时组件
 * @note  必须在系统时钟配置完成后，主循环开始前调用一次。
 */
void delay_init(void);

/**
 * @brief 微秒级延时
 * @param us 要延时的微秒数
 * @note   此函数为忙等待延时，会占用CPU。
 *         最大延时值取决于 us * (SystemCoreClock / 1000000) 是否会溢出32位整数。
 *         在168MHz主频下，最大延时约为 25 秒。
 */
void delay_us(uint32_t us);

#endif /* __DWT_DELAY_H */
