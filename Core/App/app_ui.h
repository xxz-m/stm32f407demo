/**
 * @file    app_ui.h
 * @brief   用户界面显示模块头文件
 * @date    2026-02-19
 */

#ifndef __APP_UI_H
#define __APP_UI_H

#include "main.h"
#include "u8g2_port.h"
#include "Bsp_Encoder.h"
#include "Bsp_GPS.h"

/**
 * @brief UI 模块初始化
 */
void App_UI_Init(void);

/**
 * @brief 更新 UI 显示 (电机速度模式)
 * @param m1 左电机编码器对象
 * @param m2 右电机编码器对象
 */
void App_UI_Update(Encoder_t *m1, Encoder_t *m2);

/**
 * @brief 显示 GPS 数据 (调试模式)
 * @param gps GPS数据对象指针
 */
void App_UI_ShowGPS(nmea_msg *gps);

#endif /* __APP_UI_H */
