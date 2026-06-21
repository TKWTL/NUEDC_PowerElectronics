#include "pm_ui_register.h"

/* 页面 & 项目 */
static ui_page_t Sleep_Page;
static ui_item_t Sleep_Item, SleepHead_Item;

/* 每个选项的选中状态（Switch_Widget 多选一机制：min==1 → radio 模式） */
static uint8_t opt_off_st = 1;    /* 默认选中 */
static uint8_t opt_15s_st = 0;
static uint8_t opt_30s_st = 0;
static uint8_t opt_1m_st  = 0;
static uint8_t opt_2m_st  = 0;
static uint8_t opt_5m_st  = 0;
static uint8_t opt_10m_st = 0;
static uint8_t opt_15m_st = 0;
static uint8_t opt_30m_st = 0;

/* 每个选项的 ui_item_t */
static ui_item_t Opt_Off_Item, Opt_15s_Item, Opt_30s_Item,
                 Opt_1m_Item,  Opt_2m_Item,  Opt_5m_Item,
                 Opt_10m_Item, Opt_15m_Item, Opt_30m_Item;

/* ── 回调函数 ── */
static void cb_off(ui_t *ui)  { (void)ui; pm_api_set_sleep_timeout(0); }
static void cb_15s(ui_t *ui)  { (void)ui; pm_api_set_sleep_timeout(15); }
static void cb_30s(ui_t *ui)  { (void)ui; pm_api_set_sleep_timeout(30); }
static void cb_1m(ui_t *ui)   { (void)ui; pm_api_set_sleep_timeout(60); }
static void cb_2m(ui_t *ui)   { (void)ui; pm_api_set_sleep_timeout(120); }
static void cb_5m(ui_t *ui)   { (void)ui; pm_api_set_sleep_timeout(300); }
static void cb_10m(ui_t *ui)  { (void)ui; pm_api_set_sleep_timeout(600); }
static void cb_15m(ui_t *ui)  { (void)ui; pm_api_set_sleep_timeout(900); }
static void cb_30m(ui_t *ui)  { (void)ui; pm_api_set_sleep_timeout(1800); }

/* ── 使用固定数组确保每个选项有独立的 data/element ── */
static ui_data_t    s_opt_data[9];
static ui_element_t s_opt_elem[9];
static int          s_opt_idx;

static void make_opt(const char *name, uint8_t *st,
                     ui_item_t *item, ui_item_function cb)
{
    int i = s_opt_idx++;
    s_opt_data[i].name         = name;
    s_opt_data[i].ptr          = st;
    s_opt_data[i].function     = cb;
    s_opt_data[i].functionType = UI_DATA_FUNCTION_STEP_EXECUTE;
    s_opt_data[i].dataType     = UI_DATA_SWITCH;
    s_opt_data[i].actionType   = UI_DATA_ACTION_RW;
    s_opt_data[i].min          = 1;      /* min==1 → radio 模式 */
    s_opt_data[i].max          = 1;      /* 组 ID（同组只能选一个） */
    s_opt_data[i].step         = 0;
    s_opt_elem[i].data = &s_opt_data[i];
    Create_element(item, &s_opt_elem[i]);
}

void Create_Sleep_Parameters(ui_t *ui)
{
    (void)ui;
    s_opt_idx = 0;
    make_opt(" No Auto Sleep", &opt_off_st, &Opt_Off_Item,   cb_off);
    make_opt(" 15s",        &opt_15s_st, &Opt_15s_Item,   cb_15s);
    make_opt(" 30s",        &opt_30s_st, &Opt_30s_Item,   cb_30s);
    make_opt(" 1min",       &opt_1m_st,  &Opt_1m_Item,    cb_1m);
    make_opt(" 2min",       &opt_2m_st,  &Opt_2m_Item,    cb_2m);
    make_opt(" 5min",       &opt_5m_st,  &Opt_5m_Item,    cb_5m);
    make_opt(" 10min",      &opt_10m_st, &Opt_10m_Item,   cb_10m);
    make_opt(" 15min",      &opt_15m_st, &Opt_15m_Item,   cb_15m);
    make_opt(" 30min",      &opt_30m_st, &Opt_30m_Item,   cb_30m);
}

void Add_Sleep_Items(ui_page_t *ParentPage)
{
    AddItem("-Sleep", UI_ITEM_PARENTS, img_toby_fox,
            &Sleep_Item, ParentPage, &Sleep_Page, NULL);
        AddPage("[Sleep]", &Sleep_Page, UI_PAGE_TEXT, ParentPage);
            AddItem("[Back]", UI_ITEM_RETURN, NULL, &SleepHead_Item,
                    &Sleep_Page, ParentPage, NULL);
            AddItem(" No Auto Sleep", UI_ITEM_DATA, NULL, &Opt_Off_Item,
                    &Sleep_Page, NULL, NULL);
            AddItem(" 15s", UI_ITEM_DATA, NULL, &Opt_15s_Item,
                    &Sleep_Page, NULL, NULL);
            AddItem(" 30s", UI_ITEM_DATA, NULL, &Opt_30s_Item,
                    &Sleep_Page, NULL, NULL);
            AddItem(" 1min", UI_ITEM_DATA, NULL, &Opt_1m_Item,
                    &Sleep_Page, NULL, NULL);
            AddItem(" 2min", UI_ITEM_DATA, NULL, &Opt_2m_Item,
                    &Sleep_Page, NULL, NULL);
            AddItem(" 5min", UI_ITEM_DATA, NULL, &Opt_5m_Item,
                    &Sleep_Page, NULL, NULL);
            AddItem(" 10min", UI_ITEM_DATA, NULL, &Opt_10m_Item,
                    &Sleep_Page, NULL, NULL);
            AddItem(" 15min", UI_ITEM_DATA, NULL, &Opt_15m_Item,
                    &Sleep_Page, NULL, NULL);
            AddItem(" 30min", UI_ITEM_DATA, NULL, &Opt_30m_Item,
                    &Sleep_Page, NULL, NULL);
}
