/**
 * @file    app_comm.c
 * @brief   ESP8266 WiFi 通信模块实现
 * @note    使用 UART2 中断 (IT) 接收不定长数据
 *          - 中断回调负责将数据复制到处理缓冲区
 *          - 主任务负责解析处理缓冲区的数据
 * @date    2026-02-19
 */

#include "app_comm.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

/* 配置项 */
#define WIFI_UART       huart2
#define RX_BUFFER_SIZE  128

/* 全局变量定义 */
volatile Robot_Command_t g_remote_cmd = CMD_STOP;
volatile Robot_Mode_t    g_robot_mode = MODE_MANUAL;

/* 私有变量 */
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint8_t process_buffer[RX_BUFFER_SIZE];
static volatile uint16_t process_len = 0;
static volatile uint8_t  process_ready = 0;

/**
 * @brief 通信模块初始化
 */
void App_Comm_Init(void)
{
    /* 启动 UART 接收 (中断模式) */
    /* 使用 HAL_UARTEx_ReceiveToIdle_IT 实现不定长接收 */
    HAL_UARTEx_ReceiveToIdle_IT(&WIFI_UART, rx_buffer, RX_BUFFER_SIZE);
}

/**
 * @brief 内部解析逻辑 (非公开)
 * @param data 数据指针
 * @param len  数据长度
 */
static void App_Comm_Parse_Internal(char *data, uint16_t len)
{
    /* 确保字符串以 null 结尾 (防溢出) */
    if (len >= RX_BUFFER_SIZE) len = RX_BUFFER_SIZE - 1;
    data[len] = '\0';

    /* 解析指令: MOVE:F, MOVE:B, ... */
    if (strncmp(data, "MOVE:", 5) == 0) {
        char cmd = data[5];
        switch (cmd) {
            case 'F': g_remote_cmd = CMD_FORWARD;  break;
            case 'B': g_remote_cmd = CMD_BACKWARD; break;
            case 'L': g_remote_cmd = CMD_LEFT;     break;
            case 'R': g_remote_cmd = CMD_RIGHT;    break;
            case 'S': g_remote_cmd = CMD_STOP;     break;
            default:  break; // 保持上一次状态或忽略
        }
    }
    /* 解析模式: MODE:AUTO, MODE:MANUAL */
    else if (strncmp(data, "MODE:", 5) == 0) {
        if (strncmp(&data[5], "AUTO", 4) == 0) {
            g_robot_mode = MODE_AUTO;
        } else if (strncmp(&data[5], "MANUAL", 6) == 0) {
            g_robot_mode = MODE_MANUAL;
            g_remote_cmd = CMD_STOP; // 切换回手动时先停止
        }
    }
}

/**
 * @brief 处理任务 (在主循环中调用)
 */
void App_Comm_ProcessTask(void)
{
    if (process_ready) {
        /* 处理数据 */
        App_Comm_Parse_Internal((char *)process_buffer, process_len);
        
        /* 清除标志 */
        process_ready = 0;
    }
}

/**
 * @brief HAL 库 UART 接收事件回调 (空闲中断/半满/全满)
 * @note  ISR 上下文，只做数据搬运
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == WIFI_UART.Instance) {
        /* 将数据复制到处理缓冲区 (如果上一次还没处理完，会覆盖，保证实时性) */
        if (Size > RX_BUFFER_SIZE) Size = RX_BUFFER_SIZE;
        memcpy(process_buffer, rx_buffer, Size);
        process_len = Size;
        process_ready = 1;

        /* 重新启动接收 (中断模式) */
        HAL_UARTEx_ReceiveToIdle_IT(&WIFI_UART, rx_buffer, RX_BUFFER_SIZE);
    }
}

/* 错误回调 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == WIFI_UART.Instance) {
        /* 发生错误时尝试重启接收 */
        HAL_UARTEx_ReceiveToIdle_IT(&WIFI_UART, rx_buffer, RX_BUFFER_SIZE);
    }
}
