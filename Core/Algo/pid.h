/**
 * @file    pid.h
 * @brief   PID 控制算法头文件
 * @date    2026-02-19
 */

#ifndef __PID_H
#define __PID_H

#include <stdint.h>

/**
 * @brief PID 结构体定义
 */
typedef struct {
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float Kd;           // 微分系数

    float target;       // 目标值
    float actual;       // 实际值
    float error;        // 当前误差
    float prev_error;   // 上一次误差
    float integral;     // 积分累计值
    
    float output;       // PID 输出值
    float max_output;   // 输出限幅
    float max_integral; // 积分限幅
} PID_Controller_t;

/**
 * @brief 初始化 PID 控制器
 * @param pid PID 对象指针
 * @param kp  比例系数
 * @param ki  积分系数
 * @param kd  微分系数
 * @param max_out 输出限幅
 * @param max_int 积分限幅
 */
void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd, float max_out, float max_int);

/**
 * @brief 重置 PID 控制器状态
 * @param pid PID 对象指针
 */
void PID_Reset(PID_Controller_t *pid);

/**
 * @brief 计算 PID 输出
 * @param pid    PID 对象指针
 * @param target 目标值
 * @param actual 实际值
 * @return float PID 输出结果
 */
float PID_Compute(PID_Controller_t *pid, float target, float actual);

#endif /* __PID_H */
