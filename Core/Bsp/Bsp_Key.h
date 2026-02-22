#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "main.h"

/* 按键枚举 */
typedef enum {
    KEY_NONE = 0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_MAX
} Key_e;

/* 按键状态 */
typedef enum {
    KEY_UP = 0,     // 弹起
    KEY_DOWN        // 按下
} KeyState_e;

/* 全局变量：当前按下的按键值 */
extern volatile Key_e g_current_key;

/* 函数声明 */
void BSP_Key_Init(void);
void BSP_Key_Scan_10ms(void);
uint8_t BSP_Key_IsPressed(Key_e key);

#endif /* __BSP_KEY_H */
