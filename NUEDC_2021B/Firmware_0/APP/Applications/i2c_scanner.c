/*实现i2c总线上设备的扫描
 *每5s扫描一次，确认键立刻重新扫描，返回键则退出
 *目前不支持修改扫描速率与扫描的I2C总线，仅适用于12864屏幕
 *字体使用font_h24w12以显示名称，font_home_h6w4方便全屏显示扫描结果
 *依赖外部提供的HAL_I2C_IsDeviceReady()提供地址扫描功能
 *
 *TKWTL 2025/04/22
 */
#include "application.h"

void I2C_Scanner(ui_t *ui){
    uint8_t i = 0;
    uint8_t color;
    char buf[16];

    //两种颜色下的清屏
    Disp_SetDrawColor(&ui->bgColor);
    Disp_DrawBox(0, 0, UI_HOR_RES, UI_VER_RES);
    color = ui->bgColor^ 0x01;
    Disp_SetDrawColor(&color);
    Disp_SetFont(font_h24w12);
    Disp_DrawStr(17, 24, "I2C");
    Disp_DrawStr(17, 50, "Scanner");

    Disp_SendBuffer();
    osDelay(pdMS_TO_TICKS(1500));
    
    Disp_SetFont(font_home_h6w4);
    while(1){
        //两种颜色下的清屏
        Disp_SetDrawColor(&ui->bgColor);
        Disp_DrawBox(0, 0, UI_HOR_RES, UI_VER_RES);
        color = ui->bgColor^ 0x01;
        Disp_SetDrawColor(&color);
        //扫描
        for(i = 0; i < 128; i++){
            if(I2C_IsReady(i<<1)){
                if(i < 0x10) sprintf(buf, "0%x", i);
                else sprintf(buf, "%x", i);
                Disp_DrawStr(8*(i%16), 8*(i/16+1), buf);
            }
            else Disp_DrawStr(8*(i%16), 8*(i/16+1), "--");
            
            Disp_SendBuffer();
            osDelay(pdMS_TO_TICKS(10));
            
            switch(indevScan()){
                case UI_ACTION_BACK:
                    ui->action = UI_ACTION_ENTER;
                    return;
                default:
                    break;
            }
        }
        //延时显示
        for(i = 0; i < 50; i++){
            switch(indevScan()){
                case UI_ACTION_ENTER:
                    i = 50;
                    break;
                case UI_ACTION_BACK:
                    ui->action = UI_ACTION_ENTER;
                    return;
                default:
                    break;
            }
            osDelay(pdMS_TO_TICKS(100));
        }
    }
}