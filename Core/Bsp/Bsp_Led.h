/**
 * @file    Bsp_Led.h
 * @brief   LED 驱动模块头文件
 * @date    2026-02-20
 */
#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "main.h"

// LED 枚举
typedef enum {
    LED_1 = 0, // PD14
    LED_2,     // PD15
    LED_3,     // PC9
    LED_4      // PC8
} LED_Index_t;

/**
 * @brief 初始化 LED 状态
 */
void BSP_LED_Init(void);

/**
 * @brief 点亮指定 LED
 * @param led LED 索引
 */
void BSP_LED_On(LED_Index_t led);

/**
 * @brief 熄灭指定 LED
 * @param led LED 索引
 */
void BSP_LED_Off(LED_Index_t led);

/**
 * @brief 翻转指定 LED
 * @param led LED 索引
 */
void BSP_LED_Toggle(LED_Index_t led);

#endif /* __BSP_LED_H */
