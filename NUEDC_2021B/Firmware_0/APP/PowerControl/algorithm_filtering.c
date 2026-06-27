#include "algorithm_filtering.h"
#include <math.h>
#include <armdsp.h>

/*一阶低通滤波*/
//off_freq单位为rad/s
//1/(1+TS)  off_freq为1/T
inline void LpFilter(ST_LPF *lpf)
{
	float fir_a = 1 / (1 + lpf->off_freq * lpf->samp_tim);
	lpf->out = fir_a * lpf->preout + (1 - fir_a) * lpf->in;
	lpf->preout = lpf->out;
}

/*仪表显示专用滤波算法
 *既保证小变化下的稳定输出，也保证了快速变化时的即时响应
 */
float VarFilter(FilterVar_t *pFilter, float filter_input){
    //累计误差
    pFilter->integral_error += filter_input- pFilter->display_value;
    //定时更新
    pFilter->filter_ticks++;
    if(pFilter->filter_ticks > pFilter->update_ticks){
        pFilter->filter_ticks = 0;
        if(fabsf(pFilter->integral_error) > pFilter->UpdateThreshold){
            pFilter->display_value += pFilter->integral_error/ pFilter->update_ticks;
            pFilter->integral_error = 0;
        }
    }
    //变化量过大时立刻更新
    if(fabsf(pFilter->integral_error) > pFilter->UpdateThreshold* 20){
        pFilter->display_value = filter_input;
        pFilter->integral_error = 0;
    }
    
    return pFilter->display_value;
}

/*******************************************************************************
 * 滑动窗口有效值
 ******************************************************************************/
void SlideRms_Init(SlideRms_t *p, float *buf, uint16_t len)
{
    p->buf    = buf;
    p->len    = len;
    p->ptr    = 0;
    p->sum_sq = 0.0f;
    p->rms    = 0.0f;
}

void SlideRms_Store(SlideRms_t *p, float val)
{
    //减去最旧值
    p->sum_sq -= p->buf[p->ptr] * p->buf[p->ptr];
    //写入新值
    p->buf[p->ptr] = val;
    //加上新值平方
    p->sum_sq += val * val;
    //指针前进
    p->ptr++;
    if(p->ptr >= p->len) p->ptr = 0;
}

void SlideRms_Update(SlideRms_t *p)
{
    p->rms = sqrtf(p->sum_sq * (1.0f / (float)p->len));
}

float SlideRms_ReadDelayed(const SlideRms_t *p, uint16_t delay)
{
    int16_t idx = (int16_t)p->ptr - 1 - (int16_t)delay;
    while(idx < 0) idx += (int16_t)p->len;
    return p->buf[idx];
}

//PF滞后延迟查表：pf_x100 = 100~80 → 对应延迟采样点数（@30kHz/50Hz）
#define PF_TABLE_LEN    21
static const uint8_t pf_lag_tab[PF_TABLE_LEN] = {
     0, 14, 19, 23, 27, 30, 33, 36, 38, 41, 43,
    45, 47, 49, 51, 53, 55, 57, 58, 60, 61
};

uint8_t pf_to_lag(uint8_t pf_x100)
{
    if(pf_x100 > 100) pf_x100 = 100;
    if(pf_x100 < 80)  pf_x100 = 80;
    return pf_lag_tab[100 - pf_x100];
}
