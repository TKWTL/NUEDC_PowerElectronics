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
float Vab = 0;//A-B线电压 (Va - Vb)
float Vbc = 0;//B-C线电压 (Vb - Vc)
float Va = 0;// A相电压
float Vb = 0;// B相电压
float gain = 0.0;//电压前馈系数
int dpwm_state = 0;//DPWM状态
float pwm_cm = 0.0f;   // Inv/Rec 共用公共零序偏置
float mod_inv = 0.0f;  // 逆变器调制度，可用于调试观察


//Buck电压外环PID控制器
RAMVAR ST_PID BUCK_V_PID;
//PFC母线电压外环PID控制器
RAMVAR ST_PID PFC_V_PID;
extern float VO_Set;
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
    float Iref = PID_Calc(&BUCK_V_PID, Vref, Vout, -INV_3_F32, I_lim+ INV_5_F32);
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
	
/*    Vab = ReadControlVar(&VAB_t);
    Vbc = ReadControlVar(&VBC_t);
    Va = (Vab*2-Vbc)/ 3.0f;//线电压转相电压
    Vb = (Vbc+Vab)/ -3.0f; 
    gain = 1.0f/ ReadControlVar(&VBUS_t);*/

/*******************************************************************
三相整流PR结构体，此处调参
********************************************************************/
RAMVAR ST_PR PFC_PR_A={.fpE=0,.fpPreE=0,.fpPre_PreE=0,.fpU=0,.pre_fpU=0,.pre_pre_fpU=0,
	.omiga_c=0.628f,.omiga_0 = DEFAULT_AC_OMEGA,.fpDt=CTRL_DT,.Kp=3,.Kr=800              
};		
RAMVAR ST_PR PFC_PR_C={.fpE=0,.fpPreE=0,.fpPre_PreE=0,.fpU=0,.pre_fpU=0,.pre_pre_fpU=0,
	.omiga_c=0.628f,.omiga_0 = DEFAULT_AC_OMEGA,.fpDt=CTRL_DT,.Kp=3,.Kr=800              
};	

float Va_calc=0;
float Vb_calc=0;
float Vc_calc=0;
float Va_rms=0;
float Vc_rms=0;

float Va_buf[V_RMS_LEN];
float Vc_buf[V_RMS_LEN];
SlideRms_t Va_rms_inst;                 //A相有效值句柄
SlideRms_t Vc_rms_inst;                 //C相有效值句柄

/*******************************************************************
函数名称：void tri_PFC_Loop
函数功能：线电压→相电压、滑动窗口RMS、PF延迟读取
备    注：pf_x100 = PF × 100 (80~100)
********************************************************************/
inline void tri_PFC_Loop(uint8_t pf_x100)
{    
    //PF延迟读取，DAC输出移相后的A相电压用于验证
    uint8_t lag = pf_to_lag(pf_x100);
    float Va_delayed = SlideRms_ReadDelayed(&Va_rms_inst, lag);
    LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_2,
        (uint32_t)((Va_delayed / Va_rms)* 1414.0f + 2048.0f));
    float Vc_delayed = SlideRms_ReadDelayed(&Vc_rms_inst, lag);
    
    //母线电压外环 → 直流参考电流 Iref（复用PID控制代码）
    float vbus = ReadControlVar(&VBUS_t);
    float vbus_ref = fmaxf(fmaxf(SQRT6_F32 * Va_rms, SQRT6_F32 * Vc_rms), VO_Set) + 4.0f;
    float Iref = PID_Calc(&PFC_V_PID, vbus_ref, vbus, -0.5f, 4.0f);
    
    //相电压标幺化 → 电流参考（与电网电压同相的正弦）
    float Iref_A = (Va_delayed / Va_rms) * Iref;
    float Iref_C = (Vc_delayed / Vc_rms) * Iref;
    
    //PR电流内环（A相）
    PFC_PR_A.fpDes = Iref_A;
    PFC_PR_A.fpFB  = -ReadControlVar(&IA_t);
    PFC_PR_A.fpLim = ANTI_WINDUP_LIMIT * vbus;       //抗饱和限幅
    PR_Control(&PFC_PR_A);
    
    //PR电流内环（C相）
    PFC_PR_C.fpDes = Iref_C;
    PFC_PR_C.fpFB  = -ReadControlVar(&IC_t);
    PFC_PR_C.fpLim = PFC_PR_A.fpLim;
    PR_Control(&PFC_PR_C);
    
    //调制波：PR输出 + 电压前馈，B相由基尔霍夫定律得出
    float ma = (Va_calc + PFC_PR_A.output_fpU) * inv_VBUS;
    float mc = (Vc_calc + PFC_PR_C.output_fpU) * inv_VBUS;
    float mb = -(ma + mc);
    
    //DPWM-A零序注入（已经是[-1,1]范围，复用开环算法）
    float a0, a1, b0, b1, c0, c1;
    if (ma > 0.0f) { a0 = 1.0f - ma; a1 = ma; }
    else           { a0 = -ma;       a1 = 1.0f + ma; }
    if (mb > 0.0f) { b0 = 1.0f - mb; b1 = mb; }
    else           { b0 = -mb;       b1 = 1.0f + mb; }
    if (mc > 0.0f) { c0 = 1.0f - mc; c1 = mc; }
    else           { c0 = -mc;       c1 = 1.0f + mc; }
    
    float z0 = fminf(fminf(a0, b0), c0);
    float z1 = fminf(fminf(a1, b1), c1);
    float zero_seq = (z0 > z1) ? (-z1) : z0;
    
    ma += zero_seq;
    mb += zero_seq;
    mc += zero_seq;
    
    //映射到[0,1]占空比
    float duty_a = 0.5f * (thph_offset + ma);
    float duty_b = 0.5f * (thph_offset + mb);
    float duty_c = 0.5f * (thph_offset + mc);
    
    //SetDuty_Rec(duty_a, duty_b, duty_c);
}

