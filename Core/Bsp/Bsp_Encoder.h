#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H

#include "main.h"

// 电机参数宏定义（根据你的实际电机修改哦）
#define ENCODER_PPR          11     // 编码器单圈脉冲数 (Pulse Per Revolution)
#define MOTOR_REDUCTION_RATIO 50    // 电机减速比
#define SAMPLE_TIME_S        0.05f  // 采样周期 (50ms)

typedef struct {
    TIM_HandleTypeDef *htim; // 指向定时器的句柄
    int32_t last_count;      // 上次计数值
    float speed_rpm;         // 转每分钟 (RPM)
} Encoder_t;

// 函数声明
void Encoder_Init(void);
void Encoder_Update_Speed(Encoder_t *m1, Encoder_t *m2);

#endif
