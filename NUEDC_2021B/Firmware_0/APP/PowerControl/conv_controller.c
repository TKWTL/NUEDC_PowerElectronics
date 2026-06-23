/*******************************************************************************
负责控制器的具体运行逻辑
*******************************************************************************/
#include "conv_controller.h"
#include "conv_loop.h"
#include "conv_adc.h"
#include "algorithm_control.h"
#include "algorithm_filtering.h"
#include <math.h>
#include <armdsp.h>

#define LOOP_FREQ   (DEFAULT_FSW_KHZ* 1000)             // 函数执行频率 Hz
#define PHASE_BITS  32                                  // 相位累加器位宽

RAMVAR int ACFrequency = DEFAULT_AC_FREQ;                      // 输出交流电频率
RAMVAR uint32_t PhaseACC = 0;
RAMVAR uint32_t fcw = (uint32_t)(((uint64_t)DEFAULT_AC_FREQ << PHASE_BITS) / LOOP_FREQ);

RAMVAR ConvStatus_t g_conv_state = CONV_STOP;                  //转换器正在运行标志变量

RAMVAR float soft_k = 0;
RAMVAR float thph_offset;
extern ST_PR PFC_PR_A, PFC_PR_C;
extern ST_PID BUCK_V_PID;

RAMVAR float sin0, sin240;
    
float v2_instant;
float buf[800] = {0};
float v2_sum;
float vrms_out;
uint32_t p_buf = 0;
extern volatile uint16_t ADC2_Buffer[];
extern volatile uint16_t ADC3_Buffer[];
extern volatile uint16_t ADC4_Buffer[];

RAMVAR float inv_VBUS;

/*******************************环路核心函数***********************************/
//取采样值，环路计算，保护，周期性统计
inline void MainLoop(void){
    // 在这里，DMA 已经把所有 ADC 转换的数据传输到预先定义的内存缓冲区中
    LL_GPIO_SetOutputPin(TEST1_GPIO_Port, TEST1_Pin);//拉高IO用于测试执行时间
    
    PhaseACC += fcw;//相位累加
    
    CORDIC_Write(PhaseACC);
    LL_DAC_ConvertData12RightAligned(DAC2, LL_DAC_CHANNEL_1, ADC1_Buffer[0]);
    
    LoadControlVar();//从ADC采样后的数组中取得控制用的采样量
    inv_VBUS = 1.0f / ReadControlVar(&VBUS_t);
    if(g_conv_state == CONV_RUN){
        Protection();//保护函数
        
        sin0 = CORDIC_Read();
        float sin120 = CORDIC_Get_sin120();
        sin240 = CORDIC_Get_sin240();
        
        //开环DPWM-A输出（通过整流器桥臂）
        {
            //软启动（饱和递增）
            soft_k += SOFTSTART_SLOPE;
            if (soft_k > 1.0f) soft_k = 1.0f;
            
            thph_offset += MODULATION_SLOPE;
            if (thph_offset > 2.0f * MODULATION_MIDPOINT) thph_offset = 2.0f * MODULATION_MIDPOINT;
            
            float U_ref = 1.0f * soft_k;
            
            //原始三相调制波：范围 [-1, 1]
            float ma = U_ref * sin0;
            float mb = U_ref * sin120;
            float mc = U_ref * sin240;
            
            //DPWM-A零序注入（基于双极性调制波）
            float a0, a1, b0, b1, c0, c1;
            
            if (ma > 0.0f) { a0 = 1.0f - ma; a1 = ma; }
            else           { a0 = -ma;       a1 = 1.0f + ma; }
            
            if (mb > 0.0f) { b0 = 1.0f - mb; b1 = mb; }
            else           { b0 = -mb;       b1 = 1.0f + mb; }
            
            if (mc > 0.0f) { c0 = 1.0f - mc; c1 = mc; }
            else           { c0 = -mc;       c1 = 1.0f + mc; }
            
            float z0 = (a0 < b0) ? a0 : b0;
            z0 = (z0 < c0) ? z0 : c0;
            float z1 = (a1 < b1) ? a1 : b1;
            z1 = (z1 < c1) ? z1 : c1;
            
            float zero_sequence = (z0 > z1) ? (-z1) : z0;
            
            //零序注入，仍然是 [-1, 1] 范围附近
            ma += zero_sequence;
            mb += zero_sequence;
            mc += zero_sequence;
            
            //最后再映射到 [0, 1] 占空比（thph_offset/2 为调制中点）
            float Va = 0.5f * (thph_offset + ma);
            float Vb = 0.5f * (thph_offset + mb);
            float Vc = 0.5f * (thph_offset + mc);
            
            SetDuty_Rec(Va, Vb, Vc);
        }
        
        BUCK_loop(VO_Set, IO_Lim);
        //未来在此处添加整流环路函数，添加后删除该注释
        PWM_Start_Buck();
        //PWM_Start_Rec();
    }
    else{
        //停止输出
        PWM_Stop();
        //清除PR状态
        //PR_Clean(&PFC_PR_A);
        //PR_Clean(&PFC_PR_C);
        //清除软启动状态
        soft_k = 0;
        PID_Reset(&BUCK_V_PID);
        thph_offset = ReadControlVar(&VZS_t) * inv_VBUS;
    }

    /*v2_instant = ReadControlVar(&VAB_t)* ReadControlVar(&VAB_t);
    v2_sum -= buf[p_buf];
    v2_sum += v2_instant;
    buf[p_buf] = v2_instant;
    p_buf++;
    if(p_buf >= 800) p_buf = 0;
    vrms_out = sqrtf(v2_sum/ 800);*/
    
    LL_GPIO_ResetOutputPin(TEST1_GPIO_Port, TEST1_Pin);//拉低IO用于测试执行时间
}

//修改频率控制字函数，修改了PR的中心频率，需要重新计算所有PR参数
void ModifyFreqCtrlWord(ui_t *ui){
    //fcw = (uint32_t)(((uint64_t)ACFrequency << PHASE_BITS)/ LOOP_FREQ);
    PFC_PR_A.omiga_0 = 2* 3.1416f* ACFrequency;
    PFC_PR_C.omiga_0 = PFC_PR_A.omiga_0;
    PR_Init(&PFC_PR_A);
    PR_Init(&PFC_PR_C);
}
/***********************以下函数须在ui_conf.c中调用****************************/
ui_item_t ACFrequency_Item;
//创建模式调整模块所需的参数
void Create_ConvMode_Parameters(ui_t *ui){
    static ui_data_t ACFrequency_data;
    ACFrequency_data.name = "AC Frequency";
    ACFrequency_data.ptr = &ACFrequency;
    ACFrequency_data.function = ModifyFreqCtrlWord;
    ACFrequency_data.functionType = UI_DATA_FUNCTION_STEP_EXECUTE;
    ACFrequency_data.dataType = UI_DATA_INT;
    ACFrequency_data.actionType = UI_DATA_ACTION_RW;
    ACFrequency_data.max = 100;
    ACFrequency_data.min = 20;
    ACFrequency_data.step = 1;
    static ui_element_t ACFrequency_element;
    ACFrequency_element.data = &ACFrequency_data;
    Create_element(&ACFrequency_Item, &ACFrequency_element);
}
//将模式调整模块的对象添加到菜单中
void Add_ConvMode_Items(ui_page_t *ParentPage){
    AddItem(" AC Frequency", UI_ITEM_DATA, NULL, &ACFrequency_Item, ParentPage, NULL, NULL);
}

