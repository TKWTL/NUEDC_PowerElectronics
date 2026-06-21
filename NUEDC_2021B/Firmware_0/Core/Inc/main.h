/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

#include "stm32g4xx_ll_cordic.h"
#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_ll_fmac.h"
#include "stm32g4xx_ll_hrtim.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_crs.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
//RTOS接口
#include "cmsis_os2.h"

//记得在此处包含你用到的外设的头文件
#include "adc.h"
#include "dac.h"
#include "i2c.h"
#include "spi.h"
#include "quadspi.h"
#include "fdcan.h"
#include "hrtim.h"
#include "tim.h"
#include "rtc.h"
#include "cordic.h"
#include "fmac.h"
#include "usart.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define INITIAL_HRTIM_DEADTIME 200
#define INITIAL_HRTIM_PERIOD 32000
#define BT_EN_Pin LL_GPIO_PIN_4
#define BT_EN_GPIO_Port GPIOE
#define D_C_Pin LL_GPIO_PIN_5
#define D_C_GPIO_Port GPIOE
#define A_Pin LL_GPIO_PIN_13
#define A_GPIO_Port GPIOC
#define TEST0_Pin LL_GPIO_PIN_9
#define TEST0_GPIO_Port GPIOF
#define TEST1_Pin LL_GPIO_PIN_10
#define TEST1_GPIO_Port GPIOF
#define INT_Pin LL_GPIO_PIN_2
#define INT_GPIO_Port GPIOF
#define F_Pin LL_GPIO_PIN_2
#define F_GPIO_Port GPIOD
#define E_Pin LL_GPIO_PIN_3
#define E_GPIO_Port GPIOD
#define D_Pin LL_GPIO_PIN_4
#define D_GPIO_Port GPIOD
#define C_Pin LL_GPIO_PIN_5
#define C_GPIO_Port GPIOD
#define B_Pin LL_GPIO_PIN_6
#define B_GPIO_Port GPIOD
#define BT_STA_Pin LL_GPIO_PIN_7
#define BT_STA_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */
//用于链接地址分配
#define RAMFUNC __attribute__((section(".ccm_text"), used))     //高频执行的程序，放在CCM SRAM
#define RAMRO   __attribute__((section(".ram_ro"), used))       //常量池放在SRAM2
#define RAMVAR  __attribute__((section(".ram_data"), used))     //快速变量放在SRAM2

//魔数，用于校验
#define MAGIC 0x01919810U

//ADC参考电压值
#define ADC_VREF    3.3f
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
