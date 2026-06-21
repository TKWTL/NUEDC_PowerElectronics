#ifndef __ALGORITHM_FILTERING_H__
#define __ALGORITHM_FILTERING_H__

#ifdef __cplusplus
extern "C" {
#endif    

#include "algorithm_filtering.h"
#include <math.h>
#include <armdsp.h>
#include "conv_adc.h"
    
typedef struct
{
	float preout;
	float out;
	float in;
	float off_freq;
	float samp_tim;
}ST_LPF;

extern void LpFilter(ST_LPF *lpf);


typedef struct {
    //用于设置，需要初始化
    float UpdateThreshold;
    int update_ticks;
    //PowerControlVar_t *pVar;
    //内部变量
    float display_value;
    float integral_error;
    int filter_ticks;
}FilterVar_t;//仪表显示用滤波算法的参数结构体

float VarFilter(FilterVar_t *pFilter, float filter_input);//

#ifdef __cplusplus
}
#endif

#endif // __FILTERING_ALGORITHM_H__
