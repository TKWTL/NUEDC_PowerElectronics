/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdarg.h>
#include <stdio.h>
/* USER CODE END Includes */

extern UART_HandleTypeDef huart4;

extern UART_HandleTypeDef huart1;

/* USER CODE BEGIN Private defines */

/* USART 环形缓冲区收发对象 */
typedef struct {
    UART_HandleTypeDef *huart;          /* HAL 串口句柄 */

    /* TX 发送 */
    uint8_t            *tx_buf;
    uint16_t            tx_buf_size;
    volatile uint16_t   tx_head;
    volatile uint16_t   tx_tail;
    volatile uint16_t   tx_active_len;
    volatile uint8_t    tx_busy;

    /* RX 接收（DMA Circular + IDLE 中断） */
    uint8_t            *rx_buf;
    uint16_t            rx_buf_size;
    volatile uint16_t   rx_head;        /* DMA 已写入位置（由 IDLE 回调更新） */
    volatile uint16_t   rx_tail;        /* 应用层已读取位置 */
    TaskHandle_t        rx_task_handle; /* 解码任务句柄（用于任务通知） */
} bsp_uart_t;

/* 外部声明串口实例 */
extern bsp_uart_t pc_uart;    /* UART4 — PC 调试串口 */
extern bsp_uart_t bt_uart;    /* USART1 — 蓝牙串口 */

/* USER CODE END Private defines */

void MX_UART4_Init(void);
void MX_USART1_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* ─── TX ─── */
void USART_SendByte(bsp_uart_t *uart, uint8_t data);
void USART_SendString(bsp_uart_t *uart, const char *str);
void USART_Printf(bsp_uart_t *uart, const char *format, ...);

/* ─── RX ─── */
void USART_StartRxDMA(bsp_uart_t *uart);          /* 启动环形 DMA 接收 */
uint8_t USART_ReadByte(bsp_uart_t *uart, uint8_t *data);   /* 单字节读取（返回 1=成功 0=无数据） */
uint16_t USART_ReadBytes(bsp_uart_t *uart, uint8_t *buf, uint16_t max_len); /* 多字节读取（返回实际读取字节数） */
void USART_SetRxTask(bsp_uart_t *uart, TaskHandle_t task);  /* 绑定解码任务句柄 */
void USART_OnIdle(bsp_uart_t *uart);                        /* UART IDLE 中断处理（由 stm32g4xx_it.c 调用） */

/* 发送一个字节 */
void USART_SendByte(bsp_uart_t *uart, uint8_t data);

/* 发送字符串 */
void USART_SendString(bsp_uart_t *uart, const char *str);

/* 格式化打印 */
void USART_Printf(bsp_uart_t *uart, const char *format, ...);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

