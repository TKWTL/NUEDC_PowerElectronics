#ifndef __SYSTEM_OPERATION_H__
#define __SYSTEM_OPERATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "services.h"
    
//系统复位函数
void Manual_Reset(ui_t *ui);

typedef enum {
    BeepID_Short,
    BeepID_Alert,
    
    BeepID_Max
}BeepID_t;
    
extern uint8_t ClickBeep;//按键按下时发出声音
extern uint8_t BeepMute;//静音，无论如何蜂鸣器不会发出声音
    
//向蜂鸣器队列发送一条消息
void Beep_Send(uint8_t index);

void Beep_Process(void);
    
/***********************以下函数须在ui_conf.c中调用****************************/
//创建系统设置模块所需的参数
void Create_SystemCtrl_Parameters(ui_t *ui);
//将系统设置模块的对象添加到菜单中
void Add_SystemCtrl_Items(ui_page_t *ParentPage);
    
#ifdef __cplusplus
}
#endif

#endif
 