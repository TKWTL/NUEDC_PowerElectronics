#ifndef PM_API_H
#define PM_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PM_BLOCK_SW6306_LOAD    0x01U

/* PM 功能总开关 — 可在外层定义此宏为 0 禁用全部电源管理 */
#ifndef PM_FEATURE_ENABLE
#define PM_FEATURE_ENABLE  1
#endif

/* UI 任务句柄变量名（唤醒后恢复用），外层可覆盖 */
#ifndef PM_UI_TASK_HANDLE
#define PM_UI_TASK_HANDLE  UIHandle
#endif

void pm_api_refresh_idle(void);
void pm_api_force_sleep(void);
void pm_api_set_sleep_timeout(int timeout_sec);  /* 秒 → 内部定时器重装载 */
void pm_api_set_sleep_block(uint8_t mask, uint8_t enable);
void pm_api_set_unstable_wake(uint8_t unstable);
uint8_t pm_api_is_unstable_wake(void);

uint8_t pm_api_get_block_mask(void);

#ifdef __cplusplus
}
#endif

#endif
