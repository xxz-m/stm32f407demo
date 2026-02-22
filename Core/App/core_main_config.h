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
#include "bsp_encoder.h"
#include "DWT_delay.h"
/*---------------------------用户变量-----------*/



/* 电机驱动引脚定义 */
#define MOTOR_AIN1_PORT  GPIOB
#define MOTOR_AIN1_PIN   GPIO_PIN_12
#define MOTOR_AIN2_PORT  GPIOB
#define MOTOR_AIN2_PIN   GPIO_PIN_13

#define MOTOR_BIN1_PORT  GPIOB
#define MOTOR_BIN1_PIN   GPIO_PIN_14
#define MOTOR_BIN2_PORT  GPIOB
#define MOTOR_BIN2_PIN   GPIO_PIN_15

/* 电机使能引脚 */
#define MOTOR_STBY_PORT  GPIOC
#define MOTOR_STBY_PIN   GPIO_PIN_13

/* LED 宏定义 (LED4-1: PC8, PC9, PD15, PD14) */
#define LED4_PORT       GPIOC
#define LED4_PIN        GPIO_PIN_8
#define LED3_PORT       GPIOC
#define LED3_PIN        GPIO_PIN_9
#define LED2_PORT       GPIOD
#define LED2_PIN        GPIO_PIN_15
#define LED1_PORT       GPIOD
#define LED1_PIN        GPIO_PIN_14

/* 按键引脚定义 (KEY1-KEY4: PE4, PE5, PE7, PE8) */
#define KEY1_PORT       GPIOE
#define KEY1_PIN        GPIO_PIN_4
#define KEY2_PORT       GPIOE
#define KEY2_PIN        GPIO_PIN_5
#define KEY3_PORT       GPIOE
#define KEY3_PIN        GPIO_PIN_7
#define KEY4_PORT       GPIOE
#define KEY4_PIN        GPIO_PIN_8

/* 调试打印宏定义 */
#define DEBUG_GPS_PRINT    0  // 置1开启GPS原始数据打印，置0关闭
#define DEBUG_OPENMV_PRINT 0  // 置1开启OpenMV数据打印，置0关闭

// 用户自定义

#ifdef __cplusplus
}
#endif
#endif /*__CORE__MAIN_CONFIG_H__ */
