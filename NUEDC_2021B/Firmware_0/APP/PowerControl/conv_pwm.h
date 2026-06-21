#ifndef __CONV_PWM_H__
#define __CONV_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "services.h"
    
extern int SwitchingFrequency;
    
//启动定时器与频率基准
void Timer_Init(ui_t *ui);
    
//设置开关频率
void SetFrequency(ui_t *ui);
    
//设置占空比，根据周期和输入的百分比值，带占空比限幅
void SetDuty_Inv(float Du, float Dv, float Dw);
void SetDuty_Rec(float Du, float Dv, float Dw);

//打开PWM
void PWM_Start_Inv(void);
void PWM_Start_Rec(void);
//关闭PWM
void PWM_Stop(void);
//SVPWM
void SetDuty_SV(uint8_t Timer_Set,float offset,float Du,float Dv,float Dw);

/***********************以下函数须在ui_conf.c中调用****************************/
//创建PWM设置模块所需的参数
void Create_PWM_Parameters(ui_t *ui);
//将PWM设置模块的对象添加到菜单中
void Add_PWM_Items(ui_page_t *ParentPage);

#ifdef __cplusplus
}
#endif

#endif
