#include "bsp_tb6612.h"
TB6612_Motor_t motorL;
TB6612_Motor_t motorR;
/**
 * @brief  初始化电机（开启 PWM）
 */
void TB6612_Motor_Init(TB6612_Motor_t* motor) {
    if (motor == NULL || motor->htim == NULL) return;
    HAL_TIM_PWM_Start(motor->htim, motor->Channel);
    TB6612_Motor_SetSpeed(motor, 0); // 初始停止
}

/**
 * @brief  控制单个电机速度与方向
 * @param  speed: 范围 (-Max_PWM) 到 (Max_PWM)
 */
void TB6612_Motor_SetSpeed(TB6612_Motor_t* motor, int32_t speed) {
    if (motor == NULL) return;

    // 1. 限幅处理
    if (speed > (int32_t)motor->Max_PWM)  speed = motor->Max_PWM;
    if (speed < -(int32_t)motor->Max_PWM) speed = -motor->Max_PWM;

    // 2. 提取绝对值作为 PWM 占空比
    uint32_t abs_pulse = (speed >= 0) ? speed : -speed;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->Channel, abs_pulse);

    // 3. 方向控制逻辑
    if (speed > 0) {         // 正转
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_RESET);
    } else if (speed < 0) {  // 反转
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_SET);
    } else {                 // 停止
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_RESET);
    }
}

void Motor_init(void) {
    // 左电机配置 (TIM4 CH1 -> PD12, 方向引脚假设在 GPIOA)
    motorL.htim     = &htim4;
    motorL.Channel  = TIM_CHANNEL_1;
    motorL.Max_PWM  = 4200;
    motorL.IN1_Port = GPIOB;
    motorL.IN1_Pin  = MOTOR_AIN1_PIN;
    motorL.IN2_Port = GPIOB;
    motorL.IN2_Pin  = MOTOR_AIN2_PIN;

    // 右电机配置 (TIM4 CH2 -> PD13)
    motorR.htim     = &htim4;
    motorR.Channel  = TIM_CHANNEL_2;
    motorR.Max_PWM  = 4200;
    motorR.IN1_Port = GPIOB;
    motorR.IN1_Pin  = MOTOR_BIN1_PIN;
    motorR.IN2_Port = GPIOB;
    motorR.IN2_Pin  = MOTOR_BIN2_PIN;
		
    TB6612_Motor_Init(&motorL);
    TB6612_Motor_Init(&motorR);
}
