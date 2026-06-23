#include "conv_loop.h"
#include "conv_controller.h"
#include "conv_adc.h"
#include "conv_pwm.h"
#include "algorithm_control.h"
#include "algorithm_filtering.h"
#include <math.h>
#include <armdsp.h>

float OutputA = 0;
float OutputB = 0;
float OutputC = 0;
float Uuv = 0;//UV线电压
float Uwv = 0;//WV线电压
float UuN = 0;// U相电压
float UvN = 0;// V相电压
float gain = 0.0;//电压前馈系数
int dpwm_state = 0;//DPWM状态
float pwm_cm = 0.0f;   // Inv/Rec 共用公共零序偏置
float mod_inv = 0.0f;  // 逆变器调制度，可用于调试观察


//Buck电压外环PID控制器
RAMVAR ST_PID BUCK_V_PID;
/*******************************************************************
函数名称：void BUCK_loop
函数功能：Buck变换器电流内环控制
备    注：电流内环单独调试用，外层电压PI后续添加
********************************************************************/
#define BUCK_I_KP     3.6f   //电流内环比例增益 V/A
inline void BUCK_loop(float Vref, float I_lim)
{
    float Vout = ReadControlVar(&VN_t);
    float I_avg = ReadControlVar(&IN_t);
    
    //电压外环PI → 电流参考（输出限幅 = 电流限幅）
    float Iref = PID_Calc(&BUCK_V_PID, Vref, Vout, -INV_3_F32, I_lim+ INV_3_F32);
    LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1, Iref*1024);
    //电流内环P
    float Ierr = Iref - I_avg;
    float delta_V = BUCK_I_KP * Ierr;
    
    //前馈：CCM Buck平均电压 + 环路修正
    float Vsw = Vout + delta_V;
    //标幺化到占空比
    float D = Vsw * inv_VBUS;
    
    SetDuty_Buck(D);
}

//三相开环逆变，带软启动功能
/*RAMFUNC void thph_open_loop(float U_ref, uint16_t index)
{
    float third_harmonic=0;
    
    U_ref /= 1.732f;
    
    OutputA = U_ref* Wave_Table[index]/ 50;
    //third_harmonic = -U_ref* Wave_Table[(index* 3)% 2049]/ 300;
    index += 683;
    if(index >= 2049) index -= 2049;
    OutputC = U_ref* Wave_Table[index]/ 50;
    OutputB = -OutputA-OutputC;
    
    third_harmonic += thph_offset;
		
//	SetDuty(0, third_harmonic+ OutputA, third_harmonic+ OutputB, third_harmonic+ OutputC);
    SetDuty_SV(0,thph_offset,OutputA,OutputB,OutputC);
}*/
RAMFUNC static inline float CalcModIndex_ABC(float a, float b, float c)
{
    return sqrtf(0.6666667f * (a * a + b * b + c * c));
}

RAMFUNC static inline float CalcInvCommonMode(float a, float b, float c)
{
    float vmax = fmaxf(a, fmaxf(b, c));
    float vmin = fminf(a, fminf(b, c));
    float cm;

    mod_inv = CalcModIndex_ABC(a, b, c);

    // DPWM 滞回切换
    if(dpwm_state == 0){
        if(mod_inv >= DPWM_APPLY_HYS_HI) dpwm_state = 1;
    }else{
        if(mod_inv <= DPWM_APPLY_HYS_LO) dpwm_state = 0;
    }

    if(dpwm_state == 0){
        // SVPWM / min-max 零序注入
        cm = thph_offset * 0.5f - (vmax + vmin) * 0.25f;
    }else{
        // DPWM：自动选上钳位 / 下钳位
        // 若 vmax+vmin >= 0，钳到上边界 thph_offset
        // 若 vmax+vmin <  0，钳到下边界 0
        cm = ((vmax + vmin) >= 0.0f) ? (thph_offset - 0.5f * vmax)
                                     : (-0.5f * vmin);
    }

    pwm_cm = cm;
    return cm;
}
	
/*    Uuv = ReadControlVar(&VAB_t);
    Uwv = ReadControlVar(&VBC_t);
    UuN = (Uuv*2-Uwv)/ 3.0f;//线电压转相电压
    UvN = (Uwv+Uuv)/ -3.0f; 
    gain = 1.0f/ ReadControlVar(&VBUS_t);//获取母线电压信息用于抗饱和和标幺化*/

/*******************************************************************
三相整流PR结构体，此处调参
********************************************************************/
RAMVAR ST_PR PFC_PR_A={.fpE=0,.fpPreE=0,.fpPre_PreE=0,.fpU=0,.pre_fpU=0,.pre_pre_fpU=0,
	.omiga_c=0.628f,.omiga_0 = DEFAULT_AC_OMEGA,.fpDt=CTRL_DT,.Kp=3,.Kr=800              
};		
RAMVAR ST_PR PFC_PR_B={.fpE=0,.fpPreE=0,.fpPre_PreE=0,.fpU=0,.pre_fpU=0,.pre_pre_fpU=0,
	.omiga_c=0.628f,.omiga_0 = DEFAULT_AC_OMEGA,.fpDt=CTRL_DT,.Kp=3,.Kr=800              
};		
RAMVAR ST_PR PFC_PR_C={.fpE=0,.fpPreE=0,.fpPre_PreE=0,.fpU=0,.pre_fpU=0,.pre_pre_fpU=0,
	.omiga_c=0.628f,.omiga_0 = DEFAULT_AC_OMEGA,.fpDt=CTRL_DT,.Kp=3,.Kr=800              
};	

/*******************************************************************
函数名称：void tri_PFC_Loop(float I_ref)
函数功能：进行三相PFC控制
备    注：零序分量（因为直接并联）和U，V，W电压由逆变器端提供（为了节省采样电路功耗），必须在thph_loop()后执行
********************************************************************/
float I_Output_A=0;
float I_Output_B=0;
float I_Output_C=0;
float UuN_P=0;
float UvN_P=0;
float UwN_P=0;
inline void tri_PFC_Loop(float I_ref, float sin0, float sin120, float sin240)
{    
    I_ref = I_ref* 1.0198f- 0.028f;//神秘校准用数字
    I_ref *= soft_k;
    I_ref *= 1.414f;
    
    PFC_PR_A.fpDes = I_ref* sin0;
    PFC_PR_C.fpDes = I_ref* sin120;
    PFC_PR_B.fpDes = I_ref* sin240;
    

    UuN_P = (Uuv* 2- Uwv)/ 3.0f; 
    UvN_P = (Uwv+ Uuv)/ -3.0f;
    UwN_P = (Uuv- 2* Uwv)/ -3.0f;
    
    PFC_PR_A.fpFB = ReadControlVar(&VZS_t);
    PFC_PR_A.fpLim = ReadControlVar(&VBUS_t)* ANTI_WINDUP_LIMIT;
    PFC_PR_C.fpFB = ReadControlVar(&VN_t);
    PFC_PR_C.fpLim = PFC_PR_A.fpLim;
    
    PR_Control(&PFC_PR_A);
    PR_Control(&PFC_PR_B);
    PR_Control(&PFC_PR_C);
    //标幺化
    I_Output_A = (UuN_P- PFC_PR_A.output_fpU)* gain; 
    I_Output_B = (UvN_P- PFC_PR_B.output_fpU)* gain; 
    I_Output_C = (UwN_P- PFC_PR_C.output_fpU)* gain;
    //使用与逆变器相等的零序分量
    I_Output_A = 0.5f * I_Output_A + pwm_cm;
    I_Output_B = 0.5f * I_Output_B + pwm_cm;
    I_Output_C = 0.5f * I_Output_C + pwm_cm;
    
    SetDuty_Rec(I_Output_A, I_Output_B, I_Output_C);
}

