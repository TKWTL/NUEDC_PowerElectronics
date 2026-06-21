/*显示仪表盘*/
#include "dashboard.h"
#include "algorithm_filtering.h"
#include <stdio.h>


//需要设置的变量 和对应结构体
float VO_Set = 32.0f;//输出电压
float IO_Set = 2.0f;//输出电压限值（仅主机）

SetVar_t SetVar[] = {
    {&VO_Set,   1.0f, 48.0f,  8.0f,   NULL,   NULL},//设定输出电压
    {&IO_Set,   0.1f, 4.0f,   0.5f,   NULL,   NULL}//设定输出电流
};//{&(Name),   Step, Max,    Min,    MaxLim, MinLim}

void VarInc(uint8_t index){
    if(SetVar[index].pvalue == NULL) return;
    *SetVar[index].pvalue += SetVar[index].step;
    if(*SetVar[index].pvalue > SetVar[index].max) *SetVar[index].pvalue = SetVar[index].max;
    if(SetVar[index].pmaxlim != NULL && *SetVar[index].pvalue > *SetVar[index].pmaxlim) *SetVar[index].pvalue = *SetVar[index].pmaxlim;
}
void VarDec(uint8_t index){
    if(SetVar[index].pvalue == NULL) return;
    *SetVar[index].pvalue -= SetVar[index].step;
    if(*SetVar[index].pvalue < SetVar[index].min) *SetVar[index].pvalue = SetVar[index].min;
    if(SetVar[index].pminlim != NULL && *SetVar[index].pvalue < *SetVar[index].pminlim) *SetVar[index].pvalue = *SetVar[index].pminlim;
}
extern float vrms_out;
FilterVar_t 
    POUT_Filter = {1.2f, 500},
    VBUS_Filter = {0.2f, 50},
    UV_A_Filter = {0.2f, 20};

//需要显示的变量
//输出电压有效值，输出电流有效值，输入电压，日期时间，测量到的输出频率
extern uint8_t LoopMode;
//供ui_conf调用的显示函数
void DashBoard(ui_t *ui){
    char buf[20];
    uint8_t color;
    int16_t value = 128, value2;
    static uint8_t var_switch = 0;//菜单选中指示
    
    while(1){
        //两种颜色下的清屏
        Disp_SetDrawColor(&ui->bgColor);
        Disp_DrawBox(0, 0, UI_HOR_RES, UI_VER_RES);
        color = ui->bgColor ? 0 : 1;
        Disp_SetDrawColor(&color);
        //入场动画效果
        value = (int16_t)UI_Animation(68, (float)value, &ui->animation.optionbarPos_ani);
        //标题
        Disp_SetFont(font_menu_main_h12w6);
        if(LoopMode) Disp_DrawStr(69-value, 10, "-Feedbacker-");
        else Disp_DrawStr(69-value, 10, "-Inverter-");
            
        
        //VL Set
        sprintf(buf, "VOSet:%2.1fV", VO_Set);
        Disp_DrawStr(69-value, 36, buf);
        //IO Set
        sprintf(buf, "IO Set:%1.1fA", IO_Set);
        Disp_DrawStr(69-value, 49, buf);
        
        //VH
        sprintf(buf, "Po:%.2fW", VarFilter(&POUT_Filter, ReadControlVar(&WV_INV_t)* ReadControlVar(&IV_INV_t)- (ReadControlVar(&UV_INV_t)- ReadControlVar(&WV_INV_t))* ReadControlVar(&IU_INV_t)));
        Disp_DrawStr(value+ 1, 23, buf);
        sprintf(buf, "BUS:%2.2fV", VarFilter(&VBUS_Filter, ReadControlVar(&VBUS_t)));
        Disp_DrawStr(value+ 1, 36, buf);
        
        sprintf(buf, "UV:%.2fV", VarFilter(&UV_A_Filter, vrms_out));
        Disp_DrawStr(value+ 1, 49, buf);
        /*sprintf(buf, "WV:%1.3fV", VarFilter(&WV_I_Filter, ReadControlVar(&VBUS_t)));
        Disp_DrawStr(value+ 1, 62, buf);
        sprintf(buf, "IW:%1.3fA", VarFilter(&IW_R_Filter, ReadControlVar(&VBUS_t)));
        Disp_DrawStr(69-value, 62, buf);*/
        
        
        Disp_SetFont(font_home_h6w4);
        //时间
        sprintf(buf, "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
        Disp_DrawStr(value+ 24, 6, buf);
        //日期
        sprintf(buf, "20%02d/%02d/%02d", sTime.Year, sTime.Month, sTime.Date);
        Disp_DrawStr(value+ 20, 12, buf);
        
        
        //选择框
        color = 2;
        Disp_SetDrawColor(&color);
        value2 = (int16_t)UI_Animation(13* var_switch+ 27, (float)value2, &ui->animation.cursor_ani);
        Disp_DrawRBox(0, value2, 67, 10, 0);
        //启动框
        if(g_conv_state == CONV_RUN){
            if(LoopMode) Disp_DrawRBox(0, 0, 73, 11, 0);//W=6*12+1=73
            else Disp_DrawRBox(0, 0, 61, 11, 0);
        }
        //横线
        color = ui->bgColor ? 0 : 1;
        Disp_SetDrawColor(&color);
        Disp_DrawLine(0, 13, UI_HOR_RES, 13);
        //Disp_DrawLine(0, 51, UI_HOR_RES, 51);
        //发送到屏幕
        Disp_SendBuffer();
        
        //按键控制
        switch(indevScan()){
            case UI_ACTION_BACK:
                ui->action = UI_ACTION_ENTER;
                return;
            case UI_ACTION_ENTER:
                if(g_conv_state == CONV_STOP) g_conv_state = CONV_RUN;
                else g_conv_state = CONV_STOP;
                break;
            case UI_ACTION_PLUS:
                if(var_switch == 0) var_switch = 1;
                else var_switch--;
                break;
            case UI_ACTION_MINUS:
                if(var_switch >= 1) var_switch = 0;
                else var_switch++;
                break;
            case UI_ACTION_UP:
                VarDec(var_switch);
                break;
            case UI_ACTION_DOWN:
                VarInc(var_switch);
                break;
            default:
                break;
        }
        
        osDelay(pdMS_TO_TICKS(20));
    }
}
