/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "comp.h"
#include "cordic.h"
#include "dac.h"
#include "dma.h"
#include "fdcan.h"
#include "fmac.h"
#include "hrtim.h"
#include "i2c.h"
#include "quadspi.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*******************修复因为VBAT掉电导致LSE无法启动的问题**********************/
void RTC_Restart_Init(void)
{
    // === 【修正版】强制复位 Backup Domain（彻底解决 Errata 2.2.6）===
    // 只在真正掉电/上电复位时清一次（保留电池供电时的 RTC 时间）
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTCAPB);

    LL_PWR_EnableBkUpAccess();
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != MAGIC) {
        LL_RCC_ForceBackupDomainReset();
        LL_RCC_ReleaseBackupDomainReset();
        LL_RCC_ClearResetFlags();
        // 这里故意不写 magic！留给后面 rtc.c 设置时间后再写
    }
}

/***********************移动中断向量表到SRAM以提升速度*************************/
#define VECT_TAB_START  0x10007E00
#define VECT_TAB_SIZE   0x200U
extern uint32_t Load$$ER_IROM1$$Base; // 获取 Flash 中原始向量表地址

void RelocateVectorTable(void) {
    const uint32_t *pSrc  = &Load$$ER_IROM1$$Base;              // Flash 中的向量表起始
    uint32_t *pDest = (uint32_t *)VECT_TAB_START;         // CCM RAM 中的目标位置

    __disable_irq();
    /* 1. 将向量表从 Flash 拷贝到 CCM SRAM */
    for (int i = 0; i < VECT_TAB_SIZE / 4; i++) {
        pDest[i] = pSrc[i];
    }

    /* 2. 更新 VTOR 寄存器，告知 CPU 新的地址 */
    __DSB(); // 数据同步屏障
    SCB->VTOR = VECT_TAB_START;
    __DSB(); // 数据同步屏障
    __ISB();
    
    __enable_irq();
}

/**************************从120MHz超频至240MHz********************************/
//不影响外设时钟
void STM32G474_OverClock(void)
{
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1) {}
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI) {}
        
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_5) { }
    
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_4);
    
    LL_RCC_PLL_Disable();
    while (LL_RCC_PLL_IsReady() == 1) {}
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_ConfigDomain_ADC(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLP_DIV_8);
    LL_RCC_PLL_EnableDomain_SYS();
    LL_RCC_PLL_Enable();
    while(LL_RCC_PLL_IsReady() != 1){}

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL){}
        
    for (__IO uint32_t i = (170 >> 1); i !=0; i--);
    
    LL_SetSystemCoreClock(240000000);
    if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK) Error_Handler();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
    //避免LSE卡住
    RTC_Restart_Init();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
    //配置错误中断重定向
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;
    //重定向中断向量表
    RelocateVectorTable();
    //手动超频到240MHz
    //STM32G474_OverClock();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CORDIC_Init();
  MX_FMAC_Init();
  MX_HRTIM1_Init();
  MX_RTC_Init();
  MX_TIM4_Init();
  MX_DAC1_Init();
  MX_DAC2_Init();
  MX_FDCAN1_Init();
  MX_SPI4_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC4_Init();
  MX_COMP1_Init();
  MX_COMP2_Init();
  MX_COMP3_Init();
  MX_COMP4_Init();
  MX_COMP5_Init();
  MX_COMP6_Init();
  MX_COMP7_Init();
  MX_DAC3_Init();
  MX_DAC4_Init();
  MX_I2C1_Init();
  MX_QUADSPI1_Init();
  MX_UART4_Init();
  MX_ADC5_Init();
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_3)
  {
  }
  LL_PWR_EnableRange1BoostMode();
  LL_RCC_HSE_Enable();
   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {
  }

  LL_PWR_EnableBkUpAccess();
  LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_MEDIUMLOW);
  LL_RCC_LSE_Enable();
   /* Wait till LSE is ready */
  while(LL_RCC_LSE_IsReady() != 1)
  {
  }

  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 20, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_ConfigDomain_ADC(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 20, LL_RCC_PLLP_DIV_4);
  LL_RCC_PLL_EnableDomain_SYS();
  LL_RCC_PLL_EnableDomain_ADC();
  LL_RCC_PLL_Enable();
   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Insure 1us transition state at intermediate medium speed clock*/
  for (__IO uint32_t i = (170 >> 1); i !=0; i--);

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_SetSystemCoreClock(120000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
