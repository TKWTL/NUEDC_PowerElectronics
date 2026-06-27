/*菜单驱动任务函数
 *负责U8G2与UI的初始化与运行
 *
 *
 */
#include "application.h"

ui_t ui;    
u8g2_t u8g2;

void UI_Task(void *argument)
{
    //初始化显示器（含 u8g2 设置、硬件初始化、唤醒、清屏、对比度）
    diapInit();
    
    MiaoUi_Setup(&ui);
    
    for(;;)
    {
        ui_loop(&ui);
        osDelay(pdMS_TO_TICKS(25));
    }
}