#include "application.h"


void Button_Task(void *argument)
{
    uint8_t i;
    uint16_t btn;
    Key_Init();
    for(;;)
    {
        Key_DebounceService_10ms();
        Key_Scand();
        for(i = 0; i < KeyIndex_Max; i++){
            if(KEY_GetDASClick(i)){
                btn = (i << 8)| 0x01;
                osMessageQueuePut(Button_QueueHandle, &btn, 0, osWaitForever);
                pm_api_refresh_idle();      /* 任意按键触发唤醒信号 */
            }
        }
        osDelay(pdMS_TO_TICKS(10));
    }
}
