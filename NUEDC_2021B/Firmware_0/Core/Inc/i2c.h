/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_I2C1_Init(void);

/* USER CODE BEGIN Prototypes */
    
void I2C_Transmit(uint8_t addr, uint8_t* p_buf, uint16_t len);
void I2C_Receive(uint8_t addr, uint8_t* p_buf, uint16_t len);//只有接收功能

void I2C_RegWrite(uint8_t addr, uint8_t reg, uint8_t* p_buf, uint16_t len);//带寄存器设置的发送函数
void I2C_RegRead(uint8_t addr, uint8_t reg, uint8_t * p_buf, uint16_t len);//带寄存器设置的读取函数

uint8_t I2C_IsReady(uint8_t addr);//确认地址存在函数
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

