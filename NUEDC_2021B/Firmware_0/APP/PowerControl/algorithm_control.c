#include "algorithm_control.h"
#include <math.h>
#include <armdsp.h>

void PID_Init(ST_PID *pid, float Kp, float Ki, float Kd, float Dt, float Lim)
{
    pid->fpKp = Kp;
    pid->fpKi = Ki;
    pid->fpKd = Kd;
    pid->fpDt = Dt;
    pid->fpLim = Lim;
    pid->fpErr = 0;
    pid->fpPreErr = 0;
    pid->fpIntegral = 0;
    pid->fpOutput = 0;
    pid->fpPreOutput = 0;
}

float PID_Calc(ST_PID *pid, float Des, float FB)
{
    pid->fpPreErr = pid->fpErr;
    pid->fpErr = Des - FB;
    
    pid->fpIntegral += pid->fpErr * pid->fpDt;
    
    // 积分限幅
    if(pid->fpIntegral > pid->fpLim) pid->fpIntegral = pid->fpLim;
    if(pid->fpIntegral < -pid->fpLim) pid->fpIntegral = -pid->fpLim;
    
    float output = pid->fpKp * pid->fpErr 
                + pid->fpKi * pid->fpIntegral 
                + pid->fpKd * (pid->fpErr - pid->fpPreErr) / pid->fpDt;
    
    // 输出限幅
    if(output > pid->fpLim) output = pid->fpLim;
    if(output < -pid->fpLim) output = -pid->fpLim;
    
    pid->fpPreOutput = pid->fpOutput;
    pid->fpOutput = output;
    
    return output;
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
