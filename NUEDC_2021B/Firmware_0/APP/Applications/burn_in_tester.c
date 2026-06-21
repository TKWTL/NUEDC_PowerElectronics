#include "burn_in_tester.h"

//烧屏测试实现函数
void Burn_in_Tester(ui_t *ui){
    uint8_t prtC = 1;
    int show_time = 0;//展示时间计数器
    int declaration_time = 500;//说明展示时间
    
    PopUp_Show(1, 1, 126, 62);
    Disp_SetFont(font_menu_main_h12w6);
    Disp_DrawStr(4, 16, "Burn-in test is for");
    Disp_DrawStr(4, 30, "find out decaying of");
    Disp_DrawStr(4, 44, "OLED screen after");
    Disp_DrawStr(4, 58, "long time lightup.");
    Disp_SendBuffer();
    while(1){
        if(indevScan() != UI_ACTION_NONE) break;
        if(show_time >= declaration_time) break;
        osDelay(pdMS_TO_TICKS(10));
        show_time++;
    }
    
    //屏幕涂白来检测烧屏
    Disp_SetDrawColor(&prtC);
    Disp_DrawBox(0, 0, UI_HOR_RES, UI_VER_RES);
    Disp_SendBuffer();
    
    while(1){
        if(indevScan() != UI_ACTION_NONE) break;
        osDelay(pdMS_TO_TICKS(10));
    }
}