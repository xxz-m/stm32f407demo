/**
 * @file    pid.c
 * @brief   PID 控制算法实现
 * @date    2026-02-19
 */

#include "pid.h"
#include "bsp_openmv.h"
#include "bsp_encoder.h"
#include "bsp_tb6612.h"
#include "app_comm.h"
#include "../Bsp/Bsp_Flash.h"

#include <stdio.h> // Ensure printf is available

/* --- 宏定义 --- */
#define FOLLOW_TARGET_DIST      20.0f   // 目标跟随距离 (cm)
#define FOLLOW_TARGET_X_CENTER  80.0f   // 图像中心 X 坐标 (假设分辨率 160x120)
#define LOSS_TIMEOUT_SLOW       10      // 丢包减速阈值 (10 * 50ms = 500ms)
#define LOSS_TIMEOUT_STOP       20      // 丢包急停阈值 (40 * 50ms = 2s)

/* --- 全局变量 --- */
/* 外环：视觉位置环 */
PID_Controller_t pid_dist;      // 距离环 (输出线速度 v_linear)
PID_Controller_t pid_angle;     // 角度环 (输出角速度 v_angular)

/* 内环：电机速度环 */
PID_Controller_t pid_speed_L;   // 左电机速度环
PID_Controller_t pid_speed_R;   // 右电机速度环

/* 状态变量 */
static uint32_t loss_counter = 0;   // 丢包计数器
static float target_speed_L = 0.0f; // 左轮目标速度 (RPM)
static float target_speed_R = 0.0f; // 右轮目标速度 (RPM)

/**
 * @brief 更新 PID 参数（从全局配置 g_app_params 加载）
 */
void App_Follow_Update_PID_Params(void)
{
    /* 距离环 */
    pid_dist.Kp = g_app_params.dist_kp;
    pid_dist.Ki = g_app_params.dist_ki;
    pid_dist.Kd = g_app_params.dist_kd;
    
    /* 角度环 */
    pid_angle.Kp = g_app_params.angle_kp;
    pid_angle.Ki = g_app_params.angle_ki;
    pid_angle.Kd = g_app_params.angle_kd;
    
    /* 速度环 (左轮) */
    pid_speed_L.Kp = g_app_params.speed_L_kp;
    pid_speed_L.Ki = g_app_params.speed_L_ki;
    pid_speed_L.Kd = g_app_params.speed_L_kd;
    
    /* 速度环 (右轮) */
    pid_speed_R.Kp = g_app_params.speed_R_kp;
    pid_speed_R.Ki = g_app_params.speed_R_ki;
    pid_speed_R.Kd = g_app_params.speed_R_kd;
}

/**
 * @brief 初始化跟随控制相关的 PID 控制器
 * @note  参数需要根据实际机械结构进行整定
 */
void App_Follow_Init(void)
{
    /* 1. 加载 Flash 参数 */
    App_Flash_Init();
    
    /* 2. 初始化 PID 对象 (先使用 Flash 参数) */
    
    /* 初始化距离环 (目标: 保持距离) */
    /* 输出限幅 100 RPM */
    PID_Init(&pid_dist, g_app_params.dist_kp, g_app_params.dist_ki, g_app_params.dist_kd, 150.0f, 50.0f);

    /* 初始化角度环 (目标: 保持中心) */
    /* 输出限幅 50 RPM (差速) */
    PID_Init(&pid_angle, g_app_params.angle_kp, g_app_params.angle_ki, g_app_params.angle_kd, 80.0f, 30.0f);

    /* 初始化左电机速度环 */
    /* 输出限幅 PWM满占空比 (假设 4200) */
    printf("[PID_Init] Speed_L: Kp=%.2f, Ki=%.2f, Kd=%.2f\r\n", g_app_params.speed_L_kp, g_app_params.speed_L_ki, g_app_params.speed_L_kd);
    PID_Init(&pid_speed_L, g_app_params.speed_L_kp, g_app_params.speed_L_ki, g_app_params.speed_L_kd, 4200.0f, 2000.0f);

    /* 初始化右电机速度环 */
    printf("[PID_Init] Speed_R: Kp=%.2f, Ki=%.2f, Kd=%.2f\r\n", g_app_params.speed_R_kp, g_app_params.speed_R_ki, g_app_params.speed_R_kd);
    PID_Init(&pid_speed_R, g_app_params.speed_R_kp, g_app_params.speed_R_ki, g_app_params.speed_R_kd, 4200.0f, 2000.0f);
}

/**
 * @brief 跟随控制循环 (10ms 周期调用)
 * @note  包含：手动/自动模式切换、丢包保护、外环视觉控制、运动学解算、内环速度控制
 */
void App_Follow_Control_Loop(void)
{
    static Robot_Mode_t last_mode = MODE_MANUAL;
    
    static uint8_t loop_debug_div = 0;
    if (++loop_debug_div >= 20) { // 1s alive check
        loop_debug_div = 0;
        printf("Loop Alive. Mode=%d, Cmd=%d\r\n", g_robot_mode, g_remote_cmd);
    }
    
    /* 1. 全局急停检查 (优先级最高) */
    /* 仅在手动模式或收到明确停止指令时生效？ */
    /* 目前逻辑：只要是 CMD_STOP，无论什么模式都停车 */
    /* 如果自动模式下 g_remote_cmd 默认为 CMD_STOP，就会导致无法运行 */
    if (g_robot_mode == MODE_MANUAL && g_remote_cmd == CMD_STOP) {
        /* 停止电机 */
        TB6612_Motor_SetSpeed(&motorL, 0);
        TB6612_Motor_SetSpeed(&motorR, 0);
        
        /* 重置所有 PID 防止积分饱和 */
        PID_Reset(&pid_dist);
        PID_Reset(&pid_angle);
        PID_Reset(&pid_speed_L);
        PID_Reset(&pid_speed_R);
        
        /* 重置丢包计数器 */
        loss_counter = 0;
        
        /* 记录当前模式，防止恢复时误触发切换逻辑 */
        last_mode = g_robot_mode;
        return;
    }

    /* 2. 模式切换检测 */
    if (g_robot_mode != last_mode) {
        /* 切换模式时重置 PID */
        PID_Reset(&pid_dist);
        PID_Reset(&pid_angle);
        PID_Reset(&pid_speed_L);
        PID_Reset(&pid_speed_R);
        
        /* 重置丢包计数器 */
        loss_counter = 0;
        
        last_mode = g_robot_mode;
    }

    /* 3. 分模式控制 */
    if (g_robot_mode == MODE_MANUAL)
    {
        /* ---------------- 手动模式 ---------------- */
        float pwm_L = 0.0f;
        float pwm_R = 0.0f;
        const float MANUAL_PWM = 1000.0f; // 手动模式直接 PWM (0~4200)

        switch (g_remote_cmd) {
            case CMD_FORWARD:
                pwm_L = MANUAL_PWM;
                pwm_R = MANUAL_PWM;
                break;
            case CMD_BACKWARD:
                pwm_L = -MANUAL_PWM;
                pwm_R = -MANUAL_PWM;
                break;
            case CMD_LEFT: // 原地左转
                pwm_L = -MANUAL_PWM;
                pwm_R = MANUAL_PWM;
                break;
            case CMD_RIGHT: // 原地右转
                pwm_L = MANUAL_PWM;
                pwm_R = -MANUAL_PWM;
                break;
            default:
                pwm_L = 0.0f;
                pwm_R = 0.0f;
                break;
        }
        
        TB6612_Motor_SetSpeed(&motorL, (int32_t)pwm_L);
        TB6612_Motor_SetSpeed(&motorR, (int32_t)pwm_R);
    }
    else
    {
        /* ---------------- 自动模式 (OpenMV 跟随) ---------------- */
        float v_linear = 0.0f;
        float v_angular = 0.0f;

        /* 4.1 丢包检测与保护 */
        /* 检查 FIFO 计数是否变化... (同前) */
        
        /* 临时：每次进入增加计数... */
        loss_counter++;
        
        /* 4.2 外环：视觉位置控制 (PD) */
        if (loss_counter < LOSS_TIMEOUT_STOP)
        {
					if (openmv_data.x == 0 && openmv_data.dist == 0) 
            {
                /* 目标完全丢失，立刻原地立正，不要乱转！ */
                v_linear = 0.0f;
                v_angular = 0.0f;
                
                /* 清理历史误差，防止再次看到目标时猛冲 */
                PID_Reset(&pid_angle);
                PID_Reset(&pid_dist);
            }else{
            /* 计算距离误差 -> 线速度 */
            v_linear = PID_Compute(&pid_dist, openmv_data.dist, FOLLOW_TARGET_DIST);
            //v_linear=0.0f;
            /* 计算角度误差 -> 角速度 */
						v_angular = PID_Compute(&pid_angle, FOLLOW_TARGET_X_CENTER, openmv_data.x);
						}
        }
        else
        {
            /* 急停 */
            v_linear = 0.0f;
            v_angular = 0.0f;
            
            /* 重置积分项 */
            pid_dist.integral = 0;
            pid_angle.integral = 0;
        }
        
        /* 4.3 运动学解算 (差速模型) */
				target_speed_L = v_linear - v_angular;
        target_speed_R = v_linear + v_angular;
//        target_speed_L = 50.0f; // 目标 50 RPM
//				target_speed_R = 50.0f;
//        /* 4.4 丢包保护 (减速) */
        if (loss_counter > LOSS_TIMEOUT_SLOW && loss_counter < LOSS_TIMEOUT_STOP)
        {
            target_speed_L *= 0.5f;
            target_speed_R *= 0.5f;
        }
        
        /* 4.5 内环：电机速度控制 (PI) */
        float pwm_L = PID_Compute(&pid_speed_L, target_speed_L, motor1.speed_rpm);
        float pwm_R = PID_Compute(&pid_speed_R, target_speed_R, motor2.speed_rpm);
        
        /* 4.6 执行输出 */
        TB6612_Motor_SetSpeed(&motorL, (int32_t)pwm_L);
        TB6612_Motor_SetSpeed(&motorR, (int32_t)pwm_R);

        static uint8_t debug_div = 0;
        if (++debug_div >= 10) { // 每500ms打印一次
            debug_div = 0;
//            /* 打印详细调试信息：目标/实际/PWM/误差/当前Kp/丢包数 */
//            printf("AUTO: Tgt=%.1f Act=%.1f PWM=%.0f Err=%.1f Kp=%.2f Ki=%.2f | Loss=%u\r\n", 
//                   target_speed_L, motor1.speed_rpm, pwm_L,
//                   target_speed_L - motor1.speed_rpm,
//                   pid_speed_L.Kp, pid_speed_L.Ki,
//                   loss_counter);
        }
    }
}

/**
 * @brief 供外部调用的丢包计数器重置函数
 * @note  应在 OpenMV 解析成功时调用
 */
void App_Follow_Reset_Loss_Counter(void)
{
    loss_counter = 0;
}

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
