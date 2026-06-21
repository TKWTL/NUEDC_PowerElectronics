/*系统数据更新文件
 *负责更新系统状态
 *并且在需要的时候触发弹窗
 */
/*所用引脚的宏定义在mian.h文件中，由CubeMX自动生成
 *
 *
 */
#include "system_statistic.h"
#include "stm32g4xx_ll_adc.h"   // for __LL_ADC_CALC_TEMPERATURE


//ADC转换通道枚举，需要与CubeMX中的已使能通道次序对应
typedef enum{
    CH_TJ,
    CH_VBAT,
    CH_VREF,
    
    CH_TOTAL//代表总通道数，无需修改
}ADC_ChannelNO_t;

uint16_t adc_buffer[CH_TOTAL+ 2];//存放ADC转换结果的数组
uint32_t adc_accumulator[CH_TOTAL];//累加ADC转换结果的数组，用于均值滤波

float Tjunction = 15.0f;    //MCU结温，单位摄氏度    
float Vbattery = 3.1f;      //电池电压
float Vintref = 1.2f;       //内部参考电压值

#define OVERSAMPLE_RATE     4   //均值滤波平均次数
#define SAMPLE_PERIOD       400//采样周期，单位ms

#define VBAT_DIV_RATIO      3.137f  // 电池分压比 (实测校准: 3.058V/0.975V)
#define TJ_CAL_OFFSET       12.0f    // 结温校准偏移量, 单位°C (若持续偏低可设为正值)

/*
 *采样ADC值并加以滤波
 */
void ADC_SampleandFilter(void){
    static uint8_t i = 0;
    uint8_t j;
    
    switch(i){
        case OVERSAMPLE_RATE://一轮采样完成，滤波并更新
        {
            float avg_tj   = (float)adc_accumulator[CH_TJ]   / (float)OVERSAMPLE_RATE;
            float avg_vbat = (float)adc_accumulator[CH_VBAT] / (float)OVERSAMPLE_RATE;
            float avg_vref = (float)adc_accumulator[CH_VREF] / (float)OVERSAMPLE_RATE;
            
            //=== 结温计算 (使用 LL 官方宏，与 ST 公式完全一致) ===
            // 将 ADC 读数从 Vref+ 归一化到校准参考电压 3.0V
            float tj_norm = avg_tj * ADC_VREF * 1000.0f / (float)TEMPSENSOR_CAL_VREFANALOG;
            float cal1_val = (float)(*TEMPSENSOR_CAL1_ADDR);
            float cal_diff = (float)(*TEMPSENSOR_CAL2_ADDR - *TEMPSENSOR_CAL1_ADDR);
            if(cal_diff != 0.0f) {
                Tjunction = (float)(TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP)
                          * (tj_norm - cal1_val) / cal_diff
                          + (float)TEMPSENSOR_CAL1_TEMP
                          + TJ_CAL_OFFSET;
            } else {
                Tjunction = (float)TEMPSENSOR_CAL1_TEMP + TJ_CAL_OFFSET;
            }
            
            //=== 电池电压 (分压比已实测校准) ===
            Vbattery = avg_vbat * VBAT_DIV_RATIO * ADC_VREF / 4096.0f;
            
            //=== VREFINT电压 ===
            Vintref = avg_vref * ADC_VREF / 4096.0f;
        
            for(j = 0; j < CH_TOTAL; j++) adc_accumulator[j] = 0;
            osDelay(pdMS_TO_TICKS(SAMPLE_PERIOD/ (OVERSAMPLE_RATE+ 1)));
            i = 0;
            break;
        }
        
        default://采样周期
            HAL_ADC_Start_DMA(&hadc5, (uint32_t*)adc_buffer, CH_TOTAL);
            osDelay(pdMS_TO_TICKS(SAMPLE_PERIOD/ (OVERSAMPLE_RATE+ 1)));//直接延时等待采样完成
            for(j = 0; j < CH_TOTAL; j++) adc_accumulator[j] += adc_buffer[j];
            i++;
            break;
    }
}
    
/***********************以下函数须在ui_conf.c中调用****************************/
ui_item_t Statistic_Item, StatHead_Item;
ui_item_t VBAT_Item, Tj_Item, Vrefint_Item;
//创建系统参数模块所需的参数
void Create_Statistics_Parameters(ui_t *ui){
    static ui_data_t TJ_data;
    TJ_data.name = "Tj";
    TJ_data.ptr = &Tjunction;
    TJ_data.dataType = UI_DATA_FLOAT;
    TJ_data.actionType = UI_DATA_ACTION_RO;
    static ui_element_t TJ_element;
    TJ_element.data = &TJ_data;
    Create_element(&Tj_Item, &TJ_element);
    
    static ui_data_t VBAT_data;
    VBAT_data.name = "vbkup";
    VBAT_data.ptr = &Vbattery;
    VBAT_data.dataType = UI_DATA_FLOAT;
    VBAT_data.actionType = UI_DATA_ACTION_RO;
    static ui_element_t VBAT_element;
    VBAT_element.data = &VBAT_data;
    Create_element(&VBAT_Item, &VBAT_element);
    
    static ui_data_t Vrefint_data;
    Vrefint_data.name = "vrefint";
    Vrefint_data.ptr = &Vintref;
    Vrefint_data.dataType = UI_DATA_FLOAT;
    Vrefint_data.actionType = UI_DATA_ACTION_RO;
    static ui_element_t Vrefint_element;
    Vrefint_element.data = &Vrefint_data;
    Create_element(&Vrefint_Item, &Vrefint_element);
}

ui_page_t Stat_Page;
//将系统参数模块的对象添加到菜单中
void Add_Statistics_Items(ui_page_t *ParentPage){
    AddItem("-Statistics", UI_ITEM_PARENTS, img_statistics, &Statistic_Item, ParentPage, &Stat_Page, NULL);
        AddPage("[Statistics]", &Stat_Page, UI_PAGE_TEXT, ParentPage);
            AddItem("[Menu]", UI_ITEM_RETURN, NULL, &StatHead_Item, &Stat_Page, ParentPage, NULL);
            AddItem(" CPU Temprature", UI_ITEM_DATA, NULL, &Tj_Item, &Stat_Page, NULL, NULL);
            AddItem(" Vbackup", UI_ITEM_DATA, NULL, &VBAT_Item, &Stat_Page, NULL, NULL);
            AddItem(" Internal VREF", UI_ITEM_DATA, NULL, &Vrefint_Item, &Stat_Page, NULL, NULL);
}

