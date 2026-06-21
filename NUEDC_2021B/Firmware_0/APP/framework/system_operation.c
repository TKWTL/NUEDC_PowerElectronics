#include "system_operation.h"

/***********************以下函数须在ui_conf.c中调用****************************/
ui_item_t SystemSetting_Item, SystemHead_Item, ClickBeep_Item, MuteBeep_Item, NotificationTime_Item, Reset_Item;

//创建系统设置模块所需的参数
void Create_SystemCtrl_Parameters(ui_t *ui){
    static ui_data_t ClickBeep_data;
    ClickBeep_data.name = "ClickBeep";
    ClickBeep_data.ptr = &ClickBeep;
    ClickBeep_data.dataType = UI_DATA_SWITCH;
    ClickBeep_data.actionType = UI_DATA_ACTION_RW;
    static ui_element_t ClickBeep_element;
    ClickBeep_element.data = &ClickBeep_data;
    Create_element(&ClickBeep_Item, &ClickBeep_element);
    
    static ui_data_t MuteBeep_data;
    MuteBeep_data.name = "MuteBeep";
    MuteBeep_data.ptr = &BeepMute;
    MuteBeep_data.dataType = UI_DATA_SWITCH;
    MuteBeep_data.actionType = UI_DATA_ACTION_RW;
    static ui_element_t MuteBeep_element;
    MuteBeep_element.data = &MuteBeep_data;
    Create_element(&MuteBeep_Item, &MuteBeep_element);
    
    static ui_data_t NotificationTime_data;
    NotificationTime_data.name = "Noti Time";
    NotificationTime_data.ptr = &notification_showtime;
    NotificationTime_data.dataType = UI_DATA_INT;
    NotificationTime_data.actionType = UI_DATA_ACTION_RW;
    NotificationTime_data.max = 5000;
    NotificationTime_data.min = 500;
    NotificationTime_data.step = 500;
    static ui_element_t NotificationTime_element;
    NotificationTime_element.data = &NotificationTime_data;
    Create_element(&NotificationTime_Item, &NotificationTime_element);
    
    static int Reset_Trigger = 0;
    static ui_data_t Reset_data;
    Reset_data.name = "System Reset";
    Reset_data.ptr = &Reset_Trigger;
    Reset_data.function = Manual_Reset;
    Reset_data.dataType = UI_DATA_SWITCH;
    Reset_data.actionType = UI_DATA_ACTION_RW;
    static ui_element_t Reset_element;
    Reset_element.data = &Reset_data;
    Create_element(&Reset_Item, &Reset_element);
}

ui_page_t SystemSetting_Page;
//将系统设置模块的对象添加到菜单中
void Add_SystemCtrl_Items(ui_page_t *ParentPage){
    AddItem("-System", UI_ITEM_PARENTS, img_configuration, &SystemSetting_Item, ParentPage, &SystemSetting_Page, NULL);
        AddPage("[System]", &SystemSetting_Page, UI_PAGE_TEXT, ParentPage);
            AddItem("[Back]", UI_ITEM_RETURN, NULL, &SystemHead_Item, &SystemSetting_Page, ParentPage, NULL);
            AddItem(" Click Beep", UI_ITEM_DATA, NULL, &ClickBeep_Item, &SystemSetting_Page, NULL, NULL);
            AddItem(" Mute Beep", UI_ITEM_DATA, NULL, &MuteBeep_Item, &SystemSetting_Page, NULL, NULL);
            AddItem(" Notification ShowTime", UI_ITEM_DATA, NULL, &NotificationTime_Item, &SystemSetting_Page, NULL, NULL);
            AddItem(" System Reset", UI_ITEM_DATA, NULL, &Reset_Item, &SystemSetting_Page, NULL, NULL);
}

//系统复位函数
void Manual_Reset(ui_t *ui){
    Disp_ClearBuffer();
    Disp_SendBuffer();//先清屏，避免显示错乱
    NVIC_SystemReset();
}

uint8_t ClickBeep = 0;//按键按下时发出声音
uint8_t BeepMute = 0;//静音，无论如何蜂鸣器不会发出声音

extern osMessageQueueId_t Beep_QueueHandle;

//向蜂鸣器队列发送一条消息
void Beep_Send(uint8_t index){
    if(BeepMute == 1) return;
    else osMessageQueuePut(Beep_QueueHandle, &index, NULL, 0);
}

#define BEEP_TIM    htim4
#define BEEP_TIM_CH TIM_CHANNEL_3

void Beep_Process(void){
    osStatus_t status;
    uint8_t beep_index;//用于接收消息编号
    
    status = osMessageQueueGet(Beep_QueueHandle, &beep_index, NULL, 0);
    if(status == osOK){
        switch(beep_index){
            case 0:
                __HAL_TIM_SET_COMPARE(&BEEP_TIM, BEEP_TIM_CH, 127);
                osDelay(pdMS_TO_TICKS(150));
                __HAL_TIM_SET_COMPARE(&BEEP_TIM, BEEP_TIM_CH, 0);
            break;
            
            case 1:
                __HAL_TIM_SET_COMPARE(&BEEP_TIM, BEEP_TIM_CH, 127);
                osDelay(pdMS_TO_TICKS(100));
                __HAL_TIM_SET_COMPARE(&BEEP_TIM, BEEP_TIM_CH, 0);
                osDelay(pdMS_TO_TICKS(100));
                __HAL_TIM_SET_COMPARE(&BEEP_TIM, BEEP_TIM_CH, 127);
                osDelay(pdMS_TO_TICKS(100));
                __HAL_TIM_SET_COMPARE(&BEEP_TIM, BEEP_TIM_CH, 0);
                osDelay(pdMS_TO_TICKS(100));
                __HAL_TIM_SET_COMPARE(&BEEP_TIM, BEEP_TIM_CH, 127);
                osDelay(pdMS_TO_TICKS(100));
                __HAL_TIM_SET_COMPARE(&BEEP_TIM, BEEP_TIM_CH, 0);
            break;
            
            default:
            break;
        }
    }
    else osDelay(pdMS_TO_TICKS(100));
}
                
