#ifndef __CONV_LOOP_H__
#define __CONV_LOOP_H__

#include "services.h"
#include "algorithm_filtering.h"

//相电压滑动窗口长度（300 = 二分之一周期 @30kHz/50Hz）
#define V_RMS_LEN       300U

void thph_open_loop(float U_ref,uint16_t index);
void tri_PFC_Loop(uint8_t pf_x100);

//Buck变换器控制（电流内环，外层电压PI后续添加）
void BUCK_loop(float Vref, float I_lim);

//相电压与有效值（供外部模块使用）
extern float Vab, Vbc;
extern float Va_calc, Vb_calc, Vc_calc;
extern float Va_rms, Vc_rms;
extern SlideRms_t Va_rms_inst, Vc_rms_inst;
extern float Va_buf[], Vc_buf[];

#endif
