/**
 * @file    pid.h
 * @brief   PID 控制算法头文件 (PID Control Algorithm Header)
 * @date    2026-02-19
 */

#ifndef __PID_H
#define __PID_H

#include <stdint.h>

/**
 * @brief PID 控制器结构体 (PID Controller Structure)
 */
typedef struct {
    float Kp;           // 比例系数 (Proportional Gain)
    float Ki;           // 积分系数 (Integral Gain)
    float Kd;           // 微分系数 (Derivative Gain)

    float target;       // 目标值 (Target Value)
    float actual;       // 实际值 (Actual Value)
    float error;        // 当前误差 (Current Error)
    float prev_error;   // 上一次误差 (Previous Error)
    float integral;     // 积分项 (Integral Term)
    
    float output;       // PID 输出 (PID Output)
    float max_output;   // 输出限幅 (Output Limit)
    float max_integral; // 积分限幅 (Integral Limit)
} PID_Controller_t;

/**
 * @brief 初始化 PID 控制器 (Initialize PID Controller)
 * @param pid PID 对象指针
 * @param kp  比例系数
 * @param ki  积分系数
 * @param kd  微分系数
 * @param max_out 输出限幅
 * @param max_int 积分限幅
 */
void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd, float max_out, float max_int);

/**
 * @brief 重置 PID 控制器状态 (Reset PID Controller State)
 * @param pid PID 对象指针
 */
void PID_Reset(PID_Controller_t *pid);

/**
 * @brief 计算 PID 输出 (Compute PID Output)
 * @param pid    PID 对象指针
 * @param target 目标值
 * @param actual 实际值
 * @return float PID 输出值
 */
float PID_Compute(PID_Controller_t *pid, float target, float actual);

/* 应用层跟随控制接口 (Application Layer Follow Control Interface) */
void App_Follow_Init(void);
void App_Follow_Control_Loop(void);
void App_Follow_Reset_Loss_Counter(void);
void App_Follow_Update_PID_Params(void);

#endif /* __PID_H */
