/*
 *重新封装了HAL库的时间相关格式
 *避免在默认的packed数据结构下M0+的非对齐访问出现Hardfault
 */
#include "time.h"
#include "image.h"//使用的图片资源

RTC_TimeTypeDef_Aligned sTime;//时间全局变量，值全部4字节对齐以避免M0+内核的Hardfault错误

RTC_TimeTypeDef RTC_Time;//时间全局变量，与HAL库的接口
RTC_DateTypeDef RTC_Date;

void RTC_UpdateTime(void){//从RTC更新时间
    HAL_RTC_GetTime(&hrtc, &RTC_Time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &RTC_Date, RTC_FORMAT_BIN);
    sTime.Year = RTC_Date.Year;
    sTime.Month = RTC_Date.Month;
    sTime.Date = RTC_Date.Date;
    sTime.WeekDay = RTC_Date.WeekDay;
    sTime.Hours = RTC_Time.Hours;
    sTime.Minutes = RTC_Time.Minutes;
    sTime.Seconds = RTC_Time.Seconds;
}

void SetTime(ui_t *ui){//将时间更新到RTC
    RTC_Date.Year = sTime.Year;
    RTC_Date.Month = sTime.Month;
    RTC_Date.Date = sTime.Date;
    RTC_Date.WeekDay = sTime.WeekDay;
    HAL_RTC_SetDate(&hrtc, &RTC_Date, RTC_FORMAT_BIN);
    RTC_Time.Hours = sTime.Hours;
    RTC_Time.Minutes = sTime.Minutes;
    RTC_Time.Seconds = sTime.Seconds;
    HAL_RTC_SetTime(&hrtc, &RTC_Time, RTC_FORMAT_BIN);
}

/***********************以下函数须在ui_conf.c中调用****************************/
ui_item_t Time_Item, TimeHead_Item, Year_Item, Month_Item, Days_Item, Hour_Item, Minute_Item, Second_Item;
//创建显示屏模块所需的参数
void Create_TimeSet_Parameters(ui_t *ui){
    static ui_data_t Year_data;
    Year_data.name = "Year";
    Year_data.ptr = &sTime.Year;
    Year_data.function = SetTime;
    Year_data.functionType = UI_DATA_FUNCTION_STEP_EXECUTE;
    Year_data.dataType = UI_DATA_INT;
    Year_data.actionType = UI_DATA_ACTION_RW;
    Year_data.max = 99;
    Year_data.min = 0;
    Year_data.step = 1;
    static ui_element_t Year_element;
    Year_element.data = &Year_data;
    Create_element(&Year_Item, &Year_element);
    
    static ui_data_t Month_data;
    Month_data.name = "Month";
    Month_data.ptr = &sTime.Month;
    Month_data.function = SetTime;
    Month_data.functionType = UI_DATA_FUNCTION_STEP_EXECUTE;
    Month_data.dataType = UI_DATA_INT;
    Month_data.actionType = UI_DATA_ACTION_RW;
    Month_data.max = 12;
    Month_data.min = 1;
    Month_data.step = 1;
    static ui_element_t Month_element;
    Month_element.data = &Month_data;
    Create_element(&Month_Item, &Month_element);
    
    static ui_data_t Days_data;
    Days_data.name = "Date";
    Days_data.ptr = &sTime.Date;
    Days_data.function = SetTime;
    Days_data.functionType = UI_DATA_FUNCTION_STEP_EXECUTE;
    Days_data.dataType = UI_DATA_INT;
    Days_data.actionType = UI_DATA_ACTION_RW;
    Days_data.max = 31;
    Days_data.min = 1;
    Days_data.step = 1;
    static ui_element_t Days_element;
    Days_element.data = &Days_data;
    Create_element(&Days_Item, &Days_element);
    
    static ui_data_t Hour_data;
    Hour_data.name = "Hour";
    Hour_data.ptr = &sTime.Hours;
    Hour_data.function = SetTime;
    Hour_data.functionType = UI_DATA_FUNCTION_STEP_EXECUTE;
    Hour_data.dataType = UI_DATA_INT;
    Hour_data.actionType = UI_DATA_ACTION_RW;
    Hour_data.max = 23;
    Hour_data.min = 0;
    Hour_data.step = 1;
    static ui_element_t Hour_element;
    Hour_element.data = &Hour_data;
    Create_element(&Hour_Item, &Hour_element);
    
    static ui_data_t Minute_data;
    Minute_data.name = "Minute";
    Minute_data.ptr = &sTime.Minutes;
    Minute_data.function = SetTime;
    Minute_data.functionType = UI_DATA_FUNCTION_STEP_EXECUTE;
    Minute_data.dataType = UI_DATA_INT;
    Minute_data.actionType = UI_DATA_ACTION_RW;
    Minute_data.max = 59;
    Minute_data.min = 0;
    Minute_data.step = 1;
    static ui_element_t Minute_element;
    Minute_element.data = &Minute_data;
    Create_element(&Minute_Item, &Minute_element);
    
    static ui_data_t Second_data;
    Second_data.name = "Second";
    Second_data.ptr = &sTime.Seconds;
    Second_data.function = SetTime;
    Second_data.functionType = UI_DATA_FUNCTION_STEP_EXECUTE;
    Second_data.dataType = UI_DATA_INT;
    Second_data.actionType = UI_DATA_ACTION_RW;
    Second_data.max = 59;
    Second_data.min = 0;
    Second_data.step = 1;
    static ui_element_t Second_element;
    Second_element.data = &Second_data;
    Create_element(&Second_Item, &Second_element);
}

ui_page_t Time_Page;
//将显示屏模块的对象添加到菜单中
void Add_TimeSet_Items(ui_page_t *ParentPage){
    AddItem("-Time & Date", UI_ITEM_PARENTS, img_date, &Time_Item, ParentPage, &Time_Page, NULL);
        AddPage("[Time]", &Time_Page, UI_PAGE_TEXT, ParentPage);
            AddItem("[Back]", UI_ITEM_RETURN, NULL, &TimeHead_Item, &Time_Page, ParentPage, NULL);
            AddItem(" Second", UI_ITEM_DATA, NULL, &Second_Item, &Time_Page, NULL, NULL);
            AddItem(" Minute", UI_ITEM_DATA, NULL, &Minute_Item, &Time_Page, NULL, NULL);
            AddItem(" Hour", UI_ITEM_DATA, NULL, &Hour_Item, &Time_Page, NULL, NULL);
            AddItem(" Date", UI_ITEM_DATA, NULL, &Days_Item, &Time_Page, NULL, NULL);
            AddItem(" Month", UI_ITEM_DATA, NULL, &Month_Item, &Time_Page, NULL, NULL);
            AddItem(" Year", UI_ITEM_DATA, NULL, &Year_Item, &Time_Page, NULL, NULL);
}
