#ifndef __CORE__MAIN_CONFIG_H__
#define __CORE__MAIN_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/*-----------------------------APP层-------------*/
#include "core_main.h"
/*-----------------------------BSP层-------------*/
#include "tim.h"
#include "bsp_tb6612.h"

/*---------------------------引脚配置-----------*/



/* 电机方向控制引脚定义 */
#define MOTOR_AIN1_PORT  GPIOB
#define MOTOR_AIN1_PIN   GPIO_PIN_12
#define MOTOR_AIN2_PORT  GPIOB
#define MOTOR_AIN2_PIN   GPIO_PIN_13

#define MOTOR_BIN1_PORT  GPIOB
#define MOTOR_BIN1_PIN   GPIO_PIN_14
#define MOTOR_BIN2_PORT  GPIOB
#define MOTOR_BIN2_PIN   GPIO_PIN_15

/* 待机使能引脚 */
#define MOTOR_STBY_PORT  GPIOC
#define MOTOR_STBY_PIN   GPIO_PIN_13
//公共变量

#ifdef __cplusplus
}
#endif
#endif /*__CORE__MAIN_CONFIG_H__ */


