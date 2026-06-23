/*
 *用户应用文件夹下应用程序统一头文件
 *包含收音机、各种游戏、烧屏测试和水平仪的界面实现
 */
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#ifdef __cplusplus
extern "C" {
#endif
    
//系统
#include "main.h"

//MiaoUI库
#include "ui_conf.h"

//该层次各应用函数头文件
#include "game_snake.h"
//#include "../MiaoUI/notifications/notifications_draw.h"
#include "burn_in_tester.h"
#include "i2c_scanner.h"
#include "game_dinosaur.h"
#include "dashboard.h"

#include "conv_adc.h"
#include "conv_controller.h"
#include "conv_loop.h"
#include "conv_protection.h"
#include "conv_pwm.h"
#include "algorithm_control.h"

#include "pm_ui_register.h"

//UI对象句柄
extern ui_t ui;
    
//菜单界面句柄
extern TaskHandle_t MenuHandle;
    
//接收按键操作的队列
extern osMessageQueueId_t Button_QueueHandle;

#ifdef __cplusplus
}
#endif

#endif
