#include "core_main.h"
#include "app_ui.h"
#include "app_comm.h"
#include "os.h"
#include "pid.h"
#include "Bsp_GPS.h"
#include "Bsp_Led.h"
#include <stdio.h>

/* 左右电机驱动对象（在其他模块中定义） */
extern TB6612_Motor_t motorL;
extern TB6612_Motor_t motorR;
/* 左右编码器对象（在其他模块中定义） */
extern Encoder_t motor1;
extern Encoder_t motor2;

/* PID 控制器对象 */
static PID_Controller_t pid_L;
static PID_Controller_t pid_R;

/* 任务函数声明 */
static void Task_RobotControl(void *arg);
static void Task_UIUpdate(void *arg);
static void Task_Comm(void *arg);
static void Task_LedRun(void *arg);

/**
 * @brief 核心模块初始化
 * @note  完成电机驱动与编码器模块初始化，建议在系统启动时调用一次
 */
void Core_Main_Init(void)
{
    printf("[Core_Main_Init] Start...\r\n");

	/* 初始化 TB6612 电机驱动 */
	Motor_init();
    printf("[Core_Main_Init] Motor Init done.\r\n");

	/* 初始化编码器采集 */
	Encoder_Init();
    printf("[Core_Main_Init] Encoder Init done.\r\n");

	/* 初始化 DWT 精密延时 */
	delay_init();
    printf("[Core_Main_Init] DWT Delay Init done.\r\n");

	/* 初始化通信模块 */
	App_Comm_Init();
    printf("[Core_Main_Init] Comm Init done.\r\n");
	
	/* 初始化 GPS 模块 (PE9使能, DMA+IDLE接收) */
	GPS_Init();
    printf("[Core_Main_Init] GPS Init done.\r\n");
	
	/* 初始化 LED 模块 (默认熄灭) */
	BSP_LED_Init();
    printf("[Core_Main_Init] LED Init done.\r\n");

    /* 初始化 PID (速度环通常仅使用 PI 控制，微分项 Kd 设为 0 以避免噪声放大) */
    /* 假设目标速度 100RPM, PWM max 4200. Kp=10~50? Ki=0.5? Kd=0.0 */
    PID_Init(&pid_L, 10.0f, 0.5f, 0.0f, 4200.0f, 2000.0f);
    PID_Init(&pid_R, 12.0f, 0.5f, 0.0f, 4200.0f, 2000.0f);
    printf("[Core_Main_Init] PID Init done.\r\n");

		/* 初始化 UI 显示 */
		App_UI_Init();
    printf("[Core_Main_Init] UI Init done.\r\n");
	
    /* 初始化 OS */
    OS_Init();
    printf("[Core_Main_Init] OS Init done.\r\n");

    /* 创建任务 */
    // 机器人主控任务：优先级 2 (高)
    OS_CreateTask(Task_RobotControl, NULL, 2);
    
    // UI 更新任务：优先级 1 (低)
    OS_CreateTask(Task_UIUpdate, NULL, 1);
    
    // GPS 处理任务：优先级 1 (低, 200ms周期)
    OS_CreateTask(GPS_Process_Task, NULL, 1);
    
    // LED 跑马灯任务：优先级 0 (最低, 阻塞式逻辑)
    OS_CreateTask(Task_LedRun, NULL, 0);

    // 通信处理任务：优先级 3 (最高)
    OS_CreateTask(Task_Comm, NULL, 3);
    
    printf("[Core_Main_Init] All tasks created. System Ready.\r\n");
}

/**
 * @brief 主控任务示例
 * @note  当前逻辑为左右电机设置相同速度，可按需替换为控制算法输出
 */
void Core_Main_Master(void)
{
    /* 启动 OS 调度器 (死循环) */
    OS_Start();
}

/**
 * @brief 机器人主控任务
 * @param arg 任务参数 (未使用)
 */
static void Task_RobotControl(void *arg)
{
    /* 目标值变量 */
    float pwm_L = 0.0f;
    float pwm_R = 0.0f;
    float target_speed_L = 0.0f;
    float target_speed_R = 0.0f;
    
    /* 基础参数 */
    const float MANUAL_PWM = 500.0f; // 手动模式直接 PWM (0~4200)
    const float AUTO_RPM   = 500.0f;  // 自动模式目标转速 (RPM)
    
    /* 1. 模式判断与控制量计算 */
    if (g_robot_mode == MODE_MANUAL) {
        /* ---------------- 手动模式 (开环 PWM 控制) ---------------- */
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
            case CMD_STOP:
            default:
                pwm_L = 0.0f;
                pwm_R = 0.0f;
                break;
        }
        
        /* 手动模式下重置 PID，避免积分累积 */
        PID_Reset(&pid_L);
        PID_Reset(&pid_R);
        
    } else {
        /* ---------------- 自动模式 (闭环 PID 直线行驶) ---------------- */
        
        /* 设定目标速度: 左轮定速，右轮跟随 */
        target_speed_L = AUTO_RPM;
        target_speed_R = motor1.speed_rpm; // 主从控制：右轮跟随左轮实际速度
        
        /* 运行 PID 计算 PWM 输出 */
        pwm_L = target_speed_L;
        pwm_R = PID_Compute(&pid_R, target_speed_R, motor2.speed_rpm);
    }
    
    /* 2. 执行电机控制 */
    TB6612_Motor_SetSpeed(&motorL, (int16_t)pwm_L);
    TB6612_Motor_SetSpeed(&motorR, (int16_t)pwm_R);
    
    // 延时 20ms，让出 CPU
    OS_DelayMs(10);
}

/**
 * @brief 通信处理任务
 */
static void Task_Comm(void *arg)
{
    /* 调用通信模块的处理函数 */
    App_Comm_ProcessTask();
    
    /* 延时 10ms */
    OS_DelayMs(10);
}

/**
 * @brief UI 更新任务
 */
static void Task_UIUpdate(void *arg)
{
    /* 更新 UI 显示 (传入 GPS 数据) */
    // App_UI_Update(&motor1, &motor2); // 暂时屏蔽电机速度显示
    App_UI_ShowGPS(&gps_data);
    
    // 延时 200ms，让出 CPU
    OS_DelayMs(200);
}

/**
 * @brief LED 跑马灯任务
 * @param arg 任务参数 (未使用)
 * @note  阻塞式逻辑，利用 OS_DelayMs 切换状态
 *        顺序: LED1 -> LED2 -> LED3 -> LED4 -> LED1 ...
 */
static void Task_LedRun(void *arg)
{
    static uint8_t step = 0;
    const uint32_t LED_DELAY = 200; // 200ms 切换一次

    // 先熄灭所有 LED (或者只熄灭上一个)
    // 为了简单且健壮，根据 step 决定亮哪个，其他灭
    
    switch (step)
    {
        case 0:
            BSP_LED_On(LED_1);
            BSP_LED_Off(LED_2);
            BSP_LED_Off(LED_3);
            BSP_LED_Off(LED_4);
            break;
        case 1:
            BSP_LED_Off(LED_1);
            BSP_LED_On(LED_2);
            BSP_LED_Off(LED_3);
            BSP_LED_Off(LED_4);
            break;
        case 2:
            BSP_LED_Off(LED_1);
            BSP_LED_Off(LED_2);
            BSP_LED_On(LED_3);
            BSP_LED_Off(LED_4);
            break;
        case 3:
            BSP_LED_Off(LED_1);
            BSP_LED_Off(LED_2);
            BSP_LED_Off(LED_3);
            BSP_LED_On(LED_4);
            break;
        default:
            step = 0;
            break;
    }

    // 切换到下一个状态
    step++;
    if (step >= 4) {
        step = 0;
    }

    // 阻塞延时，让出 CPU
    OS_DelayMs(LED_DELAY);
}

/**
 * @brief 定时器周期中断回调函数
 * @param htim 触发回调的定时器句柄
 * @note  仅在 TIM14 中断时更新编码器速度
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    /* 由 TIM14 周期中断触发编码器速度更新 */
    if (htim->Instance == TIM14) {
        Encoder_Update_Speed(&motor1, &motor2);
    }
}
