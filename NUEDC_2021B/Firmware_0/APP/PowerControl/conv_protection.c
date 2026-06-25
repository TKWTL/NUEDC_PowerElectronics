/*******************************************************************************

*******************************************************************************/
#include "conv_protection.h"
#include <math.h>
#include <armdsp.h>

#define OV_H 61.0f
#define OV_L 24.0f
#define OOC  9.0f
#define OOV  61.0f

//在环路中运行的软件保护函数
inline void Protection(void){
    /*if(fabsf(ReadControlVar(&VZS_t)) > OOV){
            PWM_Stop();
            g_conv_state = CONV_FAULT;
            Notification_Send_FromISR("U Over Current!!");
    }*/
}

/*******************************************************************************
 * FLT1 过流中断服务函数 — 由 stm32g4xx_it.c 中的 HRTIM1_FLT_IRQHandler 调用
 * FLT1 硬件已自动封锁 TIMER_D 输出，软件处理状态切换与通知
 ******************************************************************************/
RAMFUNC void POWER_FLT_IRQHandler(void)
{
    //仅处理 FLT1
    if (LL_HRTIM_IsActiveFlag_FLT1(HRTIM1) == 0U) return;
    
    LL_HRTIM_ClearFlag_FLT1(HRTIM1);
    
    //软件再确保一次 PWM 关闭
    PWM_Stop();
    
    //状态机：仅在运行时跳故障，停机时不切换
    if (g_conv_state == CONV_RUN) {
        g_conv_state = CONV_FAULT;
    }
    
    Notification_Send_FromISR("Over Current!!");
}

/**********************硬件保护&保护参数设置区*********************************/
//与设置项有关的可调参数变量
int UpperLimit_VBUS = 55;

//初始化保护参数计算，这是给AWD实现的硬件保护提供的，需要转换成对应的无符号整数
//会打断转换器运行
void Protection_Init1(ui_t *ui){
    uint16_t RegTemp;
    uint8_t AD3START_STA = 0;
    
    if(g_conv_state == CONV_RUN) g_conv_state = CONV_STOP;                      //强制转换器停止运行
    
    //G4不允许在ADC转换过程中修改看门狗阈值
    //通过关闭触发的方式保证DMA搬运序列的正确性
    if(ADC3->CR & ADC_CR_ADSTART){
        AD3START_STA = 1;
        LL_HRTIM_SetADCTrigSrc(HRTIM1, LL_HRTIM_ADCTRIG_1, LL_HRTIM_ADCTRIG_SRC13_NONE);// 1) 立刻禁止后续外部触发（关键：不让新序列开始）
        ADC3->ISR = ADC_ISR_EOS;                     // 写1清EOS
        while ((ADC3->ISR & ADC_ISR_EOS) == 0U) {}   // 等到本次序列结束
        ADC3->CR |= ADC_CR_ADSTP;
        while (ADC3->CR & ADC_CR_ADSTP) {}
    }
        
    RegTemp = (uint16_t)((UpperLimit_VBUS- 1.0947f)* 4096/ (19.5077f* ADC_VREF));
    MODIFY_REG(ADC3->TR1, ADC_TR1_HT1,
               (RegTemp << ADC_TR1_HT1_Pos) & ADC_TR1_HT1);
    
    //如果关闭了ADC记得重启
    if(AD3START_STA){
        ADC3->CR |= ADC_CR_ADSTART;
        LL_HRTIM_ConfigADCTrig(HRTIM1, LL_HRTIM_ADCTRIG_1, LL_HRTIM_ADCTRIG_UPDATE_TIMER_F, LL_HRTIM_ADCTRIG_SRC13_TIMFRST);
    }
    
    //LL_HRTIM_ConfigADCTrig(HRTIM1, LL_HRTIM_ADCTRIG_3, LL_HRTIM_ADCTRIG_UPDATE_TIMER_A, LL_HRTIM_ADCTRIG_SRC13_NONE);
    //LL_HRTIM_ConfigADCTrig(HRTIM1, LL_HRTIM_ADCTRIG_3, LL_HRTIM_ADCTRIG_UPDATE_TIMER_A, LL_HRTIM_ADCTRIG_SRC13_TIMARST);
}

//ADC看门狗回调函数，STM32G474的AWDT只支持单门限触发，用途十分有限
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc){
    if(g_conv_state == CONV_RUN){//只在运行时触发
        if (hadc->Instance == ADC3){   
            PWM_Stop();
            g_conv_state = CONV_FAULT;
            Notification_Send_FromISR("BUS Over Voltage!!");
        }
    }
}

//初始化保护参数计算，这是给DAC+COMP实现的硬件保护提供的，需要转换成对应的无符号整数
//不会打断转换器运行
void Protection_Init2(ui_t *ui){
    
}

//以下函数须在ui_conf.c中调用
ui_item_t VBUSLimit_Item;
//创建保护设置模块所需的参数
void Create_Protection_Parameters1(ui_t *ui){
    static ui_data_t VBUSLimit_data;
    VBUSLimit_data.name = "VBUS Limit";
    VBUSLimit_data.ptr = &UpperLimit_VBUS;
    VBUSLimit_data.function = Protection_Init1;
    VBUSLimit_data.functionType = UI_DATA_FUNCTION_STEP_EXECUTE;
    VBUSLimit_data.dataType = UI_DATA_INT;
    VBUSLimit_data.actionType = UI_DATA_ACTION_RW;
    VBUSLimit_data.max = 65;
    VBUSLimit_data.min = 16;
    VBUSLimit_data.step = 1;
    static ui_element_t VBUSLimit_element;
    VBUSLimit_element.data = &VBUSLimit_data;
    Create_element(&VBUSLimit_Item, &VBUSLimit_element);
}
//将保护设置模块的对象添加到菜单中
void Add_ProtectionLevel_Items(ui_page_t *ParentPage){
    AddItem(" VBUS Voltage Limit", UI_ITEM_DATA, NULL, &VBUSLimit_Item, ParentPage, NULL, NULL);
}

/************************触发保护后的自动恢复功能区****************************/
//与设置项有关的可调参数变量
uint8_t AutoRecoverEN = 1;                                                      //自动恢复使能，0：不会自动恢复；1：试图自动恢复
int AutoRecoverDelay = 3000;                                                    //自动恢复延迟，单位毫秒
int AutoRecoverTime = 3;                                                        //自动重试次数，为0时会无限尝试

//重试次数计数器
int AutoRecoverCounter = 0;

//自动恢复任务函数，CubeMX FreeRTOS也要配置添加该任务
void AutoRecover_Task(void *argument){
    for(;;){
        if(g_conv_state == CONV_FAULT){
            if(AutoRecoverEN){
                if(AutoRecoverTime && AutoRecoverCounter >= AutoRecoverTime){   //重试次数为0时不触发防循环机制
                    g_conv_state = CONV_STOP;   //放弃重启
                    AutoRecoverCounter = 0;     //复位计数器
                    continue;
                }
                osDelay(AutoRecoverDelay);      //进行延迟
                if(g_conv_state == CONV_FAULT){
                    if(AutoRecoverTime) AutoRecoverCounter++;//累加重试次数计数器
                    g_conv_state = CONV_RUN;
                }
            }
            else{
                g_conv_state = CONV_STOP;
            }
        }
        
        osDelay(200);
    }
}

//以下函数须在ui_conf.c中调用
ui_item_t AutoRecoverEN_Item, AutoRecoverDelay_Item, AutoRecoverTime_Item;
//创建保护设置模块所需的参数
void Create_Protection_Parameters2(ui_t *ui){
    static ui_data_t AutoRecoverEN_data;
    AutoRecoverEN_data.name = "Auto Recover Enable";
    AutoRecoverEN_data.ptr = &AutoRecoverEN;
    AutoRecoverEN_data.dataType = UI_DATA_SWITCH;
    AutoRecoverEN_data.actionType = UI_DATA_ACTION_RW;
    static ui_element_t AutoRecoverEN_element;
    AutoRecoverEN_element.data = &AutoRecoverEN_data;
    Create_element(&AutoRecoverEN_Item, &AutoRecoverEN_element);
    
    static ui_data_t AutoRecoverDelay_data;
    AutoRecoverDelay_data.name = "Restart Delay in ms";
    AutoRecoverDelay_data.ptr = &AutoRecoverDelay;
    AutoRecoverDelay_data.dataType = UI_DATA_INT;
    AutoRecoverDelay_data.actionType = UI_DATA_ACTION_RW;
    AutoRecoverDelay_data.max = 10000;
    AutoRecoverDelay_data.min = 400;
    AutoRecoverDelay_data.step = 200;
    static ui_element_t AutoRecoverDelay_element;
    AutoRecoverDelay_element.data = &AutoRecoverDelay_data;
    Create_element(&AutoRecoverDelay_Item, &AutoRecoverDelay_element);
    
    static ui_data_t AutoRecoverTime_data;
    AutoRecoverTime_data.name = "Restart Times";
    AutoRecoverTime_data.ptr = &AutoRecoverTime;
    AutoRecoverTime_data.dataType = UI_DATA_INT;
    AutoRecoverTime_data.actionType = UI_DATA_ACTION_RW;
    AutoRecoverTime_data.max = 10;
    AutoRecoverTime_data.min = 0;
    AutoRecoverTime_data.step = 1;
    static ui_element_t AutoRecoverTime_element;
    AutoRecoverTime_element.data = &AutoRecoverTime_data;
    Create_element(&AutoRecoverTime_Item, &AutoRecoverTime_element);
}
//将保护设置模块的对象添加到菜单中
void Add_Protection_Items(ui_page_t *ParentPage){
    AddItem(" Auto Recover Enable", UI_ITEM_DATA, NULL, &AutoRecoverEN_Item, ParentPage, NULL, NULL);
    AddItem(" Auto Recover Delay", UI_ITEM_DATA, NULL, &AutoRecoverDelay_Item, ParentPage, NULL, NULL);
    AddItem(" Auto Recover Times", UI_ITEM_DATA, NULL, &AutoRecoverTime_Item, ParentPage, NULL, NULL);
}

/***************************创建保护选项设置页面*******************************/
#include "image.h"
ui_page_t Protection_Page;
ui_item_t Protection_Item, ProtectionHead_Item;

void Add_Protection_Page(ui_page_t *ParentPage){
    AddItem("-Protection Set", UI_ITEM_PARENTS, img_shield, &Protection_Item, ParentPage, &Protection_Page, NULL);
        AddPage("[Setting]", &Protection_Page, UI_PAGE_TEXT, ParentPage);
            AddItem("[Menu]", UI_ITEM_RETURN, NULL, &ProtectionHead_Item, &Protection_Page, ParentPage, NULL);
            Add_ProtectionLevel_Items(&Protection_Page);
            Add_Protection_Items(&Protection_Page);
}
    