#include "services.h"

#include "pm_controller.h"
#include "pm_device.h"

#if PM_FEATURE_ENABLE
extern osThreadId_t PM_UI_TASK_HANDLE;   /* 由 PM_UI_TASK_HANDLE 宏指定的 UI 任务句柄 */
#endif

static pm_controller_t s_pm_ctrl;
static uint8_t s_pm_ready;
static volatile uint8_t s_pm_refresh_req;
static volatile uint8_t s_pm_block_mask;

/** 唤醒后按键不响应静默计数 (~100ms) */
static uint8_t s_wk_btn;

void pm_api_refresh_idle(void)
{
    s_pm_refresh_req = 1;
}

void pm_api_force_sleep(void)
{
    taskENTER_CRITICAL();
    s_pm_refresh_req = 0;                    /* 丢弃 indev 暂存的刷新请求 */
    pm_sleep_timer_force_expire();           /* 定时器立即到期 */
    pm_controller_mark_ui_active(&s_pm_ctrl, 0); /* 标记 UI 不活跃 */
    taskEXIT_CRITICAL();
}


void pm_api_set_sleep_block(uint8_t mask, uint8_t enable)
{
    if (enable != 0) {
        s_pm_block_mask |= mask;
    } else {
        s_pm_block_mask &= (uint8_t)(~mask);
    }
}

static volatile uint8_t s_unstable_wake;

void pm_api_set_unstable_wake(uint8_t unstable)
{
    s_unstable_wake = unstable;
}

uint8_t pm_api_is_unstable_wake(void)
{
    return s_unstable_wake;
}

uint8_t pm_api_get_block_mask(void)
{
    return s_pm_block_mask;
}

static void pm_enter_deep_sleep(void)
{
    /* ── 休眠前排空 ── */

    /* ① 挂起 FreeRTOS 调度器：防止休眠过程中其他任务启动新的事务 */
    vTaskSuspendAll();

    /* ② 忙等 100ms，排空所有正在处理的 I2C/SPI/USART 事务和中断 */
    HAL_Delay(100);

    /* 停止 TIM7 计数器并屏蔽其中断 */
    TIM7->CR1   &= ~TIM_CR1_CEN;        /* 关闭计数器 */
    TIM7->DIER  &= ~TIM_DIER_UIE;       /* 关闭更新中断 */
    NVIC_ClearPendingIRQ(TIM7_DAC_IRQn);
    TIM7->SR    = ~TIM_SR_UIF;          /* 清除更新标志 */

    /* ⑥ 禁止 SysTick 中断并清 pending：防止 WFI 被时间片中断立即唤醒 */
    SysTick->CTRL &= ~(uint32_t)SysTick_CTRL_TICKINT_Msk;
    SysTick->VAL = 0;
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;

    /* ── 进入 DeepSleep ── */
    LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
    __DSB();
    __WFI();
    __ISB();

    /* ──── 唤醒后 ──── */

    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;     /* 清除 SLEEPDEEP 位 */

/* ── 恢复系统时钟 ──
     *
     * Stop 模式唤醒后系统时钟自动切回 HSI16（16MHz），
     * PLL 配置丢失，需根据休眠前的时钟模式重新配置。
     * SystemCoreClock 在 RAM 中保留，可据此判断模式。
     */

    /* ── 恢复系统时钟 ── */
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1) {}
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI) {}

    if (SystemCoreClock == 240000000)
    {
        /* 超频 240MHz */
        LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
        while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_5) {}
        LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
        LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
        LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_4);
        LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
        LL_RCC_PLL_ConfigDomain_ADC(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLP_DIV_8);
    }
    else
    {
        /* 默认 120MHz */
        LL_PWR_EnableRange1BoostMode();
        LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
        while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_3) {}
        LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 20, LL_RCC_PLLR_DIV_2);
        LL_RCC_PLL_ConfigDomain_ADC(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 20, LL_RCC_PLLP_DIV_4);
    }

    LL_RCC_PLL_EnableDomain_SYS();
    if (SystemCoreClock == 240000000)
        LL_RCC_PLL_EnableDomain_ADC();
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1) {}

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {}

    for (__IO uint32_t i = (170 >> 1); i != 0; i--);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    if (SystemCoreClock != 240000000) {
        LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
        LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
    }
    LL_SetSystemCoreClock(SystemCoreClock);

    /* ⑤ 恢复 HAL 时基（SysTick） */
    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
        Error_Handler();

    /* ③ 恢复 FreeRTOS 调度（先于 SysTick，确保调度器恢复时 tick 计数干净） */
    xTaskResumeAll();

    /* ④ 恢复 SysTick 中断 */
    SysTick->CTRL |= (uint32_t)SysTick_CTRL_TICKINT_Msk;

    /* ⑤ 恢复 HAL_tick（TMR7）时基 */
    TIM7->CNT  = 0;                     /* 复位计数器 */
    TIM7->SR   = ~TIM_SR_UIF;          /* 清除更新标志 */
    TIM7->CR1 |= TIM_CR1_CEN;          /* 开启计数器 */
    TIM7->DIER |= TIM_DIER_UIE;        /* 开启更新中断 */

    /* ⑥ 设置唤醒静默期：刚唤醒时不响应按键，避免误操作 */
    s_wk_btn = 2;   /* ~100ms (2×50ms) */
    pm_api_set_unstable_wake(1);
}

static void pm_enter_standby(void)
{
    /* STM32G474 Standby 模式 */
    /* 使能 WakeUp Pin (PA0 = WKUP1，需根据硬件接线确认) */
    LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN1);
    LL_PWR_ClearFlag_WU();
    LL_PWR_ClearFlag_SB();

    LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
    __DSB();
    __WFI();
    /* 唤醒后相当于从 Reset 启动，不会执行到这里 */
}

static void pm_task_init_once(void)
{
    if (s_pm_ready != 0) {
        return;
    }

    /* 默认关闭自动休眠（超时设最大值，永不超时），由 UI 单选菜单开启 */
    pm_controller_init(&s_pm_ctrl, 0xFFFFFFFFUL);
    pm_controller_mark_ui_active(&s_pm_ctrl, 1);
    pm_device_register_all();

    s_pm_ready = 1;
}

void pm_api_set_sleep_timeout(int timeout_sec)
{
    /* 秒 → 内部计数器重装载值(毫秒)：
     *   0 = 关闭自动休眠（设最大值永不超时），
     *   >0 = 设定超时秒数 */
    if (timeout_sec <= 0) {
        pm_sleep_timer_set(0xFFFFFFFFUL);
    } else {
        pm_sleep_timer_set((uint32_t)timeout_sec * 1000UL);
    }
}

void powerdown_task(void *pvParameters)
{
    pm_power_state_t state;
    pm_power_state_t prev_state = PM_STATE_RUN;
    uint8_t sleep_blocked = 0;

    (void)pvParameters;
    pm_task_init_once();

    while (1) {
        if (s_pm_refresh_req != 0) {
            pm_controller_refresh_idle(&s_pm_ctrl);
            s_pm_refresh_req = 0;
        }

        if (s_pm_block_mask != 0) {
            if (sleep_blocked == 0) {
                pm_controller_pause_idle(&s_pm_ctrl);
                sleep_blocked = 1;
            }
            pm_controller_notify_wake(&s_pm_ctrl);
        } else if (sleep_blocked != 0) {
            pm_controller_resume_idle(&s_pm_ctrl);
            sleep_blocked = 0;
        }

        /* 空闲超时：清除 UI 活跃标志，允许状态机离开 RUN（但禁用状态下不触发） */
        if (s_pm_refresh_req == 0 && pm_sleep_timer_expired() != 0
            && !pm_sleep_timer_is_disabled()) {
            pm_controller_mark_ui_active(&s_pm_ctrl, 0);
        }

        /* 唤醒静默期：递减计数器 */
        if (s_wk_btn > 0 && --s_wk_btn == 0) {
            pm_api_set_unstable_wake(0);  /* ~100ms 后恢复按键响应 */
        }

        state = pm_controller_step(&s_pm_ctrl);

#if PM_FEATURE_ENABLE
        /* 回到 RUN → 恢复 UI 任务（由 UI 任务自行调用 vTaskSuspend(NULL) 挂起） */
        if (state == PM_STATE_RUN && prev_state != PM_STATE_RUN) {
            s_wk_btn = 2;                    /* ~100ms 按键静默 */
            pm_api_set_unstable_wake(1);     /* 标记唤醒不稳定期 */
            vTaskResume(PM_UI_TASK_HANDLE);
        }
#endif
        prev_state = state;

        switch (state) {
        case PM_STATE_RUN:
            vTaskDelay(pdMS_TO_TICKS(50));
            break;
        case PM_STATE_UI_OFF:
            vTaskDelay(pdMS_TO_TICKS(100));
            break;
        case PM_STATE_SLEEP_PREPARE:
            vTaskDelay(pdMS_TO_TICKS(20));
            break;
        case PM_STATE_DEEPSLEEP:
            pm_enter_deep_sleep();
            vTaskDelay(pdMS_TO_TICKS(10));
            break;
        case PM_STATE_SLEEP_PENDING:
            vTaskDelay(pdMS_TO_TICKS(30));
            break;
        case PM_STATE_STANDBY:
            pm_enter_standby();
            vTaskDelay(pdMS_TO_TICKS(200));
            break;
        default:
            vTaskDelay(pdMS_TO_TICKS(50));
            break;
        }
    }
}
