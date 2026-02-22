#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H

#include "main.h"

/* Flash 存储地址 (STM32F407 Sector 11: 0x080E0000 - 0x080FFFFF) */
#define FLASH_USER_START_ADDR   0x080E0000 
#define FLASH_SECTOR_ID         FLASH_SECTOR_11
#define FLASH_MAGIC_NUM         0xDEADBEEF

/* 参数结构体 (Parameter Structure) */
typedef struct {
    uint32_t magic;      // 校验码 (Magic)
    
    /* 距离 PID (Distance PID) */
    float dist_kp;
    float dist_ki;
    float dist_kd;
    
    /* 角度 PID (Angle PID) */
    float angle_kp;
    float angle_ki;
    float angle_kd;
    
    /* 左电机速度 PID (Left Speed PID) */
    float speed_L_kp;
    float speed_L_ki;
    float speed_L_kd;

    /* 右电机速度 PID (Right Speed PID) */
    float speed_R_kp;
    float speed_R_ki;
    float speed_R_kd;
    
} App_Params_t;

extern App_Params_t g_app_params;

/* 函数原型 (Function Prototypes) */
void App_Flash_Init(void);
void App_Flash_Save(void);
void App_Flash_Load(void);

#endif /* __BSP_FLASH_H */
