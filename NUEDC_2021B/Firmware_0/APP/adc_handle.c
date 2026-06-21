/*******************************************************************************
负责数字电源环路及外设的进一步配置和初始化
之后循环进行时间更新和低速ADC采样，周期400ms
*******************************************************************************/
#include "application.h"


//在调度器启动后才开始初始化开关电源相关步骤
void ADC_Task(void *argument)
{
    Protection_Init1(&ui);
    Protection_Init2(&ui);
    
    //ADC_Init();
    
    Timer_Init(&ui);
    
    for(;;)
    {
        ADC_SampleandFilter();
        RTC_UpdateTime();//时间也放在此处更新
        //USART_SendString(&pc_uart, "114514\n");
        //USART_SendString(&bt_uart, "1919810\n");
    }
}