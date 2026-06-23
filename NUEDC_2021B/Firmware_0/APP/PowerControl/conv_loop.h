#ifndef __CONV_LOOP_H__
#define __CONV_LOOP_H__

#include "services.h"

void thph_open_loop(float U_ref,uint16_t index);
void tri_PFC_Loop(float I_ref, float sin0, float sin120, float sin240);

//Buck变换器控制（电流内环，外层电压PI后续添加）
void BUCK_loop(float Vref, float I_lim);

#endif
