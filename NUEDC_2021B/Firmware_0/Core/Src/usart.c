/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_uart4_tx;
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;

/* UART4 init function */
void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}
/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspInit 0 */

  /* USER CODE END UART4_MspInit 0 */
    LL_RCC_SetUARTClockSource(LL_RCC_UART4_CLKSOURCE_PCLK1);

    /* UART4 clock enable */
    __HAL_RCC_UART4_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**UART4 GPIO Configuration
    PC10     ------> UART4_TX
    PC11     ------> UART4_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF5_UART4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_UART4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* UART4 DMA Init */
    /* UART4_TX Init */
    hdma_uart4_tx.Instance = DMA1_Channel4;
    hdma_uart4_tx.Init.Request = DMA_REQUEST_UART4_TX;
    hdma_uart4_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart4_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart4_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart4_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart4_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart4_tx.Init.Mode = DMA_NORMAL;
    hdma_uart4_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_uart4_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_uart4_tx);

    /* UART4_RX Init */
    hdma_uart4_rx.Instance = DMA1_Channel5;
    hdma_uart4_rx.Init.Request = DMA_REQUEST_UART4_RX;
    hdma_uart4_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart4_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart4_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart4_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart4_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart4_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart4_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_uart4_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_uart4_rx);

    /* UART4 interrupt Init */
    HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspInit 1 */

  /* USER CODE END UART4_MspInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */

    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);

    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PE0     ------> USART1_TX
    PE1     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA1_Channel2;
    hdma_usart1_tx.Init.Request = DMA_REQUEST_USART1_TX;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart1_tx);

    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA1_Channel3;
    hdma_usart1_rx.Init.Request = DMA_REQUEST_USART1_RX;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspDeInit 0 */

  /* USER CODE END UART4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART4_CLK_DISABLE();

    /**UART4 GPIO Configuration
    PC10     ------> UART4_TX
    PC11     ------> UART4_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11);

    /* UART4 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* UART4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspDeInit 1 */

  /* USER CODE END UART4_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PE0     ------> USART1_TX
    PE1     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0|GPIO_PIN_1);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* ─── 串口实例定义 ─── */

/* PC 调试串口 (UART4) */
#define PC_UART_TX_BUF_SIZE   512
#define PC_UART_RX_BUF_SIZE   512
static uint8_t s_pc_tx_buf[PC_UART_TX_BUF_SIZE];
static uint8_t s_pc_rx_buf[PC_UART_RX_BUF_SIZE];
bsp_uart_t pc_uart = {
    .huart       = &huart4,
    .tx_buf      = s_pc_tx_buf,
    .tx_buf_size = PC_UART_TX_BUF_SIZE,
    .rx_buf      = s_pc_rx_buf,
    .rx_buf_size = PC_UART_RX_BUF_SIZE,
};

/* 蓝牙串口 (USART1) */
#define BT_UART_TX_BUF_SIZE   256
#define BT_UART_RX_BUF_SIZE   256
static uint8_t s_bt_tx_buf[BT_UART_TX_BUF_SIZE];
static uint8_t s_bt_rx_buf[BT_UART_RX_BUF_SIZE];
bsp_uart_t bt_uart = {
    .huart       = &huart1,
    .tx_buf      = s_bt_tx_buf,
    .tx_buf_size = BT_UART_TX_BUF_SIZE,
    .rx_buf      = s_bt_rx_buf,
    .rx_buf_size = BT_UART_RX_BUF_SIZE,
};

/* ─── 内部函数 ─── */

/* 启动 DMA 搬运下一段连续数据 */
static void usart_start_tx_dma(bsp_uart_t *uart)
{
    uint16_t head = uart->tx_head;
    uint16_t tail = uart->tx_tail;
    uint16_t cnt;

    if (head == tail) {
        uart->tx_active_len = 0;          /* 缓冲区已空 */
        return;
    }

    /* 计算从 head 到 tail 的连续长度（处理回绕） */
    if (head < tail) {
        cnt = tail - head;
    } else {
        cnt = uart->tx_buf_size - head;
    }

    uart->tx_active_len = cnt;
    uart->tx_busy = 1;
    if (HAL_UART_Transmit_DMA(uart->huart, &uart->tx_buf[head], cnt) != HAL_OK) {
        /* DMA 启动失败：复位状态，避免死锁 */
        uart->tx_busy = 0;
        uart->tx_active_len = 0;
    }
}

/* 向环形缓冲区写入一个字节 */
static void usart_tx_write(bsp_uart_t *uart, uint8_t data)
{
    uint16_t next = (uart->tx_tail + 1) & (uart->tx_buf_size - 1);

    /* 缓冲区满则阻塞等待 */
    while (next == uart->tx_head);

    uart->tx_buf[uart->tx_tail] = data;
    uart->tx_tail = next;

    /* 如果 DMA 空闲，立即启动传输 */
    if (uart->tx_busy == 0) {
        usart_start_tx_dma(uart);
    }
}

/* ─── 公开 API ─── */

/* 发送一个字节 */
void USART_SendByte(bsp_uart_t *uart, uint8_t data)
{
    usart_tx_write(uart, data);
}

/* 发送字符串 */
void USART_SendString(bsp_uart_t *uart, const char *str)
{
    while (*str) {
        usart_tx_write(uart, (uint8_t)*str++);
    }
}

/* 格式化打印 */
void USART_Printf(bsp_uart_t *uart, const char *format, ...)
{
    char buf[64];
    va_list arg;

    va_start(arg, format);
    vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);

    USART_SendString(uart, buf);
}

/* ─── RX ─── */

/* 启动环形 DMA 接收 */
void USART_StartRxDMA(bsp_uart_t *uart)
{
    uart->rx_head = 0;
    uart->rx_tail = 0;
    /* 启动 Circular DMA 接收，缓冲区满后自动回绕 */
    HAL_UART_Receive_DMA(uart->huart, uart->rx_buf, uart->rx_buf_size);
    /* 使能 IDLE 中断 */
    __HAL_UART_ENABLE_IT(uart->huart, UART_IT_IDLE);
}

/* 绑定解码任务句柄（用于任务通知） */
void USART_SetRxTask(bsp_uart_t *uart, TaskHandle_t task)
{
    uart->rx_task_handle = task;
}

/* 单字节读取：成功返回 1，无数据返回 0 */
uint8_t USART_ReadByte(bsp_uart_t *uart, uint8_t *data)
{
    if (uart->rx_head == uart->rx_tail) return 0;

    *data = uart->rx_buf[uart->rx_tail];
    uart->rx_tail = (uart->rx_tail + 1) & (uart->rx_buf_size - 1);
    return 1;
}

/* 多字节读取：返回实际读取的字节数 */
uint16_t USART_ReadBytes(bsp_uart_t *uart, uint8_t *buf, uint16_t max_len)
{
    uint16_t cnt = 0;
    while (cnt < max_len) {
        if (uart->rx_head == uart->rx_tail) break;
        buf[cnt++] = uart->rx_buf[uart->rx_tail];
        uart->rx_tail = (uart->rx_tail + 1) & (uart->rx_buf_size - 1);
    }
    return cnt;
}

/* ─── IDLE 回调 ─── */

/* 由 UART ISR 的 USER CODE 段调用，处理 IDLE 中断 */
void USART_OnIdle(bsp_uart_t *uart)
{
    DMA_HandleTypeDef *hdma = uart->huart->hdmarx;
    uint16_t new_head;

    /* 读取 DMA 当前计数，计算已写入位置 */
    uint32_t cndtr = __HAL_DMA_GET_COUNTER(hdma);
    new_head = uart->rx_buf_size - (uint16_t)cndtr;
    if (new_head == uart->rx_buf_size) new_head = 0;

    /* 更新 rx_head。若数据已可用，通知解码任务 */
    uart->rx_head = new_head;

    if (uart->rx_head != uart->rx_tail && uart->rx_task_handle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(uart->rx_task_handle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* ─── HAL 回调 ─── */

/* DMA 发送完成回调，通过 huart 反查 bsp_uart_t 实例 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    bsp_uart_t *uart = NULL;

    if (huart->Instance == UART4) {
        uart = &pc_uart;
    } else if (huart->Instance == USART1) {
        uart = &bt_uart;
    }

    if (uart == NULL) return;

    uart->tx_head = (uart->tx_head + uart->tx_active_len) & (uart->tx_buf_size - 1);
    uart->tx_active_len = 0;
    uart->tx_busy = 0;

    /* 继续发送缓冲区中剩余的数据 */
    usart_start_tx_dma(uart);
}

#if 0
/* ─── 回声测试任务 ─── */

/* 切换串口：注释/取消注释下面这行 */
#define ECHO_USE_USART1
#ifndef ECHO_USE_USART1
  #define ECHO_UART   (&pc_uart)
#else
  #define ECHO_UART   (&bt_uart)
#endif

void USART_EchoTask(void *argument)
{
    uint8_t buf[128];
    uint16_t len;

    (void)argument;

    /* 启动串口接收 */
    USART_StartRxDMA(ECHO_UART);
    /* 绑定本任务句柄，IDLE 中断才能通知到 */
    USART_SetRxTask(ECHO_UART, xTaskGetCurrentTaskHandle());

    for (;;)
    {
        /* 等待 IDLE 中断通知 */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* 读取收到的数据 */
        len = USART_ReadBytes(ECHO_UART, buf, sizeof(buf));

        /* 原样发回 */
        for (uint16_t i = 0; i < len; i++) {
            USART_SendByte(ECHO_UART, buf[i]);
        }
    }
}
#endif

/* USER CODE END 1 */

