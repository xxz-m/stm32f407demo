/**
 * @file    app_comm.h
 * @brief   ESP8266 WiFi 通信模块头文件
 * @date    2026-02-19
 */

#ifndef __APP_COMM_H
#define __APP_COMM_H

#include <stdint.h>
#include <stdbool.h>

/* 控制指令枚举 */
typedef enum {
    CMD_STOP = 0,
    CMD_FORWARD,
    CMD_BACKWARD,
    CMD_LEFT,
    CMD_RIGHT
} Robot_Command_t;

/* 运行模式枚举 */
typedef enum {
    MODE_MANUAL = 0,
    MODE_AUTO
} Robot_Mode_t;

/* 导出变量（只读） */
extern volatile Robot_Command_t g_remote_cmd;
extern volatile Robot_Mode_t    g_robot_mode;

/* 函数声明 */
void App_Comm_Init(void);
void App_Comm_ProcessTask(void);

#endif /* __APP_COMM_H */
