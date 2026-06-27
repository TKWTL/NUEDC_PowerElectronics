/*******************************************************************************
负责数字电源环路及外设的进一步配置和初始化
之后循环进行时间更新和低速ADC采样，周期400ms
*******************************************************************************/
#include "application.h"

extern ST_PID BUCK_V_PID;
extern ST_PID PFC_V_PID;
extern ST_PR PFC_PR_A, PFC_PR_C;
extern SlideRms_t Va_rms_inst, Vc_rms_inst;
extern float Va_buf[], Vc_buf[];

//在调度器启动后才开始初始化开关电源相关步骤
void ADC_Task(void *argument)
{
    Protection_Init1(&ui);
    Protection_Init2(&ui);
    
    ADC_Init();
    
    //初始化滑动窗口有效值
    SlideRms_Init(&Va_rms_inst, Va_buf, V_RMS_LEN);
    SlideRms_Init(&Vc_rms_inst, Vc_buf, V_RMS_LEN);
    
    //初始化PR差分方程系数，必须在环路开始前执行
    PR_Init(&PFC_PR_A);
    PR_Init(&PFC_PR_C);
    //Buck电压外环PI初始化
    PID_Init(&BUCK_V_PID, 0.5f, 64.0f, 0.0f, CTRL_DT, 0.1f);
    //PFC母线电压外环PI初始化
    PID_Init(&PFC_V_PID, 0.5f, 64.0f, 0.0f, CTRL_DT, 0.1f);
    
    Timer_Init(&ui);
    
    for(;;)
    {
        ADC_SampleandFilter();
        RTC_UpdateTime();//时间也放在此处更新
        //USART_SendString(&pc_uart, "114514\n");
        //USART_SendString(&bt_uart, "1919810\n");
    }
}