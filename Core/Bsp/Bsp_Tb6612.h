#ifndef __BSP_TB6612_H
#define __BSP_TB6612_H

#include "main.h"
#include "core_main_config.h"
/* 电机对象结构体 */

typedef struct {
    // GPIO 控制引脚
    GPIO_TypeDef* IN1_Port;
    uint16_t      IN1_Pin;
    GPIO_TypeDef* IN2_Port;
    uint16_t      IN2_Pin;
    
    // PWM 定时器配置
    TIM_HandleTypeDef* htim;
    uint32_t           Channel;
    
    // 最大计数值 (用于限幅)
    uint32_t           Max_PWM;
} TB6612_Motor_t;

/* BSP 接口函数 */
void TB6612_Motor_Init(TB6612_Motor_t* motor);
void TB6612_Motor_SetSpeed(TB6612_Motor_t* motor, int32_t speed);
void Motor_init(void);
#endif
