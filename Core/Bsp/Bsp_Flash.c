#include "Bsp_Flash.h"
#include <string.h>

/* 全局参数变量 (Global Parameters) */
App_Params_t g_app_params;

/**
 * @brief 从 Flash 加载参数 (Load Parameters from Flash)
 * @note  检查 Flash 有效标志，如果有效则加载，否则使用默认值
 */
void App_Flash_Load(void)
{
    App_Params_t *flash_params = (App_Params_t *)FLASH_USER_START_ADDR;
    
    /* 检查魔数 (Check Magic Number) */
    if (flash_params->magic == FLASH_MAGIC_NUM) {
        /* 有效: 加载参数 (Valid: Load parameters) */
        memcpy(&g_app_params, flash_params, sizeof(App_Params_t));
    } else {
        /* 无效: 设置默认参数 (Invalid: Set default parameters) */
        g_app_params.magic = FLASH_MAGIC_NUM;
        
        /* 距离 PID 参数 (Distance PID) */
        g_app_params.dist_kp = 2.0f;
        g_app_params.dist_ki = 0.1f;
        g_app_params.dist_kd = 0.5f;
        
        /* 角度 PID 参数 (Angle PID) */
        g_app_params.angle_kp = 1.5f;
        g_app_params.angle_ki = 0.0f;
        g_app_params.angle_kd = 0.2f;
        
        /* 左电机速度 PID 参数 (Left Speed PID) */
        g_app_params.speed_L_kp = 5.0f;
        g_app_params.speed_L_ki = 2.5f;
        g_app_params.speed_L_kd = 0.0f;

        /* 右电机速度 PID 参数 (Right Speed PID) */
        g_app_params.speed_R_kp = 5.0f;
        g_app_params.speed_R_ki = 2.5f;
        g_app_params.speed_R_kd = 0.0f;
    }
}

/**
 * @brief 保存参数到 Flash (Save Parameters to Flash)
 */
void App_Flash_Save(void)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;
    
    /* 1. 解锁 Flash (Unlock Flash) */
    HAL_FLASH_Unlock();
    
    /* 2. 擦除扇区 (Erase Sector) */
    EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector       = FLASH_SECTOR_ID;
    EraseInitStruct.NbSectors    = 1;
    
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
        HAL_FLASH_Lock();
        return; // 擦除失败 (Erase Failed)
    }
    
    /* 3. 写入数据 (按字写入) (Write Data) */
    uint32_t *p_data = (uint32_t *)&g_app_params;
    uint32_t num_words = sizeof(App_Params_t) / 4;
    /* 处理对齐 (Handle alignment padding) */
    if (sizeof(App_Params_t) % 4 != 0) num_words++; 
    
    for (uint32_t i = 0; i < num_words; i++) {
        uint32_t address = FLASH_USER_START_ADDR + (i * 4);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, p_data[i]) != HAL_OK) {
            break; // 写入失败 (Write Failed)
        }
    }
    
    /* 4. 上锁 Flash (Lock Flash) */
    HAL_FLASH_Lock();
}

/**
 * @brief Flash 模块初始化 (Flash Initialization)
 */
void App_Flash_Init(void)
{
    App_Flash_Load();
}
