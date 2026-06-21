/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    cordic.c
  * @brief   This file provides code for the configuration
  *          of the CORDIC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "cordic.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* CORDIC init function */
void MX_CORDIC_Init(void)
{

  /* USER CODE BEGIN CORDIC_Init 0 */

  /* USER CODE END CORDIC_Init 0 */

  /* Peripheral clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CORDIC);

  /* USER CODE BEGIN CORDIC_Init 1 */
    //配置：COSINE + 输出2个（cos then sin），q1.31，最高精度 6 cycles :contentReference[oaicite:4]{index=4}
    LL_CORDIC_Config(CORDIC,
        LL_CORDIC_FUNCTION_COSINE,
        LL_CORDIC_PRECISION_6CYCLES,
        LL_CORDIC_SCALE_0,
        LL_CORDIC_NBWRITE_1,          // 只写 angle；modulus 用复位后的默认 1
        LL_CORDIC_NBREAD_2,           // 读2次：先 cos 再 sin
        LL_CORDIC_INSIZE_32BITS,
        LL_CORDIC_OUTSIZE_32BITS);
  /* USER CODE END CORDIC_Init 1 */

  /* nothing else to be configured */

  /* USER CODE BEGIN CORDIC_Init 2 */

  /* USER CODE END CORDIC_Init 2 */

}

/* USER CODE BEGIN 1 */
// 预缩放常量：将 Q31 转 float 的 (1/2^31) 直接整合进系数
#define K_Q_TO_F      (4.6566128730773926e-10f) // 1/2147483648
#define K_HALF_Q      (-0.5f * K_Q_TO_F)
#define K_SQRT3_2_Q   (0.8660254037844386f * K_Q_TO_F)

inline void CORDIC_Write(uint32_t PhaseWord){
    /* 利用强制类型转换：
       uint32_t 的 [0, 0x7FFFFFFF] 映射为 int32_t 的 [0, 2147483647]
       uint32_t 的 [0x80000000, 0xFFFFFFFF] 映射为 int32_t 的 [-2147483648, -1]
       这完美符合 CORDIC 要求的 q31 格式输入范围 [-1, 1)
    */
    LL_CORDIC_WriteData(CORDIC, (uint32_t)(int32_t)PhaseWord);
}

float CORDIC_sin0, CORDIC_cos_scaled, CORDIC_common;

inline float CORDIC_Read() {
    int32_t res_cos = (int32_t)LL_CORDIC_ReadData(CORDIC);
    CORDIC_cos_scaled = (float)res_cos * K_SQRT3_2_Q; // 预乘 sqrt(3)/2
    int32_t res_sin = (int32_t)LL_CORDIC_ReadData(CORDIC);

    CORDIC_common = (float)res_sin * K_HALF_Q;       // 预乘 -0.5
    return (float)res_sin * K_Q_TO_F; 
}

inline float CORDIC_Get_sin120(){
    return CORDIC_common + CORDIC_cos_scaled;
}

inline float CORDIC_Get_sin240(){
    return CORDIC_common - CORDIC_cos_scaled;
}
/* USER CODE END 1 */

