/**
 * @file    u8g2_port.c
 * @brief   U8G2 驱动库针对 STM32 硬件 I2C 的移植文件
 * @author  朱治豪 (移植与优化)
 * @date    2026-02-19
 */

#include "u8g2_port.h"
#include "DWT_delay.h" // 引入微秒级延时库

// 声明外部 I2C 句柄，该句柄在 i2c.c 中由 CubeMX 定义
extern I2C_HandleTypeDef hi2c1;

// 定义 U8g2 实例
u8g2_t u8g2;

/**
 * @brief   U8g2 的 I2C 底层发送函数
 * @note    此函数通过 HAL 库的硬件 I2C 发送数据。
 *          它内部带有一个缓冲区，以减少 HAL_I2C_Master_Transmit 的调用次数，提高效率。
 * @param   u8x8    指向 u8x8_t 结构体的指针
 * @param   msg     消息类型
 * @param   arg_int 参数整数
 * @param   arg_ptr 指向数据的指针
 * @return  1 表示成功，0 表示失败
 */
static uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buf[128]; // 发送缓冲区
    static uint16_t idx;     // 缓冲区索引
    uint8_t *data;

    switch (msg)
    {
    case U8X8_MSG_BYTE_INIT:
        // 初始化消息，无需操作
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        // 开始传输，清空缓冲区索引
        idx = 0;
        break;

    case U8X8_MSG_BYTE_SEND:
        // 发送数据块
        data = (uint8_t *)arg_ptr;
        while (arg_int--)
        {
            buf[idx++] = *data++;
            // 当缓冲区满时，立即发送
            if (idx >= sizeof(buf))
            {
                if (HAL_I2C_Master_Transmit(&hi2c1, u8x8_GetI2CAddress(u8x8), buf, idx, 100) != HAL_OK)
                    return 0; // 发送失败
                idx = 0;
            }
        }
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        // 结束传输，发送缓冲区剩余数据
        if (idx > 0)
        {
            if (HAL_I2C_Master_Transmit(&hi2c1, u8x8_GetI2CAddress(u8x8), buf, idx, 100) != HAL_OK)
                return 0; // 发送失败
        }
        break;

    default:
        return 0;
    }
    return 1;
}

/**
 * @brief   U8g2 的 GPIO 和延时回调函数
 * @note    为 U8g2 提供毫秒和微秒延时。
 *          由于使用硬件 I2C，GPIO 操作相关的消息被忽略。
 * @param   u8x8    指向 u8x8_t 结构体的指针
 * @param   msg     消息类型
 * @param   arg_int 参数整数
 * @param   arg_ptr 指向数据的指针
 * @return  始终为 1
 */
static uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)u8x8; (void)arg_ptr;

    switch (msg)
    {
    case U8X8_MSG_DELAY_MILLI:
        // 毫秒延时
        HAL_Delay(arg_int);
        break;

    case U8X8_MSG_DELAY_10MICRO:
        // 10微秒延时
        delay_us(arg_int * 10);
        break;

    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        // GPIO 和延时初始化，硬件 I2C 无需操作
        break;

    // 以下 GPIO 消息在使用硬件 I2C 时无需处理
    case U8X8_MSG_GPIO_I2C_CLOCK:
    case U8X8_MSG_GPIO_I2C_DATA:
    case U8X8_MSG_GPIO_RESET:
        break;

    default:
        // 对于未处理的消息，返回 1 表示已处理，避免 U8g2 尝试软件模拟
        return 1;
    }
    return 1;
}

/**
 * @brief   初始化 U8G2 显示屏
 * @note    此函数会配置 U8G2 库，设置 I2C 地址并初始化显示。
 */
void U8G2_Init(void)
{
    // 1. 选择适合您屏幕的 Setup 函数
    //    - ssd1306: 驱动名称
    //    - i2c: 通信接口
    //    - 128x64: 分辨率
    //    - noname: 字体/ROM 类型
    //    - f: 表示全缓冲模式 (Full Buffer)
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
        &u8g2,
        U8G2_R0, // 旋转设置，U8G2_R0 表示不旋转
        u8x8_byte_stm32_hw_i2c, // 设置 I2C 发送回调
        u8x8_gpio_and_delay_stm32 // 设置 GPIO 和延时回调
    );

    // 2. 设置 I2C 地址
    //    U8g2 使用 8 位地址，对于 7 位地址 0x3C，需要左移一位变为 0x78
    //    如果 0x78 不工作，请尝试 0x7A
    u8g2_SetI2CAddress(&u8g2, 0x78);

    // 3. 初始化并点亮屏幕
    u8g2_InitDisplay(&u8g2);    // 初始化显示器
    u8g2_SetPowerSave(&u8g2, 0); // 唤醒显示器
    u8g2_ClearBuffer(&u8g2);     // 清空本地缓冲区
    u8g2_SendBuffer(&u8g2);      // 将缓冲区内容发送到屏幕
}
