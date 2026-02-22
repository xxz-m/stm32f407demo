#include "bsp_encoder.h"
#include "tim.h"
#include "stdio.h"
// 定义两个电机实例
Encoder_t motor1 = {&htim3, 0, 0.0f};
Encoder_t motor2 = {&htim5, 0, 0.0f};

/**
 * @brief 初始化编码器并开启定时器
 */
void Encoder_Init(void) {
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);
    // HAL_TIM_Base_Start_IT(&htim14); // 移到 Core_Main_Init 中统一启动
}

/**
 * @brief 在定时器中断里调用，更新速度
 * @note 4倍频下，一圈的总脉冲 = PPR * 4 * 减速比
 */
void Encoder_Update_Speed(Encoder_t *m1, Encoder_t *m2) {
    // 1. 获取当前计数值并计算增量（利用16位溢出特性）
    int16_t cnt1 = (int16_t)__HAL_TIM_GET_COUNTER(m1->htim);
    int16_t cnt2 = (int16_t)__HAL_TIM_GET_COUNTER(m2->htim);

    // 2. 计算 RPM = (脉冲数 / (单圈脉冲 * 4 * 减速比)) / 时间(s) * 60
    /* 注意：分母为 (11 * 4 * 50 * 0.01) = 22.0 */
    /* 如果转速很低，10ms 内可能只有1-2个脉冲，cnt=1 -> RPM = 60/22 ≈ 2.7 RPM */
    /* 如果转速为0，cnt=0 */
    // float common_factor = (ENCODER_PPR * 4.0f * MOTOR_REDUCTION_RATIO * SAMPLE_TIME_S);
    
    // Debug: 直接输出 cnt1/cnt2 看是否有变化 (调试时取消注释)
    // printf("Encoder: cnt1=%d, cnt2=%d\r\n", cnt1, cnt2);

    /* 增加速度为0的判断逻辑（防止抖动或极低速时的噪声） */
    if (cnt1 == 0) m1->speed_rpm = 0.0f;
    else m1->speed_rpm = -(float)cnt1 * 60.0f / (ENCODER_PPR * 4.0f * MOTOR_REDUCTION_RATIO * SAMPLE_TIME_S);

    if (cnt2 == 0) m2->speed_rpm = 0.0f;
    else m2->speed_rpm = (float)cnt2 * 60.0f / (ENCODER_PPR * 4.0f * MOTOR_REDUCTION_RATIO * SAMPLE_TIME_S);

    // 3. 计数值清零，为下一次采样准备
    __HAL_TIM_SET_COUNTER(m1->htim, 0);
    __HAL_TIM_SET_COUNTER(m2->htim, 0);
}
