/**
 * @file    Bsp_Led.h
 * @brief   LED 驱动头文件 (LED Driver Header)
 * @date    2026-02-20
 */
#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "main.h"

// LED 索引枚举 (LED Index)
typedef enum {
    LED_1 = 0, // PD14
    LED_2,     // PD15
    LED_3,     // PC9
    LED_4      // PC8
} LED_Index_t;

/**
 * @brief 初始化 LED (Initialize LED)
 */
void BSP_LED_Init(void);

/**
 * @brief 点亮 LED (Turn ON LED)
 * @param led LED 索引 (LED Index)
 */
void BSP_LED_On(LED_Index_t led);

/**
 * @brief 熄灭 LED (Turn OFF LED)
 * @param led LED 索引 (LED Index)
 */
void BSP_LED_Off(LED_Index_t led);

/**
 * @brief 翻转 LED (Toggle LED)
 * @param led LED 索引 (LED Index)
 */
void BSP_LED_Toggle(LED_Index_t led);

#endif /* __BSP_LED_H */
