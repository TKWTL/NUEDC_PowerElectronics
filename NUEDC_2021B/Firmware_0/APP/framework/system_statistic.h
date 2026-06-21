
#ifndef __SYSTEM_STATISTIC_H__
#define __SYSTEM_STATISTIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "services.h"

extern float Tjunction;     //MCU结温，单位摄氏度    
extern float Vbattery;      //电池电压
extern float Vintref;       //内部参考电压值
    
void ADC_SampleandFilter(void);
    
/***********************以下函数须在ui_conf.c中调用****************************/
//创建系统参数模块所需的参数
void Create_Statistics_Parameters(ui_t *ui);
//将系统参数模块的对象添加到菜单中
void Add_Statistics_Items(ui_page_t *ParentPage);

#ifdef __cplusplus
}
#endif

#endif
