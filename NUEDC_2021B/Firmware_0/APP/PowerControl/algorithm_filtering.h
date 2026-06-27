#ifndef __ALGORITHM_FILTERING_H__
#define __ALGORITHM_FILTERING_H__

#ifdef __cplusplus
extern "C" {
#endif    

#include <math.h>
#include <armdsp.h>
#include <stdint.h>
    
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
    //内部变量
    float display_value;
    float integral_error;
    int filter_ticks;
}FilterVar_t;//仪表显示用滤波算法的参数结构体

float VarFilter(FilterVar_t *pFilter, float filter_input);

/*******************************************************************************
 * 滑动窗口有效值计算句柄
 * 维护一个环形缓冲区，使用平方和累加器实现 O(1) 更新
 ******************************************************************************/
typedef struct {
    float *buf;         // 环形缓冲区指针（外部提供存储）
    uint16_t len;       // 缓冲区长度
    uint16_t ptr;       // 当前写入位置
    float sum_sq;       // 平方和累加器
    float rms;          // 当前有效值
} SlideRms_t;

void SlideRms_Init(SlideRms_t *p, float *buf, uint16_t len);
void SlideRms_Store(SlideRms_t *p, float val);
void SlideRms_Update(SlideRms_t *p);
float SlideRms_ReadDelayed(const SlideRms_t *p, uint16_t delay);

//PF滞后延迟查表（pf_x100 = PF×100, 80~100）
uint8_t pf_to_lag(uint8_t pf_x100);

#ifdef __cplusplus
}
#endif

#endif // __FILTERING_ALGORITHM_H__
