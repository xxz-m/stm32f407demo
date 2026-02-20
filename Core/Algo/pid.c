/**
 * @file    pid.c
 * @brief   PID 控制算法实现
 * @date    2026-02-19
 */

#include "pid.h"

/**
 * @brief 初始化 PID 控制器
 */
void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd, float max_out, float max_int)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    
    pid->max_output = max_out;
    pid->max_integral = max_int;
    
    PID_Reset(pid);
}

/**
 * @brief 重置 PID 控制器状态
 */
void PID_Reset(PID_Controller_t *pid)
{
    pid->target = 0;
    pid->actual = 0;
    pid->error = 0;
    pid->prev_error = 0;
    pid->integral = 0;
    pid->output = 0;
}

/**
 * @brief 计算 PID 输出
 * @note  这是一个位置式 PID 实现
 */
float PID_Compute(PID_Controller_t *pid, float target, float actual)
{
    pid->target = target;
    pid->actual = actual;
    
    /* 计算误差 */
    pid->error = pid->target - pid->actual;
    
    /* 积分项 (带限幅) */
    pid->integral += pid->error;
    if (pid->integral > pid->max_integral) {
        pid->integral = pid->max_integral;
    } else if (pid->integral < -pid->max_integral) {
        pid->integral = -pid->max_integral;
    }
    
    /* 微分项 */
    float derivative = pid->error - pid->prev_error;
    
    /* 计算输出 */
    pid->output = (pid->Kp * pid->error) + 
                  (pid->Ki * pid->integral) + 
                  (pid->Kd * derivative);
    
    /* 输出限幅 */
    if (pid->output > pid->max_output) {
        pid->output = pid->max_output;
    } else if (pid->output < -pid->max_output) {
        pid->output = -pid->max_output;
    }
    
    /* 更新历史误差 */
    pid->prev_error = pid->error;
    
    return pid->output;
}
