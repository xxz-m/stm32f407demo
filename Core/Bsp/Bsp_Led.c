/**
 * @file    Bsp_Led.c
 * @brief   LED 驱动程序 (LED Driver)
 * @date    2026-02-20
 * @note    LED1: PD14, LED2: PD15, LED3: PC9, LED4: PC8
 *          共阳极 (Common Anode)
 */
#include "Bsp_Led.h"
#include "core_main_config.h"

// LED 端口/引脚数组映射 (对应 LED_Index_t)
static GPIO_TypeDef* LED_Ports[] = {LED1_PORT, LED2_PORT, LED3_PORT, LED4_PORT};
static uint16_t      LED_Pins[]  = {LED1_PIN,  LED2_PIN,  LED3_PIN,  LED4_PIN};

/* LED 电平定义 (共阳极: 低电平亮, 高电平灭) */
#define LED_ON_LEVEL  GPIO_PIN_RESET
#define LED_OFF_LEVEL GPIO_PIN_SET

/**
 * @brief 初始化 LED 状态 (默认关闭)
 */
void BSP_LED_Init(void)
{
    // GPIO 初始化已在 CubeMX 的 main.c 中完成
    // 此处仅设置默认状态为关闭
    for(int i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(LED_Ports[i], LED_Pins[i], LED_OFF_LEVEL);
    }
}

/**
 * @brief 点亮 LED
 * @param led LED 索引
 */
void BSP_LED_On(LED_Index_t led)
{
    if(led <= LED_4) {
        HAL_GPIO_WritePin(LED_Ports[led], LED_Pins[led], LED_ON_LEVEL);
    }
}

/**
 * @brief 熄灭 LED
 * @param led LED 索引
 */
void BSP_LED_Off(LED_Index_t led)
{
    if(led <= LED_4) {
        HAL_GPIO_WritePin(LED_Ports[led], LED_Pins[led], LED_OFF_LEVEL);
    }
}

/**
 * @brief 翻转 LED 状态
 * @param led LED 索引
 */
void BSP_LED_Toggle(LED_Index_t led)
{
    if(led <= LED_4) {
        HAL_GPIO_TogglePin(LED_Ports[led], LED_Pins[led]);
    }
}
