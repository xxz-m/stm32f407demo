/**
 * @file    bsp_tb6612.c
 * @brief   TB6612FNG 电机驱动板级支持包
 * @author  (朱治豪)
 * @date    2026-02-19
 */

#include "bsp_tb6612.h"

/* 私有变量 ---------------------------------------------------------*/
TB6612_Motor_t motorL; /*!< 左电机实例 */
TB6612_Motor_t motorR; /*!< 右电机实例 */

/**
 * @brief  初始化单个电机
 * @note   此函数为电机启动 PWM 信号，并将其初始速度设置为 0。
 * @param  motor: 指向 TB6612_Motor_t 结构体的指针
 * @retval 无
 */
void TB6612_Motor_Init(TB6612_Motor_t* motor) {
    if (motor == NULL || motor->htim == NULL) return;

    // 启动 PWM 信号生成
    HAL_TIM_PWM_Start(motor->htim, motor->Channel);
    // 设置初始速度为 0 (停止)
    TB6612_Motor_SetSpeed(motor, 0);
}

/**
 * @brief  设置单个电机的速度和方向
 * @param  motor: 指向 TB6612_Motor_t 结构体的指针
 * @param  speed: 电机速度，范围从 -motor->Max_PWM (全速反转) 到 +motor->Max_PWM (全速正转)
 * @retval 无
 */
void TB6612_Motor_SetSpeed(TB6612_Motor_t* motor, int32_t speed) {
    if (motor == NULL) return;

    // 1. 将速度限制在最大 PWM 值范围内
    if (speed > (int32_t)motor->Max_PWM)  speed = motor->Max_PWM;
    if (speed < -(int32_t)motor->Max_PWM) speed = -motor->Max_PWM;

    // 2. 根据速度绝对值设置 PWM 占空比
    uint32_t abs_pulse = (speed >= 0) ? speed : -speed;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->Channel, abs_pulse);

    // 3. 根据速度符号控制电机方向
    // 修正：由于电机接线或机械安装原因，原逻辑导致方向相反，此处交换高低电平
    if (speed > 0) {         // 正转 (Forward)
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_SET);
    } else if (speed < 0) {  // 反转 (Backward)
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_RESET);
    } else {                 // 停止 (刹车)
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief  初始化左右两个电机
 * @note   此函数配置两个电机的 GPIO 引脚和 TIM 通道，
 *         并使 TB6612 驱动器退出待机模式。
 * @retval 无
 */
void Motor_init(void) {
    // 左电机配置 (TIM4 CH2 -> PD13)
    // 修正：原左电机配置对应右轮，现交换以匹配物理连接
    motorL.htim     = &htim4;
    motorL.Channel  = TIM_CHANNEL_2;
    motorL.Max_PWM  = 4200;
    motorL.IN1_Port = GPIOB;
    motorL.IN1_Pin  = MOTOR_BIN1_PIN;
    motorL.IN2_Port = GPIOB;
    motorL.IN2_Pin  = MOTOR_BIN2_PIN;

    // 右电机配置 (TIM4 CH1 -> PD12)
    // 修正：原右电机配置对应左轮，现交换以匹配物理连接
    motorR.htim     = &htim4;
    motorR.Channel  = TIM_CHANNEL_1;
    motorR.Max_PWM  = 4200;
    motorR.IN1_Port = GPIOB;
    motorR.IN1_Pin  = MOTOR_AIN1_PIN;
    motorR.IN2_Port = GPIOB;
    motorR.IN2_Pin  = MOTOR_AIN2_PIN;

    // 将 STBY 引脚拉高以使能 TB6612 驱动器
    HAL_GPIO_WritePin(MOTOR_STBY_PORT, MOTOR_STBY_PIN, GPIO_PIN_SET);

    // 初始化两个电机
    TB6612_Motor_Init(&motorL);
    TB6612_Motor_Init(&motorR);
}

