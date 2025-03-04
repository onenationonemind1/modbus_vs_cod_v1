#include "main.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

uint8_t g_tick_sec_1 = DISABLE;
uint16_t g_tick_battery = 0;

uint8_t g_timer_ms_1     = DISABLE;
uint8_t g_timer_ms_100   = DISABLE;
uint8_t g_timer_ms_1000  = DISABLE;


void timer_init(void)
{
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start_IT(&htim3);
}

void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim)
{
    // 1 ms
    if (htim->Instance == TIM3)
    {
        if (g_timer_ms_1 == DISABLE)
        {
            g_timer_ms_1 = ENABLE;
        }
    }
    // 1 Sec Timer
    else if (htim->Instance == TIM2)
    {
        g_tick_sec_1 = ENABLE;
    }
}
