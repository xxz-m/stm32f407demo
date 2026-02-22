#include "Bsp_OpenMV.h"
#include "core_main_config.h"
#include "os.h"
#include <stdio.h>
#include "pid.h"

/* 串口句柄 (UART Handle) */
static UART_HandleTypeDef *openmv_huart;
#define OPENMV_RX_BUF_SIZE 512
static uint8_t openmv_rx_buffer[OPENMV_RX_BUF_SIZE];
static uint16_t last_rx_index = 0;
static volatile uint8_t omv_process_flag = 0; // 空闲中断标志 (IDLE flag)

/* OpenMV 数据实例 (OpenMV Data Instance) */
OpenMV_Data_t openmv_data = {0};
OpenMV_FIFO_t openmv_fifo = {0};

/* 将数据写入 FIFO (Write Data to FIFO) */
static void OpenMV_FIFO_Push(OpenMV_Data_t data) {
    /* 写入数据到缓冲区 (Write data to buffer) */
    openmv_fifo.buffer[openmv_fifo.head] = data;
    
    /* 更新 head 指针 (Update head pointer) */
    uint8_t next_head = (openmv_fifo.head + 1) % OPENMV_FIFO_SIZE;
    
    /* 检查缓冲区是否满 (Check if buffer full) */
    if (next_head == openmv_fifo.tail) {
        /* 缓冲区满，覆盖旧数据 (Buffer full, overwrite old data) */
        openmv_fifo.tail = (openmv_fifo.tail + 1) % OPENMV_FIFO_SIZE;
        /* count 不需要增加，因为覆盖了一个旧的 (count remains same) */
    } else {
        /* 未满，增加计数 (Not full, increment count) */
        openmv_fifo.count++;
    }
    
    openmv_fifo.head = next_head;
}

/* 从 FIFO 读取数据 (Read Data from FIFO) */
static uint8_t OpenMV_FIFO_Pop(OpenMV_Data_t *data) {
    /* 检查缓冲区是否空 (Check if buffer empty) */
    if (openmv_fifo.head == openmv_fifo.tail) {
        return 0; // 空 (Empty)
    }
    
    /* 读取数据 (Read data) */
    *data = openmv_fifo.buffer[openmv_fifo.tail];
    
    /* 更新 tail 指针 (Update tail pointer) */
    openmv_fifo.tail = (openmv_fifo.tail + 1) % OPENMV_FIFO_SIZE;
    
    /* 减少计数 (Decrement count) */
    __disable_irq();
    if (openmv_fifo.count > 0) openmv_fifo.count--;
    __enable_irq();
    
    return 1;
}

/* 协议解析状态 (Protocol States) */
typedef enum {
    OMV_STATE_HEADER = 0, // 包头 (Header)
    OMV_STATE_X,          // X坐标 (X Coordinate)
    OMV_STATE_DIST,       // 距离 (Distance)
    OMV_STATE_TAIL        // 包尾 (Tail)
} OMV_State_t;

static OMV_State_t omv_state = OMV_STATE_HEADER;
static uint8_t temp_x = 0;
static uint8_t temp_dist = 0;

/* 协议解码函数 (Protocol Decode Function) */
static uint8_t OpenMV_Decode(uint8_t byte) {
    uint8_t packet_ready = 0;
    
    switch (omv_state) {
        case OMV_STATE_HEADER:
            if (byte == 0xAA) {
                omv_state = OMV_STATE_X;
            }
            break;
            
        case OMV_STATE_X:
            temp_x = byte;
            omv_state = OMV_STATE_DIST;
            break;
            
        case OMV_STATE_DIST:
            temp_dist = byte;
            omv_state = OMV_STATE_TAIL;
            break;
            
        case OMV_STATE_TAIL:
            if (byte == 0x55) {
                /* 有效数据包 (Valid Packet) */
                openmv_data.x = temp_x;
                openmv_data.dist = temp_dist;
                
                /* 推入 FIFO (Push to FIFO) */
                OpenMV_FIFO_Push(openmv_data);
                packet_ready = 1;
            }
            /* 无论是否正确包尾，都重置状态 (Reset state) */
            omv_state = OMV_STATE_HEADER; 
            break;
            
        default:
            omv_state = OMV_STATE_HEADER;
            break;
    }
    
    return packet_ready;
}
