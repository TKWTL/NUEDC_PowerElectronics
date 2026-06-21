#include "application.h"

//弹窗显示时间（单位：毫秒）
int notification_showtime = 1500;

/* ISR 安全通知缓冲区 */
static volatile const char *g_pending_notification = NULL;

//向弹窗队列发送一条消息（任务上下文调用）
void Notification_Send(const char* p){
    Beep_Send(1);
    osMessageQueuePut(Notification_QueueHandle, &p, NULL, 0);
}

//从 ISR 安全地发送通知（仅指针存储，由 Notification_Task 转发）
void Notification_Send_FromISR(const char* p){
    g_pending_notification = p;
}

/* 将 ISR 缓存的待发送通知排入队列 */
static void ProcessPendingNotification(void)
{
    const char *msg;
    taskENTER_CRITICAL();
    msg = (const char *)g_pending_notification;
    g_pending_notification = NULL;
    taskEXIT_CRITICAL();
    if (msg != NULL) {
        Notification_Send(msg);
    }
}

/*
 *弹窗功能实现任务函数
 *循环检查队列，无内容则阻塞线程
 *有内容时，阻塞Menu_Task，内容的值作为索引值查询notifications_draw.h中定义的字符串数组，并通过动画效果显示出来
 *在用户按键或者到达了系统设置的展示时间后退出
 */
void Notification_Process(void){
    osStatus_t status;
    const char *p_notification;
    uint8_t bgC = ui.bgColor;
    int notification_timer;
    extern osThreadId_t UIHandle;
    
    status = osMessageQueueGet(Notification_QueueHandle, &p_notification, NULL, 0);
    if(status == osOK){
        Disp_SendBuffer();
        vTaskSuspend(UIHandle);
        //显示
        PopUp_Show(2, 10, 122, 42);
        Disp_SetFont(font_menu_main_h12w6);
        Disp_DrawStr(8, 36, p_notification);
        Disp_SendBuffer();
        
        //延时展示，有按键时退出
        for(notification_timer = notification_showtime; notification_timer > 0; notification_timer -= 100){
            if(indevScan() != UI_ACTION_NONE) break;
            osDelay(pdMS_TO_TICKS(100));
        }
        
        vTaskResume(UIHandle);
    }
}
    
void Notification_Task(void){
    while(1){
        ProcessPendingNotification();
        Notification_Process();
        osDelay(pdMS_TO_TICKS(100));
    }
}
