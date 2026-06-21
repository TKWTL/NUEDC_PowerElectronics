#include "display_effects.h"

/*创建一个逐渐成型的弹窗
 *
 */
void PopUp_Show(uint8_t x, uint8_t y, uint8_t w, uint8_t h){
    uint8_t bx = x+ w, by = y+ h;
    uint8_t i = w/ 2,j = h/ 2;
    uint8_t istep = w/ 8, jstep = h/ 8;
    uint8_t bgC = ui.bgColor;
    
    Disp_SetDrawColor(&bgC);
    Disp_DrawBox(x, y, w, h);
    bgC = bgC^ 0x01;
    Disp_SetDrawColor(&bgC);
    while(1){
        i += istep;
        if(i > w) i = w;
        j += jstep;
        if(j > h) j = h;
        if(i >= w && j >= h) break;
        Disp_DrawLine(x, y, x+ i, y);
        Disp_DrawLine(x, y, x, y+ j);
        Disp_DrawLine(bx- i, by, bx, by);
        Disp_DrawLine(bx, by- j, bx, by);
        Disp_SendBuffer();
    }
}