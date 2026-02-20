/**
 * @file    Bsp_Led.c
 * @brief   LED 驱动模块实现文件
 * @date    2026-02-20
 * @note    LED1: PD14, LED2: PD15, LED3: PC9, LED4: PC8
 *          假设低电平点亮 (Common Anode)
 */
#include "Bsp_Led.h"
#include "core_main_config.h"

// LED 端口和引脚数组，方便索引 (顺序对应 LED_Index_t)
static GPIO_TypeDef* LED_Ports[] = {LED1_PORT, LED2_PORT, LED3_PORT, LED4_PORT};
static uint16_t      LED_Pins[]  = {LED1_PIN,  LED2_PIN,  LED3_PIN,  LED4_PIN};

/* 假设低电平点亮，如果实际是高电平点亮，请修改此处宏定义 */
#define LED_ON_LEVEL  GPIO_PIN_RESET
#define LED_OFF_LEVEL GPIO_PIN_SET

/**
 * @brief 初始化 LED 状态 (默认熄灭)
 */
void BSP_LED_Init(void)
{
    // GPIO 时钟与模式初始化由 CubeMX 在 main.c 中完成
    // 此处仅设置初始电平为熄灭
    for(int i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(LED_Ports[i], LED_Pins[i], LED_OFF_LEVEL);
    }
}

/**
 * @brief 点亮指定 LED
 * @param led LED 索引
 */
void BSP_LED_On(LED_Index_t led)
{
    if(led <= LED_4) {
        HAL_GPIO_WritePin(LED_Ports[led], LED_Pins[led], LED_ON_LEVEL);
    }
}

/**
 * @brief 熄灭指定 LED
 * @param led LED 索引
 */
void BSP_LED_Off(LED_Index_t led)
{
    if(led <= LED_4) {
        HAL_GPIO_WritePin(LED_Ports[led], LED_Pins[led], LED_OFF_LEVEL);
    }
}

/**
 * @brief 翻转指定 LED
 * @param led LED 索引
 */
void BSP_LED_Toggle(LED_Index_t led)
{
    if(led <= LED_4) {
        HAL_GPIO_TogglePin(LED_Ports[led], LED_Pins[led]);
    }
}
