#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H

#include "main.h"

// �������������壨���������ʵ������޸ģ�
#define ENCODER_PPR          11     // ������ÿת������ (Pulse Per Revolution)
#define MOTOR_REDUCTION_RATIO 50    // ������ٱ�
#define SAMPLE_TIME_S        0.05f  // 速度采样时间 (50ms)

typedef struct {
    TIM_HandleTypeDef *htim; // 指向定时器的句柄
    int32_t last_count;      // 上次计数值
    float speed_rpm;         // 转速（转/分钟, RPM）
} Encoder_t;

extern Encoder_t motor1;
extern Encoder_t motor2;

// 函数声明
void Encoder_Init(void);
void Encoder_Update_Speed(Encoder_t *m1, Encoder_t *m2);

#endif
