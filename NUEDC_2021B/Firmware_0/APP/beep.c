#include "application.h"

void Beep_Task(void *argument){
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
    while(1){
        Beep_Process();
    }
}