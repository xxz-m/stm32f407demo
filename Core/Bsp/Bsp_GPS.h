#ifndef __BSP_GPS_H
#define __BSP_GPS_H

#include "main.h"

/* GPS NMEA Message Structure */
typedef struct {
    struct {
        uint16_t year;
        uint8_t month;
        uint8_t date;
        uint8_t hour;
        uint8_t min;
        uint8_t sec;
    } utc;

    uint8_t svnum;          // 可见卫星数
    struct {
        uint8_t num;        // 卫星编号
        uint8_t eledeg;     // 仰角
        uint16_t azideg;    // 方位角
        uint8_t sn;         // 信噪比
    } slmsg[12];            // 最多12颗卫星

    double latitude;        // 纬度
    uint8_t nshemi;         // 北纬 'N' / 南纬 'S'
    double longitude;       // 经度
    uint8_t ewhemi;         // 东经 'E' / 西经 'W'
    
    float speed;            // 地面速率 (km/h)
    
    uint8_t fixmode;        // 定位类型: 1=未定位, 2=2D, 3=3D
    uint8_t posslnum;       // 用于定位的卫星数
    
    float pdop;             // 位置精度因子
    float hdop;             // 水平精度因子
    float vdop;             // 垂直精度因子
    
    float altitude;         // 海拔高度
} nmea_msg;

/* Global GPS Data */
extern nmea_msg gps_data;

/* Functions */
void GPS_Init(void);
void GPS_Process_Task(void *arg);
void GPS_Rx_Callback(void); /* UART Rx Idle Callback */

#endif /* __BSP_GPS_H */
