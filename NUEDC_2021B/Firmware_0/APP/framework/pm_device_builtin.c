#include "pm_device.h"

#include "OLED.h"
#include "dispDriver.h"
//#include "sw6306.h"


static void pm_OLED_prepare(void *ctx)
{
    (void)ctx;
    Disp_SetPowerSave(1);       /* UI_OFF 时立即关屏 */
}

static void pm_OLED_suspend(void *ctx)
{
    (void)ctx;
    /* 已由 prepare 关屏，此处可做更彻底的掉电 */
}

static void pm_OLED_resume(void *ctx)
{
    (void)ctx;
    Disp_SetPowerSave(0);
}

static const pm_device_node_t s_pm_OLED = {
    "OLED",
    pm_OLED_prepare,            /* RUN → UI_OFF 时调用 */
    pm_OLED_suspend,            /* SLEEP_PREPARE → DEEPSLEEP 时调用 */
    pm_OLED_resume,             /* 任何非 RUN → RUN 时调用 */
    0,
    10
};

/*
static void pm_sw6306_suspend(void *ctx)
{
    (void)ctx;
    //SW6306_ForceOff();
}

static void pm_sw6306_resume(void *ctx)
{
    (void)ctx;
    if (SW6306_IsInitialized() == 0) {
        SW6306_Init();
    }
}

static const pm_device_node_t s_pm_sw6306 = {
    "sw6306",
    0,
    pm_sw6306_suspend,
    pm_sw6306_resume,
    0,
    20
};*/

void pm_device_register_builtin(void)
{
    pm_device_register(&s_pm_OLED);
    //pm_device_register(&s_pm_sw6306);
}
