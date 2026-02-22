#ifndef __BSP_OPENMV_H__
#define __BSP_OPENMV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* OpenMV 数据结构 (OpenMV Data Structure) */
typedef struct {
    uint8_t x;           // X 坐标 (0-160)
    uint8_t dist;        // 距离 (cm)
    // uint8_t update_flag; // 更新标志 (Update flag)
} OpenMV_Data_t;

/* FIFO 缓冲区定义 (FIFO Buffer Definition) */
#define OPENMV_FIFO_SIZE 10 // 存储最近 10 个数据包
typedef struct {
    OpenMV_Data_t buffer[OPENMV_FIFO_SIZE];
    volatile uint8_t head; // 写位置 (Write pos)
    volatile uint8_t tail; // 读位置 (Read pos)
    volatile uint8_t count; // 数据计数 (Data count)
} OpenMV_FIFO_t;

extern OpenMV_Data_t openmv_data;
extern OpenMV_FIFO_t openmv_fifo;

/* 函数原型 (Function Prototypes) */
void OpenMV_Init(UART_HandleTypeDef *huart);
void OpenMV_Rx_Callback(void); // 在 USART6 中断中调用
void OpenMV_Parse_Callback(void); // 在 IDLE 中断中调用
void OpenMV_Print_Task(void *arg); // OS 任务

#ifdef __cplusplus
}
#endif
#endif /* __BSP_OPENMV_H__ */
