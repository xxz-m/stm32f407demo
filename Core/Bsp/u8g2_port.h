#ifndef __U8G2_PORT_H
#define __U8G2_PORT_H

#include "u8g2.h"
#include "stm32f4xx_hal.h"

// U8g2 实例，设为 extern 以便在其他文件中调用
extern u8g2_t u8g2;

/**
 * @brief 初始化 U8G2 显示屏
 * @note  此函数会配置 U8G2 库并初始化显示。
 */
void U8G2_Init(void);

#endif /* __U8G2_PORT_H */
