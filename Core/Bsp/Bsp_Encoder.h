#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H

#include "main.h"

// 编码器参数定义（请根据您的实际情况修改）
#define ENCODER_PPR          11     // 编码器每转脉冲数 (Pulse Per Revolution)
#define MOTOR_REDUCTION_RATIO 50    // 电机减速比
#define SAMPLE_TIME_S        0.05f  // 速度采样时间 (50ms)

typedef struct {
    TIM_HandleTypeDef *htim; // 指向定时器的句柄
    int32_t last_count;      // 上次计数值
    float speed_rpm;         // 转速（转/分钟, RPM）
} Encoder_t;

// 函数声明
void Encoder_Init(void);
void Encoder_Update_Speed(Encoder_t *m1, Encoder_t *m2);

#endif
