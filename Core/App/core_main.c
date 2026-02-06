#include "core_main.h"

extern TB6612_Motor_t motorL;
extern TB6612_Motor_t motorR;
extern Encoder_t motor1;
extern Encoder_t motor2;

void Core_Main_Init(void)
{
	//tb6612³õÊ¼»¯
	Motor_init();
	Encoder_Init();
}

void Core_Main_Master(void)
{
	TB6612_Motor_SetSpeed(&motorL, 2100);
	TB6612_Motor_SetSpeed(&motorR, 2100);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM14) {
        Encoder_Update_Speed(&motor1, &motor2);
    }
}
