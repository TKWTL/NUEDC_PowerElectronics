/*
 *重新封装了HAL库的时间相关格式
 *避免在默认的packed数据结构下M0+的非对齐访问出现Hardfault
 */
#ifndef __TIME_H__
#define __TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "services.h"

typedef struct{
    int __attribute__((aligned(4))) Seconds;
    int __attribute__((aligned(4))) Minutes;
    int __attribute__((aligned(4))) Hours;
    int __attribute__((aligned(4))) WeekDay;
    int __attribute__((aligned(4))) Date;
    int __attribute__((aligned(4))) Month;
    int __attribute__((aligned(4))) Year;
} RTC_TimeTypeDef_Aligned;//值全部4字节对齐以避免M0+内核的Hardfault错误
    
extern RTC_TimeTypeDef_Aligned sTime;    

extern RTC_TimeTypeDef RTC_Time;
extern RTC_DateTypeDef RTC_Date;
    
void RTC_UpdateTime(void);//从RTC更新时间
void SetTime(ui_t *ui);//将时间更新到RTC
    
/***********************以下函数须在ui_conf.c中调用****************************/
//创建显示屏模块所需的参数
void Create_TimeSet_Parameters(ui_t *ui);
//将显示屏模块的对象添加到菜单中
void Add_TimeSet_Items(ui_page_t *ParentPage);

#ifdef __cplusplus
}
#endif

#endif
 