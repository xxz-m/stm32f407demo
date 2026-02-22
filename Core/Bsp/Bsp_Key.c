#include "Bsp_Key.h"
#include "core_main_config.h"

/* 
 * 模块描述: 按键驱动库 (BSP_Key)
 * 依赖硬件: KEY1(PE4), KEY2(PE5), KEY3(PE7), KEY4(PE8)
 * 依赖资源: 外部 10ms 定时器中断调用 BSP_Key_Scan_10ms()
 * 逻辑说明: 
 *   - 采用状态机或计数器消抖
 *   - 假设按键低电平有效 (Active Low)
 */

/* 全局变量：当前按下的按键值 */
volatile Key_e g_current_key = KEY_NONE;

/* 内部消抖计数器 */
static uint8_t key_cnt[KEY_MAX] = {0};
/* 内部按键状态 */
static KeyState_e key_state[KEY_MAX] = {KEY_UP};

/* 消抖阈值 (10ms * 2 = 20ms) */
#define KEY_DEBOUNCE_THRESHOLD  2

/**
  * @brief  按键 GPIO 初始化
  * @note   配置 PE4, PE5, PE7, PE8 为输入模式，带上拉
  */
void BSP_Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Port E Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /* Configure GPIO pins : KEY1_PIN KEY2_PIN KEY3_PIN KEY4_PIN */
    GPIO_InitStruct.Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN | KEY4_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; // 假设低电平有效
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/**
  * @brief  按键扫描函数 (需在 10ms 定时器中断中调用)
  * @note   实现简单的消抖逻辑
  */
void BSP_Key_Scan_10ms(void)
{
    /* 读取各按键物理电平 (低电平表示按下) */
    uint8_t raw_state[KEY_MAX];
    
    raw_state[KEY_1] = (HAL_GPIO_ReadPin(KEY1_PORT, KEY1_PIN) == GPIO_PIN_RESET) ? 1 : 0;
    raw_state[KEY_2] = (HAL_GPIO_ReadPin(KEY2_PORT, KEY2_PIN) == GPIO_PIN_RESET) ? 1 : 0;
    raw_state[KEY_3] = (HAL_GPIO_ReadPin(KEY3_PORT, KEY3_PIN) == GPIO_PIN_RESET) ? 1 : 0;
    raw_state[KEY_4] = (HAL_GPIO_ReadPin(KEY4_PORT, KEY4_PIN) == GPIO_PIN_RESET) ? 1 : 0;

    /* 遍历处理每个按键 */
    for (int i = KEY_1; i < KEY_MAX; i++)
    {
        if (raw_state[i]) // 物理按下
        {
            if (key_cnt[i] < KEY_DEBOUNCE_THRESHOLD)
            {
                key_cnt[i]++;
            }
            else // 超过阈值，确认按下
            {
                if (key_state[i] == KEY_UP)
                {
                    key_state[i] = KEY_DOWN;
                    g_current_key = (Key_e)i; // 更新全局按键值 (简单处理：后按覆盖)
                }
            }
        }
        else // 物理弹起
        {
            key_cnt[i] = 0; // 计数清零
            if (key_state[i] == KEY_DOWN)
            {
                key_state[i] = KEY_UP;
                if (g_current_key == (Key_e)i)
                {
                    g_current_key = KEY_NONE; // 释放
                }
            }
        }
    }
}

/**
  * @brief  查询指定按键是否按下
  * @param  key: 按键枚举值 (KEY_1 ~ KEY_4)
  * @retval 1: 按下, 0: 未按下
  */
uint8_t BSP_Key_IsPressed(Key_e key)
{
    if (key <= KEY_NONE || key >= KEY_MAX) return 0;
    return (key_state[key] == KEY_DOWN);
}
