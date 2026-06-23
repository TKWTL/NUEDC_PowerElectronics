#ifndef __DASHBOARD_H__
#define __DASHBOARD_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "services.h"

typedef struct {
    float *pvalue;  //要修改的变量指针
    float step;     //步进值
    float max;      //上限值
    float min;      //下限值
    float *pmaxlim; //上限变量指针，不用时初始化为NULL
    float *pminlim; //下限变量指针，不用时初始化为NULL
}SetVar_t;//参数设置结构体定义

extern float VO_Set, IO_Lim, PF_Set;

void DashBoard(ui_t *ui);//供ui_conf调用的显示函数

#ifdef __cplusplus
}
#endif

#endif
