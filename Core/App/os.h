/**
 * @file    os.h
 * @brief   简单协同式任务调度器头文件
 * @date    2026-02-19
 */

#ifndef __OS_H
#define __OS_H

#include <stdint.h>

/* 配置项 */
#define OS_MAX_TASKS    8       // 最大任务数
#define OS_TICK_MS      1       // 调度器时基 (ms)

/* 任务状态枚举 */
typedef enum {
    OS_TASK_UNUSED = 0,
    OS_TASK_READY,
    OS_TASK_RUNNING,
    OS_TASK_SUSPENDED,
    OS_TASK_DELAYED
} OS_TaskState_t;

/* 任务控制块 (TCB) */
typedef struct {
    void (*entry)(void *arg);   // 任务函数指针
    void           *arg;        // 任务参数
    uint8_t         priority;   // 优先级 (数值越大越高)
    OS_TaskState_t  state;      // 当前状态
    uint32_t        delayTicks; // 延时计数器
} OS_TCB_t;

/* 宏定义无效任务 ID */
#define OS_INVALID_TASK (-1)

/* API 函数声明 */

/**
 * @brief 初始化操作系统
 */
void OS_Init(void);

/**
 * @brief 创建新任务
 * @param entry    任务入口函数 (注意：对于协同式调度，此函数应执行一次后返回，不能死循环)
 * @param arg      传递给任务的参数
 * @param priority 任务优先级 (0-255，越大越高)
 * @return 任务 ID，失败返回 OS_INVALID_TASK
 */
int32_t OS_CreateTask(void (*entry)(void *), void *arg, uint8_t priority);

/**
 * @brief 删除任务
 * @param id 任务 ID
 */
void OS_DeleteTask(int32_t id);

/**
 * @brief 挂起任务
 * @param id 任务 ID
 */
void OS_SuspendTask(int32_t id);

/**
 * @brief 恢复任务
 * @param id 任务 ID
 */
void OS_ResumeTask(int32_t id);

/**
 * @brief 主动让出 CPU (对于本协同调度器，等同于 return)
 */
void OS_Yield(void);

/**
 * @brief 任务延时 (毫秒)
 * @param ms 延时时间
 */
void OS_DelayMs(uint32_t ms);

/**
 * @brief 任务延时 (Ticks)
 * @param ticks 延时 Tick 数
 */
void OS_DelayTicks(uint32_t ticks);

/**
 * @brief OS 时基心跳，需要在定时器中断中调用
 */
void OS_Tick(void);

/**
 * @brief 启动调度器 (死循环，不会返回)
 */
void OS_Start(void);

/**
 * @brief 调度一次 (供内部或特殊用途调用)
 */
void OS_ScheduleOnce(void);

#endif /* __OS_H */
