#include "algorithm_control.h"
#include <math.h>
#include <armdsp.h>

static inline float PID_Clamp(float x, float min, float max)
{
    if (x > max) return max;
    if (x < min) return min;
    return x;
}


void PID_Init(ST_PID *pid,
              float Kp,
              float Ki,
              float Kd,
              float Dt,
              float Kaw)
{
    pid->fpKp = Kp;
    pid->fpKi = Ki;
    pid->fpKd = Kd;
    pid->fpDt = Dt;

    pid->fpKiDt = Ki * Dt;
    pid->fpKdDivDt = Kd / Dt;

    pid->fpKaw = Kaw;

    pid->fpErr = 0.0f;
    pid->fpPreErr = 0.0f;

    pid->fpIntegral = 0.0f;
    pid->fpOutput = 0.0f;
    pid->fpRawOutput = 0.0f;
}


void PID_SetParam(ST_PID *pid,
                  float Kp,
                  float Ki,
                  float Kd,
                  float Dt,
                  float Kaw)
{
    pid->fpKp = Kp;
    pid->fpKi = Ki;
    pid->fpKd = Kd;
    pid->fpDt = Dt;

    pid->fpKiDt = Ki * Dt;
    pid->fpKdDivDt = Kd / Dt;

    pid->fpKaw = Kaw;
}


void PID_Reset(ST_PID *pid)
{
    pid->fpErr = 0.0f;
    pid->fpPreErr = 0.0f;

    pid->fpIntegral = 0.0f;
    pid->fpOutput = 0.0f;
    pid->fpRawOutput = 0.0f;
}


float PID_Calc(ST_PID *pid,
               float Des,
               float FB,
               float OutMin,
               float OutMax)
{
    float err;
    float p_out;
    float i_out;
    float d_out;
    float raw_out;
    float sat_out;
    float aw_err;

    // 防止上下限传反
    if (OutMin > OutMax)
    {
        float temp = OutMin;
        OutMin = OutMax;
        OutMax = temp;
    }

    pid->fpPreErr = pid->fpErr;
    err = Des - FB;
    pid->fpErr = err;

    /*
     * 积分项直接以“输出单位”累加：
     *
     * 原来：
     *     integral += err * Dt;
     *     Iout = Ki * integral;
     *
     * 现在：
     *     integral += Ki * Dt * err;
     *
     * 所以 fpIntegral 本身就是 Iout。
     */
    i_out = pid->fpIntegral + pid->fpKiDt * err;

    p_out = pid->fpKp * err;
    d_out = pid->fpKdDivDt * (err - pid->fpPreErr);

    // 未限幅输出
    raw_out = p_out + i_out + d_out;

    // 实际输出限幅
    sat_out = PID_Clamp(raw_out, OutMin, OutMax);

    /*
     * 反算法 / 回算抗饱和：
     *
     * aw_err = sat_out - raw_out
     *
     * 当没有饱和时：
     *     sat_out == raw_out
     *     aw_err = 0
     *     积分项正常累加
     *
     * 当上限饱和时：
     *     sat_out < raw_out
     *     aw_err < 0
     *     积分项被往回拉
     *
     * 当下限饱和时：
     *     sat_out > raw_out
     *     aw_err > 0
     *     积分项被往回拉
     */
    aw_err = sat_out - raw_out;

    pid->fpIntegral = i_out + pid->fpKaw * aw_err;

    pid->fpRawOutput = raw_out;
    pid->fpOutput = sat_out;

    return sat_out;
}

/*******************************************************************
函数名称：PR_Controller
函数功能：比例谐振控制器
备    注：实际上是准PR，抗饱和算法使用香港理工大学论文《用于逆变器的比例-谐振控制器的抗饱和方案》
********************************************************************/
RAMFUNC void PR_Control(ST_PR *pstPR)
{
    float PR_Limit = pstPR->fpLim;
    
    pstPR->fpE = (pstPR->pre_is_cut) ? 
                ((pstPR->output_fpU - pstPR->fpU) * pstPR->InvKp + pstPR->fpPreE) : 
                (pstPR->fpDes - pstPR->fpFB);
    
	pstPR->fpU =    pstPR->n0* pstPR->fpE+
                    pstPR->n1* pstPR->fpPreE+
                    pstPR->n2* pstPR->fpPre_PreE-
                    pstPR->d1* pstPR->pre_fpU-
                    pstPR->d2* pstPR->pre_pre_fpU;
	
	pstPR->fpPre_PreE = pstPR->fpPreE;//迭代结果
	pstPR->fpPreE = pstPR->fpE; //保存本次偏差
	
    pstPR->output_fpU = fminf(fmaxf(pstPR->fpU, -PR_Limit), PR_Limit);

    pstPR->pre_is_cut = (pstPR->fpU != pstPR->output_fpU);
    
	pstPR->pre_pre_fpU = pstPR->pre_fpU;
	pstPR->pre_fpU = pstPR->fpU;
}

//计算PR算法离散参数/初始化
RAMFUNC void PR_Init(ST_PR *pstPR)
{		
    pstPR->InvKp = 1/ pstPR->Kp;
    pstPR->n0=((4+4*pstPR->fpDt*pstPR->omiga_c+pstPR->omiga_0*pstPR->omiga_0*pstPR->fpDt*pstPR->fpDt)*pstPR->Kp+4*pstPR->Kr*pstPR->fpDt*pstPR->omiga_c)/(4+4*pstPR->fpDt*pstPR->omiga_c+pstPR->omiga_0*pstPR->omiga_0*pstPR->fpDt*pstPR->fpDt);
    pstPR->n1=pstPR->Kp*(2*pstPR->omiga_0*pstPR->omiga_0*pstPR->fpDt*pstPR->fpDt-8)/(4+4*pstPR->fpDt*pstPR->omiga_c+pstPR->omiga_0*pstPR->omiga_0*pstPR->fpDt*pstPR->fpDt);
    pstPR->n2=((4-4*pstPR->fpDt*pstPR->omiga_c+pstPR->omiga_0*pstPR->omiga_0*pstPR->fpDt*pstPR->fpDt)*pstPR->Kp-4*pstPR->Kr*pstPR->fpDt*pstPR->omiga_c)/(4+4*pstPR->fpDt*pstPR->omiga_c+pstPR->omiga_0*pstPR->omiga_0*pstPR->fpDt*pstPR->fpDt);
    pstPR->d1=(2*pstPR->omiga_0*pstPR->omiga_0*pstPR->fpDt*pstPR->fpDt-8)/(4+4*pstPR->fpDt*pstPR->omiga_c+pstPR->omiga_0*pstPR->omiga_0*pstPR->fpDt*pstPR->fpDt);
    pstPR->d2=(4-4*pstPR->fpDt*pstPR->omiga_c+pstPR->omiga_0*pstPR->omiga_0*pstPR->fpDt*pstPR->fpDt)/(4+4*pstPR->fpDt*pstPR->omiga_c+pstPR->omiga_0*pstPR->omiga_0*pstPR->fpDt*pstPR->fpDt);
}

//重置PR控制器状态
RAMFUNC void PR_Clean(ST_PR *pstPR)
{
	pstPR->fpE = 0;
	pstPR->fpPreE = 0;
	pstPR->fpPre_PreE = 0;
	pstPR->fpU = 0;
	pstPR->pre_fpU = 0;
	pstPR->pre_pre_fpU = 0;
    pstPR->output_fpU = 0;
    pstPR->pre_is_cut = 0;
}
	
//PR控制器结构体示例	
/*
RAMVAR ST_PR PR_A={.fpE=0,.fpPreE=0,.fpPre_PreE=0,.fpU=0,.pre_fpU=0,.pre_pre_fpU=0,
	.omiga_c=0.628f,.omiga_0=DEFAULT_AC_OMEGA,.fpDt=CTRL_DT,.Kp=0.1f,.Kr=500             
};		
RAMVAR ST_PR PR_B={.fpE=0,.fpPreE=0,.fpPre_PreE=0,.fpU=0,.pre_fpU=0,.pre_pre_fpU=0,
	.omiga_c=0.628f,.omiga_0=DEFAULT_AC_OMEGA,.fpDt=CTRL_DT,.Kp=0.1f,.Kr=500               
};*/
