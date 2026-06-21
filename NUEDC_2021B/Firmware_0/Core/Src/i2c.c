/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10A077A8;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */
    LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PA15     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PA15     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

    /* I2C1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
TaskHandle_t i2c_wait_task = NULL;   // 当前等待I2C完成的任务句柄

extern SemaphoreHandle_t mutex_i2c_Handle;   //I2C互斥锁

//发送函数
void I2C_Transmit(uint8_t addr, uint8_t* p_buf, uint16_t len){
    //xSemaphoreTake(mutex_i2c_handle, portMAX_DELAY);
    i2c_wait_task = xTaskGetCurrentTaskHandle();
    (void)ulTaskNotifyTake(pdTRUE, 0);
    HAL_I2C_Master_Transmit_IT(&hi2c1, addr, p_buf, len);
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    i2c_wait_task = NULL;
    //xSemaphoreGive(mutex_i2c_handle);
}

//带寄存器设置的发送函数
void I2C_RegWrite(uint8_t addr, uint8_t reg, uint8_t* p_buf, uint16_t len){
    //xSemaphoreTake(mutex_i2c_handle, portMAX_DELAY);
    i2c_wait_task = xTaskGetCurrentTaskHandle();
    (void)ulTaskNotifyTake(pdTRUE, 0);
    HAL_I2C_Mem_Write_IT(&hi2c1, addr, reg, I2C_MEMADD_SIZE_8BIT, p_buf, len);
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    i2c_wait_task = NULL;
    //xSemaphoreGive(mutex_i2c_handle);
}

//主机发送完成中断回调函数
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (i2c_wait_task != NULL) {
        vTaskNotifyGiveFromISR(
            i2c_wait_task,
            &xHigherPriorityTaskWoken
        );
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

//读取函数
void I2C_Receive(uint8_t addr, uint8_t* p_buf, uint16_t len){
    //xSemaphoreTake(mutex_i2c_handle, portMAX_DELAY);
    i2c_wait_task = xTaskGetCurrentTaskHandle();
    (void)ulTaskNotifyTake(pdTRUE, 0);
    HAL_I2C_Master_Receive_IT(&hi2c1, addr, p_buf, len);
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    i2c_wait_task = NULL;
    //xSemaphoreGive(mutex_i2c_handle);
}

//带寄存器设置的读取函数
void I2C_RegRead(uint8_t addr, uint8_t reg, uint8_t* p_buf, uint16_t len){
    //xSemaphoreTake(mutex_i2c_handle, portMAX_DELAY);
    i2c_wait_task = xTaskGetCurrentTaskHandle();
    (void)ulTaskNotifyTake(pdTRUE, 0);
    HAL_I2C_Mem_Read_IT(&hi2c1, addr, reg, I2C_MEMADD_SIZE_8BIT, p_buf, len);
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    i2c_wait_task = NULL;
    //xSemaphoreGive(mutex_i2c_handle);
}

//主机发送完成中断回调函数
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (i2c_wait_task != NULL) {
        vTaskNotifyGiveFromISR(
            i2c_wait_task,
            &xHigherPriorityTaskWoken
        );
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


//确认地址存在函数
uint8_t I2C_IsReady(uint8_t addr){
    uint8_t ready, i2cbuf;
    xSemaphoreTake(mutex_i2c_Handle, portMAX_DELAY);
    if(HAL_I2C_Master_Transmit(&hi2c1, addr, &i2cbuf, 1, 0x249F0) == HAL_OK) ready = 1;
    else ready = 0;
    xSemaphoreGive(mutex_i2c_Handle);
    return ready;
}

//I2C错误中断回调函数
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* 清除错误标志，避免死锁 */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF | I2C_FLAG_ARLO | I2C_FLAG_BERR | I2C_FLAG_OVR);
    if (i2c_wait_task != NULL) {
        vTaskNotifyGiveFromISR(
            i2c_wait_task,
            &xHigherPriorityTaskWoken
        );
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* USER CODE END 1 */

