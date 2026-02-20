/**
 * @file    app_ui.c
 * @brief   用户界面显示模块实现文件
 * @date    2026-02-19
 */

#include "app_ui.h"
#include "app_comm.h"
#include <stdio.h>

/**
 * @brief UI 模块初始化
 * @note  初始化 U8g2 显示屏
 */
void App_UI_Init(void)
{
    // 初始化 U8g2
    U8G2_Init();
    
    // 设置字体
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
}

/**
 * @brief 更新 UI 显示 (电机模式)
 */
void App_UI_Update(Encoder_t *m1, Encoder_t *m2)
{
    char buf[32];

    u8g2_ClearBuffer(&u8g2);
    
    // 绘制标题
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    u8g2_DrawStr(&u8g2, 0, 12, "Motor Speed");
    
    // 绘制分割线
    u8g2_DrawHLine(&u8g2, 0, 14, 128);

    // 设置内容字体
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);

    // 显示左电机速度
    snprintf(buf, sizeof(buf), "L: %.1f RPM", m1->speed_rpm);
    u8g2_DrawStr(&u8g2, 0, 30, buf);

    // 显示右电机速度
    snprintf(buf, sizeof(buf), "R: %.1f RPM", m2->speed_rpm);
    u8g2_DrawStr(&u8g2, 0, 45, buf);
    
    // 显示当前模式和指令
    const char *mode_str = (g_robot_mode == MODE_AUTO) ? "AUTO" : "MANUAL";
    const char *cmd_str = "";
    if (g_robot_mode == MODE_MANUAL) {
        switch (g_remote_cmd) {
            case CMD_FORWARD: cmd_str = "FWD"; break;
            case CMD_BACKWARD: cmd_str = "BWD"; break;
            case CMD_LEFT: cmd_str = "LEFT"; break;
            case CMD_RIGHT: cmd_str = "RIGHT"; break;
            case CMD_STOP: cmd_str = "STOP"; break;
            default: cmd_str = "---"; break;
        }
    }
    snprintf(buf, sizeof(buf), "[%s] %s", mode_str, cmd_str);
    u8g2_DrawStr(&u8g2, 0, 60, buf);
    
    // 发送缓冲区内容到屏幕
    u8g2_SendBuffer(&u8g2);
}

/**
 * @brief 显示 GPS 数据
 */
void App_UI_ShowGPS(nmea_msg *gps)
{
    char buf[32];

    u8g2_ClearBuffer(&u8g2);
    
    // 绘制标题
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    u8g2_DrawStr(&u8g2, 0, 12, "GPS Status");
    
    // 绘制分割线
    u8g2_DrawHLine(&u8g2, 0, 14, 128);

    // 设置内容字体
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);

    // 显示定位状态
    const char *fix_str = (gps->fixmode > 1) ? "FIXED" : "NO FIX";
    snprintf(buf, sizeof(buf), "State: %s (%d)", fix_str, gps->posslnum);
    u8g2_DrawStr(&u8g2, 0, 28, buf);

    // 显示纬度
    // 格式: 30.12345 N
    snprintf(buf, sizeof(buf), "Lat: %.5f %c", gps->latitude, gps->nshemi ? gps->nshemi : '-');
    u8g2_DrawStr(&u8g2, 0, 42, buf);

    // 显示经度
    // 格式: 120.12345 E
    snprintf(buf, sizeof(buf), "Lon: %.5f %c", gps->longitude, gps->ewhemi ? gps->ewhemi : '-');
    u8g2_DrawStr(&u8g2, 0, 56, buf);
    
    // 发送缓冲区内容到屏幕
    u8g2_SendBuffer(&u8g2);
}
