/*******************************************************************************
负责控制器的具体运行逻辑
*******************************************************************************/
#include "conv_controller.h"
#include "conv_loop.h"
#include "algorithm_control.h"
#include "algorithm_filtering.h"
#include <math.h>
#include <armdsp.h>

#define LOOP_FREQ   (DEFAULT_FSW_KHZ* 1000)             // 函数执行频率 Hz
#define PHASE_BITS  32                                  // 相位累加器位宽

int ACFrequency = DEFAULT_AC_FREQ;                      // 输出交流电频率
uint32_t PhaseACC = 0;
uint32_t fcw = (uint32_t)(((uint64_t)DEFAULT_AC_FREQ << PHASE_BITS) / LOOP_FREQ);

ConvStatus_t g_conv_state = CONV_STOP;                  //转换器正在运行标志变量

uint8_t LoopMode = 1;//1:能量回馈 0:单逆变

extern float soft_k;
extern float thph_offset;
extern ST_PR PR_A, PR_B, PFC_PR_A, PFC_PR_B, PFC_PR_C;

float sin0, sin240;
    
float v2_instant;
float buf[800] = {0};
float v2_sum;
float vrms_out;
uint32_t p_buf = 0;

/*******************************环路核心函数***********************************/
//取采样值，环路计算，保护，周期性统计
inline void MainLoop(void){
    // 在这里，DMA 已经把所有 ADC 转换的数据传输到预先定义的内存缓冲区中
    LL_GPIO_SetOutputPin(TEST1_GPIO_Port, TEST1_Pin);//拉高IO用于测试执行时间
    
    PhaseACC += fcw;//相位累加
    
    CORDIC_Write(PhaseACC);
    
    LoadControlVar();//从ADC采样后的数组中取得控制用的采样量
    if(g_conv_state == CONV_RUN){
        //Protection();//保护函数
        
        sin0 = CORDIC_Read();
        float sin120 = CORDIC_Get_sin120();
        sin240 = CORDIC_Get_sin240();
        
        //开环SVPWM输出（通过整流器桥臂）
        {
            //软启动
            if(soft_k < 1.0f) soft_k += SOFTSTART_SLOPE;
            if(thph_offset < 2.0f * MODULATION_MIDPOINT) thph_offset += MODULATION_SLOPE;
            
            float U_ref = 0.5f; //调制度（可后续改为VO_Set归一化）
            U_ref *= soft_k;
            
            float Va = U_ref * sin0;
            float Vb = U_ref * sin120;
            float Vc = U_ref * sin240;
            
            //SVPWM零序注入（与thph_loop中CalcInvCommonMode一致）
            float vmax = fmaxf(Va, fmaxf(Vb, Vc));
            float vmin = fminf(Va, fminf(Vb, Vc));
            float voffset = thph_offset * 0.5f - (vmax + vmin) * 0.25f;
            
            Va = 0.5f * Va + voffset;
            Vb = 0.5f * Vb + voffset;
            Vc = 0.5f * Vc + voffset;
            
            SetDuty_Rec(Va, Vb, Vc);
        }
        
        //LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1, sin0* 2000+ 2048U);//调试用：观察sin0波形
        //LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_2, CORDIC_Get_sin120()* 2000+ 2048U);
        //LL_DAC_ConvertData12RightAligned(DAC2, LL_DAC_CHANNEL_1, sin240* 2000+ 2048U);
        //thph_loop(VO_Set, sin0, sin240);
        //if(LoopMode) tri_PFC_Loop(IO_Set, sin0, CORDIC_Get_sin120(), sin240);
        PWM_Start_Inv();
        if(LoopMode) PWM_Start_Rec();
    }
    else{
        //停止输出
        PWM_Stop();
        //清除PR状态
        PR_Clean(&PR_A);
        PR_Clean(&PR_B);
        PR_Clean(&PFC_PR_A);
        PR_Clean(&PFC_PR_B);
        PR_Clean(&PFC_PR_C);
        //清除软启动状态
        soft_k = 0;
        thph_offset = MODULATION_START;
    }
           
    v2_instant = ReadControlVar(&UV_INV_t)* ReadControlVar(&UV_INV_t);
    v2_sum -= buf[p_buf];
    v2_sum += v2_instant;
    buf[p_buf] = v2_instant;
    p_buf++;
    if(p_buf >= 800) p_buf = 0;
    vrms_out = sqrtf(v2_sum/ 800);
    
    LL_GPIO_ResetOutputPin(TEST1_GPIO_Port, TEST1_Pin);//拉低IO用于测试执行时间
}

//避免在运行时修改能量回馈状态，并利用自动恢复机制
void SetEnergyFeedBack(ui_t *ui){
    if(g_conv_state == CONV_RUN) g_conv_state = CONV_FAULT;
}

//修改频率控制字函数，修改了PR的中心频率，需要重新计算所有PR参数
void ModifyFreqCtrlWord(ui_t *ui){
    fcw = (uint32_t)(((uint64_t)ACFrequency << PHASE_BITS)/ LOOP_FREQ);
    PR_A.omiga_0 = 2* 3.1416f* ACFrequency;
    PR_B.omiga_0 = PR_A.omiga_0;
    PFC_PR_A.omiga_0 = PR_A.omiga_0;
    PFC_PR_B.omiga_0 = PR_A.omiga_0;
    PFC_PR_C.omiga_0 = PR_A.omiga_0;
    PR_Init(&PR_A);
    PR_Init(&PR_B);
    PR_Init(&PFC_PR_A);
    PR_Init(&PFC_PR_B);
    PR_Init(&PFC_PR_C);
}
/***********************以下函数须在ui_conf.c中调用****************************/
ui_item_t ACFrequency_Item, FeedbackMode_Item;
//创建模式调整模块所需的参数
void Create_ConvMode_Parameters(ui_t *ui){
    static ui_data_t FeedbackMode_data;
    FeedbackMode_data.name = "Energy Feedback";
    FeedbackMode_data.ptr = &LoopMode;
    FeedbackMode_data.function = SetEnergyFeedBack;
    FeedbackMode_data.functionType = UI_DATA_FUNCTION_STEP_EXECUTE;
    FeedbackMode_data.dataType = UI_DATA_SWITCH;
    FeedbackMode_data.actionType = UI_DATA_ACTION_RW;
    static ui_element_t FeedbackMode_element;
    FeedbackMode_element.data = &FeedbackMode_data;
    Create_element(&FeedbackMode_Item, &FeedbackMode_element);
    
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
    AddItem(" Energy Feedback", UI_ITEM_DATA, NULL, &FeedbackMode_Item, ParentPage, NULL, NULL);
    AddItem(" AC Frequency", UI_ITEM_DATA, NULL, &ACFrequency_Item, ParentPage, NULL, NULL);
}

