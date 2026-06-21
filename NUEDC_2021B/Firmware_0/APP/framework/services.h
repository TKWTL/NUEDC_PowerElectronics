#ifndef __SERVICES_H__
#define __SERVICES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
    
#include "oled.h"
#include "buttons.h"
    
//MiaoUI库相关API，必须排在最前面以避免循环依赖
#include "ui_conf.h"
#include "ui.h"
#include "image.h"
#include "dispDriver.h"
#include "indevDriver.h"
#include "display_effects.h"
#include "notifications_draw.h"
    
#include "app_event.h"
#include "pm_api.h"
#include "pm_controller.h"
#include "pm_device.h"
#include "pm_policy.h"
#include "pm_sleep_timer.h"
#include "system_operation.h"
#include "system_statistic.h"
#include "time.h"

#ifdef __cplusplus
}
#endif

#endif // __SERVICES_H__