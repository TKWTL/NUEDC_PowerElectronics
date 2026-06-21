#ifndef __CONV_LOOP_H__
#define __CONV_LOOP_H__

#include "services.h"

void thph_open_loop(float U_ref,uint16_t index);
void thph_loop(float U_ref, float sin0, float sin240);
void tri_PFC_Loop(float I_ref, float sin0, float sin120, float sin240);

#endif
