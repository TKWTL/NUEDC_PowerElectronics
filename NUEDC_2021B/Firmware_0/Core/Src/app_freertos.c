/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for powerdown_Task */
osThreadId_t powerdown_TaskHandle;
uint32_t powerdownTaskBuffer[ 128 ];
osStaticThreadDef_t powerdownTaskControlBlock;
const osThreadAttr_t powerdown_Task_attributes = {
  .name = "powerdown_Task",
  .stack_mem = &powerdownTaskBuffer[0],
  .stack_size = sizeof(powerdownTaskBuffer),
  .cb_mem = &powerdownTaskControlBlock,
  .cb_size = sizeof(powerdownTaskControlBlock),
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for UI */
osThreadId_t UIHandle;
uint32_t UI_TaskBuffer[ 512 ];
osStaticThreadDef_t UI_TaskControlBlock;
const osThreadAttr_t UI_attributes = {
  .name = "UI",
  .stack_mem = &UI_TaskBuffer[0],
  .stack_size = sizeof(UI_TaskBuffer),
  .cb_mem = &UI_TaskControlBlock,
  .cb_size = sizeof(UI_TaskControlBlock),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Button */
osThreadId_t ButtonHandle;
uint32_t ButtonBuffer[ 96 ];
osStaticThreadDef_t ButtonControlBlock;
const osThreadAttr_t Button_attributes = {
  .name = "Button",
  .stack_mem = &ButtonBuffer[0],
  .stack_size = sizeof(ButtonBuffer),
  .cb_mem = &ButtonControlBlock,
  .cb_size = sizeof(ButtonControlBlock),
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Beep */
osThreadId_t BeepHandle;
uint32_t BeepBuffer[ 64 ];
osStaticThreadDef_t BeepControlBlock;
const osThreadAttr_t Beep_attributes = {
  .name = "Beep",
  .stack_mem = &BeepBuffer[0],
  .stack_size = sizeof(BeepBuffer),
  .cb_mem = &BeepControlBlock,
  .cb_size = sizeof(BeepControlBlock),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Notification */
osThreadId_t NotificationHandle;
uint32_t NotificationBuffer[ 128 ];
osStaticThreadDef_t NotificationControlBlock;
const osThreadAttr_t Notification_attributes = {
  .name = "Notification",
  .stack_mem = &NotificationBuffer[0],
  .stack_size = sizeof(NotificationBuffer),
  .cb_mem = &NotificationControlBlock,
  .cb_size = sizeof(NotificationControlBlock),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for AutoRecover */
osThreadId_t AutoRecoverHandle;
uint32_t AutoRecoverBuffer[ 64 ];
osStaticThreadDef_t AutoRecoverControlBlock;
const osThreadAttr_t AutoRecover_attributes = {
  .name = "AutoRecover",
  .stack_mem = &AutoRecoverBuffer[0],
  .stack_size = sizeof(AutoRecoverBuffer),
  .cb_mem = &AutoRecoverControlBlock,
  .cb_size = sizeof(AutoRecoverControlBlock),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Update */
osThreadId_t UpdateHandle;
uint32_t UpdateBuffer[ 256 ];
osStaticThreadDef_t UpdateControlBlock;
const osThreadAttr_t Update_attributes = {
  .name = "Update",
  .stack_mem = &UpdateBuffer[0],
  .stack_size = sizeof(UpdateBuffer),
  .cb_mem = &UpdateControlBlock,
  .cb_size = sizeof(UpdateControlBlock),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Beep_Queue */
osMessageQueueId_t Beep_QueueHandle;
uint8_t Beep_QueueBuffer[ 1 * sizeof( uint8_t ) ];
osStaticMessageQDef_t Beep_QueueControlBlock;
const osMessageQueueAttr_t Beep_Queue_attributes = {
  .name = "Beep_Queue",
  .cb_mem = &Beep_QueueControlBlock,
  .cb_size = sizeof(Beep_QueueControlBlock),
  .mq_mem = &Beep_QueueBuffer,
  .mq_size = sizeof(Beep_QueueBuffer)
};
/* Definitions for Button_Queue */
osMessageQueueId_t Button_QueueHandle;
uint8_t Button_QueueBuffer[ 16 * sizeof( uint16_t ) ];
osStaticMessageQDef_t Button_QueueControlBlock;
const osMessageQueueAttr_t Button_Queue_attributes = {
  .name = "Button_Queue",
  .cb_mem = &Button_QueueControlBlock,
  .cb_size = sizeof(Button_QueueControlBlock),
  .mq_mem = &Button_QueueBuffer,
  .mq_size = sizeof(Button_QueueBuffer)
};
/* Definitions for Notification_Queue */
osMessageQueueId_t Notification_QueueHandle;
uint8_t Notification_QueueBuffer[ 16 * sizeof( const char* ) ];
osStaticMessageQDef_t Notification_QueueControlBlock;
const osMessageQueueAttr_t Notification_Queue_attributes = {
  .name = "Notification_Queue",
  .cb_mem = &Notification_QueueControlBlock,
  .cb_size = sizeof(Notification_QueueControlBlock),
  .mq_mem = &Notification_QueueBuffer,
  .mq_size = sizeof(Notification_QueueBuffer)
};
/* Definitions for mutex_gspi_ */
osMutexId_t mutex_gspi_Handle;
osStaticMutexDef_t mutex_gspi_handleControlBlock;
const osMutexAttr_t mutex_gspi__attributes = {
  .name = "mutex_gspi_",
  .cb_mem = &mutex_gspi_handleControlBlock,
  .cb_size = sizeof(mutex_gspi_handleControlBlock),
};
/* Definitions for mutex_i2c_ */
osMutexId_t mutex_i2c_Handle;
osStaticMutexDef_t mutex_i2c_ControlBlock;
const osMutexAttr_t mutex_i2c__attributes = {
  .name = "mutex_i2c_",
  .cb_mem = &mutex_i2c_ControlBlock,
  .cb_size = sizeof(mutex_i2c_ControlBlock),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void powerdown_task(void *argument);
void UI_Task(void *argument);
void Button_Task(void *argument);
void Beep_Task(void *argument);
void Notification_Task(void *argument);
void AutoRecover_Task(void *argument);
void ADC_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
    __WFI();
}
/* USER CODE END 2 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of mutex_gspi_ */
  mutex_gspi_Handle = osMutexNew(&mutex_gspi__attributes);

  /* creation of mutex_i2c_ */
  mutex_i2c_Handle = osMutexNew(&mutex_i2c__attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Beep_Queue */
  Beep_QueueHandle = osMessageQueueNew (1, sizeof(uint8_t), &Beep_Queue_attributes);

  /* creation of Button_Queue */
  Button_QueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &Button_Queue_attributes);

  /* creation of Notification_Queue */
  Notification_QueueHandle = osMessageQueueNew (16, sizeof(const char*), &Notification_Queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of powerdown_Task */
  powerdown_TaskHandle = osThreadNew(powerdown_task, NULL, &powerdown_Task_attributes);

  /* creation of UI */
  UIHandle = osThreadNew(UI_Task, NULL, &UI_attributes);

  /* creation of Button */
  ButtonHandle = osThreadNew(Button_Task, NULL, &Button_attributes);

  /* creation of Beep */
  BeepHandle = osThreadNew(Beep_Task, NULL, &Beep_attributes);

  /* creation of Notification */
  NotificationHandle = osThreadNew(Notification_Task, NULL, &Notification_attributes);

  /* creation of AutoRecover */
  AutoRecoverHandle = osThreadNew(AutoRecover_Task, NULL, &AutoRecover_attributes);

  /* creation of Update */
  UpdateHandle = osThreadNew(ADC_Task, NULL, &Update_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_powerdown_task */
/**
  * @brief  Function implementing the powerdown_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_powerdown_task */
__weak void powerdown_task(void *argument)
{
  /* USER CODE BEGIN powerdown_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END powerdown_task */
}

/* USER CODE BEGIN Header_UI_Task */
/**
* @brief Function implementing the UI thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_UI_Task */
__weak void UI_Task(void *argument)
{
  /* USER CODE BEGIN UI_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END UI_Task */
}

/* USER CODE BEGIN Header_Button_Task */
/**
* @brief Function implementing the Button thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Button_Task */
__weak void Button_Task(void *argument)
{
  /* USER CODE BEGIN Button_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Button_Task */
}

/* USER CODE BEGIN Header_Beep_Task */
/**
* @brief Function implementing the Beep thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Beep_Task */
__weak void Beep_Task(void *argument)
{
  /* USER CODE BEGIN Beep_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Beep_Task */
}

/* USER CODE BEGIN Header_Notification_Task */
/**
* @brief Function implementing the Notification thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Notification_Task */
__weak void Notification_Task(void *argument)
{
  /* USER CODE BEGIN Notification_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Notification_Task */
}

/* USER CODE BEGIN Header_AutoRecover_Task */
/**
* @brief Function implementing the AutoRecover thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AutoRecover_Task */
__weak void AutoRecover_Task(void *argument)
{
  /* USER CODE BEGIN AutoRecover_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END AutoRecover_Task */
}

/* USER CODE BEGIN Header_ADC_Task */
/**
* @brief Function implementing the Update thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ADC_Task */
__weak void ADC_Task(void *argument)
{
  /* USER CODE BEGIN ADC_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END ADC_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

