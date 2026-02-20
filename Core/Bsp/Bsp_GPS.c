#include "Bsp_GPS.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "os.h"

/* Buffer Configuration */
#define GPS_RX_BUF_SIZE 512
uint8_t gps_rx_buffer[GPS_RX_BUF_SIZE];
uint8_t gps_proc_buffer[GPS_RX_BUF_SIZE];
volatile uint16_t gps_rx_len = 0;
volatile uint8_t gps_data_ready = 0;

/* Global GPS Data Instance */
nmea_msg gps_data;

/* Helper Functions (Adapted from Source) */
static uint8_t NMEA_Comma_Pos(uint8_t *buf, uint8_t cx) {
    uint8_t *p = buf;
    while (cx) {
        if (*buf == '*' || *buf < ' ' || *buf > 'z') return 0xFF;
        if (*buf == ',') cx--;
        buf++;
    }
    return buf - p;
}

static uint32_t NMEA_Pow(uint8_t m, uint8_t n) {
    uint32_t result = 1;
    while (n--) result *= m;
    return result;
}

static int NMEA_Str2num(uint8_t *buf, uint8_t *dx) {
    uint8_t *p = buf;
    uint32_t ires = 0, fres = 0;
    uint8_t ilen = 0, flen = 0, i;
    uint8_t mask = 0;
    int res;
    while (1) {
        if (*p == '-') { mask |= 0x02; p++; }
        if (*p == ',' || (*p == '*')) break;
        if (*p == '.') { mask |= 0x01; p++; }
        else if (*p > '9' || (*p < '0')) {
            ilen = 0; flen = 0; break;
        }
        if (mask & 0x01) flen++;
        else ilen++;
        p++;
    }
    if (mask & 0x02) buf++;
    for (i = 0; i < ilen; i++) {
        ires += NMEA_Pow(10, ilen - 1 - i) * (buf[i] - '0');
    }
    if (flen > 5) flen = 5;
    *dx = flen;
    for (i = 0; i < flen; i++) {
        fres += NMEA_Pow(10, flen - 1 - i) * (buf[ilen + 1 + i] - '0');
    }
    res = ires * NMEA_Pow(10, flen) + fres;
    if (mask & 0x02) res = -res;
    return res;
}

/* Analysis Functions */
void NMEA_GPGGA_Analysis(nmea_msg *gpsx, uint8_t *buf) {
    uint8_t *p1, dx;
    uint8_t posx;
    p1 = (uint8_t *)strstr((const char *)buf, "GGA");
    if (!p1) return;
    
    posx = NMEA_Comma_Pos(p1, 9);
    if (posx != 0xFF) gpsx->altitude = NMEA_Str2num(p1 + posx, &dx) / (float)NMEA_Pow(10, dx);
    
    posx = NMEA_Comma_Pos(p1, 7);
    if (posx != 0xFF) gpsx->posslnum = NMEA_Str2num(p1 + posx, &dx);
}

void NMEA_GPRMC_Analysis(nmea_msg *gpsx, uint8_t *buf) {
    uint8_t *p1, dx;
    uint8_t posx;
    uint32_t temp;
    float rs;
    
    p1 = (uint8_t *)strstr((const char *)buf, "RMC");
    if (!p1) return;
    
    posx = NMEA_Comma_Pos(p1, 1);
    if (posx != 0xFF) {
        temp = NMEA_Str2num(p1 + posx, &dx) / NMEA_Pow(10, dx);
        gpsx->utc.hour = temp / 10000;
        gpsx->utc.min = (temp / 100) % 100;
        gpsx->utc.sec = temp % 100;
    }
    
    posx = NMEA_Comma_Pos(p1, 3);
    if (posx != 0xFF) {
        temp = NMEA_Str2num(p1 + posx, &dx);
        gpsx->latitude = temp / NMEA_Pow(10, dx + 2);
        rs = temp % NMEA_Pow(10, dx + 2);
        gpsx->latitude = gpsx->latitude * NMEA_Pow(10, 5) + (rs * NMEA_Pow(10, 5 - dx)) / 60;
    }
    
    posx = NMEA_Comma_Pos(p1, 4);
    if (posx != 0xFF) gpsx->nshemi = *(p1 + posx);
    
    posx = NMEA_Comma_Pos(p1, 5);
    if (posx != 0xFF) {
        temp = NMEA_Str2num(p1 + posx, &dx);
        gpsx->longitude = temp / NMEA_Pow(10, dx + 2);
        rs = temp % NMEA_Pow(10, dx + 2);
        gpsx->longitude = gpsx->longitude * NMEA_Pow(10, 5) + (rs * NMEA_Pow(10, 5 - dx)) / 60;
    }
    
    posx = NMEA_Comma_Pos(p1, 6);
    if (posx != 0xFF) gpsx->ewhemi = *(p1 + posx);
    
    posx = NMEA_Comma_Pos(p1, 7);
    if (posx != 0xFF) {
        /* Speed handling: NMEA speed is in Knots. */
        /* Original code logic: temp = temp*10*1.852; itemp = temp; gpsx->speed = itemp; */
        /* Assuming Str2num returns integer with dx decimals. */
        /* e.g. 1.23 knots -> 123, dx=2 */
        /* We want km/h. 1 knot = 1.852 km/h */
        temp = NMEA_Str2num(p1 + posx, &dx);
        gpsx->speed = ((float)temp / NMEA_Pow(10, dx)) * 1.852f;
    }
    
    posx = NMEA_Comma_Pos(p1, 9);
    if (posx != 0xFF) {
        temp = NMEA_Str2num(p1 + posx, &dx);
        gpsx->utc.date = temp / 10000;
        gpsx->utc.month = (temp / 100) % 100;
        gpsx->utc.year = 2000 + temp % 100;
    }
}

/* Initialization */
void GPS_Init(void) {
    /* 1. Enable GPS Module (PE9) */
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);
    
    /* 2. Start UART DMA Reception with Idle Interrupt */
    HAL_UART_Receive_DMA(&huart3, gps_rx_buffer, GPS_RX_BUF_SIZE);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
}

/* UART Rx Idle Callback - To be called from USART3_IRQHandler */
void GPS_Rx_Callback(void) {
    if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE)) {
        __HAL_UART_CLEAR_IDLEFLAG(&huart3);
        
        /* Stop DMA to read length */
        HAL_UART_DMAStop(&huart3);
        
        /* Calculate received length */
        uint16_t rx_len = GPS_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart3.hdmarx);
        
        /* Copy to process buffer */
        if (rx_len > 0) {
            memcpy(gps_proc_buffer, gps_rx_buffer, rx_len);
            gps_proc_buffer[rx_len] = 0; // Null terminate
            gps_rx_len = rx_len;
            gps_data_ready = 1;
        }
        
        /* Restart DMA */
        HAL_UART_Receive_DMA(&huart3, gps_rx_buffer, GPS_RX_BUF_SIZE);
    }
}

/* OS Task */
void GPS_Process_Task(void *arg) {
        /* Check if new data arrived */
        if (gps_data_ready) {
            /* Clear flag */
            gps_data_ready = 0;

            /* Print Raw Data */
            printf("[GPS_RAW] %s\r\n", gps_proc_buffer);
            
            /* Parse Data */
            NMEA_GPRMC_Analysis(&gps_data, gps_proc_buffer);
            NMEA_GPGGA_Analysis(&gps_data, gps_proc_buffer);
        }
        
        /* Yield 200ms */
        OS_DelayMs(200);
}
