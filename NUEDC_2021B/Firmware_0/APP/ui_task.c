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
    // 初始化U8g2库，为OLED显示做准备
    u8g2_Setup_sh1106_128x64_noname_f(&u8g2, UI_DEFAULT_ROTATION, u8x8_byte_4wire_hw_spi, u8x8_gpio_and_delay);
    u8g2_InitDisplay(&u8g2);
    Disp_SetPowerSave(0);
    Disp_ClearBuffer();
    Disp_SetContrast2(Contrast);
    
    MiaoUi_Setup(&ui);
    
    for(;;)
    {
        ui_loop(&ui);
        osDelay(pdMS_TO_TICKS(25));
    }
}