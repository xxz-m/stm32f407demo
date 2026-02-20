/**
 * @file    os.c
 * @brief   简单协同式任务调度器实现文件
 * @date    2026-02-19
 */

#include "os.h"

static OS_TCB_t g_tasks[OS_MAX_TASKS];
static int32_t  g_currentTask = OS_INVALID_TASK;

// 找一个空闲的任务槽位
static int32_t OS_FindEmptySlot(void)
{
    for (int i = 0; i < OS_MAX_TASKS; ++i) {
        if (g_tasks[i].state == OS_TASK_UNUSED) {
            return i;
        }
    }
    return OS_INVALID_TASK;
}

// 从所有 READY 任务中选出优先级最高的一个
static int32_t OS_SelectNextTask(void)
{
    int32_t bestId = OS_INVALID_TASK;
    uint8_t bestPrio = 0;
    int     found = 0; // 标记是否找到至少一个 READY 任务

    for (int i = 0; i < OS_MAX_TASKS; ++i) {
        if (g_tasks[i].state == OS_TASK_READY) {
            // 如果是第一个找到的任务，或者优先级更高
            if (!found || g_tasks[i].priority > bestPrio) {
                bestId = i;
                bestPrio = g_tasks[i].priority;
                found = 1;
            }
        }
    }
    return bestId;
}

void OS_Init(void)
{
    for (int i = 0; i < OS_MAX_TASKS; ++i) {
        g_tasks[i].entry      = 0;
        g_tasks[i].arg        = 0;
        g_tasks[i].priority   = 0;
        g_tasks[i].state      = OS_TASK_UNUSED;
        g_tasks[i].delayTicks = 0;
    }
    g_currentTask = OS_INVALID_TASK;
}

// 优先级数字越大，优先级越高
int32_t OS_CreateTask(void (*entry)(void *),
                      void *arg,
                      uint8_t priority)
{
    int32_t id = OS_FindEmptySlot();
    if (id == OS_INVALID_TASK || entry == 0) {
        return OS_INVALID_TASK;
    }

    g_tasks[id].entry      = entry;
    g_tasks[id].arg        = arg;
    g_tasks[id].priority   = priority;
    g_tasks[id].state      = OS_TASK_READY;
    g_tasks[id].delayTicks = 0;

    return id;
}

// 删除任务
void OS_DeleteTask(int32_t id)
{
    if (id < 0 || id >= OS_MAX_TASKS) return;

    g_tasks[id].state      = OS_TASK_UNUSED;
    g_tasks[id].entry      = 0;
    g_tasks[id].arg        = 0;
    g_tasks[id].priority   = 0;
    g_tasks[id].delayTicks = 0;

    if (g_currentTask == id) {
        g_currentTask = OS_INVALID_TASK;
    }
}

// 挂起
void OS_SuspendTask(int32_t id)
{
    if (id < 0 || id >= OS_MAX_TASKS) return;
    if (g_tasks[id].state == OS_TASK_UNUSED) return;

    // 不管是 READY / RUNNING / DELAYED，都可以直接挂起
    g_tasks[id].state      = OS_TASK_SUSPENDED;
    g_tasks[id].delayTicks = 0;              // 防止残留延时，下次恢复时直接 READY

    if (g_currentTask == id) {
        g_currentTask = OS_INVALID_TASK;
    }
}

// 恢复
void OS_ResumeTask(int32_t id)
{
    if (id < 0 || id >= OS_MAX_TASKS) return;
    if (g_tasks[id].state == OS_TASK_SUSPENDED) {
        g_tasks[id].state = OS_TASK_READY;
    }
}

// 当前任务主动让出 CPU
void OS_Yield(void)
{
    // 对于协同式调度，Yield 实际上只是为了改变状态，真正的让出是函数 return
    if (g_currentTask < 0 || g_currentTask >= OS_MAX_TASKS) return;

    // 把自己改回 READY，让调度器有机会调度别人
    if (g_tasks[g_currentTask].state == OS_TASK_RUNNING) {
        g_tasks[g_currentTask].state = OS_TASK_READY;
        g_currentTask = OS_INVALID_TASK;
    }
}

// 非阻塞延时：让当前任务 N 个 tick 不再被调度
void OS_DelayTicks(uint32_t ticks)
{
    if (ticks == 0) return;
    if (g_currentTask < 0 || g_currentTask >= OS_MAX_TASKS) return;

    g_tasks[g_currentTask].delayTicks = ticks;
    g_tasks[g_currentTask].state      = OS_TASK_DELAYED;
    g_currentTask = OS_INVALID_TASK;
}

void OS_DelayMs(uint32_t ms)
{
    if (ms == 0) return;

#if (OS_TICK_MS == 1)
    OS_DelayTicks(ms);
#else
    uint32_t ticks = ms / OS_TICK_MS;
    if (ticks == 0) ticks = 1;
    OS_DelayTicks(ticks);
#endif
}

// 时基：在定时器中断里调用
void OS_Tick(void)
{
    // 扫描所有 DELAYED 任务，做倒计时
    for (int i = 0; i < OS_MAX_TASKS; ++i) {
        if (g_tasks[i].state == OS_TASK_DELAYED && g_tasks[i].delayTicks > 0) {
            g_tasks[i].delayTicks--;
            if (g_tasks[i].delayTicks == 0) {
                g_tasks[i].state = OS_TASK_READY;
            }
        }
    }
}

// 调度一次：选任务 → 调用其函数一次
void OS_ScheduleOnce(void)
{
    int32_t next = OS_SelectNextTask();
    if (next == OS_INVALID_TASK) {
        // 没有 READY 任务，啥也不干（可以在这里挂个 idle 任务）
        return;
    }

    g_currentTask = next;
    g_tasks[next].state = OS_TASK_RUNNING;

    // 调用任务函数：任务内部不要写 while(1)，只做一步逻辑就 return
    if (g_tasks[next].entry) {
        g_tasks[next].entry(g_tasks[next].arg);
    }

    // 如果任务没自己改状态（比如延时/挂起/删除），默认跑完一次回到 READY
    if (g_currentTask == next &&
        g_tasks[next].state == OS_TASK_RUNNING) {
        g_tasks[next].state = OS_TASK_READY;
        g_currentTask = OS_INVALID_TASK;
    } else {
        // 任务在内部可能调用了 Delay/Suspend/Delete 等，我们尊重它的状态
        // g_currentTask 已经被修改为 OS_INVALID_TASK 或其他值
    }
}

// 启动调度器：死循环调度
void OS_Start(void)
{
    while (1) {
        OS_ScheduleOnce();
        // 这里也可以插 Idle 行为，比如省电 / 喂狗 等
    }
}
