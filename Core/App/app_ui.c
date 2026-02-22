/**
 * @file    app_ui.c
 * @brief   用户界面显示模块实现文件 (U8g2 菜单框架)
 * @date    2026-02-19
 */

#include "app_ui.h"
#include "app_comm.h"
#include "../Bsp/Bsp_Flash.h"
#include "../Algo/pid.h"
#include <stdio.h>

/* 外部变量引用 */
extern Encoder_t motor1;
extern Encoder_t motor2;
extern nmea_msg gps_data;

/* 内部状态变量 */
static UI_State_t g_ui_state = {PAGE_MAIN, 0, 0, 0};

/* 菜单项数量定义 */
#define MAIN_MENU_ITEMS 3
#define PID_MENU_ITEMS  13
#define PID_VIEW_ITEMS  4

/* 内部函数声明 */
static void Draw_MainPage(void);
static void Draw_MotorPage(void);
static void Draw_GPSPage(void);
static void Draw_PIDPage(void);

/**
 * @brief UI 模块初始化
 */
void App_UI_Init(void)
{
    U8G2_Init();
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
}

/**
 * @brief 按键边沿检测与事件处理
 * @note  检测 Key1-Key4 的按下事件 (Rising Edge)
 */
void App_UI_KeyProcess(void)
{
    static Key_e last_key = KEY_NONE;
    Key_e current_key = g_current_key; // 读取 BSP 中的当前按键状态

    /* 检测按下边沿 (上一次无按键 -> 这一次有按键) */
    if (current_key != KEY_NONE && last_key == KEY_NONE)
    {
        /* 处理按键逻辑 */
        switch (g_ui_state.current_page)
        {
            /* ---------------- 主菜单逻辑 ---------------- */
            case PAGE_MAIN:
                if (current_key == KEY_1) { // Up
                    if (g_ui_state.cursor_index > 0) g_ui_state.cursor_index--;
                    else g_ui_state.cursor_index = MAIN_MENU_ITEMS - 1;
                }
                else if (current_key == KEY_4) { // Down
                    if (g_ui_state.cursor_index < MAIN_MENU_ITEMS - 1) g_ui_state.cursor_index++;
                    else g_ui_state.cursor_index = 0;
                }
                else if (current_key == KEY_2) { // Enter
                    switch (g_ui_state.cursor_index) {
                        case 0: g_ui_state.current_page = PAGE_MOTOR; break;
                        case 1: g_ui_state.current_page = PAGE_GPS; break;
                        case 2: g_ui_state.current_page = PAGE_PID; break;
                    }
                    g_ui_state.cursor_index = 0; // 重置光标
                }
                break;

            /* ---------------- 电机/GPS 页面逻辑 (View Only) ---------------- */
            case PAGE_MOTOR:
            case PAGE_GPS:
                if (current_key == KEY_3) { // Back
                    g_ui_state.current_page = PAGE_MAIN;
                    g_ui_state.cursor_index = 0; // 回到主菜单第一项
                }
                break;

            /* ---------------- PID 参数页面逻辑 ---------------- */
            case PAGE_PID:
                {
                    /* 定义参数映射与调节步长 */
                    struct { float *val; float step; } params[] = {
                        {&g_app_params.dist_kp, 0.1f}, {&g_app_params.dist_ki, 0.01f}, {&g_app_params.dist_kd, 0.1f},
                        {&g_app_params.angle_kp, 0.1f}, {&g_app_params.angle_ki, 0.01f}, {&g_app_params.angle_kd, 0.1f},
                        {&g_app_params.speed_L_kp, 0.1f}, {&g_app_params.speed_L_ki, 0.01f}, {&g_app_params.speed_L_kd, 0.1f},
                        {&g_app_params.speed_R_kp, 0.1f}, {&g_app_params.speed_R_ki, 0.01f}, {&g_app_params.speed_R_kd, 0.1f},
                        {NULL, 0.0f}
                    };

                    if (g_ui_state.is_editing) {
                        /* 编辑模式 */
                        float *val_ptr = params[g_ui_state.cursor_index].val;
                        float step = params[g_ui_state.cursor_index].step;
                        
                        if (val_ptr != NULL) {
                            if (current_key == KEY_1) { // Up -> Increase
                                *val_ptr += step;
                            }
                            else if (current_key == KEY_4) { // Down -> Decrease
                                *val_ptr -= step;
                                if (*val_ptr < 0.0f) *val_ptr = 0.0f; 
                            }
                        }

                        if (current_key == KEY_2) { // OK -> Exit Edit
                            g_ui_state.is_editing = 0;
                        }
                    } else {
                        /* 导航模式 */
                        if (current_key == KEY_1) { // Up
                            if (g_ui_state.cursor_index > 0) {
                                g_ui_state.cursor_index--;
                                if (g_ui_state.cursor_index < g_ui_state.scroll_offset) {
                                    g_ui_state.scroll_offset = g_ui_state.cursor_index;
                                }
                            }
                            else { // Wrap to bottom
                                g_ui_state.cursor_index = PID_MENU_ITEMS - 1;
                                g_ui_state.scroll_offset = PID_MENU_ITEMS - PID_VIEW_ITEMS;
                            }
                        }
                        else if (current_key == KEY_4) { // Down
                            if (g_ui_state.cursor_index < PID_MENU_ITEMS - 1) {
                                g_ui_state.cursor_index++;
                                if (g_ui_state.cursor_index >= g_ui_state.scroll_offset + PID_VIEW_ITEMS) {
                                    g_ui_state.scroll_offset = g_ui_state.cursor_index - PID_VIEW_ITEMS + 1;
                                }
                            }
                            else { // Wrap to top
                                g_ui_state.cursor_index = 0;
                                g_ui_state.scroll_offset = 0;
                            }
                        }
                        else if (current_key == KEY_2) { // Enter
                            /* 如果选中的是 "Save & Exit" (最后一项) */
                            if (g_ui_state.cursor_index == PID_MENU_ITEMS - 1) {
                                App_Flash_Save(); // 写入 Flash
                                App_Follow_Update_PID_Params(); // 更新 PID 参数
                                
                                /* 返回主菜单 */
                                g_ui_state.current_page = PAGE_MAIN;
                                g_ui_state.cursor_index = 2; 
                                g_ui_state.scroll_offset = 0;
                            } else {
                                /* 进入编辑模式 */
                                g_ui_state.is_editing = 1;
                            }
                        }
                        else if (current_key == KEY_3) { // Back -> 返回主菜单 (不保存)
                            g_ui_state.current_page = PAGE_MAIN;
                            g_ui_state.cursor_index = 2; 
                            g_ui_state.scroll_offset = 0;
                        }
                    }
                }
                break;
                
            default:
                break;
        }
    }

    /* 更新历史按键状态 */
    last_key = current_key;
}

/**
 * @brief UI 刷新主函数
 */
void App_UI_Refresh(void)
{
    /* 1. 处理按键输入 */
    App_UI_KeyProcess();

    /* 2. 开始绘制 */
    u8g2_ClearBuffer(&u8g2);

    /* 3. 根据页面分发绘制函数 */
    switch (g_ui_state.current_page)
    {
        case PAGE_MAIN:  Draw_MainPage();  break;
        case PAGE_MOTOR: Draw_MotorPage(); break;
        case PAGE_GPS:   Draw_GPSPage();   break;
        case PAGE_PID:   Draw_PIDPage();   break;
        default:         Draw_MainPage();  break;
    }

    /* 4. 发送缓冲区 */
    u8g2_SendBuffer(&u8g2);
}

/* ================== 页面绘制函数实现 ================== */

static void Draw_MainPage(void)
{
    const char *items[] = {"1. Motor Speed", "2. GPS Status", "3. PID Config"};
    
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    u8g2_DrawStr(&u8g2, 0, 12, "Main Menu");
    u8g2_DrawHLine(&u8g2, 0, 14, 128);

    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    for (int i = 0; i < MAIN_MENU_ITEMS; i++) {
        /* 选中项反色显示或加 > */
        if (i == g_ui_state.cursor_index) {
            u8g2_DrawStr(&u8g2, 0, 30 + i * 14, ">"); 
        }
        u8g2_DrawStr(&u8g2, 10, 30 + i * 14, items[i]);
    }
}

static void Draw_MotorPage(void)
{
    char buf[32];
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    u8g2_DrawStr(&u8g2, 0, 12, "Motor Monitor");
    u8g2_DrawHLine(&u8g2, 0, 14, 128);

    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    
    /* 显示当前模式 */
    const char *mode_str = (g_robot_mode == MODE_MANUAL) ? "Mode: MANUAL" : "Mode: AUTO";
    u8g2_DrawStr(&u8g2, 0, 28, mode_str);

    snprintf(buf, sizeof(buf), "L: %.1f RPM", motor1.speed_rpm);
    u8g2_DrawStr(&u8g2, 0, 42, buf);
    
    snprintf(buf, sizeof(buf), "R: %.1f RPM", motor2.speed_rpm);
    u8g2_DrawStr(&u8g2, 0, 56, buf);
    
    // u8g2_DrawStr(&u8g2, 0, 62, "[Back: Key3]");
}

static void Draw_GPSPage(void)
{
    char buf[32];
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    u8g2_DrawStr(&u8g2, 0, 12, "GPS Info");
    u8g2_DrawHLine(&u8g2, 0, 14, 128);

    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    const char *fix_str = (gps_data.fixmode > 1) ? "FIX" : "NO";
    snprintf(buf, sizeof(buf), "St: %s Sat:%d", fix_str, gps_data.posslnum);
    u8g2_DrawStr(&u8g2, 0, 28, buf);

    snprintf(buf, sizeof(buf), "Lat:%.4f%c", gps_data.latitude, gps_data.nshemi ? gps_data.nshemi : 'N');
    u8g2_DrawStr(&u8g2, 0, 42, buf);
    
    snprintf(buf, sizeof(buf), "Lon:%.4f%c", gps_data.longitude, gps_data.ewhemi ? gps_data.ewhemi : 'E');
    u8g2_DrawStr(&u8g2, 0, 56, buf);
}

static void Draw_PIDPage(void)
{
    char buf[32];
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    u8g2_DrawStr(&u8g2, 0, 12, "PID Config");
    u8g2_DrawHLine(&u8g2, 0, 14, 128);

    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    
    /* 参数列表映射 */
    struct { const char* name; float *val; } params[] = {
        {"Dist P", &g_app_params.dist_kp},
        {"Dist I", &g_app_params.dist_ki},
        {"Dist D", &g_app_params.dist_kd},
        {"Ang  P", &g_app_params.angle_kp},
        {"Ang  I", &g_app_params.angle_ki},
        {"Ang  D", &g_app_params.angle_kd},
        {"SpdL P", &g_app_params.speed_L_kp},
        {"SpdL I", &g_app_params.speed_L_ki},
        {"SpdL D", &g_app_params.speed_L_kd},
        {"SpdR P", &g_app_params.speed_R_kp},
        {"SpdR I", &g_app_params.speed_R_ki},
        {"SpdR D", &g_app_params.speed_R_kd},
        {"[Save & Exit]", NULL}
    };

    /* 确保 scroll_offset 合法 */
    if (g_ui_state.scroll_offset > PID_MENU_ITEMS - PID_VIEW_ITEMS) 
        g_ui_state.scroll_offset = PID_MENU_ITEMS - PID_VIEW_ITEMS;
    
    int start_idx = g_ui_state.scroll_offset;

    for (int i = 0; i < PID_VIEW_ITEMS; i++) {
        int item_idx = start_idx + i;
        if (item_idx >= PID_MENU_ITEMS) break;

        int y = 28 + i * 12; //稍微紧凑一点以放下4行
        
        /* 绘制光标 */
        if (item_idx == g_ui_state.cursor_index) {
            u8g2_DrawStr(&u8g2, 0, y, ">");
        }
        
        /* 绘制参数名 */
        u8g2_DrawStr(&u8g2, 10, y, params[item_idx].name);
        
        /* 绘制参数值 */
        if (params[item_idx].val != NULL) {
            snprintf(buf, sizeof(buf), "%.2f", *params[item_idx].val);
            
            /* 编辑模式高亮 */
            if (g_ui_state.is_editing && item_idx == g_ui_state.cursor_index) {
                snprintf(buf, sizeof(buf), "[%.2f]", *params[item_idx].val);
            }
            u8g2_DrawStr(&u8g2, 70, y, buf);
        }
    }
}
